// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/API/APIData.h"

FString UAPIData::GetAPIEndPoint(const FGameplayTag& APIEndPoint)
{
    const FString ResourceName = Resources.FindChecked(APIEndPoint);

	return InvokeURL + "/" + Stage + "/" + ResourceName;
}
