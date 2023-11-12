// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/TriggerVolume.h"
#include "GameplayTagContainer.h"
#include "InteractableObject.generated.h"

// TODO: Would use this if we went through the path of having the character register here to fetch
// attributes directly from the object
//DECLARE_EVENT_TwoParams(AInteractableObject, FOnMoveObjectAttributesChanged, const float, const float);

UCLASS(BlueprintType, meta = (DisplayName = "Interactable Object"))
class SPYSCALEGAME_API AInteractableObject : public ATriggerVolume
{
	GENERATED_BODY()

public:
	// AACtor overrides
	virtual void BeginPlay() override;

	// Called when player is going over this object with gun
	UFUNCTION(BlueprintImplementableEvent, Category = "Object Type|Interface functions")
	void OnScan_BP();
	
	// Calls both C++ and Blueprint implentations
	void OnScan();

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