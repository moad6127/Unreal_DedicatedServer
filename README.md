# AWS를 활용해 DedicatedServer 를 사용하기

AWS와 Unreal DedicatedServer를 같이 사용해서 게임을 작동시키는 방법에 대해서 공부 해보기.
AWS의 GameLift플러그인을 UE5에 추가시켜 AWS의 GameLift기능들을 UE5에서 사용할수 있도록 만들고,
AWS의 Cognito기능과 DynamoDB기능을 사용해 사용자 풀을 만들고 사용자의 데이터베이스를 만들어 AWS에서 관리할수 있도록 만들었다.

AWS와 UE5를 연결하는 기능과 게임의 기능을 분리해서 다른 프로젝트에 쉽게 AWS기능을 사용할수 있도록 만들어져 있다.

![ScreenShot00001](https://github.com/user-attachments/assets/2d7fb616-ef0f-4187-98cf-1df779ada75a)


<details><summary> 구분</summary>
<p>  
  
 * [GameLift](#GameLift)
   
 * [Session](#Session)

 * [Cognito](#Cognito)

 * [DynamoDB](#DynamoDB)

 * [Carrer](#Carrer)

 * [Leaderboard](#Leaderboard)
</p>
</details>
<br/> <br>

# AWS 

## GameLift

Unreal엔진에서 AWS의 기능을 사용해 멀티플레이를 진행하기 위해서는 GameLift플러그인을 엔진 내부에 추가한후, GameMode에서 필요한 작업들을 해야한다.        
AWS에서는 Anywhere플릿과 EC2플릿이 2가지의 플릿이 존재하며 Anywhere플릿은 자체 인프라에서 서버를 구동할때 사용하는 플릿이고 EC2플릿은 AWS의 하드웨어들을 사용해 서버를 구동할때 사용되는 플릿이다.
테스트 환경에서는 Anywhere플릿을 사용할수 있도록 엔진에서 필요한 코드들을 작성할 필요가 있다.


### Anywhere플릿 구동하기

먼저 AWS에서 Anywhere플릿을 생성한후 진행한다.      

이후 엔진코드에서 필요한 함수와 코드들을 작성한후 패키징을 진행하도록 만든다.    

<details><summary> InitGameLift</summary>
  
<p>
  
``` cpp
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

void UDS_GameInstanceSubSystem::InitGameLift(const FServerParameters& ServerParams)
{
	if (bGameLiftInitialized)
	{
		return;
	}

	bGameLiftInitialized = true;

#if WITH_GAMELIFT
	UE_LOG(LogDedicatedServers, Log, TEXT("Initializing the GameLift Server"));

	FGameLiftServerSDKModule* GameLiftSdkModule = &FModuleManager::LoadModuleChecked<FGameLiftServerSDKModule>(FName("GameLiftServerSDK"));
	GameLiftSdkModule->InitSDK(ServerParams);
	auto OnGameSession = [=](Aws::GameLift::Server::Model::GameSession gameSession)
		{
			FString GameSessionId = FString(gameSession.GetGameSessionId());
			UE_LOG(LogDedicatedServers, Log, TEXT("GameSession Initializing: %s"), *GameSessionId);
			GameLiftSdkModule->ActivateGameSession();
		};

	ProcessParameters.OnStartGameSession.BindLambda(OnGameSession);

	auto OnProcessTerminate = [=]()
		{
			UE_LOG(LogDedicatedServers, Log, TEXT("Game Server process is terminating"));
			GameLiftSdkModule->ProcessEnding();
		};

	ProcessParameters.OnTerminate.BindLambda(OnProcessTerminate);

	auto OnHealthCheckLamda = []()
		{
			UE_LOG(LogDedicatedServers, Log, TEXT("Performing Health Check"));
			return true;
		};

	ProcessParameters.OnHealthCheck.BindLambda(OnHealthCheckLamda);

	int32 Port = FURL::UrlConfig.DefaultPort;
	ParseCommandLinePort(Port);

	ProcessParameters.port = Port;
	TArray<FString> LogFiles;
	LogFiles.Add(TEXT("FPSTemplate/Saved/Logs/FPSTemplate.log"));
	ProcessParameters.logParameters = LogFiles;

	UE_LOG(LogDedicatedServers, Log, TEXT("Calling Process Ready"));
	GameLiftSdkModule->ProcessReady(ProcessParameters);

#endif


```

</p>

</details>

패키징이 진행된후 CMD명령어를 통해 AWS Anywhere플릿에서 필요한 정보들을 입력후 사용하도록 만든다.     
컴퓨팅 등록을 한후 인증 토큰을 얻은뒤 서버를 구동시키는 방법으로 Anywhere플릿에서 서버를 구동할수 있게 된다.      

컴퓨팅 등록하기       
```
aws gamelift register-compute \     
    --compute-name HardwareAnywhere \      
    --fleet-id arn:aws:gamelift:us-east-1:111122223333:fleet/fleet-2222bbbb-33cc-44dd-55ee-6666ffff77aa \       
    --ip-address 10.1.2.3 \      
    --location custom-location-1
```

인증토큰 요청하기      
```
aws gamelift get-compute-auth-token \     
    --fleet-id arn:aws:gamelift:us-east-1:111122223333:fleet/fleet-2222bbbb-33cc-44dd-55ee-6666ffff77aa \     
    --compute-name HardwareAnywhere
```    

이후에 SetServerParameters함수에서 필요한 정보들을 알수있게 명령어로 알려준후 Server가 구동된다.       

```
<Server파일 위치> -log ^   
-authtoken=<토큰> ^  
-hostid=<호스트> ^   
-fleetid=<플릿 아이디> ^   
-websocketurl=<GameLiftServerSkdEndpoint > ^   
-port=<포트번호>   
```

이러한 명령어를 통해 Anywhere플릿을 사용해 서버를 구동시킬수 있게 된다.

### EC2플릿으로 구동하기

EC2는 AWS에서 자체적인 하드웨어를 제공해 Server를 구동할수 있도록 만들어져 있다.       
EC2를 사용하기 위해서는 서버 파일을 AWS에 빌드를 한후 플릿을 생성해 AWS의 하드웨어를 사용해 서버를 구동시키는 방법이다.

CMD명령어로 AWS로 빌드파일을 보낼수 있다.        
```
aws gamelift upload-build ^
--name <name> ^
--operating-system <name>
--server-sdk-version <"version">
--build-root <Path>
--build-version <version>
--region <name>
```

이후 빌드가 완성되면 AWS에서 확인할수 있으며 업로드된 빌드를 바탕으로 EC2플릿을 만들어서 사용할수 있다.    


## Session

AWS의 플릿을 사용해 서버를 구동하게되면 게임 세션을 만들어 멀티플레이환경의 게임을 작동시킬수 있게 된다.        
이때 게임 세션을 만들수 있도록 AWS와 언리얼 엔진의 코드와 트리거 될수 있도록 하는게 HTTPRequest 이다.              
먼저 언리얼 엔진을 통해 만들어진 UI등으로 게임 세션을 만들려는 요청이 들어오게 되면 HTTPRequest를 AWS로 보내 해당 작업이 진행된후 다시 Reponse를 받아 필요한 정보들을 받아 완료를 할수 있게 만든다.              
이때 정보를 JSON형태로 주고 받으며 AWS에서 해당 요청을 받고 처리할수 있도록 만들어진 기능이 바로 Lambda이다.                

![ScreenShot00005](https://github.com/user-attachments/assets/0271a377-9b13-4894-bc33-454fc569b83b)
> 해당 위젯은 Join버튼을 누르면 게임세션을 찾거나 게임세션이 없을경우 게임세션을 만들어서 접속할수 있도록 만드는 위젯이다.


``` C++
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

	UDSLocalPlayerSubssytem* LocalPlayerSubSystem = GetDSLocalPlayerSubSystem();
	if (IsValid(LocalPlayerSubSystem))
	{
		Request->SetHeader(TEXT("Authorization"),LocalPlayerSubSystem->GetAuthResult().AccessToken);
	}

	Request->ProcessRequest();
}
```

Join버튼을 클릭하게되면 해당 함수가 실행되며 언리얼 엔진의 코드를 통해 HTTP가 요청되 AWS의 Lambda함수가 작동하게 된다.


```mjs
import { GameLiftClient, ListFleetsCommand, DescribeFleetAttributesCommand, DescribeGameSessionsCommand, CreateGameSessionCommand    } from "@aws-sdk/client-gamelift";

export const handler = async (event) => {

  const gameLiftClient = new GameLiftClient( {region : process.env.REGION} );

  try{
    
  const listFleetsInput = {
    Limit: 10
  };
    const listFleetsCommand = new ListFleetsCommand(listFleetsInput);
    const listFleetsResponse = await gameLiftClient.send(listFleetsCommand);
    const fleetIds = listFleetsResponse.FleetIds;

    const describeFleetAttributesInput = { // DescribeFleetAttributesInput
      FleetIds: fleetIds,
      Limit: 10
    };

    const describeFleetAttributesCommand = new DescribeFleetAttributesCommand(describeFleetAttributesInput);
    const describeFleetAttributesResponse = await gameLiftClient.send(describeFleetAttributesCommand);

    const fleetAttributes = describeFleetAttributesResponse.FleetAttributes;
    
    let fleetId;
    for(const fleetAttribute of fleetAttributes){
      if(fleetAttribute.Status === "ACTIVE"){
        fleetId = fleetAttribute.FleetId;
        break;
      }
    }

    const describeGameSessionsInput = {
      FleetId: fleetId,
      Limit: 10,
      StatusFilter: "ACTIVE",
    };

    const describeGameSessionsCommand = new DescribeGameSessionsCommand(describeGameSessionsInput);
    const describeGameSessionsResponse = await gameLiftClient.send(describeGameSessionsCommand);

    const gameSessions = describeGameSessionsResponse.GameSessions
    let gameSession;
    for(const session of gameSessions){
      if(session.CurrentPlayerSessionCount < session.MaximumPlayerSessionCount && session.PlayerSessionCreationPolicy === "ACCEPT_ALL")
      {
        gameSession = session;
        break;
      }
    }
    if(gameSession){
      //found and active game session with room fo more players
    }
    else{
      //no game session found create one.
      const createGameSessionInput = {
          GameProperties: [ 
          { 
            Key: "difficulty",
            Value: "novice", 
          },
        ],
        FleetId: fleetId,
        MaximumPlayerSessionCount: 20,
        /*Location: "custom-home-desktop"*/
      };
      const createGameSessionCommand = new CreateGameSessionCommand(createGameSessionInput);
      const createGameSessionResponse = await gameLiftClient.send(createGameSessionCommand);
      gameSession = createGameSessionResponse.GameSession;
    }

    return gameSession;


  }catch(error){
    return error;
  }
};

```

해당 AWS의 Lambda를 통해 GameSession을 찾거나 새롭게 만들어서 언리얼 엔진의 C++의 Response 함수로 다시 들어오게 된다.

```C++
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
```

Response함수를 통해서 받은 데이터들은 Lambda에서는 Json형태로 보내지만 Unreal엔진에서 Json형태로 사용하기 위해서는 Converter를 사용해 구조체 형태로 변형해 Response를 통해 얻은 정보를 저장하게 된다.

``` C++
void UGameSessionsManager::HandleGameSessionStatus(const FString& Status, const FString& SessionId)
{
	if (Status.Equals(TEXT("ACTIVE")))
	{
		BroadcastJoinGameSessionMessage.Broadcast(TEXT("Found active Game Session. Creating a Player Session..."), false);
		
		if (UDSLocalPlayerSubssytem* LocalPlayerSubSystem = GetDSLocalPlayerSubSystem(); IsValid(LocalPlayerSubSystem))
		{
			TryCreatePlayerSession(LocalPlayerSubSystem->UserName, SessionId);
		}	
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

```
이후 받은 Session정보들을 확인한후 ACTIVE상태일경우 PlyerSession을 생성하게 되고 ACTIVE상태가 아닐경우 타이머를 사용해 일정시간이 지난후 Join함수를 다시 호출하도록 만든다.

``` C++
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

```
PlayerSession을 생성하기 위해 다시한번 HTTP요청을 AWS로 보내게 되고 이때 필요한 정보들을 Json형태로 만든후 FJsonSerializer를 통해 AWS의 Lambda의 Event로 보내게 된다.

```mjs
// AWS 의 CreatePlayerSession Lambda함수

import { GameLiftClient, CreatePlayerSessionCommand } from "@aws-sdk/client-gamelift"; // ES Modules import

export const handler = async (event) => {


  try{
    const gameLiftClient = new GameLiftClient( {region : process.env.REGION } );
    const createPlayerSessionInput = { 
      GameSessionId: event.gameSessionId, 
      PlayerId: event.playerId,
      /*Location: "custom-home-desktop"*/ //remove this for EC2 fleets
    };

    const createPlayerSessionCommand = new CreatePlayerSessionCommand(createPlayerSessionInput);
    const createPlayerSessionresponse = await gameLiftClient.send(createPlayerSessionCommand);

    return createPlayerSessionresponse.PlayerSession;

  }catch(error){
    return error;
  }
};
```
AWS의 Lambda에서는 Unreal에서 받은 정보들을 사용해 PlayerSession을 만든후 만들어진 PlayerSession의 정보들을 다시 Unreal로 보내게 된다.

```C++
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

		const FString Options = "?PlayerSessionId=" + PlayerSesssion.PlayerSessionId + "?Username=" + PlayerSesssion.PlayerId;

		const FString IPAndPort = PlayerSesssion.IpAddress + TEXT(":") + FString::FromInt(PlayerSesssion.Port);
		const FName Address(*IPAndPort);
		UGameplayStatics::OpenLevel(this, Address, true, Options);
	}
}
```
Unreal에서 AWS의 Lambda의 응답을 받게되면 Lambda를 통해 들어온 정보들을 다시 구조체의 형태로 변경한후 Game에 들어갈 준비를 한후 OpenLevel을 사용해 Map을 이동하게 된다.

```C++
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
```
이후 Player는 Lobby로 보내질 준비를 하게되고 Error가 발생하지 않게되면 PlayerSession을 바탕으로 Player를 Lobby로 받아들이게 된다.
일정한 수의 Player가 Lobby에 들어오면 Countdown을 시작해 GameMap으로 이동을 시작해 Game을 Play할수 있게 된다.




## Cognito

AWS에서는 게임에서 사용할수 있는 사용자들의 계정을 만들고 관리할수 있는 기능인 Cognito 기능이 존재해 해당 기능을 사용해서 게임에서 게임 계정을 만들고 AWS에서 관리하도록 만들수 있다.






## DynamoDB

## Carrer

## Leaderboard


----------------------------------------------------------------------------------------------------------------------------------


//EC2 Fleet를 만들기 2025 3 27
