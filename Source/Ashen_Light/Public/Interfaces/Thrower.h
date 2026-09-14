// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Thrower.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UThrower : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class ASHEN_LIGHT_API IThrower
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual float GetCurrentThrowVelocity() const = 0;
};
