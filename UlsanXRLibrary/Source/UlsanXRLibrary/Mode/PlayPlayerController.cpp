// Fill out your copyright notice in the Description page of Project Settings.


#include "Mode/PlayPlayerController.h"
#include "Global/BaseGameInstance.h"
#include "PlayPlayerController.h"
#include "OnlineSubsystem.h"
#include "Online/OnlineSessionNames.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "LobbyGameMode.h"


void APlayPlayerController::ReturnTravel()
{
	UBaseGameInstance* BaseGI = Cast<UBaseGameInstance>(GetGameInstance());

	if (BaseGI)
	{
        BaseGI->JoinLobby(TEXT("LobbyLevel"));
	}
}


void APlayPlayerController::OnClickHostLobby()
{
    UBaseGameInstance* GI = Cast<UBaseGameInstance>(GetGameInstance());
    if (GI)
    {
        GI->HostLobby(TEXT("Lobby"));

        UE_LOG(LogTemp, Log, TEXT("Hosting Voice-Enabled Lobby..."));
    }
}

void APlayPlayerController::OnClickJoinLobby()
{
    UBaseGameInstance* GI = Cast<UBaseGameInstance>(GetGameInstance());
    if (GI)
    {
        GI->JoinLobby(TEXT("Lobby"));
        UE_LOG(LogTemp, Log, TEXT("Searching for Voice-Enabled Lobby..."));
    }
}