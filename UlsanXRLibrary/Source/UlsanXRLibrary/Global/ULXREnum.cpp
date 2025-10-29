// Fill out your copyright notice in the Description page of Project Settings.


#include "Global/ULXREnum.h"

TArray<uint8> UULXREnum::GetAllValue(UEnum* _Enum)
{
	int64 MaxCount = _Enum->NumEnums() - 1;

	// _Enum->GetValueAsString()

	TArray<uint8> ArrEnum;
	ArrEnum.Reserve(MaxCount);
	for (int i = 0; i < MaxCount; i++)
	{
		int64 Value = _Enum->GetValueByIndex(i);
		ArrEnum.Add(Value);
	}
	return ArrEnum;
}