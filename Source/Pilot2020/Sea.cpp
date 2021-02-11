// Fill out your copyright notice in the Description page of Project Settings.


#include "Sea.h"
#include "BaseCharacter.h"

//For debug messages
#include "Engine/Engine.h"

// Sets default values
ASea::ASea()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Initialize mesh
	SeaMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Static Mesh"));
	SeaMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	SeaMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);

	// Initialize collision
	SeaCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("Sea Collision"));
	SeaCollision->SetupAttachment(RootComponent);
	SeaCollision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	SeaCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	SeaCollision->InitBoxExtent(FVector(50000.f, 50000.f, 5000.f));
	SeaCollision->SetRelativeLocation(FVector(0.f, 0.f, -5000.f));

	// Initialize water caustics decal
	SeaCaustics = CreateDefaultSubobject<UDecalComponent>(TEXT("Caustic Decal"));
	SeaCaustics->SetupAttachment(RootComponent);
	SeaCaustics->SetWorldScale3D(FVector(200.f, 200.f, 5.f));
	SeaCaustics->DecalSize = FVector(256.f, 256.f, 256.f);

	// Configuring the audio components
	WaterSplashSFX = CreateDefaultSubobject<UAudioComponent>(TEXT("Water Splash - SFX"));
	WaterSplashSFX->bAutoActivate = false;
	WaterSplashSFX->bAutoDestroy = false;
	WaterSplashSFX->SetupAttachment(GetRootComponent());
}

// Called after components have been initialized
void ASea::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// Attaching all the cues to the audio components
	if (WaterSplashSFX->IsValidLowLevelFast())
		WaterSplashSFX->SetSound(WaterSplashCue);
}

// Called when the game starts or when spawned
void ASea::BeginPlay()
{
	Super::BeginPlay();
	
	// Set up hit events here! Look for characters, items and destructibles.
	SeaCollision->OnComponentBeginOverlap.AddDynamic(this, &ASea::OnOverlapBegin);
	SeaCollision->OnComponentEndOverlap.AddDynamic(this, &ASea::OnOverlapEnd);

	if(SnapToCenterOfLevel)
		SetActorLocation(FVector(0.f, 0.f, 0.f));
}

// Called every frame
void ASea::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	WhileOverlapping();
}

void ASea::OnOverlapBegin(UPrimitiveComponent* OverlapComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	if (Cast<ABaseCharacter>(OtherActor))
	{
		if (Cast<ABaseCharacter>(OtherActor)->GetCapsuleComponent() == Cast<UCapsuleComponent>(OtherComp))
		{
			Cast<ABaseCharacter>(OtherActor)->CharacterSurfaceType = ECharacterSurfaceType::CST_Sea;
		}
	}
}

void ASea::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (Cast<ABaseCharacter>(OtherActor))
	{
		if (Cast<ABaseCharacter>(OtherActor)->GetCapsuleComponent() == Cast<UCapsuleComponent>(OtherComp))
		{
			Cast<ABaseCharacter>(OtherActor)->CharacterSurfaceType = ECharacterSurfaceType::CST_Sand;
		}
	}
}

void ASea::WhileOverlapping()
{
	TArray<AActor*> OverlappingCharacters;
	UClass* ActorFilter = ABaseCharacter::StaticClass();

	SeaCollision->GetOverlappingActors(OverlappingCharacters, ActorFilter);

	for (auto PossibleLaunchTarget : OverlappingCharacters)
	{
		if (Cast<ABaseCharacter>(PossibleLaunchTarget))
		{
			// Modify the depth calculation to use the same math as the material does, so wave height is taken into account.
			if ((GetActorLocation().Z - MaximumDepthBeforeLaunch) > (Cast<ABaseCharacter>(PossibleLaunchTarget)->GetActorLocation().Z))
			{
				//GEngine->AddOnScreenDebugMessage(3, 0.1f, FColor::Red, "YOU ARE TOO DEEP INTO THE SEA!", true);
				float HorizontalPushStrength{ 1000.f };
				float VerticalPushStrength{ 1000.f };

				FVector PushDirection = (Cast<ABaseCharacter>(PossibleLaunchTarget)->LastValidLocation - Cast<ABaseCharacter>(PossibleLaunchTarget)->GetActorLocation()).GetSafeNormal() * HorizontalPushStrength;
				PushDirection.Z = VerticalPushStrength;

				Cast<ABaseCharacter>(PossibleLaunchTarget)->LaunchCharacter(PushDirection, true, true);

				// Add particle FX
				if (WaterSplashCue)
				{
					WaterSplashSFX->SetWorldLocation(PossibleLaunchTarget->GetActorLocation());
					WaterSplashSFX->Play();
				}

				if (Cast<ABaseCharacter>(PossibleLaunchTarget)->WaterFX)
				{
					UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), Cast<ABaseCharacter>(PossibleLaunchTarget)->WaterFX, Cast<ABaseCharacter>(PossibleLaunchTarget)->GetActorLocation(), FRotator::ZeroRotator, FVector(5.f, 5.f, 5.f));
				}
			}
		}
	}
}