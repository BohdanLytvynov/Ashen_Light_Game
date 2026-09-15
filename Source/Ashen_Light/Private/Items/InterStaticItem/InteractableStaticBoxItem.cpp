// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/InterStaticItem/InteractableStaticBoxItem.h"
#include "Components/BoxComponent.h"

AInteractableStaticBoxItem::AInteractableStaticBoxItem(const FObjectInitializer& init) : Super(init)
{
	CollisionShape = CreateDefaultSubobject<UBoxComponent>(TEXT("Collision Box Component"));
	UpdateRoot();
}
