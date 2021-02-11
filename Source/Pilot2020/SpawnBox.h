// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "PickupItem.h"
#include "NonPlayerCharacter.h"
#include "DestructableObject.h"
#include "SpawnBox.generated.h"

UCLASS(meta = (BlueprintSpawnableComponent))
class PILOT2020_API USpawnBox : public UBoxComponent
{
	GENERATED_BODY()

public: 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sphere trace")
	float traceRadius = 200;

	UFUNCTION(BlueprintCallable)
	bool SpawnActor(TSubclassOf<class AActor> AnotherClass, float objectRadius);
	
	//Your class reference
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	TSubclassOf<class AActor> YourClass;

};
