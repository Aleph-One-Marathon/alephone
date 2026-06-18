#pragma once

#include "InfoTree.h"

// MML tag: <vr_sprites> containing <left_handed_weapon> entries.
// Each entry marks an entire weapon (by engine index or name) as naturally left-handed
// in the PC original. The VR renderer flips all sprites for that weapon when the player
// is right-handed, and leaves them as-is when left-handed — the inverse of the normal rule.
//
// Attributes (use either or both; index takes precedence if both present):
//   index="N"              engine weapon-type constant (0=fist, 4=missile_launcher, ...)
//   name="missile_launcher" engine weapon name string (see mapping in .cpp)
//
// Example:
//   <vr_sprites>
//     <left_handed_weapon name="fist"/>
//     <left_handed_weapon name="missile_launcher"/>
//   </vr_sprites>
void reset_mml_vr_sprites();
void parse_mml_vr_sprites(const InfoTree& root);

// Returns true if weapon_type (engine index) was declared left-handed in MML.
bool VR_IsWeaponNaturallyLeftHanded(short weapon_type);
