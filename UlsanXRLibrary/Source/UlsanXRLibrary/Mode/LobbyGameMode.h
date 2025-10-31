// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Global/BaseGameMode.h"

#include "LobbyGameMode.generated.h"

/**
 * 
 */
UCLASS()
class ULSANXRLIBRARY_API ALobbyGameMode : public ABaseGameMode
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
public:
	ALobbyGameMode();
	
	~ALobbyGameMode() {}

	UFUNCTION()
	void StartLobbySession();
};
