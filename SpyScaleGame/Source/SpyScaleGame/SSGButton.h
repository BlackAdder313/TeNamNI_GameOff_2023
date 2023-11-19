// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SSGButton.generated.h"

class ASSGTrigger;

UCLASS()
class SPYSCALEGAME_API ASSGButton : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASSGButton();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	void PressButton();

	UFUNCTION(BlueprintImplementableEvent)
	void BP_OnButtonPressed();

	UFUNCTION(BlueprintCallable)
	bool IsComplete() { return bIsComplete; }

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, Category = Gameplay)
	TArray<ASSGTrigger*> RequiredTriggers;

	bool bIsComplete = false;
};
