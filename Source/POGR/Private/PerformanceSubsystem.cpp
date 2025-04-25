#include "PerformanceSubsystem.h"
#include "HAL/PlatformMemory.h"

#if PLATFORM_WINDOWS
#include "Windows/WindowsPlatformMisc.h"
#endif

const FString UPerformanceSubsystem::GetCPUName() const
{
#if PLATFORM_WINDOWS
    return FWindowsPlatformMisc::GetCPUBrand();
#else
    return TEXT("Unsupported Platform");
#endif
}

const FString UPerformanceSubsystem::GetCPUBrand() const
{
#if PLATFORM_WINDOWS
    return FWindowsPlatformMisc::GetCPUVendor();
#else
    return TEXT("Unsupported Platform");
#endif
}

const FString UPerformanceSubsystem::GetGPUName() const
{
#if PLATFORM_WINDOWS
    return FWindowsPlatformMisc::GetPrimaryGPUBrand();
#else
    return TEXT("Unsupported Platform");
#endif
}

const FString UPerformanceSubsystem::GetOSVersion() const
{
#if PLATFORM_WINDOWS
    return FWindowsPlatformMisc::GetOSVersion();
#else
    return TEXT("Unsupported Platform");
#endif
}

const FString UPerformanceSubsystem::GetUsedPhysicalMemory() const
{
    FPlatformMemoryStats MemoryStats = FPlatformMemory::GetStats();
    float UsedPhysicalMemory = static_cast<float>(MemoryStats.UsedPhysical) / (1024 * 1024 * 1024);
    return FString::Printf(TEXT("%.2f GB"), UsedPhysicalMemory);
}

const FString UPerformanceSubsystem::GetAvailablePhysicalMemory() const
{
    FPlatformMemoryStats MemoryStats = FPlatformMemory::GetStats();
    float AvailablePhysicalMemory = static_cast<float>(MemoryStats.AvailablePhysical) / (1024 * 1024 * 1024);
    return FString::Printf(TEXT("%.2f GB"), AvailablePhysicalMemory);
}

const FString UPerformanceSubsystem::GetAvailableVirtualMemory() const
{
    FPlatformMemoryStats MemoryStats = FPlatformMemory::GetStats();
    float AvailableVirtualMemory = static_cast<float>(MemoryStats.AvailableVirtual) / (1024 * 1024 * 1024);
    return FString::Printf(TEXT("%.2f GB"), AvailableVirtualMemory);
}

const FString UPerformanceSubsystem::GetUsedVirtualMemory() const
{
    FPlatformMemoryStats MemoryStats = FPlatformMemory::GetStats();
    float UsedVirtualMemory = static_cast<float>(MemoryStats.UsedVirtual) / (1024 * 1024 * 1024);
    return FString::Printf(TEXT("%.2f GB"), UsedVirtualMemory);
}

const FString UPerformanceSubsystem::GetPeakUsedVirtualMemory() const
{
    FPlatformMemoryStats MemoryStats = FPlatformMemory::GetStats();
    float PeakUsedVirtualMemory = static_cast<float>(MemoryStats.PeakUsedVirtual) / (1024 * 1024 * 1024);
    return FString::Printf(TEXT("%.2f GB"), PeakUsedVirtualMemory);
}

const FString UPerformanceSubsystem::GetPeakUsedPhysicalMemory() const
{
    FPlatformMemoryStats MemoryStats = FPlatformMemory::GetStats();
    float PeakUsedPhysicalMemory = static_cast<float>(MemoryStats.PeakUsedPhysical) / (1024 * 1024 * 1024);
    return FString::Printf(TEXT("%.2f GB"), PeakUsedPhysicalMemory);
}
