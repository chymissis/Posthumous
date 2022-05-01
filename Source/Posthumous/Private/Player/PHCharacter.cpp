#include "Player/PHCharacter.h"
#include "Data/PHCharacterData.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"

APHCharacter::APHCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, SpringArm(CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm")))
	, Camera(CreateDefaultSubobject<UCameraComponent>(TEXT("Camera")))
{
	GetCapsuleComponent()->InitCapsuleSize(30.f, 92.f);

	GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -85.f));
	GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
	GetMesh()->AddTickPrerequisiteActor(this);

	SpringArm->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepWorldTransform, TEXT("Camera"));
	SpringArm->TargetArmLength = bUsingFpView ? 0.f : 200.f;
	SpringArm->bUsePawnControlRotation = true;

	Camera->SetupAttachment(SpringArm);

	GetCharacterMovement()->MovementState.bCanCrouch = true;
	GetCharacterMovement()->MovementState.bCanFly = true;
	GetCharacterMovement()->MovementState.bCanJump = true;
	GetCharacterMovement()->MovementState.bCanSwim = true;
	GetCharacterMovement()->MovementState.bCanWalk = true;

	bSavedCanWalkOffLedgesWhenCrouching = GetCharacterMovement()->bCanWalkOffLedgesWhenCrouching;
	SavedBrakingDecelerationWalking = GetCharacterMovement()->BrakingDecelerationWalking;
	SavedFallingLateralFriction = GetCharacterMovement()->FallingLateralFriction;
	SavedGroundFriction = GetCharacterMovement()->GroundFriction;

	static ConstructorHelpers::FObjectFinder<UPHCharacterData> CharacterDataAsset(TEXT("/Game/GameData/PHCharacter_DA"));
	if (CharacterDataAsset.Succeeded())
		CharacterData = CharacterDataAsset.Object;
}

void APHCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &APHCharacter::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &APHCharacter::MoveRight);
	PlayerInputComponent->BindAxis(TEXT("LookUp"), this, &APHCharacter::AddControllerPitchInput);
	PlayerInputComponent->BindAxis(TEXT("Turn"), this, &APHCharacter::AddControllerYawInput);

	PlayerInputComponent->BindAction(TEXT("TogglePerspective"), IE_Pressed, this, &APHCharacter::TogglePerspective);

	PlayerInputComponent->BindAction(TEXT("Jump"), IE_Pressed, this, &APHCharacter::Jumping);
	PlayerInputComponent->BindAction(TEXT("Jump"), IE_Released, this, &APHCharacter::StopJumping);
}

void APHCharacter::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);

	OnMovementModeChanged(PrevMovementMode, GetCharacterMovement()->MovementMode);
}

void APHCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APHCharacter, bWalking);
	DOREPLIFETIME(APHCharacter, bSprinting);
	DOREPLIFETIME(APHCharacter, MovementState);
	DOREPLIFETIME(APHCharacter, RotationMode);
}

void APHCharacter::OnMovementModeChanged(EMovementMode PrevMovementMode, EMovementMode NewMovementMode)
{
	if (bClimbingFromBelow && GetCharacterMovement()->MovementMode != EMovementMode::MOVE_Flying)
	{
		GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);
		return;
	}

	if (PrevMovementMode == EMovementMode::MOVE_Swimming && NewMovementMode == EMovementMode::MOVE_Falling)
	{
		StopJumping();
		GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
		return;
	}

	switch (NewMovementMode)
	{
	case EMovementMode::MOVE_Walking:
	case EMovementMode::MOVE_NavWalking:
		ChangeMovementState(EMovementState::Ground);
		return;
	case EMovementMode::MOVE_Falling:
		ChangeMovementState(EMovementState::Falling);
		return;
	case EMovementMode::MOVE_Swimming:
		ChangeMovementState(EMovementState::Swimming);
	}
}

void APHCharacter::ChangeMovementState(EMovementState InMovementState)
{
	if (MovementState == InMovementState || !CharacterData)
		return;

	if (InMovementState == EMovementState::Gliding)
		UKismetSystemLibrary::RetriggerableDelay(this, CharacterData->GliderSpawnDelay, FLatentActionInfo(0, 0, TEXT("SpawnGlider"), this));

	switch (MovementState)
	{
	case EMovementState::Climbing:
		bJumpingToClimb = false;
		DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		GetCharacterMovement()->SetPlaneConstraintEnabled(false);
		break;
	case EMovementState::Falling:
		bSlidingOnSlope = false;
		GetCharacterMovement()->FallingLateralFriction = SavedFallingLateralFriction;
		GetCharacterMovement()->AirControl = CharacterData->FallingAirControl;
		break;
	case EMovementState::Gliding:
		GetCharacterMovement()->GravityScale = 1.f;
		if (Glider)
			Glider->Destroy();
		break;
	case EMovementState::Swimming:
		GetCharacterMovement()->MaxAcceleration = CharacterData->MaxAcceleration;
	}

	MovementState = InMovementState;

	ApplyMovementState();
}

void APHCharacter::SpawnGlider()
{
	if (MovementState != EMovementState::Gliding || !CharacterData || CharacterData->Glider.IsNull() || !GetWorld())
		return;

	FVector Location;
	FRotator Rotation;
	FActorSpawnParameters ActorSpawnParameters;
	ActorSpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	Glider = GetWorld()->SpawnActor(CharacterData->Glider.Get(), &Location, &Rotation, ActorSpawnParameters);
	if (Glider)
		Glider->K2_AttachToComponent(GetMesh(), TEXT("GliderSocket"), EAttachmentRule::KeepRelative, EAttachmentRule::KeepRelative, EAttachmentRule::KeepRelative, true);
}

void APHCharacter::ApplyMovementState()
{
	if (!CharacterData)
		return;

	TurnOffWalkingAndSprinting();
	ChangeMaxSpeed();
	SwitchRotationSetting();

	switch (MovementState)
	{
	case EMovementState::Climbing:
	{
		ClimbingBase = nullptr;
		GetCharacterMovement()->SetPlaneConstraintEnabled(true);
		GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying, 0);
		bCanClimbing = false;
		return;
	}
	case EMovementState::Falling:
	{
		bCanGliding = true;
		bSlidingOnSlope = false;
		GetCharacterMovement()->AirControl = CharacterData->FallingAirControl;
		GetCharacterMovement()->BrakingDecelerationFalling = CharacterData->FallingLateralDeceleration;
		GetCharacterMovement()->RotationRate = CharacterData->FallingRotationRate;
		GetCharacterMovement()->GravityScale = 1.f;
		return;
	}
	case EMovementState::Gliding:
	{
		if (GetCharacterMovement()->MovementMode != EMovementMode::MOVE_Falling)
			GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Falling, 0);

		bBlockedClimbing = false;
		GetCharacterMovement()->AirControl = CharacterData->GlidingAirControl;
		GetCharacterMovement()->BrakingDecelerationFalling = CharacterData->GlidingLateralDeceleration;
		GetCharacterMovement()->RotationRate = CharacterData->GlidingRotationRate;
		GetCharacterMovement()->GravityScale = 0.f;
		return;
	}
	case EMovementState::Ground:
	{
		if (GetCharacterMovement()->MovementMode != EMovementMode::MOVE_Walking)
			GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking, 0);

		bBlockedClimbing = false;
		GetCharacterMovement()->RotationRate = CharacterData->GroundRotationRate;
		return;
	}
	case EMovementState::Mantle:
	{
		if (bMantle)
			return;

		bMantle = true;
		if (GetCharacterMovement()->MovementMode != EMovementMode::MOVE_Flying)
			GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying, 0);

		StartMantle();
		return;
	}
	case EMovementState::Swimming:
	{
		if (GetCharacterMovement()->MovementMode != EMovementMode::MOVE_Falling)
			GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Falling, 0);

		GetCharacterMovement()->RotationRate = CharacterData->SwimmingRotationRate;
		GetCharacterMovement()->MaxAcceleration = CharacterData->SwimmingMaxAcceleration;
		UnCrouch();
		GetCharacterMovement()->Velocity = FVector(0.f, 0.f, GetCharacterMovement()->Velocity.Z);
	}
	}
}

void APHCharacter::TurnOffWalkingAndSprinting()
{
	if (CharacterData && CharacterData->bPersistentWalking)
		return;

	if (bWalking)
		ServerSetWalking(false);

	if (bSprinting)
		ServerSetSprinting(false);
}

void APHCharacter::ServerSetWalking_Implementation(bool bInWalking)
{
	bWalking = bInWalking;
}

void APHCharacter::ServerSetSprinting_Implementation(bool bInSprinting)
{
	bSprinting = bInSprinting;
}

void APHCharacter::OnRepSprinting()
{
	if (bSprinting && CharacterData && !CharacterData->bPersistentWalking)
	{
		ServerSetWalking(false);
		return;
	}

	ChangeMaxSpeed();

	if (!bSprinting)
		SwitchRotationSetting();
}

void APHCharacter::StartMantle()
{
	if (CharacterData && CharacterData->MantleDisabledCollision)
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);


}

void APHCharacter::ChangeMaxSpeed()
{
	if (!CharacterData)
		return;

	switch (MovementState)
	{
	case EMovementState::Climbing:
		GetCharacterMovement()->MaxFlySpeed = CharacterData->MaxClimbingSpeed;
		return;
	case EMovementState::Falling:
	case EMovementState::Gliding:
	case EMovementState::Ground:
		GetCharacterMovement()->MaxWalkSpeed = bSprinting ? CharacterData->MaxSprintSpeed : (bWalking ? CharacterData->MaxWalkingSpeed : CharacterData->MaxRunningSpeed);
		return;
	case EMovementState::Swimming:
		GetCharacterMovement()->MaxSwimSpeed = bSprinting ? CharacterData->SwimmingSprintSpeed : CharacterData->MaxSwimmingSpeed;
	}
}

void APHCharacter::SwitchRotationSetting()
{
	switch (RotationMode)
	{
	case ERotationMode::CameraDirection:
		GetCharacterMovement()->bOrientRotationToMovement = MovementState != EMovementState::Climbing;
		GetCharacterMovement()->bUseControllerDesiredRotation = false;
		return;
	case ERotationMode::VelocityDirection:
		switch (MovementState)
		{
		case EMovementState::Climbing:
			GetCharacterMovement()->bOrientRotationToMovement = false;
			GetCharacterMovement()->bUseControllerDesiredRotation = false;
			return;
		case EMovementState::Falling:
			GetCharacterMovement()->bOrientRotationToMovement = !bUsingFpView;
			GetCharacterMovement()->bUseControllerDesiredRotation = bUsingFpView;
			return;
		case EMovementState::Gliding:
		case EMovementState::Swimming:
			GetCharacterMovement()->bOrientRotationToMovement = true;
			GetCharacterMovement()->bUseControllerDesiredRotation = bUsingFpView;
			return;
		case EMovementState::Ground:
			GetCharacterMovement()->bOrientRotationToMovement = bSprinting && !bUsingFpView;
			GetCharacterMovement()->bUseControllerDesiredRotation = true;
			return;
		case EMovementState::Mantle:
			GetCharacterMovement()->bOrientRotationToMovement = false;
			GetCharacterMovement()->bUseControllerDesiredRotation = true;
			return;
		default:
			GetCharacterMovement()->bOrientRotationToMovement = true;
			GetCharacterMovement()->bUseControllerDesiredRotation = true;
		}
	}
}

void APHCharacter::MoveForward(const float AxisValue)
{
	if (bBlockedMovement)
		return;

	switch (MovementState)
	{
	case EMovementState::Climbing:
		AddMovementInput(UKismetMathLibrary::GetUpVector(FRotator(0.f, 0.f, GetControlRotation().Yaw)), AxisValue);
		return;
	case EMovementState::Falling:
	case EMovementState::Ground:
	case EMovementState::Gliding:
	case EMovementState::Swimming:
		AddMovementInput(UKismetMathLibrary::GetForwardVector(GetActorRotation()), AxisValue);
		return;
	default:
		AddMovementInput(FVector(0.f), AxisValue);
	}
}

void APHCharacter::MoveRight(const float AxisValue)
{
	if (bBlockedMovement)
		return;

	switch (MovementState)
	{
	case EMovementState::Climbing:
		AddMovementInput(UKismetMathLibrary::GetRightVector(FRotator(0.f, 0.f, GetControlRotation().Yaw)), AxisValue);
		return;
	case EMovementState::Falling:
	case EMovementState::Ground:
	case EMovementState::Gliding:
	case EMovementState::Swimming:
		AddMovementInput(UKismetMathLibrary::GetRightVector(GetActorRotation()), AxisValue);
		return;
	default:
		AddMovementInput(FVector(0.f), AxisValue);
	}
}

void APHCharacter::TogglePerspective()
{
	bUsingFpView = !bUsingFpView;

	SpringArm->TargetArmLength = bUsingFpView ? 0.f : 200.f;
	SpringArm->SetRelativeLocation(bUsingFpView ? FVector(0.f) : FVector(-3.f, 30.f, 20.f));
}

void APHCharacter::Jumping()
{

}
