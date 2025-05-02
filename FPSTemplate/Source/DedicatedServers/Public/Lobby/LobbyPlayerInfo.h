// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Net/Serialization/FastArraySerializer.h"
#include "LobbyPlayerInfo.generated.h"

USTRUCT(BlueprintType)
struct FLobbyPlayerInfo : public FFastArraySerializerItem
{
	GENERATED_BODY()

	FLobbyPlayerInfo() {}
	FLobbyPlayerInfo(const FString& Name) : Username(Name) {}

	UPROPERTY(BlueprintReadWrite)
	FString Username{};
};

USTRUCT()
struct FLobbyPlayerInfoArray : public FFastArraySerializer
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FLobbyPlayerInfo> Players;

	// This is called when the array is serialized
	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FastArrayDeltaSerialize<FLobbyPlayerInfo, FLobbyPlayerInfoArray>(Players, DeltaParms, *this);
	}

	void AddPlayer(const FLobbyPlayerInfo& NewPlayerInfo);
	void RemovePlayer(const FString& Username);
};

template<>
struct TStructOpsTypeTraits<FLobbyPlayerInfoArray> : public TStructOpsTypeTraitsBase2<FLobbyPlayerInfoArray>
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};

