// Fill out your copyright notice in the Description page of Project Settings.


#include "AncientDoor.h"
//#include "PickupItem.h"

#include "Pilot2020Character.h"

// Sets default values
AAncientDoor::AAncientDoor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Setting up door mesh
	DoorMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Door Mesh"));
	SetRootComponent(DoorMesh);

	InteractRange = CreateDefaultSubobject<USphereComponent>(TEXT("Activation Range"));
	InteractRange->InitSphereRadius(150.f);
	InteractRange->SetupAttachment(RootComponent);

	// Configuring the audio components
	DoorSFX = CreateDefaultSubobject<UAudioComponent>(TEXT("Door - SFX"));
	DoorSFX->bAutoActivate = false;
	DoorSFX->bAutoDestroy = false;
	DoorSFX->SetupAttachment(GetRootComponent());
}

// Called when the game starts or when spawned
void AAncientDoor::BeginPlay()
{
	Super::BeginPlay();
	
	InteractRange->OnComponentBeginOverlap.AddDynamic(this, &AAncientDoor::OnOverlapBegin);
	InteractRange->OnComponentEndOverlap.AddDynamic(this, &AAncientDoor::OnOverlapEnd);

	Player = Cast<APilot2020Character>(GetWorld()->GetFirstPlayerController()->GetPawn());
}

// Called every frame
void AAncientDoor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AAncientDoor::OnOverlapBegin(UPrimitiveComponent* OverlapComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	if (Cast<APilot2020Character>(OtherActor))
	{
		// Ensures the weapon hits the characters capsule and nothing else. Might be changed into the character mesh itself.
		if ((Cast<ABaseCharacter>(OtherActor)->GetCapsuleComponent() == Cast<UCapsuleComponent>(OtherComp)))
		{
			if (Player->KeysCarried >= RequiredAmountOfKeys)
			{
				Player->InteractionType = " open ";
				Player->InteractionObjectName = "Chained Door";

				Player->InteractionIsPossible = true;
			}
			else
			{

				Player->InteractionType = " open ";
				Player->InteractionObjectName = "Chained Door [Locked]";

				Player->InteractionIsPossible = true;
			}
		}
	}
}

void AAncientDoor::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (Cast<APilot2020Character>(OtherActor))
	{
		// Ensures the weapon hits the characters capsule and nothing else. Might be changed into the character mesh itself.
		if ((Cast<ABaseCharacter>(OtherActor)->GetCapsuleComponent() == Cast<UCapsuleComponent>(OtherComp)))
		{
			Player->InteractionIsPossible = false;
		}
	}
}

void AAncientDoor::OpenDoor()
{
	if (Player->KeysCarried >= RequiredAmountOfKeys)
	{
		Player->KeysCarried = 0;

		Player->InteractionIsPossible = false;
		DoorIsOpen = true;
		Destroy();

		DoorSFX->SetSound(UseOpenSound);
		DoorSFX->Play();
	}
	else
	{
		if (!Player->VoiceSFX->IsPlaying())
		{
			Player->VoiceSFX->SetSound(Player->ActionFailCue);
			Player->VoiceSFX->Play();
		}

		DoorSFX->SetSound(UseLockedSound);
		DoorSFX->Play();
	}
}

