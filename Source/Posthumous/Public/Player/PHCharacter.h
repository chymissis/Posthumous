#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PHCharacter.generated.h"

UENUM()
enum class EMovementState : uint8
{
	None,
	Climbing,
	Falling,
	Gliding,
	Ground,
	Mantle,
	Swimming
};

UENUM()
enum class ERotationMode : uint8
{
	CameraDirection,
	VelocityDirection
};

USTRUCT()
struct FPHMantleEndTransform
{
	GENERATED_USTRUCT_BODY()

	FTransform MantleUpTransform;
	FTransform MantleForwardTransform;
	FTransform ComponentTransform;

	UPrimitiveComponent* Component;
};

enum class EMantleType : uint8;

UCLASS()
class POSTHUMOUS_API APHCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	APHCharacter(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode) override;

private:
	void OnMovementModeChanged(EMovementMode PrevMovementMode, EMovementMode NewMovementMode);
	UFUNCTION(Server, Reliable)
	void ServerChangeMovementState(EMovementState InMovementState);
	void SpawnGlider();
	void ApplyMovementState();

	void TurnOffWalkingAndSprinting();
	UFUNCTION(Server, Reliable)
	void ServerSetWalking(bool bInWalking);
	UFUNCTION()
	void OnRepWalking() { ChangeMaxSpeed(); }
	UFUNCTION(Server, Reliable)
	void ServerSetSprinting(bool bInSprinting);
	UFUNCTION()
	void OnRepSprinting();
	
	void StartMantle();

	void ChangeMaxSpeed();

	UFUNCTION()
	void OnRepRotationMode() { SwitchRotationSetting(); }
	void SwitchRotationSetting();

	void MoveForward(const float AxisValue);
	void MoveRight(const float AxisValue);

	void TogglePerspective();
	void Jumping();

private:
	UPROPERTY(ReplicatedUsing = OnRepWalking)
	bool bWalking;
	UPROPERTY(ReplicatedUsing = OnRepSprinting)
	bool bSprinting;

	bool bBlockedClimbing;
	bool bBlockedMovement;
	bool bCanClimbing;
	bool bCanGliding;
	bool bClimbingFromBelow;
	bool bClimbingFromAbove;
	bool bJumpingToClimb;
	bool bMantle;
	bool bSlidingOnSlope;
	bool bUsingFpView = true;

	bool bSavedCanWalkOffLedgesWhenCrouching;
	float SavedBrakingDecelerationWalking;
	float SavedFallingLateralFriction;
	float SavedGroundFriction;
	float MantleHeight;

	UPROPERTY(Replicated)
	EMovementState MovementState;

	UPROPERTY(ReplicatedUsing = OnRepRotationMode)
	ERotationMode RotationMode;

	UPROPERTY()
	EMantleType MantleType;
	FPHMantleEndTransform MantleEndTransform;

	UPROPERTY()
	class UPHCharacterData* CharacterData;

	UPROPERTY()
	class USpringArmComponent* SpringArm;
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	class UCameraComponent* Camera;
	
	UPROPERTY()
	UPrimitiveComponent* ClimbingBase;

	UPROPERTY()
	AActor* Glider;
};
