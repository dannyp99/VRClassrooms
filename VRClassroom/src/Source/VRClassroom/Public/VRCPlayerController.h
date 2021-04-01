// Copyright 2020 Testy

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "HttpService.h"
#include "VRCPlayerState.h"
#include "VRCPlayerController.generated.h"

USTRUCT(BlueprintType, Blueprintable)
struct FCourse {
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Course")
		int32 CourseID;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Course")
		FString CourseName;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Course")
		FDateTime StartDate;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Course")
		FDateTime EndDate;
};

/**
 * 
 */
UCLASS()
class VRCLASSROOM_API AVRCPlayerController : public APlayerController
{
	GENERATED_BODY()

		UHttpService* Http;

public:

	AVRCPlayerController();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Connection")
		float TravelTimeout;

	UFUNCTION(BlueprintCallable, Category = "Travel")
		void TravelToMap(const FString& URL, const bool SeamlessTravel);

	UFUNCTION(BlueprintCallable, Category = "Travel")
		void TravelToMapWithState(const FString& URL, const bool SeamlessTravel);

	UFUNCTION(BlueprintCallable, Category = "Travel")
		void ConnectToHub();

	void OnConnectToHubResponseRecieved(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	UFUNCTION(BlueprintCallable, Category = "Travel")
		void ConnectToMeeting(int32 MeetingId);

	void OnConnectToMeetingResponseRecieved(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	UFUNCTION(BlueprintCallable, Category = "User")
		void GetUserData();

	void OnGetUserDataRecieved(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	AVRCPlayerState* GetVRCPlayerState() const;
	
};
