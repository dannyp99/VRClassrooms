// Fill out your copyright notice in the Description page of Project Settings.

#include "OWSPlayerController.h"
#include "OWSPlugin.h"
#include "Runtime/Online/HTTP/Public/Http.h"
#include "OWSCharacter.h"
#include "OWSGameMode.h"
#include "OWSGameInstance.h"
#include "GameFramework/PlayerInput.h"
#include "Runtime/Engine/Classes/GameFramework/PlayerState.h"
#include "Runtime/Core/Public/Misc/ConfigCacheIni.h"
#include "Runtime/Engine/Classes/Components/StaticMeshComponent.h"
//#include "Runtime/HeadMountedDisplay/Public/IXRTrackingSystem.h"


AOWSPlayerController::AOWSPlayerController()
{
	GConfig->GetString(
		TEXT("/Script/EngineSettings.GeneralProjectSettings"),
		TEXT("RPGAPICustomerKey"),
		RPGAPICustomerKey,
		GGameIni
	);

	GConfig->GetString(
		TEXT("/Script/EngineSettings.GeneralProjectSettings"),
		TEXT("RPGAPIPath"),
		RPGAPIPath,
		GGameIni
	);

	GConfig->GetString(
		TEXT("/Script/EngineSettings.GeneralProjectSettings"),
		TEXT("OWS2APIPath"),
		OWS2APIPath,
		GGameIni
	);

	GConfig->GetString(
		TEXT("/Script/EngineSettings.GeneralProjectSettings"),
		TEXT("OWSEncryptionKey"),
		OWSEncryptionKey,
		GGameIni
	);

	TravelTimeout = 60.f;
	MaxPredictionPing = 120.f;
	bEnableClickEvents = true;
}

void AOWSPlayerController::TravelToMap(const FString& URL, const bool SeamlessTravel)
{
	UE_LOG(OWS, Warning, TEXT("TravelToMap: %s"), *URL);
	ClientTravel(URL, TRAVEL_Absolute, false, FGuid());
}

void AOWSPlayerController::TravelToMap2(const FString& ServerAndPort, const float X, const float Y, const float Z, const float RX, const float RY, 
	const float RZ, const FString& PlayerName, const bool SeamlessTravel)
{
	/*
	FString URL = ServerAndPort 
		+ FString(TEXT("?PLX=")) + FString::SanitizeFloat(X)
		+ FString(TEXT("?PLY=")) + FString::SanitizeFloat(Y)
		+ FString(TEXT("?PLZ=")) + FString::SanitizeFloat(Z)
		+ FString(TEXT("?PRX=")) + FString::SanitizeFloat(RX)
		+ FString(TEXT("?PRY=")) + FString::SanitizeFloat(RY)
		+ FString(TEXT("?PRZ=")) + FString::SanitizeFloat(RZ)
		+ FString(TEXT("?Player=")) + FGenericPlatformHttp::UrlEncode(PlayerName);
	*/

	if (!GetWorld())
	{
		return;
	}

	UOWSGameInstance* GameInstance = Cast<UOWSGameInstance>(GetWorld()->GetGameInstance());

	if (!GameInstance)
	{
		return;
	}

	if (!GetOWSPlayerState())
	{
		UE_LOG(OWS, Error, TEXT("Invalid OWS Player State!  You can no longer use TravelToMap2 to connect to a server from the Select Character screen!"));
	
		return;
	}

	FString IDData = FString::SanitizeFloat(X)
		+ "|" + FString::SanitizeFloat(Y)
		+ "|" + FString::SanitizeFloat(Z)
		+ "|" + FString::SanitizeFloat(RX)
		+ "|" + FString::SanitizeFloat(RY)
		+ "|" + FString::SanitizeFloat(RZ)
		+ "|" + FGenericPlatformHttp::UrlEncode(GetOWSPlayerState()->GetPlayerName())
		+ "|" + GetOWSPlayerState()->UserSessionGUID;

	FString EncryptedIDData = GameInstance->EncryptWithAES(IDData, OWSEncryptionKey);
	//UE_LOG(OWS, Log, TEXT("DATA: %s"), *IDData);
	FString URL = ServerAndPort
		+ FString(TEXT("?ID=")) + EncryptedIDData;

	UE_LOG(OWS, Warning, TEXT("TravelToMap: %s"), *URL);
	ClientTravel(URL, TRAVEL_Absolute, false, FGuid());
}

void AOWSPlayerController::SetSelectedCharacterAndConnectToLastZone(FString UserSessionGUID, FString SelectedCharacterName)
{
	//Set character name and get user session

	Http = &FHttpModule::Get();
	Http->SetHttpTimeout(TravelTimeout); //Set timeout

	TSharedRef<IHttpRequest> Request = Http->CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &AOWSPlayerController::OnSetSelectedCharacterAndConnectToLastZoneResponseReceived);
	//This is the url on which to process the request
	FString url = FString(OWS2APIPath + "api/Users/SetSelectedCharacterAndGetUserSession");

	//Trim whitespace
	SelectedCharacterName.TrimStartAndEndInline();

	TArray<FStringFormatArg> FormatParams;
	FormatParams.Add(UserSessionGUID);
	FormatParams.Add(SelectedCharacterName);
	FString PostParameters = FString::Format(TEXT("{ \"UserSessionGUID\": \"{0}\", \"SelectedCharacterName\": \"{1}\" }"), FormatParams);

	Request->SetURL(url);
	Request->SetVerb("POST");
	Request->SetHeader(TEXT("User-Agent"), "X-UnrealEngine-Agent");
	Request->SetHeader("Content-Type", TEXT("application/json"));
	Request->SetHeader(TEXT("X-CustomerGUID"), RPGAPICustomerKey);
	Request->SetContentAsString(PostParameters);
	Request->ProcessRequest();	
}

void AOWSPlayerController::OnSetSelectedCharacterAndConnectToLastZoneResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		UE_LOG(OWS, Verbose, TEXT("OnSetSelectedCharacterAndConnectToLastZone Success!"));

		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());

		if (FJsonSerializer::Deserialize(Reader, JsonObject))
		{
			ServerTravelUserSessionGUID = JsonObject->GetStringField("UserSessionGUID");
			ServerTravelCharacterName = JsonObject->GetStringField("CharName");
			ServerTravelX = JsonObject->GetNumberField("X");
			ServerTravelY = JsonObject->GetNumberField("Y");
			ServerTravelZ = JsonObject->GetNumberField("Z");
			ServerTravelRX = JsonObject->GetNumberField("RX");
			ServerTravelRY = JsonObject->GetNumberField("RY");
			ServerTravelRZ = JsonObject->GetNumberField("RZ");

			UE_LOG(OWS, Log, TEXT("OnSetSelectedCharacterAndConnectToLastZone location is %f, %f, %f"), ServerTravelX, ServerTravelY, ServerTravelZ);

			if (ServerTravelCharacterName.IsEmpty())
			{
				ErrorGetUserSession(TEXT("Cannot find User Session!"));
				return;
			}

			TravelToLastZoneServer(ServerTravelCharacterName);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("OnSetSelectedCharacterAndConnectToLastZone Server returned no data!"));
		}
	}
	else
	{
		UE_LOG(OWS, Error, TEXT("OnSetSelectedCharacterAndConnectToLastZone Error accessing server!"));
	}
}

void AOWSPlayerController::TravelToLastZoneServer(FString CharacterName)
{
	Http = &FHttpModule::Get();
	Http->SetHttpTimeout(TravelTimeout); //Set timeout

	//UE_LOG(LogTemp, Warning, TEXT("CustomerID: %s"), *RPGAPICustomerKey);
	UE_LOG(OWS, Log, TEXT("Travel Last Zone"));

	TSharedRef<IHttpRequest> Request = Http->CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &AOWSPlayerController::OnTravelToLastZoneServerResponseReceived);

	CharacterName = CharacterName.Replace(TEXT(" "), TEXT("%20"));
	//This is the url on which to process the request
	FString url = FString(TEXT("http://" + RPGAPIPath + "/RPGServer/GetServerToConnectTo"));

	FString PostParameters = FString(TEXT("id=")) + CharacterName
		+ FString(TEXT("&ZoneName=GETLASTZONENAME"))
		+ FString(TEXT("&CustomerGUID=")) + RPGAPICustomerKey;

	Request->SetURL(url);
	Request->SetVerb("POST");
	Request->SetHeader(TEXT("User-Agent"), "X-UnrealEngine-Agent");
	Request->SetHeader("Content-Type", TEXT("application/x-www-form-urlencoded"));
	Request->SetContentAsString(PostParameters);
	Request->ProcessRequest();
}

void AOWSPlayerController::OnTravelToLastZoneServerResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	FString ServerAndPort;

	if (!GetWorld())
	{
		return;
	}

	UOWSGameInstance* GameInstance = Cast<UOWSGameInstance>(GetWorld()->GetGameInstance());

	if (!GameInstance)
	{
		return;
	}

	if (bWasSuccessful)
	{
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());

		if (FJsonSerializer::Deserialize(Reader, JsonObject))
		{
			FString ServerIP = JsonObject->GetStringField("serverip");
			FString Port = JsonObject->GetStringField("port");

			if (ServerIP.IsEmpty() || Port.IsEmpty())
			{
				UE_LOG(OWS, Error, TEXT("OnTravelToLastZoneServerResponseReceived Cannot Get Server IP and Port!"));
				return;
			}

			ServerAndPort = FString(TEXT("127.0.0.1:7778"));//ServerIP + FString(TEXT(":")) + Port.Left(4);

			UE_LOG(OWS, Warning, TEXT("OnTravelToLastZoneServerResponseReceived ServerAndPort: %s"), *ServerAndPort);

			UE_LOG(OWS, Log, TEXT("Travel GUID: %s"), *ServerTravelUserSessionGUID);

			UE_LOG(OWS, Log, TEXT("IP: %s"), *ServerAndPort);

			//Encrypt data to send
			FString IDData = FString::SanitizeFloat(ServerTravelX)
				+ "|" + FString::SanitizeFloat(ServerTravelY)
				+ "|" + FString::SanitizeFloat(ServerTravelZ)
				+ "|" + FString::SanitizeFloat(ServerTravelRX)
				+ "|" + FString::SanitizeFloat(ServerTravelRY)
				+ "|" + FString::SanitizeFloat(ServerTravelRZ)
				+ "|" + FGenericPlatformHttp::UrlEncode(ServerTravelCharacterName)
				+ "|" + ServerTravelUserSessionGUID;
			FString EncryptedIDData = GameInstance->EncryptWithAES(IDData, OWSEncryptionKey);

			FString URL = ServerAndPort
				+ FString(TEXT("?ID=")) + EncryptedIDData;

			TravelToMap(URL, false);
		}
		else
		{
			UE_LOG(OWS, Error, TEXT("OnTravelToLastZoneServerResponseReceived Server returned no data!"));
		}
	}
	else
	{
		UE_LOG(OWS, Error, TEXT("OnTravelToLastZoneServerResponseReceived Error accessing server!"));
	}
}



float AOWSPlayerController::GetPredictionTime()
{
	// exact ping is in msec, divide by 1000 to get time in seconds
	//if (Role == ROLE_Authority) { UE_LOG(UT, Warning, TEXT("Server ExactPing %f"), PlayerState->ExactPing); }
	//UE_LOG(LogTemp, Error, TEXT("ExactPing: %f"), PlayerState->ExactPing);

	return (PlayerState && (GetNetMode() != NM_Standalone)) ? (0.0005f*FMath::Clamp(PlayerState->ExactPing - PredictionFudgeFactor, 0.f, MaxPredictionPing)) : 0.f;
}

float AOWSPlayerController::GetProjectileSleepTime()
{
	return 0.001f * FMath::Max(0.f, PlayerState->ExactPing - PredictionFudgeFactor - MaxPredictionPing);
}

UTexture2D * AOWSPlayerController::LoadTextureReference(const FString& TexturePath)
{
	FStringAssetReference TextureReference(TexturePath);
	return Cast<UTexture2D>(TextureReference.TryLoad());
}

void AOWSPlayerController::SetSelectedCharacter(AOWSCharacter* RPGCharacter)
{
	if (SelectedCharacter)
	{
		ClearSelectionOnCharacter(SelectedCharacter);
	}

	if (RPGCharacter)
	{
		TArray<UActorComponent*> SMcomps;

		RPGCharacter->GetComponents(SMcomps);
		for (int i = 0; i < SMcomps.Num(); ++i) //Because there may be more components
		{
			UStaticMeshComponent* thisSMComp = Cast<UStaticMeshComponent>(SMcomps[i]); //try to cast to static mesh component
			if (thisSMComp)
			{
				thisSMComp->SetRenderCustomDepth(true);
				thisSMComp->SetCustomDepthStencilValue(252);
			}

			USkeletalMeshComponent* thisSKMComp = Cast<USkeletalMeshComponent>(SMcomps[i]); //try to cast to static mesh component
			if (thisSKMComp)
			{
				thisSKMComp->SetRenderCustomDepth(true);
				thisSKMComp->SetCustomDepthStencilValue(252);
			}
		}

		SelectedCharacter = RPGCharacter;
	}
}

void AOWSPlayerController::ClearSelectedCharacter()
{
	if (SelectedCharacter)
	{
		AOWSCharacter* RPGCharacter = SelectedCharacter;
		TArray<UActorComponent*> SMcomps;

		RPGCharacter->GetComponents(SMcomps);
		for (int i = 0; i < SMcomps.Num(); ++i) //Because there may be more components
		{
			UStaticMeshComponent* thisSMComp = Cast<UStaticMeshComponent>(SMcomps[i]); //try to cast to static mesh component
			if (thisSMComp)
			{
				//if (thisSMComp->CustomDepthStencilValue != 254)
				//{
				thisSMComp->SetRenderCustomDepth(false);
				thisSMComp->SetCustomDepthStencilValue(0);
				//}
			}

			USkeletalMeshComponent* thisSKMComp = Cast<USkeletalMeshComponent>(SMcomps[i]); //try to cast to static mesh component
			if (thisSKMComp)
			{
				//if (thisSKMComp->CustomDepthStencilValue != 254)
				//{
				thisSKMComp->SetRenderCustomDepth(false);
				thisSKMComp->SetCustomDepthStencilValue(0);
				//}
			}
		}

		SelectedCharacter = NULL;
	}
}

void AOWSPlayerController::ClearSelectionOnCharacter(AOWSCharacter* RPGCharacter)
{
	if (RPGCharacter)
	{
		TArray<UActorComponent*> SMcomps;

		RPGCharacter->GetComponents(SMcomps);
		for (int i = 0; i < SMcomps.Num(); ++i) //Because there may be more components
		{
			UStaticMeshComponent* thisSMComp = Cast<UStaticMeshComponent>(SMcomps[i]); //try to cast to static mesh component
			if (thisSMComp)
			{
				//if (thisSMComp->CustomDepthStencilValue != 254)
				//{
					thisSMComp->SetRenderCustomDepth(false);
					thisSMComp->SetCustomDepthStencilValue(0);
				//}
			}

			USkeletalMeshComponent* thisSKMComp = Cast<USkeletalMeshComponent>(SMcomps[i]); //try to cast to static mesh component
			if (thisSKMComp)
			{
				//if (thisSKMComp->CustomDepthStencilValue != 254)
				//{
					thisSKMComp->SetRenderCustomDepth(false);
					thisSKMComp->SetCustomDepthStencilValue(0);
				//}
			}
		}

		SelectedCharacter = NULL;
	}
}


void AOWSPlayerController::PawnLeavingGame()
{
	AOWSCharacter* MyCharacter = Cast<AOWSCharacter>(GetPawn());

	if (MyCharacter)
	{
		UE_LOG(OWS, Verbose, TEXT("NotifyPawnLeavingGame"));
		NotifyPawnLeavingGame(MyCharacter);
	}

	Super::PawnLeavingGame();
}

AOWSPlayerState* AOWSPlayerController::GetOWSPlayerState() const
{
	return GetPlayerState<AOWSPlayerState>();
}

UOWSReplicationGraph* AOWSPlayerController::GetReplicationGraph() const
{
	return CastChecked<UOWSReplicationGraph>(GetNetDriver()->GetReplicationDriver());
}

/*bool AOWSPlayerController::InputKey(FKey Key, EInputEvent EventType, float AmountDepressed, bool bGamepad)
{

	if (GEngine->XRSystem.IsValid())
	{
		auto XRInput = GEngine->XRSystem->GetXRInput();
		if (XRInput && XRInput->HandleInputKey(PlayerInput, Key, EventType, AmountDepressed, bGamepad))
		{
			return true;
		}
	}

	bool bResult = false;
	if (PlayerInput)
	{
		bResult = PlayerInput->InputKey(Key, EventType, AmountDepressed, bGamepad);
		if (bEnableClickEvents && (ClickEventKeys.Contains(Key) || ClickEventKeys.Contains(EKeys::AnyKey)))
		{
			FVector2D MousePosition;
			UGameViewportClient* ViewportClient = CastChecked<ULocalPlayer>(Player)->ViewportClient;
			if (ViewportClient && ViewportClient->GetMousePosition(MousePosition))
			{
				UPrimitiveComponent* ClickedPrimitive = NULL;
				if (bEnableMouseOverEvents)
				{
					ClickedPrimitive = CurrentClickablePrimitive.Get();
				}
				else
				{
					FHitResult HitResult;
					const bool bHit = GetHitResultAtScreenPosition(MousePosition, CurrentClickTraceChannel, true, HitResult);
					if (bHit)
					{
						ClickedPrimitive = HitResult.Component.Get();
					}
				}
				if (GetHUD())
				{
					if (GetHUD()->UpdateAndDispatchHitBoxClickEvents(MousePosition, EventType))
					{
						ClickedPrimitive = NULL;
					}
				}

				if (ClickedPrimitive)
				{
					switch (EventType)
					{
					case IE_Pressed:
					case IE_DoubleClick:
						ClickedPrimitive->DispatchOnClicked(Key);
						break;

					case IE_Released:
						ClickedPrimitive->DispatchOnReleased(Key);
						break;

					case IE_Axis:
					case IE_Repeat:
						break;
					}
				}

				bResult = true;
			}
		}
	}

	return bResult;
}*/

bool AOWSPlayerController::InputAxis(FKey Key, float Delta, float DeltaTime, int32 NumSamples, bool bGamepad)
{
	bool bResult = false;

	if (PlayerInput)
	{
		bResult = PlayerInput->InputAxis(Key, Delta, DeltaTime, NumSamples, bGamepad);
	}

	return bResult;
}

void AOWSPlayerController::SavePlayerLocation()
{
	Http = &FHttpModule::Get();

	FString PlayerName = PlayerState->GetPlayerName();
	PlayerName = PlayerName.Replace(TEXT(" "), TEXT("%20"));

	AOWSCharacter* MyRPGCharacter = Cast<AOWSCharacter>(GetPawn());

	TSharedRef<IHttpRequest> Request = Http->CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &AOWSPlayerController::OnSavePlayerLocationResponseReceived);
	//This is the url on which to process the request
	FString url = FString(TEXT("http://" + RPGAPIPath + "/RPGServer/UpdatePosition"));	

	FString PostParameters = FString(TEXT("id=")) + PlayerName
		+ FString(TEXT("&X=")) + FString::SanitizeFloat(MyRPGCharacter->GetActorLocation().X)
		+ FString(TEXT("&Y=")) + FString::SanitizeFloat(MyRPGCharacter->GetActorLocation().Y)
		+ FString(TEXT("&Z=")) + FString::SanitizeFloat(MyRPGCharacter->GetActorLocation().Z)
		+ FString(TEXT("&RX=")) + FString::SanitizeFloat(MyRPGCharacter->GetActorRotation().Roll)
		+ FString(TEXT("&RY=")) + FString::SanitizeFloat(MyRPGCharacter->GetActorRotation().Pitch)
		+ FString(TEXT("&RZ=")) + FString::SanitizeFloat(MyRPGCharacter->GetActorRotation().Yaw)
		+ FString(TEXT("&CustomerGUID=")) + RPGAPICustomerKey;

	Request->SetURL(url);
	Request->SetVerb("POST");
	Request->SetHeader(TEXT("User-Agent"), "X-UnrealEngine-Agent");
	Request->SetHeader("Content-Type", TEXT("application/x-www-form-urlencoded"));
	Request->SetContentAsString(PostParameters);
	Request->ProcessRequest();
}

void AOWSPlayerController::OnSavePlayerLocationResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		UE_LOG(OWS, Verbose, TEXT("OnSavePlayerLocationResponseReceived Success!"));

		//UE_LOG(LogTemp, Error, TEXT("OnSavePlayerLocationResponseReceived Success: %s"), *Response->GetContentAsString());
		/*TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());

		if (FJsonSerializer::Deserialize(Reader, JsonObject))
		{

		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("OnSavePlayerLocationResponseReceived Server returned no data!"));
		}*/
	}
	else
	{
		UE_LOG(OWS, Error, TEXT("OnSavePlayerLocationResponseReceived Error accessing server!"));
	}
}


void AOWSPlayerController::SaveAllPlayerData()
{
	Http = &FHttpModule::Get();

	//FString PlayerName = "Test";
	FString PlayerName = PlayerState->GetPlayerName();
	PlayerName = PlayerName.Replace(TEXT(" "), TEXT("%20"));

	AOWSCharacter* MyRPGCharacter = Cast<AOWSCharacter>(GetPawn());

	TSharedRef<IHttpRequest> Request = Http->CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &AOWSPlayerController::OnSaveAllPlayerDataResponseReceived);
	//This is the url on which to process the request
	FString url = FString(TEXT("http://" + RPGAPIPath + "/RPGServer/UpdatePosition"));


	FString PostParameters = FString(TEXT("id=")) + PlayerName
		+ FString(TEXT("&X=")) + FString::SanitizeFloat(MyRPGCharacter->GetActorLocation().X)
		+ FString(TEXT("&Y=")) + FString::SanitizeFloat(MyRPGCharacter->GetActorLocation().Y)
		+ FString(TEXT("&Z=")) + FString::SanitizeFloat(MyRPGCharacter->GetActorLocation().Z)
		+ FString(TEXT("&RX=")) + FString::SanitizeFloat(MyRPGCharacter->GetActorRotation().Roll)
		+ FString(TEXT("&RY=")) + FString::SanitizeFloat(MyRPGCharacter->GetActorRotation().Pitch)
		+ FString(TEXT("&RZ=")) + FString::SanitizeFloat(MyRPGCharacter->GetActorRotation().Yaw)
		+ FString(TEXT("&CustomerGUID=")) + RPGAPICustomerKey;

	Request->SetURL(url);
	Request->SetVerb("POST");
	Request->SetHeader(TEXT("User-Agent"), "X-UnrealEngine-Agent");
	Request->SetHeader("Content-Type", TEXT("application/x-www-form-urlencoded"));
	Request->SetContentAsString(PostParameters);
	//UE_LOG(LogTemp, Warning, TEXT("Sent %s"), *PostParameters);
	Request->ProcessRequest();
}

void AOWSPlayerController::OnSaveAllPlayerDataResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		//UE_LOG(LogTemp, Warning, TEXT("OnSaveAllPlayerDataResponseReceived Success: %s"), *Response->GetContentAsString());
		/*TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());

		if (FJsonSerializer::Deserialize(Reader, JsonObject))
		{

		}
		else
		{
		UE_LOG(OWS, Error, TEXT("OnSavePlayerLocationResponseReceived Server returned no data!"));
		}*/
	}
	else
	{
		UE_LOG(OWS, Error, TEXT("OnSaveAllPlayerDataResponseReceived Error accessing server!"));
	}
}

//Add Player to Group
void AOWSPlayerController::AddPlayerToGroup(FString PlayerGroupName, ERPGPlayerGroupType::PlayerGroupType GroupType, FString CharacterNameToAdd)
{
	Http = &FHttpModule::Get();

	//FString PlayerName = "Test";
	FString PlayerName = PlayerState->GetPlayerName();
	PlayerName = PlayerName.Replace(TEXT(" "), TEXT("%20"));

	AOWSCharacter* MyRPGCharacter = Cast<AOWSCharacter>(GetPawn());

	TSharedRef<IHttpRequest> Request = Http->CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &AOWSPlayerController::OnAddPlayerToGroupResponseReceived);
	//This is the url on which to process the request
	FString url = FString(TEXT("http://" + RPGAPIPath + "/Groups/AddPlayerToGroupJSON"));

	FString PostParameters = FString(TEXT("id=")) + PlayerName
		+ FString(TEXT("&GroupName=")) + PlayerGroupName
		+ FString(TEXT("&GroupType=")) + FString::FromInt(GroupType)
		+ FString(TEXT("&CharacterNameToAdd=")) + CharacterNameToAdd
		+ FString(TEXT("&CustomerGUID=")) + RPGAPICustomerKey;

	Request->SetURL(url);
	Request->SetVerb("POST");
	Request->SetHeader(TEXT("User-Agent"), "X-UnrealEngine-Agent");
	Request->SetHeader("Content-Type", TEXT("application/x-www-form-urlencoded"));
	Request->SetContentAsString(PostParameters);
	//UE_LOG(LogTemp, Warning, TEXT("Sent %s"), *PostParameters);
	Request->ProcessRequest();
}

void AOWSPlayerController::OnAddPlayerToGroupResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		UE_LOG(OWS, Verbose, TEXT("OnAddPlayerGroupResponseReceived Success: %s"), *Response->GetContentAsString());
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());

		if (FJsonSerializer::Deserialize(Reader, JsonObject))
		{
			FString CharacterNameAddedToGroup = JsonObject->GetStringField("characternameaddedtogroup");
			int32 PlayerGroupID = JsonObject->GetNumberField("playergroupid");
			FString PlayerGroupName = JsonObject->GetStringField("playergroupname");
			int32 PlayerGroupTypeID = JsonObject->GetNumberField("playergrouptype");

			FPlayerGroup tempPlayerGroup;

			tempPlayerGroup.PlayerGroupID = PlayerGroupID;
			tempPlayerGroup.PlayerGroupName = PlayerGroupName;
			tempPlayerGroup.PlayerGroupTypeID = PlayerGroupTypeID;

			PlayerGroupsPlayerIsIn.Add(tempPlayerGroup);

			//Add self
			GetOWSPlayerState()->AlwaysRelevantPartyID = PlayerGroupID;
			//GetReplicationGraph()->AddPlayerToParty(GetOWSPlayerState());

			//Add other player
			GetOWSPlayerState()->AlwaysRelevantPartyID = PlayerGroupID;
			AOWSGameMode* MyGameMode = Cast<AOWSGameMode>(GetWorld()->GetAuthGameMode());
			if (MyGameMode)
			{
				AOWSPlayerState* OtherPlayerState = MyGameMode->GetPlayerControllerFromCharacterName(CharacterNameAddedToGroup)->GetOWSPlayerState();
				if (OtherPlayerState)
				{
					OtherPlayerState->AlwaysRelevantPartyID = PlayerGroupID;
					//GetReplicationGraph()->AddPlayerToParty(OtherPlayerState);
				}
			}

			NotifyAddPlayerToGroup(CharacterNameAddedToGroup);
		}
		else
		{
			UE_LOG(OWS, Error, TEXT("OnSavePlayerLocationResponseReceived Server returned no data!"));
		}	
	}
	else
	{
		UE_LOG(OWS, Error, TEXT("OnAddPlayerGroupResponseReceived Error accessing server!"));
	}
}


//Remove Player from Group
void AOWSPlayerController::RemovePlayerFromGroup(FString PlayerGroupName, FString CharacterNameToRemove)
{
	Http = &FHttpModule::Get();

	//FString PlayerName = "Test";
	FString PlayerName = PlayerState->GetPlayerName();
	PlayerName = PlayerName.Replace(TEXT(" "), TEXT("%20"));

	AOWSCharacter* MyRPGCharacter = Cast<AOWSCharacter>(GetPawn());

	TSharedRef<IHttpRequest> Request = Http->CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &AOWSPlayerController::OnRemovePlayerFromGroupResponseReceived);
	//This is the url on which to process the request
	FString url = FString(TEXT("http://" + RPGAPIPath + "/Groups/RemovePlayerFromGroupJSON"));

	FString PostParameters = FString(TEXT("id=")) + PlayerName
		+ FString(TEXT("&GroupName=")) + PlayerGroupName
		+ FString(TEXT("&CharacterNameToRemove=")) + CharacterNameToRemove
		+ FString(TEXT("&CustomerGUID=")) + RPGAPICustomerKey;

	Request->SetURL(url);
	Request->SetVerb("POST");
	Request->SetHeader(TEXT("User-Agent"), "X-UnrealEngine-Agent");
	Request->SetHeader("Content-Type", TEXT("application/x-www-form-urlencoded"));
	Request->SetContentAsString(PostParameters);
	//UE_LOG(LogTemp, Warning, TEXT("Sent %s"), *PostParameters);
	Request->ProcessRequest();
}

void AOWSPlayerController::OnRemovePlayerFromGroupResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		UE_LOG(OWS, Verbose, TEXT("OnRemovePlayerFromGroupResponseReceived Success: %s"), *Response->GetContentAsString());
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());

		if (FJsonSerializer::Deserialize(Reader, JsonObject))
		{
			FString CharacterNameAddedToGroup = JsonObject->GetStringField("characternameaddedtogroup");
			int32 PlayerGroupID = JsonObject->GetNumberField("playergroupid");

			PlayerGroupsPlayerIsIn.RemoveAll([PlayerGroupID](const FPlayerGroup& InPlayerGroup)
			{
				return InPlayerGroup.PlayerGroupID == PlayerGroupID;
			});

			NotifyRemovePlayerFromGroup(CharacterNameAddedToGroup);
		}
		else
		{
			UE_LOG(OWS, Error, TEXT("OnRemovePlayerFromGroupResponseReceived Server returned no data!"));
		}
	}
	else
	{
		UE_LOG(OWS, Error, TEXT("OnRemovePlayerFromGroupResponseReceived Error accessing server!"));
	}
}



void AOWSPlayerController::LaunchDungeon(FString MapName, ERPGPlayerGroupType::PlayerGroupType GroupType)
{
	Http = &FHttpModule::Get();

	FString PlayerName = PlayerState->GetPlayerName();
	PlayerName = PlayerName.Replace(TEXT(" "), TEXT("%20"));

	TSharedRef<IHttpRequest> Request = Http->CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &AOWSPlayerController::OnLaunchDungeonResponseReceived);
	//This is the url on which to process the request
	FString url = FString(TEXT("http://" + RPGAPIPath + "/RPGServer/GetServerToConnectTo"));

	FString PostParameters = FString(TEXT("id=")) + PlayerName
		+ FString(TEXT("&ZoneName=")) + MapName
		+ FString(TEXT("&CustomerGUID=")) + RPGAPICustomerKey
		+ FString(TEXT("&PlayerGroupType=")) + FString::FromInt(GroupType);
	Request->SetURL(url);
	Request->SetVerb("POST");
	Request->SetHeader(TEXT("User-Agent"), "X-UnrealEngine-Agent");
	Request->SetHeader("Content-Type", TEXT("application/x-www-form-urlencoded"));
	Request->SetContentAsString(PostParameters);
	Request->ProcessRequest();

	UE_LOG(OWS, Warning, TEXT("LaunchDungeon: %s, MapName: %s"), *PlayerName, *MapName);
}

void AOWSPlayerController::OnLaunchDungeonResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	FString ServerAndPort;

	if (bWasSuccessful)
	{
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());

		FString asd = Response->GetContentAsString();

		UE_LOG(OWS, Log, TEXT("%s"), *asd);

		if (FJsonSerializer::Deserialize(Reader, JsonObject))
		{
			FString ServerIP = JsonObject->GetStringField("serverip");
			FString Port = JsonObject->GetStringField("port");

			ServerAndPort = ServerIP + FString(TEXT(":")) + Port;

			UE_LOG(OWS, Warning, TEXT("OnLaunchDungeonResponseReceived: %s:%s"), *ServerIP, *Port);

			NotifyLaunchDungeon(ServerAndPort);
		}
		else
		{
			UE_LOG(OWS, Error, TEXT("OnLaunchDungeonResponseReceived Server returned no data!  This usually means the dungeon server instance failed to spin up before the timeout was reached."));
			ErrorLaunchDungeon(TEXT("Server returned no data!  This usually means the dungeon server instance failed to spin up before the timeout was reached."));
		}
	}
	else
	{
		UE_LOG(OWS, Error, TEXT("OnLaunchDungeonResponseReceived Error accessing server!"));
		ErrorLaunchDungeon(TEXT("Error accessing server!"));
	}
}



void AOWSPlayerController::PlayerLogout()
{
	Http = &FHttpModule::Get();

	FString PlayerName = PlayerState->GetPlayerName();
	PlayerName = PlayerName.Replace(TEXT(" "), TEXT("%20"));

	TSharedRef<IHttpRequest> Request = Http->CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &AOWSPlayerController::OnPlayerLogoutResponseReceived);
	//This is the url on which to process the request
	FString url = FString(TEXT("http://" + RPGAPIPath + "/RPGServer/PlayerLogout"));

	FString PostParameters = FString(TEXT("id=")) + PlayerName
		+ FString(TEXT("&CustomerGUID=")) + RPGAPICustomerKey;

	Request->SetURL(url);
	Request->SetVerb("POST");
	Request->SetHeader(TEXT("User-Agent"), "X-UnrealEngine-Agent");
	Request->SetHeader("Content-Type", TEXT("application/x-www-form-urlencoded"));
	Request->SetContentAsString(PostParameters);
	Request->ProcessRequest();
}

void AOWSPlayerController::OnPlayerLogoutResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	FString ServerAndPort;

	if (bWasSuccessful)
	{
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());

		if (FJsonSerializer::Deserialize(Reader, JsonObject))
		{
			UE_LOG(OWS, Verbose, TEXT("Logout Successful!"));
			NotifyPlayerLogout();
		}
		else
		{
			UE_LOG(OWS, Error, TEXT("OnPlayerLogoutResponseReceived Server returned no data!"));
			ErrorPlayerLogout(TEXT("Server returned no data!"));
		}
	}
	else
	{
		UE_LOG(OWS, Error, TEXT("OnPlayerLogoutResponseReceived Error accessing server!"));
		ErrorPlayerLogout(TEXT("Error accessing server!"));
	}
}


void AOWSPlayerController::GetChatGroupsForPlayer()
{
	Http = &FHttpModule::Get();

	FString PlayerName = PlayerState->GetPlayerName();
	PlayerName = PlayerName.Replace(TEXT(" "), TEXT("%20"));

	TSharedRef<IHttpRequest> Request = Http->CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &AOWSPlayerController::OnGetChatGroupsForPlayerResponseReceived);
	//This is the url on which to process the request
	FString url = FString(TEXT("http://" + RPGAPIPath + "/Chat/GetChatGroupsForPlayer"));

	FString PostParameters = FString(TEXT("id=")) + PlayerName
		+ FString(TEXT("&CustomerGUID=")) + RPGAPICustomerKey;

	Request->SetURL(url);
	Request->SetVerb("POST");
	Request->SetHeader(TEXT("User-Agent"), "X-UnrealEngine-Agent");
	Request->SetHeader("Content-Type", TEXT("application/x-www-form-urlencoded"));
	Request->SetContentAsString(PostParameters);
	Request->ProcessRequest();
}

void AOWSPlayerController::OnGetChatGroupsForPlayerResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());

		if (FJsonSerializer::Deserialize(Reader, JsonObject))
		{
			if (JsonObject->GetStringField("success") == "true")
			{
				TArray<TSharedPtr<FJsonValue>> Rows = JsonObject->GetArrayField("rows");

				TArray<FChatGroup> ChatGroups;

				for (int RowNum = 0; RowNum != Rows.Num(); RowNum++) {
					FChatGroup tempChatGroup;
					TSharedPtr<FJsonObject> tempRow = Rows[RowNum]->AsObject();
					tempChatGroup.ChatGroupID = tempRow->GetIntegerField("ChatGroupID");
					tempChatGroup.ChatGroupName = tempRow->GetStringField("ChatGroupName");

					ChatGroups.Add(tempChatGroup);
				}

				NotifyGetChatGroupsForPlayer(ChatGroups);
			}
		}
	}
	else
	{
		UE_LOG(OWS, Error, TEXT("OnLeaveChatGroupResponseReceived Error accessing login server!"));
		ErrorGetChatGroupsForPlayer(TEXT("OnLeaveChatGroupResponseReceived Error accessing login server!"));
	}
}


void AOWSPlayerController::IsPlayerOnline(FString PlayerName)
{
	Http = &FHttpModule::Get();

	PlayerName = PlayerName.Replace(TEXT(" "), TEXT("%20"));

	TSharedRef<IHttpRequest> Request = Http->CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &AOWSPlayerController::OnIsPlayerOnlineResponseReceived);
	//This is the url on which to process the request
	FString url = FString(TEXT("http://" + RPGAPIPath + "/RPGServer/IsPlayerOnline"));

	FString PostParameters = FString(TEXT("id=")) + PlayerName
		+ FString(TEXT("&CustomerGUID=")) + RPGAPICustomerKey;

	Request->SetURL(url);
	Request->SetVerb("POST");
	Request->SetHeader(TEXT("User-Agent"), "X-UnrealEngine-Agent");
	Request->SetHeader("Content-Type", TEXT("application/x-www-form-urlencoded"));
	Request->SetContentAsString(PostParameters);
	Request->ProcessRequest();
}

void AOWSPlayerController::OnIsPlayerOnlineResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());

		if (FJsonSerializer::Deserialize(Reader, JsonObject))
		{
			if (JsonObject->GetStringField("success") == "true")
			{
				bool IsOnline = JsonObject->GetBoolField("IsOnline");

				NotifyIsPlayerOnline(IsOnline);
			}
		}
	}
	else
	{
		UE_LOG(OWS, Error, TEXT("OnIsPlayerOnlineResponseReceived Error accessing login server!"));
		ErrorIsPlayerOnline(TEXT("OnIsPlayerOnlineResponseReceived Error accessing login server!"));
	}
}



void AOWSPlayerController::GetAllCharacters(FString UserSessionGUID)
{
	GConfig->GetString(
		TEXT("/Script/EngineSettings.GeneralProjectSettings"),
		TEXT("RPGAPICustomerKey"),
		RPGAPICustomerKey,
		GGameIni
	);

	Http = &FHttpModule::Get();

	TSharedRef<IHttpRequest> Request = Http->CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &AOWSPlayerController::OnGetAllCharactersResponseReceived);
	//This is the url on which to process the request
	FString url = FString(TEXT("http://" + RPGAPIPath + "/RPGUser/GetAllCharacters/")) + UserSessionGUID;

	FString PostParameters = FString(TEXT("CustomerGUID=")) + RPGAPICustomerKey;

	Request->SetURL(url);
	Request->SetVerb("POST");
	Request->SetHeader(TEXT("User-Agent"), "X-UnrealEngine-Agent");
	Request->SetHeader("Content-Type", TEXT("application/x-www-form-urlencoded"));
	Request->SetContentAsString(PostParameters);
	Request->ProcessRequest();
}

void AOWSPlayerController::OnGetAllCharactersResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());

		if (FJsonSerializer::Deserialize(Reader, JsonObject))
		{
			FString ErrorMessage = JsonObject->GetStringField("ErrorMessage");

			if (!ErrorMessage.IsEmpty())
			{
				ErrorGetAllCharacters(*ErrorMessage);
				return;
			}

			TArray<FUserCharacter> UsersCharactersData;

			if (JsonObject->HasField("rows"))
			{
				TArray<TSharedPtr<FJsonValue>> Rows = JsonObject->GetArrayField("rows");

				for (int RowNum = 0; RowNum != Rows.Num(); RowNum++) {
					FUserCharacter tempUserCharacter;
					TSharedPtr<FJsonObject> tempRow = Rows[RowNum]->AsObject();
					tempUserCharacter.CharacterName = tempRow->GetStringField("CharName");
					tempUserCharacter.ClassName = tempRow->GetStringField("ClassName");
					tempUserCharacter.Level = tempRow->GetNumberField("CharacterLevel");
					tempUserCharacter.ZoneName = tempRow->GetStringField("MapName");
					tempUserCharacter.Gender = tempRow->GetNumberField("Gender");
					tempUserCharacter.Gold = tempRow->GetNumberField("Gold");
					tempUserCharacter.Silver = tempRow->GetNumberField("Silver");
					tempUserCharacter.Copper = tempRow->GetNumberField("Copper");
					tempUserCharacter.FreeCurrency = tempRow->GetNumberField("FreeCurrency");
					tempUserCharacter.PremiumCurrency = tempRow->GetNumberField("PremiumCurrency");
					tempUserCharacter.Score = tempRow->GetNumberField("Score");
					tempUserCharacter.XP = tempRow->GetNumberField("XP");
					tempUserCharacter.LastActivity = tempRow->GetStringField("LastActivityString");
					tempUserCharacter.CreateDate = tempRow->GetStringField("CreateDateString");

					UsersCharactersData.Add(tempUserCharacter);
				}
			}

			NotifyGetAllCharacters(UsersCharactersData);
		}
		else
		{
			UE_LOG(OWS, Error, TEXT("OnGetAllCharactersResponseReceived Server returned no data!"));
			ErrorGetAllCharacters(TEXT("Unknown login error!"));
		}
	}
	else
	{
		UE_LOG(OWS, Error, TEXT("OnGetAllCharactersResponseReceived Error accessing login server!"));
		ErrorGetAllCharacters(TEXT("Unknown error connecting to server!"));
	}
}



void AOWSPlayerController::CreateCharacter(FString UserSessionGUID, FString CharacterName, FString ClassName)
{
	GConfig->GetString(
		TEXT("/Script/EngineSettings.GeneralProjectSettings"),
		TEXT("RPGAPICustomerKey"),
		RPGAPICustomerKey,
		GGameIni
	);

	Http = &FHttpModule::Get();

	TSharedRef<IHttpRequest> Request = Http->CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &AOWSPlayerController::OnCreateCharacterResponseReceived);

	/*
	//This is the url on which to process the request
	FString url = FString(TEXT("http://" + RPGAPIPath + "/RPGUser/CreateCharacter/")) + UserSessionGUID;

	FString PostParameters = FString(TEXT("CharacterName=")) + CharacterName
		+ FString(TEXT("&ClassName=")) + ClassName
		+ FString(TEXT("&CustomerGUID=")) + RPGAPICustomerKey;

	Request->SetURL(url);
	Request->SetVerb("POST");
	Request->SetHeader(TEXT("User-Agent"), "X-UnrealEngine-Agent");
	Request->SetHeader("Content-Type", TEXT("application/x-www-form-urlencoded"));
	Request->SetContentAsString(PostParameters);
	Request->ProcessRequest();
	*/

	FString url = FString(OWS2APIPath + "api/Users/CreateCharacter");

	//Trim whitespace
	CharacterName.TrimStartAndEndInline();
	ClassName.TrimStartAndEndInline();

	/*FString PostParameters = FString(TEXT("{ \"UserSessionGUID\": \"")) + UserSessionGUID + FString(TEXT("\", "))
		+ FString(TEXT("\"CharacterName\": \"")) + CharacterName + FString(TEXT("\", "))
		+ FString(TEXT("\"ClassName\": \"")) + ClassName + FString(TEXT("\" }"));*/

	TArray<FStringFormatArg> FormatParams;
	FormatParams.Add(UserSessionGUID);
	FormatParams.Add(CharacterName);
	FormatParams.Add(ClassName);
	FString PostParameters = FString::Format(TEXT("{ \"UserSessionGUID\": \"{0}\", \"CharacterName\": \"{1}\",  \"ClassName\": \"{2}\" }"), FormatParams);

	Request->SetURL(url);
	Request->SetVerb("POST");
	Request->SetHeader(TEXT("User-Agent"), "X-UnrealEngine-Agent");
	Request->SetHeader("Content-Type", TEXT("application/json"));
	Request->SetHeader(TEXT("X-CustomerGUID"), RPGAPICustomerKey);
	Request->SetContentAsString(PostParameters);
	Request->ProcessRequest();
}

void AOWSPlayerController::OnCreateCharacterResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		FString ResponseString = Response->GetContentAsString();

		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseString);

		if (FJsonSerializer::Deserialize(Reader, JsonObject))
		{
			FCreateCharacter CreateCharacter;

			if (!FJsonObjectConverter::JsonObjectStringToUStruct(ResponseString, &CreateCharacter, 0, 0))
			{
				ErrorCreateCharacter(TEXT("Could not deserialize CreateCharacter JSON to CreateCharacter struct!"));
				return;
			}

			if (!CreateCharacter.ErrorMessage.IsEmpty())
			{
				ErrorCreateCharacter(*CreateCharacter.ErrorMessage);
				return;
			}

			NotifyCreateCharacter(CreateCharacter);
		}
		else
		{
			UE_LOG(OWS, Error, TEXT("OnCreateCharacterResponseReceived Server returned no data!"));
			ErrorCreateCharacter(TEXT("Unknown login error!"));
		}
	}
	else
	{
		UE_LOG(OWS, Error, TEXT("OnCreateCharacterResponseReceived Error accessing login server!"));
		ErrorCreateCharacter(TEXT("Unknown error connecting to server!"));
	}
}


void AOWSPlayerController::GetLastZoneServerToTravelTo(FString CharacterName)
{
	Http = &FHttpModule::Get();
	Http->SetHttpTimeout(TravelTimeout); //Set timeout

	//UE_LOG(LogTemp, Warning, TEXT("CustomerID: %s"), *RPGAPICustomerKey);

	TSharedRef<IHttpRequest> Request = Http->CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &AOWSPlayerController::OnGetLastZoneServerToTravelToResponseReceived);

	CharacterName = CharacterName.Replace(TEXT(" "), TEXT("%20"));
	//This is the url on which to process the request
	FString url = FString(TEXT("http://" + RPGAPIPath + "/RPGServer/GetServerToConnectTo"));

	FString PostParameters = FString(TEXT("id=")) + CharacterName
		+ FString(TEXT("&ZoneName=GETLASTZONENAME"))
		+ FString(TEXT("&CustomerGUID=")) + RPGAPICustomerKey;

	Request->SetURL(url);
	Request->SetVerb("POST");
	Request->SetHeader(TEXT("User-Agent"), "X-UnrealEngine-Agent");
	Request->SetHeader("Content-Type", TEXT("application/x-www-form-urlencoded"));
	Request->SetContentAsString(PostParameters);
	Request->ProcessRequest();
}

void AOWSPlayerController::OnGetLastZoneServerToTravelToResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	FString ServerAndPort;

	if (bWasSuccessful)
	{
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());

		if (FJsonSerializer::Deserialize(Reader, JsonObject))
		{
			FString ServerIP = JsonObject->GetStringField("serverip");
			FString Port = JsonObject->GetStringField("port");

			if (ServerIP.IsEmpty() || Port.IsEmpty())
			{
				ErrorLastZoneServerToTravelTo(TEXT("Cannot connect to server!"));
				return;
			}

			ServerAndPort = ServerIP + FString(TEXT(":")) + Port.Left(4);

			UE_LOG(OWS, Warning, TEXT("OnGetLastZoneServerToTravelToResponseReceived ServerAndPort: %s"), *ServerAndPort);

			NotifyLastZoneServerToTravelTo(ServerAndPort);
		}
		else
		{
			UE_LOG(OWS, Error, TEXT("OnGetLastZoneServerToTravelToResponseReceived Server returned no data!"));
			ErrorLastZoneServerToTravelTo(TEXT("There was a problem connecting to the server.  Please try again."));
		}
	}
	else
	{
		UE_LOG(OWS, Error, TEXT("OnGetLastZoneServerToTravelToResponseReceived Error accessing server!"));
		ErrorLastZoneServerToTravelTo(TEXT("Unknown error connecting to server!"));
	}
}

void AOWSPlayerController::NotifyLastZoneServerToTravelTo_Implementation(const FString &ServerAndPort)
{

}

void AOWSPlayerController::ErrorLastZoneServerToTravelTo_Implementation(const FString &ErrorMsg)
{

}


//Get User Session
void AOWSPlayerController::GetUserSession(FString UserSessionGUID)
{
	Http = &FHttpModule::Get();
	Http->SetHttpTimeout(TravelTimeout); //Set timeout

	//UE_LOG(LogTemp, Warning, TEXT("CustomerID: %s"), *RPGAPICustomerKey);

	TSharedRef<IHttpRequest> Request = Http->CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &AOWSPlayerController::OnGetUserSessionResponseReceived);
	//This is the url on which to process the request
	//FString url = FString(TEXT("http://" + RPGAPIPath + "/RPGUser/GetUserSession/")) + UserSessionGUID;
	FString url = FString(OWS2APIPath + "api/Users/GetUserSession?UserSessionGUID=" + UserSessionGUID);
	
	Request->SetURL(url);
	Request->SetVerb("GET");
	Request->SetHeader(TEXT("User-Agent"), "X-UnrealEngine-Agent");
	Request->SetHeader("Content-Type", TEXT("application/json"));
	Request->SetHeader(TEXT("X-CustomerGUID"), RPGAPICustomerKey);
	Request->ProcessRequest();
}

void AOWSPlayerController::OnGetUserSessionResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	FString ServerAndPort;

	if (bWasSuccessful)
	{
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());

		if (FJsonSerializer::Deserialize(Reader, JsonObject))
		{
			FString CharacterName = JsonObject->GetStringField("CharName");
			FString Email = JsonObject->GetStringField("Email");
			FString FirstName = JsonObject->GetStringField("FirstName");
			FString LastName = JsonObject->GetStringField("LastName");
			FString ZoneName = JsonObject->GetStringField("ZoneName");
			float X = JsonObject->GetNumberField("X");
			float Y = JsonObject->GetNumberField("Y");
			float Z = JsonObject->GetNumberField("Z");
			float RX = JsonObject->GetNumberField("RX");
			float RY = JsonObject->GetNumberField("RY");
			float RZ = JsonObject->GetNumberField("RZ");

			UE_LOG(OWS, Log, TEXT("GetUserSession location is %f, %f, %f"), X, Y, Z);

			if (CharacterName.IsEmpty())
			{
				ErrorGetUserSession(TEXT("Cannot find User Session!"));
				return;
			}

			NotifyGetUserSession(CharacterName, Email, FirstName, LastName, ZoneName, X, Y, Z, RX, RY, RZ);
		}
		else
		{
			UE_LOG(OWS, Error, TEXT("OnGetUserSessionResponseReceived Server returned no data!"));
			ErrorGetUserSession(TEXT("There was a problem connecting to the server.  Please try again."));
		}
	}
	else
	{
		UE_LOG(OWS, Error, TEXT("OnGetUserSessionResponseReceived Error accessing server!"));
		ErrorGetUserSession(TEXT("Unknown error connecting to server!"));
	}
}

void AOWSPlayerController::NotifyGetUserSession_Implementation(const FString &CharacterName, const FString &Email, const FString &FirstName, const FString &LastName, const FString &ZoneName,
	const float &X, const float &Y, const float &Z, const float &RX, const float &RY, const float &RZ)
{

}

void AOWSPlayerController::ErrorGetUserSession_Implementation(const FString &ErrorMsg)
{

}

//UserSessionSetSelectedCharacter
void AOWSPlayerController::UserSessionSetSelectedCharacter(FString UserSessionGUID, FString SelectedCharacterName)
{
	Http = &FHttpModule::Get();
	Http->SetHttpTimeout(TravelTimeout); //Set timeout

	TSharedRef<IHttpRequest> Request = Http->CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &AOWSPlayerController::OnUserSessionSetSelectedCharacterResponseReceived);
	//This is the url on which to process the request
	FString url = FString(OWS2APIPath + "api/Users/UserSessionSetSelectedCharacter");

	//Trim whitespace
	SelectedCharacterName.TrimStartAndEndInline();

	TArray<FStringFormatArg> FormatParams;
	FormatParams.Add(UserSessionGUID);
	FormatParams.Add(SelectedCharacterName);
	FString PostParameters = FString::Format(TEXT("{ \"UserSessionGUID\": \"{0}\", \"SelectedCharacterName\": \"{1}\" }"), FormatParams);

	Request->SetURL(url);
	Request->SetVerb("POST");
	Request->SetHeader(TEXT("User-Agent"), "X-UnrealEngine-Agent");
	Request->SetHeader("Content-Type", TEXT("application/json"));
	Request->SetHeader(TEXT("X-CustomerGUID"), RPGAPICustomerKey);
	Request->SetContentAsString(PostParameters);
	Request->ProcessRequest();

	/*
	FString url = FString(TEXT("http://" + RPGAPIPath + "/RPGUser/UserSessionSetSelectedCharacter/")) + UserSessionGUID;

	FString PostParameters = FString(TEXT("SelectedCharacterName=")) + SelectedCharacterName
		+ FString(TEXT("&CustomerGUID=")) + RPGAPICustomerKey;


	Request->SetURL(url);
	Request->SetVerb("POST");
	Request->SetHeader(TEXT("User-Agent"), "X-UnrealEngine-Agent");
	Request->SetHeader("Content-Type", TEXT("application/x-www-form-urlencoded"));
	Request->SetContentAsString(PostParameters);
	Request->ProcessRequest();
	*/
}

void AOWSPlayerController::OnUserSessionSetSelectedCharacterResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	FString ServerAndPort;

	if (bWasSuccessful)
	{
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());

		if (FJsonSerializer::Deserialize(Reader, JsonObject))
		{
			NotifyUserSessionSetSelectedCharacter();
		}
		else
		{
			UE_LOG(OWS, Error, TEXT("OnUserSessionSetSelectedCharacterResponseReceived Server returned no data!"));
			ErrorUserSessionSetSelectedCharacter(TEXT("There was a problem connecting to the server.  Please try again."));
		}
	}
	else
	{
		UE_LOG(OWS, Error, TEXT("OnUserSessionSetSelectedCharacterResponseReceived Error accessing server!"));
		ErrorUserSessionSetSelectedCharacter(TEXT("Unknown error connecting to server!"));
	}
}

//Get Cosmetic Custom Character Data
void AOWSPlayerController::GetCosmeticCustomCharacterData(FString UserSessionGUID, FString CharacterName)
{
	//AOWSPlayerController* PC = Cast<AOWSPlayerController>(this->Controller);
	FString PlayerName = CharacterName;
	PlayerName = PlayerName.Replace(TEXT(" "), TEXT("%20"));

	TSharedRef<IHttpRequest> Request = Http->CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &AOWSPlayerController::OnGetCosmeticCustomCharacterDataResponseReceived);
	//This is the url on which to process the request
	FString url = FString(TEXT("http://" + RPGAPIPath + "/RPGServer/GetCustomCharacterData"));

	FString PostParameters = FString(TEXT("id="))  + PlayerName
		+ FString(TEXT("&CustomerGUID=")) + RPGAPICustomerKey;

	Request->SetURL(url);
	Request->SetVerb("POST");
	Request->SetHeader(TEXT("User-Agent"), "X-UnrealEngine-Agent");
	Request->SetHeader("Content-Type", TEXT("application/x-www-form-urlencoded"));
	Request->SetContentAsString(PostParameters);
	Request->ProcessRequest();
}

void AOWSPlayerController::OnGetCosmeticCustomCharacterDataResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());

		if (FJsonSerializer::Deserialize(Reader, JsonObject))
		{
			TArray<FCustomCharacterDataStruct> CustomCharacterData;

			if (JsonObject->HasField("rows"))
			{
				TArray<TSharedPtr<FJsonValue>> Rows = JsonObject->GetArrayField("rows");

				for (int RowNum = 0; RowNum != Rows.Num(); RowNum++) {
					FCustomCharacterDataStruct tempCustomData;
					TSharedPtr<FJsonObject> tempRow = Rows[RowNum]->AsObject();
					tempCustomData.CustomFieldName = tempRow->GetStringField("CustomFieldName").Replace(TEXT("COSMETIC_"), TEXT(""));
					tempCustomData.FieldValue = tempRow->GetStringField("FieldValue");

					CustomCharacterData.Add(tempCustomData);
				}
			}

			NotifyGetCosmeticCustomCharacterData(CustomCharacterData);
		}
		else
		{
			UE_LOG(OWS, Error, TEXT("OnGetCharacterStatusesResponseReceived Server returned no data!"));
		}
	}
	else
	{
		UE_LOG(OWS, Error, TEXT("OnGetCharacterStatusesResponseReceived Error accessing server!"));
	}
}


void AOWSPlayerController::AddOrUpdateCosmeticCustomCharacterData(FString UserSessionGUID, FString CharacterName, FString CustomFieldName, FString CustomValue)
{
	TSharedRef<IHttpRequest> Request = Http->CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &AOWSPlayerController::OnAddOrUpdateCosmeticCustomCharacterDataResponseReceived);

	CharacterName = CharacterName.Replace(TEXT(" "), TEXT("%20"));
	//This is the url on which to process the request
	FString url = FString(TEXT("http://" + RPGAPIPath + "/RPGServer/AddOrUpdateCosmeticCharacterData"));

	FString PostParameters = FString(TEXT("id=")) + CharacterName
		+ FString(TEXT("&UserSessionGUID=")) + UserSessionGUID
		+ FString(TEXT("&FieldName=")) + CustomFieldName
		+ FString(TEXT("&FieldValue=")) + CustomValue
		+ FString(TEXT("&CustomerGUID=")) + RPGAPICustomerKey;

	UE_LOG(LogTemp, Log, TEXT("OnAddOrUpdateCosmeticCustomCharacterDataResponseReceived - PlayerName: %s, FieldName: %s, Value: %s"), *CharacterName, *CustomFieldName, *CustomValue);

	Request->SetURL(url);
	Request->SetVerb("POST");
	Request->SetHeader(TEXT("User-Agent"), "X-UnrealEngine-Agent");
	Request->SetHeader("Content-Type", TEXT("application/x-www-form-urlencoded"));
	Request->SetContentAsString(PostParameters);
	Request->ProcessRequest();
}

void AOWSPlayerController::OnAddOrUpdateCosmeticCustomCharacterDataResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());

		if (FJsonSerializer::Deserialize(Reader, JsonObject))
		{
			UE_LOG(OWS, Verbose, TEXT("OnAddOrUpdateCosmeticCustomCharacterDataResponseReceived Success!"));
		}
		else
		{
			UE_LOG(OWS, Error, TEXT("OnAddOrUpdateCosmeticCustomCharacterDataResponseReceived Server returned no data!"));
		}
	}
	else
	{
		UE_LOG(OWS, Error, TEXT("OnAddOrUpdateCosmeticCustomCharacterDataResponseReceived Error accessing server!"));
	}
}



void AOWSPlayerController::JoinQueue(FString UserSessionGUID, FString CharacterName, FString QueueName)
{
	//AOWSPlayerController* PC = Cast<AOWSPlayerController>(this->Controller);
	FString PlayerName = CharacterName;
	PlayerName = PlayerName.Replace(TEXT(" "), TEXT("%20"));

	TSharedRef<IHttpRequest> Request = Http->CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &AOWSPlayerController::OnJoinQueueResponseReceived);
	//This is the url on which to process the request
	FString url = FString(TEXT("http://" + RPGAPIPath + "/RPGUser/JoinQueue"));

	FString PostParameters = FString(TEXT("id=")) + PlayerName
		+ FString(TEXT("&UserSessionGUID=")) + UserSessionGUID
		+ FString(TEXT("&QueueName=")) + QueueName
		+ FString(TEXT("&CustomerGUID=")) + RPGAPICustomerKey;

	Request->SetURL(url);
	Request->SetVerb("POST");
	Request->SetHeader(TEXT("User-Agent"), "X-UnrealEngine-Agent");
	Request->SetHeader("Content-Type", TEXT("application/x-www-form-urlencoded"));
	Request->SetContentAsString(PostParameters);
	Request->ProcessRequest();
}

void AOWSPlayerController::OnJoinQueueResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());

		if (FJsonSerializer::Deserialize(Reader, JsonObject))
		{
			FString Success = JsonObject->GetStringField("success");

			if (Success == "true")
			{
				int32 SuccessCode = JsonObject->GetNumberField("successcode");
				UE_LOG(OWS, Verbose, TEXT("OnJoinQueueResponseReceived Success!"));
				NotifyJoinQueue(SuccessCode);
			}
			else
			{
				FString ErrorMessage = JsonObject->GetStringField("errmsg");
				UE_LOG(OWS, Verbose, TEXT("OnJoinQueueResponseReceived Error: %s"), *ErrorMessage);
				ErrorJoinQueue(ErrorMessage);
			}
		}
		else
		{
			UE_LOG(OWS, Error, TEXT("OnJoinQueueResponseReceived Server returned no data!"));
		}
	}
	else
	{
		UE_LOG(OWS, Error, TEXT("OnJoinQueueResponseReceived Error accessing server!"));
	}
}



void AOWSPlayerController::LeaveQueue(FString UserSessionGUID, FString CharacterName, FString QueueName)
{
	//AOWSPlayerController* PC = Cast<AOWSPlayerController>(this->Controller);
	FString PlayerName = CharacterName;
	PlayerName = PlayerName.Replace(TEXT(" "), TEXT("%20"));

	TSharedRef<IHttpRequest> Request = Http->CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &AOWSPlayerController::OnLeaveQueueResponseReceived);
	//This is the url on which to process the request
	FString url = FString(TEXT("http://" + RPGAPIPath + "/RPGUser/LeaveQueue"));

	FString PostParameters = FString(TEXT("id=")) + PlayerName
		+ FString(TEXT("&UserSessionGUID=")) + UserSessionGUID
		+ FString(TEXT("&QueueName=")) + QueueName
		+ FString(TEXT("&CustomerGUID=")) + RPGAPICustomerKey;

	Request->SetURL(url);
	Request->SetVerb("POST");
	Request->SetHeader(TEXT("User-Agent"), "X-UnrealEngine-Agent");
	Request->SetHeader("Content-Type", TEXT("application/x-www-form-urlencoded"));
	Request->SetContentAsString(PostParameters);
	Request->ProcessRequest();
}

void AOWSPlayerController::OnLeaveQueueResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());

		if (FJsonSerializer::Deserialize(Reader, JsonObject))
		{
			FString Success = JsonObject->GetStringField("success");

			if (Success == "true")
			{
				int32 SuccessCode = JsonObject->GetNumberField("successcode");
				UE_LOG(OWS, Verbose, TEXT("OnLeaveQueueResponseReceived Success!"));
				NotifyLeaveQueue(SuccessCode);
			}
			else
			{
				FString ErrorMessage = JsonObject->GetStringField("errmsg");
				UE_LOG(OWS, Verbose, TEXT("OnLeaveQueueResponseReceived Error: %s"), *ErrorMessage);
				ErrorLeaveQueue(ErrorMessage);
			}
		}
		else
		{
			UE_LOG(OWS, Error, TEXT("OnLeaveQueueResponseReceived Server returned no data!"));
		}
	}
	else
	{
		UE_LOG(OWS, Error, TEXT("OnLeaveQueueResponseReceived Error accessing server!"));
	}
}


void AOWSPlayerController::GetPlayerGroupsCharacterIsIn(FString UserSessionGUID, FString CharacterName, int32 PlayerGroupTypeID)
{

	Http = &FHttpModule::Get();
	Http->SetHttpTimeout(TravelTimeout); //Set timeout

	//AOWSPlayerController* PC = Cast<AOWSPlayerController>(this->Controller);
	FString PlayerName = CharacterName;
	PlayerName = PlayerName.Replace(TEXT(" "), TEXT("%20"));

	TSharedRef<IHttpRequest> Request = Http->CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &AOWSPlayerController::OnGetPlayerGroupsCharacterIsInResponseReceived);
	//This is the url on which to process the request
	/*FString url = FString(TEXT("http://" + RPGAPIPath + "/RPGUser/GetPlayerGroupsCharacterIsIn"));

	FString PostParameters = FString(TEXT("id=")) + PlayerName
		+ FString(TEXT("&UserSessionGUID=")) + UserSessionGUID
		+ FString(TEXT("&PlayerGroupTypeID=")) + FString::FromInt(PlayerGroupTypeID)
		+ FString(TEXT("&CustomerGUID=")) + RPGAPICustomerKey;

	Request->SetURL(url);
	Request->SetVerb("POST");
	Request->SetHeader(TEXT("User-Agent"), "X-UnrealEngine-Agent");
	Request->SetHeader("Content-Type", TEXT("application/x-www-form-urlencoded"));
	Request->SetContentAsString(PostParameters);
	Request->ProcessRequest();*/

	FString url = FString(OWS2APIPath + "api/Users/GetPlayerGroupsCharacterIsIn");

	//Trim whitespace
	PlayerName.TrimStartAndEndInline();

	TArray<FStringFormatArg> FormatParams;
	FormatParams.Add(UserSessionGUID);
	FormatParams.Add(CharacterName);
	FormatParams.Add(PlayerGroupTypeID);
	FString PostParameters = FString::Format(TEXT("{ \"UserSessionGUID\": \"{0}\", \"CharacterName\": \"{1}\",  \"PlayerGroupTypeID\": \"{2}\" }"), FormatParams);

	Request->SetURL(url);
	Request->SetVerb("POST");
	Request->SetHeader(TEXT("User-Agent"), "X-UnrealEngine-Agent");
	Request->SetHeader("Content-Type", TEXT("application/json"));
	Request->SetHeader(TEXT("X-CustomerGUID"), RPGAPICustomerKey);
	Request->SetContentAsString(PostParameters);
	Request->ProcessRequest();
}

void AOWSPlayerController::OnGetPlayerGroupsCharacterIsInResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());

		if (FJsonSerializer::Deserialize(Reader, JsonObject))
		{
			FString Success = JsonObject->GetStringField("success");

			if (Success == "true")
			{

				if (JsonObject->HasField("rows"))
				{
					TArray<TSharedPtr<FJsonValue>> Rows = JsonObject->GetArrayField("rows");
					TArray<FPlayerGroup> tempPlayerGroups;

					PlayerGroupsPlayerIsIn.Empty();

					for (int RowNum = 0; RowNum != Rows.Num(); RowNum++) {
						FPlayerGroup tempPlayerGroup;
						TSharedPtr<FJsonObject> tempRow = Rows[RowNum]->AsObject();
						tempPlayerGroup.PlayerGroupID = tempRow->GetNumberField("PlayerGroupID");
						tempPlayerGroup.PlayerGroupName = tempRow->GetStringField("PlayerGroupName");
						tempPlayerGroup.PlayerGroupTypeID = tempRow->GetNumberField("PlayerGroupTypeID");
						tempPlayerGroup.ReadyState = tempRow->GetNumberField("ReadyState");
						tempPlayerGroup.TeamNumber = tempRow->GetNumberField("TeamNumber");

						FDateTime OutDateTime;
						FDateTime::Parse(tempRow->GetStringField("DateAdded"), OutDateTime);
						tempPlayerGroup.DateAdded = OutDateTime;

						PlayerGroupsPlayerIsIn.Add(tempPlayerGroup);
						tempPlayerGroups.Add(tempPlayerGroup);
					}

					NotifyGetPlayerGroupsCharacterIsIn(tempPlayerGroups);
				}
				ErrorGetPlayerGroupsCharacterIsIn(TEXT("OnGetPlayerGroupsCharacterIsInResponseReceived No rows in JSON!"));
			}
			else
			{
				FString ErrorMessage = JsonObject->GetStringField("errmsg");
				ErrorGetPlayerGroupsCharacterIsIn(ErrorMessage);
			}
		}
		else
		{
			UE_LOG(OWS, Error, TEXT("OnGetPlayerGroupsCharacterIsInResponseReceived Server returned no data!"));
		}
	}
	else
	{
		UE_LOG(OWS, Error, TEXT("OnGetPlayerGroupsCharacterIsInResponseReceived Error accessing server!"));
	}
}

void AOWSPlayerController::LeaveAllMatches(FString CharacterName)
{

	Http = &FHttpModule::Get();
	Http->SetHttpTimeout(TravelTimeout); //Set timeout
	//AOWSPlayerController* PC = Cast<AOWSPlayerController>(this->Controller);
	FString PlayerName = CharacterName;
	PlayerName = PlayerName.Replace(TEXT(" "), TEXT("%20"));

	TSharedRef<IHttpRequest> Request = Http->CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &AOWSPlayerController::OnLeaveAllMatchesResponseReceived);
	//This is the url on which to process the request
	FString url = FString(TEXT("http://" + RPGAPIPath + "/RPGUser/LeaveAllMatches"));

	FString PostParameters = FString(TEXT("id=")) + PlayerName
		+ FString(TEXT("&CustomerGUID=")) + RPGAPICustomerKey;

	Request->SetURL(url);
	Request->SetVerb("POST");
	Request->SetHeader(TEXT("User-Agent"), "X-UnrealEngine-Agent");
	Request->SetHeader("Content-Type", TEXT("application/x-www-form-urlencoded"));
	Request->SetContentAsString(PostParameters);
	Request->ProcessRequest();
}

void AOWSPlayerController::OnLeaveAllMatchesResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());

		if (FJsonSerializer::Deserialize(Reader, JsonObject))
		{
			FString Success = JsonObject->GetStringField("success");

			if (Success == "true")
			{
				NotifyLeaveAllMatches();
			}
			else
			{
				FString ErrorMessage = JsonObject->GetStringField("errmsg");
				ErrorLeaveAllMatches(ErrorMessage);
			}
		}
		else
		{
			UE_LOG(OWS, Error, TEXT("OnLeaveQueueResponseReceived Server returned no data!"));
		}
	}
	else
	{
		UE_LOG(OWS, Error, TEXT("OnLeaveQueueResponseReceived Error accessing server!"));
	}
}



void AOWSPlayerController::GetMapServerToTravelTo(FString ZoneName)
{
	Http = &FHttpModule::Get();
	Http->SetHttpTimeout(TravelTimeout); //Set timeout

	FString PlayerName = PlayerState->GetPlayerName();
	PlayerName = PlayerName.Replace(TEXT(" "), TEXT("%20"));

	UE_LOG(LogTemp, Warning, TEXT("PlayerName: %s"), *PlayerName);
	UE_LOG(LogTemp, Warning, TEXT("ZoneName: %s"), *ZoneName);

	TSharedRef<IHttpRequest> Request = Http->CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &AOWSPlayerController::OnGetMapServerToTravelToResponseReceived);
	//This is the url on which to process the request
	FString url = FString(TEXT("http://" + RPGAPIPath + "/RPGServer/GetServerToConnectTo"));

	FString PostParameters = FString(TEXT("id=")) + PlayerName
		+ FString(TEXT("&ZoneName=")) + ZoneName
		+ FString(TEXT("&CustomerGUID=")) + RPGAPICustomerKey;

	Request->SetURL(url);
	Request->SetVerb("POST");
	Request->SetHeader(TEXT("User-Agent"), "X-UnrealEngine-Agent");
	Request->SetHeader("Content-Type", TEXT("application/x-www-form-urlencoded"));
	Request->SetContentAsString(PostParameters);
	Request->ProcessRequest();
}

void AOWSPlayerController::OnGetMapServerToTravelToResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	FString ServerAndPort;

	if (bWasSuccessful)
	{
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());

		if (FJsonSerializer::Deserialize(Reader, JsonObject))
		{
			FString ServerIP = JsonObject->GetStringField("serverip");
			FString Port = JsonObject->GetStringField("port");

			if (ServerIP.IsEmpty() || Port.IsEmpty())
			{
				UE_LOG(OWS, Error, TEXT("OnGetMapServerToTravelToResponseReceived Cannot connect to server!  IP or Port is empty!"));
				ErrorMapServerToTravelTo(TEXT("Cannot connect to server!"));
				return;
			}

			ServerAndPort = ServerIP + FString(TEXT(":")) + Port.Left(4);

			UE_LOG(OWS, Warning, TEXT("OnGetMapServerToTravelToResponseReceived ServerAndPort: %s"), *ServerAndPort);

			NotifyMapServerToTravelTo(ServerAndPort);
		}
		else
		{
			UE_LOG(OWS, Error, TEXT("OnGetMapServerToTravelToResponseReceived Server returned no data!  This usually means the server did not spin up.  Check the RPG World Server app to make sure that it spun up the right server.  If it didn't, check the dashboard log to see if a server was even asked for."));
			ErrorMapServerToTravelTo(TEXT("There was a problem connecting to the server.  Please try again."));
		}
	}
	else
	{
		UE_LOG(OWS, Error, TEXT("OnGetMapServerToTravelToResponseReceived Error accessing server!"));
		ErrorMapServerToTravelTo(TEXT("Unknown error connecting to server!"));
	}
}

//Remove Character
void AOWSPlayerController::RemoveCharacter(FString UserSessionGUID, FString CharacterName)
{
	//AOWSPlayerController* PC = Cast<AOWSPlayerController>(this->Controller);
	FString PlayerName = CharacterName;
	PlayerName = PlayerName.Replace(TEXT(" "), TEXT("%20"));

	TSharedRef<IHttpRequest> Request = Http->CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &AOWSPlayerController::OnRemoveCharacterResponseReceived);
	//This is the url on which to process the request
	FString url = FString(TEXT("http://" + RPGAPIPath + "/RPGUser/RemoveCharacter"));

	FString PostParameters = FString(TEXT("id=")) + UserSessionGUID
		+ FString(TEXT("&CharacterName=")) + PlayerName
		+ FString(TEXT("&CustomerGUID=")) + RPGAPICustomerKey;

	Request->SetURL(url);
	Request->SetVerb("POST");
	Request->SetHeader(TEXT("User-Agent"), "X-UnrealEngine-Agent");
	Request->SetHeader("Content-Type", TEXT("application/x-www-form-urlencoded"));
	Request->SetContentAsString(PostParameters);
	Request->ProcessRequest();
}

void AOWSPlayerController::OnRemoveCharacterResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());

		if (FJsonSerializer::Deserialize(Reader, JsonObject))
		{
			FString Success = JsonObject->GetStringField("success");

			if (Success == "true")
			{
				UE_LOG(OWS, Verbose, TEXT("OnRemoveCharacterResponseReceived Success!"));
				NotifyRemoveCharacter();
			}
			else
			{
				FString ErrorMessage = JsonObject->GetStringField("ErrorMessage");
				UE_LOG(OWS, Verbose, TEXT("OnRemoveCharacterResponseReceived Error: %s"), *ErrorMessage);
				ErrorRemoveCharacter(ErrorMessage);
			}
		}
		else
		{
			UE_LOG(OWS, Error, TEXT("OnRemoveCharacterResponseReceived Server returned no data!"));
		}
	}
	else
	{
		UE_LOG(OWS, Error, TEXT("OnRemoveCharacterResponseReceived Error accessing server!"));
	}
}

AOWSGameMode* AOWSPlayerController::GetGameMode()
{
	return (AOWSGameMode*)GetWorld()->GetAuthGameMode();
}

void AOWSPlayerController::SynchUpLocalMeshItemsMap()
{
	UOWSGameInstance* GameInstance = Cast<UOWSGameInstance>(GetWorld()->GetGameInstance());

	if (!GameInstance)
		return;

	AOWSGameMode* OWSGameMode = GetGameMode();

	if (!OWSGameMode)
		return;

	for (auto& MapItem : OWSGameMode->MeshItemsMap)
	{
		AddItemToLocalMeshItemsMap(MapItem.Key, MapItem.Value);
	}
}

void AOWSPlayerController::AddItemToLocalMeshItemsMap(const FString& ItemName, const int32 ItemMeshID)
{
	UOWSGameInstance* GameInstance = Cast<UOWSGameInstance>(GetWorld()->GetGameInstance());

	if (!GameInstance)
		return;

	if (!LocalMeshItemsMap.Contains(ItemName))
	{
		LocalMeshItemsMap.Add(ItemName, ItemMeshID);

		Client_AddItemToLocalMeshItemsMap(ItemName, ItemMeshID);
	}
}

void AOWSPlayerController::Client_AddItemToLocalMeshItemsMap_Implementation(const FString& ItemName, const int32 ItemMeshID)
{
	UOWSGameInstance* GameInstance = Cast<UOWSGameInstance>(GetWorld()->GetGameInstance());

	if (!GameInstance)
		return;

	GameInstance->LocalMeshItemsMap.Add(ItemName, ItemMeshID);
}

void AOWSPlayerController::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//DOREPLIFETIME_CONDITION(AOWSPlayerController, MaxPredictionPing, COND_OwnerOnly);
	//DOREPLIFETIME_CONDITION(AOWSPlayerController, PredictionFudgeFactor, COND_OwnerOnly);
}
