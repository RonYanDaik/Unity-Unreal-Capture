// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"

#include "Components/SceneCaptureComponent2D.h"
#include "VirtualSceneCaptureMComponent2D.generated.h"
class UnrealCamMDLL;
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class UVirtualSceneCaptureMComponent2D : public USceneCaptureComponent2D
{
	GENERATED_BODY()
	
public:
	enum DLLReturCodes
	{
		RET_SUCCESS = 0,
		RET_WARNING_FRAMESKIP = 1,
		RET_WARNING_CAPTUREINACTIVE = 2,
		RET_ERROR_UNSUPPORTEDGRAPHICSDEVICE = 100,
		RET_ERROR_PARAMETER = 101,
		RET_ERROR_TOOLARGERESOLUTION = 102,
		RET_ERROR_TEXTUREFORMAT = 103,
		RET_ERROR_READTEXTURE = 104,
	};
	
	UVirtualSceneCaptureMComponent2D();
	UnrealCamMDLL* ptrDLL =0;
	/** Output render target of the scene capture that can be read in materals. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=SceneCapture)
	UTextureRenderTarget2D* TextureTarget2;
protected:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void BeginPlay() override;
		UCHAR* data=0;
	int32 allocated_size=0;
	int32 b=0;
	void* D3DDevice =0;
};
