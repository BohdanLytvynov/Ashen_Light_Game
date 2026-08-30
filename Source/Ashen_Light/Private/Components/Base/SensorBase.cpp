// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Base/SensorBase.h"

// Sets default values for this component's properties
USensorBase::USensorBase(const FObjectInitializer& init) : Super(init)
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	// ...
}

bool USensorBase::CanScan() const
{
	return 	m_component != nullptr;
}

