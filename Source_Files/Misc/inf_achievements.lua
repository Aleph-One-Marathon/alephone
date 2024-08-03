R"LUA(
Triggers = {}

function got_achievement(achievement)
   set_achievement(achievement)
end

function Triggers.init(restored)
   if Game.replay then
      Triggers = {}
      return
   end

   if Game.ticks == 0 then
      Game._initial_level = Level.index
      Game._min_difficulty = Game.difficulty
   elseif restored then
      if not Game.restore_saved() or
         Game._initial_level == nil or
         Game._min_difficulty == nil
      then
         Players.print("Achievements disabled (missing saved game data)")
         Triggers = {}
         return
      end
   else
      if not Game.restore_passed() then
         Players.print("Achievements disabled (level transfer failure)")
         Triggers = {}
         return
      end
   end

   if Game.difficulty.index < Game._min_difficulty.index then
      Game._min_difficulty = Game.difficulty
   end

   if Level.index == 1 and Game._initial_level == 0 then
      Triggers.terminal_enter = rrr_terminal_enter
   end

   if Level.index == 2 then
      Triggers.got_item = py_got_item
   end

   if Level.index == 5 then
      Triggers.monster_killed = wamid_monster_killed
   end

   if Level.index == 6 and Game._initial_level == 0 then
      Triggers.projectile_switch = as_projectile_switch
   end

   if Level.index == 9 then
      Triggers.platform_activated = twk_platform_activated
      if Players[0]._doors_cycled == nil then
         Players[0]._doors_cycled = {}
      end
   else
      Players[0]._doors_cycled = nil
   end

   if Level.index == 10 then
      Triggers.idle = es2_idle
   end

   if Level.index == 12 and Game._initial_level == 0 then
      Triggers.platform_activated = nmhc_platform_activated
   end

   if Level.index == 16 then
      Triggers.terminal_enter = etp_terminal_enter
      if Players[0]._terminals_read == nil then
         Players[0]._terminals_read = {}
      end
   else
      Players[0]._terminals_read = nil
   end

   if Level.index == 17 and Game._initial_level == 0 then
      Triggers.projectile_switch = bc_projectile_switch
   end

   if Level.index == 19 then
      if not Players[0]._accivi_over then
		 Triggers.idle = accivi_idle
         Triggers.platform_switch = accivi_platform_switch
         Triggers.projectile_switch = accivi_projectile_switch
         Triggers.tag_switch = accivi_tag_switch
      end
   else
      Players[0]._accivi_over = nil
   end
end

function Triggers.cleanup()
   if not Level.completed then
      return
   end

   if Level.index == 24 and Game._initial_level == 0 then
      got_achievement("ACH_VICTORY")
      if Game._min_difficulty == "total carnage" then
         got_achievement("ACH_TOTAL_VICTORY")
      end
   end

   if Level.index == 32 and
      (Game._initial_level == 0 or Game._initial_level == 30) and
      Game._min_difficulty == "total carnage"
   then
      got_achievement("ACH_VIDMASTER")
   end
end

function rrr_terminal_enter(terminal)
   if terminal.index == 0 then
      got_achievement("ACH_INSTIGATION")
      Triggers.terminal_enter = nil
   end
end

function as_projectile_switch()
   if Level.calculate_completion_state() == "finished" then
      got_achievement("ACH_ACME")
      Triggers.projectile_switch = nil
   end
end

function nmhc_platform_activated(polygon)
   if polygon.index == 259 then
      got_achievement("ACH_NAW_MAN")
      Triggers.platform_activated = nil
   end
end

function etp_terminal_enter(terminal)
   local p = Players[0]
   p._terminals_read[terminal.index] = true
   local complete = true
   for i=0,6 do
      if not p._terminals_read[i] then
         complete = false
         break
      end
   end
   if complete then
      got_achievement("ACH_PATHS")
      Triggers.terminal_enter = nil
   end
end

function bc_projectile_switch()
   if Level.calculate_completion_state() == "finished" then
      got_achievement("ACH_BY_COMMITTEE")
      Triggers.platform_activated = nil
   end
end

function accivi_path_unblocked()
   for p in Platforms() do
      if p.tag.index == 2 and not p.has_been_activated then
         return false
      end
   end

   return true
end

function accivi_over()
   Players[0]._accivi_over = true
   
   Triggers.idle = nil
   Triggers.platform_switch = nil
   Triggers.projectile_switch = nil
   Triggers.tag_switch = nil
end   

function accivi_idle()
   if Players[0].monster.polygon.index == 639 then
	  accivi_over()
   end
end

function accivi_platform_switch(polygon, _, side)
   if accivi_path_unblocked() then
      got_achievement("ACH_CONVERTED_PRIEST")
      accivi_over()
   end
end

function accivi_projectile_switch(_, side)
   if side.index == 2598 then
      accivi_over()
   elseif accivi_path_unblocked() then
      got_achievement("ACH_CONVERTED_PRIEST")
      accivi_over()
   end
end
         
function accivi_tag_switch(_, _, side)
   if side.index == 2598 then
      accivi_over()
   end
end

function wamid_monster_killed(monster, _, projectile)
   if monster.type == "major defender" and projectile.type == "missile" then
      got_achievement("ACH_DREAM_MONSTER")
      Triggers.monster_killed = nil
   end
end

function py_got_item(_, _)
   if Players[0].items["shotgun"] == 1 and
      Players[0].items["smg"] == 1
   then
      got_achievement("ACH_FILM_BUFF")
      Triggers.got_item = nil
   end
end

function twk_platform_activated(polygon)
   if polygon.index == 28 or polygon.index == 33 then
      local platform = Platforms[polygon.permutation]
      if not platform.active
         and platform.ceiling_height == platform.minimum_ceiling_height
      then
         Players[0]._doors_cycled[polygon.index] = true

         if Players[0]._doors_cycled[28] and Players[0]._doors_cycled[33] then
            got_achievement("ACH_THING_WHAT_KICKS")
            Triggers.platform_activated = nil
         end
      end
   end
end

function es2_idle()
   if Players[0].teleporting and Players[0].polygon.index == 63 then
      got_achievement("ACH_CYCLICAL")
      Triggers.idle = nil
   end
end

)LUA"
