// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/StateComponentBase.h"
#include "PhysicalMovementComponent.generated.h"

/**
 * 
 */
UCLASS()
class ASHEN_LIGHT_API UPhysicalMovementComponent : public UStateComponentBase
{
	GENERATED_BODY()
public:
	UPhysicalMovementComponent(const FObjectInitializer& init);
	void OnStateEnter() override;
};
