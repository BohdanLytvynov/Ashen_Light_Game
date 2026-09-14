// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/InterSkelItem/InteractableSkeletalCapsuleItem.h"
#include "Components/CapsuleComponent.h"

AInteractableSkeletalCapsuleItem::AInteractableSkeletalCapsuleItem(const FObjectInitializer& init) : Super(init)
{
	CollisionShape = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule Collision Component"));
	UpdateRoot();
}
