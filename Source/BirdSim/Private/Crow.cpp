// Fill out your copyright notice in the Description page of Project Settings.


#include "Crow.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"

// Sets default values
ACrow::ACrow()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bUseControllerRotationYaw = false;

	LastWPressedTime = -1.0;
	LastSpacePressedTime = -1.0;
	IsRunning = false;
	IsFlying = false;
	IsAlive = true;

	GetCapsuleComponent()->SetNotifyRigidBodyCollision(true);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 500.f, 0.f);
	GetCharacterMovement()->MaxWalkSpeed = 45.f;



	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 240.f;
	CameraBoom->bUsePawnControlRotation = true;

	ViewCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ViewCamera"));
	ViewCamera->SetupAttachment(CameraBoom);

	DefaultCameraDistance = 240.f;
	DefaultCameraOffset = FVector(0.f, 0.f, 0.f);

	LastFireTime = -999.0f;

	CurrentRecoil = FRotator(0.f, 0.f, 0.f);

	CurrentHeadTurn = FRotator(0.f,0.f,0.f);

	TargetHeadTurn = 0.0f;

	AimPitch = 0.0f;

	bGunEquipped = false;

	bShouldTurnHead = false;

	bIsAiming = false;

	AutoPossessPlayer = EAutoReceiveInput::Player0;

}

// Called when the game starts or when spawned
void ACrow::BeginPlay()
{
	Super::BeginPlay();

	GetCapsuleComponent()->OnComponentHit.AddDynamic(this, &ACrow::OnImpact);

	if (APlayerController* PlayerController = Cast<APlayerController>(GetController())) {
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer())) {
			Subsystem->AddMappingContext(BirdSimContext, 0);
		}
		if (CrosshairWidgetClass) {
			CrosshairWidget = CreateWidget<UUserWidget>(PlayerController, CrosshairWidgetClass);
			if (CrosshairWidget) {
				CrosshairWidget->AddToViewport();
				CrosshairWidget->SetVisibility(ESlateVisibility::Hidden);
			}
		}
	}
}

void ACrow::Walk(const FInputActionValue& Value)
{
	if (!IsAlive) {
		return;
	}
	if (IsFlying) {
		return;
	}
	
	const FVector2d MovementVector = Value.Get<FVector2d>();

	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	AddMovementInput(ForwardDirection, MovementVector.Y);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	AddMovementInput(RightDirection, MovementVector.X);
}

void ACrow::WalkStarted()
{
	if (!IsAlive) {
		return;
	}
	float CurrentTime = GetWorld()->GetTimeSeconds();

	if (CurrentTime - LastWPressedTime < DoubleTapWindow) {
		IsRunning = true;
		GetCharacterMovement()->MaxWalkSpeed = 250.f;
	}
	else {
		IsRunning = false;
		GetCharacterMovement()->MaxWalkSpeed = 45.f;
	}
	LastWPressedTime = CurrentTime;
}

void ACrow::Look(const FInputActionValue& Value)
{
	if (IsFlying && bIsAiming && bGunEquipped) {
		return;
	}
	const FVector2d LookAxisValue = Value.Get<FVector2d>();
	AddControllerYawInput(LookAxisValue.X);
	AddControllerPitchInput(LookAxisValue.Y);
}

void ACrow::JumpStarted()
{
	if (!IsAlive) {
		return;
	}
	float CurrentTime = GetWorld()->GetTimeSeconds();

	if (CurrentTime - LastSpacePressedTime < DoubleTapWindow) {
		IsFlying = !IsFlying;

		if (IsFlying) {
			GetCharacterMovement()->SetMovementMode(MOVE_Flying);
			GetCharacterMovement()->MaxFlySpeed = 1500.f;
			GetCharacterMovement()->bOrientRotationToMovement = false;

			CameraBoom->bUsePawnControlRotation = true;
			CameraBoom->SocketOffset = FVector(0.f, 0.f, 50.f);

			if (bGunEquipped) {
				CameraBoom->bUsePawnControlRotation = false;
				CameraBoom->bInheritPitch = true;
				CameraBoom->bInheritYaw = true;
				CameraBoom->bInheritRoll = false;
				CameraBoom->SocketOffset = FlightAimCameraOffset;
				CameraBoom->TargetArmLength = FlightAimCameraDistance;
				CurrentRecoil = FRotator(0.f, 0.f, 13.f);

			}

			UpdateCrosshairVisibility();
		}
		else {
			float currentHeight = GetHeightAboveGround();

			HeightWhenLandingStarted = currentHeight;

			GetCharacterMovement()->SetMovementMode(MOVE_Walking);
			GetCharacterMovement()->bOrientRotationToMovement = true;

			CameraBoom->bUsePawnControlRotation = true;
			CameraBoom->bInheritPitch = true;
			CameraBoom->bInheritYaw = true;
			CameraBoom->bInheritRoll = true;
			CameraBoom->SocketOffset = FVector(0.f, 0.f, 0.f);
			CameraBoom->SetRelativeRotation(FRotator(0.f, 0.f, 0.f));

			if (bGunEquipped) {
				CurrentRecoil = FRotator(0.f, 0.f, -17.f);
			}

			UpdateCrosshairVisibility();
		}
	}
	else {
		if (!IsFlying && GetCharacterMovement()->IsMovingOnGround()) {
			LaunchCharacter(FVector(0.f, 0.f, 200.f), false, true);
		}
	}
	LastSpacePressedTime = CurrentTime;
}

void ACrow::Fly(const FInputActionValue& Value)
{
	if (!IsAlive) {
		return;
	}
	if (!IsFlying) {
		return;
	}
	const FVector2d FlyInput = Value.Get<FVector2d>();
	FRotator CurrentRotation = GetActorRotation();
	
	CurrentRotation.Yaw += FlyInput.X*0.8f;

	CurrentRotation.Pitch += FlyInput.Y * 0.9f;
	CurrentRotation.Pitch = FMath::Clamp(CurrentRotation.Pitch, -89.f, 45.f);

	float TargetRoll = FlyInput.X * 25.f;
	CurrentRotation.Roll = FMath::FInterpTo(CurrentRotation.Roll, TargetRoll, GetWorld()->GetDeltaSeconds(), 5.0f);

	SetActorRotation(CurrentRotation);
}

float ACrow::GetHeightAboveGround()
{
	FVector Start = GetActorLocation();
	FVector End = Start - FVector(0.f, 0.f, 100000.f);

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, Params)) {
		float height = Start.Z - HitResult.Location.Z;
		return height;
	}
	return 21.f;
}

void ACrow::Die1()
{
	IsAlive = false;
	IsFlying = false;

	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->DisableMovement();

	GetMesh()->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	FVector MeshLocation = GetMesh()->GetComponentLocation();
	MeshLocation.Z += 50.f;
	GetMesh()->SetWorldLocation(MeshLocation);

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->SetSimulatePhysics(true);

	GetMesh()->AddImpulse(FVector(0.f, 0.f, 10000.f));

	UpdateCrosshairVisibility();

	if (DeathWidgetClass) {
		APlayerController* PlayerController = Cast<APlayerController>(GetController());
		if (PlayerController) {
			DeathWidget = CreateWidget<UUserWidget>(PlayerController, DeathWidgetClass);
			if (DeathWidget) {
				DeathWidget->AddToViewport();
			}
		}
	}
}

void ACrow::Die2()
{
	IsAlive = false;
	IsFlying = false;

	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->DisableMovement();

	GetMesh()->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->SetSimulatePhysics(true);

	UpdateCrosshairVisibility();

	if (DeathWidgetClass) {
		APlayerController* PlayerController = Cast<APlayerController>(GetController());
		if (PlayerController) {
			DeathWidget = CreateWidget<UUserWidget>(PlayerController, DeathWidgetClass);
			if (DeathWidget) {
				DeathWidget->AddToViewport();
			}
		}
	}
}

void ACrow::OnImpact(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if(!IsAlive) {
		return;
	}
	if (Hit.Normal.Z > 0.9f) {
		if (HeightWhenLandingStarted > 700.f) {
			Die1();
		}
	}
	else if (IsFlying) {
		Die2();
	}
}

void ACrow::Fire()
{
	if (!bGunEquipped || !EquippedGun) {
		return;
	}
	if (!GetMesh()) {
		return;
	}
	float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastFireTime < FireRate) {
		return;
	}
	LastFireTime = CurrentTime;

	FVector SocketLocation = GetMesh()->GetSocketLocation(GunSocketName);
	FRotator SocketRotation = GetMesh()->GetSocketRotation(GunSocketName);

	FVector ForwardVector = FRotationMatrix(SocketRotation).GetUnitAxis(EAxis::X);
	FVector MuzzleOffset = SocketLocation + (ForwardVector * 500.f);
	FVector TraceEnd = SocketLocation + (ForwardVector * 10000.f);

	if (MuzzleFlash) {
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlash, MuzzleOffset, SocketRotation);
	}
	if (FireSound) {
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, SocketLocation);
	}

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, SocketLocation, TraceEnd, ECC_Visibility, QueryParams);

	FVector BeamEnd;
	if (bHit) {
		BeamEnd = HitResult.Location;
	}
	else {
		BeamEnd = TraceEnd;
	}

	if (TracerEffect) {
		UParticleSystemComponent* Tracer = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TracerEffect, SocketLocation);
		if (Tracer) {
			Tracer->SetVectorParameter(FName("Target"), BeamEnd);
		}
	}

	if (bHit) {
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactEffect, HitResult.Location, HitResult.Normal.Rotation());
	}

	CurrentRecoil = RecoilAmount;

}

void ACrow::ToggleGun()
{
	if (bGunEquipped) {
		if (EquippedGun) {
			EquippedGun->Destroy();
			EquippedGun = nullptr;
		}
		bGunEquipped = false;
		bShouldTurnHead = true;
		CurrentRecoil = FRotator(0.f, 0.f, 0.f);

		if (IsFlying) {
			CameraBoom->bUsePawnControlRotation = true;
			CameraBoom->bInheritPitch = true;
			CameraBoom->bInheritYaw = true;
			CameraBoom->bInheritRoll = true;
		}
	}
	else {
		if (GunClass) {
			EquippedGun = GetWorld()->SpawnActor<AActor>(GunClass);
			if (EquippedGun) {
				EquippedGun->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, GunSocketName);
			}
			
			
			if (IsFlying) {
				CameraBoom->bUsePawnControlRotation = false;
				CameraBoom->bInheritPitch = true;
				CameraBoom->bInheritYaw = true;
				CameraBoom->bInheritRoll = false;
				bGunEquipped = true;
				bShouldTurnHead = true;
				CurrentRecoil = FRotator(0.f, 0.f, 13.f);

			}
			else {
				bGunEquipped = true;
				bShouldTurnHead = true;
				CurrentRecoil = FRotator(0.f, 0.f, -17.f);
			}
		}
	}
	UpdateCrosshairVisibility();
}

void ACrow::AimStarted()
{
	if (!IsAlive) {
		return;
	}
	bIsAiming = true;
	
	UpdateCrosshairVisibility();
}

void ACrow::AimCompleted()
{
	bIsAiming = false;
	AimPitch = 0.0f;

	UpdateCrosshairVisibility();
}

void ACrow::UpdateCrosshairVisibility()
{
	if (!CrosshairWidget) {
		return;
	}
	bool bShouldShow = false;
	if (bGunEquipped) {
		if (IsFlying) {
			bShouldShow = true;
		}
		else if (bIsAiming) {
			bShouldShow = true;
		}
	}
	if (bShouldShow) {
		CrosshairWidget->SetVisibility(ESlateVisibility::Visible);
	}
	else {
		CrosshairWidget->SetVisibility(ESlateVisibility::Hidden);
	}
}

// Called every frame
void ACrow::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CurrentRecoil.Yaw<0.f) {
		if (!IsFlying) {
			CurrentRecoil = FMath::RInterpTo(CurrentRecoil, FRotator(0.f, 0.f, -17.f), DeltaTime, RecoilRecoverySpeed);
		}
		else {
			CurrentRecoil = FMath::RInterpTo(CurrentRecoil, FRotator(0.f, 0.f, 13.f), DeltaTime, RecoilRecoverySpeed);
		}
	}

	if (bShouldTurnHead) {
		CurrentHeadTurn = FMath::RInterpTo(CurrentHeadTurn, HeadTurnAmount, DeltaTime, HeadTurnSpeed);
		if (CurrentHeadTurn.Yaw >= HeadTurnAmount.Yaw) {
			bShouldTurnHead = false;
		}	
	}
	else {
		CurrentHeadTurn = FMath::RInterpTo(CurrentHeadTurn, FRotator(0.f,0.f,0.f), DeltaTime, HeadTurnRecoverySpeed);
	}

	if (bGunEquipped) {
		if (!IsFlying) {
			if (bIsAiming) {
				ViewCamera->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
				FRotator ControlRotation = GetControlRotation();
				FRotator TargetRotation = FRotator(0.0f, ControlRotation.Yaw, 0.0f);
				FRotator NewRotation = FMath::RInterpTo(GetActorRotation(), TargetRotation, DeltaTime, AimTurnSpeed);

				SetActorRotation(NewRotation);
				AimPitch = ControlRotation.Pitch;

				CameraBoom->TargetArmLength = FMath::FInterpTo(CameraBoom->TargetArmLength, AimCameraDistance, DeltaTime, AimCameraSpeed);
				CameraBoom->SocketOffset = FMath::VInterpTo(CameraBoom->SocketOffset, AimCameraOffset, DeltaTime, AimCameraSpeed);
			}
			else {
				ViewCamera->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
				AimPitch = 0.f;
				CameraBoom->TargetArmLength = FMath::FInterpTo(CameraBoom->TargetArmLength, DefaultCameraDistance, DeltaTime, AimCameraSpeed);
				CameraBoom->SocketOffset = FMath::VInterpTo(CameraBoom->SocketOffset, DefaultCameraOffset, DeltaTime, AimCameraSpeed);
			}
		}
		else {
			FRotator BirdRotation = GetActorRotation();

			// Offset camera to align crosshair with bullet trajectory
			ViewCamera->SetWorldRotation(FRotator(BirdRotation.Pitch-21.f, BirdRotation.Yaw, BirdRotation.Roll));
			CameraBoom->TargetArmLength = FMath::FInterpTo(CameraBoom->TargetArmLength, FlightAimCameraDistance, DeltaTime, AimCameraSpeed);
			CameraBoom->SocketOffset = FMath::VInterpTo(CameraBoom->SocketOffset, FlightAimCameraOffset, DeltaTime, AimCameraSpeed);
		}
	}
	else {
		AimPitch = 0.0f;
		ViewCamera->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));

		if (!IsFlying) {
			CameraBoom->TargetArmLength = FMath::FInterpTo(CameraBoom->TargetArmLength, DefaultCameraDistance, DeltaTime, AimCameraSpeed);
			CameraBoom->SocketOffset = FMath::VInterpTo(CameraBoom->SocketOffset, DefaultCameraOffset, DeltaTime, AimCameraSpeed);
		}
		else {
			CameraBoom->TargetArmLength = FMath::FInterpTo(CameraBoom->TargetArmLength, DefaultCameraDistance, DeltaTime, AimCameraSpeed);
			CameraBoom->SocketOffset = FMath::VInterpTo(CameraBoom->SocketOffset, DefaultCameraOffset, DeltaTime, AimCameraSpeed);
		}
	}

	if (IsFlying) {
		float height = GetHeightAboveGround();
		if (height <= 20.f) {
			Die1();
		}

		FVector ForwardDirection = GetActorForwardVector();
		AddMovementInput(ForwardDirection, 1.0f);

		FRotator CurrentRotation = GetActorRotation();
		//CurrentRotation.Pitch = FMath::FInterpTo(CurrentRotation.Pitch, 0.f, DeltaTime, 2.0f);
		CurrentRotation.Roll = FMath::FInterpTo(CurrentRotation.Roll, 0.f, DeltaTime, 3.0f);
		SetActorRotation(CurrentRotation);

	}

}

// Called to bind functionality to input
void ACrow::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent * EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent)){
		EnhancedInputComponent->BindAction(WalkAction, ETriggerEvent::Triggered, this, &ACrow::Walk);
		EnhancedInputComponent->BindAction(WalkAction, ETriggerEvent::Started, this, &ACrow::WalkStarted);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ACrow::Look);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACrow::JumpStarted);
		EnhancedInputComponent->BindAction(FlyingAction, ETriggerEvent::Triggered, this, &ACrow::Fly);
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &ACrow::Fire);
		EnhancedInputComponent->BindAction(ToggleGunAction, ETriggerEvent::Started, this, &ACrow::ToggleGun);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Started, this, &ACrow::AimStarted);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &ACrow::AimCompleted);
	}

}

bool ACrow::GetFlyingStatus()
{
	return IsFlying;
}

bool ACrow::GetLandingStatus()
{
	float Height = GetHeightAboveGround();
	if (Height > 700.f) {
		return false;
	}
	else if (Height <= 700.f && Height > 0) {
		return true;
	}
	else {
		return false;
	}
}

