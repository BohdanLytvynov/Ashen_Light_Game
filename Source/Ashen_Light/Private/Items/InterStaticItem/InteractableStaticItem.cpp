// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/InterStaticItem/InteractableStaticItem.h"

AInteractableStaticItem::AInteractableStaticItem(const FObjectInitializer& init) : Super(init)
{
	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Static Mesh Component"));
	if (ItemMesh)
	{
		DisableCollision(ItemMesh);
	}
	GlowMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Glow Static Mesh Component"));
	if (GlowMesh)
	{
		DisableCollision(GlowMesh);
	}
}
