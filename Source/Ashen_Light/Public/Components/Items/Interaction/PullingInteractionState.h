// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/Items/Base/InteractableItemStateBase.h"
#include "PullingInteractionState.generated.h"

/**
 * 
 */
UCLASS()
class ASHEN_LIGHT_API UPullingInteractionState : public UInteractableItemStateBase
{
	GENERATED_BODY()
public:
	UPullingInteractionState(const FObjectInitializer& init);
	void OnStateTick(float DeltaTime) override;
	void OnStateEnter() override;
protected:
	UPROPERTY(EditAnywhere, Category = "Pulling State")
	float GrabThresholdDistance = 2.f;

	UPROPERTY(EditAnywhere, Category = "Pulling State")
	float PullingSpeed = 2.f;
};
