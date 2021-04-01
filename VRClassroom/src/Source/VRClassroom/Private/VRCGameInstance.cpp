// Copyright 2020 Testy


#include "VRCGameInstance.h"
#include "HttpService.h"

UVRCGameInstance::UVRCGameInstance(const class FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {
	Http = NewObject<UHttpService>(UHttpService::StaticClass());
}

UHttpService* UVRCGameInstance::GetHttpService() {
	return Http->GetHttp();
}

void UVRCGameInstance::SetUserToken(FString token) {
	User.Token = token;
	User.bIsLoggedIn = true;
	Http->SetAuthToken(token);
}

FString UVRCGameInstance::GetUserToken() {
	return User.Token;
}
