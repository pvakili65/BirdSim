// Fill out your copyright notice in the Description page of Project Settings.


#include "CrowAnimInstance.h"
#include "Crow.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

void UCrowAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	Crow = Cast<ACrow>(TryGetPawnOwner());
	if (Crow) {
		CrowMovement = Crow->GetCharacterMovement();
	}
}

void UCrowAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);
	if (CrowMovement) {
		GroundSpeed = UKismetMathLibrary::VSizeXY(CrowMovement->Velocity);

		IsFalling = CrowMovement->IsFalling();

		IsFlying = Crow->GetFlyingStatus();

		CanLand = Crow->GetLandingStatus();
	}
}
