// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RawCpp/RectMatrix.h"
#include "StateBlackboard.h"
#include "Components/ActorComponent.h"
#include "StateManagerComponent.generated.h"

typedef FString(*EnumToStrConverter)(uint8);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent), DefaultToInstanced, Blueprintable)
class ASHEN_LIGHT_API UStateManagerComponent : public UActorComponent
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
	template<class Enum>
	inline Enum GetPrevState()
	{
		const uint8 e = GetPrevStateEnum();
		return static_cast<Enum>(e);
	}
	template<class Enum>
	inline Enum GetCurrentState()
	{
		const uint8 e = GetCurrentStateEnum();
		return static_cast<Enum>(e);
	}
		
	FORCEINLINE void SetEnumToStrConverter(EnumToStrConverter conv)
	{
		if (!conv) return;
		m_EnumToStrConverter = conv;
	}
	
protected:
	bool CanTransit(uint8 origState, uint8 destState) const;

	template<class Enum>
	bool CanTransit(Enum os, Enum ds) const;
	UPROPERTY()//GC will handle this
	UStateBlackboard* StateBlackBoard;

	UPROPERTY()
	UStateBlackboard* GlobalBlackBoard;

	UPROPERTY(EditAnywhere, Category = "State Manager Debug")
	bool EnableDebug = false;

	UPROPERTY(EditAnywhere, Category = "State Manager Debug")
	FName InternalName = FName("My State manager");

	UPROPERTY(EditAnywhere, Category = "State Manager Debug")
	int32 CurrentMsgIndex;

	UPROPERTY(EditAnywhere, Category = "State Manager Debug")
	int32 PrevMsgIndex;

	UPROPERTY(EditAnywhere, Category = "State Manager Debug")
	FColor DebugColor = FColor::Green;

	UPROPERTY(EditAnywhere, Category = "State Manager Debug")
	float DisplayTime = 0.7f;

	UPROPERTY(EditAnywhere, Category = "State Manager Debug")
	uint8 DebugState;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

private:
	UPROPERTY()
	TMap<uint8, UStateComponentBase*> m_EnumStateMap;

	UPROPERTY()
	UStateComponentBase* CurrentState;

	uint8 PrevState;

	FRectMatrix<bool> StateMatrix;

	EnumToStrConverter m_EnumToStrConverter;

	void Debug();
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
