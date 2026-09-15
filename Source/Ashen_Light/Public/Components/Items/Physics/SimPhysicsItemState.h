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
	UPROPERTY(EditAnywhere, Category = "Physics Item State")
	float ThrowSpeed = 200.f;
	UPROPERTY(EditAnywhere, Category = "Physics Item State", meta = (Tooltip = "Do we need to override calculated move speed of the item before we release it."))
	bool OverrideThrowSpeed = false;
	UPROPERTY(EditAnywhere, Category = "Physics Item State", meta = (Tooltip = "Use forward vector of the Item as the throw direction"))
	bool UseForwardVectorForThrow = false;
	UFUNCTION()
	void OnRootSleep(UPrimitiveComponent* SleepingComponent, FName BoneName);
};
