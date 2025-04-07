// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Portal/SignIn/SignUpPage.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
void USignUpPage::UpdateStatusMessage(const FString& Message, bool bShouldResetWidget)
{
	TextBlock_StatusMessage->SetText(FText::FromString(Message));
	if (bShouldResetWidget)
	{
		Button_SignUp->SetIsEnabled(true);
	}
}

void USignUpPage::ClearTextBoxes()
{
	TextBox_UserName->SetText(FText::GetEmpty());
	TextBox_Password->SetText(FText::GetEmpty());
	TextBox_ConfirmPassword->SetText(FText::GetEmpty());
	TextBox_Email->SetText(FText::GetEmpty());
}

void USignUpPage::NativeConstruct()
{
	Super::NativeConstruct();

	TextBox_UserName->OnTextChanged.AddDynamic(this, &USignUpPage::UpdateSignUpButtonState);
	TextBox_Password->OnTextChanged.AddDynamic(this, &USignUpPage::UpdateSignUpButtonState);
	TextBox_ConfirmPassword->OnTextChanged.AddDynamic(this, &USignUpPage::UpdateSignUpButtonState);
	TextBox_Email->OnTextChanged.AddDynamic(this, &USignUpPage::UpdateSignUpButtonState);

	Button_SignUp->SetIsEnabled(false);
}

void USignUpPage::UpdateSignUpButtonState(const FText& Text)
{
	const bool bIsUserNameValid = !TextBox_UserName->GetText().IsEmpty();
	const bool bArePasswordsEqual = TextBox_Password->GetText().ToString()
		== TextBox_ConfirmPassword->GetText().ToString();
	const bool bIsPasswordLongEnough = TextBox_Password->GetText().ToString().Len() >= 8;
	const bool bIsValidEmail = IsValidEmail(TextBox_Email->GetText().ToString());

	FString StatusMessage;
	const bool bIsStrongPassword = IsStrongPassword(TextBox_Password->GetText().ToString(), StatusMessage);
	if (!bIsStrongPassword)
	{
		TextBlock_StatusMessage->SetText(FText::FromString(StatusMessage));
	}
	else if(!bIsUserNameValid)
	{
		TextBlock_StatusMessage->SetText(FText::FromString(TEXT("Please enter a valid username.")));
	}
	else if (!bArePasswordsEqual)
	{
		TextBlock_StatusMessage->SetText(FText::FromString(TEXT("Passwords do not match.")));
	}
	else if (!bIsValidEmail)
	{
		TextBlock_StatusMessage->SetText(FText::FromString(TEXT("Please enter a valid email address.")));
	}
	else if (!bIsPasswordLongEnough)
	{
		TextBlock_StatusMessage->SetText(FText::FromString(TEXT("Password must be at least 8 characters long.")));
	}
	else
	{
		TextBlock_StatusMessage->SetText(FText::GetEmpty());
	}
	Button_SignUp->SetIsEnabled(bIsUserNameValid && bArePasswordsEqual && bIsValidEmail && bIsStrongPassword);
}

bool USignUpPage::IsValidEmail(const FString& Email)
{
	const FRegexPattern EmailPattern(TEXT(R"((^[^\s@]+@[^\s@]+\.[^\s@]{2,}$))"));

	FRegexMatcher Matcher(EmailPattern, Email);
	return Matcher.FindNext();
}

bool USignUpPage::IsStrongPassword(const FString& Password, FString& StatusMessage)
{
	const FRegexPattern NumberPattern(TEXT(R"(\d)")); // 적어도 하나의 숫자 포함
	const FRegexPattern SpecialCharPattern(TEXT(R"([^\w\s])")); // 적어도 하나의 특수 문자 포함
	const FRegexPattern UpperCasePattern(TEXT(R"([A-Z])")); // 적어도 하나의 대문자 포함
	const FRegexPattern LowerCasePattern(TEXT(R"([a-z])")); // 적어도 하나의 소문자 포함

	FRegexMatcher NumberMatcher(NumberPattern, Password);
	FRegexMatcher SpecialCharMatcher(SpecialCharPattern, Password);
	FRegexMatcher UpperCaseMatcher(UpperCasePattern, Password);
	FRegexMatcher LowerCaseMatcher(LowerCasePattern, Password);

	if (!NumberMatcher.FindNext())
	{
		StatusMessage = TEXT("Password must contain at least one number.");
		return false;
	}
	if (!SpecialCharMatcher.FindNext())
	{
		StatusMessage = TEXT("Password must contain at least one special character.");
		return false;
	}
	if (!UpperCaseMatcher.FindNext())
	{
		StatusMessage = TEXT("Password must contain at least one uppercase letter.");
		return false;
	}
	if (!LowerCaseMatcher.FindNext())
	{
		StatusMessage = TEXT("Password must contain at least one lowercase letter.");
		return false;
	}
	return true;
}
