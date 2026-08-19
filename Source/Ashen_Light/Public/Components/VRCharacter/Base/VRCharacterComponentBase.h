// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../../Base/StateComponentBase.h"
#include "../../../../Public/Interfaces/VRCharacterInterface.h"
#include "VRCharacterComponentBase.generated.h"

/**
 * 
 */
UCLASS()
class ASHEN_LIGHT_API UVRCharacterComponentBase : public UStateComponentBase
{
	GENERATED_BODY()
public:
	UVRCharacterComponentBase(const FObjectInitializer& init);
protected:
	IVRCharacterInterface* GetContext() override;
private:
	IVRCharacterInterface* vrCharacter;
};
