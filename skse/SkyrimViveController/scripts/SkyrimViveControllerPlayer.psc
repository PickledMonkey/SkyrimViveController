ScriptName SkyrimViveControllerLoader extends Actor

Race Property WerewolfBeastRace Auto
Race Property VampireLordRace Auto

Event OnRaceSwitchComplete()

	; Check that player is a werewolf

	Race PlayerRace = Game.GetPlayer().GetRace(WerewolfBeastRace)

	If 
		
	endif
endEvent


Event OnRaceSwitchComplete()
  Debug.Trace("This actor's race has finished changing")
  Actor player = Game.GetPlayer()
  Race playerRace = player.GetRace()

  IsWerewolf

endEvent