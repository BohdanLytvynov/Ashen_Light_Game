// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Items/Interaction/IdleInteractionState.h"
#include "Components/Base/StateManagerComponent.h"

UIdleInteractionState::UIdleInteractionState(const FObjectInitializer& init) : Super(init)
{
}

void UIdleInteractionState::OnStateTick(float DeltaTime)
{	
	if (CurrentOpacity <= 0.f) return;
	UpdateGlowMatOpacity(0.f, DeltaTime, OpacityInterpConstantExit);
}

void UIdleInteractionState::OnStateEnter()
{
	Super::OnStateEnter();

	IInteractable* inter = GetContext();
	if (!inter) return;
	inter->ResetState();

	UStateManagerComponent* stateManager = GetStateManager();
	if (stateManager)
	{
		const EItemInteractiveState prevState = stateManager->GetPrevState<EItemInteractiveState>();
		const bool bWasInteracting = (prevState == EItemInteractiveState::EIIS_Grab ||
			prevState == EItemInteractiveState::EIIS_Pulling);
		if (bWasInteracting)
		{
			inter->SwitchPhysicsManager(EItemPhysicsState::EIGS_SimulatePhysics);
		}		
	}
}
