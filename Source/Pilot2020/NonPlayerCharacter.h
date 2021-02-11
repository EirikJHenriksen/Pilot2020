// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseCharacter.h"

#include "UObject/Class.h"
#include "UtilityAIController.h"

#include "NonPlayerCharacter.generated.h"


UENUM(BlueprintType)
enum class ECharacterDisposition : uint8
{
	CD_Friendly		UMETA(DisplayName = "Friendly"),
	CD_Hostile		UMETA(DisplayName = "Hostile"),
	CD_Scared		UMETA(DisplayName = "Scared")
};

typedef ECharacterDisposition ECD;

UCLASS()
class PILOT2020_API ANonPlayerCharacter : public ABaseCharacter
{
	GENERATED_BODY()

public:
	ANonPlayerCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character - NPC - Values")
		bool CanInteractWith{ false };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character - NPC - Values")
		FName CharacterName{ "Stranger" };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character - NPC - States")
		ECharacterDisposition CharacterDisposition{ ECD::CD_Friendly };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character - NPC - User Interface")
		float HealthBarHeight{ 100.f };

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		bool ShowHealthBar{ false };

	bool AttachedToAIController{ false };
};
