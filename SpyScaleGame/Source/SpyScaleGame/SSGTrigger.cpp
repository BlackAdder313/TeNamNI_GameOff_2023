// Fill out your copyright notice in the Description page of Project Settings.


#include "SSGTrigger.h"

#include "SSGInteractable.h"

// Sets default values
ASSGTrigger::ASSGTrigger()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

}

// Called when the game starts or when spawned
void ASSGTrigger::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASSGTrigger::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CurrentMass = 0.f;

	for (int32 Index = ActiveInteractables.Num() - 1; Index >= 0; --Index)
	{
		const ASSGInteractable* Interactable = ActiveInteractables[Index].Get();
		if (Interactable == nullptr)
		{
			ActiveInteractables.RemoveAtSwap(Index);
			continue;
		}

		// only allow mass on things that have settled...
		if (Interactable->GetVelocity().Size() < MaxInteractableVelocity)
		{
			CurrentMass += Interactable->GetMass();
		}
	}

	bool bWasComplete = bIsComplete;

	if (ActiveInteractables.Num() && TriggerMassThreshold <= CurrentMass)
	{
		if (!bIsComplete)
		{
			CurrentCompletitonTimer += DeltaTime;
		}

		bIsComplete = CompleteTimer <= CurrentCompletitonTimer;
	}
	else
	{
		bIsComplete = false;
		CurrentCompletitonTimer = 0.f;
	}

	if (bWasComplete != bIsComplete)
	{
		On_CompleteChange(bIsComplete);
	}
}

float ASSGTrigger::GetCompletePercentage() const
{
	if (ActiveInteractables.IsEmpty())
	{
		return 0.f;
	}

	if (TriggerMassThreshold <= 0.f)
	{
		return 1.f;
	}

	return FMath::Min(CurrentMass / TriggerMassThreshold, 1.f);
}

void ASSGTrigger::BP_TriggerOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (const ASSGInteractable* Interactable = Cast<ASSGInteractable>(OtherActor))
	{
		Track(*Interactable);
	}
}

void ASSGTrigger::BP_EndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (const ASSGInteractable* Interactable = Cast<ASSGInteractable>(OtherActor))
	{
		Untrack(*Interactable);
	}
}

void ASSGTrigger::Track(const ASSGInteractable& Interactable)
{
	ActiveInteractables.AddUnique(&Interactable);
}

void ASSGTrigger::Untrack(const ASSGInteractable& Interactable)
{
	ActiveInteractables.Remove(&Interactable);
}

