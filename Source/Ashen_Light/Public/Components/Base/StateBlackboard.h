// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Misc/TVariant.h"
#include "UObject/WeakObjectPtr.h"
#include "GameplayTagContainer.h"
#include "StateBlackboard.generated.h"

using FBlackboardValue = TVariant <
	// --- 0. Empty State ---
	FEmptyVariantState,           // Represents "Value not set / None"
	// --- 1. Primitive Numeric & Logic Types ---
	bool,                       // Flags and conditional states
	int32,                      // Integers (counters, indices, enums)
	float,                      // Scalar values (distance, health, timer)
	// --- 2. Strings & Identifiers ---
	FName,                      // Fast tags and identifiers (O(1) comparison)
	FString,                    // Dynamic text, names, and debug strings
	FGameplayTag,               // Hierarchical state tags for gameplay systems
	// --- 3. Geometry & Math ---
	FVector,                    // World locations, velocity vectors
	FRotator,                   // Euler rotations
	FQuat,                      // Quaternions for complex transforms
	FTransform,                 // Full transform (Location + Rotation + Scale)
	// --- 4. Safe Object Pointers ---
	TWeakObjectPtr<UObject>,    // Weak pointer to any UObject (components, data assets)
	TWeakObjectPtr<AActor>      // Weak pointer to Actors (targets, enemies, points of interest)
>;

/**
 * 
 */
UCLASS()
class ASHEN_LIGHT_API UStateBlackboard : public UObject
{
	GENERATED_BODY()
public:
	UStateBlackboard() {}
	virtual ~UStateBlackboard() { Clear(); }

	template <typename T>
	void SetValue(const FString& Key, const T& Value)
	{
		m_blackboard.FindOrAdd(Key).Set<T>(Value);
	}
	
	template <typename T>
	void SetValue(const wchar_t* key, const T& Value)
	{
		SetValue(FString(key), Value);
	}

	template <typename T>
	bool TryGetValue(const FString& Key, T& OutValue) const
	{
		const FBlackboardValue* FoundValue = m_blackboard.Find(Key);

		if (!FoundValue)
		{
			UE_LOG(LogTemp, Warning, TEXT("[Blackboard] Key [%s] NOT found! Check sender logic."), *Key);
			return false;
		}

		if (FoundValue->IsType<T>())
		{
			OutValue = FoundValue->Get<T>();
			return true;
		}

		UE_LOG(LogTemp, Error, TEXT("[Blackboard] Type Mismatch for Key [%s]! Requested type does not match stored TVariant Index [%d]."),
			*Key,
			FoundValue->GetIndex());

		return false;
	}

	template<typename T>
	bool TryGetValue(const wchar_t* key, T& OutValue)
	{
		return TryGetValue(FString(key), OutValue);
	}

	void Clear() { m_blackboard.Empty(); }
private:
	TMap<FString, FBlackboardValue> m_blackboard;
};
