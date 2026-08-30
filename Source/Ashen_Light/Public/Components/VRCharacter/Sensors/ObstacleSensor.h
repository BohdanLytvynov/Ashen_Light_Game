// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/Base/HitSensor.h"
#include "ObstacleSensor.generated.h"

/**
 * 
 */
UCLASS()
class ASHEN_LIGHT_API UObstacleSensor : public UHitSensor
{
	GENERATED_BODY()
public:
	UObstacleSensor(const FObjectInitializer& init);

	FORCEINLINE void SetScanDirection(const FVector& dir)
	{
		m_ScanDirection = dir;
	}
	void DoScan(float DeltaTime) override;
	void Debug(UWorld* w, const FVector& start, const FVector& end, float traceShapeRadius) const override;
protected:
	UPROPERTY(VisibleAnywhere, Category = "Obstacle Sensor")
	float ObstackeScanDistance = 20.f;

	UPROPERTY(VisibleAnywhere, Category = "Obstacle Sensor")
	float CollisionShapeHeightScaleFactor = 0.7f;
	UPROPERTY(VisibleAnywhere, Category = "Obstacle Sensor")
	float CollisionShapeRadiusScaleFactor = 1.f;
private:
	FVector m_ScanDirection;
	float m_CapsHalfHeight;//Used for Debug
};
