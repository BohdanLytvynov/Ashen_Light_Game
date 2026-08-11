// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "../../Public/Interfaces/StateDriven.h"
#include "StateComponentBase.generated.h"


UCLASS(Abstract, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ASHEN_LIGHT_API UStateComponentBase : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UStateComponentBase(const FObjectInitializer& init);

	virtual void InitializeState(uint8 stateEnum, IStateDriven* stateDriven, class UStateManagerComponent* stateManager);
	virtual void OnStateEnter() {}
	virtual void OnStateExit() {}
	virtual void OnStateTick(float DeltaTime) {}

	FORCEINLINE uint8 GetStateEnum() const
	{
		return m_StateEnum;
	}

	virtual IStateDriven* GetContext()
	{
		return m_Context;
	}

	FORCEINLINE UStateManagerComponent* GetStateManager()
	{
		return m_StateManagerComponent;
	}

private:
	uint8 m_StateEnum;//Type of the current State
	IStateDriven* m_Context;//Context Object, to have an access to.
	UStateManagerComponent* m_StateManagerComponent;//Reference to the State Manager Component
};
