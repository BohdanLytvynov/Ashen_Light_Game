// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/InterStaticItem/InteractableStaticCapsuleItem.h"
#include "Components/CapsuleComponent.h"

AInteractableStaticCapsuleItem::AInteractableStaticCapsuleItem(const FObjectInitializer& init) : Super(init)
{
	CollisionShape = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule Collision Component"));
	UpdateRoot();
}
