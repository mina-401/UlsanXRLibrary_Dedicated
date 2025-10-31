// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Global/BasePlayerController.h"
#include "PlayPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class ULSANXRLIBRARY_API APlayPlayerController : public ABasePlayerController
{
	GENERATED_BODY()
	
	
public:
	// ���� RPC
	UFUNCTION(/*Server, Reliable,*/BlueprintCallable)
	void RequestStartBookTravel();

	UFUNCTION(/*Server, Reliable,*/BlueprintCallable)
	void JoinLobby();

	UFUNCTION(BlueprintCallable)
	void OnClickHostLobby();

	UFUNCTION(BlueprintCallable)
	void OnClickJoinLobby();
	
	//void Server_RequestStartBookTravel_Implementation();
	
};
