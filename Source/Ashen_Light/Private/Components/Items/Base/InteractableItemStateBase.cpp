// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Items/Base/InteractableItemStateBase.h"

UInteractableItemStateBase::UInteractableItemStateBase(const FObjectInitializer& init) : Super(init)
{
	CurrentOpacity = 0.f;
}

IInteractable* UInteractableItemStateBase::GetContext()
{
	if (!m_interactable)
	{
		IStateDriven* context = Super::GetContext();
		if (!context) return m_interactable;
		m_interactable = Cast<IInteractable>(context);
	}

	return m_interactable;
}

UMaterialInstanceDynamic* UInteractableItemStateBase::GetMatInstance()
{
	if (!GlowMat)
	{
		IInteractable* inter = GetContext();
		if (!inter) return GlowMat;
		GlowMat = inter->GetGlowMaterialInstance();
	}
	return GlowMat;
}

void UInteractableItemStateBase::UpdateGlowMatOpacity(float targetOpacity, float DeltaTime, float interpConstant)
{
	UMaterialInstanceDynamic* glowMat = GetMatInstance();
	if (glowMat)
	{
		CurrentOpacity = FMath::Clamp(FMath::FInterpTo(CurrentOpacity, targetOpacity, DeltaTime, interpConstant), 0.f, 1.f);
		glowMat->SetScalarParameterValue(FName("Opacity"), CurrentOpacity);
	}
}
