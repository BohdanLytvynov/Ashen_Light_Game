// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RawCpp/RectMatrix.h"
#include "StateBlackboard.h"
#include "StateManagerComponent.generated.h"

UCLASS(NotBlueprintable)
class ASHEN_LIGHT_API UStateManagerComponent : public UObject
{
	GENERATED_BODY()
	typedef void (*StateMatConfigDelegate) (FRectMatrix<bool>*);
public:	
	// Sets default values for this component's properties
	UStateManagerComponent(const FObjectInitializer& init);	
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
	virtual void BeginPlay();
	uint8 GetCurrentStateEnum() const;
	uint8 GetPrevStateEnum() const;
	void BuildStateMatrix(int32 size);
	void ConfigureStateMatrix(StateMatConfigDelegate config);
	UStateBlackboard* GetBlackboard();
	UStateBlackboard* GetGlobalBlackboard()  const;
	void AddGlobalBlackBoard(UStateBlackboard* globalBlackBoard);
protected:
	bool CanTransit(uint8 origState, uint8 destState) const;
	template<class Enum>
	bool CanTransit(Enum os, Enum ds) const;
	UPROPERTY()//GC will handle this
	UStateBlackboard* StateBlackBoard;
	UPROPERTY();
	UStateBlackboard* GlobalBlackBoard;
private:
	TMap<uint8, UStateComponentBase*> m_EnumStateMap;
	UStateComponentBase* CurrentState;
	uint8 PrevState;
	FRectMatrix<bool> StateMatrix;
};

template<class Enum>
inline void UStateManagerComponent::SwitchState(Enum state)
{
	const uint8 e = static_cast<uint8>(state);
	SwitchState(e);
}

template<class Enum>
inline bool UStateManagerComponent::CanTransit(Enum os, Enum ds) const
{
	uint8 origState = static_cast<uint8>(os);
	uint8 destState = static_cast<uint8>(ds);
	return CanTransit(origState, destState);
}
