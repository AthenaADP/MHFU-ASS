#pragma once
#include "LoadedData.h"
#include "frmAbout.h"
#include "frmAdvanced.h"
#include "frmFind.h"
#include "Version.h"

namespace MHArmor 
{
	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

#pragma region Comparison Functions
	int Compare( const int x, const int y )
	{
		return x < y ? -1 : y < x ? 1 : 0;
	}

	int ReverseCompare( const int x, const int y )
	{
		return x < y ? 1 : y < x ? -1 : 0;
	}

	int CompareSolutionByDefence( Solution^ s1, Solution^ s2 )
	{
		return ReverseCompare( s1->defence, s2->defence );
	}

	int CompareSolutionByFireRes( Solution^ s1, Solution^ s2 )
	{
		return ReverseCompare( s1->fire_res, s2->fire_res );
	}

	int CompareSolutionByIceRes( Solution^ s1, Solution^ s2 )
	{
		return ReverseCompare( s1->ice_res, s2->ice_res );
	}

	int CompareSolutionByWaterRes( Solution^ s1, Solution^ s2 )
	{
		return ReverseCompare( s1->water_res, s2->water_res );
	}

	int CompareSolutionByThunderRes( Solution^ s1, Solution^ s2 )
	{
		return ReverseCompare( s1->thunder_res, s2->thunder_res );
	}

	int CompareSolutionByDragonRes( Solution^ s1, Solution^ s2 )
	{
		return ReverseCompare( s1->dragon_res, s2->dragon_res );
	}

	int CompareSolutionByRarity( Solution^ s1, Solution^ s2 )
	{
		return ReverseCompare( s1->rarity, s2->rarity );
	}

	int CompareSolutionByDifficulty( Solution^ s1, Solution^ s2 )
	{
		return Compare( s1->difficulty, s2->difficulty );
	}

	int CompareSolutionBySlotsSpare( Solution^ s1, Solution^ s2 )
	{
		return ReverseCompare( s1->total_slots_spare, s2->total_slots_spare );
	}
#pragma endregion
	/// <summary>
	/// Summary for Form1
	///
	/// WARNING: If you change the name of this class, you will need to change the
	///          'Resource File Name' property for the managed resource compiler tool
	///          associated with all .resx files this class depends on.  Otherwise,
	///          the designers will not be able to interact properly with localized
	///          resources associated with this form.
	/// </summary>
	public ref class Form1 : public System::Windows::Forms::Form
	{
		const static int MAX_LIMIT = 1000;
		const static int NumSkills = 5;
		static Threading::Mutex^ progress_mutex = gcnew Threading::Mutex;
		static Threading::Mutex^ results_mutex = gcnew Threading::Mutex;
		String^ CFG_FILE;
		String^ endl;
		String^ last_result;
		bool lock_skills, sort_off, can_save, last_search_gunner;
		LoadedData^ data;
		String^ last_lang;
		Query^ query;
		frmFind^ find_dialog;
		frmAdvanced::Result_t^ blast_options;
		frmAdvanced::Result_t^ glast_options;
		typedef Generic::Dictionary< unsigned, unsigned > IndexMap;

		List_t< Solution^ > final_solutions;
		List_t< String^ > strings;

		List_t< ComboBox^ > bSkills, gSkills;
		List_t< ComboBox^ > bSkillFilters, gSkillFilters;
		List_t< IndexMap^ > bIndexMaps, gIndexMaps;
#pragma region Members
	private: System::Windows::Forms::MenuStrip^  menuStrip1;
	private: System::Windows::Forms::ToolStripMenuItem^  fileToolStripMenuItem;
	private: System::Windows::Forms::ToolStripMenuItem^  exitToolStripMenuItem;
	private: System::Windows::Forms::ToolStripMenuItem^  dataToolStripMenuItem;
	private: System::Windows::Forms::ToolStripMenuItem^  reloadToolStripMenuItem;
	private: System::Windows::Forms::ToolStripMenuItem^  helpToolStripMenuItem;
	private: System::Windows::Forms::ToolStripMenuItem^  aboutToolStripMenuItem;
	private: System::Windows::Forms::ToolStripMenuItem^  mnuLanguages;
	private: System::Windows::Forms::GroupBox^  groupBox6;
	private: System::Windows::Forms::RadioButton^  rdoFemale;
	private: System::Windows::Forms::RadioButton^  rdoMale;
	private: System::Windows::Forms::CheckBox^  chkBadSkills;
	private: System::Windows::Forms::CheckBox^  chkPiercings;
	private: System::Windows::Forms::CheckBox^  chkTorsoInc;
	private: System::Windows::Forms::GroupBox^  groupBox7;
	private: System::Windows::Forms::Button^  btnAdvancedSearch;
	private: System::Windows::Forms::TabControl^  tabHunterType;
	private: System::Windows::Forms::TabPage^  tabBlademaster;
	private: System::Windows::Forms::TabPage^  tabGunner;
	private: System::Windows::Forms::GroupBox^  grpGSkillFilters;
	private: System::Windows::Forms::GroupBox^  grpGSkills;
	private: System::Windows::Forms::NumericUpDown^  nudHR;
	private: System::Windows::Forms::NumericUpDown^  nudWeaponSlots;
	private: System::Windows::Forms::NumericUpDown^  nudElder;
	private: System::Windows::Forms::GroupBox^  groupBox1;
	private: System::Windows::Forms::Label^  lblHR;
	private: System::Windows::Forms::Label^  lblElder;
	private: System::Windows::Forms::Label^  lblSlots;
	private: System::Windows::Forms::Button^  btnCancel;
	private: System::Windows::Forms::GroupBox^  grpBSkills;
	private: System::Windows::Forms::Button^  btnSearch;
	private: System::Windows::Forms::ProgressBar^  progressBar1;
	private: System::Windows::Forms::RichTextBox^  txtSolutions;
	private: System::Windows::Forms::GroupBox^  groupBox4;
	private: System::Windows::Forms::GroupBox^  grpResults;
	private: System::Windows::Forms::GroupBox^  grpBSkillFilters;
	private: System::Windows::Forms::CheckBox^  chkDummy;
	private: System::Windows::Forms::GroupBox^  grpSort;
	private: System::Windows::Forms::ComboBox^  cmbSort;
	private: System::Windows::Forms::CheckBox^  chkUkanlos;
	private: System::Windows::Forms::ContextMenuStrip^  contextMenuStrip1;
#pragma endregion

		List_t< BackgroundWorker^ >  workers;
		unsigned finished_workers, total_progress;

		void ClearFilters()
		{
			for each( ComboBox^ box in bSkillFilters )
				box->Items->Clear();
			for each( ComboBox^ box in gSkillFilters )
				box->Items->Clear();
		}
	
		void AddFilter( System::String^ str )
		{
			for each( ComboBox^ box in bSkillFilters )
				box->Items->Add( str );
			for each( ComboBox^ box in gSkillFilters )
				box->Items->Add( str );
		}

		void InitFilters()
		{
			ClearFilters();
			for each( SkillTag^ tag in data->tags )
				AddFilter( tag->name );
			for each( ComboBox^ box in gSkillFilters )
				box->SelectedIndex = 0;
			for each( ComboBox^ box in bSkillFilters )
				box->SelectedIndex = 0;
		}

		unsigned GetDifficulty()
		{
			return (int)nudHR->Value > 6 ? 3 : (int)nudHR->Value > 3 || (int)nudElder->Value > 6 ? 2 : 1;
		}

		void ResetSkill( ComboBox^ box, IndexMap^ map, Skill^ skill )
		{
			if( skill == nullptr ) return;
			IndexMap::Enumerator iter = map->GetEnumerator();
			while( iter.MoveNext() )
			{
				if( Skill::static_skills[ iter.Current.Value ] == skill )
				{
					box->SelectedIndex = iter.Current.Key;
					return;
				}
			}
		}

		void InitSkills( ComboBox^ box, IndexMap^ map, const int filter, List_t< Ability^ >^ disallowed )
		{
			map->Clear();
			box->SelectedIndex = -1;
			box->Items->Clear();
			if( filter == -1 ) return;
			if( strings[ 11 ] == L"" )
				box->Items->Add( L"(none)" );
			else box->Items->Add( L"(" + strings[ 11 ] + L")" );
			for( int i = 0; i < Skill::static_skills.Count; ++i )
			{
				Skill^ skill = Skill::static_skills[ i ];
				if( skill->points_required <= 0 || 
					filter == 1 && !skill->ability->tags.Count == 0 ||
					Utility::Contains( disallowed, skill->ability ) )
					continue;

				if( filter == 0 || filter == 1 && skill->ability->tags.Count == 0 || 
					!!Utility::FindByName( %skill->ability->tags, data->tags[ filter ]->name ) )
				{
					map[ box->Items->Count ] = i;
					box->Items->Add( skill->name );
				}
			}
		}

		void InitSkills()
		{
			for( unsigned i = 0; i < NumSkills; ++i )
			{
				InitSkills( gSkills[ i ], gIndexMaps[ i ], gSkillFilters[ i ]->SelectedIndex, gcnew List_t< Ability^ > );
				InitSkills( bSkills[ i ], bIndexMaps[ i ], bSkillFilters[ i ]->SelectedIndex, gcnew List_t< Ability^ > );
			}
		}

		ComboBox^ GetNewComboBox( const unsigned width, const unsigned i )
		{
			ComboBox^ box = gcnew ComboBox;
			box->Location = System::Drawing::Point( 6, 19 + i * 27 );
			box->Size = System::Drawing::Size( width, 21 );
			box->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			return box;
		}

		void InitializeComboBoxes()
		{
			for( unsigned i = 0; i < NumSkills; ++i )
			{
				gSkillFilters.Add( GetNewComboBox( 134, i ) );
				bSkillFilters.Add( GetNewComboBox( 134, i ) );
				gSkillFilters[ i ]->SelectedIndexChanged += gcnew System::EventHandler(this, &Form1::cmbSkillFilter_SelectedIndexChanged);
				bSkillFilters[ i ]->SelectedIndexChanged += gcnew System::EventHandler(this, &Form1::cmbSkillFilter_SelectedIndexChanged);
				grpGSkillFilters->Controls->Add( gSkillFilters[ i ] );
				grpBSkillFilters->Controls->Add( bSkillFilters[ i ] );

				gSkills.Add( GetNewComboBox( 152, i ) );
				bSkills.Add( GetNewComboBox( 152, i ) );
				gSkills[ i ]->SelectedIndexChanged += gcnew System::EventHandler(this, &Form1::cmbSkill_SelectedIndexChanged);
				bSkills[ i ]->SelectedIndexChanged += gcnew System::EventHandler(this, &Form1::cmbSkill_SelectedIndexChanged);
				grpGSkills->Controls->Add( gSkills[ i ] );
				grpBSkills->Controls->Add( bSkills[ i ] );

				gIndexMaps.Add( gcnew IndexMap );
				bIndexMaps.Add( gcnew IndexMap );
			}
			cmbSort->SelectedIndex = 0;
		}
	public:

		Form1(void) : CFG_FILE( L"settings.cfg" ), endl( L"\r\n" )
		{
			sort_off = false;
			can_save = true;
			last_search_gunner = false;
			InitializeComponent();
			InitializeComboBoxes();

			for( unsigned i = 0; i < 18; ++i )
				strings.Add( L"" );
			
			data = gcnew LoadedData();
			data->ImportTextFiles();
			mnuLanguages->DropDownItems->Clear();
			for each( String^ lang in data->languages )
			{
				ToolStripMenuItem^ item = gcnew ToolStripMenuItem( lang );
				item->Click += gcnew System::EventHandler(this, &Form1::LanguageSelect_Click);
				mnuLanguages->DropDownItems->Add( item );
			}

			InitFilters();
			InitSkills();
		
			lock_skills = false;
			btnCancel->Enabled = false;

			LoadConfig();

			Text += " v" +  STRINGIZE( VERSION_NO ) ;
		}

		void AddSolution( String^ line )
		{
			List_t< String^ > split;
			Utility::SplitString( %split, line, L' ' );
			Solution^ solution = gcnew Solution;
			for( unsigned i = 0; i < 5; ++i )
				solution->armors.Add( Armor::static_armors[ i ][ Convert::ToUInt32( split[ i ] ) ] );
			const unsigned num_decorations = Convert::ToUInt32( split[ 6 ] );
			for( unsigned i = 0; i < num_decorations; ++i )
				solution->decorations.Add( Decoration::static_decorations[ Convert::ToUInt32( split[ 7 + i ] ) ] );
			
			const unsigned num_skills = Convert::ToUInt32( split[ 7 + num_decorations ] );
			for( unsigned i = 0; i < num_skills; ++i )
				solution->extra_skills.Add( Skill::static_skills[ Convert::ToUInt32( split[ 8 + num_decorations + i ] ) ] );
			solution->CalculateData( GetDifficulty() );
			solution->total_slots_spare = Convert::ToUInt32( split[ 5 ] );
			final_solutions.Add( solution );
		}

		void LoadConfig()
		{
			can_save = false;
			if( IO::File::Exists( CFG_FILE ) )
			{
				IO::StreamReader fin( CFG_FILE );
				String^ version = fin.ReadLine();
				if( version != L"3" )
				{
					fin.Close();
					can_save = true;
					return;
				}
				else
				{
					String^ lang = fin.ReadLine();
					if( lang == L"" )
						LoadLanguage( last_lang = L"English MHFU" );
					else LoadLanguage( lang );

					last_search_gunner = Convert::ToBoolean( fin.ReadLine() );
					const int hunter_type = Convert::ToInt32( fin.ReadLine() );
					rdoMale->Checked = Convert::ToBoolean( fin.ReadLine() );
					rdoFemale->Checked = !rdoMale->Checked;
					chkBadSkills->Checked = Convert::ToBoolean( fin.ReadLine() );
					chkPiercings->Checked = Convert::ToBoolean( fin.ReadLine() );
					chkTorsoInc->Checked = Convert::ToBoolean( fin.ReadLine() );
					chkDummy->Checked = Convert::ToBoolean( fin.ReadLine() );
					chkUkanlos->Checked = Convert::ToBoolean( fin.ReadLine() );
					cmbSort->SelectedIndex = Convert::ToInt32( fin.ReadLine() );

					nudHR->Value = Convert::ToInt32( fin.ReadLine() );
					nudElder->Value = Convert::ToInt32( fin.ReadLine() );
					nudWeaponSlots->Value = Convert::ToInt32( fin.ReadLine() );

					tabHunterType->SuspendLayout();
					for( unsigned i = 0; i < NumSkills; ++i )
					{
						tabHunterType->SelectedIndex = 0;
						bSkillFilters[ i ]->SelectedIndex = Convert::ToInt32( fin.ReadLine() );
						bSkills[ i ]->SelectedIndex = SearchIndexMap( bIndexMaps[ i ], Convert::ToInt32( fin.ReadLine() ) );
						tabHunterType->SelectedIndex = 1;
						gSkillFilters[ i ]->SelectedIndex = Convert::ToInt32( fin.ReadLine() );
						gSkills[ i ]->SelectedIndex = SearchIndexMap( gIndexMaps[ i ], Convert::ToInt32( fin.ReadLine() ) );
					}
					tabHunterType->SelectedIndex = hunter_type;
					tabHunterType->ResumeLayout();

					FormulateQuery( false, last_search_gunner );
					
					while( !fin.EndOfStream )
						AddSolution( fin.ReadLine() );
					last_result = nullptr;
					UpdateResultString( %final_solutions );

					fin.Close();
					can_save = true;
					SaveConfig();
					return;
				}	
			}
			LoadLanguage( last_lang = L"English MHFU" );
			can_save = true;
			SaveConfig();
		}

		void SaveConfig()
		{
			if( !can_save ) return;

			IO::StreamWriter fout( CFG_FILE );
			fout.WriteLine( L"3" );
			fout.WriteLine( last_lang );
			fout.WriteLine( last_search_gunner );
			fout.WriteLine( tabHunterType->SelectedIndex );
			fout.WriteLine( rdoMale->Checked );
			fout.WriteLine( chkBadSkills->Checked );
			fout.WriteLine( chkPiercings->Checked );
			fout.WriteLine( chkTorsoInc->Checked );
			fout.WriteLine( chkDummy->Checked );
			fout.WriteLine( chkUkanlos->Checked );
			fout.WriteLine( cmbSort->SelectedIndex );
			fout.WriteLine( nudHR->Value );
			fout.WriteLine( nudElder->Value );
			fout.WriteLine( nudWeaponSlots->Value );
			for( unsigned i = 0; i < NumSkills; ++i )
			{
				fout.WriteLine( bSkillFilters[ i ]->SelectedIndex );
				if( bSkills[ i ]->SelectedIndex == -1 )
					fout.WriteLine( -1 );
				else fout.WriteLine( bIndexMaps[ i ][ bSkills[ i ]->SelectedIndex ] );
				fout.WriteLine( gSkillFilters[ i ]->SelectedIndex );
				if( gSkills[ i ]->SelectedIndex == -1 )
					fout.WriteLine( -1 );
				else fout.WriteLine( gIndexMaps[ i ][ gSkills[ i ]->SelectedIndex ] );
			}
			for each( Solution^ solution in final_solutions )
			{
				for( unsigned i = 0; i < 5; ++i )
				{
					fout.Write( Convert::ToString( Utility::GetIndexOf( Armor::static_armors[ i ], solution->armors[ i ] ) ) );
					fout.Write( L" " );
				}
				fout.Write( Convert::ToString( solution->total_slots_spare ) );
				fout.Write( L" " );
				fout.Write( Convert::ToString( solution->decorations.Count ) );
				fout.Write( L" " );
				for each( Decoration^ decoration in solution->decorations )
				{
					fout.Write( Convert::ToString( Utility::GetIndexOf( %Decoration::static_decorations, decoration ) ) );
					fout.Write( L" " );
				}
				fout.Write( Convert::ToString( solution->extra_skills.Count ) );
				fout.Write( L" " );
				for each( Skill^ skill in solution->extra_skills )
				{
					fout.Write( Convert::ToString( Utility::GetIndexOf( %Skill::static_skills, skill ) ) );
					fout.Write( L" " );
				}
				fout.WriteLine();
			}
			fout.Close();
		}

		int SearchIndexMap( IndexMap^ imap, int skill_index )
		{
			for each( Generic::KeyValuePair< unsigned, unsigned > kvp in imap )
			{
				if( kvp.Value == skill_index )
					return kvp.Key;
			}
			return -1;
		}

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~Form1()
		{
			SaveConfig();
		}
private: System::ComponentModel::IContainer^  components;
protected: 

	protected: 

	private:
		/// <summary>
		/// Required designer variable.
		/// </summary>


#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
			this->components = (gcnew System::ComponentModel::Container());
			this->nudHR = (gcnew System::Windows::Forms::NumericUpDown());
			this->groupBox1 = (gcnew System::Windows::Forms::GroupBox());
			this->nudWeaponSlots = (gcnew System::Windows::Forms::NumericUpDown());
			this->lblElder = (gcnew System::Windows::Forms::Label());
			this->lblSlots = (gcnew System::Windows::Forms::Label());
			this->nudElder = (gcnew System::Windows::Forms::NumericUpDown());
			this->lblHR = (gcnew System::Windows::Forms::Label());
			this->grpBSkills = (gcnew System::Windows::Forms::GroupBox());
			this->btnSearch = (gcnew System::Windows::Forms::Button());
			this->progressBar1 = (gcnew System::Windows::Forms::ProgressBar());
			this->txtSolutions = (gcnew System::Windows::Forms::RichTextBox());
			this->contextMenuStrip1 = (gcnew System::Windows::Forms::ContextMenuStrip(this->components));
			this->groupBox4 = (gcnew System::Windows::Forms::GroupBox());
			this->btnAdvancedSearch = (gcnew System::Windows::Forms::Button());
			this->btnCancel = (gcnew System::Windows::Forms::Button());
			this->grpResults = (gcnew System::Windows::Forms::GroupBox());
			this->grpBSkillFilters = (gcnew System::Windows::Forms::GroupBox());
			this->menuStrip1 = (gcnew System::Windows::Forms::MenuStrip());
			this->fileToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->exitToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->dataToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->reloadToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->mnuLanguages = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->helpToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->aboutToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->groupBox6 = (gcnew System::Windows::Forms::GroupBox());
			this->rdoFemale = (gcnew System::Windows::Forms::RadioButton());
			this->rdoMale = (gcnew System::Windows::Forms::RadioButton());
			this->chkBadSkills = (gcnew System::Windows::Forms::CheckBox());
			this->chkPiercings = (gcnew System::Windows::Forms::CheckBox());
			this->chkTorsoInc = (gcnew System::Windows::Forms::CheckBox());
			this->groupBox7 = (gcnew System::Windows::Forms::GroupBox());
			this->chkUkanlos = (gcnew System::Windows::Forms::CheckBox());
			this->chkDummy = (gcnew System::Windows::Forms::CheckBox());
			this->tabHunterType = (gcnew System::Windows::Forms::TabControl());
			this->tabBlademaster = (gcnew System::Windows::Forms::TabPage());
			this->tabGunner = (gcnew System::Windows::Forms::TabPage());
			this->grpGSkillFilters = (gcnew System::Windows::Forms::GroupBox());
			this->grpGSkills = (gcnew System::Windows::Forms::GroupBox());
			this->grpSort = (gcnew System::Windows::Forms::GroupBox());
			this->cmbSort = (gcnew System::Windows::Forms::ComboBox());
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->nudHR))->BeginInit();
			this->groupBox1->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->nudWeaponSlots))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->nudElder))->BeginInit();
			this->groupBox4->SuspendLayout();
			this->grpResults->SuspendLayout();
			this->menuStrip1->SuspendLayout();
			this->groupBox6->SuspendLayout();
			this->groupBox7->SuspendLayout();
			this->tabHunterType->SuspendLayout();
			this->tabBlademaster->SuspendLayout();
			this->tabGunner->SuspendLayout();
			this->grpSort->SuspendLayout();
			this->SuspendLayout();
			// 
			// nudHR
			// 
			this->nudHR->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Right));
			this->nudHR->Location = System::Drawing::Point(155, 20);
			this->nudHR->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) {9, 0, 0, 0});
			this->nudHR->Minimum = System::Decimal(gcnew cli::array< System::Int32 >(4) {1, 0, 0, 0});
			this->nudHR->Name = L"nudHR";
			this->nudHR->Size = System::Drawing::Size(35, 20);
			this->nudHR->TabIndex = 2;
			this->nudHR->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) {9, 0, 0, 0});
			this->nudHR->ValueChanged += gcnew System::EventHandler(this, &Form1::HRChanged);
			// 
			// groupBox1
			// 
			this->groupBox1->Controls->Add(this->nudWeaponSlots);
			this->groupBox1->Controls->Add(this->nudHR);
			this->groupBox1->Controls->Add(this->lblElder);
			this->groupBox1->Controls->Add(this->lblSlots);
			this->groupBox1->Controls->Add(this->nudElder);
			this->groupBox1->Controls->Add(this->lblHR);
			this->groupBox1->Location = System::Drawing::Point(12, 27);
			this->groupBox1->Name = L"groupBox1";
			this->groupBox1->Size = System::Drawing::Size(200, 115);
			this->groupBox1->TabIndex = 5;
			this->groupBox1->TabStop = false;
			// 
			// nudWeaponSlots
			// 
			this->nudWeaponSlots->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Right));
			this->nudWeaponSlots->Location = System::Drawing::Point(155, 80);
			this->nudWeaponSlots->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) {3, 0, 0, 0});
			this->nudWeaponSlots->Name = L"nudWeaponSlots";
			this->nudWeaponSlots->Size = System::Drawing::Size(35, 20);
			this->nudWeaponSlots->TabIndex = 7;
			// 
			// lblElder
			// 
			this->lblElder->AutoSize = true;
			this->lblElder->Location = System::Drawing::Point(11, 52);
			this->lblElder->Name = L"lblElder";
			this->lblElder->Size = System::Drawing::Size(65, 13);
			this->lblElder->TabIndex = 4;
			this->lblElder->Text = L"Village Elder";
			// 
			// lblSlots
			// 
			this->lblSlots->AutoSize = true;
			this->lblSlots->Location = System::Drawing::Point(11, 82);
			this->lblSlots->Name = L"lblSlots";
			this->lblSlots->Size = System::Drawing::Size(97, 13);
			this->lblSlots->TabIndex = 1;
			this->lblSlots->Text = L"Max Weapon Slots";
			// 
			// nudElder
			// 
			this->nudElder->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Right));
			this->nudElder->Location = System::Drawing::Point(155, 50);
			this->nudElder->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) {9, 0, 0, 0});
			this->nudElder->Minimum = System::Decimal(gcnew cli::array< System::Int32 >(4) {1, 0, 0, 0});
			this->nudElder->Name = L"nudElder";
			this->nudElder->Size = System::Drawing::Size(35, 20);
			this->nudElder->TabIndex = 5;
			this->nudElder->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) {9, 0, 0, 0});
			this->nudElder->ValueChanged += gcnew System::EventHandler(this, &Form1::DeleteOptions);
			// 
			// lblHR
			// 
			this->lblHR->AutoSize = true;
			this->lblHR->Location = System::Drawing::Point(10, 22);
			this->lblHR->Name = L"lblHR";
			this->lblHR->Size = System::Drawing::Size(23, 13);
			this->lblHR->TabIndex = 6;
			this->lblHR->Text = L"HR";
			// 
			// grpBSkills
			// 
			this->grpBSkills->Location = System::Drawing::Point(6, 8);
			this->grpBSkills->Name = L"grpBSkills";
			this->grpBSkills->Size = System::Drawing::Size(164, 155);
			this->grpBSkills->TabIndex = 8;
			this->grpBSkills->TabStop = false;
			this->grpBSkills->Text = L"Skills";
			// 
			// btnSearch
			// 
			this->btnSearch->Location = System::Drawing::Point(6, 12);
			this->btnSearch->Name = L"btnSearch";
			this->btnSearch->Size = System::Drawing::Size(107, 27);
			this->btnSearch->TabIndex = 9;
			this->btnSearch->Text = L"&Quick Search";
			this->btnSearch->UseVisualStyleBackColor = true;
			this->btnSearch->Click += gcnew System::EventHandler(this, &Form1::btnSearch_Click);
			// 
			// progressBar1
			// 
			this->progressBar1->Location = System::Drawing::Point(12, 511);
			this->progressBar1->Name = L"progressBar1";
			this->progressBar1->Size = System::Drawing::Size(338, 10);
			this->progressBar1->Step = 1;
			this->progressBar1->TabIndex = 10;
			// 
			// txtSolutions
			// 
			this->txtSolutions->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom) 
				| System::Windows::Forms::AnchorStyles::Left) 
				| System::Windows::Forms::AnchorStyles::Right));
			this->txtSolutions->ContextMenuStrip = this->contextMenuStrip1;
			this->txtSolutions->Location = System::Drawing::Point(6, 19);
			this->txtSolutions->Name = L"txtSolutions";
			this->txtSolutions->ReadOnly = true;
			this->txtSolutions->ScrollBars = System::Windows::Forms::RichTextBoxScrollBars::Vertical;
			this->txtSolutions->Size = System::Drawing::Size(397, 470);
			this->txtSolutions->TabIndex = 11;
			this->txtSolutions->Text = L"";
			this->txtSolutions->KeyDown += gcnew System::Windows::Forms::KeyEventHandler(this, &Form1::KeyDown);
			// 
			// contextMenuStrip1
			// 
			this->contextMenuStrip1->Name = L"contextMenuStrip1";
			this->contextMenuStrip1->Size = System::Drawing::Size(61, 4);
			this->contextMenuStrip1->Opening += gcnew System::ComponentModel::CancelEventHandler(this, &Form1::contextMenuStrip1_Opening);
			// 
			// groupBox4
			// 
			this->groupBox4->Controls->Add(this->btnAdvancedSearch);
			this->groupBox4->Controls->Add(this->btnCancel);
			this->groupBox4->Controls->Add(this->btnSearch);
			this->groupBox4->Location = System::Drawing::Point(12, 459);
			this->groupBox4->Name = L"groupBox4";
			this->groupBox4->Size = System::Drawing::Size(338, 46);
			this->groupBox4->TabIndex = 14;
			this->groupBox4->TabStop = false;
			// 
			// btnAdvancedSearch
			// 
			this->btnAdvancedSearch->Location = System::Drawing::Point(119, 12);
			this->btnAdvancedSearch->Name = L"btnAdvancedSearch";
			this->btnAdvancedSearch->Size = System::Drawing::Size(122, 27);
			this->btnAdvancedSearch->TabIndex = 11;
			this->btnAdvancedSearch->Text = L"&Advanced Search";
			this->btnAdvancedSearch->UseVisualStyleBackColor = true;
			this->btnAdvancedSearch->Click += gcnew System::EventHandler(this, &Form1::btnAdvancedSearch_Click);
			// 
			// btnCancel
			// 
			this->btnCancel->Location = System::Drawing::Point(247, 12);
			this->btnCancel->Name = L"btnCancel";
			this->btnCancel->Size = System::Drawing::Size(85, 27);
			this->btnCancel->TabIndex = 10;
			this->btnCancel->Text = L"&Cancel";
			this->btnCancel->UseVisualStyleBackColor = true;
			this->btnCancel->Click += gcnew System::EventHandler(this, &Form1::btnCancel_Click);
			// 
			// grpResults
			// 
			this->grpResults->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom) 
				| System::Windows::Forms::AnchorStyles::Left) 
				| System::Windows::Forms::AnchorStyles::Right));
			this->grpResults->Controls->Add(this->txtSolutions);
			this->grpResults->Location = System::Drawing::Point(356, 27);
			this->grpResults->Name = L"grpResults";
			this->grpResults->Size = System::Drawing::Size(409, 496);
			this->grpResults->TabIndex = 15;
			this->grpResults->TabStop = false;
			this->grpResults->Text = L"Results";
			// 
			// grpBSkillFilters
			// 
			this->grpBSkillFilters->Location = System::Drawing::Point(176, 8);
			this->grpBSkillFilters->Name = L"grpBSkillFilters";
			this->grpBSkillFilters->Size = System::Drawing::Size(148, 155);
			this->grpBSkillFilters->TabIndex = 9;
			this->grpBSkillFilters->TabStop = false;
			this->grpBSkillFilters->Text = L"Skill Filters";
			// 
			// menuStrip1
			// 
			this->menuStrip1->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(4) {this->fileToolStripMenuItem, 
				this->dataToolStripMenuItem, this->mnuLanguages, this->helpToolStripMenuItem});
			this->menuStrip1->Location = System::Drawing::Point(0, 0);
			this->menuStrip1->Name = L"menuStrip1";
			this->menuStrip1->Size = System::Drawing::Size(777, 24);
			this->menuStrip1->TabIndex = 16;
			this->menuStrip1->Text = L"menuStrip1";
			// 
			// fileToolStripMenuItem
			// 
			this->fileToolStripMenuItem->DropDownItems->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(1) {this->exitToolStripMenuItem});
			this->fileToolStripMenuItem->Name = L"fileToolStripMenuItem";
			this->fileToolStripMenuItem->Size = System::Drawing::Size(37, 20);
			this->fileToolStripMenuItem->Text = L"&File";
			// 
			// exitToolStripMenuItem
			// 
			this->exitToolStripMenuItem->Name = L"exitToolStripMenuItem";
			this->exitToolStripMenuItem->Size = System::Drawing::Size(92, 22);
			this->exitToolStripMenuItem->Text = L"E&xit";
			this->exitToolStripMenuItem->Click += gcnew System::EventHandler(this, &Form1::exitToolStripMenuItem_Click);
			// 
			// dataToolStripMenuItem
			// 
			this->dataToolStripMenuItem->DropDownItems->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(1) {this->reloadToolStripMenuItem});
			this->dataToolStripMenuItem->Name = L"dataToolStripMenuItem";
			this->dataToolStripMenuItem->Size = System::Drawing::Size(43, 20);
			this->dataToolStripMenuItem->Text = L"&Data";
			// 
			// reloadToolStripMenuItem
			// 
			this->reloadToolStripMenuItem->Name = L"reloadToolStripMenuItem";
			this->reloadToolStripMenuItem->Size = System::Drawing::Size(110, 22);
			this->reloadToolStripMenuItem->Text = L"&Reload";
			this->reloadToolStripMenuItem->Click += gcnew System::EventHandler(this, &Form1::reloadToolStripMenuItem_Click);
			// 
			// mnuLanguages
			// 
			this->mnuLanguages->Name = L"mnuLanguages";
			this->mnuLanguages->Size = System::Drawing::Size(71, 20);
			this->mnuLanguages->Text = L"&Language";
			// 
			// helpToolStripMenuItem
			// 
			this->helpToolStripMenuItem->DropDownItems->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(1) {this->aboutToolStripMenuItem});
			this->helpToolStripMenuItem->Name = L"helpToolStripMenuItem";
			this->helpToolStripMenuItem->Size = System::Drawing::Size(44, 20);
			this->helpToolStripMenuItem->Text = L"&Help";
			// 
			// aboutToolStripMenuItem
			// 
			this->aboutToolStripMenuItem->Name = L"aboutToolStripMenuItem";
			this->aboutToolStripMenuItem->Size = System::Drawing::Size(107, 22);
			this->aboutToolStripMenuItem->Text = L"&About";
			this->aboutToolStripMenuItem->Click += gcnew System::EventHandler(this, &Form1::aboutToolStripMenuItem_Click);
			// 
			// groupBox6
			// 
			this->groupBox6->Controls->Add(this->rdoFemale);
			this->groupBox6->Controls->Add(this->rdoMale);
			this->groupBox6->Location = System::Drawing::Point(218, 27);
			this->groupBox6->Name = L"groupBox6";
			this->groupBox6->Size = System::Drawing::Size(132, 65);
			this->groupBox6->TabIndex = 19;
			this->groupBox6->TabStop = false;
			// 
			// rdoFemale
			// 
			this->rdoFemale->AutoSize = true;
			this->rdoFemale->Location = System::Drawing::Point(13, 39);
			this->rdoFemale->Name = L"rdoFemale";
			this->rdoFemale->Size = System::Drawing::Size(59, 17);
			this->rdoFemale->TabIndex = 18;
			this->rdoFemale->TabStop = true;
			this->rdoFemale->Text = L"Female";
			this->rdoFemale->UseVisualStyleBackColor = true;
			this->rdoFemale->CheckedChanged += gcnew System::EventHandler(this, &Form1::DeleteOptions);
			// 
			// rdoMale
			// 
			this->rdoMale->AutoSize = true;
			this->rdoMale->Checked = true;
			this->rdoMale->Location = System::Drawing::Point(13, 16);
			this->rdoMale->Name = L"rdoMale";
			this->rdoMale->Size = System::Drawing::Size(48, 17);
			this->rdoMale->TabIndex = 0;
			this->rdoMale->TabStop = true;
			this->rdoMale->Text = L"Male";
			this->rdoMale->UseVisualStyleBackColor = true;
			this->rdoMale->CheckedChanged += gcnew System::EventHandler(this, &Form1::DeleteOptions);
			// 
			// chkBadSkills
			// 
			this->chkBadSkills->AutoSize = true;
			this->chkBadSkills->Location = System::Drawing::Point(6, 16);
			this->chkBadSkills->Name = L"chkBadSkills";
			this->chkBadSkills->Size = System::Drawing::Size(100, 17);
			this->chkBadSkills->TabIndex = 20;
			this->chkBadSkills->Text = L"Allow Bad Skills";
			this->chkBadSkills->UseVisualStyleBackColor = true;
			// 
			// chkPiercings
			// 
			this->chkPiercings->AutoSize = true;
			this->chkPiercings->Checked = true;
			this->chkPiercings->CheckState = System::Windows::Forms::CheckState::Checked;
			this->chkPiercings->Location = System::Drawing::Point(6, 40);
			this->chkPiercings->Name = L"chkPiercings";
			this->chkPiercings->Size = System::Drawing::Size(97, 17);
			this->chkPiercings->TabIndex = 21;
			this->chkPiercings->Text = L"Allow Piercings";
			this->chkPiercings->UseVisualStyleBackColor = true;
			this->chkPiercings->CheckedChanged += gcnew System::EventHandler(this, &Form1::DeleteOptions);
			// 
			// chkTorsoInc
			// 
			this->chkTorsoInc->AutoSize = true;
			this->chkTorsoInc->Checked = true;
			this->chkTorsoInc->CheckState = System::Windows::Forms::CheckState::Checked;
			this->chkTorsoInc->Location = System::Drawing::Point(6, 64);
			this->chkTorsoInc->Name = L"chkTorsoInc";
			this->chkTorsoInc->Size = System::Drawing::Size(99, 17);
			this->chkTorsoInc->TabIndex = 22;
			this->chkTorsoInc->Text = L"Allow Torso Inc";
			this->chkTorsoInc->UseVisualStyleBackColor = true;
			this->chkTorsoInc->CheckedChanged += gcnew System::EventHandler(this, &Form1::DeleteOptions);
			// 
			// groupBox7
			// 
			this->groupBox7->Controls->Add(this->chkUkanlos);
			this->groupBox7->Controls->Add(this->chkDummy);
			this->groupBox7->Controls->Add(this->chkBadSkills);
			this->groupBox7->Controls->Add(this->chkTorsoInc);
			this->groupBox7->Controls->Add(this->chkPiercings);
			this->groupBox7->Location = System::Drawing::Point(12, 144);
			this->groupBox7->Name = L"groupBox7";
			this->groupBox7->Size = System::Drawing::Size(200, 108);
			this->groupBox7->TabIndex = 23;
			this->groupBox7->TabStop = false;
			// 
			// chkUkanlos
			// 
			this->chkUkanlos->AutoSize = true;
			this->chkUkanlos->Location = System::Drawing::Point(6, 110);
			this->chkUkanlos->Name = L"chkUkanlos";
			this->chkUkanlos->Size = System::Drawing::Size(102, 17);
			this->chkUkanlos->TabIndex = 24;
			this->chkUkanlos->Text = L"Beaten Ukanlos";
			this->chkUkanlos->UseVisualStyleBackColor = true;
			this->chkUkanlos->Visible = false;
			// 
			// chkDummy
			// 
			this->chkDummy->AutoSize = true;
			this->chkDummy->Location = System::Drawing::Point(6, 87);
			this->chkDummy->Name = L"chkDummy";
			this->chkDummy->Size = System::Drawing::Size(93, 17);
			this->chkDummy->TabIndex = 23;
			this->chkDummy->Text = L"Allow (dummy)";
			this->chkDummy->UseVisualStyleBackColor = true;
			this->chkDummy->CheckedChanged += gcnew System::EventHandler(this, &Form1::DeleteOptions);
			// 
			// tabHunterType
			// 
			this->tabHunterType->Controls->Add(this->tabBlademaster);
			this->tabHunterType->Controls->Add(this->tabGunner);
			this->tabHunterType->Location = System::Drawing::Point(12, 258);
			this->tabHunterType->Name = L"tabHunterType";
			this->tabHunterType->SelectedIndex = 0;
			this->tabHunterType->Size = System::Drawing::Size(338, 195);
			this->tabHunterType->TabIndex = 24;
			// 
			// tabBlademaster
			// 
			this->tabBlademaster->BackColor = System::Drawing::SystemColors::Control;
			this->tabBlademaster->Controls->Add(this->grpBSkills);
			this->tabBlademaster->Controls->Add(this->grpBSkillFilters);
			this->tabBlademaster->Location = System::Drawing::Point(4, 22);
			this->tabBlademaster->Name = L"tabBlademaster";
			this->tabBlademaster->Padding = System::Windows::Forms::Padding(3);
			this->tabBlademaster->Size = System::Drawing::Size(330, 169);
			this->tabBlademaster->TabIndex = 0;
			this->tabBlademaster->Text = L"Blademaster";
			// 
			// tabGunner
			// 
			this->tabGunner->BackColor = System::Drawing::SystemColors::Control;
			this->tabGunner->Controls->Add(this->grpGSkillFilters);
			this->tabGunner->Controls->Add(this->grpGSkills);
			this->tabGunner->Location = System::Drawing::Point(4, 22);
			this->tabGunner->Name = L"tabGunner";
			this->tabGunner->Padding = System::Windows::Forms::Padding(3);
			this->tabGunner->Size = System::Drawing::Size(330, 169);
			this->tabGunner->TabIndex = 1;
			this->tabGunner->Text = L"Gunner";
			// 
			// grpGSkillFilters
			// 
			this->grpGSkillFilters->Location = System::Drawing::Point(176, 8);
			this->grpGSkillFilters->Name = L"grpGSkillFilters";
			this->grpGSkillFilters->Size = System::Drawing::Size(148, 155);
			this->grpGSkillFilters->TabIndex = 10;
			this->grpGSkillFilters->TabStop = false;
			this->grpGSkillFilters->Text = L"Skill Filters";
			// 
			// grpGSkills
			// 
			this->grpGSkills->Location = System::Drawing::Point(6, 8);
			this->grpGSkills->Name = L"grpGSkills";
			this->grpGSkills->Size = System::Drawing::Size(164, 155);
			this->grpGSkills->TabIndex = 9;
			this->grpGSkills->TabStop = false;
			this->grpGSkills->Text = L"Skills";
			// 
			// grpSort
			// 
			this->grpSort->Controls->Add(this->cmbSort);
			this->grpSort->Location = System::Drawing::Point(218, 97);
			this->grpSort->Name = L"grpSort";
			this->grpSort->Size = System::Drawing::Size(132, 45);
			this->grpSort->TabIndex = 0;
			this->grpSort->TabStop = false;
			this->grpSort->Text = L"Sort By";
			// 
			// cmbSort
			// 
			this->cmbSort->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Left) 
				| System::Windows::Forms::AnchorStyles::Right));
			this->cmbSort->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->cmbSort->FormattingEnabled = true;
			this->cmbSort->Items->AddRange(gcnew cli::array< System::Object^  >(10) {L"None", L"Dragon res", L"Fire res", L"Ice res", 
				L"Thunder res", L"Water res", L"Base defence", L"Difficulty", L"Rarity", L"Slots spare"});
			this->cmbSort->Location = System::Drawing::Point(6, 16);
			this->cmbSort->Name = L"cmbSort";
			this->cmbSort->Size = System::Drawing::Size(120, 21);
			this->cmbSort->TabIndex = 0;
			this->cmbSort->SelectedIndexChanged += gcnew System::EventHandler(this, &Form1::cmbSort_SelectedIndexChanged);
			// 
			// Form1
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(777, 528);
			this->Controls->Add(this->grpSort);
			this->Controls->Add(this->groupBox6);
			this->Controls->Add(this->groupBox7);
			this->Controls->Add(this->tabHunterType);
			this->Controls->Add(this->progressBar1);
			this->Controls->Add(this->grpResults);
			this->Controls->Add(this->groupBox1);
			this->Controls->Add(this->groupBox4);
			this->Controls->Add(this->menuStrip1);
			this->Name = L"Form1";
			this->Text = L"Athena\'s A.S.S.";
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->nudHR))->EndInit();
			this->groupBox1->ResumeLayout(false);
			this->groupBox1->PerformLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->nudWeaponSlots))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->nudElder))->EndInit();
			this->groupBox4->ResumeLayout(false);
			this->grpResults->ResumeLayout(false);
			this->menuStrip1->ResumeLayout(false);
			this->menuStrip1->PerformLayout();
			this->groupBox6->ResumeLayout(false);
			this->groupBox6->PerformLayout();
			this->groupBox7->ResumeLayout(false);
			this->groupBox7->PerformLayout();
			this->tabHunterType->ResumeLayout(false);
			this->tabBlademaster->ResumeLayout(false);
			this->tabGunner->ResumeLayout(false);
			this->grpSort->ResumeLayout(false);
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion


private: 
	System::Void FormulateQuery( const bool danger, const bool use_gunner_skills )
	{
		query = gcnew Query;
		query->danger = danger;
		for( int i = 0; i < int( Armor::ArmorType::NumArmorTypes ); ++i )
		{
			query->rel_armor.Add( gcnew List_t< Armor^ >() );
			query->inf_armor.Add( gcnew List_t< Armor^ >() );
		}
		query->weapon_slots_allowed = int( nudWeaponSlots->Value );
		query->elder_star = int( nudElder->Value );
		query->hr = int( nudHR->Value );

		query->difficulty_level = GetDifficulty();

		query->gender = rdoMale->Checked ? Gender::MALE : Gender::FEMALE;
		query->hunter_type = !use_gunner_skills ? HunterType::BLADEMASTER : HunterType::GUNNER;
		query->include_piercings = chkPiercings->Checked;
		query->allow_bad = chkBadSkills->Checked;
		query->allow_torso_inc = chkTorsoInc->Checked;
		query->allow_dummy = chkDummy->Checked;

		if( !use_gunner_skills )
		{
			for( unsigned i = 0; i < NumSkills; ++i )
				if( bSkills[ i ]->SelectedIndex >= 0 )
					query->skills.Add( data->FindSkill( bIndexMaps[ i ][ bSkills[ i ]->SelectedIndex ] ) );
		}
		else
		{
			for( unsigned i = 0; i < NumSkills; ++i )
				if( gSkills[ i ]->SelectedIndex >= 0 )
					query->skills.Add( data->FindSkill( gIndexMaps[ i ][ gSkills[ i ]->SelectedIndex ] ) );
		}
		data->GetRelevantData( query );
	}

	System::Void StartThread( cliext::pair< Query^, unsigned >^ data )
	{
		BackgroundWorker^ new_thread = gcnew BackgroundWorker;
		new_thread->WorkerSupportsCancellation = true;
		new_thread->WorkerReportsProgress = true;
		new_thread->DoWork += gcnew System::ComponentModel::DoWorkEventHandler(this, &Form1::backgroundWorker1_DoWork);
		new_thread->RunWorkerCompleted += gcnew System::ComponentModel::RunWorkerCompletedEventHandler(this, &Form1::backgroundWorker1_RunWorkerCompleted);
		new_thread->ProgressChanged += gcnew System::ComponentModel::ProgressChangedEventHandler(this, &Form1::backgroundWorker1_ProgressChanged);
		new_thread->RunWorkerAsync( data );
		workers.Add( new_thread );
	}

	System::Void StartSearch()
	{
		progressBar1->Value = 0;
		total_progress = 0;
		if( query->skills.Count > 0 )
		{
			query->CreateEquivalences();
			btnSearch->Enabled = false;
			btnAdvancedSearch->Enabled = false;
			btnCancel->Enabled = true;
			cmbSort->Enabled = false;
			txtSolutions->Clear();
			final_solutions.Clear();
			last_result = nullptr;
			last_search_gunner = tabHunterType->SelectedIndex == 1;
			finished_workers = 0;
			for( int i = 0; i < query->armor_eq[ int( Armor::ArmorType::HEAD ) ]->Count; ++i )
				StartThread( gcnew cliext::pair< Query^, unsigned >( query, i ) );
		}
	}

	System::Void btnAdvancedSearch_Click(System::Object^  sender, System::EventArgs^  e)
	{
		FormulateQuery( true, tabHunterType->SelectedIndex == 1 );
		frmAdvanced advanced_search( query, btnCancel->Text, btnAdvancedSearch->Text->Trim( L'&' ), %strings );
		if( tabHunterType->SelectedIndex == 0 )
		{
			advanced_search.CheckResult( blast_options );
			advanced_search.ShowDialog( this );
			blast_options = advanced_search.result;
		}
		else if( tabHunterType->SelectedIndex == 1 )
		{
			advanced_search.CheckResult( glast_options );
			advanced_search.ShowDialog( this );
			glast_options = advanced_search.result;
		}
		else return;

		if( advanced_search.DialogResult != ::DialogResult::OK ) return;

		for( int p = 0; p < int( Armor::ArmorType::NumArmorTypes ); ++p )
		{
			query->rel_armor[ p ]->Clear();
			for( int i = 0; i < query->inf_armor[ p ]->Count; ++i )
			{
				if( advanced_search.boxes[ p ]->Items[ i ]->Checked )
					query->rel_armor[ p ]->Add( query->inf_armor[ p ][ i ] );
			}
		}
		query->rel_decorations.Clear();
		for( int i = 0; i < query->inf_decorations.Count; ++i )
		{
			if( advanced_search.boxes[ int( Armor::ArmorType::NumArmorTypes ) ]->Items[ i ]->Checked )
				query->rel_decorations.Add( query->inf_decorations[ i ] );
		}

		StartSearch();
	}

	System::Void btnSearch_Click(System::Object^  sender, System::EventArgs^  e)
	{		
		FormulateQuery( false, tabHunterType->SelectedIndex == 1 );
		
		StartSearch();
	}

	System::Void btnCancel_Click(System::Object^  sender, System::EventArgs^  e)
	{
		for each( BackgroundWorker^ worker in workers )
			worker->CancelAsync();
		btnCancel->Enabled = false;
		btnSearch->Enabled = true;
		btnAdvancedSearch->Enabled = true;
		progressBar1->Value = 0;
	}

	System::Void cmbSkillFilter_SelectedIndexChanged(System::Object^  sender, List_t< ComboBox^ >% skills, List_t< ComboBox^ >% skill_filters, List_t< IndexMap^ >% index_maps )
	{
		List_t< Ability^ > old_skills;
		int index = -1;
		Skill^ selected_skill = nullptr;
		for( unsigned i = 0; i < NumSkills; ++i )
		{
			if( sender == skill_filters[ i ] )
				index = i;
			if( skills[ i ]->SelectedIndex == -1 ) continue;
			Skill^ skill = Skill::static_skills[ index_maps[ i ][ skills[ i ]->SelectedIndex ] ];
			if( sender == skill_filters[ i ] )
				selected_skill = skill;
			else old_skills.Add( skill->ability );
		}
		if( index == -1 ) return;
		skills[ index ]->BeginUpdate();
		InitSkills( skills[ index ], index_maps[ index ], skill_filters[ index ]->SelectedIndex, %old_skills );
		ResetSkill( skills[ index ], index_maps[ index ], selected_skill );
		skills[ index ]->EndUpdate();
	}

	System::Void cmbSkillFilter_SelectedIndexChanged(System::Object^  sender, System::EventArgs^  e)
	{
		if( tabHunterType->SelectedIndex == 0 )
		{
			blast_options = nullptr;
			cmbSkillFilter_SelectedIndexChanged( sender, bSkills, bSkillFilters, bIndexMaps );
		}
		else if( tabHunterType->SelectedIndex == 1 )
		{
			glast_options = nullptr;
			cmbSkillFilter_SelectedIndexChanged( sender, gSkills, gSkillFilters, gIndexMaps );
		}
	}

	System::Void cmbSkill_SelectedIndexChanged( System::Object^ sender, List_t< ComboBox^ >% skills, List_t< ComboBox^ >% skill_filters, List_t< IndexMap^ >% index_maps )
	{
		int index = -1;
		for( int i = 0; i < NumSkills; ++i )
			if( sender == skills[ i ] )
				index = i;
		if( index == -1 ) return;
		if( skills[ index ]->SelectedIndex == 0 )
		{
			skills[ index ]->SelectedIndex = -1;
			return;
		}
		lock_skills = true;
		for( int i = 0; i < NumSkills; ++i )
		{
			if( i == index ) continue;
			Skill^ skill = skills[ i ]->SelectedIndex == -1 ? nullptr : Skill::static_skills[ index_maps[ i ][ skills[ i ]->SelectedIndex ] ];
			List_t< Ability^ > old_skills;
			for( int j = 0; j < NumSkills; ++j )
				if( j != i && skills[ j ]->SelectedIndex != -1 )
					old_skills.Add( Skill::static_skills[ index_maps[ j ][ skills[ j ]->SelectedIndex ] ]->ability );
			skills[ i ]->BeginUpdate();
			InitSkills( skills[ i ], index_maps[ i ], skill_filters[ i ]->SelectedIndex, %old_skills );
			ResetSkill( skills[ i ], index_maps[ i ], skill );
			skills[ i ]->EndUpdate();
		}
		lock_skills = false;
	}

	System::Void cmbSkill_SelectedIndexChanged(System::Object^  sender, System::EventArgs^  e) 
	{
		if( lock_skills ) return;
		if( tabHunterType->SelectedIndex == 0 )
		{
			blast_options = nullptr;
			cmbSkill_SelectedIndexChanged( sender, bSkills, bSkillFilters, bIndexMaps );
		}
		else if( tabHunterType->SelectedIndex == 1 )
		{
			glast_options = nullptr;
			cmbSkill_SelectedIndexChanged( sender, gSkills, gSkillFilters, gIndexMaps );
		}
	}

	System::Void UpdateResultString( List_t< Solution^ >^ solutions )
	{
		//if( !solutions || solutions->Count == 0 ) return;
		System::Text::StringBuilder sb( solutions->Count * 1024 );
		int limit = solutions->Count;

		if( last_result ) sb.Append( last_result );
		
		System::String^ dash = L"-----------------";
		for each( Solution^ solution in solutions )
		{
			sb.Append( endl );
			for each( Armor^ armor in solution->armors )
			{
				if( armor )
				{
					sb.Append( armor->name );
					if( armor->no_skills )
						sb.Append( L" (" )->Append( strings[ StringTable::OrAnythingWith ] )->Append( L" " )->Append( Convert::ToString( armor->num_slots ) )->Append( L" " )->Append( strings[ int( StringTable::Slots ) ])->Append( L")" );
					else if( armor->torso_inc )
						sb.Append( L" (" )->Append( strings[ StringTable::OrAnythingWithTorsoInc ] )->Append( L")" );
					sb.Append( endl );
				}
			}
			if( solution->decorations.Count > 0 )
			{
				sb.AppendLine( dash );
				Generic::Dictionary< Decoration^, unsigned > deco_dict;
				for each( Decoration^ decoration in solution->decorations )
				{
					if( !deco_dict.ContainsKey( decoration ) )
						deco_dict.Add( decoration, 1 );
					else deco_dict[ decoration ]++;
				}
				Generic::Dictionary< Decoration^, unsigned >::Enumerator iter = deco_dict.GetEnumerator();
				while( iter.MoveNext() )
					sb.Append( Convert::ToString( iter.Current.Value ) )->Append( L"x " )->AppendLine( iter.Current.Key->name );
			}
			if( solution->total_slots_spare > 0 || cmbSort->SelectedIndex == 9 )
			{
				sb.Append( Convert::ToString( solution->total_slots_spare ) )->Append( L" " );
				if( solution->total_slots_spare == 1 )
					sb.AppendLine( strings[ StringTable::SlotSpare ] );
				else
					sb.AppendLine( strings[ StringTable::SlotsSpare ] );
			}
			if( cmbSort->SelectedIndex > 0 && cmbSort->SelectedIndex != 9 )
			{
				if( cmbSort->SelectedIndex == 1 ) sb.Append( solution->dragon_res );
				else if( cmbSort->SelectedIndex == 2 ) sb.Append( solution->fire_res );
				else if( cmbSort->SelectedIndex == 3 ) sb.Append( solution->ice_res );
				else if( cmbSort->SelectedIndex == 4 ) sb.Append( solution->thunder_res );
				else if( cmbSort->SelectedIndex == 5 ) sb.Append( solution->water_res );
				else if( cmbSort->SelectedIndex == 6 ) sb.Append( solution->defence );
				else if( cmbSort->SelectedIndex == 7 ) sb.Append( solution->difficulty );
				else if( cmbSort->SelectedIndex == 8 ) sb.Append( solution->rarity );
				sb.Append( L" " )->AppendLine( (String^)cmbSort->SelectedItem );
			}
			if( solution->extra_skills.Count > 0 )
			{
				sb.AppendLine( dash );
				for each( Skill^ skill in solution->extra_skills )
				{
					if( !Utility::Contains( %query->skills, skill ) )
						sb.AppendLine( skill->name );
				}
			}
		}

		if( %final_solutions != solutions )
			final_solutions.AddRange( solutions );

		System::Text::StringBuilder sb2;
		if( limit >= MAX_LIMIT )
		{
			sb2.Append( strings[ StringTable::ShowingFirst ] )->Append( L" " )->Append( Convert::ToString( MAX_LIMIT ) )->Append( L" " )->AppendLine( strings[ StringTable::SolutionsOnly ] );
			limit = MAX_LIMIT;
		}
		else sb2.Append( strings[ StringTable::SolutionsFound ] )->Append( L": " )->AppendLine( Convert::ToString( final_solutions.Count ) );

		sb2.Append( last_result = sb.ToString() );
		
		results_mutex->WaitOne();
		txtSolutions->Text = sb2.ToString();
		results_mutex->ReleaseMutex();
	}

#pragma region Worker Thread Stuff
	System::Void backgroundWorker1_RunWorkerCompleted( Object^ /*sender*/, RunWorkerCompletedEventArgs^ e )
	{
		if( e->Error != nullptr )
		{
			MessageBox::Show( e->Error->Message );
			progressBar1->Value = 0;
			return;
		}
		else if( e->Cancelled )
		{
			progressBar1->Value = 0;
			return;
		}
		else
		{
			progress_mutex->WaitOne();
			if( ++finished_workers >= unsigned( workers.Count ) )
			{
				btnSearch->Enabled = true;
				btnAdvancedSearch->Enabled = true;
				btnCancel->Enabled = false;
				cmbSort->Enabled = true;
				progressBar1->Value = 100;
				workers.Clear();
			}
			progress_mutex->ReleaseMutex();
		}

		if( e->Result )
		{
			UpdateResultString( static_cast< List_t< Solution^ >^ >( e->Result ) );
			SortResults();
			last_result = nullptr;
			UpdateResultString( %final_solutions );
		}		

		SaveConfig();
	}

	System::Void backgroundWorker1_ProgressChanged( Object^ /*sender*/, ProgressChangedEventArgs^ e )
	{
		progress_mutex->WaitOne();
		total_progress += e->ProgressPercentage;
		progressBar1->Value = total_progress / workers.Count;
		progress_mutex->ReleaseMutex();
		if( e->UserState )
			UpdateResultString( static_cast< List_t< Solution^ >^ >( e->UserState ) );
	}
	System::Void backgroundWorker1_DoWork( System::Object^ sender, System::ComponentModel::DoWorkEventArgs^ e )
	{
		BackgroundWorker^ worker = static_cast< BackgroundWorker^ >( sender );
		cliext::pair< Query^, unsigned >^ data = static_cast< cliext::pair< Query^, unsigned >^ >( e->Argument );
		Query^ query = data->first;
		ArmorEquivalence^ head_eq = query->armor_eq[ int( Armor::ArmorType::HEAD ) ][ data->second ];

		//List_t< ArmorEquivalence^ >^ head_eq  = query->armor_eq[ int( Armor::ArmorType::HEAD ) ];
		List_t< ArmorEquivalence^ >^ body_eq  = query->armor_eq[ int( Armor::ArmorType::BODY ) ];
		List_t< ArmorEquivalence^ >^ arms_eq  = query->armor_eq[ int( Armor::ArmorType::ARMS ) ];
		List_t< ArmorEquivalence^ >^ waist_eq = query->armor_eq[ int( Armor::ArmorType::WAIST ) ];
		List_t< ArmorEquivalence^ >^ legs_eq  = query->armor_eq[ int( Armor::ArmorType::LEGS ) ];

		List_t< Solution^ >^ solutions = gcnew List_t< Solution^ >;

		unsigned num_solutions = 0;
		unsigned last_percent = 0;
		bool new_stuff = false;

		for( int j = 0; j < body_eq->Count; ++j )
		{
			const int progress = ( 100 * j ) / body_eq->Count;

			if( !new_stuff )
			{
				worker->ReportProgress( progress - last_percent );
				last_percent = progress;
			}
			else
			{
				List_t< Solution ^ >^ temp = solutions; //handle race-condition: shouldn't modify solution list while iterating through it
				solutions = gcnew List_t< Solution^ >;
				worker->ReportProgress( progress - last_percent, temp );
				last_percent = progress;
				new_stuff = false;
			}
			for each( ArmorEquivalence^ earms in arms_eq )
			{
				for each( ArmorEquivalence^ ewaist in waist_eq )
				{
					if( worker->CancellationPending )
					{
						e->Result = solutions;
						return;
					}
					for each( ArmorEquivalence^ elegs in legs_eq )
					{
						EquivalenceSolution^ esolution = gcnew EquivalenceSolution;
						esolution->armor_eq.Add( head_eq );
						esolution->armor_eq.Add( body_eq[ j ] );
						esolution->armor_eq.Add( earms );
						esolution->armor_eq.Add( ewaist );
						esolution->armor_eq.Add( elegs );

						if( esolution->MatchesQuery( query ) )
						{
							for each( Armor^ head in head_eq->armors )
							{
								for each( Armor^ body in body_eq[ j ]->armors )
								{
									for each( Armor^ arms in earms->armors )
									{
										for each( Armor^ waist in ewaist->armors )
										{
											for each( Armor^ legs in elegs->armors )
											{
												Solution^ solution = gcnew Solution( esolution );
												solution->armors.Add( head );
												solution->armors.Add( body );
												solution->armors.Add( arms );
												solution->armors.Add( waist );
												solution->armors.Add( legs );

												if( solution->CheckBadSkills( query ) )
												{
													solution->CalculateData( query->difficulty_level );
													solutions->Add( solution );
													new_stuff = true;
													if( ++num_solutions >= MAX_LIMIT )
													{
														e->Result = solutions;
														worker->ReportProgress( 100 - last_percent );
														return;
													}
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
		worker->ReportProgress( 100 - last_percent );
		e->Result = solutions;
	}
#pragma endregion

	System::Void FindDialogClosed( System::Object^ sender, System::EventArgs^ e )
	{
		find_dialog = nullptr;
	}

	System::Void FindDialogFoundText( System::Object^ sender, System::EventArgs^ e )
	{
		frmFind^ find = (frmFind^)sender;
		if( find->index == -1 )
		{
			txtSolutions->SelectionStart = txtSolutions->Text->Length;
			txtSolutions->SelectionLength = 0;
		}
		else
		{
			txtSolutions->SelectionStart = find->index;
			txtSolutions->SelectionLength = find->length;
		}
		txtSolutions->ScrollToCaret();
		txtSolutions->Focus();
	}

	System::Void KeyDown( System::Object^ sender, System::Windows::Forms::KeyEventArgs^ e )
	{
		if( sender == txtSolutions && e->Control )
		{
			if( e->KeyValue == L'A' )
			{
				txtSolutions->SelectAll();
				e->Handled = true;
				e->SuppressKeyPress = true;
			}
			else if( e->KeyValue == L'F' && !find_dialog )
			{
				find_dialog = gcnew frmFind( txtSolutions );
				find_dialog->DialogClosing += gcnew EventHandler( this, &Form1::FindDialogClosed );
				find_dialog->TextFound += gcnew EventHandler( this, &Form1::FindDialogFoundText );
				find_dialog->Show( this );
				e->Handled = true;
				e->SuppressKeyPress = true;
			}
			else if( e->KeyValue == L'X' )
			{
				
			}
		}
	}

	System::Void exitToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e)
	{
		Close();
	}

	System::Void cmbSort_SelectedIndexChanged(System::Object^  sender, System::EventArgs^  e)
	{
		static int last_index = -1;
		if( cmbSort->SelectedIndex == last_index ) return;
		last_index = cmbSort->SelectedIndex;
		if( data )
		{
			SaveConfig();
			if( final_solutions.Count > 0 )
			{
				SortResults();
				last_result = nullptr;
				UpdateResultString( %final_solutions );
			}
		}
	}

	System::Void ApplyLanguageString( List_t< String^ >^ lang, List_t< SkillTag^ >^ x, String^ file )
	{
		if( lang->Count < x->Count )
		{
			MessageBox::Show( this, L"Error: " + file + L" is invalid" );
			return;
		}
		for( int i = 0; i < lang->Count; ++i )
		{
			String^ str = lang[ i ];
			x[ i ]->name = str;
		}
	}

	System::Void ApplyLanguageArmor( List_t< String^ >^ lang, List_t< Armor^ >^ x, String^ file, Map_t< String^, List_t< Armor^ >^ >^ map )
	{
		if( lang->Count < x->Count )
		{
			MessageBox::Show( this, L"Error: " + file + L" is invalid" );
			return;
		}
		for( int i = 0; i < lang->Count; ++i )
		{
			x[ i ]->name = lang[ i ];
			if( !map->ContainsKey( lang[ i ] ) )
				map->Add( lang[ i ], gcnew List_t< Armor^ > );
			map[ lang[ i ] ]->Add( x[ i ] );
		}
	}

	template< class T >
	System::Void ApplyLanguage( List_t< String^ >^ lang, List_t< T^ >^ x, String^ file, Map_t< String^, T^ >^ map )
	{
		if( lang->Count < x->Count )
		{
			MessageBox::Show( this, L"Error: " + file + L" is invalid" );
			return;
		}
		map->Clear();
		for( int i = 0; i < lang->Count; ++i )
		{
			String^ str = lang[ i ];
			x[ i ]->name = str;
			map->Add( str, x[ i ] );
		}
	}

	System::Void LoadLanguage( System::String^ lang )
	{
		last_lang = lang;
		for each( ToolStripMenuItem^ item in mnuLanguages->DropDownItems )
		{
			if( item->Text == lang )
			{
				LanguageSelect_Click( item, nullptr );
				item->Checked = true;
				return;
			}
		}
	}

	System::Void LanguageSelect_Click(System::Object^  sender, System::EventArgs^  e)
	{		
		ToolStripMenuItem^ item = static_cast< ToolStripMenuItem^ >( sender );
		last_lang = item->Text;
		const int index = mnuLanguages->DropDownItems->IndexOf( item );
		Language^ lang = data->LoadLanguage( index );
		if( lang->string_table.Count < 46 )
		{
			MessageBox::Show( this, L"Error: strings.txt is invalid" );
			return;
		}
		fileToolStripMenuItem->Text = lang->string_table[ 0 ];
		dataToolStripMenuItem->Text = lang->string_table[ 1 ];
		mnuLanguages->Text = lang->string_table[ 2 ];
		helpToolStripMenuItem->Text = lang->string_table[ 3 ];
		exitToolStripMenuItem->Text = lang->string_table[ 4 ];
		reloadToolStripMenuItem->Text = lang->string_table[ 5 ];
		aboutToolStripMenuItem->Text = lang->string_table[ 6 ];
		tabBlademaster->Text = lang->string_table[ 7 ];
		tabGunner->Text = lang->string_table[ 8 ];
		rdoMale->Text = lang->string_table[ 9 ];
		rdoFemale->Text = lang->string_table[ 10 ];
		lblHR->Text = lang->string_table[ 11 ];
		lblElder->Text = lang->string_table[ 12 ];
		lblSlots->Text = lang->string_table[ 13 ];
		grpBSkills->Text = lang->string_table[ 14 ];
		grpGSkills->Text = lang->string_table[ 14 ];
		grpBSkillFilters->Text = lang->string_table[ 15 ];
		grpGSkillFilters->Text = lang->string_table[ 15 ];
		chkBadSkills->Text = lang->string_table[ 16 ];
		chkPiercings->Text = lang->string_table[ 17 ];
		chkTorsoInc->Text = lang->string_table[ 18 ];
		chkDummy->Text = lang->string_table[ 19 ];
		chkUkanlos->Text = lang->string_table[ 20 ];
		btnSearch->Text = lang->string_table[ 21 ];
		btnAdvancedSearch->Text = lang->string_table[ 22 ];
		btnCancel->Text = lang->string_table[ 23 ];
		grpResults->Text = lang->string_table[ 24 ];
		strings[ 0 ] = lang->string_table[ 25 ]; // "or anything with Torso Inc"
		strings[ 1 ] = lang->string_table[ 26 ]; // "or anything with"
		strings[ 2 ] = lang->string_table[ 27 ]; // "slots"
		strings[ 3 ] = lang->string_table[ 28 ]; // "Showing first"
		strings[ 4 ] = lang->string_table[ 29 ]; // "solutions only"
		strings[ 5 ] = lang->string_table[ 30 ]; // "solutions found"
		strings[ 6 ] = lang->string_table[ 31 ]; // "slot spare"
		strings[ 7 ] = lang->string_table[ 32 ]; // "slots spare"
		strings[ 8 ] = lang->string_table[ 33 ]; // "Select Armor"
		strings[ 9 ] = lang->string_table[ 34 ]; // "Search"
		strings[ 10 ] = lang->string_table[ 35 ]; // "Default"
		grpSort->Text = lang->string_table[ 36 ]; // "Sort By"
		strings[ 11 ] = lang->string_table[ 37 ]; // "None"
		strings[ 12 ] = lang->string_table[ 38 ]; // "Dragon"
		strings[ 13 ] = lang->string_table[ 39 ]; // "Fire Res"
		strings[ 14 ] = lang->string_table[ 40 ]; // "Ice"
		strings[ 15 ] = lang->string_table[ 41 ]; // "Thunder Res"
		strings[ 16 ] = lang->string_table[ 42 ]; // "Water"
		strings[ 17 ] = lang->string_table[ 43 ]; // "Base Defence"
		
		cmbSort->SuspendLayout();
		sort_off = true;
		for( unsigned i = 0; i < 10; ++i )
			cmbSort->Items[ i ] = lang->string_table[ 37 + i ];
		sort_off = false;
		cmbSort->ResumeLayout();

		ApplyLanguage( %lang->abilities, %Ability::static_abilities, L"skills.txt", %Ability::static_ability_map );
		ApplyLanguage( %lang->skills, %Skill::static_skills, L"skills.txt", %Skill::static_skill_map );
		ApplyLanguage( %lang->decorations, %Decoration::static_decorations, L"decorations.txt", %Decoration::static_decoration_map );
		ApplyLanguage( %lang->components, %Material::static_materials, L"components.txt", %Material::static_material_map );
		ApplyLanguageString( %lang->tags, %data->tags, L"tags.txt" );
		Armor::static_armor_map.Clear();
		ApplyLanguageArmor( lang->armors[ 0 ], Armor::static_armors[ 0 ], L"head.txt", %Armor::static_armor_map );
		ApplyLanguageArmor( lang->armors[ 1 ], Armor::static_armors[ 1 ], L"body.txt", %Armor::static_armor_map );
		ApplyLanguageArmor( lang->armors[ 2 ], Armor::static_armors[ 2 ], L"arms.txt", %Armor::static_armor_map );
		ApplyLanguageArmor( lang->armors[ 3 ], Armor::static_armors[ 3 ], L"waist.txt", %Armor::static_armor_map );
		ApplyLanguageArmor( lang->armors[ 4 ], Armor::static_armors[ 4 ], L"legs.txt", %Armor::static_armor_map );

		for each( ToolStripMenuItem^ thang in mnuLanguages->DropDownItems )
			thang->Checked = false;
		item->Checked = true;

		lock_skills = true;
		for( int j = 0; j < data->tags.Count; ++j )
		{
			for( unsigned i = 0; i < NumSkills; ++i )
			{
				bSkillFilters[ i ]->Items[ j ] = data->tags[ j ]->name;
				gSkillFilters[ i ]->Items[ j ] = data->tags[ j ]->name;
			}
		}

		for( int i = 0; i < NumSkills; ++i )
		{
			bSkills[ i ]->Items[ 0 ] = strings[ 11 ];
			gSkills[ i ]->Items[ 0 ] = strings[ 11 ];
			for( int j = 1; j < bSkills[ i ]->Items->Count; ++j )
				bSkills[ i ]->Items[ j ] = data->FindSkill( bIndexMaps[ i ][ j ] )->name;
			for( int j = 1; j < gSkills[ i ]->Items->Count; ++j )
				gSkills[ i ]->Items[ j ] = data->FindSkill( gIndexMaps[ i ][ j ] )->name;
		}
		lock_skills = false;

		if( final_solutions.Count > 0 )
		{
			last_result = nullptr;
			UpdateResultString( %final_solutions );
		}

		for each( List_t< Armor^ >^ armor_list in Armor::static_armors )
			for each( Armor^ armor in armor_list )
				armor->is_dummy = armor->name->Contains( L"(dummy)" );
	}
	
	System::Void reloadToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e)
	{
		for( unsigned i = 0; i < NumSkills; ++i )
		{
			bSkills[ i ]->SelectedIndex = -1;
			gSkills[ i ]->SelectedIndex = -1;
		}
		
		data->ImportTextFiles();
		mnuLanguages->DropDownItems->Clear();
		for each( String^ lang in data->languages )
		{
			ToolStripMenuItem^ item = gcnew ToolStripMenuItem( lang );
			item->Click += gcnew System::EventHandler(this, &Form1::LanguageSelect_Click);
			mnuLanguages->DropDownItems->Add( item );
		}
		InitFilters();

		LoadLanguage( last_lang );
	}
	System::Void aboutToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e)
	{
		frmAbout about_form;
		about_form.ShowDialog( this );
	}

	System::Void HRChanged(System::Object^  sender, System::EventArgs^  e)
	{
		if( int( nudHR->Value ) == 9 )
			chkUkanlos->Enabled = true;
		else
		{
			chkUkanlos->Enabled = false;
			chkUkanlos->Checked = false;
		}
		DeleteOptions( sender, e );
	}

	System::Void DeleteOptions(System::Object^  sender, System::EventArgs^  e)
	{
		blast_options = glast_options = nullptr;
	}

	System::Void SortResults()
	{
		if( cmbSort->SelectedIndex < 1 || sort_off ) return;
		else if( cmbSort->SelectedIndex == 1 )
			final_solutions.Sort( gcnew Comparison< Solution^ >( CompareSolutionByDragonRes ) );
		else if( cmbSort->SelectedIndex == 2 )
			final_solutions.Sort( gcnew Comparison< Solution^ >( CompareSolutionByFireRes ) );
		else if( cmbSort->SelectedIndex == 3 )
			final_solutions.Sort( gcnew Comparison< Solution^ >( CompareSolutionByIceRes ) );
		else if( cmbSort->SelectedIndex == 4 )
			final_solutions.Sort( gcnew Comparison< Solution^ >( CompareSolutionByThunderRes ) );
		else if( cmbSort->SelectedIndex == 5 )
			final_solutions.Sort( gcnew Comparison< Solution^ >( CompareSolutionByWaterRes ) );
		else if( cmbSort->SelectedIndex == 6 )
			final_solutions.Sort( gcnew Comparison< Solution^ >( CompareSolutionByDefence ) );
		else if( cmbSort->SelectedIndex == 7 )
			final_solutions.Sort( gcnew Comparison< Solution^ >( CompareSolutionByDifficulty ) );
		else if( cmbSort->SelectedIndex == 8 )
			final_solutions.Sort( gcnew Comparison< Solution^ >( CompareSolutionByRarity ) );
		else if( cmbSort->SelectedIndex == 9 )
			final_solutions.Sort( gcnew Comparison< Solution^ >( CompareSolutionBySlotsSpare ) );
	}

	System::Void contextMenuStrip1_Opening(System::Object^  sender, System::ComponentModel::CancelEventArgs^  e)
	{
		if( txtSolutions->Text == L"" ) return;
		contextMenuStrip1->Items->Clear();
		e->Cancel = true;

		Point pos = txtSolutions->PointToClient( txtSolutions->MousePosition );
		int chr = txtSolutions->GetCharIndexFromPosition( pos );
		int line = txtSolutions->GetLineFromCharIndex( chr );
		String^ str = txtSolutions->Lines[ line ];
		if( str == L"" ) return;
		
		if( str->Length >= 4 )
		{
			if( str->Substring( 1, 2 ) == L"x " || str->Substring( 2, 2 ) == L"x " )
			{
				Decoration^ decoration = Decoration::FindDecoration( str->Substring( str->IndexOf( L' ' ) + 1 ) );
				if( decoration )
				{
					Utility::UpdateContextMenu( contextMenuStrip1, decoration );
					e->Cancel = false;
				}
				return;
			}
		}
		Armor^ armor = Armor::FindArmor( str );
		if( armor )
		{
			Utility::UpdateContextMenu( contextMenuStrip1, armor, %strings );
			e->Cancel = false;
		}
		else 
		{
			int bracket = str->LastIndexOf( L'(' );
			if( bracket != -1 )
			{
				str = str->Substring( 0, bracket - 1 );
				armor = Armor::FindArmor( str );
				if( armor )
				{
					Utility::UpdateContextMenu( contextMenuStrip1, armor, %strings );
					e->Cancel = false;
				}
			}
		}
	}
};
}

