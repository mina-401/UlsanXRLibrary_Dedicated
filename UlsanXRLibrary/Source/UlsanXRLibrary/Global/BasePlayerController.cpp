// Fill out your copyright notice in the Description page of Project Settings.


#include "Global/BasePlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include <SocketSubsystem.h>
#include <OnlineSessionSettings.h>
#include "VoiceChat.h"
#include "VoiceChatResult.h"

#define VOICE_LOG(Verbosity, Format, ...) UE_LOG(LogTemp, Verbosity, TEXT("[VOICE_CLIENT] %s: " Format), TEXT(__FUNCTION__), ##__VA_ARGS__)

void ABasePlayerController::BeginPlay()
{
	Super::BeginPlay();
	if (IsLocalController() && GetWorld()->GetNetMode() != NM_DedicatedServer)
	{
		VOICE_LOG(Log, "Scheduling voice initialization...");
		InitializeVoiceWhenReady();
	}
}

void ABasePlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// Voice 세션에서 나가기
	LeaveVoiceSession();
}

// ==================== Voice 초기화 ====================
void ABasePlayerController::InitializeVoiceWhenReady()
{
	// 1. SessionInterface 가져오기
		// SessionInterface 가져오기
	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	if (OnlineSubsystem)
	{
		SessionInterface = OnlineSubsystem->GetSessionInterface();
	}

	// LocalPlayer가 준비될 때까지 대기
	if (!GetLocalPlayer())
	{
		VOICE_LOG(Warning, "LocalPlayer not ready, retrying...");
		GetWorld()->GetTimerManager().SetTimer(
			VoiceInitTimerHandle,
			this,
			&ABasePlayerController::InitializeVoiceWhenReady,
			0.5f,
			false
		);
		return;
	}

	// Voice 초기화
	InitializeVoiceChat();
}
void ABasePlayerController::InitializeVoiceChat()
{
	IVoiceChat* VoiceChat = IVoiceChat::Get();
	if (!VoiceChat)
	{
		VOICE_LOG(Error, "VoiceChat module not available!");
		return;
	}

	if (!VoiceChat->IsInitialized())
	{
		VoiceChat->Initialize();
		VOICE_LOG(Log, "VoiceChat system initialized");
	}

	// CreateUser는 raw pointer를 반환하므로 MakeShareable 사용
	IVoiceChatUser* NewUser = VoiceChat->CreateUser();
	if (!NewUser)
	{
		VOICE_LOG(Error, "Failed to create VoiceChatUser!");
		return;
	}

	VoiceChatUser = MakeShareable(NewUser);

	// Player ID 생성 (EOS의 경우 PlatformUserId 필요)
	const FString PlayerName = TEXT("Player_") + FString::FromInt(GetLocalPlayer()->GetControllerId());

	// Login 시작 (UE 5.4 API: PlatformUserId, PlayerName, Credentials, Delegate)
	FPlatformUserId UserId = GetLocalPlayer()->GetPlatformUserId();

	VoiceChatUser->Login(
		UserId,
		PlayerName,
		TEXT(""), // Credentials (empty for EOS)
		FOnVoiceChatLoginCompleteDelegate::CreateUObject(this, &ABasePlayerController::OnVoiceLoginComplete)
	);

	VOICE_LOG(Log, "Voice login initiated for: %s", *PlayerName);
}

void ABasePlayerController::OnVoiceLoginComplete(const FString& PlayerName, const FVoiceChatResult& Result)
{
	if (!Result.IsSuccess())
	{
		VOICE_LOG(Error, "Voice Login failed: %s", *Result.ErrorDesc);
		return;
	}

	VOICE_LOG(Log, "Voice Login Success: %s", *PlayerName);

	// 채널 참여 대기 (서버에서 SessionId 받을 때까지)
}

// ==================== Voice 세션 참여 (서버에서 호출) ====================

void ABasePlayerController::ClientJoinVoiceSession_Implementation(const FString& SessionId)
{
	VOICE_LOG(Log, "Received voice session ID: %s", *SessionId);

	CurrentVoiceSessionId = SessionId;

	// 약간의 딜레이 후 Voice 채널 참여
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(
		TimerHandle,
		[this]()
	{
		JoinVoiceChannel(CurrentVoiceSessionId);
	},
		0.5f,
		false
	);
}

void ABasePlayerController::JoinVoiceChannel(const FString& ChannelName)
{
	if (!VoiceChatUser.IsValid())
	{
		VOICE_LOG(Error, "VoiceChatUser is invalid!");
		return;
	}

	// 3D 음성 속성
	FVoiceChatChannel3dProperties Props;

	// JoinChannel API (UE 5.4):
	// void JoinChannel(const FString& ChannelName, const FString& ChannelCredentials, 
	//                  EVoiceChatChannelType ChannelType, const FOnVoiceChatChannelJoinCompleteDelegate& Delegate, 
	//                  TOptional<FVoiceChatChannel3dProperties> Channel3dProperties)

	VoiceChatUser->JoinChannel(
		ChannelName,
		TEXT(""), // ChannelCredentials (empty for EOS)
		EVoiceChatChannelType::NonPositional, // 또는 Positional for 3D voice
		FOnVoiceChatChannelJoinCompleteDelegate::CreateUObject(this, &ABasePlayerController::OnVoiceChannelJoinComplete),
		TOptional<FVoiceChatChannel3dProperties>(Props)
	);

	VOICE_LOG(Log, "Attempting to join voice channel: %s", *ChannelName);
}

void ABasePlayerController::OnVoiceChannelJoinComplete(const FString& ChannelName, const FVoiceChatResult& Result)
{
	if (!Result.IsSuccess())
	{
		VOICE_LOG(Error, "Join Channel Failed: %s", *Result.ErrorDesc);
		return;
	}

	VOICE_LOG(Log, "Successfully joined voice channel: %s", *ChannelName);

	bIsVoiceConnected = true;

	// 자동으로 음성 송신 시작
	StartVoiceTransmission();
}

void ABasePlayerController::LeaveVoiceChannel()
{
	if (!VoiceChatUser.IsValid())
		return;

	// LeaveChannel API (UE 5.4):
	// void LeaveChannel(const FString& ChannelName, const FOnVoiceChatChannelLeaveCompleteDelegate& Delegate)

	VoiceChatUser->LeaveChannel(
		CurrentVoiceSessionId,
		FOnVoiceChatChannelLeaveCompleteDelegate::CreateLambda([this](const FString& ChannelName, const FVoiceChatResult& Result)
	{
		if (Result.IsSuccess())
		{
			VOICE_LOG(Log, "Left voice channel: %s", *ChannelName);
		}
		else
		{
			VOICE_LOG(Warning, "Failed to leave channel: %s", *Result.ErrorDesc);
		}
	})
	);
}

void ABasePlayerController::LeaveVoiceSession()
{
	if (!bIsVoiceConnected)
		return;

	StopVoiceTransmission();

	// 채널 떠나기
	LeaveVoiceChannel();

	// 로그아웃 (UE 5.4):
	// void Logout(const FOnVoiceChatLogoutCompleteDelegate& Delegate)
	if (VoiceChatUser.IsValid())
	{
		VoiceChatUser->Logout(
			FOnVoiceChatLogoutCompleteDelegate::CreateLambda([this](const FString& PlayerName, const FVoiceChatResult& Result)
		{
			if (Result.IsSuccess())
			{
				VOICE_LOG(Log, "Voice logout successful: %s", *PlayerName);
			}
		})
		);

		VoiceChatUser.Reset();
	}

	bIsVoiceConnected = false;
	CurrentVoiceSessionId.Empty();

	VOICE_LOG(Log, "Left voice session completely");
}

// ==================== Voice 송수신 제어 ====================

void ABasePlayerController::StartVoiceTransmission()
{
	if (!VoiceChatUser.IsValid())
	{
		VOICE_LOG(Error, "Cannot start transmission: VoiceChatUser invalid");
		return;
	}

	if (!bIsVoiceConnected)
	{
		VOICE_LOG(Warning, "Cannot start transmission: Not connected to channel");
		return;
	}

	// 음성 송신 활성화 (Always On)
	VoiceChatUser->SetAudioInputDeviceMuted(false);
	VoiceChatUser->SetAudioOutputDeviceMuted(false);

	bIsVoiceMuted = false;

	VOICE_LOG(Log, "Voice transmission started (Always On)");
}

void ABasePlayerController::StopVoiceTransmission()
{
	if (!VoiceChatUser.IsValid())
		return;

	// 음성 송신 비활성화
	VoiceChatUser->SetAudioInputDeviceMuted(true);

	bIsVoiceMuted = true;

	VOICE_LOG(Log, "Voice transmission stopped (Muted)");
}

void ABasePlayerController::ToggleMute()
{
	if (!bIsVoiceConnected)
	{
		VOICE_LOG(Warning, "Not connected to voice session!");
		return;
	}

	if (bIsVoiceMuted)
	{
		StartVoiceTransmission();
		VOICE_LOG(Log, "Unmuted");
	}
	else
	{
		StopVoiceTransmission();
		VOICE_LOG(Log, "Muted");
	}
}

// ==================== 세션 이벤트 핸들러 ====================

void ABasePlayerController::OnSessionJoined(FName SessionName, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		VOICE_LOG(Log, "Session joined: %s", *SessionName.ToString());

		// 게임 세션 참여 성공 후 Voice 초기화
		if (!VoiceChatUser.IsValid())
		{
			InitializeVoiceChat();
		}
	}
}






ABasePlayerController::ABasePlayerController()
{
}
FString ABasePlayerController::GetPlayerIP() const
{
	bool bIsLocalHost = false;
	TSharedRef<FInternetAddr> LocalAddr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->GetLocalHostAddr(*GLog, bIsLocalHost);
	return LocalAddr->ToString(false); // false = 포트 제외

}

void ABasePlayerController::AddMappingContext(UInputMappingContext* _MappingContext)
{

	if (nullptr == GetLocalPlayer())
	{
		return;
	}

	UEnhancedInputLocalPlayerSubsystem* InputSystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());


	// 컨트롤러의 내부에서는 
	TArray<FEnhancedActionKeyMapping> Map = _MappingContext->GetMappings();

	for (FEnhancedActionKeyMapping& Action : Map)
	{
		FString Name = Action.Action->GetName();

		MappingActions.Add(Name, Action.Action);
	}

	// 기존에 매핑된 키입력 다 지웁니다.
	InputSystem->ClearAllMappings();
	// 모든 입력순서중 가장 먼저 처리되는 입력으로 보겠다.
	InputSystem->AddMappingContext(_MappingContext, 0);
}

void ABasePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	SetupInputComponentEvent();
}