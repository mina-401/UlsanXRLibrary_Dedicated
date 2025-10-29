// Fill out your copyright notice in the Description page of Project Settings.

#include "Global/BaseGameInstance.h"
#include "Engine/Engine.h"
#include "Engine/NetDriver.h"
#include "Engine/World.h"
#include "Engine/NetworkDelegates.h"
#include "Kismet/GameplayStatics.h"

// Online Subsystem
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "GameFramework/PlayerController.h"

//#include <ULXRGlobal.h>
//#include <Mode/Title/UI/TitleUserWidget.h>
//#include <Mode/Title/UI/TitleRobby.h>
//#include <Data/GlobalDataTable.h>

#include "Modules/ModuleManager.h"
#include <ULXRConst.h>
#include "Misc/Paths.h"
#include "HAL/PlatformProcess.h"

FString UBaseGameInstance::GetPlayWorldLevel()
{
	// 프로젝트 의존 코드 주석 처리됨
	return TEXT("");
}

UBaseGameInstance::UBaseGameInstance()
{
	// DataTables 등은 주석 처리 상태 유지

	{
		FString DataPath = UULXRConst::Path::GlobalDataTablePath;
		ConstructorHelpers::FObjectFinder<UDataTable> FinderDataTables(*DataPath);
		if (true == FinderDataTables.Succeeded())
		{
			DataTables = FinderDataTables.Object;
		}

		if (nullptr != DataTables)
		{
			/*LevelDataTable = DataTables->FindRow<FDataTableRow>("DT_LevelDataTable", nullptr)->Resources;
			if (nullptr == LevelDataTable)
			{
				return;
			}*/
		//	BookItemDataTable = DataTables->FindRow<FDataTableRow>("DT_BookItemDataTable", nullptr)->Resources;
		//	if (nullptr == BookItemDataTable)
		//	{
		//		//return;
		//	}
		//	ActorDataTable = DataTables->FindRow<FDataTableRow>("DT_GlobalActorDataTable", nullptr)->Resources;
		//	if (nullptr == ActorDataTable)
		//	{
		//		//UE_LOG(GMLOG, Fatal, TEXT("%S(%u)> if (nullptr == ActorDataTable)"), __FUNCTION__, __LINE__);
		//	}
		}



	}
}

void UBaseGameInstance::Init()
{
	Super::Init();

	IOnlineSubsystem* OSS = IOnlineSubsystem::Get();
	if (OSS)
	{
		SessionInterface = OSS->GetSessionInterface();

		if (SessionInterface.IsValid())
		{
			// 델리게이트 바인딩
			OnCreateSessionCompleteDelegate = FOnCreateSessionCompleteDelegate::CreateUObject(this, &UBaseGameInstance::OnCreateSessionComplete);
			OnFindSessionsCompleteDelegate = FOnFindSessionsCompleteDelegate::CreateUObject(this, &UBaseGameInstance::OnFindSessionsComplete);
			OnJoinSessionCompleteDelegate = FOnJoinSessionCompleteDelegate::CreateUObject(this, &UBaseGameInstance::OnJoinSessionComplete);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No OnlineSubsystem found!"));
	}
}

void UBaseGameInstance::HostLobby(int NumPlayers)
{
	if (!SessionInterface.IsValid()) return;

	// 이미 있으면 삭제(단순화)
	FNamedOnlineSession* ExistingSession = SessionInterface->GetNamedSession(NAME_GameSession);
	if (ExistingSession)
	{
		SessionInterface->DestroySession(NAME_GameSession);
	}

	SessionSettings = MakeShared<FOnlineSessionSettings>();
	SessionSettings->bIsLANMatch = true; // Null OSS -> LAN
	SessionSettings->NumPublicConnections = static_cast<int32>(NumPlayers);
	SessionSettings->bAllowJoinInProgress = true;
	SessionSettings->bShouldAdvertise = true;
	SessionSettings->bUsesPresence = false;

	OnCreateSessionCompleteDelegateHandle =
		SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteDelegate);

	SessionInterface->CreateSession(0, NAME_GameSession, *SessionSettings);
}

void UBaseGameInstance::OnCreateSessionComplete(FName SessionName, bool bSucceeded)
{
	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteDelegateHandle);
	}

	if (bSucceeded)
	{
		UE_LOG(LogTemp, Log, TEXT("Session created: %s"), *SessionName.ToString());
		// 호스트는 Lobby 레벨로 Travel (listen server)
		if (UWorld* World = GetWorld())
		{
			// 프로젝트의 실제 로비 맵 경로로 교체하세요.
			//World->ServerTravel(TEXT("/Game/Level/LobbyLevel?listen"));

			//StartLobbyServer();

			if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
			{
				PC->ClientTravel(TEXT("127.0.0.1:30000"), TRAVEL_Absolute);
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to create session"));
	}
}

void UBaseGameInstance::StartLobbyServer()
{
	// 1) 서버 실행 파일 경로 구성
	const FString ServerExeName = TEXT("UlsanXRLibraryServer.exe"); // ProjectName + "Server.exe"
	const FString ServerPath = FPaths::Combine(FPaths::ProjectDir(), TEXT("Binaries"), TEXT("Win64"), ServerExeName);

	// 존재 여부 확인
	if (!FPaths::FileExists(ServerPath))
	{
		UE_LOG(LogTemp, Error, TEXT("Server executable not found: %s"), *ServerPath);
		return;
	}

	// 2) 커맨드라인: 맵 패키지 경로 + 옵션
	// - 서버 타겟이면 -server 생략 가능(있어도 무방)
	// - 필요시 포트 변경 가능: -port=30000
	// - 디버그 편의를 위해 -log 권장
	const FString CommandLine = TEXT("/Game/Level/LobbyLevel -log -port=30000 -nosteam");

	// 3) 워킹 디렉터리: 실행 파일이 있는 폴더 권장
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
		UE_LOG(LogTemp, Error, TEXT("Failed to launch Lobby Dedicated Server! Path: %s, Args: %s"), *ServerPath, *CommandLine);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Lobby Server Launched: %s %s"), *ServerPath, *CommandLine);
		// 필요 시 핸들 보관/종료 관리
	}
}

void UBaseGameInstance::FindLobbies()
{
	if (!SessionInterface.IsValid()) return;

	SessionSearch = MakeShared<FOnlineSessionSearch>();
	SessionSearch->bIsLanQuery = true;
	SessionSearch->MaxSearchResults = 20;
	//SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

	OnFindSessionsCompleteDelegateHandle =
		SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(OnFindSessionsCompleteDelegate);

	SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());
}

void UBaseGameInstance::OnFindSessionsComplete(bool bSucceeded)
{
	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(OnFindSessionsCompleteDelegateHandle);
	}

	if (!bSucceeded || !SessionSearch.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Find sessions failed or invalid search"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Found %d sessions"), SessionSearch->SearchResults.Num());

	// 디버그: 리스트 출력
	for (int32 i = 0; i < SessionSearch->SearchResults.Num(); ++i)
	{
		const FOnlineSessionSearchResult& Result = SessionSearch->SearchResults[i];
		FString HostAddr;
		Result.Session.SessionSettings.Get(FName("HOST_ADDR"), HostAddr); // optional
		UE_LOG(LogTemp, Log, TEXT("[%d] %s - Ping: %d"), i, *Result.GetSessionIdStr(), Result.PingInMs);
	}
	if (SessionSearch->SearchResults.Num() > 0)
	{
		OnJoinSessionCompleteDelegateHandle =
			SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegate);

		const FOnlineSessionSearchResult& First = SessionSearch->SearchResults[0];
		SessionInterface->JoinSession(0, NAME_GameSession, First);
	}
	// UI에 SessionSearch->SearchResults 를 표시하도록 전달하면 됩니다.
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

	if (Result == EOnJoinSessionCompleteResult::Success)
	{
		// 접속 문자열 얻기
		FString ConnectString;
		if (SessionInterface->GetResolvedConnectString(SessionName, ConnectString))
		{
			if (APlayerController* PC = GetFirstLocalPlayerController())
			{
				PC->ClientTravel(ConnectString, TRAVEL_Absolute);
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Join session failed: %d"), static_cast<int32>(Result));
	}
}



void UBaseGameInstance::StartServer(FString& _IP, FString& _Port)
{
	// 세션 기반 HostLobby 사용 권장
}

void UBaseGameInstance::Connect(const FString& _IP, const FString& _Port)
{
	// 직접 IP:Port 접속 (세션 미사용)
	const FString ConnectLevelName = FString::Printf(TEXT("%s:%s"), *_IP, *_Port);
	UGameplayStatics::OpenLevel(GetWorld(), FName(*ConnectLevelName));
}
