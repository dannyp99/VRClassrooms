// Copyright 2020 Testy


#include "VRCLoginWidget.h"
#include "Runtime/Core/Public/Misc/ConfigCacheIni.h"
#include "HttpService.h"
#include "VRCGameInstance.h"

UVRCLoginWidget::UVRCLoginWidget(const class FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	
}

void UVRCLoginWidget::LoginAndCreateSession(FString Username, FString Password)
{
	if (Http == NULL) {
		UVRCGameInstance* GameInstance = Cast<UVRCGameInstance>(GetWorld()->GetGameInstance());
		Http = GameInstance->GetHttpService();
	}
	Http->SetTimeout(LoginTimeout); //Set timeout

	FString UsernameA = FPlatformMisc::GetEnvironmentVariable(TEXT("UEServerUserName"));
	FString PasswordA = FPlatformMisc::GetEnvironmentVariable(TEXT("UEServerPassword"));

	UE_LOG(LogTemp, Warning, TEXT("User: %s, Pass: %s"), *UsernameA, *PasswordA);

	//Trim whitespace
	Username.TrimStartAndEndInline();
	Password.TrimStartAndEndInline();

	FString PostParameters = FString(TEXT("username=")) + Username
		+ FString(TEXT("&password=")) + Password;

	TSharedRef<IHttpRequest> Request = Http->PostRequest("login", PostParameters);
	Request->OnProcessRequestComplete().BindUObject(this, &UVRCLoginWidget::OnLoginAndCreateSessionResponseReceived);

	Request->SetHeader("Content-Type", TEXT("application/x-www-form-urlencoded"));
	Request->ProcessRequest();
}

void UVRCLoginWidget::OnLoginAndCreateSessionResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	FString content = *Response->GetContentAsString();
	if (Http->ResponseIsValid(Response, bWasSuccessful))
	{
		NotifyLoginAndCreateSession(content);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("OnUserLoginResponseReceived Error accessing login server!"));
		ErrorLoginAndCreateSession(content);
	}
}

