
#include <scripting/bindings/sq_wx/sq_wx_dialog.h>
#include <wx/button.h>



/** \defgroup sq_wxBaseWindows Base windows for wxWidgets
*  \ingroup Squirrel
*  \brief Windows (ex. wxDialog) and utility functions for loading and handling XRC/XML resource files
*
*
*  # Windows and Frames
*
*
*  ## Index
*
*  [TOC]
*
*  # Base windows
*
*  ## Dialog (wxDialog)
*
*  The base dialog. Can be loaded from a XRC file.
*   | Name                  | Parameter                                                            | Description                                        | Info                        |
*   | :---------------------| :------------------------------------------------------------------  | :------------------------------------------------- | :-------------------------- |
*   | RegisterEventHandler  | int evt_id, int id, squirrel:object base_handler, string func_name   |  register a handler function for a specific wxEVT  |                             |
*   | Destroy               |                                                                      |  destroy the dialog                                |                             |
*   | CreateTimer           |                                                                      |  create and return a timer                         |                             |
*   | GetTimer              |                                                                      |  return the timer with the id                      |                             |
*   |                       |                                                                      |                                                    |                             |
*   | LoadFromXRCFile       |  string file, string name                                            |  load the dialog _name_ from the _file_            |                             |
*   | LoadFromXRCPool       |  string name                                                         |  load the dialog _name_ from the XRC pool          |                             |
*   | IsModal               |                                                                      |  return true if it is modal                        |                             |
*   | Center                |                                                                      |  center the dialog                                 |                             |
*   | ShowModal             |                                                                      |  show the dialog as a modal dialog (blocking)      |                             |
*   | Show                  |                                                                      |  show the dialog as non modal dialog               |                             |
*   | EndModal              |  int                                                                 |  end the modal dialog and return the code          |                             |
*   | SetTitle              |                                                                      |  set the title of the dialog                       |                             |
*   | Maximize              |                                                                      |  maximize the dialog                               |                             |
*   | IsLoaded              |                                                                      |  return true if the dialog is loaded               |                             |
*   | GetControl            |  int id                                                              |  return the control element with the _id_          |                             |
*
*   ### Return value of LoadFromXRCFile and LoadFromXRCPool
*   | Name                                    | Description                                                          |  Info                                              |
*   | :---------------------------------------| :------------------------------------------------------------------  | :------------------------------------------------- |
*   | RESOURCE_LOADED_SUCCESFULLY             | The resource was loaded successfully                                 |                                                    |
*   | XRC_FILE_NOT_FOUND                      | The file could not be found                                          |                                                    |
*   | RESOURCE_ALREADY_LOADED                 | The resource is occupied by a other resource                         |                                                    |
*   | RESOURCE_NOT_FOUND_IN_LOADED_RESOURCES  | The resource could not be found in the file or int the XRC pool      |                                                    |
*
**/

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

sq_wxDialog::sq_wxDialog(HSQUIRRELVM vm) :  cb_wxBaseManagedWindow(vm)
{

}

sq_wxDialog::~sq_wxDialog()
{

}

int sq_wxDialog::LoadFromXRCFile(wxString file,wxString name)
{

    if(!wxXmlResource::Get()->Load(file))
        return XRC_FILE_NOT_FOUND;

    return LoadFromXRCPool(name);
}

int sq_wxDialog::LoadFromXRCPool(wxString name)
{
    if(GetManagedWindow() != nullptr)
        return RESOURCE_ALREADY_LOADED;

   SetManagedWindow(wxXmlResource::Get()->LoadDialog(NULL,name));

    if(GetManagedWindow() == nullptr)
        return RESOURCE_NOT_FOUND_IN_LOADED_RESOURCES;

    ConnectEvtHandler();

    return RESOURCE_LOADED_SUCCESFULLY;
}

bool sq_wxDialog::IsModal()
{
    if(GetManagedWindow() == nullptr)
        return false;

    return GetManagedWindow()->IsModal();
}

void sq_wxDialog::Center()
{
    if(GetManagedWindow() == nullptr)
        return;

    GetManagedWindow()->Centre();
}

int sq_wxDialog::ShowModal()
{
    if(GetManagedWindow() == nullptr)
        return -1;

    return GetManagedWindow()->ShowModal();
}

void sq_wxDialog::EndModal(int retcode)
{
    if(GetManagedWindow() == nullptr)
        return;

    GetManagedWindow()->EndModal(retcode);
}

int sq_wxDialog::Show(bool show)
{
    if(GetManagedWindow() == nullptr)
        return -1;
    GetManagedWindow()->Show(show);
    return 0;
}

void sq_wxDialog::SetTitle(wxString& title)
{
    if(GetManagedWindow() == nullptr)
        return;
    GetManagedWindow()->SetTitle(title);
}

void sq_wxDialog::Maximize(bool maximize)
{
    if(GetManagedWindow() == nullptr)
        return;
    GetManagedWindow()->Maximize(maximize);
}


wxWindow* sq_wxDialog::GetWindow()
{
    return dynamic_cast<wxWindow*>(GetManagedWindow());
}


void bind_wxDialog(HSQUIRRELVM vm)
{
    Sqrat::Class<cb_wxBaseManagedWindow<wxDialog>, Sqrat::NoConstructor<cb_wxBaseManagedWindow<wxDialog> > > bBasewxDialog(vm,"wxBaseDialog");
    bBasewxDialog.Func("RegisterEventHandler", &cb_wxBaseManagedWindow<wxDialog>::RegisterEventHandler);
    bBasewxDialog.Func("Destroy", &cb_wxBaseManagedWindow<wxDialog>::Destroy);
    bBasewxDialog.Func("CreateTimer", &cb_wxBaseManagedWindow<wxDialog>::CreateTimer);
    bBasewxDialog.Func("GetTimer", &cb_wxBaseManagedWindow<wxDialog>::GetTimer);
    //.Func("RegisterEventHandler", &cb_wxBaseManagedWindow<wxDialog>::RegisterEventHandler);

    Sqrat::DerivedClass<sq_wxDialog,cb_wxBaseManagedWindow<wxDialog> ,Sqrat::NoConstructor<sq_wxDialog> > bwxDialog(vm,"wxDialog"); // One parameter to get the function context in which the callbacks are searched
    bwxDialog.Func("LoadFromXRCFile", &sq_wxDialog::LoadFromXRCFile)
    .Func("LoadFromXRCPool", &sq_wxDialog::LoadFromXRCPool)
    .Func("IsModal", &sq_wxDialog::IsModal)
    .Func("Center", &sq_wxDialog::Center)
    .Func("ShowModal", &sq_wxDialog::ShowModal)
    .Func("Show", &sq_wxDialog::Show)
    .Func("EndModal", &sq_wxDialog::EndModal)
    .Func("SetTitle", &sq_wxDialog::SetTitle)
    .Func("Maximize", &sq_wxDialog::Maximize)
    .Func("IsLoaded", &sq_wxDialog::IsLoaded)
    .Func("GetWindow", &sq_wxDialog::GetWindow)
    .SquirrelFunc("GetControl", &GetControlTemplate<sq_wxDialog>);

    Sqrat::RootTable(vm).Bind(_SC("wxDialog"),bwxDialog);


    BIND_INT_CONSTANT(RESOURCE_LOADED_SUCCESFULLY);
    BIND_INT_CONSTANT(XRC_FILE_NOT_FOUND);
    BIND_INT_CONSTANT(RESOURCE_ALREADY_LOADED);
    BIND_INT_CONSTANT(RESOURCE_NOT_FOUND_IN_LOADED_RESOURCES);


}

}
