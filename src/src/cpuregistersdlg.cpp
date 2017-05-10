#include "include/sdk.h"
#include "CPURegistersDlg.h"
#include "svdparser.h"

#include "cbcolourmanager.h"

#ifdef wxUSE_UNICODE
    #define _U(x) wxString((x),wxConvUTF8)
    #define _UU(x,y) wxString((x),y)
#else
    #define _U(x) (x)
    #define _UU(x,y) (x)
#endif

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

    EVT_PG_CHANGED(ID_CUSTOM1, CPURegistersDlg::OnPropertyChanged)
    EVT_PG_CHANGING(ID_CUSTOM1, CPURegistersDlg::OnPropertyChanging)
    EVT_PG_ITEM_EXPANDED(ID_CUSTOM1, CPURegistersDlg::OnExpand)
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
	unsigned int search_control_pos = ToolSize.GetWidth() * toolbar->GetToolsCount() + 10;
	m_SearchCtrl = new wxTextCtrl(PropGridManager->GetToolBar(), ID_SEARCH_CTRL, _(""), wxPoint(search_control_pos, 0), wxSize(150, ToolSize.GetHeight()) );

	toolbar->AddControl(m_SearchCtrl);

	m_cpu_register_page->SetColumnCount(3);


    Manager::Get()->RegisterEventSink(cbEVT_PROJECT_ACTIVATE,   new cbEventFunctor<CPURegistersDlg, CodeBlocksEvent>(this, &CPURegistersDlg::OnProjectActivated));
    Manager::Get()->RegisterEventSink(cbEVT_PROJECT_CLOSE,      new cbEventFunctor<CPURegistersDlg, CodeBlocksEvent>(this, &CPURegistersDlg::OnProjectClosed));
}

CPURegistersDlg::~CPURegistersDlg()
{
	//(*Destroy(CPURegistersDlg)
	//*)
}

bool FindString(wxString a, wxString b)
{
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
    const wxColour &changedColour = Manager::Get()->GetColourManager()->GetColour(wxT("dbg_watches_changed"));
    wxPGProperty* prop = m_cpu_register_node->GetPropertyByName(reg_name);
    if(prop == nullptr)
    {
        prop = m_cpu_register_node->AppendChild( new wxStringProperty(reg_name, reg_name));
    }

    if(prop->GetValueString() != hexValue)
        m_cpu_register_page->GetGrid()->SetPropertyTextColour(prop, changedColour);
    else
        m_cpu_register_page->GetGrid()->SetPropertyColourToDefault(prop);

    prop->SetValue(hexValue);
    m_cpu_register_page->SetPropertyHelpString(prop, reg_name + wxT(" = ") +  hexValue + wxT(" (") + interpreted + wxT(")"));

    m_cpu_register_page->SetPropertyAttribute(prop, wxT("Units"), interpreted);

}

void CPURegistersDlg::SetPeripheralValue(const wxString& reg_name, const wxString& hexValue)
{
    const wxColour& changedColour = Manager::Get()->GetColourManager()->GetColour(wxT("dbg_watches_changed"));
//    wxPGProperty* prop = m_per_register_node->GetPropertyByName(reg_name);
    wxPGProperty* prop = GetChildPropertyByName(reg_name);

    if (prop == nullptr)
    {
        prop = m_cpu_register_node->AppendChild(new wxStringProperty(reg_name, reg_name));  //unnecessary? SvdParser() did it
    }

    if (prop->GetValueString() != hexValue)
        m_cpu_register_page->GetGrid()->SetPropertyTextColour(prop, changedColour);
    else
        m_cpu_register_page->GetGrid()->SetPropertyColourToDefault(prop);

    prop->SetValue(hexValue);
    m_cpu_register_page->SetPropertyHelpString(prop, reg_name + wxT(" = ") +  hexValue);
}

wxArrayString CPURegistersDlg::GetBaseAddressList()
{
    wxArrayString addressList;

    size_t peripheral_count = m_per_register_node->GetChildCount();

    for (size_t i = 0; i < peripheral_count; i++)
    {
        wxPGProperty* perReg = m_per_register_node->Item(i);

        if (perReg->IsExpanded())
        {
            for (size_t j = 0; j < perReg->GetChildCount(); j++)
            {
                wxString perRegNameWithParent = perReg->Item(j)->GetName(); //GetName() adds parent name with dot before child name, new GetName()?
                addressList.Add(perRegNameWithParent.AfterFirst(wxT('.'))); //only child name
            }
        }
    }

    return addressList;
}

wxPGProperty* CPURegistersDlg::GetChildPropertyByName(const wxString& name) const
{
    size_t peripheral_count = m_per_register_node->GetChildCount();

    for (size_t i = 0; i < peripheral_count; i++)
    {
        wxPGProperty* perGroup = m_per_register_node->Item(i);

        for (size_t j = 0; j < perGroup->GetChildCount(); j++)
        {
            wxPGProperty* perReg = perGroup->Item(j);

            if (perReg->GetName().AfterFirst(wxT('.')) == name)
                return perReg;
        }
    }

    return NULL;
}

void CPURegistersDlg::EnableWindow(bool en)
{

}

void CPURegistersDlg::Clear()
{
//    m_cpu_register_page->Clear();
    m_per_register_node->DeleteChildren(); //refresh problem after this, needed to move window
}

void CPURegistersDlg::OnPropertyChanged(wxPropertyGridEvent &event)
{
    cbDebuggerPlugin *plugin = Manager::Get()->GetDebuggerManager()->GetActiveDebugger();

    if (plugin)
        plugin->SetRegisterValue(event.GetProperty()->GetLabel(), event.GetProperty()->GetValue());

    event.GetProperty()->ChangeFlag(wxPG_PROP_MODIFIED, false);
}

void CPURegistersDlg::OnPropertyChanging(wxPropertyGridEvent &event)
{
    if (event.GetProperty()->GetChildCount() > 0)
        event.Veto(true);
}

void CPURegistersDlg::OnProjectActivated(CodeBlocksEvent& event)
{
    cbProject* project = Manager::Get()->GetProjectManager()->GetActiveProject();

    if (project)
    {
        if (project->IsSVDEnable())
        {
            wxString svdPath = project->GetSVDPath();

            if (svdPath)
            {
                SvdParser* sParser = new SvdParser(m_per_register_node, svdPath);

                sParser->ParseSvd();
                m_cpu_register_page->CollapseAll();
            }
        }
    }
}

void CPURegistersDlg::OnProjectClosed(CodeBlocksEvent& event)
{
    Clear();
}

void CPURegistersDlg::OnExpand(wxPropertyGridEvent &event)
{
    cbDebuggerPlugin* plugin = Manager::Get()->GetDebuggerManager()->GetActiveDebugger();

    if (plugin)
        plugin->RequestUpdate(cbDebuggerPlugin::CPURegisters);
}
