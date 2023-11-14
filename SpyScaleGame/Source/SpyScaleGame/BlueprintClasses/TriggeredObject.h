// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "TriggeredObjectInterface.h"
#include "TriggeredObject.generated.h"

// Should be assigned to objects that aren't interactable,
// but can accept be notified by triggers, like doors
// The interface function should be implemented on BP side

UCLASS(BlueprintType, meta = (DisplayName = "Triggered Object"))
class SPYSCALEGAME_API ATriggeredObject : public AStaticMeshActor, public ITriggeredObjectInterface
{
	GENERATED_BODY()
};