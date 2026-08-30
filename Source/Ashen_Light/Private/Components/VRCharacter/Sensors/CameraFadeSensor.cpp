// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/VRCharacter/Sensors/CameraFadeSensor.h"
#include "DrawDebugHelpers.h"

// Sets default values for this component's properties
UCameraFadeSensor::UCameraFadeSensor(const FObjectInitializer& init) : Super(init)
{
}

void UCameraFadeSensor::DoScan(float DeltaTime)
{
	if (!CanScan()) return;
	UWorld* w = GetWorld();
	if (!w) return;
	FVector start = GetTrackingComponent()->GetComponentLocation() + SensorOffset;
	FVector end = start - FVector(0.f, 0.f, m_MaxScanDistance * FadeCheckDistanceRatio);
	FHitResult outHit;
	const bool bHit = DoScanInternal(w, outHit, start, end, 
		FQuat::Identity, ECollisionChannel::ECC_WorldStatic, 
		FCollisionShape::MakeSphere(FadeCheckRadius), false, ActorsToIgnore);
	if (bHit && outHit.bBlockingHit)
	{
		Hit = outHit;
		ObjectHit = true;
	}
	else
	{
		ObjectHit = false;
	}

	Debug(w, start, end, FadeCheckRadius);
}


