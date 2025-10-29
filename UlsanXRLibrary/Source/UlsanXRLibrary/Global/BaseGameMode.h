// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "Global/ULXREnum.h"
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
protected:
	virtual void PostLogin(APlayerController* NewPlayer);
	
};
