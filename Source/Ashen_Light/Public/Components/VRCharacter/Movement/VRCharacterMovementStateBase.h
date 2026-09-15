// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/VRCharacter/Base/VRCharacterComponentBase.h"
#include "VRCharacterMovementStateBase.generated.h"

/**
 * 
 */
UCLASS()
class ASHEN_LIGHT_API UVRCharacterMovementStateBase : public UVRCharacterComponentBase
{
	GENERATED_BODY()
public:
	UVRCharacterMovementStateBase(const FObjectInitializer& init);
protected:
	UPROPERTY(EditAnywhere, Category = "VR Locomotion")
	float JumpHeight = 80.f;

	UPROPERTY(EditAnywhere, Category = "VR Locomotion")
	float JumpThreshold = 5.f;
	void HandleJump();
	bool m_CanTriggerJump;
	virtual bool CanJump(IVRCharacterInterface* context);
	virtual void JumpPhysical(IVRCharacterInterface* vrChar);
};
