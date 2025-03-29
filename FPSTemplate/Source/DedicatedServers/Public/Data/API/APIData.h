// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTags/DedicatedServersTag.h"
#include "APIData.generated.h"

/**
 * 
 */
UCLASS()
class DEDICATEDSERVERS_API UAPIData : public UDataAsset
{
	GENERATED_BODY()
public:
	FString GetAPIEndPoint(const FGameplayTag& APIEndPoint);

protected:

	// name of this api - for labeling in the data asset; this is not used by any code
	UPROPERTY(EditDefaultsOnly)
	FString Name;
	
	UPROPERTY(EditDefaultsOnly)
	FString InvokeURL;

	UPROPERTY(EditDefaultsOnly)
	FString Stage;

	UPROPERTY(EditDefaultsOnly)
	TMap<FGameplayTag, FString> Resources;
};
