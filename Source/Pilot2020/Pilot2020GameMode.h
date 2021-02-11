// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Pilot2020GameMode.generated.h"

class ALevelManager;

UCLASS(minimalapi)
class APilot2020GameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	APilot2020GameMode();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level")
	ALevelManager* myLevelManager{ nullptr };

};



