// Fill out your copyright notice in the Description page of Project Settings.


#include "Mode/UI/TitleLobby.h"

void UTitleLobby::OpenServer()
{
    

    if (UBaseGameInstance* BaseGI = Cast<UBaseGameInstance>(GetGameInstance()))
    {
        // 필요 시 인원 수를 UI에서 받아와 전달하세요.
        BaseGI->HostLobby(10);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("OpenServer: GameInstance is not UBaseGameInstance"));
    }
}

void UTitleLobby::JoinServer()
{
    if (UBaseGameInstance* BaseGI = Cast<UBaseGameInstance>(GetGameInstance()))
    {
        // 필요 시 인원 수를 UI에서 받아와 전달하세요.
        BaseGI->FindLobbies();
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("JoinServer: GameInstance is not UBaseGameInstance"));
    }
}
