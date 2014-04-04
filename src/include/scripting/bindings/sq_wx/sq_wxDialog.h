#ifndef SQ_WXDIALOG
#define SQ_WXDIALOG

#include "sc_binding_util.h"
#include "sc_cb_vm.h"
#include "sc_base_types.h"
#include <wx/string.h>
#include <wx/event.h>
#include <wx/xrc/xmlres.h>
#include <map>

namespace ScriptBindings
{

    const int RESOURCE_LOADED_SUCCESFULLY               =  0;
    const int XRC_FILE_NOT_FOUND                        = -1;
    const int RESOURCE_ALREADY_LOADED                   = -2;
    const int RESOURCE_NOT_FOUND_IN_LOADED_RESOURCES    = -3;


    SQInteger sq_wxDialog_constructor(HSQUIRRELVM vm);

    void bind_wxDialog(HSQUIRRELVM vm);



    class sq_wxDialog : public wxEvtHandler
    {
    public:

        sq_wxDialog(HSQUIRRELVM vm);
        ~sq_wxDialog();

        int LoadFromXRCFile(wxString file,wxString name);
        int LoadFromXRCPool(wxString name);

        void RegisterEventHandler(wxEventType type,int id,Sqrat::Object obj,wxString handler);

        /** \brief Shows a Modal dialog.
         *
         * \return int returns the return code of EndModal or -1 if no dialog was loaded
         *
         */
        int ShowModal();
        int Show(bool show);
        void EndModal(int retcode);

        void SetTitle(wxString& title);
        void Maximize(bool maximize);

        bool IsModal();

        void Centre();

        friend SQInteger sq_wxDialog_constructor(HSQUIRRELVM vm);

    protected:
        virtual bool TryBefore(wxEvent& event);
        void OnEvt(wxEvent& event);

    private:

        struct evt_func
        {
            wxString func_name;
            Sqrat::Object env;
        };

        typedef std::map<int,evt_func>                  evt_id_func_map;
        typedef std::map<wxEventType,evt_id_func_map>   evt_type_id_map;
        evt_type_id_map m_evt_map;

        Sqrat::Object m_object;
        HSQUIRRELVM m_vm;
        wxDialog* m_dialog;

    };
}


#endif // SQ_WXDIALOG
