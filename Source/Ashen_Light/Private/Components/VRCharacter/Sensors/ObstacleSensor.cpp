// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/VRCharacter/Sensors/ObstacleSensor.h"
#include "Components/CapsuleComponent.h"
#include "DrawDebugHelpers.h"

UObstacleSensor::UObstacleSensor(const FObjectInitializer& init) : Super(init)
{
}

void UObstacleSensor::DoScan(float DeltaTime)
{
	if (!CanScan()) return;
	UWorld* w = GetWorld();
	if (!w) return;
	USceneComponent* comp = GetTrackingComponent();
	if (!comp) return;
	UCapsuleComponent* caps = Cast<UCapsuleComponent>(comp);
	if (!caps) return;
	FHitResult hitResult;
	FVector start = caps->GetComponentLocation();
	FVector normDirection = m_ScanDirection.GetSafeNormal();
	FVector end = start + normDirection * ObstackeScanDistance;
	float radius = caps->GetScaledCapsuleRadius() * CollisionShapeRadiusScaleFactor;
	float halfHeight = caps->GetScaledCapsuleHalfHeight() * CollisionShapeHeightScaleFactor;
	m_CapsHalfHeight = halfHeight;
	FCollisionShape capsule = FCollisionShape::MakeCapsule(radius, halfHeight);
	bool hit = DoScanInternal(w, hitResult, start, end, FQuat::Identity, capsule, false);
	if (hit && hitResult.bBlockingHit)
	{
		Hit = hitResult;
		ObjectHit = hit;
	}
	else
	{
		ObjectHit = false;
	}
	if (!EnableDebug) return;
	Debug(w, start, end, radius);
}

void UObstacleSensor::Debug(UWorld* w, const FVector& start, const FVector& end, float traceShapeRadius) const
{
	if (!w) return;
	if (ObjectHit)
	{
		DrawDebugCapsule(w, Hit.ImpactPoint, m_CapsHalfHeight, traceShapeRadius, FQuat::Identity, TraceHitColor, false, DebugDrawTime);
	}
	DrawDebugCapsule(w, start, m_CapsHalfHeight, traceShapeRadius, FQuat::Identity, TraceColorStart, false, DebugDrawTime);
	DrawDebugCapsule(w, end, m_CapsHalfHeight, traceShapeRadius, FQuat::Identity, TraceColorEnd, false, DebugDrawTime);
}
