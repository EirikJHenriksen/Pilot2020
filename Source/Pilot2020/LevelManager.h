// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LevelManager.generated.h"

UCLASS()
class PILOT2020_API ALevelManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ALevelManager();

	// for which meshes trees should have in a level
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
		int32 EnvironmentType{0};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
		TArray<UStaticMesh*> IslandMeshArrayAvailable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
		TArray<UStaticMesh*> IslandMeshArrayOwned;

	UFUNCTION(BlueprintCallable)
	void SetInGameMode();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
