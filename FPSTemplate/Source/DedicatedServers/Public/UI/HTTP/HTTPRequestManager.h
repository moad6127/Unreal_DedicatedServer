// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "HTTPRequestManager.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAPIStatusMessage, const FString&, StatusMessage, bool, bShouldResetWidgets);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAPIRequestSucceed);
/**
 * 
 */
class UAPIData;
class FJsonObject;

UCLASS(Blueprintable)
class DEDICATEDSERVERS_API UHTTPRequestManager : public UObject
{
	GENERATED_BODY()	
protected:

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UAPIData> APIData;

	bool ContainsErrors(TSharedPtr<FJsonObject> JsonObject);

	void DumpMetaData(TSharedPtr<FJsonObject> JsonObject);

	FString SerializeJsonContent(const TMap<FString, FString>& Params);
};
