// Fill out your copyright notice in the Description page of Project Settings.


#include "UtilityAIController.h"

AUtilityAIController::AUtilityAIController()
{
	// Set up the perception component
	NPCVision = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("NPC Vision"));
	NPCVision->OnPerceptionUpdated.AddDynamic(this, &AUtilityAIController::OnSeeSomething);
	SetPerceptionComponent(*NPCVision);
	
	// Set up the sense of sight, so the NPC can see other characters
	SenseSight = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("Eyes"));
	SenseSight->DetectionByAffiliation.bDetectEnemies = true;
	SenseSight->DetectionByAffiliation.bDetectFriendlies = true;
	SenseSight->DetectionByAffiliation.bDetectNeutrals = true;
	NPCVision->ConfigureSense(*SenseSight);
}

void AUtilityAIController::BeginPlay()
{
	Super::BeginPlay();

	// Initialize tasks
	AddTasksToLibrary();

	// Disable tick until it is re-enabled by the NPC
	SetActorTickEnabled(false);

	SetActorTickInterval(FMath::RandRange(0.4f, 0.8f));

	// Set up navigation pointer
	NavSystem = UNavigationSystemV1::GetCurrent(GetGameInstance()->GetWorld());
}

void AUtilityAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// If things are running as they should
	if (NavSystem && ControlledCharacter)
	{
		voiceCooldown -= DeltaTime;

		EvaluateTasks();
		PerformingTaskCheck(DeltaTime);

		if ((ActiveUtilityBucket == EBucketType::BT_Alert) || (ActiveUtilityBucket == EBucketType::BT_Combat))
		{
			EvaluateEnemyDistance();
		}
		else
		{
			if (ControlledCharacter->DamageReceivedByCharacter)
			{
				EnemyCharacters.Add(ControlledCharacter->DamageReceivedByCharacter);
				ActiveUtilityBucket = EBucketType::BT_Combat;

				ControlledCharacter->SetWeaponReady(true);
				
				EvaluateEnemyDistance();
			}
		}
	}
	else
	{
		// Error message, if pointers aren't set correctly
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "UTILITY AI ERROR: MISSING POINTERS!", true);
		}
	}
}

void AUtilityAIController::AddTasksToLibrary()
{
	int32 InitializerValue{ 0 };

	// Idle
	TaskLibrary.Add(InitializerValue++, TaskStandIdle);
	TaskLibrary.Add(InitializerValue++, TaskWalkAround);

	// Combat
	TaskLibrary.Add(InitializerValue++, TaskAttackEnemyLight);
	TaskLibrary.Add(InitializerValue++, TaskAttackEnemyHeavy);
	TaskLibrary.Add(InitializerValue++, TaskDodgeEnemy);
}

void AUtilityAIController::OnSeeSomething(const TArray<AActor*>& SeenActors)
{
	if (ControlledCharacter)
	{
		for (auto ActorToEvaluate : SeenActors)
		{
			if (Cast<ABaseCharacter>(ActorToEvaluate))
			{
				if (Cast<ABaseCharacter>(ActorToEvaluate)->CharacterLifeState != ECharacterLifeState::CLS_Dead)
				{
					if (Cast<ABaseCharacter>(ActorToEvaluate))
					{
						if (!FriendCharacters.Contains(ActorToEvaluate) && (Cast<ABaseCharacter>(ActorToEvaluate)->CharacterFaction == ControlledCharacter->CharacterFaction))
						{
							FriendCharacters.Add(Cast<ABaseCharacter>(ActorToEvaluate));
						}

						if (!EnemyCharacters.Contains(ActorToEvaluate) && (Cast<ABaseCharacter>(ActorToEvaluate)->CharacterFaction != ControlledCharacter->CharacterFaction))
						{
							EnemyCharacters.Add(Cast<ABaseCharacter>(ActorToEvaluate));

							LastUtilityBucket = ActiveUtilityBucket;
							ActiveUtilityBucket = EBucketType::BT_Alert;

							ControlledCharacter->SetWeaponReady(true);
						}
					}
				}
			}
		}

		if (ActiveUtilityBucket == EBucketType::BT_Alert)
		{
			float AlertTimer = 15.f;
			GetWorld()->GetTimerManager().SetTimer(AlertTimeoutHandle, this, &AUtilityAIController::StopAlert, AlertTimer, false);
		}
	}
}

void AUtilityAIController::StopAlert()
{
	if (ActiveUtilityBucket != EBucketType::BT_Combat || EnemyPriorityMap.begin().Value()->CharacterLifeState == ECharacterLifeState::CLS_Dead)
	{
		ActiveUtilityBucket = EBucketType::BT_Idle;
		ControlledCharacter->SetWeaponReady(false);
		ControlledCharacter->CharacterSpeedState = ECharacterSpeed::CS_Walking;

		StopMovement();
		EnemyCharacters.Empty();
	}
}

void AUtilityAIController::EvaluateEnemyDistance()
{
	// Clear the map, then fill with Enemies.
	EnemyPriorityMap.Empty();

	if (EnemyCharacters.Num() > 0)
	{
		for (auto Enemy : EnemyCharacters)
		{
			EnemyPriorityMap.Add(FVector::Distance(Enemy->GetActorLocation(), ControlledCharacter->GetActorLocation()), Enemy);
		}

		// Sort Map based on closest enemy.
		EnemyPriorityMap.KeySort([](float A, float B)
		{return A < B; });

		// Check if the enemy is too close.
		if (FVector::Distance(EnemyPriorityMap.begin().Value()->GetActorLocation(), ControlledCharacter->GetActorLocation()) < 2048.f)
		{
			if (ActiveUtilityBucket == EBucketType::BT_Alert)
			{
				// voice
				ControlledCharacter->PlaySound_SpotPlayer();
			}

			LastUtilityBucket = EBucketType::BT_Alert;
			ActiveUtilityBucket = EBucketType::BT_Combat;

			if (ControlledCharacter->CharacterActionState != ECharacterActionState::CAS_Dodging || ControlledCharacter->CharacterMovementState != ECharacterMovementState::CMS_Dodging)
			{
				// Move this into task evaluation!!!
				MoveToActor(EnemyPriorityMap.begin().Value(), AcceptableRange);
			}

			if (FVector::Distance(EnemyPriorityMap.begin().Value()->GetActorLocation(), ControlledCharacter->GetActorLocation()) < 512.f)
			{
				ControlledCharacter->StopSprint();
				LockOntoEnemy(EnemyPriorityMap.begin().Value());

				if (EnemyPriorityMap.begin().Value()->CharacterLifeState == ECharacterLifeState::CLS_Dead)
				{
					StopAlert();
				}
			}
			else
			{
				ControlledCharacter->StartSprint();
				LockOntoEnemy(nullptr);
			}

			if (FVector::Distance(EnemyPriorityMap.begin().Value()->GetActorLocation(), ControlledCharacter->GetActorLocation()) < AcceptableRange * 5.f)
			{
				EnemyWithinAttackRange = true;
			}
			else
			{
				EnemyWithinAttackRange = false;
			}
		}
		else
		{
			ControlledCharacter->StopSprint();
			LockOntoEnemy(nullptr);

			LastUtilityBucket = EBucketType::BT_Combat;
			ActiveUtilityBucket = EBucketType::BT_Alert;
		}
	}
	else
	{
		LockOntoEnemy(nullptr);
	}
}

void AUtilityAIController::LockOntoEnemy(ABaseCharacter* Target)
{
	if (Target)
	{
		ControlledCharacter->IsLockedOn = true;
		ControlledCharacter->LockedTargetCharacter = Target;
	}
	else
	{
		ControlledCharacter->LockedTargetCharacter = nullptr;
		ControlledCharacter->IsLockedOn = false;
	}
}

void AUtilityAIController::AttackEnemy(bool PowerAttack, bool ShouldChain)
{
	// Set timer before the attack is actually executed
	if (PowerAttack)
	{
		GetWorld()->GetTimerManager().SetTimer(AttackChargeTimer, this, &AUtilityAIController::AttackEnemyExecute, FMath::FRandRange(0.1f, 0.5f), false);
	}
	else
	{
		GetWorld()->GetTimerManager().SetTimer(AttackChargeTimer, this, &AUtilityAIController::AttackEnemyExecute, FMath::FRandRange(0.8f, 1.6f), false);
	}

	// Start the attack process
	ControlledCharacter->ReceivingAttackInput = true;
	ControlledCharacter->AttackStart();
}

void AUtilityAIController::AttackEnemyExecute()
{
	// Do attack
	ControlledCharacter->ReceivingAttackInput = false;
	ControlledCharacter->AttackEnd();
}

void AUtilityAIController::DodgeRoll(ECharacterDirectionState Direction)
{
	StopMovement();
	GetWorld()->GetTimerManager().ClearTimer(AttackChargeTimer);
	ControlledCharacter->CharacterDirectionState = Direction;
	ControlledCharacter->DodgeRoll();
}

void AUtilityAIController::ExecuteTask(UUtilityTask* Task)
{
	if (Task)
	{
		Task->TaskExecute();

		switch (*TaskLibrary.FindKey(Task))
		{
		case 0:
			StandIdle();
			break;
		case 1:
			WanderRandomly();
			break;
		case 2:
			if (ControlledCharacter->CharacterActionState != ECharacterActionState::CAS_Dodging)
			{
				AttackEnemy(false, false);
			}

			DodgeTired -= 0.2f;
			break;
		case 3:
			if (ControlledCharacter->CharacterActionState != ECharacterActionState::CAS_Dodging)
			{
				AttackEnemy(true, false);
			}

			DodgeTired -= 0.2f;
			break;
		case 4:
			if (ControlledCharacter->CharacterActionState != ECharacterActionState::CAS_Dodging)
			{
				switch (FMath::RandRange(0, 2))
				{
				case 0:
					DodgeRoll(ECharacterDirectionState::CDS_Backwards);
					break;
				case 1:
					DodgeRoll(ECharacterDirectionState::CDS_Left);
					break;
				case 2:
					DodgeRoll(ECharacterDirectionState::CDS_Right);
					break;
				default:
					DodgeRoll(ECharacterDirectionState::CDS_Backwards);
					break;
				}

				DodgeTired += 0.2f;
			}
			break;
		default:
			break;
		}
	}

	DodgeTired = FMath::Clamp(DodgeTired, 0.f, 1.f);
}

void AUtilityAIController::EvaluateTasks()
{
	// Wiping the map first
	TaskValueMap.Empty();

	switch (ActiveUtilityBucket)
	{
	case EBucketType::BT_Idle:
		// Decreases tick rate for optimization
		SetActorTickInterval(FMath::RandRange(0.4f, 0.8f));

		if (!DecisionInertia)
		{
			// Does the calculations
			float WalkScore = TaskWalkAround->EvaluateTask(Boredom);
			float IdleScore = TaskStandIdle->EvaluateTask(Tired);

			// Adding tasks to the map newly wiped map, with the score value as keys
			TaskValueMap.Add(WalkScore, TaskWalkAround);
			TaskValueMap.Add(IdleScore, TaskStandIdle);

			DecisionInertia = true;
		}
		break;
	case EBucketType::BT_Alert:
		// Increases tick rate for responsivness
		// SetActorTickInterval(FMath::RandRange(0.1f, 0.4f));
		// Search behavior goes here.

		break;
	case EBucketType::BT_Combat:
		// Higher tick rate for more responsive combat
		SetActorTickInterval(FMath::RandRange(0.05f, 0.1f));

		if (EnemyWithinAttackRange)
		{
			if (ControlledCharacter->CharacterActionState != ECharacterActionState::CAS_Attacking && ControlledCharacter->CharacterActionState != ECharacterActionState::CAS_Dodging && !ControlledCharacter->ReceivingAttackInput)
			{
				// Finding the total number of allies nearby
				TArray<AActor*> AllyCheck;
				UClass* ActorFilter = ABaseCharacter::StaticClass();
				ControlledCharacter->LockOnRangeSphere->GetOverlappingActors(AllyCheck, ActorFilter);

				int32 AllyCounter{ 0 };

				for (auto PotentialAlly : AllyCheck)
				{
					if (Cast<ABaseCharacter>(PotentialAlly)->CharacterFaction == ControlledCharacter->CharacterFaction)
					{
						AllyCounter++;
					}
				}

				// Attack scores
				float GeneralAttackScore{ 1.f };
				GeneralAttackScore *= (ControlledCharacter->Health / ControlledCharacter->HealthMaximum) - (EnemyPriorityMap.begin().Value()->ReceivingAttackInput * FMath::RandRange(0.2f, 0.3f) + (0.1f * AllyCounter));
				GeneralAttackScore = FMath::Clamp(GeneralAttackScore, 0.f, 1.f);

				float AttackLScore{ 1.f };
				AttackLScore *= GeneralAttackScore + (1.f - (EnemyPriorityMap.begin().Value()->Health / EnemyPriorityMap.begin().Value()->HealthMaximum)) * FMath::RandRange(0.1f, 0.3f);
				AttackLScore = FMath::Clamp(AttackLScore, 0.f, 1.f);
				AttackLScore = TaskAttackEnemyLight->EvaluateTask(AttackLScore);

				float AttackHScore{ 1.f };
				AttackHScore *= GeneralAttackScore + ((EnemyPriorityMap.begin().Value()->Health / EnemyPriorityMap.begin().Value()->HealthMaximum) * FMath::RandRange(0.1f, 0.3f));
				AttackHScore = FMath::Clamp(AttackHScore, 0.f, 1.f);
				AttackHScore = TaskAttackEnemyHeavy->EvaluateTask(AttackHScore);

				// Dodge score
				float DodgeScore{ 1.f };
				DodgeScore *= FMath::Clamp(1.f - (ControlledCharacter->Health / ControlledCharacter->HealthMaximum), 0.f, 0.85f) * (EnemyPriorityMap.begin().Value()->ReceivingAttackInput * FMath::RandRange(0.1f, 0.3f)) + FMath::RandRange(0.1f, 0.3f) * (1.f - DodgeTired);
				
				if (ControlledCharacter->HasReceivedDamage)
				{
					DodgeScore *= 1.8f;
					ControlledCharacter->HasReceivedDamage = false;
				}
				
				DodgeScore = FMath::Clamp(DodgeScore, 0.f, 1.f);
				DodgeScore = TaskDodgeEnemy->EvaluateTask(DodgeScore);



				// Adding tasks to the map newly wiped map, with the score value as keys
				TaskValueMap.Add(AttackLScore, TaskAttackEnemyLight);
				TaskValueMap.Add(AttackHScore, TaskAttackEnemyHeavy);
				TaskValueMap.Add(DodgeScore, TaskDodgeEnemy);
			}
		}
		break;
	}

	// Sort Map based on highest score
	if (TaskValueMap.begin())
	{
		TaskValueMap.KeySort([](float A, float B)
		{return A > B; });

		ExecuteTask(TaskValueMap.begin().Value());
	}
}

void AUtilityAIController::PerformingTaskCheck(float DeltaTime)
{
	if (DecisionInertia)
	{
		if (TaskStandIdle && TaskStandIdle->TaskInProgress)
		{
			Boredom += FMath::RandRange(0.01f, 0.1f) * DeltaTime;
			Tired -= FMath::RandRange(0.01f, 0.1f) * DeltaTime;
		}

		if (TaskWalkAround && TaskWalkAround->TaskInProgress)
		{
			Tired += FMath::RandRange(0.01f, 0.1f) * DeltaTime;
			Boredom -= FMath::RandRange(0.01f, 0.1f) * DeltaTime;
		}

		Tired = FMath::Clamp(Tired, 0.f, 1.f);
		Boredom = FMath::Clamp(Boredom, 0.f, 1.f);
	}
}

void AUtilityAIController::StopDecisionInertia()
{
	//GetWorldTimerManager().ClearTimer(DecInertiaTimerHandle);
	TaskStandIdle->TaskInProgress = false;
	TaskWalkAround->TaskInProgress = false;

	DecisionInertia = false;
}

void AUtilityAIController::StandIdle()
{
	GetWorldTimerManager().SetTimer(DecInertiaTimerHandle, this, &AUtilityAIController::StopDecisionInertia, FMath::RandRange(1.f, 3.f), false);

	// voice
	if (ControlledCharacter->CharacterFaction == ECharacterFaction::CF_Bandits)
	{
		if (voiceCooldown < 0)
		{ 
			voiceCooldown = FMath::RandRange(10.f, 20.f); // arbitrary cooldown time
			ControlledCharacter->PlaySound_Idle();
		}
		else
		{
			voiceCooldown -= 0.1f;
		}
	}
}

void AUtilityAIController::WanderRandomly()
{
	FNavLocation PointToWalkTo;

	if (NavSystem->GetRandomReachablePointInRadius(ControlledCharacter->GetActorLocation(), 1024.f, PointToWalkTo))
	{
		MoveToLocation(PointToWalkTo.Location);
	}
}

void AUtilityAIController::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult & Result)
{
	TaskWalkAround->TaskInProgress = false;
	StopDecisionInertia();
}

void AUtilityAIController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
}