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

	FORCEINLINE FName GetHandSocketName(bool right)
	{
		return right ? RightHandSocketName : LeftHandSocketName;
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
	void CalculateElbowJointTarget(float DeltaTime);
	/// <summary>
	/// Calculate the proper rotation of the spine bones. Will be called in the Tick of the pawn
	/// </summary>
	/// <param name="DeltaTime"></param>
	/// <param name="debug"></param>
	void CalculateSpineRotation(float DeltaTime);

	void CalculateFootIKEffectors(AActor* current, float DeltaTime);

	void CalculateFootHeight();

	void CalculateMotionControllerTransform(class UMotionControllerComponent* comp, bool right, float DeltaTime);
protected:
	class AVRCharacter* Self;
	class UCameraComponent* CameraComponent;
	class USkeletalMeshComponent* SkeletalMesh;
	UMotionControllerComponent* LeftController;
	UMotionControllerComponent* RightController;

	UPROPERTY(BlueprintReadOnly)
	float UseHeadIK = 1.f;//Used to turn on / off head IK
	
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

	UPROPERTY(EditDefaultsOnly, Category = "VR HAND IK")
	float ElbowJointTargetInterpolationConstant = 3.f;

	UPROPERTY(EditDefaultsOnly, Category = "VR HAND IK | Right")
	FName RightIndexSocketName = FName(TEXT("index_01_r_s"));

	UPROPERTY(EditDefaultsOnly, Category = "VR HAND IK | Right")
	FName RightMiddleSocketName = FName(TEXT("middle_01_r_s"));

	UPROPERTY(EditDefaultsOnly, Category = "VR HAND IK | Right")
	FName RightHandSocketName = FName(TEXT("hand_r_s"));

	UPROPERTY(EditDefaultsOnly, Category = "VR HAND IK | Right")
	FName RightLowerArmSocketName = FName(TEXT("lowerarm_r_s"));

	UPROPERTY(EditDefaultsOnly, Category = "VR HAND IK | Right")
	FName RightUpperArmSocketName = FName(TEXT("upperarm_r_s"));

	UPROPERTY(EditDefaultsOnly, Category = "VR HAND IK | Right")
	FName RightLowerArmBoneName = FName(TEXT("lowerarm_r"));

	UPROPERTY(VisibleAnywhere, Category = "VR HAND IK | Right")
	FVector RightPalmPlaneNormal = FVector::ZeroVector;

	UPROPERTY(EditDefaultsOnly, Category = "VR HAND IK | Right")
	float RightElbowIKThreshold = 100.f;

	UPROPERTY(EditDefaultsOnly, Category = "VR HAND IK | Right")
	FVector DefRightElbowJointTarget = FVector(-55.f, 0.f, 93.f);

	UPROPERTY(EditDefaultsOnly, Category = "VR HAND IK | Left")
	FName LeftIndexSocketName = FName(TEXT("index_01_l_s"));

	UPROPERTY(EditDefaultsOnly, Category = "VR HAND IK | Left")
	FName LeftMiddleSocketName = FName(TEXT("middle_01_l_s"));

	UPROPERTY(EditDefaultsOnly, Category = "VR HAND IK | Left")
	FName LeftHandSocketName = FName(TEXT("hand_l_s"));

	UPROPERTY(EditDefaultsOnly, Category = "VR HAND IK | Left")
	FName LeftLowerArmSocketName = FName(TEXT("lowerarm_l_s"));

	UPROPERTY(EditDefaultsOnly, Category = "VR HAND IK | Left")
	FName LeftUpperArmSocketName = FName(TEXT("upperarm_l_s"));

	UPROPERTY(EditDefaultsOnly, Category = "VR HAND IK | Left")
	FName LeftLowerArmBoneName = FName(TEXT("lowerarm_l"));

	UPROPERTY(VisibleAnywhere, Category = "VR HAND IK | Left")
	FVector LeftPalmPlaneNormal = FVector::ZeroVector;

	UPROPERTY(EditDefaultsOnly, Category = "VR HAND IK | Left")
	float LeftElbowIKThreshold = 100.f;

	UPROPERTY(EditDefaultsOnly, Category = "VR HAND IK | Left")
	FVector DefLeftElbowJointTarget = FVector(55.f, 0.f, 93.f);

	UPROPERTY(EditDefaultsOnly, Category = "VR HAND IK | Left")
	FRotator LeftHandRotOffset = FRotator(0.f, 0.f, -180.f);

	UPROPERTY(EditDefaultsOnly, Category = "VR HAND IK | Right")
	FRotator RightHandRotOffset = FRotator(0.f, -180.f, 0.f);

	UPROPERTY(EditDefaultsOnly, Category = "VR HAND IK | Right")
	float RightObstacleSensorDistanceMultipl = 1.f;

	UPROPERTY(EditDefaultsOnly, Category = "VR HAND IK | Left")
	float LeftObstacleSensorDistanceMultipl = 1.f;

	UPROPERTY(EditDefaultsOnly, Category = "VR HAND IK | Right", meta = (Tooltip = "We only need this socket for the left side."))
	FName LeftMiddleEndSocket = FName("middle_03_lSocket");

	UPROPERTY(EditDefaultsOnly, Category = "VR HAND IK")
	float middleDistantFalangLength = 10.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "VR HEAD IK")
	FName HeadName = FName(TEXT("head"));

	UPROPERTY(EditDefaultsOnly, Category = "VR LEG IK")
	float CrouchInterpolationConstant = 5.f;

	UPROPERTY(EditDefaultsOnly, Category = "VR LEG IK")
	float HeadIKInterpolationConstant = 5.f;

	UPROPERTY(EditDefaultsOnly, Category = "VR LEG IK")
	float LegIKInterpolationConstant = 3.f;

	UPROPERTY(EditDefaultsOnly, Category = "VR LEG IK")
	float LayerBlendInterpolationConstant = 0.5f;

	UPROPERTY(EditDefaultsOnly, Category = "VR LEG IK | Right")
	FName RightFootBoneSocketName = FName(TEXT("foot_rSocket"));

	UPROPERTY(EditDefaultsOnly, Category = "VR LEG IK | Left")
	FName LeftFootBoneSocketName = FName(TEXT("foot_lSocket"));

	UPROPERTY(EditDefaultsOnly, Category = "VR LEG IK")
	float FloorDetectionThreshold = 10.f;

	UPROPERTY(EditDefaultsOnly, Category = "VR LEG IK")
	FName RightBallSocketName = FName(TEXT("ball_r_Socket"));
	
	UPROPERTY(EditDefaultsOnly, Category = "VR SPINE IK")
	FName Spine03SocketName = FName(TEXT("spine_03_s"));

	UPROPERTY(EditDefaultsOnly, Category = "VR SPINE IK")
	float Spine01BoneWeight = 0.15f;

	UPROPERTY(EditDefaultsOnly, Category = "VR SPINE IK")
	float Spine02BoneWeight = 0.35f;

	UPROPERTY(EditDefaultsOnly, Category = "VR SPINE IK")
	float Spine03BoneWeight = 0.50f;

	UPROPERTY(EditDefaultsOnly, Category = "VR SPINE IK")
	float SpineIKInterpolationConstant = 3.f;

	UPROPERTY(EditDefaultsOnly, Category = "VR IK Debug")
	bool DebugElbowJointTarget = false;

	UPROPERTY(EditDefaultsOnly, Category = "VR IK Debug")
	bool DebugFootIKEffectors = false;

	UPROPERTY(EditDefaultsOnly, Category = "VR IK Debug")
	bool DebugSpineRotation = false;

	UPROPERTY(EditDefaultsOnly, Category = "VR IK Debug")
	bool DebugHandObstacleSensor = false;

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

	UPROPERTY(BlueprintReadOnly)
	float LegIkEnable;

	UPROPERTY(BlueprintReadOnly)
	float EnableLayerBlendForLegs;

	UPROPERTY(BlueprintReadOnly)
	FVector LeftFootEffectorLocation;

	UPROPERTY(BlueprintReadOnly)
	FVector RightFootEffectorLocation;

	UPROPERTY(BlueprintReadOnly, Category = "VR IK")
	FVector RightHandPushOutOffset = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "VR IK")
	FVector LeftHandPushOutOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, Category = "VR IKConfig")
	float PushOutInterpSpeed = 20.0f;

	UFUNCTION(BlueprintPure)
	bool IsGrounded() const;
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
	void CalculateElbowJointTarget(bool rigth, float deltaTime);

	void CalculateFootIKEffector(AActor* currentActor, bool right, float DeltaTime);

	/// <summary>
	/// Calculates Spine Rotation of spine_01, spine_02, spine_03 according to weights
	/// </summary>
	/// <param name="right"></param>
	/// <param name="deltaTime"></param>
	/// <param name="debug"></param>
	void CalculateSpineRotationAccordingToTheControllerLocation(bool right, float deltaTime);

	void CalculateLegIkEnable(float DeltaTime);

	void CalculateLayerBlendForLegs(float DeltaTime);
	void CalculateDefaultMotionControllerIKTransform(UMotionControllerComponent* comp,
		bool right,
		FTransform& mcTransform,
		const FVector& pushOutOffset);
#pragma region State
	bool m_Initialized;//Do we perform initialization of the blueprint
	/// IK Distances
	float handLength;//Length of the hand with scale factor
	float lowerArmLength;//Length of the lowerArm with scale factor
	float elbowJointTargetDistance;//Scalar distance from elbow to the Joint Target
	float leftSpineTargetAngle = 0.f;//Angle for spine rotation when we move left controller forward
	float rightSpineTargetAngle = 0.f;//Angle for spine rotation when we move right controller forward
	bool bIsRightGripPressed = false;
	bool bIsRightTriggerPressed = false;
	bool bIsLeftGripPressed = false;
	bool bIsLeftTriggerPressed = false;
	float CameraHeadDelta = 0.f;
	float footHeight;
	float middleFingerLength;
#pragma endregion

};
