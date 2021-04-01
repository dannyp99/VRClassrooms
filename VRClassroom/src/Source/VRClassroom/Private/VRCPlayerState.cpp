// Copyright 2020 Testy


#include "VRCPlayerState.h"
#include "UnrealNetwork.h"

AVRCPlayerState::AVRCPlayerState(const class FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    bReplicates = true;
}

void AVRCPlayerState::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AVRCPlayerState, UserID);
    DOREPLIFETIME(AVRCPlayerState, UserName);
    DOREPLIFETIME(AVRCPlayerState, DisplayName);
}