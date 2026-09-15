// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/Items/Base/InteractableItemStateBase.h"
#include "HoverInteractionState.generated.h"

/**
 * 
 */
UCLASS()
class ASHEN_LIGHT_API UHoverInteractionState : public UInteractableItemStateBase
{
	GENERATED_BODY()
public:
	UHoverInteractionState(const FObjectInitializer& init);
	void OnStateTick(float DeltaTime) override;
	void OnStateEnter() override;
protected:
	UPROPERTY(EditAnywhere, Category = "Hover State Component")
	float OpacityInterpConstantEnter = 1.f;
};
