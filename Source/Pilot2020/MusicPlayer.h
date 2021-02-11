// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Pilot2020GameInstance.h"
#include "MusicTransition.h"

#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"

#include "MusicPlayer.generated.h"

UENUM(BlueprintType)
enum class EMusicPlayerState : uint8
{
	MPS_Menu		UMETA(DisplayName = "Menu Music"),
	MPS_Exploration	UMETA(DisplayName = "Exploration Music"),
	MPS_Combat		UMETA(DisplayName = "Combat Music"),
	MPS_Defeat		UMETA(DisplayName = "Defeat Music"),
	MPS_Victory		UMETA(DisplayName = "Victory Music")
};

UCLASS()
class PILOT2020_API AMusicPlayer : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMusicPlayer();

protected:
	// Called after components have been initialized
	virtual void PostInitializeComponents() override;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//====================================
	// C O M P O N E N T S  -  A U D I O
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		UAudioComponent* MusicComponent{ nullptr };

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		UAudioComponent* TransitionComponent{ nullptr };

	//====================================
	// M U S I C  -  V A L U E S
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music System - Values")
		float ExplorationTrackLength{ 20.f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music System - Values")
		float CombatTrackLength{ 16.695f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music System - Values")
		float FadeDuration{ 1.f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music System - Values")
		float MusicTypeDelay{ 1.f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music System - Values")
		float ExplorationTrackDelay{ 2.f };

	float DelayDuration{ 0.f };
	float CurrentTrackTimer{ 0.f };
	float TransitionPlayTimer{ 999.f };

	bool FadeOutInProgress{ false };
	bool MusicShouldStop{ false };

	//====================================
	// M U S I C  -  C U E S
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music System - Cues")
		USoundCue* MenuTrackCue{ nullptr };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music System - Cues")
		USoundCue* ExplorationTrackCue{ nullptr };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music System - Cues")
		USoundCue* CombatTrackCue{ nullptr };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music System - Cues")
		USoundCue* DefeatTrackCue{ nullptr };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music System - Cues")
		USoundCue* VictoryTrackCue{ nullptr };

	// Change the name of the two arrays, first is for exploration, other is for combat
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music System - Transitions")
		TArray<UMusicTransition*> Transitions;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music System - Transitions")
		TArray<UMusicTransition*> Crashes;

	UMusicTransition* CurrentTransition{ nullptr };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music System - States")
		EMusicPlayerState CurrentTrack{ EMusicPlayerState::MPS_Menu };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music System - States")
		EMusicPlayerState TargetTrackToPlay{ EMusicPlayerState::MPS_Menu };
};
