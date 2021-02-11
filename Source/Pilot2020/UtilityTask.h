// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

#include "Curves/CurveFloat.h"

#include "UtilityTask.generated.h"


UENUM(BlueprintType)
enum class EBucketType : uint8
{
	BT_Idle		UMETA(DisplayName = "Idle"),
	BT_Alert	UMETA(DisplayName = "Alert"),
	BT_Combat	UMETA(DisplayName = "Combat"),
	BT_Fleeing	UMETA(DisplayName = "Fleeing")
};

USTRUCT(BlueprintType)
struct FTaskData
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task Data")
		FName TaskName{ "Untitled" };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task Data")
		EBucketType TaskType{ EBucketType::BT_Idle };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task Data")
		UCurveFloat* EvaluationGraph{ nullptr };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task Data")
		float TaskBasePriority{ 1.f };

	FTaskData() {}
};


class AUtilityAIController;

UCLASS()
class PILOT2020_API UUtilityTask : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Utility Task Data")
		FTaskData ContainedData;

	float EvaluateTask(float Value);

	void TaskExecute();
	void TaskComplete();

	bool TaskInProgress{ false };
};
