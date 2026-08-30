// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/VRCharacter/Gravity/VRCGravityStateComponentBase.h"
#include "InMeshStateComponent.generated.h"

/**
 * 
 */
UCLASS()
class ASHEN_LIGHT_API UInMeshStateComponent : public UVRCGravityStateComponentBase
{
	GENERATED_BODY()
public:
	UInMeshStateComponent(const FObjectInitializer& init);
	void OnStateTick(float DeltaTime) override;
	void OnStateExit() override;
protected:
	UPROPERTY(EditAnywhere, Category = "In Mesh State")
	float CameraFadeDistance = 5.f;

	UPROPERTY(EditAnywhere, Category = "In Mesh State")
	float CameraFadeInterpConstant = 2.f;
private:
	bool m_IsGrounded;
	FHitResult m_CameraHit;
	bool m_CameraInMesh;
	void CollectSensorsData(IVRCharacterInterface* vrChar);
	void CalculateFadeOpacity(IVRCharacterInterface* vrChar, float DeltaTime);
	float CurrentCameraFadeOpacity;
};
