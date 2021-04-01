// Copyright 2020 Testy

#pragma once

#include "CoreMinimal.h"
#include "Runtime/Online/HTTP/Public/Http.h"
#include "HttpService.generated.h"

/**
 * 
 */
UCLASS()
class VRCLASSROOM_API UHttpService : public UObject
{
	GENERATED_BODY()
	
	FHttpModule* Http;

public:

	UHttpService(const class FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "API")
		FString AuthToken;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "API")
		FString VRCAPIPath = FString(TEXT("http://localhost:1337/"));

	UHttpService* GetHttp();

	void SetAuthToken(FString token);

	FString GetAuthToken();

	void SetTimeout(float Timeout);

	bool ResponseIsValid(FHttpResponsePtr Response, bool bWasSuccessful);

	void SetRequestHeaders(TSharedRef<IHttpRequest>& Request);

	TSharedRef<IHttpRequest> RequestWithRoute(FString Subroute);

	TSharedRef<IHttpRequest> PostRequest(FString Subroute, FString ContentJsonString);

	TSharedRef<IHttpRequest> GetRequest(FString Subroute);

	TSharedRef<IHttpRequest> AuthPostRequest(FString Subroute, FString ContentJsonString);

	TSharedRef<IHttpRequest> AuthGetRequest(FString Subroute);
};
