// Fill out your copyright notice in the Description page of Project Settings.


#include "PickupItem.h"
#include "Components/SphereComponent.h"
#include "Pilot2020Character.h"
#include "Pilot2020GameInstance.h"
#include "Math/UnrealMathUtility.h"
#include "Kismet/GameplayStatics.h"

//For debug messages
#include "Engine/Engine.h"

// Sets default values
APickupItem::APickupItem()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Setting up item mesh
	ItemMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Item Mesh"));
	SetRootComponent(ItemMeshComp);

	ItemMeshComp->bOwnerNoSee = false;
	ItemMeshComp->bCastDynamicShadow = true;
	ItemMeshComp->CastShadow = true;
	ItemMeshComp->BodyInstance.SetObjectType(ECC_WorldDynamic);
	ItemMeshComp->BodyInstance.SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	ItemMeshComp->BodyInstance.SetResponseToAllChannels(ECR_Ignore);
	ItemMeshComp->BodyInstance.SetResponseToChannel(ECC_WorldStatic, ECR_Block);
	ItemMeshComp->BodyInstance.SetResponseToChannel(ECC_WorldDynamic, ECR_Ignore);
	ItemMeshComp->BodyInstance.SetResponseToChannel(ECC_Pawn, ECR_Overlap);
	ItemMeshComp->SetHiddenInGame(false);

	// Setting up collision spheres (outer)
	MagneticRange = CreateDefaultSubobject<USphereComponent>(TEXT("Magnetic Range"));
	MagneticRange->InitSphereRadius(250);
	MagneticRange->SetupAttachment(RootComponent);

	// Setting up collision spheres (inner)
	PickupRange = CreateDefaultSubobject<USphereComponent>(TEXT("Pickup Range"));
	PickupRange->InitSphereRadius(25);
	PickupRange->SetupAttachment(RootComponent);
	PickupRange->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
}

// Called when the game starts or when spawned
void APickupItem::BeginPlay()
{
	Super::BeginPlay();

	MagneticRange->OnComponentBeginOverlap.AddDynamic(this, &APickupItem::OnOverlapBeginOuter);
	MagneticRange->OnComponentEndOverlap.AddDynamic(this, &APickupItem::OnOverlapEndOuter);
	
	PickupRange->OnComponentBeginOverlap.AddDynamic(this, &APickupItem::OnOverlapBeginInner); // to set isWithinPickupRange
	PickupRange->OnComponentEndOverlap.AddDynamic(this, &APickupItem::OnOverlapEndInner); // to set isWithinPickupRange

	Player = Cast<APilot2020Character>(GetWorld()->GetFirstPlayerController()->GetPawn());

	if (IsMagnetic)
	{
		HasPhysics = true;

		IsMagnetic = false;
		GetWorld()->GetTimerManager().SetTimer(MagneticTimer, this, &APickupItem::ActivateMagnetism, MagneticDelay, false);
	}

	if (HasPhysics)
	{
		ItemMeshComp->SetCanEverAffectNavigation(false);

		ItemMeshComp->bAlwaysCreatePhysicsState = true;
		ItemMeshComp->SetSimulatePhysics(true);

		ItemMeshComp->BodyInstance.SetResponseToChannel(ECC_WorldStatic, ECR_Block);
		ItemMeshComp->BodyInstance.SetResponseToChannel(ECC_WorldDynamic, ECR_Ignore);
		ItemMeshComp->BodyInstance.SetResponseToChannel(ECC_Pawn, ECR_Overlap);
	}
	else
	{
		ItemMeshComp->SetCanEverAffectNavigation(true);

		ItemMeshComp->bAlwaysCreatePhysicsState = false;
		ItemMeshComp->SetSimulatePhysics(false);

		ItemMeshComp->BodyInstance.SetResponseToChannel(ECC_WorldStatic, ECR_Block);
		ItemMeshComp->BodyInstance.SetResponseToChannel(ECC_WorldDynamic, ECR_Block);
		ItemMeshComp->BodyInstance.SetResponseToChannel(ECC_Pawn, ECR_Block);
	}
}

// Called every frame
void APickupItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsMagnetic)
	{
		MagneticMove(DeltaTime);
	}

	if (!MustBeInteractedWith)
	{
		CharacterPickupCheck(DeltaTime);
	}

	if (isOverlappingPlayer)
	{
		whileOverlappingPlayer();
	}
}

void APickupItem::MagneticMove(float DeltaTime)
{
	if (!TargetFound)
	{
		ValidMagnetTargets.Empty();

		UClass* ActorFilter = ABaseCharacter::StaticClass();
		MagneticRange->GetOverlappingActors(ValidMagnetTargets, ActorFilter);
	}
	
	for (int32 i = ValidMagnetTargets.Num() - 1; i >= 0; i--)
	{
		if ((ValidMagnetTargets.Num() > 0) && (MagneticRange->IsOverlappingComponent(Cast<ABaseCharacter>(ValidMagnetTargets[i])->GetCapsuleComponent())) && (Cast<ABaseCharacter>(ValidMagnetTargets[i])->CharacterLifeState != ECharacterLifeState::CLS_Dead))
		{
			TargetFound = true;

			if (BaseMagneticForce > 250000.f)
			{
				SetActorLocation(FMath::Lerp(ValidMagnetTargets[i]->GetActorLocation(), GetActorLocation(), 0.1f));
			}
			else
			{
				ItemMeshComp->AddForce((ValidMagnetTargets[i]->GetActorLocation() - GetActorLocation()).GetSafeNormal() * BaseMagneticForce, FName("NAME_None"), false);
				BaseMagneticForce += (3.f * BaseMagneticForce * DeltaTime);
			}

		}
		else if (TargetFound)
		{
			if (BaseMagneticForce > 250000.f)
			{
				SetActorLocation(FMath::Lerp(ValidMagnetTargets[i]->GetActorLocation(), GetActorLocation(), 0.1f));
			}
			else
			{
				ItemMeshComp->AddForce((ValidMagnetTargets[i]->GetActorLocation() - GetActorLocation()).GetSafeNormal() * BaseMagneticForce, FName("NAME_None"), false);
				BaseMagneticForce += (3.f * BaseMagneticForce * DeltaTime);
			}
		}

		if ((Cast<ABaseCharacter>(ValidMagnetTargets[i])->CharacterLifeState == ECharacterLifeState::CLS_Dead))
		{
			TargetFound = false;
		}
	}
}

void APickupItem::CharacterPickupCheck(float DeltaTime)
{
	if (PickupDelayPassed)
	{
		ValidPickupTargets.Empty();

		UClass* ActorFilter = ABaseCharacter::StaticClass();
		PickupRange->GetOverlappingActors(ValidPickupTargets, ActorFilter);

		for (int32 i = ValidPickupTargets.Num() - 1; i >= 0; i--)
		{
			if ((ValidPickupTargets.Num() > 0) && (PickupRange->IsOverlappingComponent(Cast<ABaseCharacter>(ValidPickupTargets[i])->GetCapsuleComponent())) && (Cast<ABaseCharacter>(ValidPickupTargets[i])->CharacterLifeState != ECharacterLifeState::CLS_Dead))
			{
				PickUp(Cast<ABaseCharacter>(ValidPickupTargets[i]));
			}
		}
	}
	else
	{
		if (CurrentPickupTime >= MagneticDelay)
		{
			PickupDelayPassed = true;
		}
		else
		{
			CurrentPickupTime += DeltaTime;
		}
	}
}

void APickupItem::PickUp(ABaseCharacter* InputCharacter)
{
	// Sets the correct sound, and plays it if it exists
	if (UsageSound)
	{
		InputCharacter->ActionSFX->SetSound(UsageSound);
		InputCharacter->ActionSFX->Play();
	}

	switch (ItemType)
	{
	case EItemType::IT_Health:
		InputCharacter->HealingCarried++;
		break;
	case EItemType::IT_Coin:
		InputCharacter->GoldCarried++;
		break;
	case EItemType::IT_Key:
		InputCharacter->KeysCarried++;
		break;
	case EItemType::IT_Captive:
		Cast<UPilot2020GameInstance>(GetGameInstance())->OpenDialog(FMath::RandRange(TableEntryIDMin, TableEntryIDMax));
		
		// Play victory music
		if (UGameplayStatics::GetCurrentLevelName(GetWorld(), true) != "Ship_Level")
		{
			Cast<APilot2020Character>(InputCharacter)->PlaySpecialEventMusic = true;
			Cast<UPilot2020GameInstance>(GetGameInstance())->MusicSystem->TargetTrackToPlay = EMusicPlayerState::MPS_Victory;
		}
		break;
	default:
		InputCharacter->GoldCarried++;
		break;
	}

	if (ItemType != EItemType::IT_Captive)
	{
		Destroy();
	}
}

void APickupItem::OnOverlapBeginInner(UPrimitiveComponent* OverlapComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	if (Cast<APilot2020Character>(OtherActor))
	{
		// Ensures the weapon hits the characters capsule and nothing else. Might be changed into the character mesh itself.
		if ((Cast<ABaseCharacter>(OtherActor)->GetCapsuleComponent() == Cast<UCapsuleComponent>(OtherComp)))
		{
			//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "OnOverlapBeginInner", true);
			isWithinPickupRange = true;

			if (MustBeInteractedWith)
			{
				Player->InteractionIsPossible = true;
			}
		}
	}
}

void APickupItem::OnOverlapEndInner(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (Cast<APilot2020Character>(OtherActor))
	{
		// Ensures the weapon hits the characters capsule and nothing else. Might be changed into the character mesh itself.
		if ((Cast<ABaseCharacter>(OtherActor)->GetCapsuleComponent() == Cast<UCapsuleComponent>(OtherComp)))
		{
			//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "OnOverlapEndInner", true);
			isWithinPickupRange = false;
			Player->InteractionIsPossible = false;
		}
	}
}

void APickupItem::OnOverlapBeginOuter(UPrimitiveComponent* OverlapComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	if (Cast<APilot2020Character>(OtherActor))
	{
		// Ensures the weapon hits the characters capsule and nothing else. Might be changed into the character mesh itself.
		if ((Cast<ABaseCharacter>(OtherActor)->GetCapsuleComponent() == Cast<UCapsuleComponent>(OtherComp)))
		{
			//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Emerald, "OnOverlapBeginOuter", true);
			isOverlappingPlayer = true;
		}
	}
}

void APickupItem::OnOverlapEndOuter(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (Cast<APilot2020Character>(OtherActor))
	{
		// Ensures the weapon hits the characters capsule and nothing else. Might be changed into the character mesh itself.
		if ((Cast<ABaseCharacter>(OtherActor)->GetCapsuleComponent() == Cast<UCapsuleComponent>(OtherComp)))
		{
			//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Emerald, "OnOverlapEndOuter", true);
			isOverlappingPlayer = false;
			Player->InteractionIsPossible = false;
		}
	}
}

void APickupItem::whileOverlappingPlayer()
{
	switch (ItemType)
	{
	case EItemType::IT_Health:
		Player->InteractionType = " pick up ";
		Player->InteractionObjectName = ItemName;
		break;
	case EItemType::IT_Coin:
		Player->InteractionType = " pick up ";
		Player->InteractionObjectName = ItemName;
		break;
	case EItemType::IT_Key:
		Player->InteractionType = " collect ";
		Player->InteractionObjectName = ItemName;
		break;
	case EItemType::IT_Captive:
		Player->InteractionType = " talk to ";
		Player->InteractionObjectName = ItemName;
		break;
	}
}