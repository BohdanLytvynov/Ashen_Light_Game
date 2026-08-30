// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/VRCharacter/Base/VRCharacterComponentBase.h"
#include "VRCGravityStateComponentBase.generated.h"

/**
 * 
 */
UCLASS()
class ASHEN_LIGHT_API UVRCGravityStateComponentBase : public UVRCharacterComponentBase
{
	GENERATED_BODY()
public:
	UVRCGravityStateComponentBase(const FObjectInitializer& init);
protected:
	bool CheckAndHandleCameraInMesh(IVRCharacterInterface* context, UStateManagerComponent* stateManager);
	bool CheckAndHandleGroundHit(IVRCharacterInterface* context, UStateManagerComponent* stateManager, float verticalVelocity);
	bool CheckAndHandleClimbing(IVRCharacterInterface* context, UStateManagerComponent* stateManager);
	bool CheckAndHandleInAir(IVRCharacterInterface* context, UStateManagerComponent* stateManager);
};
