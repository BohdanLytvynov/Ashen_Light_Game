// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/VRCharacter/Gravity/VRCGravityStateComponentBase.h"
#include "Components/VRCharacter/Sensors/CameraFadeSensor.h"
#include "Components/VRCharacter/Sensors/GroundHitSensor.h"
#include "Components/Base/StateManagerComponent.h"
#include "Enums.h"
#include "Constants.h"

UVRCGravityStateComponentBase::UVRCGravityStateComponentBase(const FObjectInitializer& init) : Super(init)
{
}

bool UVRCGravityStateComponentBase::CheckAndHandleCameraInMesh(IVRCharacterInterface* context, UStateManagerComponent* stateManager)
{
	if (!context || !stateManager) return false;
	UCameraFadeSensor* cameraFade = context->GetCameraFadeSensor();
	if (!cameraFade) return false;
	const bool IsHit = cameraFade->IsHit();
	if (!IsHit) return false;
	stateManager->SwitchState(EGravityState::EGS_InMesh);
	return true;
}

bool UVRCGravityStateComponentBase::CheckAndHandleGroundHit(IVRCharacterInterface* context, 
	UStateManagerComponent* stateManager, float verticalVelocity)
{
	if (!context || !stateManager || verticalVelocity > 0.f) return false;
	UGroundHitSensor* groundHitSensor = context->GetGroundHitSensor();
	if (!groundHitSensor) return false;
	const bool IsHit = groundHitSensor->IsHit();
	if (!IsHit)	return false;
	UStateBlackboard* blackboard = stateManager->GetBlackboard();
	if (blackboard)
	{
		blackboard->SetValue(Constants::Gravity::LastVerticalVelocity, verticalVelocity);
	}
	stateManager->SwitchState(EGravityState::EGS_Grounded);
	return true;
}

bool UVRCGravityStateComponentBase::CheckAndHandleClimbing(IVRCharacterInterface* context, UStateManagerComponent* stateManager)
{
	if (!context || !stateManager) return false;
	if (!context->IsClimbing()) return false;
	stateManager->SwitchState(EGravityState::EGS_Climbing);
	return true;
}

bool UVRCGravityStateComponentBase::CheckAndHandleInAir(IVRCharacterInterface* context, UStateManagerComponent* stateManager)
{
	if (!context || !stateManager) return false;
	UGroundHitSensor* groundHitSensor = context->GetGroundHitSensor();
	if (!groundHitSensor) return false;
	const bool IsHit = groundHitSensor->IsHit();
	if (IsHit) return false;	
	stateManager->SwitchState(EGravityState::EGS_InAir);
	return true;
}
