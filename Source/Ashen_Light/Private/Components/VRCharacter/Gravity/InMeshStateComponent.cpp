// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/VRCharacter/Gravity/InMeshStateComponent.h"
#include "Components/VRCharacter/Sensors/CameraFadeSensor.h"
#include "Components/VRCharacter/Sensors/GroundHitSensor.h"
#include "Components/Base/StateManagerComponent.h"
#include "Camera/CameraComponent.h"
#include "Enums.h"
#include "Constants.h"

UInMeshStateComponent::UInMeshStateComponent(const FObjectInitializer& init) : Super(init)
{
	m_IsGrounded = false;
	m_CameraInMesh = false;
	CurrentCameraFadeOpacity = 0.f;
}

void UInMeshStateComponent::OnStateTick(float DeltaTime)
{
	UStateManagerComponent* stateManager = GetStateManager();
	IVRCharacterInterface* vrChar = GetContext();
	if (!stateManager || !vrChar) return;
	CollectSensorsData(vrChar);
	if (!m_CameraInMesh && m_IsGrounded)
	{
		stateManager->SwitchState(EGravityState::EGS_Grounded);
		return;
	}
	if (!m_CameraInMesh && !m_IsGrounded)
	{
		stateManager->SwitchState(EGravityState::EGS_InAir);
		return;
	}
	if (!m_CameraInMesh && vrChar->IsClimbing())
	{
		stateManager->SwitchState(EGravityState::EGS_Climbing);
		return;
	}

	CalculateFadeOpacity(vrChar, DeltaTime);
	vrChar->ApplyCameraFade(CurrentCameraFadeOpacity);
}

void UInMeshStateComponent::OnStateExit()
{
	m_CameraInMesh = false;
	m_IsGrounded = false;
	CurrentCameraFadeOpacity = 0.f;
	IVRCharacterInterface* vrChar = GetContext();
	if (vrChar)
	{
		vrChar->ApplyCameraFade(CurrentCameraFadeOpacity);
	}
}

void UInMeshStateComponent::CollectSensorsData(IVRCharacterInterface* vrChar)
{
	if (!vrChar) return;
	UGroundHitSensor* groundHitSensor = vrChar->GetGroundHitSensor();
	UCameraFadeSensor* cameraHitSensor = vrChar->GetCameraFadeSensor();
	if (!groundHitSensor && !cameraHitSensor) return;
	m_IsGrounded = groundHitSensor->IsHit();
	m_CameraInMesh = cameraHitSensor->IsHit();
	m_CameraHit = cameraHitSensor->GetHit();
}
PRAGMA_DISABLE_OPTIMIZATION
void UInMeshStateComponent::CalculateFadeOpacity(IVRCharacterInterface* vrChar, float DeltaTime)
{
	if (!vrChar) return;
	UCameraComponent* vrCamera = vrChar->GetVRCamera();
	if (!vrCamera) return;
	UCameraFadeSensor* camFadeSensor = vrChar->GetCameraFadeSensor();
	if (!camFadeSensor) return;
	const FVector CameraLoc = vrCamera->GetComponentLocation();
	const float DistToImpact = FVector::Dist(CameraLoc, m_CameraHit.ImpactPoint);
	const float fadeCheckRadius = camFadeSensor->GetFadeCheckRadius();
	const float PenetrationDepth = FMath::Abs(fadeCheckRadius - DistToImpact);
	const float SafeFadeDistance = FMath::Max(CameraFadeDistance, 0.1f);
	const float targetCameraFadeOpacity = FMath::Clamp(PenetrationDepth / SafeFadeDistance, 0.f, 1.f);
	CurrentCameraFadeOpacity = FMath::FInterpTo(CurrentCameraFadeOpacity, targetCameraFadeOpacity, DeltaTime, CameraFadeInterpConstant);
}
PRAGMA_ENABLE_OPTIMIZATION
