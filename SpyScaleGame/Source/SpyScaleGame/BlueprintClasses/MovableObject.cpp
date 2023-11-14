// Copyright Epic Games, Inc. All Rights Reserved.

#include "MovableObject.h"

// TODO: Ensure that scaling is also affecting the mass

void AMovableObject::BeginPlay()
{
	if (UPrimitiveComponent* rootPrimitiveComp = Cast<UPrimitiveComponent>(GetRootComponent()))
	{
		rootPrimitiveComp->SetSimulatePhysics(true);
	}
}

void AMovableObject::OnInteract()
{
	UE_LOG(LogTemp, Warning, TEXT("On interact"));

	// Call Blueprint behaviour
	OnInteract_BP();
}