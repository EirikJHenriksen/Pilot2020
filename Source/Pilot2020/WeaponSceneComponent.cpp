// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponSceneComponent.h"
#include "BaseCharacter.h"
#include "Components/SkeletalMeshComponent.h"

//For debug messages
#include "Engine/Engine.h"

// Sets default values for this component's properties
UWeaponSceneComponent::UWeaponSceneComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// Creating the mesh of the weapon component
	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Weapon Visual"));
	WeaponMesh->SetupAttachment(this);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetVisibility(false);

	// Adding a collision capsule
	WeaponCollision = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Weapon Collision"));
	
	WeaponCollision->InitCapsuleSize(6.f, 80.0f);
	WeaponCollision->SetupAttachment(WeaponMesh);
	WeaponCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	WeaponCollision->SetRelativeLocation(FVector(0.f, 0.f, 40.f));
	
	WeaponCollision->OnComponentBeginOverlap.AddDynamic(this, &UWeaponSceneComponent::OnOverlapBegin);
}

// Called when the game starts
void UWeaponSceneComponent::BeginPlay()
{
	Super::BeginPlay();

	// Checks if the owner has a data table.
	if (WeaponOwner->WeaponDataTable)
	{
		// Load information from data table.
		LoadData();

		WeaponOwner->EquippedWeapon = this;

		// Attach weapon to socket.
		UpdateSocketAttachement(false);

		if (WeaponOwner->ShowWeaponCollision)
		{
			WeaponCollision->SetHiddenInGame(false);
		}
	}
}


// Called every frame
void UWeaponSceneComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UWeaponSceneComponent::LoadData()
{
	FWeaponData* WeaponStruct = WeaponOwner->WeaponDataTable->FindRow<FWeaponData>(WeaponOwner->WeaponRowName, TEXT("Weapon Data"), true);

	if (WeaponStruct)
	{
		WeaponData = *WeaponStruct;

		if (WeaponMesh)
		{
			WeaponMesh->SetStaticMesh(WeaponData.WeaponMesh);

			// If the weapon is an item and not a part of the characters innate abilities, make it visible.
			if (WeaponData.WeaponAnimationType != EWeaponAnimType::WAT_Natural)
			{
				WeaponMesh->SetVisibility(true);
			}
		}

		WeaponCollision->SetCapsuleSize(WeaponData.WeaponCollisionRadius, WeaponData.WeaponCollisionLength/2.f);

		WeaponCollision->SetRelativeLocation(FVector(0.f, 0.f, WeaponData.WeaponCollisionOffsetZ));
	}
}

// This function is called from the anim notify event placed in the equip animation
void UWeaponSceneComponent::UpdateSocketAttachement(bool weaponDrawn)
{
	if (!WeaponOwner->WeaponReady)
	{
		switch (WeaponData.WeaponAnimationType)
		{
		case EWeaponAnimType::WAT_OneHanded:
			this->AttachToComponent(WeaponOwner->GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, "SheathOneHanded");
			break;
		case EWeaponAnimType::WAT_TwoHanded:
			this->AttachToComponent(WeaponOwner->GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, "SheathTwoHanded");
			break;
		default:
			this->AttachToComponent(WeaponOwner->GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, "SheathOneHanded");
			break;
		}
	}
	else
	{
		this->AttachToComponent(WeaponOwner->GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, "WeaponRight");
	}
}

void UWeaponSceneComponent::OnOverlapBegin(UPrimitiveComponent* OverlapComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	if (OtherActor && WeaponOwner->DamageCanBeDealt)
	{
		if (OtherActor != WeaponOwner)
		{
			if (Cast<ABaseCharacter>(OtherActor))
			{
				// Ensures the weapon hits the characters capsule and nothing else. Might be changed into the character mesh itself.
				if ((Cast<ABaseCharacter>(OtherActor)->GetCapsuleComponent() == Cast<UCapsuleComponent>(OtherComp)))
				{
					// Applies damage to the character
					Cast<ABaseCharacter>(OtherActor)->ReceiveAttackDamage(WeaponOwner, -WeaponData.WeaponDamage * WeaponOwner->DamageMultiplier * WeaponOwner->AttackPower, SweepResult.ImpactNormal);
				}
			}
		}
	}
}
