// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Base/StateComponentBase.h"
#include "Components/Base/StateManagerComponent.h"

// Sets default values for this component's properties
UStateComponentBase::UStateComponentBase(const FObjectInitializer& init) : Super(init)
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;//Disable Tick
}

void UStateComponentBase::InitializeState(uint8 stateEnum, IStateDriven* stateDriven, UStateManagerComponent* stateManager)
{
	m_StateEnum = stateEnum;
	m_Context = stateDriven;
	m_StateManagerComponent = stateManager;
}

UStateManagerComponent* UStateComponentBase::GetStateManager() const
{
	return m_StateManagerComponent;
}



