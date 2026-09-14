// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/Base/StateManagerComponent.h"
#include "Components/Base/StateComponentBase.h"

// Sets default values for this component's properties
UStateManagerComponent::UStateManagerComponent(const FObjectInitializer& init) : Super(init)
{
	bAutoActivate = false;
	PrimaryComponentTick.bCanEverTick = false;
	PrevState = 0;
}

void UStateManagerComponent::OnTick(float DeltaTime)
{
	if (EnableDebug && m_EnumToStrConverter && CurrentMsgIndex >= 0 && PrevMsgIndex >= 0 && CurrentMsgIndex != PrevMsgIndex)
	{
		Debug();
	}

	if (CurrentState)
	{
		CurrentState->OnStateTick(DeltaTime);
	}	
}

uint8 UStateManagerComponent::GetCurrentStateEnum() const
{
	return CurrentState->GetStateEnum();
}

uint8 UStateManagerComponent::GetPrevStateEnum() const
{
	return PrevState;
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

#if WITH_EDITOR

void UStateManagerComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	FName propName = PropertyChangedEvent.GetPropertyName();
	if (propName.IsNone()) return;
	if (propName == GET_MEMBER_NAME_CHECKED(UStateManagerComponent, DebugState))
	{
		SwitchState(DebugState);
	}
}

#endif

void UStateManagerComponent::Debug()
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(CurrentMsgIndex, DisplayTime, DebugColor, FString::Printf(TEXT("Current <%s> State: %s "), *InternalName.ToString(), *m_EnumToStrConverter(GetCurrentStateEnum())));
		GEngine->AddOnScreenDebugMessage(PrevMsgIndex, DisplayTime, DebugColor, FString::Printf(TEXT("Prev <%s> State: %s "), *InternalName.ToString(), *m_EnumToStrConverter(GetPrevStateEnum())));
	}
}

UStateBlackboard* UStateManagerComponent::GetBlackboard()
{
	if (!StateBlackBoard)
	{
		StateBlackBoard = NewObject<UStateBlackboard>();
	}
	return StateBlackBoard;
}

UStateBlackboard* UStateManagerComponent::GetGlobalBlackboard() const
{
	return GlobalBlackBoard;
}

void UStateManagerComponent::AddGlobalBlackBoard(UStateBlackboard* globalBlackBoard)
{
	GlobalBlackBoard = globalBlackBoard;
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
		if (!CanTransit(CurrentState->GetStateEnum(), state))
			return;
		CurrentState->OnStateExit();
		PrevState = CurrentState->GetStateEnum();
	}

	CurrentState = *pNewState;

	if (CurrentState)
	{
		CurrentState->OnStateEnter();
	}
}

void UStateManagerComponent::BeginPlay()
{
	for (auto k : m_EnumStateMap)
	{
		k.Value->BeginPlay();
	}
}

