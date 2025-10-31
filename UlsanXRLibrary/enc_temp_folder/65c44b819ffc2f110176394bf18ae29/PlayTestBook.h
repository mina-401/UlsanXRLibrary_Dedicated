// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Mode/UI/TitleUserWidget.h"
#include "PlayTestBook.generated.h"

/**
 * 
 */
UCLASS()
class ULSANXRLIBRARY_API UPlayTestBook : public UTitleUserWidget
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintCallable)
	void OnCreateLobbyClicked();
	UFUNCTION(BlueprintCallable)
	void OnJoinLobbyClicked();
	
	
};
