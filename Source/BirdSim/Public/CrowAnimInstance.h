// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "CrowAnimInstance.generated.h"

class ACrow;
class UCharacterMovementComponent;

UCLASS()
class BIRDSIM_API UCrowAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaTime) override;

	UPROPERTY(BlueprintReadOnly)
	ACrow* Crow;

	UPROPERTY(BlueprintReadOnly, Category=Movement)
	UCharacterMovementComponent* CrowMovement;

	UPROPERTY(BlueprintReadOnly, Category=Movement)
	float GroundSpeed;

	UPROPERTY(BlueprintReadOnly, Category=Movement)
	bool IsFalling;

	UPROPERTY(BlueprintReadOnly, Category=Movement)
	bool CanLand;

	UPROPERTY(BlueprintReadOnly, Category=Movement)
	bool IsFlying;
};
