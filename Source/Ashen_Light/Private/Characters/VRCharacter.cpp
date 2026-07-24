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
	CameraComponent->bLockToHmd = true;//Bind Transform of the HMD to the Camera in Tracking Space
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

	bIsObstacleHit = false;

	CurrentCharacterState = EVRCharacterState::VRCS_FreeRoam;
	bCanPerformBattleStep = false;
	bCanRun = false;

	CurrentVelocity = FVector::ZeroVector;
	PrevCameraPosition = FVector::ZeroVector;

	bCameraInAMesh = false;
	bVirtControllerLocked = false;
}

void AVRCharacter::BeginPlay()
{
	Super::BeginPlay();
	//Set Tracking origin to Floor.When the game starts the camera will be placed in the center of the Tracking Space idealy. 
	//Then it will be rised up according to the distance to the floor in the real room
	UHeadMountedDisplayFunctionLibrary::SetTrackingOrigin(EHMDTrackingOrigin::Floor);
	//Bind to the Recenter event
	FCoreDelegates::VRHeadsetRecenter.AddUObject(this, &AVRCharacter::OnHMD_Recentered);
	FTimerHandle SpawnRecenterTimerHandle;
	//Try to move Tracking Space root to the position of the HMD
	GetWorldTimerManager().SetTimer(
		SpawnRecenterTimerHandle,
		this,
		&AVRCharacter::RecenterTrackingSpaceToActor,
		0.15f,
		false
	);
	//Initialize dynamic material instance to controll parameters during runtime (Opacity)
	if (FadeMaterialBase)
	{
		FadeDynamicMaterial = UMaterialInstanceDynamic::Create(FadeMaterialBase, this);
		if (CameraFadeComponent && FadeDynamicMaterial)
		{
			CameraFadeComponent->SetMaterial(0, FadeDynamicMaterial);
		}
	}
	//Configure Decals
	ConfigureDecal(DeadZoneDecalComponent, DeadZoneHeight, DeadZoneRadius, true);
	ConfigureDecal(ActiveZoneComponent, ActiveZoneHeight, ActiveZoneRadius, true);
	ConfigureDecal(PlayerAnchorDecalComponent, PlayerAnchorZoneHeight, PlayerAnchorZoneRadius, true);

	//Set Initial Location of the motion controllers and Camera in Tracking Space when the game begins
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

float AVRCharacter::GetGroundVelocityRatio() const
{
	if (!PawnMovement || FMath::IsNearlyZero(runSpeed)) return 0.f;
	//We calculate normalize Velocity clamped to the [0 ; 1] in XY plane
	return FMath::Clamp(PawnMovement->Velocity.Size2D() / runSpeed, 0.f, 1.f);
}

void AVRCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (!PlayerInputComponent) return;
	//Setup binding
	PlayerInputComponent->BindAction(FName("ToggleBattleMode"), IE_Pressed, this, &AVRCharacter::OnToggleBattleModePressed);
	PlayerInputComponent->BindAction(FName("ToggleRun"), IE_Pressed, this, &AVRCharacter::OnSprintPressed);
	PlayerInputComponent->BindAction(FName("ToggleRun"), IE_Released, this, &AVRCharacter::OnSprintReleased);
}

void AVRCharacter::OnToggleBattleModePressed()
{
	//We are in free Roam Mode switch to Battle Mode and Lock Virtual Joystick movement
	if (CurrentCharacterState == EVRCharacterState::VRCS_FreeRoam)
	{
		CurrentCharacterState = EVRCharacterState::VRCS_Battle;
		bCanPerformBattleStep = true;
	}
	else//We are in battle mode, wsitch to Free Roam
	{
		CurrentCharacterState = EVRCharacterState::VRCS_FreeRoam;
	}
}

void AVRCharacter::OnSprintPressed()
{
	bCanRun = true;//Enable run
}

void AVRCharacter::OnSprintReleased()
{
	bCanRun = false;//Disable run
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
		MeshLoc.Z = -halfHeight;//the root of the Skeletal mesh is located at the feet of the model we need to move it down to halfheight of the capsule
		SkeletalMeshComponent->SetRelativeLocation(MeshLoc);
	}
}

void AVRCharacter::ConfigureDecal(UDecalComponent* decal, float thicknes, float r, bool redraw)
{
	if (!decal) return;
	//Update decal dimensions
	decal->DecalSize = FVector(thicknes, r, r);
	//Should we use force redraw of the decal
	if(redraw)
		decal->MarkRenderStateDirty();
}

void AVRCharacter::HandleMovement(float DeltaTime)
{		
	if (!CameraComponent) return;
	//1 Get HMD Device coordinates in TrackSpace relative to the TrackSpace's origin
	FVector CameraInTrackSpace = CameraComponent->GetRelativeLocation();
	//2 Create the 2D vector 
	FVector CameraInTrackSpace2D(CameraInTrackSpace.X, CameraInTrackSpace.Y, 0.f);
	//Calculate distance
	float Dist2D = CameraInTrackSpace2D.Size();
	if (Dist2D > DeadZoneRadius)//Camera is outside the dead zone start moving
	{
		//Transform HMD location in the Track Space to Unreal Engine World Space
		FVector WorldDirVector = ActorToWorld().TransformVectorNoScale(CameraInTrackSpace2D);

		CheckObstacles();

		if (bIsObstacleHit)//We hit obstacle
		{		
			//Normal that is perpendicular to the Mesh Face
			FVector ImpactNormal = CurrentObstacleHit.ImpactNormal;
			FVector MovementDir = WorldDirVector.GetSafeNormal();

			// 1. Check hit angle (Scalar Project movement dir on the Impact normal)
			//  < 0  : Move in the obstacle direction
			// -1.0f : Perpendicular Hit to the wall
			//  0.0f : Sliding across the wall
			float Dot = FVector::DotProduct(MovementDir, ImpactNormal);
			//Calculate degrees to dot product dp = Cos(degr) 
			float DotThreshold = -FMath::Cos(FMath::DegreesToRadians(StopMovementAngleThreshold));
			if (Dot <= DotThreshold)
			{
				WorldDirVector = FVector::ZeroVector;//Stop Movement
			}
			else
			{
				//Calculate vector along the Static Mesh to slide across
				WorldDirVector = FVector::VectorPlaneProject(WorldDirVector, ImpactNormal);
			}
		}

		if (!WorldDirVector.IsNearlyZero())
		{
			if (IsSwiningArms(DeltaTime) && bCanRun)//Run
			{
				Run();
			}
			else//Walk
			{
				Walk();
			}
			//Modify direction vector to slide across the floor normals
			FVector SlopeAdjustedWorldDirVector = AdjustInputForSlope(WorldDirVector);
			//Call movement via UPawnFloatingMovement
			AddMovementInput(SlopeAdjustedWorldDirVector.GetSafeNormal());
		}
		else
		{
			Stop();
		}
	}
	else
	{
		Stop();
	}
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

float AVRCharacter::GetMotionControllerSpeed(UMotionControllerComponent* motionController, FVector* prevMotionControllerPosition, float DeltaTime)
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

void AVRCharacter::Stop()
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
	//Get the Location of the Capsule Component in the World Space, it is the center of the Capsule
	const FVector Start = CapsuleCollisionComponent->GetComponentLocation();
	//Get the half height of the capsule. Must be recalculated cause we do recalibration according to the player height
	const float HalfHeight = CapsuleCollisionComponent->GetScaledCapsuleHalfHeight();
	const float SphereRadius = 20.f;//Radius of the cast sphere
	//End of the Trace. Sphere has the radius and the hit point will be located in the center of it sphere so we take it's raadius to account
	const FVector End = Start - FVector(0.f, 0.f, HalfHeight - SphereRadius + GroundDetectionThreshold);
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

	if (bIsGrounded)
	{
		//We set some negative velocity, so we try to place pawn on the floor 
		VerticalVelocity = -10.f;
	}
	else
	{
		//Vertical velocity accumulation v = v + g * dt, where g = -9.8 m / sec^2 = -980 cm / sec^2
		VerticalVelocity += GravityConstant * DeltaTime;
		//Clamp Vertiacal velocity between terminal velocity and max fall velocity
		VerticalVelocity = FMath::Clamp(VerticalVelocity, TerminalVelocity, MaxFallVelocity);
		
		if (PawnMovement)
		{
			PawnMovement->Velocity.Z = VerticalVelocity;//Set Velocity Directly
		}
	}
}

FVector AVRCharacter::AdjustInputForSlope(FVector InputVector) const
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

void AVRCharacter::CheckObstacles()
{
	if (!CameraComponent) return;
	//No need to check obstacles when we are not moving
	if (CurrentVelocity.IsNearlyZero())
	{
		bIsObstacleHit = false;
		return;
	}
	//Camera in World Space
	FVector start = CameraComponent->GetComponentLocation();
	//Vector in direction of the Pawn Velocity from the Camera in a World space,
	//We use the Instant Velocity of the Pawn in 3d Space to have an ability to check obstacles in all 360 directions
	FVector end = start + CurrentVelocity.GetSafeNormal() * ObstacleDistanceDetection;
	TArray<AActor*> actorsToIgnore;
	actorsToIgnore.Add(this);
	FHitResult hit;
	const float headRadius = 15.f;//Radisu of the check sphere
	bool bHit = UKismetSystemLibrary::SphereTraceSingle(this, 
		start,
		end,
		headRadius,
		UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_WorldStatic),
		false, actorsToIgnore, EDrawDebugTrace::ForDuration, hit, true, 
		FLinearColor::Red, FLinearColor::Green, 1.f);

	if (bHit && hit.bBlockingHit)//We faced the odstacle
	{
		bIsObstacleHit = true;
		CurrentObstacleHit = hit;
	}
	else
	{
		bIsObstacleHit = false;
	}
}

void AVRCharacter::CheckCameraFade(float DeltaTime)
{
	if (!CameraComponent) return;
	//Camera Location in the World Space
	const FVector startEnd = CameraComponent->GetComponentLocation();	
	TArray<AActor*> actorsToIgnore;
	actorsToIgnore.Add(this);
	FHitResult hitResult;
	//We trace sphere from camera to camera, FadeCheckRadius - Radius of the Trace Sphere
	bool bHit = UKismetSystemLibrary::SphereTraceSingle(
	this, startEnd, startEnd, FadeCheckRadius, 
		UEngineTypes::ConvertToTraceType(ECC_WorldStatic), false, actorsToIgnore, EDrawDebugTrace::None, hitResult, true);

	float targetOpacity = 0.f;
	FString cameraOutWarning = TEXT("");
	if (bHit && hitResult.bBlockingHit)//Camera is in mesh
	{
		bCameraInAMesh = true;
		//Calculate Penetration Depth
		//Dist from impact point to the Camera
		FVector ImactToCamera = CameraComponent->GetComponentLocation() - hitResult.ImpactPoint;
		float PenetrationDepth = FadeCheckRadius - ImactToCamera.Size();
		//Normalized fase distance. As we closer to 5 cm - it will give us 1 and totaly fade the camera
		float SafeFadeDistance = (CameraFadeDistance > 0.1f) ? CameraFadeDistance : 5.0f;
		targetOpacity = FMath::Clamp(PenetrationDepth / SafeFadeDistance, 0.f, 1.f);

		if (targetOpacity > 0.3f)
		{
			cameraOutWarning = FString(TEXT("Please return back to the borders of the Tracking Space.")).ToUpper();
			DrawMsg(cameraOutWarning);
		}
	}
	else
	{
		bCameraInAMesh = false;
	}

	CurrentCameraFadeOpacity = FMath::FInterpTo(CurrentCameraFadeOpacity, targetOpacity, DeltaTime, 10.f);
}

void AVRCharacter::ApplyCameraFade()
{
	if (!CameraFadeComponent || !FadeDynamicMaterial) return;

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

	FVector CurrentCameraPosition = CameraComponent->GetComponentLocation();
	CurrentVelocity = (CurrentCameraPosition - PrevCameraPosition) / DeltaTime;
	PrevCameraPosition = CurrentCameraPosition;
}

void AVRCharacter::UpdateCapsuleComponentPosition()
{
	if (!CameraComponent || !CapsuleCollisionComponent || bVirtControllerLocked) return;

	FVector CameraLoc = CameraComponent->GetRelativeLocation();
	float currentHalfHeight = CapsuleCollisionComponent->GetUnscaledCapsuleHalfHeight();
	FVector TargetCapsuleLoc(CameraLoc.X, CameraLoc.Y, currentHalfHeight);
	CapsuleCollisionComponent->SetRelativeLocation(TargetCapsuleLoc, true);
}

void AVRCharacter::ApplyRotationFromCameraToMesh(float DeltaTime)
{
	if (!CameraComponent || !SkeletalMeshComponent) return;

	float HeadYaw = CameraComponent->GetRelativeRotation().Yaw;
	float TargetMeshYaw = HeadYaw - 90.0f;
	FRotator CurrentRot = SkeletalMeshComponent->GetRelativeRotation();
	FRotator TargetRot = FRotator(0.f, TargetMeshYaw, 0.f);	
	FRotator SmoothedRot = FMath::RInterpTo(CurrentRot, TargetRot, DeltaTime, 10.0f);
	SkeletalMeshComponent->SetRelativeRotation(SmoothedRot);
}

#if WITH_EDITOR

void AVRCharacter::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	FName propName = PropertyChangedEvent.GetPropertyName();
	if (propName == NAME_None) return;
	if (propName == GET_MEMBER_NAME_CHECKED(AVRCharacter, DeadZoneRadius) 
		|| propName == GET_MEMBER_NAME_CHECKED(AVRCharacter, DeadZoneHeight))
	{
		ConfigureDecal(DeadZoneDecalComponent, DeadZoneHeight, DeadZoneRadius, true);
	}
	else if (propName == GET_MEMBER_NAME_CHECKED(AVRCharacter, ActiveZoneRadius)
		|| propName == GET_MEMBER_NAME_CHECKED(AVRCharacter, ActiveZoneHeight))
	{
		ConfigureDecal(ActiveZoneComponent, ActiveZoneHeight, ActiveZoneRadius, true);
	}
	else if (propName == GET_MEMBER_NAME_CHECKED(AVRCharacter, PlayerAnchorZoneRadius)
		|| propName == GET_MEMBER_NAME_CHECKED(AVRCharacter, PlayerAnchorZoneHeight))
	{
		ConfigureDecal(PlayerAnchorDecalComponent, PlayerAnchorZoneHeight, PlayerAnchorZoneRadius, true);
	}
}

#endif

void AVRCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	RecalibrateCapsuleAndMeshComponent();
	UpdateCapsuleComponentPosition();
	ApplyRotationFromCameraToMesh(DeltaTime);
	CalculateCurrentVelocity(DeltaTime);
	DetectGround();

	/*if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(1, 5, FColor::Blue, FString::Printf(TEXT("On Ground: %s"), bIsGrounded ? TEXT("True") : TEXT("False")));
	}*/

	ApplyGravity(DeltaTime);
	HandleMovement(DeltaTime);
	CheckCameraFade(DeltaTime);
	ApplyCameraFade();
}
