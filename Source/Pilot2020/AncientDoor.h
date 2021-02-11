// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"

#include "AncientDoor.generated.h"

class APilot2020Character;

UCLASS()
class PILOT2020_API AAncientDoor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAncientDoor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	APilot2020Character* Player{ nullptr };

	UFUNCTION()
		void OnOverlapBegin(UPrimitiveComponent * OverlapComponent, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);
	
	UFUNCTION()
		void OnOverlapEnd(UPrimitiveComponent * OverlappedComp, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ancient Door Mesh")
		USkeletalMeshComponent* DoorMesh{ nullptr };

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Component")
		USphereComponent* InteractRange { nullptr };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Component")
		int32 RequiredAmountOfKeys { 3 };

	bool DoorIsOpen{ false };

	void OpenDoor();

	//====================================
	// C O M P O N E N T S  -  A U D I O
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		UAudioComponent* DoorSFX{ nullptr };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door Audio")
		USoundCue* UseLockedSound{ nullptr };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door Audio")
		USoundCue* UseOpenSound{ nullptr };
};
