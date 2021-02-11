// Fill out your copyright notice in the Description page of Project Settings.


#include "FootstepRightANotify.h"
#include "BaseCharacter.h"

void UFootstepRightANotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	//GEngine->AddOnScreenDebugMessage(-1, 4.5f, FColor::Purple, __FUNCTION__);
	
	if (Cast<ABaseCharacter>(MeshComp->GetOwner()))
	{
		Cast<ABaseCharacter>(MeshComp->GetOwner())->PlayFootstepFX(true);
	}
}