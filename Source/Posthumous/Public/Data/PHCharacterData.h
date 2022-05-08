#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PHCharacterData.generated.h"

UENUM()
enum class EMantleType : uint8
{
	High_1,
	High_2,
	StepUp
};

USTRUCT()
struct FPHMantleParam
{
	GENERATED_USTRUCT_BODY()

	bool bKeepRotation;

	float MaxHeight;
	float MinHeight;
	float MaxHeightTime;
	float MinHeightTime;
	float MoveForwardTime;
	float ForwardDistance;

	UPROPERTY()
	class UAnimMontage* Montage;
};

UCLASS()
class POSTHUMOUS_API UPHCharacterData : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, Category = "Climbing")
	float JumpToClimbingAnimPlayRate;

	UPROPERTY(EditDefaultsOnly, Category = "Climbing")
	float JumpToClimbingInitialDelay;

	UPROPERTY(EditDefaultsOnly, Category = "Climbing")
	float JumpToClimbingLength;

	UPROPERTY(EditDefaultsOnly, Category = "Climbing")
	float JumpToClimbingVelocity;

	UPROPERTY(EditDefaultsOnly, Category = "Climbing")
	float MaxClimbingSpeed;

	UPROPERTY(EditDefaultsOnly, Category = "Falling")
	float FallingAirControl;

	UPROPERTY(EditDefaultsOnly, Category = "Falling")
	float FallingLateralDeceleration;

	UPROPERTY(EditDefaultsOnly, Category = "Falling")
	FRotator FallingRotationRate;

	UPROPERTY(EditDefaultsOnly, Category = "Gliding")
	TSoftClassPtr<AActor> Glider;

	UPROPERTY(EditDefaultsOnly, Category = "Gliding")
	float GliderSpawnDelay;

	UPROPERTY(EditDefaultsOnly, Category = "Gliding")
	float GlidingAirControl;

	UPROPERTY(EditDefaultsOnly, Category = "Gliding")
	float GlidingLateralDeceleration;

	UPROPERTY(EditDefaultsOnly, Category = "Gliding")
	FRotator GlidingRotationRate;

	UPROPERTY(EditDefaultsOnly, Category = "Gliding")
	float GlidingStartHeight;

	UPROPERTY(EditDefaultsOnly, Category = "Ground")
	FRotator GroundRotationRate;

	UPROPERTY(EditDefaultsOnly, Category = "Ground")
	float MaxAcceleration;

	UPROPERTY(EditDefaultsOnly, Category = "Ground")
	float MaxRunningSpeed;

	UPROPERTY(EditDefaultsOnly, Category = "Ground")
	float MaxSprintingSpeed;

	UPROPERTY(EditDefaultsOnly, Category = "Ground")
	float MaxWalkingSpeed;

	UPROPERTY(EditDefaultsOnly, Category = "Mantle")
	TMap<EMantleType, FPHMantleParam> MantleParamMap;

	UPROPERTY(EditDefaultsOnly, Category = "Mantle")
	float MantlePlayRate;

	UPROPERTY(EditDefaultsOnly, Category = "Swimming")
	float MaxSwimmingSpeed;

	UPROPERTY(EditDefaultsOnly, Category = "Swimming")
	float SwimmingMaxAcceleration;

	UPROPERTY(EditDefaultsOnly, Category = "Swimming")
	FRotator SwimmingRotationRate;

	UPROPERTY(EditDefaultsOnly, Category = "Swimming")
	float SwimmingSprintSpeed;

	UPROPERTY(EditDefaultsOnly, Category = "Option")
	bool bDirectionalJumpOff;

	UPROPERTY(EditDefaultsOnly, Category = "Option")
	bool bMantleDisabledCollision;

	UPROPERTY(EditDefaultsOnly, Category = "Option")
	bool bPersistentWalking;
};
