// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"

#include "Engine/DataTable.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundCue.h"

#include "WeaponSceneComponent.generated.h"

UENUM(BlueprintType)
enum class EWeaponAnimType : uint8
{
	WAT_Unarmed		UMETA(DisplayName = "Unarmed"),
	WAT_OneHanded 	UMETA(DisplayName = "One Handed"),
	WAT_TwoHanded	UMETA(DisplayName = "Two Handed"),
	WAT_Bow			UMETA(DisplayName = "Bow"),
	WAT_Thrown		UMETA(DisplayName = "Thrown"),
	WAT_Natural		UMETA(DisplayName = "Natural")
};

UENUM(BlueprintType)
enum class EWeaponDamageType : uint8
{
	WDT_Shovel	UMETA(DisplayName = "Shovel"),
	WDT_Blunt	UMETA(DisplayName = "Blunt"),
	WDT_Sharp	UMETA(DisplayName = "Sharp")
};

USTRUCT(BlueprintType)
struct FWeaponData : public FTableRowBase
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Data - Type")
		EWeaponAnimType WeaponAnimationType{ EWeaponAnimType::WAT_TwoHanded };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Data - Type")
		EWeaponDamageType WeaponDamageType{ EWeaponDamageType::WDT_Shovel };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Data - Values")
		FName WeaponName{ "UNNAMED" };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Data - Values")
		float WeaponDamage{ 1.f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Data - Values")
		float WeaponBaseSpeed{ 1.f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Data - Visual")
		UStaticMesh* WeaponMesh{ nullptr };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Data - Visual")
		UParticleSystem* WeaponSwingFX{ nullptr };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Data - Visual")
		UParticleSystem* WeaponImpactFX{ nullptr };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Data - Visual")
		float WeaponBaseHandOffsetZ{ 0.f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Data - Visual")
		float WeaponSecondHandOffsetZ{ 0.f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Data - Collision")
		float WeaponCollisionOffsetZ{ 40.f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Data - Collision")
		float WeaponCollisionLength{ 80.f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Data - Collision")
		float WeaponCollisionRadius{ 6.f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Data - Audio")
		USoundCue* WeaponSwingCue{ nullptr };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Data - Audio")
		USoundCue* WeaponImpactCue{ nullptr };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Data - Audio")
		USoundCue* WeaponDrawCue{ nullptr };

	FWeaponData() {}
};

class ABaseCharacter;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PILOT2020_API UWeaponSceneComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UWeaponSceneComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	ABaseCharacter* WeaponOwner{ nullptr };

	UCapsuleComponent* WeaponCollision{ nullptr };

	UStaticMeshComponent* WeaponMesh{ nullptr };

	//=========================
	// W E A P O N   S T A T S
	UPROPERTY()
		FWeaponData WeaponData;

	void LoadData();

	//=====================================
	// S O C K E T   A T T A C H E M E N T
	void UpdateSocketAttachement(bool weaponDrawn);

	//=========================
	// W E A P O N   F U N C T I O N S
	UFUNCTION()
		void OnOverlapBegin(class UPrimitiveComponent* OverlapComponent, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);
};
