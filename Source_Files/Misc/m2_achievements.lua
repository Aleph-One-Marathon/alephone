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

   if Level.index == 0 then
      Triggers.tag_switch = ww_tag_switch
   end

   if Level.index == 4 then
      Triggers.terminal_enter = catym_terminal_enter
   end

   if Level.index == 5 then
      if Players[0]._fusion_pistols == nil then
         Players[0]._fusion_pistols = Players[0].items["fusion pistol"]
      end
      Triggers.terminal_enter = we_terminal_enter
      Triggers.idle = we_idle
   else
      Players[0]._fusion_pistols = nil
   end

   if Level.index == 9 then
      Triggers.idle = eivb_idle
   end

   if Level.index == 13 then
      if not Level._surfs_up_over then
         Triggers.idle = iiharl_idle
         Triggers.platform_activated = iiharl_platform_activated
         Triggers.projectile_created = iiharl_projectile_created
      end
   else
      Level._surfs_up_over = nil
   end

   if Level.index == 16 then
      Triggers.projectile_switch = bfmmma_projectile_switch
   end

   if Level.index == 21 then
      Triggers.terminal_enter = kyt_terminal_enter
   end

   if Level.index == 27 then
      Triggers.light_activated = arlts_light_activated
   end
end

function Triggers.cleanup()
   if not Level.completed then
      return
   end

   if Level.index == 27 and Game._initial_level == 0 then
      got_achievement("ACH_VICTORY")
      if Game._min_difficulty == "total carnage" then
         got_achievement("ACH_TOTAL_VICTORY")
      end
   end
end

function Triggers.monster_damaged(monster, _, damage_type, damage_amount)
   if monster.type == "mother of all hunters" and
      damage_type == "fusion" and
      damage_amount >= 500 and -- it's actually 1000 because of weakness
      monster.vitality <= 0
   then
      got_achievement("ACH_HIGH_ENERGY")
      Triggers.monster_damaged = nil
   end
end

function catym_terminal_enter(terminal)
   if terminal.index == 1 then
      got_achievement("ACH_SPHT_TALK")
      Triggers.terminal_enter = nil
   end
end

function we_terminal_enter()
   if Players[0].items["fusion pistol"] == Players[0]._fusion_pistols then
      got_achievement("ACH_VID_BUOY")
      Triggers.terminal_enter = nil
   end
end

function arlts_light_activated(light)
   if light.index == 15 then
      got_achievement("ACH_OATH")
      Triggers.light_activated = nil
   end
end

function eivb_idle()
   if Game._initial_level == 0 and Players[0].monster.polygon.index == 14 then
      got_achievement("ACH_CITADEL")
      Triggers.idle = nil
   end
end

function bfmmma_projectile_switch()
   if Level.calculate_completion_state() == "finished" and
      Game._initial_level == 0
   then
      got_achievement("ACH_DURANDAL")
      Triggers.idle = nil
   end
end

function kyt_terminal_enter(terminal)
   if terminal.index == 1 and
      Level.calculate_completion_state() == "finished" and
      Game._initial_level == 0
   then
      got_achievement("ACH_THOTH")
      Triggers.terminal_enter = nil
   end
end

function ww_tag_switch()
   if Level.calculate_completion_state() == "finished" then
      got_achievement("ACH_CHIPS")
      Triggers.tag_switch = nil
   end
end

function iiharl_idle()
   if Players[0].monster.polygon.index == 146 and
      Players[0].x > -8.0
   then
      got_achievement("ACH_SURFS_UP")
      Triggers.idle = nil
      Triggers.projectile_created = nil
      Triggers.platform_activated = nil
   end
end

function iiharl_projectile_created(projectile)
   if projectile.owner and
      projectile.owner.player and
      projectile.type ~= "fist"
   then
      Level._surfs_up_over = true
      Triggers.idle = nil
      Triggers.projectile_created = nil
      Triggers.platform_activated = nil
   end
end

function iiharl_platform_activated(polygon)
   if polygon.index == 29 or
      polygon.index == 33 or
      polygon.index == 64
   then
      Level._surfs_up_over = true
      Triggers.idle = nil
      Triggers.projectile_created = nil
      Triggers.platform_activated = nil
   end
end

function we_idle()
   if Players[0].monster.polygon.index == 256 then
      got_achievement("ACH_EVERYWHERE")
      Triggers.idle = nil
   end
end
   
)LUA"
