// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "PhysicsSolverSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class ASHEN_LIGHT_API UPhysicsSolverSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
public:	
	UFUNCTION(BlueprintPure)
	FVector CalculateLinearMomentum(float mass, const FVector& velocity) const
	{
		return CalculateLinearMomentum_S(mass, velocity);
	}

	static FORCEINLINE FVector CalculateLinearMomentum_S(float mass, const FVector& velocity)
	{
		return mass * velocity;
	}
};
