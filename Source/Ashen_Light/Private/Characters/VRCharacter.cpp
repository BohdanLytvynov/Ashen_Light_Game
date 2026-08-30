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
#include "Constants.h"

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
	CameraFadeComponent->SetupAttachment(CameraComponent);
	CameraFadeComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);//Disable collision for fade Palne Mesh
	CameraFadeComponent->SetHiddenInGame(true);//Hide it during start
	CameraFadeComponent->SetCastShadow(false);//We don't want to use extra resources for shdes calculations
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

	//Right motion Controller
	RightMotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("Right Motion Controller"));
	if (!RightMotionController) return;
	RightMotionController->SetupAttachment(TrackingSpaceOrigin);
	RightMotionController->MotionSource = FName("Right");
	RightMotionController->bDisableLowLatencyUpdate = false;
	RightMotionController->bDisplayDeviceModel = true;//Allow to render the defaul mesh for motion controllers
	RightMotionController->DisplayModelSource = FName("OculusHMD");//Set default for Oculus Quest
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
		LocomotionStateManager->SwitchState(ELocomotionSpace::ELS_TrackingSpace);
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

	initialPlayerMetricsCalculated = false;
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
	if (GetMesh() && PawnMovement)
	{
		GetMesh()->PrimaryComponentTick.AddPrerequisite(PawnMovement, PawnMovement->PrimaryComponentTick);
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
		LocomotionStateManager->BeginPlay();
	}
	if (GravityStateManager)
	{
		GravityStateManager->BeginPlay();
	}

	FadeDynamicMaterial = UMaterialInstanceDynamic::Create(FadeMaterialBase, this);
	if (FadeDynamicMaterial && CameraFadeComponent)
	{
		CameraFadeComponent->SetMaterial(0, FadeDynamicMaterial);
	}
}

void AVRCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	//Clear all the bindings when we exit the game
	FCoreDelegates::VRHeadsetRecenter.RemoveAll(this);
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
	GetWorldTimerManager().SetTimerForNextTick(this, &AVRCharacter::RecalibrateCapsuleAndMeshComponent);
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
		USkeletalMeshComponent* skelMesh = GetMesh();
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
	if (!PawnMovement || FMath::IsNearlyZero(runSpeed)) return 0.f;
	//We calculate normalize Velocity clamped to the [0 ; 1] in XY plane
	return FMath::Clamp(PawnMovement->Velocity.Size2D() / runSpeed, 0.f, 1.f);
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

void AVRCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (!PlayerInputComponent) return;
	//Setup binding
	//Locomotion
	PlayerInputComponent->BindAction(FName("ToggleMovement"), IE_Pressed, this, &AVRCharacter::OnToggleMovementPressed);
	PlayerInputComponent->BindAction(FName("ToggleMovement"), IE_Released, this, &AVRCharacter::OnToggleMovementReleased);
	//Grip Mechanics
	PlayerInputComponent->BindAction(FName("GrabRight"), IE_Pressed, this, &AVRCharacter::OnRightGrabButtonPressed);
	PlayerInputComponent->BindAction(FName("GrabRight"), IE_Released, this, &AVRCharacter::OnRightGrabButtonReleased);
	PlayerInputComponent->BindAction(FName("GrabLeft"), IE_Pressed, this, &AVRCharacter::OnLeftGrabButtonPressed);
	PlayerInputComponent->BindAction(FName("GrabLeft"), IE_Released, this, &AVRCharacter::OnLeftGrabButtonReleased);
	//Trigger Mechanics
	PlayerInputComponent->BindAction(FName("TriggerRight"), IE_Pressed, this, &AVRCharacter::OnRightTriggerButtonPressed);
	PlayerInputComponent->BindAction(FName("TriggerRight"), IE_Released, this, &AVRCharacter::OnRightTriggerButtonReleased);
	PlayerInputComponent->BindAction(FName("TriggerLeft"), IE_Pressed, this, &AVRCharacter::OnLeftTriggerButtonPressed);
	PlayerInputComponent->BindAction(FName("TriggerLeft"), IE_Released, this, &AVRCharacter::OnLeftTriggerButtonReleased);
}

void AVRCharacter::OnToggleMovementPressed()
{
	if (LocomotionStateManager)
	{
		LocomotionStateManager->SwitchState(uint8(ELocomotionSpace::ELS_TrackingSpace));
	}
}

void AVRCharacter::OnToggleMovementReleased()
{
	if (LocomotionStateManager)
	{
		LocomotionStateManager->SwitchState(uint8(ELocomotionSpace::ELS_Physical));
	}
}

void AVRCharacter::OnRightGrabButtonPressed()
{
	UVRCharacterAnimInstance* inst = GetCharAnimInstance();
	if (!inst) return;
	inst->SetGripState(true, true);
}

void AVRCharacter::OnRightGrabButtonReleased()
{
	UVRCharacterAnimInstance* inst = GetCharAnimInstance();
	if (!inst) return;
	inst->SetGripState(true, false);
}

void AVRCharacter::OnLeftGrabButtonPressed()
{
	UVRCharacterAnimInstance* inst = GetCharAnimInstance();
	if (!inst) return;
	inst->SetGripState(false, true);
}

void AVRCharacter::OnLeftGrabButtonReleased()
{
	UVRCharacterAnimInstance* inst = GetCharAnimInstance();
	if (!inst) return;
	inst->SetGripState(false, false);
}

void AVRCharacter::OnRightTriggerButtonPressed()
{
	UVRCharacterAnimInstance* inst = GetCharAnimInstance();
	if (!inst) return;
	inst->SetTriggerState(true, true);
}

void AVRCharacter::OnRightTriggerButtonReleased()
{
	UVRCharacterAnimInstance* inst = GetCharAnimInstance();
	if (!inst) return;
	inst->SetTriggerState(true, false);
}

void AVRCharacter::OnLeftTriggerButtonPressed()
{
	UVRCharacterAnimInstance* inst = GetCharAnimInstance();
	if (!inst) return;
	inst->SetTriggerState(false, true);
}

void AVRCharacter::OnLeftTriggerButtonReleased()
{
	UVRCharacterAnimInstance* inst = GetCharAnimInstance();
	if (!inst) return;
	inst->SetTriggerState(false, false);
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
	PawnMovement->MaxSpeed = walkSpeed;
}

void AVRCharacter::Run()
{
	if (!PawnMovement) return;
	PawnMovement->MaxSpeed = runSpeed;
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
		CameraFadeComponent->SetHiddenInGame(false);
		FadeDynamicMaterial->SetScalarParameterValue("Opacity", cameraFadeOpacity);
	}
	else
	{
		CameraFadeComponent->SetHiddenInGame(true);
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
	return h > CrouchThreshold;
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
	else if (propName == GET_MEMBER_NAME_CHECKED(AVRCharacter, PreviewLocomotionState) && LocomotionStateManager)
	{
		LocomotionStateManager->SwitchState(PreviewLocomotionState);
	}
}

#endif

FString ConvertEnumToStr(uint8 state)
{
	ELocomotionSpace e = static_cast<ELocomotionSpace>(state);
	switch (e)
	{
	case ELocomotionSpace::ELS_Physical:
		return FString("Physical");
	case ELocomotionSpace::ELS_TrackingSpace:
		return FString("Tracking Space");
	}
	return FString("Unable to map to the Enum!");
}

FString ConvertGEnumToStr(uint8 state)
{
	EGravityState e = static_cast<EGravityState>(state);
	switch (e)
	{
	case EGravityState::EGS_InAir:
		return FString("In Air");
	case EGravityState::EGS_Grounded:
		return FString("Grounded");
	case EGravityState::EGS_InMesh:
		return FString("In Mesh");
	case EGravityState::EGS_Climbing:
		return FString("Climbing");
	}
	return FString("Unable to map to the Enum!");
}

void AVRCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (GEngine && EnableStateManagersDebug)
	{
		GEngine->AddOnScreenDebugMessage(20, 3.f, FColor::Red, FString::Printf(TEXT("Curr Locomotion: %s"), *ConvertEnumToStr(LocomotionStateManager->GetCurrentStateEnum())));
		GEngine->AddOnScreenDebugMessage(21, 3.f, FColor::Red, FString::Printf(TEXT("Prev Locomotion: %s"), *ConvertEnumToStr(LocomotionStateManager->GetPrevStateEnum())));

		GEngine->AddOnScreenDebugMessage(22, 3.f, FColor::Green, FString::Printf(TEXT("Curr Gravity: %s"), *ConvertGEnumToStr(GravityStateManager->GetCurrentStateEnum())));
		GEngine->AddOnScreenDebugMessage(23, 3.f, FColor::Green, FString::Printf(TEXT("Prev Gravity: %s"), *ConvertGEnumToStr(GravityStateManager->GetPrevStateEnum())));
	}
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
		inst->CalculateElbowJointTarget(DeltaTime, EnableIkDebug);
		inst->CalculateFootIKEffectors(this, DeltaTime, EnableIkDebug);
		inst->CalculateSpineRotation(DeltaTime, EnableIkDebug);
	}
}
