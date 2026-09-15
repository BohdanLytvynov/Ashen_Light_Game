// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/VRCharacter/Movement/VRCharacterMovementStateBase.h"
#include "PhysicalMovementComponent.generated.h"

/**
 * 
 */
UCLASS()
class ASHEN_LIGHT_API UPhysicalMovementComponent : public UVRCharacterMovementStateBase
{
	GENERATED_BODY()
public:
	UPhysicalMovementComponent(const FObjectInitializer& init);
	void OnStateEnter() override;
	void OnStateTick(float DeltaTime) override;
	void BeginPlay() override;
};
