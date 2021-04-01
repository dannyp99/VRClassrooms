// Copyright 2020 Testy

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "HttpService.h"
#include "VRCGameMode.generated.h"

USTRUCT(BlueprintType, Blueprintable)
struct FUserPermissions {
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Permissions")
		bool bCanShareScreen;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Permissions")
		bool bCanWriteOnBoard;
};

/**
 * 
 */
UCLASS()
class VRCLASSROOM_API AVRCGameMode : public AGameMode
{
	GENERATED_BODY()

		UHttpService* Http;

protected:

	FString InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId, const FString& Options, const FString& Portal);

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "API")
		FString ServerToken;

	AVRCGameMode();

	UFUNCTION(BlueprintCallable, Category = "API")
		void LoginSeverUser(FString UserName, FString Password);

	void OnLoginSeverUser(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	UFUNCTION(BlueprintImplementableEvent, Category = "API")
		void NotifyLoginSeverUser(const FString& Token);

	UFUNCTION(BlueprintCallable, Category = "Engine", meta = (HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject", Keywords = "Shut Down"))
	void ShutDown();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Connection")
		float EndTimeout;

	UFUNCTION(BlueprintCallable, Category = "API")
		void RemoveInstanceFromAPI(const FString& route, const FString& IP, const FString& PORT);

	UFUNCTION(BlueprintCallable, Category = "API")
		void RemoveHubFromAPI(const FString& IP, const FString& PORT);

	UFUNCTION(BlueprintCallable, Category = "API")
		void RemoveMeetingFromAPI(const FString& IP, const FString& PORT);

	void OnRemoveInstanceFromAPI(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	UFUNCTION(BlueprintImplementableEvent, Category = "API")
	void NotifyRemoveInstanceFromAPI();

	UFUNCTION(BlueprintCallable, Category = "API")
		void GetMeetingData(int32 MeetingId);

	void OnGetMeetingData(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	UFUNCTION(BlueprintImplementableEvent, Category = "API")
		void NotifyGetMeetingData(const FString& MeetingName, int creatorId);
	
};
