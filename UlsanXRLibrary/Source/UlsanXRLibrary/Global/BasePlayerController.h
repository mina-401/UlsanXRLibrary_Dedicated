// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputMappingContext.h"
#include "BasePlayerController.generated.h"

/**
 * 
 */
UCLASS()
class ULSANXRLIBRARY_API ABasePlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	ABasePlayerController();


	UFUNCTION(BlueprintCallable)
	void AddMappingContext(UInputMappingContext* MappingContext);



	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void SetupInputComponentEvent();
protected:


	void SetupInputComponent() override;


private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* MappingContext = nullptr;

	// 관리 => 자료구조
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TMap<FString, const UInputAction*> MappingActions;
};
