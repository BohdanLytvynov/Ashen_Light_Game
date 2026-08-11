// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "StateDriven.h"
#include "CharacterInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UCharacterInterface : public UStateDriven
{
	GENERATED_BODY()
};

/**
 * 
 */
class ASHEN_LIGHT_API ICharacterInterface : public IStateDriven
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual bool IsGrounded() const = 0;
	virtual void StopMovement() = 0;
	virtual void Walk() = 0;
	virtual void Run() = 0;
	virtual void Move(const FVector& dir, float value) = 0;
	virtual class UCapsuleComponent* GetCapsuleComponent() const = 0;
	virtual FVector GetCurrentVelocity() const = 0;
	virtual class USkeletalMeshComponent* GetMesh() const = 0;
	virtual FVector AdjustInputForSlope(const FVector& worldDir) const = 0;
};