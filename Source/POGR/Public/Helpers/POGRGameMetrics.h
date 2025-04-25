#pragma once

#include "CoreMinimal.h"
#include "POGRGameMetrics.generated.h"

USTRUCT(BlueprintType)
struct FGameMetricsData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Pogr Game Metrics")
    int32 players_online = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Pogr Game Metrics")
    float average_latency_ms = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Pogr Game Metrics")
    float server_load_percentage = 0.0f;
};

USTRUCT(BlueprintType)
struct FGameTags
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Pogr Game Metrics")
    FString location;

    UPROPERTY(BlueprintReadWrite, Category = "Pogr Game Metrics")
    FString game_mode;
};

USTRUCT(BlueprintType)
struct FGameMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Pogr Game Metrics")
    FString service;

    UPROPERTY(BlueprintReadWrite, Category = "Pogr Game Metrics")
    FString environment;

    UPROPERTY(BlueprintReadWrite, Category = "Pogr Game Metrics")
    FGameMetricsData metrics;

    UPROPERTY(BlueprintReadWrite, Category = "Pogr Game Metrics")
    FGameTags tags;
};