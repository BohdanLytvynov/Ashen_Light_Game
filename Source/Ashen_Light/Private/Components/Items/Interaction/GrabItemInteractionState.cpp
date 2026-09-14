// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/Items/Interaction/GrabItemInteractionState.h"
#include "Components/Base/StateManagerComponent.h"
#include "Dto/FVRControllerButtonsState.h"
#include "Subsystems/VRInteractionSubsystem.h"
#include "Enums.h"

UGrabItemInteractionState::UGrabItemInteractionState(const FObjectInitializer& init) : Super(init)
{
}

void UGrabItemInteractionState::OnStateEnter()
{
	IInteractable* inter = GetContext();
	if (!inter) return;

	inter->SwitchPhysicsManager(EItemPhysicsState::EIGS_Kinematic);

	const EVRControllerHand primaryHand = inter->GetVRHand(true);
	const EVRControllerHand secondaryHand = inter->GetVRHand(false);

	m_OwningActor = inter->GetItemOwner();
	m_OrigItemScale = inter->GetActorTransform().GetScale3D();

	UVRInteractionSubsystem* vrInter = inter->GetVRInterSubsystem();
	if (vrInter)
	{
		m_RightHandGrabSocketName = vrInter->GetGrabSocketName(true);
		m_LeftHandGrabSocketName = vrInter->GetGrabSocketName(false);
	}

	if (m_OwningActor)
	{
		m_ownersSkelMesh = m_OwningActor->FindComponentByClass<USkeletalMeshComponent>();
	}

	// 1. Initialize Primary Grip
	if (primaryHand != EVRControllerHand::None)
	{
		m_IsGrabbed = true;
		TryAttachBySocket(inter, primaryHand, true);
	}

	// 2. Initialize Secondary Grip (if both hands are grabbing upon state entry)
	if (secondaryHand != EVRControllerHand::None && secondaryHand != primaryHand)
	{
		m_IsTwoHanded = true;
		TryAttachBySocket(inter, secondaryHand, false);
	}
	else
	{
		m_IsTwoHanded = false;
	}
}

void UGrabItemInteractionState::OnStateTick(float DeltaTime)
{
	Super::OnStateTick(DeltaTime);

	UStateManagerComponent* stateManager = GetStateManager();
	IInteractable* inter = GetContext();
	if (!stateManager || !inter) return;

	const EVRControllerHand primHand = inter->GetVRHand(true);
	const EVRControllerHand secHand = inter->GetVRHand(false);

	// 1. Verify primary hand input state
	bool bIsPrimaryStillGrabbing = false;
	if (primHand != EVRControllerHand::None)
	{
		FVRControllerButtonsState primButtons;
		inter->GetVRControllerButtonState(primHand, primButtons);
		bIsPrimaryStillGrabbing = primButtons.bGripPressed;
	}

	// 2. Verify secondary hand input state
	bool bIsSecondaryStillGrabbing = false;
	if (secHand != EVRControllerHand::None)
	{
		FVRControllerButtonsState secButtons;
		inter->GetVRControllerButtonState(secHand, secButtons);
		bIsSecondaryStillGrabbing = secButtons.bGripPressed;
	}

	if (!CacheItemScale)
	{
		m_OrigItemScale = inter->GetActorTransform().GetScale3D();
	}

	// 3. Handle hand release logic, Hand Swapping, and dynamically attaching/detaching Secondary Grip
	if (!bIsPrimaryStillGrabbing)
	{
		if (bIsSecondaryStillGrabbing)
		{
			// Promote the secondary hand to become the new primary hand
			inter->PromoteSecondaryToPrimary();

			// Fetch updated primary hand after promotion
			const EVRControllerHand newPrimHand = inter->GetVRHand(true);

			// Recalculate m_PrimaryGripRelativeTransform for the new primary hand
			TryAttachBySocket(inter, newPrimHand, true);
			m_IsTwoHanded = false;
		}
		else
		{
			// Both hands released -> transition back to Idle state
			m_IsGrabbed = false;
			m_IsTwoHanded = false;
			stateManager->SwitchState(EItemInteractiveState::EIIS_Idle);
			return;
		}
	}
	else
	{
		// Primary hand is actively holding the item
		if (secHand != EVRControllerHand::None && bIsSecondaryStillGrabbing)
		{
			// Secondary hand is gripping: dynamically attach secondary socket if not attached yet
			if (!m_IsTwoHanded)
			{
				m_IsTwoHanded = true;
				TryAttachBySocket(inter, secHand, false);
			}
		}
		else
		{
			// Secondary hand released or not gripping
			if (m_IsTwoHanded)
			{
				m_IsTwoHanded = false;
				inter->ResetStateForHand(false);
			}
		}
	}

	// 4. Update item spatial transform for the current frame (Tick)
	const EVRControllerHand activePrimHand = inter->GetVRHand(true);
	const EVRControllerHand activeSecHand = inter->GetVRHand(false);

	if (activePrimHand == EVRControllerHand::None) return;

	if (m_IsTwoHanded && activeSecHand != EVRControllerHand::None && activeSecHand != activePrimHand)
	{
		// Two-handed grip update (Position + Aim Vector alignment)
		UpdateTwoHandedTransform(inter, activePrimHand, activeSecHand, DeltaTime);
	}
	else
	{
		// Single-handed grip update (Direct matrix composition)
		UpdateSingleHandTransform(inter, activePrimHand);
	}
}

bool UGrabItemInteractionState::TryAttachBySocket(IInteractable* inter, EVRControllerHand hand, bool primary)
{
	if (!m_OwningActor) return false;

	USkeletalMeshComponent* ownersSkelMesh = m_OwningActor->FindComponentByClass<USkeletalMeshComponent>();
	if (!ownersSkelMesh) return false;

	const FName handSocketName = (hand == EVRControllerHand::Right) ? m_RightHandGrabSocketName : m_LeftHandGrabSocketName;
	if (handSocketName.IsNone() || !ownersSkelMesh->DoesSocketExist(handSocketName))
	{
		return false;
	}

	const FTransform HandSocketWorld = ownersSkelMesh->GetSocketTransform(handSocketName, ERelativeTransformSpace::RTS_World);
	return TryAttachBySocket_Internal(inter, HandSocketWorld, primary);
}

void UGrabItemInteractionState::UpdateSingleHandTransform(IInteractable* inter, EVRControllerHand primaryHand)
{
	if (!m_OwningActor || !inter) return;

	const FName primarySocketName = (primaryHand == EVRControllerHand::Right) ? m_RightHandGrabSocketName : m_LeftHandGrabSocketName;

	USkeletalMeshComponent* ownersSkelMesh = m_OwningActor->FindComponentByClass<USkeletalMeshComponent>();
	if (!ownersSkelMesh) return;

	const FTransform PrimaryHandWorld = ownersSkelMesh->GetSocketTransform(primarySocketName, ERelativeTransformSpace::RTS_World);
	FTransform NewRootWorld = m_PrimaryGripRelativeTransform * PrimaryHandWorld;

	NewRootWorld.SetScale3D(m_OrigItemScale);
	inter->SetNewActorTransform(NewRootWorld, false, nullptr, ETeleportType::TeleportPhysics);
}

void UGrabItemInteractionState::UpdateTwoHandedTransform(IInteractable* inter, EVRControllerHand primaryHand, EVRControllerHand secondaryHand, float DeltaTime)
{
	if (!m_OwningActor || !inter) return;

	const FName primarySocketName = (primaryHand == EVRControllerHand::Right) ? m_RightHandGrabSocketName : m_LeftHandGrabSocketName;
	const FName secondarySocketName = (secondaryHand == EVRControllerHand::Right) ? m_RightHandGrabSocketName : m_LeftHandGrabSocketName;

	USkeletalMeshComponent* ownersSkelMesh = m_OwningActor->FindComponentByClass<USkeletalMeshComponent>();
	if (!ownersSkelMesh) return;

	// 1. Fetch world transforms for both hand (controller) sockets
	const FTransform PrimaryHandWorld = ownersSkelMesh->GetSocketTransform(primarySocketName, ERelativeTransformSpace::RTS_World);
	const FTransform SecondaryHandWorld = ownersSkelMesh->GetSocketTransform(secondarySocketName, ERelativeTransformSpace::RTS_World);

	// 2. Compute baseline item transform as if held by the primary hand only
	FTransform BaseRootWorld = m_PrimaryGripRelativeTransform * PrimaryHandWorld;
	BaseRootWorld.SetScale3D(FVector::OneVector);

	// 3. Retrieve local grab socket transforms on the item itself
	UMeshComponent* ItemMesh = inter->GetMesh();
	if (!ItemMesh) return;

	const FName primItemSocketName = inter->GetItemSocketName(true);
	const FName secItemSocketName = inter->GetItemSocketName(false);

	if (primItemSocketName.IsNone() || secItemSocketName.IsNone() ||
		!ItemMesh->DoesSocketExist(primItemSocketName) || !ItemMesh->DoesSocketExist(secItemSocketName))
	{
		BaseRootWorld.SetScale3D(m_OrigItemScale);
		inter->SetNewActorTransform(BaseRootWorld, false, nullptr, ETeleportType::TeleportPhysics);
		return;
	}

	// Fetch world Item Socket's transforms
	const FTransform PrimItemSocketWorld = ItemMesh->GetSocketTransform(primItemSocketName, ERelativeTransformSpace::RTS_World);
	const FTransform SecItemSocketWorld = ItemMesh->GetSocketTransform(secItemSocketName, ERelativeTransformSpace::RTS_World);
	//Get true root (Physical Collsiion Component)
	USceneComponent* itemRoot = ItemMesh->GetAttachmentRoot();
	if (!itemRoot)
	{
		itemRoot = ItemMesh;
	}
	const FTransform ItemRootTransform = itemRoot->GetComponentTransform();
	//Primary and Secondary Item Socket's Transforms relative to the Item's Root Component
	const FTransform PrimItemSocketLocal = PrimItemSocketWorld.GetRelativeTransform(ItemRootTransform);
	const FTransform SecItemSocketLocal = SecItemSocketWorld.GetRelativeTransform(ItemRootTransform);

	// Calculate expected world locations of item sockets based on BaseRootWorld
	const FTransform ExpectedPrimItemSocketWorld = PrimItemSocketLocal * BaseRootWorld;
	const FTransform ExpectedSecItemSocketWorld = SecItemSocketLocal * BaseRootWorld;

	// 4. Calculate aim vectors (Current item alignment vs Target hand-to-hand direction)
	const FVector CurrentItemAimVector = (ExpectedSecItemSocketWorld.GetLocation() - ExpectedPrimItemSocketWorld.GetLocation()).GetSafeNormal();
	const FVector TargetHandAimVector = (SecondaryHandWorld.GetLocation() - PrimaryHandWorld.GetLocation()).GetSafeNormal();

	if (CurrentItemAimVector.IsNearlyZero() || TargetHandAimVector.IsNearlyZero()) return;

	// 5. Calculate rotation delta required to align item axis with the hand vector
	const FQuat TargetDeltaRotation = FQuat::FindBetweenVectors(CurrentItemAimVector, TargetHandAimVector);
	const FQuat AimDeltaRotation = FQuat::Slerp(FQuat::Identity, TargetDeltaRotation, FMath::Clamp(DeltaTime * 30.0f, 0.0f, 1.0f));

	// 6. Rotate root around the primary hand grab socket (Pivot point)
	const FVector PivotPoint = ExpectedPrimItemSocketWorld.GetLocation();
	const FVector NewLocation = PivotPoint + AimDeltaRotation.RotateVector(BaseRootWorld.GetLocation() - PivotPoint);
	const FQuat NewRotation = AimDeltaRotation * BaseRootWorld.GetRotation();

	const FTransform FinalTwoHandedTransform(NewRotation, NewLocation, m_OrigItemScale);

	inter->SetNewActorTransform(FinalTwoHandedTransform, false, nullptr, ETeleportType::TeleportPhysics);
}

bool UGrabItemInteractionState::TryAttachBySocket_Internal(IInteractable* inter, const FTransform& handSocketWorld, bool primary)
{
	if (!inter || !m_ownersSkelMesh || !m_OwningActor) return false;
	UMeshComponent* ItemMesh = inter->GetMesh();
	if (!ItemMesh) return false;

	const FName socketName = inter->GetItemSocketName(primary);
	if (socketName.IsNone() || !ItemMesh->DoesSocketExist(socketName))
	{
		return false;
	}

	// 1. Strip scale from sockets to process Position and Rotation only
	FTransform UnscaledItemSocket = ItemMesh->GetSocketTransform(socketName, ERelativeTransformSpace::RTS_World);
	UnscaledItemSocket.SetScale3D(FVector::OneVector);

	FTransform UnscaledHandSocket = handSocketWorld;
	UnscaledHandSocket.SetScale3D(FVector::OneVector);

	USceneComponent* rootComp = ItemMesh->GetAttachmentRoot();
	USceneComponent* targetComp = (rootComp != nullptr) ? rootComp : ItemMesh;

	if (primary)
	{
		// 2. Compute clean alignment delta
		const FTransform delta = UnscaledItemSocket.Inverse() * UnscaledHandSocket;

		// 3. Strip scale from item root before matrix multiplication
		FTransform rootT = targetComp->GetComponentTransform();
		rootT.SetScale3D(FVector::OneVector);

		// 4. Final transform with manually assigned m_OrigItemScale
		FTransform targetRootT = rootT * delta;
		targetRootT.SetScale3D(m_OrigItemScale);

		inter->SetNewActorTransform(targetRootT, false, nullptr, ETeleportType::TeleportPhysics);

		// 5. Relative grip transform (Position and Rotation only)
		m_PrimaryGripRelativeTransform = targetRootT.GetRelativeTransform(UnscaledHandSocket);
		m_PrimaryGripRelativeTransform.SetScale3D(FVector::OneVector);
	}
	else
	{
		// 6. Calculate secondary grip offset relative to unscaled hand socket
		m_SecondaryGripRelativeTransform = UnscaledItemSocket.GetRelativeTransform(UnscaledHandSocket);
		m_SecondaryGripRelativeTransform.SetScale3D(FVector::OneVector);
	}

	return true;
}