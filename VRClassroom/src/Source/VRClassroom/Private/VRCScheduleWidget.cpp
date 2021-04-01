// Copyright 2020 Testy


#include "VRCScheduleWidget.h"
#include "HttpService.h"
#include "VRCGameInstance.h"
#include "JsonObjectConverter.h"

void UVRCScheduleWidget::GetMeetings() {

	UVRCGameInstance* GameInstance = Cast<UVRCGameInstance>(GetWorld()->GetGameInstance());
	Http = GameInstance->GetHttpService();
	FString token = GameInstance->GetUserToken();

	Http->SetTimeout(5.0f);

	TSharedRef<IHttpRequest> Request = Http->GetRequest("users/meetings");
	Request->SetHeader("Accepts", TEXT("application/json"));
	Request->OnProcessRequestComplete().BindUObject(this, &UVRCScheduleWidget::OnGetMeetingsResponseRecieved);
	FString auth = "Bearer ";
	auth.Append(token);
	Request->SetHeader("Authorization", auth);

	Request->ProcessRequest();
}

void UVRCScheduleWidget::OnGetMeetingsResponseRecieved(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful) {

	if (Http->ResponseIsValid(Response, bWasSuccessful)) {
		TArray<FMeetingInfo> responseMeetings;
		FString content = FString(Response->GetContentAsString());
		if (FJsonObjectConverter::JsonArrayStringToUStruct(content, &responseMeetings, 0, 0)) {
			meetings = responseMeetings;
			UE_LOG(LogTemp, Log, TEXT("Meetings found"));

			NotifyGetMeetings();
		}
		else {
			UE_LOG(LogTemp, Log, TEXT("No meetings found"));
		}
	}
	else {
		UE_LOG(LogTemp, Error, TEXT("GetMeetings Error accessing server!"));
	}
}