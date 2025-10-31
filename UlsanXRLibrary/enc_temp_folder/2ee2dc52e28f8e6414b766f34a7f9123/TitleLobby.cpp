// Fill out your copyright notice in the Description page of Project Settings.


#include "Mode/UI/TitleLobby.h"
#include <Kismet/GameplayStatics.h>
#include <Mode/PlayPlayerController.h>

void UTitleLobby::OnCreateLobbyClicked()
{
    // LobbyServer.exe 실행
    FString LobbyServerPath = FPaths::ProjectDir() / TEXT("Binaries/Win64/UlsanXRLibraryServer.exe");

    // ?listen 옵션 없이 단순 서버 실행
    FString CommandLine = TEXT("/Game/Level/LobbyLevel -server -log");

    FPlatformProcess::CreateProc(*LobbyServerPath, *CommandLine, true, false, false, nullptr, 0, nullptr, nullptr);

    if (UBaseGameInstance* GI = Cast<UBaseGameInstance>(UGameplayStatics::GetGameInstance(this)))
    {
        FTimerHandle TimerHandle;
        GetWorld()->GetTimerManager().SetTimer(
            TimerHandle,
            FTimerDelegate::CreateLambda([GI]()
        {
            GI->JoinLobbySession();
        }),
            5.0f, // 필요 시 조정
            false
        );
    }

}

void UTitleLobby::OnJoinLobbyClicked()
{
    UBaseGameInstance* GI = Cast<UBaseGameInstance>(UGameplayStatics::GetGameInstance(this));
    if (GI)
    {
        GI->JoinLobbySession();
    }
}