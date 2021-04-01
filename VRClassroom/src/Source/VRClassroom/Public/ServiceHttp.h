// Copyright 2020 Testy

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ServiceHttp.generated.h"

UCLASS()
class VRCLASSROOM_API AServiceHttp : public AActor
{
	GENERATED_BODY()

	
public:	
	// Sets default values for this actor's properties
	AServiceHttp();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
