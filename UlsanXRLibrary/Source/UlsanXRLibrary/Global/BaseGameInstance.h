#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Engine/NetDriver.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "BaseGameInstance.generated.h"

UCLASS()
class ULSANXRLIBRARY_API UBaseGameInstance : public UGameInstance
{
	GENERATED_BODY()

	friend class UGlobalDataTable;
	friend class UFallGlobal;

public:
	UPROPERTY()
	FString BaseSessionName = TEXT("UXLR");


public:
	UBaseGameInstance();

	// 클라이언트/서버 공통 초기화(델리게이트 바인딩)
	virtual void Init() override;

	

	// 월드 시작 시(서버에서 세션 생성 트리거로 사용)
	virtual void OnStart() override;

	// 클라이언트: 전용 서버 실행(세션 생성 X)
	UFUNCTION(BlueprintCallable, Category="Session")
	void HostLobby(int NumPlayers = 6, FString MapName="TitleLevel");

	// 클라이언트: 서버 세션 검색/참가
	UFUNCTION(BlueprintCallable, Category="Session")
	void FindLobbies();

	UFUNCTION(BlueprintCallable, Category="Session")
	void JoinLobby(int SearchIndex);

	// 기존
	void StartServer(FString& _IP, FString& _Port);
	void Connect(const FString& _IP, const FString& _Port);

	

	//로비서버 생성
	void StartLobbyServer(const FString MapName);

	// 서버 콘솔에서 호출 가능한 Exec 명령: 지연 후 세션 생성
	UFUNCTION(Exec)
	void AutoCreateSession(int32 MaxPlayers = 6, FString MapName = TEXT("TitleLevel"), float DelaySeconds = 1.0f);
	UFUNCTION()
	void HandleAutoCreateAfterLoad(UWorld* LoadedWorld);
	FString MakeSessionNameFromMap(const FString& MapName);
protected:
	// 전용 서버에서 세션을 생성/광고
	void CreateServerSession(int32 NumPublicConnections = 6,FString MapName=TEXT("TitleLevel"));

	void CreateAutoLanSession(int32 NumPublicConnections, const FString MapName);

	// 세션 콜백
	void OnCreateSessionComplete(FName SessionName, bool bSucceeded);


	
	void OnFindSessionsComplete(bool bSucceeded);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

protected:
	// OSS
	IOnlineSessionPtr SessionInterface;
	TSharedPtr<FOnlineSessionSettings> SessionSettings;
	TSharedPtr<FOnlineSessionSearch> SessionSearch;

	FOnCreateSessionCompleteDelegate OnCreateSessionCompleteDelegate;
	FDelegateHandle OnCreateSessionCompleteDelegateHandle;

	FOnFindSessionsCompleteDelegate OnFindSessionsCompleteDelegate;
	FDelegateHandle OnFindSessionsCompleteDelegateHandle;

	FOnJoinSessionCompleteDelegate OnJoinSessionCompleteDelegate;
	FDelegateHandle OnJoinSessionCompleteDelegateHandle;

	// 전용 서버에서 중복 생성 방지
	bool bDedicatedSessionCreated = false;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Server", meta = (AllowPrivateAccess = "true"))
	FString IP = TEXT("127.0.0.1");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Server", meta = (AllowPrivateAccess = "true"))
	FString Port = TEXT("30000");

public:
	// 요청한 플레이어만 플레이 서버로 이동시키기

	UFUNCTION(BlueprintCallable, Category = "Server")
	void StartBookTravel(APlayerController* RequesterPC, int32 ServerPort= 30010);


protected:
	// 플레이 맵 경로(패키지 경로)
	UPROPERTY(EditDefaultsOnly, Category = "Server")
	FString PlayLevelName = TEXT("/Game/Level/PlayLevel");

	UPROPERTY(EditDefaultsOnly, Category = "Server")
	FString LobbyLevelName = TEXT("/Game/Level/LobbyLevel");

	// 서버 실행 파일명(패키징/개발 환경에 맞춰 조정)
	UPROPERTY(EditDefaultsOnly, Category = "Server")
	FString ServerExeName = TEXT("UlsanXRLibraryServer.exe");

	// 로비 서버의 외부 접속 IP(클라이언트가 접속해야 하는 서버 IP)
	// 로컬 테스트는 127.0.0.1, 배포 환경은 공인/내부 IP를 셋업
	UPROPERTY(EditDefaultsOnly, Category = "Server")
	FString PublicServerIP = TEXT("127.0.0.1");

	


public:
	UFUNCTION(BlueprintCallable)
	FString GetPlayWorldLevel();

private:
	UPROPERTY(VisibleAnywhere, Category = "Data")
	class UDataTable* DataTables = nullptr;
	class UDataTable* ResourceDataTable = nullptr;
	class UDataTable* LevelDataTable = nullptr;
	class UDataTable* BookItemDataTable = nullptr;
	class UDataTable* ActorDataTable = nullptr;
	/*void SetIP(const FString& _IP)
	{
		IP = _IP;
	}

	void SetPort(const FString& _Port)
	{
		Port = _Port;
	}

	FString GetIP()
	{
		return IP;
	}

	FString GetPort()
	{
		return Port;
	}*/
};
