// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Interfaces/Items/Interactable.h"
#include "Dto/FVRPlayerInputState.h"
#include "VRInteractionSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class ASHEN_LIGHT_API UVRInteractionSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:
	/// <summary>
	/// If sensor hit some IInteractable object, Hover state will be called on the hovered object
	/// </summary>
	/// <param name="sensor"> - Sensor for evaluation</param>
	/// <param name="hand"> - VR Controller Hand</param>
	/// <param name="owner"> - Owner of the VR Controller Hand</param>
	virtual void ProcessMotionControllerHitSensor(class UMotionControllerHitSensor* sensor, EVRControllerHand hand, APawn* owner);
	/// <summary>
	/// Add the Pawn pointer to the Map that stores the VR Controller Buttons State for left and right hand
	/// </summary>
	/// <param name="pawn"> - Pawn to register</param>
	virtual void RegisterPawn(APawn* pawn);
	/// <summary>
	/// Removes the record with the registered Pawn from the Map and all state for VR Controller Buttons for both hands
	/// </summary>
	/// <param name="pawn"> - Pawn to remove from the Map</param>
	virtual void UnRegisterPawn(APawn* pawn);
	/// <summary>
	/// Updates 2D Axis Value of Thumstick for particular hand of the particular Player
	/// </summary>
	/// <param name="PlayerPawn"> - Player</param>
	/// <param name="Hand"> - Player's hand</param>
	/// <param name="value"> - new axis value</param>
	/// <param name="xAxis"> - Update X axis if false, else update Y axis</param>
	virtual void UpdatePlayerAxisThumbstickState(APawn* PlayerPawn, EVRControllerHand Hand, float value, bool xAxis);
	/// <summary>
	/// Updates Axis Value for the VR Controller Buttons State for particular hand of the player
	/// </summary>
	/// <param name="PlayerPawn"> - Player</param>
	/// <param name="Hand"> - Player's hand</param>
	/// <param name="Button"> - Button to update state of</param>
	/// <param name="AxisValue"> - new float axis value</param>
	virtual void UpdatePlayerAxisState(APawn* PlayerPawn, EVRControllerHand Hand, EVRButtonType Button, float AxisValue);
	/// <summary>
	/// Update the state of the discrete controller's button
	/// </summary>
	/// <param name="PlayerPawn"> - Player</param>
	/// <param name="Hand"> - Players Hand</param>
	/// <param name="Button"> - Button that was pressed / released</param>
	/// <param name="bIsPressed"> - True if we press the button, false if button was released</param>
	virtual void UpdatePlayerButtonState(APawn* PlayerPawn, EVRControllerHand Hand, EVRButtonType Button, bool bIsPressed);
	/// <summary>
	/// Update the state of the discrete touch controller's button
	/// </summary>
	/// <param name="PlayerPawn"> - Player</param>
	/// <param name="Hand"> - Players Hand</param>
	/// <param name="Button"> - Button that was pressed / released</param>
	/// <param name="bIsPressed"> - True if we touch the button, false if button was released</param>
	virtual void UpdatePlayerTouchState(APawn* PlayerPawn, EVRControllerHand Hand, EVRButtonType Button, bool bIsTouched);
	/// <summary>
	/// Returns state of the VR Motion Controller Buttons
	/// </summary>
	/// <param name="PlayerPawn"> - Player</param>
	/// <param name="Hand"> - Player's hand</param>
	/// <param name="outControllerButtonState"> - State of the Buttons</param>
	virtual void GetVRControllerButtonsState(APawn* PlayerPawn, EVRControllerHand Hand, FVRControllerButtonsState& outControllerButtonState);
	/// <summary>
	/// Set name of the Grab Hand Socket
	/// </summary>
	/// <param name="name"> - Skel Mesh Socket Name</param>
	/// <param name="right"> - True - for the right hand, false - for the left hand</param>
	FORCEINLINE void SetGrabSocketName(const FName& name, bool right)
	{
		if (right)
		{
			m_RightGrabSocketName = name;
		}
		else
		{
			m_LeftGrabSocketName = name;
		}
	}
	/// <summary>
	/// Gets Grab Socket Name for the VRCharacter
	/// </summary>
	/// <param name="bIsRightHand"> - True - right hand, false - left hand</param>
	/// <returns>Name of the Grab Socket</returns>
	FORCEINLINE FName GetGrabSocketName(bool bIsRightHand) const
	{
		return bIsRightHand ? m_RightGrabSocketName : m_LeftGrabSocketName;
	}
private:
	TMap<TWeakObjectPtr<APawn>, FVRPlayerInputState> PlayersInputMap;//We store the Pointer to the Pawn and it's VR Controllers buttons states
	FName m_RightGrabSocketName;
	FName m_LeftGrabSocketName;
};
