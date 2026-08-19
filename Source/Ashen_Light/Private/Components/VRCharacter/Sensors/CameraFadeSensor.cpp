// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/VRCharacter/Sensors/CameraFadeSensor.h"
#include "DrawDebugHelpers.h"

// Sets default values for this component's properties
UCameraFadeSensor::UCameraFadeSensor(const FObjectInitializer& init) : Super(init)
{
}

void UCameraFadeSensor::DoScan(TArray<AActor*> ignoredActors)
{
	UWorld* w = GetWorld();
	if (!w) return;
	FVector start = GetPosition();
	FVector end = start - FVector(0.f, 0.f ,m_MaxScanDistance * FadeCheckDistanceRatio);
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActors(ignoredActors);
	QueryParams.bTraceComplex = false;
	FHitResult outHit;
	const bool bHit = w->SweepSingleByChannel(
		outHit,
		start,
		end,
		FQuat::Identity,
		ECC_WorldStatic,
		FCollisionShape::MakeSphere(FadeCheckRadius),
		QueryParams
	);

	if (bHit && outHit.bBlockingHit)
	{
		Hit = outHit;
		ObjectHit = true;
	}
	else
	{
		ObjectHit = false;
	}

	if (!EnableDebug) return;

	if (ObjectHit)
	{
		DrawDebugSphere(w, outHit.ImpactPoint, FadeCheckRadius, 8, TraceHitColor, false, DebugDrawTime);
	}
	
	DrawDebugSphere(w, start, FadeCheckRadius, 8, TraceColorStart, false, DebugDrawTime);
	DrawDebugSphere(w, end, FadeCheckRadius, 8, TraceColorEnd, false, DebugDrawTime);
}


