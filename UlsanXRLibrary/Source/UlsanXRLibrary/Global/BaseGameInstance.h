#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Engine/NetDriver.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "BaseGameInstance.generated.h"

// 블루프린트 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLoginSuccessDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLoginFailedDelegate, FString, ErrorMessage);

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

	// ==================== EOS Connect 로그인 (Device ID) ====================



	// 블루프린트 이벤트
	UPROPERTY(BlueprintAssignable, Category = "Online")
	FOnLoginSuccessDelegate OnLoginSuccessEvent;

	UPROPERTY(BlueprintAssignable, Category = "Online")
	FOnLoginFailedDelegate OnLoginFailedEvent;

	// ==================== 세션 관리 ====================

	UFUNCTION(BlueprintCallable, Category = "Online")
	void HostLobby(const FString& SessionType);

	UFUNCTION(BlueprintCallable, Category = "Online")
	void JoinLobby(const FString& SessionType);

	// 세션 콜백
	void OnCreateSessionComplete(FName SessionName, bool bSucceeded);
	void OnFindSessionsComplete(bool bSucceeded);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

	// ==================== 기존 Direct Connect (레거시) ====================

	//void StartServer(FString& _IP, FString& _Port);
//	void Connect(const FString& _IP, const FString& _Port);
//
protected:
	// ==================== Online Subsystem ====================

	IOnlineSessionPtr SessionInterface;
	TSharedPtr<FOnlineSessionSettings> SessionSettings;
	TSharedPtr<FOnlineSessionSearch> SessionSearch;

	// 델리게이트 핸들
	FDelegateHandle OnLoginCompleteDelegateHandle;
	FDelegateHandle OnCreateSessionCompleteDelegateHandle;
	FDelegateHandle OnFindSessionsCompleteDelegateHandle;
	FDelegateHandle OnJoinSessionCompleteDelegateHandle;

	FString WantedSessionType;

	// 전용 서버에서 중복 생성 방지
	bool bDedicatedSessionCreated = false;

protected:
	// ==================== Server Settings ====================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Server", meta = (AllowPrivateAccess = "true"))
	FString IP = TEXT("127.0.0.1");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Server", meta = (AllowPrivateAccess = "true"))
	FString Port = TEXT("30000");

	// 플레이 맵 경로(패키지 경로)
	UPROPERTY(EditDefaultsOnly, Category = "Server")
	FString PlayLevelName = TEXT("/Game/Level/PlayLevel");

	UPROPERTY(EditDefaultsOnly, Category = "Server")
	FString LobbyLevelName = TEXT("/Game/Level/LobbyLevel");

	// 서버 실행 파일명(패키징/개발 환경에 맞춰 조정)
	UPROPERTY(EditDefaultsOnly, Category = "Server")
	FString ServerExeName = TEXT("UlsanXRLibraryServer.exe");

	// 로비 서버의 외부 접속 IP(클라이언트가 접속해야 하는 서버 IP)
	UPROPERTY(EditDefaultsOnly, Category = "Server")
	FString PublicServerIP = TEXT("127.0.0.1");

private:
	// ==================== Data Tables ====================

	UPROPERTY(VisibleAnywhere, Category = "Data")
	class UDataTable* DataTables = nullptr;

	class UDataTable* ResourceDataTable = nullptr;
	class UDataTable* LevelDataTable = nullptr;
	class UDataTable* BookItemDataTable = nullptr;
	class UDataTable* ActorDataTable = nullptr;


};