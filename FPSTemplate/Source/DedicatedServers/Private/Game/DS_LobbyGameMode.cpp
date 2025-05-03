// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/DS_LobbyGameMode.h"
#include "Game/DS_GameInstanceSubSystem.h"
#include "DedicatedServers/DedicatedServers.h"
#include "Kismet/GameplayStatics.h"
#include "Player/DSPlayerController.h"
#include "Game/DS_GameState.h"
#include "Lobby/LobbyState.h"

ADS_LobbyGameMode::ADS_LobbyGameMode()
{
	bUseSeamlessTravel = true;
	LobbyStatus = ELobbyStatus::WaitingForPlayers;
	MinPlayer = 1;
	LobbyCountdownTimerHandle.Type = ECountdownTimerType::LobbyCountdown;
}

void ADS_LobbyGameMode::PostLogin(APlayerController* NewPlayer)
{

	Super::PostLogin(NewPlayer);
	CheckAndStartLobbyCountdown();
}

void ADS_LobbyGameMode::InitSeamlessTravelPlayer(AController* NewPlayer)
{
	//Seamless는 PostLogin을 호출하지 않기 때문에 이함수를 Override해서 사용한다.

	Super::InitSeamlessTravelPlayer(NewPlayer);
	CheckAndStartLobbyCountdown();

	if (LobbyStatus != ELobbyStatus::SeamlessTraveling)
	{
		AddPlayerInfoToLobbyStaet(NewPlayer);
	}
}

void ADS_LobbyGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
	CheckAndStopLobbyCountdown();

	RemovePlayerSession(Exiting);

	if (LobbyStatus != ELobbyStatus::SeamlessTraveling)
	{
		RemovePlayerInfoFormLobbyState(Exiting);
	}
}

FString ADS_LobbyGameMode::InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId, const FString& Options, const FString& Portal)
{
	FString InitializedString = Super::InitNewPlayer(NewPlayerController, UniqueId, Options, Portal);

	const FString PlayerSessionId = UGameplayStatics::ParseOption(Options, TEXT("PlayerSessionId"));
	const FString Username = UGameplayStatics::ParseOption(Options, TEXT("Username"));

	if (ADSPlayerController* DSPlayerController = Cast<ADSPlayerController>(NewPlayerController); IsValid(DSPlayerController))
	{
		DSPlayerController->PlayerSessionId = PlayerSessionId;
		DSPlayerController->Username = Username;
	}

	if (LobbyStatus != ELobbyStatus::SeamlessTraveling)
	{
		AddPlayerInfoToLobbyStaet(NewPlayerController);
	}

	return InitializedString;
}

void ADS_LobbyGameMode::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	Super::PreLogin(Options, Address, UniqueId, ErrorMessage);

	const FString PlayerSessionId = UGameplayStatics::ParseOption(Options, TEXT("PlayerSessionId"));
	const FString Username = UGameplayStatics::ParseOption(Options, TEXT("Username"));

	TryAcceptPlayerSession(PlayerSessionId, Username, ErrorMessage);

}

void ADS_LobbyGameMode::TryAcceptPlayerSession(const FString& PlayerSessionId, const FString& Username, FString& OutErrorMessage)
{
	if (PlayerSessionId.IsEmpty() || Username.IsEmpty())
	{
		OutErrorMessage = TEXT("PlayerSessionId and/or Username is empty");
		return;
	}
	//
#if WITH_GAMELIFT
	Aws::GameLift::Server::Model::DescribePlayerSessionsRequest DescribePlayerSessionsRequest;
	DescribePlayerSessionsRequest.SetPlayerSessionId(TCHAR_TO_ANSI(*PlayerSessionId));

	const auto& DescribePlayerSessionsOutcome = Aws::GameLift::Server::DescribePlayerSessions(DescribePlayerSessionsRequest);
	if (!DescribePlayerSessionsOutcome.IsSuccess())
	{
		OutErrorMessage = TEXT("DescribePlayerSessions Failed.");
		return;
	}

	const auto& DescribePlayerSessionsResult = DescribePlayerSessionsOutcome.GetResult();
	int32 Count = 0;
	const Aws::GameLift::Server::Model::PlayerSession* PlayerSessions = DescribePlayerSessionsResult.GetPlayerSessions(Count);
	if (PlayerSessions == nullptr || Count == 0)
	{
		OutErrorMessage = TEXT("GetPlayerSessions failed.");
		return;
	}


	for (int32 i = 0; i < Count; i++)
	{
		const Aws::GameLift::Server::Model::PlayerSession& PlayerSession = PlayerSessions[i];
		if (!Username.Equals(PlayerSession.GetPlayerId())) continue;
		if (PlayerSession.GetStatus() != Aws::GameLift::Server::Model::PlayerSessionStatus::RESERVED)
		{
			OutErrorMessage = FString::Printf(TEXT("Session for %s not RESERVED; Fail PreLogin."), *Username);
			return;
		}

		const auto& AcceptPlayerSessionOutcome = Aws::GameLift::Server::AcceptPlayerSession(TCHAR_TO_ANSI(*PlayerSessionId));
		OutErrorMessage = AcceptPlayerSessionOutcome.IsSuccess() ? "" : FString::Printf(TEXT("Failed to accept player session for %s"), *Username);

	}
	//
#endif
}

void ADS_LobbyGameMode::AddPlayerInfoToLobbyStaet(AController* Player) const
{
	ADSPlayerController* DSPlayerController = Cast<ADSPlayerController>(Player);
	ADS_GameState* DSGameState = GetGameState<ADS_GameState>();

	if (IsValid(DSGameState) && IsValid(DSGameState->LobbyState) && IsValid(DSPlayerController))
	{
		FLobbyPlayerInfo PlayerInfo(DSPlayerController->Username);
		DSGameState->LobbyState->AddPlayerInfo(PlayerInfo);
	}
}

void ADS_LobbyGameMode::RemovePlayerInfoFormLobbyState(AController* Player) const
{
	ADSPlayerController* DSPlayerController = Cast<ADSPlayerController>(Player);
	ADS_GameState* DSGameState = GetGameState<ADS_GameState>();

	if (IsValid(DSGameState) && IsValid(DSGameState->LobbyState) && IsValid(DSPlayerController))
	{
		DSGameState->LobbyState->RemovePlayerInfo(DSPlayerController->Username);
	}
}

void ADS_LobbyGameMode::CheckAndStartLobbyCountdown()
{

	if (GetNumPlayers() >= MinPlayer && LobbyStatus == ELobbyStatus::WaitingForPlayers)
	{
		LobbyStatus = ELobbyStatus::CountdownToSeamlessTravel;
		StartCountdownTimer(LobbyCountdownTimerHandle);
	}
}

void ADS_LobbyGameMode::CheckAndStopLobbyCountdown()
{
	if (GetNumPlayers() - 1 < MinPlayer && LobbyStatus == ELobbyStatus::CountdownToSeamlessTravel)
	{
		LobbyStatus = ELobbyStatus::WaitingForPlayers;
		StopCountdownTimer(LobbyCountdownTimerHandle);
	}
}

void ADS_LobbyGameMode::BeginPlay()
{
	Super::BeginPlay();

	InitGameLift();

}

void ADS_LobbyGameMode::OnCountdownTimerFinished(ECountdownTimerType Type)
{
	Super::OnCountdownTimerFinished(Type);
	if (Type == ECountdownTimerType::LobbyCountdown)
	{
		StopCountdownTimer(LobbyCountdownTimerHandle);
		LobbyStatus = ELobbyStatus::SeamlessTraveling;
		TrySeamlessTravel(DestinationMap);

	}
}

void ADS_LobbyGameMode::InitGameLift()
{
	if (UGameInstance* GameInstance = GetGameInstance(); IsValid(GameInstance))
	{
		if (DSGameInstanceSubSystem = GameInstance->GetSubsystem<UDS_GameInstanceSubSystem>())
		{
			FServerParameters ServerParameters;
			SetServerParameters(ServerParameters);
			DSGameInstanceSubSystem->InitGameLift(ServerParameters);

		}
	}
}

void ADS_LobbyGameMode::SetServerParameters(FServerParameters& OutServerParameters)
{
	//AuthToken returned from the "aws gamelift get-compute-auth-token" API. Note this will expire and require a new call to the API after 15 minutes.
	if (FParse::Value(FCommandLine::Get(), TEXT("-authtoken="), OutServerParameters.m_authToken))
	{
		UE_LOG(LogDedicatedServers, Log, TEXT("AUTH_TOKEN: %s"), *OutServerParameters.m_authToken)
	}

	//The Host/compute-name of the Amazon GameLift Servers Anywhere instance.
	if (FParse::Value(FCommandLine::Get(), TEXT("-hostid="), OutServerParameters.m_hostId))
	{
		UE_LOG(LogDedicatedServers, Log, TEXT("HOST_ID: %s"), *OutServerParameters.m_hostId)
	}

	//The Anywhere Fleet ID.
	if (FParse::Value(FCommandLine::Get(), TEXT("-fleetid="), OutServerParameters.m_fleetId))
	{
		UE_LOG(LogDedicatedServers, Log, TEXT("FLEET_ID: %s"), *OutServerParameters.m_fleetId)
	}

	//The WebSocket URL (GameLiftServiceSdkEndpoint).
	if (FParse::Value(FCommandLine::Get(), TEXT("-websocketurl="), OutServerParameters.m_webSocketUrl))
	{
		UE_LOG(LogDedicatedServers, Log, TEXT("WEBSOCKET_URL: %s"), *OutServerParameters.m_webSocketUrl)
	}

	//The PID of the running process
	OutServerParameters.m_processId = FString::Printf(TEXT("%d"), GetCurrentProcessId());
	UE_LOG(LogDedicatedServers, Log, TEXT("PID: %s"), *OutServerParameters.m_processId);
}

