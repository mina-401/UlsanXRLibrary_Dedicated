// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Global/BaseGameMode.h"
#include "TitleGameMode.generated.h"

/**
 * 
 */
UCLASS()
class ULSANXRLIBRARY_API ATitleGameMode : public ABaseGameMode
{
	GENERATED_BODY()
public:


	UPROPERTY(EditAnywhere,BlueprintReadOnly, Category = "Level")
	bool bIsSessionLevel = true;
protected:


};
