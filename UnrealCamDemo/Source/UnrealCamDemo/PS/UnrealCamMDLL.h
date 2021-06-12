// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

class UnrealCamMDLL
{
	UnrealCamMDLL()
	{
		//importDLL("UnrealCam/Binaries/Win64", "UE4Editor-UnrealCam.dll");
		
		importDLL("ExternalDLL", "UnityCapturePlugin.dll");
		importMethodSendTexture();
	}
public:
	static UnrealCamMDLL* ucdll;
	static int32 ucdllcnt;
	static UnrealCamMDLL* init();
	static void release();
	bool SendTextureFromDLL(const unsigned char* data, int width, int height);
	int SendTexture2DLL(void* TextureNativePtr, int Timeout, bool UseDoubleBuffering, bool IsLinearColorSpace);
	bool importDLL(FString folder, FString name);
	bool SetDeviceDLL(void* device, int eventType);
	 void freeDLL();

	 bool importMethodSendTexture();
};
