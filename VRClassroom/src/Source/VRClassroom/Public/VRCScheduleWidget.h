// Copyright 2020 Testy

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HttpService.h"
#include "VRCGameInstance.h"
#include "VRCScheduleWidget.generated.h"


USTRUCT(BlueprintType)
struct FMeetingInfo {

	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadOnly)
		int32 id;

	UPROPERTY(BlueprintReadOnly)
		int32 map_id;
	UPROPERTY(BlueprintReadOnly)
		FString name;
	UPROPERTY(BlueprintReadOnly)
		FString password;
	UPROPERTY(BlueprintReadOnly)
		int32 repeating;
	UPROPERTY(BlueprintReadOnly)
		int32 active;
	UPROPERTY(BlueprintReadOnly)
		int32 creator_id;
};
/**
 * 
 */
UCLASS()
class VRCLASSROOM_API UVRCScheduleWidget : public UUserWidget
{
	GENERATED_BODY()

	UHttpService* Http;

public:

	UPROPERTY(BlueprintReadWrite, Category = "Meetings")
	TArray<FMeetingInfo> meetings;

	UFUNCTION(BlueprintCallable, Category = "Meetings")
		void GetMeetings();

	void OnGetMeetingsResponseRecieved(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	UFUNCTION(BlueprintImplementableEvent, Category = "Meetings")
		void NotifyGetMeetings();
};
