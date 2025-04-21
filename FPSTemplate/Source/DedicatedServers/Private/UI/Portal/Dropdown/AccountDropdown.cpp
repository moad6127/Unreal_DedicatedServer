// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Portal/Dropdown/AccountDropdown.h"
#include "Player/DSLocalPlayerSubssytem.h"
#include "Components/TextBlock.h"

void UAccountDropdown::NativeConstruct()
{
	Super::NativeConstruct();
	UDSLocalPlayerSubssytem* LocalPlayerSubSystem = GetLocalPlayerSubSystem();
	if (IsValid(LocalPlayerSubSystem))
	{
		TextBlock_ButtonText->SetText(FText::FromString(LocalPlayerSubSystem->UserName));
	}
}

UDSLocalPlayerSubssytem* UAccountDropdown::GetLocalPlayerSubSystem()
{
	APlayerController* PlayerController = GetOwningPlayer();
	if (IsValid(PlayerController) && IsValid(PlayerController->GetLocalPlayer()))
	{
		UDSLocalPlayerSubssytem* LocalPlayerSubSystem = PlayerController->GetLocalPlayer()->GetSubsystem<UDSLocalPlayerSubssytem>();
		if (IsValid(LocalPlayerSubSystem))
		{
			return LocalPlayerSubSystem;
		}
	}
	return nullptr;
}
