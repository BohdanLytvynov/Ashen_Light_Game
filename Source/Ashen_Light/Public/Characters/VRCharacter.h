// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/CharacterBase.h"
#include "VRCharacter.generated.h"
/**
 * 
 */
UENUM()
enum class EVRCharacterState
{
	VRCS_FreeRoam UMETA(DisplayName = "Free Roam State"),
	VRCS_Battle UMETA(DisplayName = "Battle State")
};

UCLASS()
class ASHEN_LIGHT_API AVRCharacter : public APawn
{
	GENERATED_BODY()
	
	AVRCharacter(const FObjectInitializer& init);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

#pragma region Components

	UPROPERTY(VisibleAnywhere, meta = (Tooltip = "Origin of the Tracking Space. Camera Component moves in this coordinate space."))
	class USceneComponent* TrackingSpaceOrigin;

	UPROPERTY(VisibleAnywhere, meta=(Tooltip = "VRCamera that will follow the VR Device in a Tracking Space"))
	class UCameraComponent* CameraComponent;

	UPROPERTY(VisibleDefaultsOnly, meta = (Tooltip = "Collision Capsule"))
	class UCapsuleComponent* CapsuleCollisionComponent;

	UPROPERTY(VisibleAnywhere)
	class UMotionControllerComponent* RightMotionController;

	UPROPERTY(VisibleAnywhere)
	UMotionControllerComponent* LeftMotionController;

	UPROPERTY(VisibleAnywhere)
	class UWidgetInteractionComponent* WidgetInteractionLeft;

	UPROPERTY(VisibleAnywhere)
	UWidgetInteractionComponent* WidgetInteractionRight;

	UPROPERTY(VisibleAnywhere)
	class UDecalComponent* DeadZoneDecalComponent;

	UPROPERTY(VisibleAnywhere)
	UDecalComponent* ActiveZoneComponent;

	UPROPERTY(VisibleAnywhere)
	UDecalComponent* PlayerAnchorDecalComponent;

	UPROPERTY(VisibleAnywhere)
	class UFloatingPawnMovement* PawnMovement;

	UPROPERTY(VisibleAnywhere)
	class UStaticMeshComponent* CameraFadeComponent;

	UPROPERTY(VisibleAnywhere)
	USkeletalMeshComponent* SkeletalMeshComponent;

	UPROPERTY(Transient)
	class UMaterialInstanceDynamic* FadeDynamicMaterial;

	UPROPERTY(EditDefaultsOnly)
	class UMaterialInterface* FadeMaterialBase;

#pragma endregion

#pragma region UPROPERTIES

	UPROPERTY(EditAnywhere, Category = "VR Locomotion Decals", meta = (DisplayName = "Locomotion Dead Zone Radius", ClampMin = "0.001", UIMin = "0.001"))
	float DeadZoneRadius = 40.f;

	UPROPERTY(EditAnywhere, Category = "VR Locomotion Decals", meta = (DisplayName = "Locomotion Dead Zone Height", ClampMin = "0.001", UIMin = "0.001"))
	float DeadZoneHeight = 300.f;
	
	UPROPERTY(EditAnywhere, Category = "VR Locomotion Decals", meta = (DisplayName = "Locomotion Active Zone Radius", ClampMin = "0.001", UIMin = "0.001"))
	float ActiveZoneRadius = 120.f;

	UPROPERTY(EditAnywhere, Category = "VR Locomotion Decals", meta = (DisplayName = "Locomotion Active Zone Height", ClampMin = "0.001", UIMin = "0.001"))
	float ActiveZoneHeight = 300.f;
	
	UPROPERTY(EditAnywhere, Category = "VR Locomotion Decals", meta = (DisplayName = "Player Anchor Radius", ClampMin = "0.001", UIMin = "0.001"))
	float PlayerAnchorZoneRadius = 60.f;

	UPROPERTY(EditAnywhere, Category = "VR Locomotion Decals", meta = (DisplayName = "Locomotion Player Anchor Height", ClampMin = "0.001", UIMin = "0.001"))
	float PlayerAnchorZoneHeight = 300.f;
		
	UPROPERTY(EditAnywhere, Category = "VR Locomotion", meta = (DisplayName = "Swinging Threshold", ClampMin = "0.001", UIMin = "0.001"))
	float SwiningThreshold = 40.f;

	UPROPERTY(EditAnywhere, Category = "VR Locomotion", meta = (DisplayName = "Walk Speed"))
	float walkSpeed = 200.f;

	UPROPERTY(EditAnywhere, Category = "VR Locomotion", meta = (DisplayName = "Run Speed"))
	float runSpeed = 300.f;

	UPROPERTY(EditAnywhere, Category = "VR Locomotion", meta = (DisplayName = "Ground Detection Threshold", Tooltip = "Value that will be added to the end point of the Sphere Trace for Ground Detection"))
	float GroundDetectionThreshold = 10.f;

	UPROPERTY(EditAnywhere, Category = "VR Locomotion Gravity", meta = (DisplayName = "Gravity Constant g"))
	float GravityConstant = -980.f; //cm / sec^2

	UPROPERTY(EditAnywhere, Category = "VR Locomotion Gravity", meta = (DisplayName = "Max Fall Velocity"))
	float MaxFallVelocity = 2000.f; //cm / sec

	UPROPERTY(EditAnywhere, Category = "VR Locomotion Gravity", meta = (DisplayName = "Terminal Velocity"))
	float TerminalVelocity = -4000.f; //cm / sec

	UPROPERTY(EditAnywhere, Category = "VR Locomotion Obstacle Detection", meta = (DisplayName = "Obstacle Distance Detection", ClampMin = "0.001", UIMin = "0.001"))
	float ObstacleDistanceDetection = 140.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Locomotion Obstacle Detection", meta = (ClampMin = "0.0", ClampMax = "90.0"))
	float StopMovementAngleThreshold = 20.0f;

	UPROPERTY(EditAnywhere, Category = "VR Camera Fade", meta = (DisplayName = "Fade Chech Radius", ClampMin = "0.001", UIMin = "0.001"))
	float FadeCheckRadius = 14.f;

	UPROPERTY(EditAnywhere, Category = "VR Camera Fade", meta = (DisplayName = "Camera Fade Distance", ClampMin = "0.001", UIMin = "0.001"))
	float CameraFadeDistance = 5.f;

	UPROPERTY(EditAnywhere, Category = "VR IK", meta = (Tooltip = "Name of the head bone in Skeletal Mesh."))
	FString HeadBoneName = "head";

	UPROPERTY()
	AActor* CurrentObstacle = nullptr;

#pragma endregion

#pragma region UFunctions
	UFUNCTION()
	void OnHMD_Recentered();

	UFUNCTION(BlueprintCallable)
	void RecenterTrackingSpaceToLocation(FVector TargetWorldLocation);
	
	UFUNCTION(BlueprintCallable)
	void RecenterTrackingSpaceToActor();
#pragma endregion


#pragma region C++ Functions	
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	void OnToggleBattleModePressed();
	void OnSprintPressed();
	void OnSprintReleased();
	void RecalibrateCapsuleComponent();
	void ConfigureDecal(UDecalComponent* decal, float thicknes, float r, bool redraw = false);
	virtual void HandleMovement(float DeltaTime);
	bool IsSwiningArms(float DeltaTime);
	float GetMotionControllerSpeed(UMotionControllerComponent* motionController, FVector* prevMotionControllerPosition, float DeltaTime);
	virtual void Stop();
	virtual void Walk();
	virtual void Run();
	void DetectGround();
	void ApplyGravity(float DeltaTime);
	FVector AdjustInputForSlope(FVector InputVector) const;
	void CheckObstacles();
	void CheckCameraFade(float DeltaTime);
	void ApplyCameraFade();
	void DrawMsg(const FString& msg);
	void CalculateCurrentVelocity(float DeltaTime);
#pragma endregion

#pragma region Editor Callable functions

#if WITH_EDITOR

	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

#endif

#pragma endregion

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	float GetGroundVelocityRatio() const;

	FORCEINLINE UCameraComponent* GetVRCamera() const
	{
		return CameraComponent;
	}

	FORCEINLINE USkeletalMeshComponent* GetMesh() const
	{
		return SkeletalMeshComponent;
	}

	FORCEINLINE UMotionControllerComponent* GetLeftMotionController() const
	{
		return LeftMotionController;
	}

	FORCEINLINE UMotionControllerComponent* GetRightMotionController() const
	{
		return RightMotionController;
	}

private:
	float leftHandSpeed;
	float rightHandSpeed;

	FVector prevLeftHandLocation;
	FVector prevRightHandLocation;

	bool bIsObstacleHit;
	bool bIsGrounded;
	FHitResult CurrentGroundHit;
	FHitResult CurrentObstacleHit;
	float VerticalVelocity = 0.f;
	bool bCanPerformBattleStep;
	bool bCanRun;
	bool bCameraInAMesh;

	EVRCharacterState CurrentCharacterState;
	FVector CurrentVelocity;
	FVector PrevCameraPosition;

	float CurrentCameraFadeOpacity;
};
