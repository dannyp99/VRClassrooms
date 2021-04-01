// Fill out your copyright notice in the Description page of Project Settings.


#include "TextureCopier.h"

UTextureCopier::UTextureCopier()
{
}

UTextureCopier::~UTextureCopier()
{
}

TArray<FColor> UTextureCopier::GetPixelsData(UTextureRenderTarget2D* source) {
	TArray<FColor> SurfData; 
	FRenderTarget* RenderTarget = source->GameThread_GetRenderTargetResource();
	RenderTarget->ReadPixels(SurfData);

	return SurfData;
}

UTexture2D* UTextureCopier::CopyToTexture(const int32 sizeX, const int32 sizeY, const bool srgb, TArray<FColor> pixelsData) {
	UTexture2D* Texture = UTexture2D::CreateTransient(sizeX, sizeY, PF_B8G8R8A8);
#if WITH_EDITORONLY_DATA
	Texture->MipGenSettings = TMGS_NoMipmaps;
#endif
	Texture->SRGB = srgb;

	void* TextureData = Texture->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
	const int32 TextureDataSize = pixelsData.Num() * 4;
	FMemory::Memcpy(TextureData, pixelsData.GetData(), TextureDataSize);
	Texture->PlatformData->Mips[0].BulkData.Unlock();

	Texture->UpdateResource();

	return Texture;
}

int32 UTextureCopier::CalculateNumOfSubArrays(const int32 arraySize) {
	return (arraySize - 1) / N + 1;
}

TArray<FColor> UTextureCopier::GetSubArray(const TArray<FColor> colorArray, const int32 arraySize, const int32 chunkNum) {
	int32 start = chunkNum * N;
	int32 end = chunkNum * N + N;

	subArray.Empty();

	int32 size = N;

	if (end > arraySize) {
		end = arraySize - 1;
		size = arraySize - chunkNum * N;
	}

	for (int i = 0; i < size; i++) {
		subArray.Add(colorArray[start + i]);
	}

	return subArray;
}