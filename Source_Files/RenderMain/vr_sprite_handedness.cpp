#include "vr_sprite_handedness.h"
#include "weapons.h"
#include <set>
#include <map>
#include <string>

static const std::map<std::string, short> kWeaponNames = {
	{"fist",             _weapon_fist},
	{"pistol",           _weapon_pistol},
	{"plasma_pistol",    _weapon_plasma_pistol},
	{"assault_rifle",    _weapon_assault_rifle},
	{"missile_launcher", _weapon_missile_launcher},
	{"flamethrower",     _weapon_flamethrower},
	{"alien_shotgun",    _weapon_alien_shotgun},
	{"shotgun",          _weapon_shotgun},
	{"ball",             _weapon_ball},
	{"smg",              _weapon_smg},
};

static std::set<short> s_leftHandedWeapons;

void reset_mml_vr_sprites()
{
	s_leftHandedWeapons.clear();
}

void parse_mml_vr_sprites(const InfoTree& root)
{
	for (const InfoTree& child : root.children_named("left_handed_weapon"))
	{
		short index = -1;
		child.read_attr("index", index);
		if (index < 0)
		{
			std::string name;
			if (child.read_attr("name", name))
			{
				auto it = kWeaponNames.find(name);
				if (it != kWeaponNames.end())
					index = it->second;
			}
		}
		if (index >= 0)
			s_leftHandedWeapons.insert(index);
	}
}

bool VR_IsWeaponNaturallyLeftHanded(short weapon_type)
{
	return s_leftHandedWeapons.count(weapon_type) > 0;
}
