// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/Base/HitSensor.h"
#include "MotionControllerHitSensor.generated.h"

/**
 * 
 */
UCLASS()
class ASHEN_LIGHT_API UMotionControllerHitSensor : public UHitSensor
{
	GENERATED_BODY()
public:
	UMotionControllerHitSensor(const FObjectInitializer& init);
	void DoScan(float DeltaTime) override;
protected:
	UPROPERTY(EditAnywhere, Category = "Motion Controller Sensor")
	float ScanDistance = 70.f;

	UPROPERTY(EditAnywhere, Category = "Motion Controller Sensor")
	float TraceSphereRadius = 10.f;
};
