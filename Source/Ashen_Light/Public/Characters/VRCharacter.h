// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/CharacterBase.h"
#include "Enums.h"
#include "Interfaces/VRCharacterInterface.h"
#include "VRCharacter.generated.h"
/**
 * 
 */

UCLASS()
class ASHEN_LIGHT_API AVRCharacter : public APawn, public IVRCharacterInterface
{
	GENERATED_BODY()
	/// <summary>
	/// Ctor
	/// </summary>
	/// <param name="init">Object Initializer</param>
	AVRCharacter(const FObjectInitializer& init);

protected:
	/// <summary>
	/// Called when the game starts
	/// </summary>
	virtual void BeginPlay() override;

	/// <summary>
	/// Called when the game ends
	/// </summary>
	/// <param name="EndPlayReason"></param>
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
	
	UPROPERTY(EditDefaultsOnly, Category = "VR Camera Fade Material")
	class UMaterialInterface* FadeMaterialBase;
			
	//---State Managers---

	UPROPERTY(VisibleAnywhere)
	class UStateManagerComponent* LocomotionStateManager;

	UPROPERTY(VisibleAnywhere)
	UStateManagerComponent* GravityStateManager;

	UPROPERTY(VisibleAnywhere)
	class UStateBlackboard* GlobalStateBlackboard;

	//---State Components

	UPROPERTY(VisibleAnywhere)
	class UPhysicalMovementComponent* PhysicalMovementComponent;

	UPROPERTY(VisibleAnywhere)
	class UTrackingSpaceMovementComponent* TrackingSpaceMovementComponent;

	UPROPERTY(VisibleAnywhere)
	class UGroundedStateComponent* GroundedStateComponent;

	UPROPERTY(VisibleAnywhere)
	class UInAirStateComponent* InAirStateComponent;

	UPROPERTY(VisibleAnywhere)
	class UInMeshStateComponent* InMeshStateComponent;

	UPROPERTY(VisibleAnywhere)
	class UClimbingStateComponent* ClimbingStateComponent;

	//---Sensors---

	UPROPERTY(VisibleAnywhere)
	class UCameraFadeSensor* CameraFadeSensor;

	UPROPERTY(VisibleAnywhere)
	class UGroundHitSensor* GroundHitSensor;

	UPROPERTY(VisibleAnywhere)
	class UVelocitySensor* CameraVelocitySensor;

	UPROPERTY(VisibleAnywhere)
	UVelocitySensor* RightMotionControllerVelocitySensor;

	UPROPERTY(VisibleAnywhere)
	UVelocitySensor* LeftMotionControllerVelocitySensor;

	UPROPERTY(VisibleAnywhere)
	class UObstacleSensor* ObstacleSensor;

#pragma endregion

#pragma region UPROPERTIES

	UPROPERTY(EditAnywhere, Category = "VR Locomotion", meta = (DisplayName = "Walk Speed"))
	float walkSpeed = 200.f;

	UPROPERTY(EditAnywhere, Category = "VR Locomotion", meta = (DisplayName = "Run Speed"))
	float runSpeed = 300.f;

	UPROPERTY(EditAnywhere, Category = "VR Locomotion")
	float JumpWaitTimer = 5.f;
	
	UPROPERTY(EditAnywhere, Category = "VR Locomotion Thresholds")
	float CrouchThreshold = 10.f;
				
	UPROPERTY(EditAnywhere, Category = "VR Camera Fade", meta = ( ClampMin = "0.001", UIMin = "0.001"))
	float CameraFadeDistance = 5.f;
	
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

	UPROPERTY(EditAnywhere, Category = "VR Simulation", meta = (Tooltip = "Player height that will be used in Simulation"))
	float PlayerPreviewHeight = 186.f;

	UPROPERTY(EditAnywhere, Category = "VR Simulation")
	ELocomotionSpace PreviewLocomotionState;

	UPROPERTY(EditAnywhere, Category = "VR Simulation")
	bool EnableIkDebug = false;

	UPROPERTY(EditAnywhere, Category = "VR Simulation")
	bool EnableStateManagersDebug = false;
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
	/// <summary>
	/// Lazly get the VRCharacterAnimInstance
	/// </summary>
	/// <returns></returns>
	class UVRCharacterAnimInstance* GetCharAnimInstance();

	/// <summary>
	/// Set all Input actions binding here
	/// </summary>
	/// <param name="PlayerInputComponent">Component where we perform binding</param>
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

#pragma region Input Actions Binding
	///----LOCOMOTION----

	/// <summary>
	/// Called when we press Y button on the left motion controller
	/// </summary>
	void OnToggleMovementPressed();
	/// <summary>
	/// Called when we release Y button on the left motion controller
	/// </summary>
	void OnToggleMovementReleased();

	///----HAND GRABS----

	/// <summary>
	/// Called when we press Right Grab Button
	/// </summary>
	void OnRightGrabButtonPressed();
	/// <summary>
	/// Called when we release right grab button
	/// </summary>
	void OnRightGrabButtonReleased();
	/// <summary>
	/// Called when we press left grab button
	/// </summary>
	void OnLeftGrabButtonPressed();
	/// <summary>
	/// Called when we release left grab button
	/// </summary>
	void OnLeftGrabButtonReleased();

	///----HAND TRIGGERS----

	/// <summary>
	/// Called when we press right hand trigger
	/// </summary>
	void OnRightTriggerButtonPressed();
	/// <summary>
	/// Called when we release right hand trigger
	/// </summary>
	void OnRightTriggerButtonReleased();
	/// <summary>
	/// Called when we press left hand trigger
	/// </summary>
	void OnLeftTriggerButtonPressed();
	/// <summary>
	/// Called when we release left hand trigger
	/// </summary>
	void OnLeftTriggerButtonReleased();

	///----BATTLE MODE----	
#pragma endregion
	
	/// <summary>
	/// During playing the height of the player changes. And here we resize ze Physics collision capsule component, and we also ensure that 
	/// capsule is moved to the floor. And also we need to move Skeletal mesh down for -Capsule Half Height
	/// </summary>
	void RecalibrateCapsuleAndMeshComponent();
	/// <summary>
	/// Sets the configuration for Decals
	/// </summary>
	/// <param name="decal">Pointer to the decal</param>
	/// <param name="thicknes">the length of the X Axis of the decal</param>
	/// <param name="r">Decal radius</param>
	/// <param name="redraw">Redraw decal in force mode?</param>
	void ConfigureDecalSize(UDecalComponent* decal, float thicknes, float r, bool redraw = false);
	/// <summary>
	/// Adds message to the screen
	/// </summary>
	/// <param name="msg">Message to display</param>
	void DrawMsg(const FString& msg);
	/// <summary>
	/// Updates capsule component position according to the camera position in the Tracking Space
	/// </summary>
	void UpdateCapsuleComponentPosition();
	/// <summary>
	/// Rotates Skeletal mesh according to HMD orientation
	/// </summary>
	/// <param name="DeltaTime"></param>
	void ApplyRotationFromCameraToCapsule(float DeltaTime);
#pragma endregion

#pragma region Editor Callable functions

#if WITH_EDITOR
	/// <summary>
	/// Called when property changed in the Editor UI
	/// </summary>
	/// <param name="PropertyChangedEvent"></param>
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

#endif

#pragma endregion

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	/// <summary>
	/// Gets Ground Velocity and the divides it by RunSpeed
	/// </summary>
	/// <returns>[0 ; 1] Ground Velocity Ratio</returns>
	float GetGroundVelocityRatio() const;
	
	/// <summary>
	/// Updates delta between camera and head bone of the Skeletal Mesh
	/// </summary>
	/// <param name="delta"></param>
	FORCEINLINE void UpdateCameraHeadDelta(float delta)
	{
		cameraHeadDelta = delta;
	}
	
	AActor* GetObstacle(bool& outHit, FHitResult& ouHitResult) const;
	bool GetGroundHit(FHitResult& hit) const;

#pragma region IStateDriven
	virtual void Move(const FVector& dir, float value, bool instant = false) override;
	/// <summary>
	/// Stops all the movement immediatly
	/// </summary>
	virtual void StopMovement() override;
	/// <summary>
	/// Sets the Walk speed to UPawnFloatingMovement
	/// </summary>
	virtual void Walk() override;
	/// <summary>
	/// Sets the Run speed to UPawnFloatingMovement
	/// </summary>
	virtual void Run() override;
	virtual UCapsuleComponent* GetCapsuleComponent() const override
	{
		return CapsuleCollisionComponent;
	}
	virtual UCameraComponent* GetVRCamera() const override
	{
		return CameraComponent;
	}
	virtual USkeletalMeshComponent* GetMesh() const override
	{
		return SkeletalMeshComponent;
	}
	virtual UMotionControllerComponent* GetLeftMotionController() const override
	{
		return LeftMotionController;
	}
	virtual UMotionControllerComponent* GetRightMotionController() const override
	{
		return RightMotionController;
	}
	virtual class UObstacleSensor* GetObstacleSensor() const override;
	virtual class UVelocitySensor* GetCameraVelocitySensor() const override;
	virtual UVelocitySensor* GetMotionControllerVelocitySensor(bool right) const override;
	/// <summary>
	/// Do we do crouching. Calculated by comparing the difference between the player height and current camera Z position.
	/// </summary>
	/// <param name="crouchDepth">Calculated depth of the crouch. Can be used in IK Calculation</param>
	/// <returns></returns>
	virtual bool IsCrouching(float* crouchDepth) const;
	virtual float GetDeadZoneRadius() const
	{
		return DeadZoneRadius;
	}
	virtual FTransform GetActorTransform() override;
	virtual class UCameraFadeSensor* GetCameraFadeSensor() const override;
	bool IsClimbing() const override { return false; } //TO DO
	virtual class UGroundHitSensor* GetGroundHitSensor() const override;
	bool IsGrounded() const override;
	virtual void SetNewActorLocation(const FVector& worldLocation, bool sweep, FHitResult* outHit, ETeleportType teleType) override;
	virtual float GetInitPlayerHeight() const
	{
		return InitialPlayerHeight;
	}
	/// <summary>
	/// Apply camera fade effect to the special fade plane
	/// </summary>
	void ApplyCameraFade(float cameraFadeOpacity);
	bool IsJumping() const;
#pragma endregion

private:
	float cameraHeadDelta;//Delta between Camera Z coordinate and head bone Z coordinate. we use half of it to place Mesh correctly		
	float InitialPlayerHeight;//Height of the player that was calculated during the first start of the game
	UVRCharacterAnimInstance* VRCharacterAnimInstance;
	bool initialPlayerMetricsCalculated;
	void InitializeBodyMetrics();
	UPROPERTY()
	TArray<AActor*> m_ActorsToIgnore;
};
