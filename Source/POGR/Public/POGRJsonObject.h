#pragma once

#include "CoreMinimal.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Policies/CondensedJsonPrintPolicy.h"
#include "Serialization/JsonWriter.h"
#include "POGRJsonObject.generated.h"


UCLASS(BlueprintType)
class UJsonRequestObject : public UObject
{
	GENERATED_BODY()
public:
	void ConstructJsonObject();

public:
	UFUNCTION(BlueprintCallable, Category = "POGR|Json")
	void SetStringField(const FString& FieldName, const FString& StringValue);

	UFUNCTION(BlueprintCallable, Category = "POGR|Json")
	void SetBoolField(const FString& FieldName, bool InValue);

	UFUNCTION(BlueprintCallable, Category = "POGR|Json")
	void SetNumberField(const FString& FieldName, double Number);

	UFUNCTION(BlueprintCallable, Category = "POGR|Json")
	void SetArrayField(const FString& FieldName, const TArray<FString>& Array);

	UFUNCTION(BlueprintCallable, Category = "POGR|Json")
	void SetObjectField(const FString& FieldName, const UJsonRequestObject* SubJsonObject);

public:
	const TSharedPtr<FJsonObject> GetJsonRequestObject() const { return JsonObject; }

private:
	TArray<TSharedPtr<FJsonValue>> StringArrayToJsonValues(const TArray<FString>& StringArray);

private:
	TSharedPtr<FJsonObject> JsonObject;
};