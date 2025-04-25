#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PerformanceSubsystem.generated.h"

UCLASS(BlueprintType)
class UPerformanceSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Performance Subsystem")
	const FString GetCPUName() const;

	UFUNCTION(BlueprintPure, Category = "Performance Subsystem")
	const FString GetCPUBrand() const;

	UFUNCTION(BlueprintPure, Category = "Performance Subsystem")
	const FString GetGPUName() const;

	UFUNCTION(BlueprintPure, Category = "Performance Subsystem")
	const FString GetOSVersion() const;

	UFUNCTION(BlueprintPure, Category = "Performance Subsystem")
	const FString GetUsedPhysicalMemory() const;

	UFUNCTION(BlueprintPure, Category = "Performance Subsystem")
	const FString GetAvailablePhysicalMemory() const;

	UFUNCTION(BlueprintPure, Category = "Performance Subsystem")
	const FString GetAvailableVirtualMemory() const;

	UFUNCTION(BlueprintPure, Category = "Performance Subsystem")
	const FString GetUsedVirtualMemory() const;

	UFUNCTION(BlueprintPure, Category = "Performance Subsystem")
	const FString GetPeakUsedVirtualMemory() const;

	UFUNCTION(BlueprintPure, Category = "Performance Subsystem")
	const FString GetPeakUsedPhysicalMemory() const;
};