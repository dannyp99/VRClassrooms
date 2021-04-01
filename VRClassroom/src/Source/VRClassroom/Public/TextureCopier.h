// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <memory>
#include "Engine/Texture2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "TextureCopier.generated.h"

/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class VRCLASSROOM_API UTextureCopier : public UObject
{
	GENERATED_BODY()

public:
	UTextureCopier();
	~UTextureCopier();

	UFUNCTION(BlueprintCallable)
		TArray<FColor> GetPixelsData(UTextureRenderTarget2D* source);

	UFUNCTION(BlueprintCallable)
		UTexture2D* CopyToTexture(const int32 sizeX, const int32 sizeY, const bool srgb, TArray<FColor> pixelsData);

	UFUNCTION(BlueprintCallable)
		int32 CalculateNumOfSubArrays(const int32 arraySize);

	UFUNCTION(BlueprintCallable)
		TArray<FColor> GetSubArray(const TArray<FColor> colorArray, const int32 arraySize, const int32 chunkNum);
		//FColor[] GetSubArray(const FColor[] colorArray, const int32 arraySize, const int32 chunkNum);

private:
	int32 N = 65535;
	TArray<FColor> subArray;
};
