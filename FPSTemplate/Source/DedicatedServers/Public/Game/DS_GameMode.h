// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Game/DSGameModeBase.h"
#include "GameLiftServerSDK.h"
#include "DS_GameMode.generated.h"


DECLARE_LOG_CATEGORY_EXTERN(LogDS_GameMode, Log, All);
/**
 * 
 */
UCLASS()
class DEDICATEDSERVERS_API ADS_GameMode : public ADSGameModeBase
{
	GENERATED_BODY()
public:

	
protected:

	virtual void BeginPlay() override;

private:

	FProcessParameters ProcessParameters;

	void SetServerParameters(FServerParameters& OutServerParameters);
	void ParseCommandLinePort(int32& OutPort);
	void InitGameLift();
	
};
