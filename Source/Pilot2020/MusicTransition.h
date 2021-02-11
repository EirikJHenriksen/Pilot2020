// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

#include "Sound/SoundCue.h"

#include "MusicTransition.generated.h"

/**
 * 
 */
UCLASS()
class PILOT2020_API UMusicTransition : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music - Data")
		USoundCue* TransitionTrack{ nullptr };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music - Data")
		float TransitionHeadStart{ 0.f };
};
