// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "TriggeredObject.h"
#include "TriggeredObjectInterface.h"
#include "InteractableObject.generated.h"

// Kostas - Self note: Would use this if we went through the path of having the character register here to fetch
// attributes directly from the object
//DECLARE_EVENT_TwoParams(AInteractableObject, FOnMoveObjectAttributesChanged, const float, const float);


UENUM()
enum class EObjectInteractionType
{
	None,
	Press,
	Proximity,
	Weight
};

UCLASS(BlueprintType, meta = (DisplayName = "Interactable Object"))
class SPYSCALEGAME_API AInteractableObject : public AStaticMeshActor, public ITriggeredObjectInterface
{
	GENERATED_BODY()

public:

	AInteractableObject();

	// AACtor overrides
	virtual void BeginPlay() override;

	//~ Begin UObject Interface
	virtual void PostActorCreated() override;
	//virtual void PostLoad() override;

	// Called when player is trying to interact with this object
	UFUNCTION(BlueprintImplementableEvent, Category = "Object Type|Interface functions")
	void OnInteract_BP();
	
	// Calls both C++ and Blueprint implentations
	void OnInteract();

	UPROPERTY(EditAnywhere, Category = Gameplay)
	EObjectInteractionType ObjectInteractionType = EObjectInteractionType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	FVector MaxObjectScale = FVector(3.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	FVector MinObjectScale = FVector(.5f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	TObjectPtr<class ATriggeredObject> ObjectToNotify;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	float TriggerEnableMassThreshold = 0.f;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	TObjectPtr<class UBoxComponent> TriggerVolume;
	
	void SetupTriggerVolume();

private:
	// overlap begin function
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp,
						AActor* OtherActor,
						UPrimitiveComponent* OtherComp,
						int32 OtherBodyIndex,
						bool bFromSweep,
						const FHitResult& SweepResult);

	// overlap end function
	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp,
					  AActor* OtherActor,
					  UPrimitiveComponent* OtherComp,
					  int32 OtherBodyIndex);
};