// Fill out your copyright notice in the Description page of Project Settings.


#include "Mode/LobbyGameMode.h"
#include "OnlineSubsystem.h"
#include "Online/OnlineSessionNames.h"
#include "Interfaces/OnlineSessionInterface.h"
#include <Global/BaseGameInstance.h>

ALobbyGameMode::ALobbyGameMode()
{
	PlayerRoom = EPlayerRoom::Lobby;

	//PlayerControllerClass = APlayPlayerController::StaticClass();
}

void ALobbyGameMode::BeginPlay()
{
    Super::BeginPlay();

    if (IsRunningDedicatedServer())
    {
        StartLobbySession();
    }
}


void ALobbyGameMode::StartLobbySession()
{
    if (bLobbySessionStarted)
    {
        UE_LOG(LogTemp, Log, TEXT("[LobbyGM] Lobby session already started (guard)"));
        return;
    }

    UWorld* World = GetWorld();
    if (!World) { UE_LOG(LogTemp, Warning, TEXT("[LobbyGM] World is null")); return; }

    UBaseGameInstance* GI = Cast<UBaseGameInstance>(World->GetGameInstance());
    if (!GI) { UE_LOG(LogTemp, Warning, TEXT("[LobbyGM] GameInstance is not UBaseGameInstance")); return; }

    // 기존 세션 존재 여부 확인
    IOnlineSubsystem* OSS = IOnlineSubsystem::Get();
    IOnlineSessionPtr SI = OSS ? OSS->GetSessionInterface() : nullptr;
    if (SI.IsValid() && SI->GetNamedSession(FName("LobbySession")) != nullptr)
    {
        UE_LOG(LogTemp, Log, TEXT("[LobbyGM] LobbySession already exists on OSS"));
        bLobbySessionStarted = true;
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("[LobbyGM] Calling HostLobby()"));
    bLobbySessionStarted = true;
    GI->HostLobby();
}
