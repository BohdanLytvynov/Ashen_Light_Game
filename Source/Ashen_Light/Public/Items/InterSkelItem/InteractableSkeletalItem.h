// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/Base/InteractableItem.h"
#include "InteractableSkeletalItem.generated.h"

/**
 * 
 */
UCLASS(Abstract)
class ASHEN_LIGHT_API AInteractableSkeletalItem : public AInteractableItem
{
	GENERATED_BODY()
public:
	AInteractableSkeletalItem(const FObjectInitializer& init);
};
