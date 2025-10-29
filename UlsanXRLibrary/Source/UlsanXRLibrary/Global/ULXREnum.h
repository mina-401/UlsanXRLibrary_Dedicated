// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ULXREnum.generated.h"

/**
 * 
 */
UENUM(BlueprintType)
enum class ECursorColorType : uint8
{
	Red UMETA(DisplayName = "빨강 플레이어"),
	Yellow UMETA(DisplayName = "노랑 플레이어"),
	Green UMETA(DisplayName = "초록 플레이어"),
	Blue UMETA(DisplayName = "파랑 플레이어"),

	MAX UMETA(DisplayName = "MAX"),
};


UENUM(BlueprintType)
enum class EPuzzleGameUIType : uint8
{
	WordLink UMETA(DisplayName = "단어 잇기"),
	WordBlock UMETA(DisplayName = "퍼즐 맞추기"),
	WordBlank UMETA(DisplayName = "빈칸 맞추기"),
	
	
	
	
	
	MAX UMETA(DisplayName = "MAX"),
};

UENUM(BlueprintType)
enum class EPlayerRoom : uint8
{
	Lobby UMETA(DisplayName = "로비"),
	Book UMETA(DisplayName = "책"),
	MAX UMETA(DisplayName = "MAX"),
};

UENUM(BlueprintType)
enum class EPlayerAnimation : uint8
{
	Idle UMETA(DisplayName = "서있기"),
	Walk UMETA(DisplayName = "걷기"),
	MAX UMETA(DisplayName = "MAX"),
};

UENUM(BlueprintType)
enum class ETitleUIType : uint8
{
	TitleRoom UMETA(DisplayName = "방입장"),
	TitleMain UMETA(DisplayName = "메인화면"),
	TitleServer UMETA(DisplayName = "서버오픈"),
	Ready UMETA(DisplayName = "대기화면"),
	PlayBlank UMETA(DisplayName = "빈칸퍼즐"),
	
	MAX UMETA(DisplayName = "MAX"),
};


UCLASS()
class ULSANXRLIBRARY_API UULXREnum : public UObject
{
	GENERATED_BODY()

public:
	// UClass와는 다른
	// UEnum이라는 객체를 미리 조사해서 만듭니다

	// 언리얼에서 블루프린트에서 템플릿은 사용이 불가능합니다.
	template<typename EnumType>
	static TArray<EnumType> GetAllEnum()
	{
		TArray<EnumType> Result;
		UEnum* Enum = StaticEnum<EnumType>();
		TArray<uint8> Values = GetAllValue(Enum);

		for (size_t i = 0; i < Values.Num(); i++)
		{
			Result.Add(static_cast<EnumType>(Values[i]));
		}
		return Result;
	}

	static TArray<uint8> GetAllValue(UEnum* _Enum);
};
