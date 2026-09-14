// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/Base/AttributeComponent.h"
#include "VRCharacterAttributeComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ASHEN_LIGHT_API UVRCharacterAttributeComponent : public UAttributeComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UVRCharacterAttributeComponent(const FObjectInitializer& init);

#pragma region UPROPERTIES

	UPROPERTY(EditAnywhere, Category = "VR Locomotion", meta = (DisplayName = "Walk Speed"))
	float WalkSpeed = 200.f;

	UPROPERTY(EditAnywhere, Category = "VR Locomotion", meta = (DisplayName = "Run Speed"))
	float RunSpeed = 300.f;

	UPROPERTY(EditAnywhere, Category = "VR Locomotion Thresholds")
	float CrouchThreshold = 10.f;

	UPROPERTY(EditAnywhere, Category = "VR Camera Fade", meta = (ClampMin = "0.001", UIMin = "0.001"))
	float CameraFadeDistance = 5.f;
	
	UPROPERTY(EditAnywhere, Category = "VR Interaction")
	float ThrowSpeed = 200.f;
#pragma endregion

		
};
