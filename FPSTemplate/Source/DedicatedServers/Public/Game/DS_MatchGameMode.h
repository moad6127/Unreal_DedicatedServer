// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Game/DS_GameModeBase.h"
#include "DS_MatchGameMode.generated.h"

/**
 * 
 */
UCLASS()
class DEDICATEDSERVERS_API ADS_MatchGameMode : public ADSGameModeBase
{
	GENERATED_BODY()
	
public:

	ADS_MatchGameMode();

	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;;
	virtual void InitSeamlessTravelPlayer(AController* NewPlayer) override;
	UPROPERTY()
	EMatchStatus DS_MatchStatus;
	

protected:
	virtual void OnCountdownTimerFinished(ECountdownTimerType Type) override;

	void SetClientInuptEnabled(bool bEnabled);

	UPROPERTY(EditDefaultsOnly)
	FCountdownTimerHandle PreMatchTimer;

	UPROPERTY(EditDefaultsOnly)
	FCountdownTimerHandle MatchTimer;

	UPROPERTY(EditDefaultsOnly)
	FCountdownTimerHandle PostMatchTimer;

	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<UWorld> LobbyMap;
	

};
