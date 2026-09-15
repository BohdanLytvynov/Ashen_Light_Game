// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Items/Physics/DisablePhysicsItemState.h"

UDisablePhysicsItemState::UDisablePhysicsItemState(const FObjectInitializer& init) : Super(init)
{
}

void UDisablePhysicsItemState::OnStateEnter()
{
	Super::OnStateEnter();

	IInteractable* inter = GetContext();
	if (!inter) return;

	UMeshComponent* stMesh = inter->GetMesh();
	if (!stMesh) return;

	USceneComponent* rootComp = stMesh->GetAttachmentRoot() ? stMesh->GetAttachmentRoot() : stMesh;
	UPrimitiveComponent* primRoot = Cast<UPrimitiveComponent>(rootComp);

	if (primRoot)
	{
		// Fully disable physics simulation and scene collision queries
		// We need this to be able react on World Static 
		primRoot->SetSimulatePhysics(false);
		primRoot->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		primRoot->SetCollisionResponseToAllChannels(ECR_Block);
		primRoot->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	}
}
