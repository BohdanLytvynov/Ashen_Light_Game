// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enums.h"

/**
 * 
 */
class ASHEN_LIGHT_API FVRControllerButtonsState
{
public:
    FVRControllerButtonsState() {}
    ~FVRControllerButtonsState() {}

    // 1. CONTINUOUS AXES
   
    /** Index trigger pull strength (0.0 .. 1.0) */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VR Input|Axes")
    float TriggerAxis = 0.0f;

    /** Side grip squeeze strength (0.0 .. 1.0) */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VR Input|Axes")
    float GripAxis = 0.0f;

    /** Thumbstick 2D position (X: -1.0..1.0, Y: -1.0..1.0) */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VR Input|Axes")
    FVector2D ThumbstickVector = FVector2D::ZeroVector;
    
    // 2. DIGITAL BUTTON PRESSES

    /** True if the index trigger is fully down */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VR Input|Buttons")
    bool bTriggerPressed = false;

    /** True if the side grip trigger is fully down */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VR Input|Buttons")
    bool bGripPressed = false;

    /** Primary face button state: X (Left Hand) or A (Right Hand) */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VR Input|Buttons")
    bool bPrimaryButtonPressed = false;

    /** Secondary face button state: Y (Left Hand) or B (Right Hand) */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VR Input|Buttons")
    bool bSecondaryButtonPressed = false;

    /** Menu or System action button state */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VR Input|Buttons")
    bool bMenuOrSystemPressed = false;

    /** Thumbstick click state (L3 / R3) */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VR Input|Buttons")
    bool bThumbstickClick = false;

    // 3. CAPACITIVE TOUCH SENSORS    

    /** Finger rests on the index trigger capacitive sensor */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VR Input|Touch")
    bool bTriggerTouched = false;

    /** Finger rests on the thumbstick capacitive surface */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VR Input|Touch")
    bool bThumbstickTouched = false;

    /** Finger rests on the ergonomic thumbrest plate */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VR Input|Touch")
    bool bThumbRestTouched = false;

    /** Finger rests on the primary face button (X/A) */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VR Input|Touch")
    bool bPrimaryButtonTouched = false;

    /** Finger rests on the secondary face button (Y/B) */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VR Input|Touch")
    bool bSecondaryButtonTouched = false;
   
    /** Evaluates whether a given button type is currently pressed down */
    FORCEINLINE bool IsButtonPressed(EVRButtonType Button) const
    {
        switch (Button)
        {
            case EVRButtonType::Trigger:
                return bTriggerPressed;
            case EVRButtonType::Grip:
                return bGripPressed;
            case EVRButtonType::Primary:
                return bPrimaryButtonPressed;
            case EVRButtonType::Secondary:
                return bSecondaryButtonPressed;
            case EVRButtonType::MenuOrSystem:
                return bMenuOrSystemPressed;
            case EVRButtonType::ThumbstickClick:
                return bThumbstickClick;
            default:
                return false;
        }
    }

    /** Checks whether the thumbstick vector exceeds the specified deadzone threshold */
    FORCEINLINE bool HasThumbstickInput(float DeadZone = 0.2f) const
    {
        return ThumbstickVector.SizeSquared() > (DeadZone * DeadZone);
    }

    /** Resets all states back to default values (e.g., when losing tracking or disconnecting) */
    void Reset()
    {
        TriggerAxis = 0.0f;
        GripAxis = 0.0f;
        ThumbstickVector = FVector2D::ZeroVector;

        bTriggerPressed = false;
        bGripPressed = false;
        bPrimaryButtonPressed = false;
        bSecondaryButtonPressed = false;
        bMenuOrSystemPressed = false;
        bThumbstickClick = false;

        bTriggerTouched = false;
        bThumbstickTouched = false;
        bThumbRestTouched = false;
        bPrimaryButtonTouched = false;
        bSecondaryButtonTouched = false;
    }
};

