R"LUA(
Triggers = {}

-- M1 mnemonics
MonsterClasses[1024].mnemonic = "hulk"
ProjectileTypes[14].mnemonic = "fusion bolt major"
ProjectileTypes[16].mnemonic = "fist"
MonsterTypes[38].mnemonic = "alien leader"

function got_achievement(achievement)
   set_achievement(achievement)
end

function Triggers.init(restored)
   if Game.replay or Level.map_checksum ~= 969 then
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

   for p in Players() do
      if p.local_ then
         local_player = p
         p._ticks_flying = 0
      end
   end
   
   if Game.difficulty.index < Game._min_difficulty.index then
      Game._min_difficulty = Game.difficulty
   end

   if Level.index == 1 and Game._initial_level == 0 then
      Triggers.got_item = bgn_got_item
   end

   if Level.index == 2 then
      Triggers.terminal_enter = nbm_terminal_enter
   end

   if Level.index == 3 then
      Triggers.player_damaged = dt_player_damaged
   end

   if Level.index == 8 and Game._initial_level == 0 then
      Triggers.terminal_enter = g4_terminal_enter
   end

   if Level.index == 13 then
      local_player._took_grenade_damage = false
      Triggers.player_damaged = csfsc_player_damaged
   end

   if Level.index == 14 then
      Triggers.player_damaged = hq_player_damaged
   end

   if Level.index == 16 and Game._initial_level == 0 then
      got_achievement("ACH_ALIEN_SHIP")
   end

   if Level.index == 22 and Game._initial_level == 0 then
      Triggers.monster_killed = pfhoraphobia_monster_killed
   end
end

function Triggers.idle()
   if local_player._ticks_flying ~= nil then
      if local_player.z > local_player.polygon.z then
         local_player._ticks_flying = local_player._ticks_flying + 1
         if local_player._ticks_flying == 7 * 30 then
            got_achievement("ACH_FLY")
            local_player._ticks_flying = nil
         end
      else
         local_player._ticks_flying = 0
      end
   end
end

function Triggers.cleanup()
   if not Level.completed then
      return
   end

   if Level.index == 13 and not local_player._took_grenade_damage then
      got_achievement("ACH_LAWFUL_GOOD")
   elseif Level.index == 26 and Game._initial_level == 0 then
      got_achievement("ACH_VICTORY")
      if Game._min_difficulty == "total carnage" then
         got_achievement("ACH_TOTAL_VICTORY")
      end
   end
end

-- TODO: install this only on levels with hulks?
function Triggers.monster_killed(monster, aggressor_player, projectile)
   if monster.type.class == "hulk" 
      and aggressor_player == local_player
      and projectile.type == "fist"
   then
      got_achievement("ACH_SLAP_FIGHT")
   end
end

function bgn_got_item(type, player)
   if player == local_player and type == "assault rifle" then
      got_achievement("ACH_BIGGER_GUN")
      Triggers.got_item = nil
   end
end

function nbm_terminal_enter(_, player)
   -- for now, the terminal itself is nil due to issue #467 so award this based
   -- on where the player is standing
   if player == local_player and
      (player.polygon.index == 203 or player.polygon.index == 204)
   then
      got_achievement("ACH_INFAMOUS")
      Triggers.terminal_enter = nil
   end
end

function dt_player_damaged(victim, _, _, damage_type)
   if victim == local_player
      and damage_type == "crushing"
      and victim.polygon.index == 32
      and victim.life < 0
   then
      got_achievement("ACH_DEFEND_SQUISH")
      Triggers.player_damaged = nil
   end
end

function g4_terminal_enter(_, player)
   -- once again, terminal is nil due to issue #467
   if player.polygon.index ~= 142 and
      player.polygon.index ~= 143 and
      player.polygon.index ~= 144 and
      Level.calculate_completion_state() == "finished"
   then
      got_achievement("ACH_WARN_EARTH")
      Triggers.terminal_enter = nil
   end
end

function csfsc_player_damaged(victim, aggressor_player, _, damage_type)
   if victim == local_player
      and damage_type == "explosion"
      and aggressor_player ~= nil
   then
      victim._took_grenade_damage = true
      Triggers.player_damaged = nil
   end
end

function hq_player_damaged(victim, _, _, damage_type)
   if victim == local_player and damage_type == "lava" and victim.life < 0 then
      got_achievement("ACH_LAVA")
      Triggers.player_damaged = nil
   end
end

function pfhoraphobia_monster_killed(monster, _, _)
   if monster.type == "alien leader" then
      got_achievement("ACH_PFHOR_CYBORG")
      Triggers.monster_killed = nil
   end
end

)LUA"
