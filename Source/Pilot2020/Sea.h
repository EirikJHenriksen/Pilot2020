// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "UObject/UObjectGlobals.h"
#include "Components/StaticMeshComponent.h"
#include "Components/DecalComponent.h"
#include "Components/BoxComponent.h"
#include "Pilot2020Character.h"

#include "Sea.generated.h"

UCLASS()
class PILOT2020_API ASea : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASea();

protected:
	// Called after components have been initialized
	void PostInitializeComponents();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//====================================
	// C O M P O N E N T S  -  A U D I O
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		UAudioComponent* WaterSplashSFX { nullptr };

	//==============================
	// A U D I O   C U E S
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sea - Audio - Cues")
		USoundCue* WaterSplashCue{ nullptr };

	UFUNCTION()
		void OnOverlapBegin(UPrimitiveComponent * OverlapComponent, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

	UFUNCTION()
		void OnOverlapEnd(UPrimitiveComponent * OverlappedComp, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex);

	void WhileOverlapping();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sea - Settings")
		bool SnapToCenterOfLevel{ false };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sea - Settings")
		float MaximumDepthBeforeLaunch{ 20.f };

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Sea - Components")
		UBoxComponent* SeaCollision{ nullptr };

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Sea - Components")
		UStaticMeshComponent* SeaMesh{ nullptr };

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Sea - Components")
		UDecalComponent* SeaCaustics{ nullptr };
};
