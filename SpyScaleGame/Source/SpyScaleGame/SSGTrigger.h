// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SSGTrigger.generated.h"

class UPrimitiveComponent;
class ASSGInteractable;

struct FHitResult;

UCLASS()
class SPYSCALEGAME_API ASSGTrigger : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASSGTrigger();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	bool IsComplete() const { return ActiveInteractables.Num() && TriggerMassThreshold <= CurrentMass; }

protected:
	// How much mass it requires to complete the object when it enters the trigger
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	float TriggerMassThreshold = 0.f;

	// How long it takes to complete once the conditions are satisfied to complete 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	float CompleteTimer = 1.f;

	// The max speed an interactable can be moving before considered for completition
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	float MaxInteractableVelocity = 15.f;

	UFUNCTION(BlueprintCallable)
	void BP_TriggerOverlap(UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	UFUNCTION(BlueprintCallable)
	void BP_EndOverlap(UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);

	UFUNCTION(BlueprintImplementableEvent)
	void On_CompleteChange(bool bHasCompleted);

	void Track(const ASSGInteractable& Interactable);
	void Untrack(const ASSGInteractable& Interactable);

	TArray<TWeakObjectPtr<const ASSGInteractable>> ActiveInteractables;
	float CurrentMass = 0.f;
	float CurrentCompletitonTimer = 0.f;
	bool bIsComplete = false;
};
