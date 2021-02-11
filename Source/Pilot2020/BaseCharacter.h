// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"

#include "UObject/Class.h"
#include "Templates/SubclassOf.h"
#include "Components/TimelineComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/StaticMeshActor.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Engine/DecalActor.h"
#include "Components/DecalComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"

#include "WeaponSceneComponent.h"

#include "BaseCharacter.generated.h"


UENUM(BlueprintType)
enum class ECharacterType : uint8
{
	CT_Humanoid	UMETA(DisplayName = "Humanoid"),
	CT_Monster 	UMETA(DisplayName = "Monster"),
	CT_Critter	UMETA(DisplayName = "Critter")
};

UENUM(BlueprintType)
enum class ECharacterFaction : uint8
{
	CF_Player	UMETA(DisplayName = "Mizu's Crew"),
	CF_Bandits 	UMETA(DisplayName = "Bandits"),
	CF_Monsters	UMETA(DisplayName = "Monsters"),
	CF_Critters	UMETA(DisplayName = "Creatures")
};

UENUM(BlueprintType)
enum class ECharacterLifeState : uint8
{
	CLS_Alive		UMETA(DisplayName = "Alive"),
	CLS_Wounded 	UMETA(DisplayName = "Wounded"),
	CLS_NearDeath	UMETA(DisplayName = "Near Death"),
	CLS_Dead		UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class ECharacterActionState : uint8
{
	CAS_None		UMETA(DisplayName = "None"),
	CAS_KnockedDown	UMETA(DisplayName = "Knocked Down"),
	CAS_Ragdoll		UMETA(DisplayName = "Ragdolled"),
	CAS_Attacking	UMETA(DisplayName = "Attacking"),
	CAS_Dodging		UMETA(DisplayName = "Dodging"),
	CAS_Interacting	UMETA(DisplayName = "Interacting")
};

UENUM(BlueprintType)
enum class ECharacterAttackState : uint8
{
	CAT_None			UMETA(DisplayName = "Attack None"),
	CAT_AttackBegin		UMETA(DisplayName = "Attack Begin"),
	CAT_ChargeUp		UMETA(DisplayName = "Attack Charge"),
	CAT_LightAttack		UMETA(DisplayName = "Attack Light"),
	CAT_HeavyAttack		UMETA(DisplayName = "Attack Heavy"),
	CAT_DropAttack		UMETA(DisplayName = "Attack Drop"),
	CAT_SpecialAttack	UMETA(DisplayName = "Attack Special"),
	CAT_AttackFinished	UMETA(DisplayName = "Attack Finished"),
	CAT_AttackChain		UMETA(DisplayName = "Attack Chain")
};

UENUM(BlueprintType)
enum class ECharacterSpeed : uint8
{
	CS_Walking		UMETA(DisplayName = "Walking"),
	CS_Sprinting	UMETA(DisplayName = "Sprinting")
};

UENUM(BlueprintType)
enum class ECharacterMovementState : uint8
{
	CMS_Idle		UMETA(DisplayName = "Idle"),
	CMS_Moving		UMETA(DisplayName = "Moving"),
	CMS_Jumping		UMETA(DisplayName = "Jumping"),
	CMS_Knockback	UMETA(DisplayName = "Knockback"),
	CMS_Dodging		UMETA(DisplayName = "Dodging")
};

UENUM(BlueprintType)
enum class ECharacterDirectionState : uint8
{
	CDS_None			UMETA(DisplayName = "No Direction"),
	CDS_Forwards		UMETA(DisplayName = "Forwards"),
	CDS_Backwards		UMETA(DisplayName = "Backwards"),
	CDS_Right			UMETA(DisplayName = "Right"),
	CDS_Left			UMETA(DisplayName = "Left"),
	CDS_ForwardsRight	UMETA(DisplayName = "Forwards Right"),
	CDS_ForwardsLeft	UMETA(DisplayName = "Forwards Left"),
	CDS_BackwardsRight	UMETA(DisplayName = "Backwards Right"),
	CDS_BackwardsLeft	UMETA(DisplayName = "Backwards Left")
};

UENUM(BlueprintType)
enum class ECharacterSurfaceType : uint8
{
	CST_Sand	UMETA(DisplayName = "Sand"),
	CST_Grass	UMETA(DisplayName = "Grass"),
	CST_Rock	UMETA(DisplayName = "Rock"),
	CST_Wood	UMETA(DisplayName = "Wood"),
	CST_Water	UMETA(DisplayName = "Water"),
	CST_Sea		UMETA(DisplayName = "Sea")
};

class APickupItem;

UCLASS()
class PILOT2020_API ABaseCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABaseCharacter();

protected:
	// Called after components have been initialized
	virtual void PostInitializeComponents() override;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//==============================
	// D E B U G G I N G
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character - Debugging")
		bool ShowWeaponCollision{ false };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character - Debugging")
		bool ShowLockOnSphereCollision{ false };

	//==================================
	// C O M P O N E N T S  -  M I S C
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		UWeaponSceneComponent* EquippedWeapon{ nullptr };

	//====================================
	// C O M P O N E N T S  -  A U D I O
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		UAudioComponent* FootstepSFX{ nullptr };

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		UAudioComponent* AttackSwingSFX{ nullptr };

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		UAudioComponent* AttackImpactSFX{ nullptr };

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		UAudioComponent* VoiceSFX{ nullptr };

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		UAudioComponent* ActionSFX{ nullptr };

	//==============================
	// A U D I O   C U E S
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character - Audio - Cues")
		USoundCue* FootstepCue{ nullptr };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character - Audio - Cues")
		USoundCue* DodgeCue{ nullptr };
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character - Audio - Cues")
		USoundCue* VoiceCue{ nullptr };
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character - Audio - Cues")
		USoundCue* ActionCue{ nullptr };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character - Audio - Cues")
		USoundCue* EatCue{ nullptr };
	
	//==============================
	// S T A T S
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character - General - Values")
		bool IsCharacterPlayer{ false };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character - General - Values")
		float Health{ 100.f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character - General - Values")
		float HealthMaximum{ 100.f };

	// A variable for temporarily adjusting the movement speed of the character.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character - General - Values")
		float SpeedBaseMultiplier{ 1.f };

	// Apply a permanent modifier to the characters move speed.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character - General - Values")
		float SpeedMovementMultiplierTarget{ 1.f };

	UPROPERTY()
		float SpeedMovementMultiplier{ 1.f };

	// Apply a permanent modifier to the characters attack speed.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character - Combat - Attacking")
		float SpeedAttackMultiplier{ 1.f };

	// Apply a permanent modifier to the characters attack damage.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character - Combat - Attacking")
		float DamageMultiplier{ 1.f };

	// Apply a permanent modifier to the characters damage resistance.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character - Combat - Defense")
		float DamageResistance{ 0.f };

	// If the character has their weapon drawn.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character - Combat - Attacking")
		bool WeaponReady{ false };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character - Combat - Attacking")
		bool AttackInProgress{ false };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character - Combat - Attacking")
		bool DamageCanBeDealt{ false };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character - General - Information")
		ECharacterType CharacterType{ ECharacterType::CT_Humanoid };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character - General - Information")
		ECharacterFaction CharacterFaction{ ECharacterFaction::CF_Player };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character - General - States")
		ECharacterLifeState CharacterLifeState{ ECharacterLifeState::CLS_Alive };
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character - General - States")
		ECharacterActionState CharacterActionState{ ECharacterActionState::CAS_None };
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character - General - States")
		ECharacterAttackState CharacterAttackState{ ECharacterAttackState::CAT_None };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character - General - States")
		ECharacterSpeed CharacterSpeedState{ ECharacterSpeed::CS_Walking };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character - General - States")
		ECharacterMovementState CharacterMovementState{ ECharacterMovementState::CMS_Idle };

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character - General - States")
		ECharacterDirectionState CharacterDirectionState{ ECharacterDirectionState::CDS_None };

	//==============================
	// A T T A C K I N G
	void SetWeaponReady(bool ReadyWeapon);

	void UpdateAttackSpeed();
	void AttackStart();
	void AttackEnd();
	void AttackExecute();
	void AttackMovement();
	void AttackCancel();

	//void KickAttack(); <-- Add a kick attack if there is time.

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character - Combat - Attacking")
		bool FinishedChargingAttack{ false };

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character - Combat - Attacking")
		bool ActiveAttackChainWindow{ false };

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character - Combat - Attacking")
		int32 AttackChainCount{ 0 };

	float AttackChainSlowdownMult{ 1.f };

	float AttackChainDamageMult{ 2.5f };

	bool AttackChainFinale{ false };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character - Combat - Attacking")
		bool IsPowerAttack{ false };

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character - Combat - Attacking")
		float AttackPower{ 0.f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character - Combat - Attacking")
		float AttackMaximum{ 1.f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character - Combat - Attacking")
		float AttackMinimum{ 0.5f };

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character - Combat - Attacking")
		float CalculatedAttackSpeed{ 1.f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character - Weapon")
		UDataTable* WeaponDataTable{ nullptr };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character - Weapon")
		FName WeaponRowName{ "NONE" };

	//==============================
	// I N V E N T O R Y
	UPROPERTY(EditDefaultsOnly, Category = "Character - Inventory")
		TSubclassOf<class APickupItem> HealingItemClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character - Inventory")
		int32 HealingCarried{ 0 };

	UPROPERTY(EditDefaultsOnly, Category = "Character - Inventory")
		TSubclassOf<class APickupItem> GoldItemClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character - Inventory")
		int32 GoldCarried{ 0 };

	UPROPERTY(EditDefaultsOnly, Category = "Character - Inventory")
		TSubclassOf<class APickupItem> KeyItemClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character - Inventory")
		int32 KeysCarried{ 0 };

	//==============================
	// A C T I O N S
	void RestoreHealthEat();
	
	void AffectHealth(float magnitude, bool AttackDamage);

	void OnDeath(bool CausedByAttack);
	void DropItems();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character - Combat")
		bool EquipOrUnequipInProgress{ false };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character - Combat")
		bool ReceivingAttackInput{ false };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character - Combat")
		bool HasReceivedDamage{ false };

	bool IsDrinkingHealth{ false };

	//==============================
	// R E A C T I O N S
	void ReceiveAttackDamage(ABaseCharacter* damageDealer, float amount, FVector ImpactPoint);

	void Knockback(float DeltaTime);

	ABaseCharacter* DamageReceivedByCharacter{ nullptr };

	ABaseCharacter* CollidingCharacter{ nullptr };

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character - Combat - Defense")
		FVector DamageImpactDirection;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character - Combat - Defense")
		bool AttackDamageReceived{ false };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character - Combat - Defense")
		bool CharacterIsKnockedBack{ false };

	FVector KnockbackStart;

	// When this character last received attack damage.
	UPROPERTY()
		float LastReceivedDamage{ 0.f };

	// Cooldown before the character can receive another hit.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character - Combat - Defense")
		float DamageCooldown{ 0.25f };

	UFUNCTION()
		void OnOverlapCapsuleResponse(UPrimitiveComponent* OverlapComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

	void CharacterPushForce(float DeltaTime);

	//==============================
	// S U R F A C E   T R A C E
	FVector LastValidLocation{0.f, 0.f, 0.f};

	void SurfaceTypeCheck();

	ECharacterSurfaceType CharacterSurfaceType{ ECharacterSurfaceType::CST_Sand };

	//==============================
	// L O C K - O N
	bool CanLockOnto();
	void LockOntoTarget();
	void StopLockOntoTarget();

	void CheckIfLockStillValid();

	USphereComponent* LockOnRangeSphere{ nullptr };

	float LockOnSphereRadius{ 1024.f };

	TArray<AActor*> ValidTargets;

	ABaseCharacter* LockedTargetCharacter{ nullptr };
	
	ABaseCharacter* LockedOnBy{ nullptr };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character - Combat - Lock-On")
		bool IsHiddenFromLockOn{ false };

	// Only does something if this is the player character.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
		float LockOnCameraHeight{ 20.f };

	// If the character is locked onto a target.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character - Combat - Lock-On")
		bool IsLockedOn{ false };

	// If the character is being the target of someone elses lock on.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character - Combat - Lock-On")
		bool IsTargetOfOtherLockOn{ false };

	//==============================
	// M O V E M E N T
	void StartSprint();
	void StopSprint();

	void DodgeRoll();

	bool ReceivingSprintInput = false;

	void MovementStateHandler();
	void MovementVariableHandler();
	void MovementDirectionHandler(ECharacterDirectionState InputValue);

	// Adjusts the "SpeedBaseMultiplier" value dynamicly, based on the state the character is in.
	void UpdateBaseMovementSpeed(); 

	void UpdateSpeedVariables();

	void InertiaLeaning(float DeltaTime);

	void CalculateVelocity();

	// Bipeds will lean slightly when turning while moving, and quadrupeds needs to be aligned to the ground surface.
	void CharacterRotationHandler(float DeltaTime);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom Movement")
		float MovementSpeedCurrent{ 600.f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom Movement")
		float MovementSpeedWalking{ 350.f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom Movement")
		float MovementSpeedSprinting{ 1200.f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom Movement")
		float DefaultGroundFriction{ 8.f };

	bool SprintingShouldStop{ false };

	bool WasSprinting{ false };

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Custom Movement - Velocity")
		float ForwardVelocity{ 0.f };

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Custom Movement - Velocity")
		float RightVelocity{ 0.f };

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Custom Movement - Velocity")
		float RotationalVelocity{ 0.f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom Movement - Leaning")
		float LeaningInterpSpeed{ 1.f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom Movement - Leaning")
		float LeaningInterpSpeedStop{ 5.f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom Movement - Leaning")
		float LeaningMultiplier{ 1.25f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom Movement")
		FRotator DefaultRotationRate{ FRotator(0.f, 512.f, 0.f) };

	//==============================
	// E F F E C T S
	UFUNCTION()
		void PlayFootstepFX(bool Right);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character - Effects")
		bool IKDisabled = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character - Effects")
		UParticleSystem* DustFX{ nullptr };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character - Effects")
		UParticleSystem* WaterFX{ nullptr };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character - Effects")
		UParticleSystem* ImpactFX{ nullptr };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character - Effects")
		UMaterialInterface* FootprintDecalRight{ nullptr };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character - Effects")
		UMaterialInterface* FootprintDecalLeft{ nullptr };

	//==============================
	// C U R V E S
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom Movement - Curves")
		UCurveFloat* SprintAcceleration{ nullptr };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom Movement - Curves")
		UCurveFloat* DodgeMovement{ nullptr };

	//==============================
	// T I M E L I N E S
	void InitializeTimelines();

	FTimeline SprintTimeline;

	FTimeline DodgeTimeline;

	UFUNCTION()
		void HandleSprintProgress(float Value);

	UFUNCTION()
		void HandleSprintFinished();

	UFUNCTION()
		void HandleDodgeProgress(float Value);

	UFUNCTION()
		void HandleDodgeFinished();

	//==============================
	// A U D I O   -   V O I C E
	float voicePitch{1.f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio - Voice Acting")
		USoundCue* DeathCue{nullptr};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio - Voice Acting")
		USoundCue* HurtCue{nullptr};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio - Voice Acting")
		USoundCue* SpotPlayerCue{nullptr};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio - Voice Acting")
		USoundCue* IdleCue{nullptr};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio - Voice Acting")
		USoundCue* AttackCue{nullptr};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio - Voice Acting")
		USoundCue* ActionFailCue{ nullptr };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio - Voice Acting")
		USoundCue* EatVoiceCue{ nullptr };

	UFUNCTION()
		void PlaySound_Attack();
	UFUNCTION()
		void PlaySound_Idle();
	UFUNCTION()
		void PlaySound_SpotPlayer();

};
