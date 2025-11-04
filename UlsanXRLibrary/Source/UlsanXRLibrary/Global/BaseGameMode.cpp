// Fill out your copyright notice in the Description page of Project Settings.


#include "Global/BaseGameMode.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Global/BasePlayerController.h"

#define VOICE_LOG(Verbosity, Format, ...) UE_LOG(LogTemp, Verbosity, TEXT("[VOICE] %s: " Format), TEXT(__FUNCTION__), ##__VA_ARGS__)
void ABaseGameMode::BeginPlay()
{
    if (HasAuthority())
    {
        IOnlineSubsystem* OSS = IOnlineSubsystem::Get();
        if (OSS)
        {
            SessionInterface = OSS->GetSessionInterface();

            if (SessionInterface.IsValid())
            {
                // 현재 세션이 Voice 활성화되어 있는지 확인
                if (IsVoiceEnabledSession())
                {
                    VOICE_LOG(Log, "Voice - enabled session detected!");

                    // 세션 ID 저장
                    FNamedOnlineSession* Session = SessionInterface->GetNamedSession(NAME_GameSession);
                    if (Session && Session->SessionInfo.IsValid())
                    {
                        VoiceSessionId = Session->GetSessionIdStr();
                        VOICE_LOG(Log, "Session ID: %s", *VoiceSessionId);
                    }
                }
                else
                {
                    VOICE_LOG(Warning, "Voice is NOT enabled for this session");
                }
            }
        }
    }
}

void ABaseGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    FTimerHandle H;
    GetWorld()->GetTimerManager().SetTimer(H, FTimerDelegate::CreateLambda([this, NewPlayer]()
    {
        // 기존 스폰/포제션 로직 실행
        RestartPlayer(NewPlayer);
        FTimerHandle VoiceTimer;
        GetWorld()->GetTimerManager().SetTimer(VoiceTimer, FTimerDelegate::CreateLambda([this, NewPlayer]()
        {
            NotifyClientVoiceSession(NewPlayer);
        }), 0.5f, false);
    }), 0.15f, false);

    
}

void ABaseGameMode::Logout(AController* Exiting)
{
    Super::Logout(Exiting);

    VOICE_LOG(Log, "Player logout: %s", Exiting ? *Exiting->GetName() : TEXT("Unknown"));
}
bool ABaseGameMode::IsVoiceEnabledSession() const
{
    if (!SessionInterface.IsValid())
        return false;

    FNamedOnlineSession* Session = SessionInterface->GetNamedSession(NAME_GameSession);
    if (!Session)
        return false;

    bool bVoiceEnabled = false;
    Session->SessionSettings.Get(FName("EOSVoiceChat_Enabled"), bVoiceEnabled);

    return bVoiceEnabled;
}
void ABaseGameMode::NotifyClientVoiceSession(APlayerController* PC)
{
    if (!PC || VoiceSessionId.IsEmpty())
        return;

    // BasePlayerController로 캐스팅 시도
    ABasePlayerController* BasePC = Cast<ABasePlayerController>(PC);
    if (BasePC)
    {
        VOICE_LOG(Log,"Notifying client about voice session: %s", *VoiceSessionId);

        // RPC 호출 (BasePlayerController에 구현 필요)
        BasePC->ClientJoinVoiceSession(VoiceSessionId);
    }
}