#pragma once
#include "LoadedData.h"
#include "frmAbout.h"
#include "Version.h"

namespace MHArmor {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

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
		String^ CFG_FILE;
		String^ endl;
		bool lock_skills;
		LoadedData^ data;
		String^ last_lang;
		Query^ query;
		typedef Generic::Dictionary< unsigned, unsigned > IndexMap;
		IndexMap imSkill1;
		IndexMap imSkill2;
		IndexMap imSkill3;
		IndexMap imSkill4;
	private: System::Windows::Forms::MenuStrip^  menuStrip1;
	private: System::Windows::Forms::ToolStripMenuItem^  fileToolStripMenuItem;
	private: System::Windows::Forms::ToolStripMenuItem^  exitToolStripMenuItem;
	private: System::Windows::Forms::ToolStripMenuItem^  dataToolStripMenuItem;
	private: System::Windows::Forms::ToolStripMenuItem^  reloadToolStripMenuItem;
	private: System::Windows::Forms::ToolStripMenuItem^  helpToolStripMenuItem;
	private: System::Windows::Forms::ToolStripMenuItem^  aboutToolStripMenuItem;
	private: System::Windows::Forms::ComboBox^  cmbAllowBadSkills;
	private: System::Windows::Forms::ToolStripMenuItem^  mnuLanguages;

		BackgroundWorker^  backgroundWorker1;

		void ClearFilters()
		{
			cmbSkillFilter1->Items->Clear();
			cmbSkillFilter2->Items->Clear();
			cmbSkillFilter3->Items->Clear();
			cmbSkillFilter4->Items->Clear();
		}
	
		void AddFilter( System::String^ str )
		{
			cmbSkillFilter1->Items->Add( str );
			cmbSkillFilter2->Items->Add( str );
			cmbSkillFilter3->Items->Add( str );
			cmbSkillFilter4->Items->Add( str );
		}

		void InitFilters()
		{
			ClearFilters();
			List< String^ >::Enumerator iter = data->tags.GetEnumerator();
			while( iter.MoveNext() )
			{
				AddFilter( iter.Current );
			}
		}

		void ResetSkill( ComboBox^ box, IndexMap^ map, Skill^ skill )
		{
			if( skill == nullptr ) return;
			IndexMap::Enumerator iter = map->GetEnumerator();
			while( iter.MoveNext() )
			{
				if( data->skills[ iter.Current.Value ] == skill )
				{
					box->SelectedIndex = iter.Current.Key;
					return;
				}
			}
		}

		void InitSkills2( ComboBox^ box, IndexMap^ map, const int filter, Ability^ disallow1, Ability^ disallow2, Ability^ disallow3 )
		{
			map->Clear();
			box->SelectedIndex = -1;
			box->Items->Clear();
			if( filter == -1 ) return;
			box->Items->Add( L"(none)" );
			for( int i = 0; i < data->skills.Count; ++i )
			{
				if( data->skills[ i ]->points_required <= 0 || 
					filter == 1 && !data->skills[ i ]->ability->tags.Count == 0 ||
					disallow1 != nullptr && data->skills[ i ]->ability == disallow1 ||
					disallow2 != nullptr && data->skills[ i ]->ability == disallow2 ||
					disallow3 != nullptr && data->skills[ i ]->ability == disallow3 ) continue;
				
				if( filter == 0 || filter == 1 && data->skills[ i ]->ability->tags.Count == 0 || 
					Utility::ContainsString( data->skills[ i ]->ability->tags, data->tags[ filter ] ) )
				{
					map[ box->Items->Count ] = i;
					box->Items->Add( data->skills[ i ]->name );
				}
			}
		}

		void InitSkills( ComboBox^ box, IndexMap^ map, const int filter, Skill^ disallow1, Skill^ disallow2, Skill^ disallow3 )
		{
			InitSkills2( box, map, filter,
				disallow1 ? disallow1->ability : nullptr,
				disallow2 ? disallow2->ability : nullptr,
				disallow3 ? disallow3->ability : nullptr );
		}

		void InitSkills()
		{
			InitSkills( cmbSkill1, %imSkill1, cmbSkillFilter1->SelectedIndex, nullptr, nullptr, nullptr );
			InitSkills( cmbSkill2, %imSkill2, cmbSkillFilter2->SelectedIndex, nullptr, nullptr, nullptr );
			InitSkills( cmbSkill3, %imSkill3, cmbSkillFilter3->SelectedIndex, nullptr, nullptr, nullptr );
			InitSkills( cmbSkill4, %imSkill4, cmbSkillFilter4->SelectedIndex, nullptr, nullptr, nullptr );
		}
	public:

		Form1(void) : CFG_FILE( L"settings.cfg" ), endl( L"\r\n" )
		{
			InitializeComponent();
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
			cmbSkillFilter1->SelectedIndex = 0;
			cmbSkillFilter2->SelectedIndex = 0;
			cmbSkillFilter3->SelectedIndex = 0;
			cmbSkillFilter4->SelectedIndex = 0;
			cmbPiercings->SelectedIndex = 0;
			InitSkills();
			cmbGender->SelectedIndex = 0;
			cmbHunterType->SelectedIndex = 0;
			cmbAllowBadSkills->SelectedIndex = 1;
			lock_skills = false;
			btnCancel->Enabled = false;
			backgroundWorker1->WorkerSupportsCancellation = true;
			backgroundWorker1->WorkerReportsProgress = true;

			LoadConfig();

			Text += " " +  STRINGIZE( VERSION_NO ) ;
		}

		void LoadConfig()
		{
			if( IO::File::Exists( CFG_FILE ) )
			{
				IO::StreamReader fin( CFG_FILE );
				if( fin.ReadLine() == L"1" )
				{
					String^ lang = fin.ReadLine();
					LoadLanguage( lang );
					cmbHunterType->SelectedIndex = Convert::ToInt32( fin.ReadLine() );
					cmbGender->SelectedIndex = Convert::ToInt32( fin.ReadLine() );
					cmbPiercings->SelectedIndex = Convert::ToInt32( fin.ReadLine() );
					cmbAllowBadSkills->SelectedIndex = Convert::ToInt32( fin.ReadLine() );

					nudHR->Value = Convert::ToInt32( fin.ReadLine() );
					nudElder->Value = Convert::ToInt32( fin.ReadLine() );
					nudWeaponSlots->Value = Convert::ToInt32( fin.ReadLine() );

					cmbSkillFilter1->SelectedIndex = Convert::ToInt32( fin.ReadLine() );
					cmbSkill1->SelectedIndex = Convert::ToInt32( fin.ReadLine() );
					cmbSkillFilter2->SelectedIndex = Convert::ToInt32( fin.ReadLine() );
					cmbSkill2->SelectedIndex = Convert::ToInt32( fin.ReadLine() );
					cmbSkillFilter3->SelectedIndex = Convert::ToInt32( fin.ReadLine() );
					cmbSkill3->SelectedIndex = Convert::ToInt32( fin.ReadLine() );
					cmbSkillFilter4->SelectedIndex = Convert::ToInt32( fin.ReadLine() );
					cmbSkill4->SelectedIndex = Convert::ToInt32( fin.ReadLine() );
					
					System::Text::StringBuilder sb;
					while( !fin.EndOfStream )
						sb.Append( fin.ReadLine() )->Append( endl );
					txtSolutions->Text = sb.ToString();

					fin.Close();
					SaveConfig();
					return;
				}	
			}
			LoadLanguage( last_lang = L"TeamHGG MHP2ndG" );
			SaveConfig();
		}

		void SaveConfig()
		{
			IO::StreamWriter fout( CFG_FILE );
			fout.WriteLine( L"1" );
			fout.WriteLine( last_lang );
			fout.WriteLine( cmbHunterType->SelectedIndex );
			fout.WriteLine( cmbGender->SelectedIndex );
			fout.WriteLine( cmbPiercings->SelectedIndex );
			fout.WriteLine( cmbAllowBadSkills->SelectedIndex );
			fout.WriteLine( nudHR->Value );
			fout.WriteLine( nudElder->Value );
			fout.WriteLine( nudWeaponSlots->Value );
			fout.WriteLine( cmbSkillFilter1->SelectedIndex );
			fout.WriteLine( cmbSkill1->SelectedIndex );
			fout.WriteLine( cmbSkillFilter2->SelectedIndex );
			fout.WriteLine( cmbSkill2->SelectedIndex );
			fout.WriteLine( cmbSkillFilter3->SelectedIndex );
			fout.WriteLine( cmbSkill3->SelectedIndex );
			fout.WriteLine( cmbSkillFilter4->SelectedIndex );
			fout.WriteLine( cmbSkill4->SelectedIndex );

			fout.WriteLine( txtSolutions->Text->Trim() );
			fout.Close();
		}

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~Form1()
		{
			SaveConfig();
		}


	protected: 


	private: System::Windows::Forms::NumericUpDown^  nudHR;
	private: System::Windows::Forms::NumericUpDown^  nudWeaponSlots;
	private: System::Windows::Forms::NumericUpDown^  nudElder;
	private: System::Windows::Forms::GroupBox^  groupBox1;
	private: System::Windows::Forms::Label^  lblHR;
	private: System::Windows::Forms::Label^  lblElder;
	private: System::Windows::Forms::Label^  lblSlots;
	private: System::Windows::Forms::Button^  btnCancel;
	private: System::Windows::Forms::GroupBox^  groupBox2;
	private: System::Windows::Forms::ComboBox^  cmbPiercings;
	private: System::Windows::Forms::GroupBox^  grpSkills;
	private: System::Windows::Forms::ComboBox^  cmbSkill2;
	private: System::Windows::Forms::ComboBox^  cmbSkill1;
	private: System::Windows::Forms::ComboBox^  cmbSkill4;
	private: System::Windows::Forms::ComboBox^  cmbSkill3;
	private: System::Windows::Forms::Button^  btnSearch;
	private: System::Windows::Forms::ProgressBar^  progressBar1;
	private: System::Windows::Forms::TextBox^  txtSolutions;
	private: System::Windows::Forms::ComboBox^  cmbHunterType;
	private: System::Windows::Forms::GroupBox^  groupBox3;
	private: System::Windows::Forms::ComboBox^  cmbGender;
	private: System::Windows::Forms::GroupBox^  groupBox4;
	private: System::Windows::Forms::GroupBox^  grpResults;
	private: System::Windows::Forms::GroupBox^  grpSkillFilters;
	private: System::Windows::Forms::ComboBox^  cmbSkillFilter4;
	private: System::Windows::Forms::ComboBox^  cmbSkillFilter3;
	private: System::Windows::Forms::ComboBox^  cmbSkillFilter2;
	private: System::Windows::Forms::ComboBox^  cmbSkillFilter1;

	protected: 

	private:
		/// <summary>
		/// Required designer variable.
		/// </summary>
		System::ComponentModel::Container ^components;

#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
			this->nudHR = (gcnew System::Windows::Forms::NumericUpDown());
			this->groupBox1 = (gcnew System::Windows::Forms::GroupBox());
			this->nudWeaponSlots = (gcnew System::Windows::Forms::NumericUpDown());
			this->lblElder = (gcnew System::Windows::Forms::Label());
			this->lblSlots = (gcnew System::Windows::Forms::Label());
			this->nudElder = (gcnew System::Windows::Forms::NumericUpDown());
			this->lblHR = (gcnew System::Windows::Forms::Label());
			this->grpSkills = (gcnew System::Windows::Forms::GroupBox());
			this->cmbSkill4 = (gcnew System::Windows::Forms::ComboBox());
			this->cmbSkill3 = (gcnew System::Windows::Forms::ComboBox());
			this->cmbSkill2 = (gcnew System::Windows::Forms::ComboBox());
			this->cmbSkill1 = (gcnew System::Windows::Forms::ComboBox());
			this->btnSearch = (gcnew System::Windows::Forms::Button());
			this->progressBar1 = (gcnew System::Windows::Forms::ProgressBar());
			this->txtSolutions = (gcnew System::Windows::Forms::TextBox());
			this->cmbHunterType = (gcnew System::Windows::Forms::ComboBox());
			this->groupBox3 = (gcnew System::Windows::Forms::GroupBox());
			this->cmbGender = (gcnew System::Windows::Forms::ComboBox());
			this->groupBox4 = (gcnew System::Windows::Forms::GroupBox());
			this->btnCancel = (gcnew System::Windows::Forms::Button());
			this->grpResults = (gcnew System::Windows::Forms::GroupBox());
			this->grpSkillFilters = (gcnew System::Windows::Forms::GroupBox());
			this->cmbSkillFilter4 = (gcnew System::Windows::Forms::ComboBox());
			this->cmbSkillFilter3 = (gcnew System::Windows::Forms::ComboBox());
			this->cmbSkillFilter2 = (gcnew System::Windows::Forms::ComboBox());
			this->cmbSkillFilter1 = (gcnew System::Windows::Forms::ComboBox());
			this->backgroundWorker1 = (gcnew System::ComponentModel::BackgroundWorker());
			this->groupBox2 = (gcnew System::Windows::Forms::GroupBox());
			this->cmbAllowBadSkills = (gcnew System::Windows::Forms::ComboBox());
			this->cmbPiercings = (gcnew System::Windows::Forms::ComboBox());
			this->menuStrip1 = (gcnew System::Windows::Forms::MenuStrip());
			this->fileToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->exitToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->dataToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->reloadToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->mnuLanguages = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->helpToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->aboutToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->nudHR))->BeginInit();
			this->groupBox1->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->nudWeaponSlots))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->nudElder))->BeginInit();
			this->grpSkills->SuspendLayout();
			this->groupBox3->SuspendLayout();
			this->groupBox4->SuspendLayout();
			this->grpResults->SuspendLayout();
			this->grpSkillFilters->SuspendLayout();
			this->groupBox2->SuspendLayout();
			this->menuStrip1->SuspendLayout();
			this->SuspendLayout();
			// 
			// nudHR
			// 
			this->nudHR->Location = System::Drawing::Point(120, 18);
			this->nudHR->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) {9, 0, 0, 0});
			this->nudHR->Name = L"nudHR";
			this->nudHR->Size = System::Drawing::Size(35, 20);
			this->nudHR->TabIndex = 2;
			this->nudHR->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) {9, 0, 0, 0});
			// 
			// groupBox1
			// 
			this->groupBox1->Controls->Add(this->nudWeaponSlots);
			this->groupBox1->Controls->Add(this->nudHR);
			this->groupBox1->Controls->Add(this->lblElder);
			this->groupBox1->Controls->Add(this->lblSlots);
			this->groupBox1->Controls->Add(this->nudElder);
			this->groupBox1->Controls->Add(this->lblHR);
			this->groupBox1->Location = System::Drawing::Point(12, 72);
			this->groupBox1->Name = L"groupBox1";
			this->groupBox1->Size = System::Drawing::Size(161, 95);
			this->groupBox1->TabIndex = 5;
			this->groupBox1->TabStop = false;
			// 
			// nudWeaponSlots
			// 
			this->nudWeaponSlots->Location = System::Drawing::Point(120, 69);
			this->nudWeaponSlots->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) {3, 0, 0, 0});
			this->nudWeaponSlots->Name = L"nudWeaponSlots";
			this->nudWeaponSlots->Size = System::Drawing::Size(35, 20);
			this->nudWeaponSlots->TabIndex = 7;
			// 
			// lblElder
			// 
			this->lblElder->AutoSize = true;
			this->lblElder->Location = System::Drawing::Point(10, 45);
			this->lblElder->Name = L"lblElder";
			this->lblElder->Size = System::Drawing::Size(65, 13);
			this->lblElder->TabIndex = 4;
			this->lblElder->Text = L"Village Elder";
			// 
			// lblSlots
			// 
			this->lblSlots->AutoSize = true;
			this->lblSlots->Location = System::Drawing::Point(10, 71);
			this->lblSlots->Name = L"lblSlots";
			this->lblSlots->Size = System::Drawing::Size(97, 13);
			this->lblSlots->TabIndex = 1;
			this->lblSlots->Text = L"Max Weapon Slots";
			// 
			// nudElder
			// 
			this->nudElder->Location = System::Drawing::Point(120, 44);
			this->nudElder->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) {9, 0, 0, 0});
			this->nudElder->Name = L"nudElder";
			this->nudElder->Size = System::Drawing::Size(35, 20);
			this->nudElder->TabIndex = 5;
			this->nudElder->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) {9, 0, 0, 0});
			// 
			// lblHR
			// 
			this->lblHR->AutoSize = true;
			this->lblHR->Location = System::Drawing::Point(10, 19);
			this->lblHR->Name = L"lblHR";
			this->lblHR->Size = System::Drawing::Size(23, 13);
			this->lblHR->TabIndex = 6;
			this->lblHR->Text = L"HR";
			// 
			// grpSkills
			// 
			this->grpSkills->Controls->Add(this->cmbSkill4);
			this->grpSkills->Controls->Add(this->cmbSkill3);
			this->grpSkills->Controls->Add(this->cmbSkill2);
			this->grpSkills->Controls->Add(this->cmbSkill1);
			this->grpSkills->Location = System::Drawing::Point(180, 27);
			this->grpSkills->Name = L"grpSkills";
			this->grpSkills->Size = System::Drawing::Size(166, 140);
			this->grpSkills->TabIndex = 8;
			this->grpSkills->TabStop = false;
			this->grpSkills->Text = L"Skills";
			// 
			// cmbSkill4
			// 
			this->cmbSkill4->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->cmbSkill4->FormattingEnabled = true;
			this->cmbSkill4->Location = System::Drawing::Point(6, 100);
			this->cmbSkill4->Name = L"cmbSkill4";
			this->cmbSkill4->Size = System::Drawing::Size(152, 21);
			this->cmbSkill4->TabIndex = 3;
			this->cmbSkill4->SelectedIndexChanged += gcnew System::EventHandler(this, &Form1::cmbSkill4_SelectedIndexChanged);
			// 
			// cmbSkill3
			// 
			this->cmbSkill3->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->cmbSkill3->FormattingEnabled = true;
			this->cmbSkill3->Location = System::Drawing::Point(6, 73);
			this->cmbSkill3->Name = L"cmbSkill3";
			this->cmbSkill3->Size = System::Drawing::Size(152, 21);
			this->cmbSkill3->TabIndex = 2;
			this->cmbSkill3->SelectedIndexChanged += gcnew System::EventHandler(this, &Form1::cmbSkill3_SelectedIndexChanged);
			// 
			// cmbSkill2
			// 
			this->cmbSkill2->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->cmbSkill2->FormattingEnabled = true;
			this->cmbSkill2->Location = System::Drawing::Point(6, 46);
			this->cmbSkill2->Name = L"cmbSkill2";
			this->cmbSkill2->Size = System::Drawing::Size(152, 21);
			this->cmbSkill2->TabIndex = 1;
			this->cmbSkill2->SelectedIndexChanged += gcnew System::EventHandler(this, &Form1::cmbSkill2_SelectedIndexChanged);
			// 
			// cmbSkill1
			// 
			this->cmbSkill1->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->cmbSkill1->FormattingEnabled = true;
			this->cmbSkill1->Location = System::Drawing::Point(6, 19);
			this->cmbSkill1->Name = L"cmbSkill1";
			this->cmbSkill1->Size = System::Drawing::Size(152, 21);
			this->cmbSkill1->TabIndex = 0;
			this->cmbSkill1->SelectedIndexChanged += gcnew System::EventHandler(this, &Form1::cmbSkill1_SelectedIndexChanged);
			// 
			// btnSearch
			// 
			this->btnSearch->Location = System::Drawing::Point(6, 10);
			this->btnSearch->Name = L"btnSearch";
			this->btnSearch->Size = System::Drawing::Size(92, 27);
			this->btnSearch->TabIndex = 9;
			this->btnSearch->Text = L"&Search";
			this->btnSearch->UseVisualStyleBackColor = true;
			this->btnSearch->Click += gcnew System::EventHandler(this, &Form1::btnSearch_Click);
			// 
			// progressBar1
			// 
			this->progressBar1->Location = System::Drawing::Point(12, 218);
			this->progressBar1->Name = L"progressBar1";
			this->progressBar1->Size = System::Drawing::Size(463, 10);
			this->progressBar1->Step = 1;
			this->progressBar1->TabIndex = 10;
			// 
			// txtSolutions
			// 
			this->txtSolutions->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom) 
				| System::Windows::Forms::AnchorStyles::Left) 
				| System::Windows::Forms::AnchorStyles::Right));
			this->txtSolutions->Location = System::Drawing::Point(6, 19);
			this->txtSolutions->Multiline = true;
			this->txtSolutions->Name = L"txtSolutions";
			this->txtSolutions->ScrollBars = System::Windows::Forms::ScrollBars::Vertical;
			this->txtSolutions->Size = System::Drawing::Size(435, 210);
			this->txtSolutions->TabIndex = 11;
			this->txtSolutions->KeyDown += gcnew System::Windows::Forms::KeyEventHandler(this, &Form1::KeyDown);
			// 
			// cmbHunterType
			// 
			this->cmbHunterType->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->cmbHunterType->FormattingEnabled = true;
			this->cmbHunterType->Items->AddRange(gcnew cli::array< System::Object^  >(2) {L"Blademaster", L"Gunner"});
			this->cmbHunterType->Location = System::Drawing::Point(6, 11);
			this->cmbHunterType->Name = L"cmbHunterType";
			this->cmbHunterType->Size = System::Drawing::Size(87, 21);
			this->cmbHunterType->TabIndex = 12;
			// 
			// groupBox3
			// 
			this->groupBox3->Controls->Add(this->cmbGender);
			this->groupBox3->Controls->Add(this->cmbHunterType);
			this->groupBox3->Location = System::Drawing::Point(12, 27);
			this->groupBox3->Name = L"groupBox3";
			this->groupBox3->Size = System::Drawing::Size(162, 39);
			this->groupBox3->TabIndex = 13;
			this->groupBox3->TabStop = false;
			// 
			// cmbGender
			// 
			this->cmbGender->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->cmbGender->FormattingEnabled = true;
			this->cmbGender->Items->AddRange(gcnew cli::array< System::Object^  >(2) {L"Male", L"Female"});
			this->cmbGender->Location = System::Drawing::Point(99, 11);
			this->cmbGender->Name = L"cmbGender";
			this->cmbGender->Size = System::Drawing::Size(57, 21);
			this->cmbGender->TabIndex = 13;
			// 
			// groupBox4
			// 
			this->groupBox4->Controls->Add(this->btnCancel);
			this->groupBox4->Controls->Add(this->btnSearch);
			this->groupBox4->Location = System::Drawing::Point(272, 173);
			this->groupBox4->Name = L"groupBox4";
			this->groupBox4->Size = System::Drawing::Size(203, 41);
			this->groupBox4->TabIndex = 14;
			this->groupBox4->TabStop = false;
			// 
			// btnCancel
			// 
			this->btnCancel->Location = System::Drawing::Point(104, 10);
			this->btnCancel->Name = L"btnCancel";
			this->btnCancel->Size = System::Drawing::Size(92, 27);
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
			this->grpResults->Location = System::Drawing::Point(12, 234);
			this->grpResults->Name = L"grpResults";
			this->grpResults->Size = System::Drawing::Size(463, 251);
			this->grpResults->TabIndex = 15;
			this->grpResults->TabStop = false;
			this->grpResults->Text = L"Results";
			// 
			// grpSkillFilters
			// 
			this->grpSkillFilters->Controls->Add(this->cmbSkillFilter4);
			this->grpSkillFilters->Controls->Add(this->cmbSkillFilter3);
			this->grpSkillFilters->Controls->Add(this->cmbSkillFilter2);
			this->grpSkillFilters->Controls->Add(this->cmbSkillFilter1);
			this->grpSkillFilters->Location = System::Drawing::Point(352, 27);
			this->grpSkillFilters->Name = L"grpSkillFilters";
			this->grpSkillFilters->Size = System::Drawing::Size(123, 140);
			this->grpSkillFilters->TabIndex = 9;
			this->grpSkillFilters->TabStop = false;
			this->grpSkillFilters->Text = L"Skill Filters";
			// 
			// cmbSkillFilter4
			// 
			this->cmbSkillFilter4->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->cmbSkillFilter4->FormattingEnabled = true;
			this->cmbSkillFilter4->Location = System::Drawing::Point(6, 100);
			this->cmbSkillFilter4->Name = L"cmbSkillFilter4";
			this->cmbSkillFilter4->Size = System::Drawing::Size(111, 21);
			this->cmbSkillFilter4->TabIndex = 3;
			this->cmbSkillFilter4->SelectedIndexChanged += gcnew System::EventHandler(this, &Form1::cmbSkillFilter4_SelectedIndexChanged);
			// 
			// cmbSkillFilter3
			// 
			this->cmbSkillFilter3->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->cmbSkillFilter3->FormattingEnabled = true;
			this->cmbSkillFilter3->Location = System::Drawing::Point(6, 73);
			this->cmbSkillFilter3->Name = L"cmbSkillFilter3";
			this->cmbSkillFilter3->Size = System::Drawing::Size(111, 21);
			this->cmbSkillFilter3->TabIndex = 2;
			this->cmbSkillFilter3->SelectedIndexChanged += gcnew System::EventHandler(this, &Form1::cmbSkillFilter3_SelectedIndexChanged);
			// 
			// cmbSkillFilter2
			// 
			this->cmbSkillFilter2->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->cmbSkillFilter2->FormattingEnabled = true;
			this->cmbSkillFilter2->Location = System::Drawing::Point(6, 46);
			this->cmbSkillFilter2->Name = L"cmbSkillFilter2";
			this->cmbSkillFilter2->Size = System::Drawing::Size(111, 21);
			this->cmbSkillFilter2->TabIndex = 1;
			this->cmbSkillFilter2->SelectedIndexChanged += gcnew System::EventHandler(this, &Form1::cmbSkillFilter2_SelectedIndexChanged);
			// 
			// cmbSkillFilter1
			// 
			this->cmbSkillFilter1->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->cmbSkillFilter1->FormattingEnabled = true;
			this->cmbSkillFilter1->Location = System::Drawing::Point(6, 19);
			this->cmbSkillFilter1->Name = L"cmbSkillFilter1";
			this->cmbSkillFilter1->Size = System::Drawing::Size(111, 21);
			this->cmbSkillFilter1->TabIndex = 0;
			this->cmbSkillFilter1->SelectedIndexChanged += gcnew System::EventHandler(this, &Form1::cmbSkillFilter1_SelectedIndexChanged);
			// 
			// backgroundWorker1
			// 
			this->backgroundWorker1->DoWork += gcnew System::ComponentModel::DoWorkEventHandler(this, &Form1::backgroundWorker1_DoWork);
			this->backgroundWorker1->RunWorkerCompleted += gcnew System::ComponentModel::RunWorkerCompletedEventHandler(this, &Form1::backgroundWorker1_RunWorkerCompleted);
			this->backgroundWorker1->ProgressChanged += gcnew System::ComponentModel::ProgressChangedEventHandler(this, &Form1::backgroundWorker1_ProgressChanged);
			// 
			// groupBox2
			// 
			this->groupBox2->Controls->Add(this->cmbAllowBadSkills);
			this->groupBox2->Controls->Add(this->cmbPiercings);
			this->groupBox2->Location = System::Drawing::Point(12, 173);
			this->groupBox2->Name = L"groupBox2";
			this->groupBox2->Size = System::Drawing::Size(254, 39);
			this->groupBox2->TabIndex = 14;
			this->groupBox2->TabStop = false;
			// 
			// cmbAllowBadSkills
			// 
			this->cmbAllowBadSkills->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->cmbAllowBadSkills->FormattingEnabled = true;
			this->cmbAllowBadSkills->Items->AddRange(gcnew cli::array< System::Object^  >(2) {L"Allow Bad Skills", L"Disallow Bad Skills"});
			this->cmbAllowBadSkills->Location = System::Drawing::Point(120, 12);
			this->cmbAllowBadSkills->Name = L"cmbAllowBadSkills";
			this->cmbAllowBadSkills->Size = System::Drawing::Size(128, 21);
			this->cmbAllowBadSkills->TabIndex = 14;
			// 
			// cmbPiercings
			// 
			this->cmbPiercings->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->cmbPiercings->FormattingEnabled = true;
			this->cmbPiercings->Items->AddRange(gcnew cli::array< System::Object^  >(2) {L"Use Piercings", L"No Piercings"});
			this->cmbPiercings->Location = System::Drawing::Point(6, 12);
			this->cmbPiercings->Name = L"cmbPiercings";
			this->cmbPiercings->Size = System::Drawing::Size(108, 21);
			this->cmbPiercings->TabIndex = 13;
			// 
			// menuStrip1
			// 
			this->menuStrip1->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(4) {this->fileToolStripMenuItem, 
				this->dataToolStripMenuItem, this->mnuLanguages, this->helpToolStripMenuItem});
			this->menuStrip1->Location = System::Drawing::Point(0, 0);
			this->menuStrip1->Name = L"menuStrip1";
			this->menuStrip1->Size = System::Drawing::Size(489, 24);
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
			// Form1
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(489, 497);
			this->Controls->Add(this->groupBox2);
			this->Controls->Add(this->groupBox4);
			this->Controls->Add(this->progressBar1);
			this->Controls->Add(this->grpSkillFilters);
			this->Controls->Add(this->grpResults);
			this->Controls->Add(this->groupBox3);
			this->Controls->Add(this->grpSkills);
			this->Controls->Add(this->groupBox1);
			this->Controls->Add(this->menuStrip1);
			this->Name = L"Form1";
			this->Text = L"Athena\'s A.S.S.";
			this->Load += gcnew System::EventHandler(this, &Form1::Form1_Load);
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->nudHR))->EndInit();
			this->groupBox1->ResumeLayout(false);
			this->groupBox1->PerformLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->nudWeaponSlots))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->nudElder))->EndInit();
			this->grpSkills->ResumeLayout(false);
			this->groupBox3->ResumeLayout(false);
			this->groupBox4->ResumeLayout(false);
			this->grpResults->ResumeLayout(false);
			this->grpResults->PerformLayout();
			this->grpSkillFilters->ResumeLayout(false);
			this->groupBox2->ResumeLayout(false);
			this->menuStrip1->ResumeLayout(false);
			this->menuStrip1->PerformLayout();
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion


private: 
	System::Void btnSearch_Click(System::Object^  sender, System::EventArgs^  e)
	{
		progressBar1->Value = 0;
		query = gcnew Query;
		query->weapon_slots_allowed = int( nudWeaponSlots->Value );
		query->elder_star = int( nudElder->Value );
		query->hr = int( nudHR->Value );
		query->gender = cmbGender->SelectedIndex == 0 ? Gender::MALE : Gender::FEMALE;
		query->hunter_type = cmbHunterType->SelectedIndex == 0 ? HunterType::BLADEMASTER : HunterType::GUNNER;
		query->include_piercings = cmbPiercings->SelectedIndex == 0;
		query->allow_bad = cmbAllowBadSkills->SelectedIndex == 0;
		query->allow_torso_inc = true;

		if( cmbSkill1->SelectedIndex >= 0 ) query->skills.Add( data->FindSkill( imSkill1[ cmbSkill1->SelectedIndex ] ) );
		if( cmbSkill2->SelectedIndex >= 0 ) query->skills.Add( data->FindSkill( imSkill2[ cmbSkill2->SelectedIndex ] ) );
		if( cmbSkill3->SelectedIndex >= 0 ) query->skills.Add( data->FindSkill( imSkill3[ cmbSkill3->SelectedIndex ] ) );
		if( cmbSkill4->SelectedIndex >= 0 ) query->skills.Add( data->FindSkill( imSkill4[ cmbSkill4->SelectedIndex ] ) );
		
		if( query->skills.Count > 0 )
		{
			btnSearch->Enabled = false;
			btnCancel->Enabled = true;
			txtSolutions->Clear();
			backgroundWorker1->RunWorkerAsync( query );
		}
	}

	System::Void btnCancel_Click(System::Object^  sender, System::EventArgs^  e)
	{
		backgroundWorker1->CancelAsync();
		btnCancel->Enabled = false;
		btnSearch->Enabled = true;
		progressBar1->Value = 0;
	}

	System::Void Form1_Load(System::Object^  sender, System::EventArgs^  e)
	{

	}
#pragma region Skill stuff
	System::Void cmbSkillFilter1_SelectedIndexChanged(System::Object^  sender, System::EventArgs^  e)
	{
		Skill^ sk1 = cmbSkill1->SelectedIndex == -1 ? nullptr : data->skills[ imSkill1[ cmbSkill1->SelectedIndex ] ];
		Skill^ sk2 = cmbSkill2->SelectedIndex == -1 ? nullptr : data->skills[ imSkill2[ cmbSkill2->SelectedIndex ] ];
		Skill^ sk3 = cmbSkill3->SelectedIndex == -1 ? nullptr : data->skills[ imSkill3[ cmbSkill3->SelectedIndex ] ];
		Skill^ sk4 = cmbSkill4->SelectedIndex == -1 ? nullptr : data->skills[ imSkill4[ cmbSkill4->SelectedIndex ] ];
		InitSkills( cmbSkill1, %imSkill1, cmbSkillFilter1->SelectedIndex, sk2, sk3, sk4 );
		ResetSkill( cmbSkill1, %imSkill1, sk1 );
	}

	System::Void cmbSkillFilter2_SelectedIndexChanged(System::Object^  sender, System::EventArgs^  e)
	{
		Skill^ sk1 = cmbSkill1->SelectedIndex == -1 ? nullptr : data->skills[ imSkill1[ cmbSkill1->SelectedIndex ] ];
		Skill^ sk2 = cmbSkill2->SelectedIndex == -1 ? nullptr : data->skills[ imSkill2[ cmbSkill2->SelectedIndex ] ];
		Skill^ sk3 = cmbSkill3->SelectedIndex == -1 ? nullptr : data->skills[ imSkill3[ cmbSkill3->SelectedIndex ] ];
		Skill^ sk4 = cmbSkill4->SelectedIndex == -1 ? nullptr : data->skills[ imSkill4[ cmbSkill4->SelectedIndex ] ];
		InitSkills( cmbSkill2, %imSkill2, cmbSkillFilter2->SelectedIndex, sk1, sk3, sk4 );
		ResetSkill( cmbSkill2, %imSkill2, sk2 );
	}

	System::Void cmbSkillFilter3_SelectedIndexChanged(System::Object^  sender, System::EventArgs^  e)
	{
		Skill^ sk1 = cmbSkill1->SelectedIndex == -1 ? nullptr : data->skills[ imSkill1[ cmbSkill1->SelectedIndex ] ];
		Skill^ sk2 = cmbSkill2->SelectedIndex == -1 ? nullptr : data->skills[ imSkill2[ cmbSkill2->SelectedIndex ] ];
		Skill^ sk3 = cmbSkill3->SelectedIndex == -1 ? nullptr : data->skills[ imSkill3[ cmbSkill3->SelectedIndex ] ];
		Skill^ sk4 = cmbSkill4->SelectedIndex == -1 ? nullptr : data->skills[ imSkill4[ cmbSkill4->SelectedIndex ] ];
		InitSkills( cmbSkill3, %imSkill3, cmbSkillFilter3->SelectedIndex, sk1, sk2, sk4 );
		ResetSkill( cmbSkill3, %imSkill3, sk3 );
	}

	System::Void cmbSkillFilter4_SelectedIndexChanged(System::Object^  sender, System::EventArgs^  e)
	{
		Skill^ sk1 = cmbSkill1->SelectedIndex == -1 ? nullptr : data->skills[ imSkill1[ cmbSkill1->SelectedIndex ] ];
		Skill^ sk2 = cmbSkill2->SelectedIndex == -1 ? nullptr : data->skills[ imSkill2[ cmbSkill2->SelectedIndex ] ];
		Skill^ sk3 = cmbSkill3->SelectedIndex == -1 ? nullptr : data->skills[ imSkill3[ cmbSkill3->SelectedIndex ] ];
		Skill^ sk4 = cmbSkill4->SelectedIndex == -1 ? nullptr : data->skills[ imSkill4[ cmbSkill4->SelectedIndex ] ];
		InitSkills( cmbSkill4, %imSkill4, cmbSkillFilter4->SelectedIndex, sk1, sk2, sk3 );
		ResetSkill( cmbSkill4, %imSkill4, sk4 );
	}

	System::Void cmbSkill1_SelectedIndexChanged(System::Object^  sender, System::EventArgs^  e) 
	{
		if( lock_skills ) return;
		if( cmbSkill1->SelectedIndex == 0 )
		{
			cmbSkill1->SelectedIndex = -1;
			return;
		}
		lock_skills = true;
		Skill^ sk1 = cmbSkill1->SelectedIndex == -1 ? nullptr : data->skills[ imSkill1[ cmbSkill1->SelectedIndex ] ];
		Skill^ sk2 = cmbSkill2->SelectedIndex == -1 ? nullptr : data->skills[ imSkill2[ cmbSkill2->SelectedIndex ] ];
		Skill^ sk3 = cmbSkill3->SelectedIndex == -1 ? nullptr : data->skills[ imSkill3[ cmbSkill3->SelectedIndex ] ];
		Skill^ sk4 = cmbSkill4->SelectedIndex == -1 ? nullptr : data->skills[ imSkill4[ cmbSkill4->SelectedIndex ] ];
		InitSkills( cmbSkill2, %imSkill2, cmbSkillFilter2->SelectedIndex, sk1, sk3, sk4 );
		ResetSkill( cmbSkill2, %imSkill2, sk2 );
		InitSkills( cmbSkill3, %imSkill3, cmbSkillFilter3->SelectedIndex, sk1, sk2, sk4 );
		ResetSkill( cmbSkill3, %imSkill3, sk3 );
		InitSkills( cmbSkill4, %imSkill4, cmbSkillFilter4->SelectedIndex, sk1, sk2, sk3 );
		ResetSkill( cmbSkill4, %imSkill4, sk4 );
		lock_skills = false;
	}

	System::Void cmbSkill2_SelectedIndexChanged(System::Object^  sender, System::EventArgs^  e)
	{
		if( lock_skills ) return;
		if( cmbSkill2->SelectedIndex == 0 )
		{
			cmbSkill2->SelectedIndex = -1;
			return;
		}
		lock_skills = true;
		Skill^ sk1 = cmbSkill1->SelectedIndex == -1 ? nullptr : data->skills[ imSkill1[ cmbSkill1->SelectedIndex ] ];
		Skill^ sk2 = cmbSkill2->SelectedIndex == -1 ? nullptr : data->skills[ imSkill2[ cmbSkill2->SelectedIndex ] ];
		Skill^ sk3 = cmbSkill3->SelectedIndex == -1 ? nullptr : data->skills[ imSkill3[ cmbSkill3->SelectedIndex ] ];
		Skill^ sk4 = cmbSkill4->SelectedIndex == -1 ? nullptr : data->skills[ imSkill4[ cmbSkill4->SelectedIndex ] ];
		InitSkills( cmbSkill1, %imSkill1, cmbSkillFilter1->SelectedIndex, sk2, sk3, sk4 );
		ResetSkill( cmbSkill1, %imSkill1, sk1 );
		InitSkills( cmbSkill3, %imSkill3, cmbSkillFilter3->SelectedIndex, sk1, sk2, sk4 );
		ResetSkill( cmbSkill3, %imSkill3, sk3 );
		InitSkills( cmbSkill4, %imSkill4, cmbSkillFilter4->SelectedIndex, sk1, sk2, sk3 );
		ResetSkill( cmbSkill4, %imSkill4, sk4 );
		lock_skills = false;
	}

	System::Void cmbSkill3_SelectedIndexChanged(System::Object^  sender, System::EventArgs^  e)
	{
		if( lock_skills ) return;
		if( cmbSkill3->SelectedIndex == 0 )
		{
			cmbSkill3->SelectedIndex = -1;
			return;
		}
		lock_skills = true;
		Skill^ sk1 = cmbSkill1->SelectedIndex == -1 ? nullptr : data->skills[ imSkill1[ cmbSkill1->SelectedIndex ] ];
		Skill^ sk2 = cmbSkill2->SelectedIndex == -1 ? nullptr : data->skills[ imSkill2[ cmbSkill2->SelectedIndex ] ];
		Skill^ sk3 = cmbSkill3->SelectedIndex == -1 ? nullptr : data->skills[ imSkill3[ cmbSkill3->SelectedIndex ] ];
		Skill^ sk4 = cmbSkill4->SelectedIndex == -1 ? nullptr : data->skills[ imSkill4[ cmbSkill4->SelectedIndex ] ];
		InitSkills( cmbSkill1, %imSkill1, cmbSkillFilter1->SelectedIndex, sk2, sk3, sk4 );
		ResetSkill( cmbSkill1, %imSkill1, sk1 );
		InitSkills( cmbSkill2, %imSkill2, cmbSkillFilter2->SelectedIndex, sk1, sk3, sk4 );
		ResetSkill( cmbSkill2, %imSkill2, sk2 );
		InitSkills( cmbSkill4, %imSkill4, cmbSkillFilter4->SelectedIndex, sk1, sk2, sk3 );
		ResetSkill( cmbSkill4, %imSkill4, sk4 );
		lock_skills = false;
	}

	System::Void cmbSkill4_SelectedIndexChanged(System::Object^  sender, System::EventArgs^  e)
	{
		if( lock_skills ) return;
		if( cmbSkill4->SelectedIndex == 0 )
		{
			cmbSkill4->SelectedIndex = -1;
			return;
		}
		lock_skills = true;
		Skill^ sk1 = cmbSkill1->SelectedIndex == -1 ? nullptr : data->skills[ imSkill1[ cmbSkill1->SelectedIndex ] ];
		Skill^ sk2 = cmbSkill2->SelectedIndex == -1 ? nullptr : data->skills[ imSkill2[ cmbSkill2->SelectedIndex ] ];
		Skill^ sk3 = cmbSkill3->SelectedIndex == -1 ? nullptr : data->skills[ imSkill3[ cmbSkill3->SelectedIndex ] ];
		Skill^ sk4 = cmbSkill4->SelectedIndex == -1 ? nullptr : data->skills[ imSkill4[ cmbSkill4->SelectedIndex ] ];
		InitSkills( cmbSkill1, %imSkill1, cmbSkillFilter1->SelectedIndex, sk2, sk3, sk4 );
		ResetSkill( cmbSkill1, %imSkill1, sk1 );
		InitSkills( cmbSkill2, %imSkill2, cmbSkillFilter2->SelectedIndex, sk1, sk3, sk4 );
		ResetSkill( cmbSkill2, %imSkill2, sk2 );
		InitSkills( cmbSkill3, %imSkill3, cmbSkillFilter3->SelectedIndex, sk1, sk2, sk4 );
		ResetSkill( cmbSkill3, %imSkill3, sk3 );
		lock_skills = false;
	}
#pragma endregion

#pragma region Worker Thread Stuff
	String^ CreateResultString( List< Solution^ >^ solutions )
	{
		System::Text::StringBuilder sb( solutions->Count * 1024 );
		int limit = solutions->Count;

		if( limit >= MAX_LIMIT )
		{
			sb.Append( L"Showing first " )->Append( Convert::ToString( MAX_LIMIT ) )->Append( L" solutions only" )->Append( endl );
			limit = MAX_LIMIT;
		}
		else sb.Append( L"Solutions found: " )->Append( Convert::ToString( solutions->Count ) )->Append( endl );
		
		System::String^ dash = L"-----------------" + endl;
		for( int i = 0; i < limit; ++i )
		{
			sb.Append( endl );
			for( int j = 0; j < solutions[ i ]->armors.Count; ++j )
				sb.Append( solutions[ i ]->armors[ j ]->name )->Append( endl );
			if( solutions[ i ]->decorations.Count > 0 )
			{
				sb.Append( dash );
				Generic::Dictionary< Decoration^, unsigned > deco_dict;
				for( int j = 0; j < solutions[ i ]->decorations.Count; ++j )
				{
					if( !deco_dict.ContainsKey( solutions[ i ]->decorations[ j ] ) )
						deco_dict.Add( solutions[ i ]->decorations[ j ], 1 );
					else deco_dict[ solutions[ i ]->decorations[ j ] ]++;
				}
				Generic::Dictionary< Decoration^, unsigned >::Enumerator iter = deco_dict.GetEnumerator();
				while( iter.MoveNext() )
					sb.Append( Convert::ToString( iter.Current.Value ) )->Append( L"x " )->Append( iter.Current.Key->name )->Append( endl );
			}
			if( solutions[ i ]->extra_weapon_slots > 0 )
			{
				sb.Append( Convert::ToString( solutions[ i ]->extra_weapon_slots ) );
				if( solutions[ i ]->extra_weapon_slots == 1 )
					sb.Append( L" slot spare" )->Append( endl );
				else
					sb.Append( L" slots spare" )->Append( endl );
			}
			if( query->skills.Count < solutions[ i ]->skills.Count )
			{
				sb.Append( dash );
				for( int j = 0; j < solutions[ i ]->skills.Count; ++j )
				{
					Skill^ skill = solutions[ i ]->skills[ j ];
					if( !Utility::Contains( %query->skills, skill ) )
						sb.Append( skill->name )->Append( endl );
				}
			}
		}
		return sb.ToString();
	}

	System::Void backgroundWorker1_RunWorkerCompleted( Object^ /*sender*/, RunWorkerCompletedEventArgs^ e )
	{
		btnSearch->Enabled = true;
		btnCancel->Enabled = false;

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
		else progressBar1->Value = 100;

		txtSolutions->Text = CreateResultString( static_cast< List< Solution^ >^ >( e->Result ) )->Normalize();

		SaveConfig();
	}

	System::Void backgroundWorker1_ProgressChanged( Object^ /*sender*/, ProgressChangedEventArgs^ e )
	{
		progressBar1->Value = e->ProgressPercentage;
		txtSolutions->Text = static_cast< String^ >( e->UserState );
	}

	System::Void backgroundWorker1_DoWork( System::Object^ sender, System::ComponentModel::DoWorkEventArgs^ e )
	{
		BackgroundWorker^ worker = static_cast< BackgroundWorker^ >( sender );
		Query^ query = static_cast< Query^ >( e->Argument );

		List< List< Armor^ >^ > rel_armor;
		for( int i = 0; i < int( Armor::ArmorType::NumArmorTypes ); ++i )
			rel_armor.Add( gcnew List< Armor^ >() );
		List< Ability^ > rel_abilities;
		List< Decoration^ > rel_decorations;

		data->GetRelevantData( query, %rel_abilities, %rel_armor, %rel_decorations );

#define MATCH( a, b, c, d, e ) i == a && j == b && k == c && l == d && m == e

		List< Solution ^ >^ solutions = gcnew List< Solution^ >;
		List< Armor^ >^ head  = rel_armor[ int( Armor::ArmorType::HEAD ) ];
		List< Armor^ >^ body  = rel_armor[ int( Armor::ArmorType::BODY ) ];
		List< Armor^ >^ arms  = rel_armor[ int( Armor::ArmorType::ARMS ) ];
		List< Armor^ >^ waist = rel_armor[ int( Armor::ArmorType::WAIST ) ];
		List< Armor^ >^ legs  = rel_armor[ int( Armor::ArmorType::LEGS ) ];
		
		int solutions_found = 0;
		for( int i = 0; i < head->Count; ++i )
		{
			for( int j = 0; j < body->Count; ++j )
			{
				const int progress = ( 100 * ( i * body->Count + j ) ) / ( head->Count * body->Count );
				backgroundWorker1->ReportProgress( progress, CreateResultString( solutions ) );
				for( int k = 0; k < arms->Count; ++k )
				{
					for( int l = 0; l < waist->Count; ++l )
					{
						for( int m = 0; m < legs->Count; ++m )
						{
							if( backgroundWorker1->CancellationPending )
							{
								//e->Cancel = true;
								//backgroundWorker1->ReportProgress( 0 );
								e->Result = solutions;
								return;
							}
							Solution^ job = gcnew Solution();
							job->armors.Add( head[ i ] );
							job->armors.Add( body[ j ] );
							job->armors.Add( arms[ k ] );
							job->armors.Add( waist[ l ] );
							job->armors.Add( legs[ m ] );

							const bool cool = MATCH( 2, 1, 3, 6, 8 );

							if( job->MatchesQuery( query->skills, query->weapon_slots_allowed, rel_decorations, query->allow_bad ) )
							{
								solutions->Add( job );
								if( ++solutions_found >= MAX_LIMIT )
								{
									e->Result = solutions;
									return;
								}
							}							
						}
					}
				}
			}
		}
		e->Result = solutions;
	}
#pragma endregion

	System::Void KeyDown( System::Object^ sender, System::Windows::Forms::KeyEventArgs^ e )
	{
		if( sender == txtSolutions && e->Control && e->KeyValue == L'A' )
		{
			txtSolutions->SelectAll();
			e->Handled = true;
			e->SuppressKeyPress = true;
		}
	}

	System::Void exitToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e)
	{
		Close();
	}

	template< class T >
	System::Void ApplyLanguage( List< String^ >^ lang, List< T >^ x, String^ file )
	{
		if( lang->Count < x->Count )
		{
			MessageBox::Show( this, L"Error: " + file + L" is invalid" );
			return;
		}
		for( int i = 0; i < lang->Count; ++i )
			x[ i ]->name = lang[ i ];
	}

	template<>
	System::Void ApplyLanguage( List< String^ >^ lang, List< String^ >^ x, String^ file )
	{
		if( lang->Count < x->Count )
		{
			MessageBox::Show( this, L"Error: " + file + L" is invalid" );
			return;
		}
		const int limit = std::min( lang->Count, x->Count );
		for( int i = 0; i < limit; ++i )
			x[ i ] = lang[ i ];
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
			}
		}
	}

	System::Void LanguageSelect_Click(System::Object^  sender, System::EventArgs^  e)
	{
		const int skill1 = cmbSkill1->SelectedIndex;
		const int skill2 = cmbSkill2->SelectedIndex;
		const int skill3 = cmbSkill3->SelectedIndex;
		const int skill4 = cmbSkill4->SelectedIndex;
		const int skillf1 = cmbSkillFilter1->SelectedIndex;
		const int skillf2 = cmbSkillFilter2->SelectedIndex;
		const int skillf3 = cmbSkillFilter3->SelectedIndex;
		const int skillf4 = cmbSkillFilter4->SelectedIndex;

		ToolStripMenuItem^ item = static_cast< ToolStripMenuItem^ >( sender );
		last_lang = item->Text;
		const int index = mnuLanguages->DropDownItems->IndexOf( item );
		Language^ lang = data->LoadLanguage( index );
		if( lang->string_table.Count < 23 )
		{
			MessageBox::Show( this, L"Error: string.txt is invalid" );
			return;
		}
		fileToolStripMenuItem->Text = lang->string_table[ 0 ];
		dataToolStripMenuItem->Text = lang->string_table[ 1 ];
		mnuLanguages->Text = lang->string_table[ 2 ];
		helpToolStripMenuItem->Text = lang->string_table[ 3 ];
		exitToolStripMenuItem->Text = lang->string_table[ 4 ];
		reloadToolStripMenuItem->Text = lang->string_table[ 5 ];
		aboutToolStripMenuItem->Text = lang->string_table[ 6 ];
		cmbHunterType->Items[ 0 ] = lang->string_table[ 7 ];
		cmbHunterType->Items[ 1 ] = lang->string_table[ 8 ];
		cmbGender->Items[ 0 ] = lang->string_table[ 9 ];
		cmbGender->Items[ 1 ] = lang->string_table[ 10 ];
		lblHR->Text = lang->string_table[ 11 ];
		lblElder->Text = lang->string_table[ 12 ];
		lblSlots->Text = lang->string_table[ 13 ];
		grpSkills->Text = lang->string_table[ 14 ];
		grpSkillFilters->Text = lang->string_table[ 15 ];
		cmbPiercings->Items[ 0 ] = lang->string_table[ 16 ];
		cmbPiercings->Items[ 1 ] = lang->string_table[ 17 ];
		cmbAllowBadSkills->Items[ 1 ] = lang->string_table[ 18 ];
		cmbAllowBadSkills->Items[ 0 ] = lang->string_table[ 19 ];
		btnSearch->Text = lang->string_table[ 20 ];
		btnCancel->Text = lang->string_table[ 21 ];
		grpResults->Text = lang->string_table[ 22 ];
		ApplyLanguage( %lang->abilities, %data->abilities, L"skills.txt" );
		ApplyLanguage( %lang->skills, %data->skills, L"skills.txt" );
		ApplyLanguage( %lang->decorations, %data->decorations, L"decorations.txt" );
		ApplyLanguage( %lang->tags, %data->tags, L"tags.txt" );
		ApplyLanguage( lang->armors[ 0 ], data->armors[ 0 ], L"head.txt" );
		ApplyLanguage( lang->armors[ 1 ], data->armors[ 1 ], L"body.txt" );
		ApplyLanguage( lang->armors[ 2 ], data->armors[ 2 ], L"arms.txt" );
		ApplyLanguage( lang->armors[ 3 ], data->armors[ 3 ], L"waist.txt" );
		ApplyLanguage( lang->armors[ 4 ], data->armors[ 4 ], L"legs.txt" );

		for each( ToolStripMenuItem^ thang in mnuLanguages->DropDownItems )
			thang->Checked = false;
		item->Checked = true;

		lock_skills = true;
		for( int i = 0; i < cmbSkillFilter1->Items->Count; ++i )
			cmbSkillFilter1->Items[ i ] = data->tags[ i ];
		for( int i = 0; i < cmbSkillFilter2->Items->Count; ++i )
			cmbSkillFilter2->Items[ i ] = data->tags[ i ];
		for( int i = 0; i < cmbSkillFilter3->Items->Count; ++i )
			cmbSkillFilter3->Items[ i ] = data->tags[ i ];
		for( int i = 0; i < cmbSkillFilter4->Items->Count; ++i )
			cmbSkillFilter4->Items[ i ] = data->tags[ i ];
		lock_skills = false;
	}
	
	System::Void reloadToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e)
	{
		cmbSkill1->SelectedIndex = -1;
		cmbSkill2->SelectedIndex = -1;
		cmbSkill3->SelectedIndex = -1;
		cmbSkill4->SelectedIndex = -1;
		
		data->ImportTextFiles();
		mnuLanguages->DropDownItems->Clear();
		for each( String^ lang in data->languages )
		{
			ToolStripMenuItem^ item = gcnew ToolStripMenuItem( lang );
			item->Click += gcnew System::EventHandler(this, &Form1::LanguageSelect_Click);
			mnuLanguages->DropDownItems->Add( item );
		}
		InitFilters();
		cmbSkillFilter1->SelectedIndex = 0;
		cmbSkillFilter2->SelectedIndex = 0;
		cmbSkillFilter3->SelectedIndex = 0;
		cmbSkillFilter4->SelectedIndex = 0;

		LoadLanguage( last_lang );
	}
	System::Void aboutToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e)
	{
		frmAbout about_form;
		about_form.ShowDialog( this );
	}
};
}

