// Fill out your copyright notice in the Description page of Project Settings.

// This serves as the base for all characters in the game. Covers humanoids, monsters and critters.

#include "BaseCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"

#include "PickupItem.h"
#include "Engine/World.h"

//For debug messages
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"

// Sets default values
ABaseCharacter::ABaseCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Creating and adding a weapon for the character.
	EquippedWeapon = CreateDefaultSubobject<UWeaponSceneComponent>(TEXT("Equipped Weapon"));
	EquippedWeapon->SetupAttachment(GetMesh(), TEXT("SheathTwoHanded"));
	EquippedWeapon->WeaponOwner = this;

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	GetCapsuleComponent()->SetGenerateOverlapEvents(true);
	GetCapsuleComponent()->BodyInstance.SetResponseToChannel(ECC_Pawn, ECR_Overlap);
	GetCapsuleComponent()->BodyInstance.SetResponseToChannel(ECC_Camera, ECR_Overlap);

	// Make camera ignore characters
	GetMesh()->BodyInstance.SetResponseToAllChannels(ECR_Ignore);

	// Make other character attempt to avoid this character when walking
	GetMesh()->SetCanEverAffectNavigation(true);

	// No decals should be displayed on this mesh
	GetMesh()->bReceivesDecals = false;

	// Set variables for the lock on system.
	LockOnRangeSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Lock-On Sphere"));
	LockOnRangeSphere->SetupAttachment(RootComponent);
	LockOnRangeSphere->InitSphereRadius(LockOnSphereRadius);

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->bUseSeparateBrakingFriction = true;
	GetCharacterMovement()->JumpZVelocity = 1000.f;
	GetCharacterMovement()->AirControl = 0.2f;

	GetCharacterMovement()->GravityScale = 2.f;

	GetCharacterMovement()->RotationRate = DefaultRotationRate;
	GetCharacterMovement()->GroundFriction = DefaultGroundFriction;

	// Configuring the audio components
	FootstepSFX = CreateDefaultSubobject<UAudioComponent>(TEXT("Footsteps - SFX"));
	FootstepSFX->bAutoActivate = false;
	FootstepSFX->bAutoDestroy = false;
	FootstepSFX->SetupAttachment(GetRootComponent());

	AttackSwingSFX = CreateDefaultSubobject<UAudioComponent>(TEXT("Attack Swing - SFX"));
	AttackSwingSFX->bAutoActivate = false;
	AttackSwingSFX->bAutoDestroy = false;
	AttackSwingSFX->SetupAttachment(GetMesh(), TEXT("hand_r"));

	AttackImpactSFX = CreateDefaultSubobject<UAudioComponent>(TEXT("Attack Impact - SFX"));
	AttackImpactSFX->bAutoActivate = false;
	AttackImpactSFX->bAutoDestroy = false;
	AttackImpactSFX->SetupAttachment(GetRootComponent());

	VoiceSFX = CreateDefaultSubobject<UAudioComponent>(TEXT("Voice - SFX"));
	VoiceSFX->bAutoActivate = false;
	VoiceSFX->bAutoDestroy = false;
	VoiceSFX->SetupAttachment(GetMesh(), TEXT("head"));

	ActionSFX = CreateDefaultSubobject<UAudioComponent>(TEXT("Action - SFX"));
	ActionSFX->bAutoActivate = false;
	ActionSFX->bAutoDestroy = false;
	ActionSFX->SetupAttachment(GetRootComponent());
}

// Called after components have been initialized
void ABaseCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// Attaching all the cues to the audio components
	if (FootstepSFX->IsValidLowLevelFast())
		FootstepSFX->SetSound(FootstepCue);

	if (VoiceSFX->IsValidLowLevelFast())
		VoiceSFX->SetSound(VoiceCue);

	if (ActionSFX->IsValidLowLevelFast())
		ActionSFX->SetSound(ActionCue);
}

// Called when the game starts or when spawned
void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();

	GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &ABaseCharacter::OnOverlapCapsuleResponse);

	InitializeTimelines();

	// Calculate what the final attack speed, if a weapon is equipped.
	if (EquippedWeapon)
	{
		CalculatedAttackSpeed = EquippedWeapon->WeaponData.WeaponBaseSpeed * SpeedAttackMultiplier;
	}

	// For debugging purposes.
	if (ShowLockOnSphereCollision)
	{
		LockOnRangeSphere->SetHiddenInGame(false);
	}

	// set pitch - this pirate has his own voice now
	if (VoiceSFX)
		VoiceSFX->SetPitchMultiplier(FMath::RandRange(.75f, 1.5f));
	else
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Purple, "no voice comp!?", true);

}

// Called every frame
void ABaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CharacterLifeState != ECharacterLifeState::CLS_Dead)
	{
		// Update the movement variables of the player
		MovementStateHandler();
		MovementVariableHandler();

		CharacterRotationHandler(DeltaTime);

		UpdateBaseMovementSpeed();

		CheckIfLockStillValid();

		CharacterPushForce(DeltaTime);

		Knockback(DeltaTime);

		AttackMovement();

		UpdateAttackSpeed();

		// Modifies the character movement speed based on what state they are in
		UpdateBaseMovementSpeed();

		// Update the character velocity values
		CalculateVelocity();

		// Check and update the surface type underneath the character
		SurfaceTypeCheck();

		// Tick Timelines
		SprintTimeline.TickTimeline(DeltaTime);
		DodgeTimeline.TickTimeline(DeltaTime);
	}
}

void ABaseCharacter::SetWeaponReady(bool ReadyWeapon)
{
	if (UGameplayStatics::GetCurrentLevelName(GetWorld(), true) != "Ship_Level")
	{
		if ((CharacterActionState == ECharacterActionState::CAS_None) && EquippedWeapon)
		{
			WeaponReady = ReadyWeapon;

			// Both of these variables are reset in the animation BP.
			EquipOrUnequipInProgress = true;
			CharacterActionState = ECharacterActionState::CAS_Interacting;
		}
	}
	else
	{
		VoiceSFX->SetSound(ActionFailCue);
		VoiceSFX->Play();
	}
}

void ABaseCharacter::UpdateAttackSpeed()
{
	// Slowdown when an attack enters the chain window stage
	if (ActiveAttackChainWindow)
	{
		AttackChainSlowdownMult = 0.5f;
	}
	else
	{
		AttackChainSlowdownMult = 1.f;
	}

	if (EquippedWeapon)
	{
		CalculatedAttackSpeed = EquippedWeapon->WeaponData.WeaponBaseSpeed * SpeedAttackMultiplier * AttackChainSlowdownMult;
	}
}

void ABaseCharacter::AttackStart()
{
	if (!WeaponReady)
	{
		// Nothing yet!
	}
	else if (ActiveAttackChainWindow && CharacterActionState != ECharacterActionState::CAS_Dodging)
	{
		switch (AttackChainCount)
		{
		case 0:
			//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Purple, "Chain 1!", true);
			CharacterAttackState = ECharacterAttackState::CAT_AttackChain;
			AttackChainCount = 1;
			break;
		case 1:
			//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Purple, "Chain 2!", true);
			CharacterAttackState = ECharacterAttackState::CAT_AttackChain;
			AttackChainCount = 0;
			AttackChainFinale = true;
			break;
		}

		// Lets the system know that the weapon can damage enemies
		CharacterActionState = ECharacterActionState::CAS_Attacking;
		AttackInProgress = true;

		// Play voice acting for attack
		PlaySound_Attack();
	}
	else if (!ActiveAttackChainWindow && CharacterActionState == ECharacterActionState::CAS_None && CharacterAttackState == ECharacterAttackState::CAT_None && !AttackInProgress)
	{
		AttackChainFinale = false;
		AttackInProgress = true; // The animation blueprint resets this after the attack montage has finished playing!
		AttackChainCount = 0;

		CharacterAttackState = ECharacterAttackState::CAT_ChargeUp;
	}

	if (WeaponReady && AttackInProgress)
	{
		EquippedWeapon->UpdateSocketAttachement(true);
		StopSprint();
	}
}

void ABaseCharacter::AttackEnd()
{
	if (!WeaponReady)
	{
		SetWeaponReady(true);
	}
	else if (!ActiveAttackChainWindow)
	{
		if (CharacterActionState != ECharacterActionState::CAS_Attacking &&
			CharacterAttackState == ECharacterAttackState::CAT_ChargeUp)
		{
			CharacterActionState = ECharacterActionState::CAS_Attacking;
			CharacterAttackState = ECharacterAttackState::CAT_AttackBegin;

			AttackExecute();
		}
	}
}

// Merge this function into "AttackEnd()" ???
void ABaseCharacter::AttackExecute()
{
	if (IsPowerAttack && !AttackChainFinale)
	{
		CharacterAttackState = ECharacterAttackState::CAT_HeavyAttack;
		CalculatedAttackSpeed *= 0.75f;

		AttackPower = AttackMaximum;
	}
	else if (!AttackChainFinale)
	{
		CharacterAttackState = ECharacterAttackState::CAT_LightAttack;

		AttackPower = AttackMinimum;
	}
	else if (AttackChainFinale)
	{
		CharacterAttackState = ECharacterAttackState::CAT_SpecialAttack;

		AttackPower = AttackMaximum * AttackChainDamageMult;
	}

	// Play voice acting for attack
	PlaySound_Attack();
}

void ABaseCharacter::AttackMovement()
{
	if ((CharacterActionState == ECharacterActionState::CAS_Attacking && !ReceivingAttackInput))
	{
		AddMovementInput(GetActorForwardVector(), 1.f);
		SetActorRotation(FRotator(0.f, FMath::Lerp(GetActorRotation(), GetControlRotation(), 0.3f).Yaw, 0.f));
	}
}

void ABaseCharacter::AttackCancel()
{
	CharacterActionState = ECharacterActionState::CAS_None;
	CharacterAttackState = ECharacterAttackState::CAT_None;
	AttackInProgress = false;
	DamageCanBeDealt = false;
}

void ABaseCharacter::RestoreHealthEat()
{
	if ((CharacterMovementState != ECharacterMovementState::CMS_Knockback))
	{
		if (HealingCarried > 0)
		{
			HealingCarried--;

			AffectHealth(25.f, false);

			ActionSFX->SetSound(EatCue);
			ActionSFX->Play();

			VoiceSFX->SetSound(EatVoiceCue);
			VoiceSFX->Play();
		}
		else
		{
			VoiceSFX->SetSound(ActionFailCue);
			VoiceSFX->Play();
		}
	}
}

void ABaseCharacter::AffectHealth(float magnitude, bool AttackDamage)
{
	if (FMath::IsNegativeFloat(magnitude))
	{
		HasReceivedDamage = true;

		// voice
		VoiceSFX->Stop();
		VoiceSFX->SetSound(HurtCue);
		VoiceSFX->Play();
	}

	Health += magnitude;

	if (Health > HealthMaximum)
	{
		Health = HealthMaximum;
	}

	if (Health < 0.f)
	{
		// Character dies. :'(
		OnDeath(AttackDamage);
	}
}

void ABaseCharacter::OnDeath(bool CausedByAttack)
{
	CharacterLifeState = ECharacterLifeState::CLS_Dead;
	Health = 0.f;

	StopLockOntoTarget();

	LockOnRangeSphere->DestroyComponent();

	DetachFromControllerPendingDestroy();

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);

	GetMovementComponent()->StopMovementImmediately();
	GetMovementComponent()->SetComponentTickEnabled(false);

	GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
	SetActorEnableCollision(true);
	
	GetMesh()->BodyInstance.SetResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->BodyInstance.SetResponseToChannel(ECC_Visibility, ECR_Ignore);

	GetMesh()->SetAllBodiesSimulatePhysics(true);
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->WakeAllRigidBodies();
	GetMesh()->bBlendPhysics = true;

	// Calculate ragdoll knockback force
	if (CausedByAttack)
	{
		FVector KnockbackDirection{ GetActorLocation() - DamageReceivedByCharacter->GetActorLocation() };
		KnockbackDirection.Normalize();
		float BaseForce{ 10000.f };

		GetMesh()->AddImpulse(KnockbackDirection * (BaseForce * DamageReceivedByCharacter->AttackPower), "spine_03", true);
	}

	DropItems();

	// voice
	VoiceSFX->SetSound(DeathCue);
	VoiceSFX->Play();
}

void ABaseCharacter::DropItems()
{
	FVector RandomDirection = FVector(FMath::RandRange(0.f, 10.f), FMath::RandRange(0.f, 10.f), FMath::RandRange(0.f, 10.f)).GetSafeNormal() * 1000.f;

	if (GoldItemClass)
	{
		for (int32 i = GoldCarried; i > 0; i--)
		{
			RandomDirection = FVector(FMath::RandRange(0.f, 10.f), FMath::RandRange(0.f, 10.f), FMath::RandRange(0.f, 10.f)).GetSafeNormal() * 1000.f;
			GetWorld()->SpawnActor<APickupItem>(GoldItemClass, GetActorLocation(), FRotator(0.f, FMath::RandRange(0.f, 359.f), 0.f))->ItemMeshComp->AddImpulse(RandomDirection);
		}
	}
	else
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "ERROR! GOLD INVENTORY ITEM CLASS WAS NOT SET IN BLUEPRINT!", true);
		}
	}

	if (HealingItemClass)
	{
		for (int32 i = HealingCarried; i > 0; i--)
		{
			RandomDirection = FVector(FMath::RandRange(0.f, 10.f), FMath::RandRange(0.f, 10.f), FMath::RandRange(0.f, 10.f)).GetSafeNormal() * 1000.f;
			GetWorld()->SpawnActor<APickupItem>(HealingItemClass, GetActorLocation(), FRotator(0.f, FMath::RandRange(0.f, 359.f), 0.f))->ItemMeshComp->AddImpulse(RandomDirection);
		}
	}
	else
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "ERROR! FOOD/HEALTH INVENTORY ITEM CLASS WAS NOT SET IN BLUEPRINT!", true);
		}
	}

	if (KeyItemClass)
	{
		for (int32 i = KeysCarried; i > 0; i--)
		{
			FActorSpawnParameters SpawnParameters;
			SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			GetWorld()->SpawnActor<APickupItem>(KeyItemClass, GetActorLocation(), FRotator::ZeroRotator, SpawnParameters);
		}
	}
	else
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "ERROR! KEY INVENTORY ITEM CLASS WAS NOT SET IN BLUEPRINT!", true);
		}
	}

	// Drop potions, gold and unsheathed weapons on death.
	if (EquippedWeapon && WeaponReady)
	{
		AStaticMeshActor* DroppedWeapon = GetWorld()->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), EquippedWeapon->GetComponentLocation(), EquippedWeapon->GetComponentRotation());
		EquippedWeapon->SetVisibility(false, true);
		
		DroppedWeapon->GetStaticMeshComponent()->SetMobility(EComponentMobility::Movable);
		DroppedWeapon->GetStaticMeshComponent()->SetStaticMesh(EquippedWeapon->WeaponData.WeaponMesh);

		DroppedWeapon->GetStaticMeshComponent()->bCastDynamicShadow = true;
		DroppedWeapon->GetStaticMeshComponent()->CastShadow = true;
		DroppedWeapon->GetStaticMeshComponent()->BodyInstance.SetObjectType(ECC_WorldDynamic);
		DroppedWeapon->GetStaticMeshComponent()->BodyInstance.SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		DroppedWeapon->GetStaticMeshComponent()->BodyInstance.SetResponseToAllChannels(ECR_Ignore);
		DroppedWeapon->GetStaticMeshComponent()->BodyInstance.SetResponseToChannel(ECC_WorldStatic, ECR_Block);
		DroppedWeapon->GetStaticMeshComponent()->BodyInstance.SetResponseToChannel(ECC_WorldDynamic, ECR_Block);
		DroppedWeapon->GetStaticMeshComponent()->BodyInstance.SetResponseToChannel(ECC_Pawn, ECR_Overlap);
		DroppedWeapon->GetStaticMeshComponent()->SetHiddenInGame(false);
											 
		DroppedWeapon->GetStaticMeshComponent()->SetCanEverAffectNavigation(false);
											 
		DroppedWeapon->GetStaticMeshComponent()->bAlwaysCreatePhysicsState = true;
		DroppedWeapon->GetStaticMeshComponent()->SetSimulatePhysics(true);
	}

	EquippedWeapon->WeaponMesh->DestroyComponent();
	EquippedWeapon->WeaponCollision->DestroyComponent();
	EquippedWeapon->DestroyComponent();
}

void ABaseCharacter::ReceiveAttackDamage(ABaseCharacter* DamageDealer, float Amount, FVector ImpactPoint)
{
	if ((DamageDealer->CharacterFaction != CharacterFaction) && (CharacterMovementState != ECharacterMovementState::CMS_Dodging))
	{
		bool ApplyDamage = false;

		// Checks if the damage cooldown has passed.
		if (((GetWorld()->GetTimeSeconds() - LastReceivedDamage) < DamageCooldown))
		{
			// If it has not passed, is the damage dealt by the same character as last time?
			if (DamageReceivedByCharacter == DamageDealer)
			{
				// Nothing happens...
			}
			else
			{
				ApplyDamage = true;
			}
		}
		else
		{
			ApplyDamage = true;
		}

		if (ApplyDamage)
		{
			// Sets and plays the appropriate sound cue for the weapon
			AttackImpactSFX->SetSound(DamageDealer->EquippedWeapon->WeaponData.WeaponImpactCue);
			AttackImpactSFX->Play();

			if (ImpactFX)
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactFX, GetActorLocation() + FVector(0.f, 0.f, 45.f));
			}

			LastReceivedDamage = GetWorld()->GetTimeSeconds();
			DamageReceivedByCharacter = DamageDealer;
			AffectHealth(Amount * (1 - DamageResistance), true);

			KnockbackStart = GetActorLocation();
			CharacterIsKnockedBack = true;
		}
	}
}

void ABaseCharacter::Knockback(float DeltaTime)
{
	if (FVector::Distance(GetActorLocation(), KnockbackStart) > 75.f)
	{
		CharacterIsKnockedBack = false;
	}

	if (CharacterIsKnockedBack && DamageReceivedByCharacter)
	{
		// Knockback effect
		FVector BaseDirection = GetActorLocation() - DamageReceivedByCharacter->GetActorLocation();
		BaseDirection.Normalize();

		float DistanceMult = 15.f;// * DeltaTime;

		FVector NewDirection = GetActorLocation() + (BaseDirection * DistanceMult);
		FVector FinalCalculation = FVector(NewDirection.X, NewDirection.Y, GetActorLocation().Z);

		SetActorLocation(FinalCalculation);
	}
}

void ABaseCharacter::StartSprint()
{
	if (!IsLockedOn)
	{
		CharacterSpeedState = ECharacterSpeed::CS_Sprinting;
		SprintTimeline.Play();
	}
}

void ABaseCharacter::StopSprint()
{
	SprintingShouldStop = true;
	SprintTimeline.Reverse();
}

void ABaseCharacter::DodgeRoll()
{
	if (CharacterActionState == ECharacterActionState::CAS_None
		&& (CharacterMovementState == ECharacterMovementState::CMS_Moving
		&& CharacterAttackState != ECharacterAttackState::CAT_AttackBegin
		&& CharacterAttackState != ECharacterAttackState::CAT_AttackChain
		&& !EquipOrUnequipInProgress
		&& !ReceivingAttackInput))
	{
		// Cancel any attack that is being charged up!
		AttackCancel();

		CharacterSpeedState = ECharacterSpeed::CS_Sprinting;

		CharacterActionState = ECharacterActionState::CAS_Dodging;
		CharacterMovementState = ECharacterMovementState::CMS_Dodging;

		// Play SFX
		ActionSFX->SetSound(DodgeCue);
		ActionSFX->Play();

		if (ImpactFX)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactFX, GetMesh()->GetBoneLocation("root"));
		}

		DodgeTimeline.PlayFromStart();
	}
}

void ABaseCharacter::OnOverlapCapsuleResponse(UPrimitiveComponent * OverlapComponent, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	if (Cast<ABaseCharacter>(OtherActor))
	{
		// Ensures the weapon hits the characters capsule and nothing else. Might be changed into the character mesh itself.
		if ((Cast<ABaseCharacter>(OtherActor)->GetCapsuleComponent() == Cast<UCapsuleComponent>(OtherComp)))
		{
			CollidingCharacter = Cast<ABaseCharacter>(OtherActor);
		}
	}
}

void ABaseCharacter::CharacterPushForce(float DeltaTime)
{
	// Enables/Disables the blocking collision of the capsule component, so a locked on character cant just run through its target
	if (IsTargetOfOtherLockOn || IsLockedOn || (CharacterActionState == ECharacterActionState::CAS_Attacking))
	{
		GetCapsuleComponent()->BodyInstance.SetResponseToChannel(ECC_Pawn, ECR_Block);
	}
	else
	{
		GetCapsuleComponent()->BodyInstance.SetResponseToChannel(ECC_Pawn, ECR_Overlap);
	}

	if (CollidingCharacter && GetCapsuleComponent()->IsOverlappingComponent(CollidingCharacter->GetCapsuleComponent()))
	{
		//float CharacterDistance = FVector::DistSquared(CollidingCharacter->GetActorLocation(), GetActorLocation());

		FVector BaseDirection = GetActorLocation() - CollidingCharacter->GetActorLocation();
		BaseDirection.Normalize();
		
		float DistanceMult = 50.f * DeltaTime;

		FVector NewDirection = GetActorLocation() + (BaseDirection * DistanceMult);
		FVector FinalCalculation = FVector(NewDirection.X, NewDirection.Y, GetActorLocation().Z);

		SetActorLocation(FinalCalculation);
	}
	else
	{
		CollidingCharacter = nullptr;
	}
}

void ABaseCharacter::SurfaceTypeCheck()
{
	// Makes sure the character is not in the sea or in the air
	if ((CharacterSurfaceType != ECharacterSurfaceType::CST_Sea) && (!GetCharacterMovement()->IsFalling()))
	{
		if (GetWorld() != NULL)
		{
			const FVector TraceStart = GetActorLocation();
			const FVector TraceEnd = GetActorLocation() - FVector(0.f, 0.f, 128.f);

			FHitResult TraceResult;// (ForceInit);

			FCollisionQueryParams CollisionQParams;
			CollisionQParams.bTraceComplex = false;
			CollisionQParams.bReturnPhysicalMaterial = true;
			CollisionQParams.AddIgnoredActor(this);
			
			FCollisionResponseParams CollisionRParams;
			CollisionRParams.DefaultResponseParam;

			GetWorld()->LineTraceSingleByChannel(TraceResult, TraceStart, TraceEnd, ECC_Visibility, CollisionQParams, CollisionRParams);

			//DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Cyan, false, 15.f);
			if (TraceResult.IsValidBlockingHit())
			{
				if (TraceResult.GetActor())
				{
					switch (UPhysicalMaterial::DetermineSurfaceType(TraceResult.PhysMaterial.Get()))
					{
					case EPhysicalSurface::SurfaceType1:
						CharacterSurfaceType = ECharacterSurfaceType::CST_Sand;
						break;
					case EPhysicalSurface::SurfaceType2:
						CharacterSurfaceType = ECharacterSurfaceType::CST_Rock;
						break;
					case EPhysicalSurface::SurfaceType3:
						CharacterSurfaceType = ECharacterSurfaceType::CST_Grass;
						break;
					case EPhysicalSurface::SurfaceType4:
						CharacterSurfaceType = ECharacterSurfaceType::CST_Wood;
						break;
					case EPhysicalSurface::SurfaceType5:
						CharacterSurfaceType = ECharacterSurfaceType::CST_Water;
						break;
					case EPhysicalSurface::SurfaceType6:
						// Updating the last valid location if the character is on an island
						LastValidLocation = GetActorLocation();

						if (GetActorLocation().Z < 225.f)
						{
							CharacterSurfaceType = ECharacterSurfaceType::CST_Sand;
						}
						else
						{
							// Dot product value of up-vector * the polygon normal of the terrain below the character
							if (FVector::DotProduct(TraceResult.ImpactNormal, FVector::UpVector) < 0.8f)
							{
								CharacterSurfaceType = ECharacterSurfaceType::CST_Rock;
							}
							else
							{
								CharacterSurfaceType = ECharacterSurfaceType::CST_Grass;
							}
						}
						break;
					default:
						//GEngine->AddOnScreenDebugMessage(1, 5.f, FColor::Red, "ERROR: UNDEFINED SURFACE TYPE! NO PHYSICAL MATERIAL SET!", true);
						break;
					}
				}
			}
		}
	}

	switch (CharacterSurfaceType)
	{
	case ECharacterSurfaceType::CST_Sand:
		FootstepSFX->SetIntParameter(FName("TerrainType"), 0);
		ActionSFX->SetIntParameter(FName("TerrainType"), 0);
		//GEngine->AddOnScreenDebugMessage(1, 5.f, FColor::Silver, "Sand!", true);
		break;
	case ECharacterSurfaceType::CST_Grass:
		FootstepSFX->SetIntParameter(FName("TerrainType"), 1);
		ActionSFX->SetIntParameter(FName("TerrainType"), 1);
		//GEngine->AddOnScreenDebugMessage(1, 5.f, FColor::Green, "Grass!", true);
		break;
	case ECharacterSurfaceType::CST_Rock:
		FootstepSFX->SetIntParameter(FName("TerrainType"), 3);
		ActionSFX->SetIntParameter(FName("TerrainType"), 3);
		//GEngine->AddOnScreenDebugMessage(1, 5.f, FColor::Black, "Rock!", true);
		break;
	case ECharacterSurfaceType::CST_Wood:
		FootstepSFX->SetIntParameter(FName("TerrainType"), 4);
		ActionSFX->SetIntParameter(FName("TerrainType"), 4);
		//GEngine->AddOnScreenDebugMessage(1, 5.f, FColor::Cyan, "Wood!", true);
		break;
	case ECharacterSurfaceType::CST_Water:
		FootstepSFX->SetIntParameter(FName("TerrainType"), 2);
		ActionSFX->SetIntParameter(FName("TerrainType"), 2);
		//GEngine->AddOnScreenDebugMessage(1, 5.f, FColor::Blue, "Water!", true);
		break;
	case ECharacterSurfaceType::CST_Sea:
		FootstepSFX->SetIntParameter(FName("TerrainType"), 2);
		ActionSFX->SetIntParameter(FName("TerrainType"), 2);
		//GEngine->AddOnScreenDebugMessage(1, 5.f, FColor::Blue, "Water!", true);
		break;
	}
}

bool ABaseCharacter::CanLockOnto()
{
	ValidTargets.Empty();

	// Decides what actor type to allow, then adds all actors of type to the array.
	UClass* ActorFilter = ABaseCharacter::StaticClass();
	LockOnRangeSphere->GetOverlappingActors(ValidTargets, ActorFilter);

	ABaseCharacter* TempCharacter = nullptr;

	// Filter out the player.
	ValidTargets.RemoveSingle(this);

	if (ValidTargets.Num() > 0)
	{
		for (auto PossibleTarget : ValidTargets)
		{
			TempCharacter = Cast<ABaseCharacter>(PossibleTarget);

			// Ensures that the character is not of the same faction as this character or dead.
			if ((TempCharacter->CharacterFaction != CharacterFaction) &&
				(TempCharacter->CharacterLifeState != ECharacterLifeState::CLS_Dead))
			{
				if (!LockedTargetCharacter)
				{
					LockedTargetCharacter = Cast<ABaseCharacter>(PossibleTarget);
				}

				if (PossibleTarget != LockedTargetCharacter)
				{
					if (FVector::Distance(GetActorLocation(), PossibleTarget->GetActorLocation()) < FVector::Distance(GetActorLocation(), LockedTargetCharacter->GetActorLocation()))
					{
						if (true) // REPLACE WITH A BASIC FUNCTION THAT IS OVERWRITTEN IN NPC AND PLAYER CLASSES!!!
						{
							LockedTargetCharacter = Cast<ABaseCharacter>(PossibleTarget);
						}
					}
				}
			}
		}

		if (LockedTargetCharacter)
		{
			// Makes the character aware of the lock on.
			LockedTargetCharacter->IsTargetOfOtherLockOn = true;
			LockedTargetCharacter->LockedOnBy = this;
			return true;
		}

		return false;		
	}
	else
	{
		return false;
	}
}

void ABaseCharacter::LockOntoTarget()
{
	if (CharacterSpeedState == ECharacterSpeed::CS_Sprinting)
	{
		StopSprint();
	}

	if (CanLockOnto())
	{
		IsLockedOn = true;
	}
	else
	{
		StopLockOntoTarget();
	}
}

void ABaseCharacter::StopLockOntoTarget()
{
	if (LockedTargetCharacter)
	{
		LockedTargetCharacter->IsTargetOfOtherLockOn = false;
		LockedTargetCharacter->LockedOnBy = nullptr;
		LockedTargetCharacter = nullptr;
	}

	IsLockedOn = false;
	bUseControllerRotationYaw = false;

	CharacterSpeedState = ECharacterSpeed::CS_Walking;
}

void ABaseCharacter::CheckIfLockStillValid()
{
	if (IsLockedOn)
	{
		if	(LockedTargetCharacter && (LockedTargetCharacter->CharacterLifeState != ECharacterLifeState::CLS_Dead) &&
			(LockedTargetCharacter->GetActorLocation() - GetActorLocation()).Size() < LockOnSphereRadius * 2.f)
		{
			// Set the rotation of the actor to face its target if its locked on.
			FVector LockedOnDirection = LockedTargetCharacter->GetActorLocation() - GetActorLocation();
			FRotator TowardsTargetDirection = LockedOnDirection.ToOrientationRotator();
			Controller->ClientSetRotation(FRotator(-LockOnCameraHeight, TowardsTargetDirection.Yaw, 0.f));

			CharacterSpeedState = ECharacterSpeed::CS_Walking;
		}
		else
		{
			StopLockOntoTarget();
		}
	}
}

void ABaseCharacter::MovementStateHandler()
{
	if (GetCharacterMovement()->IsFalling())
	{
		//CharacterMovementStatePrevious = CharacterMovementState;
		CharacterMovementState = ECharacterMovementState::CMS_Jumping;
	}
	else if (CharacterMovementState != ECharacterMovementState::CMS_Dodging)
	{
		if ((GetVelocity().Size() > 0.f) && (CharacterMovementState != ECharacterMovementState::CMS_Knockback))
		{
			//CharacterMovementStatePrevious = CharacterMovementState;
			CharacterMovementState = ECharacterMovementState::CMS_Moving;
		}
		else
		{
			//CharacterMovementStatePrevious = CharacterMovementState;
			CharacterMovementState = ECharacterMovementState::CMS_Idle;
		}
	}
}

void ABaseCharacter::MovementVariableHandler()
{
	switch (CharacterMovementState)
	{
	case ECharacterMovementState::CMS_Idle:
		GetCharacterMovement()->RotationRate = DefaultRotationRate;
		GetCharacterMovement()->GroundFriction = DefaultGroundFriction;
		SprintingShouldStop = false;
		break;
	case ECharacterMovementState::CMS_Moving:
		UpdateSpeedVariables();
		break;
	case ECharacterMovementState::CMS_Jumping:
		GetCharacterMovement()->RotationRate = FRotator(0.f, 64.f, 0.f);
		GetCharacterMovement()->GroundFriction = DefaultGroundFriction;
		break;
	case ECharacterMovementState::CMS_Knockback:
		GetCharacterMovement()->RotationRate = FRotator(0.f, 64.f, 0.f);
		GetCharacterMovement()->GroundFriction = 0.f;
		//MovementSpeedCurrent = knockbackPower???
		break;
	case ECharacterMovementState::CMS_Dodging:
		GetCharacterMovement()->RotationRate = FRotator(0.f, 512.f, 0.f);
		GetCharacterMovement()->GroundFriction = 0.f;
		MovementSpeedCurrent = MovementSpeedSprinting * SpeedBaseMultiplier * SpeedMovementMultiplier;
		// Actual movement is handled by "HandleDodgeProgress"
	}

	GetCharacterMovement()->MaxWalkSpeed = MovementSpeedCurrent;
}

void ABaseCharacter::MovementDirectionHandler(ECharacterDirectionState InputValue)
{
	if (CharacterMovementState == ECharacterMovementState::CMS_Moving || CharacterMovementState == ECharacterMovementState::CMS_Dodging || CharacterMovementState == ECharacterMovementState::CMS_Jumping)
	{
		CharacterDirectionState = InputValue;
	}
	else
	{
		CharacterDirectionState = ECharacterDirectionState::CDS_None;
	}
}

void ABaseCharacter::UpdateBaseMovementSpeed()
{
	float AdditionalMultiplier{ 1.f };

	if (WeaponReady)
		AdditionalMultiplier *= 0.85f;
	
	if (IsLockedOn)
		AdditionalMultiplier *= 0.9f;

	switch (CharacterActionState)
	{
	case ECharacterActionState::CAS_None:
		SpeedBaseMultiplier = 1.f * SpeedMovementMultiplierTarget * AdditionalMultiplier;
		break;
	case ECharacterActionState::CAS_KnockedDown:
		SpeedBaseMultiplier = 0.f * AdditionalMultiplier;
		break;
	case ECharacterActionState::CAS_Ragdoll:
		SpeedBaseMultiplier = 0.f * AdditionalMultiplier;
		break;
	case ECharacterActionState::CAS_Attacking:
		SpeedBaseMultiplier = 0.5f * SpeedMovementMultiplierTarget * AdditionalMultiplier;
		break;
	case ECharacterActionState::CAS_Dodging:
		SpeedBaseMultiplier = 1.f * SpeedMovementMultiplierTarget * AdditionalMultiplier;
		break;
	case ECharacterActionState::CAS_Interacting:
		SpeedBaseMultiplier = 0.75f * SpeedMovementMultiplierTarget * AdditionalMultiplier;
		break;
	default:
		SpeedBaseMultiplier = 1.f * SpeedMovementMultiplierTarget * AdditionalMultiplier;
		break;
	}
}

void ABaseCharacter::UpdateSpeedVariables()
{
	if (CharacterActionState != ECharacterActionState::CAS_Dodging)
	{
		switch (CharacterSpeedState)
		{
		case ECharacterSpeed::CS_Walking:
			GetCharacterMovement()->RotationRate = DefaultRotationRate;
			GetCharacterMovement()->GroundFriction = DefaultGroundFriction;
			MovementSpeedCurrent = MovementSpeedWalking * SpeedBaseMultiplier * SpeedMovementMultiplier;
			break;
		case ECharacterSpeed::CS_Sprinting:
			GetCharacterMovement()->RotationRate = FRotator(0.f, 350.f, 0.f);
			GetCharacterMovement()->GroundFriction = DefaultGroundFriction;
			// Sprinting speed handled by the "HandleSprintProgress" function.
			break;
		}
	}
}

void ABaseCharacter::PlayFootstepFX(bool Right)
{
	if (!GetCharacterMovement()->IsFalling())
	{
		// Name of the skeletal bones used to orient the footprint decal
		FName FootBoneName{"NULL"};
		FName BallBoneName{"NULL"};

		// Angle used to adjust the footprint rotation
		float AngleCorrection{ 0.f };

		// Decal to currently use
		UMaterialInterface* FootprintDecal{ nullptr };

		if (Right)
		{
			FootBoneName = "foot_r";
			BallBoneName = "ball_r";
			AngleCorrection = 95.f;
			FootprintDecal = FootprintDecalRight;
		}
		else
		{
			FootBoneName = "foot_l";
			BallBoneName = "ball_l";
			AngleCorrection = -95.f;
			FootprintDecal = FootprintDecalLeft;
		}

		switch (CharacterSurfaceType)
		{
		case ECharacterSurfaceType::CST_Grass:
			if (DustFX)
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), DustFX, GetMesh()->GetBoneLocation(FootBoneName));

			if (FootprintDecal)
				UGameplayStatics::SpawnDecalAtLocation(GetWorld(), FootprintDecal, FVector(32.f, 12.f, 12.f), GetMesh()->GetBoneLocation(FootBoneName), FRotator(-90.f, AngleCorrection, (GetMesh()->GetBoneAxis(BallBoneName, EAxis::X) * -1.f).Rotation().Yaw), 60.f);
			break;
		case ECharacterSurfaceType::CST_Rock:
			if (DustFX)
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), DustFX, GetMesh()->GetBoneLocation(FootBoneName));
			break;
		case ECharacterSurfaceType::CST_Sand:
			if (DustFX)
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), DustFX, GetMesh()->GetBoneLocation(FootBoneName));

			if (FootprintDecal)
				UGameplayStatics::SpawnDecalAtLocation(GetWorld(), FootprintDecal, FVector(32.f, 12.f, 12.f), GetMesh()->GetBoneLocation(FootBoneName), FRotator(-90.f, AngleCorrection, (GetMesh()->GetBoneAxis(BallBoneName, EAxis::X) * -1.f).Rotation().Yaw), 60.f);
			break;
		case ECharacterSurfaceType::CST_Sea:
			if (WaterFX)
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), WaterFX, GetMesh()->GetBoneLocation(FootBoneName));

			if (FootprintDecal)
				UGameplayStatics::SpawnDecalAtLocation(GetWorld(), FootprintDecal, FVector(32.f, 12.f, 12.f), GetMesh()->GetBoneLocation(FootBoneName), FRotator(-90.f, AngleCorrection, (GetMesh()->GetBoneAxis(BallBoneName, EAxis::X) * -1.f).Rotation().Yaw), 60.f);
			break;
		case ECharacterSurfaceType::CST_Water:
			if (WaterFX)
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), WaterFX, GetMesh()->GetBoneLocation(FootBoneName));

			if (FootprintDecal)
				UGameplayStatics::SpawnDecalAtLocation(GetWorld(), FootprintDecal, FVector(32.f, 12.f, 12.f), GetMesh()->GetBoneLocation(FootBoneName), FRotator(-90.f, AngleCorrection, (GetMesh()->GetBoneAxis(BallBoneName, EAxis::X) * -1.f).Rotation().Yaw), 60.f);
			break;
		case ECharacterSurfaceType::CST_Wood:
			if (DustFX)
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), DustFX, GetMesh()->GetBoneLocation(FootBoneName));
			break;
		}

		FootstepSFX->Play();
	}
}

void ABaseCharacter::InitializeTimelines()
{
	if (SprintAcceleration)
	{
		FOnTimelineFloat ProgressFunction;
		ProgressFunction.BindUFunction(this, FName("HandleSprintProgress"));

		SprintTimeline.AddInterpFloat(SprintAcceleration, ProgressFunction);
		SprintTimeline.SetLooping(false);
		SprintTimeline.SetTimelineLengthMode(ETimelineLengthMode::TL_LastKeyFrame);
	}

	if (DodgeMovement)
	{
		FOnTimelineFloat ProgressFunction;
		ProgressFunction.BindUFunction(this, FName("HandleDodgeProgress"));

		FOnTimelineEvent FinishedFunction;
		FinishedFunction.BindUFunction(this, FName("HandleDodgeFinished"));

		DodgeTimeline.AddInterpFloat(DodgeMovement, ProgressFunction);
		DodgeTimeline.SetTimelineFinishedFunc(FinishedFunction);
		DodgeTimeline.SetLooping(false);
		DodgeTimeline.SetTimelineLengthMode(ETimelineLengthMode::TL_LastKeyFrame);
	}
}

void ABaseCharacter::InertiaLeaning(float DeltaTime)
{
	RotationalVelocity = GetCapsuleComponent()->GetPhysicsAngularVelocityInDegrees().Z;

	FRotator OriginalRotation = FRotator(0.f, -90.f, 0.f);
	FRotator CurrentRotation = GetMesh()->GetRelativeRotation();

	float LeanDirection = 1.f;

	if (FMath::IsNegativeFloat(RotationalVelocity))
	{
		LeanDirection = -1.f;
	}
	else
	{
		LeanDirection = 1.f;
	}

	if (CharacterMovementState == ECharacterMovementState::CMS_Moving && !IsLockedOn)
	{
		if (FMath::Abs(RotationalVelocity) > 1.f)
		{
			switch (CharacterSpeedState)
			{
			case ECharacterSpeed::CS_Walking:
				GetMesh()->SetRelativeRotation(FMath::RInterpTo(CurrentRotation, FRotator(5.f * LeaningMultiplier * LeanDirection, -90.f, 0.f), DeltaTime, LeaningInterpSpeed));
				break;
			case ECharacterSpeed::CS_Sprinting:
				GetMesh()->SetRelativeRotation(FMath::RInterpTo(CurrentRotation, FRotator(15.f * LeaningMultiplier * LeanDirection, -90.f, 0.f), DeltaTime, LeaningInterpSpeed));
				break;
			default:
				GetMesh()->SetRelativeRotation(FMath::RInterpTo(CurrentRotation, OriginalRotation, DeltaTime, 5.f));
				break;
			}
		}
		else
		{
			GetMesh()->SetRelativeRotation(FMath::RInterpTo(CurrentRotation, OriginalRotation, DeltaTime, LeaningInterpSpeedStop));
		}
	}
	else
	{
		GetMesh()->SetRelativeRotation(FMath::RInterpTo(CurrentRotation, OriginalRotation, DeltaTime, LeaningInterpSpeedStop));
	}
}

void ABaseCharacter::CalculateVelocity()
{
	FVector DirectionalVelocityVector = UKismetMathLibrary::InverseTransformDirection(GetActorTransform(), GetVelocity());

	ForwardVelocity = DirectionalVelocityVector.X;
	RightVelocity = DirectionalVelocityVector.Y;
}

void ABaseCharacter::HandleSprintProgress(float Value)
{
	MovementSpeedCurrent = FMath::Lerp(MovementSpeedWalking, MovementSpeedSprinting * SpeedBaseMultiplier * SpeedMovementMultiplier, Value);

	if (SprintTimeline.IsReversing() && SprintingShouldStop && MovementSpeedCurrent < (MovementSpeedWalking + 10.f))
	{
		HandleSprintFinished();
	}
}

void ABaseCharacter::HandleSprintFinished()
{
	CharacterMovementState = ECharacterMovementState::CMS_Idle;
	CharacterSpeedState = ECharacterSpeed::CS_Walking;
	
	SprintingShouldStop = false;
}

void ABaseCharacter::HandleDodgeProgress(float Value)
{
	if (!IsLockedOn)
	{
		AddMovementInput(GetVelocity(), Value);
	}
	else
	{
		FVector LockedOnDirection = LockedTargetCharacter->GetActorLocation() - GetActorLocation();
		FRotator TowardsTargetDirection = LockedOnDirection.ToOrientationRotator();

		bool InvertDirection = false;

		switch (CharacterDirectionState)
		{
		case ECharacterDirectionState::CDS_Forwards:
			AddMovementInput(GetVelocity(), Value);
			break;
		case ECharacterDirectionState::CDS_Backwards:
			AddMovementInput(GetVelocity(), Value);
			break;
		case ECharacterDirectionState::CDS_Right:
			InvertDirection = false;
			break;
		case ECharacterDirectionState::CDS_Left:
			InvertDirection = true;
			break;
		case ECharacterDirectionState::CDS_ForwardsRight:
			AddMovementInput(GetVelocity(), Value);
			break;
		case ECharacterDirectionState::CDS_ForwardsLeft:
			AddMovementInput(GetVelocity(), Value);
			break;
		case ECharacterDirectionState::CDS_BackwardsRight:
			AddMovementInput(GetVelocity(), Value);
			break;
		case ECharacterDirectionState::CDS_BackwardsLeft:
			AddMovementInput(GetVelocity(), Value);
			break;
		default:
			AddMovementInput(GetVelocity(), Value);
			break;
		}

		if (InvertDirection)
		{
			// Subtracts "Value * 9" from Yaw, to roughly keep the character moving in a circle around the target.
			FRotator FixedRotation(0.f, TowardsTargetDirection.Yaw + Value * 9.f, 0.f);

			LockedOnDirection = FRotationMatrix(FixedRotation).GetUnitAxis(EAxis::Y);
			LockedOnDirection.Normalize();

			AddMovementInput(-LockedOnDirection, Value);
		}
		else
		{
			// Subtracts "Value * 9" from Yaw, to roughly keep the character moving in a circle around the target.
			FRotator FixedRotation(0.f, TowardsTargetDirection.Yaw - Value * 9.f, 0.f);

			LockedOnDirection = FRotationMatrix(FixedRotation).GetUnitAxis(EAxis::Y);
			LockedOnDirection.Normalize();

			AddMovementInput(LockedOnDirection, Value);
		}
	}
}

void ABaseCharacter::HandleDodgeFinished()
{
	CharacterActionState = ECharacterActionState::CAS_None;
	CharacterMovementState = ECharacterMovementState::CMS_Idle;

	if (ReceivingSprintInput)
	{
		CharacterSpeedState = ECharacterSpeed::CS_Sprinting;
	}
	else
	{
		CharacterSpeedState = ECharacterSpeed::CS_Walking;
	}
}

void ABaseCharacter::CharacterRotationHandler(float DeltaTime)
{
	// LERP Character rotation towards the locked on target!
	if (IsLockedOn && CharacterType == ECharacterType::CT_Humanoid)
	{
		// Add funtionality that can signal the AnimBP that a turning animations should be played when a character is turning in a spot!
		SetActorRotation(FMath::RInterpTo(GetActorRotation(), FRotator(0.f, GetControlRotation().Yaw, 0.f), DeltaTime, 10.f));

		if (FMath::IsNearlyEqual(GetActorRotation().Yaw, GetControlRotation().Yaw, 0.5f))
		{
			bUseControllerRotationYaw = true;
		}
	}

	switch (CharacterType)
	{
	case ECharacterType::CT_Humanoid:
		InertiaLeaning(DeltaTime);
		break;
	case ECharacterType::CT_Monster:
		// Function for aligning character with ground goes here.
		break;
	case ECharacterType::CT_Critter:
		// Possibly the same as above?
		break;
	default:
		break;
	}
}

void ABaseCharacter::PlaySound_Attack()
{
	if (!VoiceSFX->IsPlaying())
	{
		VoiceSFX->SetSound(AttackCue);
		VoiceSFX->Play();
	}
};

void ABaseCharacter::PlaySound_Idle()
{
	VoiceSFX->SetSound(IdleCue);
	VoiceSFX->Play();
};

void ABaseCharacter::PlaySound_SpotPlayer()
{
	VoiceSFX->SetSound(SpotPlayerCue);
	VoiceSFX->Play();
};