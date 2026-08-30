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
	void BeginPlay() override;
protected:

	UPROPERTY(VisibleAnywhere, Category = "VR Locomotion")
	float JumpHeight = 80.f;

	UPROPERTY(EditAnywhere, Category = "VR Locomotion")
	float JumpThreshold = 10.f;

private:
	bool m_CanTriggerJump;
	bool CanJump(IVRCharacterInterface* context);
	void JumpPhysical(IVRCharacterInterface* vrChar);
};
