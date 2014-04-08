#ifndef SQ_WXBASECONTROLS
#define SQ_WXBASECONTROLS

#include <sq_wx/sq_wx_type_handler.h>
#include <sc_cb_vm.h>
#include <sc_base_types.h>
#include <wx/string.h>
#include <wx/event.h>
#include <wx/xrc/xmlres.h>
#include <wx/button.h>
#include <wx/textctrl.h>
#include <wx/animate.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/collpane.h>
#include <wx/combobox.h>
#include <map>

namespace ScriptBindings
{
#define CHECK_CONTROL_BEGIN(ctrl,name,ptr)   if(name == wxT(#ctrl))  \
    {                                                               \
        ctrl* ctr = dynamic_cast<ctrl*>(ptr);                        \
        sa.PushInstance<ctrl>(ctr);                                 \
        return SC_RETURN_VALUE;                                     \
    }

#define CHECK_CONTROL(ctrl,name,ptr)    else if(name == wxT(#ctrl))  \
    {                                                               \
        ctrl* ctr = dynamic_cast<ctrl*>(ptr);                        \
        sa.PushInstance<ctrl>(ctr);                                 \
        return SC_RETURN_VALUE;                                     \
    }

#define CHECK_CONTROL_END(name)    else                         \
    {                                                               \
        sa.ThrowError(_("i don't handle ") + name);                 \
        return SC_RETURN_OK;                                        \
    }

template<class B>
class cb_wxBaseManagedWindow : public wxEvtHandler
{
public:
    cb_wxBaseManagedWindow(HSQUIRRELVM vm)    : m_vm(vm), m_managed_obj(nullptr)
    {
        Manager::Get()->RegisterEventSink(cbEVT_APP_START_SHUTDOWN, new cbEventFunctor<cb_wxBaseManagedWindow, CodeBlocksEvent>(this, &cb_wxBaseManagedWindow::OnClose));
    };

    ~cb_wxBaseManagedWindow()       {};

    B* GetManagedWindow()
    {
        return m_managed_obj;
    };
    void SetManagedWindow(B* wind)
    {
        m_managed_obj = wind;
    };

    void OnClose(CodeBlocksEvent& event)
    {
        DeConnectEvtHandler();
        Destroy();
    }

    void ConnectEvtHandler()
    {
        GetManagedWindow()->SetNextHandler(this);
        SetPreviousHandler(GetManagedWindow());
    }

    void Destroy()
    {
        if(GetManagedWindow() != nullptr)
        {
            GetManagedWindow()->Destroy();
            SetManagedWindow(nullptr);
        }

    }

    void DeConnectEvtHandler()
    {
        GetManagedWindow()->SetNextHandler(nullptr);
        SetPreviousHandler(nullptr);
    }

    void RegisterEventHandler(wxEventType type,int id,Sqrat::Object obj,wxString handler)
    {
        if(GetManagedWindow() == nullptr)
            return;

        GetManagedWindow()->Connect(id,type,(wxObjectEventFunction)&cb_wxBaseManagedWindow::OnEvt,NULL,this);

        evt_func func;// TODO (bluehazzard#1#): parameter in constructor
        func.env = obj;
        func.func_name = handler;

        typename evt_type_id_map::iterator type_itr = m_evt_map.find(type);
        if(type_itr == m_evt_map.end())
        {
            //event type not found, create
            evt_id_func_map id_func;
            id_func.insert(id_func.end(),std::make_pair(id,func));
            m_evt_map.insert(type_itr, std::make_pair(type,id_func));
            return;
        }
        typename evt_id_func_map::iterator id_itr = type_itr->second.find(id);
        if(id_itr == type_itr->second.end())
        {
            type_itr->second.insert(id_itr, std::make_pair(id,func));
            return;
        }

        id_itr->second = func;
        return;
    }

protected:

    void OnEvt(wxEvent& event)
    {
        typename evt_type_id_map::iterator type_itr = m_evt_map.find(event.GetEventType());
        if(type_itr != m_evt_map.end())
        {
            typename evt_id_func_map::iterator id_itr = type_itr->second.find(event.GetId());
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
    }

    struct evt_func
    {
        wxString func_name;
        Sqrat::Object env;
    };

    typedef std::map<int,evt_func>                  evt_id_func_map;
    typedef std::map<wxEventType,evt_id_func_map>   evt_type_id_map;
    evt_type_id_map m_evt_map;


    HSQUIRRELVM m_vm;
    Sqrat::Object m_object;
    B* m_managed_obj;
};




template <typename A> SQInteger GetControlTemplate(HSQUIRRELVM vm)
{
    StackHandler sa(vm);

    A* base = sa.GetInstance<A>(1);

    wxString name = sa.GetValue<wxString>(2);
    int id = wxXmlResource::Get()->GetXRCID(name);

    wxWindow* wind = base->GetManagedWindow()->FindWindow(id);
    if(wind == nullptr)
        return sa.ThrowError(_("Could not find the window ") + name);

    wxClassInfo* c_info = wind->GetClassInfo();
    wxString c_name = c_info->GetClassName();

    CHECK_CONTROL_BEGIN(wxTextCtrl,c_name,wind)
    CHECK_CONTROL(wxButton,c_name,wind)
    //CHECK_CONTROL(wxAnyButton,c_name,wind)
    CHECK_CONTROL(wxAnimationCtrl,c_name,wind)
#if wxCHECK_VERSION(2, 9, 0)
    CHECK_CONTROL(wxCommandLinkButton,c_name,wind)
#endif
    CHECK_CONTROL(wxCheckBox,c_name,wind)
    CHECK_CONTROL(wxChoice,c_name,wind)
    CHECK_CONTROL(wxCollapsiblePane,c_name,wind)
    CHECK_CONTROL(wxComboBox,c_name,wind)

    CHECK_CONTROL_END(c_name)

    return SC_RETURN_OK;
}

void bind_wxBaseControls(HSQUIRRELVM vm);
}


#endif // SQ_WXBASECONTROLS

