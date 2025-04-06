#include "GameplayTags/DedicatedServersTag.h"

namespace DedicatedServersTag
{
	namespace GameSessionsAPI
	{
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(ListFleets, "DedicatedServersTag.GameSessionsAPI.ListFleets", "List Fleets resource on the GameSessions API")
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(FindOrCreateGameSession, "DedicatedServersTag.GameSessionsAPI.FindOrCreateGameSession", "Retrieves an ACTIVE game session, creating one if one doesn't exist on the GameSessions API")
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(CreatePlayerSession, "DedicatedServersTag.GameSessionsAPI.CreatePlayerSession", "Create Player Session on the GameSessions API")

	}
	namespace Portal
	{
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(SignUp, "DedicatedServersTag.Portal.SignUp", "Create a new User in Portal API")
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(ConfirmSignUp, "DedicatedServersTag.Portal.ConfirmSignUp", "Confrim a new User in Portal API")
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(SignIn, "DedicatedServersTag.Portal.SignIn", "Retrieves Access Token, ID Token, and Refresh Token in Portal API")

	}
}