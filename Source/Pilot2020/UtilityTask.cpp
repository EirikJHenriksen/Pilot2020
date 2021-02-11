// Fill out your copyright notice in the Description page of Project Settings.


#include "UtilityTask.h"

float UUtilityTask::EvaluateTask(float Value)
{
	return ContainedData.EvaluationGraph->GetFloatValue(Value);
}

void UUtilityTask::TaskExecute()
{
	TaskInProgress = true;
}

void UUtilityTask::TaskComplete()
{
	TaskInProgress = false;
}
