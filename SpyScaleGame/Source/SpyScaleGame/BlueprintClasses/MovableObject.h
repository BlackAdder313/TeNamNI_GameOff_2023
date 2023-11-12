// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "GameplayTagContainer.h"
#include "MovableObject.generated.h"

namespace ObjectTypeGameplayTags
{
	extern const FName MoveableObjectTypeGameplayTag;
}

// TODO: Would use this if we went through the path of having the character register here to fetch
// attributes directly from the object
//DECLARE_EVENT_TwoParams(AMovableObject, FOnMoveObjectAttributesChanged, const float, const float);

UCLASS(BlueprintType, meta = (DisplayName = "Movable Object"))
class SPYSCALEGAME_API AMovableObject : public AStaticMeshActor
{
	GENERATED_BODY()

public:
	/** Sets default values for this component's properties */
	AMovableObject();

	// AACtor overrides
	virtual void BeginPlay() override;

	// Called when player is trying to interact with this object
	UFUNCTION(BlueprintImplementableEvent, Category = "Object Type|Interface functions")
	void OnInteract_BP();
	
	// Calls both C++ and Blueprint implentations
	void OnInteract();

	UPROPERTY(VisibleAnywhere, Category = Gameplay)
	FGameplayTagContainer ObjectTypeTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	FVector MaxObjectScale = FVector(3.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	FVector MinObjectScale = FVector(.5f);
};