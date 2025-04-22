// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Portal/Dashboard/GamePage.h"
#include "UI/API/GameSessions/JoinGame.h"
#include "UI/GameSessions/GameSessionsManager.h"
#include "Components/Button.h"



void UGamePage::NativeConstruct()
{
	Super::NativeConstruct();

	GameSessionsManager = NewObject<UGameSessionsManager>(this, GameSessionsManagerClass);

	GameSessionsManager->BroadcastJoinGameSessionMessage.AddDynamic(JoinGameWidet, &UJoinGame::SetStatusMessage);
	JoinGameWidet->Button_JoinGame->OnClicked.AddDynamic(this, &UGamePage::JoinGameButtonClicked);
}

void UGamePage::JoinGameButtonClicked()
{
	JoinGameWidet->Button_JoinGame->SetIsEnabled(false);
	GameSessionsManager->JoinGameSession();
}
