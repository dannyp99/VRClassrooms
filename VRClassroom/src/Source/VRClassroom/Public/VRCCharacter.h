// Copyright 2020 Testy

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "VRCGameInstance.h"
#include "VRCCharacter.generated.h"

UCLASS()
class VRCLASSROOM_API AVRCCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AVRCCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintCallable, Category = "Instance")
		UVRCGameInstance* GetVRCGameInstance() const;
	UFUNCTION(BlueprintCallable, Category = "Instance")
		bool IsPlayerLoggedIn() const;

};
