// Fill out your copyright notice in the Description page of Project Settings.

#include "LevelManager.h"

#include "Kismet/GameplayStatics.h"
#include "Pilot2020GameMode.h"
#include "Pilot2020GameInstance.h"
#include "Engine/Engine.h"

// Sets default values
ALevelManager::ALevelManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ALevelManager::BeginPlay()
{
	Super::BeginPlay();

	//UPilot2020GameInstance* GameInstance = GetWorld()->GetGameInstance<UPilot2020GameInstance>();
	//GameInstance->EnvironmentType = 2; 
}

// Called every frame
void ALevelManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ALevelManager::SetInGameMode()
{
	// set game mode to point at this
	APilot2020GameMode* GameMode = GetWorld()->GetAuthGameMode<APilot2020GameMode>();
	GameMode->myLevelManager = this;
}