// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/Items/Interactable.h"
#include "Interfaces/StateDriven.h"
#include "Enums.h"
#include "InteractableItem.generated.h"

UCLASS(Abstract)
class ASHEN_LIGHT_API AInteractableItem : public AActor, public IInteractable
{
	GENERATED_BODY()
public:
	// Sets default values for this actor's properties
	AInteractableItem(const FObjectInitializer& init);
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	void DisableCollision(UPrimitiveComponent* comp);
	void ConfigureRoot();
	void UpdateRoot();
	float mutable PhysicalMass = 0.f;

#pragma region Components
	UPROPERTY(VisibleAnywhere)
	class UShapeComponent* CollisionShape;

	UPROPERTY(VisibleAnywhere)
	UMeshComponent* ItemMesh;

	UPROPERTY(VisibleAnywhere)
	UMeshComponent* GlowMesh;

	//---State Managers---

	UPROPERTY(VisibleAnywhere)
	class UStateManagerComponent* InteractiveStateManager;

	UPROPERTY(VisibleAnywhere)
	UStateManagerComponent* LifeCycleStateManger;

	UPROPERTY(VisibleAnywhere)
	UStateManagerComponent* PhysicsStateManager;

	//---States---
	//---Interaction---
	UPROPERTY(VisibleAnywhere)
	class UIdleInteractionState* IdleInteractionState;

	UPROPERTY(VisibleAnywhere)
	class UHoverInteractionState* HoverInteractionstate;

	UPROPERTY(VisibleAnywhere)
	class UPullingInteractionState* PullingInteractionState;

	UPROPERTY(VisibleAnywhere)
	class UGrabItemInteractionState* GrabItemInteractionState;
	//---Physics---
	
	UPROPERTY(VisibleAnywhere)
	class UDisablePhysicsItemState* DisablePhysicsState;

	UPROPERTY(VisibleAnywhere)
	class UKinematicItemState* KinematicItemState;

	UPROPERTY(VisibleAnywhere)
	class USimPhysicsItemState* SimPhysicsState;

	//---Blackboards---
	UPROPERTY(VisibleAnywhere)
	class UStateBlackboard* GlobalBlackboard;
	
	//---Sensors---
	UPROPERTY(VisibleAnywhere)
	class UVelocitySensor* VelocitySensor;
#pragma endregion

#pragma region UPROPERTIES

	UPROPERTY(EditAnywhere, Category = "Interactable Config")
	TArray<TEnumAsByte<EItemCapabilities>> ItemCapabilities;

	UPROPERTY(EditAnywhere, Category = "Hover State Component")
	float HoverDuration = 0.2f;

	//---Materials---
	UPROPERTY(EditAnywhere, Category = "Hover State Component")
	class UMaterialInterface* GlowMaterialBase;

	UPROPERTY(Transient)
	class UMaterialInstanceDynamic* GlowMaterialInstance;

	UPROPERTY(EditAnywhere, Category = "VR Interaction Sockets")
	FName PrimaryGrabSocket = FName("PrimaryGrabSocket");

	UPROPERTY(EditAnywhere, Category = "VR Interaction Sockets")
	FName SecondaryGrabSocket = FName("SecondaryGrabSocket");

	UPROPERTY(EditAnywhere, Category = "Debug")
	bool EnableInteractionStatesDebug;

	UPROPERTY(EditAnywhere, Category = "Debug")
	bool EnablePhysicsStatesDebug;

	UPROPERTY(EditAnywhere, Category = "Debug")
	bool EnableLifeCycleDebug;
#pragma endregion

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
#pragma region IInteractable
	virtual void OnHoverCmd_Implementation(USceneComponent* hoverReasonComponent, EVRControllerHand hand, APawn* hoverReasonActor) override;
	virtual bool HasCapability(uint8 capability) const override;
	virtual class UMeshComponent* GetGlowMesh() const override;
	virtual class UMeshComponent* GetMesh() const override;
	virtual UMaterialInstanceDynamic* GetGlowMaterialInstance() const override;
	void GetVRControllerButtonState(EVRControllerHand hand, class FVRControllerButtonsState& state) override;
	EVRControllerHand GetVRHand(bool primary) const override;
	USceneComponent* GetVRHoverReason(bool primary) const override;
	void ResetState() override;
	void ResetStateForHand(bool primary) override;
	class UShapeComponent* GetCollsiionShape() const override;
	void SwitchPhysicsManager(EItemPhysicsState physicsState) override;
	FVector GetLinearVelocity() const override;
	void PromoteSecondaryToPrimary() override;
	APawn* GetItemOwner() const override;
	FORCEINLINE UVRInteractionSubsystem* GetVRInterSubsystem() override;
	FName GetItemSocketName(bool primary) override;
	/// <summary>
	/// Mass in Kg
	/// </summary>
	/// <returns></returns>
	float GetPhysicalMass() const override;
	UPrimitiveComponent* GetPhysicsRootComponent() const override;
	/// <summary>
	/// Get basis Vector according to Axis: XYZ
	/// </summary>
	/// <param name="axis"></param>
	/// <returns></returns>
	FVector GetBasisVector(FName axis) const override;
#pragma endregion

#pragma region IStateDriven
	virtual FTransform GetActorTransform() override;
	virtual void SetNewActorLocation(const FVector& worldLocation, bool sweep, FHitResult* outHit, ETeleportType teleType) override;
	virtual void SetNewActorTransform(const FTransform& newTransform, bool sweep, FHitResult* outHit, ETeleportType teleType) override;
#pragma endregion
private:
	//Weak pointer to the primary hover reason component
	TWeakObjectPtr<USceneComponent> m_hoverReasonPrimaryComponent;
	//Weak pointer to the secondary hover reason component
	TWeakObjectPtr<USceneComponent> m_hoverReasonSecondaryComponent;
	//Weak pointer to the Item's owner
	TWeakObjectPtr<APawn> m_OwningActor;
	//Support for two Hand Grip
	EVRControllerHand m_PrimaryHand;
	EVRControllerHand m_SecondaryHand;
	//Hover timer. Starts when we hover the object. When it ellapse, we will switch to the idle state
	FTimerHandle m_HoverTimer;
	void StartHoverTimer();
	void OnHoverTimerElapsed();
	class UVRInteractionSubsystem* m_VRSub;
};
