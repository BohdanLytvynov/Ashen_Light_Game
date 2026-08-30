// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/VRCharacter/Gravity/VRCGravityStateComponentBase.h"
#include "GroundedStateComponent.generated.h"

/**
 * 
 */
UCLASS()
class ASHEN_LIGHT_API UGroundedStateComponent : public UVRCGravityStateComponentBase
{
	GENERATED_BODY()
public:
	UGroundedStateComponent(const FObjectInitializer& init);
	void OnStateTick(float DeltaTime) override;
	void OnStateEnter() override;
protected:

	UPROPERTY(VisibleAnywhere, Category = "Grounded State")
	float FallDamageVelocityThreshold;

	UPROPERTY(VisibleDefaultsOnly, Category = "Ground State")
	float SnapToGroundVelocity = 10.f;
private:
	float CalculateFallDamage(float verticalVelocity) const;
	void ApplyFallDamage(float damage);
	void SnapToGround(IVRCharacterInterface* context);
	void AdjustCapsuleLocation(IVRCharacterInterface* context);
	bool ForceToInAirState(UStateManagerComponent* stateManger);
};
