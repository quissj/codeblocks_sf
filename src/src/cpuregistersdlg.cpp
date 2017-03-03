#include "include/sdk.h"
#include "CPURegistersDlg.h"

#ifndef WX_PRECOMP
	//(*InternalHeadersPCH(CPURegistersDlg)
	#include <wx/intl.h>
	#include <wx/string.h>
	//*)
#endif
//(*InternalHeaders(CPURegistersDlg)
//*)

//(*IdInit(CPURegistersDlg)
const long CPURegistersDlg::ID_CUSTOM1 = wxNewId();
//*)
const long CPURegistersDlg::ID_SEARCH_CTRL = wxNewId();

BEGIN_EVENT_TABLE(CPURegistersDlg,wxPanel)
	//(*EventTable(CPURegistersDlg)
	//*)
	EVT_TEXT( ID_SEARCH_CTRL, CPURegistersDlg::OnSearchCtrl )
END_EVENT_TABLE()

CPURegistersDlg::CPURegistersDlg(wxWindow* parent)
{
	//(*Initialize(CPURegistersDlg)
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer1 = new wxFlexGridSizer(1, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer1->AddGrowableRow(0);
	PropGridManager = new wxPropertyGridManager(this,ID_CUSTOM1,wxDefaultPosition,wxSize(195,85),wxPG_TOOLBAR| wxPG_DESCRIPTION| wxPG_BOLD_MODIFIED| wxPG_SPLITTER_AUTO_CENTER,_T("ID_CUSTOM1"));
	FlexGridSizer1->Add(PropGridManager, 1, wxALL|wxEXPAND, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);
	//*)



	#if wxCHECK_VERSION(3, 0, 0)
        m_cpu_register_page = PropGridManager->AddPage(_("CPU register"));
    #else
        int page = PropGridManager->AddPage(_("CPU register"));
        m_cpu_register_page = PropGridManager->GetPage(page);
    #endif

	m_cpu_register_node = m_cpu_register_page->Append(new wxPropertyCategory(_("CPU"), _("CPU")));
	m_per_register_node = m_cpu_register_page->Append(new wxPropertyCategory(_("periphery"), _("periphery")));

	wxToolBar* toolbar = PropGridManager->GetToolBar();
	wxSize ToolSize = toolbar->GetToolSize();
	int search_control_pos = ToolSize.GetWidth() * toolbar->GetToolsCount() + 10;
	m_SearchCtrl = new wxTextCtrl(PropGridManager->GetToolBar(), ID_SEARCH_CTRL, _(""), wxPoint(search_control_pos, 0), wxSize(150, ToolSize.GetHeight()) );

	toolbar->AddControl(m_SearchCtrl);

	m_cpu_register_page->SetColumnCount(3);

}

CPURegistersDlg::~CPURegistersDlg()
{
	//(*Destroy(CPURegistersDlg)
	//*)
}

bool FindString(wxString a, wxString b)
{
    int match = 0;
    for(size_t i = 0 ; i < a.length();i++)
    {
        size_t k = 0;
        for(k = 0; k < b.length() && i + k < a.length();k++)
        {
            if(a[i+k] != b[k]) break;
        }
        if(k == b.length())
            return true;
    }
    return false;
}

void CPURegistersDlg::OnSearchCtrl(wxCommandEvent& event)
{
    size_t child_count = m_cpu_register_node->GetChildCount();
   wxString searchStr = event.GetString();

   if(searchStr == wxEmptyString)
   {
        for (size_t i = 0 ; i < child_count; i++)
           m_cpu_register_node->Item(i)->Hide( false );

        m_cpu_register_page->ExpandAll();
        return;
   }

   for (size_t i = 0 ; i < child_count; i++)
   {
       wxPGProperty* prop = m_cpu_register_node->Item(i);
       if (FindString(prop->GetLabel(),searchStr) == false )
           prop->Hide( true );
       else
           prop->Hide( false );
   }
    m_cpu_register_page->ExpandAll();

}

void CPURegistersDlg::SetRegisterValue(const wxString& reg_name, const wxString& hexValue, const wxString& interpreted)
{
    wxPGProperty* prop = m_cpu_register_node->GetPropertyByName(reg_name);
    if(prop == nullptr)
    {
        prop = m_cpu_register_node->AppendChild( new wxStringProperty(reg_name, reg_name));
    }
    prop->SetValue(hexValue);
    m_cpu_register_page->SetPropertyAttribute(prop, wxT("Units"), interpreted);
}

void CPURegistersDlg::EnableWindow(bool en)
{

}

void CPURegistersDlg::Clear()
{

}
