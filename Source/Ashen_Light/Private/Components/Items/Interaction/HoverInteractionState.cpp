// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Items/Interaction/HoverInteractionState.h"
#include "Interfaces/Items/Interactable.h"
#include "Components/StaticMeshComponent.h"
#include "Components/Base/StateManagerComponent.h"
#include "Dto/FVRControllerButtonsState.h"

UHoverInteractionState::UHoverInteractionState(const FObjectInitializer& init) : Super(init)
{
	
}

void UHoverInteractionState::OnStateTick(float DeltaTime)
{
    Super::OnStateTick(DeltaTime);
    UStateManagerComponent* stateManger = GetStateManager();
    IInteractable* inter = GetContext();
    if (!inter || !stateManger) return;
    EVRControllerHand primHand = inter->GetVRHand(true);
    FVRControllerButtonsState primHandState;
    inter->GetVRControllerButtonState(primHand, primHandState);

    //We press Grip -> go to Pulling State
    if (primHandState.IsButtonPressed(EVRButtonType::Grip) && inter->HasCapability((uint8)EItemCapabilities::EIC_Pull))
    {
        stateManger->SwitchState(EItemInteractiveState::EIIS_Pulling);
        return;
    }
    UMaterialInstanceDynamic* glowMat = GetMatInstance();
    if (!glowMat) return;
    const bool bIsFree = (primHand == EVRControllerHand::None);
    const float TargetOpacity = bIsFree ? 0.0f : 1.0f;
    if (CurrentOpacity >= 1.f) return;
    UpdateGlowMatOpacity(TargetOpacity, DeltaTime, OpacityInterpConstantEnter);
}

void UHoverInteractionState::OnStateEnter()
{
    IInteractable* inter = GetContext();
    if (!inter) return;
    UMaterialInstanceDynamic* matInst = inter->GetGlowMaterialInstance();
    if (!matInst) return;
    matInst->GetScalarParameterValue(FHashedMaterialParameterInfo("Opacity"), CurrentOpacity);
}


