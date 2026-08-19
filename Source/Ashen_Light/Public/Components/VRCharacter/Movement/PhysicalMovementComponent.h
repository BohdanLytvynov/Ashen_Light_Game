// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/VRCharacter/Base/VRCharacterComponentBase.h"
#include "PhysicalMovementComponent.generated.h"

/**
 * 
 */
UCLASS()
class ASHEN_LIGHT_API UPhysicalMovementComponent : public UVRCharacterComponentBase
{
	GENERATED_BODY()
public:
	UPhysicalMovementComponent(const FObjectInitializer& init);
	void OnStateEnter() override;
	void OnStateTick(float DeltaTime) override;
	void HandleJump();
protected:

	UPROPERTY(VisibleAnywhere, Category = "VR Locomotion")
	float JumpHeight = 80.f;
};
