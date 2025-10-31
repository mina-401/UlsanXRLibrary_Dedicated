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

FString UBaseGameInstance::GetPlayWorldLevel()
{
	return TEXT("");
}

UBaseGameInstance::UBaseGameInstance()
{
	// DataTables 로딩(생략/주석 유지)
	{
		FString DataPath = UULXRConst::Path::GlobalDataTablePath;
		ConstructorHelpers::FObjectFinder<UDataTable> FinderDataTables(*DataPath);
		if (FinderDataTables.Succeeded())
		{
			DataTables = FinderDataTables.Object;
		}
	}
	{
		// 필요 시 데이터 테이블 로딩 로직 사용 (현재는 비활성화)
// FString DataPath = UULXRConst::Path::GlobalDataTablePath;
// ConstructorHelpers::FObjectFinder<UDataTable> FinderDataTables(*DataPath);
// if (FinderDataTables.Succeeded())
// {
// 	DataTables = FinderDataTables.Object;
// }
// if (nullptr != DataTables)
// {
// 	LevelDataTable = DataTables->FindRow<FDataTableRow>("DT_LevelDataTable", nullptr)->Resources;
// 	BookItemDataTable = DataTables->FindRow<FDataTableRow>("DT_BookItemDataTable", nullptr)->Resources;
// 	ActorDataTable = DataTables->FindRow<FDataTableRow>("DT_GlobalActorDataTable", nullptr)->Resources;
// }
	}
}

void UBaseGameInstance::Init()
{
	Super::Init();

	SESS_LOG(Log, "Init called");

	IOnlineSubsystem* OSS = IOnlineSubsystem::Get();
	if (OSS)
	{
		SessionInterface = OSS->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			SESS_LOG(Log, "OnlineSubsystem found and SessionInterface valid");
			OnCreateSessionCompleteDelegate = FOnCreateSessionCompleteDelegate::CreateUObject(this, &UBaseGameInstance::OnCreateSessionComplete);
			OnFindSessionsCompleteDelegate = FOnFindSessionsCompleteDelegate::CreateUObject(this, &UBaseGameInstance::OnFindSessionsComplete);
			OnJoinSessionCompleteDelegate = FOnJoinSessionCompleteDelegate::CreateUObject(this, &UBaseGameInstance::OnJoinSessionComplete);
		}
		else
		{
			SESS_LOG(Warning, "OnlineSubsystem present but SessionInterface invalid");
		}
	}
	else
	{
		SESS_LOG(Warning, "No OnlineSubsystem found (OSS == nullptr). For LAN this is fine if configured as NULL OSS.");
	}

	 //Dedicated 전용 자동 세션 생성 바인딩
	/*if (IsRunningDedicatedServer())
	{
		const TCHAR* Cmd = FCommandLine::Get();
		if (FParse::Param(Cmd, TEXT("AutoCreateSession")))
		{
			SESS_LOG(Log, "AutoCreateSession flag detected -> Will create session after map load");
			FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this,&UBaseGameInstance::HandleAutoCreateAfterLoad);
		}
		else
		{
			SESS_LOG(Log, "Dedicated server running but AutoCreateSession flag NOT present");
		}
	}*/
}

void UBaseGameInstance::HandleAutoCreateAfterLoad(UWorld* LoadedWorld)
{
	SESS_LOG(Log, "HandleAutoCreateAfterLoad invoked");

	if (!IsRunningDedicatedServer())
	{
		SESS_LOG(Warning, "Not a dedicated server -> abort");
		return;
	}
	if (!LoadedWorld)
	{
		SESS_LOG(Warning, "LoadedWorld is null -> abort");
		return;
	}
	if (bDedicatedSessionCreated)
	{
		SESS_LOG(Log, "Session already created earlier -> abort");
		return;
	}
	if (!SessionInterface.IsValid())
	{
		SESS_LOG(Warning, "SessionInterface invalid -> abort");
		return;
	}

	// Parse commandline args
	const TCHAR* Cmd = FCommandLine::Get();
	FString MapArg;
	int32 MaxArg = 6;
	float DelayArg = 1.0f;

	FParse::Value(Cmd, TEXT("SessionMap="), MapArg);
	FParse::Value(Cmd, TEXT("SessionMax="), MaxArg);
	FParse::Value(Cmd, TEXT("SessionDelay="), DelayArg);

	if (MapArg.IsEmpty())
	{
		MapArg = UGameplayStatics::GetCurrentLevelName(LoadedWorld, true);
	}

	SESS_LOG(Log, "AutoCreate params: Map=%s, Max=%d, Delay=%.2f", *MapArg, MaxArg, DelayArg);

	// Delay a bit to ensure world and subsystems fully ready
	FTimerHandle H;
	FTimerDelegate D = FTimerDelegate::CreateUObject(this, &UBaseGameInstance::CreateAutoLanSession, MaxArg, MapArg);
	LoadedWorld->GetTimerManager().SetTimer(H, D, FMath::Max(0.f, DelayArg), false);

	// unregister (one-shot)
	FCoreUObjectDelegates::PostLoadMapWithWorld.RemoveAll(this);
}
// Exec 콘솔 커맨드 구현(서버에서만 동작)
void UBaseGameInstance::AutoCreateSession(int32 MaxPlayers, FString MapName, float DelaySeconds)
{
	if (!IsRunningDedicatedServer())
	{
		UE_LOG(LogTemp, Warning, TEXT("[Exec] AutoCreateSession ignored (not dedicated)."));
		return;
	}

	UWorld* World = GetWorld();
	if (!World || !SessionInterface.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("[Exec] AutoCreateSession prerequisites not met."));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[Exec] AutoCreateSession schedule: Map=%s, Max=%d, Delay=%.2fs"), *MapName, MaxPlayers, DelaySeconds);

	FTimerHandle H;
	FTimerDelegate D = FTimerDelegate::CreateUObject(this, &UBaseGameInstance::CreateServerSession, MaxPlayers, MapName);
	World->GetTimerManager().SetTimer(H, D, FMath::Max(0.f, DelaySeconds), false);
}

void UBaseGameInstance::OnStart()
{
	Super::OnStart();


}

void UBaseGameInstance::HostLobby(int NumPlayers,FString MapName)
{
	// 클라이언트: 전용 서버만 실행
	StartLobbyServer(MapName);

	

	if (UWorld* World = GetWorld())
	{
		FTimerHandle H;
		World->GetTimerManager().SetTimer(H, this, &UBaseGameInstance::FindLobbies, 8.0f, false);
	}
	
}

void UBaseGameInstance::CreateServerSession(int32 NumPublicConnections, const FString MapName)
{
    if (UWorld* W = GetWorld())
    {
        UE_LOG(LogTemp, Log, TEXT("[GI] CreateServerSession(enter): Dedicated=%d, NetMode=%d, World=%s"),
            IsRunningDedicatedServer()?1:0, (int32)W->GetNetMode(), *GetNameSafe(W));
    }
    if (!IsRunningDedicatedServer())
    {
        UE_LOG(LogTemp, Warning, TEXT("CreateServerSession ignored (not dedicated)."));
        return;
    }
	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("CreateServerSession: SessionInterface invalid"));
		return;
	}





	if (FNamedOnlineSession* Existing = SessionInterface->GetNamedSession(NAME_GameSession))
	{
		SessionInterface->DestroySession(NAME_GameSession);
	}

	SessionSettings = MakeShared<FOnlineSessionSettings>();
	SessionSettings->bIsLANMatch = true;
	SessionSettings->bIsDedicated = true;
	SessionSettings->bAllowJoinInProgress = true;
	SessionSettings->bShouldAdvertise = true;
	SessionSettings->bUsesPresence = false;
	SessionSettings->NumPublicConnections = NumPublicConnections;

	SessionSettings->Set(SETTING_MAPNAME, MapName, EOnlineDataAdvertisementType::ViaOnlineService);

	OnCreateSessionCompleteDelegateHandle =
		SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteDelegate);

	const bool bCreateIssued = SessionInterface->CreateSession(0, NAME_GameSession, *SessionSettings);
	UE_LOG(LogTemp, Log, TEXT("[GI] CreateServerSession: CreateSession issued=%d"), bCreateIssued ? 1 : 0);
}

void UBaseGameInstance::CreateAutoLanSession(int32 NumPublicConnections, const FString MapName)
{
	SESS_LOG(Log, "CreateAutoLanSession called (Map=%s, MaxPlayers=%d)", *MapName, NumPublicConnections);

	if (!IsRunningDedicatedServer())
	{
		SESS_LOG(Warning, "CreateAutoLanSession ignored - not a dedicated server");
		return;
	}
	if (!SessionInterface.IsValid())
	{
		SESS_LOG(Error, "SessionInterface invalid - cannot create session");
		return;
	}

	// Prevent double-create if session exists
	if (bDedicatedSessionCreated)
	{
		SESS_LOG(Log, "Session already created, skipping");
		return;
	}

	// If a named session already exists, destroy it first
	FName InternalSessionName = NAME_GameSession;
	if (FNamedOnlineSession* Existing = SessionInterface->GetNamedSession(InternalSessionName))
	{
		SESS_LOG(Log, "Existing named session found - destroying before create");
		SessionInterface->DestroySession(InternalSessionName);
	}

	// Build session settings for LAN
	SessionSettings = MakeShared<FOnlineSessionSettings>();
	SessionSettings->bIsLANMatch = true;
	SessionSettings->bIsDedicated = true;
	SessionSettings->bAllowJoinInProgress = true;
	SessionSettings->bShouldAdvertise = true;
	SessionSettings->bUsesPresence = false;
	SessionSettings->NumPublicConnections = FMath::Max(1, NumPublicConnections);

	// Save map name in session settings for search/filter
	SessionSettings->Set(SETTING_MAPNAME, MapName, EOnlineDataAdvertisementType::ViaOnlineService);

	// Use map-based session name (user-visible)
	FString UserSessionName = MakeSessionNameFromMap(MapName);
	SESS_LOG(Log, "UserSessionName=%s", *UserSessionName);

	// Bind delegate
	OnCreateSessionCompleteDelegateHandle =
		SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteDelegate);

	// We use local user index 0 for dedicated server
	const bool bCreateIssued = SessionInterface->CreateSession(0, InternalSessionName, *SessionSettings);
	SESS_LOG(Log, "CreateSession issued=%d (internal=%s, userSession=%s)", bCreateIssued ? 1 : 0, *InternalSessionName.ToString(), *UserSessionName);

	// Note: we set bDedicatedSessionCreated only after successful create+start in OnCreateSessionComplete
}
void UBaseGameInstance::OnCreateSessionComplete(FName SessionName, bool bSucceeded)
{
	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteDelegateHandle);
	}

	SESS_LOG(Log, "OnCreateSessionComplete: Name=%s, Success=%d", *SessionName.ToString(), bSucceeded ? 1 : 0);

	if (!bSucceeded)
	{
		SESS_LOG(Error, "CreateSession failed");
		return;
	}

	if (IsRunningDedicatedServer() && SessionInterface.IsValid())
	{
		// Mark created so we don't recreate
		bDedicatedSessionCreated = true;

		const bool bStartIssued = SessionInterface->StartSession(SessionName);
		SESS_LOG(Log, "StartSession issued=%d", bStartIssued ? 1 : 0);
	}
}
void UBaseGameInstance::StartBookTravel(APlayerController* RequesterPC, int32 ServerPort /*=30010*/)
{


	UWorld* World = RequesterPC->GetWorld();
	if (!World)
	{
		return;
	}

	// 1) 플레이 전용 서버 프로세스 실행
	const FString ServerExePath = FPaths::Combine(FPaths::ProjectDir(), TEXT("Binaries"), TEXT("Win64"), ServerExeName);
	if (!FPaths::FileExists(ServerExePath))
	{
		UE_LOG(LogTemp, Error, TEXT("Play server executable not found: %s"), *ServerExePath);
		return;
	}

	const FString CommandLine = FString::Printf(TEXT("%s -log -port=%d -nosteam"), *PlayLevelName, ServerPort);
	const FString WorkingDir = FPaths::GetPath(ServerExePath);

	FProcHandle ProcHandle = FPlatformProcess::CreateProc(
		*ServerExePath,
		*CommandLine,
		true,   // bLaunchDetached
		false,  // bLaunchHidden
		false,  // bLaunchReallyHidden
		nullptr,
		0,
		*WorkingDir,
		nullptr
	);

	if (!ProcHandle.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to launch personal play server! Path: %s, Args: %s"), *ServerExePath, *CommandLine);
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Play Server Launched: %s %s"), *ServerExePath, *CommandLine);

	// 2) 해당 플레이어만 새 서버로 이동 (서버가 기동될 시간을 1초 정도 부여)
	const FString TargetUrl = FString::Printf(TEXT("%s:%d"), *PublicServerIP, ServerPort);

	FTimerHandle DelayTravelHandle;
	World->GetTimerManager().SetTimer(
		DelayTravelHandle,
		FTimerDelegate::CreateLambda([RequesterPC, TargetUrl]()
		{
			if (IsValid(RequesterPC))
			{
				RequesterPC->ClientTravel(TargetUrl, TRAVEL_Absolute);
			}
		}),
		5.0f, // 지연 시간(필요 시 조정)
		false
	);
}
void UBaseGameInstance::StartLobbyServer(const FString MapName)
{
	const FString ServerPath = FPaths::Combine(FPaths::ProjectDir(), TEXT("Binaries"), TEXT("Win64"), ServerExeName);
	if (!FPaths::FileExists(ServerPath))
	{
		UE_LOG(LogTemp, Error, TEXT("[GI] Lobby server exe not found: %s"), *ServerPath);
		return;
	}

	const int32 MaxPlayers = 6;
	const float DelaySeconds = 2.0f;

	// AutoCreateSession 플래그만 사용 (ExecCmds 제거)
	const FString Flags = FString::Printf(
		TEXT("-AutoCreateSession -SessionMap=%s -SessionMax=%d -SessionDelay=%.1f"),
		*MapName, MaxPlayers, DelaySeconds
	);

	// 반드시 -server 옵션 유지
	const FString CommandLine = FString::Printf(
		TEXT("/Game/Level/%s -server -log -port=30000 -nosteam %s"),
		*MapName, *Flags
	);

	UE_LOG(LogTemp, Log, TEXT("[GI] StartLobbyServer(1run): %s %s"), *ServerPath, *CommandLine);

	const FString WorkingDir = FPaths::GetPath(ServerPath);
	FProcHandle ProcHandle = FPlatformProcess::CreateProc(
		*ServerPath,
		*CommandLine,
		true,   // bLaunchDetached
		false,  // bLaunchHidden
		false,  // bLaunchReallyHidden
		nullptr,
		0,
		*WorkingDir,
		nullptr
	);

	if (!ProcHandle.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("[GI] Failed to launch lobby server: %s %s"), *ServerPath, *CommandLine);
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[GI] Lobby server launched"));



	//// 서버 기동 여유 후 클라이언트 트래블

	//ABasePlayerController* PC = Cast<ABasePlayerController>(GetWorld()->GetFirstPlayerController());


	//const FString HostIP = PC->GetPlayerIP();
	//const FString TargetUrl = FString::Printf(TEXT("%s:%d"), *HostIP, LobbyPort);

	//if (UWorld* World = GetWorld())
	//{
	//	FTimerHandle TravelHandle;
	//	World->GetTimerManager().SetTimer(
	//		TravelHandle,
	//		FTimerDelegate::CreateLambda([this, PC,TargetUrl]()
	//	{
	//		if (PC)
	//		{
	//			SESS_LOG(Log, "ClientTravel to %s", *TargetUrl);
	//			PC->ClientTravel(TargetUrl, TRAVEL_Absolute);
	//		}
	//		else
	//		{
	//			SESS_LOG(Warning, "No local PlayerController for ClientTravel");
	//		}
	//	}),
	//		5.0f, // 필요 시 조정
	//		false
	//	);
	//}
}


void UBaseGameInstance::FindLobbies()
{
	if (!SessionInterface.IsValid())
	{
		SESS_LOG(Warning, "FindLobbies: SessionInterface invalid");
		return;
	}

	SessionSearch = MakeShared<FOnlineSessionSearch>();
	SessionSearch->bIsLanQuery = true;
	SessionSearch->MaxSearchResults = 20;

	OnFindSessionsCompleteDelegateHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(OnFindSessionsCompleteDelegate);

	int32 LocalUserNum = 0;
	if (APlayerController* PC = GetFirstLocalPlayerController())
	{
		LocalUserNum = PC->GetLocalPlayer()->GetControllerId();
	}
	bool bIssued = SessionInterface->FindSessions(LocalUserNum, SessionSearch.ToSharedRef());
	SESS_LOG(Log, "FindSessions issued=%d", bIssued ? 1 : 0);
}


void UBaseGameInstance::OnFindSessionsComplete(bool bSucceeded)
{
	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(OnFindSessionsCompleteDelegateHandle);
	}

	SESS_LOG(Log, "OnFindSessionsComplete: Success=%d, Results=%d", bSucceeded ? 1 : 0, SessionSearch.IsValid() ? SessionSearch->SearchResults.Num() : 0);

	if (!bSucceeded || !SessionSearch.IsValid())
	{
		SESS_LOG(Warning, "Find sessions failed or invalid search");
		return;
	}

	FString WantedSessionName = FString(TEXT("LobbySession")); // map-based rule expects "LobbySession" for LobbyLevel

	int32 ChosenIndex = INDEX_NONE;
	for (int32 i = 0; i < SessionSearch->SearchResults.Num(); ++i)
	{
		const FOnlineSessionSearchResult& R = SessionSearch->SearchResults[i];

		FString MapName;
		R.Session.SessionSettings.Get(SETTING_MAPNAME, MapName);

		SESS_LOG(Log, "[%d] Id=%s Map=%s Ping=%d", i, *R.GetSessionIdStr(), *MapName, R.PingInMs);

		// Map -> Session name mapping (as used by server)
		FString CandidateSessionName = MakeSessionNameFromMap(MapName);
		if (CandidateSessionName == WantedSessionName)
		{
			ChosenIndex = i;
			break;
		}
	}

	if (ChosenIndex == INDEX_NONE)
	{
		SESS_LOG(Warning, "No matching lobby session (Wanted=%s)", *WantedSessionName);
		return;
	}

	OnJoinSessionCompleteDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegate);


	int32 LocalUserNum = 0;
	if (APlayerController* PC = GetFirstLocalPlayerController())
	{
		LocalUserNum = PC->GetLocalPlayer()->GetControllerId();
	}


	bool bJoinIssued = SessionInterface->JoinSession(LocalUserNum, NAME_GameSession, SessionSearch->SearchResults[ChosenIndex]);
	SESS_LOG(Log, "JoinSession issued=%d (index=%d)", bJoinIssued ? 1 : 0, ChosenIndex);
}


void UBaseGameInstance::JoinLobby(int SearchIndex)
{
	if (!SessionInterface.IsValid() || !SessionSearch.IsValid() || !SessionSearch->SearchResults.IsValidIndex(SearchIndex)) return;

	const FOnlineSessionSearchResult& Result = SessionSearch->SearchResults[SearchIndex];

	OnJoinSessionCompleteDelegateHandle =
		SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegate);

	SessionInterface->JoinSession(0, NAME_GameSession, Result);
}

void UBaseGameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegateHandle);
	}

	SESS_LOG(Log, "OnJoinSessionComplete: SessionName=%s (%d)",
		*SessionName.ToString(), (int32)Result);

	if (Result != EOnJoinSessionCompleteResult::Success)
	{
		// 흔한 실패 원인 힌트 로그
		switch (Result)
		{
		case EOnJoinSessionCompleteResult::CouldNotRetrieveAddress:
			SESS_LOG(Warning, "Could not retrieve address. Check server advertised address/port and firewall.");
			break;
		case EOnJoinSessionCompleteResult::AlreadyInSession:
			SESS_LOG(Warning, "Already in a local session. Destroy it before joining another.");
			break;
		default:
			break;
		}
		return;
	}

	if (Result != EOnJoinSessionCompleteResult::Success)
	{
		SESS_LOG(Warning, "Join session failed: %d", static_cast<int32>(Result));
		return;
	}

	FString ConnectString;
	if (SessionInterface->GetResolvedConnectString(SessionName, ConnectString))
	{
		SESS_LOG(Log, "Resolved connect string: %s", *ConnectString);
		if (APlayerController* PC = GetFirstLocalPlayerController())
		{
			PC->ClientTravel(ConnectString, TRAVEL_Absolute);
		}
	}
	else
	{
		SESS_LOG(Error, "GetResolvedConnectString failed");
	}
}

void UBaseGameInstance::StartServer(FString& _IP, FString& _Port)
{
	// 미사용(세션 기반 권장)
}

void UBaseGameInstance::Connect(const FString& _IP, const FString& _Port)
{
	const FString ConnectLevelName = FString::Printf(TEXT("%s:%s"), *_IP, *_Port);
	UGameplayStatics::OpenLevel(GetWorld(), FName(*ConnectLevelName));
}
FString UBaseGameInstance::MakeSessionNameFromMap(const FString& MapName)
{
	// MapName: e.g. "LobbyLevel" or "/Game/Level/LobbyLevel"
	FString Clean = MapName;
	// If path-style, pick last segment
	if (Clean.Contains(TEXT("/")))
	{
		TArray<FString> Parts;
		Clean.ParseIntoArray(Parts, TEXT("/"));
		if (Parts.Num() > 0) Clean = Parts.Last();
	}
	// Remove query params if any (e.g. ?Name=)
	int32 QIdx;
	if (Clean.FindChar(TEXT('?'), QIdx))
	{
		Clean = Clean.Left(QIdx);
	}
	// Remove extension or prefix if any
	// Strip "Level" suffix if exists (LobbyLevel -> Lobby)
	FString Base = Clean;
	if (Base.EndsWith(TEXT("Level")))
	{
		Base = Base.LeftChop(5); // remove "Level"
	}

	if (Base.IsEmpty())
	{
		Base = TEXT("Game");
	}

	// Final session name: e.g. Lobby -> LobbySession
	return FString::Printf(TEXT("%sSession"), *Base);
}
