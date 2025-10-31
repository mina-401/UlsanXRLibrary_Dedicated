// Fill out your copyright notice in the Description page of Project Settings.


#include "Mode/PlayPlayerController.h"
#include "Global/BaseGameInstance.h"
#include "PlayPlayerController.h"



//void APlayPlayerController::Server_RequestStartBookTravel_Implementation()
//{
//
//	UBaseGameInstance* BaseGI = Cast<UBaseGameInstance>(GetGameInstance());
//
//	if (BaseGI)
//	{
//		BaseGI->StartBookTravel(this);
//	}
//}

void APlayPlayerController::RequestStartBookTravel()
{
	UBaseGameInstance* BaseGI = Cast<UBaseGameInstance>(GetGameInstance());

	if (BaseGI)
	{
		BaseGI->StartBookTravel(this);
	}
}
