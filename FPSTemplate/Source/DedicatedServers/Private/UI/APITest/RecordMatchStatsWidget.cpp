// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/APITest/RecordMatchStatsWidget.h"
#include "UI/GameStats/GameStatsManager.h"
#include "Components/Button.h"
#include "UI/HTTP/HTTPRequestTypes.h"
#include "Player/DSLocalPlayerSubssytem.h"

void URecordMatchStatsWidget::NativeConstruct()
{
	Super::NativeConstruct();

	GameStatsManager = NewObject<UGameStatsManager>(this, GameStatsManagerClass);

	Button_RecordMatchStats->OnClicked.AddDynamic(this, &URecordMatchStatsWidget::RecordMatchStatsButtonClicked);
}

void URecordMatchStatsWidget::RecordMatchStatsButtonClicked()
{
	check(GameStatsManager);
	FDSRecordMatchStatsInput RecordMatchStatsInput;
	UDSLocalPlayerSubssytem* LocalPlayerSubssytem = GetOwningLocalPlayer()->GetSubsystem<UDSLocalPlayerSubssytem>();
	if (!IsValid(LocalPlayerSubssytem))
	{
		return;
	}
	const FString Username = TEXT("TestUser");
	RecordMatchStatsInput.username = Username;
	RecordMatchStatsInput.matchStats.hits = 1;
	RecordMatchStatsInput.matchStats.headShotElims= 5;


	GameStatsManager->RecordMatchStats(RecordMatchStatsInput);
}
