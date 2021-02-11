// Fill out your copyright notice in the Description page of Project Settings.


#include "ChargeUpANotifyState.h"
#include "BaseCharacter.h"

void UChargeUpANotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime)
{
	//GEngine->AddOnScreenDebugMessage(-1, 4.5f, FColor::Green, __FUNCTION__);
}

void UChargeUpANotifyState::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime)
{
}

void UChargeUpANotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	//GEngine->AddOnScreenDebugMessage(-1, 4.5f, FColor::Red, __FUNCTION__);
}