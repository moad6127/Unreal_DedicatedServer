// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/DS_GameModeBase.h"
#include "Player/DSPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameLiftServerSDK.h"

void ADSGameModeBase::StartCountdownTimer(FCountdownTimerHandle& CountdownTimerHandle)
{
	CountdownTimerHandle.TimerFinishDelegate.BindWeakLambda(this, [&]()
		{
			OnCountdownTimerFinished(CountdownTimerHandle.Type);
		});

	GetWorldTimerManager().SetTimer(
		CountdownTimerHandle.TimerFinishHandle,
		CountdownTimerHandle.TimerFinishDelegate,
		CountdownTimerHandle.CountdownTime,
		false);

	CountdownTimerHandle.TimerUpdateDelegate.BindWeakLambda(this, [&]()
		{
			UpdateCountdownTimer(CountdownTimerHandle);
		});
	GetWorldTimerManager().SetTimer(
		CountdownTimerHandle.TimerUpdateHandle,
		CountdownTimerHandle.TimerUpdateDelegate,
		CountdownTimerHandle.CountdownUpdateInterval,
		true);
	UpdateCountdownTimer(CountdownTimerHandle);
}

void ADSGameModeBase::StopCountdownTimer(FCountdownTimerHandle& CountdownTimerHandle)
{
	CountdownTimerHandle.State = ECountdownTimerState::Stopped;
	GetWorldTimerManager().ClearTimer(CountdownTimerHandle.TimerFinishHandle);
	GetWorldTimerManager().ClearTimer(CountdownTimerHandle.TimerUpdateHandle);
	if (CountdownTimerHandle.TimerFinishDelegate.IsBound())
	{
		CountdownTimerHandle.TimerFinishDelegate.Unbind();
	}
	if (CountdownTimerHandle.TimerUpdateDelegate.IsBound())
	{
		CountdownTimerHandle.TimerUpdateDelegate.Unbind();
	}

	for (FConstPlayerControllerIterator iterator = GetWorld()->GetPlayerControllerIterator(); iterator; ++iterator)
	{
		ADSPlayerController* DSPlayerController = Cast<ADSPlayerController>(iterator->Get());
		if (IsValid(DSPlayerController))
		{
			DSPlayerController->Client_TimerStopped(0.f,CountdownTimerHandle.Type);
		}
	}

}

void ADSGameModeBase::UpdateCountdownTimer(const FCountdownTimerHandle& CountdownTimerHandle)
{
	for (FConstPlayerControllerIterator iterator = GetWorld()->GetPlayerControllerIterator(); iterator; ++iterator)
	{
		ADSPlayerController* DSPlayerController = Cast<ADSPlayerController>(iterator->Get());
		if (IsValid(DSPlayerController))
		{
			const float CountdownTimeLeft =
				CountdownTimerHandle.CountdownTime
				- GetWorldTimerManager().GetTimerElapsed(CountdownTimerHandle.TimerFinishHandle);
			DSPlayerController->Client_TimerUpdated(CountdownTimeLeft, CountdownTimerHandle.Type);
		}
	}
}

void ADSGameModeBase::OnCountdownTimerFinished(ECountdownTimerType Type)
{
}

void ADSGameModeBase::TrySeamlessTravel(TSoftObjectPtr<UWorld> DestinationMap)
{
	const FString MapName = DestinationMap.ToSoftObjectPath().GetAssetName();
	if (GIsEditor)
	{
		UGameplayStatics::OpenLevelBySoftObjectPtr(this, DestinationMap);
	}
	else
	{
		GetWorld()->ServerTravel(MapName);
	}
}

void ADSGameModeBase::RemovePlayerSession(AController* Exiting)
{
	ADSPlayerController* DSPlayerController = Cast<ADSPlayerController>(Exiting);
	if (!IsValid(DSPlayerController))
	{
		return;
	}
	//
#if WITH_GAMELIFT
	const FString& PlayerSessionID = DSPlayerController->PlayerSessionId;
	if (!PlayerSessionID.IsEmpty())
	{
		Aws::GameLift::Server::RemovePlayerSession(TCHAR_TO_ANSI(*PlayerSessionID));
	}
	//
#endif
}
