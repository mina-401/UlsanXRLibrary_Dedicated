// Fill out your copyright notice in the Description page of Project Settings.

#include "Global/BaseGameInstance.h"
#include "Engine/Engine.h"
#include "Engine/NetDriver.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

#include "OnlineSubsystem.h"
#include "Online/OnlineSessionNames.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "GameFramework/PlayerController.h"

#include "Modules/ModuleManager.h"
#include <ULXRConst.h>
#include "Misc/Paths.h"
#include "HAL/PlatformProcess.h"
#include "Global/BasePlayerController.h"



#define SESS_LOG(Verbosity, Format, ...) UE_LOG(LogTemp, Verbosity, TEXT("[SESS] %s: " Format), TEXT(__FUNCTION__), ##__VA_ARGS__)

UBaseGameInstance::UBaseGameInstance()
{
}

// BaseGameInstance.cpp
void UBaseGameInstance::Init()
{
    IOnlineSubsystem* OSS = IOnlineSubsystem::Get();
    if (OSS)
    {
        SessionInterface = OSS->GetSessionInterface();
        if (SessionInterface.IsValid())
        {
            SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UBaseGameInstance::OnCreateSessionComplete);
            SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UBaseGameInstance::OnFindSessionsComplete);
            SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UBaseGameInstance::OnJoinSessionComplete);
        }
    }
}

void UBaseGameInstance::HostLobby()
{
    if (!SessionInterface.IsValid()) return;

    FOnlineSessionSettings Settings;
    Settings.bIsLANMatch = true;
    Settings.NumPublicConnections = 4;
    Settings.bShouldAdvertise = true;
    Settings.bUsesPresence = true;
    Settings.Set(FName("SessionType"), FString("LobbySession"), EOnlineDataAdvertisementType::ViaOnlineService);


  //  SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UBaseGameInstance::OnCreateSessionComplete);

    SessionInterface->CreateSession(0, FName("LobbySession"), Settings);
}


void UBaseGameInstance::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
    if (bWasSuccessful)
    {
        
    }
}

void UBaseGameInstance::JoinLobby()
{
    if (!SessionInterface.IsValid()) return;

    SessionSearch = MakeShared<FOnlineSessionSearch>();
    SessionSearch->bIsLanQuery = true;
    SessionSearch->MaxSearchResults = 10;

   // SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UBaseGameInstance::OnFindSessionsComplete);
    SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());
}
void UBaseGameInstance::JoinLobbySession()
{
    IOnlineSubsystem* OSS = IOnlineSubsystem::Get();
    if (!OSS) return;

    SessionInterface = OSS->GetSessionInterface();
    if (!SessionInterface.IsValid()) return;

    SessionSearch = MakeShareable(new FOnlineSessionSearch());
    SessionSearch->bIsLanQuery = true;
    SessionSearch->MaxSearchResults = 10;
    SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

    //SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UBaseGameInstance::OnFindSessionsComplete);
    SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());
}

void UBaseGameInstance::OnFindSessionsComplete(bool bWasSuccessful)
{
    if (!bWasSuccessful || !SessionSearch.IsValid()) return;

    for (const FOnlineSessionSearchResult& Result : SessionSearch->SearchResults)
    {
        FString FoundName;
        if (Result.Session.SessionSettings.Get(FName("SessionType"), FoundName) && FoundName == "LobbySession")
        {
            SessionInterface->JoinSession(0, FName("LobbySession"), Result);
            break;
        }
    }
}
void UBaseGameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
    if (!SessionInterface.IsValid()) return;

    FString TravelURL;
    if (SessionInterface->GetResolvedConnectString(SessionName, TravelURL))
    {
        APlayerController* PC = GetFirstLocalPlayerController();
        if (PC)
        {
            PC->ClientTravel(TravelURL, TRAVEL_Absolute);
        }
    }
}
void UBaseGameInstance::StartServer(FString& _IP, FString& _Port)
{
}

void UBaseGameInstance::Connect(const FString& _IP, const FString& _Port)
{
}
