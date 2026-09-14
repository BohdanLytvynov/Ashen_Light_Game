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

UENUM(BlueprintType, meta = (ToolTip = "Items Lifecycle States"))
enum class EItemLifecycleState : uint8
{
    EILS_Active UMETA(DisplayName = "Active State", ToolTip = "Item is the Actor in the Scene. Collision type is World Static"),
    EILS_InPool UMETA(DisplayName = "Item is in the Pool. Metadata in the Scene")
};

UENUM(BlueprintType, meta = (ToolTip = "Items Gravity States"))
enum class EItemPhysicsState : uint8
{
    EIGS_SimulatePhysics UMETA(DisplayName = "Simulate Physics State"),
    EIGS_Kinematic UMETA(DisplayName = "Kinematic State"),
    EIGS_DisablePhysics UMETA(DisplayName = "Disable Physics State")
};

UENUM(BlueprintType, meta = (ToolTip = "Items Interactive States"))
enum class EItemInteractiveState : uint8
{
    EIIS_Idle UMETA(DisplayName = "Idle Interactive State"),
    EIIS_Hovered UMETA(DisplayName = "Hovered State"),
    EIIS_Pulling UMETA(DisplayName = "Pulling State"),
    EIIS_Grab UMETA(DisplayName = "Held State")
};

UENUM(BlueprintType, meta = (ToolTip = "Item Capabilities"))
enum class EItemCapabilities : uint8
{
    EIC_None = 0,
    EIC_Hover = 1 << 0,
    EIC_Pull = 1 << 1,
    EIC_Grab = 1 << 2, 
    EIC_Equip = 1 << 3,
    EIC_Climb = 1 << 4
};

UENUM(BlueprintType)
enum class EVRControllerHand : uint8
{
    None,
    Left,
    Right
};

UENUM(BlueprintType)
enum class EVRButtonType : uint8
{
    Trigger,
    Grip,
    Primary,   // X Left / A Right
    Secondary, // Y Left / B Right
    MenuOrSystem,
    ThumbstickClick,
    ThumbstickAxis
};