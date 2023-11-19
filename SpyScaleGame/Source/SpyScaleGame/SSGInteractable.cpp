// Fill out your copyright notice in the Description page of Project Settings.


#include "SSGInteractable.h"

// Sets default values
ASSGInteractable::ASSGInteractable()
{
	PrimaryActorTick.bCanEverTick = true;

	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	StaticMeshComponent->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
	StaticMeshComponent->Mobility = EComponentMobility::Movable;
	StaticMeshComponent->SetSimulatePhysics(true);
	StaticMeshComponent->SetGenerateOverlapEvents(true);
	StaticMeshComponent->bUseDefaultCollision = true;

	RootComponent = StaticMeshComponent;
}

// Called when the game starts or when spawned
void ASSGInteractable::BeginPlay()
{
	Super::BeginPlay();
	
	OriginalScale = StaticMeshComponent->GetRelativeScale3D();
	OriginalMassScale = StaticMeshComponent->GetMassScale();
}

void ASSGInteractable::AdjustScale(float Direction)
{
	CurrentScaleAdjustment += Direction * ScaleAdjustmentSpeed;
	CurrentScaleAdjustment = FMath::Clamp(CurrentScaleAdjustment, ScaleAdjustmentInterval.Min, ScaleAdjustmentInterval.Max);

	const FVector Adjustment = CurrentScaleAdjustment * OriginalScale.GetSafeNormal();

	StaticMeshComponent->SetRelativeScale3D(OriginalScale + Adjustment);
}

// Called every frame
void ASSGInteractable::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

