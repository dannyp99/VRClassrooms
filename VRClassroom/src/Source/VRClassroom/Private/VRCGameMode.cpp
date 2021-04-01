// Copyright 2020 Testy


#include "VRCGameMode.h"
#include "HttpService.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "VRCPlayerState.h"
#include "VRCGameInstance.h"

AVRCGameMode::AVRCGameMode() {
	EndTimeout = 10.0f;
}

FString AVRCGameMode::InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId, const FString& Options, const FString& Portal)
{
	FString retString = Super::InitNewPlayer(NewPlayerController, UniqueId, Options, Portal);

	UE_LOG(LogTemp, Warning, TEXT("InitNewPlayer Started"));

	FString SUserID = "";
	FString SUserName = "";
	FString SDisplayName = "";

	FString IDData = UGameplayStatics::ParseOption(Options, TEXT("ID"));

	if (!IDData.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("Raw options: %s"), *IDData);

		FString DecodedIDData = FGenericPlatformHttp::UrlDecode(IDData);

		UE_LOG(LogTemp, Warning, TEXT("Decoded options: %s"), *DecodedIDData);

		TArray<FString> SplitArray;
		DecodedIDData.ParseIntoArray(SplitArray, TEXT("|"), false);

		SUserID = SplitArray[0];
		SUserName = SplitArray[1];
		SDisplayName = SplitArray[2];

		UE_LOG(LogTemp, Warning, TEXT("PlayerName: %s"), *SUserName);

	}
	else {
		SUserName = "testy";
	}
	AVRCPlayerState* NewPlayerState = CastChecked<AVRCPlayerState>(NewPlayerController->PlayerState);

	if (!SUserID.IsEmpty() && !SDisplayName.IsEmpty()) {
		NewPlayerState->UserID = FCString::Atoi(*SUserID);
		NewPlayerState->DisplayName = SDisplayName;
	}
	else {
		NewPlayerState->UserID = 111;
		NewPlayerState->DisplayName = "Testy";
	}

	NewPlayerState->UserName = SUserName;

	return retString;
}

void AVRCGameMode::LoginSeverUser(FString UserName, FString Password) {

	if (Http == NULL) {
		UVRCGameInstance* GameInstance = Cast<UVRCGameInstance>(GetWorld()->GetGameInstance());
		Http = GameInstance->GetHttpService();
	}
	Http->SetTimeout(20.0f); //Set timeout


	UE_LOG(LogTemp, Warning, TEXT("User: %s, Pass: %s"), *UserName, *Password);
	if (UserName.IsEmpty() || Password.IsEmpty()) {
		UE_LOG(LogTemp, Error, TEXT("Server Login Credentials not set correctly"));
		return;
	}

	//Trim whitespace
	UserName.TrimStartAndEndInline();
	Password.TrimStartAndEndInline();

	FString PostParameters = FString(TEXT("username=")) + UserName
		+ FString(TEXT("&password=")) + Password;

	TSharedRef<IHttpRequest> Request = Http->PostRequest("login", PostParameters);
	Request->OnProcessRequestComplete().BindUObject(this, &AVRCGameMode::OnLoginSeverUser);

	Request->SetHeader("Content-Type", TEXT("application/x-www-form-urlencoded"));
	Request->ProcessRequest();
}

void AVRCGameMode::OnLoginSeverUser(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful) {
	FString content = *Response->GetContentAsString();
	if (Http->ResponseIsValid(Response, bWasSuccessful))
	{
		UE_LOG(LogTemp, Warning, TEXT("OnLoginSeverUser Success!"));
		NotifyLoginSeverUser(content);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("OnLoginSeverUser Error accessing login server!"));
	}
}

void AVRCGameMode::ShutDown() {
	GIsRequestingExit = true;
}

void AVRCGameMode::RemoveInstanceFromAPI(const FString& route, const FString& IP, const FString& PORT) {
	UVRCGameInstance* GameInstance = Cast<UVRCGameInstance>(GetWorld()->GetGameInstance());
	Http = GameInstance->GetHttpService();
	FString token = GameInstance->GetUserToken();

	Http->SetTimeout(EndTimeout);

	TArray<FStringFormatArg> args;
	args.Add(FStringFormatArg(IP));
	args.Add(FStringFormatArg(PORT));
	FString content = FString::Format(TEXT("{\"ip\":\"{0}\", \"port\":\"{1}\"}"), args);
	UE_LOG(LogTemp, Log, TEXT("Content: %s"), *content);
	TSharedRef<IHttpRequest> Request = Http->PostRequest(route, content);
	Request->OnProcessRequestComplete().BindUObject(this, &AVRCGameMode::OnRemoveInstanceFromAPI);
	Request->SetHeader("Content-Type", TEXT("application/json"));

	FString auth = "Bearer ";
	auth.Append(token);
	Request->SetHeader("Authorization", auth);

	Request->ProcessRequest();
}


void AVRCGameMode::OnRemoveInstanceFromAPI(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful) {
	if (Http->ResponseIsValid(Response, bWasSuccessful)) {
		NotifyRemoveInstanceFromAPI();
	}
	else {
		UE_LOG(LogTemp, Error, TEXT("OnRemoveInstanceFromAPI Error removing from API!"));
	}
}

void AVRCGameMode::RemoveHubFromAPI(const FString& IP, const FString& PORT) {
	RemoveInstanceFromAPI(TEXT("instances/remove-hub"), IP, PORT);
}

void AVRCGameMode::RemoveMeetingFromAPI(const FString& IP, const FString& PORT) {
	RemoveInstanceFromAPI(TEXT("instances/remove-meeting"), IP, PORT);
}

void AVRCGameMode::GetMeetingData(int32 MeetingId) {
	UVRCGameInstance* GameInstance = Cast<UVRCGameInstance>(GetWorld()->GetGameInstance());
	Http = GameInstance->GetHttpService();

	Http->SetTimeout(EndTimeout);

	FString SubRoute = "meetings/";
	SubRoute.AppendInt(MeetingId);

	TSharedRef<IHttpRequest> Request = Http->GetRequest(SubRoute);
	Request->OnProcessRequestComplete().BindUObject(this, &AVRCGameMode::OnGetMeetingData);
	Request->SetHeader("Accepts", TEXT("application/json"));

	FString auth = "Bearer ";
	auth.Append(ServerToken);
	Request->SetHeader("Authorization", auth);
	Request->SetHeader("Accepts", TEXT("application/json"));

	Request->ProcessRequest();
}

void AVRCGameMode::OnGetMeetingData(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful) {
	if (Http->ResponseIsValid(Response, bWasSuccessful)) {
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());

		if (FJsonSerializer::Deserialize(Reader, JsonObject))
		{
			FString MeetingName = JsonObject->GetStringField("name");
			int CreatorId = JsonObject->GetIntegerField("creator_id");

			NotifyGetMeetingData(MeetingName, CreatorId);
		}
		else {
			UE_LOG(LogTemp, Error, TEXT("OnGetMeetingData Server returned no data!"));
		}
	}
	else {
		UE_LOG(LogTemp, Error, TEXT("OnGetMeetingData Error getting meeting data!"));
	}
}