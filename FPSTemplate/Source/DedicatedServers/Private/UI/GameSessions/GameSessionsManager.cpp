// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/GameSessions/GameSessionsManager.h"
#include "HttpModule.h"
#include "DedicatedServers/DedicatedServers.h"
#include "Data/API/APIData.h"
#include "UI/HTTP/HTTPRequestTypes.h"
#include "Interfaces/IHttpResponse.h"
#include "JsonObjectConverter.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"

void UGameSessionsManager::JoinGameSession()
{
	BroadcastJoinGameSessionMessage.Broadcast(TEXT("Searching for Game Sessions..."), false);

	check(APIData);

	TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &UGameSessionsManager::FindOrCrateGameSession_Response);

	const FString APIUrl = APIData->GetAPIEndPoint(DedicatedServersTag::GameSessionsAPI::FindOrCreateGameSession);

	Request->SetURL(APIUrl);
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->ProcessRequest();
}

void UGameSessionsManager::FindOrCrateGameSession_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (!bWasSuccessful)
	{
		BroadcastJoinGameSessionMessage.Broadcast(HTTPStatusMessage::SomethingWentWrong, true);
	}

	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
	if (FJsonSerializer::Deserialize(JsonReader, JsonObject))
	{
		if (ContainsErrors(JsonObject))
		{
			BroadcastJoinGameSessionMessage.Broadcast(HTTPStatusMessage::SomethingWentWrong, true);
		}

		FDSGameSession GameSession;
		FJsonObjectConverter::JsonObjectToUStruct(JsonObject.ToSharedRef(), &GameSession);

		const FString GameSessionId = GameSession.GameSessionId;
		const FString GameSessionStatus = GameSession.Status;
		HandleGameSessionStatus(GameSessionStatus, GameSessionId);

	}
}

FString UGameSessionsManager::GetUniquePlayerId() const
{
	APlayerController* LocalPlayerController = GEngine->GetFirstLocalPlayerController(GetWorld());
	if (IsValid(LocalPlayerController))
	{
		APlayerState* LocalPlayerState = LocalPlayerController->GetPlayerState<APlayerState>();
		if (IsValid(LocalPlayerState) && LocalPlayerState->GetUniqueId().IsValid())
		{
			return TEXT("Player_") + FString::FromInt(LocalPlayerState->GetUniqueID());
		}
	}
	return FString();
}

void UGameSessionsManager::CreatePlayerSession_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (!bWasSuccessful)
	{
		BroadcastJoinGameSessionMessage.Broadcast(HTTPStatusMessage::SomethingWentWrong, true);
	}

	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
	if (FJsonSerializer::Deserialize(JsonReader, JsonObject))
	{
		if (ContainsErrors(JsonObject))
		{
			BroadcastJoinGameSessionMessage.Broadcast(HTTPStatusMessage::SomethingWentWrong, true);
		}

		FDSPlayerSession PlayerSesssion;
		FJsonObjectConverter::JsonObjectToUStruct(JsonObject.ToSharedRef(), &PlayerSesssion);
		PlayerSesssion.Dump();

		APlayerController* LocalPlayerController = GEngine->GetFirstLocalPlayerController(GetWorld());
		if (IsValid(LocalPlayerController))
		{
			FInputModeGameOnly InputMode;
			LocalPlayerController->SetInputMode(InputMode);
			LocalPlayerController->SetShowMouseCursor(false);
		}

		const FString IPAndPort = PlayerSesssion.IpAddress + TEXT(":") + FString::FromInt(PlayerSesssion.Port);
		const FName Address(*IPAndPort);
		UGameplayStatics::OpenLevel(this, Address);
	}
}



void UGameSessionsManager::HandleGameSessionStatus(const FString& Status, const FString& SessionId)
{
	if (Status.Equals(TEXT("ACTIVE")))
	{
		BroadcastJoinGameSessionMessage.Broadcast(TEXT("Found active Game Session. Creating a Player Session..."), false);
		TryCreatePlayerSession(GetUniquePlayerId(), SessionId);
	}
	else if (Status.Equals(TEXT("ACTIVATING"))) // ACTIVATING 상태이면 잠시후 Join함수를 다시 실행하도록 만들기
	{
		FTimerDelegate CreateTimerDelegate;
		CreateTimerDelegate.BindUObject(this, &UGameSessionsManager::JoinGameSession);
		APlayerController* LocalPlayerController = GEngine->GetFirstLocalPlayerController(GetWorld());
		if (IsValid(LocalPlayerController))
		{
			LocalPlayerController->GetWorldTimerManager().SetTimer(CreateSessionTimer, CreateTimerDelegate, 0.5f, false);
		}
	}
	else
	{
		BroadcastJoinGameSessionMessage.Broadcast(HTTPStatusMessage::SomethingWentWrong, true);
	}

}

void UGameSessionsManager::TryCreatePlayerSession(const FString& PlayerId, const FString& GameSessionId)
{
	check(APIData);

	TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &UGameSessionsManager::CreatePlayerSession_Response);

	const FString APIUrl = APIData->GetAPIEndPoint(DedicatedServersTag::GameSessionsAPI::CreatePlayerSession);

	Request->SetURL(APIUrl);
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

	TMap<FString, FString> ContentParams = {
		{TEXT("playerId"),PlayerId},
		{TEXT("gameSessionId"),GameSessionId}
	};

	const FString Content = SerializeJsonContent(ContentParams);
	Request->SetContentAsString(Content);
	Request->ProcessRequest();
}
