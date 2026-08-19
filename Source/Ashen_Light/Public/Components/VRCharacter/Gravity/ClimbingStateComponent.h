// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/VRCharacter/Base/VRCharacterComponentBase.h"
#include "ClimbingStateComponent.generated.h"

/**
 * 
 */
UCLASS()
class ASHEN_LIGHT_API UClimbingStateComponent : public UVRCharacterComponentBase
{
	GENERATED_BODY()
public:
	UClimbingStateComponent(const FObjectInitializer& init);
	
};
