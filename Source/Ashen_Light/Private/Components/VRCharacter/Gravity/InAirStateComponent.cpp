// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/VRCharacter/Gravity/InAirStateComponent.h"
#include "Components/VRCharacter/Sensors/CameraFadeSensor.h"

UInAirStateComponent::UInAirStateComponent(const FObjectInitializer& init) : Super(init)
{

}

void UInAirStateComponent::OnStateTick(float DeltaTime)
{
	Super::OnStateTick(DeltaTime);

	IVRCharacterInterface* context = GetContext();
	UStateManagerComponent* stateManager = GetStateManager();
	if (!context || !stateManager) return;
	UCameraFadeSensor* cameraFade = context->GetCameraFadeSensor();
	if (cameraFade && cameraFade->IsHit())
	{
		
	}
}


