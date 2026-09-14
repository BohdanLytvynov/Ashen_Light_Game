// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/Base/StateComponentBase.h"
#include "Interfaces/Items/Interactable.h"
#include "InteractableItemStateBase.generated.h"

/**
 * 
 */
UCLASS()
class ASHEN_LIGHT_API UInteractableItemStateBase : public UStateComponentBase
{
	GENERATED_BODY()
public:
	UInteractableItemStateBase(const FObjectInitializer& init);
	IInteractable* GetContext() override;
protected:
	float CurrentOpacity;
	UMaterialInstanceDynamic* GetMatInstance();
	virtual void UpdateGlowMatOpacity(float targetOpacity, float DeltaTime, float interpConstant);
private:
	IInteractable* m_interactable;
	UMaterialInstanceDynamic* GlowMat;
};
