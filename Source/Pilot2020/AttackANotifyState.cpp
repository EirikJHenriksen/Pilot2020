// Fill out your copyright notice in the Description page of Project Settings.


#include "AttackANotifyState.h"
#include "BaseCharacter.h"

void UAttackANotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime)
{
	//GEngine->AddOnScreenDebugMessage(-1, 4.5f, FColor::Green, __FUNCTION__);

	if (Cast<ABaseCharacter>(MeshComp->GetOwner()))
	{
		Cast<ABaseCharacter>(MeshComp->GetOwner())->DamageCanBeDealt = true;
		
		if (Cast<ABaseCharacter>(MeshComp->GetOwner())->EquippedWeapon)
		{
			Cast<ABaseCharacter>(MeshComp->GetOwner())->AttackSwingSFX->SetSound(Cast<ABaseCharacter>(MeshComp->GetOwner())->EquippedWeapon->WeaponData.WeaponSwingCue);
			
			if (!Cast<ABaseCharacter>(MeshComp->GetOwner())->AttackSwingSFX->IsPlaying())
				Cast<ABaseCharacter>(MeshComp->GetOwner())->AttackSwingSFX->Play();
		}
	}
}

void UAttackANotifyState::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime)
{
}

void UAttackANotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	//GEngine->AddOnScreenDebugMessage(-1, 4.5f, FColor::Red, __FUNCTION__);

	if (Cast<ABaseCharacter>(MeshComp->GetOwner()))
	{
		Cast<ABaseCharacter>(MeshComp->GetOwner())->AttackCancel();

		if (Cast<ABaseCharacter>(MeshComp->GetOwner())->ReceivingSprintInput)
		{
			Cast<ABaseCharacter>(MeshComp->GetOwner())->StartSprint();
		}
	}
}