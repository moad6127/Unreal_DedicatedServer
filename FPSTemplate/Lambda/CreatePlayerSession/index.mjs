import { GameLiftClient, CreatePlayerSessionCommand } from "@aws-sdk/client-gamelift"; // ES Modules import

export const handler = async (event) => {


  try{
    const gameLiftClient = new GameLiftClient( {region : process.env.REGION } );
    const createPlayerSessionInput = { 
      GameSessionId: event.gameSessionId, 
      PlayerId: event.playerId,
      Location: "custom-home-desktop" //remove this for EC2 fleets
    };

    const createPlayerSessionCommand = new CreatePlayerSessionCommand(createPlayerSessionInput);
    const createPlayerSessionresponse = await gameLiftClient.send(createPlayerSessionCommand);

    return createPlayerSessionresponse.PlayerSession;

  }catch(error){
    return error;
  }
};
