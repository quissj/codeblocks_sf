
#include <sq_wx/sq_wxDialog.h>
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

void sq_wxDialog::Centre()
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


void bind_wxDialog(HSQUIRRELVM vm)
{
    Sqrat::Class<cb_wxBaseManagedWindow<wxDialog>, Sqrat::NoConstructor<cb_wxBaseManagedWindow<wxDialog> > > bBasewxDialog(vm,"wxBaseDialog");
    bBasewxDialog.Func("RegisterEventHandler", &cb_wxBaseManagedWindow<wxDialog>::RegisterEventHandler);
    bBasewxDialog.Func("Destroy", &cb_wxBaseManagedWindow<wxDialog>::Destroy);
    //.Func("RegisterEventHandler", &cb_wxBaseManagedWindow<wxDialog>::RegisterEventHandler);

    Sqrat::DerivedClass<sq_wxDialog,cb_wxBaseManagedWindow<wxDialog> ,Sqrat::NoCopy<sq_wxDialog> > bwxDialog(vm,"wxDialog");
    bwxDialog.SquirrelFunc("constructor",&sq_wxDialog_constructor)                  // One parameter to get the function context in which the callbacks are searched
    .Func("LoadFromXRCFile", &sq_wxDialog::LoadFromXRCFile)
    .Func("LoadFromXRCPool", &sq_wxDialog::LoadFromXRCPool)
    .Func("IsModal", &sq_wxDialog::IsModal)
    .Func("Centre", &sq_wxDialog::Centre)
    .Func("ShowModal", &sq_wxDialog::ShowModal)
    .Func("Show", &sq_wxDialog::Show)
    .Func("EndModal", &sq_wxDialog::EndModal)
    .Func("SetTitle", &sq_wxDialog::SetTitle)
    .Func("Maximize", &sq_wxDialog::Maximize)
    .Func("IsLoaded", &sq_wxDialog::IsLoaded)
    .SquirrelFunc("GetControl", &GetControlTemplate<sq_wxDialog>);
    Sqrat::RootTable(vm).Bind(_SC("wxDialog"),bwxDialog);


    BIND_INT_CONSTANT(RESOURCE_LOADED_SUCCESFULLY);
    BIND_INT_CONSTANT(XRC_FILE_NOT_FOUND);
    BIND_INT_CONSTANT(RESOURCE_ALREADY_LOADED);
    BIND_INT_CONSTANT(RESOURCE_NOT_FOUND_IN_LOADED_RESOURCES);


}

}
