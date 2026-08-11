// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Interfaces/CharacterInterface.h"
#include "VRCharacterInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UVRCharacterInterface : public UCharacterInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class ASHEN_LIGHT_API IVRCharacterInterface : public ICharacterInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual class UMotionControllerComponent* GetLeftMotionController() const = 0;
	virtual class UMotionControllerComponent* GetRightMotionController() const = 0;
	virtual class UCameraComponent* GetVRCamera() const = 0;
	virtual bool IsCrouching(float* crouchDepth) const = 0;
	virtual float GetDeadZoneRadius() const = 0;
	virtual bool IsSwiningArms(float DeltaTime) = 0;
	virtual bool CheckObstacles(float obstacleDistDetection, float halfHeightMultipl, FHitResult& OutHit) = 0;
	virtual bool CheckObstaclesInDirection(const FVector& NormDirection, float obstacleDistDetection, float halfHeightMultipl, FHitResult& hit) = 0;
	virtual bool IsJumping(float jumpHeadThreshold) const = 0;
};