// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include"FVRControllerButtonsState.h"
#include "CoreMinimal.h"

/**
 * 
 */
class ASHEN_LIGHT_API FVRPlayerInputState
{
public:
	FVRPlayerInputState() {}
	~FVRPlayerInputState() {}

    UPROPERTY(BlueprintReadWrite, Category = "VR Input")
    FVRControllerButtonsState LeftHand;
    
    UPROPERTY(BlueprintReadWrite, Category = "VR Input")
    FVRControllerButtonsState RightHand;

    /// <summary>
    /// Gets the Controller Buttons State for the particular hand
    /// </summary>
    /// <param name="Hand"> - VR Controller Hand</param>
    /// <returns>VR Controller Buttons State</returns>
    const FVRControllerButtonsState& GetHandState(EVRControllerHand Hand) const
    {
        return (Hand == EVRControllerHand::Left) ? LeftHand : RightHand;
    }

    /// <summary>
    /// Gets the Controller Buttons State for the particular hand
    /// </summary>
    /// <param name="Hand"> - VR Controller Hand</param>
    /// <returns>VR Controller Buttons State</returns>
    FVRControllerButtonsState& GetHandState(EVRControllerHand Hand)
    {
        return (Hand == EVRControllerHand::Left) ? LeftHand : RightHand;
    }
};
