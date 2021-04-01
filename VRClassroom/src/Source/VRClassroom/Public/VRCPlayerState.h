// Copyright 2020 Testy

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "VRCPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class VRCLASSROOM_API AVRCPlayerState : public APlayerState
{
	GENERATED_BODY()

public:

	AVRCPlayerState(const class FObjectInitializer& ObjectInitializer);

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "User", Replicated)
		int UserID;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "User", Replicated)
		FString UserName;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "User", Replicated)
		FString DisplayName;
	
	void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const;
};
