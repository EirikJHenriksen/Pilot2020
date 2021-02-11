// Fill out your copyright notice in the Description page of Project Settings.


#include "FootstepLeftANotify.h"
#include "BaseCharacter.h"

void UFootstepLeftANotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	//GEngine->AddOnScreenDebugMessage(-1, 4.5f, FColor::Purple, __FUNCTION__);

	if (Cast<ABaseCharacter>(MeshComp->GetOwner()))
	{
		Cast<ABaseCharacter>(MeshComp->GetOwner())->PlayFootstepFX(false);
	}
}