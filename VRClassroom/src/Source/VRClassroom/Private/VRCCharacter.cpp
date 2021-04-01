// Copyright 2020 Testy


#include "VRCCharacter.h"

// Sets default values
AVRCCharacter::AVRCCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AVRCCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AVRCCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AVRCCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

UVRCGameInstance* AVRCCharacter::GetVRCGameInstance() const {
	return Cast<UVRCGameInstance>(GetWorld()->GetGameInstance());
}

bool AVRCCharacter::IsPlayerLoggedIn() const {
	return AVRCCharacter::GetVRCGameInstance()->User.bIsLoggedIn;
}

