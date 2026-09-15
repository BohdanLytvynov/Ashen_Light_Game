// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Interfaces/StateDriven.h"
#include "Enums.h"
#include "Interactable.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, Blueprintable)
class UInteractable : public UStateDriven
{
	GENERATED_BODY()
};

/**
 * 
 */

class ASHEN_LIGHT_API IInteractable : public IStateDriven
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	/// <summary>
	/// Switch state to hover. That also starts polling of the VR Controller Buttons State
	/// </summary>
	/// <param name="hoverReasonComponent"> - Component that was cause of the Hover</param>
	/// <param name="hand"> - Hand that hovered the item</param>
	/// <param name="hoverReasonActor"> - Hand's owner</param>
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnHoverCmd(USceneComponent* hoverReasonComponent, EVRControllerHand hand, APawn* hoverReasonActor);
	/// <summary>
	/// Does item has a certain capability?
	/// </summary>
	/// <param name="capability"> - Capability to check</param>
	/// <returns>True if item has Capability, false if not.</returns>	
	virtual bool HasCapability(uint8 capability) const = 0; 
	/// <summary>
	/// Get the mesh responsible for Glow effect
	/// </summary>
	/// <returns>Static Mesh Component</returns>
	virtual class UMeshComponent* GetGlowMesh() const = 0;
	/// <summary>
	/// Gets the Static Mesh Component of the Item
	/// </summary>
	/// <returns>Static Mesh Component of the Item</returns>
	virtual UMeshComponent* GetMesh() const = 0;
	/// <summary>
	/// Gets Glow material instance dynamic
	/// </summary>
	/// <returns>Mat instance that effects the glow effect</returns>
	virtual UMaterialInstanceDynamic* GetGlowMaterialInstance() const = 0;
	/// <summary>
	/// Using current Item owner gets the state of the VR Controller Buttons State for some hand
	/// </summary>
	/// <param name="hand"> - Owner's hand</param>
	/// <param name="state"> - State of the VR Controller Buttons</param>
	virtual void GetVRControllerButtonState(EVRControllerHand hand, class FVRControllerButtonsState& state) = 0;
	/// <summary>
	/// Get Primary / Secondary hand that grabbed the Item
	/// </summary>
	/// <param name="primary"> - True if hand is primary</param>
	/// <returns>Returns primary or secondary hand</returns>
	virtual EVRControllerHand GetVRHand(bool primary) const = 0;
	/// <summary>
	/// Get Primary or Secondary Hover Reason Component
	/// </summary>
	/// <param name="primary"> - True if we want the primary hover reason component</param>
	/// <returns>Primary / Secondary hover reason component</returns>
	virtual USceneComponent* GetVRHoverReason(bool primary) const = 0;
	/// <summary>
	/// Get the owner of the item
	/// </summary>
	/// <returns>Owner of the Item</returns>
	virtual APawn* GetItemOwner() const = 0;
	/// <summary>
	/// Reset Primary / Secondary and the owner info to default
	/// </summary>
	virtual void ResetState() = 0;
	/// <summary>
	/// Gets the root of the Interactable Object (It will be some shape)
	/// </summary>
	virtual class UShapeComponent* GetCollsiionShape() const = 0;
	/// <summary>
	/// Switch physics state to the next state
	/// </summary>
	/// <param name="physicsState"> - Next physics state to switch to</param>
	virtual void SwitchPhysicsManager(EItemPhysicsState physicsState) = 0;
	/// <summary>
	/// Gets linear velocity of the item. In case of Kinematic mode - we get velocity 
	/// from the Velocity sensor V = V0 + ds/dt, dt -> 0
	/// </summary>
	/// <returns>Linear Velocity</returns>
	virtual FVector GetLinearVelocity() const = 0;
	/// <summary>
	/// Switch the primary to secondary data, case when we pass item to other hand
	/// </summary>
	virtual void PromoteSecondaryToPrimary() = 0;
	/// <summary>
	/// Set Primary / Secondary Hand data to default
	/// </summary>
	/// <param name="primary"></param>
	virtual void ResetStateForHand(bool primary) = 0;
	virtual class UVRInteractionSubsystem* GetVRInterSubsystem() = 0;
	virtual FName GetItemSocketName(bool primary) = 0;
	virtual float GetPhysicalMass() const = 0;
	virtual UPrimitiveComponent* GetPhysicsRootComponent() const = 0;
	/// <summary>
	/// Get basis Vector according to Axis: XYZ
	/// </summary>
	/// <param name="axis"></param>
	/// <returns></returns>
	virtual FVector GetBasisVector(FName axis) const = 0;
protected:
};
