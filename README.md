# AWS를 활용해 DedicatedServer 를 사용하기

AWS와 Unreal DedicatedServer를 같이 사용해서 게임을 작동시키는 방법에 대해서 공부 해보기.
AWS의 GameLift플러그인을 UE5에 추가시켜 AWS의 GameLift기능들을 UE5에서 사용할수 있도록 만들고,
AWS의 Cognito기능과 DynamoDB기능을 사용해 사용자 풀을 만들고 사용자의 데이터베이스를 만들어 AWS에서 관리할수 있도록 만들었다.

AWS와 UE5를 연결하는 기능과 게임의 기능을 분리해서 다른 프로젝트에 쉽게 AWS기능을 사용할수 있도록 만들어져 있다.

![Career](https://github.com/user-attachments/assets/12ef9ffb-f8db-40c8-82ae-bc96caf4eb76)



<details><summary> 구분</summary>
<p>  
  
 * [GameLift](#GameLift)
   
 * [Session](#Session)

 * [Cognito](#Cognito)

 * [Career](#Career)

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

해당 AWS의 Lambda를 서버가 존재할경우 ACTIVE 되어있는 GameSession을 찾거나 새롭게 만들어서 언리얼 엔진의 C++의 Response 함수로 다시 들어오게 된다.

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

![Lobby](https://github.com/user-attachments/assets/6e224308-2539-4c02-840f-6577dda3d7af)
> LobbyMap으로 이동한 모습이다. Lobby로 들어온 Player들을 체크하고 일정한 수가 넘어가면 Countdown을 시작하도록 만든다.

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

### SignIn

![ScreenShot00007](https://github.com/user-attachments/assets/49cc867c-a20b-4339-850c-b321073c753c)
> 만약 AWS의 Cognito계정이 존재할경우 아이디와 비밀번호를 입력해 접속할수 있도록 만들었다.

```C++
void UPortalManager::SignIn(const FString& UserName, const FString& Password)
{
	SignInStatusMessageDelegate.Broadcast(TEXT("SignIn in..."), false);
	check(APIData);

	TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &UPortalManager::SignIn_Response);

	const FString APIUrl = APIData->GetAPIEndPoint(DedicatedServersTag::Portal::SignIn);

	Request->SetURL(APIUrl);
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

	LastUserName = UserName;
	TMap<FString, FString> ContentParams = {
		{TEXT("username"),UserName},
		{TEXT("password"),Password}
	};
	const FString Content = SerializeJsonContent(ContentParams);
	Request->SetContentAsString(Content);
	Request->ProcessRequest();
}
```

Http요청으로 AWS로 SignIn에 필요한 정보들을 보낸후 응답받도록 만들어진 함수들이다.
응답에 성공하게 되면 PlayerSubsystem에 사용자에 필요한 토큰, 사용자 이메일등을 따로 저장해 사용할수 있도록 한다.

```mjs
import { CognitoIdentityProviderClient, InitiateAuthCommand, GetUserCommand } from "@aws-sdk/client-cognito-identity-provider"; // ES Modules import

export const handler = async (event) => {

  const cognitoIdentityProviderClient = new CognitoIdentityProviderClient({region : process.env.REGION});

  const { username, password,refreshToken } = event;
  if(refreshToken){
    const refreshTokensInput = {
      AuthFlow: "REFRESH_TOKEN_AUTH",
      ClientId: process.env.CLIENT_ID,
      AuthParameters: {
        REFRESH_TOKEN: refreshToken
      }
    };
    const initiateAuthCommand = new InitiateAuthCommand(refreshTokensInput);
    try{
      const initiateAuthResponse = await cognitoIdentityProviderClient.send(initiateAuthCommand);
      return initiateAuthResponse;
    }
    catch(error)
    {
      return error;
    }
    
  }else{
    const initateAutInput = {
      AuthFlow: "USER_PASSWORD_AUTH",
      ClientId: process.env.CLIENT_ID,
      AuthParameters: {
        USERNAME: username,
        PASSWORD: password
      }
    };
  
    const initiateAuthCommand = new InitiateAuthCommand(initateAutInput);
  
    try{
      const initiateAuthResponse = await cognitoIdentityProviderClient.send(initiateAuthCommand);

      const getUserInput = {
        AccessToken: initiateAuthResponse.AuthenticationResult.AccessToken
      };
      const getUserCommand = new GetUserCommand(getUserInput);
      const getUserResponse = await cognitoIdentityProviderClient.send(getUserCommand);

      let emailAtrribute;
      for(const attribute of getUserResponse.UserAttributes){
        if(attribute.Name === "email"){
          emailAtrribute = attribute.Value;
          break;
        }
      }
      const response = {
        ...initiateAuthResponse,
        email: emailAtrribute
      };

      return response;
    }
    catch(error)
    {
      return error;
    }
  }


};

```
Unreal 엔진의 SignIn함수를 통해서 보내진 ID와 Password를 통해서 Cognito계정에 접속을 하게된후 AccessToken과 Player의 Email을 다시 Response함수로 보내게 주는 Lambda이다.
이때 ID와 Password대신 refreshToken이 들어온 경우 일정 시간이 지나 Token을 다시 받도록 하는 함수가 작동된것으로, 이 함수는 Timer를 사용해 일정시간마다 refreshToken을 보내 Token을 다시 받는다.



```C++
void UPortalManager::SignIn_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (!bWasSuccessful)
	{
		SignInStatusMessageDelegate.Broadcast(HTTPStatusMessage::SomethingWentWrong, true);
	}
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
	if (FJsonSerializer::Deserialize(JsonReader, JsonObject))
	{
		if (ContainsErrors(JsonObject))
		{
			SignInStatusMessageDelegate.Broadcast(HTTPStatusMessage::SomethingWentWrong, true);
			return;
		}
		FDSInitiateAuthResponse InitiateAuthResponse;

		FJsonObjectConverter::JsonObjectToUStruct(JsonObject.ToSharedRef(), &InitiateAuthResponse);
		InitiateAuthResponse.Dump();

		UDSLocalPlayerSubssytem* LocalPlayerSubSystem = GetDSLocalPlayerSubSystem();
		if (IsValid(LocalPlayerSubSystem))
		{
			LocalPlayerSubSystem->InitializeTokens(InitiateAuthResponse.AuthenticationResult,this);
			LocalPlayerSubSystem->UserName = LastUserName;
			LocalPlayerSubSystem->Email = InitiateAuthResponse.email;
		}

		APlayerController* LocalPlayerController = GEngine->GetFirstLocalPlayerController(GetWorld());
		if (IsValid(LocalPlayerController))
		{
			if (IHUDManagement* HUDManagement = Cast<IHUDManagement>(LocalPlayerController->GetHUD()))
			{
				HUDManagement->OnSignIn();
			}
		}
	}
}
```

### SignUp

![ScreenShot00011](https://github.com/user-attachments/assets/d303865d-a2fe-4d4c-ace1-46a7ad4dfe8f)

Cognito계정을 Unreal엔진을 통해서 만들수 있도록 하는 것으로 사용할 Id, Password,Email을 Unreal의 Http로 AWS Cognito로 보내 사용자 계정을 만들도록 하는 방법이다.

```C++
void UPortalManager::SignUp(const FString& UserName, const FString& Password, const FString& Email)
{
	SignUpStatusMessageDelegate.Broadcast(TEXT("Creating a new account."), false);
	check(APIData);

	TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &UPortalManager::SignUp_Response);

	const FString APIUrl = APIData->GetAPIEndPoint(DedicatedServersTag::Portal::SignUp);

	Request->SetURL(APIUrl);
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

	LastUserName = UserName;
	TMap<FString, FString> ContentParams = {
		{TEXT("username"),UserName},
		{TEXT("password"),Password},
		{TEXT("email"),Email}
	};
	const FString Content = SerializeJsonContent(ContentParams);
	Request->SetContentAsString(Content);
	Request->ProcessRequest();
}
```
각각의 텍스트에 필요한 정보들을 넣어서 버튼을 누르게 되면 SignUp함수가 호출되며 각각의 정보들을 HTTP요청으로 보내게 된다.

```mjs
import { CognitoIdentityProviderClient, SignUpCommand } from "@aws-sdk/client-cognito-identity-provider"; // ES Modules import

export const handler = async (event) => {
  
  const cognitoIdentityProviderClient = new CognitoIdentityProviderClient({region : process.env.REGION});

  const clientId = process.env.CLIENT_ID;

  const{ username,password,email }= event;

  const signUpInput = {
    ClientId: clientId,
    Username: username,
    Password: password,
    UserAttributes: [
      {
        Name: "email",
        Value: email
      }
    ]
  };

  try{
    const signUpCommand = new SignUpCommand(signUpInput);
    const signUpResponse = await cognitoIdentityProviderClient.send(signUpCommand);
  
    return signUpResponse;
  }catch(error)
  {
    return error;
  }

};
```
이러한 Lambda함수를 통해 새롭게 계정이 만들어 지게 된다.     

```mjs
import { CognitoIdentityProviderClient, ListUsersCommand } from "@aws-sdk/client-cognito-identity-provider"; // ES Modules import

export const handler = async (event) => {
  const email = event.request.userAttributes.email;

  const cognitoIdentityProviderClient = new CognitoIdentityProviderClient({region : process.env.REGION });
  const listUsersInput = {
    UserPoolId : event.userPoolId,
    Filter : `email = "${email}"`
  }
  const listUsersCommand = new ListUsersCommand(listUsersInput);

  try{
    const listUsersResponse = await cognitoIdentityProviderClient.send(listUsersCommand);
    if(listUsersResponse.Users.length > 0)
    {
      throw new Error("A user with this email aleady exists.")
    }
    return event;
  }catch(error){
    console.error(error);
    throw new Error(error.message);
  }
};

```
이때 사용자가 전해준 Email정보가 이미 존재할경우를 대비해 Cognito의 확장기능인 사전 가입 Lambda트리거를 사용해 SignUp이 되기전에 해당 Lambda를 실행해 이미 Email이 존재하는지 확인한다음 문제가 없을경우 다음단계로 넘어가게 된다.

```C++
void UPortalManager::SignUp_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (!bWasSuccessful)
	{
		SignUpStatusMessageDelegate.Broadcast(HTTPStatusMessage::SomethingWentWrong, true);
	}
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
	if (FJsonSerializer::Deserialize(JsonReader, JsonObject))
	{
		if (ContainsErrors(JsonObject))
		{
			SignUpStatusMessageDelegate.Broadcast(HTTPStatusMessage::SomethingWentWrong, true);
			return;
		}

		FJsonObjectConverter::JsonObjectToUStruct(JsonObject.ToSharedRef(), &LastSignUpResponse);
		OnSignUpSucceeded.Broadcast();
	}

}
```
계정 생성시 문제가 없을경우 HTTP의 응답함수에서 성공하게 되며 델리게이트를 통해 성공을 Broadcast하게 된다.

```C++
void USignInOverlay::OnSignUpSucceeded()
{
	SignUpPage->ClearTextBoxes();
	SignUpPage->Button_SignUp->SetIsEnabled(true);
	ConfirmSignUpPage->TextBlock_Destination->SetText(FText::FromString(PortalManager->LastSignUpResponse.CodeDeliveryDetails.Destination));
	ShowConfirmPage();
}
```
델리게이트가 broadcast됬을때 반응되는 함수로 각각의 TextBlock들의 내용을 지운후 Confirm Page로 넘어가게 된다.


### Confirm

![ScreenShot00016](https://github.com/user-attachments/assets/64568038-a7f7-41f1-83d8-0088eee30ca9)

SignUp버튼을 누르게 되면 SignUp에서 사용된 Email주소로 사용자의 확인 코드가 전해지게 된다.
AWS에서는 UserPool을 만들때 확인할수 있는것들을 선택할수 있는데, 이때 Email로 선택할시 Email로 확인코드가 담긴 메일이 보내지고 이러한 확인코드를 다시 Unreal엔진에서 받아 HTTP요청을 보내 확인할수 있도록 한다.

```C++
void UPortalManager::Confirm(const FString& ConfirmationCode)
{
	check(APIData);
	ConfirmStatusMessageDelegate.Broadcast(TEXT("Checking verification code..."), false);

	TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &UPortalManager::Confirm_Response);

	const FString APIUrl = APIData->GetAPIEndPoint(DedicatedServersTag::Portal::ConfirmSignUp);

	Request->SetURL(APIUrl);
	Request->SetVerb(TEXT("PUT"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

	TMap<FString, FString> ContentParams = {
		{TEXT("username"),LastUserName},
		{TEXT("confirmationCode"),ConfirmationCode}
	};
	const FString Content = SerializeJsonContent(ContentParams);
	Request->SetContentAsString(Content);
	Request->ProcessRequest();
}
```
기존에 저장된 UserName과 이번에 획득한 확인 코드를 HTTP로 보내 AWS의 Lambda로 보내지게 된다.

```mjs
import { CognitoIdentityProviderClient, ConfirmSignUpCommand } from "@aws-sdk/client-cognito-identity-provider"; // ES Modules import

export const handler = async (event) => {
  const cognitoIdentityProviderClient = new CognitoIdentityProviderClient({region : process.env.REGION});

  const{ username,confirmationCode }= event;

  const confrimSignUpInput = {
    ClientId : process.env.CLIENT_ID,
    Username : username,
    ConfirmationCode : confirmationCode
  };

  const confirmSignUpCommand = new ConfirmSignUpCommand(confrimSignUpInput);

  try{
    const response = await cognitoIdentityProviderClient.send(confirmSignUpCommand);
    return response;
  }
  catch(error){
    return error;
  }
};

```
Lambda에서는 해당 UserName과 코드를 확인한후 결과가 담긴 값을 다시 Unreal엔진으로 보내게 된다.

```C++
void UPortalManager::Confirm_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (!bWasSuccessful)
	{
		ConfirmStatusMessageDelegate.Broadcast(HTTPStatusMessage::SomethingWentWrong, true);
	}
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
	if (FJsonSerializer::Deserialize(JsonReader, JsonObject))
	{
		if (ContainsErrors(JsonObject))
		{
			if (JsonObject->HasField(TEXT("name")))
			{
				FString Exception = JsonObject->GetStringField(TEXT("name"));
				if (Exception.Equals(TEXT("CodeMismatchException")))
				{
					ConfirmStatusMessageDelegate.Broadcast(TEXT("Incorrect verification code."), true);
					return;
				}
			}
			ConfirmStatusMessageDelegate.Broadcast(HTTPStatusMessage::SomethingWentWrong, true);
			return;
		}

		OnConfirmSucceeded.Broadcast();
	}
}
```
Unreal엔진의 Response함수에서 해당 값을 받은후 올바르게 되었을경우 OnConfirmSucceeded델리게이트를 Broadcast하게 된다.

![ScreenShot00018](https://github.com/user-attachments/assets/26dba947-a19d-4f67-b85d-5db90a5183b2)

알맞은 코드가 들어왔을경우 코드 확인 UI가 뜨게 되고 버튼을 누르면 SignIn페이지로 돌아가 SignUp에서 사용한 Username과 Password를 사용해 게임에 접속할수 있게 만들었다.


## Career



![ScreenShot00022](https://github.com/user-attachments/assets/01879c4a-9787-4260-aa61-1c01f05cf682)

AWS의 DynamoDB기능과 Cognito기능을 같이 사용해 Player가 지금까지 play한 게임들의 스텟과 승리, 패배등을 Database에 저장해 엔진에서 요청시 AWS의 정보들을 엔진으로 보내 확인할수 있도록 하는 기능들이 있다.

```mjs

//계정 생성시 사후 확인 Lambda트리거를 사용해 해당 람다가 실행되게 된다.

import { DynamoDBClient, PutItemCommand } from "@aws-sdk/client-dynamodb"; // ES Modules import

export const handler = async (event) => {

  console.log(JSON.stringify(event));


  if(event.triggerSource ==='PostConfirmation_ConfirmSignUp' )
  {
    const dynamoDBClient = new DynamoDBClient({region : process.env.REGION});

    const username = event.userName;
    const cognitosub = event.request.userAttributes.sub;
    const email = event.request.userAttributes.email;

    const putItemInput = {
      TableName: "Players",
      Item:{
        "databaseid":{ S:cognitosub },
        "username":{ S:username },
        "email":{ S:email },
      }
    }
    const putItemCommand = new PutItemCommand(putItemInput);
    try{
      const putItemResponse = await dynamoDBClient.send(putItemCommand);
    }catch(error)
    {
      return error;
    }
  }
  return event;
};

```
먼저 플레이어의 계정이 만들어 지게 될경우 Cognito의 확장기능인 Lambda 트리거를 사용해 해당 계정의 sub, email, username등을 사용해 DynamoDB의 테이블을에 데이터들을 추가시켜 준다.

```C++
void UGameStatsManager::RetrieveMatchStats()
{
	RetrieveMatchStatsStatusMessage.Broadcast(TEXT("Retrieving match stats..."), false);
	
	UDSLocalPlayerSubssytem* LocalPlayerSubSystem = GetDSLocalPlayerSubSystem();
	if (!IsValid(LocalPlayerSubSystem))
	{
		return;
	}
	check(APIData);

	TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
	const FString APIUrl = APIData->GetAPIEndPoint(DedicatedServersTag::GameStatsAPI::RetrieveMatchStats);
	Request->OnProcessRequestComplete().BindUObject(this, &UGameStatsManager::RetrieveMatchStats_Response);

	Request->SetURL(APIUrl);
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

	TMap<FString, FString> Params = {
		{TEXT("accessToken"), LocalPlayerSubSystem->GetAuthResult().AccessToken}
	};
	const FString Content = SerializeJsonContent(Params);
	Request->SetContentAsString(Content);
	Request->ProcessRequest();
}
```
이후에 해당 플레이어가 개인 Carrer를 확인하고 싶을때 해당 요청을 AWS로 보내게 되고 
```mjs
import { CognitoIdentityProviderClient, GetUserCommand } from "@aws-sdk/client-cognito-identity-provider"; // ES Modules import
import { DynamoDBClient, GetItemCommand } from "@aws-sdk/client-dynamodb"; // ES Modules import
import {marshall,unmarshall} from "@aws-sdk/util-dynamodb";

export const handler = async (event) => {
  const cognitoClient = new CognitoIdentityProviderClient({region : process.env.REGION});
  const dynamoDBClient = new DynamoDBClient({region : process.env.REGION});

  try{
    const getUserCommand = new GetUserCommand({AccessToken: event.accessToken});
    const getUserResponse = await cognitoClient.send(getUserCommand);
    const sub = getUserResponse.UserAttributes.find(attribute => attribute.Name === "sub").Value;

    const getItemInput = {
      TableName : "Players",
      Key: marshall( { databaseid : sub} )
    };
    const getItemCommand = new GetItemCommand(getItemInput);
    const getItemResponse = await dynamoDBClient.send(getItemCommand);
    const playerStats = getItemResponse.Item ? unmarshall(getItemResponse.Item) : {};

    return playerStats;

  }catch(error){
    return error;
  }

};
```

AWS에서는 해당 Lambda가 실행된후 데이터 베이스에서 Player를 검색후 해당 Player의 데이터들을 Unreal엔진으로 보내주게 된다.

```C++
void UGameStatsManager::RetrieveMatchStats_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (!bWasSuccessful)
	{
		OnRetrieveMatchStatsResponseReceived.Broadcast(FDSRetrieveMatchStatsResponse());
		RetrieveMatchStatsStatusMessage.Broadcast(HTTPStatusMessage::SomethingWentWrong, false);
		return;
	}
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
	if (FJsonSerializer::Deserialize(JsonReader, JsonObject))
	{
		if (ContainsErrors(JsonObject))
		{
			OnRetrieveMatchStatsResponseReceived.Broadcast(FDSRetrieveMatchStatsResponse());
			RetrieveMatchStatsStatusMessage.Broadcast(HTTPStatusMessage::SomethingWentWrong, false);
			return;
		}
		FDSRetrieveMatchStatsResponse RetrieveMatchStatsResponse;
		FJsonObjectConverter::JsonObjectToUStruct(JsonObject.ToSharedRef(), &RetrieveMatchStatsResponse);
		RetrieveMatchStatsResponse.Dump();

		OnRetrieveMatchStatsResponseReceived.Broadcast(RetrieveMatchStatsResponse);
		RetrieveMatchStatsStatusMessage.Broadcast(TEXT(""), false);
	}
}
```
해당 정보들은 Unreal엔진에서 다시 구조체의 형태로 변환한후 델리게이트를 통해 UI에게 보내지게 되고 해당 정보들을 토대로 각각의 Carrer들을 만들어 표시하게 된다.


```C++
void UShooterCareerPage::OnRetrieveMatchStats(const FDSRetrieveMatchStatsResponse& RetrieveMatchStatsResponse)
{
	Super::OnRetrieveMatchStats(RetrieveMatchStatsResponse);

	ScrollBox_Achievement->ClearChildren();

	TMap<ESpecialElimType, int32> AchievementData;

	if (RetrieveMatchStatsResponse.hits > 0) AchievementData.Emplace(ESpecialElimType::Hits, RetrieveMatchStatsResponse.hits);
	if (RetrieveMatchStatsResponse.misses > 0) AchievementData.Emplace(ESpecialElimType::Misses, RetrieveMatchStatsResponse.misses);
	if (RetrieveMatchStatsResponse.scoredElims > 0) AchievementData.Emplace(ESpecialElimType::ScoredElims, RetrieveMatchStatsResponse.scoredElims);
	if (RetrieveMatchStatsResponse.defeats > 0) AchievementData.Emplace(ESpecialElimType::Defeats, RetrieveMatchStatsResponse.defeats);
	if (RetrieveMatchStatsResponse.highestStreak > 0) AchievementData.Emplace(ESpecialElimType::Streak, RetrieveMatchStatsResponse.highestStreak);
	if (RetrieveMatchStatsResponse.dethroneElims > 0) AchievementData.Emplace(ESpecialElimType::Dethrone, RetrieveMatchStatsResponse.dethroneElims);
	if (RetrieveMatchStatsResponse.gotFirstBlood > 0) AchievementData.Emplace(ESpecialElimType::FirstBlood, RetrieveMatchStatsResponse.gotFirstBlood);
	if (RetrieveMatchStatsResponse.revengeElims > 0) AchievementData.Emplace(ESpecialElimType::Revenge, RetrieveMatchStatsResponse.revengeElims);
	if (RetrieveMatchStatsResponse.showstopperElims > 0) AchievementData.Emplace(ESpecialElimType::Showstopper, RetrieveMatchStatsResponse.showstopperElims);
	if (RetrieveMatchStatsResponse.headShotElims > 0) AchievementData.Emplace(ESpecialElimType::Headshot, RetrieveMatchStatsResponse.headShotElims);
	
	check(SpecialElimData);

	for (const TPair<ESpecialElimType, int32>& Pair : AchievementData)
	{
		const FString& CareerAchievementName = SpecialElimData->SpecialElimInfo.FindChecked(Pair.Key).CareerPageAchievementName;
		UTexture2D* Icon = SpecialElimData->SpecialElimInfo.FindChecked(Pair.Key).ElimIcon;

		UCareerAchievement* CareerAchievement = CreateWidget<UCareerAchievement>(this, CareerAchievementClass);
		if (IsValid(CareerAchievement))
		{
			CareerAchievement->SetAchievementText(CareerAchievementName, Pair.Value);
			if (Icon)
			{
				CareerAchievement->SetAchievementIcon(Icon);
			}
		}

		ScrollBox_Achievement->AddChild(CareerAchievement);
	}
}
```
델리게이트의 boradcast를 통해서 들어온 데이터들을 각각 UI로 만들어서 스크롤 박스에 추가시켜 화면에 표시하도록 만든다.

게임의 Match가 끝났을 경우 Game에서 변경된 점들을 다시 AWS의 데이터로 보내주는 기능또한 가지고 있다.

```C++
void AMatchPlayerState::OnMatchEnded(const FString& Username)
{
	Super::OnMatchEnded(Username);

	AMatchGameState* MatchGameState = Cast<AMatchGameState>(UGameplayStatics::GetGameState(this));
	if (IsValid(MatchGameState))
	{
		bWinner = MatchGameState->GetLeader() == this;
	}

	FDSRecordMatchStatsInput RecordMatchStatsInput;
	RecordMatchStatsInput.username = Username;

	RecordMatchStatsInput.matchStats.ScoredElims = ScoredElims;
	RecordMatchStatsInput.matchStats.defeats = Defeats;
	RecordMatchStatsInput.matchStats.hits = Hits;
	RecordMatchStatsInput.matchStats.misses = Misses;
	RecordMatchStatsInput.matchStats.headShotElims = HeadShotElims;
	RecordMatchStatsInput.matchStats.highestStreak = HighestStreak;
	RecordMatchStatsInput.matchStats.revengeElims = RevengeElims;
	RecordMatchStatsInput.matchStats.dethroneElims = DethroneElims;
	RecordMatchStatsInput.matchStats.showstopperElims = ShowStopperElims;
	RecordMatchStatsInput.matchStats.gotFirstBlood = bFirstBlood ? 1 : 0;
	RecordMatchStatsInput.matchStats.matchWins = bWinner ? 1 : 0;
	RecordMatchStatsInput.matchStats.matchLosses = bWinner ? 0 : 1;

	RecordMatchStats(RecordMatchStatsInput);
}
```
경기가 끝났을경우 해당 PlayerState의 함수에서 AWS로 보낼 Input들을 구조체 형식으로 저장하게 되고 HTTP요청을 통해 AWS로 데이터들을 보내게 된다.

```C++
void UGameStatsManager::RecordMatchStats(const FDSRecordMatchStatsInput& RecordMatchStatsInput)
{
	//TSharedPtr<FJsonObject> JsonObject = MakeShared<FJsonObject>();
	//FJsonObjectConverter::UStructToJsonObject(FDSRecordMatchStatsInput::StaticStruct(), &RecordMatchStatsInput, JsonObject.ToSharedRef());

	FString JsonString;
	FJsonObjectConverter::UStructToJsonObjectString(FDSRecordMatchStatsInput::StaticStruct(), &RecordMatchStatsInput, JsonString);

	TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
	const FString APIUrl = APIData->GetAPIEndPoint(DedicatedServersTag::GameStatsAPI::RecordMatchStats);
	Request->OnProcessRequestComplete().BindUObject(this, &UGameStatsManager::RecordMatchStats_Response);

	Request->SetURL(APIUrl);
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetContentAsString(JsonString);

	Request->ProcessRequest();
}
```

```mjs
import { CognitoIdentityProviderClient, AdminGetUserCommand } from "@aws-sdk/client-cognito-identity-provider"; // ES Modules import
import { DynamoDBClient, GetItemCommand, PutItemCommand  } from "@aws-sdk/client-dynamodb"; // ES Modules import
import { marshall,unmarshall } from "@aws-sdk/util-dynamodb";

export const handler = async (event) => {
  const cognitoIdentityProviderClient = new CognitoIdentityProviderClient({region : process.env.REGION});
  const dynamoDBClient = new DynamoDBClient({region : process.env.REGION});

  try{
    const adminGetUserInput = {
      Username : event.username,
      UserPoolId:process.env.USER_POOL_ID
    };
    const adminGetUserCommand = new AdminGetUserCommand(adminGetUserInput);
    const adminGetUserResponse = await cognitoIdentityProviderClient.send(adminGetUserCommand);

    const sub = adminGetUserResponse.UserAttributes.find(attribute => attribute.Name === "sub").Value;
    const email = adminGetUserResponse.UserAttributes.find(attribute => attribute.Name === "email").Value;

    const getItemInput = {
      TableName : "Players",
      Key: marshall ({ databaseid  : sub}),
    };

    const getItemCommand = new GetItemCommand(getItemInput);
    const dbResponse = await dynamoDBClient.send(getItemCommand);
    let statsFromDB = unmarshall(dbResponse.Item);

    const eventMatchStats = event.matchStats;

    for(const key in eventMatchStats){
      if(statsFromDB[key] !== undefined){
        statsFromDB[key] += eventMatchStats[key];
      } else{
        statsFromDB[key] = eventMatchStats[key];
      }
    }

    const putItemInput ={
      TableName : "Players",
      Item : marshall({...statsFromDB})
    };
    const putItemCommand  =new PutItemCommand(putItemInput);
    await dynamoDBClient.send(putItemCommand);

    return {
      statusCode : 200,
      body : `Update match stats for ${event.username}`
    };
  }catch(error)
  {
    return error;
  }
};

```
Unreal에서 받은 데이터들을 AWS의 DynamoDB의 데이터베이스에 추가시킨후 단순히 확인의 용도로 Response함수를 받게 된다.

```C++
void UGameStatsManager::RecordMatchStats_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (!bWasSuccessful)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to send RecordMatchStats request"));
	}
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
	if (FJsonSerializer::Deserialize(JsonReader, JsonObject))
	{
		ContainsErrors(JsonObject);
	}
}
```
Response함수에서 문제가 존재할경우 Log등을 통해 Error를 식별하고 조치할수 있게 만들어 두었다.


## Leaderboard

![ScreenShot00000](https://github.com/user-attachments/assets/550986d8-798e-4a28-8737-3590fc4ee88b)

Leaderboard Page를 만들어서 사용자들의 Wins의 순위를 정하고 화면에 표시할수 있도록 만들어져 있다.


```C++
void UGameStatsManager::RetrieveLeaderboard()
{
	RetrieveLeaderboardStatusMessage.Broadcast(TEXT("Retrieving Leaderboard..."),false);

	TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
	const FString APIUrl = APIData->GetAPIEndPoint(DedicatedServersTag::GameStatsAPI::RetrieveLeaderboard);
	Request->OnProcessRequestComplete().BindUObject(this, &UGameStatsManager::RetrieveLeaderboard_Response);

	Request->SetURL(APIUrl);
	Request->SetVerb(TEXT("GET"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

	Request->ProcessRequest();
}
```
Page를 클릭해 화면에 표시하게 되면 Show함수가 호출되고 Show함수에서 Leaderboard를 Retrieve하는 HTTP함수를 호출해 AWS로 보내게 된다.

```mjs
import {DynamoDBClient , ScanCommand} from "@aws-sdk/client-dynamodb";
import {unmarshall} from "@aws-sdk/util-dynamodb";

export const handler = async (event) => {

    const dynamoDBClient = new DynamoDBClient({ region: process.env.REGION });
    const scanCommand = new ScanCommand({
        TableName: "Leaderboard"
    });

    try{
        const scanResponse = await dynamoDBClient.send(scanCommand);
        const leaderboard = scanResponse.Items.map(item => unmarshall(item));
        return {Leaderboard : leaderboard};
    }catch(error){
        return error
    }


};

```

AWS에서는 Leaderboard의 데이터베이스에서 저장된 username과 match의 승리횟수등을 return하게 된다.

```C++
void UGameStatsManager::RetrieveLeaderboard_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (!bWasSuccessful)
	{
		RetrieveLeaderboardStatusMessage.Broadcast(HTTPStatusMessage::SomethingWentWrong, false);
		UE_LOG(LogDedicatedServers, Error, TEXT("Falied to retrieve leaderboard."));
		return;
	}
	TArray<FDSLeaderboardItem> LeaderboardItems;
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
	if (FJsonSerializer::Deserialize(JsonReader, JsonObject))
	{
		if (ContainsErrors(JsonObject))
		{
			RetrieveLeaderboardStatusMessage.Broadcast(HTTPStatusMessage::SomethingWentWrong, false);
			return;
		}
		const TArray<TSharedPtr<FJsonValue>>* LeaderboardJsonArray;
		if (JsonObject->TryGetArrayField(TEXT("Leaderboard"), LeaderboardJsonArray))
		{
			for (const TSharedPtr<FJsonValue>& ItemValue : *LeaderboardJsonArray)
			{
				TSharedPtr<FJsonObject> ItemObject = ItemValue->AsObject();
				if (ItemObject.IsValid())
				{
					FDSLeaderboardItem Item;
					if (FJsonObjectConverter::JsonObjectToUStruct(ItemObject.ToSharedRef(), &Item))
					{
						LeaderboardItems.Add(Item);
					}
					else
					{
						UE_LOG(LogDedicatedServers, Error, TEXT("Falied to parse leaderboard item."));
					}
				}
			}
		}
	}
	OnRetrieveLeaderboard.Broadcast(LeaderboardItems);
	RetrieveLeaderboardStatusMessage.Broadcast(TEXT(""), false);
}
```
응답 함수에서 받은 데이터들은 구조체 형식으로 변환된후 vector에 Add하게 된후 모든 데이터들 받게되면 델리게이트를 통해 Leaderboard의 순위를 결정하기 위해 UI클래스로 넘어가게 된다.

```C++
void ULeaderboardPage::PopulateLeaderboard(TArray<FDSLeaderboardItem>& Leaderboard)
{
	ScrollBox_Leaderboard->ClearChildren();

	CalculateLeaderboardPlaces(Leaderboard);

	for (const FDSLeaderboardItem& Item : Leaderboard)
	{
		ULeaderboardCard* LeaderboardCard = CreateWidget<ULeaderboardCard>(this, LeaderboardCardClass);
		if (IsValid(LeaderboardCard))
		{
			LeaderboardCard->SetPlayerInfo(Item.username, Item.matchWins, Item.place);
			ScrollBox_Leaderboard->AddChild(LeaderboardCard);
		}
	}
}

void ULeaderboardPage::CalculateLeaderboardPlaces(TArray<FDSLeaderboardItem>& OutLeaderboard)
{
	OutLeaderboard.Sort([](const FDSLeaderboardItem& A, const FDSLeaderboardItem& B)
		{
			return A.matchWins > B.matchWins;
		});

	// assign place based on wins, accounting for ties;
	int32 CurrentRank = 1;
	for (int32 i = 0; i < OutLeaderboard.Num(); i++)
	{
		if (i > 0 && OutLeaderboard[i].matchWins == OutLeaderboard[i - 1].matchWins)
		{
			//만약 Win이 같을경우 동일한 Rank부여
			OutLeaderboard[i].place = OutLeaderboard[i - 1].place;
		}
		else
		{
			OutLeaderboard[i].place = CurrentRank++;
		}
	}
}
```
순위를 결정하기 위해 MatchWin에 따라서 들어온 Vector를 정렬하게 되고 정렬된 순서에 따라서 Rank를 부여해 PlayerCard클래스에 보내준후 해당 클래스를 ScrollBox에 추가해 화면에 표시하게 된다.

Leaderboard또한 Game이 종료되었을때 Update하는 기능이 존재 하고 있다.

```C++
void AShooterGameModeBase::OnMatchEnded()
{
	Super::OnMatchEnded();

	TArray<FString> LeaderIds;
	if (AMatchGameState* MatchGameState = GetGameState<AMatchGameState>(); IsValid(MatchGameState))
	{
		TArray<AMatchPlayerState*> Leaders = MatchGameState->GetLeaders();
		for (AMatchPlayerState* Leader : Leaders)
		{
			if (ADSPlayerController* LeaderPC = Cast<ADSPlayerController>(Leader->GetPlayerController());IsValid(LeaderPC))
			{
				LeaderIds.Add(LeaderPC->Username);
			}
		}
	}
	UpdateLeaderboard(LeaderIds);
}
```
GameMode에서 경기 시간이 종료되었을때 호출되는 함수로 GameState에 저장된 Game의 Leader들을 모아서 Update함수로 넘겨주게 된다.
```C++
void UGameStatsManager::UpdateLeaderboard(const TArray<FString>& WinnerUsername)
{

	check(APIData);

	TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
	const FString APIUrl = APIData->GetAPIEndPoint(DedicatedServersTag::GameStatsAPI::UpdateLeaderboard);
	Request->OnProcessRequestComplete().BindUObject(this, &UGameStatsManager::UpdateLeaderboard_Response);

	Request->SetURL(APIUrl);
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
	TArray<TSharedPtr<FJsonValue>> PlayerIdArray;

	for (const FString& Username : WinnerUsername)
	{
		PlayerIdArray.Add(MakeShareable(new FJsonValueString(Username)));
	}
	JsonObject->SetArrayField(TEXT("playerIds"), PlayerIdArray);
	FString Content;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Content);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

	Request->SetContentAsString(Content);
	Request->ProcessRequest();
}
```
Update함수에서는 GameState에서 획득한 Leader들을 FJsonObject형태로 구성한후 HTTP요청을 통해 AWS에게 보내주게 된다.

```mjs
import { CognitoIdentityProviderClient, AdminGetUserCommand } from "@aws-sdk/client-cognito-identity-provider"; // ES Modules import
import { DynamoDBClient, GetItemCommand, PutItemCommand, ScanCommand, DeleteItemCommand } from "@aws-sdk/client-dynamodb";
import { marshall, unmarshall } from "@aws-sdk/util-dynamodb";

const cognitoClient = new CognitoIdentityProviderClient({ region: process.env.REGION  });
const dynamoDBClient = new DynamoDBClient({ region: process.env.REGION  });

export const handler = async (event) => {
  
  // array of username from the match
  const playerIds = event.playerIds;
  const userPoolId = process.env.USER_POOL_ID;

  try{
  // retrieve player data (cognito sub ids)
  const playerData = await retrievePlayerData(playerIds, userPoolId)
  // retrieve player current wins
  const updatedPlayerData = await retrieveCurrentWins(playerData);
  
  // add winers leaderboard
  await updateLeaderboard(updatedPlayerData);
  // ensure only top 20 players
 await ensureTop20Players();
  return {
    statusCode: 200,
    body: "Leaderboard updated successfully"
  };

  }catch(error){

    return error;
  }

};

async function retrievePlayerData(playerIds, userPoolId){
  return await Promise.all(playerIds.map(async (playerId) => {
    const adminGetUserCommand  =new AdminGetUserCommand({
      Username: playerId,
      UserPoolId: userPoolId,
    });
    const adminGetUserResponse = await cognitoClient.send(adminGetUserCommand);
    const databaseId = adminGetUserResponse.UserAttributes.find(attr => attr.Name ==="sub").Value;
    return {playerId, databaseId};
  }));
}

async function retrieveCurrentWins(playerData){
  return await Promise.all(playerData.map( async (player) => {
    const getItemCommand = new GetItemCommand({
      TableName: "Players",
      Key : marshall( { databaseid : player.databaseId} ),
    });
    const getItemResponse = await dynamoDBClient.send(getItemCommand);
    const playerItem = getItemResponse.Item ? unmarshall(getItemResponse.Item) : {};

    let numWins = playerItem.matchWins || 0;
    numWins += 1;

    return {...player, wins : numWins || 0 };
  }));
}

async function updateLeaderboard(playerData){
  return await Promise.all(playerData.map(async (player) => {
    const putItemCommand = new PutItemCommand({
      TableName: "Leaderboard",
      Item : marshall({
        databaseid : player.databaseId,
        username : player.playerId,
        matchWins : player.wins
      })
    });
    const putItemResponse = await dynamoDBClient.send(putItemCommand);
    return putItemResponse.Item ? unmarshall(putItemResponse.Item) : {};
  }));
}

async function ensureTop20Players(){
  const scanCommand = new ScanCommand({
    TableName: "Leaderboard"
  });
  const scanResponse = await dynamoDBClient.send(scanCommand);
  const leaderboardItems = scanResponse.Items.map(item => unmarshall(item));

  // sort player by win in descending order
  leaderboardItems.sort((a, b) => b.matchWins - a.matchWins);
  const top20Players = leaderboardItems.slice(0, 20);
  const playersToRemove = leaderboardItems.slice(20);

  const deletePromises = playersToRemove.map(player => {
    const deleteItemCommand = new DeleteItemCommand({
      TableName: "Leaderboard",
      Key: marshall({ databaseid: player.databaseId })
    });
    return dynamoDBClient.send(deleteItemCommand);
  });
  await Promise.all(deletePromises);
}

```
AWS의 Lambda에서는 가장먼저 Player의 StatData를 확인후 Unreal을 통해 들어온 데이터들을 합쳐서 Update하게 된다.
이후에 Update된 Data들을 확인한후 상위 20명의 Player들을 체크해 Leaderboard Database에 저장하는 로직을 가지고 있다.



----------------------------------------------------------------------------------------------------------------------------------
