// Fill out your copyright notice in the Description page of Project Settings.


#include "Global/BaseGameMode.h"

void ABaseGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    FTimerHandle H;
    GetWorld()->GetTimerManager().SetTimer(H, FTimerDelegate::CreateLambda([this, NewPlayer]()
    {
        // 기존 스폰/포제션 로직 실행
        RestartPlayer(NewPlayer);
    }), 0.15f, false);

    
}