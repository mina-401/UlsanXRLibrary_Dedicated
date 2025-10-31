// Fill out your copyright notice in the Description page of Project Settings.


#include "Mode/PlayPlayerController.h"
#include "Global/BaseGameInstance.h"
#include "PlayPlayerController.h"
#include "OnlineSubsystem.h"
#include "Online/OnlineSessionNames.h"
#include "Interfaces/OnlineSessionInterface.h"



//void APlayPlayerController::Server_RequestStartBookTravel_Implementation()
//{
//
//	UBaseGameInstance* BaseGI = Cast<UBaseGameInstance>(GetGameInstance());
//
//	if (BaseGI)
//	{
//		BaseGI->StartBookTravel(this);
//	}
//}

void APlayPlayerController::RequestStartBookTravel()
{
	UBaseGameInstance* BaseGI = Cast<UBaseGameInstance>(GetGameInstance());

	if (BaseGI)
	{
		BaseGI->StartBookTravel(this);
	}
}
void APlayPlayerController::JoinLobby()
{
    //IOnlineSubsystem* OSS = IOnlineSubsystem::Get();
    //if (!OSS) return;

    //SessionInterface = OSS->GetSessionInterface();
    //if (!SessionInterface.IsValid()) return;

    //SearchSettings = MakeShared<FOnlineSessionSearch>();
    //SearchSettings->bIsLanQuery = true;
    //SearchSettings->MaxSearchResults = 20;

    //SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &APlayPlayerController::OnFindSessionsComplete);
    //SessionInterface->FindSessions(0, SearchSettings.ToSharedRef());
}
