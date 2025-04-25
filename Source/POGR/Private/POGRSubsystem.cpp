#include "POGRSubsystem.h"
#include "POGR.h"
#include "IWebSocket.h"
#include "WebSocketsModule.h"
#include "IWebSocketsManager.h"
#include "JsonObjectConverter.h"
#include "Misc/Guid.h"
#include "Http.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "POGRSettings.h"
#include "POGRJsonObject.h"
#include "Async/TaskGraphInterfaces.h"
#include "Helpers/POGRGameEvent.h"
#include "Helpers/POGRGameLogs.h"
#include "Helpers/POGRGameMetrics.h"
#include "Helpers/POGRGameMonitor.h"
#include "Helpers/POGRURLFilters.h"
#include "Helpers/POGROrganizationData.h"
#include "Helpers/POGRDataPayload.h"
#include "Helpers/POGRProfileData.h"
#include "Helpers/POGRGameData.h"
#include "Blueprint/AsyncTaskDownloadImage.h"

const FString POGR_CLIENT = FString("5XJ4WBU1Q52PV6RKKNI6EB7AF3RDU2U8HTW2YA0Z2YQIZCGIZ6NLUD52ERPV0IGAXG19WM9ZF3PVBUYGWTRCLIQG75J3P11WRPVOCGXCHZLEN86WL3DDL7GOHIEM7E8G");


const FString UPOGRSubsystem::GenerateUniqueUserAssociationId() const
{
    FGuid RandomGuid = FGuid::NewGuid();
    FString AssociationId = FString::Printf(TEXT("%s"), *RandomGuid.ToString());
    return AssociationId;
}

void UPOGRSubsystem::CreateSessionWithAssociationId(const FString& ClientId, const FString& BuildId, const FString& AssociationId)
{    
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    TSharedRef<FJsonObject> RequestObj = MakeShared<FJsonObject>();

    RequestObj->SetStringField("association_id", AssociationId);

    FString RequestBody;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
    FJsonSerializer::Serialize(RequestObj, Writer);


    Request->SetURL(POGRSettings->GetInitEndpoint());
    Request->SetVerb(TEXT("POST"));

    Request->SetHeader("Content-Type", "application/json");
    Request->SetHeader("POGR_CLIENT", ClientId);
    Request->SetHeader("POGR_BUILD", BuildId);
    
    Request->SetContentAsString(RequestBody);

    Request->OnProcessRequestComplete().BindLambda(
        [this](FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSuccess)
        {
            if (bSuccess && HttpResponse.IsValid() && HttpResponse->GetResponseCode() == EHttpResponseCodes::Type::Ok)
            {
                TSharedPtr<FJsonObject> ResponseObj;
                if (HttpResponse != NULL) {
                    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(HttpResponse->GetContentAsString());
                    FJsonSerializer::Deserialize(Reader, ResponseObj);
                }

                if (ResponseObj->HasField(TEXT("payload")))
                {
                    // Get the "payload" object
                    TSharedPtr<FJsonObject> PayloadObject = ResponseObj->GetObjectField(TEXT("payload"));

                    // Check if the "payload" object contains the "redirect_url" field
                    if (PayloadObject->HasField(TEXT("session_id")))
                    {
                        FString SessionId = PayloadObject->GetStringField(TEXT("session_id"));

                        if (!SessionId.IsEmpty())
                        {
                            SetSessionId(SessionId);
                            bIsSessionActive = true;
                            OnSessionCreationCallback.Broadcast();
                        }
                    }
                }
            }
            else
                UE_LOG(LogTemp, Error, TEXT("Failed to Create Session"));
        }
    );

    Request->ProcessRequest();
}

void UPOGRSubsystem::CreateSessionWithTokken(const FString& ClientId, const FString& BuildId, const FString& JwtToken)
{
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    TSharedRef<FJsonObject> RequestObj = MakeShared<FJsonObject>();

    FString RequestBody;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
    FJsonSerializer::Serialize(RequestObj, Writer);


    Request->SetURL(POGRSettings->GetInitEndpoint());
    Request->SetVerb(TEXT("POST"));

    Request->SetHeader("Content-Type", "application/json");
    Request->SetHeader("POGR_CLIENT", ClientId);
    Request->SetHeader("POGR_BUILD", BuildId);
    Request->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *JwtToken));

    Request->SetContentAsString(RequestBody);

    Request->OnProcessRequestComplete().BindLambda(
        [this](FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSuccess)
        {
            if (bSuccess && HttpResponse.IsValid() && HttpResponse->GetResponseCode() == EHttpResponseCodes::Type::Ok)
            {
                TSharedPtr<FJsonObject> ResponseObj;
                if (HttpResponse != NULL) {
                    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(HttpResponse->GetContentAsString());
                    FJsonSerializer::Deserialize(Reader, ResponseObj);
                }

                if (ResponseObj->HasField(TEXT("payload")))
                {
                    // Get the "payload" object
                    TSharedPtr<FJsonObject> PayloadObject = ResponseObj->GetObjectField(TEXT("payload"));

                    // Check if the "payload" object contains the "redirect_url" field
                    if (PayloadObject->HasField(TEXT("session_id")))
                    {
                        FString SessionId = PayloadObject->GetStringField(TEXT("session_id"));

                        if (!SessionId.IsEmpty())
                        {
                            SetSessionId(SessionId);
                            bIsSessionActive = true;
                            OnSessionCreationCallback.Broadcast();
                        }
                    }
                }
            }
            else
                UE_LOG(LogTemp, Error, TEXT("Failed to Create Session"));
        }
    );

    Request->ProcessRequest();
}

void UPOGRSubsystem::DestroySession(const FString& SessionId)
{
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();

    Request->SetURL(POGRSettings->GetShutdownEndpoint());
    Request->SetVerb(TEXT("POST"));

    Request->SetHeader("Content-Type", "application/json");
    Request->SetHeader("INTAKE_SESSION_ID", SessionId);

    Request->OnProcessRequestComplete().BindLambda(
        [this](FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSuccess)
        {
            if (bSuccess && HttpResponse.IsValid() && HttpResponse->GetResponseCode() == EHttpResponseCodes::Type::Ok)
            {
                bIsSessionActive = false;
            }
        }
    );

    Request->ProcessRequest();
}

void UPOGRSubsystem::SendGameMetricsEvent(const FGameMetrics& GameMertrics, const FString& SessionId)
{
    static FCriticalSection Mutex;

    FGraphEventRef Task = FFunctionGraphTask::CreateAndDispatchWhenReady([&]()
    {
       FScopeLock Lock(&Mutex);
       UJsonRequestObject* JsonObject = this->GetJsonRequestObject();
       JsonObject->SetStringField(FString("service"), GameMertrics.service);
       JsonObject->SetStringField(FString("environment"), GameMertrics.environment);

       UJsonRequestObject* MetricsData = this->GetJsonRequestObject();
       MetricsData->SetNumberField(FString("players_online"), GameMertrics.metrics.players_online);
       MetricsData->SetNumberField(FString("average_latency_ms"), GameMertrics.metrics.average_latency_ms);
       MetricsData->SetNumberField(FString("server_load_percentage"), GameMertrics.metrics.server_load_percentage);
       JsonObject->GetJsonRequestObject()->SetObjectField(FString("metrics"), MetricsData->GetJsonRequestObject());

       UJsonRequestObject* TagsData = this->GetJsonRequestObject();
       TagsData->SetStringField(FString("location"), GameMertrics.tags.location);
       TagsData->SetStringField(FString("game_mode"), GameMertrics.tags.game_mode);
       JsonObject->GetJsonRequestObject()->SetObjectField(FString("tags"), TagsData->GetJsonRequestObject());

       SendHttpRequest(POGRSettings->GetMetricsEndpoint(), JsonObject, SessionId);
    }, TStatId(), nullptr, ENamedThreads::AnyThread);
}

void UPOGRSubsystem::SendGameUserEvent(const FGameEvent& GameEvent, const FString& SessionId)
{
    static FCriticalSection Mutex;

    FGraphEventRef Task = FFunctionGraphTask::CreateAndDispatchWhenReady([&]()
    {
       FScopeLock Lock(&Mutex);
       UJsonRequestObject* JsonObject = this->GetJsonRequestObject();
       JsonObject->SetStringField("event", GameEvent.event);
       JsonObject->SetStringField("sub_event", GameEvent.sub_event);
       JsonObject->SetStringField("event_type", GameEvent.event_type);
       JsonObject->SetStringField("event_flag", GameEvent.event_flag);
       JsonObject->SetStringField("event_key", GameEvent.event_key);

       UJsonRequestObject* EventData = this->GetJsonRequestObject();
       EventData->SetStringField("player_id", GameEvent.event_data.player_id);
       EventData->SetStringField("achievement_name", GameEvent.event_data.achievement_name);
       JsonObject->GetJsonRequestObject()->SetObjectField("event_data", EventData->GetJsonRequestObject());

       SendHttpRequest(FString(POGRSettings->GetEventEndpoint()), JsonObject, SessionId);
    }, TStatId(), nullptr, ENamedThreads::AnyThread);
}

void UPOGRSubsystem::SendGameDataEvent(const UJsonRequestObject* JsonObject, const FString& SessionId)
{
    FGraphEventRef Task = FFunctionGraphTask::CreateAndDispatchWhenReady([&]()
    {
       SendHttpRequest(POGRSettings->GetDataEndpoint(), JsonObject, SessionId);
    }, TStatId(), nullptr, ENamedThreads::AnyThread);
}

void UPOGRSubsystem::SendGameLogsEvent(const FGameLog& GameLog, const FString& SessionId)
{
    static FCriticalSection Mutex;

    FGraphEventRef Task = FFunctionGraphTask::CreateAndDispatchWhenReady([&]()
    {
            FScopeLock Lock(&Mutex);
            UJsonRequestObject* JsonObject = this->GetJsonRequestObject();
            JsonObject->SetStringField(FString("service"), GameLog.service);
            JsonObject->SetStringField(FString("environment"), GameLog.environment);
            JsonObject->SetStringField(FString("severity"), GameLog.severity);
            JsonObject->SetStringField(FString("type"), GameLog.type);
            JsonObject->SetStringField(FString("log"), GameLog.log);

            UJsonRequestObject* DataObject = this->GetJsonRequestObject();
            DataObject->SetStringField(FString("user_id"), GameLog.data.user_id);
            DataObject->SetStringField(FString("timestamp"), GameLog.data.timestamp);
            DataObject->SetStringField(FString("ip_address"), GameLog.data.ip_address);
            JsonObject->GetJsonRequestObject()->SetObjectField(FString("Data"), DataObject->GetJsonRequestObject());

            UJsonRequestObject* TagsObject = this->GetJsonRequestObject();
            TagsObject->SetStringField(FString("system"), GameLog.tags.system);
            TagsObject->SetStringField(FString("action"), GameLog.tags.action);
            JsonObject->GetJsonRequestObject()->SetObjectField(FString("tags"), TagsObject->GetJsonRequestObject());

            SendHttpRequest(POGRSettings->GetLogsEndpoint(), JsonObject, SessionId);
    }, TStatId(), nullptr, ENamedThreads::AnyThread);
}

void UPOGRSubsystem::SendGamePerformanceEvent(const FGameSystemMetrics& GamePerformanceMonitor, const FString& SessionId)
{
    static FCriticalSection Mutex;

    FGraphEventRef Task = FFunctionGraphTask::CreateAndDispatchWhenReady([&]()
    {
       FScopeLock Lock(&Mutex);
       UJsonRequestObject* JsonObject = this->GetJsonRequestObject();
       JsonObject->SetNumberField(FString("cpu_usage"), GamePerformanceMonitor.cpu_usage);
       JsonObject->SetNumberField(FString("memory_usage"), GamePerformanceMonitor.memory_usage);
       JsonObject->SetArrayField(FString("dlls_loaded"), GamePerformanceMonitor.dlls_loaded);

       UJsonRequestObject* SettingsObject = this->GetJsonRequestObject();
       SettingsObject->SetStringField(FString("graphics_quality"), GamePerformanceMonitor.settings.graphics_quality);
       JsonObject->GetJsonRequestObject()->SetObjectField(FString("Data"), SettingsObject->GetJsonRequestObject());

       SendHttpRequest(POGRSettings->GetMonitorEndpoint(), JsonObject, SessionId);
    }, TStatId(), nullptr, ENamedThreads::AnyThread);
}

void UPOGRSubsystem::SetGameEventAttributes(
    const FString& Event, const FString& SubEvent,
    const FString& EventType, const FString& EventFlag,
    const FString& EventKey, const FString& PlayerId,
    const FString& AchivementName, FGameEvent& GameEvent)
{
    FGameEvent GameEventData;
    GameEventData.event = Event;
    GameEventData.sub_event = SubEvent;
    GameEventData.event_type = EventType;
    GameEventData.event_flag = EventFlag;
    GameEventData.event_key = EventKey;
    GameEventData.event_data.player_id = PlayerId;
    GameEventData.event_data.achievement_name = AchivementName;

    GameEvent = GameEventData;
}

void UPOGRSubsystem::SetGameLogsAttributes(
    const FString& Service, const FString& Environment,
    const FString& Severity, const FString& Type,
    const FString& Log, const FString& User_id,
    const FString& Timestamp, const FString& Ip_address,
    const FString& System, const FString& Action, FGameLog& GameLog)
{
    FGameLog GameLogData;
    GameLogData.service = Service;
    GameLogData.environment = Environment;
    GameLogData.severity = Severity;
    GameLogData.type = Type;
    GameLogData.log = Log;
    GameLogData.data.user_id = User_id;
    GameLogData.data.timestamp = Timestamp;
    GameLogData.data.ip_address = Ip_address;
    GameLogData.tags.system = System;
    GameLogData.tags.action = Action;

    GameLog = GameLogData;
}

void UPOGRSubsystem::SetGameMetricsAttributes(
    const FString& Service, const FString& Environment,
    const int32& Players_online, const float& Average_latency_ms,
    const float& Server_load_percentage, const FString& Location,
    const FString& Game_mode, FGameMetrics& GameMetrics)
{
    FGameMetrics GameMetricsData;
    GameMetricsData.service = Service;
    GameMetricsData.environment = Environment;
    GameMetricsData.metrics.players_online = Players_online;
    GameMetricsData.metrics.average_latency_ms = Average_latency_ms;
    GameMetricsData.metrics.server_load_percentage = Server_load_percentage;
    GameMetricsData.tags.location = Location;
    GameMetricsData.tags.game_mode = Game_mode;

    GameMetrics = GameMetricsData;
}

void UPOGRSubsystem::SetGameMonitorAttributes(
    const float& CPU_usage, const float& Memory_usage,
    const TArray<FString>& Dlls_loaded, const FString& Graphics_quality,
    FGameSystemMetrics& GamePerformanceMonitor)
{
    FGameSystemMetrics GameSystemMetrics;
    GameSystemMetrics.cpu_usage = CPU_usage;
    GameSystemMetrics.memory_usage = Memory_usage;
    GameSystemMetrics.dlls_loaded = Dlls_loaded;
    GameSystemMetrics.settings.graphics_quality = Graphics_quality;

    GamePerformanceMonitor = GameSystemMetrics;
}

void UPOGRSubsystem::GetOrganizationData()
{
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();

    Request->SetURL(POGRSettings->GetOrgDataEndpoint());
    Request->SetVerb(TEXT("GET"));

    Request->SetHeader("Content-Type", "application/json");
    Request->SetHeader("POGR_CLIENT", POGR_CLIENT);
    Request->SetHeader("POGR_SESSION", GetAccessTokken());

    Request->OnProcessRequestComplete().BindLambda(
        [this](FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSuccess)
        {
            if (bSuccess && HttpResponse.IsValid() && HttpResponse->GetResponseCode() == EHttpResponseCodes::Type::Ok)
            {
                TSharedPtr<FJsonObject> ResponseObj;
                if (HttpResponse != NULL) {
                    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(HttpResponse->GetContentAsString());
                    FJsonSerializer::Deserialize(Reader, ResponseObj);
                }

                if (ResponseObj->HasField(TEXT("payload")))
                {
                    // Get the "payload" object
                    TSharedPtr<FJsonObject> PayloadObject = ResponseObj->GetObjectField(TEXT("payload"));

                    // Check if the "payload" object contains the "organizations" field
                    const TArray<TSharedPtr<FJsonValue>>* OrganizationsArray;
                    if (PayloadObject->TryGetArrayField(TEXT("organizations"), OrganizationsArray))
                    {
                        OrganizationData.Empty();

                        for (const auto& OrganizationValue : *OrganizationsArray)
                        {
                            const TSharedPtr<FJsonObject> OrganizationObject = OrganizationValue->AsObject();
                            if (OrganizationObject.IsValid())
                            {
                                FOrganizationData Organization;

                                Organization.UUID = OrganizationObject->GetStringField(TEXT("uid"));
                                Organization.Name = OrganizationObject->GetStringField(TEXT("name"));
                                Organization.CreatedOn = OrganizationObject->GetStringField(TEXT("created_on"));
                                Organization.Type = OrganizationObject->GetStringField(TEXT("type"));

                                const TSharedPtr<FJsonObject> LogoObject = OrganizationObject->GetObjectField(TEXT("logo"));
                                if (LogoObject.IsValid())
                                {
                                    Organization.URL = LogoObject->GetStringField(TEXT("url"));
                                }

                                OrganizationData.AddUnique(Organization);
                            }
                        }

                        if (OrganizationData.Num() > 0)
                        {
                            OrganizationDetails = OrganizationData[0];
                        }
                    }
                }
                OnOrganizationCallback.Broadcast();
            }
            else
                UE_LOG(LogTemp, Error, TEXT("Failed to Fetch Oragnization Data"));
        }
    );

    Request->ProcessRequest();
}

void UPOGRSubsystem::GetOrganizationGameData(const FString& GameUUID)
{
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();

    Request->SetURL(ConstructOrgGameURL(GameUUID));
    Request->SetVerb(TEXT("GET"));

    Request->SetHeader("Content-Type", "application/json");
    Request->SetHeader("POGR_CLIENT", POGR_CLIENT);
    Request->SetHeader("POGR_SESSION", GetAccessTokken());

    Request->OnProcessRequestComplete().BindLambda(
        [this](FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSuccess)
        {
            if (bSuccess && HttpResponse.IsValid() && HttpResponse->GetResponseCode() == EHttpResponseCodes::Type::Ok)
            {
                TSharedPtr<FJsonObject> ResponseObj;
                if (HttpResponse != NULL) {
                    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(HttpResponse->GetContentAsString());
                    FJsonSerializer::Deserialize(Reader, ResponseObj);
                }

                const TArray<TSharedPtr<FJsonValue>>* PayloadArray;
                if (ResponseObj->TryGetArrayField(TEXT("payload"), PayloadArray))
                {
                    if (GetUpdateOptions())
                    {
                        OrganizationGameData.Empty();

                        for (const auto& PayloadValue : *PayloadArray)
                        {
                            const TSharedPtr<FJsonObject> PayloadObject = PayloadValue->AsObject();
                            if (PayloadObject.IsValid())
                            {
                                FOrganizationGameData Organization;

                                Organization.UUID = PayloadObject->GetStringField(TEXT("uid"));
                                Organization.StudioUUID = PayloadObject->GetStringField(TEXT("studio_org_uid"));
                                Organization.GameTitle = PayloadObject->GetStringField(TEXT("name"));
                                Organization.CreatedOn = PayloadObject->GetStringField(TEXT("created_on"));

                                const TSharedPtr<FJsonObject> LogoObject = PayloadObject->GetObjectField(TEXT("logo"));
                                if (LogoObject.IsValid())
                                {
                                    Organization.URL = LogoObject->GetStringField(TEXT("url"));
                                }

                                const TSharedPtr<FJsonObject> GameBuildIdObject = PayloadObject->GetObjectField(TEXT("game_build"));
                                if (GameBuildIdObject.IsValid())
                                {
                                    SetGameBuildId(Organization.UUID);
                                }

                                OrganizationGameData.AddUnique(Organization);
                            }
                        }
                        Game = OrganizationGameData[0];
                        OnGameCallback.Broadcast();
                        bUpdateOptions = false;
                    }
                }
            }
            else
                UE_LOG(LogTemp, Error, TEXT("Failed to Fetch Oragnization Game Data"));
        }
    );

    Request->ProcessRequest();
}

void UPOGRSubsystem::GetUserProfileData()
{
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();

    Request->SetURL(POGRSettings->GetProfileDataEndpoint());
    Request->SetVerb(TEXT("GET"));

    Request->SetHeader("Content-Type", "application/json");
    Request->SetHeader("POGR_CLIENT", POGR_CLIENT);
    Request->SetHeader("POGR_SESSION", GetAccessTokken());

    Request->OnProcessRequestComplete().BindLambda(
        [this](FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSuccess)
        {
            if (bSuccess && HttpResponse.IsValid() && HttpResponse->GetResponseCode() == EHttpResponseCodes::Type::Ok)
            {
                TSharedPtr<FJsonObject> ResponseObj;
                if (HttpResponse != NULL) {
                    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(HttpResponse->GetContentAsString());
                    FJsonSerializer::Deserialize(Reader, ResponseObj);
                }

                if (ResponseObj->HasField(TEXT("payload")))
                {
                    // Get the "payload" object
                    TSharedPtr<FJsonObject> PayloadObject = ResponseObj->GetObjectField(TEXT("payload"));

                    UserProfileData.UserName = PayloadObject->GetStringField(TEXT("username"));
                    UserProfileData.DisplayName = PayloadObject->GetStringField(TEXT("display_name"));
                    UserProfileData.AvatarURL = PayloadObject->GetStringField(TEXT("avatar"));
                    UserProfileData.Level = FCString::Atoi(*PayloadObject->GetStringField(TEXT("level")));
                    UserProfileData.Exp = FCString::Atoi(*PayloadObject->GetStringField(TEXT("total_exp")));
                    UserProfileData.RequiredExp = FCString::Atoi(*PayloadObject->GetStringField(TEXT("exp_required")));
                }
            }
            else
                UE_LOG(LogTemp, Error, TEXT("Failed to Fetch User Profile Data"));
        }
    );

    Request->ProcessRequest();
}

void UPOGRSubsystem::ListDataPayloads()
{
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();

    Request->SetURL(GetPogrUrl(URLAction::List, GetPayloadDatatype(), EAcceptedStatus::Ignored, GetGameBuildId()));
    Request->SetVerb(TEXT("GET"));

    Request->SetHeader("Content-Type", "application/json");
    Request->SetHeader("POGR_CLIENT", POGR_CLIENT);
    Request->SetHeader("POGR_SESSION", GetAccessTokken());

    Request->OnProcessRequestComplete().BindLambda(
        [this](FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSuccess)
        {
            if (bSuccess && HttpResponse.IsValid() && HttpResponse->GetResponseCode() == EHttpResponseCodes::Type::Ok)
            {
                TSharedPtr<FJsonObject> ResponseObj;
                if (HttpResponse != NULL) {
                    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(HttpResponse->GetContentAsString());
                    FJsonSerializer::Deserialize(Reader, ResponseObj);
                }
                
                if (ResponseObj->HasField(TEXT("payload")))
                {
                    // Get the "payload" object
                    TSharedPtr<FJsonObject> PayloadObject = ResponseObj->GetObjectField(TEXT("payload"));

                    // Check if the "payload" object contains the "organizations" field
                    const TArray<TSharedPtr<FJsonValue>>* DataArray;
                    if (PayloadObject->TryGetArrayField(TEXT("data"), DataArray))
                    {
                        DataPayloads.Empty();

                        for (const auto& DataValue : *DataArray)
                        {
                            const TSharedPtr<FJsonObject> DataObject = DataValue->AsObject();
                            if (DataObject.IsValid())
                            {
                                FDataPayload DataPayload;

                                DataPayload.Id = DataObject->GetStringField(TEXT("id"));
                                DataPayload.ClientId = DataObject->GetStringField(TEXT("client_id"));
                                DataPayload.BuildId = DataObject->GetStringField(TEXT("build_id"));
                                DataPayload.AcceptedStatus = (EAcceptedStatus)FCString::Atoi(*DataObject->GetStringField(TEXT("accepted_status")));
                                DataPayload.DataReceivedDate = DataObject->GetStringField(TEXT("first_data_received_date"));
                                DataPayload.Calls = FCString::Atoi(*DataObject->GetStringField(TEXT("calls")));

                                DataPayloads.Add(DataPayload);
                            }
                        }
                        OnPayloadCreationCallback.Broadcast();
                    }
                    else {
                        DataPayloads.Empty();
                        OnPayloadCreationCallback.Broadcast();
                    }
                }
            }
            else
                UE_LOG(LogTemp, Error, TEXT("Failed to Payloads List Data"));
        }
    );

    Request->ProcessRequest();
}

void UPOGRSubsystem::UpdateDataStatus(EAcceptedStatus AcceptedStatus, FString DataId)
{
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();

    Request->SetURL(GetPogrUrl(URLAction::Update, GetPayloadDatatype(), AcceptedStatus, GetGameBuildId(), DataId));
    Request->SetVerb(TEXT("PUT"));

    Request->SetHeader("Content-Type", "application/json");
    Request->SetHeader("POGR_CLIENT", POGR_CLIENT);
    Request->SetHeader("POGR_SESSION", GetAccessTokken());

    Request->OnProcessRequestComplete().BindLambda(
        [this](FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSuccess)
        {
            if (bSuccess && HttpResponse.IsValid() && HttpResponse->GetResponseCode() == EHttpResponseCodes::Type::Ok)
            {
                OnPayloadDataUpdate.Broadcast();
                UE_LOG(LogTemp, Warning, TEXT("%s"), *HttpResponse->GetContentAsString());
            }
            else
                UE_LOG(LogTemp, Error, TEXT("Failed to Update Payload Data Status"));
        }
    );

    Request->ProcessRequest();
}

void UPOGRSubsystem::GetDataPayloadDefinition(FString DataId)
{
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();

    Request->SetURL(GetPogrUrl(URLAction::Definition, GetPayloadDatatype(), EAcceptedStatus::None, GetGameBuildId(), DataId));
    Request->SetVerb(TEXT("GET"));

    Request->SetHeader("Content-Type", "application/json");
    Request->SetHeader("POGR_CLIENT", POGR_CLIENT);
    Request->SetHeader("POGR_SESSION", GetAccessTokken());

    Request->OnProcessRequestComplete().BindLambda(
        [this](FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSuccess)
        {
            if (bSuccess && HttpResponse.IsValid() && HttpResponse->GetResponseCode() == EHttpResponseCodes::Type::Ok)
            {
                TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);

                TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(HttpResponse->GetContentAsString());
                if (FJsonSerializer::Deserialize(JsonReader, JsonObject))
                {
                    // Serialize the JSON object back to a formatted string
                    FString JsonString;
                    TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&JsonString);
                    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), JsonWriter);
                    SetContentAsString(JsonString);
                }
            }
            else
                UE_LOG(LogTemp, Error, TEXT("Failed to Payloads List Data"));
        }
    );

    Request->ProcessRequest();
}

void UPOGRSubsystem::GetDataReceived(FString DataId)
{
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();

    Request->SetURL(GetPogrUrl(URLAction::Receiving, GetPayloadDatatype(), EAcceptedStatus::None, GetGameBuildId(), DataId));
    Request->SetVerb(TEXT("GET"));

    Request->SetHeader("Content-Type", "application/json");
    Request->SetHeader("POGR_CLIENT", POGR_CLIENT);
    Request->SetHeader("POGR_SESSION", GetAccessTokken());

    Request->OnProcessRequestComplete().BindLambda(
        [this](FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSuccess)
        {
            if (bSuccess && HttpResponse.IsValid() && HttpResponse->GetResponseCode() == EHttpResponseCodes::Type::Ok)
            {
                TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);

                TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(HttpResponse->GetContentAsString());
                if (FJsonSerializer::Deserialize(JsonReader, JsonObject))
                {
                    // Serialize the JSON object back to a formatted string
                    FString JsonString;
                    TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&JsonString);
                    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), JsonWriter);
                    SetContentAsString(JsonString);
                }
            }
            else
                UE_LOG(LogTemp, Error, TEXT("Failed to Payloads List Data"));
        }
    );

    Request->ProcessRequest();
}

FString UPOGRSubsystem::GetPogrUrl(URLAction Action, URLDefinition Definition, EAcceptedStatus Status, FString BuildId, FString DataId)
{
    FString URL = FString();
    FString BaseURL = POGRSettings->GetPogrUrlDefinationEndpoint();

    switch (Action)
    {
    case URLAction::List:
        switch (Definition)
        {
            case URLDefinition::Data:
                URL = BaseURL + "list-data-payloads?build_id=" + BuildId;
                break;
            case URLDefinition::Events:
                URL = BaseURL + "list-events-payloads?build_id=" + BuildId;
                break;
            case URLDefinition::Metrics:
                URL = BaseURL + "list-metrics-payloads?build_id=" + BuildId;
                break;
            case URLDefinition::Logs:
                URL = BaseURL + "list-logs-payloads?build_id=" + BuildId;
                break;
        }
        break;
    case URLAction::Update:
        switch (Definition)
        {
            case URLDefinition::Data:
                switch (Status)
                {
                    case EAcceptedStatus::Pending:
                        URL = BaseURL + "update-status/data/" + DataId + "?status=Pending&build_id=" + BuildId;
                        break;
                    case EAcceptedStatus::Saved:
                        URL = BaseURL + "update-status/data/" + DataId + "?status=Save&build_id=" + BuildId;
                        break;
                    case EAcceptedStatus::Ignored:
                        URL = BaseURL + "update-status/data/" + DataId + "?status=Ignore&build_id=" + BuildId;
                        break;
                }
                break;
            case URLDefinition::Events:
                switch (Status)
                {
                    case EAcceptedStatus::Pending:
                        URL = BaseURL + "update-status/events/" + DataId + "?status=Pending&build_id=" + BuildId;
                        break;
                    case EAcceptedStatus::Saved:
                        URL = BaseURL + "update-status/events/" + DataId + "?status=Save&build_id=" + BuildId;
                        break;
                    case EAcceptedStatus::Ignored:
                        URL = BaseURL + "update-status/events/" + DataId + "?status=Ignore&build_id=" + BuildId;
                        break;
                }
                break;
            case URLDefinition::Metrics:
                switch (Status)
                {
                    case EAcceptedStatus::Pending:
                        URL = BaseURL + "update-status/metrics/" + DataId + "?status=Pending&build_id=" + BuildId;
                        break;
                    case EAcceptedStatus::Saved:
                        URL = BaseURL + "update-status/metrics/" + DataId + "?status=Save&build_id=" + BuildId;
                        break;
                    case EAcceptedStatus::Ignored:
                        URL = BaseURL + "update-status/metrics/" + DataId + "?status=Ignore&build_id=" + BuildId;
                        break;
                }
                break;
            case URLDefinition::Logs:
                switch (Status)
                {
                    case EAcceptedStatus::Pending:
                        URL = BaseURL + "update-status/logs/" + DataId + "?status=Pending&build_id=" + BuildId;
                        break;
                    case EAcceptedStatus::Saved:
                        URL = BaseURL + "update-status/logs/" + DataId + "?status=Save&build_id=" + BuildId;
                        break;
                    case EAcceptedStatus::Ignored:
                        URL = BaseURL + "update-status/logs/" + DataId + "?status=Ignore&build_id=" + BuildId;
                        break;
                }
                break;
        }
        break;
    case URLAction::Definition:
        switch (Definition)
        {
            case URLDefinition::Data:
                URL = BaseURL + "get-data-payload-definition/" + DataId + "?build_id=" + BuildId;
                break;
            case URLDefinition::Events:
                URL = BaseURL + "get-events-payload-definition/" + DataId + "?build_id=" + BuildId;
                break;
            case URLDefinition::Metrics:
                URL = BaseURL + "get-metrics-payload-definition/" + DataId + "?build_id=" + BuildId;
                break;
            case URLDefinition::Logs:
                URL = BaseURL + "get-logs-payload-definition/" + DataId + "?build_id=" + BuildId;
                break;
        }
        break;
    case URLAction::Receiving:
        switch (Definition)
        {
            case URLDefinition::Data:
                URL = BaseURL + "get-data-received/" + DataId + "?build_id=" + BuildId;
                break;
            case URLDefinition::Events:
                URL = BaseURL + "get-events-received/" + DataId + "?build_id=" + BuildId;
                break;
            case URLDefinition::Metrics:
                URL = BaseURL + "get-metrics-received/" + DataId + "?build_id=" + BuildId;
                break;
            case URLDefinition::Logs:
                URL = BaseURL + "get-logs-received/" + DataId + "?build_id=" + BuildId;
                break;
        }
        break;
    }

    return URL;
}

UJsonRequestObject* UPOGRSubsystem::GetJsonRequestObject()
{
    jsonObject = NewObject<UJsonRequestObject>();
    jsonObject->ConstructJsonObject();
    return jsonObject;
}

void UPOGRSubsystem::SetGameTitleTexture(UTexture2DDynamic* Texture)
{
    GameTitleTexture = Texture;
}

void UPOGRSubsystem::SetOrganizationTitleTexture(UTexture2DDynamic* Texture)
{
    OrganizationTitleTexture = Texture;
}

void UPOGRSubsystem::SetUserProfileTexture(UTexture2DDynamic* Texture)
{
    UserProfileData.ProfileImage = Texture;
}

void UPOGRSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    POGRSettings = GetMutableDefault<UPOGREndpointSettings>();
}

void UPOGRSubsystem::Deinitialize()
{
    if (IsSessionActive())
    {
        DestroySession(GetSessionId());
    }
}

void UPOGRSubsystem::SendHttpRequest(const FString& URL, const UJsonRequestObject* JsonObject, const FString& SessionId)
{
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    TSharedRef<FJsonObject> RequestObj = MakeShared<FJsonObject>();

    FString RequestBody;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
    FJsonSerializer::Serialize(JsonObject->GetJsonRequestObject().ToSharedRef(), Writer);

    Request->SetURL(URL);
    Request->SetVerb(TEXT("POST"));

    Request->SetHeader("Content-Type", "application/json");
    Request->SetHeader("INTAKE_SESSION_ID", SessionId);
    Request->SetContentAsString(RequestBody);
    Request->OnProcessRequestComplete().BindUObject(this, &UPOGRSubsystem::OnResponseReceived);

    Request->ProcessRequest();
}

void UPOGRSubsystem::SetSessionId(FString SessionId)
{
    ActiveSessionId = SessionId;
}

void UPOGRSubsystem::SetAccessTokken(FString Payload)
{
    AccessTokken = *Payload;
}

void UPOGRSubsystem::SetIsUserLoggedIn(bool LoginStatus)
{
    IsLoggedIn = LoginStatus;
}

void UPOGRSubsystem::SetGameBuildId(FString GameUUID)
{
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();

    Request->SetURL(ConstructGameDetailURL(GameUUID));
    Request->SetVerb(TEXT("GET"));

    Request->SetHeader("Content-Type", "application/json");
    Request->SetHeader("POGR_CLIENT", POGR_CLIENT);
    Request->SetHeader("POGR_SESSION", GetAccessTokken());

    Request->OnProcessRequestComplete().BindLambda(
        [this](FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSuccess)
        {
            if (bSuccess && HttpResponse.IsValid() && HttpResponse->GetResponseCode() == EHttpResponseCodes::Type::Ok)
            {
                TSharedPtr<FJsonObject> ResponseObj;
                if (HttpResponse != NULL) {
                    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(HttpResponse->GetContentAsString());
                    FJsonSerializer::Deserialize(Reader, ResponseObj);
                }

                const auto PayloadObject = ResponseObj->GetObjectField(TEXT("payload"));
                if (PayloadObject.IsValid())
                {
                    const TSharedPtr<FJsonObject> GameBuildIdObject = PayloadObject->GetObjectField(TEXT("game_build"));
                    if (GameBuildIdObject.IsValid())
                    {
                        GameBuildId = GameBuildIdObject->GetStringField(TEXT("build_id"));
                    }
                }
            }
        }
    );

    Request->ProcessRequest();
}

void UPOGRSubsystem::OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully)
{
    if (!bConnectedSuccessfully) {
        UE_LOG(LogTemp, Error, TEXT("(Null Response) bConnectedSuccessfully: %i"), bConnectedSuccessfully);
        return;
    }

    TSharedPtr<FJsonObject> ResponseObj;
    if (Response != NULL) {
        TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
        FJsonSerializer::Deserialize(Reader, ResponseObj);
    }
    else {
        UE_LOG(LogTemp, Error, TEXT("(Null Response)"));
    }

    // Logging Data in Output Log Window Received from Backend.
    if (Response != NULL) {
        UE_LOG(LogTemp, Display, TEXT("Data-ContentAsString: %s"), *Response->GetContentAsString());
        UE_LOG(LogTemp, Display, TEXT("Data-URL: %s"), *Response->GetURL());
    }
}

void UPOGRSubsystem::Login()
{
    FWebSocketsModule& WebSocketModule = FModuleManager::LoadModuleChecked<FWebSocketsModule>("WebSockets");
    WebSocket = WebSocketModule.CreateWebSocket(FString("wss://developer-websocket-stage.pogr.io/ws"));

    if (WebSocket.IsValid())
    {
        /* *Binding Delagetes for the Callbacks */
        WebSocket->OnConnected().AddUObject(this, &UPOGRSubsystem::OnWebSocketConnected);
        WebSocket->OnConnectionError().AddUObject(this, &UPOGRSubsystem::OnWebSocketError);
        WebSocket->OnClosed().AddUObject(this, &UPOGRSubsystem::OnWebSocketClosed);
        WebSocket->OnMessage().AddUObject(this, &UPOGRSubsystem::OnWebSocketMessage);

        WebSocket->Connect(); // Connecting with the Server
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create WebSocket object for URL"));
    }
}

void UPOGRSubsystem::LogOut()
{
    SetAccessTokken(FString());
    SetIsUserLoggedIn(false);
    UserProfileData = FUserProfileData();
    OrganizationData.Empty();
    OrganizationGameData.Empty();
}

void UPOGRSubsystem::OnWebSocketConnected()
{
    UE_LOG(LogTemp, Log, TEXT("WebSocket connected successfully!"));
}

void UPOGRSubsystem::OnWebSocketError(const FString& Error)
{
    UE_LOG(LogTemp, Error, TEXT("WebSocket error: %s"), *Error);
}

void UPOGRSubsystem::OnWebSocketClosed(int32 StatusCode, const FString& Reason, bool bWasClean)
{
    UE_LOG(LogTemp, Log, TEXT("WebSocket closed with status code: %d, reason: %s"), StatusCode, *Reason);
}

void UPOGRSubsystem::OnWebSocketMessage(const FString& Message)
{
    // Parse the JSON string into a JSON object
    TSharedPtr<FJsonObject> JsonDataObject;
    TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(Message);

    if (FJsonSerializer::Deserialize(JsonReader, JsonDataObject))
    {
        // Check if the JSON object contains the "payload" field
        if (JsonDataObject->HasField(TEXT("payload")))
        {
            // Get the "payload" object
            TSharedPtr<FJsonObject> PayloadObject = JsonDataObject->GetObjectField(TEXT("payload"));

            // Check if the "payload" object contains the "redirect_url" field
            if (PayloadObject->HasField(TEXT("redirect_url")))
            {
                // Get the value of the "redirect_url" field
                FString RedirectUrl = PayloadObject->GetStringField(TEXT("redirect_url"));

                if (!RedirectUrl.IsEmpty())
                    FPlatformProcess::LaunchURL(*RedirectUrl, nullptr, nullptr);
            }
            
            // Check if the "payload" object contains the "access_token" field
            if (PayloadObject->HasField(TEXT("access_token")))
            {
                // Get the value of the "access_token" field
                AccessTokken = PayloadObject->GetStringField(TEXT("access_token"));

                if (!AccessTokken.IsEmpty())
                {
                    SetAccessTokken(AccessTokken);
                    SetIsUserLoggedIn(true);
                    OnLoginComplete.Broadcast(GetIsUserLoggedIn());
                    GetUserProfileData();
                    GetOrganizationData();
                }

                WebSocket->Close();
            }
        }
        else
            UE_LOG(LogTemp, Warning, TEXT("JSON object does not contain the 'payload' field"));
    }
    else
        UE_LOG(LogTemp, Error, TEXT("Failed to deserialize JSON string"));
}

void UPOGRSubsystem::SetContentAsString(FString Content)
{
    ContentAsString = Content;
    OnPayloadCallback.Broadcast(ContentAsString);
}

void UPOGRSubsystem::SetPayloadDatatype(URLDefinition PayloadDatatype)
{
    PayloadDefinition = PayloadDatatype;
    OnPayloadDataUpdate.Broadcast();
}

const FString UPOGRSubsystem::GetSelectedOptionValue() const
{
    switch (GetPayloadDatatype()) {
        case URLDefinition::Data: return FString("Data"); break;
        case URLDefinition::Events: return FString("Event"); break;
        case URLDefinition::Metrics: return FString("Metric"); break;
        case URLDefinition::Logs: return FString("Log"); break;
    }
    return FString("None Selected");
}

void UPOGRSubsystem::SetOrganizationOption(FString OrganizationValue)
{
    FString OrganizationName = OrganizationValue;

    for (const auto& OrgElem : GetOrganizationDataArray())
    {
        if (OrganizationName == OrgElem.Name)
        {
            OrganizationDetails = OrgElem;
            bUpdateOptions = true;
            GetOrganizationGameData(OrganizationDetails.UUID);
        }
    }
}

FString UPOGRSubsystem::ConstructOrgGameURL(const FString& Id)
{
    FString BaseURL = POGRSettings->GetOrgGameEndpoint();
    FString URL = BaseURL + Id + "/plugins/games";
    return URL;
}

FString UPOGRSubsystem::ConstructGameDetailURL(const FString& Id)
{
    FString BaseURL = POGRSettings->GetOrgGameDataEndpoint();
    FString URL = BaseURL + Id;
    return URL;
}

void UPOGRSubsystem::SetGameOption(FString GameValue)
{
    FString GameTitle = GameValue;

    for (const auto& GameElem : GetOrganizationGameDataArray())
    {
        if (GameTitle == GameElem.GameTitle)
        {
            Game = GameElem;
            SetGameBuildId(Game.UUID);
        }
    }
}