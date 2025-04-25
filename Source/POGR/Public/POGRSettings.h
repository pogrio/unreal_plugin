#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/DeveloperSettings.h"
#include "POGRSettings.generated.h"


UCLASS(config = Game, defaultconfig, meta = (DisplayName = "POGR"))
class UPOGREndpointSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UPOGREndpointSettings(const FObjectInitializer& ObjectInitializer);

	/**Get Listening Endpoint*/
	FString GetInitEndpoint() const;
	FString GetShutdownEndpoint() const;
	FString GetEventEndpoint() const;
	FString GetDataEndpoint() const;
	FString GetLogsEndpoint() const;
	FString GetMetricsEndpoint() const;
	FString GetMonitorEndpoint() const;
	FString GetOrgDataEndpoint() const;
	FString GetPogrUrlDefinationEndpoint() const;
	FString GetOrgGameDataEndpoint() const;
	FString GetOrgGameEndpoint() const;
	FString GetProfileDataEndpoint() const;

	/**Validate weather Endpoint is Valid*/
	bool IsValidIPAddress(const FString& IPAddressString) const;
	bool IsValidURL(const FString& URLString) const;

private:
	UPROPERTY(config, EditAnywhere, Category = Settings)
	FString InitEndpoint = "";
	
	UPROPERTY(config, EditAnywhere, Category = Settings)
	FString ShutdownEndpoint = "";

	UPROPERTY(config, EditAnywhere, Category = Settings)
	FString EventEndpoint = "";

	UPROPERTY(config, EditAnywhere, Category = Settings)
	FString DataEndpoint = "";

	UPROPERTY(config, EditAnywhere, Category = Settings)
	FString LogsEndpoint = "";

	UPROPERTY(config, EditAnywhere, Category = Settings)
	FString MetricsEndpoint = "";
	
	UPROPERTY(config, EditAnywhere, Category = Settings)
	FString MonitorEndpoint = "";

	UPROPERTY(config, EditAnywhere, Category = Settings)
	FString OrganizationDataEndpoint = "";

	UPROPERTY(config, EditAnywhere, Category = Settings)
	FString POGRUrlDefinationEndpoint = "";

	UPROPERTY(config, EditAnywhere, Category = Settings)
	FString OragnizationGameEndpoint = "";

	UPROPERTY(config, EditAnywhere, Category = Settings)
	FString GameDataEndpoint = "";

	UPROPERTY(config, EditAnywhere, Category = Settings)
	FString ProfileDataEndpoint = "";
};