// Fill out your copyright notice in the Description page of Project Settings.


#include "SpawnBox.h"
#include "IslandManager.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Runtime/Engine/Classes/Engine/EngineTypes.h"
#include "Engine/World.h"


//For debug messages
#include "Engine/Engine.h"
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("test: %f"), radius));

bool USpawnBox::SpawnActor(TSubclassOf<class AActor> AnotherClass, float objectRadius)
{
	//bool successfullySpawned{ false };
	AActor* spawnedActor{ nullptr };

	// First do a line trace
	FVector BoundOrigin;
	FVector BoundBoxExtent;
	float BoundRadius;
	UKismetSystemLibrary::GetComponentBounds(this, BoundOrigin, BoundBoxExtent, BoundRadius);

	FVector RandomPoint = UKismetMathLibrary::RandomPointInBoundingBox(BoundOrigin, BoundBoxExtent);

	//FVector traceStart(FVector(RandomPoint.X, RandomPoint.Y, BoundBoxExtent.Z)); // height on top of box
	FVector traceStart(FVector(RandomPoint.X, RandomPoint.Y, 10000.f)); // height under box?
	FVector traceEnd(FVector(RandomPoint.X, RandomPoint.Y, 0));  // height only down to sea level

	TArray<TEnumAsByte<EObjectTypeQuery>> objectTypes;
	objectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_WorldStatic));
	objectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_WorldDynamic));

	TArray<AActor*> actorsToIgnore;
	//actorsToIgnore.Add(this);

	FHitResult SphereTraceHitResult;

	// send in radius
	traceRadius = objectRadius;

	bool didHit = UKismetSystemLibrary::SphereTraceSingleForObjects(this, traceStart, traceEnd, traceRadius, objectTypes, false, actorsToIgnore, EDrawDebugTrace::None, SphereTraceHitResult, false);


	// check if you hit something (else your outside the islands land)
	if (didHit)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "hit something");

		// is it an island?
		if (SphereTraceHitResult.GetActor()->GetClass()->IsChildOf(AIslandManager::StaticClass()))
		{

			// found suitable spot for spawning - now spawn the thing!

			FVector RandomDirection;

					if (AnotherClass)
					{
						// trace line down to ground to get more precise spawn location
						FHitResult LineTraceHitResult;
						UKismetSystemLibrary::LineTraceSingleForObjects(this, traceStart, traceEnd, objectTypes, false, actorsToIgnore, EDrawDebugTrace::None, LineTraceHitResult, false);
						FVector spawnLocation = LineTraceHitResult.Location;


						//NewActor = GetWorld()->SpawnActor<AActor>(FoodItemClass, spawnLocation, FRotator::ZeroRotator);
						RandomDirection = FVector(FMath::RandRange(0.f, 10.f), FMath::RandRange(0.f, 10.f), FMath::RandRange(0.f, 10.f)).GetSafeNormal() * 1000.f;
						//successfullySpawned = GetWorld()->SpawnActor<AActor>(AnotherClass, spawnLocation+FVector(0,0,0), FRotator(0.f, FMath::RandRange(0.f, 359.f), 0.f));
						spawnedActor = GetWorld()->SpawnActor<AActor>(AnotherClass, spawnLocation + FVector(0, 0, 0), FRotator(SphereTraceHitResult.ImpactNormal.ToOrientationRotator())+FRotator(-90.f, 0,0));
						//TraceHitResult.ImpactNormal
					}
		}
	}
	if (spawnedActor)
		return true;
	else
		return false;
}

