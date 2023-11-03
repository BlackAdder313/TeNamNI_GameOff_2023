// Copyright Epic Games, Inc. All Rights Reserved.

#include "SpyScaleGameGameMode.h"
#include "SpyScaleGameCharacter.h"
#include "UObject/ConstructorHelpers.h"

ASpyScaleGameGameMode::ASpyScaleGameGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

}
