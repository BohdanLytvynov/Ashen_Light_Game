// Fill out your copyright notice in the Description page of Project Settings.


#include "Animations/VRCharacterAnimInstance.h"
#include "../../Public/Characters/VRCharacter.h"
#include "Camera/CameraComponent.h"
#include "MotionControllerComponent.h"

void UVRCharacterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	APawn* pawn = TryGetPawnOwner();
	if (!pawn) return;

	Self = Cast<AVRCharacter>(pawn);
	if (!Self) return;

	CameraComponent = Self->GetVRCamera();
	SkeletalMesh = Self->GetMesh();
	RightController = Self->GetRightMotionController();
	LeftController = Self->GetLeftMotionController();
}

void UVRCharacterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	GetCameraIKTransform();
	GetMotionControllersIKTransform();
}

void UVRCharacterAnimInstance::GetCameraIKTransform()
{
	if (!CameraComponent || !SkeletalMesh) return;
	//Get the Location of the Camera component in a Component Space of the Skeletal Mesh
	CameraIKTransform = CameraComponent->GetComponentTransform().GetRelativeTransform(SkeletalMesh->GetComponentTransform());
}

void UVRCharacterAnimInstance::GetGroundVelocity()
{
	if (!Self) return;
	GroundVelocity = Self->GetGroundVelocityRatio();
}

void UVRCharacterAnimInstance::GetMotionControllersIKTransform()
{
	if (!SkeletalMesh) return;
	if (RightController)
	{
		RightMotionControllerIKTransform = RightController->GetComponentTransform().GetRelativeTransform(SkeletalMesh->GetComponentTransform());
	}
	
	if (LeftController)
	{
		LeftMotionControllerIKTransform = LeftController->GetComponentTransform().GetRelativeTransform(SkeletalMesh->GetComponentTransform());
	}
	
}
