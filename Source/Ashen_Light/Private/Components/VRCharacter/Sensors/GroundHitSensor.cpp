// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/VRCharacter/Sensors/GroundHitSensor.h"

UGroundHitSensor::UGroundHitSensor(const FObjectInitializer& init) : Super(init)
{
	
}

void UGroundHitSensor::DoScan(float DeltaTime)
{
	if (!CanScan()) return;
	UWorld* w = GetWorld();
	if (!w) return;
	FVector start = GetTrackingComponent()->GetComponentLocation();
	FVector end = start - FVector(0.f, 0.f, (CapsuleHalfHeight * 2 + GroundDetectionThreshold));
	TArray<FHitResult> outHits;
	const bool hitAny = DoScanInternalMulti(w, outHits, start, end, FQuat::Identity, 
		ECollisionChannel::ECC_WorldStatic, FCollisionShape::MakeSphere(GroundTraceSphere), false, ActorsToIgnore);
	ObjectHit = false;
	if (hitAny)
	{
		for (const FHitResult& hitRes : outHits)
		{
			if (hitRes.bBlockingHit && hitRes.ImpactNormal.Z > 0.5f)
			{
				Hit = hitRes;
				ObjectHit = true;
				break;
			}
		}
	}
	Debug(w, start, end, GroundTraceSphere);
}

bool UGroundHitSensor::DoScanInternalMulti(UWorld* w, TArray<FHitResult>& outHit, const FVector& start, const FVector& end, const FQuat& quat, ECollisionChannel channel, const FCollisionShape& shape, bool traceComplex, const TArray<AActor*>* ignoredActors) const
{
	if (!w) return false;
	FCollisionQueryParams QueryParams;
	if (ignoredActors && ignoredActors->Num() > 0)
	{
		QueryParams.AddIgnoredActors(*ignoredActors);
	}
	QueryParams.bTraceComplex = traceComplex;
	return w->SweepMultiByChannel(outHit, start, end, quat, channel,
		shape, QueryParams);
}
