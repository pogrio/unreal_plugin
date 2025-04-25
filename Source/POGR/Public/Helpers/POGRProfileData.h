#pragma once

#include "CoreMinimal.h"
#include "POGRProfileData.generated.h"

class UTexture2DDynamic;

USTRUCT(BlueprintType)
struct FUserProfileData
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadOnly, Category = "Pogr Profile Data")
	FString UserName;
	FString DisplayName;

	UPROPERTY(BlueprintReadOnly, Category = "Pogr Profile Data")
	FString AvatarURL;
	int32 Level;
	int32 Exp;
	int32 RequiredExp;

	UPROPERTY(BlueprintReadOnly, Category = "Pogr Profile Data")
	UTexture2DDynamic* ProfileImage;
};