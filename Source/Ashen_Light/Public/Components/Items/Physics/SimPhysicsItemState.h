// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/Items/Base/InteractableItemStateBase.h"
#include "SimPhysicsItemState.generated.h"

/**
 * 
 */
UCLASS()
class ASHEN_LIGHT_API USimPhysicsItemState : public UInteractableItemStateBase
{
	GENERATED_BODY()
public:
	USimPhysicsItemState(const FObjectInitializer& init);
	void OnStateEnter() override;
	void BeginPlay() override;
protected:
	UFUNCTION()
	void OnRootSleep(UPrimitiveComponent* SleepingComponent, FName BoneName);
};
