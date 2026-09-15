// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/VRCharacter/Sensors/MotionControllerHitSensor.h"

UMotionControllerHitSensor::UMotionControllerHitSensor(const FObjectInitializer& init) : Super(init)
{
	CollisionChannels.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_WorldDynamic));
}

void UMotionControllerHitSensor::DoScan(float DeltaTime)
{
	if (!CanScan()) return;
	UWorld* w = GetWorld();
	if (!w) return;	
	USceneComponent* comp = GetTrackingComponent();
	if (!comp) return;
	FVector start = comp->GetComponentLocation();
	FVector dir = comp->GetUpVector() * -1.f;
	FVector end = start + dir * ScanDistance;
	FCollisionShape shape = FCollisionShape::MakeSphere(TraceSphereRadius);
	FHitResult outHit;
	const bool hit = DoScanInternal(w, outHit, start, end, FQuat::Identity, shape, false);

	if (hit && outHit.bBlockingHit)
	{
		ObjectHit = true;
		Hit = outHit;
	}
	else
	{
		ObjectHit = false;
	}
	if (!EnableDebug) return;
	Debug(w, start, end, TraceSphereRadius);
}
