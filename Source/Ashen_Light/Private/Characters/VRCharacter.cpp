// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/VRCharacter.h"
#include "MotionControllerComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Components/DecalComponent.h"
#include "Components/WidgetInteractionComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Misc/CoreDelegates.h"
#include "Animations/VRCharacterAnimInstance.h"
#include "Enums.h"
#include "Components/Base/StateManagerComponent.h"
#include "Components/VRCharacter/Movement/PhysicalMovementComponent.h"
#include "Components/VRCharacter/Movement/TrackingSpaceMovementComponent.h"
#include "Components/VRCharacter/Gravity/ClimbingStateComponent.h"
#include "Components/VRCharacter/Gravity/GroundedStateComponent.h"
#include "Components/VRCharacter/Gravity/InAirStateComponent.h"
#include "Components/VRCharacter/Gravity/InMeshStateComponent.h"
#include "Components/VRCharacter/Sensors/CameraFadeSensor.h"
#include "Components/VRCharacter/Sensors/GroundHitSensor.h"
#include "Components/Base/VelocitySensor.h"
#include "IXRTrackingSystem.h"
#include "DrawDebugHelpers.h"
#include "Components/VRCharacter/Sensors/ObstacleSensor.h"
#include "Components/VRCharacter/Sensors/MotionControllerHitSensor.h"
#include "Constants.h"
#include "Subsystems/VRInteractionSubsystem.h"
#include "Components/VRCharacter/Attributes/VRCharacterAttributeComponent.h"

AVRCharacter::AVRCharacter(const FObjectInitializer& init) : Super(init)
{
	//Disable using the Orientation from the Controller. All this we need only in fly simulations.
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;

	//Location in a center of the Tracking Space
	TrackingSpaceOrigin = CreateDefaultSubobject<USceneComponent>(TEXT("Tracking Space Origin"));
	if (!TrackingSpaceOrigin) return;
	SetRootComponent(TrackingSpaceOrigin);
	//VR camera (Camera that will send image to the HMD device)
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("VR Camera Component"));
	if (!CameraComponent) return;
	CameraComponent->SetupAttachment(TrackingSpaceOrigin);
	CameraComponent->bUsePawnControlRotation = false;//Don't send orientation from the camera to the Pawn
	//Camera Fade Component
	CameraFadeComponent = CreateDefaultSubobject<UStaticMeshComponent>("Camera Fade Component");
	if (!CameraFadeComponent) return;
	DisablePhysicsAndCollision(CameraFadeComponent);
	CameraFadeComponent->SetHiddenInGame(true);//Hide it during start
	CameraFadeComponent->SetCastShadow(false);//We don't want to use extra resources for shades calculations
	CameraFadeComponent->SetupAttachment(CameraComponent);
	CameraFadeComponent->bReceivesDecals = false;
	//Capsule Collision Component
	CapsuleCollisionComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Physics Collision Capsule"));
	if (!CapsuleCollisionComponent) return;
	CapsuleCollisionComponent->SetupAttachment(TrackingSpaceOrigin);
	CapsuleCollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CapsuleCollisionComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	CapsuleCollisionComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	CapsuleCollisionComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	CapsuleCollisionComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Block);
	CapsuleCollisionComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
	CapsuleCollisionComponent->SetSimulatePhysics(false);
	CapsuleCollisionComponent->SetHiddenInGame(false);
	CapsuleCollisionComponent->SetGenerateOverlapEvents(false);
	CapsuleCollisionComponent->SetNotifyRigidBodyCollision(true);
	CapsuleCollisionComponent->SetCollisionObjectType(ECollisionChannel::ECC_Pawn);

	//Skeletal mesh component
	SkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Skeletal Mesh Component"));
	if (!SkeletalMeshComponent) return;
	SkeletalMeshComponent->SetupAttachment(CapsuleCollisionComponent);
	SkeletalMeshComponent->SetReceivesDecals(false);
	DisablePhysicsAndCollision(SkeletalMeshComponent);
	SkeletalMeshComponent->bReceivesDecals = false;

	//Right motion Controller
	RightMotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("Right Motion Controller"));
	if (!RightMotionController) return;
	RightMotionController->SetupAttachment(TrackingSpaceOrigin);
	RightMotionController->MotionSource = FName("Right");
	RightMotionController->bDisableLowLatencyUpdate = false;
	RightMotionController->bDisplayDeviceModel = true;//Allow to render the defaul mesh for motion controllers
	RightMotionController->DisplayModelSource = FName("OculusHMD");//Set default for Oculus Quest
	DisablePhysicsAndCollision(RightMotionController);
	//Rigth Widget Interaction Component
	WidgetInteractionRight = CreateDefaultSubobject<UWidgetInteractionComponent>(TEXT("Widget Interaction Right"));
	if (!WidgetInteractionRight) return;
	WidgetInteractionRight->SetupAttachment(RightMotionController);
	//Left Motion Controller
	LeftMotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("Left Motion Controller"));
	if (!LeftMotionController) return;
	LeftMotionController->SetupAttachment(TrackingSpaceOrigin);
	LeftMotionController->MotionSource = FName("Left");
	LeftMotionController->bDisableLowLatencyUpdate = false;
	LeftMotionController->bDisplayDeviceModel = true;
	LeftMotionController->DisplayModelSource = FName("OculusHMD");
	DisablePhysicsAndCollision(LeftMotionController);
	//Left Widget Interaction Component
	WidgetInteractionLeft = CreateDefaultSubobject<UWidgetInteractionComponent>(TEXT("Widget Interaction left"));
	if (!WidgetInteractionLeft) return;
	WidgetInteractionLeft->SetupAttachment(LeftMotionController);
	//Locomotion Decals
	//Dead Zone Decal
	DeadZoneDecalComponent = CreateDefaultSubobject<UDecalComponent>(TEXT("Dead Zone Decal Component"));
	if (!DeadZoneDecalComponent) return;	
	DeadZoneDecalComponent->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));//Rotate it, so X axis will be pointed down
	DeadZoneDecalComponent->SetupAttachment(TrackingSpaceOrigin);

	//Active Zone Decal
	ActiveZoneComponent = CreateDefaultSubobject<UDecalComponent>(TEXT("Active Zone Decal Component"));
	if (!ActiveZoneComponent) return;	
	ActiveZoneComponent->SetupAttachment(TrackingSpaceOrigin);
	ActiveZoneComponent->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));//Rotate it, so X axis will be pointed down
	//Player Anchor Component
	PlayerAnchorDecalComponent = CreateDefaultSubobject<UDecalComponent>(TEXT("Player Anchor Decal"));
	if (!PlayerAnchorDecalComponent) return;
	PlayerAnchorDecalComponent->SetupAttachment(CameraComponent);	
	PlayerAnchorDecalComponent->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));//Rotate it, so X axis will be pointed down
	PlayerAnchorDecalComponent->SetUsingAbsoluteRotation(true);
	//Movement Component
	PawnMovement = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("Pawn Floating Movement"));
	m_ActorsToIgnore.Add(this);
	//Sensors
	CameraFadeSensor = CreateDefaultSubobject<UCameraFadeSensor>(TEXT("Camera Fade Sensor"));
	if (CameraFadeSensor)
	{
		CameraFadeSensor->SetIgnoredActors(&m_ActorsToIgnore);
		CameraFadeSensor->SetTrackingComponent(CameraComponent);
	}
	GroundHitSensor = CreateDefaultSubobject<UGroundHitSensor>(TEXT("Ground Hit Sensor"));
	if (GroundHitSensor)
	{
		GroundHitSensor->SetIgnoredActors(&m_ActorsToIgnore);
		GroundHitSensor->SetTrackingComponent(CameraComponent);
	}
	CameraVelocitySensor = CreateDefaultSubobject<UVelocitySensor>(TEXT("Camera Velocity Sensor"));
	if (CameraVelocitySensor)
	{
		CameraVelocitySensor->ConfigureSpace(RTS_World);
		CameraVelocitySensor->SetTrackingComponent(CameraComponent);
	}
	RightMotionControllerVelocitySensor = CreateDefaultSubobject<UVelocitySensor>(TEXT("Right Motion Controller Velocity Sensor"));
	if (RightMotionControllerVelocitySensor)
	{
		RightMotionControllerVelocitySensor->ConfigureSpace(RTS_Component);
		RightMotionControllerVelocitySensor->SetTrackingComponent(RightMotionController);
	}
	LeftMotionControllerVelocitySensor = CreateDefaultSubobject<UVelocitySensor>(TEXT("Left Motion Controller velocity Sensor"));
	if (LeftMotionControllerVelocitySensor)
	{
		LeftMotionControllerVelocitySensor->ConfigureSpace(RTS_Component);
		LeftMotionControllerVelocitySensor->SetTrackingComponent(LeftMotionController);
	}
	ObstacleSensor = CreateDefaultSubobject<UObstacleSensor>(TEXT("Obstacle Sensor"));
	if (ObstacleSensor)
	{
		ObstacleSensor->SetIgnoredActors(&m_ActorsToIgnore);
		ObstacleSensor->SetTrackingComponent(CapsuleCollisionComponent);
	}
	LeftMotionControllerHitSensor = CreateDefaultSubobject<UMotionControllerHitSensor>(TEXT("Left Motion Controller Hit Sensor"));
	if (LeftMotionControllerHitSensor)
	{
		LeftMotionControllerHitSensor->SetIgnoredActors(&m_ActorsToIgnore);
		LeftMotionControllerHitSensor->SetTrackingComponent(LeftMotionController);
	}
	RightMotionControllerHitSensor = CreateDefaultSubobject<UMotionControllerHitSensor>(TEXT("Right Motion Controller Hit Sensor"));
	if (RightMotionControllerHitSensor)
	{
		RightMotionControllerHitSensor->SetIgnoredActors(&m_ActorsToIgnore);
		RightMotionControllerHitSensor->SetTrackingComponent(RightMotionController);
	}

	GlobalStateBlackboard = NewObject<UStateBlackboard>();
	//Configure State Managers
	//Locomotion
	LocomotionStateManager = CreateDefaultSubobject<UStateManagerComponent>(TEXT("Locomotion State Manager Component"));
	if (LocomotionStateManager)
	{
		PhysicalMovementComponent = CreateDefaultSubobject<UPhysicalMovementComponent>(TEXT("Physical Movement"));
		if (PhysicalMovementComponent)
		{
			PhysicalMovementComponent->InitializeState(ELocomotionSpace::ELS_Physical, this, LocomotionStateManager);
		}
		TrackingSpaceMovementComponent = CreateDefaultSubobject<UTrackingSpaceMovementComponent>(TEXT("Tracking Space Movement"));
		if (TrackingSpaceMovementComponent)
		{
			TrackingSpaceMovementComponent->InitializeState(ELocomotionSpace::ELS_TrackingSpace, this, LocomotionStateManager);
		}
		LocomotionStateManager->RegisterState(PhysicalMovementComponent);
		LocomotionStateManager->RegisterState(TrackingSpaceMovementComponent);
		LocomotionStateManager->BuildStateMatrix(2);
		LocomotionStateManager->ConfigureStateMatrix([](FRectMatrix<bool>* m)
			{
				m->Set(true, ELocomotionSpace::ELS_Physical, ELocomotionSpace::ELS_TrackingSpace);
				m->Set(true, ELocomotionSpace::ELS_TrackingSpace, ELocomotionSpace::ELS_Physical);
			});
		LocomotionStateManager->AddGlobalBlackBoard(GlobalStateBlackboard);
		LocomotionStateManager->SwitchState(ELocomotionSpace::ELS_Physical);
	}	

	//Gravity
	GravityStateManager = CreateDefaultSubobject<UStateManagerComponent>(TEXT("Gravity State Manager Component"));
	if (GravityStateManager)
	{
		GroundedStateComponent = CreateDefaultSubobject<UGroundedStateComponent>(TEXT("Grounded State Component"));
		if (GroundedStateComponent)
		{
			GroundedStateComponent->InitializeState(EGravityState::EGS_Grounded, this, GravityStateManager);
		}
		InAirStateComponent = CreateDefaultSubobject<UInAirStateComponent>(TEXT("In Air State Component"));
		if (InAirStateComponent)
		{
			InAirStateComponent->InitializeState(EGravityState::EGS_InAir, this, GravityStateManager);
		}
		InMeshStateComponent = CreateDefaultSubobject<UInMeshStateComponent>(TEXT("In Mesh State Component"));
		if (InMeshStateComponent)
		{
			InMeshStateComponent->InitializeState(EGravityState::EGS_InMesh, this, GravityStateManager);
		}
		ClimbingStateComponent = CreateDefaultSubobject<UClimbingStateComponent>(TEXT("Climbing State Component"));
		if (ClimbingStateComponent)
		{
			ClimbingStateComponent->InitializeState(EGravityState::EGS_Climbing, this, GravityStateManager);
		}
		GravityStateManager->RegisterState(GroundedStateComponent);
		GravityStateManager->RegisterState(InAirStateComponent);
		GravityStateManager->RegisterState(InMeshStateComponent);
		GravityStateManager->RegisterState(ClimbingStateComponent);
		GravityStateManager->BuildStateMatrix(4);
		GravityStateManager->ConfigureStateMatrix([](FRectMatrix<bool>* m) 
			{
				m->Set(true, EGravityState::EGS_InAir, EGravityState::EGS_Grounded);
				m->Set(true, EGravityState::EGS_InAir, EGravityState::EGS_Climbing);
				m->Set(true, EGravityState::EGS_InAir, EGravityState::EGS_InMesh);

				m->Set(true, EGravityState::EGS_Grounded, EGravityState::EGS_InAir);
				m->Set(true, EGravityState::EGS_Grounded, EGravityState::EGS_Climbing);
				m->Set(true, EGravityState::EGS_Grounded, EGravityState::EGS_InMesh);

				m->Set(true, EGravityState::EGS_Climbing, EGravityState::EGS_InAir);
				m->Set(true, EGravityState::EGS_Climbing, EGravityState::EGS_Grounded);
				m->Set(true, EGravityState::EGS_Climbing, EGravityState::EGS_InMesh);

				m->Set(true, EGravityState::EGS_InMesh, EGravityState::EGS_InAir);
				m->Set(true, EGravityState::EGS_InMesh, EGravityState::EGS_Grounded);
			});
		GravityStateManager->AddGlobalBlackBoard(GlobalStateBlackboard);
		GravityStateManager->SwitchState(EGravityState::EGS_InAir);
	}
	
	//VR Controllers Hit Targets
	LeftMotionControllerHitTarget = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Left Motion Controller Hit Target"));
	if (LeftMotionControllerHitTarget)
	{
		LeftMotionControllerHitTarget->SetupAttachment(GetRootComponent());
		DisablePhysicsAndCollision(LeftMotionControllerHitTarget);
		LeftMotionControllerHitTarget->bReceivesDecals = false;
	}
	RightMotionControllerHitTarget = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Right Motion Controller Hit Target"));
	if (RightMotionControllerHitTarget)
	{
		RightMotionControllerHitTarget->SetupAttachment(GetRootComponent());
		DisablePhysicsAndCollision(RightMotionControllerHitTarget);
		RightMotionControllerHitTarget->bReceivesDecals = false;
	}

	//Attributes
	CharacterAttributeComponent = CreateDefaultSubobject <UVRCharacterAttributeComponent>(TEXT("Character Attributes"));

	initialPlayerMetricsCalculated = false;
}

void AVRCharacter::SetNewActorTransform(const FTransform& transform, bool sweep, FHitResult* outHit, ETeleportType teleType)
{
	SetActorTransform(transform, sweep, outHit, teleType);
}

bool AVRCharacter::IsJumping() const
{
	if (!GlobalStateBlackboard) return false;
	bool pendingJump = false;
	if (!GlobalStateBlackboard->TryGetValue(Constants::JumpState::PendingJumping, pendingJump))
	{
		return false;
	}
	return pendingJump;
}

void AVRCharacter::BeginPlay()
{
	Super::BeginPlay();

	//We need this to ensure that Tick Component of the UFloatingPawnMovement will be executed first then -> VRCharacter's Tick function 
	if (GetSkeletalMesh() && PawnMovement)
	{
		GetSkeletalMesh()->PrimaryComponentTick.AddPrerequisite(PawnMovement, PawnMovement->PrimaryComponentTick);
	}

	bool isInVR = GEngine && GEngine->XRSystem.IsValid() && GEngine->XRSystem->IsHeadTrackingAllowed();
	//We are in VR Preview Mode
	if (isInVR)
	{
		//Set Tracking origin to Floor.When the game starts the camera will be placed in the center of the Tracking Space idealy. 
		//Then it will be rised up according to the distance to the floor in the real room, 
		// we also need to wait some period of time for proper initialization
		UHeadMountedDisplayFunctionLibrary::SetTrackingOrigin(EHMDTrackingOrigin::Floor);
		//Bind to the Recenter event
		FCoreDelegates::VRHeadsetRecenter.AddUObject(this, &AVRCharacter::OnHMD_Recentered);
		if (CameraComponent)
		{
			InitialPlayerHeight = CameraComponent->GetRelativeLocation().Z;
			CameraComponent->bLockToHmd = true;//Bind Transform of the HMD to the Camera in Tracking Space
		}
	}
	else//Other PIE mode (Simulation)
	{
		if (CameraComponent)
		{
			CameraComponent->bLockToHmd = false;
			CameraComponent->SetRelativeLocation(FVector(0.f, 0.f, PlayerPreviewHeight));
			InitialPlayerHeight = PlayerPreviewHeight;
		}
	}
			
	//Configure Decals
	ConfigureDecalSize(DeadZoneDecalComponent, DeadZoneHeight, DeadZoneRadius, true);
	ConfigureDecalSize(ActiveZoneComponent, ActiveZoneHeight, ActiveZoneRadius, true);
	ConfigureDecalSize(PlayerAnchorDecalComponent, PlayerAnchorZoneHeight, PlayerAnchorZoneRadius, true);	
	
	if (LeftMotionControllerVelocitySensor)
	{
		LeftMotionControllerVelocitySensor->SyncPosition();
	}
	if (RightMotionControllerVelocitySensor)
	{
		RightMotionControllerVelocitySensor->SyncPosition();
	}
	if (CameraVelocitySensor)
	{
		CameraVelocitySensor->SyncPosition();
	}
	if (LocomotionStateManager)
	{
		LocomotionStateManager->SetEnumToStrConverter([](uint8 e) ->FString
			{
				ELocomotionSpace s = static_cast<ELocomotionSpace>(e);
				switch (s)
				{
				case ELocomotionSpace::ELS_Physical:
					return FString("Physical Space");
				case ELocomotionSpace::ELS_TrackingSpace:
					return FString("Tracking Space");
				}
				return FString("");
			});
		LocomotionStateManager->BeginPlay();
	}
	if (GravityStateManager)
	{
		GravityStateManager->SetEnumToStrConverter([](uint8 e) -> FString
			{
				EGravityState gs = static_cast<EGravityState>(e);
				switch (gs)
				{
				case EGravityState::EGS_InAir:
					return FString("In Air");
				case EGravityState::EGS_Grounded:
					return FString("Grounded");
				case EGravityState::EGS_Climbing:
					return FString("Climbing");
				case EGravityState::EGS_InMesh:
					return FString("In Mesh");
				}
				return FString("");
			});
		GravityStateManager->BeginPlay();
	}

	FadeDynamicMaterial = UMaterialInstanceDynamic::Create(FadeMaterialBase, this);
	if (FadeDynamicMaterial && CameraFadeComponent)
	{
		CameraFadeComponent->SetMaterial(0, FadeDynamicMaterial);
	}

	UVRInteractionSubsystem* vrInter = GetVRSubSystem();
	if (vrInter)
	{
		vrInter->RegisterPawn(this);
		vrInter->SetGrabSocketName(RightHandGrabSocketName, true);
		vrInter->SetGrabSocketName(LeftHandGrabSocketName, false);
	}
}

void AVRCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	//Clear all the bindings when we exit the game
	FCoreDelegates::VRHeadsetRecenter.RemoveAll(this);
	UVRInteractionSubsystem* vrInter = GetVRSubSystem();
	if (vrInter)
	{
		vrInter->UnRegisterPawn(this);
	}
	Super::EndPlay(EndPlayReason);
}

void AVRCharacter::OnHMD_Recentered()
{
	//When we recalibrate we need to get the new camera position relative to the Tracking Space
	if (CameraVelocitySensor)
	{
		CameraVelocitySensor->SyncPosition();
	}
	if (LeftMotionControllerVelocitySensor)
	{
		LeftMotionControllerVelocitySensor->SyncPosition();
	}
	if (RightMotionControllerVelocitySensor)
	{
		RightMotionControllerVelocitySensor->SyncPosition();
	}
	//Start Recalibration of the Capsule and Mesh
	GetWorldTimerManager().SetTimerForNextTick(this, &AVRCharacter::OnHMDRecenterFinished);	
}

void AVRCharacter::RecenterTrackingSpaceToLocation(FVector TargetWorldLocation)
{
	if (!TrackingSpaceOrigin || !CameraComponent) return;
	//Get the Camera World Location
	FVector CurrentCameraWorldLoc = CameraComponent->GetComponentLocation();
	//Calculate the vector where we should move Tracking Space in XY Plane
	FVector Offset2D = TargetWorldLocation - CurrentCameraWorldLoc;
	Offset2D.Z = 0.f;
	//Perform movement
	FVector NewOriginWorldLoc = TrackingSpaceOrigin->GetComponentLocation() + Offset2D;
	TrackingSpaceOrigin->SetWorldLocation(NewOriginWorldLoc);
	//Recalculate Camera previous Location
	if (CameraVelocitySensor)
	{
		CameraVelocitySensor->SyncPosition();
	}
	if (LeftMotionControllerVelocitySensor)
	{
		LeftMotionControllerVelocitySensor->SyncPosition();
	}
	if (RightMotionControllerVelocitySensor)
	{
		RightMotionControllerVelocitySensor->SyncPosition();
	}
	RecalibrateCapsuleAndMeshComponent();
}

void AVRCharacter::RecenterTrackingSpaceToActor()
{
	RecenterTrackingSpaceToLocation(GetActorLocation());
}

UVRCharacterAnimInstance* AVRCharacter::GetCharAnimInstance()
{
	//If anim instance wasn't initialized
	if (!VRCharacterAnimInstance)
	{
		//Get skeletal mesh component
		USkeletalMeshComponent* skelMesh = GetSkeletalMesh();
		if (!skelMesh) return VRCharacterAnimInstance;
		//Get Anim Instance
		UAnimInstance* inst = skelMesh->GetAnimInstance();
		if (!inst) return VRCharacterAnimInstance;
		//Convert Anim instance
		VRCharacterAnimInstance = Cast<UVRCharacterAnimInstance>(inst);
	}
	
	return VRCharacterAnimInstance;
}

float AVRCharacter::GetGroundVelocityRatio() const
{
	if (!PawnMovement || FMath::IsNearlyZero(CharacterAttributeComponent->RunSpeed)) return 0.f;
	//We calculate normalize Velocity clamped to the [0 ; 1] in XY plane
	return FMath::Clamp(PawnMovement->Velocity.Size2D() / CharacterAttributeComponent->RunSpeed, 0.f, 1.f);
}

void AVRCharacter::InitializeBodyMetrics()
{
	if (!initialPlayerMetricsCalculated && CameraComponent)
	{
		float CurrentCamZ = CameraComponent->GetRelativeLocation().Z;
		if (CurrentCamZ > 80.0f)
		{
			InitialPlayerHeight = CurrentCamZ;
			initialPlayerMetricsCalculated = true;
			UVRCharacterAnimInstance* inst = GetCharAnimInstance();
			if (inst)
			{
				inst->CalculateUniversalScaleFactor(CurrentCamZ);
				inst->CalculateFootHeight();
			}
		}
	}
}

UVRInteractionSubsystem* AVRCharacter::GetVRSubSystem()
{
	if (!VRInteractionSubSystem)
	{
		UWorld* w = GetWorld();
		if (!w) return VRInteractionSubSystem;
		VRInteractionSubSystem = w->GetSubsystem<UVRInteractionSubsystem>();
	}

	return VRInteractionSubSystem;
}

void AVRCharacter::UpdateVRControllerButtonState(EVRControllerHand hand, EVRButtonType butType, bool bIsPressed)
{
	UVRInteractionSubsystem* vrInter = GetVRSubSystem();
	if (!vrInter) return;
	vrInter->UpdatePlayerButtonState(this, hand, butType, bIsPressed);
}

void AVRCharacter::UpdadeVRControllerAxisState(EVRControllerHand hand, EVRButtonType butType, float axisValue)
{
	UVRInteractionSubsystem* vrInter = GetVRSubSystem();
	if (!vrInter) return;
	vrInter->UpdatePlayerAxisState(this, hand, butType, axisValue);
}

void AVRCharacter::UpdateVRControllerThumbstickAxis(EVRControllerHand hand, float axisValue, bool xAxis)
{
	UVRInteractionSubsystem* vrInter = GetVRSubSystem();
	if (!vrInter) return;
	vrInter->UpdatePlayerAxisThumbstickState(this, hand, axisValue, xAxis);
}

void AVRCharacter::UpdateVRControllerTouchState(EVRControllerHand hand, EVRButtonType butType, bool bIsTouched)
{
	UVRInteractionSubsystem* vrInter = GetVRSubSystem();
	if (!vrInter) return;
	vrInter->UpdatePlayerTouchState(this, hand, butType, bIsTouched);
}

void AVRCharacter::UpdateVRAnimInstanceGripState(bool right, bool pressed)
{
	UVRCharacterAnimInstance* inst = GetCharAnimInstance();
	if (!inst) return;
	inst->SetGripState(right, pressed);
}

void AVRCharacter::UpdateVRAnimInstanceTriggerState(bool right, bool pressed)
{
	UVRCharacterAnimInstance* inst = GetCharAnimInstance();
	if (!inst) return;
	inst->SetTriggerState(right, pressed);
}

void AVRCharacter::DisablePhysicsAndCollision(UPrimitiveComponent* comp)
{
	if (!comp) return;
	comp->SetSimulatePhysics(false);
	comp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	comp->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
}

void AVRCharacter::ProcessHitTarget(UStaticMeshComponent* hitTarget, UMotionControllerHitSensor* hitSensor)
{
	if (!hitSensor || !hitTarget) return;
	FHitResult hit = hitSensor->GetHit();
	if (!hitSensor->IsHit() && !hit.bBlockingHit)
	{
		hitTarget->SetVisibility(false);
	}
	else
	{
		hitTarget->SetVisibility(true);
		FVector SurfaceLocation = hit.ImpactPoint + (hit.ImpactNormal * 0.1f);
		FQuat TargetRotation = FRotationMatrix::MakeFromZ(hit.ImpactNormal).ToQuat();
		hitTarget->SetWorldLocationAndRotation(SurfaceLocation, TargetRotation);
	}
	
}

void AVRCharacter::OnHMDRecenterFinished()
{
	if (!CameraComponent) return;
	InitialPlayerHeight = CameraComponent->GetRelativeLocation().Z;
	RecalibrateCapsuleAndMeshComponent();
}

void AVRCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (!PlayerInputComponent) return;
	//Setup binding
	
	//Grips
	PlayerInputComponent->BindAction(FName("GrabRight"), IE_Pressed, this, &AVRCharacter::OnRightGrabButtonPressed);
	PlayerInputComponent->BindAction(FName("GrabRight"), IE_Released, this, &AVRCharacter::OnRightGrabButtonReleased);
	PlayerInputComponent->BindAction(FName("GrabLeft"), IE_Pressed, this, &AVRCharacter::OnLeftGrabButtonPressed);
	PlayerInputComponent->BindAction(FName("GrabLeft"), IE_Released, this, &AVRCharacter::OnLeftGrabButtonReleased);
	//Triggers
	PlayerInputComponent->BindAction(FName("TriggerRight"), IE_Pressed, this, &AVRCharacter::OnRightTriggerButtonPressed);
	PlayerInputComponent->BindAction(FName("TriggerRight"), IE_Released, this, &AVRCharacter::OnRightTriggerButtonReleased);
	PlayerInputComponent->BindAction(FName("TriggerLeft"), IE_Pressed, this, &AVRCharacter::OnLeftTriggerButtonPressed);
	PlayerInputComponent->BindAction(FName("TriggerLeft"), IE_Released, this, &AVRCharacter::OnLeftTriggerButtonReleased);
	//Menu Buttons
	PlayerInputComponent->BindAction(FName("MenuToggleLeft"), IE_Pressed, this, &AVRCharacter::OnLeftMenuButtonPressed);
	PlayerInputComponent->BindAction(FName("MenuToggleLeft"), IE_Released, this, &AVRCharacter::OnLeftMenuButtonReleased);
	PlayerInputComponent->BindAction(FName("MenuToggleRight"), IE_Pressed, this, &AVRCharacter::OnRightMenuButtonPressed);
	PlayerInputComponent->BindAction(FName("MenuToggleRight"), IE_Released, this, &AVRCharacter::OnRightMenuButtonReleased);
	//Reset VR
	PlayerInputComponent->BindAction(FName("ResetVR"), IE_Pressed, this, &AVRCharacter::OnResetVRPressed);
	PlayerInputComponent->BindAction(FName("ResetVR"), IE_Released, this, &AVRCharacter::OnResetVRReleased);
	//Primary Buttons
	PlayerInputComponent->BindAction(FName("LeftPrimaryPressed"), IE_Pressed, this, &AVRCharacter::OnLeftPrimaryButtonPressed);
	PlayerInputComponent->BindAction(FName("LeftPrimaryPressed"), IE_Released, this, &AVRCharacter::OnLeftPrimaryButtonReleased);
	PlayerInputComponent->BindAction(FName("RightPrimaryPressed"), IE_Pressed, this, &AVRCharacter::OnRightPrimaryButtonPressed);
	PlayerInputComponent->BindAction(FName("RightPrimaryPressed"), IE_Released, this, &AVRCharacter::OnRightPrimaryButtonReleased);
	//Secondary Buttons
	PlayerInputComponent->BindAction(FName("LeftSecondaryPressed"), IE_Pressed, this, &AVRCharacter::OnLeftSecondaryButtonPressed);
	PlayerInputComponent->BindAction(FName("LeftSecondaryPressed"), IE_Released, this, &AVRCharacter::OnLeftSecondaryButtonReleased);
	PlayerInputComponent->BindAction(FName("RightSecondaryPressed"), IE_Pressed, this, &AVRCharacter::OnRightSecondaryButtonPressed);
	PlayerInputComponent->BindAction(FName("RightSecondaryPressed"), IE_Released, this, &AVRCharacter::OnRightSecondaryButtonReleased);
	//Grab Axis
	PlayerInputComponent->BindAxis(FName("GrabAxisLeft"), this, &AVRCharacter::OnLeftGrabAxisChanged);
	PlayerInputComponent->BindAxis(FName("GrabAxisRight"), this, &AVRCharacter::OnRightGrabAxisChanged);
	//Movement Axis
	PlayerInputComponent->BindAxis(FName("MovementAxisLeft_X"), this, &AVRCharacter::OnLeftThumbstickXChanged);
	PlayerInputComponent->BindAxis(FName("MovementAxisLeft_Y"), this, &AVRCharacter::OnLeftThumbstickYChanged);
	PlayerInputComponent->BindAxis(FName("MovementAxisRight_X"), this, &AVRCharacter::OnRightThumbstickXChanged);
	PlayerInputComponent->BindAxis(FName("MovementAxisRight_Y"), this, &AVRCharacter::OnRightThumbstickYChanged);
	//Trigger Axis
	PlayerInputComponent->BindAxis(FName("TriggerAxisLeft"), this, &AVRCharacter::OnLeftTriggerAxisChanged);
	PlayerInputComponent->BindAxis(FName("TriggerAxisRight"), this, &AVRCharacter::OnRightTriggerAxisChanged);
}

void AVRCharacter::OnRightGrabButtonPressed()
{
	UpdateVRControllerButtonState(EVRControllerHand::Right, EVRButtonType::Grip, true);
	UpdateVRAnimInstanceGripState(true, true);
}

void AVRCharacter::OnRightGrabButtonReleased()
{
	UpdateVRControllerButtonState(EVRControllerHand::Right, EVRButtonType::Grip, false);
	UpdateVRAnimInstanceGripState(true, false);	
}

void AVRCharacter::OnLeftGrabButtonPressed()
{
	UpdateVRControllerButtonState(EVRControllerHand::Left, EVRButtonType::Grip, true);
	UpdateVRAnimInstanceGripState(false, true);
}

void AVRCharacter::OnLeftGrabButtonReleased()
{
	UpdateVRControllerButtonState(EVRControllerHand::Left, EVRButtonType::Grip, false);
	UpdateVRAnimInstanceGripState(false, false);
}

void AVRCharacter::OnLeftMenuButtonPressed()
{
	UpdateVRControllerButtonState(EVRControllerHand::Left, EVRButtonType::MenuOrSystem, true);
}

void AVRCharacter::OnLeftMenuButtonReleased()
{
	UpdateVRControllerButtonState(EVRControllerHand::Left, EVRButtonType::MenuOrSystem, false);
}

void AVRCharacter::OnRightMenuButtonPressed()
{
	UpdateVRControllerButtonState(EVRControllerHand::Right, EVRButtonType::MenuOrSystem, true);
}

void AVRCharacter::OnRightMenuButtonReleased()
{
	UpdateVRControllerButtonState(EVRControllerHand::Right, EVRButtonType::MenuOrSystem, false);
}

void AVRCharacter::OnRightTriggerButtonPressed()
{
	UpdateVRControllerButtonState(EVRControllerHand::Right, EVRButtonType::Trigger, true);
	UpdateVRAnimInstanceTriggerState(true, true);
}

void AVRCharacter::OnRightTriggerButtonReleased()
{
	UpdateVRControllerButtonState(EVRControllerHand::Right, EVRButtonType::Trigger, false);
	UpdateVRAnimInstanceTriggerState(true, false);
}

void AVRCharacter::OnLeftTriggerButtonPressed()
{
	UpdateVRControllerButtonState(EVRControllerHand::Left, EVRButtonType::Trigger, true);
	UpdateVRAnimInstanceTriggerState(false, true);
}

void AVRCharacter::OnLeftTriggerButtonReleased()
{
	UpdateVRControllerButtonState(EVRControllerHand::Left, EVRButtonType::Trigger, false);
	UpdateVRAnimInstanceTriggerState(false, false);
}

void AVRCharacter::OnLeftPrimaryButtonPressed()
{
	UpdateVRControllerButtonState(EVRControllerHand::Left, EVRButtonType::Primary, true);
}

void AVRCharacter::OnLeftPrimaryButtonReleased()
{
	UpdateVRControllerButtonState(EVRControllerHand::Left, EVRButtonType::Primary, false);
}

void AVRCharacter::OnRightPrimaryButtonPressed()
{
	UpdateVRControllerButtonState(EVRControllerHand::Right, EVRButtonType::Primary, true);
	//To do Should be moved from future plugin
	if (LocomotionStateManager && LocomotionStateManager->GetCurrentState<ELocomotionSpace>() != ELocomotionSpace::ELS_TrackingSpace)
	{
		LocomotionStateManager->SwitchState(ELocomotionSpace::ELS_TrackingSpace);
	}
}

void AVRCharacter::OnRightPrimaryButtonReleased()
{
	UpdateVRControllerButtonState(EVRControllerHand::Right, EVRButtonType::Primary, false);
	//To do Should be moved from future plugin
	if (LocomotionStateManager && LocomotionStateManager->GetCurrentState<ELocomotionSpace>() != ELocomotionSpace::ELS_Physical)
	{
		LocomotionStateManager->SwitchState(ELocomotionSpace::ELS_Physical);
	}
}

void AVRCharacter::OnLeftSecondaryButtonPressed()
{
	UpdateVRControllerButtonState(EVRControllerHand::Left, EVRButtonType::Secondary, true);
}

void AVRCharacter::OnLeftSecondaryButtonReleased()
{
	UpdateVRControllerButtonState(EVRControllerHand::Left, EVRButtonType::Secondary, false);
}

void AVRCharacter::OnRightSecondaryButtonPressed()
{
	UpdateVRControllerButtonState(EVRControllerHand::Right, EVRButtonType::Secondary, true);
}

void AVRCharacter::OnRightSecondaryButtonReleased()
{
	UpdateVRControllerButtonState(EVRControllerHand::Right, EVRButtonType::Secondary, false);
}

void AVRCharacter::OnLeftThumbstickPressed()
{
	UpdateVRControllerButtonState(EVRControllerHand::Left, EVRButtonType::ThumbstickClick, true);
}

void AVRCharacter::OnLeftThumbstickReleased()
{
	UpdateVRControllerButtonState(EVRControllerHand::Left, EVRButtonType::ThumbstickClick, false);
}

void AVRCharacter::OnRightThumbstickPressed()
{
	UpdateVRControllerButtonState(EVRControllerHand::Right, EVRButtonType::ThumbstickClick, true);
}

void AVRCharacter::OnRightThumbstickReleased()
{
	UpdateVRControllerButtonState(EVRControllerHand::Right, EVRButtonType::ThumbstickClick, false);
}

void AVRCharacter::OnLeftGrabAxisChanged(float value)
{
	UpdadeVRControllerAxisState(EVRControllerHand::Left, EVRButtonType::Grip, value);
}

void AVRCharacter::OnRightGrabAxisChanged(float value)
{
	UpdadeVRControllerAxisState(EVRControllerHand::Right, EVRButtonType::Grip, value);
}

void AVRCharacter::OnLeftThumbstickXChanged(float value)
{
	UpdateVRControllerThumbstickAxis(EVRControllerHand::Left, value, true);
}

void AVRCharacter::OnLeftThumbstickYChanged(float value)
{
	UpdateVRControllerThumbstickAxis(EVRControllerHand::Left, value, false);
}

void AVRCharacter::OnRightThumbstickXChanged(float value)
{
	UpdateVRControllerThumbstickAxis(EVRControllerHand::Right, value, true);
}

void AVRCharacter::OnRightThumbstickYChanged(float value)
{
	UpdateVRControllerThumbstickAxis(EVRControllerHand::Right, value, false);
}

void AVRCharacter::OnLeftTriggerAxisChanged(float value)
{
	UpdadeVRControllerAxisState(EVRControllerHand::Left, EVRButtonType::Trigger, value);
}

void AVRCharacter::OnRightTriggerAxisChanged(float value)
{
	UpdadeVRControllerAxisState(EVRControllerHand::Right, EVRButtonType::Trigger, value);
}

void AVRCharacter::RecalibrateCapsuleAndMeshComponent()
{
	if (!CapsuleCollisionComponent || !CameraComponent) return;
	//Get the heigth of the person in HMD
	float playerHeight = FMath::Max(CameraComponent->GetRelativeLocation().Z, 40.f);
	//Calculate half height
	float halfHeight = playerHeight * 0.5f;
	//Get Current unscaled radius of the Collision Capsule
	float currRadius = CapsuleCollisionComponent->GetUnscaledCapsuleRadius();
	//Set new half heigth according to the height of the Palyer
	CapsuleCollisionComponent->SetCapsuleSize(currRadius, halfHeight);
	
	if (SkeletalMeshComponent)
	{
		//Get the location of the skeletal mesh in a Capsule Space
		FVector MeshLoc = SkeletalMeshComponent->GetRelativeLocation();
		//the root of the Skeletal mesh is located at the feet of the model we need to move it down to halfheight of the capsule
		//And the origin of the Skeletal Mesh is located not at the bottom of the foot it is raised up for some value
		MeshLoc.Z = -halfHeight + (cameraHeadDelta / 2.f);//Cause we resize mesh from the center
		SkeletalMeshComponent->SetRelativeLocation(MeshLoc);
	}
}

void AVRCharacter::ConfigureDecalSize(UDecalComponent* decal, float thicknes, float r, bool redraw)
{
	if (!decal) return;
	//Update decal dimensions
	decal->DecalSize = FVector(thicknes, r, r);
	//Should we use force redraw of the decal
	if(redraw)
		decal->MarkRenderStateDirty();
}

AActor* AVRCharacter::GetObstacle(bool& outHit, FHitResult& outHitResult) const
{
	if (!ObstacleSensor)
	{
		outHit = false;
		return nullptr;
	}
	
	bool hit = ObstacleSensor->IsHit();
	if (hit)
	{
		outHit = hit;
		outHitResult = ObstacleSensor->GetHit();
		return ObstacleSensor->GetHitActor();
	}

	outHit = false;
	return nullptr;
}

bool AVRCharacter::GetGroundHit(FHitResult& hit) const
{
	if (!GroundHitSensor) return false;
	hit = GroundHitSensor->GetHit();
	return GroundHitSensor->IsHit();
}

float AVRCharacter::GetCurrentThrowVelocity() const
{
	return CharacterAttributeComponent->ThrowSpeed;
}

void AVRCharacter::Move(const FVector& dir, float value, bool instant)
{
	if (!PawnMovement) return;
	if (instant)
	{
		PawnMovement->Velocity = dir * value;
	}
	else
	{
		AddMovementInput(dir, value);
	}
}

void AVRCharacter::StopMovement()
{	
	if (!PawnMovement) return;
	//Stop all the movement
	PawnMovement->StopMovementImmediately();
}

void AVRCharacter::Walk()
{
	if (!PawnMovement) return;
	PawnMovement->MaxSpeed = CharacterAttributeComponent->WalkSpeed;
}

void AVRCharacter::Run()
{
	if (!PawnMovement) return;
	PawnMovement->MaxSpeed = CharacterAttributeComponent->RunSpeed;
}

FTransform AVRCharacter::GetActorTransform()
{
	return this->GetTransform();
}

UCameraFadeSensor* AVRCharacter::GetCameraFadeSensor() const
{
	return CameraFadeSensor;
}

UGroundHitSensor* AVRCharacter::GetGroundHitSensor() const
{
	return GroundHitSensor;
}

bool AVRCharacter::IsGrounded() const
{
	if (GroundHitSensor)
	{
		return GroundHitSensor->IsHit();
	}
	return false;
}

void AVRCharacter::SetNewActorLocation(const FVector& worldLocation, bool sweep, FHitResult* outHit, ETeleportType teleType)
{
	this->SetActorLocation(worldLocation, sweep, outHit, teleType);
}

void AVRCharacter::ApplyCameraFade(float cameraFadeOpacity)
{
	if (!CameraFadeComponent || !FadeDynamicMaterial) return;
	//Apply new opacity to the camera fade mesh via dynamic material instance
	if (cameraFadeOpacity > 0.01f)
	{
		if (CameraFadeComponent->bHiddenInGame)
		{
			CameraFadeComponent->SetHiddenInGame(false);
		}
		FadeDynamicMaterial->SetScalarParameterValue("Opacity", cameraFadeOpacity);
	}
	else
	{
		if (!CameraFadeComponent->bHiddenInGame)
		{
			CameraFadeComponent->SetHiddenInGame(true);
		}
	}
}

void AVRCharacter::DrawMsg(const FString& msg)
{
	if (msg.IsEmpty()) return;

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(1, 5.f, FColor::Red, msg, true);
	}
}

void AVRCharacter::UpdateCapsuleComponentPosition()
{
	if (!CameraComponent || !CapsuleCollisionComponent) return;
	//We take the position of the VR Camera relative to the Track Space origin
	FVector CameraLoc = CameraComponent->GetRelativeLocation();
	//Dont forget to shift down Capsule Component by -half height of the Capsule Component
	float currentHalfHeight = CapsuleCollisionComponent->GetUnscaledCapsuleHalfHeight();
	FVector TargetCapsuleLoc(CameraLoc.X, CameraLoc.Y, currentHalfHeight);
	//Take to the account Mesh Offset, as Capsule is the Parent of the Mesh. And we have to shift it back in X axis so Camera will be in the eye location
	CapsuleCollisionComponent->SetRelativeLocation(TargetCapsuleLoc, false, nullptr, ETeleportType::TeleportPhysics);
}

void AVRCharacter::ApplyRotationFromCameraToCapsule(float DeltaTime)
{
	if (!CameraComponent || !CapsuleCollisionComponent) return;
	//Get the yaw component of the rotaion (Along Z axis) in a tracking Space
	float HeadYaw = CameraComponent->GetRelativeRotation().Yaw;
	//Get Current Rotation of the Skeletal Mesh, so we don't copy it, we accumulate
	FRotator CurrentRot = CapsuleCollisionComponent->GetRelativeRotation();
	//Build Yaw rotation Matrix
	FRotator TargetRot = FRotator(0.f, HeadYaw, 0.f);
	//Calculate new interpolated rotation
	FRotator SmoothedRot = FMath::RInterpTo(CurrentRot, TargetRot, DeltaTime, 10.0f);
	//Apply rotation
	CapsuleCollisionComponent->SetRelativeRotation(SmoothedRot);
}

UObstacleSensor* AVRCharacter::GetObstacleSensor() const
{
	return ObstacleSensor;
}

UVelocitySensor* AVRCharacter::GetCameraVelocitySensor() const
{
	return CameraVelocitySensor;
}

UVelocitySensor* AVRCharacter::GetMotionControllerVelocitySensor(bool right) const
{
	return right ? RightMotionControllerVelocitySensor : LeftMotionControllerVelocitySensor;
}

bool AVRCharacter::IsCrouching(float* crouchDepth) const
{
	if (!CameraComponent) return false;
	//Get camera location relative to the Tracking Space, take Z coord
	float currPlayerHeight = CameraComponent->GetRelativeLocation().Z;
	//Deapth of the Crouch
	float h = (InitialPlayerHeight - currPlayerHeight);
	if (crouchDepth)
	{
		*crouchDepth = h;
	}	
	return h > CharacterAttributeComponent->CrouchThreshold;
}

#if WITH_EDITOR

void AVRCharacter::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	//Get the properety name that was changed by the canging it's value in editor
	FName propName = PropertyChangedEvent.GetPropertyName();
	if (propName == NAME_None) return;
	if (propName == GET_MEMBER_NAME_CHECKED(AVRCharacter, DeadZoneRadius) 
		|| propName == GET_MEMBER_NAME_CHECKED(AVRCharacter, DeadZoneHeight))
	{
		ConfigureDecalSize(DeadZoneDecalComponent, DeadZoneHeight, DeadZoneRadius, true);
	}
	else if (propName == GET_MEMBER_NAME_CHECKED(AVRCharacter, ActiveZoneRadius)
		|| propName == GET_MEMBER_NAME_CHECKED(AVRCharacter, ActiveZoneHeight))
	{
		ConfigureDecalSize(ActiveZoneComponent, ActiveZoneHeight, ActiveZoneRadius, true);
	}
	else if (propName == GET_MEMBER_NAME_CHECKED(AVRCharacter, PlayerAnchorZoneRadius)
		|| propName == GET_MEMBER_NAME_CHECKED(AVRCharacter, PlayerAnchorZoneHeight))
	{
		ConfigureDecalSize(PlayerAnchorDecalComponent, PlayerAnchorZoneHeight, PlayerAnchorZoneRadius, true);
	}
}

#endif

void AVRCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	//Call one time after game started. Need for Bone Scale factor calculation
	InitializeBodyMetrics();
	//Controls Capsule Mesh Component Relations
	RecalibrateCapsuleAndMeshComponent();
	UpdateCapsuleComponentPosition();
	ApplyRotationFromCameraToCapsule(DeltaTime);
	//Call Sensors
	float capsHalfHeight = CapsuleCollisionComponent->GetScaledCapsuleHalfHeight();
	if (CameraFadeSensor)
	{
		CameraFadeSensor->SetMaxScanDistance(capsHalfHeight * 2.f);
		CameraFadeSensor->DoScan(DeltaTime);
	}
	if (GroundHitSensor && CapsuleCollisionComponent)
	{
		GroundHitSensor->UpdateCapsuleHalfHeight(capsHalfHeight);
		GroundHitSensor->DoScan(DeltaTime);
	}
	if (CameraVelocitySensor)
	{
		CameraVelocitySensor->DoScan(DeltaTime);
	}
	if (LeftMotionControllerVelocitySensor)
	{
		LeftMotionControllerVelocitySensor->DoScan(DeltaTime);
	}
	if (RightMotionControllerVelocitySensor)
	{
		RightMotionControllerVelocitySensor->DoScan(DeltaTime);
	}
	if (ObstacleSensor)
	{
		ObstacleSensor->SetScanDirection(CameraVelocitySensor->GetVelocity());
		ObstacleSensor->DoScan(DeltaTime);
	}
	if (LeftMotionControllerHitSensor)
	{
		LeftMotionControllerHitSensor->DoScan(DeltaTime);
		ProcessHitTarget(LeftMotionControllerHitTarget, LeftMotionControllerHitSensor);
	}
	if (RightMotionControllerHitSensor)
	{
		RightMotionControllerHitSensor->DoScan(DeltaTime);
		ProcessHitTarget(RightMotionControllerHitTarget, RightMotionControllerHitSensor);
	}
	//Subsytems
	UVRInteractionSubsystem* vrInter = GetVRSubSystem();
	if (vrInter)
	{
		vrInter->ProcessMotionControllerHitSensor(RightMotionControllerHitSensor, EVRControllerHand::Right, this);
		vrInter->ProcessMotionControllerHitSensor(LeftMotionControllerHitSensor, EVRControllerHand::Left, this);
	}

	//Physics Gravity Calculations
	if (GravityStateManager)
	{
		GravityStateManager->OnTick(DeltaTime);
	}		
	if (LocomotionStateManager)
	{
		LocomotionStateManager->OnTick(DeltaTime);
	}	
	//IK Calculations
	UVRCharacterAnimInstance* inst = GetCharAnimInstance();
	if (inst)
	{
		//We require additional sensors for Elbows or some Math 
		inst->CalculateElbowJointTarget(DeltaTime);
		inst->CalculateFootIKEffectors(this, DeltaTime);
		inst->CalculateSpineRotation(DeltaTime);
		inst->CalculateMotionControllerTransform(RightMotionController, true, DeltaTime);
		inst->CalculateMotionControllerTransform(LeftMotionController, false, DeltaTime);
	}
}
