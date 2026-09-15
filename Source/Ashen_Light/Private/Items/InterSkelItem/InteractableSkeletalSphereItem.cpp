// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/InterSkelItem/InteractableSkeletalSphereItem.h"
#include "Components/SphereComponent.h"

AInteractableSkeletalSphereItem::AInteractableSkeletalSphereItem(const FObjectInitializer& init) : Super(init)
{
	CollisionShape = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere Collision Component"));
	UpdateRoot();
}
