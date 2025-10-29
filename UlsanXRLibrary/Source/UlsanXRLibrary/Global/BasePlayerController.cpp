// Fill out your copyright notice in the Description page of Project Settings.


#include "Global/BasePlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"

ABasePlayerController::ABasePlayerController()
{
}

void ABasePlayerController::AddMappingContext(UInputMappingContext* _MappingContext)
{

	if (nullptr == GetLocalPlayer())
	{
		return;
	}

	UEnhancedInputLocalPlayerSubsystem* InputSystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());


	// 컨트롤러의 내부에서는 
	TArray<FEnhancedActionKeyMapping> Map = _MappingContext->GetMappings();

	for (FEnhancedActionKeyMapping& Action : Map)
	{
		FString Name = Action.Action->GetName();

		MappingActions.Add(Name, Action.Action);
	}

	// 기존에 매핑된 키입력 다 지웁니다.
	InputSystem->ClearAllMappings();
	// 모든 입력순서중 가장 먼저 처리되는 입력으로 보겠다.
	InputSystem->AddMappingContext(_MappingContext, 0);
}

void ABasePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	SetupInputComponentEvent();
}