// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Game/DS_GameModeBase.h"
#include "GameLiftServerSDK.h"
#include "DS_LobbyGameMode.generated.h"

/**
 * 
 */
class UDS_GameInstanceSubSystem;

UCLASS()
class DEDICATEDSERVERS_API ADS_LobbyGameMode : public ADSGameModeBase
{
	GENERATED_BODY()
public:
	ADS_LobbyGameMode();
	virtual void PostLogin(APlayerController* NewPlayer) override;
	void CheckAndStartLobbyCountdown();
protected:

	virtual void BeginPlay() override;

	virtual void OnCountdownTimerFinished(ECountdownTimerType Type) override;
	virtual void InitSeamlessTravelPlayer(AController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
	virtual void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;
	virtual FString InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId, const FString& Options, const FString& Portal = TEXT("")) override;

	void CheckAndStopLobbyCountdown();
	UPROPERTY()
	ELobbyStatus LobbyStatus;

	UPROPERTY(EditDefaultsOnly)
	int32 MinPlayer;

	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<UWorld> DestinationMap;
private:
	void InitGameLift();
	void SetServerParameters(FServerParameters& OutServerParameters);
	void TryAcceptPlayerSession(const FString& PlayerSessionId, const FString& Username, FString& OutErrorMessage);
	
	UPROPERTY()
	TObjectPtr<UDS_GameInstanceSubSystem> DSGameInstanceSubSystem;

	UPROPERTY(EditDefaultsOnly)
	FCountdownTimerHandle LobbyCountdownTimerHandle;


	
};
