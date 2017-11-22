#pragma once
#include "Common.h"
#include "Skill.h"

ref struct Query;

ref struct Armor
{
	enum class ArmorType { HEAD = 0, BODY, ARMS, WAIST, LEGS, NumArmorTypes };
	System::String^ name;
	unsigned hr, hr_req, elder_star, elder_star_req, num_slots, defence, rarity, difficulty1, difficulty2, difficulty3;
	int ice_res, water_res, fire_res, thunder_res, dragon_res;
	Gender gender;
	HunterType type;
	bool torso_inc, no_skills, is_piercing, is_dummy;
	Ability^ danger;
	List_t< AbilityPair^ > abilities, eq_skills, extra_skills;
	List_t< MaterialComponent^ > components;

	bool IsBetterThan( Armor^ other, List_t< Ability^ >^ rel_abilities );
	int GetSkillAt( Ability^ ability );

	bool MatchesQuery( Query^ query, List_t< Ability^ >^ danger_skills, const unsigned max_slots );

	void AdjustDifficulty( System::String^ material );
	static void Load( System::String^ filename, List_t< Armor^ >^ armors );
	static List_t< List_t< Armor^ >^ > static_armors;
	static Map_t< System::String^, List_t< Armor^ >^ > static_armor_map;
	static Armor^ FindArmor( System::String^ name );
};

ref struct ArmorEquivalence
{
	bool torso_inc, no_skills;
	unsigned num_slots;
	List_t< AbilityPair^ > abilities;
	List_t< Armor^ > armors;

	bool Matches( Armor^ armor );

	void Add( Armor^ armor, List_t< Ability^ >^ rel_abilities );

	ArmorEquivalence( Armor^ armor, List_t< Ability^ >^ rel_abilities );
};
