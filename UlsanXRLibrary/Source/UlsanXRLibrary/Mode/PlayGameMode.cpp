// Fill out your copyright notice in the Description page of Project Settings.


#include "Mode/PlayGameMode.h"
#include <BaseGameInstance.h>
#include "GameFramework/GameState.h" // 추가

APlayGameMode::APlayGameMode()
{
    PlayerRoom = EPlayerRoom::Book;

    //PlayerControllerClass = APlayPlayerController::StaticClass();
}

void APlayGameMode::BeginPlay()
{
    Super::BeginPlay();

    if (IsRunningDedicatedServer())
    {
        StartPlaySession();
    }

    if (IsRunningDedicatedServer())
    {
        // 정기적으로 플레이어 수 체크 시작
        GetWorld()->GetTimerManager().SetTimer(
            EmptyCheckTimerHandle,
            this,
            &APlayGameMode::CheckEmptyAndMaybeShutdown,
            EmptyCheckInterval,
            true
        );

        UE_LOG(LogTemp, Log, TEXT("[PlayGM] Started empty-check timer (interval=%.2f)"), EmptyCheckInterval);
    }
}

void APlayGameMode::StartPlaySession()
{
    if (bPlaySessionStarted)
    {
        UE_LOG(LogTemp, Log, TEXT("[PlayGM] Play session already started (guard)"));
        return;
    }

    UWorld* World = GetWorld();
    if (!World) { UE_LOG(LogTemp, Warning, TEXT("[PlayGM] World is null")); return; }

    UBaseGameInstance* GI = Cast<UBaseGameInstance>(World->GetGameInstance());
    if (!GI) { UE_LOG(LogTemp, Warning, TEXT("[LobbyGM] GameInstance is not UBaseGameInstance")); return; }

    // 기존 세션 존재 여부 확인
    IOnlineSubsystem* OSS = IOnlineSubsystem::Get();
    IOnlineSessionPtr SI = OSS ? OSS->GetSessionInterface() : nullptr;
    if (SI.IsValid() && SI->GetNamedSession(FName("PlaySession")) != nullptr)
    {
        UE_LOG(LogTemp, Log, TEXT("[PlayGM] PlaySession already exists on OSS"));
        bPlaySessionStarted = true;
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("[PlayGM] Calling HostBook()"));
    bPlaySessionStarted = true;
    GI->HostLobby(TEXT("PlayLevel"));
}
void APlayGameMode::Logout(AController* Exiting)
{
    Super::Logout(Exiting);

    int32 CurrentPlayers = 0;
    if (AGameState* GS = GetGameState<AGameState>())
    {
        CurrentPlayers = GS->PlayerArray.Num();
    }

    UE_LOG(LogTemp, Warning, TEXT("[PlayGM] Player left. Current Players: %d"), CurrentPlayers);

    // 아무도 없으면 서버 종료
    if (CurrentPlayers <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("[PlayGM] No players left. Closing Play Server in 3 seconds..."));

        FTimerHandle Timer;
        GetWorld()->GetTimerManager().SetTimer(Timer, this, &APlayGameMode::ShutdownServer, 3.0f, false);
    }
}
void APlayGameMode::CheckEmptyAndMaybeShutdown()
{
    int32 CurrentPlayers = GetNumPlayers();

    if (CurrentPlayers <= 0)
    {
        EmptyCount++;

        if (EmptyCount >= 3) // 3초 동안 아무도 없으면 종료
        {
            UE_LOG(LogTemp, Warning, TEXT("[PlayGM] Empty for 3 checks. Shutting down."));
            ShutdownServer();
        }
    }
    else
    {
        EmptyCount = 0; // 다시 누군가 있으면 카운트 리셋
    }
}
void APlayGameMode::ShutdownServer()
{
    UE_LOG(LogTemp, Error, TEXT("[PlayGM] >>> PLAY SERVER SHUTDOWN <<<"));
    FGenericPlatformMisc::RequestExit(false);  // 정상 종료
}