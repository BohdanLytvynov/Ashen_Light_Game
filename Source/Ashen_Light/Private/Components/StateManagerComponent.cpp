// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/StateManagerComponent.h"
#include "../../Public/Components/StateComponentBase.h"

// Sets default values for this component's properties
UStateManagerComponent::UStateManagerComponent(const FObjectInitializer& init) : Super(init)
{
	
}

void UStateManagerComponent::OnTick(float DeltaTime)
{
	if (CurrentState)
	{
		CurrentState->OnStateTick(DeltaTime);
	}	
}

uint8 UStateManagerComponent::GetCurrentStateEnum()
{
	return CurrentState->GetStateEnum();
}

void UStateManagerComponent::RegisterState(UStateComponentBase* state)
{
	const uint8 Enum = state->GetStateEnum();
	if (m_EnumStateMap.Contains(Enum))
		return;
	m_EnumStateMap.Add(Enum, state);
}

void UStateManagerComponent::SwitchState(uint8 state)
{
	auto pNewState = m_EnumStateMap.Find(state);
	if (!pNewState || !(*pNewState)) return;

	if (CurrentState)
	{
		CurrentState->OnStateExit();
	}
	
	CurrentState = *pNewState;
	CurrentState->OnStateEnter();
}


