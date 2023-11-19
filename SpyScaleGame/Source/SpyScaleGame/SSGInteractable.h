// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SSGInteractable.generated.h"


UCLASS(Blueprintable)
class SPYSCALEGAME_API ASSGInteractable : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASSGInteractable();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	void AdjustScale(float Direction);

	UStaticMeshComponent* GetStaticMesh() const { return StaticMeshComponent;  }
	float GetMass() const { return StaticMeshComponent->GetMass(); }

protected:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<class UStaticMeshComponent> StaticMeshComponent;

	UPROPERTY(EditAnywhere, Category = Gameplay)
	float ScaleAdjustmentSpeed = 1.f;

	UPROPERTY(EditAnywhere, Category = Gameplay)
	FFloatInterval ScaleAdjustmentInterval = FFloatInterval(-2.f, 2.f);

	FVector OriginalScale = FVector::ZeroVector;
	float OriginalMassScale = 1.f;

	float CurrentScaleAdjustment = 1.f;
};
