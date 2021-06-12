// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Texture2D.h"
#include "VirtualCamMComponent.generated.h"

class UnrealCamMDLL;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UVirtualCamMComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UVirtualCamMComponent();
	UnrealCamMDLL* ptrDLL =0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* WebcamRenderTexture;
	UCHAR* data=0;
	int32 allocated_size=0;
virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
protected:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
};
