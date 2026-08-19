// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/VRCharacter/Base/VRCharacterComponentBase.h"

UVRCharacterComponentBase::UVRCharacterComponentBase(const FObjectInitializer& init) : Super(init)
{
	
}

IVRCharacterInterface* UVRCharacterComponentBase::GetContext()
{
	if (!vrCharacter)
	{
		IStateDriven* state = UStateComponentBase::GetContext();
		if (!state) return vrCharacter;
		vrCharacter = Cast<IVRCharacterInterface>(state);
	}
	return vrCharacter;
}