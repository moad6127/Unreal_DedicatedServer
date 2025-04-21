#include "UI/Portal/SignIn/AccountDropdown_Expanded.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "UI/Portal/PortalManager.h"
#include "Player/DSLocalPlayerSubssytem.h"

void UAccountDropdown_Expanded::NativeConstruct()
{
	Super::NativeConstruct();

	check(PortalManagerClass);
	PortalManager = NewObject<UPortalManager>(this, PortalManagerClass);

	Button_SignOut->OnHovered.AddDynamic(this, &UAccountDropdown_Expanded::SignOutButton_Hover);
	Button_SignOut->OnUnhovered.AddDynamic(this, &UAccountDropdown_Expanded::SignOutButton_UnHover);
	Button_SignOut->OnClicked.AddDynamic(this, &UAccountDropdown_Expanded::SignOutButton_OnClicked);
}

void UAccountDropdown_Expanded::NativePreConstruct()
{
	Super::NativePreConstruct();

	SetSignOutButtonStyleTransparent();
	SignOutButton_UnHover();
	UDSLocalPlayerSubssytem* LocalPlayerSubSystem = GetLocalPlayerSubSystem();
	if (IsValid(LocalPlayerSubSystem))
	{
		TextBlock_Email->SetText(FText::FromString(LocalPlayerSubSystem->Email));
	}

}

void UAccountDropdown_Expanded::SignOutButton_Hover()
{
	TextBlock_SignOutButtonText->SetColorAndOpacity(HoveredTextColor);
}

void UAccountDropdown_Expanded::SignOutButton_UnHover()
{
	TextBlock_SignOutButtonText->SetColorAndOpacity(UnHoveredTextColor);
}

void UAccountDropdown_Expanded::SignOutButton_OnClicked()
{
	Button_SignOut->SetIsEnabled(false);

	check(PortalManager);

	UDSLocalPlayerSubssytem* LocalPlayerSubSystem = GetLocalPlayerSubSystem();
	if (IsValid(LocalPlayerSubSystem))
	{
		FDSAuthenticationResult AuthResult = LocalPlayerSubSystem->GetAuthResult();
		PortalManager->SignOut(AuthResult.AccessToken);
	}
}

void UAccountDropdown_Expanded::SetSignOutButtonStyleTransparent()
{
	FButtonStyle Style;
	FSlateBrush Brush;
	Brush.TintColor = FSlateColor(FLinearColor(0.f, 0.f, 0.f, 0.f));
	Style.Disabled = Brush;
	Style.Hovered = Brush;
	Style.Normal = Brush;
	Style.Pressed = Brush;
	Button_SignOut->SetStyle(Style);
}

UDSLocalPlayerSubssytem* UAccountDropdown_Expanded::GetLocalPlayerSubSystem()
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
