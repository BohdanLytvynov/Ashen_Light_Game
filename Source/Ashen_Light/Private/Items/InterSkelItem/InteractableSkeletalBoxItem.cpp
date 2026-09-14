// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/InterSkelItem/InteractableSkeletalBoxItem.h"
#include "Components/BoxComponent.h"

AInteractableSkeletalBoxItem::AInteractableSkeletalBoxItem(const FObjectInitializer& init) : Super(init)
{
	CollisionShape = CreateDefaultSubobject<UBoxComponent>(TEXT("Box Collision Component"));
	UpdateRoot();
}
