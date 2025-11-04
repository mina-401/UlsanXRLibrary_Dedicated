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
protected:
    // Voice 세션 ID (클라이언트에게 전달용)
    UPROPERTY()
    FString VoiceSessionId;

private:
    // Online Interfaces
    IOnlineSessionPtr SessionInterface;

public:

};
