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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	float TriggerMassThreshold = 0.f;

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

	void Track(const ASSGInteractable& Interactable);
	void Untrack(const ASSGInteractable& Interactable);

	TArray<TWeakObjectPtr<const ASSGInteractable>> ActiveInteractables;
	float CurrentMass = 0.f;
};
