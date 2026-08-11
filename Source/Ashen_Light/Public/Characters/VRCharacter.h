// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/CharacterBase.h"
#include "../../Public/Enums.h"
#include "../../Public/Interfaces/VRCharacterInterface.h"
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

	UPROPERTY(VisibleAnywhere)
	class UPhysicalMovementComponent* PhysicalMovementComponent;

	UPROPERTY(VisibleAnywhere)
	class UTrackingSpaceMovementComponent* TrackingSpaceMovementComponent;

	UPROPERTY(VisibleAnywhere)
	class UStateManagerComponent* LocomotionStateManager;

#pragma endregion

#pragma region UPROPERTIES

	UPROPERTY(EditAnywhere, Category = "VR Locomotion", meta = (DisplayName = "Swinging Threshold", ClampMin = "0.001", UIMin = "0.001"))
	float SwiningThreshold = 40.f;

	UPROPERTY(EditAnywhere, Category = "VR Locomotion", meta = (DisplayName = "Walk Speed"))
	float walkSpeed = 200.f;

	UPROPERTY(EditAnywhere, Category = "VR Locomotion", meta = (DisplayName = "Run Speed"))
	float runSpeed = 300.f;

	UPROPERTY(EditAnywhere, Category = "VR Locomotion", meta = (DisplayName = "Ground Detection Threshold", Tooltip = "Value that will be added to the end point of the Sphere Trace for Ground Detection"))
	float GroundDetectionThreshold = 10.f;

	UPROPERTY(EditAnywhere, Category = "VR Locomotion")
	float CrouchThreshold = 10.f;

	UPROPERTY(EditAnywhere, Category = "VR Locomotion Gravity", meta = (DisplayName = "Gravity Constant g"))
	float GravityConstant = -980.f; //cm / sec^2

	UPROPERTY(EditAnywhere, Category = "VR Locomotion Gravity", meta = (DisplayName = "Max Fall Velocity"))
	float MaxFallVelocity = 2000.f; //cm / sec

	UPROPERTY(EditAnywhere, Category = "VR Locomotion Gravity", meta = (DisplayName = "Terminal Velocity"))
	float TerminalVelocity = -4000.f; //cm / sec
	
	UPROPERTY(EditAnywhere, Category = "VR Camera Fade", meta = (DisplayName = "Fade Chech Radius", ClampMin = "0.001", UIMin = "0.001"))
	float FadeCheckRadius = 14.f;

	UPROPERTY(EditAnywhere, Category = "VR Camera Fade", meta = (DisplayName = "Camera Fade Distance", ClampMin = "0.001", UIMin = "0.001"))
	float CameraFadeDistance = 5.f;

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
	/// Calculates speed of motion controller
	/// </summary>
	/// <param name="motionController">Pointer to the controller</param>
	/// <param name="prevMotionControllerPosition">Pointer to the previous motion controller position</param>
	/// <param name="DeltaTime">Time passed between two adjacent frames</param>
	/// <returns>Instant velocity of the controller</returns>
	float GetMotionControllerSpeed(UMotionControllerComponent* motionController, FVector* prevMotionControllerPosition, float DeltaTime) const;
	/// <summary>
	/// Traces sphere from the half of the capsule to the ground. The length of the trace is half of the capsule + threshold.
	/// </summary>
	void DetectGround();
	/// <summary>
	/// Applys gravity during each tick to the object
	/// </summary>
	/// <param name="DeltaTime">Time passed between two adjacent frames</param>
	void ApplyGravity(float DeltaTime);
	/// <summary>
	/// Adjusts Movement direction using surface normals
	/// </summary>
	/// <param name="InputVector"></param>
	/// <returns></returns>
	FVector AdjustInputForSlope(FVector InputVector) const;
	/// <summary>
	/// Checks condition when we need to apply camera fade effect
	/// </summary>
	/// <param name="DeltaTime"></param>
	void CheckCameraFade(float DeltaTime);
	/// <summary>
	/// Apply camera fade effect to the special fade plane
	/// </summary>
	void ApplyCameraFade();
	/// <summary>
	/// Adds message to the screen
	/// </summary>
	/// <param name="msg">Message to display</param>
	void DrawMsg(const FString& msg);
	/// <summary>
	/// Calculates the velocity of the moving Pawn
	/// </summary>
	/// <param name="DeltaTime">Time passed between two adjacent frames</param>
	void CalculateCurrentVelocity(float DeltaTime);
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

#pragma region IStateDriven
	virtual void Move(const FVector& dir, float value) override;

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

	virtual FVector GetCurrentVelocity() const override
	{
		return CurrentVelocity;
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

	/// <summary>
	/// Do we do crouching. Calculated by comparing the difference between the player height and current camera Z position.
	/// </summary>
	/// <param name="crouchDepth">Calculated depth of the crouch. Can be used in IK Calculation</param>
	/// <returns></returns>
	virtual bool IsCrouching(float* crouchDepth) const override;

	virtual float GetDeadZoneRadius() const
	{
		return DeadZoneRadius;
	}
	/// <summary>
	/// Calculates the Velocity of the right and left motion controllers, then compares them with a threshold
	/// </summary>
	/// <param name="DeltaTime">Time passed between two adjacent frames</param>
	/// <returns>True - running, fals - walking</returns>
	virtual bool IsSwiningArms(float DeltaTime) override;

	virtual bool CheckObstacles(float obstacleDistDetection, float halfHeightMultipl, FHitResult& OutHit);

	virtual FVector AdjustInputForSlope(const FVector& worldDir) const override;

	virtual bool CheckObstaclesInDirection(const FVector& NormDirection, float obstacleDistDetection, float halfHeightMultipl, FHitResult& hit) override;

	virtual bool IsGrounded() const override;

	virtual bool IsJumping(float jumpHeadThreshold) const override;

	virtual FTransform GetActorTransform() override;
#pragma endregion


#pragma region Public UPROPERTIES
	UPROPERTY(EditAnywhere, Category = "VR Simulation", meta = (Tooltip="Player height that will be used in Simulation"))
	float PlayerPreviewHeight = 186.f;

	//UPROPERTY(EditAnywhere, Category = "Adjustments Constants", meta = (DisplayName = "Mesh Offset", Tooltip = "Offset for camera. We cannot manualy edit camera location, cause it is controlled by HMD. But we can shift mesh back in X Axis. Also it is used to adjust IK Camera Position"))
	//FVector MeshOffset = FVector(0.f, 0.f, 0.f);
#pragma endregion

private:
	FVector prevLeftHandLocation;//Left Hand Location in Tracking Space calculated in the previous frame
	FVector prevRightHandLocation;//Right Hand Location in Tracking Space calculated in the previous frame
	bool bIsObstacleHit;//Do we hit some Static or Dynamic Mesh?
	bool bIsGrounded;//Do we stay on a ground
	FHitResult CurrentGroundHit;//Hit of the ground
	FHitResult CurrentObstacleHit;//Hit of the obstacle
	float VerticalVelocity = 0.f;//Velocity applied to the Pawn in -Z direction
	bool bCameraInAMesh;//Has camera entered the mesh?
	float cameraHeadDelta;//Delta between Camera Z coordinate and head bone Z coordinate. we use half of it to place Mesh correctly	
	FVector CurrentVelocity;
	FVector PrevCameraPosition;//Camera Location in the Tracking Space calculated in the previous frame
	float InitialPlayerHeight;//Height of the player that was calculated during the first start of the game
	float CurrentCameraFadeOpacity;
	UVRCharacterAnimInstance* VRCharacterAnimInstance;
	bool initialPlayerHeightCalculated;
	void CalculatePlayerHeight();
};
