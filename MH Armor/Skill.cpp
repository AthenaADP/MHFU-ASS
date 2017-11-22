#include "stdafx.h"
#include <fstream>
#include "Skill.h"
#include "Solution.h"
#include "LoadedData.h"

using namespace System;

Skill^ Ability::GetSkill( const int amount )
{
	if( amount == 0 ) return nullptr;
	int best = 0;
	SkillMap_t::Enumerator iter = skills.GetEnumerator();
	if( amount > 0 )
	{
		while( iter.MoveNext() )
			if( iter.Current.Key <= amount && iter.Current.Key > best )
				best = iter.Current.Key;
	}
	else
	{
		while( iter.MoveNext() )
			if( iter.Current.Key >= amount && iter.Current.Key < best )
				best = iter.Current.Key;
	}
	if( best == 0 ) return nullptr;
	assert( skills.ContainsKey( best ) );
	return skills[ best ];
}

bool Ability::MatchesQuery( Query^ query )
{
	SkillMap_t::Enumerator iter = skills.GetEnumerator();
	while( iter.MoveNext() )
	{
		if( iter.Current.Key > 0 && Utility::Contains( %query->skills, iter.Current.Value ) )
			return true;
	}
	return false;
}

Ability^ Ability::FindAbility( System::String^ name )
{
	if( static_ability_map.ContainsKey( name ) )
		return static_ability_map[ name ];
	return nullptr;
}

bool ContainsString( List_t< String^ >% vec, String^ item )
{
	for( int i = 0; i < vec.Count; ++i )
		if( vec[ i ] == item ) return true;
	return false;
}

int FindSkillIndex( Skill^ skill, List_t< Skill^ >^ vec )
{
	for( int i = 0; i < vec->Count; ++i )
		if( vec[ i ] == skill )
			return i;
	return -1;
}

void Skill::Load( String^ filename, List_t< SkillTag^ >% tags )
{
	IO::StreamReader fin( filename );

	Ability::static_abilities.Clear();
	Ability::static_ability_map.Clear();
	static_skills.Clear();
	static_skill_map.Clear();
	tags.Clear();
	tags.Add( gcnew SkillTag( L"All" ) );
	tags.Add( gcnew SkillTag( L"Untagged" ) );

	Ability::static_abilities.Capacity = 128;
	static_skills.Capacity = 256;

	String^ ability_name = fin.ReadLine();
	
	while( ability_name != L"" && !fin.EndOfStream )
	{
		Ability^ ability = gcnew Ability();
		ability->name = Utility::RemoveQuotes( ability_name );
		while( !fin.EndOfStream )
		{
			String^ skill_name = fin.ReadLine();
			if( skill_name == L"" )
			{
				Ability::static_abilities.Add( ability );
				Ability::static_ability_map.Add( ability->name, ability );
				ability_name = fin.ReadLine();
				break;
			}
			else if( skill_name->Length > 3 && skill_name->Substring( 0, 3 )->ToLower() == L"tag" )
			{
				skill_name = Utility::RemoveQuotes( skill_name->Substring( 4 ) );
				SkillTag^ t = Utility::FindByName( %tags, skill_name );
				if( !t )
				{
					t = gcnew SkillTag( skill_name );
					tags.Add( t );
				}
				ability->tags.Add( t );
			}
			else if( skill_name[ 0 ] == L'\"' )
			{
				ability_name = skill_name;
				Ability::static_abilities.Add( ability );
				Ability::static_ability_map.Add( ability->name, ability );
				break;
			}
			else
			{
				Skill^ skill = gcnew Skill();
				skill->ability = ability;
				skill->points_required = Convert::ToInt32( skill_name->Substring( 0, 3 ) );
				skill->name = Utility::RemoveQuotes( skill_name->Substring( skill_name->IndexOf( L'\"' ) ) );
				static_skills.Add( skill );
				static_skill_map.Add( skill->name, skill );
				ability->skills[ skill->points_required ] = skill;
			}
		}
	}

	if( ability_name != L"" )
	{
		Ability::torso_inc = gcnew Ability();
		Ability::torso_inc->name = Utility::RemoveQuotes( ability_name );
		Ability::static_abilities.Add( Ability::torso_inc );
		Ability::static_ability_map.Add( Ability::torso_inc->name, Ability::torso_inc );
	}
	else assert( false );

	//Ability::torso_inc = nullptr;

	fin.Close();
	tags.TrimExcess();
	static_skills.TrimExcess();
	Ability::static_abilities.TrimExcess();
}

Skill^ Skill::FindSkill( System::String^ name )
{
	if( static_skill_map.ContainsKey( name ) )
		return static_skill_map[ name ];
	return nullptr;
}
