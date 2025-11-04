// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "Global/ULXREnum.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "BaseGameMode.generated.h"

/**
 * 
 */
UCLASS()
class ULSANXRLIBRARY_API ABaseGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, Category = "Play")
	EPlayerRoom PlayerRoom = EPlayerRoom::MAX;


public:
    virtual void BeginPlay() override;
    virtual void PostLogin(APlayerController* NewPlayer) override;
    virtual void Logout(AController* Exiting) override;

protected:
    // Voice 세션 ID (클라이언트에게 전달용)
    UPROPERTY()
    FString VoiceSessionId;

private:
    // Online Interfaces
    IOnlineSessionPtr SessionInterface;

    // 세션이 Voice 활성화되어 있는지 확인
    bool IsVoiceEnabledSession() const;

    // 클라이언트에게 Voice 세션 정보 전달
    void NotifyClientVoiceSession(APlayerController* PC);
};
