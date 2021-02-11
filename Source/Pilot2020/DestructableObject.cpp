// Fill out your copyright notice in the Description page of Project Settings.


#include "DestructableObject.h"
#include "PickupItem.h"
#include "Components/PrimitiveComponent.h"
#include "WeaponSceneComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Math/UnrealMathUtility.h" // for random number
#include "IslandManager.h"

//For debug messages
#include "Engine/Engine.h"

// Sets default values
ADestructableObject::ADestructableObject()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootSceneComp = CreateDefaultSubobject<USceneComponent>(TEXT("root"));
	RootComponent = RootSceneComp;

	DestructableComp = CreateDefaultSubobject<UDestructibleComponent>(TEXT("mesh"));
	DestructableComp->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void ADestructableObject::BeginPlay()
{
	Super::BeginPlay();

	// give a random mesh
	if (DestructibleMeshArray.Num() > 0)
	{
		int whichMesh = FMath::RandRange(int(0), DestructibleMeshArray.Num() - 1);
		DestructableComp->SetDestructibleMesh(DestructibleMeshArray[whichMesh]);
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "Error: DestructibleMeshArray is empty (BP_DestructableObject)", true);
	}
	
	DestructableComp->OnComponentBeginOverlap.AddDynamic(this, &ADestructableObject::myOnOverlapBegin);
	DestructableComp->OnComponentHit.AddDynamic(this, &ADestructableObject::myOnComponentHit);

	// set its weight
	TArray<FName> sockets = DestructableComp->GetAllSocketNames();
	for (int i = 0; i < sockets.Num(); i++)
	{
		DestructableComp->SetMassOverrideInKg(sockets[i], weightKG, true);
	}
	
	
}

// Called every frame
void ADestructableObject::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// for being hit with weapon
void ADestructableObject::myOnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (HealthPoints > 0)
	{ 
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "ADestructableObject::OnOverlapBegin", true);

		// if overlapping component is a weapon
		if (OtherComp->GetFName().ToString() == "Weapon Collision")
		{
			DestructableComp->WakeAllRigidBodies(); // turn on physics

			//HealthPoints -= 34;
			HealthPoints = 0;

			if (HealthPoints <= 0)
			{
				BecomesBroken();
			}

		}
	}
}

// called a lot, whenever the object moves on the ground
void ADestructableObject::myOnComponentHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit)
{
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "ADestructableObject::OnHit !!!!!!!!!!!", true);

	if (OtherActor->IsA(AIslandManager::StaticClass()) != true) // so if it's not the ground itself that is "hitting"
	{ 
		if (sleeping == true) // for bugtesting
		{
			sleeping = false;
			//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, OtherActor->GetName(), true);

			if ((OtherActor->GetName().Mid(0, 18) != "BP_PlayerCharacter") && (OtherActor->GetName().Mid(0, 18) != "BP_BanditCharacter")) // this is a quick-fix...
			{
				//GEngine->AddOnScreenDebugMessage(-1, 120.f, FColor::Red, OtherActor->GetName().Mid(0, 18), true);
				this->Destroy();
			}
		}
	
		DestructableComp->WakeAllRigidBodies(); // turn on physics

		float myVelocity = DestructableComp->GetComponentVelocity().Size(); // how fast it's moving upon impact, so gentle rolling doesn't hurt

		if (myVelocity > 200.f) // is it moving fast enough to matter?
		{
			if (HealthPoints > 0)
			{
				if (DestructableComp->GetComponentVelocity().Size() > 100.f)
				{
					HealthPoints -= FMath::TruncToInt(myVelocity / 100);

					if (HealthPoints <= 0)
					{
						BecomesBroken();
					}
					else
					{
						playSoundDamage();
					}
				}
			}
		}
	}

}

void ADestructableObject::DropItems()
{
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "boop", true);
	FVector RandomDirection = FVector(FMath::RandRange(0.f, 10.f), FMath::RandRange(0.f, 10.f), FMath::RandRange(0.f, 10.f)).GetSafeNormal() * 1000.f;

	FVector spawnLocation = DestructableComp->GetComponentLocation();

	if (GoldItemClass)
	{
		for (int32 i = GoldCarried; i > 0; i--)
		{
			RandomDirection = FVector(FMath::RandRange(0.f, 10.f), FMath::RandRange(0.f, 10.f), FMath::RandRange(0.f, 10.f)).GetSafeNormal() * 1000.f;
			GetWorld()->SpawnActor<APickupItem>(GoldItemClass, spawnLocation, FRotator(0.f, FMath::RandRange(0.f, 359.f), 0.f))->ItemMeshComp->AddImpulse(RandomDirection);
		}
	}
	else
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "ERROR! GOLD INVENTORY ITEM CLASS WAS NOT SET IN BLUEPRINT!", true);
		}
	}

	if (FoodItemClass)
	{
		for (int32 i = FoodCarried; i > 0; i--)
		{
			RandomDirection = FVector(FMath::RandRange(0.f, 10.f), FMath::RandRange(0.f, 10.f), FMath::RandRange(0.f, 10.f)).GetSafeNormal() * 1000.f;
			GetWorld()->SpawnActor<APickupItem>(FoodItemClass, spawnLocation, FRotator(0.f, FMath::RandRange(0.f, 359.f), 0.f))->ItemMeshComp->AddImpulse(RandomDirection);
		}
	}
	else
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "ERROR! FOOD/HEALTH INVENTORY ITEM CLASS WAS NOT SET IN BLUEPRINT!", true);
		}
	}
}

void ADestructableObject::BecomesBroken()
{
	playSoundDestroy();


	DropItems();

	DestructableComp->ApplyRadiusDamage(5000.0f, DestructableComp->GetRelativeLocation(), 10.f, 1000.f, false);

	DestructableComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	//FOutputDeviceNull ar;
	//this->CallFunctionByNameWithArguments(TEXT("mySetCollisionPhysicsOnly"), ar, nullptr, true);

}

