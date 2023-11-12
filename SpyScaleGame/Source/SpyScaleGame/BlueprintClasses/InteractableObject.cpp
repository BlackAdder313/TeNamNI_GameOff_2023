// Copyright Epic Games, Inc. All Rights Reserved.

#include "InteractableObject.h"


void AInteractableObject::BeginPlay()
{
	if (UPrimitiveComponent* rootPrimitiveComp = Cast<UPrimitiveComponent>(GetRootComponent()))
	{
		rootPrimitiveComp->SetSimulatePhysics(true);
	}
};

void AInteractableObject::OnScan()
{ 
	UE_LOG(LogTemp, Warning, TEXT("On scan"));

	// Call Blueprint behaviour
	OnScan_BP();
}

void AInteractableObject::OnInteract()
{
	UE_LOG(LogTemp, Warning, TEXT("On interact"));

	// Call Blueprint behaviour
	OnInteract_BP();
}