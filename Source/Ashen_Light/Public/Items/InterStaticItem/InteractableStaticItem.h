// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/Base/InteractableItem.h"
#include "InteractableStaticItem.generated.h"

/**
 * 
 */
UCLASS(Abstract)
class ASHEN_LIGHT_API AInteractableStaticItem : public AInteractableItem
{
	GENERATED_BODY()
public:
	AInteractableStaticItem(const FObjectInitializer& init);

};
