#include "stdafx.h"
#include "Solution.h"
#include "Armor.h"
#include "Decoration.h"
#include "Skill.h"

using namespace System;

bool Solution::Detrimental( Decoration^ dec, Query^ query )
{
	if( dec->abilities.Count < 2 ) return false;
	for each( Skill^ skill in query->skills )
		if( skill->ability == dec->abilities[ 1 ]->ability )
			return true;
	return false;
}

bool Solution::AddDecoration( Decoration^ decoration )
{
	decorations.Add( decoration );
	if( decoration->abilities.Count == 2 )
	{
		if( !abilities.ContainsKey( decoration->abilities[ 1 ]->ability ) )
			abilities.Add( decoration->abilities[ 1 ]->ability, decoration->abilities[ 1 ]->amount );
		else abilities[ decoration->abilities[ 1 ]->ability ] += decoration->abilities[ 1 ]->amount;
	}
	abilities[ decoration->abilities[ 0 ]->ability ] += decoration->abilities[ 0 ]->amount;
	return abilities[ decoration->abilities[ 0 ]->ability ] > -10;
}

bool Solution::FixBadSkill( Skill^ skill, Query^ query )
{
	int torso_slots = torso_slots_spare;
	while( torso_slots > 0 )
	{
		Decoration^ dec = Decoration::GetBestDecoration( skill->ability, torso_slots, query );
		if( Detrimental( dec, query ) )
			return false;
		if( AddDecoration( dec ) )
			return true;
		torso_slots -= dec->slots_required;
	}

	int max_slots = 3;
	while( max_slots > 0 && !slots_spare[ max_slots ] ) max_slots--;

	if( max_slots == 0 ) return false;

	Decoration^ dec = Decoration::GetBestDecoration( skill->ability, max_slots, query );
	if( Detrimental( dec, query ) )
		return false;

	while( max_slots > 0 )
	{
		if( slots_spare[ dec->slots_required ] == 0 )
		{
			if( slots_spare[ dec->slots_required + 1 ] == 0 )
			{
				slots_spare[ 3 ]--;
				slots_spare[ 2 ]++;
			}
			else
			{
				slots_spare[ dec->slots_required + 1 ]--;
				slots_spare[ 1 ]++;
			}
		}
		else slots_spare[ dec->slots_required ]--;

		if( AddDecoration( dec ) )
			return true;

		while( max_slots > 0 && !slots_spare[ max_slots ] )
			dec = Decoration::GetBestDecoration( skill->ability, --max_slots, query );
	}
	return false;
}


bool Solution::FixBadSkills( List_t< Skill^ >% bad_skills, Query^ query )
{
	for each( Skill^ skill in bad_skills )
		if( !FixBadSkill( skill, query ) )
			return false;
	CalculateExtraSkills();
	return true;
}

void Query::CreateEquivalences()
{
	armor_eq.Clear();
	for each( List_t< Armor^ >^ armor_set in rel_armor )
	{
		List_t< ArmorEquivalence^ >^ list = gcnew List_t< ArmorEquivalence^ >;
		for each( Armor^ armor in armor_set )
		{
			bool need_new = true;
			for each( ArmorEquivalence^ eq in list )
			{
				if( eq->Matches( armor ) )
				{
					eq->Add( armor, %rel_abilities );
					need_new = false;
					break;
				}
			}
			if( need_new )
				list->Add( gcnew ArmorEquivalence( armor, %rel_abilities ) );
		}
		armor_eq.Add( list );
	}
	for( unsigned i = 0; i < 4; ++i )
		sorted_decorations.Add( gcnew List_t< Decoration^ > );
	for each( Decoration^ decoration in rel_decorations )
		sorted_decorations[ decoration->slots_required ]->Add( decoration );

	for( int i = 0; i < rel_abilities.Count; ++i )
		ability_indices.Add( rel_abilities[ i ], i );
}

bool EquivalenceSolution::MatchesQuery( Query^ query )
{
	List_t< int > skill_points, desired;
	torso_multiplier = 1;
	slots_spare = gcnew cli::array< unsigned >( 4 );
	torso_slots_spare = 0;
	for( int i = 0; i < query->skills.Count; ++i )
	{
		skill_points.Add( 0 );
		desired.Add( 0 );
	}
	
	CalculateEquivalenceInfo( query->ability_indices, armor_eq[ 0 ], skill_points, false );
	CalculateEquivalenceInfo( query->ability_indices, armor_eq[ 2 ], skill_points, false );
	CalculateEquivalenceInfo( query->ability_indices, armor_eq[ 3 ], skill_points, false );
	CalculateEquivalenceInfo( query->ability_indices, armor_eq[ 4 ], skill_points, false );
	CalculateEquivalenceInfo( query->ability_indices, armor_eq[ 1 ], skill_points, true );
	slots_spare[ torso_slots_spare = armor_eq[ 1 ]->num_slots ]--;

	for each( Skill^ skill in query->skills )
		desired[ query->ability_indices[ skill->ability ] ] = skill->points_required;

	CalculateDecorations( query, skill_points, desired );

	for( int i = 0; i < desired.Count; ++i )
		if( skill_points[ i ] < desired[ i ] )
			return false;

	return true;
}

void EquivalenceSolution::CalculateEquivalenceInfo( AbilityMap% ability_map, ArmorEquivalence^ eq, List_t< int >% skills, const bool torso )
{
	torso_multiplier += eq->torso_inc;
	slots_spare[ eq->num_slots ]++;
	if( eq->torso_inc || eq->no_skills ) return;
	for each( AbilityPair^ apair in eq->abilities )
		skills[ ability_map[ apair->ability ] ] += apair->amount * ( torso * ( torso_multiplier - 1 ) + 1 );
}

void EquivalenceSolution::CalculateDecorations( Query^ query, List_t< int >% skill_points, List_t< int >% desired )
{
	unsigned body_slots[ 4 ] = { 0, 0, 0, 0 };
	unsigned temp_slots[ 4 ] = { 0, 0, 0, 0 };
	//get initial slot configuration and sort decorations by size
	if( query->weapon_slots_allowed ) slots_spare[ query->weapon_slots_allowed ]++;
	for( int i = 1; i < 4; ++i )
	{
		temp_slots[ i ] = slots_spare[ i ];
	}
	if( torso_slots_spare )
		body_slots[ torso_slots_spare ]++;

	//try adding decorations
	AddDecorations23( query->ability_indices, query->sorted_decorations[ 3 ], 3, torso_multiplier, body_slots[ 3 ], skill_points, desired );
	body_slots[ 1 ] += body_slots[ 3 ];
	body_slots[ 2 ] += body_slots[ 3 ];
	body_slots[ 3 ] = 0;
	AddDecorations23( query->ability_indices, query->sorted_decorations[ 3 ], 3, 1, temp_slots[ 3 ], skill_points, desired );
	slots_spare[ 3 ] -= slots_spare[ 3 ] - temp_slots[ 3 ];
	temp_slots[ 1 ] += temp_slots[ 3 ];
	temp_slots[ 2 ] += temp_slots[ 3 ];
	temp_slots[ 3 ] = 0;

	AddDecorations23( query->ability_indices, query->sorted_decorations[ 2 ], 2, torso_multiplier, body_slots[ 2 ], skill_points, desired );
	body_slots[ 1 ] += 2 * body_slots[ 2 ];
	body_slots[ 2 ] = 0;
	{
		const unsigned temp = temp_slots[ 2 ];
		AddDecorations23( query->ability_indices, query->sorted_decorations[ 2 ], 2, 1, temp_slots[ 2 ], skill_points, desired );
		const unsigned num_2slot_decorations = temp - temp_slots[ 2 ];
		if( num_2slot_decorations > slots_spare[ 2 ] )
		{
			const unsigned to_convert = num_2slot_decorations - slots_spare[ 2 ];
			slots_spare[ 3 ] -= to_convert;
			slots_spare[ 2 ] = 0;
			slots_spare[ 1 ] += to_convert;
		}
		else slots_spare[ 2 ] -= num_2slot_decorations;
	}
	temp_slots[ 1 ] += 2 * temp_slots[ 2 ];
	temp_slots[ 2 ] = 0;

	AddDecorations1( query->ability_indices, query->sorted_decorations[ 1 ], torso_multiplier, body_slots[ 1 ], skill_points, desired );
	{
		const unsigned temp = temp_slots[ 1 ];
		AddDecorations1( query->ability_indices, query->sorted_decorations[ 1 ], 1, temp_slots[ 1 ], skill_points, desired );
		unsigned num_1slot_decorations = temp - temp_slots[ 1 ];
		if( num_1slot_decorations > slots_spare[ 1 ] )
		{
			unsigned extra = num_1slot_decorations - slots_spare[ 1 ];
			if( extra > slots_spare[ 2 ] * 2 )
			{
				const unsigned extra3 = extra - slots_spare[ 2 ] * 2;
				const unsigned num_3_convert = extra3 / 3 + !!( extra3 % 3 );
				slots_spare[ 3 ] -= num_3_convert;
				if( num_3_convert * 3 > extra )
				{
					const unsigned super_extra = num_3_convert * 3 - extra;
					slots_spare[ 2 ] += super_extra / 2;
					slots_spare[ 1 ] += super_extra % 2;
					num_1slot_decorations -= extra;
					extra = 0;
				}
				else
				{
					extra -= num_3_convert * 3;
					num_1slot_decorations -= num_3_convert * 3;
				}
			}
			const unsigned num_2_slots = extra / 2 + extra % 2;
			slots_spare[ 2 ] -= num_2_slots;
			slots_spare[ 1 ] -= num_1slot_decorations - num_2_slots * 2;
		}
		else slots_spare[ 1 ] -= num_1slot_decorations;
	}
	torso_slots_spare = body_slots[ 1 ];
}

void EquivalenceSolution::AddDecorations1( AbilityMap% ability_map, List_t< Decoration^ >^ rel_decorations, const int multiplier, unsigned& num_slots, List_t< int >% skill_points, List_t< int >% desired )
{
	if( num_slots == 0 || rel_decorations->Count == 0 ) return;
	bool loop = true;
	int i = 0;
	List_t< cliext::pair< Decoration^, unsigned >^ > possible_decorations;
	List_t< unsigned > possible_indices;
	while( loop )
	{
		Decoration^ decoration = rel_decorations[ i ];
		AbilityPair^ apair = decoration->abilities[ 0 ];
		assert( apair->amount > 0 );
		const unsigned index = ability_map[ apair->ability ];

		const int current_amount = skill_points[ index ];
		const int decoration_amount = apair->amount * multiplier;
		const int needed_amount = desired[ index ];
		if( decoration_amount > 0 && current_amount < needed_amount && current_amount + decoration_amount <= needed_amount + 1 )
		{
			loop = true;
			AddDecoration( ability_map, skill_points, decoration, index, decoration_amount, multiplier );
			if( --num_slots == 0 ) return;
			--i;
		}
		if( ++i == rel_decorations->Count )
		{
			loop = false;
			i = 0;
		}
	}
}

void EquivalenceSolution::AddDecorations23( AbilityMap% ability_map, List_t< Decoration^ >^ rel_decorations, const unsigned size, const int multiplier, unsigned& num_slots, List_t< int >% skill_points, List_t< int >% desired )
{
	if( num_slots == 0 || rel_decorations->Count == 0 ) return;
	bool loop = true;
	int i = 0;
	List_t< PossibleDecoration^ > possible_decorations;
	List_t< unsigned > possible_indices;
	while( loop )
	{
		PossibleDecoration^ possible = gcnew PossibleDecoration;
		possible->decoration = rel_decorations[ i ];
		AbilityPair^ apair = possible->decoration->abilities[ 0 ];
		assert( apair->amount > 0 );
		possible->index = ability_map[ apair->ability ];

		const int current_amount = skill_points[ possible->index ];
		const int decoration_amount = apair->amount * multiplier;
		const int needed_amount = desired[ possible->index ];
		if( decoration_amount > 0 && current_amount < needed_amount && current_amount + decoration_amount <= needed_amount + 1 )
		{
			loop = true;
			possible->points_given = Math::Min( decoration_amount, needed_amount - current_amount );
			possible_decorations.Add( possible );
		}
		if( ++i == rel_decorations->Count )
		{
			loop = false;
			i = 0;
		}
	}
	if( possible_decorations.Count > 0 )
	{
		PossibleDecoration^ best = GetBestDecoration( possible_decorations, ability_map, skill_points );
		AddDecoration( ability_map, skill_points, best->decoration, best->index, best->decoration->abilities[ 0 ]->amount * multiplier, multiplier );
		if( --num_slots > 0 )
			AddDecorations23( ability_map, rel_decorations, size, multiplier, num_slots, skill_points, desired );
	}
}

//pair( good, bad )
cliext::pair< int, int > EquivalenceSolution::GetScore( Decoration^ decoration, AbilityMap% ability_map, List_t< int >% skill_points, const unsigned actual_good )
{
	static int point_score[ 6 ] = { 0, 1, 2, 3, 4, 3 };
	const int bad = decoration->abilities.Count == 1 ? 0 : ability_map.ContainsKey( decoration->abilities[ 1 ]->ability ) ? skill_points[ ability_map[ decoration->abilities[ 1 ]->ability ] ] : 0;
	return cliext::make_pair( point_score[ actual_good ], bad );
}

PossibleDecoration^ EquivalenceSolution::GetBestDecoration( List_t< PossibleDecoration^ >% decorations, AbilityMap% ability_map, List_t< int >% skill_points )
{
	unsigned best = 0;
	cliext::pair< int, int > score = GetScore( decorations[ best ]->decoration, ability_map, skill_points, decorations[ best ]->points_given );
	for( int i = 1; i < decorations.Count; ++i )
	{
		cliext::pair< int, int > curr_score = GetScore( decorations[ i ]->decoration, ability_map, skill_points, decorations[ i ]->points_given );
		if( curr_score.first >= score.first || curr_score.second > score.second )
		{
			score = curr_score;
			best = i;
		}
	}
	return decorations[ best ];
}

void EquivalenceSolution::AddDecoration( AbilityMap% ability_map, List_t< int >% skill_points, Decoration^ decoration, const unsigned index, const int amount, const unsigned multiplier )
{
	decorations.Add( decoration );
	multipliers.Add( multiplier );
	skill_points[ index ] += amount;
	if( decoration->dangerous )
		skill_points[ ability_map[ decoration->abilities[ 1 ]->ability ] ] += decoration->abilities[ 1 ]->amount * multiplier;
}

bool Solution::CheckBadSkills( Query^ query )
{
	CalculateExtraAbilities();
	CalculateExtraSkills();
	if( query->allow_bad ) return true;
	List_t< Skill^ > bad_skills;
	for each( Skill^ skill in extra_skills )
		if( skill->points_required < 0 )
			bad_skills.Add( skill );
	return bad_skills.Count == 0 || ( total_slots_spare > 0 && FixBadSkills( bad_skills, query ) ) || ReorderGems( bad_skills, query );
}

void Solution::CalculateData( const unsigned difficulty_level )
{
	total_slots_spare = 0;
	for( unsigned i = 1; i < 4; ++i )
		total_slots_spare += i * slots_spare[ i ];
	total_slots_spare += torso_slots_spare;
	fire_res = ice_res = thunder_res = water_res = dragon_res = defence = rarity = difficulty = 0;
	for each( Armor^ armor in armors )
	{
		fire_res += armor->fire_res;
		ice_res += armor->ice_res;
		thunder_res += armor->thunder_res;
		water_res += armor->water_res;
		dragon_res += armor->dragon_res;
		defence += armor->defence;
		rarity += armor->rarity;
		difficulty += difficulty_level == 1 ? armor->difficulty1 : difficulty_level == 2 ? armor->difficulty2 : armor->difficulty3;
	}
}

Solution::Solution( EquivalenceSolution^ solution )
{
	total_slots_spare = 0;
	for( int i = 0; i < solution->decorations.Count; ++i )
	{
		decorations.Add( solution->decorations[ i ] );
		if( solution->decorations[ i ]->abilities.Count == 2 )
			AddExtraSkill( solution->decorations[ i ]->abilities[ 1 ], solution->multipliers[ i ] );
	}
	slots_spare = gcnew cli::array< int >( 4 );
	for( unsigned i = 1; i < 4; ++i )
		total_slots_spare += slots_spare[ i ] = solution->slots_spare[ i ];
	total_slots_spare += torso_slots_spare = solution->torso_slots_spare;
	torso_multiplier = solution->torso_multiplier;
}

Solution::Solution()
{
	slots_spare = gcnew cli::array< int >( 4 );
	total_slots_spare = fire_res = ice_res = thunder_res = water_res = dragon_res = defence = rarity = difficulty = 0;
}

void Solution::CalculateExtraAbilities()
{
	for each( Armor^ armor in armors )
		for each( AbilityPair^ apair in armor->extra_skills )
			AddExtraSkill( apair, armor == armors[ 1 ] ? torso_multiplier : 1 );
}

void Solution::AddExtraSkill( AbilityPair^ apair, const int mult )
{
	if( abilities.ContainsKey( apair->ability ) )
		abilities[ apair->ability ] += apair->amount * mult;
	else abilities.Add( apair->ability, apair->amount * mult );
}

void Solution::CalculateExtraSkills()
{
	extra_skills.Clear();
	for each( System::Collections::Generic::KeyValuePair< Ability^, int >^ apair in abilities )
	{
		Ability^ ability = apair->Key;
		Skill^ skill = ability->GetSkill( apair->Value );
		if( skill )
			extra_skills.Add( skill );
	}
}

Decoration^ Solution::Count1SocketGems( Ability^ ability, int& num )
{
	num = 0;
	Decoration^ res = nullptr;
	for each( Decoration^ dec in decorations )
	{
		if( dec->slots_required == 1 && dec->abilities.Count == 2 && dec->abilities[ 1 ]->ability == ability )
		{
			num++;
			res = dec;
		}
	}
	return res;
}

Decoration^ GetMostSlotted( Ability^ ability )
{
	List_t< Decoration^ >^ dex = Decoration::static_decoration_ability_map[ ability ];
	return dex[ dex->Count - 1 ];
}

int Solution::GetReplacable( Ability^ ability, unsigned slots )
{
	for( int i = 0; i < decorations.Count; ++i )
		if( decorations[ i ]->slots_required == slots && decorations[ i ]->abilities[ 1 ]->ability != ability )
			return i;
	return -1;
}

bool Solution::ReorderGems( List_t< Skill^ >% bad_skills, Query^ query )
{
	//return false;
	for each( Skill^ skill in bad_skills )
	{
		if( abilities[ skill->ability ] < -9 )
		{
			int num_1_socketed;
			Decoration^ one_slot = Count1SocketGems( skill->ability, num_1_socketed );
			if( num_1_socketed == 0 ) return false;
			
			Decoration^ bigger_slotted = GetMostSlotted( one_slot->abilities[ 0 ]->ability );
			const int num_required = bigger_slotted->abilities[ 0 ]->amount;
			if( num_1_socketed < num_required ) return false;
			
			const int other = GetReplacable( skill->ability, bigger_slotted->slots_required );
			if( other != -1 && num_1_socketed >= decorations[ other ]->abilities[ 0 ]->amount )
			{
				Decoration^ other_one_slot = Decoration::static_decoration_ability_map[ decorations[ other ]->abilities[ 0 ]->ability ][ 0 ];
				Replace( other, bigger_slotted );
				int num_replaced = 0;
				int num_to_replace = decorations[ other ]->abilities[ 0 ]->amount > num_required ? decorations[ other ]->abilities[ 0 ]->amount : num_required;
				for( int i = 0; i < decorations.Count; ++i )
					if( decorations[ i ] == one_slot && num_replaced++ < num_to_replace )
						Replace( i, other_one_slot );
				if( abilities[ skill->ability ] < -9 )
					return false;
			}
			else return false;
		}
	}
	CalculateExtraSkills();
	return true;
}

void Solution::Replace( const unsigned index, Decoration^ decoration )
{
	abilities[ decorations[ index ]->abilities[ 1 ]->ability ] -= decorations[ index ]->abilities[ 1 ]->amount;
	decorations[ index ] = decoration;
	for each( AbilityPair^ apair in decoration->abilities )
	{
		if( abilities.ContainsKey( apair->ability ) )
			abilities[ apair->ability ] += apair->amount;
		else abilities.Add( apair->ability, apair->amount );
	}
}
