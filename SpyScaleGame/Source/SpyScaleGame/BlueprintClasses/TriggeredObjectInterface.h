// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "UObject/Interface.h"

#include "TriggeredObjectInterface.generated.h"

UINTERFACE(Blueprintable)
class SPYSCALEGAME_API UTriggeredObjectInterface : public UInterface
{
	GENERATED_BODY()
};

class SPYSCALEGAME_API ITriggeredObjectInterface
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Trigger Communication")
	void OnTriggerEnabled();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Trigger Communication")
	void OnTriggerDisabled();
};

#if UE_ENABLE_INCLUDE_ORDER_DEPRECATED_IN_5_2
#include "CoreMinimal.h"
#endif