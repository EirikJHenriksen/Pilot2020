// Fill out your copyright notice in the Description page of Project Settings.


#include "ChainANotifyState.h"
#include "BaseCharacter.h"

void UChainANotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime)
{
	//GEngine->AddOnScreenDebugMessage(-1, 4.5f, FColor::Green, __FUNCTION__);

	if (Cast<ABaseCharacter>(MeshComp->GetOwner()))
	{
		Cast<ABaseCharacter>(MeshComp->GetOwner())->ActiveAttackChainWindow = true;
	}
}

void UChainANotifyState::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime)
{
}

void UChainANotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	//GEngine->AddOnScreenDebugMessage(-1, 4.5f, FColor::Red, __FUNCTION__);

	if (Cast<ABaseCharacter>(MeshComp->GetOwner()))
	{
		Cast<ABaseCharacter>(MeshComp->GetOwner())->ActiveAttackChainWindow = false;
	}
}