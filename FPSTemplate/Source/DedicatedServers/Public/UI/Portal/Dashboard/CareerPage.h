// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CareerPage.generated.h"

/**
 * 
 */
struct FDSRetrieveMatchStatsResponse;
class UScrollBox;
class UCareerAchievement;
class UTextBlock;

UCLASS()
class DEDICATEDSERVERS_API UCareerPage : public UUserWidget
{
	GENERATED_BODY()
	
	
public:

	UFUNCTION()
	virtual void OnRetrieveMatchStats(const FDSRetrieveMatchStatsResponse& RetrieveMatchStatsResponse);

	UFUNCTION()
	void SetStatusMessage(const FString& Message, bool bShouldResetWidget);

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextBlock_Username;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextBlock_Wins;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextBlock_Losses;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextBlock_StatusMessage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox> ScrollBox_Achievement;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UCareerAchievement> CareerAchievementClass;

protected:

	virtual void NativeConstruct() override;
};
