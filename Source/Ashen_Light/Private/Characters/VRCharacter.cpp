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
#include "../../Public/Animations/VRCharacterAnimInstance.h"
#include "../../Public/Enums.h"
#include "IXRTrackingSystem.h"


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

	bIsObstacleHit = false;

	CurrentCharacterState = EVRCharacterState::VRCS_Blocked;//Set this first to have an opportunity to recalibrate position
	CachedCharacterState = EVRCharacterState::VRCS_FreeRoam;//We need to switch to Free Roam when game starts
	bCanPerformBattleStep = false;
	bCanRun = false;

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

	//Initialize dynamic material instance to controll parameters during runtime
	ApplyMaterialToComponent(CameraFadeComponent, 0, FadeMaterialBase, FadeDynamicMaterial);
	ApplyMaterialToComponent(ActiveZoneComponent, ActiveZoneMaterialBase, ActiveZoneMaterialInstance);
	ApplyMaterialToComponent(DeadZoneDecalComponent, DeadZoneMaterialBase, DeadZoneMaterialInstance);
	ApplyMaterialToComponent(PlayerAnchorDecalComponent, VRSliderMaterialBase, VRSliderMaterialInstance);
	
	//Configure Decals
	ConfigureDecal(DeadZoneDecalComponent, DeadZoneHeight, DeadZoneRadius, true);
	ConfigureDecal(ActiveZoneComponent, ActiveZoneHeight, ActiveZoneRadius, true);
	ConfigureDecal(PlayerAnchorDecalComponent, PlayerAnchorZoneHeight, PlayerAnchorZoneRadius, true);

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

void AVRCharacter::ApplyMaterialToComponent(UPrimitiveComponent* comp, int32 matIndex, UMaterialInterface* materialBase, UMaterialInstanceDynamic*& dynamicInstance)
{
	if (!comp || !materialBase) return;
	if (matIndex < 0 || matIndex >= comp->GetNumMaterials()) return;
	dynamicInstance = CreateMaterialInstance(materialBase);
	if (!dynamicInstance) return;
	comp->SetMaterial(matIndex, dynamicInstance);
}

void AVRCharacter::ApplyMaterialToComponent(UDecalComponent* comp, UMaterialInterface* materialBase, UMaterialInstanceDynamic*& dynamicInstance)
{
	if (!comp || !materialBase) return;
	dynamicInstance = CreateMaterialInstance(materialBase);
	if (!dynamicInstance) return;	
	comp->SetDecalMaterial(dynamicInstance);
}

void AVRCharacter::SetDecalColors()
{
	switch (CurrentCharacterState)
	{
	case EVRCharacterState::VRCS_Blocked:
		SetDecalsColor(MovementLockedDecalColor);
		break;
	case EVRCharacterState::VRCS_FreeRoam:
		SetDecalsColor(FreeRoamDecalColor);
		break;
	case EVRCharacterState::VRCS_Battle:
		SetDecalsColor(BattleModeDecalColor);
		break;
	}
}

void AVRCharacter::SetDecalsColor(FLinearColor color)
{
	if (!VRSliderMaterialInstance) return;
	VRSliderMaterialInstance->SetVectorParameterValue(FName("Slider Color"), color);
	if (!DeadZoneMaterialInstance) return;
	DeadZoneMaterialInstance->SetVectorParameterValue(FName("Emisive Color"), color);
	if (!ActiveZoneMaterialInstance) return;
	ActiveZoneMaterialInstance->SetVectorParameterValue(FName("Emisive Color"), color);
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

UMaterialInstanceDynamic* AVRCharacter::CreateMaterialInstance(UMaterialInterface* interface)
{
	if (!interface) return nullptr;
	return UMaterialInstanceDynamic::Create(interface, this);
}

void AVRCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (!PlayerInputComponent) return;
	//Setup binding
	//Locomotion
	PlayerInputComponent->BindAction(FName("ToggleBattleMode"), IE_Released, this, &AVRCharacter::OnToggleBattleModePressed);
	PlayerInputComponent->BindAction(FName("ToggleRun"), IE_Pressed, this, &AVRCharacter::OnSprintPressed);
	PlayerInputComponent->BindAction(FName("ToggleRun"), IE_Released, this, &AVRCharacter::OnSprintReleased);
	//Grip Mechanics
	PlayerInputComponent->BindAction(FName("GrabRight"), IE_Pressed, this, &AVRCharacter::OnRightGrabButtonPressed);
	PlayerInputComponent->BindAction(FName("GrabRight"), IE_Released, this, &AVRCharacter::OnRightGrabButtonReleased);
	PlayerInputComponent->BindAction(FName("GrabLeft"), IE_Pressed, this, &AVRCharacter::OnLeftGrabButtonPressed);
	PlayerInputComponent->BindAction(FName("GrabLeft"), IE_Released, this, &AVRCharacter::OnLeftGrabButtonReleased);

	PlayerInputComponent->BindAction(FName("TriggerRight"), IE_Pressed, this, &AVRCharacter::OnRightTriggerButtonPressed);
	PlayerInputComponent->BindAction(FName("TriggerRight"), IE_Released, this, &AVRCharacter::OnRightTriggerButtonReleased);
	PlayerInputComponent->BindAction(FName("TriggerLeft"), IE_Pressed, this, &AVRCharacter::OnLeftTriggerButtonPressed);
	PlayerInputComponent->BindAction(FName("TriggerLeft"), IE_Released, this, &AVRCharacter::OnLeftTriggerButtonReleased);

	PlayerInputComponent->BindAction(FName("BlockMovement"), IE_Released, this, &AVRCharacter::OnBlockMovementPressed);
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

	SetDecalColors();
}

void AVRCharacter::OnSprintPressed()
{
	bCanRun = true;//Enable run
}

void AVRCharacter::OnSprintReleased()
{
	bCanRun = false;//Disable run
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

void AVRCharacter::OnBlockMovementPressed()
{
	if (CurrentCharacterState == EVRCharacterState::VRCS_Battle || CurrentCharacterState == EVRCharacterState::VRCS_FreeRoam)
	{
		CachedCharacterState = CurrentCharacterState;
		CurrentCharacterState = EVRCharacterState::VRCS_Blocked;
	}
	else if(CurrentCharacterState == EVRCharacterState::VRCS_Blocked)
	{
		CurrentCharacterState = CachedCharacterState;
	}

	CachedCharacterState = CurrentCharacterState;

	SetDecalColors();
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
		MeshLoc.Z = -halfHeight + (cameraHeadDelta / 2.f) + MeshOriginAdjustment;//Cause we resize mesh from the center
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
	if (CurrentCharacterState == EVRCharacterState::VRCS_Blocked) return;
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
				WorldDirVector = FVector::VectorPlaneProject(MovementDir, ImpactNormal);
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

	if (bIsGrounded)
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
	//Fade opacity interpolation
	CurrentCameraFadeOpacity = FMath::FInterpTo(CurrentCameraFadeOpacity, targetOpacity, DeltaTime, 10.f);
}

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
	CapsuleCollisionComponent->SetRelativeLocation(TargetCapsuleLoc + MeshOffset, false, nullptr, ETeleportType::TeleportPhysics);
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

bool AVRCharacter::IsCrouching(float* crouchDepth)
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

FString ConvertEnumToStr(EVRCharacterState state)
{
	switch (state)
	{
	case EVRCharacterState::VRCS_Blocked:
		return FString("Block");
		
	case EVRCharacterState::VRCS_FreeRoam:
		return FString("Free Roam");
	case EVRCharacterState::VRCS_Battle:
		return FString("Battle");
	}
	return FString();
}

void AVRCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	/*if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(1, 5.f, FColor::Red, FString::Printf(TEXT("Curr State: %s"), *ConvertEnumToStr(CurrentCharacterState)));
		GEngine->AddOnScreenDebugMessage(2, 5.f, FColor::Blue, FString::Printf(TEXT("Cached State: %s"), *ConvertEnumToStr(CachedCharacterState)));
	}*/
	CalculatePlayerHeight();
	//Controls Capsule Mesh Component Relations
	RecalibrateCapsuleAndMeshComponent();
	UpdateCapsuleComponentPosition();
	ApplyRotationFromCameraToCapsule(DeltaTime);
	//Physics Calculations
	CalculateCurrentVelocity(DeltaTime);
	DetectGround();
	ApplyGravity(DeltaTime);
	//Camera Fade
	CheckCameraFade(DeltaTime);
	ApplyCameraFade();
	//IK Calculations
	UVRCharacterAnimInstance* inst = GetCharAnimInstance();
	if (inst)
	{
		//We require additional sensors for Elbows or some Math 
		inst->CalculateElbowJointTarget(DeltaTime, false);
		inst->CalculateSpineRotation(DeltaTime, false);
	}
	
}
