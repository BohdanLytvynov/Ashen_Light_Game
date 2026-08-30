// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "../../Base/HitSensor.h"
#include "CameraFadeSensor.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ASHEN_LIGHT_API UCameraFadeSensor : public UHitSensor
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCameraFadeSensor(const FObjectInitializer& init);
	virtual void DoScan(float DeltaTime) override;
	FORCEINLINE void SetMaxScanDistance(float maxDistance)
	{
		m_MaxScanDistance = maxDistance;
	}

	FORCEINLINE float GetFadeCheckRadius() const
	{
		return FadeCheckRadius;
	}
protected:

	UPROPERTY(EditAnywhere, Category = "Camera Sensor", meta = (ClampMin = "0.0", UIMin = "1.0"))
	float FadeCheckDistanceRatio = 0.4f;

	UPROPERTY(EditAnywhere, Category = "Camera Sensor", meta = (ClampMin = "0.001", UIMin = "0.001"))
	float FadeCheckRadius = 14.f;

private:
	float m_MaxScanDistance;
};
