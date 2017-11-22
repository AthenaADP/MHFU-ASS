#include "stdafx.h"
#include "Armor.h"
#include "Skill.h"
#include "Solution.h"
#include "StringManipulation.h"
#include <fstream>

using namespace System;

bool ArmorExists( List_t< Armor^ >^ armors, Armor^ armor )
{
	for each( Armor^ existing in armors )
		if( existing->name == armor->name && existing->gender == armor->gender )
			return true;
	return false;
}

void Armor::Load( String^ filename, List_t< Armor^ >^ armors )
{
	armors->Clear();
	armors->Capacity = 512;
	
	IO::StreamReader fin( filename );
	String^ temp;
	temp = fin.ReadLine();
	temp = fin.ReadLine();
	while( !fin.EndOfStream && temp != L"" )
	{
		temp = fin.ReadLine();
		if( temp == L"" ) break;
		List_t< String^ > split;
		Utility::SplitString( %split, temp, L',' );
		Armor^ armor = gcnew Armor();
		armor->torso_inc = false;
		armor->danger = nullptr;
		//"Name","Price","Material 1","Amount","Material 2","Amount","Material 3","Amount","Material 4","Amount","Defence","Fire Res","Thunder Res","Dragon Res","Water Res","Ice Res","Gender","Hunter Type","Rarity","HR","Elder*","Slots","Skill 1","Skill Point","Skill 2","Skill Point","Skill 3","Skill Point","Skill 4","Skill Point","Skill 5","Skill Point"
		armor->name = Utility::RemoveQuotes( split[ 0 ] );
		armor->is_piercing = armor->name->Contains( L"Piercing" );
		armor->is_dummy = armor->name->Contains( L"(dummy)" );
		armor->defence = Convert::ToUInt32( split[ 10 ] );
		armor->fire_res = Convert::ToInt32( split[ 11 ] );
		armor->thunder_res = Convert::ToInt32( split[ 12 ] );
		armor->dragon_res = Convert::ToInt32( split[ 13 ] );
		armor->water_res = Convert::ToInt32( split[ 14 ] );
		armor->ice_res = Convert::ToInt32( split[ 15 ] );
		String^ split_gender = Utility::RemoveQuotes( split[ 16 ] );
		armor->gender = split_gender == L"Male" ? Gender::MALE : split_gender == L"Female" ? Gender::FEMALE : Gender::BOTH_GENDERS;
		if( ArmorExists( armors, armor ) ) continue;
		String^ split_type = Utility::RemoveQuotes( split[ 17 ] );
		armor->type = split_type == L"Blade" ? HunterType::BLADEMASTER : split_type == L"Gunner" ? HunterType::GUNNER : HunterType::BOTH_TYPES;
		armor->rarity = Convert::ToInt32( split[ 18 ] );
		Utility::FindLevelReqs( armor->hr, armor->hr_req, split[ 19 ] );
		Utility::FindLevelReqs( armor->elder_star, armor->elder_star_req, split[ 20 ] );
		armor->num_slots = Utility::CountChars( split[ 21 ], L'O' );
		armor->difficulty1 = armor->difficulty2 = armor->difficulty3 = 0;
		for( unsigned i = 1; i <= 4; ++i )
		{
			armor->AdjustDifficulty( split[ i * 2 ] );
			if( split[ i * 2 ] != L"" )
			{
				MaterialComponent^ comp = gcnew MaterialComponent;
				comp->material = /*gcnew Material;//*/Material::FindMaterial( Utility::RemoveQuotes( split[ i * 2 ] ) );
				//comp->material->name = Utility::RemoveQuotes( split[ i * 2 ] );
				comp->amount = Convert::ToInt32( split[ i * 2 + 1 ] );
				armor->components.Add( comp );
			}
		}
		
		for( unsigned i = 0; i < 5; ++i )
		{
			const unsigned index = 22 + i * 2;
			if( split[ index ] != L"" )
			{
				AbilityPair^ p = gcnew AbilityPair();
				String^ ability_name = Utility::RemoveQuotes( split[ index ] );
				p->ability = Ability::FindAbility( ability_name );
				if( split[ index + 1 ] != L"" )
					p->amount = Convert::ToInt32( split[ index + 1 ] );
				armor->abilities.Add( p );
				if( p->ability == Ability::torso_inc )
					armor->torso_inc = true;
			}
		}
		armors->Add( armor );
		if( !static_armor_map.ContainsKey( armor->name ) )
			static_armor_map.Add( armor->name, gcnew List_t< Armor^ > );
		static_armor_map[ armor->name ]->Add( armor );
	}

	fin.Close();
	armors->TrimExcess();
}

bool Armor::MatchesQuery( Query^ query, List_t< Ability^ >^ danger_skills, const unsigned max_slots )
{
	//check requirements
	if( !query->include_piercings && this->is_piercing ||
		!query->allow_dummy && this->is_dummy ||
		type != HunterType::BOTH_TYPES && query->hunter_type != type ||
		gender != Gender::BOTH_GENDERS && gender != query->gender ||
		hr > query->hr && elder_star > query->elder_star ||
		hr_req > query->hr || elder_star_req > query->elder_star ||
		torso_inc && !query->allow_torso_inc )
		return false;
	//check for torso inc
	if( torso_inc )
		return true;
	//check for danger skills
	danger = nullptr;
	for each( AbilityPair^ apair in abilities )
	{
		if( apair->amount < 0 && Utility::Contains( danger_skills, apair->ability ) )
		{
			danger = apair->ability;
			break;
		}
	}
	//check for relevant skills
	no_skills = true;
	for( int i = 0; i < abilities.Count; ++i )
	{
		if( abilities[ i ]->amount > 0 )
		{
			Ability^ ability = abilities[ i ]->ability;
			Ability::SkillMap_t::Enumerator iter = ability->skills.GetEnumerator();
			while( iter.MoveNext() )
			{
				if( iter.Current.Key > 0 && Utility::Contains( %query->skills, iter.Current.Value ) )
				{
					no_skills = false;
					return true;
				}
			}
		}
	}
	
	return num_slots >= max_slots && !danger;
}

bool Armor::IsBetterThan( Armor^ other, List_t< Ability^ >^ rel_abilities )
{
	if( num_slots > other->num_slots /*||
		no_skills && !other->no_skills */) return true;
	if( no_skills && other->no_skills )
	{
		return this->defence == other->defence ? this->rarity > other->rarity : this->defence > other->defence;
	}
	if( this->torso_inc && other->torso_inc )
	{
		return this->rarity == other->rarity ? this->defence > other->defence : this->rarity > other->rarity;
	}
	else if( this->torso_inc || other->torso_inc )
		return true;
	for each( Ability^ ability in rel_abilities )
	{
		if( this->GetSkillAt( ability ) > other->GetSkillAt( ability ) )
			return true;
	}
	return false;
}

int Armor::GetSkillAt( Ability^ ability )
{
	for each( AbilityPair^ apair in abilities )
		if( apair->ability == ability )
			return apair->amount;
	return 0;
}

void Armor::AdjustDifficulty( System::String^ _material )
{
	String^ material = _material->Trim( L'\"' );
	if( material == L"Elder Dragon Jwl" )
		difficulty3 += 1;
	else if( material == L"RathlosDvnePlate" ||
		material == L"TigrexDvinePlate" ||
		material == L"RathianDvnePlate" ||
		material == L"NarugaDvinePlate" ||
		material == L"GaorenDvinePlate" ||
		material == L"Ucamu Divine Jwl" ||
		material == L"LaoS Dvine Plate" ||
		material == L"BlkGravDvnePlate" ||
		material == L"GraviosDvnePlate" )
		difficulty3 += 1;
	else if( material == L"Lao-Shan's Ruby" ||
		material == L"Rathalos Ruby" ||
		material == L"Rathian Ruby" ||
		material == L"Fire Drgn Jwl" ||
		material == L"Chameleos Jewel" ||
		material == L"Daora Jewel" )
		difficulty2 += 1;
	else if( material == L"Rathalos Plate" ||
		material == L"Rathian Plate" )
		difficulty1 += 1;
}

Armor^ Armor::FindArmor( System::String^ name )
{
	if( static_armor_map.ContainsKey( name ) )
		for each( Armor^ armor in static_armor_map[ name ] )
			if( armor->name == name )
				return armor;
	return nullptr;
}
bool ArmorEquivalence::Matches( Armor^ armor )
{
	if( armor->num_slots != num_slots ) return false;
	if( torso_inc ) return armor->torso_inc;
	else if( armor->torso_inc ) return false;
	if( no_skills ) return armor->no_skills;
	else if( armor->no_skills ) return false;
	for each( AbilityPair^ apair in abilities )
		if( armor->GetSkillAt( apair->ability ) != apair->amount )
			return false;
	return true;
}

ArmorEquivalence::ArmorEquivalence( Armor^ armor, List_t< Ability^ >^ rel_abilities )
{
	armors.Add( armor );
	num_slots = armor->num_slots;

	no_skills = armor->no_skills;
	torso_inc = armor->torso_inc;

	if( no_skills || torso_inc ) return;

	armor->extra_skills.Clear();
	armor->eq_skills.Clear();
	for each( AbilityPair^ apair in armor->abilities )
	{
		if( Utility::Contains( rel_abilities, apair->ability ) )
		{
			abilities.Add( apair );
			armor->eq_skills.Add( apair );
		}
		else armor->extra_skills.Add( apair );
	}
}

void ArmorEquivalence::Add( Armor^ armor, List_t< Ability^ >^ rel_abilities )
{
	armor->extra_skills.Clear();
	armor->eq_skills.Clear();
	for each( AbilityPair^ apair in armor->abilities )
	{
		if( Utility::Contains( rel_abilities, apair->ability ) )
			armor->eq_skills.Add( apair );
		else armor->extra_skills.Add( apair );
	}
	armors.Add( armor );
}
