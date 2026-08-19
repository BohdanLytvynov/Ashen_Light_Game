// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateBlackboardBase.h"

/**
 * 
 */
class ASHEN_LIGHT_API FGravityStateBlackBoard : public FStateBlackboardBase
{
public:
	FGravityStateBlackBoard() = default;
	virtual ~FGravityStateBlackBoard() = default;

	bool CameraInMesh;
	FHitResult CameraInMeshHit;
};
