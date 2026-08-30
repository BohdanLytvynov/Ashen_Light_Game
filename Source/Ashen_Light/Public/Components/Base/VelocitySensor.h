// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/Base/SensorBase.h"
#include "VelocitySensor.generated.h"

/**
 * 
 */
UCLASS()
class ASHEN_LIGHT_API UVelocitySensor : public USensorBase
{
	GENERATED_BODY()
public:
	UVelocitySensor(const FObjectInitializer& init);
	virtual void DoScan(float DeltaTime) override;
	void SyncPosition();
	void ConfigureSpace(ERelativeTransformSpace space);
	FORCEINLINE FVector GetVelocity() const
	{
		return m_Velocity;
	}
protected:
	UPROPERTY(VisibleAnywhere, Category = "Sensor Debug")
	int32 MsgDebugIndex = 1;
	UPROPERTY(EditAnywhere, Category = "Sensor Debug")
	float MsgDebugDuration = 5.f;
	UPROPERTY(EditAnywhere, Category = "Sensor Debug")
	FColor MsgDebugColor = FColor::Red;
	UPROPERTY(EditAnywhere, Category = "Velocity Sensor")
	uint8 SpaceType = ERelativeTransformSpace::RTS_World;
private:
	FVector m_Velocity;
	FVector m_PrevPosition;
	void Debug(const FString& compName);
	FVector GetComponentPositionInternal();
};
