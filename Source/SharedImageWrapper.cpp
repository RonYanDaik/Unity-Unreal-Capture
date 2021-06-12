#include "SharedImageWrapper.h"


#include "shared.inl"

using namespace std;

namespace mray
{

SharedImageWrapper::SharedImageWrapper() {
	totalTime = 0;
	m_sender = new SharedImageMemory(1);
	_start = std::chrono::system_clock::now();
}

SharedImageWrapper::~SharedImageWrapper()
{
	delete m_sender;
}
bool SharedImageWrapper::Ready()
{
	auto now = std::chrono::system_clock::now();
	auto diff = now - _prev;
	long long diffInt = std::chrono::duration_cast<std::chrono::milliseconds>(diff).count();
	if (diffInt < 1000 / 30)
		return false;
	return true;
}


bool SharedImageWrapper::SendImage(const unsigned char* data, int width, int height) {

	if (!Ready())
		return false;
	auto now = std::chrono::system_clock::now();
	_prev = now;
	long long diffInt = std::chrono::duration_cast<std::chrono::milliseconds>(now - _start).count();

	//push the captured data to directshow filter
	HRESULT hr = m_sender->SendUnrl(diffInt, width, height, data);

	return true;
}
 
	SharedImageWrapper::ESendResultWr SharedImageWrapper::SendImage2(int width, 
		int height, 
		int stride, 
		DWORD DataSize, 
		EFormatWr format,
		SharedImageWrapper::EResizeModeWr resizemode, 
		SharedImageWrapper::EMirrorModeWr mirrormode, int timeout, const uint8_t* buffer)
	
	{

	//if (!Ready()) return ESendResultWr::SENDRES_TOOLARGE;
	//auto now = std::chrono::system_clock::now();
	//_prev = now;
	//long long diffInt = std::chrono::duration_cast<std::chrono::milliseconds>(now - _start).count();

	//push the captured data to directshow filter

	return (SharedImageWrapper::ESendResultWr)m_sender->Send( width, 
		 height, 
		 stride, 
		 DataSize, 
		 (SharedImageMemory::EFormat) format,
		 SharedImageMemory::RESIZEMODE_DISABLED, 
		 SharedImageMemory::MIRRORMODE_DISABLED,  
		 timeout,  buffer);

	//return SharedImageWrapper::ESendResultWr::SENDRES_OK;
}
}