// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "Pilot2020Character.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetMathLibrary.h"

#include "Pilot2020GameInstance.h"
#include "UtilityAIController.h"
#include "NonPlayerCharacter.h"

#include "PickupItem.h"
#include "AncientDoor.h"

//For debug messages
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

//////////////////////////////////////////////////////////////////////////
// APilot2020Character

APilot2020Character::APilot2020Character()
{
	// Set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->AddLocalOffset(FVector(0.f, 0.f, 50.f));
	CameraBoom->TargetArmLength = CameraBoomLength; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Camera lag
	CameraBoom->bEnableCameraLag = true;
	CameraBoom->bEnableCameraRotationLag = true;
	CameraBoom->CameraLagSpeed = 10.f;
	CameraBoom->CameraRotationLagSpeed = 25.f;
	CameraBoom->CameraLagMaxDistance = 128.f;

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	IsCharacterPlayer = true;
}

void APilot2020Character::BeginPlay()
{
	Super::BeginPlay();

	VoiceSFX->SetPitchMultiplier(1.f);

	if (Cast<UPilot2020GameInstance>(GetGameInstance())->CurrentGameState == EGameState::GS_Respawned)
	{
		GoldCarried = Cast<UPilot2020GameInstance>(GetGameInstance())->PlayerGold * 0.5f;
		HealingCarried = 0;
	}
	else
	{
		GoldCarried = Cast<UPilot2020GameInstance>(GetGameInstance())->PlayerGold;
		HealingCarried = Cast<UPilot2020GameInstance>(GetGameInstance())->PlayerHealingItems;
	}
}

void APilot2020Character::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateMovementDirection();
	LERPCameraDistance(DeltaTime);
	handleDeath(DeltaTime);

	HandleMusicTransition();

	// Checks if the player is idle...
	if (Inactive && Cast<UPilot2020GameInstance>(GetGameInstance())->CurrentGameState != EGameState::GS_NewStart && !IsInCombat && !VoiceSFX->IsPlaying() && UGameplayStatics::GetCurrentLevelName(GetWorld(), true) != "Ship_Level")
	{
		InactiveTimer += 1.f * DeltaTime;

		if (InactiveTimer > InactiveTimerCap)
		{
			VoiceSFX->SetSound(IdleCue);
			VoiceSFX->Play();

			InactiveTimer = 0.f;
		}
	}
	else
	{
		InactiveTimer = 0.f;
		Inactive = true;
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void APilot2020Character::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &APilot2020Character::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("UseFood", IE_Pressed, this, &ABaseCharacter::RestoreHealthEat);

	PlayerInputComponent->BindAction("ReadyWeapon", IE_Pressed, this, &APilot2020Character::ToggleWeaponReady);

	PlayerInputComponent->BindAction("Attack", IE_Pressed, this, &APilot2020Character::StartAttackInput);
	PlayerInputComponent->BindAction("Attack", IE_Released, this, &APilot2020Character::StopAttackInput);

	PlayerInputComponent->BindAxis("MoveForward", this, &APilot2020Character::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &APilot2020Character::MoveRight);

	PlayerInputComponent->BindAction("LockOn", IE_Pressed, this, &ABaseCharacter::LockOntoTarget);
	PlayerInputComponent->BindAction("LockOn", IE_Released, this, &ABaseCharacter::StopLockOntoTarget);
	
	PlayerInputComponent->BindAction("Dodge", IE_Pressed, this, &ABaseCharacter::DodgeRoll);

	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &APilot2020Character::StartSprintInput);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &APilot2020Character::StopSprintInput);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APilot2020Character::TurnAtRate);
	PlayerInputComponent->BindAxis("TurnRate", this, &APilot2020Character::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APilot2020Character::LookUpAtRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &APilot2020Character::LookUpAtRate);

	PlayerInputComponent->BindAxis("CameraDistance", this, &APilot2020Character::AdjustCameraDistance);

	PlayerInputComponent->BindAction("Activate", IE_Pressed, this, &APilot2020Character::Activate);
}

void APilot2020Character::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	if (!IsLockedOn)
	{
		AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
	}
}

void APilot2020Character::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	if (!IsLockedOn)
	{
		AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
	}
}

void APilot2020Character::Jump()
{
	Super::Jump();

	VoiceSFX->SetSound(JumpCue);
	VoiceSFX->Play();

	// Stops the inactivity timer
	Inactive = false;
}

// When pressing the activation key
void APilot2020Character::Activate()
{
	FindActorsInRange(); // Looking for acceptable actors.
	bool HasActivatedItem{ false };

	// Searches for pickup items
	for (auto object : pickupItem)
	{
		APickupItem* tempItem = Cast<APickupItem>(object);

		if (tempItem->MustBeInteractedWith)
		{
			if (tempItem->isWithinPickupRange)
			{
				if (!VoiceSFX->IsPlaying() && tempItem->ItemType == EItemType::IT_Key)
				{
					if (KeysCarried > 0)
					{
						if (FMath::RandBool())
						{
							VoiceSFX->SetSound(PickupKeyFirstCue);
						}
						else
						{
							VoiceSFX->SetSound(PickupKeySecondCue);
						}

						VoiceSFX->Play();
					}
					else if (KeysCarried == 0)
					{
						VoiceSFX->SetSound(PickupKeyFirstCue);
						VoiceSFX->Play();
					}
				}

				Cast<APickupItem>(object)->PickUp(this);
				HasActivatedItem = true;
			}
		}
	}

	if (!HasActivatedItem)
	{
		for (auto Door : AncientDoor)
		{
			AAncientDoor* DoorToOpen = Cast<AAncientDoor>(Door);

			if (DoorToOpen)
			{
				DoorToOpen->OpenDoor();
			}
		}
	}

	// Stops the inactivity timer
	Inactive = false;
}

void APilot2020Character::ToggleWeaponReady()
{
	if (WeaponReady)
	{
		SetWeaponReady(false);
	}
	else
	{
		SetWeaponReady(true);
	}

	// Stops the inactivity timer
	Inactive = false;
}

void APilot2020Character::StartAttackInput()
{
	ReceivingAttackInput = true;
	AttackStart();

	// Stops the inactivity timer
	Inactive = false;
}

void APilot2020Character::StopAttackInput()
{
	ReceivingAttackInput = false;
	AttackEnd();
}

void APilot2020Character::FindActorsInRange()
{
	pickupItem.Empty();

	// Decides what actor type to allow, then adds all actors of type to the array.
	UClass* ActorFilterItem = APickupItem::StaticClass();
	LockOnRangeSphere->GetOverlappingActors(pickupItem, ActorFilterItem);

	AncientDoor.Empty();

	// Decides what actor type to allow, then adds all actors of type to the array.
	UClass* ActorFilterDoor = AAncientDoor::StaticClass();
	LockOnRangeSphere->GetOverlappingActors(AncientDoor, ActorFilterDoor);
}

void APilot2020Character::MoveForward(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector - Is different when the the character is locked onto another
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		if (!IsLockedOn)
		{
			AddMovementInput(Direction, Value);
		}
		else if (LockedTargetCharacter)
		{
			FVector LockedOnDirection = LockedTargetCharacter->GetActorLocation() - GetActorLocation();
			LockedOnDirection.Normalize();

			AddMovementInput(UKismetMathLibrary::GetForwardVector(LockedOnDirection.ToOrientationRotator()), Value);
		}

		// Stops the inactivity timer
		Inactive = false;
	}

	if (Value > 0.f)
	{
		IsMovingForwards = true;
		IsMovingBackwards = false;
	}
	else if (Value < 0.f)
	{
		IsMovingForwards = false;
		IsMovingBackwards = true;
	}
	else
	{
		IsMovingForwards = false;
		IsMovingBackwards = false;
	}
}

void APilot2020Character::MoveRight(float Value)
{
	if ( (Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector - Is different when the the character is locked onto another
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		if (!IsLockedOn)
		{
			AddMovementInput(Direction, Value);
		}
		else if (LockedTargetCharacter)
		{
			FVector LockedOnDirection = LockedTargetCharacter->GetActorLocation() - GetActorLocation();
			LockedOnDirection.Normalize();

			AddMovementInput(UKismetMathLibrary::GetRightVector(LockedOnDirection.ToOrientationRotator()), Value);
		}

		// Stops the inactivity timer
		Inactive = false;
	}

	if (Value > 0.f)
	{
		IsMovingRight = true;
		IsMovingLeft = false;
	}
	else if (Value < 0.f)
	{
		IsMovingRight = false;
		IsMovingLeft = true;
	}
	else
	{
		IsMovingRight = false;
		IsMovingLeft = false;
	}
}

void APilot2020Character::AdjustCameraDistance(float Value)
{	
	CameraBoomLength += Value * CameraBoomZoomRate;

	if (CameraBoomLength > CameraBoomMaxLength)
	{
		CameraBoomLength = CameraBoomMaxLength;
	}
	else if (CameraBoomLength < 100.f)
	{
		CameraBoomLength = 100.f;
	}
}

void APilot2020Character::LERPCameraDistance(float DeltaTime)
{
	CameraBoom->TargetArmLength = FMath::Lerp(CameraBoom->TargetArmLength, CameraBoomLength, 0.1f);
}

void APilot2020Character::UpdateMovementDirection()
{
	if (!IsLockedOn)
	{
		CharacterDirectionState = ECharacterDirectionState::CDS_Forwards;
	}
	else
	{
		if (IsMovingForwards && IsMovingRight) 	// Forwards!
		{
			CharacterDirectionState = ECharacterDirectionState::CDS_ForwardsRight;
		}
		else if (IsMovingForwards && IsMovingLeft)
		{
			CharacterDirectionState = ECharacterDirectionState::CDS_ForwardsLeft;
		}
		else if (IsMovingForwards && !IsMovingRight && !IsMovingLeft)
		{
			CharacterDirectionState = ECharacterDirectionState::CDS_Forwards;
		}
		else if (IsMovingBackwards && IsMovingRight) // Backwards!
		{
			CharacterDirectionState = ECharacterDirectionState::CDS_BackwardsRight;
		}
		else if (IsMovingBackwards && IsMovingLeft)
		{
			CharacterDirectionState = ECharacterDirectionState::CDS_BackwardsLeft;
		}
		else if (IsMovingBackwards && !IsMovingRight && !IsMovingLeft)
		{
			CharacterDirectionState = ECharacterDirectionState::CDS_Backwards;
		}
		else if (IsMovingRight && !IsMovingForwards && !IsMovingBackwards) // Right!
		{
			CharacterDirectionState = ECharacterDirectionState::CDS_Right;
		}
		else if (IsMovingLeft && !IsMovingForwards && !IsMovingBackwards) // Left!
		{
			CharacterDirectionState = ECharacterDirectionState::CDS_Left;
		}
	}
}

void APilot2020Character::StartSprintInput()
{
	ReceivingSprintInput = true;
	StartSprint();
}

void APilot2020Character::StopSprintInput()
{
	ReceivingSprintInput = false;
	StopSprint();
}

void APilot2020Character::handleDeath(float DeltaTime)
{
	if (CharacterLifeState == ECharacterLifeState::CLS_Dead)
	{
		if (!PlaySpecialEventMusic)
		{
			if (Cast<UPilot2020GameInstance>(GetGameInstance()) && Cast<UPilot2020GameInstance>(GetGameInstance())->MusicSystem)
			{
				Cast<UPilot2020GameInstance>(GetGameInstance())->MusicSystem->TargetTrackToPlay = EMusicPlayerState::MPS_Defeat;
				PlaySpecialEventMusic = true;
			}
		}

		if (secondsDead >= 5.f)
		{ 
			secondsDead = 0;

			Cast<UPilot2020GameInstance>(GetGameInstance())->CurrentGameState = EGameState::GS_Respawned;
			Cast<UPilot2020GameInstance>(GetGameInstance())->OpenGameOverScreen();
		}
		else
		{
			secondsDead += DeltaTime;
		}
	}
}

void APilot2020Character::HandleMusicTransition()
{
	// Check if the music system is loaded and set
	if (Cast<UPilot2020GameInstance>(GetGameInstance()) && Cast<UPilot2020GameInstance>(GetGameInstance())->MusicSystem && !PlaySpecialEventMusic)
	{
		TArray<AActor*> OverlapContainer;
		UClass* ActorFilterEnemy = ANonPlayerCharacter::StaticClass();
		IsInCombat = false;

		LockOnRangeSphere->GetOverlappingActors(OverlapContainer, ActorFilterEnemy);
		ANonPlayerCharacter* TempChar{ nullptr };

		for (auto CharacterCheck : OverlapContainer)
		{
			TempChar = Cast<ANonPlayerCharacter>(CharacterCheck);

			if ((TempChar->CharacterLifeState != ECharacterLifeState::CLS_Dead) && (TempChar->CharacterFaction != CharacterFaction) && (Cast<AUtilityAIController>(TempChar->GetController())->ActiveUtilityBucket == EBucketType::BT_Combat))
			{
				IsInCombat = true;
			}
		}

		if (!PlaySpecialEventMusic)
		{
			if (IsInCombat)
			{
				Cast<UPilot2020GameInstance>(GetGameInstance())->MusicSystem->TargetTrackToPlay = EMusicPlayerState::MPS_Combat;
			}
			else
			{
				Cast<UPilot2020GameInstance>(GetGameInstance())->MusicSystem->TargetTrackToPlay = EMusicPlayerState::MPS_Exploration;
			}
		}
	}
}