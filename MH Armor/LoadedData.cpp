#include "stdafx.h"

#include "Skill.h"
#include "Common.h"
#include "LoadedData.h"

using namespace System;

Skill^ LoadedData::FindSkill( const unsigned skill )
{
	assert( int( skill ) < Skill::static_skills.Count );
	return Skill::static_skills[ skill ];
}

void LoadedData::ImportTextFiles()
{
	Armor::static_armor_map.Clear();
	Armor::static_armors.Clear();
	Armor::static_armors.Capacity = 5;
	for( int i = 0; i < int( Armor::ArmorType::NumArmorTypes ); ++i )
		Armor::static_armors.Add( gcnew List_t< Armor^ > );

	Material::LoadMaterials( L"Data/components.txt" );
	Skill::Load( L"Data/skills.txt", tags );
	Armor::Load( L"Data/head.csv",  Armor::static_armors[ int( Armor::ArmorType::HEAD  ) ] );
	Armor::Load( L"Data/body.csv",  Armor::static_armors[ int( Armor::ArmorType::BODY  ) ] );
	Armor::Load( L"Data/arms.csv",  Armor::static_armors[ int( Armor::ArmorType::ARMS  ) ] );
	Armor::Load( L"Data/waist.csv", Armor::static_armors[ int( Armor::ArmorType::WAIST ) ] );
	Armor::Load( L"Data/legs.csv",  Armor::static_armors[ int( Armor::ArmorType::LEGS  ) ] );
	Decoration::Load( L"Data/decorations.csv" );
/*
	IO::StreamReader fin( "test.csv" );
	IO::StreamWriter fout( L"decorations.csv" );

	while( !fin.EndOfStream )
	{
		String^ line = fin.ReadLine();
		List_t< String^ > split;
		Utility::SplitString( %split, line, L',' );
		//"Paralysis Jewel","200z","0--","Paralysis",1,"Poison",-1,"1x Genprey Fang;1x Suiko Jewel",""
		Decoration^ deco = gcnew Decoration;
		deco->name = Utility::RemoveQuotes( split[ 0 ] );
		deco->price = Utility::RemoveQuotes( split[ 1 ] );
		String^ slots = Utility::RemoveQuotes( split[ 2 ] );
		deco->slots_required = slots == L"000" ? 3 : slots == L"00-" ? 2 : slots == L"0--" ? 1 : 0;
		for( int i = 0; i < 2; ++i )
		{
			String^ ab_name = Utility::RemoveQuotes( split[ 3 + 2 * i ] );
			if( ab_name == L"" ) break;
			AbilityPair^ apair = gcnew AbilityPair;
			apair->ability = FindAbility( ab_name, %abilities );
			apair->amount = Convert::ToInt32( split[ 4 + 2 * i ] );
			deco->abilities.Add( apair );
		}
		List_t< String^ > comps, comps2;
		Utility::SplitString( %comps, Utility::RemoveQuotes( split[ 7 ] ), L';' );
		if( split[ 8 ] != L"" )
			Utility::SplitString( %comps2, Utility::RemoveQuotes( split[ 8 ] ), L';' );

		for each( String^ c in comps )
		{
			int x = c->IndexOf( L'x' );
			MaterialComponent^ mc = gcnew MaterialComponent;
			mc->amount = Convert::ToUInt32( c->Substring( 0, x ) );
			mc->component = c->Substring( x + 2 );
			deco->components.Add( mc );
		}
		for each( String^ c in comps2 )
		{
			int x = c->IndexOf( L'x' );
			MaterialComponent^ mc = gcnew MaterialComponent;
			mc->amount = Convert::ToUInt32( c->Substring( 0, x ) );
			mc->component = c->Substring( x + 2 );
			deco->components2.Add( mc );
		}

		Decoration^ mine = FindDecoration( deco->name );
		deco->hr = mine->hr;
		deco->hr_req = mine->hr_req;
		deco->elder_star = mine->elder_star;
		deco->elder_star_req = mine->elder_star_req;
		//write

		fout.Write( L"\"" + deco->name + L"\",\"" + deco->price + L"\",\"" );
		fout.Write( Utility::SlotString( deco->slots_required ) + L"\",\"" + mine->hre_hr + L"\",\"" + mine->hre_el + L"\"," );
		fout.Write( Convert::ToString( deco->abilities[ 0 ]->amount ) + L",\"" + deco->abilities[ 0 ]->ability->name + L"\"," );
		//abilities
		if( deco->abilities.Count == 2 )
			fout.Write( Convert::ToString( deco->abilities[ 1 ]->amount ) );
		fout.Write( L"," );
		if( deco->abilities.Count == 2 )
			fout.Write( L"\"" + deco->abilities[ 1 ]->ability->name + L"\"" );
		
		//components
		assert( deco->components.Count < 5 && deco->components2.Count < 5 );
		for( int i = 0; i < 4; ++i )
		{
			fout.Write( L"," );
			if( i < deco->components.Count )
				fout.Write( Convert::ToString( deco->components[ i ]->amount ) );
			fout.Write( L"," );
			if( i < deco->components.Count )
				fout.Write( L"\"" + deco->components[ i ]->component + L"\"" );
		}
		for( int i = 0; i < 4; ++i )
		{
			fout.Write( L"," );
			if( i < deco->components2.Count )
				fout.Write( Convert::ToString( deco->components2[ i ]->amount ) );
			fout.Write( L"," );
			if( i < deco->components2.Count )
				fout.Write( L"\"" + deco->components2[ i ]->component + L"\"" );
		}
		fout.WriteLine();
	}
	fout.Close();*/

	LoadLanguages();
}

void AddToList( List_t< Decoration^ >^ list, Decoration^ item, List_t< Ability^ >^ rel_abilities, List_t< Decoration^ >^ inf_decs )
{
	for( int i = 0; i < list->Count; ++i )
	{
		if( item->IsBetterThan( list[ i ], rel_abilities ) )
		{
			if( !list[ i ]->IsBetterThan( item, rel_abilities ) )
				list->Remove( list[ i-- ] );
		}
		else if( list[ i ]->IsBetterThan( item, rel_abilities ) )
			return;
	}
	list->Add( item );
	inf_decs->Add( item );
}

void AddToList( List_t< Armor^ >^ list, Armor^ armor, List_t< Ability^ >^ rel_abilities, List_t< Armor^ >^ inf_armor )
{
	for( int i = 0; i < list->Count; ++i )
	{
		if( armor->IsBetterThan( list[ i ], rel_abilities ) )
		{
			if( ( !armor->danger || armor->danger == list[ i ]->danger ) && !list[ i ]->IsBetterThan( armor, rel_abilities ) )
			{
				if( list[ i ]->no_skills )
					inf_armor->Remove( list[ i ] );
				list->Remove( list[ i-- ] );
			}
		}
		else if( ( !list[ i ]->danger || list[ i ]->danger == armor->danger ) && list[ i ]->IsBetterThan( armor, rel_abilities ) )
		{
			if( !armor->no_skills )
				inf_armor->Add( armor );
			return;
		}
	}
	list->Add( armor );
	inf_armor->Add( armor );
}

void GetRelevantArmors( Query^ query, List_t< Armor^ >^ rel_armor, List_t< Armor^ >^ list, List_t< Armor^ >^ inf_armor, List_t< Ability^ >^ danger_skills )
{
	unsigned max_slots = 0;
	for each( Armor^ armor in list )
	{
		//const bool blue = armor->name->IndexOf( L"Blue" ) >= 0;
		if( armor->MatchesQuery( query, danger_skills, max_slots ) )
		{
			max_slots = max_slots < armor->num_slots ? armor->num_slots : max_slots;
			AddToList( rel_armor, armor, %query->rel_abilities, inf_armor );
		}
	}
}

void GetRelevantDecorations( Query^ query )
{
	for each( Skill^ skill in query->skills )
	{
		if( !Decoration::static_decoration_ability_map.ContainsKey( skill->ability ) )
			continue;
		for each( Decoration^ decoration in Decoration::static_decoration_ability_map[ skill->ability ] )
		{
			if( decoration->MatchesQuery( query ) )
				AddToList( %query->rel_decorations, decoration, %query->rel_abilities, %query->inf_decorations );
		}
	}
	for each( Decoration^ decoration in query->rel_decorations )
		decoration->dangerous = decoration->abilities.Count == 2 && Utility::Contains( %query->rel_abilities, decoration->abilities[ 1 ]->ability );
}

int NeededPoints( List_t< Skill^ >^ skills, Ability^ ability )
{
	for each( Skill^ skill in skills )
		if( skill->ability == ability )
			return skill->points_required;
	return 0;
}

void LoadedData::GetRelevantData( Query^ query )
{
	List_t< Ability^ > danger_skills;
	//get relevant abilities
	for each( Ability^ ability in Ability::static_abilities )
	{
		if( ability->MatchesQuery( query ) )
		{
			assert( !Utility::Contains( %query->rel_abilities, ability ) );
			query->rel_abilities.Add( ability );
		}
	}
	query->rel_abilities.TrimExcess();

	//get relevant decorations
	GetRelevantDecorations( query );
	query->rel_decorations.TrimExcess();

	//get danger skills
	if( query->danger )
		for each( Decoration^ decoration in query->rel_decorations )
		{
			Ability^ bad_ability = nullptr;
			bool points20 = false;
			for each( AbilityPair^ apair in decoration->abilities )
			{
				if( apair->amount < 0 )
				{
					if( !Utility::Contains( %danger_skills, apair->ability ) )
						bad_ability = apair->ability;
				}
				else if( NeededPoints( %query->skills, apair->ability ) == 20 )
					points20 = true;
			}
			if( points20 && bad_ability )
				danger_skills.Add( bad_ability );
		}

	//get relevant armors
	for( int i = 0; i < int( Armor::ArmorType::NumArmorTypes ); ++i )
	{
		GetRelevantArmors( query, query->rel_armor[ i ], Armor::static_armors[ i ], query->inf_armor[ i ], %danger_skills );
		query->rel_armor[ i ]->TrimExcess();
	}
	query->rel_armor.TrimExcess();
}

void Load( List_t< String^ >^ vec, String^ filename )
{
	IO::StreamReader fin( filename );
	while( !fin.EndOfStream )
	{
		String^ line = fin.ReadLine();
		if( line == L"" ) continue;
		const int comment = line->IndexOf( L';' );
		if( comment == 0 ) continue;
		else if( comment != -1 )
			line = line->Substring( 0, comment );
		vec->Add( line );
	}
}

void LoadSkills( Language^ lang, String^ filename )
{
	IO::StreamReader fin( filename );
	bool space_found = false;
	while( !fin.EndOfStream )
	{
		String^ line = fin.ReadLine();
		if( line == L"" )
		{
			space_found = true;
			continue;
		}
		const int comment = line->IndexOf( L';' );
		if( comment == 0 ) continue;
		else if( comment != -1 )
			line = line->Substring( 0, comment );
		if( space_found )
			lang->skills.Add( line );
		else lang->abilities.Add( line );
	}
}

void LoadedData::LoadLanguages()
{
	languages.Clear();
	cli::array< String^ >^ dirs = IO::Directory::GetDirectories( L"Data\\Languages" );
	for each( String^ dir in dirs )
	{
		if( IO::File::Exists( dir + L"\\head.txt" ) && 
			IO::File::Exists( dir + L"\\body.txt" ) && 
			IO::File::Exists( dir + L"\\arms.txt" ) && 
			IO::File::Exists( dir + L"\\waist.txt" ) && 
			IO::File::Exists( dir + L"\\legs.txt" ) && 
			IO::File::Exists( dir + L"\\decorations.txt" ) && 
			IO::File::Exists( dir + L"\\skills.txt" ) && 
			IO::File::Exists( dir + L"\\tags.txt" ) && 
			IO::File::Exists( dir + L"\\strings.txt" ) )
		{
			languages.Add( dir->Substring( dir->LastIndexOf( L'\\' ) + 1 ) );
		}
	}
}

Language^ LoadedData::LoadLanguage( const int index )
{
	Language^ lang = gcnew Language;
	String^ dir = L"Data\\Languages\\" + languages[ index ];
	Load( %lang->decorations, dir + L"\\decorations.txt" );
	Load( %lang->components, dir + L"\\components.txt" );
	Load( lang->armors[ int( Armor::ArmorType::HEAD ) ], dir + L"\\head.txt" );
	Load( lang->armors[ int( Armor::ArmorType::BODY ) ], dir + L"\\body.txt" );
	Load( lang->armors[ int( Armor::ArmorType::ARMS ) ], dir + L"\\arms.txt" );
	Load( lang->armors[ int( Armor::ArmorType::WAIST ) ], dir + L"\\waist.txt" );
	Load( lang->armors[ int( Armor::ArmorType::LEGS ) ], dir + L"\\legs.txt" );
	Load( %lang->tags, dir + L"\\tags.txt" );
	Load( %lang->string_table, dir + L"\\strings.txt" );
	LoadSkills( lang, dir + L"\\skills.txt" );
	return lang;
}
