// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Lobby/PlayerLable.h"
#include "Components/TextBlock.h"

void UPlayerLable::SetUsername(const FString& Username) const
{
	TextBlock_Username->SetText(FText::FromString(Username));
}

FString UPlayerLable::GetUsername() const
{
	return TextBlock_Username->GetText().ToString();
}
