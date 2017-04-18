#include "include/sdk.h"
#include "CPURegistersDlg.h"

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
    const wxColour &changedColour = Manager::Get()->GetColourManager()->GetColour(wxT("dbg_watches_changed"));
    wxPGProperty* prop = m_per_register_node->GetPropertyByName(reg_name);
    if(prop == nullptr)
    {
        prop = m_cpu_register_node->AppendChild( new wxStringProperty(reg_name, reg_name));
    }

    if(prop->GetValueString() != hexValue)
        m_cpu_register_page->GetGrid()->SetPropertyTextColour(prop, changedColour);
    else
        m_cpu_register_page->GetGrid()->SetPropertyColourToDefault(prop);

    prop->SetValue(hexValue);
    m_cpu_register_page->SetPropertyHelpString(prop, reg_name + wxT(" = ") +  hexValue);
}

void CPURegistersDlg::SvdParser()
{
    wxString m_SvdPath = wxEmptyString;

    cbProject* project = Manager::Get()->GetProjectManager()->GetActiveProject();

    if (project)
    {
        if (project->IsSVDEnable())
        {
            m_SvdPath = project->GetSVDPath();
        }

        if (m_SvdPath)
        {
            TiXmlDocument* doc = TinyXML::LoadDocument(m_SvdPath);

            if (doc)
            {
//            m_pSt->SetLabel(wxT("Filename: ") + m_SvdPath);

//            TiXmlHandle docHandle( doc );

//            TiXmlElement *deviceName =
//                    docHandle.FirstChild( "device" ).FirstChild( "name" ).ToElement();
//            if(deviceName)
//            {
//                m_pTree->AddRoot(_U(deviceName->GetText()));
//            }

                TiXmlElement* peripheral = GetFirstPeripheral(m_SvdPath);

                for (; peripheral; peripheral = peripheral->NextSiblingElement())
                {
                    TiXmlElement* tempPeripheral = peripheral;

                    TiXmlElement* name = peripheral->FirstChildElement("name");
                    wxString peripheralName = wxT("??");

                    if (name)
                        peripheralName = _U(name->GetText());
                    else
                        wxLogError(wxT("Peripheral has no name element"));


                    TiXmlElement* baseAddress = peripheral->FirstChildElement("baseAddress");
                    wxString peripheralbaseAddress = wxT("??");

                    if (baseAddress)
                        peripheralbaseAddress = _U(baseAddress->GetText());


                    const char* derivedFrom = peripheral->Attribute("derivedFrom");

                    if (derivedFrom)
                    {
                        peripheral = GetDerivedPeriphal(peripheral, derivedFrom);
                    }

                    TiXmlElement* description = peripheral->FirstChildElement("description");
                    wxString peripheralDescription = wxT("??");

                    if (description)
                        peripheralDescription = _U(description->GetText());

                    TiXmlElement* groupName = peripheral->FirstChildElement("groupName");
                    wxString peripheralgroupName = wxT("??");

                    if (groupName)
                        peripheralgroupName = _U(groupName->GetText());


                    //        m_svdModel->AddPeripheral( peripheralName, peripheralbaseAddress, peripheralgroupName, peripheralDescription );
//                wxTreeItemId peripheralNodeId = m_pTree->AppendItem(m_pTree->GetRootItem(),peripheralName);

//                size_t child_count = m_cpu_register_node->GetChildCount();

                    wxPGProperty* prop = m_per_register_node->AppendChild(new wxStringProperty(peripheralName, peripheralbaseAddress));


                    TiXmlHandle peripheralHandle(peripheral);
                    TiXmlElement* xregister = peripheralHandle.FirstChild("registers").FirstChild("register").ToElement();

                    for (; xregister; xregister = xregister->NextSiblingElement())
                    {
                        TiXmlElement* rName = xregister->FirstChildElement("name");
                        wxString registerName = wxT("??");

                        if (rName)
                            registerName = _U(rName->GetText());
                        else
                            wxLogError(wxT("Register has no name element"));

                        TiXmlElement* rDescription = xregister->FirstChildElement("description");
                        wxString registerDescription = wxT("??");

                        if (rDescription)
                            registerDescription = _U(rDescription->GetText());

                        TiXmlElement* rDispName = xregister->FirstChildElement("displayName");
                        wxString registerDisplayName = wxT("??");

                        if (rDispName)
                            registerDisplayName = _U(rDispName->GetText());

                        TiXmlElement* rAddressOffset = xregister->FirstChildElement("addressOffset");
                        wxString registerAddressOffset = wxT("??");

                        if (rAddressOffset)
                        {
                            registerAddressOffset = CalcOffset(peripheralbaseAddress, _U(rAddressOffset->GetText()));
                        }

                        TiXmlElement* rSize = xregister->FirstChildElement("size");
                        wxString registerSize = wxT("??");

                        if (rSize)
                        {
                            registerSize = _U(rSize->GetText());
                        }

                        TiXmlElement* access = xregister->FirstChildElement("access");
                        wxString registerAccess = wxT("??");

                        if (access)
                            registerAccess = _U(access->GetText());

                        TiXmlElement* resetValue = xregister->FirstChildElement("resetValue");
                        wxString registerResetValue = wxT("??");

                        if (resetValue)
                            registerResetValue = _U(resetValue->GetText());


//                    m_pTree->AppendItem(peripheralNodeId,registerName);
//                    prop = prop->AppendChild(new wxStringProperty(registerName, registerName));
                        prop->AppendChild(new wxStringProperty(registerName, registerAddressOffset));
                    }

                    peripheral = tempPeripheral; // In case of derivedFrom, continue from the untouched peripheral
//                m_pTree->Expand(m_pTree->GetRootItem());
                }
                m_cpu_register_page->CollapseAll();
            }

            else
            {
//            Clear();
//            m_pSt->SetLabel(wxT("File error."));
            }

        }
    }
    else
    {
//        Clear();
//        m_pSt->SetLabel(wxT("Add the .svd file to the project."));
    }
}

wxString CPURegistersDlg::CalcOffset( const wxString &baseAddress, const wxString &offset )
{
    wxString result = wxEmptyString;
    unsigned long numBase = 0;
    unsigned long numOffset = 0;

    if( baseAddress.ToULong( &numBase, 16 ) && offset.ToULong( &numOffset, 16 ) )
    {
        numBase += numOffset;
        result = ( wxT("0x") + wxString::Format( wxT( "%x" ), numBase ) ); //wxT( "%X" )
    }

    return result;
}

TiXmlElement *CPURegistersDlg::GetDerivedPeriphal( TiXmlElement *peripheral, const char *derivedName )
{
    for( peripheral = peripheral->Parent()->FirstChildElement( "peripheral" ); peripheral;
            peripheral = peripheral->NextSiblingElement() )
    {
        TiXmlElement *name = peripheral->FirstChildElement( "name" );

        if( _U( name->GetText() ) == _U( derivedName ) ) // const char* works under button but not here?
            return peripheral;
    }

    wxLogError( wxT("No ") + _U( derivedName ) + wxT(" found in peripherals") );
    return 0;
}

TiXmlElement* CPURegistersDlg::GetFirstPeripheral(const wxString& m_SvdPath)
{
    TiXmlDocument *doc = TinyXML::LoadDocument( m_SvdPath );

    if(doc)
    {
        TiXmlHandle docHandle( doc );

        TiXmlElement *peripheral =
                docHandle.FirstChild( "device" ).FirstChild( "peripherals" ).FirstChild( "peripheral" ).ToElement();

        return peripheral;
    }

    return 0;
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
    SvdParser();
}

void CPURegistersDlg::OnProjectClosed(CodeBlocksEvent& event)
{
    Clear();
}
