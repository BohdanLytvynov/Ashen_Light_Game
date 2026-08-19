// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SensorBase.h"
#include "HitSensor.generated.h"


UCLASS(Abstract, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ASHEN_LIGHT_API UHitSensor : public USensorBase
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UHitSensor(const FObjectInitializer& init);
	FORCEINLINE FHitResult GetHit() const
	{
		return Hit;
	}

	FORCEINLINE bool IsHit() const
	{
		return ObjectHit;
	}
protected:
	UPROPERTY(EditAnywhere, Category = "Sensor Debug")
	FColor TraceColorStart = FColor::Green;

	UPROPERTY(EditAnywhere, Category = "Sensor Debug")
	FColor TraceColorEnd = FColor::Blue;

	UPROPERTY(EditAnywhere, Category = "Sensor Debug")
	FColor TraceHitColor = FColor::Red;

	UPROPERTY(EditAnywhere, Category = "Sensor Debug")
	float DebugDrawTime = 1.f;

	FHitResult Hit;
	bool ObjectHit;
};
