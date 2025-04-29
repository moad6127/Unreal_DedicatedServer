// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Types/DSTypes.h"
#include "DSPlayerController.generated.h"

/**
 * 
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTimerStateChagnedDelegate, float, Time, ECountdownTimerType, Type);

UCLASS()
class DEDICATEDSERVERS_API ADSPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:

	ADSPlayerController();
	virtual void ReceivedPlayer() override;

	virtual void OnRep_PlayerState() override;

	virtual void PostSeamlessTravel() override;

	virtual void BeginPlay() override;


	UFUNCTION(Client, Reliable)
	void Client_TimerUpdated(float CountdownTimeLeft, ECountdownTimerType Type) const;

	UFUNCTION(Client, Reliable)
	void Client_TimerStopped(float CountdownTimeLeft, ECountdownTimerType Type) const;

	UFUNCTION(Client, Reliable)
	void Client_SetInputEnabled(bool bEnabled);

	UPROPERTY(BlueprintAssignable)
	FOnTimerStateChagnedDelegate OnTimerUpdated;

	UPROPERTY(BlueprintAssignable)
	FOnTimerStateChagnedDelegate OnTimerStopped;

	UPROPERTY(BlueprintReadOnly)
	FString Username;

	UPROPERTY(BlueprintReadOnly)
	FString PlayerSessionId;

	/*	
	UPROPERTY(BlueprintAssignable)
	FOnTimerStateChagnedDelegate OnTimerPaused;

	UPROPERTY(BlueprintAssignable)
	FOnTimerStateChagnedDelegate OnTimerResumed;
	*/

protected:
	
	UFUNCTION(Server,Reliable)
	void Server_Ping(float TimeOfRequest);
	
	UFUNCTION(Client, Reliable)
	void Clinet_Pong(float TimeOfRequest);
private:

	float SingleTripTime;
	
};
