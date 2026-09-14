// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/VRCharacter/Movement/VRCharacterMovementStateBase.h"
#include "Interfaces/VRCharacterInterface.h"
#include "TrackingSpaceMovementComponent.generated.h"

/**
 * 
 */
UCLASS()
class ASHEN_LIGHT_API UTrackingSpaceMovementComponent : public UVRCharacterMovementStateBase
{
	GENERATED_BODY()

public:
	UTrackingSpaceMovementComponent(const FObjectInitializer& init);
	void OnStateTick(float DeltaTime) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Locomotion | Obstacle Detection", meta = (ClampMin = "0.0", ClampMax = "90.0"))
	float StopMovementAngleThreshold = 20.0f;

	UPROPERTY(EditAnywhere, Category = "VR Locomotion | Thresholds", meta = (DisplayName = "Swinging Threshold", ClampMin = "0.001", UIMin = "0.001"))
	float SwiningThreshold = 40.f;
private:
	void HandleMovement(float DeltaTime);
	bool IsSwiningArms(IVRCharacterInterface* vrChar);
	bool CheckObstacles(IVRCharacterInterface* vrChar, FHitResult& outRes);
	bool CheckObstaclesInDirection(IVRCharacterInterface* vrChar, FVector normDir, FHitResult& outRes);
	FVector AdjustInputForSlope(IVRCharacterInterface* vrChar, const FVector& input);
};
