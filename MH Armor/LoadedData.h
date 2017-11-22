#include <vcclr.h>

#include "Armor.h"
#include "Decoration.h"
#include "Solution.h"
#include "Language.h"

ref struct SkillTag
{
	System::String^ name;

	SkillTag( System::String^ s ) : name( s ) {}
};

ref class LoadedData
{
public:
	List_t< SkillTag^ > tags;
	List_t< System::String^ > languages;

	void ImportTextFiles();
	void LoadLanguages();
	Language^ LoadLanguage( const int index );

	Skill^ FindSkill( const unsigned skill );

	void GetRelevantData( Query^ query );
};
