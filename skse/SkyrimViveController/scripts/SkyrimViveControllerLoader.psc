ScriptName SkyrimViveControllerLoader extends Quest

import SkyrimViveController
import ModEvent
import Debug
import Game
import Actor
import Utility


;NOTE TO ME! YOU CAN PROBABLY HANDLE WAITING ON THE UPDATE FROM WITHIN YOUR OWN CODE
;JUST MAKE THE ONUPDATE FROM THIS TO BE AN INFINITE WHILE LOOP.
;Essentially, make some function that just waits for a signal from your vive controller and
;returns some kind of return code. That code can then be processed to mean a specific papyrus function


;int viveCancelAttackEventNum = -1

;bool Function registerViveInputEvents()
;	RegisterForModEvent("OnViveStopAnimation", "viveStopAnimation")
;	viveCancelAttackEventNum = ModEvent.Create("viveStopAnimation")
;	setViveCommandEvent(0, viveCancelAttackEventNum)
;
;	Debug.Notification("Events Registered: canelAttack" + viveCancelAttackEventNum)
;	Return True
;EndFunction

bool Function InitVive()
	If (isSkyrimViveControllerRunning() == 0)
		Debug.Notification("Vive initializing")
		InitializeSkyrimViveController()
		Debug.Notification("Vive threads have started")
;		registerViveInputEvents()
		Return True
	EndIf
	Return False
EndFunction

Event OnInit() 
    InitVive()
	RegisterForUpdate(10)
EndEvent

Event OnUpdate()
	; Debug.Notification("Vive updating")
	; This block gets called by the game every 10 seconds...
	InitVive()

EndEvent


;Event OnViveStopAnimation()
;	Debug.Notification("ViveStopAnimationEvent")
;	Debug.SendAnimationEvent(Game.GetPlayer(), "attackStop")
;
;	;Reset controller to send another of this event
;	viveCancelAttackEventNum = ModEvent.Create("viveStopAnimation")
;	setViveCommandEvent(0, viveCancelAttackEventNum)
;EndEvent
