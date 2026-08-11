// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/VRCharacter/PhysicalMovementComponent.h"
#include "../../../Public/Interfaces/VRCharacterInterface.h"

UPhysicalMovementComponent::UPhysicalMovementComponent(const FObjectInitializer& init) : Super(init)
{
	
}

void UPhysicalMovementComponent::OnStateEnter()
{
	//When we have the Physical movement State - we should stop all movement
	IStateDriven* stateDriven = GetContext();
	if (!stateDriven) return;
	IVRCharacterInterface* vrChar = Cast<IVRCharacterInterface>(stateDriven);//O(1)
	if (vrChar)
	{
		vrChar->StopMovement();
	}
}
