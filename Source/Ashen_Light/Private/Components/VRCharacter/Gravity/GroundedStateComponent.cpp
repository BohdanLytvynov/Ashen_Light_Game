// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/VRCharacter/Gravity/GroundedStateComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/Base/StateManagerComponent.h"
#include "Components/VRCharacter/Sensors/GroundHitSensor.h"
#include "Enums.h"
#include "Constants.h"

UGroundedStateComponent::UGroundedStateComponent(const FObjectInitializer& init) : Super(init)
{

}

void UGroundedStateComponent::OnStateTick(float DeltaTime)
{	
	UStateManagerComponent* stateManager = GetStateManager();
	IVRCharacterInterface* context = GetContext();
	if (!stateManager && !context) return;
	if (ForceToInAirState(stateManager)) return;//Case when we are jumping
	if (CheckAndHandleCameraInMesh(context, stateManager)) return;
	if (CheckAndHandleClimbing(context, stateManager)) return;
	if (CheckAndHandleInAir(context, stateManager)) return;
	SnapToGround(context);
}

void UGroundedStateComponent::OnStateEnter()
{
	UStateManagerComponent* stateManager = GetStateManager();
	UStateBlackboard* stateBlackboard = stateManager->GetBlackboard();
	UStateBlackboard* global = stateManager->GetGlobalBlackboard();
	if (!stateManager || !stateBlackboard) return;	
	//Case when we come here from the In Air State
	if (stateManager->GetPrevStateEnum() == (uint8)EGravityState::EGS_InAir)
	{
		bool pendingJumpimg = false;
		//We come to the ground after Jumping
		if (global && global->TryGetValue(Constants::JumpState::PendingJumping, pendingJumpimg) && pendingJumpimg)
		{
			//If we hit the ground - we can jump again
			global->SetValue(Constants::JumpState::PendingJumping, false);
			global->SetValue(Constants::Gravity::GlobalVerticalVelocity, 0.f);
		}
		float lastVerticalVelocity = 0.f;
		stateBlackboard->TryGetValue(Constants::Gravity::LastVerticalVelocity, lastVerticalVelocity);
		ApplyFallDamage(CalculateFallDamage(lastVerticalVelocity));
	}
}

float UGroundedStateComponent::CalculateFallDamage(float verticalVelocity) const
{
	return 0.0f;
}

void UGroundedStateComponent::ApplyFallDamage(float damage)
{
	// To Do
}

void UGroundedStateComponent::SnapToGround(IVRCharacterInterface* context)
{
	if (!context) return;
	context->Move(FVector(0.f, 0.f, -1.f), SnapToGroundVelocity, true);
	AdjustCapsuleLocation(context);
}

void UGroundedStateComponent::AdjustCapsuleLocation(IVRCharacterInterface* context)
{
	if (!context) return;
	UGroundHitSensor* groundHitSensor = context->GetGroundHitSensor();
	if (!groundHitSensor) return;
	UCapsuleComponent* caps = context->GetCapsuleComponent();
	if (!caps) return;
	const float halfHeight = caps->GetScaledCapsuleHalfHeight();
	FVector capsLocation = caps->GetComponentLocation();
	if (!groundHitSensor->IsHit()) return;
	FHitResult hit = groundHitSensor->GetHit();
	const float penetrDepth = hit.ImpactPoint.Z - (capsLocation.Z - halfHeight);
	if (FMath::IsNearlyZero(penetrDepth)) return;
	FVector CurrentActorLocation = context->GetActorTransform().GetLocation();
	CurrentActorLocation.Z += penetrDepth;
	context->SetNewActorLocation(CurrentActorLocation, false, nullptr, ETeleportType::TeleportPhysics);
}

bool UGroundedStateComponent::ForceToInAirState(UStateManagerComponent* stateManger)
{
	UStateBlackboard* global = stateManger->GetGlobalBlackboard();
	if (!global) return false;
	float verticalVelocity;
	if (!global->TryGetValue(Constants::Gravity::GlobalVerticalVelocity, verticalVelocity))
		return false;
	bool pendingJumping;
	if (!global->TryGetValue(Constants::JumpState::PendingJumping, pendingJumping))
		return false;
	if (pendingJumping && verticalVelocity > 0.f)
	{
		stateManger->SwitchState(EGravityState::EGS_InAir);
		return true;
	}
	
	return false;
}
