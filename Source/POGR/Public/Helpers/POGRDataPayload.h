#pragma once

#include "CoreMinimal.h"
#include "POGRURLFilters.h"
#include "POGRDataPayload.generated.h"

USTRUCT(BlueprintType)
struct FDataPayload
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Pogr Data Payload")
	FString Id;

	UPROPERTY(BlueprintReadWrite, Category = "Pogr Data Payload")
	FString ClientId;

	UPROPERTY(BlueprintReadWrite, Category = "Pogr Data Payload")
	FString BuildId;

	UPROPERTY(BlueprintReadWrite, Category = "Pogr Data Payload")
	TEnumAsByte<EAcceptedStatus> AcceptedStatus;

	UPROPERTY(BlueprintReadWrite, Category = "Pogr Data Payload")
	FString DataReceivedDate;

	UPROPERTY(BlueprintReadWrite, Category = "Pogr Data Payload")
	int32 Calls;
};