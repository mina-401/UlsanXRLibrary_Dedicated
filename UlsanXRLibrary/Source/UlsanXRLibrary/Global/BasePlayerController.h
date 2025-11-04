// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputMappingContext.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "VoiceChat.h" // IVoiceChat API
#include "BasePlayerController.generated.h"

/**
 *
 */
UCLASS()
class ULSANXRLIBRARY_API ABasePlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ABasePlayerController();

	UFUNCTION(BlueprintCallable)
	void AddMappingContext(UInputMappingContext* MappingContext);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void SetupInputComponentEvent();

	UFUNCTION()
	FString GetPlayerIP() const;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	void SetupInputComponent() override;

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* MappingContext = nullptr;

	// 관리 => 자료구조
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TMap<FString, const UInputAction*> MappingActions;

	// ==================== Voice Chat (IVoiceChat API) ====================
public:
	// 서버에서 Voice 세션 정보 받기 (RPC)
	UFUNCTION(Client, Reliable)
	void ClientJoinVoiceSession(const FString& SessionId);

	// 음소거 토글 (블루프린트에서 호출)
	UFUNCTION(BlueprintCallable, Category = "Voice")
	void ToggleMute();

	// 음소거 상태 확인
	UFUNCTION(BlueprintPure, Category = "Voice")
	bool IsVoiceMuted() const { return bIsVoiceMuted; }

	// Voice 연결 상태 확인
	UFUNCTION(BlueprintPure, Category = "Voice")
	bool IsVoiceConnected() const { return bIsVoiceConnected; }

protected:
	// Voice 초기화
	void InitializeVoiceWhenReady();
	void InitializeVoiceChat();

	// Voice Login 콜백
	void OnVoiceLoginComplete(const FString& PlayerName, const FVoiceChatResult& Result);

	// Voice 채널 참여
	void JoinVoiceChannel(const FString& ChannelName);
	void OnVoiceChannelJoinComplete(const FString& ChannelName, const FVoiceChatResult& Result);

	// Voice 채널 나가기
	void LeaveVoiceChannel();
	void LeaveVoiceSession();

	// Voice 자동 송신 시작/중지
	void StartVoiceTransmission();
	void StopVoiceTransmission();

	// 세션 참여 콜백
	void OnSessionJoined(FName SessionName, bool bWasSuccessful);

private:
	// Online Session Interface (게임 세션용)
	IOnlineSessionPtr SessionInterface;

	// Voice Chat User (IVoiceChat API)
	TSharedPtr<IVoiceChatUser, ESPMode::ThreadSafe> VoiceChatUser;

	// Voice 초기화 타이머
	FTimerHandle VoiceInitTimerHandle;

	// Voice 상태
	UPROPERTY()
	bool bIsVoiceConnected;

	UPROPERTY()
	bool bIsVoiceMuted;

	UPROPERTY()
	FString CurrentVoiceSessionId;

	UPROPERTY()
	FString PlatformUserId; // EOS Product User ID
};