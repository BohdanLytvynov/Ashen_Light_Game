// Fill out your copyright notice in the Description page of Project Settings.


#include "Animations/VRCharacterAnimInstance.h"
#include "Characters/VRCharacter.h"
#include "Camera/CameraComponent.h"
#include "MotionControllerComponent.h"
#include "DrawDebugHelpers.h"


UVRCharacterAnimInstance::UVRCharacterAnimInstance(const FObjectInitializer& init) : Super(init)
{
	m_Initialized = false;
	RightElbowJointTargetLocation = DefRightElbowJointTarget;
	LeftElbowJointTargetLocation = DefLeftElbowJointTarget;
}

void UVRCharacterAnimInstance::Initialize()
{
	if (!m_Initialized)
	{
		//Get Owning Sekeltal Mesh Component
		SkeletalMesh = GetSkelMeshComponent();
		APawn* pawn = TryGetPawnOwner();
		if (!pawn) return;
		//Fail to Cast To Self
		Self = Cast<AVRCharacter>(pawn);
		if (!Self) return;
		//Init local pointers
		CameraComponent = Self->GetVRCamera();
		RightController = Self->GetRightMotionController();
		LeftController = Self->GetLeftMotionController();
		SkeletalMesh->HideBoneByName(FName(HeadName), EPhysBodyOp::PBO_None);
		m_Initialized = true;
		return;
	}
}

bool UVRCharacterAnimInstance::IsInGameWorld()
{
	UWorld* world = GetWorld();
	return world && world->IsGameWorld();
}

void UVRCharacterAnimInstance::GetCameraIKPositionForPreview()
{	
	FTransform headTransform = GetBoneTransform(FName(HeadName));
	CameraIKTransform.SetRotation(headTransform.GetRotation());
	CameraIKTransform.SetLocation(headTransform.GetLocation() + PreviewVRCameraLocation);
	CameraIKTransform.SetScale3D(FVector::OneVector);
}

void UVRCharacterAnimInstance::GetLeftMotionControllerPositionForPreview()
{
	FTransform rightHandTransform = GetBoneTransform(FName(LeftLowerArmBoneName));
	RightMotionControllerIKTransform.SetRotation(rightHandTransform.GetRotation());
	RightMotionControllerIKTransform.SetLocation(rightHandTransform.GetLocation() + RightMotionControllerLocation);
	RightMotionControllerIKTransform.SetScale3D(FVector::OneVector);
}

void UVRCharacterAnimInstance::GetRightMotionControllerPositionForPreview()
{
	FTransform leftHandTransform = GetBoneTransform(FName(RightLowerArmBoneName));
	LeftMotionControllerIKTransform.SetRotation(leftHandTransform.GetRotation());
	LeftMotionControllerIKTransform.SetLocation(leftHandTransform.GetLocation() + LeftMotionControllerLocation);
	LeftMotionControllerIKTransform.SetScale3D(FVector::OneVector);
}

FTransform UVRCharacterAnimInstance::GetBoneTransform(FName boneName)
{
	if (!SkeletalMesh) return FTransform::Identity;
	//Socket == bone
	return SkeletalMesh->GetSocketTransform(boneName, ERelativeTransformSpace::RTS_Component);
}

FTransform UVRCharacterAnimInstance::GetBoneTransform(FName boneName, ERelativeTransformSpace space)
{
	if (!SkeletalMesh) return FTransform::Identity;
	//Socket == bone
	return SkeletalMesh->GetSocketTransform(boneName, space);
}

void UVRCharacterAnimInstance::CalculateUniversalScaleFactor(float cameraLocation)
{
	if (!SkeletalMesh || !Self) return;
	//Now we need reference skeleton. In the initial pose.
	const FReferenceSkeleton& RefSkeleton = SkeletalMesh->SkeletalMesh->GetRefSkeleton();
	//Get index of the Bone
	int32 HeadIndex = RefSkeleton.FindBoneIndex(FName(HeadName));
	if (HeadIndex == INDEX_NONE) return;
	FTransform RefHeadTransform = FTransform::Identity;
	int32 CurrBoneIndex = HeadIndex;
	//Head bone is located within the hierarchy of the bones. 
	// We need to go from current bone to its root bone and multiply Transformation Matrices
	const FTransformArrayA2 trArray = RefSkeleton.GetRefBonePose();
	while (CurrBoneIndex != INDEX_NONE)
	{
		RefHeadTransform = RefHeadTransform * trArray[CurrBoneIndex];
		//Parent Bone index
		CurrBoneIndex = RefSkeleton.GetParentIndex(CurrBoneIndex);
	}
	//Transform of the bone relative to the root bone of the Skeletal mesh
	float BaseHeadZ = RefHeadTransform.GetLocation().Z;
	if (FMath::IsNearlyZero(BaseHeadZ)) return;
	//Send delta (|headBone - CameraPosition|) to Pawn to adjust mesh in Z coord
	CameraHeadDelta = FMath::Abs(cameraLocation - BaseHeadZ);
	Self->UpdateCameraHeadDelta(CameraHeadDelta);
	UniversalScaleFactor = cameraLocation / BaseHeadZ;//Scale factor
	//As we change scale factor we need to recalculate distances for IK
	CalculateIKDistances();
}

void UVRCharacterAnimInstance::CalculateElbowJointTarget(float DeltaTime, bool debug)
{
	CalculateElbowJointTarget(true, DeltaTime, debug);
	CalculateElbowJointTarget(false, DeltaTime, debug);
}

void UVRCharacterAnimInstance::CalculateSpineRotation(float DeltaTime, bool debug)
{
	CalculateSpineRotationAccordingToTheControllerLocation(true, DeltaTime, debug);
	CalculateSpineRotationAccordingToTheControllerLocation(false, DeltaTime, debug);
}

float UVRCharacterAnimInstance::GetUniversalScaleFactorForPreview()
{
	//Here logic is simple we use value from the editor
	if (!SkeletalMesh) return 1.0f;
	FTransform headBoneTransform = GetBoneTransform(FName(HeadName));
	float headZ = headBoneTransform.GetLocation().Z;
	if (FMath::IsNearlyZero(headZ))
	{
		return 1.0f;
	}
	float factor = PreviewVRCameraLocation.Z / headZ;
	return FMath::IsNearlyZero(factor) ? 1.f : factor;
}

void UVRCharacterAnimInstance::CalculateCrouching(float DeltaTime)
{
	if (!Self) return;
	float depth = 0.f;
	bool crouching = Self->IsCrouching(&depth);
	float newCrouching = crouching == true ? 1.f : 0.f;

	if (crouching)
	{
		CrouchDepth = depth;
		UseHeadIK = FMath::FInterpTo(UseHeadIK, 0.f, DeltaTime, HeadIKInterpolationConstant);
	}
	else
	{
		CrouchDepth = CameraHeadDelta;
		UseHeadIK = FMath::FInterpTo(UseHeadIK, 1.f, DeltaTime, HeadIKInterpolationConstant);
	}
}

void UVRCharacterAnimInstance::CalculateIKDistances()
{
	if (!SkeletalMesh) return;
	FVector shoulderPos = SkeletalMesh->GetSocketTransform(FName(RightUpperArmSocketName), ERelativeTransformSpace::RTS_Component).GetLocation();
	FVector elbowPos = SkeletalMesh->GetSocketTransform(FName(RightLowerArmSocketName), ERelativeTransformSpace::RTS_Component).GetLocation();
	FVector wristPos = SkeletalMesh->GetSocketTransform(FName(RightHandSocketName), ERelativeTransformSpace::RTS_Component).GetLocation();

	lowerArmLength = FVector::Distance(shoulderPos, elbowPos);
	handLength = FVector::Distance(elbowPos, wristPos);
	elbowJointTargetDistance = (lowerArmLength + handLength) * 0.5f;
}

void UVRCharacterAnimInstance::CalculateElbowJointTarget(bool right, float deltaTime, bool debug)
{
	if (!SkeletalMesh) return;
	UMotionControllerComponent* controller = right ? RightController : LeftController;
	if (!controller) return;
	//Get All required Sockets	
	FString handSocket = right ? RightHandSocketName : LeftHandSocketName;
	FString middleSocket = right ? RightMiddleSocketName : LeftMiddleSocketName;
	FString indexSocket = right ? RightIndexSocketName : LeftIndexSocketName;
	FString shoulderSocket = right ? RightUpperArmSocketName : LeftUpperArmSocketName;
	FString elbowSocket = right ? RightLowerArmSocketName : LeftLowerArmSocketName;
	//Build Palm Plane
	FVector middleToHand = (SkeletalMesh->GetSocketLocation(FName(middleSocket)) - SkeletalMesh->GetSocketLocation(FName(handSocket))).GetSafeNormal();
	FVector middleToIndex = (SkeletalMesh->GetSocketLocation(FName(indexSocket)) - SkeletalMesh->GetSocketLocation(FName(handSocket))).GetSafeNormal();
	FVector planeNormal = FVector::CrossProduct(middleToHand, middleToIndex);
	FVector handAxis = (SkeletalMesh->GetSocketLocation(FName(handSocket)) - SkeletalMesh->GetSocketLocation(FName(shoulderSocket))).GetSafeNormal();
	FVector elbowLocation = SkeletalMesh->GetSocketLocation(FName(elbowSocket));
	FVector referencedNormalPlane;	
	if (right)
	{
		planeNormal *= -1.f;
	}
	float rightAngle = 0.f;
	float leftAngle = 0.f;
	FVector prjPlaneNormal = FVector::VectorPlaneProject(planeNormal, handAxis).GetSafeNormal();	
	FTransform skelMeshTransform = SkeletalMesh->GetComponentTransform();
	FVector elbowDir = FVector::CrossProduct(handAxis, prjPlaneNormal).GetSafeNormal();
	if (right)
	{
		elbowDir *= -1.f;
	}
	FVector newElbowJointTarget = elbowLocation + elbowDir * elbowJointTargetDistance;
	FVector newElbowJointTargetLocal = skelMeshTransform.InverseTransformPosition(newElbowJointTarget);
	if (right)
	{
		if (RightPalmPlaneNormal.IsNearlyZero())
			RightPalmPlaneNormal = prjPlaneNormal;

		referencedNormalPlane = RightPalmPlaneNormal;
		//Always Clamp [-1 ; 1]
		const float DotVal = FMath::Clamp(FVector::DotProduct(RightPalmPlaneNormal, prjPlaneNormal), -1.f, 1.f);
		rightAngle = FMath::RadiansToDegrees(FMath::Acos(DotVal));
		if (rightAngle > RightElbowIKThreshold)
		{
			RightElbowJointTargetLocation = FMath::VInterpTo(RightElbowJointTargetLocation, newElbowJointTargetLocal,
				deltaTime, ElbowJointTargetInterpolationConstant);
		}
		else
		{
			RightElbowJointTargetLocation = FMath::VInterpTo(RightElbowJointTargetLocation, DefRightElbowJointTarget, 
				deltaTime, ElbowJointTargetInterpolationConstant);
		}
	}
	else
	{
		if (LeftPalmPlaneNormal.IsNearlyZero())
			LeftPalmPlaneNormal = prjPlaneNormal;

		referencedNormalPlane = LeftPalmPlaneNormal;
		//Always Clamp [-1 ; 1]
		const float DotVal = FMath::Clamp(FVector::DotProduct(LeftPalmPlaneNormal, prjPlaneNormal), -1.f, 1.f);
		leftAngle = FMath::RadiansToDegrees(FMath::Acos(DotVal));
		if (leftAngle > LeftElbowIKThreshold)
		{
			LeftElbowJointTargetLocation = FMath::VInterpTo(LeftElbowJointTargetLocation, newElbowJointTargetLocal,
				deltaTime,  ElbowJointTargetInterpolationConstant);
		}
		else
		{
			LeftElbowJointTargetLocation = FMath::VInterpTo(LeftElbowJointTargetLocation, DefLeftElbowJointTarget,
				deltaTime, ElbowJointTargetInterpolationConstant);
		}
	}

	if (!debug)	return;
	FVector controllerPos = controller->GetComponentLocation();
	UWorld* w = GetWorld();
	DrawDebugDirectionalArrow(w, controllerPos, controllerPos + handAxis * 60.f, 10.f, FColor::Orange);
	DrawDebugDirectionalArrow(w, controllerPos, controllerPos + prjPlaneNormal * 40.f, 10.f, FColor::Blue);
	DrawDebugDirectionalArrow(w, controllerPos, controllerPos + referencedNormalPlane * 40.f, 10.f, FColor::Red);
	DrawDebugDirectionalArrow(w, controllerPos, controllerPos + referencedNormalPlane * 40.f, 10.f, FColor::Red);
	DrawDebugDirectionalArrow(w, elbowLocation, elbowLocation + elbowDir * 40.f, 10.f, FColor::Green);
	DrawDebugDirectionalArrow(w, elbowLocation, elbowLocation + elbowDir * 40.f, 10.f, FColor::Green);
	if (right)
	{
		if (GEngine)
			GEngine->AddOnScreenDebugMessage(1, 5, FColor::Red, FString::Printf(TEXT("Right Angle: %f"), rightAngle));
		DrawDebugSphere(w, skelMeshTransform.TransformPosition(RightElbowJointTargetLocation), 10.f, 8, FColor::Red);
	}
	else
	{
		if (GEngine)
			GEngine->AddOnScreenDebugMessage(2, 5, FColor::Red, FString::Printf(TEXT("Left Angle: %f"), leftAngle));
		DrawDebugSphere(w, skelMeshTransform.TransformPosition(LeftElbowJointTargetLocation), 10.f, 8, FColor::Green);
	}
}

void UVRCharacterAnimInstance::CalculateSpineRotationAccordingToTheControllerLocation(bool right, float deltaTime, bool debug)
{
	if (!SkeletalMesh) return;
	FString shoulderSocket = right ? RightUpperArmSocketName : LeftUpperArmSocketName;
	UMotionControllerComponent* controller = right ? RightController : LeftController;
	if (!controller) return;
	FVector shoulderPos = SkeletalMesh->GetSocketLocation(FName(shoulderSocket));
	//Calculate arm Vector
	FVector armVector = controller->GetComponentLocation() - shoulderPos;
	//Calculate current distance from shoulder to controller (dist squared)
	float distFromShoulderToController = armVector.Size();
	//Length of the arm
	float handLengthCached = handLength + lowerArmLength;
	float delta = distFromShoulderToController - handLengthCached;
	//Get Forwar Vector of the Skeletal Mesh Component
	FVector skelMeshForward = SkeletalMesh->GetRightVector();
	//Normalized armVector
	FVector armAxis = armVector.GetSafeNormal();
	float prjAxis = FVector::DotProduct(skelMeshForward, armAxis);
	float angle = 0.f;

	if (delta > 0.f && prjAxis > 0.f)
	{
		//Calculate Spine Rotation
		//Calculate angle for rotation
		FVector spine_03Location = SkeletalMesh->GetSocketLocation(FName(Spine03SocketName));
		float r = (shoulderPos - spine_03Location).Size();
		//sin(a) = delta / r, asin(sin(a)) = asin(delta / r) => a = asin(delta / r), asin(x) x in [-1 ; 1]
		angle = FMath::RadiansToDegrees(FMath::Asin(FMath::Clamp(delta / r, -1.f, 1.f)));
	}
	//Cache angle for right and left hand
	if (right)
	{
		angle *= -1.f;
		rightSpineTargetAngle = angle;
	}
	else
	{
		leftSpineTargetAngle = angle;
	}

	float targetRotAngle = rightSpineTargetAngle + leftSpineTargetAngle;

	//Calculate the propriate rotations of spine03, spine02, spine01
	Spine03Rot = FMath::FInterpTo(Spine03Rot, targetRotAngle * Spine03BoneWeight, deltaTime, SpineIKInterpolationConstant);
	Spine02Rot = FMath::FInterpTo(Spine02Rot, targetRotAngle * Spine02BoneWeight, deltaTime, SpineIKInterpolationConstant);
	Spine01Rot = FMath::FInterpTo(Spine01Rot, targetRotAngle * Spine01BoneWeight, deltaTime, SpineIKInterpolationConstant);
	
	if (!debug) return;
	UWorld* w = GetWorld();
	DrawDebugDirectionalArrow(w, controller->GetComponentLocation(),
		controller->GetComponentLocation() + armAxis * 20.f, 10.f, FColor::Orange);
	DrawDebugDirectionalArrow(w, controller->GetComponentLocation(),
		controller->GetComponentLocation() + skelMeshForward * 20.f, 10.f, FColor::Red);
	if (right)
	{
		if (GEngine)
			GEngine->AddOnScreenDebugMessage(4, 0.f, FColor::Red, FString::Printf(TEXT("Right Spine angle: %f"), angle));
	}
	else
	{
		if (GEngine)
			GEngine->AddOnScreenDebugMessage(5, 0.f, FColor::Red, FString::Printf(TEXT("Left Spine angle: %f"), angle));
	}
}

void UVRCharacterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	Initialize();
	if (IsInGameWorld())
	{
		//We are in real game World. We need real velocity and real IK Effectors Positions from devices
		//Don't compute the FTransforms of Bones and Sockets here use Pawn Tick for that
		GetGroundVelocity();
		GetCameraIKTransform();
		GetMotionControllersIKTransform();
		CalculateCrouching(DeltaSeconds);
	}
}

void UVRCharacterAnimInstance::NativePostEvaluateAnimation()
{
	if (!m_Initialized)
	{
		SkeletalMesh = GetSkelMeshComponent();
		UniversalScaleFactor = GetUniversalScaleFactorForPreview();
		m_Initialized = true;
	}

	//Not in PIE
	if (!IsInGameWorld())
	{
		GetCameraIKPositionForPreview();
		GetLeftMotionControllerPositionForPreview();
		GetRightMotionControllerPositionForPreview();
	}
}

void UVRCharacterAnimInstance::GetCameraIKTransform()
{
	if (!SkeletalMesh || !CameraComponent) return;
	const FTransform MeshTransform = SkeletalMesh->GetComponentTransform();
	const FTransform HMDTransform = CameraComponent->GetComponentTransform();
	FTransform RelativeHMD = HMDTransform.GetRelativeTransform(MeshTransform);
	FVector NeckLocation = RelativeHMD.GetLocation();
	CameraIKTransform.SetLocation(NeckLocation);
	CameraIKTransform.SetRotation(FQuat::Identity);
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
	CalculateMotionControllerTransform(RightController, FRotator(90.f, 0.f, 180.f), RightMotionControllerIKTransform);
	CalculateMotionControllerTransform(LeftController, FRotator(-90.f, 0.f, 180.f), LeftMotionControllerIKTransform);
}

void UVRCharacterAnimInstance::CalculateMotionControllerTransform(
	UMotionControllerComponent* comp, 
	const FRotator& rotationOffset, 
	FTransform& motionControllerTransform )
{
	if (!comp) return;

	//World Location of the Skeletal Mesh Component
	const FTransform MeshTransform = SkeletalMesh->GetComponentTransform();
	//World Location of the Right Motion Controller
	const FTransform ControllerWorldTransform = comp->GetComponentTransform();
	//Transform of the motion controller in the Skeletal Mesh Space
	FTransform RelativeController = ControllerWorldTransform.GetRelativeTransform(MeshTransform);
	//Multiplication of the Quaternions will give us the new rotation
	FQuat NewRotation = RelativeController.GetRotation() * rotationOffset.Quaternion();
	//Update Location, Rotation, Scale must be (1 , 1, 1)
	//This will be pined to the Effector Location
	motionControllerTransform.SetLocation(RelativeController.GetLocation());
	//Rotation will be used in Modify Bone Mode to rotate the wrist
	motionControllerTransform.SetRotation(NewRotation);
	motionControllerTransform.SetScale3D(FVector::OneVector);
}

bool UVRCharacterAnimInstance::IsGrounded() const
{
	if (Self)
	{
		return Self->IsGrounded();
	}

	return false;
}

