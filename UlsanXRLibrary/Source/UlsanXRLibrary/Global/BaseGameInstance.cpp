// BaseGameInstance.cpp
#include "Global/BaseGameInstance.h"
#include "Engine/Engine.h"
#include "Engine/NetDriver.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "OnlineSubsystem.h"
#include "Online/OnlineSessionNames.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "GameFramework/PlayerController.h"
#include "BaseGameInstance.h"

#define SESS_LOG(Verbosity, Format, ...) UE_LOG(LogTemp, Verbosity, TEXT("[SESS] %s: " Format), TEXT(__FUNCTION__), ##__VA_ARGS__)

UBaseGameInstance::UBaseGameInstance()
{
}



// ==================== 세션 생성 ====================

void UBaseGameInstance::HostLobby(const FString& SessionType)
{
    if (!SessionInterface.IsValid())
    {
        SESS_LOG(Error, "SessionInterface invalid!");
        return;
    }

    const FName LocalSessionName(*SessionType);
    FNamedOnlineSession* ExistingSession = SessionInterface->GetNamedSession(LocalSessionName);
    if (ExistingSession)
    {
        SESS_LOG(Log, "Destroying existing session...");
        SessionInterface->DestroySession(LocalSessionName);
    }

    FOnlineSessionSettings Settings;

    IOnlineSubsystem* OSS = IOnlineSubsystem::Get();
    FName SubsystemName = OSS ? OSS->GetSubsystemName() : NAME_None;


    Settings.NumPublicConnections = 4;
    Settings.bShouldAdvertise = true;
    Settings.bAllowJoinInProgress = true;
    Settings.bAllowJoinViaPresence = true;

    Settings.Set(FName("SessionType"), SessionType, EOnlineDataAdvertisementType::ViaOnlineService);
    SessionInterface->CreateSession(0, LocalSessionName, Settings);
}
// ==================== 세션 찾기 ====================

void UBaseGameInstance::JoinLobby(const FString& SessionType)
{
    IOnlineSubsystem* OSS = IOnlineSubsystem::Get();
    if (!OSS)
    {
        SESS_LOG(Error, "OSS not available!");
        return;
    }

    SessionInterface = OSS->GetSessionInterface();
    if (!SessionInterface.IsValid())
    {
        SESS_LOG(Error, "SessionInterface invalid!");
        return;
    }

    WantedSessionType = SessionType;

    SessionSearch = MakeShareable(new FOnlineSessionSearch());

    FName SubsystemName = OSS->GetSubsystemName();
    if (SubsystemName == "EOS")
    {
        SessionSearch->bIsLanQuery = false;
        SESS_LOG(Log, "Searching EOS sessions...");
    }
    else
    {
        SessionSearch->bIsLanQuery = true;
        SESS_LOG(Log, "Searching LAN sessions...");
    }

    SessionSearch->MaxSearchResults = 10;
    SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

    SESS_LOG(Log, "Looking for SessionType: %s", *WantedSessionType);
    SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());
}