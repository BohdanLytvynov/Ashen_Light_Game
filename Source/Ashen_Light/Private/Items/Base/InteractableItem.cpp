// Fill out your copyright notice in the Description page of Project Settings.

#include "Items/Base/InteractableItem.h"
#include "Components/Base/StateManagerComponent.h"
#include "Components/Base/StateBlackboard.h"
#include "Components/Items/Interaction/IdleInteractionState.h"
#include "Components/Items/Interaction/HoverInteractionState.h"
#include "Components/Items/Interaction/PullingInteractionState.h"
#include "Components/Items/Interaction/GrabItemInteractionState.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Subsystems/VRInteractionSubsystem.h"
#include "Components/ShapeComponent.h"
#include "Components/Items/Physics/DisablePhysicsItemState.h"
#include "Components/Items/Physics/KinematicItemState.h"
#include "Components/Items/Physics/SimPhysicsItemState.h"
#include "Components/Base/VelocitySensor.h"
#include "Engine/StaticMeshSocket.h"
#include "Engine/SkeletalMeshSocket.h"

AInteractableItem::AInteractableItem(const FObjectInitializer& init) : Super(init)
{
	PrimaryActorTick.bCanEverTick = true;
	// Create shared state blackboard
	GlobalBlackboard = CreateDefaultSubobject<UStateBlackboard>(TEXT("Global State Blackboard"));
	LifeCycleStateManger = CreateDefaultSubobject<UStateManagerComponent>(TEXT("Life Cycle Manager Component1"));
	InteractiveStateManager = CreateDefaultSubobject<UStateManagerComponent>(TEXT("Interaction Manager Component1"));

	// Initialize and register interactive state machine
	if (InteractiveStateManager)
	{
		IdleInteractionState = CreateDefaultSubobject<UIdleInteractionState>(TEXT("Idle Interaction State"));
		if (IdleInteractionState)
		{
			IdleInteractionState->InitializeState(EItemInteractiveState::EIIS_Idle, this, InteractiveStateManager);
		}

		HoverInteractionstate = CreateDefaultSubobject<UHoverInteractionState>(TEXT("Hover Interaction State"));
		if (HoverInteractionstate)
		{
			HoverInteractionstate->InitializeState(EItemInteractiveState::EIIS_Hovered, this, InteractiveStateManager);
		}

		PullingInteractionState = CreateDefaultSubobject<UPullingInteractionState>(TEXT("Pulling Interaction State"));
		if (PullingInteractionState)
		{
			PullingInteractionState->InitializeState(EItemInteractiveState::EIIS_Pulling, this, InteractiveStateManager);
		}

		GrabItemInteractionState = CreateDefaultSubobject<UGrabItemInteractionState>(TEXT("Grab Interaction State"));
		if (GrabItemInteractionState)
		{
			GrabItemInteractionState->InitializeState(EItemInteractiveState::EIIS_Grab, this, InteractiveStateManager);
		}

		InteractiveStateManager->RegisterState(IdleInteractionState);
		InteractiveStateManager->RegisterState(HoverInteractionstate);
		InteractiveStateManager->RegisterState(PullingInteractionState);
		InteractiveStateManager->RegisterState(GrabItemInteractionState);
		InteractiveStateManager->AddGlobalBlackBoard(GlobalBlackboard);

		// Configure allowed transitions between interactive states
		InteractiveStateManager->BuildStateMatrix(4);
		InteractiveStateManager->ConfigureStateMatrix([](FRectMatrix<bool>* mat)
			{
				mat->Set(true, EItemInteractiveState::EIIS_Idle, EItemInteractiveState::EIIS_Hovered);
				mat->Set(true, EItemInteractiveState::EIIS_Hovered, EItemInteractiveState::EIIS_Idle);
				mat->Set(true, EItemInteractiveState::EIIS_Hovered, EItemInteractiveState::EIIS_Pulling);
				mat->Set(true, EItemInteractiveState::EIIS_Pulling, EItemInteractiveState::EIIS_Idle);
				mat->Set(true, EItemInteractiveState::EIIS_Hovered, EItemInteractiveState::EIIS_Grab);
				mat->Set(true, EItemInteractiveState::EIIS_Grab, EItemInteractiveState::EIIS_Idle);
			});
		InteractiveStateManager->SwitchState(EItemInteractiveState::EIIS_Idle);
	}

	// Initialize and register physics state machine
	PhysicsStateManager = CreateDefaultSubobject<UStateManagerComponent>(TEXT("Physics State Manager1"));
	if (PhysicsStateManager)
	{
		DisablePhysicsState = CreateDefaultSubobject<UDisablePhysicsItemState>(TEXT("Disable Physics State"));
		if (DisablePhysicsState)
		{
			DisablePhysicsState->InitializeState(EItemPhysicsState::EIGS_DisablePhysics, this, PhysicsStateManager);
		}

		KinematicItemState = CreateDefaultSubobject<UKinematicItemState>(TEXT("Kinematic Item State"));
		if (KinematicItemState)
		{
			KinematicItemState->InitializeState(EItemPhysicsState::EIGS_Kinematic, this, PhysicsStateManager);
		}

		SimPhysicsState = CreateDefaultSubobject<USimPhysicsItemState>(TEXT("Simulate Physics State"));
		if (SimPhysicsState)
		{
			SimPhysicsState->InitializeState(EItemPhysicsState::EIGS_SimulatePhysics, this, PhysicsStateManager);
		}

		PhysicsStateManager->RegisterState(DisablePhysicsState);
		PhysicsStateManager->RegisterState(KinematicItemState);
		PhysicsStateManager->RegisterState(SimPhysicsState);
		PhysicsStateManager->AddGlobalBlackBoard(GlobalBlackboard);
		PhysicsStateManager->SwitchState(EItemPhysicsState::EIGS_DisablePhysics);
	}

	//Sensors
	VelocitySensor = CreateDefaultSubobject<UVelocitySensor>(TEXT("Velocity Sensor"));
}

void AInteractableItem::BeginPlay()
{
	Super::BeginPlay();
			
	if (VelocitySensor)
	{
		VelocitySensor->ConfigureSpace(ERelativeTransformSpace::RTS_World);
		VelocitySensor->SetTrackingComponent(GetRootComponent());
		VelocitySensor->SyncPosition();
	}

	// Create dynamic material instance for glow effect if a base material is provided
	if (GlowMaterialBase)
	{
		GlowMaterialInstance = UMaterialInstanceDynamic::Create(GlowMaterialBase, this);
		if (GlowMaterialInstance && GlowMesh)
		{
			GlowMesh->SetMaterial(0, GlowMaterialInstance);
			GlowMaterialInstance->SetScalarParameterValue("Opacity", 0.f);
		}
	}

	if (InteractiveStateManager)
	{
		InteractiveStateManager->BeginPlay();
		InteractiveStateManager->SetEnumToStrConverter([](uint8 e) -> FString
				{
					EItemInteractiveState is = static_cast<EItemInteractiveState>(e);
					switch (is)
					{
					case EItemInteractiveState::EIIS_Idle:
						return FString("Idle");
					case EItemInteractiveState::EIIS_Hovered:
						return FString("Hovered");
					case EItemInteractiveState::EIIS_Pulling:
						return FString("Pulling");
					case EItemInteractiveState::EIIS_Grab:
						return FString("Grab");
					}
					return FString("");
				});
	}
	if (PhysicsStateManager)
	{
		PhysicsStateManager->BeginPlay();
		PhysicsStateManager->SetEnumToStrConverter([](uint8 e) -> FString
				{
					EItemPhysicsState ps = static_cast<EItemPhysicsState>(e);
					switch (ps)
					{
					case EItemPhysicsState::EIGS_SimulatePhysics:
						return FString("Simulate Physics");
					case EItemPhysicsState::EIGS_Kinematic:
						return FString("Kinematic");
					case EItemPhysicsState::EIGS_DisablePhysics:
						return FString("Disable Physics");
					}
					return FString("");
				});
	}
	if (LifeCycleStateManger)
	{
		LifeCycleStateManger->BeginPlay();
	}
}

void AInteractableItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (VelocitySensor)
	{
		VelocitySensor->DoScan(DeltaTime);
	}

	if (InteractiveStateManager)
	{
		InteractiveStateManager->OnTick(DeltaTime);
	}

	if (PhysicsStateManager)
	{
		PhysicsStateManager->OnTick(DeltaTime);
	}

	if (LifeCycleStateManger)
	{
		LifeCycleStateManger->OnTick(DeltaTime);
	}
}

void AInteractableItem::OnHoverCmd_Implementation(USceneComponent* hoverReasonComponent, EVRControllerHand hand, APawn* hoverReasonActor)
{
	m_OwningActor = hoverReasonActor;
	if (!InteractiveStateManager) return;

	const EItemInteractiveState currItemInterState = InteractiveStateManager->GetCurrentState<EItemInteractiveState>();

	// 1. If the item is already grabbed or pulled, assign the secondary hand for two-handed interactions
	if (currItemInterState == EItemInteractiveState::EIIS_Grab ||
		currItemInterState == EItemInteractiveState::EIIS_Pulling)
	{
		if (hand != m_PrimaryHand && m_SecondaryHand == EVRControllerHand::None)
		{
			m_SecondaryHand = hand;
			m_hoverReasonSecondaryComponent = hoverReasonComponent;
		}
		return;
	}

	// 2. If already in Hovered state, refresh the watchdog timer and handle secondary hover target
	if (currItemInterState == EItemInteractiveState::EIIS_Hovered)
	{
		if (hand != m_PrimaryHand && m_SecondaryHand == EVRControllerHand::None)
		{
			m_SecondaryHand = hand;
			m_hoverReasonSecondaryComponent = hoverReasonComponent;
		}

		// Keep resetting the timer every frame as long as hover events keep coming
		StartHoverTimer();
		return;
	}

	// 3. Initial transition to Hovered state (e.g. from Idle), hover for the first time
	if (!m_hoverReasonPrimaryComponent.IsValid())
	{
		m_hoverReasonPrimaryComponent = hoverReasonComponent;
		m_PrimaryHand = hand;

		InteractiveStateManager->SwitchState(EItemInteractiveState::EIIS_Hovered);
		StartHoverTimer();
	}
}

bool AInteractableItem::HasCapability(uint8 capability) const
{
	return ItemCapabilities.Contains(static_cast<EItemCapabilities>(capability));
}

UMeshComponent* AInteractableItem::GetGlowMesh() const
{
	return GlowMesh;
}

UMeshComponent* AInteractableItem::GetMesh() const
{
	return ItemMesh;
}

UMaterialInstanceDynamic* AInteractableItem::GetGlowMaterialInstance() const
{
	return GlowMaterialInstance;
}

void AInteractableItem::GetVRControllerButtonState(EVRControllerHand hand, FVRControllerButtonsState& state)
{
	UVRInteractionSubsystem* vrInterSubsys = GetVRInterSubsystem();
	if (!vrInterSubsys || !m_OwningActor.IsValid()) return;

	vrInterSubsys->GetVRControllerButtonsState(m_OwningActor.Get(), hand, state);
}

EVRControllerHand AInteractableItem::GetVRHand(bool primary) const
{
	return primary ? m_PrimaryHand : m_SecondaryHand;
}

USceneComponent* AInteractableItem::GetVRHoverReason(bool primary) const
{
	return primary ? m_hoverReasonPrimaryComponent.Get() : m_hoverReasonSecondaryComponent.Get();
}

FTransform AInteractableItem::GetActorTransform()
{
	return Super::GetActorTransform();
}

void AInteractableItem::SetNewActorLocation(const FVector& worldLocation, bool sweep, FHitResult* outHit, ETeleportType teleType)
{
	SetActorLocation(worldLocation, sweep, outHit, teleType);
}

void AInteractableItem::SetNewActorTransform(const FTransform& newTransform, bool sweep, FHitResult* outHit, ETeleportType teleType)
{
	SetActorTransform(newTransform, sweep, outHit, teleType);
}

void AInteractableItem::StartHoverTimer()
{
	if (UWorld* w = GetWorld())
	{
		w->GetTimerManager().SetTimer(
			m_HoverTimer,
			this,
			&AInteractableItem::OnHoverTimerElapsed,
			HoverDuration,
			false
		);
	}
}

void AInteractableItem::OnHoverTimerElapsed()
{
	if (!InteractiveStateManager) return;

	const EItemInteractiveState itemInterState = InteractiveStateManager->GetCurrentState<EItemInteractiveState>();
	if (itemInterState == EItemInteractiveState::EIIS_Hovered)
	{
		InteractiveStateManager->SwitchState(EItemInteractiveState::EIIS_Idle);
	}

	if (UWorld* w = GetWorld())
	{
		w->GetTimerManager().ClearTimer(m_HoverTimer);
	}
}

UVRInteractionSubsystem* AInteractableItem::GetVRInterSubsystem()
{
	if (!m_VRSub)
	{
		if (UWorld* w = GetWorld())
		{
			m_VRSub = w->GetSubsystem<UVRInteractionSubsystem>();
		}
	}
	return m_VRSub;
}

void AInteractableItem::ResetState()
{
	m_hoverReasonPrimaryComponent = nullptr;
	m_hoverReasonSecondaryComponent = nullptr;
	m_OwningActor = nullptr;
	m_PrimaryHand = EVRControllerHand::None;
	m_SecondaryHand = EVRControllerHand::None;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(m_HoverTimer);
	}
}

void AInteractableItem::ResetStateForHand(bool primary)
{
	if (primary)
	{
		m_PrimaryHand = EVRControllerHand::None;
		m_hoverReasonPrimaryComponent = nullptr;
	}
	else
	{
		m_SecondaryHand = EVRControllerHand::None;
		m_hoverReasonSecondaryComponent = nullptr;
	}
}

UShapeComponent* AInteractableItem::GetCollsiionShape() const
{
	return CollisionShape;
}

void AInteractableItem::SwitchPhysicsManager(EItemPhysicsState physicsState)
{
	if (PhysicsStateManager)
	{
		PhysicsStateManager->SwitchState(physicsState);
	}
}

FVector AInteractableItem::GetLinearVelocity() const
{
	if (PhysicsStateManager &&
		PhysicsStateManager->GetCurrentState<EItemPhysicsState>() == EItemPhysicsState::EIGS_SimulatePhysics &&
		CollisionShape)
	{
		return CollisionShape->GetPhysicsLinearVelocity();
	}

	if (VelocitySensor)
	{
		return VelocitySensor->GetVelocity();
	}

	return FVector::ZeroVector;
}

void AInteractableItem::PromoteSecondaryToPrimary()
{
	if (m_SecondaryHand == EVRControllerHand::None) return;

	m_PrimaryHand = m_SecondaryHand;
	m_SecondaryHand = EVRControllerHand::None;
	m_hoverReasonPrimaryComponent = m_hoverReasonSecondaryComponent;
	m_hoverReasonSecondaryComponent = nullptr;
}

APawn* AInteractableItem::GetItemOwner() const
{
	return m_OwningActor.Get();
}

bool AInteractableItem::TryGetItemGrabSocketTransform(bool primary, FTransform& outTransform)
{
	const FName socketName = primary ? PrimaryGrabSocket : SecondaryGrabSocket;
	if (socketName.IsNone() || !ItemMesh) return false;

	if (ItemMesh->DoesSocketExist(socketName))
	{
		outTransform = ItemMesh->GetSocketTransform(socketName, ERelativeTransformSpace::RTS_World);
		return true;
	}

	return false;
}

FName AInteractableItem::GetItemSocketName(bool primary)
{
	if (primary)
	{
		return PrimaryGrabSocket;
	}
	else
	{
		return SecondaryGrabSocket;
	}
	return NAME_None;
}

void AInteractableItem::DisableCollision(UPrimitiveComponent* comp)
{
	if (!comp) return;
	comp->SetSimulatePhysics(false);
	comp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	comp->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
}

void AInteractableItem::ConfigureRoot()
{
	if (!CollisionShape) return;
	CollisionShape->SetSimulatePhysics(false);
	CollisionShape->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionShape->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionShape->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
}

void AInteractableItem::UpdateRoot()
{
	if (CollisionShape)
	{
		ConfigureRoot();
		SetRootComponent(CollisionShape);
	}
	if (ItemMesh)
	{
		ItemMesh->SetupAttachment(CollisionShape);
	}
	if (GlowMesh && ItemMesh)
	{
		GlowMesh->SetupAttachment(ItemMesh);
	}
}

