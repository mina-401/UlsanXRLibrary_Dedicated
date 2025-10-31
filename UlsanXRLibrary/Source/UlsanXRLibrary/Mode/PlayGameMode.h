// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Global/BaseGameMode.h"
#include "PlayGameMode.generated.h"

/**
 * 
 */
UCLASS()
class ULSANXRLIBRARY_API APlayGameMode : public ABaseGameMode
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	virtual void Logout(AController* Exiting) override;

	
	
public:
	APlayGameMode();	

	~APlayGameMode() {}



private:
	UFUNCTION()
	void StartPlaySession();
	bool bPlaySessionStarted = false;

	void ShutdownServer();
	void CheckEmptyAndMaybeShutdown();

private:
	FTimerHandle EmptyCheckTimerHandle;
	float EmptyCheckInterval = 5.0f; // 2�ʸ��� üũ (�ʿ�� ����)
	int32 EmptyGraceSeconds = 3;     // �� ���� �������� Ȯ��(�׷��̽�)
	int32 EmptyCount = 0;
};
