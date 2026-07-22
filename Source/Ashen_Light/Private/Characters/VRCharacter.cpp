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
	//Location in a center of the Tracking Space
	TrackingSpaceOrigin = CreateDefaultSubobject<USceneComponent>(TEXT("Tracking Space Origin"));
	if (!TrackingSpaceOrigin) return;
	SetRootComponent(TrackingSpaceOrigin);
	//VR camera
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("VR Camera Component"));
	if (!CameraComponent) return;
	CameraComponent->SetupAttachment(TrackingSpaceOrigin);
	CameraComponent->bUsePawnControlRotation = false;
	CameraComponent->bLockToHmd = true;	
	//Camera Fade Component
	CameraFadeComponent = CreateDefaultSubobject<UStaticMeshComponent>("Camera Fade Component");
	if (!CameraFadeComponent) return;
	CameraFadeComponent->SetupAttachment(CameraComponent);
	CameraFadeComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CameraFadeComponent->SetHiddenInGame(true);
	CameraFadeComponent->SetCastShadow(false);
	//Capsule Collision Component
	CapsuleCollisionComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Physics Collision Capsule"));
	if (!CapsuleCollisionComponent) return;
	CapsuleCollisionComponent->SetupAttachment(CameraComponent);
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
	SkeletalMeshComponent->HideBoneByName(FName(HeadBoneName), EPhysBodyOp::PBO_None);

	//Right motion Controller
	RightMotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("Right Motion Controller"));
	if (!RightMotionController) return;
	RightMotionController->SetupAttachment(TrackingSpaceOrigin);
	RightMotionController->MotionSource = FName("Right");
	RightMotionController->bDisableLowLatencyUpdate = false;
	RightMotionController->bDisplayDeviceModel = true;
	RightMotionController->DisplayModelSource = FName("OculusHMD");;
	rightHandSpeed = 0.f;
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
	leftHandSpeed = 0.f;
	WidgetInteractionLeft = CreateDefaultSubobject<UWidgetInteractionComponent>(TEXT("Widget Interaction left"));
	if (!WidgetInteractionLeft) return;
	WidgetInteractionLeft->SetupAttachment(LeftMotionController);
	//Locomotion Decals
	//Dead Zone Decal
	DeadZoneDecalComponent = CreateDefaultSubobject<UDecalComponent>(TEXT("Dead Zone Decal Component"));
	if (!DeadZoneDecalComponent) return;	
	DeadZoneDecalComponent->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));
	DeadZoneDecalComponent->SetupAttachment(TrackingSpaceOrigin);
	//Active Zone Decal
	ActiveZoneComponent = CreateDefaultSubobject<UDecalComponent>(TEXT("Active Zone Decal Component"));
	if (!ActiveZoneComponent) return;	
	ActiveZoneComponent->SetupAttachment(TrackingSpaceOrigin);
	ActiveZoneComponent->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));
	//Player Anchor Component
	PlayerAnchorDecalComponent = CreateDefaultSubobject<UDecalComponent>(TEXT("Player Anchor Decal"));
	if (!PlayerAnchorDecalComponent) return;
	PlayerAnchorDecalComponent->SetupAttachment(CameraComponent);	
	PlayerAnchorDecalComponent->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));
	PlayerAnchorDecalComponent->SetUsingAbsoluteRotation(true);

	PawnMovement = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("Pawn Floating Movement"));

	bIsObstacleHit = false;

	CurrentCharacterState = EVRCharacterState::VRCS_FreeRoam;
	bCanPerformBattleStep = false;
	bCanRun = false;

	CurrentVelocity = FVector::ZeroVector;
	PrevCameraPosition = FVector::ZeroVector;

	bCameraInAMesh = false;
}

void AVRCharacter::BeginPlay()
{
	Super::BeginPlay();
	UHeadMountedDisplayFunctionLibrary::SetTrackingOrigin(EHMDTrackingOrigin::Floor);
	FCoreDelegates::VRHeadsetRecenter.AddUObject(this, &AVRCharacter::OnHMD_Recentered);
	FTimerHandle SpawnRecenterTimerHandle;

	GetWorldTimerManager().SetTimer(
		SpawnRecenterTimerHandle,
		this,
		&AVRCharacter::RecenterTrackingSpaceToActor,
		0.15f,
		false
	);

	if (FadeMaterialBase)
	{
		FadeDynamicMaterial = UMaterialInstanceDynamic::Create(FadeMaterialBase, this);
		if (CameraFadeComponent && FadeDynamicMaterial)
		{
			CameraFadeComponent->SetMaterial(0, FadeDynamicMaterial);
		}
	}

	ConfigureDecal(DeadZoneDecalComponent, DeadZoneHeight, DeadZoneRadius, true);	
	ConfigureDecal(ActiveZoneComponent, ActiveZoneHeight, ActiveZoneRadius, true);		
	ConfigureDecal(PlayerAnchorDecalComponent, PlayerAnchorZoneHeight, PlayerAnchorZoneRadius, true);

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
	FCoreDelegates::VRHeadsetRecenter.RemoveAll(this);
	Super::EndPlay(EndPlayReason);
}

void AVRCharacter::OnHMD_Recentered()
{
	if (CameraComponent)
	{
		PrevCameraPosition = CameraComponent->GetComponentLocation();
	}
	
	GetWorldTimerManager().SetTimerForNextTick(this, &AVRCharacter::RecalibrateCapsuleComponent);
}

void AVRCharacter::RecenterTrackingSpaceToLocation(FVector TargetWorldLocation)
{
	if (!TrackingSpaceOrigin || !CameraComponent) return;
	
	FVector CurrentCameraWorldLoc = CameraComponent->GetComponentLocation();
	
	FVector Offset2D = TargetWorldLocation - CurrentCameraWorldLoc;
	Offset2D.Z = 0.f;

	FVector NewOriginWorldLoc = TrackingSpaceOrigin->GetComponentLocation() + Offset2D;
	TrackingSpaceOrigin->SetWorldLocation(NewOriginWorldLoc);
	
	PrevCameraPosition = CameraComponent->GetComponentLocation();
	
	RecalibrateCapsuleComponent();
}

void AVRCharacter::RecenterTrackingSpaceToActor()
{
	RecenterTrackingSpaceToLocation(GetActorLocation());
}

float AVRCharacter::GetGroundVelocityRatio() const
{
	if (!PawnMovement || FMath::IsNearlyZero(runSpeed)) return 0.f;
	return FMath::Clamp(PawnMovement->Velocity.Size2D() / runSpeed, 0.f, 1.f);
}

void AVRCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (!PlayerInputComponent) return;

	PlayerInputComponent->BindAction(FName("ToggleBattleMode"), IE_Pressed, this, &AVRCharacter::OnToggleBattleModePressed);
	PlayerInputComponent->BindAction(FName("ToggleRun"), IE_Pressed, this, &AVRCharacter::OnSprintPressed);
	PlayerInputComponent->BindAction(FName("ToggleRun"), IE_Released, this, &AVRCharacter::OnSprintReleased);
}

void AVRCharacter::OnToggleBattleModePressed()
{
	if (CurrentCharacterState == EVRCharacterState::VRCS_FreeRoam)
	{
		CurrentCharacterState = EVRCharacterState::VRCS_Battle;
		bCanPerformBattleStep = true;
	}
	else
	{
		CurrentCharacterState = EVRCharacterState::VRCS_FreeRoam;
	}
}

void AVRCharacter::OnSprintPressed()
{
	bCanRun = true;
}

void AVRCharacter::OnSprintReleased()
{
	bCanRun = false;
}

void AVRCharacter::RecalibrateCapsuleComponent()
{
	if (!CapsuleCollisionComponent || !CameraComponent) return;

	float playerHeight = CameraComponent->GetRelativeLocation().Z;

	if (playerHeight < 40.f)
		playerHeight = 40.f;

	float halfHeight = playerHeight / 2.f;	
	float currRadius = CapsuleCollisionComponent->GetScaledCapsuleRadius();
	currRadius = FMath::Min(currRadius, halfHeight - 1.0f);
	CapsuleCollisionComponent->SetCapsuleSize(currRadius, halfHeight);
	CapsuleCollisionComponent->SetRelativeLocation(FVector(0.f, 0.f, -halfHeight));
}

void AVRCharacter::ConfigureDecal(UDecalComponent* decal, float thicknes, float r, bool redraw)
{
	if (!decal) return;

	decal->DecalSize = FVector(thicknes, r, r);

	if(redraw)
		decal->MarkRenderStateDirty();
}

void AVRCharacter::HandleMovement(float DeltaTime)
{
	RecalibrateCapsuleComponent();

	//1 Get VR Device coordinates in TrackSpace relative to its origin
	if (!CameraComponent) return;
	FVector CameraInTrackSpace = CameraComponent->GetRelativeLocation();
	FVector CameraInTrackSpace2D(CameraInTrackSpace.X, CameraInTrackSpace.Y, 0.f);
	float Dist2D = CameraInTrackSpace2D.Size();
	if (Dist2D > DeadZoneRadius)
	{
		FVector WorldDirVector = ActorToWorld().TransformVectorNoScale(CameraInTrackSpace2D);

		CheckObstacles();

		if (bIsObstacleHit)//We hit obstacle
		{		
			FVector ImpactNormal = CurrentObstacleHit.ImpactNormal;
			FVector MovementDir = WorldDirVector.GetSafeNormal();

			// 1. Check hit angle
			//  < 0  : Move in the obstacle direction
			// -1.0f : Perpendiular Hit to the wall
			//  0.0f : Sliding across the wall
			float Dot = FVector::DotProduct(MovementDir, ImpactNormal);
			float DotThreshold = -FMath::Cos(FMath::DegreesToRadians(StopMovementAngleThreshold));
			// 45 deg
			if (Dot <= DotThreshold)
			{
				//90 deg, Stop movement
				WorldDirVector = FVector::ZeroVector;
			}
			else
			{
				//Calculate vector along the Static Mesh to slide across
				WorldDirVector = FVector::VectorPlaneProject(WorldDirVector, ImpactNormal);
			}
		}

		if (!WorldDirVector.IsNearlyZero())
		{
			if (IsSwiningArms(DeltaTime) && bCanRun)
			{
				Run();
			}
			else
			{
				Walk();
			}

			FVector SlopeAdjustedWorldDirVector = AdjustInputForSlope(WorldDirVector);

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
	float rightContrVelocity = GetMotionControllerSpeed(RightMotionController, &prevRightHandLocation, DeltaTime);
	float leftContrVelocity = GetMotionControllerSpeed(LeftMotionController, &prevLeftHandLocation, DeltaTime);
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(1, 3, FColor::Orange, FString::Printf(TEXT("Right MC Vel: %f"), rightContrVelocity));
		GEngine->AddOnScreenDebugMessage(1, 3, FColor::Orange, FString::Printf(TEXT("Left MC Vel: %f"), leftContrVelocity));
	}
	if (rightContrVelocity > SwiningThreshold && leftContrVelocity > SwiningThreshold)
		return true;
	return false;
}

float AVRCharacter::GetMotionControllerSpeed(UMotionControllerComponent* motionController, FVector* prevMotionControllerPosition, float DeltaTime)
{
	float res = -1.f;
	if (!motionController || DeltaTime == 0 || !prevMotionControllerPosition) return res;
	const FVector MotionControllerLocationInVROriginSpace = motionController->GetRelativeLocation();
	res = ((MotionControllerLocationInVROriginSpace - *prevMotionControllerPosition).Size()) / DeltaTime;
	*prevMotionControllerPosition = MotionControllerLocationInVROriginSpace;
	return res;
}

void AVRCharacter::Stop()
{	
	if (!PawnMovement) return;
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

	const FVector Start = CapsuleCollisionComponent->GetComponentLocation();
	const float HalfHeight = CapsuleCollisionComponent->GetScaledCapsuleHalfHeight();
	const float SphereRadius = 20.f;
	const FVector End = Start - FVector(0.f, 0.f, HalfHeight - SphereRadius + GroundDetectionThreshold);

	TArray<AActor*> IgnoreActors;
	IgnoreActors.Add(this);
	FHitResult HitResult;	
	bool bHit = UKismetSystemLibrary::SphereTraceSingle(
		this,
		Start,
		End,
		SphereRadius,
		UEngineTypes::ConvertToTraceType(ECC_WorldStatic),
		false, 
		IgnoreActors,
		EDrawDebugTrace::None,
		HitResult,
		true,
		FLinearColor::Red,
		FLinearColor::Green,
		1.f
	);
	
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
	if (bCameraInAMesh)
	{
		VerticalVelocity = 0.f;
		return;
	}

	if (bIsGrounded)
	{
		VerticalVelocity = -10.f;
	}
	else
	{
		//Accumulate fall speed
		VerticalVelocity += GravityConstant * DeltaTime;
		VerticalVelocity = FMath::Clamp(VerticalVelocity, TerminalVelocity, MaxFallVelocity);

		FVector GravityDirection = FVector::UpVector;
		AddMovementInput(GravityDirection, VerticalVelocity * DeltaTime);
	}
}

FVector AVRCharacter::AdjustInputForSlope(FVector InputVector) const
{
	if (!bIsGrounded)
	{
		return InputVector;
	}
	
	return FVector::VectorPlaneProject(InputVector, CurrentGroundHit.ImpactNormal).GetSafeNormal() * InputVector.Size();
}

void AVRCharacter::CheckObstacles()
{
	if (!CameraComponent) return;

	if (CurrentVelocity.IsNearlyZero())
	{
		bIsObstacleHit = false;
		return;
	}

	FVector start = CameraComponent->GetComponentLocation();
	FVector end = start + CurrentVelocity.GetSafeNormal() * ObstacleDistanceDetection;
	TArray<AActor*> actorsToIgnore;
	actorsToIgnore.Add(this);
	FHitResult hit;
	const float headRadius = 15.f;
	bool bHit = UKismetSystemLibrary::SphereTraceSingle(this, 
		start,
		end,
		headRadius,
		UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_WorldStatic),
		false, actorsToIgnore, EDrawDebugTrace::ForDuration, hit, true, 
		FLinearColor::Red, FLinearColor::Green, 1.f);

	if (bHit && hit.bBlockingHit)
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

	const FVector startEnd = CameraComponent->GetComponentLocation();	
	TArray<AActor*> actorsToIgnore;
	actorsToIgnore.Add(this);
	FHitResult hitResult;
	bool bHit = UKismetSystemLibrary::SphereTraceSingle(
	this, startEnd, startEnd, FadeCheckRadius, 
		UEngineTypes::ConvertToTraceType(ECC_WorldStatic), false, actorsToIgnore, EDrawDebugTrace::None, hitResult, true);

	float targetOpacity = 0.f;
	FString camraOuWarning = TEXT("");
	if (bHit && hitResult.bBlockingHit)
	{
		bCameraInAMesh = true;

		FVector ImactToCamera = CameraComponent->GetComponentLocation() - hitResult.ImpactPoint;
		float PenetrationDepth = FadeCheckRadius - ImactToCamera.Size();
		float SafeFadeDistance = (CameraFadeDistance > 0.1f) ? CameraFadeDistance : 5.0f;
		targetOpacity = FMath::Clamp(PenetrationDepth / SafeFadeDistance, 0.f, 1.f);

		if (targetOpacity > 0.3f)
		{
			camraOuWarning = FString(TEXT("Please return back to the borders of the Tracking Space.")).ToUpper();
			DrawMsg(camraOuWarning);
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
