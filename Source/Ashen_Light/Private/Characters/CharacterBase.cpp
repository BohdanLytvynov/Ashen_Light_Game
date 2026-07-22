// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/CharacterBase.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
ACharacterBase::ACharacterBase(const FObjectInitializer& init) : Super(init)
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ACharacterBase::BeginPlay()
{
	Super::BeginPlay();
	
}

void ACharacterBase::UpdateGroundSpeed()
{
	UCharacterMovementComponent* charMv = this->GetCharacterMovement();
	if (!charMv) return;
	const FVector& currSpeed = charMv->Velocity;
	FVector2D GroundVelocity(currSpeed.X, currSpeed.Y);
	const float groundVelMagnitude = GroundVelocity.Size();
	const float maxWalkSpeed = charMv->MaxWalkSpeed;
	if (maxWalkSpeed == 0) return;
	GroundSpeed = FMath::Clamp(groundVelMagnitude / maxWalkSpeed, 0.f, 1.f);
}

FVector ACharacterBase::GetLocalPositionRelativeToOrigin(USceneComponent* origin, USceneComponent* component)
{	
	if (!origin || !component) return FVector::ZeroVector;
	return origin->GetComponentTransform().InverseTransformPosition(component->GetComponentLocation());
}

// Called every frame
void ACharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateGroundSpeed();
}

// Called to bind functionality to input
void ACharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

