// Fill out your copyright notice in the Description page of Project Settings.

#include "UnrealCamMDLL.h"
#include "Misc/Paths.h"

typedef bool(*_SendTexture)(const unsigned char* data, int width, int height);
typedef bool(*_UnrealSetGraphicsDevice)(void* device, int eventType);
typedef int (*_SendTexture2)(void* TextureNativePtr, int Timeout, bool UseDoubleBuffering,  bool IsLinearColorSpace);


_SendTexture m_SendTextureFromDLL;
_SendTexture2 m_DLLSendTexture2;
_UnrealSetGraphicsDevice m_UnrlSetGrDev;

void* v_dllHandle;

bool UnrealCamMDLL::importDLL(FString folder, FString name)
{
	FString filePath = *FPaths::ProjectPluginsDir()  + folder + "/" + name;

	if (FPaths::FileExists(filePath))
	{
		v_dllHandle = FPlatformProcess::GetDllHandle(*filePath);
		if (v_dllHandle != NULL)
		{
			return true;
		}
	}
	return false;
}

bool UnrealCamMDLL::importMethodSendTexture()
{
	if (v_dllHandle != NULL)
	{
		m_SendTextureFromDLL = NULL;
		FString procName = "SendTexture";
		m_SendTextureFromDLL = (_SendTexture)FPlatformProcess::GetDllExport(v_dllHandle, *procName);
		if (m_SendTextureFromDLL != NULL)
		{
			//return true;
		}

		m_DLLSendTexture2 = NULL;
		FString procName2 = "SendTexture2";
		m_DLLSendTexture2 = (_SendTexture2)FPlatformProcess::GetDllExport(v_dllHandle, *procName2);
		if (m_DLLSendTexture2 != NULL)
		{
			//return true;
		}

		m_UnrlSetGrDev = NULL;
		FString procName3 = "UnrealSetGraphicsDevice";
		m_UnrlSetGrDev = (_UnrealSetGraphicsDevice)FPlatformProcess::GetDllExport(v_dllHandle, *procName3);
		if (m_UnrlSetGrDev != NULL)
		{
			//return true;
		}
	}
	return false;
}

UnrealCamMDLL* UnrealCamMDLL::ucdll=0;
int32 UnrealCamMDLL::ucdllcnt=0;

UnrealCamMDLL* UnrealCamMDLL::init()
{
 	ucdllcnt++;
	if(!ucdll)
		ucdll = new UnrealCamMDLL();
 	return ucdll;
}
void UnrealCamMDLL::release()
 {
	 if((ucdllcnt-1) == 0)
	 {
		 if(ucdll)
		 {
		 	ucdll->freeDLL();
			delete ucdll;
		 }
	 	ucdll =0;
	 }
 	ucdllcnt--;
 }

bool UnrealCamMDLL::SendTextureFromDLL(const unsigned char* data, int width, int height)
{
	if (m_SendTextureFromDLL != NULL)
	{
		bool out = bool(m_SendTextureFromDLL(data, width, height));
		return out;
	}
	return false;
}
int UnrealCamMDLL::SendTexture2DLL(void* TextureNativePtr, int Timeout, bool UseDoubleBuffering,  bool IsLinearColorSpace)
{
	if (m_DLLSendTexture2 != NULL)
	{
		int out = int(m_DLLSendTexture2(TextureNativePtr,1000,UseDoubleBuffering,IsLinearColorSpace));
		return out;
	}
	return -1;
}
bool UnrealCamMDLL::SetDeviceDLL(void* device, int eventType)
{
	if (m_UnrlSetGrDev != NULL)
	{
		bool out = bool(m_UnrlSetGrDev(device,0));
		return out;
	}
	return false;
}
void UnrealCamMDLL::freeDLL()
{
	if (v_dllHandle != NULL)
	{
		m_SendTextureFromDLL = NULL;

		FPlatformProcess::FreeDllHandle(v_dllHandle);
		v_dllHandle = NULL;
	}
}