// Copyright Epic Games, Inc. All Rights Reserved.

#include "MovableObject.h"

namespace ObjectTypeGameplayTags
{
	const FName MoveableGameplayTag = FName("Gameplay.ObjectType.Movable");
}

// Sets default values for this component's properties
AMovableObject::AMovableObject()
{
	ObjectTypeTag.AddTag(FGameplayTag::RequestGameplayTag(ObjectTypeGameplayTags::MoveableGameplayTag));
}

void AMovableObject::BeginPlay()
{
	if (UPrimitiveComponent* rootPrimitiveComp = Cast<UPrimitiveComponent>(GetRootComponent()))
	{
		rootPrimitiveComp->SetSimulatePhysics(true);
	}
};

void AMovableObject::OnInteract()
{
	UE_LOG(LogTemp, Warning, TEXT("On interact"));

	// Call Blueprint behaviour
	OnInteract_BP();
}