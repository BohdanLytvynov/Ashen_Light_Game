// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/Items/Base/InteractableItemStateBase.h"
#include "IdleInteractionState.generated.h"

/**
 * 
 */
UCLASS()
class ASHEN_LIGHT_API UIdleInteractionState : public UInteractableItemStateBase
{
	GENERATED_BODY()
public:
	UIdleInteractionState(const FObjectInitializer& init);
	void OnStateTick(float DeltaTime) override;
	void OnStateEnter() override;
protected:
	UPROPERTY(EditAnywhere, Category = "Hover State Component")
	float OpacityInterpConstantExit = 1.f;
};
