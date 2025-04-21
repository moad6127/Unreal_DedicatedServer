#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AccountDropdown_Expanded.generated.h"

/**
 * 
 */
class UButton;
class UTextBlock;
class UPortalManager;
class UDSLocalPlayerSubssytem;

UCLASS()
class DEDICATEDSERVERS_API UAccountDropdown_Expanded : public UUserWidget
{
	GENERATED_BODY()
	
public:

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_SignOut;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextBlock_SignOutButtonText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextBlock_Email;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FSlateColor HoveredTextColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FSlateColor UnHoveredTextColor;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UPortalManager> PortalManagerClass;
protected:

	virtual void NativeConstruct() override;
	virtual void NativePreConstruct() override;

	UFUNCTION()
	void SignOutButton_Hover();

	UFUNCTION()
	void SignOutButton_UnHover();

	UFUNCTION()
	void SignOutButton_OnClicked();

private:
	UPROPERTY()
	TObjectPtr<UPortalManager> PortalManager;

	void SetSignOutButtonStyleTransparent();

	UDSLocalPlayerSubssytem* GetLocalPlayerSubSystem();
};
