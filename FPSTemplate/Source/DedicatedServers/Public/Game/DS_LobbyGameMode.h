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

protected:

	virtual void BeginPlay() override;

private:
	void InitGameLift();
	void SetServerParameters(FServerParameters& OutServerParameters);

	UPROPERTY()
	TObjectPtr<UDS_GameInstanceSubSystem> DSGameInstanceSubSystem;


	
};
