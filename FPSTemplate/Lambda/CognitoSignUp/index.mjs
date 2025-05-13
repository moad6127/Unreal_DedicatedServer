import { CognitoIdentityProviderClient, SignUpCommand } from "@aws-sdk/client-cognito-identity-provider"; // ES Modules import

export const handler = async (event) => {
  
  const cognitoIdentityProviderClient = new CognitoIdentityProviderClient({region : process.env.REGION});

  const clientId = process.env.CLIENT_ID;

  const{ username,password,email }= event;

  const signUpInput = {
    ClientId: clientId,
    Username: username,
    Password: password,
    UserAttributes: [
      {
        Name: "email",
        Value: email
      }
    ]
  };

  try{
    const signUpCommand = new SignUpCommand(signUpInput);
    const signUpResponse = await cognitoIdentityProviderClient.send(signUpCommand);
  
    return signUpResponse;
  }catch(error)
  {
    return error;
  }

};
