// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RawCpp/RectMatrix.h"
#include "StateBlackboardBase.h"
#include "StateManagerComponent.generated.h"

UCLASS(NotBlueprintable)
class ASHEN_LIGHT_API UStateManagerComponent : public UObject
{
	GENERATED_BODY()
	typedef void (*StateMatConfigDelegate) (FRectMatrix<bool>*);
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

	template<class Enum>
	void SwitchState(Enum state);

	/// <summary>
	/// Called every tick. Processes the main state logic
	/// </summary>
	/// <param name="DeltaTime"></param>
	void OnTick(float DeltaTime);
	uint8 GetCurrentStateEnum();
	void BuildStateMatrix(int32 size);
	void ConfigureStateMatrix(StateMatConfigDelegate config);
	void Cleanup();
	virtual FStateBlackboardBase* GetBlackboard();
protected:
	bool CanTransit(uint8 origState, uint8 destState) const;
private:
	TMap<uint8, UStateComponentBase*> m_EnumStateMap;
	UStateComponentBase* CurrentState;
	FRectMatrix<bool> StateMatrix;
	FStateBlackboardBase* StateBlackBoard;
	bool StateBlackBoardInitialized;
};

template<class Enum>
inline void UStateManagerComponent::SwitchState(Enum state)
{
	const uint8 e = static_cast<uint8>(state);
	SwitchState(e);
}
