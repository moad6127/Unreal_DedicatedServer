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
	* 토큰 만료시간인 1시간의 75%의 시간동안 타이머 돌리기
	* (IdToken, AccessToken)
	*/
	float TokenRefreshInterval = 2700.f; 
	FTimerHandle RefreshTimer;

};
