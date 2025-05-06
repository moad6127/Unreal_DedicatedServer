
#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"

namespace DedicatedServersTag
{
	namespace GameSessionsAPI
	{
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(ListFleets);
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(FindOrCreateGameSession);
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(CreatePlayerSession);
	}
	namespace Portal
	{
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(SignUp);
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(ConfirmSignUp);
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(SignIn);
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(SignOut);
	}
	namespace GameStatsAPI
	{
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(RecordMatchStats);
	}
}
