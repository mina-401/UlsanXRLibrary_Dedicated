// Fill out your copyright notice in the Description page of Project Settings.


#include "Mode/UI/PlayTestBook.h"
#include <Kismet/GameplayStatics.h>

void UPlayTestBook::OnCreateGameClicked()
{
    // LobbyServer.exe ����
    FString PlayServerPath = FPaths::ProjectDir() / TEXT("Binaries/Win64/UlsanXRLibraryServer.exe");

    // ?listen �ɼ� ���� �ܼ� ���� ����
    FString CommandLine = TEXT("/Game/Level/PlayLevel -server -log");

    const FProcHandle Handle=FPlatformProcess::CreateProc(*PlayServerPath, *CommandLine, true, false, false, nullptr, 0, nullptr, nullptr);

    if (!Handle.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("[PlayTestBook] Failed to launch: %s %s"), *PlayServerPath, *CommandLine);
        return;
    }

    if (UBaseGameInstance* GI = Cast<UBaseGameInstance>(UGameplayStatics::GetGameInstance(this)))
    {
        FTimerHandle TimerHandle;
        GetWorld()->GetTimerManager().SetTimer(
            TimerHandle,
            FTimerDelegate::CreateLambda([GI]()
        {
            GI->JoinLobby(TEXT("PlayLevel"));
        }),
            5.0f, // �ʿ� �� ����
            false
        );
    }

}

void UPlayTestBook::OnJoinGameClicked()
{
    UBaseGameInstance* GI = Cast<UBaseGameInstance>(UGameplayStatics::GetGameInstance(this));
    if (GI)
    {
        GI->JoinLobby(TEXT("PlayLevel"));
    }
}