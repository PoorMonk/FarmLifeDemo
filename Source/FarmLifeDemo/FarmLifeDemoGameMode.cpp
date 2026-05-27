// Copyright Epic Games, Inc. All Rights Reserved.

#include "FarmLifeDemoGameMode.h"
#include "FarmLifeDemoCharacter.h"
#include "UObject/ConstructorHelpers.h"

AFarmLifeDemoGameMode::AFarmLifeDemoGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
