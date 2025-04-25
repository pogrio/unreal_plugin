#pragma once

#include "CoreMinimal.h"
#include "POGRGameData.generated.h"

class UTexture2D;

USTRUCT(BlueprintType)
struct FOrganizationGameData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Pogr Game Data")
	FString UUID;
	FString StudioUUID;

	UPROPERTY(BlueprintReadOnly, Category = "Pogr Game Data")
	FString GameTitle;

	UPROPERTY(BlueprintReadOnly, Category = "Pogr Game Data")
	FString URL;
	FString CreatedOn;

	UPROPERTY(BlueprintReadOnly, Category = "Pogr Game Data")
	UTexture2D* GameImage;

	bool operator==(const FOrganizationGameData& Other) const
	{
		return UUID == Other.UUID && StudioUUID == Other.StudioUUID && GameTitle == Other.GameTitle && CreatedOn == Other.CreatedOn && URL == Other.URL;
	}
};