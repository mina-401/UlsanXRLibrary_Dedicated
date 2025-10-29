// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include <ULXREnum.h>
#include "BaseAnimInstance.generated.h"


/**
 * 
 */
UCLASS()
class ULSANXRLIBRARY_API UBaseAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
public:

	void NativeBeginPlay() override;

	void NativeUpdateAnimation(float DeltaSeconds) override;

	void ChangeAnimation(EPlayerAnimation _CurAnimationType, FName _SectionName);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void ChangeAnimationEvent(UAnimMontage* _Montage, FName _SectionName);

protected:
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "Play")
	USkeletalMeshComponent* SkeletalMeshComponent = nullptr;

	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Play")
	USkeletalMeshComponent* FPSSkeletalMeshComponent = nullptr;

private:
	EPlayerAnimation CurAnimationType = EPlayerAnimation::Idle;

	FName SectionName = TEXT("");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Play", meta = (AllowPrivateAccess = "true"))
	TMap<EPlayerAnimation, UAnimMontage*> AnimMontages;

	UAnimMontage* CurMontage;
};
