// Fill out your copyright notice in the Description page of Project Settings.
#include "VirtualCamMComponent.h"
#include "UnrealCamMDLL.h"
#include "Engine/Classes/Kismet/KismetRenderingLibrary.h"

UVirtualCamMComponent::UVirtualCamMComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	data=0;
	allocated_size=0;
}
void UVirtualCamMComponent::BeginPlay()
{
	Super::BeginPlay();
	ptrDLL= UnrealCamMDLL::init();
	
};
void UVirtualCamMComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	UnrealCamMDLL::release();
		
}


// Called every frame
void UVirtualCamMComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (WebcamRenderTexture)
	{
		int size = WebcamRenderTexture->GetSizeX() * WebcamRenderTexture->GetSizeY();
		if(!data || (allocated_size<(size*3)))
		{
			allocated_size = size * 3;
			if(data)
				delete data;
			
			data = new UCHAR[size * 3];
		}
		
		switch (WebcamRenderTexture->GetPixelFormat())
		{
			case EPixelFormat::PF_B8G8R8A8:
			{
				const FColor* FormatedImageData = static_cast<const FColor*>(WebcamRenderTexture->PlatformData->Mips[0].BulkData.LockReadOnly());
				
				for (int32 i = 0; i < size; i++)
				{
					int32 dataIndex = size * 3 - 1 - i * 3;
					data[dataIndex + 2] = FormatedImageData[i].R;
					data[dataIndex + 1] = FormatedImageData[i].G;
					data[dataIndex] = FormatedImageData[i].B;
				}
				ptrDLL->SendTextureFromDLL(data, WebcamRenderTexture->GetSizeX(), WebcamRenderTexture->GetSizeY());
				WebcamRenderTexture->PlatformData->Mips[0].BulkData.Unlock();
				break;
			}
		}
	}
}

