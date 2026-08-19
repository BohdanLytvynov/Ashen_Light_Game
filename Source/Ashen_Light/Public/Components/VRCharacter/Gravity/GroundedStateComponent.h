// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/VRCharacter/Base/VRCharacterComponentBase.h"
#include "GroundedStateComponent.generated.h"

/**
 * 
 */
UCLASS()
class ASHEN_LIGHT_API UGroundedStateComponent : public UVRCharacterComponentBase
{
	GENERATED_BODY()
public:
	UGroundedStateComponent(const FObjectInitializer& init);
	
};
