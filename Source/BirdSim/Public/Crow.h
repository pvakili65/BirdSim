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
class UParticleSystem;

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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input)
	UInputAction* FireAction;

	UPROPERTY()
	UUserWidget* DeathWidget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Weapon)
	AActor* EquippedGun;

	UPROPERTY(EditAnywhere, Category=Weapon)
	FName GunSocketName = "CROW_-BeakSocket";

	UPROPERTY(EditAnywhere, Category=Weapon)
	UParticleSystem* MuzzleFlash;

	UPROPERTY(EditAnywhere, Category=Weapon)
	UParticleSystem* ImpactEffect;

	UPROPERTY(EditAnywhere, Category=Weapon)
	float FireDamage = 10.0f;

	UPROPERTY(EditAnywhere, Category=Weapon)
	float FireRate = 0.5f;

	UPROPERTY(EditAnywhere, Category=Weapon)
	USoundBase* FireSound;

	UPROPERTY(EditAnywhere, Category=Weapon)
	UParticleSystem* TracerEffect;

	UPROPERTY(EditAnywhere, Category=Weapon)
	FRotator RecoilAmount = FRotator(0.f, -130.f, 0.f);
	//float RecoilAmount = 130.0f;

	UPROPERTY(EditAnywhere, Category=Weapon)
	float RecoilRecoverySpeed = 7.f;

	UPROPERTY(BlueprintReadOnly, Category=Weapon)
	FRotator CurrentRecoil;
	//float CurrentRecoil;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input)
	UInputAction* ToggleGunAction;

	UPROPERTY(EditDefaultsOnly, Category=Weapon)
	TSubclassOf<AActor> GunClass;

	UPROPERTY(EditAnywhere, Category=Weapon)
	FRotator HeadTurnAmount = FRotator(0.f, 100.f, 0.f);
	//float HeadTurnAmount = 100.f;

	UPROPERTY(EditAnywhere, Category=Weapon)
	float HeadTurnSpeed = 20.f;

	UPROPERTY(EditAnywhere, Category=Weapon)
	float HeadTurnRecoverySpeed = 3.f;

	UPROPERTY(BlueprintReadOnly, Category=Weapon)
	FRotator CurrentHeadTurn;
	//float CurrentHeadTurn;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input)
	UInputAction* AimAction;

	UPROPERTY(EditDefaultsOnly, Category=UI)
	TSubclassOf<UUserWidget> CrosshairWidgetClass;

	UPROPERTY()
	UUserWidget* CrosshairWidget;

	UPROPERTY(EditAnywhere, Category=Aiming)
	float AimTurnSpeed = 5.0f;

	UPROPERTY(BlueprintReadOnly, Category=Aiming)
	float AimPitch;

	UPROPERTY(EditAnywhere, Category = Aiming)
	float AimCameraDistance = 30.f;

	UPROPERTY(EditAnywhere, Category = Aiming)
	FVector AimCameraOffset = FVector(0.0f, 20.0f, 0.0f);

	UPROPERTY(EditAnywhere, Category = Aiming)
	float FlightAimCameraDistance = -2.5f;

	UPROPERTY(EditAnywhere, Category = Aiming)
	FVector FlightAimCameraOffset = FVector(0.f, 11.f, 3.f);

	UPROPERTY(EditAnywhere, Category = Aiming)
	float AimCameraSpeed = 10.0f;

	float LastFireTime;


	void Walk(const FInputActionValue& Value);
	void WalkStarted();
	void Look(const FInputActionValue& Value);
	void JumpStarted();
	void Fly(const FInputActionValue& Value);
	float GetHeightAboveGround();
	void Die1();
	void Die2();
	UFUNCTION()
	void OnImpact(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
	void Fire();
	void ToggleGun();
	void AimStarted();
	void AimCompleted();
	void UpdateCrosshairVisibility();

private:

	UPROPERTY(VisibleAnywhere)
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere)
	UCameraComponent* ViewCamera;

	float LastWPressedTime;
	float LastSpacePressedTime;
	float DoubleTapWindow = 0.37;
	float HeightWhenLandingStarted;
	bool IsRunning;
	bool IsFlying;
	bool CanLand;
	bool IsAlive;
	bool bGunEquipped;
	bool bShouldTurnHead;
	float TargetHeadTurn;
	bool bIsAiming;
	FRotator StoredCameraRotation;
	float DefaultCameraDistance;
	FVector DefaultCameraOffset;

};
