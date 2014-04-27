#ifndef SQ_WXDIALOG
#define SQ_WXDIALOG

#include <scripting/bindings/sq_wx/sq_wx_base_controls.h>
#include <scripting/bindings/sc_binding_util.h>
#include <scripting/bindings/sc_cb_vm.h>
#include <scripting/bindings/sc_base_types.h>
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

    class sq_wxDialog : public cb_wxBaseManagedWindow<wxDialog>
    {
    public:

        sq_wxDialog(HSQUIRRELVM vm);
        ~sq_wxDialog();

        int LoadFromXRCFile(wxString file,wxString name);
        int LoadFromXRCPool(wxString name);

        //void RegisterEventHandler(wxEventType type,int id,Sqrat::Object obj,wxString handler);

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

        bool IsLoaded()     {return (GetManagedWindow() != nullptr); };

        void Center();

        friend SQInteger sq_wxDialog_constructor(HSQUIRRELVM vm);


    protected:

        //void OnEvt(wxEvent& event);

    private:


    };
}


#endif // SQ_WXDIALOG
