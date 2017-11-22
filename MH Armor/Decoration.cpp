#include "stdafx.h"
#include "Decoration.h"
#include "Skill.h"
#include "Solution.h"

using namespace System;
/*
void Decoration::Load( System::String^ filename, List_t< Decoration^ >^ decorations, List_t< Ability^ >^ abilities )
{
	decorations->Clear();
	decorations->Capacity = 256;
	IO::StreamReader fin( filename );
	while( true )
	{
		String^ line;
		do line = fin.ReadLine();
		while( !fin.EndOfStream && line != L"" && line[ 0 ] == L';' );
		if( line == L"" ) continue;
		if( fin.EndOfStream ) break;
		//read name and slots
		int quote1 = line->IndexOf( L'\"' );
		int quote2 = line->IndexOf( L'\"', quote1 + 1 );
		Decoration^ decoration = gcnew Decoration();
		decoration->name = line->Substring( quote1 + 1, quote2 - quote1 - 1 );
		List_t< String^ > split;
		Utility::SplitString( %split, line->Substring( quote2 + 1 ), L' ' );
		assert( split.Count == 3 );
		decoration->slots_required = Convert::ToUInt32( split[ 0 ] );
		Utility::FindLevelReqs( decoration->hr, decoration->hr_req, split[ 1 ] );
		decoration->hre_hr = split[ 1 ];
		decoration->hre_el = split[ 2 ];
		Utility::FindLevelReqs( decoration->elder_star, decoration->elder_star_req, split[ 2 ] );
		//read abilities
		while( !fin.EndOfStream )
		{
			do line = fin.ReadLine();
			while( line != L"" && line[ 0 ] == L';' );
			if( line == L"" ) break;
			quote1 = line->IndexOf( L'\"' );
			quote2 = line->IndexOf( L'\"', quote1 + 1 );
			AbilityPair^ a = gcnew AbilityPair();
			a->ability = FindAbility( line->Substring( quote1 + 1, quote2 - quote1 - 1 ), abilities );
			a->amount = Convert::ToInt32( line->Substring( 0, quote1 ) );
			decoration->abilities.Add( a );
		}
		decorations->Add( decoration );
		if( !static_decoration_map.ContainsKey( decoration->abilities[ 0 ]->ability ) )
			static_decoration_map.Add( decoration->abilities[ 0 ]->ability, gcnew List_t< Decoration^ > );
		static_decoration_map[ decoration->abilities[ 0 ]->ability ]->Add( decoration );
	}
	decorations->TrimExcess();
	static_decorations = decorations;
}*/

void Decoration::Load( System::String^ filename )
{
	static_decoration_map.Clear();
	static_decoration_ability_map.Clear();
	static_decorations.Clear();
	static_decorations.Capacity = 256;
	IO::StreamReader fin( filename );
	while( !fin.EndOfStream )
	{
		String^ line = fin.ReadLine();
		if( line == L"" ) continue;
		List_t< String^ > split;
		Utility::SplitString( %split, line, L',' );
		Decoration^ decoration = gcnew Decoration;
		//"Paralysis Jewel","200z","O--","1","1",1,"Paralysis",-1,"Poison",1,"Genprey Fang",1,"Suiko Jewel",,,,,,,,,,,,
		decoration->name = Utility::RemoveQuotes( split[ 0 ] );
		decoration->price = Utility::RemoveQuotes( split[ 1 ] );
		decoration->slots_required = Utility::CountChars( split[ 2 ], L'O' );
		Utility::FindLevelReqs( decoration->hr, decoration->hr_req, split[ 3 ] );
		Utility::FindLevelReqs( decoration->elder_star, decoration->elder_star_req, split[ 4 ] );
		for( int i = 0; i < 2; ++i )
		{
			if( split[ 5 + i * 2 ] == L"" ) continue;
			AbilityPair^ apair = gcnew AbilityPair;
			apair->ability = Ability::FindAbility( Utility::RemoveQuotes( split[ 6 + i * 2 ] ) );
			apair->amount = Convert::ToInt32( split[ 5 + i * 2 ] );
			decoration->abilities.Add( apair );
		}
		for( unsigned i = 0; i < 4; ++i )
		{
			if( split[ 9 + i * 2 ] != L"" )
			{
				MaterialComponent^ mc = gcnew MaterialComponent;
				mc->amount = Convert::ToUInt32( split[ 9 + i * 2 ] );
				mc->material = Material::FindMaterial( Utility::RemoveQuotes( split[ 10 + i * 2 ] ) );
				decoration->components.Add( mc );
			}
			if( split[ 17 + i * 2 ] != L"" )
			{
				MaterialComponent^ mc = gcnew MaterialComponent;
				mc->amount = Convert::ToUInt32( split[ 17 + i * 2 ] );
				mc->material = Material::FindMaterial( Utility::RemoveQuotes( split[ 18 + i * 2 ] ) );
				decoration->components2.Add( mc );
			}
		}
		
		static_decorations.Add( decoration );
		if( !static_decoration_ability_map.ContainsKey( decoration->abilities[ 0 ]->ability ) )
			static_decoration_ability_map.Add( decoration->abilities[ 0 ]->ability, gcnew List_t< Decoration^ > );
		static_decoration_ability_map[ decoration->abilities[ 0 ]->ability ]->Add( decoration );
		static_decoration_map.Add( decoration->name, decoration );
	}
	static_decorations.TrimExcess();
}

int Decoration::GetSkillAt( Ability^ ability )
{
	for( int i = 0; i < abilities.Count; ++i )
		if( abilities[ i ]->ability == ability )
			return abilities[ i ]->amount;
	return 0;
}

inline bool NotWorse( Decoration^ a, Decoration^ b )
{
	return b->abilities.Count == 2 && ( a->abilities.Count == 1 || a->abilities[ 1 ]->amount * b->slots_required < b->abilities[ 1 ]->amount * a->slots_required );
}

bool Decoration::IsBetterThan( Decoration^ other, List_t< Ability^ >^ rel_abilities )
{
	if( slots_required < other->slots_required || abilities[ 0 ]->ability != other->abilities[ 0 ]->ability )
		return true;
	const int a = abilities[ 0 ]->amount * other->slots_required,
			  b = other->abilities[ 0 ]->amount * slots_required;
	return a != b ? a > b : NotWorse( this, other );
}

Decoration^ Decoration::GetBestDecoration( Ability^ ability, const unsigned max_slots, Query^ query )
{
	Decoration^ best = nullptr;
	List_t< Ability^ > rel;
	rel.Add( ability );
	for each( Decoration^ dec in static_decoration_ability_map[ ability ] )
	{
		if( dec->hr > query->hr && dec->elder_star > query->elder_star ||
			dec->hr_req > query->hr || dec->elder_star_req > query->elder_star ||
			dec->slots_required > max_slots ) continue;
		for each( AbilityPair^ ap in dec->abilities )
		{
			if( ap->amount > 0 && ap->ability == ability )
			{
				if( !best || dec->IsBetterThan( best, %rel ) )
					best = dec;
			}
		}
	}
	return best;
}

bool Decoration::MatchesQuery( Query^ query )
{
	//check requirements
	if( hr > query->hr && elder_star > query->elder_star ||
		hr_req > query->hr || elder_star_req > query->elder_star )
		return false;
	//check for relevant skills
	for each( Skill^ skill in query->skills )
		if( skill->ability == abilities[ 0 ]->ability )
			return true;
	return false;
}

Decoration^ Decoration::FindDecoration( System::String^ name )
{
	if( static_decoration_map.ContainsKey( name ) )
		return static_decoration_map[ name ];
	return nullptr;
}