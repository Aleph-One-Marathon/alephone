-- Cheats.lua
-- 
-- To use this script, select it as the solo script in environment
-- preferences
--
-- To cheat, type the console key, then the function in the left
-- column, then enter. Press the microphone button to jump.
--					    	  
-- nrg()		Energy (1x)
-- otwo()		Oxygen
-- bye()		Invisible
-- nuke()		Invincible
-- see()		Infravision
-- wow()		Extravision
-- mag()		Pistol
-- rif()		Assault rifle
-- pow()		Rocket launcher
-- toast()		Flamethrower
-- melt()		Fusion gun
-- puff()		Shotgun
-- zip()		SMG
-- pzbxay()		Alien weapon
-- ammo()		All weapons' ammo
-- qwe()		Jump
-- shit()		Everything (almost)
-- yourmom()		Save at this spot

function nrg()
   if Players[0].life < 150 then 
      Players[0].life = 150
   elseif Players[0].life < 300 then
      Players[0].life = 300
   elseif Players[0].life < 450 then
      Players[0].life = 450
   end
end

function otwo()
   Players[0].oxygen = 10800
end

function bye()
   Players[0].items["invisibility"] = 1
end

function nuke()
   Players[0].items["invincibility"] = 1
end

function wow()
   Players[0].items["extravision"] = 1
end

function mag()
   local items = Players[0].items
   items["pistol"] = items["pistol"] + 1
   items["pistol ammo"] = items["pistol ammo"] + 10
end

function rif()
   local items = Players[0].items
   items["assault rifle"] = items["assault rifle"] + 1
   items["assault rifle ammo"] = items["assault rifle ammo"] + 10
   items["assault rifle grenades"] = items["assault rifle grenades"] + 10
end

function pow()
   local items = Players[0].items
   items["missile launcher"] = items["missile launcher"] + 1
   items["missile launcher ammo"] = items["missile launcher ammo"] + 10
end

function toast()
   local items = Players[0].items
   items["flamethrower"] = items["flamethrower"] + 1
   items["flamethrower ammo"] = items["flamethrower ammo"] + 10
end

function melt()
   local items = Players[0].items
   items["fusion pistol"] = items["fusion pistol"] + 1
   items["fusion pistol ammo"] = items["fusion pistol ammo"] + 10
end

function puff()
   local items = Players[0].items
   items["shotgun"] = items["shotgun"] + 1
   items["shotgun ammo"] = items["shotgun ammo"] + 10
end

function zip()
   local items = Players[0].items
   items["smg"] = items["smg"] + 1
   items["smg ammo"] = items["smg ammo"] + 10
end

function pzbxay()
   local items = Players[0].items
   items["alien weapon"] = items["alien weapon"] + 1
end

function yourmom()
   Game.save()
end

function ammo()
   local items = { "pistol ammo", "fusion pistol ammo", "assault rifle ammo", "assault rifle grenades", "missile launcher ammo", "alien weapon ammo", "flamethrower ammo", "shotgun ammo", "smg ammo" }
   for _, item in pairs(items) do
      Players[0].items[item] = Players[0].items[item] + 10
   end
end

function shit()
   ammo()
   local weapons = { "alien weapon", "pistol", "fusion pistol", "assault rifle", "missile launcher", "flamethrower", "shotgun", "shotgun", "smg" }
   for _, weapon in pairs(weapons) do
      Players[0].items[weapon] = Players[0].items[weapon] + 1
   end

   if Players[0].life < 450 then 
      Players[0].life = 450
   end
end

function qwe()
   Players[0]:accelerate(0, 0, 0.1)
end

Triggers = {}

function Triggers.idle()

   if Game.ticks == 0 then
      Players.print("Cheats enabled")
   end

   -- handle jumping
   for p in Players() do
      if p.action_flags.microphone_button then
	 if not p._latched then
	    p._latched = true
	    p:accelerate(0, 0, 0.1)
	 end
	 p.action_flags.microphone_button = false
      else
	 p._latched = false
      end
   end

end
