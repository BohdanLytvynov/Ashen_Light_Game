// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enums.generated.h"

UENUM(BlueprintType)
enum class EVRControllerState : uint8
{
    VRCS_Neutral UMETA(DisplayName = "Neutral Controller State"),
    VRCS_Grabbing UMETA(DisplayName = "Grabbing Controller State"),
    VRCS_Triggering UMETA(DisplayName = "Triggering Controller State")
};

UENUM(BlueprintType)
enum class ELocomotionSpace : uint8
{
    ELS_Physical UMETA(DisplayName = "Physical Movement"),
    ELS_TrackingSpace UMETA(DisplayName = "Tracking Space Movement")
};

UENUM(BlueprintType)
enum class EMotionState : uint8
{
    EMS_Idle UMETA(DisplayName = "Idle"),
    EMS_Walk UMETA(DisplayName = "Walk"),
    EMS_Run UMETA(DisplayName = "Run")
};