

#include "UI/GameStats/GameStatsManager.h"
#include "JsonObjectConverter.h"
#include "UI/HTTP/HTTPRequestTypes.h"
#include "Interfaces/IHttpRequest.h"
#include "HttpModule.h"
#include "Data/API/APIData.h"
#include "GameplayTags/DedicatedServersTag.h"

void UGameStatsManager::RecordMatchStats(const FDSRecordMatchStatsInput& RecordMatchStatsInput)
{
	TSharedPtr<FJsonObject> JsonObject = MakeShared<FJsonObject>();
	FJsonObjectConverter::UStructToJsonObject(FDSRecordMatchStatsInput::StaticStruct(), &RecordMatchStatsInput, JsonObject.ToSharedRef());

	FString JsonString;
	FJsonObjectConverter::UStructToJsonObjectString(FDSRecordMatchStatsInput::StaticStruct(), &RecordMatchStatsInput, JsonString);

	GEngine->AddOnScreenDebugMessage(-1, 600.f, FColor::Red, JsonString);

	TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();

	const FString APIUrl = APIData->GetAPIEndPoint(DedicatedServersTag::GameStatsAPI::RecordMatchStats);

	Request->SetURL(APIUrl);
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetContentAsString(JsonString);

	Request->ProcessRequest();
}
