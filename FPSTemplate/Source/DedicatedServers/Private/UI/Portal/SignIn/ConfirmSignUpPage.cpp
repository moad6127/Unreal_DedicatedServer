// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Portal/SignIn/ConfirmSignUpPage.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"

void UConfirmSignUpPage::ClearTextBoxes()
{
	TextBox_ConfirmationCode->SetText(FText::GetEmpty());
	TextBlock_StatusMessage->SetText(FText::GetEmpty());
	TextBlock_Destination->SetText(FText::GetEmpty());
}

void UConfirmSignUpPage::UpdateStatusMessage(const FString& Message, bool bShouldResetWidget)
{
	TextBlock_StatusMessage->SetText(FText::FromString(Message));
	if (bShouldResetWidget)
	{
		Button_Confirm->SetIsEnabled(true);
	}
}

void UConfirmSignUpPage::NativeConstruct()
{
	Super::NativeConstruct();

	TextBox_ConfirmationCode->OnTextChanged.AddDynamic(this, &UConfirmSignUpPage::UpdateConfirmButtonState);
	Button_Confirm->SetIsEnabled(false);
}

void UConfirmSignUpPage::UpdateConfirmButtonState(const FText& Text)
{
	const FRegexPattern SixDigitPattern(TEXT(R"(^\d{6}$)"));
	FRegexMatcher Matcher(SixDigitPattern, Text.ToString());

	const bool bValidConfirmationCode = Matcher.FindNext();

	Button_Confirm->SetIsEnabled(bValidConfirmationCode);
	if(bValidConfirmationCode)
	{
		TextBlock_StatusMessage->SetText(FText::FromString(TEXT("")));
	}
	else
	{
		TextBlock_StatusMessage->SetText(FText::FromString(TEXT("Please enter six numerical digits")));
	}
}
