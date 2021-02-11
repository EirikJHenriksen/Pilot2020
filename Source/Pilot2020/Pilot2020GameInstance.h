// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"

#include "PickupItem.h"
#include "MusicPlayer.h"

#include "Pilot2020GameInstance.generated.h"

UENUM(BlueprintType)
enum class EGameState : uint8
{
	GS_NewStart		UMETA(DisplayName = "New Game"),
	GS_Continue		UMETA(DisplayName = "Continue Game"),
	GS_Respawned	UMETA(DisplayName = "Respawned Game")
};

USTRUCT(BlueprintType)
struct FDialogData : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialog Data - System")
		FString SpeakerName{ "EMPTY" };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialog Data - System")
		FString DialogText{ "EMPTY" };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialog Data - System")
		int32 NextEntry{ -1 };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialog Data - Audio")
		USoundCue* VoiceTrack{ nullptr };

	FDialogData() {}
};

class AMusicPlayer;
class APickupItem;
/**
 * 
 */
UCLASS()
class PILOT2020_API UPilot2020GameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:

	// Number of rescued characters
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game System - State")
		int32 NumberOfRescues{ 0 };

	// If the player is playing post-story-mode
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game System - State")
		bool NamisQuestActive{ false };
	
	// State of the game
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game System - State")
		EGameState CurrentGameState{ EGameState::GS_NewStart };

	//===========================
	// D I A L O G   S Y S T E M
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game System - Dialog")
		UDataTable* DialogDataTable{ nullptr };

	//===========================
	// E V E N T S
	UFUNCTION(BlueprintImplementableEvent, Category = "Game System - Events")
		void OpenDialog(int32 EntryID);

	UFUNCTION(BlueprintImplementableEvent, Category = "Game System - Events")
		void OpenGameOverScreen();


	//==================================
	// P L A Y E R   I N V E N T O R Y
	UFUNCTION(BlueprintCallable, Category = "Game System - Functions")
		void StorePlayerInventory(int32 Gold, int32 FoodItems);

	int32 PlayerGold{ 0 };
	int32 PlayerHealingItems{ 0 };

	//=========================
	// M U S I C   S Y S T E M
	AMusicPlayer* MusicSystem{ nullptr };
};
