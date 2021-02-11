// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "DestructibleComponent.h"
#include "Sound/SoundCue.h"

#include "DestructableObject.generated.h"



class PickupItem;

UCLASS()
class PILOT2020_API ADestructableObject : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADestructableObject();

	//==============================
// COMPONENTS
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Mesh")
		USceneComponent* RootSceneComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Mesh")
		UDestructibleComponent* DestructableComp;

	bool sleeping{true}; // for bug testing
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
		TArray<UDestructibleMesh*> DestructibleMeshArray;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destructable")
		int HealthPoints = 100;

	UFUNCTION()
		void myOnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	UFUNCTION()
		void myOnComponentHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit);

	// "virtual" functions, you'll find them in the blueprint
	UFUNCTION(BlueprintImplementableEvent)
		void playSoundDamage();
	UFUNCTION(BlueprintImplementableEvent)
		void playSoundDestroy();

	UFUNCTION()
		void BecomesBroken();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
		float weightKG{ 100.f };

	//==============================
	// I N V E N T O R Y
	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
		TSubclassOf<class APickupItem> FoodItemClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
		int32 FoodCarried{ 0 };

	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
		TSubclassOf<class APickupItem> DrinkItemClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
		int32 DrinkCarried{ 0 };

	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
		TSubclassOf<class APickupItem> GoldItemClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
		int32 GoldCarried{ 0 };

	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
		TSubclassOf<class APickupItem> CrystalItemClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
		int32 CrystalsCarried{ 0 };

	UFUNCTION(BlueprintCallable)
	void DropItems();


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
