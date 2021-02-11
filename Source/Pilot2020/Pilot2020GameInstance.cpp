// Fill out your copyright notice in the Description page of Project Settings.


#include "Pilot2020GameInstance.h"


void UPilot2020GameInstance::StorePlayerInventory(int32 Gold, int32 HealingItems)
{
	PlayerGold = Gold;
	PlayerHealingItems = HealingItems;
}
