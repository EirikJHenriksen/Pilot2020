// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"

#include "UtilityTask.h"
#include "NavigationSystem.h"
#include "Engine/World.h"
#include "Engine/EngineTypes.h"
#include "BaseCharacter.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "TimerManager.h" 

#include "UtilityAIController.generated.h"

UCLASS()
class PILOT2020_API AUtilityAIController : public AAIController
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AUtilityAIController();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	//=================================
	// B A S E   V A R I A B L E S
	UPROPERTY()
		UNavigationSystemV1* NavSystem = nullptr;

	UPROPERTY()
		ABaseCharacter* ControlledCharacter = nullptr;

	// Override from the AI Controller.
	void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result) override;

	void EndPlay(const EEndPlayReason::Type EndPlayReason);

	UPROPERTY()
		UAIPerceptionComponent* NPCVision = nullptr;

	UPROPERTY()
		UAISenseConfig_Sight* SenseSight = nullptr;

	//===================================
	// D E T E C T   C H A R A C T E R S
	UFUNCTION()
		void OnSeeSomething(const TArray<AActor*>& SeenActors);

	void StopAlert();

	void EvaluateEnemyDistance();

	UPROPERTY()
		TMap<float, ABaseCharacter*> EnemyPriorityMap;

	// Time before the NPC leaves alert mode if no enemy is detected.
	UPROPERTY()
		FTimerHandle AlertTimeoutHandle;

	//=================================
	// C O M B A T   S Y S T E M
	FTimerHandle AttackChargeTimer;

	bool EnemyWithinAttackRange{ false };

	//=================================
	// T A S K   E V A L U A T I O N
	void AddTasksToLibrary();

	void ExecuteTask(UUtilityTask* Task);

	void EvaluateTasks();

	void PerformingTaskCheck(float DeltaTime);

	EBucketType ActiveUtilityBucket = EBucketType::BT_Idle;
	EBucketType LastUtilityBucket = EBucketType::BT_Idle;

	// Every task gets an integer assigned to them
	UPROPERTY()
		TMap<int32, UUtilityTask*> TaskLibrary;

	// Value of considered task, meant to be used for prioritizing tasks
	UPROPERTY()
		TMap<float, UUtilityTask*> TaskValueMap;

	// Decision inertia for tasks that lasts a significant duration
	void StopDecisionInertia();

	FTimerHandle DecInertiaTimerHandle;

	bool DecisionInertia = false;

	//====================================
	// T A S K   F U N C T I O N S

	// Idle
	void StandIdle();
	void WanderRandomly();

	// Alert
	float AcceptableRange{ 45.f };

	// Combat
	void LockOntoEnemy(ABaseCharacter* Target);

	UFUNCTION()
		void AttackEnemy(bool PowerAttack, bool ShouldChain);

	UFUNCTION()
		void AttackEnemyExecute();

	void DodgeRoll(ECharacterDirectionState Direction);

	//====================================
	// A R R A Y S - C H A R A C T E R S
	UPROPERTY()
		TArray<ABaseCharacter*> FriendCharacters;

	UPROPERTY()
		TArray<ABaseCharacter*> EnemyCharacters;

	//====================================
	// T A S K S - I D L E
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Utility AI - Task Pointers - Idle")
		UUtilityTask* TaskStandIdle = nullptr;

	float Boredom{ 0.f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Utility AI - Task Pointers - Idle")
		UUtilityTask* TaskWalkAround = nullptr;

	float Tired{ 0.f };

	//====================================
	// T A S K S - C O M B A T
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Utility AI - Task Pointers - Combat")
		UUtilityTask* TaskAttackEnemyLight = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Utility AI - Task Pointers - Combat")
		UUtilityTask* TaskAttackEnemyHeavy = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Utility AI - Task Pointers - Combat")
		UUtilityTask* TaskDodgeEnemy = nullptr;

	float DodgeTired{ 0.f };

	//==============================
	// A I  -  A U D I O 
	float voiceCooldown{ 0.f };
};
