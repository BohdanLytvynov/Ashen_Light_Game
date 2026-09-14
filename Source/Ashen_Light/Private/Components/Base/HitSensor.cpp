// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Base/HitSensor.h"
#include "DrawDebugHelpers.h"
#include "Utilities/Sensors/SensorTraceUtility.h"

// Sets default values for this component's properties
UHitSensor::UHitSensor(const FObjectInitializer& init) : Super(init)
{
	ObjectHit = false;
	CollisionChannels.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_WorldStatic));
}

bool UHitSensor::DoScanInternal(UWorld* w, FHitResult& outHit, 
	const FVector& start, const FVector& end, 
	const FQuat& quat, 
	const FCollisionShape& shape, bool traceComplex) const
{
	if (!w) return false;
	FCollisionQueryParams QueryParams;
	ConfigureCollisionQueryParams(QueryParams);
	QueryParams.bTraceComplex = traceComplex;
	FCollisionObjectQueryParams CollisionObjectQueryParams;
	ConfigureCollisionObjectQueryParams(CollisionObjectQueryParams);
	return w->SweepSingleByObjectType(outHit, start, end, quat, CollisionObjectQueryParams,
		shape, QueryParams);
}

void UHitSensor::ConfigureCollisionQueryParams(FCollisionQueryParams& params) const
{
	if (ActorsToIgnore && ActorsToIgnore->Num() > 0)
	{
		params.AddIgnoredActors(*ActorsToIgnore);
	}
}

void UHitSensor::ConfigureCollisionObjectQueryParams(FCollisionObjectQueryParams& params) const
{
	for (const EObjectTypeQuery ObjectType : CollisionChannels)
	{
		ECollisionChannel CollisionChannel = UEngineTypes::ConvertToCollisionChannel(ObjectType);
		if (CollisionChannel != ECC_MAX)
		{
			params.AddObjectTypesToQuery(CollisionChannel);
		}
	}
}

void UHitSensor::Debug(UWorld* w, const FVector& start, const FVector& end,  float traceShapeRadius) const
{
	if (!w) return;
	if (ObjectHit)
	{
		DrawDebugSphere(w, Hit.ImpactPoint, traceShapeRadius, DebugSphereSegments, TraceHitColor, false, DebugDrawTime);
	}
	DrawDebugSphere(w, start, traceShapeRadius, DebugSphereSegments, TraceColorStart, false, DebugDrawTime);
	DrawDebugSphere(w, end, traceShapeRadius, DebugSphereSegments, TraceColorEnd, false, DebugDrawTime);
}






