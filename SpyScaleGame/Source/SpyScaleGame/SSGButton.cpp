// Fill out your copyright notice in the Description page of Project Settings.


#include "SSGButton.h"

#include "SSGTrigger.h"

// Sets default values
ASSGButton::ASSGButton()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ASSGButton::BeginPlay()
{
	Super::BeginPlay();
	
}

void ASSGButton::PressButton()
{
	if (!IsComplete())
	{
		return;
	}

	BP_OnButtonPressed();
}

// Called every frame
void ASSGButton::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	bIsComplete = true;

	for (ASSGTrigger* Trigger : RequiredTriggers)
	{
		if (!Trigger->IsComplete())
		{
			bIsComplete = false;
			break;
		}
	}
}

