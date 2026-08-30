// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/VRCharacter/Gravity/InAirStateComponent.h"
#include "Components/Base/StateBlackboard.h"
#include "Components/Base/StateManagerComponent.h"
#include "Components/Base/VelocitySensor.h"
#include "Constants.h"
#include "Enums.h"

UInAirStateComponent::UInAirStateComponent(const FObjectInitializer& init) : Super(init)
{
	VerticalVelocity = 0.f;
}

void UInAirStateComponent::BeginPlay()
{
	UStateManagerComponent* stateManager = GetStateManager();
	if (!stateManager) return;
	UStateBlackboard* global = stateManager->GetGlobalBlackboard();
	if (!global) return;
	global->SetValue(Constants::Gravity::GravityConstant, GravityConstant);
}

void UInAirStateComponent::OnStateTick(float DeltaTime)
{
	Super::OnStateTick(DeltaTime);

	IVRCharacterInterface* context = GetContext();
	UStateManagerComponent* stateManager = GetStateManager();
	if (CheckAndHandleCameraInMesh(context, stateManager)) return;
	if (CheckAndHandleClimbing(context, stateManager)) return;
	if (CheckAndHandleGroundHit(context, stateManager, VerticalVelocity)) return;
	ApplyGravity(DeltaTime);	
	context->Move(FVector(LastHorizontalVelocity.X, LastHorizontalVelocity.Y, VerticalVelocity), 1.f, true);
}

void UInAirStateComponent::OnStateEnter()
{
	UStateManagerComponent* stateManager = GetStateManager();
	if (!stateManager) return;
	UStateBlackboard* global = stateManager->GetGlobalBlackboard();
	if (!global) return;
	if (stateManager->GetPrevStateEnum() == (uint8)EGravityState::EGS_Grounded)
	{
		//Seems that we perform some jump
		float vertVelocity;
		if (!global->TryGetValue(Constants::Gravity::GlobalVerticalVelocity, vertVelocity))
		{
			vertVelocity = 0.f;
		}
		VerticalVelocity = vertVelocity;
		IVRCharacterInterface* vrChar = GetContext();
		if (!vrChar) return;
		UVelocitySensor* velSensor = vrChar->GetCameraVelocitySensor();
		if (!velSensor) return;
		FVector vel = velSensor->GetVelocity();
		LastHorizontalVelocity = FVector(vel.X, vel.Y, 0.f);
	}
}

void UInAirStateComponent::OnStateExit()
{
	VerticalVelocity = 0.f;
}

void UInAirStateComponent::ApplyGravity(float DeltaTime)
{
	//Vertical velocity accumulation v = v + g * dt, where g = -9.8 m / sec^2 = -980 cm / sec^2
	VerticalVelocity += -1.f * GravityConstant * DeltaTime;
	//Clamp Vertiacal velocity between terminal velocity and max fall velocity
	VerticalVelocity = FMath::Clamp(VerticalVelocity, TerminalVelocity, MaxFallVelocity);
}


