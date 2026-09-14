// Fill out your copyright notice in the Description page of Project Settings.


#include "Animations/VRCharacterAnimInstance.h"
#include "Characters/VRCharacter.h"
#include "Camera/CameraComponent.h"
#include "MotionControllerComponent.h"
#include "DrawDebugHelpers.h"
#include "Components/CapsuleComponent.h"
#include "Utilities/Sensors/SensorTraceUtility.h"
#include <Components/Base/HitSensor.h>


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
		RightController = Self->GetMotionController(true);
		LeftController = Self->GetMotionController(false);
		SkeletalMesh->HideBoneByName(HeadName, EPhysBodyOp::PBO_None);
		m_Initialized = true;
		return;
	}
}

bool UVRCharacterAnimInstance::IsGrounded() const
{
	if (!Self) return false;
	return Self->IsGrounded();
}

bool UVRCharacterAnimInstance::IsInGameWorld()
{
	UWorld* world = GetWorld();
	return world && world->IsGameWorld();
}

void UVRCharacterAnimInstance::GetCameraIKPositionForPreview()
{	
	FTransform headTransform = GetBoneTransform(HeadName);
	CameraIKTransform.SetRotation(headTransform.GetRotation());
	CameraIKTransform.SetLocation(headTransform.GetLocation() + PreviewVRCameraLocation);
	CameraIKTransform.SetScale3D(FVector::OneVector);
}

void UVRCharacterAnimInstance::GetLeftMotionControllerPositionForPreview()
{
	FTransform rightHandTransform = GetBoneTransform(LeftLowerArmBoneName);
	RightMotionControllerIKTransform.SetRotation(rightHandTransform.GetRotation());
	RightMotionControllerIKTransform.SetLocation(rightHandTransform.GetLocation() + RightMotionControllerLocation);
	RightMotionControllerIKTransform.SetScale3D(FVector::OneVector);
}

void UVRCharacterAnimInstance::GetRightMotionControllerPositionForPreview()
{
	FTransform leftHandTransform = GetBoneTransform(RightLowerArmBoneName);
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
	int32 HeadIndex = RefSkeleton.FindBoneIndex(HeadName);
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

void UVRCharacterAnimInstance::CalculateElbowJointTarget(float DeltaTime)
{
	CalculateElbowJointTarget(true, DeltaTime);
	CalculateElbowJointTarget(false, DeltaTime);
}

void UVRCharacterAnimInstance::CalculateSpineRotation(float DeltaTime)
{
	CalculateSpineRotationAccordingToTheControllerLocation(true, DeltaTime);
	CalculateSpineRotationAccordingToTheControllerLocation(false, DeltaTime);
}

void UVRCharacterAnimInstance::CalculateFootIKEffectors(AActor* current, float DeltaTime)
{
	CalculateFootIKEffector(current, true, DeltaTime);
	CalculateFootIKEffector(current, false, DeltaTime);
}

void UVRCharacterAnimInstance::CalculateFootHeight()
{
	if (!SkeletalMesh) return;
	FVector start = SkeletalMesh->GetSocketLocation(RightFootBoneSocketName);
	FVector end = SkeletalMesh->GetSocketLocation(RightBallSocketName);
	footHeight = start.Z - end.Z;
}

float UVRCharacterAnimInstance::GetUniversalScaleFactorForPreview()
{
	//Here logic is simple we use value from the editor
	if (!SkeletalMesh) return 1.0f;
	FTransform headBoneTransform = GetBoneTransform(HeadName);
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
	FVector shoulderPos = SkeletalMesh->GetSocketTransform(RightUpperArmSocketName, ERelativeTransformSpace::RTS_Component).GetLocation();
	FVector elbowPos = SkeletalMesh->GetSocketTransform(RightLowerArmSocketName, ERelativeTransformSpace::RTS_Component).GetLocation();
	FVector wristPos = SkeletalMesh->GetSocketTransform(RightHandSocketName, ERelativeTransformSpace::RTS_Component).GetLocation();
	FVector midStart = SkeletalMesh->GetSocketTransform(LeftMiddleSocketName, ERelativeTransformSpace::RTS_Component).GetLocation();
	FVector midEnd = SkeletalMesh->GetSocketTransform(LeftMiddleEndSocket, ERelativeTransformSpace::RTS_Component).GetLocation();
	lowerArmLength = FVector::Distance(shoulderPos, elbowPos);
	handLength = FVector::Distance(elbowPos, wristPos);
	elbowJointTargetDistance = (lowerArmLength + handLength) * 0.5f;
	middleFingerLength = FVector::Distance(midStart, midEnd) + middleDistantFalangLength;
}

void UVRCharacterAnimInstance::CalculateElbowJointTarget(bool right, float deltaTime)
{
	if (!SkeletalMesh) return;
	UMotionControllerComponent* controller = right ? RightController : LeftController;
	if (!controller) return;
	//Get All required Sockets	
	FName handSocket = right ? RightHandSocketName : LeftHandSocketName;
	FName middleSocket = right ? RightMiddleSocketName : LeftMiddleSocketName;
	FName indexSocket = right ? RightIndexSocketName : LeftIndexSocketName;
	FName shoulderSocket = right ? RightUpperArmSocketName : LeftUpperArmSocketName;
	FName elbowSocket = right ? RightLowerArmSocketName : LeftLowerArmSocketName;
	//Build Palm Plane
	FVector middleToHand = (SkeletalMesh->GetSocketLocation(middleSocket) - SkeletalMesh->GetSocketLocation(handSocket)).GetSafeNormal();
	FVector middleToIndex = (SkeletalMesh->GetSocketLocation(indexSocket) - SkeletalMesh->GetSocketLocation(handSocket)).GetSafeNormal();
	FVector planeNormal = FVector::CrossProduct(middleToHand, middleToIndex);
	FVector handAxis = (SkeletalMesh->GetSocketLocation(handSocket) - SkeletalMesh->GetSocketLocation(shoulderSocket)).GetSafeNormal();
	FVector elbowLocation = SkeletalMesh->GetSocketLocation(elbowSocket);
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

	if (!DebugElbowJointTarget)	return;
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

void UVRCharacterAnimInstance::CalculateFootIKEffector(AActor* currentActor, bool right, float DeltaTime)
{
	UWorld* w = GetWorld();
	if (!SkeletalMesh || !Self || !w || !currentActor) return;
	FName footSocket = right ? RightFootBoneSocketName : LeftFootBoneSocketName;
	FVector wFootSocketPos = SkeletalMesh->GetSocketLocation(footSocket);
	UCapsuleComponent* caps = Self->GetCapsuleComponent();
	if (!caps) return;
	FVector wCaps = caps->GetComponentLocation();
	FVector start = FVector(wFootSocketPos.X, wFootSocketPos.Y, wCaps.Z);
	FVector end = start - FVector(0.f, 0.f, caps->GetScaledCapsuleHalfHeight() + FloorDetectionThreshold);
	FHitResult outHit;
	bool obstacleHit;
	FHitResult obstacleHitResult;
	AActor* obstacle = Self->GetObstacle(obstacleHit, obstacleHitResult);
	FCollisionQueryParams queryParams;
	if (obstacleHit && obstacle)
	{
		queryParams.AddIgnoredActor(obstacle);
	}
	queryParams.AddIgnoredActor(currentActor);
	bool gHit = w->LineTraceSingleByChannel(outHit, start, end, ECollisionChannel::ECC_WorldStatic, queryParams);
	FVector TargetEffector = FVector::ZeroVector;
	if (gHit && outHit.bBlockingHit)
	{
		float rawFloorDelta = outHit.ImpactPoint.Z - wFootSocketPos.Z;
		float crouchDepth = 0.f;
		bool isCrouching = Self->IsCrouching(&crouchDepth);
		float finalDelta = 0.f;
		if (right)
		{
			finalDelta = -(rawFloorDelta + crouchDepth);
			if (isCrouching)
			{
				finalDelta += footHeight;
			}
		}
		else
		{
			finalDelta = rawFloorDelta + crouchDepth;
			if (isCrouching)
			{
				finalDelta -= footHeight;
			}
		}
		
		TargetEffector = FVector(finalDelta, 0.f, 0.f);
	}
	float InterpSpeed = 15.0f;
	if (right)
	{
		RightFootEffectorLocation = FMath::VInterpTo(RightFootEffectorLocation, TargetEffector, DeltaTime, InterpSpeed);
	}
	else
	{
		LeftFootEffectorLocation = FMath::VInterpTo(LeftFootEffectorLocation, TargetEffector, DeltaTime, InterpSpeed);
	}
	if (!DebugFootIKEffectors) return;
	if (gHit)
	{
		DrawDebugSphere(w, outHit.ImpactPoint, 5.f, 8, FColor::Red);
	}

	DrawDebugSphere(w, start, 5.f, 8, FColor::Blue);
	DrawDebugSphere(w, end, 5.f, 8, FColor::Green);
}

void UVRCharacterAnimInstance::CalculateSpineRotationAccordingToTheControllerLocation(bool right, float deltaTime)
{
	if (!SkeletalMesh) return;
	FName shoulderSocket = right ? RightUpperArmSocketName : LeftUpperArmSocketName;
	UMotionControllerComponent* controller = right ? RightController : LeftController;
	if (!controller) return;
	FVector shoulderPos = SkeletalMesh->GetSocketLocation(shoulderSocket);
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
	
	if (!DebugSpineRotation) return;
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

void UVRCharacterAnimInstance::CalculateLegIkEnable(float DeltaTime)
{
	if (!Self) return;
	const bool isGrounded = Self->IsGrounded();
	if (!isGrounded)
	{
		LegIkEnable = FMath::FInterpTo(LegIkEnable, 0.f, DeltaTime, LegIKInterpolationConstant);
	}
	else
	{
		LegIkEnable = FMath::FInterpTo(LegIkEnable, 1.f, DeltaTime, LegIKInterpolationConstant);
	}
}

void UVRCharacterAnimInstance::CalculateLayerBlendForLegs(float DeltaTime)
{
	if (!Self) return;
	if (Self->IsJumping())
	{
		EnableLayerBlendForLegs = FMath::FInterpTo(EnableLayerBlendForLegs, 1.f, DeltaTime, LayerBlendInterpolationConstant);
	}
	else
	{
		EnableLayerBlendForLegs = FMath::FInterpTo(EnableLayerBlendForLegs, 0.f, DeltaTime, LayerBlendInterpolationConstant);
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
		CalculateCrouching(DeltaSeconds);
		CalculateLegIkEnable(DeltaSeconds);
		CalculateLayerBlendForLegs(DeltaSeconds);
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

void UVRCharacterAnimInstance::CalculateMotionControllerTransform(
	UMotionControllerComponent* comp,
	bool right, float DeltaTime)
{
	if (!comp || !SkeletalMesh || !Self) return;

	FName elbowSocket = right ? RightLowerArmSocketName : LeftLowerArmSocketName;
	if (elbowSocket.IsNone()) return;

	const FVector elbowPos = SkeletalMesh->GetSocketLocation(elbowSocket);
	const FVector motionContrPos = comp->GetComponentLocation();

	UHitSensor* hitSensor = (UHitSensor*)Self->GetMotionControllerHitSensor(right);
	FVector targetPushOutOffset = FVector::ZeroVector;
	FHitResult objHit;

	if (hitSensor)
	{
		const float SphereRadius = 10.0f;
		float multipl = right ? RightObstacleSensorDistanceMultipl : LeftObstacleSensorDistanceMultipl;
		FVector elbowToMotionContr = (motionContrPos - elbowPos) * multipl;
		FVector end = elbowPos + elbowToMotionContr;

		bool bHit = FSensorTraceUtility::PerformTraceWithSensorConfig(
			hitSensor,
			elbowPos,
			end,
			FQuat::Identity,
			FCollisionShape::MakeSphere(SphereRadius),
			false,
			objHit,
			DebugHandObstacleSensor
		);

		if (bHit && objHit.bBlockingHit)
		{
			FVector impactToMotionContr = motionContrPos - objHit.ImpactPoint;
			float depthAlongNormal = FVector::DotProduct(impactToMotionContr, objHit.ImpactNormal);
			if (depthAlongNormal < SphereRadius)
			{
				float pushDistance = SphereRadius - depthAlongNormal;
				EVRControllerState currState = right ? RightMotionControllerAnimState : LeftMotionControllerAnimState;
				if (currState == EVRControllerState::VRCS_Neutral)
				{
					pushDistance += middleFingerLength;
				}
				targetPushOutOffset = objHit.ImpactNormal * pushDistance;
			}
		}
	}

	// 1. Interpolate push-out offset
	FVector& currentOffset = right ? RightHandPushOutOffset : LeftHandPushOutOffset;
	currentOffset = FMath::VInterpTo(currentOffset, targetPushOutOffset, DeltaTime, PushOutInterpSpeed);
		
	// 3. Apply base transform + surface rotation together
	FTransform& targetIKTransform = right ? RightMotionControllerIKTransform : LeftMotionControllerIKTransform;
	CalculateDefaultMotionControllerIKTransform(comp, right, targetIKTransform, currentOffset);
}

void UVRCharacterAnimInstance::CalculateDefaultMotionControllerIKTransform(
	UMotionControllerComponent* comp,
	bool right,
	FTransform& mcTransform,
	const FVector& pushOutOffset)
{
	if (!SkeletalMesh || !comp) return;
	//Get World Mesh Transform
	const FTransform MeshTransform = SkeletalMesh->GetComponentTransform();
	//Calculate final world location of the hand
	FVector finalWorldLocation = comp->GetComponentLocation() + pushOutOffset;	
	//Get current world rotation of the motion controller as a Q
	FQuat worldRotation = comp->GetComponentTransform().GetRotation();
	//Get default rotation offset for each Motion Controller
	FQuat offsetRot = right ? RightHandRotOffset.Quaternion() : LeftHandRotOffset.Quaternion();
	//Apply rotation offset
	FQuat baseHandWorldRotation = worldRotation * offsetRot;
	//Build final world transform
	FTransform worldTransform(baseHandWorldRotation, finalWorldLocation, FVector::OneVector);
	//Convert transform to the Skeletal Mesh origin
	mcTransform = worldTransform.GetRelativeTransform(MeshTransform);
}
