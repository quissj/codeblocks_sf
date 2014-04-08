
#include <sq_wx/sq_wxBaseControls.h>
#include <wx/button.h>
#include <wx/textctrl.h>
#include <wx/animate.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/collpane.h>
#include <wx/combobox.h>


#if wxCHECK_VERSION(2, 9, 0)
    #include <wx/anybutton.h>
    #include <wx/commandlinkbutton.h>
#endif


namespace ScriptBindings
{

static int GetIDfromXRC(wxString name)
{
    return  wxXmlResource::Get()->GetXRCID(name);
}

static wxString GetNameFromIDFromXRC(int id)
{
#if wxCHECK_VERSION(2, 9, 0)
    return wxXmlResource::Get()->FindXRCIDById(id);
#else
    return wxString(_("only in wxWidgets > 2.9.0"));
#endif
}



void bind_wxBaseControls(HSQUIRRELVM vm)
{
    Sqrat::RootTable(vm).Func(_SC("XRCID"),&GetIDfromXRC);
    Sqrat::RootTable(vm).Func(_SC("XRCNAME"),&GetNameFromIDFromXRC);


    //**************************************************************************************************************/
    // wxWindow
    //**************************************************************************************************************/
    Sqrat::Class<wxEvent,Sqrat::NoConstructor<wxEvent> > bwxEvent(vm,"wxEvent");
    bwxEvent.Func("GetEventType",&wxEvent::GetEventType)
    .Func("GetId",&wxEvent::GetId)
    .Func("GetTimestamp",&wxEvent::GetTimestamp)
    .Func("IsCommandEvent",&wxEvent::IsCommandEvent);
    Sqrat::RootTable(vm).Bind(_SC("wxEvent"),bwxEvent);



    //**************************************************************************************************************/
    // wxWindow
    //**************************************************************************************************************/
    Sqrat::Class<wxWindow,Sqrat::NoConstructor<wxWindow> > bwxWindow(vm,"wxWindow");
    bwxWindow.Func("GetLabel",&wxWindow::GetLabel)
    .Func("SetLabel",&wxWindow::SetLabel);
    Sqrat::RootTable(vm).Bind(_SC("wxWindow"),bwxWindow);

    /***************************************************************************************************************/
    // wxControl
    /***************************************************************************************************************/
    Sqrat::DerivedClass<wxControl,wxWindow,Sqrat::NoConstructor<wxControl> > bwxControl(vm,"wxControl");
    bwxControl.Func("GetLabel",&wxControl::GetLabel)
#if wxCHECK_VERSION(2, 9, 0)
    .Func("SetLabelMarkup",&wxControl::SetLabelMarkup)
#endif // wxCHECK_VERSION
    .Func("SetLabel",&wxControl::SetLabel);
    Sqrat::RootTable(vm).Bind(_SC("wxControl"),bwxControl);


#if wxCHECK_VERSION(2, 9, 0)
    /***************************************************************************************************************/
    // wxAnyButton
    /***************************************************************************************************************/
    Sqrat::DerivedClass<wxAnyButton,wxControl,Sqrat::NoConstructor<wxControl> > bwxAnyButton(vm,"wxAnyButton");
    Sqrat::RootTable(vm).Bind(_SC("wxAnyButton"),bwxAnyButton);
#endif

    /***************************************************************************************************************/
    // wxTextCtrl
    /***************************************************************************************************************/
    Sqrat::Class<wxTextCtrl,Sqrat::NoConstructor<wxTextCtrl> > bwxTextCtrl(vm,"wxTextCtrl");
    bwxTextCtrl.Func("GetValue",&wxTextCtrl::GetValue)
    .Func("SetValue",&wxTextCtrl::SetValue);
    Sqrat::RootTable(vm).Bind(_SC("wxTextCtrl"),bwxTextCtrl);

    /***************************************************************************************************************/
    // wxButton
    /***************************************************************************************************************/
#if wxCHECK_VERSION(2, 9, 0)
    Sqrat::DerivedClass<wxButton,wxAnyButton, Sqrat::NoConstructor<wxButton> > bwxButton(vm,"wxButton");
#else
    Sqrat::DerivedClass<wxButton,wxControl, Sqrat::NoConstructor<wxButton> > bwxButton(vm,"wxButton");
#endif
    bwxButton.Func("GetLabel",&wxButton::GetLabel)
    .Func("SetLabel",&wxButton::SetLabel);
    Sqrat::RootTable(vm).Bind(_SC("wxButton"),bwxButton);


    /***************************************************************************************************************/
    // wxAnimationCtrl
    /***************************************************************************************************************/
    Sqrat::DerivedClass<wxAnimationCtrl,wxControl, Sqrat::NoConstructor<wxAnimationCtrl> >  bwxAnimationCtrl(vm,"wxAnimationCtrl");
    bwxAnimationCtrl.Func("IsPlaying",&wxAnimationCtrl::IsPlaying)
    .Func("LoadFile",&wxAnimationCtrl::LoadFile)
    .Overload<bool (wxAnimationCtrl::*)(bool)>("Play",&wxAnimationCtrl::Play)
    .Func("Stop",&wxAnimationCtrl::Stop)
    .Func("GetAnimation",&wxAnimationCtrl::GetAnimation);
    Sqrat::RootTable(vm).Bind(_SC("wxAnimationCtrl"),bwxAnimationCtrl);

    BIND_INT_CONSTANT(wxANIMATION_TYPE_INVALID);
    BIND_INT_CONSTANT(wxANIMATION_TYPE_GIF);
    BIND_INT_CONSTANT(wxANIMATION_TYPE_ANI);
    BIND_INT_CONSTANT(wxANIMATION_TYPE_ANY);

#if wxCHECK_VERSION(2, 9, 0)
    /***************************************************************************************************************/
    // wxCommandLinkButton
    /***************************************************************************************************************/
    Sqrat::DerivedClass<wxCommandLinkButton,wxControl, Sqrat::NoConstructor<wxCommandLinkButton> > bwxCommandLinkButton(vm,"wxCommandLinkButton");
    bwxCommandLinkButton.Func("SetMainLabelAndNote",&wxCommandLinkButton::SetMainLabelAndNote)
    .Func("SetLabel",&wxCommandLinkButton::SetLabel)
    .Func("GetLabel",&wxCommandLinkButton::GetLabel)
    .Func("SetNote",&wxCommandLinkButton::SetNote)
    .Func("GetNote",&wxCommandLinkButton::GetNote)
    .Func("GetMainLabel",&wxCommandLinkButton::GetMainLabel)
    .Func("SetMainLabel",&wxCommandLinkButton::SetMainLabel);
    Sqrat::RootTable(vm).Bind(_SC("wxCommandLinkButton"),bwxCommandLinkButton);
#endif

    /***************************************************************************************************************/
    // wxCheckBox
    /***************************************************************************************************************/
    Sqrat::DerivedClass<wxCheckBox,wxControl, Sqrat::NoConstructor<wxCheckBox> > bwxCheckBox(vm,"wxCheckBox");
    bwxCheckBox.Func("GetValue",&wxCheckBox::GetValue)
    .Func("Get3StateValue",&wxCheckBox::Get3StateValue)
    .Func("Is3State",&wxCheckBox::Is3State)
    .Func("Is3rdStateAllowedForUser",&wxCheckBox::Is3rdStateAllowedForUser)
    .Func("IsChecked",&wxCheckBox::IsChecked)
    .Func("SetValue",&wxCheckBox::SetValue)
    .Func("Set3StateValue",&wxCheckBox::Set3StateValue );
    Sqrat::RootTable(vm).Bind(_SC("wxCheckBox"),bwxCheckBox);

    BIND_INT_CONSTANT(wxCHK_UNCHECKED);
    BIND_INT_CONSTANT(wxCHK_CHECKED);
    BIND_INT_CONSTANT(wxCHK_UNDETERMINED);

    /***************************************************************************************************************/
    // wxChoice
    /***************************************************************************************************************/
    Sqrat::DerivedClass<wxChoice,wxControl, Sqrat::NoConstructor<wxChoice> > bwxChoice(vm,"wxChoice");
    bwxChoice.Func("GetColumns",&wxChoice::GetColumns)
    .Func("GetCurrentSelection ",&wxChoice::GetCurrentSelection)
    .Func("GetSelection",&wxChoice::GetSelection)
    .Func("SetColumns",&wxChoice::SetColumns)
    .Func("GetCount",&wxChoice::GetCount )
    .Func("SetSelection",&wxChoice::SetSelection)
    .Func("FindString",&wxChoice::FindString)
    .Func("GetString",&wxChoice::GetString)
    .Func("SetString",&wxChoice::SetString)
    //wxItemContainer
    .Func("Clear",&wxChoice::Clear)
    .Func("Delete",&wxChoice::Delete)
    .Overload<int (wxChoice::*)(const wxString&)>("Append",&wxChoice::Append)
    .Overload<int (wxChoice::*)(const wxString&,unsigned int)>("Insert",&wxChoice::Insert);
    Sqrat::RootTable(vm).Bind(_SC("wxChoice"),bwxChoice);

    /***************************************************************************************************************/
    // wxCollapsiblePane
    /***************************************************************************************************************/
    Sqrat::DerivedClass<wxCollapsiblePane,wxControl, Sqrat::NoConstructor<wxCollapsiblePane> > bwxCollapsiblePane(vm,"wxCollapsiblePane");
    bwxCollapsiblePane.Func("Expand",&wxCollapsiblePane::Expand)
    .Func("IsCollapsed",&wxCollapsiblePane::IsCollapsed)
    .Func("IsExpanded",&wxCollapsiblePane::IsExpanded)
    .Func("Collapse",&wxCollapsiblePane::Collapse);
    Sqrat::RootTable(vm).Bind(_SC("wxCollapsiblePane"),bwxCollapsiblePane);


    /***************************************************************************************************************/
    // wxComboBox
    /***************************************************************************************************************/
    Sqrat::DerivedClass<wxComboBox,wxControl, Sqrat::NoConstructor<wxComboBox> > bwxComboBox(vm,"wxComboBox");
    bwxComboBox
    //.Func("IsListEmpty",&wxComboBox::IsListEmpty)
    //.Func("IsTextEmpty",&wxComboBox::IsTextEmpty)
    //.Func("SetValue",&wxComboBox::SetValue)
    //.Func("Dismiss",&wxComboBox::Dismiss)
    .Overload<int (wxComboBox::*)() const>("GetSelection",&wxComboBox::GetSelection)
    .Overload<void (wxComboBox::*)(long*,long*) const>("GetSelection",&wxComboBox::GetSelection)
    .Func("FindString",&wxComboBox::FindString)
    .Func("GetString",&wxComboBox::GetString);
    //.Func("Popup",&wxComboBox::Popup);
    Sqrat::RootTable(vm).Bind(_SC("wxComboBox"),bwxComboBox);

}
}

