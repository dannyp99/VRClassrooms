// Copyright 2020 Testy

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VRCWidgetComponent.h"
#include "VRCWidgetActor.generated.h"

UCLASS()
class VRCLASSROOM_API AVRCWidgetActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AVRCWidgetActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UVRCWidgetComponent* WidgetVRC;

	UFUNCTION(BlueprintCallable, Category = "Widget")
		void ToggleWidget(bool bActive);

	UFUNCTION(BlueprintImplementableEvent, Category = "Widget")
		void BeforeToggleWidget(bool bActive);

};
