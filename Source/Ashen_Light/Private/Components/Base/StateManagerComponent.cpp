// Fill out your copyright notice in the Description page of Project Settings.

#include "../../../Public/Components/Base/StateManagerComponent.h"
#include "../../../Public/Components/Base/StateComponentBase.h"

// Sets default values for this component's properties
UStateManagerComponent::UStateManagerComponent(const FObjectInitializer& init) : Super(init)
{
	StateBlackBoardInitialized = false;
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

void UStateManagerComponent::BuildStateMatrix(int32 size)
{
	StateMatrix = FRectMatrix<bool>(size);
}

void UStateManagerComponent::ConfigureStateMatrix(StateMatConfigDelegate config)
{
	if (!config) return;
	if (StateMatrix.GetSize() == 0) return;
	config(&StateMatrix);
}

bool UStateManagerComponent::CanTransit(uint8 origState, uint8 destState) const
{
	if (StateMatrix.GetSize() == 0) return true;
	const bool* temp = StateMatrix.Get(origState, destState);
	return temp != nullptr ? *temp : false;
}

FStateBlackboardBase* UStateManagerComponent::GetBlackboard()
{
	if (!StateBlackBoard)
	{
		StateBlackBoard = new FStateBlackboardBase();
		StateBlackBoardInitialized = true;
	}
	return StateBlackBoard;
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
		if (!CanTransit((uint8)CurrentState->GetStateEnum(), state))
			return;
	}

	if (CurrentState)
	{
		CurrentState->OnStateExit();
	}
	
	CurrentState = *pNewState;
	CurrentState->OnStateEnter();
}

void UStateManagerComponent::Cleanup()
{
	if (StateBlackBoardInitialized)
	{
		delete StateBlackBoard;
	}
}

