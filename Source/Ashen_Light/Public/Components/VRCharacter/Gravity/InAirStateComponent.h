// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/VRCharacter/Gravity/VRCGravityStateComponentBase.h"
#include "InAirStateComponent.generated.h"

/**
 * 
 */
UCLASS()
class ASHEN_LIGHT_API UInAirStateComponent : public UVRCGravityStateComponentBase
{
	GENERATED_BODY()
public:
	UInAirStateComponent(const FObjectInitializer& init);
	void BeginPlay() override;
	virtual void OnStateTick(float DeltaTime) override;
	virtual void OnStateEnter() override;
	virtual void OnStateExit() override;
protected:

	UPROPERTY(EditAnywhere, Category = "In Air State", meta = (DisplayName = "Gravity Constant g"))
	float GravityConstant = 980.f; //cm / sec^2

	UPROPERTY(EditAnywhere, Category = "In Air State", meta = (DisplayName = "Max Fall Velocity"))
	float MaxFallVelocity = 2000.f; //cm / sec

	UPROPERTY(EditAnywhere, Category = "In Air State", meta = (DisplayName = "Terminal Velocity"))
	float TerminalVelocity = -4000.f; //cm / sec
	
private:
	float VerticalVelocity;
	FVector LastHorizontalVelocity;
	void ApplyGravity(float DeltaTime);
};
