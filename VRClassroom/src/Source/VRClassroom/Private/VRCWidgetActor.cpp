// Copyright 2020 Testy


#include "VRCWidgetActor.h"

// Sets default values
AVRCWidgetActor::AVRCWidgetActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AVRCWidgetActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AVRCWidgetActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AVRCWidgetActor::ToggleWidget(bool bActive) {
	if (!WidgetVRC) return;

	BeforeToggleWidget(bActive);

	WidgetVRC->ToggleWidget(bActive);

}