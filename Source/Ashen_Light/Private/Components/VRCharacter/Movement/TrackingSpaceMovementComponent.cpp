// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/VRCharacter/Movement/TrackingSpaceMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "Characters/VRCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/Base/StateManagerComponent.h"
#include "Components/VRCharacter/Sensors/GroundHitSensor.h"
#include "Components/Base/VelocitySensor.h"
#include "Components/VRCharacter/Sensors/ObstacleSensor.h"
#include "Components/VRCharacter/Sensors/CameraFadeSensor.h"
#include "Enums.h"

UTrackingSpaceMovementComponent::UTrackingSpaceMovementComponent(const FObjectInitializer& init) : Super(init)
{

}

void UTrackingSpaceMovementComponent::OnStateTick(float DeltaTime)
{
	Super::HandleJump();
	HandleMovement(DeltaTime);
}

void UTrackingSpaceMovementComponent::HandleMovement(float DeltaTime)
{
	IVRCharacterInterface* vrChar = GetContext();
	if (!vrChar) return;
	UCameraComponent* camera = vrChar->GetVRCamera();
	if (!camera) return;
	UGroundHitSensor* groundHitSensor = vrChar->GetGroundHitSensor();
	if (!vrChar->IsGrounded()) return;//No need to calculate the movement when we are in air
	UCameraFadeSensor* camFadeSensor = vrChar->GetCameraFadeSensor();
	if (!camFadeSensor) return;
	//No movement, if we are inside the Mesh
	if (camFadeSensor->IsHit()) return;
	//1 Get HMD Device coordinates in TrackSpace relative to the TrackSpace's origin
	FVector CameraInTrackSpace = camera->GetRelativeLocation();
	//2 Create the 2D vector 
	FVector CameraInTrackSpace2D(CameraInTrackSpace.X, CameraInTrackSpace.Y, 0.f);
	//Calculate distance
	float Dist2D = CameraInTrackSpace2D.Size();
	if (Dist2D > vrChar->GetDeadZoneRadius())//Camera is outside the dead zone - start moving
	{
		FTransform actorWorld = vrChar->GetActorTransform();
		//Transform HMD location in the Track Space to Unreal Engine World Space
		FVector WorldDirVector = actorWorld.TransformVectorNoScale(CameraInTrackSpace2D);
		FHitResult hit;
		if (CheckObstacles(vrChar, hit))//We hit obstacle
		{		
			//Normal that is perpendicular to the Mesh Face
			FVector ImpactNormal = hit.ImpactNormal;
			FVector MovementDir = WorldDirVector.GetSafeNormal();

			// 1. Check hit angle (Scalar Project movement dir on the Impact normal)
			//  < 0  : Move in the obstacle direction
			// -1.0f : Perpendicular Hit to the wall
			//  0.0f : Sliding across the wall
			float Dot = FVector::DotProduct(MovementDir, ImpactNormal);
			//Calculate degrees to dot product dp = Cos(degr) 
			float DotThreshold = -FMath::Cos(FMath::DegreesToRadians(StopMovementAngleThreshold));
			if (Dot <= DotThreshold)
			{
				WorldDirVector = FVector::ZeroVector;//Stop Movement
			}
			else
			{
				//Calculate vector along the Static Mesh to slide across
				WorldDirVector = FVector::VectorPlaneProject(MovementDir, ImpactNormal);
				FHitResult secondHit;
				if (CheckObstaclesInDirection(vrChar, WorldDirVector, secondHit))
				{
					WorldDirVector = FVector::ZeroVector;
				}
			}
		}

		if (!WorldDirVector.IsNearlyZero())
		{
			if (IsSwiningArms(vrChar))//Run
			{
				vrChar->Run();
			}
			else//Walk
			{
				vrChar->Walk();
			}
			//Modify direction vector to slide across the floor normals
			FVector SlopeAdjustedWorldDirVector = AdjustInputForSlope(vrChar, WorldDirVector);
			//Call movement via UPawnFloatingMovement
			vrChar->Move(SlopeAdjustedWorldDirVector.GetSafeNormal(), 1.f);
		}
		else
		{
			GetStateManager()->SwitchState((uint8)ELocomotionSpace::ELS_Physical);
		}
	}
}

bool UTrackingSpaceMovementComponent::IsSwiningArms(IVRCharacterInterface* vrChar)
{
	if (!vrChar) return false;
	UVelocitySensor* rightHandSensor = vrChar->GetMotionControllerVelocitySensor(true);
	if (!rightHandSensor) return false;
	UVelocitySensor* leftHandSensor = vrChar->GetMotionControllerVelocitySensor(false);
	if (!leftHandSensor) return false;
	float rightContrVelocity = rightHandSensor->GetVelocity().SizeSquared();
	float leftContrVelocity = leftHandSensor->GetVelocity().SizeSquared();
	return rightContrVelocity > SwiningThreshold * SwiningThreshold && leftContrVelocity > SwiningThreshold * SwiningThreshold;
}

bool UTrackingSpaceMovementComponent::CheckObstacles(IVRCharacterInterface* vrChar, FHitResult& outRes)
{
	if (!vrChar) return false;
	UObstacleSensor* sensor = vrChar->GetObstacleSensor();
	if (!sensor) return false;
	outRes = sensor->GetHit();
	return sensor->IsHit();
}

bool UTrackingSpaceMovementComponent::CheckObstaclesInDirection(IVRCharacterInterface* vrChar, FVector normDir, FHitResult& outRes)
{
	if (!vrChar) return false;
	UObstacleSensor* sensor = vrChar->GetObstacleSensor();
	if (!sensor) return false;
	sensor->SetScanDirection(normDir);
	sensor->DoScan(1.f);
	outRes = sensor->GetHit();
	return sensor->IsHit();
}

FVector UTrackingSpaceMovementComponent::AdjustInputForSlope(IVRCharacterInterface* vrChar, const FVector& input)
{
	if (!vrChar) return input;
	UGroundHitSensor* groundHitSensor = vrChar->GetGroundHitSensor();
	if (!groundHitSensor) return input;
	if (!groundHitSensor->IsHit()) return input;
	return FVector::VectorPlaneProject(input, groundHitSensor->GetHit().ImpactNormal).GetSafeNormal() * input.Size();
}
