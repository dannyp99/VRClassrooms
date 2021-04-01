// Copyright 2020 Testy

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HttpService.h"
#include "VRCGameInstance.h"
#include "VRCLoginWidget.generated.h"

/**
 * 
 */
UCLASS()
class VRCLASSROOM_API UVRCLoginWidget : public UUserWidget
{
	GENERATED_BODY()
	
	UHttpService* Http;

public:

	UVRCLoginWidget(const class FObjectInitializer& ObjectInitializer);

	UPROPERTY(BlueprintReadWrite, Category = "Config")
		FString VRCAPIPath = "";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Login")
		float LoginTimeout = 20.f;

	//LoginAndCreateSession
	UFUNCTION(BlueprintCallable, Category = "Login")
		void LoginAndCreateSession(FString Username, FString Password);

	void OnLoginAndCreateSessionResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	UFUNCTION(BlueprintImplementableEvent, Category = "Login")
		void NotifyLoginAndCreateSession(const FString& Token);

	UFUNCTION(BlueprintImplementableEvent, Category = "Login")
		void ErrorLoginAndCreateSession(const FString& ErrorMsg);
};
