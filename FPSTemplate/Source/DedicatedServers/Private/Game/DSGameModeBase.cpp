// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/DSGameModeBase.h"
#include "Player/DSPlayerController.h"

void ADSGameModeBase::StartCountdownTimer(FCountdownTimerHandle& CountdownTimerHandle)
{
	CountdownTimerHandle.TimerFinishDelegate.BindWeakLambda(this, [&]()
		{
			StopCountdownTimer(CountdownTimerHandle);
			OnCountdownTimerFinished(CountdownTimerHandle.Type);
		});

	GetWorldTimerManager().SetTimer(
		CountdownTimerHandle.TimerFinishHandle,
		CountdownTimerHandle.TimerFinishDelegate,
		CountdownTimerHandle.CountdownTime,
		false);

	CountdownTimerHandle.TimerUpdateDelegate.BindWeakLambda(this, [&]()
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
		});
	GetWorldTimerManager().SetTimer(
		CountdownTimerHandle.TimerUpdateHandle,
		CountdownTimerHandle.TimerUpdateDelegate,
		CountdownTimerHandle.CountdownUpdateInterval,
		true);
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

void ADSGameModeBase::OnCountdownTimerFinished(ECountdownTimerType Type)
{

}
