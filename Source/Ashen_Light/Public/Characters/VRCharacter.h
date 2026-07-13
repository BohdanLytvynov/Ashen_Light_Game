// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/CharacterBase.h"
#include "VRCharacter.generated.h"

/**
 * 
 */
UCLASS()
class ASHEN_LIGHT_API AVRCharacter : public ACharacterBase
{
	GENERATED_BODY()
	
	AVRCharacter(const FObjectInitializer& init);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

#pragma region Components

	UPROPERTY(VisibleAnywhere)
	class USceneComponent* VROriginComponent;

	UPROPERTY(VisibleAnywhere)
	class UCameraComponent* CameraComponent;

	UPROPERTY(VisibleAnywhere)
	class UMotionControllerComponent* RightMotionController;

	UPROPERTY(VisibleAnywhere)
	class UMotionControllerComponent* LeftMotionController;

#pragma endregion


public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
