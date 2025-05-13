import { CognitoIdentityProviderClient, InitiateAuthCommand, GetUserCommand } from "@aws-sdk/client-cognito-identity-provider"; // ES Modules import

export const handler = async (event) => {

  const cognitoIdentityProviderClient = new CognitoIdentityProviderClient({region : process.env.REGION});

  const { username, password,refreshToken } = event;
  if(refreshToken){
    const refreshTokensInput = {
      AuthFlow: "REFRESH_TOKEN_AUTH",
      ClientId: process.env.CLIENT_ID,
      AuthParameters: {
        REFRESH_TOKEN: refreshToken
      }
    };
    const initiateAuthCommand = new InitiateAuthCommand(refreshTokensInput);
    try{
      const initiateAuthResponse = await cognitoIdentityProviderClient.send(initiateAuthCommand);
      return initiateAuthResponse;
    }
    catch(error)
    {
      return error;
    }
    
  }else{
    const initateAutInput = {
      AuthFlow: "USER_PASSWORD_AUTH",
      ClientId: process.env.CLIENT_ID,
      AuthParameters: {
        USERNAME: username,
        PASSWORD: password
      }
    };
  
    const initiateAuthCommand = new InitiateAuthCommand(initateAutInput);
  
    try{
      const initiateAuthResponse = await cognitoIdentityProviderClient.send(initiateAuthCommand);

      const getUserInput = {
        AccessToken: initiateAuthResponse.AuthenticationResult.AccessToken
      };
      const getUserCommand = new GetUserCommand(getUserInput);
      const getUserResponse = await cognitoIdentityProviderClient.send(getUserCommand);

      let emailAtrribute;
      for(const attribute of getUserResponse.UserAttributes){
        if(attribute.Name === "email"){
          emailAtrribute = attribute.Value;
          break;
        }
      }
      const response = {
        ...initiateAuthResponse,
        email: emailAtrribute
      };

      return response;
    }
    catch(error)
    {
      return error;
    }
  }


};
