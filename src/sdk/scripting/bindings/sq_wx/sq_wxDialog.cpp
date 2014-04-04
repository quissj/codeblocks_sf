
#include "scripting/bindings/sq_wx/sq_wxDialog.h"
#include <wx/button.h>

namespace ScriptBindings
{

    class sq_wxDialog;


SQInteger sq_wxDialog_constructor(HSQUIRRELVM vm)
{
    //StackHandler sa(vm);


    sq_wxDialog* dlg = new sq_wxDialog(vm);

    //dlg->m_object = sa.GetValue<Sqrat::Object>(1);  // Get the Plugin instance (or where the functions can be found)

    sq_setinstanceup(vm, 1, dlg);
    sq_setreleasehook(vm, 1, &Sqrat::DefaultAllocator<sq_wxDialog>::Delete);
    return SC_RETURN_OK;

}

sq_wxDialog::sq_wxDialog(HSQUIRRELVM vm) : m_dialog(nullptr) , m_vm(vm)
{

}

sq_wxDialog::~sq_wxDialog()
{
    if(m_dialog != nullptr)
    {
        m_dialog->Destroy();
        //delete m_dialog;
        //m_dialog = nullptr;
    }
}

int sq_wxDialog::LoadFromXRCFile(wxString file,wxString name)
{
    if(!wxXmlResource::Get()->Load(file))
        return XRC_FILE_NOT_FOUND;

    return LoadFromXRCPool(name);
}

int sq_wxDialog::LoadFromXRCPool(wxString name)
{
    if(m_dialog != nullptr)
        return RESOURCE_ALREADY_LOADED;
    m_dialog = wxXmlResource::Get()->LoadDialog(NULL,name);

    if(m_dialog == nullptr)
        return RESOURCE_NOT_FOUND_IN_LOADED_RESOURCES;

    //wxEvtHandler *prev = m_dialog->GetPreviousHandler();
    //SetPreviousHandler(prev);
    //prev->SetNextHandler(this);
    //SetNextHandler(m_dialog);
    //m_dialog->SetPreviousHandler(this);
    m_dialog->SetNextHandler(this);
    //m_dialog->PushEventHandler(this);
    SetPreviousHandler(m_dialog);

    return RESOURCE_LOADED_SUCCESFULLY;
}


void sq_wxDialog::RegisterEventHandler(wxEventType type,int id,Sqrat::Object obj,wxString handler)
{
    if(m_dialog == nullptr)
        return;

    m_dialog->Connect(id,type,(wxObjectEventFunction)&sq_wxDialog::OnEvt,NULL,this);

    evt_func func;// TODO (bluehazzard#1#): parameter in constructor
    func.env = obj;//Sqrat::Object(obj,m_vm);
    func.func_name = handler;

    evt_type_id_map::iterator type_itr = m_evt_map.find(type);
    if(type_itr == m_evt_map.end())
    {
        //event type not found, create
        evt_id_func_map id_func;
        id_func.insert(id_func.end(),std::make_pair(id,func));
        m_evt_map.insert(type_itr, std::make_pair(type,id_func));
        return;
    }
    evt_id_func_map::iterator id_itr = type_itr->second.find(id);
    if(id_itr == type_itr->second.end())
    {
        type_itr->second.insert(id_itr, std::make_pair(id,func));
        return;
    }

    id_itr->second = func;
    return;
}


bool sq_wxDialog::TryBefore(wxEvent& event)
{
    evt_type_id_map::iterator type_itr = m_evt_map.find(event.GetEventType());
    if(type_itr != m_evt_map.end())
    {
        evt_id_func_map::iterator id_itr = type_itr->second.find(event.GetId());
        if(id_itr != type_itr->second.end())
        {
            Sqrat::Function func(id_itr->second.env,id_itr->second.func_name.mb_str());
            if (!func.IsNull())
            {
                func(&event);
                Manager::Get()->GetScriptingManager()->DisplayErrorsAndText(_("In sq_wxDialog::TryBefore: "));
            }
            else
            {
                // Could not find the registered event callback
                Manager::Get()->GetLogManager()->LogWarning(_("Scripting error: Could not find event callback \"") + id_itr->second.func_name + _("\" in Dialog") );
            }
        }
    }

    //return wxDialog::TryBefore(event);    // The base function should be called, but from m_dialog or?
    return false;
}

void sq_wxDialog::OnEvt(wxEvent& event)
{
    evt_type_id_map::iterator type_itr = m_evt_map.find(event.GetEventType());
    if(type_itr != m_evt_map.end())
    {
        evt_id_func_map::iterator id_itr = type_itr->second.find(event.GetId());
        if(id_itr != type_itr->second.end())
        {
            Sqrat::Function func(id_itr->second.env,id_itr->second.func_name.mb_str());
            if (!func.IsNull())
            {
                func(&event);
                Manager::Get()->GetScriptingManager()->DisplayErrorsAndText(_("In sq_wxDialog::OnEvt: "));
            }
            else
            {
                // Could not find the registered event callback
                Manager::Get()->GetLogManager()->LogWarning(_("Scripting error: Could not find event callback \"") + id_itr->second.func_name + _("\" in Dialog") );
            }
        }
    }
    event.Skip();

    //return wxDialog::TryBefore(event);    // The base function should be called, but from m_dialog or?
    //return false;
}

bool sq_wxDialog::IsModal()
{
    if(m_dialog == nullptr)
        return false;

    return m_dialog->IsModal();
}

void sq_wxDialog::Centre()
{
    if(m_dialog == nullptr)
        return;

    m_dialog->Centre();
}

int sq_wxDialog::ShowModal()
{
    if(m_dialog == nullptr)
        return -1;

    return m_dialog->ShowModal();
}

void sq_wxDialog::EndModal(int retcode)
{
    if(m_dialog == nullptr)
        return;

    m_dialog->EndModal(retcode);
}

int sq_wxDialog::Show(bool show)
{
    if(m_dialog == nullptr)
        return -1;
    m_dialog->Show(show);
    return 0;
}

void sq_wxDialog::SetTitle(wxString& title)
{
    if(m_dialog == nullptr)
        return;
    m_dialog->SetTitle(title);
}

void sq_wxDialog::Maximize(bool maximize)
{
    if(m_dialog == nullptr)
        return;
    m_dialog->Maximize(maximize);
}

int GetIDfromXRC(wxString name)
{
    return  wxXmlResource::Get()->GetXRCID(name);
}

wxString GetNameFromIDFromXRC(int id)
{
    #if wxCHECK_VERSION(2, 9, 0)
        return wxXmlResource::Get()->FindXRCIDById(id);
    #else
        return wxString(_("only in wxWidgets > 2.9.0"));
    #endif
}

void bind_wxDialog(HSQUIRRELVM vm)
{
    Sqrat::Class<sq_wxDialog,Sqrat::NoCopy<sq_wxDialog> > bwxDialog(vm,"wxDialog");
    bwxDialog.SquirrelFunc("constructor",&sq_wxDialog_constructor)                  // One parameter to get the function context in which the callbacks are searched
    .Func("LoadFromXRCFile", &sq_wxDialog::LoadFromXRCFile)
    .Func("LoadFromXRCPool", &sq_wxDialog::LoadFromXRCPool)
    .Func("RegisterEventHandler", &sq_wxDialog::RegisterEventHandler)
    .Func("IsModal", &sq_wxDialog::IsModal)
    .Func("Centre", &sq_wxDialog::Centre)
    .Func("ShowModal", &sq_wxDialog::ShowModal)
    .Func("Show", &sq_wxDialog::Show)
    .Func("EndModal", &sq_wxDialog::EndModal)
    .Func("SetTitle", &sq_wxDialog::SetTitle)
    .Func("Maximize", &sq_wxDialog::Maximize);

    Sqrat::RootTable(vm).Bind(_SC("wxDialog"),bwxDialog);
    Sqrat::RootTable(vm).Func(_SC("XRCID"),&GetIDfromXRC);
    Sqrat::RootTable(vm).Func(_SC("XRCNAME"),&GetNameFromIDFromXRC);

    Sqrat::Class<wxEvent,Sqrat::NoConstructor<wxEvent> > bwxEvent(vm,"wxEvent");
    bwxEvent.Func("GetEventType",&wxEvent::GetEventType)
    .Func("GetId",&wxEvent::GetId)
    .Func("GetTimestamp",&wxEvent::GetTimestamp)
    .Func("IsCommandEvent",&wxEvent::IsCommandEvent);
    Sqrat::RootTable(vm).Bind(_SC("wxEvent"),bwxEvent);


    BIND_INT_CONSTANT(RESOURCE_LOADED_SUCCESFULLY);
    BIND_INT_CONSTANT(XRC_FILE_NOT_FOUND);
    BIND_INT_CONSTANT(RESOURCE_ALREADY_LOADED);
    BIND_INT_CONSTANT(RESOURCE_NOT_FOUND_IN_LOADED_RESOURCES);

    BIND_INT_CONSTANT(wxEVT_ACTIVATE);
    BIND_INT_CONSTANT(wxEVT_ACTIVATE_APP);
    BIND_INT_CONSTANT(wxEVT_COMMAND_BUTTON_CLICKED);
    BIND_INT_CONSTANT(wxEVT_COMMAND_CHECKBOX_CLICKED);
    BIND_INT_CONSTANT(wxEVT_COMMAND_CHOICE_SELECTED);
    BIND_INT_CONSTANT(wxEVT_COMMAND_LISTBOX_SELECTED);
    BIND_INT_CONSTANT(wxEVT_COMMAND_LISTBOX_DOUBLECLICKED);
    BIND_INT_CONSTANT(wxEVT_COMMAND_CHECKLISTBOX_TOGGLED);
    // now they are in wx/textctrl.h
#if WXWIN_COMPATIBILITY_EVENT_TYPES
    BIND_INT_CONSTANT(wxEVT_COMMAND_TEXT_UPDATED);
    BIND_INT_CONSTANT(wxEVT_COMMAND_TEXT_ENTER);
    BIND_INT_CONSTANT(wxEVT_COMMAND_TEXT_URL);
    BIND_INT_CONSTANT(wxEVT_COMMAND_TEXT_MAXLEN);
#endif // WXWIN_COMPATIBILITY_EVENT_TYPES
    BIND_INT_CONSTANT(wxEVT_COMMAND_MENU_SELECTED);
    BIND_INT_CONSTANT(wxEVT_COMMAND_SLIDER_UPDATED);
    BIND_INT_CONSTANT(wxEVT_COMMAND_RADIOBOX_SELECTED);
    BIND_INT_CONSTANT(wxEVT_COMMAND_RADIOBUTTON_SELECTED);

    // wxEVT_COMMAND_SCROLLBAR_UPDATED is now obsolete since we use
    // wxEVT_SCROLL... events

    BIND_INT_CONSTANT(wxEVT_COMMAND_SCROLLBAR_UPDATED);
    BIND_INT_CONSTANT(wxEVT_COMMAND_VLBOX_SELECTED);
    BIND_INT_CONSTANT(wxEVT_COMMAND_COMBOBOX_SELECTED);
    BIND_INT_CONSTANT(wxEVT_COMMAND_TOOL_RCLICKED);
    BIND_INT_CONSTANT(wxEVT_COMMAND_TOOL_ENTER);
    BIND_INT_CONSTANT(wxEVT_COMMAND_SPINCTRL_UPDATED);

        // Sockets and timers send events, too
    BIND_INT_CONSTANT(wxEVT_TIMER);

        // Mouse event types
    BIND_INT_CONSTANT(wxEVT_LEFT_DOWN);
    BIND_INT_CONSTANT(wxEVT_LEFT_UP);
    BIND_INT_CONSTANT(wxEVT_MIDDLE_DOWN);
    BIND_INT_CONSTANT(wxEVT_MIDDLE_UP);
    BIND_INT_CONSTANT(wxEVT_RIGHT_DOWN);
    BIND_INT_CONSTANT(wxEVT_RIGHT_UP);
    BIND_INT_CONSTANT(wxEVT_MOTION);
    BIND_INT_CONSTANT(wxEVT_ENTER_WINDOW);
    BIND_INT_CONSTANT(wxEVT_LEAVE_WINDOW);
    BIND_INT_CONSTANT(wxEVT_LEFT_DCLICK);
    BIND_INT_CONSTANT(wxEVT_MIDDLE_DCLICK);
    BIND_INT_CONSTANT(wxEVT_RIGHT_DCLICK);
    BIND_INT_CONSTANT(wxEVT_SET_FOCUS);
    BIND_INT_CONSTANT(wxEVT_KILL_FOCUS);
    BIND_INT_CONSTANT(wxEVT_CHILD_FOCUS);
    BIND_INT_CONSTANT(wxEVT_MOUSEWHEEL);

        // Non-client mouse events
    BIND_INT_CONSTANT(wxEVT_NC_LEFT_DOWN);
    BIND_INT_CONSTANT(wxEVT_NC_LEFT_UP);
    BIND_INT_CONSTANT(wxEVT_NC_MIDDLE_DOWN);
    BIND_INT_CONSTANT(wxEVT_NC_MIDDLE_UP);
    BIND_INT_CONSTANT(wxEVT_NC_RIGHT_DOWN);
    BIND_INT_CONSTANT(wxEVT_NC_RIGHT_UP);
    BIND_INT_CONSTANT(wxEVT_NC_MOTION);
    BIND_INT_CONSTANT(wxEVT_NC_ENTER_WINDOW);
    BIND_INT_CONSTANT(wxEVT_NC_LEAVE_WINDOW);
    BIND_INT_CONSTANT(wxEVT_NC_LEFT_DCLICK);
    BIND_INT_CONSTANT(wxEVT_NC_MIDDLE_DCLICK);
    BIND_INT_CONSTANT(wxEVT_NC_RIGHT_DCLICK);

        // Character input event type
    BIND_INT_CONSTANT(wxEVT_CHAR);
    BIND_INT_CONSTANT(wxEVT_CHAR_HOOK);
    BIND_INT_CONSTANT(wxEVT_NAVIGATION_KEY);
    BIND_INT_CONSTANT(wxEVT_KEY_DOWN);
    BIND_INT_CONSTANT(wxEVT_KEY_UP);
#if wxUSE_HOTKEY
    BIND_INT_CONSTANT(wxEVT_HOTKEY);
#endif
        // Set cursor event
    BIND_INT_CONSTANT(wxEVT_SET_CURSOR);

        // wxScrollBar and wxSlider event identifiers
    BIND_INT_CONSTANT(wxEVT_SCROLL_TOP);
    BIND_INT_CONSTANT(wxEVT_SCROLL_BOTTOM);
    BIND_INT_CONSTANT(wxEVT_SCROLL_LINEUP);
    BIND_INT_CONSTANT(wxEVT_SCROLL_LINEDOWN);
    BIND_INT_CONSTANT(wxEVT_SCROLL_PAGEUP);
    BIND_INT_CONSTANT(wxEVT_SCROLL_PAGEDOWN);
    BIND_INT_CONSTANT(wxEVT_SCROLL_THUMBTRACK);
    BIND_INT_CONSTANT(wxEVT_SCROLL_THUMBRELEASE);
    BIND_INT_CONSTANT(wxEVT_SCROLL_CHANGED);

    BIND_INT_CONSTANT_NAMED(wxEVT_COMMAND_BUTTON_CLICKED,"wxEVT_BUTTON");

        // Scroll events from wxWindow
    BIND_INT_CONSTANT(wxEVT_SCROLLWIN_TOP);
    BIND_INT_CONSTANT(wxEVT_SCROLLWIN_BOTTOM);
    BIND_INT_CONSTANT(wxEVT_SCROLLWIN_LINEUP);
    BIND_INT_CONSTANT(wxEVT_SCROLLWIN_LINEDOWN);
    BIND_INT_CONSTANT(wxEVT_SCROLLWIN_PAGEUP);
    BIND_INT_CONSTANT(wxEVT_SCROLLWIN_PAGEDOWN);
    BIND_INT_CONSTANT(wxEVT_SCROLLWIN_THUMBTRACK);
    BIND_INT_CONSTANT(wxEVT_SCROLLWIN_THUMBRELEASE);

        // System events
    BIND_INT_CONSTANT(wxEVT_SIZE);
    BIND_INT_CONSTANT(wxEVT_MOVE);
    BIND_INT_CONSTANT(wxEVT_CLOSE_WINDOW);
    BIND_INT_CONSTANT(wxEVT_END_SESSION);
    BIND_INT_CONSTANT(wxEVT_QUERY_END_SESSION);
    BIND_INT_CONSTANT(wxEVT_ACTIVATE_APP);
    // 406..408 are power events
    BIND_INT_CONSTANT(wxEVT_ACTIVATE);
    BIND_INT_CONSTANT(wxEVT_CREATE);
    BIND_INT_CONSTANT(wxEVT_DESTROY);
    BIND_INT_CONSTANT(wxEVT_SHOW);
    BIND_INT_CONSTANT(wxEVT_ICONIZE);
    BIND_INT_CONSTANT(wxEVT_MAXIMIZE);
    BIND_INT_CONSTANT(wxEVT_MOUSE_CAPTURE_CHANGED);
    BIND_INT_CONSTANT(wxEVT_MOUSE_CAPTURE_LOST);
    BIND_INT_CONSTANT(wxEVT_PAINT);
    BIND_INT_CONSTANT(wxEVT_ERASE_BACKGROUND);
    BIND_INT_CONSTANT(wxEVT_NC_PAINT);
    BIND_INT_CONSTANT(wxEVT_PAINT_ICON);
    BIND_INT_CONSTANT(wxEVT_MENU_OPEN);
    BIND_INT_CONSTANT(wxEVT_MENU_CLOSE);
    BIND_INT_CONSTANT(wxEVT_MENU_HIGHLIGHT);
    BIND_INT_CONSTANT(wxEVT_CONTEXT_MENU);
    BIND_INT_CONSTANT(wxEVT_SYS_COLOUR_CHANGED);
    BIND_INT_CONSTANT(wxEVT_DISPLAY_CHANGED);
    BIND_INT_CONSTANT(wxEVT_SETTING_CHANGED);
    BIND_INT_CONSTANT(wxEVT_QUERY_NEW_PALETTE);
    BIND_INT_CONSTANT(wxEVT_PALETTE_CHANGED);
    BIND_INT_CONSTANT(wxEVT_JOY_BUTTON_DOWN);
    BIND_INT_CONSTANT(wxEVT_JOY_BUTTON_UP);
    BIND_INT_CONSTANT(wxEVT_JOY_MOVE);
    BIND_INT_CONSTANT(wxEVT_JOY_ZMOVE);
    BIND_INT_CONSTANT(wxEVT_DROP_FILES);
    BIND_INT_CONSTANT(wxEVT_DRAW_ITEM);
    BIND_INT_CONSTANT(wxEVT_MEASURE_ITEM);
    BIND_INT_CONSTANT(wxEVT_COMPARE_ITEM);
    BIND_INT_CONSTANT(wxEVT_INIT_DIALOG);
    BIND_INT_CONSTANT(wxEVT_UPDATE_UI);
    BIND_INT_CONSTANT(wxEVT_SIZING);
    BIND_INT_CONSTANT(wxEVT_MOVING);
    BIND_INT_CONSTANT(wxEVT_HIBERNATE);
    // more power events follow -- see wx/power.h

        // Clipboard events
    BIND_INT_CONSTANT(wxEVT_COMMAND_TEXT_COPY);
    BIND_INT_CONSTANT(wxEVT_COMMAND_TEXT_CUT);
    BIND_INT_CONSTANT(wxEVT_COMMAND_TEXT_PASTE);

        // Generic command events
        // Note: a click is a higher-level event than button down/up
    BIND_INT_CONSTANT(wxEVT_COMMAND_LEFT_CLICK);
    BIND_INT_CONSTANT(wxEVT_COMMAND_LEFT_DCLICK);
    BIND_INT_CONSTANT(wxEVT_COMMAND_RIGHT_CLICK);
    BIND_INT_CONSTANT(wxEVT_COMMAND_RIGHT_DCLICK);
    BIND_INT_CONSTANT(wxEVT_COMMAND_SET_FOCUS);
    BIND_INT_CONSTANT(wxEVT_COMMAND_KILL_FOCUS);
    BIND_INT_CONSTANT(wxEVT_COMMAND_ENTER);

        // Help events
    BIND_INT_CONSTANT(wxEVT_HELP);
    BIND_INT_CONSTANT(wxEVT_DETAILED_HELP);
}

}
