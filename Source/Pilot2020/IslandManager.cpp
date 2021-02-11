// Fill out your copyright notice in the Description page of Project Settings.


#include "IslandManager.h"
#include "Engine/Engine.h"
#include "IslandMesh.h"
#include "NonPlayerCharacter.h"
#include "Pilot2020GameMode.h"
#include "LevelManager.h"

// Sets default values
AIslandManager::AIslandManager()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootSceneComp = CreateDefaultSubobject<USceneComponent>(TEXT("root"));
	RootComponent = RootSceneComp;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("mesh"));
	MeshComp->SetupAttachment(RootSceneComp);
}

// Called when the game starts or when spawned
void AIslandManager::BeginPlay()
{
	Super::BeginPlay();

	// give a random mesh
	if (IslandMeshArray.Num() > 0)
	{
		int whichMesh = FMath::RandRange(int(0), IslandMeshArray.Num() - 1);
		MeshComp->SetStaticMesh(IslandMeshArray[whichMesh]);
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "Error: IslandMeshArray is empty (AIslandManager)", true);
	}


	if (GetWorld()->GetAuthGameMode<APilot2020GameMode>())
		{
			myLevelManager = GetWorld()->GetAuthGameMode<APilot2020GameMode>()->myLevelManager;

			if (myLevelManager->IslandMeshArrayAvailable.Num() < 1) // fill array if empty
		{
			myLevelManager->IslandMeshArrayAvailable = myLevelManager->IslandMeshArrayOwned;
		}
		if (myLevelManager->IslandMeshArrayAvailable.Num() > 0) // assuming array is not empty now, use one of the meshes
		{
			int whichMesh = FMath::RandRange(int(0), myLevelManager->IslandMeshArrayAvailable.Num() - 1);
			MeshComp->SetStaticMesh(myLevelManager->IslandMeshArrayAvailable[whichMesh]);
			myLevelManager->IslandMeshArrayAvailable.RemoveAt(whichMesh);
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "Error: IslandMeshArray is empty (AIslandManager)", true);
		}
		}
	else
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "Error: no GetWorld()->GetAuthGameMode<APilot2020GameMode>() (AIslandManager)", true);
		}

}

// Called every frame
void AIslandManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

