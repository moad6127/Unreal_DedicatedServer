// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/DS_MatchGameMode.h"

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

void ADS_MatchGameMode::OnCountdownTimerFinished(ECountdownTimerType Type)
{
	Super::OnCountdownTimerFinished(Type);

	if (Type == ECountdownTimerType::PreMatch)
	{
		DS_MatchStatus = EMatchStatus::Match;
		StartCountdownTimer(MatchTimer);
	}
	if (Type == ECountdownTimerType::Match)
	{
		DS_MatchStatus = EMatchStatus::PostMatch;
		StartCountdownTimer(PostMatchTimer);
	}
	if (Type == ECountdownTimerType::PostMatch)
	{
		DS_MatchStatus = EMatchStatus::SeamlessTravelling;
	}
}
