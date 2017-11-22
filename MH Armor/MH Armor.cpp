// MH Armor.cpp : main project file.

#include "stdafx.h"
#include "Form1.h"
#include "Skill.h"

using namespace MHArmor;

[STAThread]
int main(array<System::String ^> ^args)
{
	// Enabling Windows XP visual effects before any controls are created
	Application::EnableVisualStyles();
	Application::SetCompatibleTextRenderingDefault(false); 

	// Create the main window and run it
	try
	{
		Application::Run(gcnew Form1());
	}
	catch( System::Reflection::TargetInvocationException^ e )
	{
		throw e->InnerException;
	}
	
	return 0;
}
