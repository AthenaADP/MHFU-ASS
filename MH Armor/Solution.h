#pragma once
#include "Common.h"
#include <cliext/utility>

ref struct Skill;
ref struct Ability;
ref struct Armor;
ref struct ArmorEquivalence;
ref struct Decoration;
ref struct AbilityPair;

typedef System::Collections::Generic::Dictionary< Ability^, int > AbilityMap;

ref struct Query : public System::Object
{
	List_t< Skill^ > skills;
	HunterType hunter_type;
	Gender gender;
	unsigned hr, elder_star, weapon_slots_allowed, difficulty_level;
	bool include_piercings, allow_bad, allow_torso_inc, allow_dummy, danger;

	List_t< List_t< Armor^ >^ > rel_armor, inf_armor;
	List_t< List_t< ArmorEquivalence^ >^ > armor_eq;
	List_t< Ability^ > rel_abilities;
	List_t< Decoration^ > rel_decorations, inf_decorations;
	List_t< List_t< Decoration^ >^ > sorted_decorations;
	AbilityMap ability_indices;

	void CreateEquivalences();
};

ref struct PossibleDecoration
{
	Decoration^ decoration;
	unsigned points_given, index;
};

ref struct EquivalenceSolution
{
	List_t< ArmorEquivalence^ > armor_eq;
	List_t< Decoration^ > decorations;
	List_t< int > multipliers;
	cli::array< unsigned >^ slots_spare;
	unsigned torso_slots_spare;
	unsigned torso_multiplier;

	bool MatchesQuery( Query^ query );
private:
	void CalculateEquivalenceInfo( AbilityMap% ability_map, ArmorEquivalence^ eq, List_t< int >% skills, const bool torso );
	void CalculateDecorations( Query^ query, List_t< int >% skill_points, List_t< int >% desired );
	void AddDecoration( AbilityMap% ability_map, List_t< int >% skill_points, Decoration^ decoration, const unsigned index, const int amount, const unsigned multiplier );
	void AddDecorations1( AbilityMap% ability_map, List_t< Decoration^ >^ rel_decorations, const int multiplier, unsigned& num_slots, List_t< int >% skill_points, List_t< int >% desired );
	void AddDecorations23( AbilityMap% ability_map, List_t< Decoration^ >^ rel_decorations, const unsigned size, const int multiplier, unsigned& num_slots, List_t< int >% skill_points, List_t< int >% desired );
	cliext::pair< int, int > GetScore( Decoration^ decoration, AbilityMap% ability_map, List_t< int >% skill_points, const unsigned actual_good );
	PossibleDecoration^ GetBestDecoration( List_t< PossibleDecoration^ >% decorations, AbilityMap% ability_map, List_t< int >% skill_points );
};

ref struct Solution
{
	List_t< Armor^ > armors;
	List_t< Decoration^ > decorations;
	List_t< Skill^ > extra_skills;
	AbilityMap abilities;

	unsigned torso_slots_spare, torso_multiplier, total_slots_spare;
	int fire_res, ice_res, water_res, thunder_res, dragon_res;
	unsigned defence, rarity, difficulty;
	cli::array< int >^ slots_spare;

	bool CheckBadSkills( Query^ query );
	void CalculateData( const unsigned difficulty_level );

	Solution( EquivalenceSolution^ solution );
	Solution();

private:
	void AddExtraSkill( AbilityPair^ apair, const int mult );
	void CalculateExtraSkills();
	void CalculateExtraAbilities();
	void Replace( const unsigned index, Decoration^ decoration );
	int GetReplacable( Ability^ ability, unsigned slots );
	Decoration^ Count1SocketGems( Ability^ ability, int& num );
	bool ReorderGems( List_t< Skill^ >% bad_skills, Query^ query );
	bool FixBadSkills( List_t< Skill^ >% bad_skills, Query^ query );
	bool FixBadSkill( Skill^ bad_skill, Query^ query );
	bool AddDecoration( Decoration^ decoration );
	bool Detrimental( Decoration^ dec, Query^ query  );
};
