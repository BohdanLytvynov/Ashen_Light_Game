// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SensorBase.generated.h"


UCLASS(Abstract, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ASHEN_LIGHT_API USensorBase : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USensorBase(const FObjectInitializer& init);

	virtual void DoScan(TArray<AActor*> ignoredActors) {}
	virtual void UpdateSensorPosition(const FVector& pos);
protected:
	FORCEINLINE FVector GetPosition()
	{
		return m_SensorPosition;
	}

	UPROPERTY(EditAnywhere, Category = "Sensor Debug")
	bool EnableDebug = false;

private:
	FVector m_SensorPosition;
};
