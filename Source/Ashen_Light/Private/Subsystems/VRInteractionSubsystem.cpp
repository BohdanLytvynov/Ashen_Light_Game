// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystems/VRInteractionSubsystem.h"
#include "Components/VRCharacter/Sensors/MotionControllerHitSensor.h"

void UVRInteractionSubsystem::ProcessMotionControllerHitSensor(UMotionControllerHitSensor* sensor, EVRControllerHand hand, APawn* owner)
{
    if (!sensor || !owner) return;
    if (!sensor->IsHit()) return;
    AActor* actor = sensor->GetHitActor();
    if (!actor) return;
    if (!actor->Implements<UInteractable>()) return;
    //case when Item implements IInteractable
    USceneComponent* trackingComp = sensor->GetTrackingComponent();
    if (!trackingComp) return;
    //Inform Item that it was hovered
    IInteractable::Execute_OnHoverCmd(actor, trackingComp, hand, owner);
}

void UVRInteractionSubsystem::RegisterPawn(APawn* pawn)
{
    if (!pawn) return;
    PlayersInputMap.Add(MakeWeakObjectPtr(pawn), FVRPlayerInputState());
}

void UVRInteractionSubsystem::UnRegisterPawn(APawn* pawn)
{
    if (!pawn) return;
    PlayersInputMap.Remove(MakeWeakObjectPtr(pawn));
}

void UVRInteractionSubsystem::UpdatePlayerAxisThumbstickState(APawn* PlayerPawn, EVRControllerHand Hand, float value, bool xAxis)
{
    if (!PlayerPawn) return;

    if (FVRPlayerInputState* PlayerState = PlayersInputMap.Find(PlayerPawn))
    {
        FVRControllerButtonsState& HandState = PlayerState->GetHandState(Hand);

        if (xAxis)
        {
            HandState.ThumbstickVector.X = value;
        }
        else
        {
            HandState.ThumbstickVector.Y = value;
        }
    }
}

void UVRInteractionSubsystem::UpdatePlayerAxisState(APawn* PlayerPawn, EVRControllerHand Hand, EVRButtonType Button, float AxisValue)
{
    if(!PlayerPawn) return;

    if (FVRPlayerInputState* PlayerState = PlayersInputMap.Find(PlayerPawn))
    {
        FVRControllerButtonsState& HandState = PlayerState->GetHandState(Hand);

        if (Button == EVRButtonType::Trigger)
        {
            HandState.TriggerAxis = AxisValue;
            HandState.bTriggerPressed = (AxisValue >= 0.7f);
        }
        else if (Button == EVRButtonType::Grip)
        {
            HandState.GripAxis = AxisValue;
            HandState.bGripPressed = (AxisValue >= 0.7f);
        }
    }
}

void UVRInteractionSubsystem::UpdatePlayerButtonState(APawn* PlayerPawn, EVRControllerHand Hand, EVRButtonType Button, bool bIsPressed)
{
    if (!PlayerPawn) return;

    if (FVRPlayerInputState* PlayerState = PlayersInputMap.Find(PlayerPawn))
    {
        FVRControllerButtonsState& HandState = PlayerState->GetHandState(Hand);

        switch (Button)
        {
        case EVRButtonType::Trigger:
            HandState.bTriggerPressed = bIsPressed;
            HandState.TriggerAxis = bIsPressed ? 1.0f : 0.0f;
            break;

        case EVRButtonType::Grip:
            HandState.bGripPressed = bIsPressed;
            HandState.GripAxis = bIsPressed ? 1.0f : 0.0f;
            break;

        case EVRButtonType::Primary:
            HandState.bPrimaryButtonPressed = bIsPressed;
            break;

        case EVRButtonType::Secondary:
            HandState.bSecondaryButtonPressed = bIsPressed;
            break;

        case EVRButtonType::MenuOrSystem:
            HandState.bMenuOrSystemPressed = bIsPressed;
            break;

        case EVRButtonType::ThumbstickClick:
            HandState.bThumbstickClick = bIsPressed;
            break;

        default:
            break;
        }
    }
}

void UVRInteractionSubsystem::UpdatePlayerTouchState(APawn* PlayerPawn, EVRControllerHand Hand, EVRButtonType Button, bool bIsTouched)
{
    if (!PlayerPawn) return;

    if (FVRPlayerInputState* PlayerState = PlayersInputMap.Find(PlayerPawn))
    {
        FVRControllerButtonsState& HandState = PlayerState->GetHandState(Hand);

        switch (Button)
        {
        case EVRButtonType::Trigger:
            HandState.bTriggerTouched = bIsTouched;
            break;

        case EVRButtonType::Primary:
            HandState.bPrimaryButtonTouched = bIsTouched;
            break;

        case EVRButtonType::Secondary:
            HandState.bSecondaryButtonTouched = bIsTouched;
            break;

        case EVRButtonType::ThumbstickClick:
            HandState.bThumbstickTouched = bIsTouched;
            break;

        default:
            break;
        }
    }
}

void UVRInteractionSubsystem::GetVRControllerButtonsState(APawn* PlayerPawn, EVRControllerHand Hand, FVRControllerButtonsState& outControllerButtonState)
{
    if (!PlayerPawn) return;

    if (FVRPlayerInputState* PlayerState = PlayersInputMap.Find(PlayerPawn))
    {
        outControllerButtonState = PlayerState->GetHandState(Hand);
    }
}

