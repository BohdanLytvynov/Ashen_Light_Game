// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "../../Public/Enums.h"
#include "VRCharacterAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class ASHEN_LIGHT_API UVRCharacterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
public:	
	UVRCharacterAnimInstance(const FObjectInitializer& init);
	/// <summary>
	/// Tick Analog, 
	/// </summary>
	/// <param name="DeltaSeconds"></param>
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	/// <summary>
	/// Occurs when smth changes in Animation Blueprint Editor. Also called when Bone's Transforms were calculated
	/// </summary>
	virtual void NativePostEvaluateAnimation() override;
	/// <summary>
	/// Set new Grip State
	/// </summary>
	/// <param name="right">true - right controller, false - left</param>
	/// <param name="pressed">true - pressed, false - released</param>
	FORCEINLINE void SetGripState(bool right, bool pressed)
	{
		if (right) bIsRightGripPressed = pressed;
		else bIsLeftGripPressed = pressed;

		UpdateControllerStates();
	}
	/// <summary>
	/// Set new Trigger State
	/// </summary>
	/// <param name="right">true - right controller, false - left</param>
	/// <param name="pressed">true - pressed, false - released</param>
	FORCEINLINE void SetTriggerState(bool right, bool pressed)
	{
		if (right) bIsRightTriggerPressed = pressed;
		else bIsLeftTriggerPressed = pressed;

		UpdateControllerStates();
	}
	/// <summary>
	/// Choose propriate Controller State for propriate Motion Controller
	/// </summary>
	FORCEINLINE void UpdateControllerStates()
	{
		if (bIsRightTriggerPressed)
		{
			RightMotionControllerAnimState = EVRControllerState::VRCS_Triggering;
		}
		else if (bIsRightGripPressed)
		{
			RightMotionControllerAnimState = EVRControllerState::VRCS_Grabbing;
		}
		else
		{
			RightMotionControllerAnimState = EVRControllerState::VRCS_Neutral;
		}

		if (bIsLeftTriggerPressed)
		{
			LeftMotionControllerAnimState = EVRControllerState::VRCS_Triggering;
		}
		else if (bIsLeftGripPressed)
		{
			LeftMotionControllerAnimState = EVRControllerState::VRCS_Grabbing;
		}
		else
		{
			LeftMotionControllerAnimState = EVRControllerState::VRCS_Neutral;
		}
	}
	/// <summary>
	/// Calculates universal scale factor. Location of the camera in the Skeletal Mesh basis / Location of the head bone in the same basis. Also sets delta value for Skel  mesh position adjustment
	/// </summary>
	/// <returns></returns>
	void CalculateUniversalScaleFactor(float cameraLocation);
	/// <summary>
	/// Will be called in Tick of the Pawn
	/// </summary>
	/// <param name="DeltaTime"></param>
	void CalculateElbowJointTarget(float DeltaTime, bool debug = false);
	/// <summary>
	/// Calculate the proper rotation of the spine bones. Will be called in the Tick of the pawn
	/// </summary>
	/// <param name="DeltaTime"></param>
	/// <param name="debug"></param>
	void CalculateSpineRotation(float DeltaTime, bool debug = false);
protected:
	class AVRCharacter* Self;
	class UCameraComponent* CameraComponent;
	class USkeletalMeshComponent* SkeletalMesh;
	class UMotionControllerComponent* LeftController;
	UMotionControllerComponent* RightController;

	UPROPERTY(BlueprintReadOnly)
	float UseHeadIK = 1.f;//Used to turn on / off head IK

	UPROPERTY(BlueprintReadOnly)
	float IsCrouching = 0.f;//Used to turn on / off leg IK

	UPROPERTY(BlueprintReadOnly)
	float CrouchDepth = 0.f;

	UPROPERTY(BlueprintReadOnly)
	float GroundVelocity;

	UPROPERTY(BlueprintReadOnly)
	FTransform CameraIKTransform;

	UPROPERTY(BlueprintReadOnly)
	FTransform RightMotionControllerIKTransform;

	UPROPERTY(BlueprintReadOnly)
	FTransform LeftMotionControllerIKTransform;
		
	UPROPERTY(EditDefaultsOnly, Category = "VR IK Preview")
	FVector PreviewVRCameraLocation = FVector(0.f, 0.f, 186.f);

	UPROPERTY(EditDefaultsOnly, Category = "VR IK Preview")
	FVector RightMotionControllerLocation = FVector(0.f, 0.f, 0.f);

	UPROPERTY(EditDefaultsOnly, Category = "VR IK Preview")
	FVector LeftMotionControllerLocation = FVector(0.f, 0.f, 0.f);

	UPROPERTY(EditDefaultsOnly, Category = "VR IK Preview")
	FVector NeckOffset = FVector(-10.0f, 0.0f, -8.0f);

	UPROPERTY(EditDefaultsOnly, Category = "VR HAND IK")
	float ElbowJointTargetInterpolationConstant = 3.f;

	UPROPERTY(EditDefaultsOnly, Category = "VR HAND IK | Right")
	FString RightIndexSocketName = FString("index_01_r_s");

	UPROPERTY(EditDefaultsOnly, Category = "VR HAND IK | Right")
	FString RightMiddleSocketName = FString("middle_01_r_s");

	UPROPERTY(EditDefaultsOnly, Category = "VR HAND IK | Right")
	FString RightHandSocketName = FString("hand_r_s");

	UPROPERTY(EditDefaultsOnly, Category = "VR HAND IK | Right")
	FString RightLowerArmSocketName = FString("lowerarm_r_s");

	UPROPERTY(EditDefaultsOnly, Category = "VR HAND IK | Right")
	FString RightUpperArmSocketName = FString("upperarm_r_s");

	UPROPERTY(EditDefaultsOnly, Category = "VR HAND IK | Right")
	FString RightLowerArmBoneName = FString("lowerarm_r");

	UPROPERTY(VisibleAnywhere, Category = "VR HAND IK | Right")
	FVector RightPalmPlaneNormal = FVector::ZeroVector;

	UPROPERTY(EditDefaultsOnly, Category = "VR HAND IK | Right")
	float RightElbowIKThreshold = 100.f;

	UPROPERTY(EditDefaultsOnly, Category = "VR HAND IK | Right")
	FVector DefRightElbowJointTarget = FVector(-55.f, 0.f, 93.f);

	UPROPERTY(EditDefaultsOnly, Category = "VR HAND IK | Left")
	FString LeftIndexSocketName = FString("index_01_l_s");

	UPROPERTY(EditDefaultsOnly, Category = "VR HAND IK | Left")
	FString LeftMiddleSocketName = FString("middle_01_l_s");

	UPROPERTY(EditDefaultsOnly, Category = "VR HAND IK | Left")
	FString LeftHandSocketName = FString("hand_l_s");

	UPROPERTY(EditDefaultsOnly, Category = "VR HAND IK | Left")
	FString LeftLowerArmSocketName = FString("lowerarm_l_s");

	UPROPERTY(EditDefaultsOnly, Category = "VR HAND IK | Left")
	FString LeftUpperArmSocketName = FString("upperarm_l_s");

	UPROPERTY(EditDefaultsOnly, Category = "VR HAND IK | Left")
	FString LeftLowerArmBoneName = FString("lowerarm_l");

	UPROPERTY(VisibleAnywhere, Category = "VR HAND IK | Left")
	FVector LeftPalmPlaneNormal = FVector::ZeroVector;

	UPROPERTY(EditDefaultsOnly, Category = "VR HAND IK | Left")
	float LeftElbowIKThreshold = 100.f;

	UPROPERTY(EditDefaultsOnly, Category = "VR HAND IK | Left")
	FVector DefLeftElbowJointTarget = FVector(55.f, 0.f, 93.f);

	UPROPERTY(EditDefaultsOnly, Category = "VR HEAD IK")
	FString HeadName = FString("head");

	UPROPERTY(EditDefaultsOnly, Category = "VR LEG IK")
	float CrouchInterpolationConstant = 5.f;

	UPROPERTY(EditDefaultsOnly, Category = "VR LEG IK")
	float HeadIKInterpolationConstant = 5.f;

	UPROPERTY(EditDefaultsOnly, Category = "VR SPINE IK")
	FString Spine03SocketName = FString("spine_03_s");

	UPROPERTY(EditDefaultsOnly, Category = "VR SPINE IK")
	float Spine01BoneWeight = 0.15f;

	UPROPERTY(EditDefaultsOnly, Category = "VR SPINE IK")
	float Spine02BoneWeight = 0.35f;

	UPROPERTY(EditDefaultsOnly, Category = "VR SPINE IK")
	float Spine03BoneWeight = 0.50f;

	UPROPERTY(EditDefaultsOnly, Category = "VR SPINE IK")
	float SpineIKInterpolationConstant = 3.f;

	UPROPERTY(BlueprintReadOnly)
	EVRControllerState RightMotionControllerAnimState = EVRControllerState::VRCS_Neutral;

	UPROPERTY(BlueprintReadOnly)
	EVRControllerState LeftMotionControllerAnimState = EVRControllerState::VRCS_Neutral;

	UPROPERTY(BlueprintReadOnly)
	float UniversalScaleFactor = 1.f;

	UPROPERTY(BlueprintReadOnly)
	FVector LeftElbowJointTargetLocation;

	UPROPERTY(BlueprintReadOnly)
	FVector RightElbowJointTargetLocation;

	UPROPERTY(BlueprintReadOnly)
	float Spine03Rot;

	UPROPERTY(BlueprintReadOnly)
	float Spine02Rot;

	UPROPERTY(BlueprintReadOnly)
	float Spine01Rot;
private:
	/// <summary>
	/// Calculate Transform for Head Bone
	/// </summary>
	void GetCameraIKTransform();
	/// <summary>
	/// Get Velocity in XY Plane
	/// </summary>
	void GetGroundVelocity();
	/// <summary>
	/// Get Transform for Motion Controller IK
	/// </summary>
	void GetMotionControllersIKTransform();
	/// <summary>
	/// Calculates the Motion Controller Transform in a Skeletal Mesh Space
	/// </summary>
	/// <param name="comp">Motion Controller Component</param>
	/// <param name="rotationOffset">Additional rotation offset</param>
	void CalculateMotionControllerTransform(UMotionControllerComponent* comp, const FRotator& rotationOffset, FTransform& motionControllerTransform);		
	/// <summary>
	/// Initialization of the Anim Instance in PIE mode
	/// </summary>
	void Initialize();
	/// <summary>
	/// True if we are in PIE mode and Play button was hit
	/// </summary>
	/// <returns></returns>	
	bool IsInGameWorld();
	/// <summary>
	/// Calculates the position of the bone for Preview in VR Editor
	/// </summary>
	void GetCameraIKPositionForPreview();
	/// <summary>
	/// Calculate the position of the left motion contoller based on it's location that is set from Editor
	/// </summary>
	void GetLeftMotionControllerPositionForPreview();
	/// <summary>
	/// Calculate the position of the right motion contoller based on it's location that is set from Editor
	/// </summary>
	void GetRightMotionControllerPositionForPreview();
	/// <summary>
	/// Gets transform of the Bone in a Skeletal mesh basis
	/// </summary>
	/// <param name="boneName">Name of the bone</param>
	/// <returns></returns>
	FTransform GetBoneTransform(FName boneName);
	/// <summary>
	/// Gets bone transform relative to the particular Space
	/// </summary>
	/// <param name="boneName"></param>
	/// <param name="space"></param>
	/// <returns></returns>
	FTransform GetBoneTransform(FName boneName, ERelativeTransformSpace space);
	/// <summary>
	/// Same function but for the Preview. We use value from the editor UI
	/// </summary>
	/// <returns></returns>
	float GetUniversalScaleFactorForPreview();
	/// <summary>
	/// Do we use crouching and controls IK of the Head and Legs
	/// </summary>
	/// <param name="DeltaTime"></param>
	void CalculateCrouching(float DeltaTime);
	/// <summary>
	/// Calculates base bone distances
	/// </summary>
	void CalculateIKDistances();
	/// <summary>
	/// Elbow Joint Target Calculation
	/// </summary>
	/// <param name="rigth">True for right</param>
	/// <param name="deltaTime"></param>
	void CalculateElbowJointTarget(bool rigth, float deltaTime, bool debug = false);
	/// <summary>
	/// Calculates Spine Rotation of spine_01, spine_02, spine_03 according to weights
	/// </summary>
	/// <param name="right"></param>
	/// <param name="deltaTime"></param>
	/// <param name="debug"></param>
	void CalculateSpineRotationAccordingToTheControllerLocation(bool right, float deltaTime, bool debug = false);
#pragma region State
	bool m_Initialized;//Do we perform initialization of the blueprint
	/// IK Distances
	float handLength;//Length of the hand with scale factor
	float lowerArmLength;//Length of the lowerArm with scale factor
	float elbowJointTargetDistance;//Scalar distance from elbow to the Joint Target
	float leftSpineTargetAngle = 0.f;
	float rightSpineTargetAngle = 0.f;
	bool bIsRightGripPressed = false;
	bool bIsRightTriggerPressed = false;
	bool bIsLeftGripPressed = false;
	bool bIsLeftTriggerPressed = false;
#pragma endregion

};
