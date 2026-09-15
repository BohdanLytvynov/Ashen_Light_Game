// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/VRCharacter/Movement/VRCharacterMovementStateBase.h"
#include "Components/VRCharacter/Sensors/GroundHitSensor.h"
#include "Camera/CameraComponent.h"
#include "Components/Base/StateBlackboard.h"
#include "Components/Base/StateManagerComponent.h"
#include "Constants.h"

UVRCharacterMovementStateBase::UVRCharacterMovementStateBase(const FObjectInitializer& init) : Super(init)
{
}

bool UVRCharacterMovementStateBase::CanJump(IVRCharacterInterface* context)
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

void UVRCharacterMovementStateBase::JumpPhysical(IVRCharacterInterface* vrChar)
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

void UVRCharacterMovementStateBase::HandleJump()
{
	IVRCharacterInterface* vrChar = GetContext();
	if (!vrChar) return;
	if (!CanJump(vrChar)) return;
	JumpPhysical(vrChar);
}
