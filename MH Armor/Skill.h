#pragma once
#include "Common.h"
#include <cliext/map>

ref struct Skill;
ref struct Query;
ref struct SkillTag;

ref struct Ability
{
	typedef System::Collections::Generic::Dictionary< int, Skill^ > SkillMap_t;
	System::String^ name;
	List_t< SkillTag^ > tags;
	SkillMap_t skills;

	Skill^ GetSkill( const int amount );

	bool MatchesQuery( Query^ query );

	static Ability^ torso_inc;
	static List_t< Ability^ > static_abilities;
	static Map_t< System::String^, Ability^ > static_ability_map;
	static Ability^ FindAbility( System::String^ name );
};

ref struct Skill
{
	System::String^ name;
	int points_required;
	Ability^ ability;

	static void Load( System::String^ filename, List_t< SkillTag^ >% tags );
	static List_t< Skill^ > static_skills;
	static Map_t< System::String^, Skill^ > static_skill_map;
	static Skill^ FindSkill( System::String^ name );
};

ref struct AbilityPair
{
	Ability^ ability;
	int amount;
};

