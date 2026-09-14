// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/Items/Base/InteractableItemStateBase.h"
#include "GrabItemInteractionState.generated.h"

/**
 * 
 */
UCLASS()
class ASHEN_LIGHT_API UGrabItemInteractionState : public UInteractableItemStateBase
{
	GENERATED_BODY()
public:
	UGrabItemInteractionState(const FObjectInitializer& init);
	void OnStateEnter() override;
	void OnStateTick(float DeltaTime) override;
protected:
	UPROPERTY(VisibleAnywhere, Category = "Grab State Component", meta = (Tooltip="If true, cache will be scaled when we Enter this State."))
	bool CacheItemScale = false;
private:
	bool m_IsGrabbed;
	bool m_IsTwoHanded;
	FVector m_OrigItemScale;
	UPROPERTY()
	AActor* m_OwningActor;
	UPROPERTY()
	FName m_RightHandGrabSocketName;
	UPROPERTY()
	FName m_LeftHandGrabSocketName;
	UPROPERTY()
	FTransform m_PrimaryGripRelativeTransform;
	UPROPERTY();
	FTransform m_SecondaryGripRelativeTransform;
	UPROPERTY()
	USkeletalMeshComponent* m_ownersSkelMesh;
	bool TryAttachBySocket(IInteractable* inter, EVRControllerHand hand, bool primary);
	bool TryAttachBySocket_Internal(IInteractable* inter, const FTransform& handSocketWorld, bool primary);
	void UpdateSingleHandTransform(IInteractable* inter, EVRControllerHand primaryHand);
	void UpdateTwoHandedTransform(IInteractable* inter, EVRControllerHand primaryHand, EVRControllerHand secondaryHand, float DeltaTime);
};
