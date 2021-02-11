// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "Pilot2020GameMode.h"
#include "Pilot2020Character.h"
#include "UObject/ConstructorHelpers.h"

APilot2020GameMode::APilot2020GameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Blueprints/Characters/BP_PlayerCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
