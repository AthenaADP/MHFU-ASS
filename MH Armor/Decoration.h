#pragma once
#include "Common.h"
#include "Skill.h"

ref struct Query;

ref struct Decoration
{
	System::String^ name, ^price;//, ^hre_hr, ^hre_el;
	unsigned hr, hr_req, elder_star, elder_star_req, slots_required;
	bool dangerous;
	List_t< AbilityPair^ > abilities;
	List_t< MaterialComponent^ > components, components2;

	bool IsBetterThan( Decoration^ other, List_t< Ability^ >^ rel_abilities );
	int GetSkillAt( Ability^ ability );

	bool MatchesQuery( Query^ query );
	
	static void Load( System::String^ filename );
	static List_t< Decoration^ > static_decorations;
	static Map_t< Ability^, List_t< Decoration^ >^ > static_decoration_ability_map;
	static Map_t< System::String^, Decoration^ > static_decoration_map;
	static Decoration^ FindDecoration( System::String^ name );
	static Decoration^ GetBestDecoration( Ability^ ability, const unsigned max_slots, Query^ query );
};
