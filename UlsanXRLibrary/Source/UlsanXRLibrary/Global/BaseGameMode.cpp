// Fill out your copyright notice in the Description page of Project Settings.


#include "Global/BaseGameMode.h"

void ABaseGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);
    RestartPlayer(NewPlayer);
}