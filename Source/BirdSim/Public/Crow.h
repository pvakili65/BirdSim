// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "Crow.generated.h"

class UInputMappingContext;
class UInputAction;
class USpringArmComponent;
class UCameraComponent;

UCLASS()
class BIRDSIM_API ACrow : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ACrow();
	// Called every frame	
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	bool GetFlyingStatus();

	bool GetLandingStatus();
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category=Input)
	UInputMappingContext* BirdSimContext;

	UPROPERTY(EditAnywhere, Category=Input)
	UInputAction* WalkAction;

	UPROPERTY(EditAnywhere, Category=Input)
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, Category=Input)
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, Category=Input)
	UInputAction* FlyingAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=UI)
	TSubclassOf<UUserWidget> DeathWidgetClass;

	UPROPERTY()
	UUserWidget* DeathWidget;
	
	void Walk(const FInputActionValue& Value);
	void WalkStarted();
	void Look(const FInputActionValue& Value);
	void JumpStarted();
	void Fly(const FInputActionValue& Value);
	float GetHeightAboveGround();
	void Die();
	UFUNCTION()
	void OnImpact(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

private:

	UPROPERTY(VisibleAnywhere)
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere)
	UCameraComponent* ViewCamera;

	float LastWPressedTime;
	float LastSpacePressedTime;
	float DoubleTapWindow = 0.3;
	float HeightWhenLandingStarted;
	bool IsRunning;
	bool IsFlying;
	bool CanLand;
	bool IsAlive;

};
