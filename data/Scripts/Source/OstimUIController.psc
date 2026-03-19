ScriptName OStim_UIController extends Quest

; UI MENU NAMES

String Property OStimBarMenu = "OStimBar" Auto

; STATE

Bool Property IsBarOpen = False Auto

; MENU CONTROL

Function OpenBar()
If (!IsBarOpen)
UI.OpenCustomMenu(OStimBarMenu)
IsBarOpen = True
EndIf
EndFunction

Function CloseBar()
If (IsBarOpen)
UI.CloseCustomMenu(OStimBarMenu)
IsBarOpen = False
EndIf
EndFunction

; BAR FUNCTIONS (REPLACES SKYUI CALLS)

Function SetBarPercent(Float Value)
If (IsBarOpen)
UI.InvokeFloat(OStimBarMenu, "_root.bar._xscale", Value * 100.0)
EndIf
EndFunction

Function SetBarVisible(Bool Visible)
If (IsBarOpen)
UI.InvokeBool(OStimBarMenu, "_root.bar._visible", Visible)
EndIf
EndFunction

Function SetBarColors(Int Primary, Int Secondary = -1, Int Flash = -1)
If (IsBarOpen)
Int[] Args = new Int[3]
Args[0] = Primary
Args[1] = Secondary
Args[2] = Flash
UI.InvokeIntA(OStimBarMenu, "_root.setColors", Args)
EndIf
EndFunction
