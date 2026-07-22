// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CharacterBase.generated.h"

UCLASS(Abstract)
class ASHEN_LIGHT_API ACharacterBase : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ACharacterBase(const FObjectInitializer& init);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

#pragma region UPROPERTIES

	UPROPERTY(EditAnywhere, Category = "Locomotion", meta = (DisplayName = "Walk Speed"))
	float walkSpeed = 150.f;

	UPROPERTY(EditAnywhere, Category = "Locomotion", meta = (DisplayName = "Run Speed"))
	float runSpeed = 300.f;

#pragma region Blueprint Read Only

	UPROPERTY(BlueprintReadOnly)
	float GroundSpeed = 0.f;

#pragma endregion

#pragma endregion

#pragma region C++ Functions
	virtual void UpdateGroundSpeed();
	virtual void Walk() {}
	virtual void Stop() {}
	virtual void Run() {}
	FVector GetLocalPositionRelativeToOrigin(USceneComponent* origin, USceneComponent* component);
#pragma endregion


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
