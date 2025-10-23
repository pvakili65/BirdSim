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
			GetCharacterMovement()->MaxFlySpeed = 1000.f;
			GetCharacterMovement()->bOrientRotationToMovement = false;

			CameraBoom->bUsePawnControlRotation = true;
			CameraBoom->SocketOffset = FVector(0.f, 0.f, 50.f);
		}
		else {
			float currentHeight = GetHeightAboveGround();

			HeightWhenLandingStarted = currentHeight;

			GetCharacterMovement()->SetMovementMode(MOVE_Walking);
			GetCharacterMovement()->bOrientRotationToMovement = true;

			CameraBoom->bUsePawnControlRotation = true;
			CameraBoom->SocketOffset = FVector(0.f, 0.f, 0.f);
			CameraBoom->SetRelativeRotation(FRotator(0.f, 0.f, 0.f));
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
	CurrentRotation.Pitch = FMath::Clamp(CurrentRotation.Pitch, -30.f, 30.f);

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
	return 0.f;
}

void ACrow::Die()
{
	IsAlive = false;
	IsFlying = false;

	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->DisableMovement();

	GetMesh()->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	FVector MeshLocation = GetMesh()->GetComponentLocation();
	MeshLocation.Z += 50.f;  // Move up 50 units
	GetMesh()->SetWorldLocation(MeshLocation);

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->SetSimulatePhysics(true);

	GetMesh()->AddImpulse(FVector(0.f, 0.f, 10000.f));

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
			Die();
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("You Died"));
		}
	}
}

// Called every frame
void ACrow::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsFlying) {
		FVector ForwardDirection = GetActorForwardVector();
		AddMovementInput(ForwardDirection, 1.0f);

		FRotator CurrentRotation = GetActorRotation();
		CurrentRotation.Pitch = FMath::FInterpTo(CurrentRotation.Pitch, 0.f, DeltaTime, 2.0f);
		CurrentRotation.Roll = FMath::FInterpTo(CurrentRotation.Roll, 0.f, DeltaTime, 3.0f);
		SetActorRotation(CurrentRotation);

		float MinAltitude = 100.f;
		float CurrentHeight = GetHeightAboveGround();
		if (CurrentHeight < MinAltitude && CurrentHeight>0.f) {
			FVector CurrentLocation = GetActorLocation();
			float AdjustmentNeeded = MinAltitude - CurrentHeight;
			CurrentLocation.Z += AdjustmentNeeded * DeltaTime * 5.0f;
			SetActorLocation(CurrentLocation);
		}
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

