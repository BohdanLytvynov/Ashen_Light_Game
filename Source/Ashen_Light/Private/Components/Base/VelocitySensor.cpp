// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Base/VelocitySensor.h"

UVelocitySensor::UVelocitySensor(const FObjectInitializer& init) : Super(init)
{
}

void UVelocitySensor::DoScan(float DeltaTime)
{
	USceneComponent* component = GetTrackingComponent();
	if (!component || FMath::IsNearlyZero(DeltaTime)) return;
	FVector compLocation = GetComponentPositionInternal();
	FVector ds = compLocation - m_PrevPosition;
	//v = ds/dt. ds = |start - end|, as dt -> 0 we get the Instant Velocity
	m_Velocity = ds / DeltaTime;
	m_PrevPosition = compLocation;
	Debug(component->GetName());
}

void UVelocitySensor::SyncPosition()
{
	m_PrevPosition = GetComponentPositionInternal();
}

void UVelocitySensor::ConfigureSpace(ERelativeTransformSpace space)
{
	SpaceType = (uint8)space;
}

void UVelocitySensor::Debug(const FString& compName)
{
	if (!EnableDebug) return;
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(MsgDebugIndex, MsgDebugDuration, MsgDebugColor, FString::Printf(TEXT("%s velocity: %s"), *compName, *m_Velocity.ToString()));
	}
}

FVector UVelocitySensor::GetComponentPositionInternal()
{
	USceneComponent* component = GetTrackingComponent();
	switch (SpaceType)
	{
	case RTS_World:
		return component->GetComponentLocation();
	case RTS_Component:
		return component->GetRelativeLocation();
	}

	return FVector::ZeroVector;
}
