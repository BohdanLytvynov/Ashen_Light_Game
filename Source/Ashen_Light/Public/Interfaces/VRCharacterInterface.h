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
	virtual class UMotionControllerComponent* GetMotionController(bool right) const = 0;	
	virtual class UMotionControllerHitSensor* GetMotionControllerHitSensor(bool right) const = 0;
	virtual class UCameraComponent* GetVRCamera() const = 0;
	virtual class UCameraFadeSensor* GetCameraFadeSensor() const = 0;
	virtual class UGroundHitSensor* GetGroundHitSensor() const = 0;
	virtual class UObstacleSensor* GetObstacleSensor() const = 0;
	virtual class UVelocitySensor* GetCameraVelocitySensor() const = 0;
	virtual UVelocitySensor* GetMotionControllerVelocitySensor(bool right) const = 0;
	virtual float GetDeadZoneRadius() const = 0;
	virtual bool IsClimbing() const = 0;
	virtual bool IsGrounded() const = 0;
	virtual float GetInitPlayerHeight() const = 0;
	virtual void ApplyCameraFade(float cameraFadeOpacity) = 0;
};