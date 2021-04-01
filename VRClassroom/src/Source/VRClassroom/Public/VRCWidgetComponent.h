// Copyright 2020 Testy

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "VRCWidgetComponent.generated.h"

/**
 * 
 */
UCLASS(ClassGroup = (Custom), Blueprintable, meta = (BlueprintSpawnableComponent))
class VRCLASSROOM_API UVRCWidgetComponent : public UWidgetComponent
{
	GENERATED_BODY()

	bool bToggleWidget;

public:

	UFUNCTION(BlueprintCallable, Category = "Widget")
		void ToggleWidget(bool toggle);
};
