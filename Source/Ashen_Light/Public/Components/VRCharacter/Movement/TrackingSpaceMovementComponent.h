// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../Base/VRCharacterComponentBase.h"
#include "Interfaces/VRCharacterInterface.h"
#include "TrackingSpaceMovementComponent.generated.h"

/**
 * 
 */
UCLASS()
class ASHEN_LIGHT_API UTrackingSpaceMovementComponent : public UVRCharacterComponentBase
{
	GENERATED_BODY()

public:
	UTrackingSpaceMovementComponent(const FObjectInitializer& init);
	void OnStateTick(float DeltaTime) override;

protected:

	UPROPERTY(EditAnywhere, Category = "VR Locomotion Obstacle Detection", meta = (DisplayName = "Obstacle Distance Detection", ClampMin = "0.001", UIMin = "0.001"))
	float ObstacleDistanceDetection = 140.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Locomotion Obstacle Detection", meta = (ClampMin = "0.0", ClampMax = "90.0"))
	float StopMovementAngleThreshold = 20.0f;
	
	UPROPERTY(EditAnywhere, Category = "VR Locomotion Traces", meta = (Tooltip = "[0 ; 1] multiplier to adjust height of the tracing capsule. 1 - equal to the height of the current capsule"))
	float TraceCapsuleHalfHeightMultipl = 0.7f;

private:
	void HandleMovement(float DeltaTime);

	void HandleJump(float DeltaTime);

	bool CheckObstacles(IVRCharacterInterface* vrChar, float obstacleDistDetection, FHitResult& OutHit);
};
