// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/APITest/APITestOverlay.h"
#include "UI/API/ListFleets/ListFleetsBox.h"
#include "UI/APITest/APITestManager.h"
#include "Components/Button.h"

void UAPITestOverlay::NativeConstruct()
{
	Super::NativeConstruct();

	check(APITestManagerClass);

	APITestManager = NewObject<UAPITestManager>(this, APITestManagerClass);

	check(ListFleetsBox);
	check(ListFleetsBox->Button_ListFllets);
	ListFleetsBox->Button_ListFllets->OnClicked.AddDynamic(APITestManager, &UAPITestManager::ListFleetsButtonClicked);
}
