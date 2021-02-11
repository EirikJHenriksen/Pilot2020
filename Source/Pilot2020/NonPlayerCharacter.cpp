// Fill out your copyright notice in the Description page of Project Settings.


#include "NonPlayerCharacter.h"

ANonPlayerCharacter::ANonPlayerCharacter()
{
	IsCharacterPlayer = false;
	AIControllerClass = AUtilityAIController::StaticClass();

	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void ANonPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void ANonPlayerCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (GetController() && !AttachedToAIController)
	{
		// Links this NPC to the AI controller, and lauches the tick functionality of the controller
		Cast<AUtilityAIController>(GetController())->ControlledCharacter = Cast<ABaseCharacter>(this);
		Cast<AUtilityAIController>(GetController())->SetActorTickEnabled(true);
		AttachedToAIController = true;
	}
}