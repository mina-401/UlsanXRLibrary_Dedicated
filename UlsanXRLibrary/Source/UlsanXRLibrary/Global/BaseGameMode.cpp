// Fill out your copyright notice in the Description page of Project Settings.


#include "Global/BaseGameMode.h"

void ABaseGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    FTimerHandle H;
    GetWorld()->GetTimerManager().SetTimer(H, FTimerDelegate::CreateLambda([this, NewPlayer]()
    {
        // ���� ����/������ ���� ����
        RestartPlayer(NewPlayer);
    }), 0.15f, false);

    
}