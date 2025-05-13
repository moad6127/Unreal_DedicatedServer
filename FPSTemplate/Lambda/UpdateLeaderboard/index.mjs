import { CognitoIdentityProviderClient, AdminGetUserCommand } from "@aws-sdk/client-cognito-identity-provider"; // ES Modules import

const cognitoClient = new CognitoIdentityProviderClient({ region: process.env.REGION  });

export const handler = async (event) => {
  
  // array of username from the match
  const playerIds = event.playerIds;
  const userPoolId = process.env.USER_POOL_ID;

  try{
  // retrieve player data (cognito sub ids)
  const playerData = await retrievePlayerData(playerIds, userPoolId)
  // retrieve player current wins
  
  // add winers leaderboard

  // ensure only top 20 players
  }catch(error){

    return error;
  }

};

async function retrievePlayerData(playerIds, userPoolId){
  return await Promise.all(playerIds.map(async (playerId) => {
    const adminGetUserCommand  =new AdminGetUserCommand({
      Username: playerId,
      UserPoolId: userPoolId,
    });
    const adminGetUserResponse = await cognitoClient.send(adminGetUserCommand);
    const databaseid = adminGetUserResponse.UserAttributes.find(attr => attr.Name ==="sub").Value;
    return {playerId, databaseid};
  }));

}
