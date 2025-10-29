// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Mode/UI/TitleUserWidget.h"
#include "TitleLobby.generated.h"

/**
 * 
 */
UCLASS()
class ULSANXRLIBRARY_API UTitleLobby : public UTitleUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	void OpenServer();
	
	UFUNCTION(BlueprintCallable)
	void JoinServer();
};
