// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "DSTypes.generated.h"


UENUM(BlueprintType)
enum class ECountdownTimerState : uint8
{
	NotStarted UMETA(DisplayName = "Not Started"),
	Started UMETA(DisplayName = "Started"),
	Paused UMETA(DisplayName = "Paused"),
	Stopped UMETA(DisplayName = "Stopped")

};
UENUM(BlueprintType)
enum class ECountdownTimerType : uint8
{
	None UMETA(DisplayName = "None"),
	LobbyCountdown UMETA(DisplayName = "Lobby Countdown"),
	PreMatch UMETA(DisplayName = "Pre Match"),
	Match UMETA(DisplayName = "Match"),
	PostMatch UMETA(DisplayName = "Post Match")
};

UENUM()
enum class EMatchStatus : uint8
{
	WaitingForPlayers,
	PreMatch,
	Match,
	PostMatch,
	SeamlessTravelling
};

USTRUCT(BlueprintType)
struct FCountdownTimerHandle
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	ECountdownTimerState State = ECountdownTimerState::NotStarted;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	ECountdownTimerType Type = ECountdownTimerType::None;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float CountdownTime = 0.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float CountdownUpdateInterval = 1.f;

	FTimerHandle TimerFinishHandle{};
	FTimerHandle TimerUpdateHandle{};

	FTimerDelegate TimerFinishDelegate{};
	FTimerDelegate TimerUpdateDelegate{};
};

inline bool operator==(const FCountdownTimerHandle& lhs, const FCountdownTimerHandle& rhs)
{
	return lhs.Type == rhs.Type;
}