#pragma once

#include "CoreMinimal.h"
#include "POGROrganizationData.generated.h"

class UTexture2D;

USTRUCT(BlueprintType)
struct FOrganizationData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Pogr Organization Data")
	FString UUID;

	UPROPERTY(BlueprintReadOnly, Category = "Pogr Organization Data")
	FString Name;
	FString CreatedOn;
	FString Type;

	UPROPERTY(BlueprintReadOnly, Category = "Pogr Organization Data")
	FString URL;

	UPROPERTY(BlueprintReadOnly, Category = "Pogr Organization Data")
	UTexture2D* OrganizationImage;

	bool operator==(const FOrganizationData& Other) const
	{
		return UUID == Other.UUID && Name == Other.Name && CreatedOn == Other.CreatedOn && Type == Other.Type && URL == Other.URL;
	}
};