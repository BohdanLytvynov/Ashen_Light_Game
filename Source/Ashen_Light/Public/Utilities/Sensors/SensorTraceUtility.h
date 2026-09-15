// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class ASHEN_LIGHT_API FSensorTraceUtility
{
private:
	FSensorTraceUtility() = delete;
	~FSensorTraceUtility() = delete;
public:
	static bool PerformTraceWithSensorConfig(class UHitSensor* sensorTemplate, const FVector& start, const FVector& end, const FQuat& q, const FCollisionShape& collShape, bool traceComplex, FHitResult& hit, bool debug = false);
};
