// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/VRCharacter/Movement/TrackingSpaceMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "Characters/VRCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/Base/StateManagerComponent.h"
#include "Enums.h"

UTrackingSpaceMovementComponent::UTrackingSpaceMovementComponent(const FObjectInitializer& init) : Super(init)
{

}

void UTrackingSpaceMovementComponent::OnStateTick(float DeltaTime)
{
	HandleJump(DeltaTime);
	HandleMovement(DeltaTime);
}

void UTrackingSpaceMovementComponent::HandleMovement(float DeltaTime)
{
	IVRCharacterInterface* vrChar = GetContext();
	if (!vrChar) return;
	UCameraComponent* camera = vrChar->GetVRCamera();
	if (!camera) return;
	if (!vrChar->IsGrounded()) return;//No need to calculate the movement when we are in air
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
		
		if (vrChar->CheckObstacles(ObstacleDistanceDetection, TraceCapsuleHalfHeightMultipl, hit))//We hit obstacle
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
				FVector NormDirVector = WorldDirVector.GetSafeNormal();
				FHitResult secondHit;
				if (vrChar->CheckObstaclesInDirection(NormDirVector, ObstacleDistanceDetection, TraceCapsuleHalfHeightMultipl, secondHit))
				{
					WorldDirVector = FVector::ZeroVector;
				}
			}
		}

		if (!WorldDirVector.IsNearlyZero())
		{
			if (vrChar->IsSwiningArms(DeltaTime))//Run
			{
				vrChar->Run();
			}
			else//Walk
			{
				vrChar->Walk();
			}
			//Modify direction vector to slide across the floor normals
			FVector SlopeAdjustedWorldDirVector = vrChar->AdjustInputForSlope(WorldDirVector);
			//Call movement via UPawnFloatingMovement
			vrChar->Move(SlopeAdjustedWorldDirVector.GetSafeNormal(), 1.f);
		}
		else
		{
			GetStateManager()->SwitchState((uint8)ELocomotionSpace::ELS_Physical);
		}
	}
}

void UTrackingSpaceMovementComponent::HandleJump(float DeltaTime)
{
	IVRCharacterInterface* vrChar = GetContext();
	if (!vrChar) return;

}
