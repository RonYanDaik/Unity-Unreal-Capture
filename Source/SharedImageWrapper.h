#ifndef HELLOLIB_H__
#define HELLOLIB_H__


#include <chrono>
#include <intsafe.h>

struct SharedImageMemory;

namespace mray
{
	
	
class  SharedImageWrapper {
	
private:
	long long totalTime;
	std::chrono::system_clock::time_point _prev, _start;
	SharedImageMemory* m_sender;
public:
	enum EFormatWr { FORMAT_UINT8, FORMAT_FP16_GAMMA, FORMAT_FP16_LINEAR };
	enum EResizeModeWr { RESIZEMODE_DISABLED = 0, RESIZEMODE_LINEAR = 1 };
	enum EMirrorModeWr { MIRRORMODE_DISABLED = 0, MIRRORMODE_HORIZONTALLY = 1 };
	enum ESendResultWr { SENDRES_TOOLARGE, SENDRES_WARN_FRAMESKIP, SENDRES_OK };
	
	SharedImageWrapper();
	virtual ~SharedImageWrapper();
	bool SendImage(const unsigned char* data, int width, int height);
	ESendResultWr SendImage2(int width,	int height, int stride, DWORD DataSize, EFormatWr format,
		EResizeModeWr resizemode, EMirrorModeWr mirrormode, int timeout, const uint8_t* buffer);
	bool Ready();
};
}


#endif