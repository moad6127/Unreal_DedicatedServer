import { CognitoIdentityProviderClient, GlobalSignOutCommand } from "@aws-sdk/client-cognito-identity-provider"; // ES Modules import

export const handler = async (event) => {
  const cognitoIdentityProviderClient =new CognitoIdentityProviderClient({region : process.env.REGION});
  const input ={
    AccessToken  : event.accessToken
  };
  try{
    const globalSignOutCommand = new GlobalSignOutCommand(input);
    const globalSignOutResponse = await cognitoIdentityProviderClient.send(globalSignOutCommand);
    return globalSignOutResponse;
  }catch(error)
  {
    return error;
  }
};
