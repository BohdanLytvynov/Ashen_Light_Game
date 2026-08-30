// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/VRCharacter/Movement/PhysicalMovementComponent.h"
#include "Components/VRCharacter/Sensors/GroundHitSensor.h"
#include "Camera/CameraComponent.h"
#include "Components/Base/StateBlackboard.h"
#include "Components/Base/StateManagerComponent.h"
#include "Constants.h"

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
	HandleJump();
}

bool UPhysicalMovementComponent::CanJump(IVRCharacterInterface* context)
{
	UStateManagerComponent* stateManager = GetStateManager();
	if (!stateManager) return false;
	UStateBlackboard* global = stateManager->GetGlobalBlackboard();
	if (!global) return false;
	bool pendingJump;
	if (!global->TryGetValue(Constants::JumpState::PendingJumping, pendingJump) && pendingJump)
	{
		return false;
	}
	if (!context) return false;
	if (!context->IsGrounded()) return false;
	UCameraComponent* camera = context->GetVRCamera();
	if (!camera) return false;
	float currentHeight = camera->GetRelativeLocation().Z;
	float initHeight = context->GetInitPlayerHeight();
	float delta = currentHeight - initHeight;
	if (delta <= JumpThreshold * 0.5f)
	{
		m_CanTriggerJump = true;
		return false;
	}
	return m_CanTriggerJump && (delta > JumpThreshold);
}

void UPhysicalMovementComponent::JumpPhysical(IVRCharacterInterface* vrChar)
{
	if (!vrChar) return;
	UStateManagerComponent* stateManager = GetStateManager();
	if (!stateManager) return;
	UStateBlackboard* global = stateManager->GetGlobalBlackboard();
	if (!global) return;
	m_CanTriggerJump = false;
	float gravConstant;
	if (!global->TryGetValue(Constants::Gravity::GravityConstant, gravConstant))
	{
		gravConstant = 980.f; //cm/sec^2
	}
	float velocity = FMath::Sqrt(gravConstant * 2 * JumpHeight);
	global->SetValue(Constants::Gravity::GlobalVerticalVelocity, velocity);
	global->SetValue(Constants::JumpState::PendingJumping, true);
	vrChar->Move(FVector(0.f, 0.f, 1.f), velocity, true);
}

void UPhysicalMovementComponent::HandleJump()
{
	IVRCharacterInterface* vrChar = GetContext();
	if (!vrChar) return;
	if (!CanJump(vrChar)) return;
	JumpPhysical(vrChar);
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

