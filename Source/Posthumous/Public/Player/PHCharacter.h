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
	void ChangeMovementState(EMovementState InMovementState);
	UFUNCTION(Server, Reliable)
	void ServerChangeMovementState(EMovementState InMovementState);
	UFUNCTION(NetMulticast, Reliable)
	void MulticastChangeMovementState(EMovementState InMovementState);
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
	
	void ChangeMaxSpeed();
	
	void StartMantle();

	UFUNCTION()
	void OnRepRotationMode() { SwitchRotationSetting(); }
	void SwitchRotationSetting();

	void MoveForward(const float AxisValue);
	void MoveRight(const float AxisValue);

	void TogglePerspective();

	void Jumping();
	
	void CheckJumpingToClimb();
	
	void JumpToClimb(const FVector& JumpOrientation);
	UFUNCTION(Server, Reliable)
	void ServerJumpToClimb(const FVector& JumpOrientation);
	UFUNCTION(NetMulticast, Reliable)
	void MulticastJumpToClimb(const FVector& JumpOrientation);
	
	void JumpOffWhileClimbing(const FVector& JumpOrientation);
	UFUNCTION(Server, Reliable)
	void ServerJumpOffWhileClimbing(const FVector& JumpOrientation);
	UFUNCTION(NetMulticast, Reliable)
	void MulticastJumpOffWhileClimbing(const FVector& JumpOrientation);

	void JumpWhileSlidingOnSlope(const FVector& LaunchVelocity);
	UFUNCTION(Server, Reliable)
	void ServerJumpWhileSlidingOnSlope(const FVector& LaunchVelocity);
	UFUNCTION(NetMulticast, Reliable)
	void MulticastJumpWhileSlidingOnSlope(const FVector& LaunchVelocity);

	bool CanGlide();
	void SpawnGlider();

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
	bool bSlidingCrouched;
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
