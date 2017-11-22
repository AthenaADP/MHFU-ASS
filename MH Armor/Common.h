#pragma once
#include <cassert>

enum class HunterType { BLADEMASTER = 1, GUNNER, BOTH_TYPES };
enum class Gender { MALE = 4, FEMALE, BOTH_GENDERS };

#define List_t System::Collections::Generic::List
#define Map_t System::Collections::Generic::Dictionary

ref struct Armor;
ref struct Decoration;

namespace Utility
{

	template< class T >
	bool Contains( List_t< T^ >^ cont, const T^ val )
	{
		for( int i = 0; i < cont->Count; ++i )
			if( cont[ i ] == val ) return true;
		return false;
	}

	template< class T >
	int GetIndexOf( List_t< T^ >^ cont, const T^ val )
	{
		for( int i = 0; i < cont->Count; ++i )
			if( cont[ i ] == val ) return i;
		return -1;
	}

	template< class T >
	T^ FindByName( List_t< T^ >^ cont, System::String^ name )
	{
		for each( T^ item in cont )
		{
			if( item->name == name )
				return item;
		}
		return nullptr;
	}

	System::String^ SlotString( const unsigned slots );
	unsigned CountChars( System::String^ str, const wchar_t c );

	bool ContainsString( List_t< System::String^ >% vec, System::String^ item );

	void SplitString( List_t< System::String^ >^ vec, System::String^ str, const wchar_t c );

	void FindLevelReqs( unsigned% available, unsigned% required, System::String^ input );

	System::String^ RemoveQuotes( System::String^ in_out );

	inline unsigned Min( const unsigned a, const unsigned b ) { return a > b ? b : a; }

	void UpdateContextMenu( System::Windows::Forms::ContextMenuStrip^ strip, Decoration^ decoration );
	void UpdateContextMenu( System::Windows::Forms::ContextMenuStrip^ strip, Armor^ armor, List_t< System::String^ >^ strings );
}

ref struct Material
{
	System::String^ name;

	static List_t< Material^ > static_materials;
	static Material^ FindMaterial( System::String^ name );
	static void LoadMaterials( System::String^ filename );
	static Map_t< System::String^, Material^ > static_material_map;
};

ref struct MaterialComponent
{
	Material^ material;
	unsigned amount;
};

namespace StringTable
{
	enum { OrAnythingWithTorsoInc = 0, OrAnythingWith, Slots, ShowingFirst, SolutionsOnly, SolutionsFound, SlotSpare, SlotsSpare,
		SelectArmor, Search, Default, None, DragonRes, FireRes, IceRes, ThunderRes, WaterRes, BaseDefence, MaxDefence };
}
