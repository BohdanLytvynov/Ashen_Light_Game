// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/Base/HitSensor.h"
#include "GroundHitSensor.generated.h"

/**
 * 
 */
UCLASS()
class ASHEN_LIGHT_API UGroundHitSensor : public UHitSensor
{
	GENERATED_BODY()
public:
	UGroundHitSensor(const FObjectInitializer& init);

	void DoScan(float DeltaTime) override;
	FORCEINLINE void UpdateCapsuleHalfHeight(float halfHeight)
	{
		CapsuleHalfHeight = halfHeight;
	}
protected:
	virtual bool DoScanInternalMulti(UWorld* w, TArray<FHitResult>& outHit, const FVector& start,
		const FVector& end, const FQuat& quat, ECollisionChannel channel,
		const FCollisionShape& shape, bool traceComplex, const TArray<AActor*>* ignoredActors) const;

	UPROPERTY(EditAnywhere, Category = "Ground Sensor", meta = (DisplayName = "Ground Detection Threshold", Tooltip = "Value that will be added to the end point of the Sphere Trace for Ground Detection"))
	float GroundDetectionThreshold = 10.f;

	UPROPERTY(EditAnywhere, Category = "Ground Sensor", meta = (ToolTip = "Radius of the Trace Sphere that will be sent bottom to the ground"))
	float GroundTraceSphere = 20.f;
private:
	float CapsuleHalfHeight;
};
