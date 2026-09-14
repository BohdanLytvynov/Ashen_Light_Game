// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/InterSkelItem/InteractableSkeletalItem.h"

AInteractableSkeletalItem::AInteractableSkeletalItem(const FObjectInitializer& init) : Super(init)
{
	ItemMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Skeletal Mesh Component"));
	if (ItemMesh)
	{
		DisableCollision(ItemMesh);
	}
	GlowMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Glow Skeletal Mesh Component"));
	if (GlowMesh)
	{
		DisableCollision(GlowMesh);
	}
}
