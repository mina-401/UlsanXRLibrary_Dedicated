// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ULXRConst.generated.h"

/**
 * 
 */
UCLASS()
class ULSANXRLIBRARY_API UULXRConst : public UObject
{
	GENERATED_BODY()
public:
	class UI
	{
	public:
		static FName TitleMainPanelName;
		static FName PlayMainPanelName;
	};


	class Level
	{
	public:
		static FString PlayLevelName;
		//static FString 01PlayLevelName;
		static FString TitleLevelName;
		static FString EndLevelName;
	};

	class Title
	{
	public:
		static int MaxPlayer;
	};

	class Path
	{
	public:
		static FString GlobalDataTablePath;
	};

	class Collision
	{
	public:
		static FName ProfileName_Player;
		static FName ProfileName_Item;
		static FName ProfileName_WidgetInter;
		static FName ProfileName_Rider;
	};

	class Socket
	{
	public:
		static FName RiderSocket;
	};
};
