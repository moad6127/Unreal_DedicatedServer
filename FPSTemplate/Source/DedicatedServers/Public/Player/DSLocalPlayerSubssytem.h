// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "UI/HTTP/HTTPRequestTypes.h"
#include "DSLocalPlayerSubssytem.generated.h"

/**
 * 
 */
class IPortalManagement;

UCLASS()
class DEDICATEDSERVERS_API UDSLocalPlayerSubssytem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:
	void InitializeTokens(const FDSAuthenticationResult& AuthResult, TScriptInterface<IPortalManagement> PortalManagement);
	void SetRefreshTokenTimer();
	void UpdateToken(const FString& AccessToken, const FString& IdToken);
private:
	UPROPERTY()
	FDSAuthenticationResult AuthenticationResult;

	UPROPERTY()
	TScriptInterface<IPortalManagement> PortalManagementInterface;
	
	/*
	* ��ū ����ð��� 1�ð��� 75%�� �ð����� Ÿ�̸� ������
	* (IdToken, AccessToken)
	*/
	float TokenRefreshInterval = 2700.f; 
	FTimerHandle RefreshTimer;

};
