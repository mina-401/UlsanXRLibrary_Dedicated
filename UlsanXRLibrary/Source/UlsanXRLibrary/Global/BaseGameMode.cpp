// Fill out your copyright notice in the Description page of Project Settings.


#include "Global/BaseGameMode.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Global/BasePlayerController.h"
#include "BaseGameMode.h"

#define VOICE_LOG(GameMode, Format, ...) UE_LOG(LogTemp, GameMode, TEXT("[VOICE] %s: " Format), TEXT(__FUNCTION__), ##__VA_ARGS__)
void ABaseGameMode::BeginPlay()
{
	Super::BeginPlay();
}

