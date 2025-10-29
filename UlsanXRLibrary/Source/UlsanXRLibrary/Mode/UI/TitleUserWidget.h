// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Global/ULXREnum.h"
#include "Global/BaseGameInstance.h"
#include "TitleUserWidget.generated.h"

/**
 * 
 */
UCLASS()
class ULSANXRLIBRARY_API UTitleUserWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	void NativeConstruct();

	UFUNCTION(BlueprintCallable)
	void MainWidgetInit();

	bool IsChild(UTitleUserWidget* _Top);

	UFUNCTION(BlueprintCallable)
	void WidgetTopView(UTitleUserWidget* _Top);

	UFUNCTION(BlueprintCallable)
	void CreateChildWidget(ETitleUIType _Type, TSubclassOf<UUserWidget> _Widget, int _ZOrder);

	void SetMainWidget(class UTitleUserWidget* _MainWidget)
	{
		MainWidget = _MainWidget;
	}
	UFUNCTION(BlueprintCallable)
	class UTitleUserWidget* GetMainWidget()
	{
		return MainWidget;
	}
	UFUNCTION(BlueprintCallable)
	class UTitleUserWidget* GetWidget(ETitleUIType _Tpye)
	{
		return Wigets[_Tpye];
	}

	UFUNCTION(BlueprintCallable)
	void SetZOrder(int _Order);

	UFUNCTION(BlueprintCallable)
	void VisibleChangeUI(ETitleUIType _Type, ESlateVisibility _Value);

	// 서버 연결 함수
	UFUNCTION(BlueprintCallable)
	void StartServer();

	UFUNCTION(BlueprintCallable)
	void Connect();



	UFUNCTION(BlueprintCallable)
	virtual void CheckInput(const FString& _IP);



private:
	UPROPERTY(Category = "UI", EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UTitleUserWidget* MainWidget;

	class UCanvasPanel* CanvasPanel;
	class UTitleUserWidget* CurWidget;

	UPROPERTY()
	TMap<ETitleUIType, UTitleUserWidget*> Wigets;

	FSocket* Socket;
protected:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Server", meta = (AllowPrivateAccess = "true"))
	bool bIPIsValid = false;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Server", meta = (AllowPrivateAccess = "true"))
	FString IP = TEXT("127.0.0.1");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Server", meta = (AllowPrivateAccess = "true"))
	FString Port = TEXT("30000");

};
