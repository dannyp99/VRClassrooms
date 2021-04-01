// Copyright 2020 Testy

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "HttpService.h"
#include "VRCGameInstance.generated.h"


USTRUCT(BlueprintType, Blueprintable)
struct FUserData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Http")
		FString Token;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "User")
		FString DisplayName;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "User")
		bool bIsLoggedIn;
};

UENUM(BlueprintType)
enum class EVRCGameMode : uint8 {
	Home	UMETA(DisplayName = "Home"),
	Hub		UMETA(DisplayName = "Hub"),
	Meeting	UMETA(DisplayName = "Meeting")
};
/**
 * 
 */
UCLASS()
class VRCLASSROOM_API UVRCGameInstance : public UGameInstance
{
	GENERATED_BODY()
	UHttpService* Http;

public:
	UVRCGameInstance(const class FObjectInitializer& ObjectInitializer);

	UPROPERTY(BlueprintReadOnly)
		FUserData User;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		EVRCGameMode curGameMode;

	UFUNCTION(BlueprintCallable, Category = "Http")
		UHttpService* GetHttpService();

	UFUNCTION(BlueprintCallable, Category = "Http")
		void SetUserToken(FString token);

	UFUNCTION(BlueprintCallable, Category = "Http")
		FString GetUserToken();

};
