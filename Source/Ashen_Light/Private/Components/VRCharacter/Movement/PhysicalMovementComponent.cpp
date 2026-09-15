// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/VRCharacter/Movement/PhysicalMovementComponent.h"
#include <Components/Base/StateBlackboard.h>
#include <Components/Base/StateManagerComponent.h>
#include <Constants.h>


UPhysicalMovementComponent::UPhysicalMovementComponent(const FObjectInitializer& init) : Super(init)
{
	m_CanTriggerJump = true;
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
	Super::HandleJump();
}

void UPhysicalMovementComponent::BeginPlay()
{
	UStateManagerComponent* stateManager = GetStateManager();
	if (!stateManager) return;
	UStateBlackboard* global = stateManager->GetGlobalBlackboard();
	if (!global) return;
	global->SetValue(Constants::Gravity::GlobalVerticalVelocity, 0.f);
	global->SetValue(Constants::JumpState::PendingJumping, false);
}

