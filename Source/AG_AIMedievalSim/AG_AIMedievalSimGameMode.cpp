// Copyright Epic Games, Inc. All Rights Reserved.

#include "AG_AIMedievalSimGameMode.h"
#include "AG_AIMedievalSimCharacter.h"
#include "UObject/ConstructorHelpers.h"

AAG_AIMedievalSimGameMode::AAG_AIMedievalSimGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

}
