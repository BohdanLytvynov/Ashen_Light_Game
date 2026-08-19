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
#include "IXRTrackingSystem.h"
#include "DrawDebugHelpers.h"


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

	PawnMovement = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("Pawn Floating Movement"));

	//Sensors
	CameraFadeSensor = CreateDefaultSubobject<UCameraFadeSensor>(TEXT("Camera Fade Sensor"));

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
		GravityStateManager->SwitchState(EGravityState::EGS_InAir);
	}

	bIsObstacleHit = false;
	bIsJumping = false;
	CurrentVelocity = FVector::ZeroVector;
	PrevCameraPosition = FVector::ZeroVector;
	initialPlayerHeightCalculated = false;
	bCameraInAMesh = false;
}

void AVRCharacter::BeginPlay()
{
	Super::BeginPlay();
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

	//Set Initial Location of the motion controllers and Camera in Tracking Space when the game begins
	//Need this for Velocity Calculations
	if (LeftMotionController)
	{
		prevLeftHandLocation = LeftMotionController->GetRelativeLocation();
	}

	if (RightMotionController)
	{
		prevRightHandLocation = RightMotionController->GetRelativeLocation();
	}

	if (CameraComponent)
	{
		PrevCameraPosition = CameraComponent->GetComponentLocation();
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
	if (CameraComponent)
	{
		PrevCameraPosition = CameraComponent->GetComponentLocation();
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
	PrevCameraPosition = CameraComponent->GetComponentLocation();
	
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

void AVRCharacter::CalculatePlayerHeight()
{
	if (!initialPlayerHeightCalculated && CameraComponent)
	{
		float CurrentCamZ = CameraComponent->GetRelativeLocation().Z;
		if (CurrentCamZ > 80.0f)
		{
			InitialPlayerHeight = CurrentCamZ;
			initialPlayerHeightCalculated = true;
			UVRCharacterAnimInstance* inst = GetCharAnimInstance();
			if (inst)
			{
				inst->CalculateUniversalScaleFactor(CurrentCamZ);

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

float AVRCharacter::GetMotionControllerSpeed(UMotionControllerComponent* motionController, FVector* prevMotionControllerPosition, float DeltaTime) const
{
	float res = -1.f;
	if (!motionController || DeltaTime == 0 || !prevMotionControllerPosition) return res;
	//Get location of the motion controller in the tracking space
	const FVector MotionControllerLocationInVROriginSpace = motionController->GetRelativeLocation();
	//v = ds/dt. ds = |start - end|, as dt -> 0 we get the Instant Velocity
	res = ((MotionControllerLocationInVROriginSpace - *prevMotionControllerPosition).Size()) / DeltaTime;
	*prevMotionControllerPosition = MotionControllerLocationInVROriginSpace;//Update the previous motion controller position
	return res;
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

void AVRCharacter::DetectGround()
{
	if (!CapsuleCollisionComponent) return;
	if (bCameraInAMesh)
	{
		//We need to stop ground detection and set it to true. Cause we need to be on ground when we are out of the level boundaries
		bIsGrounded = true;
		return;
	}
	//Disable Ground Detection and set bIsGrounded to false
	if (bIsJumping)
	{
		bIsGrounded = false;
		return;
	}

	//Get the half height of the capsule. Must be recalculated cause we do recalibration according to the player height
	const float HalfHeight = CapsuleCollisionComponent->GetScaledCapsuleHalfHeight();
	//Get the Location of the Capsule Component in the World Space, it is the center of the Capsule and add Half of the Capsule to shift it up
	const FVector Start = CapsuleCollisionComponent->GetComponentLocation() + FVector(0.f, 0.f, HalfHeight);
	const float SphereRadius = 20.f;//Radius of the cast sphere
	//End of the Trace. All the height of the Capsule + Some distance check threshold, 10 cm. For landscape curvatures
	const FVector End = Start - FVector(0.f, 0.f, (HalfHeight * 2.f) + GroundDetectionThreshold);
	//Perform trace
	TArray<AActor*> IgnoreActors;
	IgnoreActors.Add(this);
	FHitResult HitResult;	
	bool bHit = UKismetSystemLibrary::SphereTraceSingle(
		this,
		Start,
		End,
		SphereRadius,
		UEngineTypes::ConvertToTraceType(ECC_WorldStatic),//Filter by ECC_WorldStatic, cause floor is the static mesh
		false, 
		IgnoreActors,
		EDrawDebugTrace::None,
		HitResult,
		true,
		FLinearColor::Red,
		FLinearColor::Green,
		1.f
	);
	//We hit the ground and Normal direction is within propriate angle
	if (bHit && HitResult.bBlockingHit && HitResult.ImpactNormal.Z > 0.5f)
	{
		bIsGrounded = true;
		CurrentGroundHit = HitResult;
	}
	else
	{
		bIsGrounded = false;
	}
}

void AVRCharacter::ApplyGravity(float DeltaTime)
{
	//Turn off gravity so we will not fall down when we walk through the mesh by our legs
	if (bCameraInAMesh) 
	{
		VerticalVelocity = 0.f;
		return;
	}
	
	if (bIsJumping && VerticalVelocity <= 0.0f)
	{
		bIsJumping = false;
	}

	if (bIsGrounded && !bIsJumping)
	{		
		VerticalVelocity = -10.f;
		//Update Z velocity
		if (PawnMovement)
		{
			PawnMovement->Velocity.Z = VerticalVelocity;
		}
		//Get HalfHeight of the capsule component
		const float ScaledHalfHeight = CapsuleCollisionComponent->GetScaledCapsuleHalfHeight();
		//Calculate Bottom of the CapsuleComponent
		const float CapsuleBottomZ = CapsuleCollisionComponent->GetComponentLocation().Z - ScaledHalfHeight;
		//Calculate penetration depth
		const float PenetrationDepth = CurrentGroundHit.ImpactPoint.Z - CapsuleBottomZ;
		
		if (!FMath::IsNearlyZero(PenetrationDepth))
		{
			//Calculate corect Actor Location
			FVector CurrentLocation = GetActorLocation();
			CurrentLocation.Z += PenetrationDepth;
			//Turn off the physics detection, because we wil be stacked in the floor mesh
			SetActorLocation(CurrentLocation, false, nullptr, ETeleportType::TeleportPhysics);
		}
	}
	else
	{
		//Vertical velocity accumulation v = v + g * dt, where g = -9.8 m / sec^2 = -980 cm / sec^2
		VerticalVelocity += -1.f * GravityConstant * DeltaTime;
		//Clamp Vertiacal velocity between terminal velocity and max fall velocity
		VerticalVelocity = FMath::Clamp(VerticalVelocity, TerminalVelocity, MaxFallVelocity);
		
		if (PawnMovement)
		{
			PawnMovement->Velocity.Z = VerticalVelocity;//Set Velocity Directly
		}
	}
}

FVector AVRCharacter::AdjustInputForSlope(const FVector& InputVector) const
{
	//We are in Air no floor normals adjustment is required
	if (!bIsGrounded)
	{
		return InputVector;
	}
	//We project the normal Input Vector on the plane using its normal (we get it from FHit GroundHit)
	//Vprj = V - (N*V)*N
	return FVector::VectorPlaneProject(InputVector, CurrentGroundHit.ImpactNormal).GetSafeNormal() * InputVector.Size();
}

bool AVRCharacter::CheckObstaclesInDirection(const FVector& NormDirection, float Distance, float halfHeightMultipl, FHitResult& OutHit)
{
	if (NormDirection.IsNearlyZero()) return false;
	if (!CapsuleCollisionComponent) return false;
	FVector Start = CapsuleCollisionComponent->GetComponentLocation();
	FVector End = Start + (NormDirection * Distance);
	TArray<AActor*> actorsToIgnore;
	actorsToIgnore.Add(this);
	const float radius = CapsuleCollisionComponent->GetScaledCapsuleRadius();
	const float halfHeight = CapsuleCollisionComponent->GetScaledCapsuleHalfHeight() * halfHeightMultipl;
	bool hit = UKismetSystemLibrary::CapsuleTraceSingle(this, Start, End, radius, halfHeight,
		UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_WorldStatic),
		false, actorsToIgnore, EDrawDebugTrace::None, OutHit, true, FColor::Orange, FColor::Blue, 1.f);
	if (hit && OutHit.bBlockingHit)
	{
		return true;
	}	
	return false;
}

bool AVRCharacter::IsGrounded() const
{
	return bIsGrounded;
}

bool AVRCharacter::CanJump() const
{
	//We can jump only if we are on the Ground
	if (!bIsGrounded) return false;
	//We can't jump if we are Jumping right Now
	if (bIsJumping) return false;
	if (!CameraComponent) return false;
	float currentHeight = CameraComponent->GetRelativeLocation().Z;
	float delta = currentHeight - InitialPlayerHeight;
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(7, 1.f, FColor::Red, FString::Printf(TEXT("Delta: %f"), delta));
	}
	if (FMath::IsNearlyZero(delta)) return false;
	if (delta > JumpThreshold) return true;
	return false;
}

FTransform AVRCharacter::GetActorTransform()
{
	return this->GetTransform();
}

void AVRCharacter::JumpPhysical(float height)
{
	if (!CanJump()) return;
	if (!PawnMovement) return;
	float vel = FMath::Sqrt(2 * GravityConstant * height);
	VerticalVelocity = vel;
	Move(FVector(0.f, 0.f, 1.f), VerticalVelocity, true);
	bIsJumping = true;
	bIsGrounded = false;
}

void AVRCharacter::JumpTracking()
{

}

UCameraFadeSensor* AVRCharacter::GetCameraFadeSensor() const
{
	return CameraFadeSensor;
}

//bool AVRCharacter::IsCameraInMesh(
//	bool debug,
//	FColor traceColor,
//	FColor traceHitColor,
//	float debugDrawTime)
//{
//	/*if (!CameraComponent || !CapsuleCollisionComponent) return false;
//	const FVector Start = CameraComponent->GetComponentLocation();
//	float checkDist = (CapsuleCollisionComponent->GetScaledCapsuleHalfHeight() * 2.0f) * FadeCheckDistanceRatio;
//	const FVector End = Start - FVector(0.f, 0.f, checkDist);
//	FCollisionQueryParams QueryParams;
//	QueryParams.AddIgnoredActor(this);
//	QueryParams.bTraceComplex = false;
//	UWorld* w = GetWorld();
//	if (!w) return false;
//	FHitResult outHit;
//	const bool bHit = w->SweepSingleByChannel(
//		outHit,
//		Start,
//		End,
//		FQuat::Identity,
//		ECC_WorldStatic,
//		FCollisionShape::MakeSphere(FadeCheckRadius),
//		QueryParams
//	);
//	const bool bValidBlockingHit = bHit && outHit.bBlockingHit;
//	if (debug)
//	{
//		if (bValidBlockingHit)
//		{
//			DrawDebugSphere(w, outHit.ImpactPoint, FadeCheckRadius, 8, traceHitColor, false, debugDrawTime);
//		}		
//		if (!FMath::IsNearlyZero(checkDist))
//		{
//			const FVector Center = Start - FVector(0.f, 0.f, checkDist * 0.5f);
//			const float HalfHeight = (checkDist * 0.5f) + FadeCheckRadius;
//			DrawDebugCapsule(w, Center, HalfHeight, FadeCheckRadius, FQuat::Identity, traceColor, false, debugDrawTime);
//		}
//	}
//
//	if (bValidBlockingHit)
//	{
//		CameraInMeshHit = outHit;
//	}
//
//	return bValidBlockingHit;*/
//
//	return false;
//}
//
//void AVRCharacter::CheckCameraFade(float DeltaTime)
//{
//	//if (!CameraComponent) return;
//	////Camera Location in the World Space
//	//const FVector startEnd = CameraComponent->GetComponentLocation();	
//	//TArray<AActor*> actorsToIgnore;
//	//actorsToIgnore.Add(this);
//	//FHitResult hitResult;
//	////We trace sphere from camera to camera, FadeCheckRadius - Radius of the Trace Sphere
//	//bool bHit = UKismetSystemLibrary::SphereTraceSingle(
//	//this, startEnd, startEnd, FadeCheckRadius, 
//	//	UEngineTypes::ConvertToTraceType(ECC_WorldStatic), false, actorsToIgnore, EDrawDebugTrace::None, hitResult, true);
//
//	//float targetOpacity = 0.f;
//	//FString cameraOutWarning = TEXT("");
//	//if (bHit && hitResult.bBlockingHit)//Camera is in mesh
//	//{
//	//	bCameraInAMesh = true;
//	//	//Calculate Penetration Depth
//	//	//Dist from impact point to the Camera
//	//	FVector ImactToCamera = CameraComponent->GetComponentLocation() - hitResult.ImpactPoint;
//	//	float PenetrationDepth = FadeCheckRadius - ImactToCamera.Size();
//	//	//Normalized fase distance. As we closer to 5 cm - it will give us 1 and totaly fade the camera
//	//	float SafeFadeDistance = (CameraFadeDistance > 0.1f) ? CameraFadeDistance : 5.0f;
//	//	targetOpacity = FMath::Clamp(PenetrationDepth / SafeFadeDistance, 0.f, 1.f);
//
//	//	if (targetOpacity > 0.3f)
//	//	{
//	//		cameraOutWarning = FString(TEXT("Please return back to the borders of the Tracking Space.")).ToUpper();
//	//		DrawMsg(cameraOutWarning);
//	//	}
//	//}
//	//else
//	//{
//	//	bCameraInAMesh = false;
//	//}
//	////Fade opacity interpolation
//	//CurrentCameraFadeOpacity = FMath::FInterpTo(CurrentCameraFadeOpacity, targetOpacity, DeltaTime, 10.f);
//}

void AVRCharacter::ApplyCameraFade()
{
	if (!CameraFadeComponent || !FadeDynamicMaterial) return;
	//Apply new opacity to the camera fade mesh via dynamic material instance
	if (CurrentCameraFadeOpacity > 0.01f)
	{
		CameraFadeComponent->SetHiddenInGame(false);
		FadeDynamicMaterial->SetScalarParameterValue("Opacity", CurrentCameraFadeOpacity);
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

void AVRCharacter::CalculateCurrentVelocity(float DeltaTime)
{
	if (!CameraComponent || FMath::IsNearlyZero(DeltaTime)) return;
	//World Camera Location
	FVector CurrentCameraPosition = CameraComponent->GetComponentLocation();
	//use V = ds / dt, where dt -> 0 - Instant Velocity
	CurrentVelocity = (CurrentCameraPosition - PrevCameraPosition) / DeltaTime;
	PrevCameraPosition = CurrentCameraPosition;
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

bool AVRCharacter::IsSwiningArms(float DeltaTime)
{
	if (!RightMotionController || !LeftMotionController) return false;
	//Get The Velocity of the right and left motion controller
	float rightContrVelocity = GetMotionControllerSpeed(RightMotionController, &prevRightHandLocation, DeltaTime);
	float leftContrVelocity = GetMotionControllerSpeed(LeftMotionController, &prevLeftHandLocation, DeltaTime);
	if (rightContrVelocity > SwiningThreshold && leftContrVelocity > SwiningThreshold)
		return true;
	return false;
}

bool AVRCharacter::CheckObstacles(float obstacleDistDetection, float halfHeightMultipl, FHitResult& OutHit)
{
	//No need to check obstacles when we are not moving	
	if (CurrentVelocity.IsNearlyZero()) return false;
	if (!CapsuleCollisionComponent) return false;
	//Camera in World Space
	FVector start = CapsuleCollisionComponent->GetComponentLocation();
	//Vector in direction of the Pawn Velocity from the Camera in a World space,
	//We use the Instant Velocity of the Pawn in 3d Space to have an ability to check obstacles in all 360 directions
	FVector end = start + CurrentVelocity.GetSafeNormal() * obstacleDistDetection;
	TArray<AActor*> actorsToIgnore;
	actorsToIgnore.Add(this);
	FHitResult hit;
	float halfHeight = CapsuleCollisionComponent->GetScaledCapsuleHalfHeight() * halfHeightMultipl;
	bool bHit = UKismetSystemLibrary::CapsuleTraceSingle(this,
		start,
		end,
		CapsuleCollisionComponent->GetScaledCapsuleRadius(),
		halfHeight,
		UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_WorldStatic),
		false, actorsToIgnore, EDrawDebugTrace::None, hit, true,
		FLinearColor::Red, FLinearColor::Green, 1.f);

	if (bHit && hit.bBlockingHit)//We faced the odstacle
	{
		OutHit = hit;
		return true;
	}

	return false;
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

void AVRCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(1, 5.f, FColor::Red, FString::Printf(TEXT("Curr State: %s"), *ConvertEnumToStr(LocomotionStateManager->GetCurrentStateEnum())));		
	}
	CalculatePlayerHeight();
	//Controls Capsule Mesh Component Relations
	RecalibrateCapsuleAndMeshComponent();
	UpdateCapsuleComponentPosition();
	ApplyRotationFromCameraToCapsule(DeltaTime);
	CalculateCurrentVelocity(DeltaTime);
	//Call Sensors
	if (CameraFadeSensor && CameraComponent)
	{
		CameraFadeSensor->UpdateSensorPosition(CameraComponent->GetComponentLocation());
		TArray<AActor*> ignore;
		ignore.Add(this);
		CameraFadeSensor->DoScan(ignore);
	}

	//Physics Gravity Calculations
	if (GravityStateManager)
	{
		GravityStateManager->OnTick(DeltaTime);
	}

	//Physics Calculations	
	//DetectGround();
	//ApplyGravity(DeltaTime);
	//Camera Fade
	//CheckCameraFade(DeltaTime);
	ApplyCameraFade();
	
	if (LocomotionStateManager)
	{
		LocomotionStateManager->OnTick(DeltaTime);
	}
	
	//IK Calculations
	UVRCharacterAnimInstance* inst = GetCharAnimInstance();
	if (inst)
	{
		//We require additional sensors for Elbows or some Math 
		inst->CalculateElbowJointTarget(DeltaTime, false);
		inst->CalculateSpineRotation(DeltaTime, false);
	}
}
