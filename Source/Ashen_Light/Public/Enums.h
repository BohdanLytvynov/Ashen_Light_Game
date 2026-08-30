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

UENUM(BlueprintType, meta = (ToolTip = "State of the Locomotion State Machine"))
enum class ELocomotionSpace : uint8
{
    ELS_Physical = 0 UMETA(DisplayName = "Physical Movement"),
    ELS_TrackingSpace = 1 UMETA(DisplayName = "Tracking Space Movement")
};

UENUM(BlueprintType)
enum class EMotionState : uint8
{
    EMS_Idle UMETA(DisplayName = "Idle"),
    EMS_Walk UMETA(DisplayName = "Walk"),
    EMS_Run UMETA(DisplayName = "Run")
};

UENUM(BlueprintType, meta = (ToolTip = "State of the Gravity State Machine"))
enum class EGravityState : uint8
{
    EGS_InAir = 0 UMETA(DisplayName = "In Air State"),
    EGS_Grounded = 1 UMETA(DisplayName = "Grounded State"),
    EGS_Climbing = 2 UMETA(DisplayName = "Climbing State"),
    EGS_InMesh = 3 UMETA(DisplayName = "In Mesh State")
};