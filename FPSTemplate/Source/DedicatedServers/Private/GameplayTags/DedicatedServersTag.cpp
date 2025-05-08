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
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(SignOut, "DedicatedServersTag.Portal.SignOut", "Signs user out and invalidateds tokens in Portal API")

	}
	namespace GameStatsAPI
	{
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(RecordMatchStats, "DedicatedServersTag.GameStatsAPI.RecordMatchStats", "Records the stats of a match in Game Stats API")
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(RetrieveMatchStats, "DedicatedServersTag.GameStatsAPI.RetrieveMatchStats", "Get the match stats from Plyers table in Game Stats API")

	}
}