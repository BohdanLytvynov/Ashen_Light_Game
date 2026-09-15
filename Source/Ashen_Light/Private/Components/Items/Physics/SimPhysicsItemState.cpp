// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Items/Physics/SimPhysicsItemState.h"
#include "Components/Base/StateManagerComponent.h"
#include "Subsystems/PhysicsSolverSubsystem.h"

USimPhysicsItemState::USimPhysicsItemState(const FObjectInitializer& init) : Super(init)
{
}

void USimPhysicsItemState::OnStateEnter()
{
	Super::OnStateEnter();

	IInteractable* inter = GetContext();
	if (!inter) return;	
	UPrimitiveComponent* primRoot = inter->GetPhysicsRootComponent();
	if (!primRoot) return;
	//We cache velocity first cause we need 
	const FVector throwVelocity = inter->GetLinearVelocity();
	float throwVel = 0.f;
	// Enable full rigid body physics simulation and gravity
	primRoot->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	primRoot->SetSimulatePhysics(true);
	const AActor* owner = inter->GetItemOwner();
	FVector linearMomentum = FVector::ZeroVector;
	float mass = inter->GetPhysicalMass();
	FVector finalVelocity = throwVelocity;
	if (OverrideThrowSpeed)
	{
		FVector throwDir = FVector::ZeroVector;
		if (UseForwardVectorForThrow)
		{
			throwDir = inter->GetBasisVector(FName("X"));
		}
		else
		{
			throwDir = throwVelocity.GetSafeNormal();
			if (throwDir.IsNearlyZero())
			{
				throwDir = inter->GetBasisVector(FName("X"));
			}
		}
		finalVelocity = throwDir * ThrowSpeed;
	}
	linearMomentum = UPhysicsSolverSubsystem::CalculateLinearMomentum_S(mass, finalVelocity);
	primRoot->AddImpulse(linearMomentum);
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
