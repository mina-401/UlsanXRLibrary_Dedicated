// Fill out your copyright notice in the Description page of Project Settings.


#include "Mode/LobbyGameMode.h"
#include "OnlineSubsystem.h"
#include "Online/OnlineSessionNames.h"
#include "Interfaces/OnlineSessionInterface.h"

ALobbyGameMode::ALobbyGameMode()
{
	PlayerRoom = EPlayerRoom::Lobby;

	//PlayerControllerClass = APlayPlayerController::StaticClass();
}
void ALobbyGameMode::StartLobbySession()
{
    IOnlineSubsystem* OSS = IOnlineSubsystem::Get();
    if (OSS)
    {
        IOnlineSessionPtr SessionInterface = OSS->GetSessionInterface();
        if (SessionInterface.IsValid())
        {
            FOnlineSessionSettings SessionSettings;
            SessionSettings.bIsLANMatch = true;
            SessionSettings.NumPublicConnections = 16;
            SessionSettings.bShouldAdvertise = true;
            SessionSettings.bUsesPresence = true;
            SessionInterface->CreateSession(0, "LobbySession", SessionSettings);
        }
    }
}
