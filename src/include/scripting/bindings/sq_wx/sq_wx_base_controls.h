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
#include <wx/radiobox.h>
#include <wx/gauge.h>
#include <wx/hyperlink.h>
#include <wx/radiobut.h>
#include <wx/listbox.h>
#include <wx/checklst.h>
#include <wx/slider.h>
#include <wx/stattext.h>
#include <wx/tglbtn.h>
#include <wx/srchctrl.h>
#include <wx/pickerbase.h>
#include <wx/clrpicker.h>
#include <wx/filepicker.h>
#include <wx/spinbutt.h>
#include <wx/spinctrl.h>
#include <wx/timer.h>
#include <map>

#if wxCHECK_VERSION(2, 9, 0)
#include <wx/commandlinkbutton.h>
#include <wx/anybutton.h>
#endif // wxCHECK_VERSION

namespace ScriptBindings
{
#define CHECK_CONTROL_BEGIN(ctrl,name,ptr)   if(name == CLASSINFO(ctrl)->GetClassName() ) \
    {                                                               \
        ctrl* ctr = dynamic_cast<ctrl*>(ptr);                       \
        sa.PushInstance<ctrl>(ctr);                                 \
        return SC_RETURN_VALUE;                                     \
    }

#define CHECK_CONTROL(ctrl,name,ptr)    else if(name == CLASSINFO(ctrl)->GetClassName())  \
    {                                                                \
        ctrl* ctr = dynamic_cast<ctrl*>(ptr);                        \
        sa.PushInstance<ctrl>(ctr);                                  \
        return SC_RETURN_VALUE;                                      \
    }

#define CHECK_CONTROL_END(name)    else                             \
    {                                                               \
        return sa.ThrowError(_("i don't handle ") + name);          \
    }


/** \brief Base class to bind wxWidgets windows like dialogs and frames to squirrel
 */

template<class B> class cb_wxBaseManagedWindow : public wxEvtHandler
{
public:

    /** \brief Base constructor for all managed wxWidgets windows
     *
     * The constructor registers a cbEVT_APP_START_SHUTDOWN event for handling the closing process and destroying the managed window
     * \param vm HSQUIRRELVM    The vm where this dialog should be created
     *
     */
    cb_wxBaseManagedWindow(HSQUIRRELVM vm)    : m_vm(vm), m_managed_obj(nullptr)
    {
        Manager::Get()->RegisterEventSink(cbEVT_APP_START_SHUTDOWN, new cbEventFunctor<cb_wxBaseManagedWindow, CodeBlocksEvent>(this, &cb_wxBaseManagedWindow::OnClose));
    };

    /** \brief Base destructor for all managed wxWidgets windows
     *
     */
    ~cb_wxBaseManagedWindow()
    {
        Destroy();
    };

    /** \brief Get a pointer to the managed window
     *
     * \return B* Pointer to the managed window
     *
     */
    B* GetManagedWindow()
    {
        return m_managed_obj;
    };


    /** \brief Set a pointer to the managed window
     *
     * If the class gets destructed and this pointer is not a _nullptr_ the destructor will call a _Destroy()_ on
     * the pointer and set it to _nullptr_. But the managed window will not be deleted (it hes to be deleted inside _Destroy()_
     * \param wind B* Pointer to the managed window.
     *
     */
    void SetManagedWindow(B* wind)
    {
        m_managed_obj = wind;
    };

    /** \brief Internal function for handling OnClose events
     *
     * If the Main application gets closed the event handler of the window has to be removed and the managed window
     * has to be destroyed. To get the OnClose event we register us in the C::B internal event system for a
     * cbEVT_APP_START_SHUTDOWN event.
     * \param event CodeBlocksEvent&
     * \return void
     *
     */
    void OnClose(CodeBlocksEvent& event)
    {
        //DeConnectEvtHandler();
        Destroy();
    }

    /** \brief Connect this class as a EventHandler to the managed window
     *
     * \return void
     *
     */
    void ConnectEvtHandler()
    {
        #if wxCHECK_VERSION(2, 9, 0)
        GetManagedWindow()->PushEventHandler(this);
        #else
        GetManagedWindow()->SetNextHandler(this);
        SetPreviousHandler(GetManagedWindow());
        #endif // wxCHECK_VERSION

    }

    /** \brief Destroy the managed window and set it to _nullptr_
     *
     * \return void
     *
     */
    void Destroy()
    {
        if(GetManagedWindow() != nullptr)
        {
            DeConnectEvtHandler();
            GetManagedWindow()->Destroy();
            SetManagedWindow(nullptr);
        }

    }

    /** \brief Remove this class as EventHandler from the managed window
     *
     * \return void
     *
     */
    void DeConnectEvtHandler()
    {
        id_timer_map::iterator itr = m_timer_map.begin();
        for(;itr != m_timer_map.end();++itr)
        {
            if(itr->second != nullptr)
                delete itr->second;
        }
        m_timer_map.clear();

        #if wxCHECK_VERSION(2, 9, 0)
        GetManagedWindow()->PopEventHandler();
        #else
        //GetManagedWindow()->SetNextHandler(nullptr);
        //SetPreviousHandler(nullptr);
        #endif // wxCHECK_VERSION


    }

    wxTimer* CreateTimer()
    {
        wxTimer* timer = new wxTimer(m_managed_obj);
        m_timer_map.insert(std::make_pair(timer->GetId(),timer));
        return timer;
    }

    wxTimer* GetTimer(int id)
    {
        id_timer_map::iterator itr = m_timer_map.find(id);
        if(itr == m_timer_map.end())
            return nullptr;

        return itr->second;
    }

    /** \brief Helper function to register a squirrel function as event handler for a wxWidgets event
     *
     * This function
     * \param type wxEventType  The wxWidgets event type (see \ref sq_wx_constants)
     * \param id int            The window id of the control
     * \param obj Sqrat::Object The squirrel base object (table or class) where the function can be found
     * \param handler wxString  The name of the event handler function (has to be a function with one parameter)
     * \return void
     *
     */
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

    /** \brief Internal event function. Gets called from every wxWidgets event
     *
     * This functions searches for the corresponding handler function in the squirrel engine and calls it
     * \param event wxEvent&
     * \return void
     *
     */
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
        //event.Skip();
    }

    struct evt_func
    {
        wxString func_name;
        Sqrat::Object env;
    };

    typedef std::map<int,evt_func>                  evt_id_func_map;
    typedef std::map<wxEventType,evt_id_func_map>   evt_type_id_map;
    evt_type_id_map m_evt_map;

    typedef std::map<int,wxTimer*>      id_timer_map;
    id_timer_map  m_timer_map;


    HSQUIRRELVM m_vm;
    Sqrat::Object m_object;
    B* m_managed_obj;
};




/** \brief Return the right control element to squirrel
 *  \ingroup sq_helper_functions
 *
 * \param vm HSQUIRRELVM
 * \return template <typename A> SQInteger
 *
 */
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
    CHECK_CONTROL(wxWindow,c_name,wind)
    CHECK_CONTROL(wxButton,c_name,wind)
    CHECK_CONTROL(wxAnimationCtrl,c_name,wind)
#if wxCHECK_VERSION(2, 9, 0)
    CHECK_CONTROL(wxCommandLinkButton,c_name,wind)
    CHECK_CONTROL(wxAnyButton,c_name,wind)
#endif
    CHECK_CONTROL(wxCheckBox,c_name,wind)
    CHECK_CONTROL(wxChoice,c_name,wind)
    CHECK_CONTROL(wxCollapsiblePane,c_name,wind)
    CHECK_CONTROL(wxComboBox,c_name,wind)
    CHECK_CONTROL(wxRadioBox,c_name,wind)
    CHECK_CONTROL(wxGauge,c_name,wind)
    CHECK_CONTROL(wxHyperlinkCtrl,c_name,wind)
    CHECK_CONTROL(wxRadioButton,c_name,wind)
    CHECK_CONTROL(wxListBox,c_name,wind)
    CHECK_CONTROL(wxCheckListBox,c_name,wind)
    CHECK_CONTROL(wxStaticText,c_name,wind)
    CHECK_CONTROL(wxSlider,c_name,wind)
    CHECK_CONTROL(wxToggleButton,c_name,wind)
    //CHECK_CONTROL(wxSearchCtrl,c_name,wind)
    CHECK_CONTROL(wxPickerBase,c_name,wind)
    CHECK_CONTROL(wxColourPickerCtrl,c_name,wind)
    CHECK_CONTROL(wxDirPickerCtrl,c_name,wind)
    CHECK_CONTROL(wxFilePickerCtrl,c_name,wind)
    CHECK_CONTROL(wxTimer,c_name,wind)
    CHECK_CONTROL(wxSpinCtrl,c_name,wind)
    CHECK_CONTROL(wxSpinButton,c_name,wind)

    CHECK_CONTROL_END(c_name)

    return SC_RETURN_OK;
}

void bind_wxBaseControls(HSQUIRRELVM vm);
}


#endif // SQ_WXBASECONTROLS

