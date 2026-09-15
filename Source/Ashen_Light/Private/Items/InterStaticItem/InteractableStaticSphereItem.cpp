// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/InterStaticItem/InteractableStaticSphereItem.h"
#include "Components/SphereComponent.h"

AInteractableStaticSphereItem::AInteractableStaticSphereItem(const FObjectInitializer& init) : Super(init)
{
	CollisionShape = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere Collision Shape"));
	UpdateRoot();
}
