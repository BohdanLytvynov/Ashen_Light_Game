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
	//We allow this class to have full access to this base sensor
	friend class FSensorTraceUtility;
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

	FORCEINLINE AActor* GetHitActor() const
	{
		return Hit.GetActor();
	}

	FORCEINLINE void SetIgnoredActors(TArray<AActor*>* ignoredActors)
	{
		ActorsToIgnore = ignoredActors;
	}

	FORCEINLINE void AddIgnoredActor(AActor* actor)
	{
		if (!actor || !ActorsToIgnore) return;
		ActorsToIgnore->AddUnique(actor);
	}

	FORCEINLINE void RemoveIgnoredActor(AActor* actor)
	{
		if (!actor || !ActorsToIgnore) return;
		ActorsToIgnore->Remove(actor);
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

	UPROPERTY(EditAnywhere, Category = "Sensor Debug")
	float DebugSphereSegments = 8.f;

	UPROPERTY(EditAnywhere, Category = "Sensor Config")
	FVector SensorOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sensor Config")
	TArray<TEnumAsByte<EObjectTypeQuery>> CollisionChannels;

	FHitResult Hit;
	bool ObjectHit;

	bool DoScanInternal(UWorld* w, FHitResult& outHit, const FVector& start, 
		const FVector& end, const FQuat& quat, 
		const FCollisionShape& shape, bool traceComplex) const;

	FORCEINLINE void ConfigureCollisionQueryParams(FCollisionQueryParams& params) const;
	FORCEINLINE void ConfigureCollisionObjectQueryParams(FCollisionObjectQueryParams& params) const;

	virtual void Debug(UWorld* w, const FVector& start, const FVector& end, float traceShapeRadius) const;

	TArray<AActor*>* ActorsToIgnore;
};
