// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/VRCharacter/Gravity/VRCGravityStateComponentBase.h"
#include "ClimbingStateComponent.generated.h"

/**
 * 
 */
UCLASS()
class ASHEN_LIGHT_API UClimbingStateComponent : public UVRCGravityStateComponentBase
{
	GENERATED_BODY()
public:
	UClimbingStateComponent(const FObjectInitializer& init);
	void OnStateTick(float DeltaTime) override;
};
