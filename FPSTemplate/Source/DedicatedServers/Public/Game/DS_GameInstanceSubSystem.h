// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameLiftServerSDK.h"
#include "DS_GameInstanceSubSystem.generated.h"

/**
 * 
 */
UCLASS()
class DEDICATEDSERVERS_API UDS_GameInstanceSubSystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	UDS_GameInstanceSubSystem();
	void InitGameLift(const FServerParameters& ServerParams);

	UPROPERTY(BlueprintReadOnly)
	bool bGameLiftInitialized;
	
private:

	void ParseCommandLinePort(int32& OutPort);

	FProcessParameters ProcessParameters;
};
