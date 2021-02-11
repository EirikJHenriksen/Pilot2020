// Fill out your copyright notice in the Description page of Project Settings.


#include "MusicPlayer.h"

//For debug messages
#include "Engine/Engine.h"

// Sets default values
AMusicPlayer::AMusicPlayer()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Should still tick when paused
	SetTickableWhenPaused(true);

	// Configuring the audio components
	MusicComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("Music Player"));
	MusicComponent->bAutoActivate = false;
	MusicComponent->bAutoDestroy = false;
	MusicComponent->bIsUISound = true;
	MusicComponent->SetTickableWhenPaused(true);
	MusicComponent->SetupAttachment(GetRootComponent());

	TransitionComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("Music Player - Transitions"));
	TransitionComponent->bAutoActivate = false;
	TransitionComponent->bAutoDestroy = false;
	TransitionComponent->bIsUISound = true;
	TransitionComponent->SetTickableWhenPaused(true);
	TransitionComponent->SetupAttachment(GetRootComponent());
}

// Called after components have been initialized
void AMusicPlayer::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// Linking cues to the audio components
	if (MusicComponent->IsValidLowLevelFast())
		MusicComponent->SetSound(ExplorationTrackCue);

	if (TransitionComponent->IsValidLowLevelFast())
		TransitionComponent->SetSound(ExplorationTrackCue);
}

// Called when the game starts or when spawned
void AMusicPlayer::BeginPlay()
{
	Super::BeginPlay();

	Cast<UPilot2020GameInstance>(GetGameInstance())->MusicSystem = this;
}

// Called every frame
void AMusicPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//How long has the current track been playing
	CurrentTrackTimer += DeltaTime;

	// This should be done through delegates for maximum performance...
	if (TargetTrackToPlay != CurrentTrack && !FadeOutInProgress && !MusicShouldStop)
	{
		MusicComponent->FadeOut(FadeDuration, 0.f);
		CurrentTrackTimer = 0.f;
		FadeOutInProgress = true;
	}

	if (FadeOutInProgress && !MusicComponent->IsPlaying())
	{
		switch (TargetTrackToPlay)
		{
		case EMusicPlayerState::MPS_Menu:
			DelayDuration = 0.f;
			MusicComponent->SetSound(MenuTrackCue);
			break;
		case EMusicPlayerState::MPS_Exploration:
			DelayDuration = MusicTypeDelay;
			MusicComponent->SetSound(ExplorationTrackCue);
			break;
		case EMusicPlayerState::MPS_Combat:
			DelayDuration = MusicTypeDelay;
			MusicComponent->SetSound(CombatTrackCue);
			break;
		case EMusicPlayerState::MPS_Defeat:
			DelayDuration = 0.f;
			MusicComponent->SetSound(DefeatTrackCue);
			MusicComponent->FadeIn(FadeDuration, 1.f);
			MusicShouldStop = true;
			break;
		case EMusicPlayerState::MPS_Victory:
			DelayDuration = 0.f;
			MusicComponent->SetSound(VictoryTrackCue);
			MusicComponent->FadeIn(FadeDuration, 1.f);
			MusicShouldStop = true;
			break;
		default:
			break;
		}

		CurrentTrack = TargetTrackToPlay;
		CurrentTrackTimer = 0.f;
		FadeOutInProgress = false;
	}

	if (!MusicComponent->IsPlaying() && (CurrentTrackTimer > DelayDuration) && !FadeOutInProgress && !MusicShouldStop)
	{
		MusicComponent->FadeIn(FadeDuration, 1.f);
		CurrentTrackTimer = 0.f;
	}

	// Get random transition track from array!
	if ((Transitions.Num() > 0) && (Crashes.Num() > 0) && !CurrentTransition)
	{
		if (CurrentTrack == EMusicPlayerState::MPS_Combat)
		{
			CurrentTransition = Crashes[FMath::RandRange(0, Crashes.Num() - 1)];
			TransitionPlayTimer = CombatTrackLength - CurrentTransition->TransitionHeadStart;
		}
		else
		{
			CurrentTransition = Transitions[FMath::RandRange(0, Transitions.Num() - 1)];
			TransitionPlayTimer = ExplorationTrackLength - CurrentTransition->TransitionHeadStart;
		}

		if (TransitionComponent->IsValidLowLevelFast() && CurrentTransition)
			TransitionComponent->SetSound(CurrentTransition->TransitionTrack);
	}

	// Plays the transition track & allows a new track to be selected
	if (MusicComponent->IsPlaying() && (CurrentTrackTimer > TransitionPlayTimer) && CurrentTransition && !TransitionComponent->IsPlaying() && !MusicShouldStop && !FadeOutInProgress)
	{
		TransitionComponent->VolumeMultiplier = 1.3f;
		TransitionComponent->Play();
		CurrentTransition = nullptr;
	}
}

