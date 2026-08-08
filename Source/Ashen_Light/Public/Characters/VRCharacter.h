// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/CharacterBase.h"
#include "../../Public/Enums.h"
#include "VRCharacter.generated.h"
/**
 * 
 */

UCLASS()
class ASHEN_LIGHT_API AVRCharacter : public APawn
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

	UPROPERTY(Transient)
	UMaterialInstanceDynamic* VRSliderMaterialInstance;

	UPROPERTY(Transient)
	UMaterialInstanceDynamic* DeadZoneMaterialInstance;

	UPROPERTY(Transient)
	UMaterialInstanceDynamic* ActiveZoneMaterialInstance;

	UPROPERTY(EditDefaultsOnly, Category = "VR Locomotion Decals")
	class UMaterialInterface* FadeMaterialBase;

	UPROPERTY(EditDefaultsOnly, Category = "VR Locomotion Decals")
	UMaterialInterface* VRSliderMaterialBase;

	UPROPERTY(EditDefaultsOnly, Category = "VR Locomotion Decals")
	UMaterialInterface* DeadZoneMaterialBase;

	UPROPERTY(EditDefaultsOnly, Category = "VR Locomotion Decals")
	UMaterialInterface* ActiveZoneMaterialBase;

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

	UPROPERTY(EditDefaultsOnly, Category = "VR Locomotion Decals")
	FLinearColor MovementLockedDecalColor = FColor::Red;

	UPROPERTY(EditDefaultsOnly, Category = "VR Locomotion Decals")
	FLinearColor BattleModeDecalColor = FColor::Yellow;

	UPROPERTY(EditDefaultsOnly, Category = "VR locomotion Decals")
	FLinearColor FreeRoamDecalColor = FColor::Green;
		
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

	UPROPERTY(EditAnywhere, Category = "VR Locomotion Obstacle Detection", meta = (DisplayName = "Obstacle Distance Detection", ClampMin = "0.001", UIMin = "0.001"))
	float ObstacleDistanceDetection = 140.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Locomotion Obstacle Detection", meta = (ClampMin = "0.0", ClampMax = "90.0"))
	float StopMovementAngleThreshold = 20.0f;

	UPROPERTY(EditAnywhere, Category = "VR Camera Fade", meta = (DisplayName = "Fade Chech Radius", ClampMin = "0.001", UIMin = "0.001"))
	float FadeCheckRadius = 14.f;

	UPROPERTY(EditAnywhere, Category = "VR Camera Fade", meta = (DisplayName = "Camera Fade Distance", ClampMin = "0.001", UIMin = "0.001"))
	float CameraFadeDistance = 5.f;
	
	UPROPERTY(EditAnywhere, Category = "VR Adjustments", meta = (Tooltip = "We need this parameter, cause Skeletal Mesh Origin is located not at the foot of the mesh. It is rised Up for some value so we need to raise our skeletal mesh up too."))
	float MeshOriginAdjustment = 0.f;

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
	/// Called when we press B button on the right motion controller
	/// </summary>
	void OnToggleBattleModePressed();
	/// <summary>
	/// Called when we press Y button on the left motion controller
	/// </summary>
	void OnSprintPressed();
	/// <summary>
	/// Called when we release Y button on the left motion controller
	/// </summary>
	void OnSprintReleased();

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
	
	/// <summary>
	/// Called when we press A button
	/// </summary>
	void OnBlockMovementPressed();
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
	void ConfigureDecal(UDecalComponent* decal, float thicknes, float r, bool redraw = false);
	/// <summary>
	/// Main function for movement control by Virtual Joystick
	/// </summary>
	/// <param name="DeltaTime">Time passed between two adjacent frames</param>
	virtual void HandleMovement(float DeltaTime);
	/// <summary>
	/// Calculates the Velocity of the right and left motion controllers, then compares them with a threshold
	/// </summary>
	/// <param name="DeltaTime">Time passed between two adjacent frames</param>
	/// <returns>True - running, fals - walking</returns>
	bool IsSwiningArms(float DeltaTime);
	/// <summary>
	/// Calculates speed of motion controller
	/// </summary>
	/// <param name="motionController">Pointer to the controller</param>
	/// <param name="prevMotionControllerPosition">Pointer to the previous motion controller position</param>
	/// <param name="DeltaTime">Time passed between two adjacent frames</param>
	/// <returns>Instant velocity of the controller</returns>
	float GetMotionControllerSpeed(UMotionControllerComponent* motionController, FVector* prevMotionControllerPosition, float DeltaTime);
	/// <summary>
	/// Stops all the movement immediatly
	/// </summary>
	virtual void Stop();
	/// <summary>
	/// Sets the Walk speed to UPawnFloatingMovement
	/// </summary>
	virtual void Walk();
	/// <summary>
	/// Sets the Run speed to UPawnFloatingMovement
	/// </summary>
	virtual void Run();
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
	/// Traces sphere in Current Velocity direction to detect Obstacles.
	/// </summary>
	void CheckObstacles();
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
	/// <summary>
	/// Updates delta between camera and head bone of the Skeletal Mesh
	/// </summary>
	/// <param name="delta"></param>
	FORCEINLINE void UpdateCameraHeadDelta(float delta)
	{
		cameraHeadDelta = delta;
	}
	/// <summary>
	/// Do we do crouching. Calculated by comparing the difference between the player height and current camera Z position.
	/// </summary>
	/// <param name="crouchDepth">Calculated depth of the crouch. Can be used in IK Calculation</param>
	/// <returns></returns>
	bool IsCrouching(float* crouchDepth);
#pragma region Public UPROPERTIES
	UPROPERTY(EditAnywhere, Category = "VR Simulation", meta = (Tooltip="Player height that will be used in Simulation"))
	float PlayerPreviewHeight = 186.f;

	UPROPERTY(EditAnywhere, Category = "Adjustments Constants", meta = (DisplayName = "Mesh Offset", Tooltip = "Offset for camera. We cannot manualy edit camera location, cause it is controlled by HMD. But we can shift mesh back in X Axis. Also it is used to adjust IK Camera Position"))
	FVector MeshOffset = FVector(-10.f, 0.f, 0.f);
#pragma endregion

private:
	FVector prevLeftHandLocation;//Left Hand Location in Tracking Space calculated in the previous frame
	FVector prevRightHandLocation;//Right Hand Location in Tracking Space calculated in the previous frame
	bool bIsObstacleHit;//Do we hit some Static or Dynamic Mesh?
	bool bIsGrounded;//Do we stay on a ground
	FHitResult CurrentGroundHit;//Hit of the ground
	FHitResult CurrentObstacleHit;//Hit of the obstacle
	float VerticalVelocity = 0.f;//Velocity applied to the Pawn in -Z direction
	bool bCanPerformBattleStep;//Can we perform Battle Step?
	bool bCanRun;//Is Run button pressed
	bool bCameraInAMesh;//Has camera entered the mesh?
	float cameraHeadDelta;//Delta between Camera Z coordinate and head bone Z coordinate. we use half of it to place Mesh correctly
	EVRCharacterState CurrentCharacterState;
	EVRCharacterState CachedCharacterState;
	FVector CurrentVelocity;
	FVector PrevCameraPosition;//Camera Location in the Tracking Space calculated in the previous frame
	float InitialPlayerHeight;//Height of the player that was calculated during the first start of the game
	float CurrentCameraFadeOpacity;
	UVRCharacterAnimInstance* VRCharacterAnimInstance;
	bool initialPlayerHeightCalculated;
	UMaterialInstanceDynamic* CreateMaterialInstance(UMaterialInterface* interface);
	void ApplyMaterialToComponent(UPrimitiveComponent* comp, int32 matindex,
		UMaterialInterface* materialBase, UMaterialInstanceDynamic*& dynamicInstance);
	void ApplyMaterialToComponent(UDecalComponent* comp,
		UMaterialInterface* materialBase, UMaterialInstanceDynamic*& dynamicInstance);
	void SetDecalColors();
	void SetDecalsColor(FLinearColor color);
	void CalculatePlayerHeight();
};
