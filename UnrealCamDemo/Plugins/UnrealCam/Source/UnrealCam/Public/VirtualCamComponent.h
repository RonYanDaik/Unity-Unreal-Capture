// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Texture2D.h"
#include "VirtualCamComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UNREALCAM_API UVirtualCamComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UVirtualCamComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* WebcamRenderTexture;
	UCHAR* data=0;
	int32 allocated_size=0;
protected:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};
