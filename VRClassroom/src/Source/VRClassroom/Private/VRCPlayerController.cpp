// Copyright 2020 Testy


#include "VRCPlayerController.h"
#include "Runtime/Online/HTTP/Public/Http.h"
#include "VRCGameInstance.h"

AVRCPlayerController::AVRCPlayerController() {

	TravelTimeout = 60.0f;
}

void AVRCPlayerController::TravelToMap(const FString& URL, const bool SeamlessTravel) {
	UE_LOG(LogTemp, Warning, TEXT("TravelToMap: %s"), *URL);
	ClientTravel(URL, TRAVEL_Absolute, false, FGuid());
}

void AVRCPlayerController::TravelToMapWithState(const FString& URL, const bool SeamlessTravel) {

	AVRCPlayerState* state = GetVRCPlayerState();
	if (!state) {
		UE_LOG(LogTemp, Error, TEXT("Invalid VRC Player State. Can't travel with state"));

		return;
	}
	FString IDData = FString::FromInt(state->UserID)
		+ "|" + FGenericPlatformHttp::UrlEncode(state->UserName)
		+ "|" + FGenericPlatformHttp::UrlEncode(state->DisplayName);
	//TODO: Encrypt IDData with AES

	FString NewUrl = URL
		+ FString(TEXT("?ID=")) + IDData;

	TravelToMap(NewUrl, SeamlessTravel);
}

void AVRCPlayerController::ConnectToHub() {

	UVRCGameInstance* GameInstance = Cast<UVRCGameInstance>(GetWorld()->GetGameInstance());
	Http = GameInstance->GetHttpService();
	FString token = GameInstance->GetUserToken();

	Http->SetTimeout(TravelTimeout);

	TSharedRef<IHttpRequest> Request = Http->GetRequest("instances/join-server");
	Request->OnProcessRequestComplete().BindUObject(this, &AVRCPlayerController::OnConnectToHubResponseRecieved);
	Request->SetHeader("Accepts", TEXT("application/json"));

	FString auth = "Bearer ";
	auth.Append(token);
	Request->SetHeader("Authorization", auth);

	Request->ProcessRequest();
}

void AVRCPlayerController::OnConnectToHubResponseRecieved(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (Http->ResponseIsValid(Response, bWasSuccessful)) {
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());

		if (FJsonSerializer::Deserialize(Reader, JsonObject))
		{
			FString IP = JsonObject->GetStringField("ip");
			FString PORT = JsonObject->GetStringField("port");

			FString URL = IP + FString(TEXT(":")) + PORT.Left(4);
			UE_LOG(LogTemp, Log, TEXT("OnConnectToHub IP is %s Port %s, Both"), *IP, *PORT, *URL);

			TravelToMapWithState(URL, true);

		}
		else {
			UE_LOG(LogTemp, Error, TEXT("OnConnectToHub Server returned no data!"));
		}
	}
	else {
		UE_LOG(LogTemp, Error, TEXT("OnConnectToHub Error accessing server!"));
	}
}


void AVRCPlayerController::ConnectToMeeting(int32 MeetingId) {

	UVRCGameInstance* GameInstance = Cast<UVRCGameInstance>(GetWorld()->GetGameInstance());
	Http = GameInstance->GetHttpService();
	FString token = GameInstance->GetUserToken();

	Http->SetTimeout(20.0f);

	FString SubRoute = "instances/join-classroom/";
	SubRoute.AppendInt(MeetingId);
	TSharedRef<IHttpRequest> Request = Http->GetRequest(SubRoute);

	FString auth = "Bearer ";
	auth.Append(token);
	Request->SetHeader("Authorization", auth);
	Request->SetHeader("Accepts", TEXT("application/json"));
	Request->OnProcessRequestComplete().BindUObject(this, &AVRCPlayerController::OnConnectToMeetingResponseRecieved);

	Request->ProcessRequest();
}

void AVRCPlayerController::OnConnectToMeetingResponseRecieved(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful) {
	if(Http->ResponseIsValid(Response, bWasSuccessful)){
		UE_LOG(LogTemp, Log, TEXT("Connecting to meeting"));

		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());

		if (FJsonSerializer::Deserialize(Reader, JsonObject))
		{
			FString IP = JsonObject->GetStringField("ip");
			FString PORT = JsonObject->GetStringField("port");

			FString URL = IP + FString(TEXT(":")) + PORT.Left(4);
			UE_LOG(LogTemp, Log, TEXT("OnConnectToMeeting IP is %s Port %s, Both %s"), *IP, *PORT, *URL);

			TravelToMapWithState(URL, true);
		}
		else {
			UE_LOG(LogTemp, Error, TEXT("OnConnectToMeeting Server returned no data!"));
		}

	}else{
		UE_LOG(LogTemp, Error, TEXT("Connect to meeting failed"));
	}
}

void AVRCPlayerController::GetUserData() {
	UVRCGameInstance* GameInstance = Cast<UVRCGameInstance>(GetWorld()->GetGameInstance());
	Http = GameInstance->GetHttpService();
	FString token = GameInstance->GetUserToken();

	Http->SetTimeout(20.0f);

	TSharedRef<IHttpRequest> Request = Http->GetRequest("users/");

	Request->SetHeader("Accepts", TEXT("application/json"));
	Request->OnProcessRequestComplete().BindUObject(this, &AVRCPlayerController::OnGetUserDataRecieved);

	FString auth = "Bearer ";
	auth.Append(token);
	Request->SetHeader("Authorization", auth);
	Request->SetHeader("Accepts", TEXT("application/json"));

	Request->ProcessRequest();
}

void AVRCPlayerController::OnGetUserDataRecieved(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful) {
	if (Http->ResponseIsValid(Response, bWasSuccessful)) {

		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());

		if (FJsonSerializer::Deserialize(Reader, JsonObject))
		{
			int UserID = JsonObject->GetIntegerField("id");
			FString UserName = JsonObject->GetStringField("username");
			FString DisplayName = JsonObject->GetStringField("display_name");

			AVRCPlayerState* playerState = GetVRCPlayerState();

			playerState->UserID = UserID;
			playerState->UserName = UserName;
			playerState->DisplayName = DisplayName;

			UE_LOG(LogTemp, Log, TEXT("OnGetUserData success"));

		}
		else {
			UE_LOG(LogTemp, Error, TEXT("OnGetUserData Server returned no data!"));
		}

	}
	else {
		UE_LOG(LogTemp, Error, TEXT("Get User Failed"));
	}
}

AVRCPlayerState* AVRCPlayerController::GetVRCPlayerState() const {
	return GetPlayerState<AVRCPlayerState>();
}
