// Fill out your copyright notice in the Description page of Project Settings.


#include "Global/ULXRConst.h"

// UI 패널 이름
FName UULXRConst::UI::TitleMainPanelName = TEXT("MainCanvasPanel");
FName UULXRConst::UI::PlayMainPanelName = TEXT("MainCanvasPanel");

// 입장할 방 이름
FString UULXRConst::Level::PlayLevelName = TEXT("PlayLevel");
//FString UULXRConst::Level::PlayLevelName = TEXT("01PlayLevel");
FString UULXRConst::Level::TitleLevelName = TEXT("TitleLevel");
FString UULXRConst::Level::EndLevelName = TEXT("EndLevel");

//콜리전 프로파일
FName UULXRConst::Collision::ProfileName_Player = TEXT("Player");
FName UULXRConst::Collision::ProfileName_Item = TEXT("Item");
FName UULXRConst::Collision::ProfileName_WidgetInter = TEXT("WidgetInter");
FName UULXRConst::Collision::ProfileName_Rider = TEXT("Rider");

//콜리전 프로파일
FName UULXRConst::Socket::RiderSocket = TEXT("RiderSocket");

//DatatablePath
FString UULXRConst::Path::GlobalDataTablePath =TEXT("/Game/BP/Global/Data/DT_GlobalDataTable.DT_GlobalDataTable");

// Title -> Play 이동 
int UULXRConst::Title::MaxPlayer = 2;

;