// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Items/Physics/SimPhysicsItemState.h"
#include "Components/Base/StateManagerComponent.h"
#include "Interfaces/Thrower.h"

USimPhysicsItemState::USimPhysicsItemState(const FObjectInitializer& init) : Super(init)
{
}

void USimPhysicsItemState::OnStateEnter()
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
		// Enable full rigid body physics simulation and gravity
		primRoot->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		primRoot->SetSimulatePhysics(true);
		// Optional: Apply cached linear velocity if the item was thrown
		FVector throwVelocity = inter->GetLinearVelocity().GetSafeNormal();
		if (!throwVelocity.IsNearlyZero())
		{
			AActor* owner = inter->GetItemOwner();
			IThrower* thrower = Cast<IThrower>(owner);
			if (!thrower)
			{
				float vel = thrower->GetCurrentThrowVelocity();
				FVector dir = throwVelocity.GetSafeNormal();
				primRoot->SetPhysicsLinearVelocity(dir * vel, true);
			}
			else
			{
				primRoot->SetPhysicsLinearVelocity(throwVelocity, true);
			}			
		}
	}
}

void USimPhysicsItemState::BeginPlay()
{
	IInteractable* inter = GetContext();
	if (!inter) return;

	UMeshComponent* stMesh = inter->GetMesh();
	if (!stMesh) return;

	USceneComponent* rootComp = stMesh->GetAttachmentRoot() ? stMesh->GetAttachmentRoot() : stMesh;
	UPrimitiveComponent* primRoot = Cast<UPrimitiveComponent>(rootComp);

	primRoot->OnComponentSleep.AddDynamic(this, &USimPhysicsItemState::OnRootSleep);
}

void USimPhysicsItemState::OnRootSleep(UPrimitiveComponent* SleepingComponent, FName BoneName)
{
	UStateManagerComponent* stateManger = GetStateManager();
	if (!stateManger) return;
	stateManger->SwitchState(EItemPhysicsState::EIGS_DisablePhysics);
}
