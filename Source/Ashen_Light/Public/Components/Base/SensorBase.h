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
	virtual void DoScan(float DeltaTime) {}
	virtual void SetTrackingComponent(USceneComponent* component)
	{
		m_component = component;
	}
	FORCEINLINE USceneComponent* GetTrackingComponent() const
	{
		return m_component;
	}
protected:
	UPROPERTY(EditAnywhere, Category = "Sensor Debug")
	bool EnableDebug = false;
	virtual bool CanScan() const;
private:
	USceneComponent* m_component;
};
