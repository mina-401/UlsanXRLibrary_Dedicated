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

#define SESS_LOG(Verbosity, Format, ...) UE_LOG(LogTemp, Verbosity, TEXT("[SESS] %s: " Format), TEXT(__FUNCTION__), ##__VA_ARGS__)

UBaseGameInstance::UBaseGameInstance()
{
}

void UBaseGameInstance::Init()
{
    Super::Init();

    IOnlineSubsystem* OSS = IOnlineSubsystem::Get();
    if (OSS)
    {
        FName SubsystemName = OSS->GetSubsystemName();
        SESS_LOG(Log, "OnlineSubsystem: %s", *SubsystemName.ToString());

        SessionInterface = OSS->GetSessionInterface();
        if (SessionInterface.IsValid())
        {
            SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UBaseGameInstance::OnCreateSessionComplete);
            SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UBaseGameInstance::OnFindSessionsComplete);
            SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UBaseGameInstance::OnJoinSessionComplete);

            SESS_LOG(Log, "SessionInterface initialized");
        }
    }

    // ⭐ 클라이언트만 자동 로그인 (서버는 제외!)
    UWorld* World = GetWorld();
    if (World)
    {
        ENetMode NetMode = World->GetNetMode();

        // 클라이언트 또는 스탠드얼론만 로그인
        if (NetMode == NM_Client || NetMode == NM_Standalone)
        {
            SESS_LOG(Log, "CLIENT - attempting guest login...");

            FTimerHandle LoginTimer;
            World->GetTimerManager().SetTimer(
                LoginTimer,
                [this]()
            {
                AutoLoginWithEOSConnect();
            },
                0.5f,
                false
            );
        }
        else if (NetMode == NM_DedicatedServer || NetMode == NM_ListenServer)
        {
            SESS_LOG(Log, "SERVER (NetMode: %d) - skipping login", (int32)NetMode);
            // ⭐ 서버는 로그인 성공으로 간주
            OnLoginSuccess();
        }
    }
}

// ⭐ EOS Connect 로그인 (Device ID)
void UBaseGameInstance::AutoLoginWithEOSConnect()
{
    IOnlineSubsystem* OSS = IOnlineSubsystem::Get();
    if (!OSS)
    {
        SESS_LOG(Error, "OnlineSubsystem not available!");
        return;
    }

    FName SubsystemName = OSS->GetSubsystemName();
    SESS_LOG(Log, "Auto-login attempt for OSS: %s", *SubsystemName.ToString());

    if (SubsystemName != "EOS")
    {
        SESS_LOG(Log, "Not EOS subsystem - auto-login skipped");
        return;
    }

    IOnlineIdentityPtr Identity = OSS->GetIdentityInterface();
    if (!Identity.IsValid())
    {
        SESS_LOG(Error, "IdentityInterface invalid!");

        FTimerHandle RetryTimer;
        GetWorld()->GetTimerManager().SetTimer(
            RetryTimer,
            [this]()
        {
            SESS_LOG(Log, "Retrying auto-login...");
            AutoLoginWithEOSConnect();
        },
            1.0f,
            false
        );
        return;
    }

    // 이미 로그인되어 있는지 확인
    ELoginStatus::Type LoginStatus = Identity->GetLoginStatus(0);
    if (LoginStatus == ELoginStatus::LoggedIn)
    {
        TSharedPtr<const FUniqueNetId> UserId = Identity->GetUniquePlayerId(0);
        if (UserId.IsValid())
        {
            SESS_LOG(Log, "✅ Already logged in! UserId: %s", *UserId->ToString());
            OnLoginSuccess();
            return;
        }
    }

    // ⭐ persistentauth 사용 (EOS Connect)
    FOnlineAccountCredentials Credentials;
    Credentials.Type = TEXT("persistentauth");
    Credentials.Id = TEXT("");
    Credentials.Token = TEXT("");

    SESS_LOG(Log, "Starting EOS Connect login (persistentauth)...");

    // 델리게이트 바인딩
    OnLoginCompleteDelegateHandle = Identity->AddOnLoginCompleteDelegate_Handle(
        0,
        FOnLoginCompleteDelegate::CreateUObject(this, &UBaseGameInstance::OnEOSConnectLoginComplete)
    );

    // 로그인 시작
    bool bLoginStarted = Identity->Login(0, Credentials);

    if (bLoginStarted)
    {
        SESS_LOG(Log, "✅ EOS Connect login started");
    }
    else
    {
        SESS_LOG(Error, "❌ Failed to start EOS Connect login!");
    }
}

void UBaseGameInstance::OnEOSConnectLoginComplete(
    int32 LocalUserNum,
    bool bWasSuccessful,
    const FUniqueNetId& UserId,
    const FString& Error
)
{
    IOnlineSubsystem* OSS = IOnlineSubsystem::Get();
    if (OSS)
    {
        IOnlineIdentityPtr Identity = OSS->GetIdentityInterface();
        if (Identity.IsValid())
        {
            Identity->ClearOnLoginCompleteDelegate_Handle(0, OnLoginCompleteDelegateHandle);
        }
    }

    if (bWasSuccessful)
    {
        SESS_LOG(Log, "✅ EOS Connect login successful!");
        SESS_LOG(Log, "   ProductUserId: %s", *UserId.ToString());
        SESS_LOG(Log, "   LocalUserNum: %d", LocalUserNum);

        OnLoginSuccess();
    }
    else
    {
        SESS_LOG(Error, "❌ EOS Connect login failed!");
        SESS_LOG(Error, "   Error: %s", *Error);

        // 실패 처리 (재시도하지 말 것!)
        OnLoginFailed(Error);
    }
}

void UBaseGameInstance::OnLoginSuccess()
{
    SESS_LOG(Log, "Guest login complete - ready to create/join sessions!");

    // 블루프린트 이벤트
    OnLoginSuccessEvent.Broadcast();
}

void UBaseGameInstance::OnLoginFailed(const FString& ErrorMessage)
{
    SESS_LOG(Error, "Login failed: %s", *ErrorMessage);

    // 블루프린트 이벤트
    OnLoginFailedEvent.Broadcast(ErrorMessage);
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

    // ⭐ 서버인지 확인
    UWorld* World = GetWorld();
    bool bIsServer = World && (World->GetNetMode() == NM_DedicatedServer || World->GetNetMode() == NM_ListenServer);

    if (SubsystemName == "EOS")
    {
        SESS_LOG(Log, "Creating EOS session with Voice...");
        Settings.bIsLANMatch = false;
        Settings.bUsesPresence = false;
        Settings.bUseLobbiesIfAvailable = true;

        // ⭐ Voice 활성화
        Settings.Set(FName("EOSVoiceChat_Enabled"), true, EOnlineDataAdvertisementType::ViaOnlineService);

        // ⭐ 서버인 경우 추가 설정
        if (bIsServer)
        {
            Settings.bIsDedicated = true;
            Settings.bUsesPresence = false;
            Settings.bAllowJoinInProgress = true;
            SESS_LOG(Log, "Dedicated Server mode - configured for hosting");
        }
    }
    else
    {
        SESS_LOG(Log, "Creating LAN session...");
        Settings.bIsLANMatch = true;
        Settings.bUsesPresence = true;
        Settings.bUseLobbiesVoiceChatIfAvailable = true;
    }

    Settings.NumPublicConnections = 4;
    Settings.bShouldAdvertise = true;
    Settings.bAllowJoinInProgress = true;
    Settings.bAllowJoinViaPresence = !bIsServer;  // 서버는 Presence 사용 안 함

    Settings.Set(FName("SessionType"), SessionType, EOnlineDataAdvertisementType::ViaOnlineService);

    SESS_LOG(Log, "Creating Voice-Enabled Session: %s (Server: %s)",
        *LocalSessionName.ToString(),
        bIsServer ? TEXT("YES") : TEXT("NO"));

    // ⭐ 서버인 경우 LocalUserId를 0이 아닌 값으로 설정
    int32 LocalUserNum = 0;

    SessionInterface->CreateSession(LocalUserNum, LocalSessionName, Settings);
}

void UBaseGameInstance::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
    if (bWasSuccessful)
    {
        SESS_LOG(Log, "✅ Session created: %s", *SessionName.ToString());

        FNamedOnlineSession* Session = SessionInterface->GetNamedSession(SessionName);
        if (Session)
        {
            bool bVoiceEnabled = false;
            Session->SessionSettings.Get(FName("EOSVoiceChat_Enabled"), bVoiceEnabled);

            SESS_LOG(Log, "Voice Enabled: %s", bVoiceEnabled ? TEXT("YES") : TEXT("NO"));
        }
    }
    else
    {
        SESS_LOG(Error, "❌ Failed to create session: %s", *SessionName.ToString());
    }
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

void UBaseGameInstance::OnFindSessionsComplete(bool bWasSuccessful)
{
    if (!bWasSuccessful || !SessionSearch.IsValid())
    {
        SESS_LOG(Error, "Find sessions failed!");
        return;
    }

    const FString WantedType = WantedSessionType.IsEmpty() ? TEXT("LobbySession") : WantedSessionType;

    SESS_LOG(Log, "Found %d sessions", SessionSearch->SearchResults.Num());

    for (const FOnlineSessionSearchResult& Result : SessionSearch->SearchResults)
    {
        FString FoundType;
        Result.Session.SessionSettings.Get(FName("SessionType"), FoundType);

        if (FoundType == WantedType)
        {
            const FName LocalSessionName(*WantedType);
            SESS_LOG(Log, "✅ Joining session: %s", *LocalSessionName.ToString());

            SessionInterface->JoinSession(0, LocalSessionName, Result);
            WantedSessionType.Empty();
            return;
        }
    }

    SESS_LOG(Warning, "No matching session found for type: %s", *WantedType);
}

void UBaseGameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
    if (!SessionInterface.IsValid())
    {
        SESS_LOG(Error, "SessionInterface invalid!");
        return;
    }

    if (Result != EOnJoinSessionCompleteResult::Success)
    {
        SESS_LOG(Error, "❌ Join session failed! Result: %d", (int32)Result);
        return;
    }

    FString TravelURL;
    if (SessionInterface->GetResolvedConnectString(SessionName, TravelURL))
    {
        SESS_LOG(Log, "✅ Join successful! Traveling to: %s", *TravelURL);

        APlayerController* PC = GetFirstLocalPlayerController();
        if (PC)
        {
            PC->ClientTravel(TravelURL, TRAVEL_Absolute);
        }
    }
    else
    {
        SESS_LOG(Error, "Failed to get connect string");
    }
}