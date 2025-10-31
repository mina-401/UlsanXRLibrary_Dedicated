// Fill out your copyright notice in the Description page of Project Settings.


#include "Mode/PlayGameMode.h"
#include <BaseGameInstance.h>
#include "GameFramework/GameState.h" // �߰�

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
        // ���������� �÷��̾� �� üũ ����
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

    // ���� ���� ���� ���� Ȯ��
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

    // �ƹ��� ������ ���� ����
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

        if (EmptyCount >= 3) // 3�� ���� �ƹ��� ������ ����
        {
            UE_LOG(LogTemp, Warning, TEXT("[PlayGM] Empty for 3 checks. Shutting down."));
            ShutdownServer();
        }
    }
    else
    {
        EmptyCount = 0; // �ٽ� ������ ������ ī��Ʈ ����
    }
}
void APlayGameMode::ShutdownServer()
{
    UE_LOG(LogTemp, Error, TEXT("[PlayGM] >>> PLAY SERVER SHUTDOWN <<<"));
    FGenericPlatformMisc::RequestExit(false);  // ���� ����
}