// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/HTTP/HTTPRequestManager.h"
#include "Interfaces/IHttpRequest.h"
#include "GameStatsManager.generated.h"

/**
 * 
 */
struct FDSRecordMatchStatsInput;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRetrieveMatchStatsResponseReceived, const FDSRetrieveMatchStatsResponse&, RetrieveMatchStatsResponse);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRetrieveLeaderboard, TArray<FDSLeaderboardItem>&, Leaderboard);

UCLASS()
class DEDICATEDSERVERS_API UGameStatsManager : public UHTTPRequestManager
{
	GENERATED_BODY()
public:

	void RecordMatchStats(const FDSRecordMatchStatsInput& RecordMatchStatsInput);
	void RetrieveMatchStats();
	void UpdateLeaderboard(const TArray<FString>& WinnerUsername);
	void RetrieveLeaderboard();

	UPROPERTY(BlueprintAssignable)
	FAPIStatusMessage RetrieveMatchStatsStatusMessage;

	UPROPERTY()
	FAPIStatusMessage RetrieveLeaderboardStatusMessage;
	
	UPROPERTY(BlueprintAssignable)
	FOnRetrieveMatchStatsResponseReceived OnRetrieveMatchStatsResponseReceived;

	FOnRetrieveLeaderboard OnRetrieveLeaderboard;

	UPROPERTY(BlueprintAssignable)
	FOnAPIRequestSucceed OnUpdateLeaderboardSucceeded;
private:
	void RecordMatchStats_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void RetrieveMatchStats_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void UpdateLeaderboard_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void RetrieveLeaderboard_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	
};
