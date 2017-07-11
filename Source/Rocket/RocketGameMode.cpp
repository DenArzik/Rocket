// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "RocketGameMode.h"
#include "RocketHUD.h"
#include "RocketCharacter.h"
#include "UObject/ConstructorHelpers.h"

ARocketGameMode::ARocketGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = ARocketHUD::StaticClass();
}
