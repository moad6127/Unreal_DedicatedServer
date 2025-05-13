import { GameLiftClient, ListFleetsCommand } from "@aws-sdk/client-gamelift";

export const handler = async (event) => {

  const client = new GameLiftClient( { region : process.env.REGION } );

  try {
    const input = { };
    const command = new ListFleetsCommand(input);
    const response = await client.send(command);
  
    return response;
  }catch(error){
    return error;
  }


};
