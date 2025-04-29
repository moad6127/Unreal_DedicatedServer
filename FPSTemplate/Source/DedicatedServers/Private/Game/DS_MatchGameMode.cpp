// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/DS_MatchGameMode.h"
#include "Player/DSPlayerController.h"

ADS_MatchGameMode::ADS_MatchGameMode()
{
	bUseSeamlessTravel = true;
	DS_MatchStatus = EMatchStatus::WaitingForPlayers;
	PreMatchTimer.Type = ECountdownTimerType::PreMatch;
	MatchTimer.Type = ECountdownTimerType::Match;
	PostMatchTimer.Type = ECountdownTimerType::PostMatch;
}

void ADS_MatchGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (DS_MatchStatus == EMatchStatus::WaitingForPlayers)
	{
		DS_MatchStatus = EMatchStatus::PreMatch;
		StartCountdownTimer(PreMatchTimer);
	}
}

void ADS_MatchGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	RemovePlayerSession(Exiting);
}

void ADS_MatchGameMode::OnCountdownTimerFinished(ECountdownTimerType Type)
{
	Super::OnCountdownTimerFinished(Type);

	if (Type == ECountdownTimerType::PreMatch)
	{
		DS_MatchStatus = EMatchStatus::Match;
		StartCountdownTimer(MatchTimer);
		SetClientInuptEnabled(true);
	}
	if (Type == ECountdownTimerType::Match)
	{
		DS_MatchStatus = EMatchStatus::PostMatch;
		StartCountdownTimer(PostMatchTimer);
		SetClientInuptEnabled(false);
	}
	if (Type == ECountdownTimerType::PostMatch)
	{
		DS_MatchStatus = EMatchStatus::SeamlessTravelling;
		TrySeamlessTravel(LobbyMap);
	}
}

void ADS_MatchGameMode::SetClientInuptEnabled(bool bEnabled)
{
	for (FConstPlayerControllerIterator iterator = GetWorld()->GetPlayerControllerIterator(); iterator; ++iterator)
	{
		ADSPlayerController* DSPlayerController = Cast<ADSPlayerController>(iterator->Get());
		if (IsValid(DSPlayerController))
		{
			DSPlayerController->Client_SetInputEnabled(bEnabled);
		}
	}
}
