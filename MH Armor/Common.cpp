#include "stdafx.h"
#include "Common.h"
#include "Armor.h"
#include "Decoration.h"

using namespace System;

namespace Utility
{
	String^ RemoveQuotes( String^ in_out )
	{
		if( in_out == L"" ) return in_out;
		in_out = in_out->Trim();
		if( in_out[ 0 ] == L'\"' ) in_out = in_out->Substring( 1 );
		if( in_out[ in_out->Length - 1 ] == L'\"' ) in_out = in_out->Substring( 0, in_out->Length - 1 );
		return in_out;
	}

	void SplitString( List_t< String^ >^ vec, String^ str, const wchar_t c )
	{
		str = str->Trim();
		int last_non_delim = 0;
		for( int i = 0; i < str->Length; ++i )
		{
			if( str[ i ] == c )
			{
				String^ substr = str->Substring( last_non_delim, i - last_non_delim );
				RemoveQuotes( substr );
				vec->Add( substr );
				last_non_delim = i + 1;
			}
		}
		str = str->Substring( last_non_delim, str->Length - last_non_delim );
		RemoveQuotes( str );
		vec->Add( str );
	}

	bool ContainsString( List_t< String^ >% vec, String^ item )
	{
		for( int i = 0; i < vec.Count; ++i )
			if( vec[ i ] == item ) return true;
		return false;
	}

	void FindLevelReqs( unsigned% available, unsigned% required, String^ input )
	{
		if( input->Length > 0 && input[ 0 ] == L'\"' ) input = Utility::RemoveQuotes( input );
		const int exclamation_point = input->LastIndexOf( L'!' );
		if( exclamation_point == -1 )
		{
			required = 0;
			available = Convert::ToUInt32( input );
			return;
		}
		required = Convert::ToUInt32( input->Substring( 0, exclamation_point ) );
		if( exclamation_point < input->Length - 1 )
			available = Convert::ToUInt32( input->Substring( exclamation_point + 1 ) );
	}

	String^ SlotString( const unsigned slots )
	{
		return slots == 0 ? L"---" :
			slots == 1 ? L"O--" :
			slots == 2 ? L"OO-" : L"OOO";
	}

	unsigned CountChars( String^ str, const wchar_t c )
	{
		unsigned total = 0;
		for( int i = 0; i < str->Length; ++i )
			if( str[ i ] == c ) ++total;
		return total;
	}

	void UpdateContextMenu( System::Windows::Forms::ContextMenuStrip^ strip, List_t< AbilityPair^ >% abilities )
	{
		for each( AbilityPair^ apair in abilities )
		{
			if( apair->amount != 0 )
				strip->Items->Add( Convert::ToString( apair->amount ) + L" " + apair->ability->name );
			else
				strip->Items->Add( apair->ability->name );
		}
	}

	void UpdateContextMenu( System::Windows::Forms::ContextMenuStrip^ strip, List_t< MaterialComponent^ >^ components )
	{
		strip->Items->Add( L"-" );
		for each( MaterialComponent^ part in components )
			strip->Items->Add( Convert::ToString( part->amount ) + L"x " + part->material->name );
	}

	void UpdateContextMenu( System::Windows::Forms::ContextMenuStrip^ strip, Decoration^ decoration )
	{
		strip->Items->Add( decoration->name );
		strip->Items->Add( L"-" );
		strip->Items->Add( SlotString( decoration->slots_required ) );
		strip->Items->Add( L"-" );
		UpdateContextMenu( strip, decoration->abilities );
		UpdateContextMenu( strip, %decoration->components );
		if( decoration->components2.Count > 0 )
			UpdateContextMenu( strip, %decoration->components2 );
	}

	void UpdateContextMenu( System::Windows::Forms::ContextMenuStrip^ strip, Armor^ armor, List_t< String^ >^ strings )
	{
		strip->Items->Add( armor->name );
		strip->Items->Add( L"-" );
		strip->Items->Add( SlotString( armor->num_slots ) );
		strip->Items->Add( L"-" );
		strip->Items->Add( strings[ StringTable::BaseDefence ] + L": " + Convert::ToString( armor->defence ) );
		strip->Items->Add( strings[ StringTable::DragonRes ] + L": " + Convert::ToString( armor->dragon_res ) );
		strip->Items->Add( strings[ StringTable::FireRes ] + L": " + Convert::ToString( armor->fire_res ) );
		strip->Items->Add( strings[ StringTable::IceRes ] + L": " + Convert::ToString( armor->ice_res ) );
		strip->Items->Add( strings[ StringTable::ThunderRes ] + L": " + Convert::ToString( armor->thunder_res ) );
		strip->Items->Add( strings[ StringTable::WaterRes ] + L": " + Convert::ToString( armor->water_res ) );
		strip->Items->Add( L"-" );
		UpdateContextMenu( strip, armor->abilities );
		UpdateContextMenu( strip, %armor->components );
	}
}

void Material::LoadMaterials( System::String^ filename )
{
	static_materials.Clear();
	IO::StreamReader fin( filename );
	while( !fin.EndOfStream )
	{
		String^ line = fin.ReadLine();
		if( line == L"" ) continue;
		Material^ mat = gcnew Material;
		mat->name = line;
		static_materials.Add( mat );
	}
	fin.Close();
	static_material_map.Clear();
	for each( Material^ mat in static_materials )
		static_material_map.Add( mat->name, mat );
}

Material^ Material::FindMaterial( System::String^ name )
{
	if( static_material_map.ContainsKey( name ) )
		return static_material_map[ name ];
	return nullptr;
}