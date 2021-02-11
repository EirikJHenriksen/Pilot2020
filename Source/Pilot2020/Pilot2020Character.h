// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BaseCharacter.h"
#include "GameFramework/Character.h"

#include "UObject/Class.h"

#include "Pilot2020Character.generated.h"

class APickupItem;

UCLASS(config=Game)
class APilot2020Character : public ABaseCharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

public:
	APilot2020Character();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float DeltaSeconds) override;

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
		float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
		float BaseLookUpRate;

	//==============================
	// T I M E L I N E S
	FTimeline CameraDistanceTimeline;

protected:
	void handleDeath(float);
	float secondsDead{ 0 };

	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);
	
	//==============================
	// C A M E R A
	void AdjustCameraDistance(float Value);
	void LERPCameraDistance(float DeltaTime);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
		float CameraBoomLength = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
		float CameraBoomCombatLength = 600.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
		float CameraBoomZoomRate = 25.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
		float CameraBoomMaxLength = 800.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
		float CameraBoomMinLength = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
		UCurveFloat* CameraCombatModeTransition;

	/** 
	 * Called via input to turn at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	virtual void Jump() override;

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	//==============================
	// M O V E M E N T
	bool IsMovingForwards = false;
	bool IsMovingBackwards = false;

	bool IsMovingRight = false;
	bool IsMovingLeft = false;

	void UpdateMovementDirection();

	void StartSprintInput();
	void StopSprintInput();

	//==============================
	// I N T E R A C T I O N
	/**
	* Called via input to activate stuff. (on pressing E)
	*/
	void Activate();

	TArray<AActor*> pickupItem;
	TArray<AActor*> AncientDoor;

	void ToggleWeaponReady();

	void StartAttackInput();
	void StopAttackInput();

	void FindActorsInRange();

	// Idle check
	bool Inactive{ false };
	float InactiveTimer{ 0.f };
	float InactiveTimerCap{ 30.f };

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player Character - Interaction")
		FName InteractionType{ "Interact " };

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player Character - Interaction")
		FName InteractionObjectName{ "No Name" };

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player Character - Interaction")
		bool InteractionIsPossible{ false };

	bool IsInCombat{ false };

	//===========================
	// M U S I C   S Y S T E M
	void HandleMusicTransition();

	bool PlaySpecialEventMusic{ false };

	//===========================
	// A U D I O
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio - Voice Acting")
		USoundCue* JumpCue{ nullptr };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio - Voice Acting")
		USoundCue* PickupKeyFirstCue{ nullptr };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio - Voice Acting")
		USoundCue* PickupKeySecondCue{ nullptr };

};

