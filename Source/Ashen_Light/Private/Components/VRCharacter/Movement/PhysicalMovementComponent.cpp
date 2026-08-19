// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/VRCharacter/Movement/PhysicalMovementComponent.h"

UPhysicalMovementComponent::UPhysicalMovementComponent(const FObjectInitializer& init) : Super(init)
{
	
}

void UPhysicalMovementComponent::OnStateEnter()
{
	//When we have the Physical movement State - we should stop all movement
	IVRCharacterInterface* vrChar = GetContext();	
	if (vrChar)
	{
		vrChar->StopMovement();
	}
}

void UPhysicalMovementComponent::OnStateTick(float DeltaTime)
{
	HandleJump();
}

void UPhysicalMovementComponent::HandleJump()
{
	IVRCharacterInterface* vrChar = GetContext();
	if (!vrChar) return;	
	vrChar->JumpPhysical(JumpHeight);
}

