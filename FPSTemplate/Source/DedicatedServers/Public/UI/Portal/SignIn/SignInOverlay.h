// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SignInOverlay.generated.h"

/**
 * 
 */
class UJoinGame;
class UPortalManager;
class UWidgetSwitcher;
class USignInPage;
class USignUpPage;
class UConfirmSignUpPage;
class USuccessConfirmedPage;
class UButton;

UCLASS()
class DEDICATEDSERVERS_API USignInOverlay : public UUserWidget
{
	GENERATED_BODY()
	
public:

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UJoinGame>JoinGameWidget;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UPortalManager> PortalManagerClass;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidgetSwitcher> WidgetSwitcher;

protected:
	virtual void NativeConstruct() override;

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USignInPage> SignInPage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USignUpPage> SignUpPage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UConfirmSignUpPage> ConfirmSignUpPage;	

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USuccessConfirmedPage> SuccessConfirmedPage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_SignIn_Test;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_SignUp_Test;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Confirm_Test;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_SuccessConfirmed_Test;


	UPROPERTY()
	TObjectPtr<UPortalManager> PortalManager;
	
	UFUNCTION()
	void OnJoinGameButtonClicked();

	UFUNCTION()
	void UpdateJoinGameStatusMessage(const FString& StatusMessage,bool bResetJoinGameButton);

	UFUNCTION()
	void ShowSignInPage();

	UFUNCTION()
	void ShowSignUpPage();

	UFUNCTION()
	void ShowConfirmPage();

	UFUNCTION()
	void ShowSuccessConfirmedPage();

	UFUNCTION()
	void SignInButtonClicked();

	UFUNCTION()
	void SignUpButtonClicked();

	UFUNCTION()
	void ConfrimButtonClicked();
};
