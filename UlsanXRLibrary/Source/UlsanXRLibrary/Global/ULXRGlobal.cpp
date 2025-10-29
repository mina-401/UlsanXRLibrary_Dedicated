// Fill out your copyright notice in the Description page of Project Settings.


#include "Global/ULXRGlobal.h"
#include "AssetRegistry/AssetRegistryModule.h"

void UULXRGlobal::AssetPackagePath(UClass* _Class, const FString& _AssetName, FString& _Path)
{
	TArray<FAssetData> AllAsset = AssetsPath(_Class);

	FName CheckName = FName(*_AssetName);

	for (const FAssetData& AssetData : AllAsset)
	{
		if (AssetData.AssetName == CheckName)
		{
			// 패키징 됐을때도 사용할수 
			_Path = AssetData.PackageName.ToString();
			return;
		}
	}
}

TArray<FAssetData> UULXRGlobal::AssetsPath(UClass* _Class)
{
	// 언리얼에 존재하는 모듈들을 관리해주는 스트럭트
	// FModuleManager
	TArray<FAssetData> MapList;
	// "Engine"
	if (false == FModuleManager::Get().IsModuleLoaded("AssetRegistry"))
	{
		return MapList;
	}

	IAssetRegistry& AssetRegistry = FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>("AssetRegistry").Get();

	// 에셋이 로딩중이니?
	if (AssetRegistry.IsLoadingAssets())
	{
		return MapList;
	}

	// UWorld

	if (AssetRegistry.GetAssetsByClass(_Class->GetClassPathName(), MapList))
	{
		return MapList;
	}

	return MapList;
}