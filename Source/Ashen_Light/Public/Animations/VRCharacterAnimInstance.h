// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "VRCharacterAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class ASHEN_LIGHT_API UVRCharacterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
public:
	virtual void NativeInitializeAnimation() override;

	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
protected:
	class AVRCharacter* Self;
	class UCameraComponent* CameraComponent;
	class USkeletalMeshComponent* SkeletalMesh;
	class UMotionControllerComponent* LeftController;
	UMotionControllerComponent* RightController;

	UPROPERTY(BlueprintReadOnly)
	float GroundVelocity;

	UPROPERTY(BlueprintReadOnly)
	FTransform CameraIKTransform;

	UPROPERTY(BlueprintReadOnly)
	FTransform RightMotionControllerIKTransform;

	UPROPERTY(BlueprintReadOnly)
	FTransform LeftMotionControllerIKTransform;
private:
	void GetCameraIKTransform();
	void GetGroundVelocity();
	void GetMotionControllersIKTransform();
};
