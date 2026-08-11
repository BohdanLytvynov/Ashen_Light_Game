// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateManagerComponent.generated.h"


UCLASS(BlueprintType, Blueprintable)
class ASHEN_LIGHT_API UStateManagerComponent : public UObject
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UStateManagerComponent(const FObjectInitializer& init);
	// Called every frame
	
	/// <summary>
	/// Register the new state to the Map
	/// </summary>
	/// <param name="state">State to register</param>
	void RegisterState(class UStateComponentBase* state);
	/// <summary>
	/// Switch to the new state
	/// </summary>
	/// <param name="state">Enum of the state to switch to</param>
	void SwitchState(uint8 state);
	/// <summary>
	/// Called every tick. Processes the main state logic
	/// </summary>
	/// <param name="DeltaTime"></param>
	void OnTick(float DeltaTime);

	uint8 GetCurrentStateEnum();
private:
	TMap<uint8, UStateComponentBase*> m_EnumStateMap;
	UStateComponentBase* CurrentState;
};
