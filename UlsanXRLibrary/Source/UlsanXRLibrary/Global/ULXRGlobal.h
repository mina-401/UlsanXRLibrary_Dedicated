// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Global/ULXRConst.h"
#include "ULXRGlobal.generated.h"

/**
 * 
 */
UCLASS()
class ULSANXRLIBRARY_API UULXRGlobal : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	

public:

	static TArray<FAssetData> AssetsPath(UClass* _Class);
	static void AssetPackagePath(UClass* _Class, const FString& _AssetName, FString& _Path);
};
