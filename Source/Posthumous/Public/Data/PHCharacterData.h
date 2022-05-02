#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PHCharacterData.generated.h"

UCLASS()
class POSTHUMOUS_API UPHCharacterData : public UDataAsset
{
	GENERATED_BODY()
	
public:
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

	UPROPERTY(EditDefaultsOnly, Category = "Ground")
	FRotator GroundRotationRate;

	UPROPERTY(EditDefaultsOnly, Category = "Ground")
	float MaxAcceleration;

	UPROPERTY(EditDefaultsOnly, Category = "Ground")
	float MaxRunningSpeed;

	UPROPERTY(EditDefaultsOnly, Category = "Ground")
	float MaxSprintSpeed;

	UPROPERTY(EditDefaultsOnly, Category = "Ground")
	float MaxWalkingSpeed;

	UPROPERTY(EditDefaultsOnly, Category = "Mantle")
	bool bMantleDisabledCollision;

	UPROPERTY(EditDefaultsOnly, Category = "Swimming")
	float MaxSwimmingSpeed;

	UPROPERTY(EditDefaultsOnly, Category = "Swimming")
	float SwimmingMaxAcceleration;

	UPROPERTY(EditDefaultsOnly, Category = "Swimming")
	FRotator SwimmingRotationRate;

	UPROPERTY(EditDefaultsOnly, Category = "Swimming")
	float SwimmingSprintSpeed;

	UPROPERTY(EditDefaultsOnly, Category = "Option")
	bool bPersistentWalking;
};
