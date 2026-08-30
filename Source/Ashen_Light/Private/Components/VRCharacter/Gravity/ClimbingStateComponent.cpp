// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/VRCharacter/Gravity/ClimbingStateComponent.h"

UClimbingStateComponent::UClimbingStateComponent(const FObjectInitializer& init) : Super(init)
{

}

void UClimbingStateComponent::OnStateTick(float DeltaTime)
{
	Super::OnStateTick(DeltaTime);

	IVRCharacterInterface* context = GetContext();
	UStateManagerComponent* stateManager = GetStateManager();
	if (CheckAndHandleCameraInMesh(context, stateManager)) return;
	if (CheckAndHandleInAir(context, stateManager)) return;
	if (CheckAndHandleGroundHit(context, stateManager, 0.f)) return;

	//To Do implement climbing
}
