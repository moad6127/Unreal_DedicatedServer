// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Lobby/LobbyPlayerBox.h"
#include "Game/DS_GameState.h"
#include "Lobby/LobbyState.h"
#include "Lobby/LobbyPlayerInfo.h"
#include "Components/ScrollBox.h"
#include "UI/Lobby/PlayerLable.h"

void ULobbyPlayerBox::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	ADS_GameState* DSGameState = GetWorld()->GetGameState<ADS_GameState>();
	if (!IsValid(DSGameState))
	{
		return;
	}


	if (IsValid(DSGameState->LobbyState))
	{
		OnLobbyStateInitialized(DSGameState->LobbyState);
	}
	else
	{
		DSGameState->OnLobbyStateInitialized.AddDynamic(this, &ULobbyPlayerBox::OnLobbyStateInitialized);
	}
}

void ULobbyPlayerBox::UpdatePlayerInfo(ALobbyState* LobbyState)
{
	ScrollBox_PlayerInfo->ClearChildren();
	for (const FLobbyPlayerInfo& PlayerInfo : LobbyState->GetPlayers())
	{
		CreateAndAddPlayerLable(PlayerInfo);
	}
}

void ULobbyPlayerBox::OnLobbyStateInitialized(ALobbyState* LobbyState)
{
	if (!IsValid(LobbyState))
	{
		return;
	}
	LobbyState->OnPlayerInfoAdded.AddDynamic(this, &ULobbyPlayerBox::CreateAndAddPlayerLable);
	LobbyState->OnPlayerInfoRemoved.AddDynamic(this, &ULobbyPlayerBox::OnPlayerRemoved);
	UpdatePlayerInfo(LobbyState);
}

void ULobbyPlayerBox::CreateAndAddPlayerLable(const FLobbyPlayerInfo& PlayrInfo)
{
	if (FindPlayerLable(PlayrInfo.Username))
	{
		return;
	}

	UPlayerLable* PlayerLable = CreateWidget<UPlayerLable>(this, PlayerLableClass);
	if (!IsValid(PlayerLable))
	{
		return;
	}

	PlayerLable->SetUsername(PlayrInfo.Username);
	ScrollBox_PlayerInfo->AddChild(PlayerLable);
}

void ULobbyPlayerBox::OnPlayerRemoved(const FLobbyPlayerInfo& PlayerInfo)
{
	if (UPlayerLable* PlayerLable = FindPlayerLable(PlayerInfo.Username))
	{
		ScrollBox_PlayerInfo->RemoveChild(PlayerLable);
	}
}

UPlayerLable* ULobbyPlayerBox::FindPlayerLable(const FString& Username)
{
	for (UWidget* Child : ScrollBox_PlayerInfo->GetAllChildren())
	{
		UPlayerLable* PlayerLable = Cast<UPlayerLable>(Child);
		if (IsValid(PlayerLable) && PlayerLable->GetUsername() == Username)
		{
			return PlayerLable;
		}
	}
	return nullptr;
}
