// Copyright 2020 Testy


#include "VRCWidgetComponent.h"

void UVRCWidgetComponent::ToggleWidget(bool toggle) {
	bToggleWidget = toggle;

	this->SetVisibility(bToggleWidget);

	if (bToggleWidget) {
		this->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
	else {
		this->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}