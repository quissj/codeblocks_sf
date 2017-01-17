#ifndef CONFIGPANEL_H
#define CONFIGPANEL_H

//(*Headers(ConfigPanel)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/radiobox.h>
#include <wx/checkbox.h>
#include <wx/slider.h>
#include <wx/panel.h>
//*)

class ConfigPanel: public wxPanel
{
	public:

		ConfigPanel(wxWindow* parent, wxWindowID &id);
		virtual ~ConfigPanel();

		//(*Declarations(ConfigPanel)
		wxCheckBox* Cfg_BrowseMarksEnabled;
		wxRadioBox* Cfg_ToggleKey;
		wxStaticText* StaticText2;
		wxStaticText* StaticText1;
		wxSlider* Cfg_LeftMouseDelay;
		wxRadioBox* Cfg_MarkStyle;
		wxRadioBox* Cfg_ClearAllKey;
		wxCheckBox* Cfg_WrapJumpEntries;
		//*)

	protected:

		//(*Identifiers(ConfigPanel)
		static const long ID_STATICTEXT1;
		static const long ID_CHECKBOX1;
		static const long ID_CHECKBOX2;
		static const long ID_RADIOBOX1;
		static const long ID_RADIOBOX3;
		static const long ID_SLIDER1;
		static const long ID_RADIOBOX2;
		static const long ID_STATICTEXT2;
		//*)

	private:

		//(*Handlers(ConfigPanel)
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
