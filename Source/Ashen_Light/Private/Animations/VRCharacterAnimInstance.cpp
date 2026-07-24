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
	GetGroundVelocity();
	GetCameraIKTransform();
	GetMotionControllersIKTransform();
}

void UVRCharacterAnimInstance::GetCameraIKTransform()
{
	if (!SkeletalMesh || !CameraComponent) return;

	const FTransform MeshTransform = SkeletalMesh->GetComponentTransform();
	const FTransform HMDTransform = CameraComponent->GetComponentTransform();
	
	FTransform RelativeHMD = HMDTransform.GetRelativeTransform(MeshTransform);
	
	FVector NeckOffsetLocation = FVector(-10.0f, 0.0f, -8.0f);
	
	FVector NeckLocation = RelativeHMD.GetLocation() + RelativeHMD.GetRotation().RotateVector(NeckOffsetLocation);
	
	CameraIKTransform.SetLocation(NeckLocation);
	CameraIKTransform.SetRotation(RelativeHMD.GetRotation());
	CameraIKTransform.SetScale3D(FVector::OneVector);
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
		const FTransform MeshTransform = SkeletalMesh->GetComponentTransform();
		const FTransform ControllerWorldTransform = RightController->GetComponentTransform();
		FTransform RelativeRight = ControllerWorldTransform.GetRelativeTransform(MeshTransform);
		FRotator RotationOffsetRight = FRotator(0.0f, 90.0f, 0.0f);
		FQuat NewRotationRight = RelativeRight.GetRotation() * RotationOffsetRight.Quaternion();
		RightMotionControllerIKTransform.SetLocation(RelativeRight.GetLocation());
		RightMotionControllerIKTransform.SetRotation(NewRotationRight);
		RightMotionControllerIKTransform.SetScale3D(FVector::OneVector);
	}
	
	if (LeftController)
	{
		const FTransform MeshTransform = SkeletalMesh->GetComponentTransform();
		const FTransform ControllerWorldTransform = LeftController->GetComponentTransform();
		FTransform RelativeLeft = ControllerWorldTransform.GetRelativeTransform(MeshTransform);
		FRotator RotationOffsetLeft = FRotator(0.0f, -90.0f, 0.0f);
		FQuat NewRotationLeft = RelativeLeft.GetRotation() * RotationOffsetLeft.Quaternion();
		LeftMotionControllerIKTransform.SetLocation(RelativeLeft.GetLocation());
		LeftMotionControllerIKTransform.SetRotation(NewRotationLeft);
		LeftMotionControllerIKTransform.SetScale3D(FVector::OneVector);
	}	
}
