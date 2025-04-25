#pragma once

#include "CoreMinimal.h"
#include "POGRGameLogs.generated.h"

USTRUCT(BlueprintType)
struct FGameLogData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Pogr Game Logs")
    FString user_id;

    UPROPERTY(BlueprintReadWrite, Category = "Pogr Game Logs")
    FString timestamp;

    UPROPERTY(BlueprintReadWrite, Category = "Pogr Game Logs")
    FString ip_address;
};

USTRUCT(BlueprintType)
struct FGameLogTags
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Pogr Game Logs")
    FString system;

    UPROPERTY(BlueprintReadWrite, Category = "Pogr Game Logs")
    FString action;
};

USTRUCT(BlueprintType)
struct FGameLog
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Pogr Game Logs")
    FString service;

    UPROPERTY(BlueprintReadWrite, Category = "Pogr Game Logs")
    FString environment;

    UPROPERTY(BlueprintReadWrite, Category = "Pogr Game Logs")
    FString severity;

    UPROPERTY(BlueprintReadWrite, Category = "Pogr Game Logs")
    FString type;

    UPROPERTY(BlueprintReadWrite, Category = "Pogr Game Logs")
    FString log;

    UPROPERTY(BlueprintReadWrite, Category = "Pogr Game Logs")
    FGameLogData data;

    UPROPERTY(BlueprintReadWrite, Category = "Pogr Game Logs")
    FGameLogTags tags;
};