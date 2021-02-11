// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "BaseCharacter.h"
#include "Engine/StaticMesh.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Sound/SoundCue.h"

#include "PickupItem.generated.h"

UENUM(BlueprintType)
enum class EItemType : uint8
{
	IT_Health	UMETA(DisplayName = "Consumable - Health"),
	IT_Coin		UMETA(DisplayName = "Coin"),
	IT_Key		UMETA(DisplayName = "Key"),
	IT_Captive	UMETA(DisplayName = "Captive")
};

class APilot2020Character;

UCLASS()
class PILOT2020_API APickupItem : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APickupItem();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	FTimerHandle MagneticTimer;

	APilot2020Character* Player{ nullptr };

	TArray<AActor*> ValidMagnetTargets;
	TArray<AActor*> ValidPickupTargets;
	
	float BaseMagneticForce{ 2000.f };

	bool TargetFound{ false };

	bool PickupDelayPassed{ false };

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Item Mesh")
		UStaticMeshComponent* ItemMeshComp{ nullptr };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Sound")
		USoundCue* UsageSound{ nullptr };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Sound")
		USoundCue* IdleSound{ nullptr };

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Item Collision Components")
		USphereComponent* MagneticRange{ nullptr };

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Item Collision Components")
		USphereComponent* PickupRange{ nullptr };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item configuration")
		EItemType ItemType{ EItemType::IT_Health };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item configuration")
		FName ItemName { "Unnamed" };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item configuration")
		float HealingStrength{ 25.f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item configuration")
		bool HasPhysics{ false };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item configuration - Magnetic")
		bool IsMagnetic{ false };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item configuration - Magnetic")
		float MagneticDelay{ 1.2f };

	float CurrentPickupTime{ 0.f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item configuration - Pickup")
		bool MustBeInteractedWith{ false };

	// Character related
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item configuration - Captive Character")
		int32 TableEntryIDMin{ -1 };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item configuration - Captive Character")
		int32 TableEntryIDMax{ -1 };

	/* overlapping */

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item stats")
		bool isWithinPickupRange{ false };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item stats")
		bool isOverlappingPlayer{ false };

	UFUNCTION()
		void OnOverlapBeginInner(class UPrimitiveComponent* OverlapComponent, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);
	UFUNCTION()
		void OnOverlapEndInner(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	UFUNCTION()
		void OnOverlapBeginOuter(class UPrimitiveComponent* OverlapComponent, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);
	UFUNCTION()
		void OnOverlapEndOuter(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
		void whileOverlappingPlayer();

	void ActivateMagnetism() { IsMagnetic = true; };

	void MagneticMove(float DeltaTime);

	void CharacterPickupCheck(float DeltaTime);

	void PickUp(ABaseCharacter* InputCharacter);
};
