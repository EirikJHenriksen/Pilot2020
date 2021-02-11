// Fill out your copyright notice in the Description page of Project Settings.


#include "IKDisableANotifyState.h"
#include "BaseCharacter.h"

void UIKDisableANotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime)
{
	//GEngine->AddOnScreenDebugMessage(-1, 4.5f, FColor::Green, __FUNCTION__);

	if (Cast<ABaseCharacter>(MeshComp->GetOwner()))
	{
		Cast<ABaseCharacter>(MeshComp->GetOwner())->IKDisabled = true;
	}
}

void UIKDisableANotifyState::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime)
{
}

void UIKDisableANotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	//GEngine->AddOnScreenDebugMessage(-1, 4.5f, FColor::Red, __FUNCTION__);

	if (Cast<ABaseCharacter>(MeshComp->GetOwner()))
	{
		Cast<ABaseCharacter>(MeshComp->GetOwner())->IKDisabled = false;
	}
}
