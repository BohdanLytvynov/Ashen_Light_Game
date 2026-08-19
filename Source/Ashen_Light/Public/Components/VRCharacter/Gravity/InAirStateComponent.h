// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/VRCharacter/Base/VRCharacterComponentBase.h"
#include "InAirStateComponent.generated.h"

/**
 * 
 */
UCLASS()
class ASHEN_LIGHT_API UInAirStateComponent : public UVRCharacterComponentBase
{
	GENERATED_BODY()
public:
	UInAirStateComponent(const FObjectInitializer& init);
	virtual void OnStateTick(float DeltaTime) override;
protected:

};
