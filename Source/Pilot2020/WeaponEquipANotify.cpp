// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponEquipANotify.h"
#include "BaseCharacter.h"

void UWeaponEquipANotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	//GEngine->AddOnScreenDebugMessage(-1, 4.5f, FColor::Purple, __FUNCTION__);

	if (Cast<ABaseCharacter>(MeshComp->GetOwner()))
	{
		Cast<ABaseCharacter>(MeshComp->GetOwner())->EquippedWeapon->UpdateSocketAttachement(Cast<ABaseCharacter>(MeshComp->GetOwner())->WeaponReady);

		if (Cast<ABaseCharacter>(MeshComp->GetOwner())->WeaponReady && Cast<ABaseCharacter>(MeshComp->GetOwner())->EquippedWeapon->WeaponData.WeaponDrawCue)
		{
			Cast<ABaseCharacter>(MeshComp->GetOwner())->ActionSFX->SetSound(Cast<ABaseCharacter>(MeshComp->GetOwner())->EquippedWeapon->WeaponData.WeaponDrawCue);
			Cast<ABaseCharacter>(MeshComp->GetOwner())->ActionSFX->Play();
		}
	}
}