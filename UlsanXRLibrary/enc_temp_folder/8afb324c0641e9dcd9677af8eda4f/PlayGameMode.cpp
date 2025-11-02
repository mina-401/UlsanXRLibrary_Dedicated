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
            &APlayGameMode::ShutdownServer,
            EmptyCheckInterval,
            true,
            20.0f
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

    int32 CurrentPlayers = GetNumPlayers();

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
   
}
void APlayGameMode::ShutdownServer()
{
  

    int32 CurrentPlayers = GetNumPlayers();
    UE_LOG(LogTemp, Warning, TEXT("[PlayGM] Player count : %d"),CurrentPlayers);
    if (CurrentPlayers <= 0)
    {
        UE_LOG(LogTemp, Error, TEXT("[PlayGM] >>> PLAY SERVER SHUTDOWN <<<"));
        FGenericPlatformMisc::RequestExitWithStatus(false, 0);
    }
}