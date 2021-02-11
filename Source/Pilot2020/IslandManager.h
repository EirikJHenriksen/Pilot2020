// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "IslandManager.generated.h"

class ALevelManager;
class ANonPlayerCharacter;

UCLASS()
class PILOT2020_API AIslandManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AIslandManager();


	//==============================
	// COMPONENTS

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Mesh")
		USceneComponent* RootSceneComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Mesh")
		UStaticMeshComponent* MeshComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
		TArray<UStaticMesh*> IslandMeshArray;

	ALevelManager* myLevelManager{ nullptr };


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
