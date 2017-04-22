#ifndef CPUREGISTERSDLG_H
#define CPUREGISTERSDLG_H

#include <cbdebugger_interfaces.h>
#include <tinyxml/tinyxml.h>
#include <tinywxuni.h>

#ifndef WX_PRECOMP
	//(*HeadersPCH(CPURegistersDlg)
	#include <wx/panel.h>
	#include <wx/sizer.h>
	//*)
#endif
//(*Headers(CPURegistersDlg)
#include <wx/propgrid/manager.h>
//*)

class CPURegistersDlg: public wxPanel, public cbCPURegistersDlg
{
	public:

		CPURegistersDlg(wxWindow* parent);
		virtual ~CPURegistersDlg();

		wxWindow* GetWindow()       {return this;};
		void SetRegisterValue(const wxString& reg_name, const wxString& hexValue, const wxString& interpreted);
		void SetPeripheralValue(const wxString& reg_name, const wxString& hexValue);
		void SvdParser();
		wxString CalcOffset( const wxString & baseAddress, const wxString & offset );
        TiXmlElement* GetDerivedPeriphal(TiXmlElement *peripheral, const char* derivedName);
        TiXmlElement* GetFirstPeripheral(const wxString& m_SvdPath);
        wxArrayString GetBaseAddressList();
        wxPGProperty* GetChildPropertyByName( const wxString& name ) const;
		void EnableWindow(bool en);

		void Clear();

		void OnSearchCtrl(wxCommandEvent& event);
		void OnPropertyChanged(wxPropertyGridEvent &event);
        void OnPropertyChanging(wxPropertyGridEvent &event);
        void OnProjectActivated(CodeBlocksEvent& event);
        void OnProjectClosed(CodeBlocksEvent& event);


		wxTextCtrl* m_SearchCtrl;

		//(*Declarations(CPURegistersDlg)
		wxPropertyGridManager* PropGridManager;
		//*)

	protected:

		//(*Identifiers(CPURegistersDlg)
		static const long ID_CUSTOM1;
		//*)

		static const long ID_SEARCH_CTRL;

	private:

		//(*Handlers(CPURegistersDlg)
		//*)

		wxPropertyGridPage* m_cpu_register_page;
		wxPGProperty*       m_cpu_register_node;
		wxPGProperty*       m_per_register_node;

		DECLARE_EVENT_TABLE()
};

#endif
