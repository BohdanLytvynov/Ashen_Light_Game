// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Items/Interaction/PullingInteractionState.h"
#include "Dto/FVRControllerButtonsState.h"
#include "Components/Base/StateManagerComponent.h"
#include "Enums.h"

UPullingInteractionState::UPullingInteractionState(const FObjectInitializer& init) : Super(init)
{
}

void UPullingInteractionState::OnStateTick(float DeltaTime)
{
	UStateManagerComponent* stateManager = GetStateManager();
	IInteractable* inter = GetContext();
	if (!stateManager || !inter) return;
	EVRControllerHand primHand = inter->GetVRHand(true);
	FVRControllerButtonsState primHandControllerButtonsState;
	inter->GetVRControllerButtonState(primHand, primHandControllerButtonsState);
	if (!primHandControllerButtonsState.IsButtonPressed(EVRButtonType::Grip) || primHand == EVRControllerHand::None)
	{
		//Go to Idle State, we released object
		stateManager->SwitchState(EItemInteractiveState::EIIS_Idle);
		return;
	}
	//Can we Grab Item?
	USceneComponent* source = inter->GetVRHoverReason(true);
	if (source)
	{
		const bool canGrab = inter->HasCapability((uint8)EItemCapabilities::EIC_Grab);
		const bool canPull = inter->HasCapability((uint8)EItemCapabilities::EIC_Pull);
		//In a world space
		FVector HandPos = source->GetComponentLocation();
		FVector ItemPos = inter->GetActorTransform().GetLocation();
		float distSq = FVector::DistSquared(HandPos, ItemPos);
		if ((distSq <= GrabThresholdDistance * GrabThresholdDistance) && canGrab)
		{
			//Do Grab
			stateManager->SwitchState(EItemInteractiveState::EIIS_Grab);
			return;
		}
		else if(canPull)
		{
			FVector dir = (HandPos - ItemPos).GetSafeNormal();
			FVector step = dir * PullingSpeed * DeltaTime;
			FVector newLocation = ItemPos + step;
			if (step.SizeSquared() >= distSq)
			{
				newLocation = HandPos;
			}

			inter->SetNewActorLocation(newLocation, false, nullptr, ETeleportType::TeleportPhysics);
		}
	}
}

void UPullingInteractionState::OnStateEnter()
{
	IInteractable* inter = GetContext();
	if (!inter) return;
	inter->SwitchPhysicsManager(EItemPhysicsState::EIGS_Kinematic);
}
