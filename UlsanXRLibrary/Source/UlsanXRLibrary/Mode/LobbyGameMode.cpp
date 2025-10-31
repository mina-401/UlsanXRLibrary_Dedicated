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
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("[LobbyGM] World is null"));
		return;
	}

	UBaseGameInstance* GI = Cast<UBaseGameInstance>(World->GetGameInstance());
	if (!GI)
	{
		UE_LOG(LogTemp, Warning, TEXT("[LobbyGM] GameInstance is not UBaseGameInstance"));
		return;
	}

	const int32 MaxPlayers = 6;
	const FString MapName = "LobbyLevel";

	UE_LOG(LogTemp, Log, TEXT("[LobbyGM] Calling HostLobby(Max=%d, Map=%s)"), MaxPlayers, *MapName);
	GI->HostLobby();
}
