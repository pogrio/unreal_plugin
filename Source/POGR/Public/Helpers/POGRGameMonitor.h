#pragma once

#include "CoreMinimal.h"
#include "POGRGameMonitor.generated.h"

USTRUCT(BlueprintType)
struct FGameSystemSettings
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Pogr Game Monitor")
    FString graphics_quality;
};


USTRUCT(BlueprintType)
struct FGameSystemMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Pogr Game Monitor")
    float cpu_usage = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Pogr Game Monitor")
    float memory_usage = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Pogr Game Monitor")
    TArray<FString> dlls_loaded;

    UPROPERTY(BlueprintReadWrite, Category = "Pogr Game Monitor")
    FGameSystemSettings settings;
};