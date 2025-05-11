// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DashboardOverlay.generated.h"

/**
 * 
 */
class UWidgetSwitcher;
class UGamePage;
class UCareerPage;
class ULeaderboardPage;
class UButton;
class UGameStatsManager;


UCLASS()
class DEDICATEDSERVERS_API UDashboardOverlay : public UUserWidget
{
	GENERATED_BODY()
	
public:

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidgetSwitcher> WidgetSwitcher;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UGamePage> GamePage;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCareerPage> CareerPage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<ULeaderboardPage> LeaderboardPage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Game;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Career;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Leaderboard;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameStatsManager> GameStatsManagerClass;

protected:
	virtual void NativeConstruct() override;

private:
	UFUNCTION()
	void ShowGamePage();

	UFUNCTION()
	void ShowCarrerPage();

	UFUNCTION()
	void ShowLeaderboardPage();

	void DisableButton(UButton* Button) const;

	UPROPERTY()
	TObjectPtr<UGameStatsManager> GameStatsManager;
};
