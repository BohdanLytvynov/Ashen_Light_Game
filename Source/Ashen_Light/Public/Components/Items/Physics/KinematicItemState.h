// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/Items/Base/InteractableItemStateBase.h"
#include "KinematicItemState.generated.h"

/**
 * 
 */
UCLASS()
class ASHEN_LIGHT_API UKinematicItemState : public UInteractableItemStateBase
{
	GENERATED_BODY()
public:
	UKinematicItemState(const FObjectInitializer& init);
	void OnStateEnter() override;
};
