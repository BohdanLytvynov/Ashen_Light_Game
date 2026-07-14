// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/VRCharacter.h"
#include "MotionControllerComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "HeadMountedDisplayFunctionLibrary.h"

AVRCharacter::AVRCharacter(const FObjectInitializer& init) : Super(init)
{
	//Get root component its Capsule
	USceneComponent* root = GetRootComponent();
	if (!root) return;
	//Check root component
	UCapsuleComponent* caps = Cast<UCapsuleComponent>(root);
	if (!caps) return;
	//Location in a center of the Tracking Space
	VROriginComponent = CreateDefaultSubobject<USceneComponent>(TEXT("VR Origin Component"));
	if (!VROriginComponent) return;
	//VR camera
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera Component"));
	if (!CameraComponent) return;
	CameraComponent->SetupAttachment(VROriginComponent);
	CameraComponent->bUsePawnControlRotation = false;
	//Right motion Controller
	RightMotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("Right Motion Controller"));
	if (!RightMotionController) return;
	RightMotionController->SetupAttachment(VROriginComponent);
	RightMotionController->MotionSource = FName("Right");
	//Left Motion Controller
	LeftMotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("Left Motion Controller"));
	if (!LeftMotionController) return;
	LeftMotionController->SetupAttachment(VROriginComponent);
	LeftMotionController->MotionSource = FName("Left");	
	VROriginComponent->SetupAttachment(root);	
	//Set the VR origin to the floor of the capsule
	float capsuleHalfHeight = caps->GetScaledCapsuleHalfHeight();
	FVector floorLocation(0.f, 0.f, -capsuleHalfHeight);
	VROriginComponent->SetRelativeLocation(floorLocation);
}

void AVRCharacter::BeginPlay()
{
	Super::BeginPlay();

	UHeadMountedDisplayFunctionLibrary::SetTrackingOrigin(EHMDTrackingOrigin::Floor);
}

void AVRCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);


}
