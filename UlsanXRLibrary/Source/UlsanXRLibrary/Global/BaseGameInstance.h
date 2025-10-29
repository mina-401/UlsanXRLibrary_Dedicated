
#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Engine/NetDriver.h"
// Online Subsystem (세션 인터페이스/설정 사용)
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

	virtual void Init() override;

	// Host/Find/Join (Blueprint에서도 호출 가능)
	UFUNCTION(BlueprintCallable, Category = "Session")
	void HostLobby(int NumPlayers = 4);

	UFUNCTION(BlueprintCallable, Category = "Session")
	void FindLobbies();

	UFUNCTION(BlueprintCallable, Category = "Session")
	void JoinLobby(int SearchIndex);

	void StartLobbyServer();
private:
	// 세션 콤포넌트
	IOnlineSessionPtr SessionInterface;
	TSharedPtr<FOnlineSessionSettings> SessionSettings;
	TSharedPtr<FOnlineSessionSearch> SessionSearch;

	// 델리게이트/핸들
	FOnCreateSessionCompleteDelegate OnCreateSessionCompleteDelegate;
	FDelegateHandle OnCreateSessionCompleteDelegateHandle;

	FOnFindSessionsCompleteDelegate OnFindSessionsCompleteDelegate;
	FDelegateHandle OnFindSessionsCompleteDelegateHandle;

	FOnJoinSessionCompleteDelegate OnJoinSessionCompleteDelegate;
	FDelegateHandle OnJoinSessionCompleteDelegateHandle;

	// 델리게이트 콜백
	void OnCreateSessionComplete(FName SessionName, bool bSucceeded);
	void OnFindSessionsComplete(bool bSucceeded);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);





public:
	void StartServer(FString& _IP, FString& _Port);

	void Connect(const FString& _IP, const FString& _Port);


	void SetIP(const FString& _IP)
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
	}


protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Server", meta = (AllowPrivateAccess = "true"))
	FString IP = TEXT("127.0.0.1");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Server", meta = (AllowPrivateAccess = "true"))
	FString Port = TEXT("30000");
private:
	class UTitleUserWidget* CurWidget = nullptr;

	UPROPERTY(VisibleAnywhere, Category = "Data")
	class UDataTable* DataTables = nullptr;
	class UDataTable* ResourceDataTable = nullptr;
	class UDataTable* LevelDataTable = nullptr;
	class UDataTable* BookItemDataTable = nullptr;
	class UDataTable* ActorDataTable = nullptr;

public:

	UFUNCTION(BlueprintCallable)
	FString GetPlayWorldLevel();

};
