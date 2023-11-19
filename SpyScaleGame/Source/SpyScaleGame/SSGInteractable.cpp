// Fill out your copyright notice in the Description page of Project Settings.


#include "SSGInteractable.h"

// Sets default values
ASSGInteractable::ASSGInteractable()
{
	PrimaryActorTick.bCanEverTick = true;

	GetStaticMeshComponent()->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
	GetStaticMeshComponent()->Mobility = EComponentMobility::Movable;
	GetStaticMeshComponent()->SetSimulatePhysics(true);
	GetStaticMeshComponent()->SetGenerateOverlapEvents(true);
	GetStaticMeshComponent()->bUseDefaultCollision = true;
}

// Called when the game starts or when spawned
void ASSGInteractable::BeginPlay()
{
	Super::BeginPlay();
	
	OriginalScale = GetStaticMeshComponent()->GetRelativeScale3D();
	OriginalMassScale = GetStaticMeshComponent()->GetMassScale();
}

void ASSGInteractable::AdjustScale(float Direction)
{
	CurrentScaleAdjustment += Direction * ScaleAdjustmentSpeed;
	CurrentScaleAdjustment = FMath::Clamp(CurrentScaleAdjustment, ScaleAdjustmentInterval.Min, ScaleAdjustmentInterval.Max);

	const FVector Adjustment = CurrentScaleAdjustment * OriginalScale.GetSafeNormal();

	GetStaticMeshComponent()->SetRelativeScale3D(OriginalScale + Adjustment);
}

// Called every frame
void ASSGInteractable::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

