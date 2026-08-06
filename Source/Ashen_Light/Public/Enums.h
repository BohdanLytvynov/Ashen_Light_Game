// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enums.generated.h"

UENUM(BlueprintType)
enum class EVRCharacterState : uint8
{
    VRCS_Blocked    UMETA(DisplayName = "Blocked State"),
    VRCS_FreeRoam   UMETA(DisplayName = "Free Roam State"),
    VRCS_Battle     UMETA(DisplayName = "Battle State")
};

UENUM(BlueprintType)
enum class EVRControllerState : uint8
{
    VRCS_Neutral UMETA(DisplayName = "Neutral Controller State"),
    VRCS_Grabbing UMETA(DisplayName = "Grabbing Controller State"),
    VRCS_Triggering UMETA(DisplayName = "Triggering Controller State")
};

