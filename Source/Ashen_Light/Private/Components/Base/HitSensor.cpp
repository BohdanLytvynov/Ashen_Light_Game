// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Base/HitSensor.h"
#include "DrawDebugHelpers.h"

// Sets default values for this component's properties
UHitSensor::UHitSensor(const FObjectInitializer& init) : Super(init)
{
	ObjectHit = false;
}

bool UHitSensor::DoScanInternal(UWorld* w, FHitResult& outHit, 
	const FVector& start, const FVector& end, 
	const FQuat& quat, ECollisionChannel channel, 
	const FCollisionShape& shape, bool traceComplex, const TArray<AActor*>* ignoredActors) const
{
	if (!w) return false;
	FCollisionQueryParams QueryParams;
	if (ignoredActors && ignoredActors->Num() > 0)
	{
		QueryParams.AddIgnoredActors(*ignoredActors);
	}
	QueryParams.bTraceComplex = traceComplex;
	return w->SweepSingleByChannel(outHit, start, end, quat, channel,
		shape, QueryParams);
}

void UHitSensor::Debug(UWorld* w, const FVector& start, const FVector& end,  float traceShapeRadius) const
{
	if (!EnableDebug) return;
	if (!w) return;
	if (ObjectHit)
	{
		DrawDebugSphere(w, Hit.ImpactPoint, traceShapeRadius, DebugSphereSegments, TraceHitColor, false, DebugDrawTime);
	}
	DrawDebugSphere(w, start, traceShapeRadius, DebugSphereSegments, TraceColorStart, false, DebugDrawTime);
	DrawDebugSphere(w, end, traceShapeRadius, DebugSphereSegments, TraceColorEnd, false, DebugDrawTime);
}






