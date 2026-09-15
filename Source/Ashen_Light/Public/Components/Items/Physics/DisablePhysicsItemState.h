// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/Items/Base/InteractableItemStateBase.h"
#include "DisablePhysicsItemState.generated.h"

/**
 * 
 */
UCLASS()
class ASHEN_LIGHT_API UDisablePhysicsItemState : public UInteractableItemStateBase
{
	GENERATED_BODY()
public:
	UDisablePhysicsItemState(const FObjectInitializer& init);
	void OnStateEnter() override;
};
