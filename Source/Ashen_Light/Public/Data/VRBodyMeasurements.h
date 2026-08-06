// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "VRBodyMeasurements.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct ASHEN_LIGHT_API FVRBodyMeasurements
{
public:
	GENERATED_BODY();

	FVRBodyMeasurements();
	~FVRBodyMeasurements();

#pragma region UProperties

	UPROPERTY(EditAnywhere, Category = "VR Real Body Bones Dimensions", meta = (DisplayName = "Shoulder Bone Length"))
	float LowerArmLength = 30.f;

	UPROPERTY(EditAnywhere, Category = "VR Real Body Bones Dimensions", meta = (DisplayName = "Forearm Bone Length"))
	float HandLength = 30.f;

	UPROPERTY(EditAnywhere, Category = "VR Real Body Bones Dimensions", meta = (DisplayName = "Thigh Bone Length"))
	float CalfLength = 40.f;

	UPROPERTY(EditAnywhere, Category = "VR Real Body Bones Dimensions", meta = (DisplayName = "Shin Bone Length"))
	float FootLength = 50.f;

	UPROPERTY(EditAnywhere, Category = "VR Real Body Bones Dimensions", meta = (DisplayName = "Clavicle Bone ength"))
	float ClavicleLength = 15.f;

	UPROPERTY(EditAnywhere, Category = "VR Real Body Bones Dimensions", meta = (DisplayName = "Сoccyx to Atlas Length"))
	float VertebralColumnLength = 57.f;

	UPROPERTY(EditAnywhere, Category = "VR Real Body Bones Dimensions", meta = (DisplayName = "Distance from pelvis to the hip joint"))
	float ThighLength = 20.f;

#pragma endregion

};
