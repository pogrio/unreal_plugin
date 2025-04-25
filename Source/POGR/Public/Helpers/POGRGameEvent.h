#pragma once

#include "CoreMinimal.h"
#include "POGRGameEvent.generated.h"

USTRUCT(BlueprintType)
struct FPlayerEventData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Pogr Game Event")
    FString player_id;

    UPROPERTY(BlueprintReadWrite, Category = "Pogr Game Event")
    FString achievement_name;
};

USTRUCT(BlueprintType)
struct FGameEvent
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Pogr Game Event")
    FString event;

    UPROPERTY(BlueprintReadWrite, Category = "Pogr Game Event")
    FString sub_event;

    UPROPERTY(BlueprintReadWrite, Category = "Pogr Game Event")
    FString event_type;

    UPROPERTY(BlueprintReadWrite, Category = "Pogr Game Event")
    FString event_flag;

    UPROPERTY(BlueprintReadWrite, Category = "Pogr Game Event")
    FString event_key;

    UPROPERTY(BlueprintReadWrite, Category = "Pogr Game Event")
    FPlayerEventData event_data;
};
