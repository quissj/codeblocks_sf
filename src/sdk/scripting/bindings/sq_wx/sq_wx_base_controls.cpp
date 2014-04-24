
#include <sq_wx/sq_wx_base_controls.h>
#include <sq_wx/sq_wx_type_handler.h>
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

/** \defgroup sq_wxBaseControls wxWidgets control elements bound to squirrel
*  \ingroup Squirrel
*  \brief Controls (ex. wxTextCtrl) and utility functions for loading and handling XRC/XML resource files
*
*
*  # wxWidgets base controls
*
*
*  ## Index
*
*
*  [TOC]
*
*  # wxWidgets base controls
*
*  ## Global resource-handling functions bound to squirrel
*
*  This functions base all on the wxWidgets equivalents so please read the wxWidgets XRC Manual for more information
*   | Name                 | Parameter                     | Description                               | Info                        |
*   | :------------------- | :---------------------------  | :---------------------------------------- | :-------------------------- |
*   | XRCID                | string                        |  Get the ID of a xrc resource file object |                             |
*   | XRCNAME              | string                        |  Get the name of a xrc object from the ID |                             |
*   | LoadXMLResourceFile  | string                        |  Load a xrc file/files using a path mask  |  see: wxXmlResource::Load   |
*
*
*  ## wxEvent class squirrel binding
*
*  This functions base all on the wxWidgets equivalents so please read the wxWidgets wxEvent manual for more information
*   | Name                 | Parameter                     | Description                              | Info       |
*   | :------------------- | :---------------------------  | :--------------------------------------- | :--------- |
*   | GetEventType         |                               |  Get the event type                      |            |
*   | GetTimestamp         |                               |  Get the event TimeStamp                 |            |
*   | IsCommandEvent       |                               |  Return if the event is a command event  |            |
*   | Skip                 |                               |  handle the event in other evt handler also|          |
*
*
*  ## wxWindow class squirrel binding
*
*  This functions base all on the wxWidgets equivalents so please read the wxWidgets wxWindow manual for more information
*   | Name                 | Parameter                     | Description                                | Info       |
*   | :------------------- | :---------------------------  | :----------------------------------------  | :--------- |
*   | GetLabel             |                               | Get the label of the window                |            |
*   | SetLabel             |                               | Set the label of the window                |            |
*   | IsCommandEvent       |                               | Return true if the event is a command event|            |
*
*
*  ## wxTextCtrl class squirrel binding
*
*  This functions base all on the wxWidgets equivalents so please read the wxWidgets wxTextCtrl manual for more information
*   | Name                 | Parameter                     | Description                                | Info       |
*   | :------------------- | :---------------------------  | :----------------------------------------  | :--------- |
*   | GetValue             |                               | Get the value of the text control          |            |
*   | SetValue             | string                        | Set the value of the text control          |            |
*
*
*  ## wxButton class squirrel binding
*
*  This functions base all on the wxWidgets equivalents so please read the wxWidgets wxButton manual for more information
*   | Name                 | Parameter                     | Description                                | Info       |
*   | :------------------- | :---------------------------  | :----------------------------------------  | :--------- |
*   | GetLabel             |                               | Get the label of the button                |            |
*   | SetLabel             | string                        | Set the label of the button                |            |
*
*
*  ## wxAnimationCtrl class squirrel binding
*
*  This functions base all on the wxWidgets equivalents so please read the wxWidgets wxAnimationCtrl manual for more information
*   | Name                  | Parameter                     | Description                                | Info       |
*   | :-------------------  | :---------------------------  | :----------------------------------------  | :--------- |
*   | IsPlaying             |                               | return true if the animation is playing    |            |
*   | LoadFile              | string , type                 | Load a animation file, specify the _type_  |            |
*   | Play                  |                               | Set the label of the button                |            |
*   | Stop                  |                               | Set the label of the button                |            |
*   | GetAnimation          |                               | Set the label of the button                |            |
*
*  ### Types of animation files
*   | Name                       | Description                                 | Info       |
*   | :------------------------  | :----------------------------------------   | :--------- |
*   | wxANIMATION_TYPE_INVALID   |                                             |            |
*   | wxANIMATION_TYPE_GIF       |                                             |            |
*   | wxANIMATION_TYPE_ANI       |                                             |            |
*   | wxANIMATION_TYPE_ANY       |                                             |            |
*
*
*  ## wxCommandLinkButton class squirrel binding
*
*  !!! Only wxWidgets 2.9
*  This functions base all on the wxWidgets equivalents so please read the wxWidgets wxCommandLinkButton manual for more information
*   | Name                  | Parameter                     | Description                                | Info       |
*   | :-------------------  | :---------------------------  | :----------------------------------------  | :--------- |
*   | SetMainLabelAndNote   | string, string                | Sets the main label and the note           |            |
*   | SetLabel              | string                        |                                            |            |
*   | GetLabel              |                               |                                            |            |
*   | SetNote               | string                        | Set the note                               |            |
*   | GetNote               |                               | Get the note                               |            |
*   | GetMainLabel          |                               | Get the main label                         |            |
*   | SetMainLabel          | string                        | Set the main label                         |            |
*
*
*  ## wxCheckBox class squirrel binding
*
*  This functions base all on the wxWidgets equivalents so please read the wxWidgets wxCheckBox manual for more information
*   | Name                      | Parameter                     | Description                                    | Info       |
*   | :-----------------------  | :---------------------------  | :--------------------------------------------- | :--------- |
*   | GetValue                  |                               | Return the checkbox value                      |            |
*   | Get3StateValue            |                               | Return 3 state value of the checkbox           |            |
*   | Is3State                  |                               | Return true if it is a 3 state control         |            |
*   | Is3rdStateAllowedForUser  |                               | Return true if the control is editable by user |            |
*   | IsChecked                 |                               | Return true if it checked                      |            |
*   | SetValue                  |                               | Set the value of the control                   |            |
*   | Set3StateValue            |                               | Set the 2 state value of the control           |            |
*
*  ### checkbox states
*   | Name                | Description                         | Info       |
*   | :-----------------  | :---------------------------------  | :--------- |
*   | wxCHK_UNCHECKED     |                                     |            |
*   | wxCHK_CHECKED       |                                     |            |
*   | wxCHK_UNDETERMINED  |                                     |            |
*
*
*  ## wxChoice class squirrel binding
*
*  This functions base all on the wxWidgets equivalents so please read the wxWidgets wxChoice manual for more information
*   | Name                 | Parameter                     | Description                                      | Info       |
*   | :------------------- | :---------------------------  | :----------------------------------------------- | :--------- |
*   | GetColumns           |                               | return the number of columns                     |            |
*   | GetCurrentSelection  |                               | get current selection                            |            |
*   | GetSelection         |                               | get current selection                            |            |
*   | SetColumns           |  int                          | set the number of columns                        |            |
*   | GetCount             |                               | returns the number of items in  the control      |            |
*   | SetSelection         |  int                          | sets the selection of the control                |            |
*   | FindString           |  string, bool case            | find item whose label matches the current string |            |
*   | GetString            |  int                          | returns label at position                        |            |
*   | SetString            |  int, string                  | set string at position n                         |            |
*   | Clear                |                               | clears the control                               |            |
*   | Delete               |  int                          | delete the item n from the control               |            |
*   | Append               |  string                       | append the item at the end                       |            |
*   | Insert               |  string, int                  | insert the item at the position                  |            |
*
*
*  ## wxCollapsiblePane class squirrel binding
*
*  This functions base all on the wxWidgets equivalents so please read the wxWidgets wxCollapsiblePane manual for more information
*   | Name                 | Parameter                     | Description                                      | Info       |
*   | :------------------- | :---------------------------  | :----------------------------------------------- | :--------- |
*   | Expand               |                               | expand the control                               |            |
*   | IsCollapsed          |                               | return true if expanded                          |            |
*   | IsExpanded           |                               | return true if collapsed                         |            |
*   | Collapse             |                               | collapse the control                             |            |
*
*
*  ## wxComboBox class squirrel binding
*
*
*  This functions base all on the wxWidgets equivalents so please read the wxWidgets wxComboBox manual for more information
*   | Name                 | Parameter                     | Description                                      | Info           |
*   | :------------------- | :---------------------------  | :----------------------------------------------- | :------------- |
*   | Dismiss              |                               | dismiss the control                              |  only > wx2.9  |
*   | Popup                |                               | expand the control                               |  only > wx2.9  |
*   | GetSelection         |                               | return the selection                             |                |
*   | GetSelection         |                               | return a table with two entries: _from_ _to_     |                |
*   | GetValue             |                               | get the value of the text control                |                |
*   | GetString            |  int                          | return the label at position                     |                |
*   | SetValue             |  string                       | set the value of the text control                |                |
*
*
*  ## wxRadioBox class squirrel binding
*
*  This functions base all on the wxWidgets equivalents so please read the wxWidgets wxRadioBox manual for more information
*   | Name                 | Parameter                     | Description                                      | Info           |
*   | :------------------- | :---------------------------  | :----------------------------------------------- | :------------- |
*   | FindString           |  string                       | returns the index of the item with the _string_  |                |
*   | IsItemEnabled        |  int                          | return true if the item is enabled               |                |
*   | GetCount             |                               | get the count of items in the control            |                |
*   | SetItemHelpText      |  int, string                  | sets the help text for the item                  |                |
*   | SetItemToolTip       |  int, string                  | sets the tooltip for the item in the radio group |                |
*   | GetSelection         |                               | returns the index of the selected item or wxNOT_FOUND|            |
*   | Show                 |  int, bool                    | Shows or hides individual buttons                |                |
*   | Enable               |  int, bool                    | enables or disables an individual button         |                |
*   | GetColumnCount       |                               | returns the number of columns in the radiobox    |                |
*   | GetString            |  int                          | get the label of the item                        |                |
*
*
*  ## wxGauge class squirrel binding
*
*  This functions base all on the wxWidgets equivalents so please read the wxWidgets wxGauge manual for more information
*   | Name                 | Parameter                     | Description                                      | Info           |
*   | :------------------- | :---------------------------  | :----------------------------------------------- | :------------- |
*   | GetRange             |                               | returns the maximum range of the gauge           |                |
*   | GetValue             |                               | returns the current position of the gauge        |                |
*   | IsVertical           |                               | returns true if vertical                         |                |
*   | Pulse                |                               | gauge to indeterminate state and pulse           |                |
*   | SetRange             |  int                          | Set the maximum range of the gauge               |                |
*   | SetValue             |  int                          | Set the current value of the gauge               |                |
*   | SetShadowWidth       |  int                          | set the shadow width                             |                |
*   | GetShadowWidth       |  int                          | get the shadow width                             |                |
*   | GetBezelFace         |                               | get the bezel face width                         |                |
*   | SetBezelFace         |  int                          | set the bezel face width                         |                |
*
*
*  ## wxHyperlinkCtrl class squirrel binding
*
*  This functions base all on the wxWidgets equivalents so please read the wxWidgets wxHyperlinkCtrl manual for more information
*   | Name                 | Parameter                     | Description                                      | Info           |
*   | :------------------- | :---------------------------  | :----------------------------------------------- | :------------- |
*   | GetURL               |                               | get the url                                      |                |
*   | GetVisited           |                               | return true if visited                           |                |
*   | SetHoverColour       |  wxColor                      | set color to use to print the url if it is hovered|               |
*   | SetNormalColour      |  wxColor                      | set color to use to print the url                |                |
*   | SetURL               |  string                       | set the url                                      |                |
*   | SetVisited           |  bool                         | set it the url is visited                        |                |
*   | SetVisitedColour     |  wxColor                      | set color to use to print the url if it was visited|              |
*
*
*  ## wxRadioButton class squirrel binding
*
*  This functions base all on the wxWidgets equivalents so please read the wxWidgets wxRadioButton manual for more information
*   | Name                 | Parameter                     | Description                                      | Info           |
*   | :------------------- | :---------------------------  | :----------------------------------------------- | :------------- |
*   | GetValue             |                               | return true if checked                           |                |
*   | SetValue             |                               | set the value                                    |                |
*
*
*
*  ## wxListBox class squirrel binding
*
*  This functions base all on the wxWidgets equivalents so please read the wxWidgets wxListBox manual for more information
*   | Name                 | Parameter                     | Description                                      | Info           |
*   | :------------------- | :---------------------------  | :----------------------------------------------- | :------------- |
*   | SetSelection         |                               |                                                  |                |
*   | GetSelections        |                               |  return squirrel array with the selections       |                |
*   | GetSelection         |                               |                                                  |                |
*   | IsSelected           |                               |                                                  |                |
*   | EnsureVisible        |                               |                                                  |                |
*   | GetCount             |                               |                                                  |                |
*   | GetString            |                               |                                                  |                |
*   | SetString            |                               |                                                  |                |
*   | FindString           |                               |                                                  |                |
*   | Delete               | int                           |                                                  |                |
*   | Clear                |                               |                                                  |                |
*   | Append               | string                        |                                                  |                |
*   | Insert               | string, int                   |                                                  |                |
*   | Set                  | string, int                   |                                                  |                |
*
*   ### Example for GetSelections
*
*  ~~~~~~~~~
*   local sel = dialog.GetControl("m_listBox1").GetSelections();
*   local txt = "listbox selected: ";
*   foreach(val in sel)
*       txt += val + " ";
*   print(txt + "\n");
*  ~~~~~~~~~
*
*  ## wxCheckListBox class squirrel binding
*
*  This functions base all on the wxWidgets equivalents so please read the wxWidgets wxCheckListBox manual for more information.
*  This control is derived from wxListBox.
*   | Name                 | Parameter                     | Description                                      | Info           |
*   | :------------------- | :---------------------------  | :----------------------------------------------- | :------------- |
*   | Check                | int, bool                     |  check a item                                    |                |
*   | IsChecked            |                               |  return true if the item is checked              |                |
*   | GetCheckedItems      |                               |  return a squirrel array with the checked items  |                |
*
*  ## wxStaticText class squirrel binding
*
*  This functions base all on the wxWidgets equivalents so please read the wxWidgets wxStaticText manual for more information.
*   | Name                 | Parameter                     | Description                                      | Info           |
*   | :------------------- | :---------------------------  | :----------------------------------------------- | :------------- |
*   | GetLabel             |                               |  return the text of the static text elem         |                |
*   | SetLabel             | string                        |  set the text of the static text elem            |                |
*
*
*
*  ## wxSlider class squirrel binding
*
*  This functions base all on the wxWidgets equivalents so please read the wxWidgets wxSlider manual for more information.
*   | Name                 | Parameter                     | Description                                      | Info           |
*   | :------------------- | :---------------------------  | :----------------------------------------------- | :------------- |
*   | ClearSel             |                               |                                                  |                |
*   | ClearTicks           |                               |                                                  |                |
*   | GetMax               |                               |                                                  |                |
*   | GetMin               |                               |                                                  |                |
*   | GetSelEnd            |                               |                                                  |                |
*   | GetSelStart          |                               |                                                  |                |
*   | GetThumbLength       |                               |                                                  |                |
*   | GetTickFreq          |                               |                                                  |                |
*   | GetValue             |                               |                                                  |                |
*   | SetMin               | int                           |                                                  |                |
*   | SetMax               | int                           |                                                  |                |
*   | SetRange             | int                           |                                                  |                |
*   | SetSelection         | int, int                      |                                                  |                |
*   | SetTick              | int                           |                                                  |                |
*   | SetTickFreq          | int                           |                                                  |                |
*   | SetValue             | int                           |                                                  |                |
*
*
*  ## wxToggleButton class squirrel binding
*
*  This functions base all on the wxWidgets equivalents so please read the wxWidgets wxToggleButton manual for more information.
*   | Name                 | Parameter                     | Description                                      | Info           |
*   | :------------------- | :---------------------------  | :----------------------------------------------- | :------------- |
*   | GetValue             |                               |  return true if the button is clicked            |                |
*   | SetValue             | bool                          |  set the state of the button (clicked or not)    |                |
*
*
*
*  ## wxSearchCtrl class squirrel binding
*  !!! not implemented because of the difficulties on different platforms and wx Versions...
*  This functions base all on the wxWidgets equivalents so please read the wxWidgets wxSearchCtrl manual for more information.
*   | Name                  | Parameter                     | Description                                            | Info           |
*   | :-------------------- | :---------------------------  | :----------------------------------------------------- | :------------- |
*   | ShowCancelButton      | bool                          |  show or hide the the cancel button                    |                |
*   | SetDescriptiveText    | string                        |  set the text what is shown before the user enters text| wx >2.9        |
*   | GetDescriptiveText    |                               |  get the text what is shown before the user enters text| wx >2.9        |
*   | IsSearchButtonVisible |                               |  true if the search button is visible                  |                |
*   | IsCancelButtonVisible |                               |  true if the cancel button is visible                  |                |
*   | ShowSearchButton      |                               |  show or hide the search button                        |                |
*
*
*  ## wxColourPickerCtrl class squirrel binding
*
*  This functions base all on the wxWidgets equivalents so please read the wxWidgets wxColourPickerCtrl manual for more information.
*   | Name                  | Parameter                     | Description                                       | Info           |
*   | :-------------------- | :---------------------------  | :------------------------------------------------ | :------------- |
*   | GetColour             |                               |  get the selected color                           |                |
*   | SetColour             | wxColour                      |  set the color                                    |                |
*
*
*  ## wxDirPickerCtrl class squirrel binding
*
*  This functions base all on the wxWidgets equivalents so please read the wxWidgets wxDirPickerCtrl manual for more information.
*   | Name                  | Parameter                     | Description                                       | Info           |
*   | :-------------------- | :---------------------------  | :------------------------------------------------ | :------------- |
*   | SetPath               |  string                       |  set the predefined path                          |                |
*   | GetPath               |                               |  get the selected path                            |                |
*
*
*  ## wxFilePickerCtrl class squirrel binding
*
*  This functions base all on the wxWidgets equivalents so please read the wxWidgets wxFilePickerCtrl manual for more information.
*   | Name                  | Parameter                     | Description                                       | Info           |
*   | :-------------------- | :---------------------------  | :------------------------------------------------ | :------------- |
*   | SetPath               |  string                       |  set the predefined path                          |                |
*   | GetPath               |                               |  get the selected path                            |                |
*
*
*  ## wxTimer class squirrel binding
*
*  This functions base all on the wxWidgets equivalents so please read the wxWidgets wxTimer manual for more information.
*   | Name              | Parameter                     | Description                                       | Info           |
*   | :-----------------| :---------------------------  | :------------------------------------------------ | :------------- |
*   | GetId             |                               |  return the id of the timer                       |                |
*   | IsOneShot         |                               |  return true if it is only one shot               |                |
*   | IsRunning         |                               |  return true if is running                        |                |
*   | Start             |  int millisec, bool oneshot   |  start the timer with the given interval          |                |
*   | Stop              |                               |  stop the timer                                   |                |
*   | GetInterval       |                               |  get the selected path                            |                |
*
*
*  ## wxSpinButton class squirrel binding
*
*  This functions base all on the wxWidgets equivalents so please read the wxWidgets wxSpinButton manual for more information.
*   | Name              | Parameter                     | Description                                       | Info           |
*   | :-----------------| :---------------------------  | :------------------------------------------------ | :------------- |
*   | GetMax            |                               |  return the max value                             |                |
*   | GetMin            |                               |  return the min value                             |                |
*   | GetValue          |                               |  return the actual value                          |                |
*   | SetRange          |  int min, int max             |  set min and max with one function                |                |
*   | SetValue          |  int                          |  set the value                                    |                |
*
*
*  ## wxSpinCtrl class squirrel binding
*
*  This functions base all on the wxWidgets equivalents so please read the wxWidgets wxSpinCtrl manual for more information.
*   | Name              | Parameter                     | Description                                       | Info           |
*   | :-----------------| :---------------------------  | :------------------------------------------------ | :------------- |
*   | GetMax            |                               |  return the max value                             |                |
*   | GetMin            |                               |  return the min value                             |                |
*   | GetValue          |                               |  return the actual value                          |                |
*   | SetRange          |  int min, int max             |  set min and max with one function                |                |
*   | SetValue          |  [int,string]                 |  set the value (integer or string)                |                |
*   | GetBase           |                               |  get the base                                     | wx > 2.9       |
*   | SetBase           |  int                          |  set the base                                     | wx > 2.9       |
*   | SetSelection      |  int from, int to             |  set the value                                    |                |
*
*
*
**/


/** \defgroup sq_helper_functions Squirrel-binding helper functions
 *  \brief The helper functions for binding various C::B and wxWidgets functions to squirrel
 **/

/** \brief wrapper function for wxXmlResource::Get()->GetXRCID()
 *  \ingroup sq_helper_functions
 *
 * \param name wxString The name of the element
 * \return int The corresponding ID
 *
 */
static int GetIDfromXRC(wxString name)
{
    return  wxXmlResource::Get()->GetXRCID(name);
}


/** \brief Wrapper function for wxXmlResource::Get()->FindXRCIDById() (only wx 2.9)
 *  \ingroup sq_helper_functions
 *
 * \param id int
 * \return wxString The name of the element or "only in wxWidgets > 2.9.0" if not implemented
 *
 */
static wxString GetNameFromIDFromXRC(int id)
{
#if wxCHECK_VERSION(2, 9, 0)
    return wxXmlResource::Get()->FindXRCIDById(id);
#else
    return wxString(_("only in wxWidgets > 2.9.0"));
#endif
}


/** \brief Wrapper function to bind wxXmlResource::Get()->Load
 *  \ingroup sq_helper_functions
 *
 * \param mask wxString A File mask. See wxXmlResource::Get()->Load() for details
 * \return bool true on success
 *
 */
bool LoadXMLResourceFile(wxString mask)
{
    return wxXmlResource::Get()->Load(mask);
}

SQInteger wxRadioBox_Show(HSQUIRRELVM vm)
{
    StackHandler sa(vm);
    // Get the this
    try
    {
    if(sa.GetParamCount() < 3)
        sa.ThrowError(_("wxRadioBox enable: wrong number of parameter"));

    wxRadioBox* radio_box = sa.GetInstance<wxRadioBox>(1);
    int item = sa.GetValue<int>(2);
    bool show = sa.GetValue<bool>(3);

    bool ret = radio_box->Show(item,show);
    sa.PushValue<bool>(ret);
    return SC_RETURN_VALUE;
    }
    catch(CBScriptException &e)
    {
       return sa.ThrowError(e.Message());
    }
}

SQInteger wxRadioBox_Enable(HSQUIRRELVM vm)
{
    StackHandler sa(vm);
    // Get the this
    try
    {
    if(sa.GetParamCount() < 3)
        sa.ThrowError(_("wxRadioBox enable: wrong number of parameter"));
    wxRadioBox* radio_box = sa.GetInstance<wxRadioBox>(1);
    int item = sa.GetValue<int>(2);
    bool show = sa.GetValue<bool>(3);

    bool ret = radio_box->Enable(item,show);
    sa.PushValue<bool>(ret);
    return SC_RETURN_VALUE;
    }
    catch(CBScriptException &e)
    {
        return sa.ThrowError(e.Message());
    }
}

SQInteger wxComboBox_GetSelection(HSQUIRRELVM vm)
{
    StackHandler sa(vm);

    try
    {
    int param_count = sa.GetParamCount();
    wxComboBox* inst = sa.GetInstance<wxComboBox>(1);
    if(param_count == 1)
    {
        // zero parameter -> normal get selection

        int ret = inst->GetSelection();
        sa.PushValue<int>(ret);
        return SC_RETURN_VALUE;
    }
    else if(param_count == 3)
    {
        long from = 0;
        long to = 0;
        inst->GetSelection(&from,&to);
        Sqrat::Table ret_val(vm);
        ret_val.SetValue("from",from);
        ret_val.SetValue("to",to);
        sa.PushValue(ret_val);
        return SC_RETURN_VALUE;
    }
    else
        return sa.ThrowError(_("wxComboBox GetSelection: wrong count of parameter"));

    }
    catch(CBScriptException &e)
    {
        return sa.ThrowError(e.Message());
    }
}

SQInteger wxListBox_GetSelections(HSQUIRRELVM vm)
{
    StackHandler sa(vm);
    try
    {
        wxListBox* inst = sa.GetInstance<wxListBox>(1);
        wxArrayInt arr;
        inst->GetSelections(arr);
        Sqrat::Array ret_val(vm);

        for(size_t i = 0; i < arr.GetCount();i++)
            ret_val.Append((int) arr[i]);

        sa.PushValue(ret_val);
        return SC_RETURN_VALUE;

    }
    catch(CBScriptException &e)
    {
        return sa.ThrowError(e.Message());
    }
}

SQInteger wxCheckListBox_GetCheckedItems(HSQUIRRELVM vm)
{
    StackHandler sa(vm);
    try
    {
        wxCheckListBox* inst = sa.GetInstance<wxCheckListBox>(1);
        wxArrayInt arr;
#if wxCHECK_VERSION(2, 9, 0)
        inst->GetCheckedItems(arr);
#else
        for(size_t i = 0;i < inst->GetCount(); i++)
        {
            if(inst->IsChecked(i))
                arr.Add(i);
        }
#endif

        Sqrat::Array ret_val(vm);

        for(size_t i = 0; i < arr.GetCount();i++)
            ret_val.Append((int) arr[i]);

        sa.PushValue(ret_val);
        return SC_RETURN_VALUE;

    }
    catch(CBScriptException &e)
    {
        return sa.ThrowError(e.Message());
    }
}

SQInteger wxSpinCtrl_SetValue(HSQUIRRELVM vm)
{
    StackHandler sa(vm);
    try
    {
        wxSpinCtrl* inst = sa.GetInstance<wxSpinCtrl>(1);

        Sqrat::Var<SQInteger> int_val(vm, 2);
        if (!Sqrat::Error::Instance().Occurred(vm))
        {
            inst->SetValue(int_val.value);
            return SC_RETURN_OK;
        }
        Sqrat::Error::Instance().Clear(vm);
        Sqrat::Var<wxString> str_val(vm, 2);
        if (!Sqrat::Error::Instance().Occurred(vm))
        {
            inst->SetValue(str_val.value);
            return SC_RETURN_OK;
        }

       return sa.ThrowError(_("wxSpinCtrl::SetValue could not get type of param"));
    }
    catch(CBScriptException &e)
    {
        return sa.ThrowError(e.Message());
    }
}

SQInteger wxAnimationCtrl_Play(HSQUIRRELVM vm)
{
    StackHandler sa(vm);
    wxAnimationCtrl*  inst = sa.GetInstance<wxAnimationCtrl>(1);
    if(inst == nullptr)
        return sa.ThrowError(_("wxAnimationCtrl: inst == nullptr"));
    sa.PushValue<bool>(inst->Play());
    return SC_RETURN_VALUE;
}

SQInteger wxSlider_SetTickFreq(HSQUIRRELVM vm)
{
    StackHandler sa(vm);
    wxSlider* inst = sa.GetInstance<wxSlider>(1);
    int freq = sa.GetValue<int>(2);
#if defined(__WXMSW__)
    inst->SetTickFreq(freq,1);  //what is the second param???
#else
    return sa.ThrowError(_("SetTickFreq not available on this platform"));
#endif // defined
    return SC_RETURN_OK;
}


void bind_wxBaseControls(HSQUIRRELVM vm)
{
    //**************************************************************************************************************/
    // global function
    //**************************************************************************************************************/
    Sqrat::RootTable(vm).Func(_SC("XRCID"),&GetIDfromXRC);
    Sqrat::RootTable(vm).Func(_SC("XRCNAME"),&GetNameFromIDFromXRC);
    Sqrat::RootTable(vm).Func(_SC("LoadXMLResourceFile"),&LoadXMLResourceFile);


    //**************************************************************************************************************/
    // wxEvent
    //**************************************************************************************************************/
    Sqrat::Class<wxEvent,Sqrat::NoConstructor<wxEvent> > bwxEvent(vm,"wxEvent");
    bwxEvent.Func("GetEventType",&wxEvent::GetEventType)
    .Func("GetId",&wxEvent::GetId)
    .Func("GetTimestamp",&wxEvent::GetTimestamp)
    .Func("IsCommandEvent",&wxEvent::IsCommandEvent)
    .Func("Skip",&wxEvent::Skip);
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
#if wxCHECK_VERSION(2, 9, 0)
    Sqrat::DerivedClass<wxControlBase,wxWindow,Sqrat::NoConstructor<wxControlBase> > bwxControlBase(vm,"wxControlBase");
    Sqrat::RootTable(vm).Bind(_SC("wxControlBase"),bwxControlBase);

    Sqrat::DerivedClass<wxControl,wxControlBase,Sqrat::NoConstructor<wxControl> > bwxControl(vm,"wxControl");
    bwxControl.Func("GetLabel",&wxControl::GetLabel)
    .Func("SetLabelMarkup",&wxControl::SetLabelMarkup)
    .Func("SetLabel",&wxControl::SetLabel);
    Sqrat::RootTable(vm).Bind(_SC("wxControl"),bwxControl);
#else

    Sqrat::DerivedClass<wxControl,wxWindow,Sqrat::NoConstructor<wxControl> > bwxControl(vm,"wxControl");
    bwxControl.Func("GetLabel",&wxControl::GetLabel)
    .Func("SetLabel",&wxControl::SetLabel);
    Sqrat::RootTable(vm).Bind(_SC("wxControl"),bwxControl);

#endif // wxCHECK_VERSION



    /***************************************************************************************************************/
    // wxTextCtrl
    /***************************************************************************************************************/
    Sqrat::Class<wxTextCtrl,Sqrat::NoConstructor<wxTextCtrl> > bwxTextCtrl(vm,"wxTextCtrl");
    bwxTextCtrl.Func("GetValue",&wxTextCtrl::GetValue)
    .Func("SetValue",&wxTextCtrl::SetValue);
    Sqrat::RootTable(vm).Bind(_SC("wxTextCtrl"),bwxTextCtrl);

#if wxCHECK_VERSION(2, 9, 0)
    /***************************************************************************************************************/
    // wxAnyButton
    /***************************************************************************************************************/

    Sqrat::DerivedClass<wxAnyButtonBase,wxControl,Sqrat::NoConstructor<wxAnyButtonBase> > bwxAnyButtonBase(vm,"wxAnyButtonBase");
    Sqrat::RootTable(vm).Bind(_SC("wxAnyButtonBase"),bwxAnyButtonBase);

    Sqrat::DerivedClass<wxAnyButton,wxAnyButtonBase,Sqrat::NoConstructor<wxAnyButton> > bwxAnyButton(vm,"wxAnyButton");
    Sqrat::RootTable(vm).Bind(_SC("wxAnyButton"),bwxAnyButton);

    Sqrat::DerivedClass<wxButtonBase,wxAnyButton,Sqrat::NoConstructor<wxButtonBase> > bwxButtonBase(vm,"wxButtonBase");
    Sqrat::RootTable(vm).Bind(_SC("wxButtonBase"),bwxButtonBase);
#endif

    /***************************************************************************************************************/
    // wxButton
    /***************************************************************************************************************/
#if wxCHECK_VERSION(2, 9, 0)

    Sqrat::DerivedClass<wxButton,wxButtonBase, Sqrat::NoConstructor<wxButton> > bwxButton(vm,"wxButton");
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
    .SquirrelFunc("Play",&wxAnimationCtrl_Play)
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
    Sqrat::DerivedClass<wxCheckBoxBase,wxControl, Sqrat::NoConstructor<wxCheckBoxBase> > bwxCheckBoxBase(vm,"wxCheckBoxBase");
    Sqrat::RootTable(vm).Bind(_SC("wxCheckBoxBase"),bwxCheckBoxBase);


    Sqrat::DerivedClass<wxCheckBox,wxCheckBoxBase, Sqrat::NoConstructor<wxCheckBox> > bwxCheckBox(vm,"wxCheckBox");
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
    Sqrat::DerivedClass<wxChoiceBase,wxControl, Sqrat::NoConstructor<wxChoiceBase> > bwxChoiceBase(vm,"wxChoiceBase");
    bwxChoiceBase.Func("GetCurrentSelection",&wxChoiceBase::GetCurrentSelection);
    Sqrat::RootTable(vm).Bind(_SC("wxChoiceBase"),bwxChoiceBase);

    Sqrat::DerivedClass<wxChoice,wxChoiceBase, Sqrat::NoConstructor<wxChoice> > bwxChoice(vm,"wxChoice");
    bwxChoice.Func("GetColumns",&wxChoice::GetColumns)
    .Func("GetCurrentSelection",&wxChoice::GetCurrentSelection)
    .Func("GetSelection",&wxChoice::GetSelection)
    .Func("SetColumns",&wxChoice::SetColumns)
    .Func("GetCount",&wxChoice::GetCount)
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
#if defined(__WXGTK__)
    Sqrat::DerivedClass<wxComboBox,wxControl, Sqrat::NoConstructor<wxComboBox> > bwxComboBox(vm,"wxComboBox");
#else
    Sqrat::DerivedClass<wxComboBox,wxChoice, Sqrat::NoConstructor<wxComboBox> > bwxComboBox(vm,"wxComboBox");
#endif

    bwxComboBox
    //.Func("IsListEmpty",&wxComboBox::IsListEmpty)
    //.Func("IsTextEmpty",&wxComboBox::IsTextEmpty)
    //.Func("SetValue",&wxComboBox::SetValue)
#if wxCHECK_VERSION(2, 9, 0)
    .Func("Dismiss",&wxComboBox::Dismiss)
    .Func("Popup",&wxComboBox::Popup)
#endif // wxCHECK_VERSION
    .SquirrelFunc("GetSelection",&wxComboBox_GetSelection)
    .Func("GetValue",&wxComboBox::GetValue)
    .Func("FindString",&wxComboBox::FindString)
    .Func("GetString",&wxComboBox::GetString)
    .Func("SetValue",&wxComboBox::GetString);
    //
    Sqrat::RootTable(vm).Bind(_SC("wxComboBox"),bwxComboBox);

    /***************************************************************************************************************/
    // wxRadioBox
    /***************************************************************************************************************/
    Sqrat::DerivedClass<wxRadioBox,wxControl, Sqrat::NoConstructor<wxRadioBox> > bwxRadioBox(vm,"wxRadioBox");
    bwxRadioBox.Func("FindString",&wxRadioBox::FindString)
    .Func("IsItemEnabled",&wxRadioBox::IsItemEnabled)
    .Func("GetCount",&wxRadioBox::GetCount)
    .Func("SetItemHelpText",&wxRadioBox::SetItemHelpText)
    .Func("SetItemToolTip",&wxRadioBox::SetItemToolTip)
    .Func("GetSelection",&wxRadioBox::GetSelection)
    .SquirrelFunc("Show",&wxRadioBox_Show)
    .Func("SetSelection",&wxRadioBox::SetSelection)
    .SquirrelFunc("Enable",&wxRadioBox_Enable)
    .Func("GetColumnCount",&wxRadioBox::GetColumnCount)
    .Func("GetString",&wxRadioBox::GetString);

    Sqrat::RootTable(vm).Bind(_SC("wxRadioBox"),bwxRadioBox);

    /***************************************************************************************************************/
    // wxGauge
    /***************************************************************************************************************/
    Sqrat::DerivedClass<wxGaugeBase,wxControl, Sqrat::NoConstructor<wxGaugeBase> > bwxGaugeBase(vm,"wxGaugeBase");
    bwxGaugeBase.Func("SetValue",&wxGauge::SetValue);
    Sqrat::RootTable(vm).Bind(_SC("wxGaugeBase"),bwxGaugeBase);

#if defined(__WXGTK__)
    Sqrat::DerivedClass<wxGauge,wxControl, Sqrat::NoConstructor<wxGauge> > bwxGauge(vm,"wxGauge");
#else
    Sqrat::DerivedClass<wxGauge,wxGaugeBase, Sqrat::NoConstructor<wxGauge> > bwxGauge(vm,"wxGauge");
#endif
    bwxGauge.Func("GetRange",&wxGauge::GetRange)
    .Func("GetValue",&wxGauge::GetValue)
    .Func("IsVertical",&wxGauge::IsVertical)
    .Func("Pulse",&wxGauge::Pulse)
    .Func("SetRange",&wxGauge::SetRange)
    .Func("SetValue",&wxGauge::SetValue)
    .Func("SetShadowWidth",&wxGauge::SetShadowWidth)
    .Func("GetShadowWidth",&wxGauge::GetShadowWidth)
    .Func("GetBezelFace",&wxGauge::GetBezelFace)
    .Func("SetBezelFace",&wxGauge::SetBezelFace);

    Sqrat::RootTable(vm).Bind(_SC("wxGauge"),bwxGauge);


    /***************************************************************************************************************/
    // wxHyperlinkCtrl
    /***************************************************************************************************************/
#if wxCHECK_VERSION(2, 9, 0)

    Sqrat::DerivedClass<wxHyperlinkCtrlBase,wxControl,Sqrat::NoConstructor<wxHyperlinkCtrlBase> > bwxHyperlinkCtrlBase(vm,"wxHyperlinkCtrlBase");
    Sqrat::RootTable(vm).Bind(_SC("wxHyperlinkCtrlBase"),bwxHyperlinkCtrlBase);

    Sqrat::DerivedClass<wxGenericHyperlinkCtrl,wxHyperlinkCtrlBase,Sqrat::NoConstructor<wxGenericHyperlinkCtrl> > bwxGenericHyperlinkCtrl(vm,"wxGenericHyperlinkCtrl");
    Sqrat::RootTable(vm).Bind(_SC("wxGenericHyperlinkCtrl"),bwxGenericHyperlinkCtrl);

    Sqrat::DerivedClass<wxHyperlinkCtrl,wxGenericHyperlinkCtrl, Sqrat::NoConstructor<wxHyperlinkCtrl> > bwxHyperlinkCtrl(vm,"wxHyperlinkCtrl");
#else
    Sqrat::DerivedClass<wxHyperlinkCtrl,wxControl, Sqrat::NoConstructor<wxHyperlinkCtrl> > bwxHyperlinkCtrl(vm,"wxHyperlinkCtrl");
#endif // wxCHECK_VERSION
    bwxHyperlinkCtrl.Func("GetURL",&wxHyperlinkCtrl::GetURL)
    .Func("GetVisited",&wxHyperlinkCtrl::GetVisited)
    .Func("SetHoverColour",&wxHyperlinkCtrl::SetHoverColour)
    .Func("SetNormalColour",&wxHyperlinkCtrl::SetNormalColour)
    .Func("SetURL",&wxHyperlinkCtrl::SetURL)
    .Func("SetVisited",&wxHyperlinkCtrl::SetVisited)
    .Func("SetVisitedColour",&wxHyperlinkCtrl::SetVisitedColour);

    Sqrat::RootTable(vm).Bind(_SC("wxHyperlinkCtrl"),bwxHyperlinkCtrl);


    /***************************************************************************************************************/
    // wxRadioButton
    /***************************************************************************************************************/
    Sqrat::DerivedClass<wxRadioButton,wxControl, Sqrat::NoConstructor<wxRadioButton> > bwxRadioButton(vm,"wxRadioButton");
    bwxRadioButton.Func("GetValue",&wxRadioButton::GetValue)
    .Func("SetValue",&wxRadioButton::SetValue);

    Sqrat::RootTable(vm).Bind(_SC("wxRadioButton"),bwxRadioButton);


    /***************************************************************************************************************/
    // wxListBox
    /***************************************************************************************************************/
    Sqrat::DerivedClass<wxListBoxBase,wxControl, Sqrat::NoConstructor<wxListBoxBase> > bwxListBoxBase(vm,"wxListBoxBase");
    bwxListBoxBase.Func<void (wxListBoxBase::*) (int,bool)>("SetSelection",&wxListBoxBase::SetSelection);
    Sqrat::RootTable(vm).Bind(_SC("wxListBoxBase"),bwxListBoxBase);

    Sqrat::DerivedClass<wxListBox,wxListBoxBase, Sqrat::NoConstructor<wxListBox> > bwxListBox(vm,"wxListBox");
    bwxListBox.SquirrelFunc("GetSelections",&wxListBox_GetSelections)
    .Func("GetSelection",&wxListBox::GetSelection)
    .Func<void (wxListBoxBase::*) (int,bool)>("SetSelection",&wxListBox::SetSelection)
    .Func("IsSelected",&wxListBox::IsSelected)
    .Func("EnsureVisible",&wxListBox::EnsureVisible)
    .Func("GetCount",&wxListBox::GetCount)
    .Func("GetString",&wxListBox::GetString)
    .Func("SetString",&wxListBox::SetString)
    .Func("FindString",&wxListBox::FindString)
    .Func("Delete",&wxListBox::Delete)
    .Func("Clear",&wxListBox::Clear)
    .Func<int (wxListBox::*)(const wxString&)>("Append",&wxListBox::Append)
#if wxCHECK_VERSION(2, 9, 0)
    .Func<int (wxListBox::*)(const wxString&, unsigned int)>("Insert",&wxListBox::Insert);
#else
    .Func<void (wxListBox::*)(const wxString&, unsigned int)>("Insert",&wxListBox::Insert);
#endif // wxCHECK_VERSION

    //.Func<void (wxListBox::*)(const wxString&,unsigned int)>("Set",&wxListBox::Set);

    Sqrat::RootTable(vm).Bind(_SC("wxListBox"),bwxListBox);

    /***************************************************************************************************************/
    // wxCheckListBox
    /***************************************************************************************************************/
    Sqrat::DerivedClass<wxCheckListBox,wxListBox, Sqrat::NoConstructor<wxCheckListBox> > bwxCheckListBox(vm,"wxCheckListBox");
    bwxCheckListBox.Func("Check",&wxCheckListBox::Check)
    .Func("IsChecked",&wxCheckListBox::IsChecked)
    .SquirrelFunc("GetCheckedItems",&wxCheckListBox_GetCheckedItems);

    Sqrat::RootTable(vm).Bind(_SC("wxCheckListBox"),bwxCheckListBox);

    /***************************************************************************************************************/
    // wxStaticText
    /***************************************************************************************************************/
    Sqrat::DerivedClass<wxStaticText,wxControl, Sqrat::NoConstructor<wxStaticText> > bwxStaticText(vm,"wxStaticText");
    bwxStaticText.Func("GetLabel",&wxStaticText::GetLabel)
    .Func("SetLabel",&wxStaticText::SetLabel);

    Sqrat::RootTable(vm).Bind(_SC("wxStaticText"),bwxStaticText);

    /***************************************************************************************************************/
    // wxSlider
    /***************************************************************************************************************/
    Sqrat::DerivedClass<wxSliderBase,wxControl, Sqrat::NoConstructor<wxSliderBase> > bwxSliderBase(vm,"wxSliderBase");
    Sqrat::RootTable(vm).Bind(_SC("wxSliderBase"),bwxSliderBase);


    Sqrat::DerivedClass<wxSlider,wxSliderBase, Sqrat::NoConstructor<wxSlider> > bwxSlider(vm,"wxSlider");
    bwxSlider.Func("ClearSel",&wxSlider::ClearSel)
    .Func("ClearTicks",&wxSlider::ClearTicks)
    .Func("GetMax",&wxSlider::GetMax)
    .Func("GetMin",&wxSlider::GetMin)
    .Func("GetSelEnd",&wxSlider::GetSelEnd)
    .Func("GetSelStart",&wxSlider::GetSelStart)
    .Func("GetThumbLength",&wxSlider::GetThumbLength)
    .Func("GetTickFreq",&wxSlider::GetTickFreq)
    .Func("GetValue",&wxSlider::GetValue)
    .Func("SetMin",&wxSlider::SetMin)
    .Func("SetMax",&wxSlider::SetMax)
    .Func("SetRange",&wxSlider::SetRange)
    .Func("SetSelection",&wxSlider::SetSelection)
    .Func("SetTick",&wxSlider::SetTick)
    .SquirrelFunc("SetTickFreq",&wxSlider_SetTickFreq)
    .Func("SetValue",&wxSlider::SetValue);

    Sqrat::RootTable(vm).Bind(_SC("wxSlider"),bwxSlider);


    /***************************************************************************************************************/
    // wxToggleButton
    /***************************************************************************************************************/
    Sqrat::DerivedClass<wxToggleButton,wxControl, Sqrat::NoConstructor<wxToggleButton> > bwxToggleButton(vm,"wxToggleButton");
    bwxToggleButton.Func("GetValue",&wxToggleButton::GetValue)
    .Func("SetValue",&wxToggleButton::SetValue);

    Sqrat::RootTable(vm).Bind(_SC("wxToggleButton"),bwxToggleButton);


    /***************************************************************************************************************/
    // wxTextCtrlBase
    /***************************************************************************************************************/
    Sqrat::DerivedClass<wxTextCtrlBase,wxControl, Sqrat::NoConstructor<wxTextCtrlBase> > bwxTextCtrlBase(vm,"wxTextCtrlBase");
    Sqrat::RootTable(vm).Bind(_SC("wxTextCtrlBase"),bwxTextCtrlBase);

    /***************************************************************************************************************/
    // wxSearchCtrl
    /***************************************************************************************************************/
// TODO (bluehazzard#1#): Check the possibilities to implement this...
/*#if (wxUSE_NATIVE_SEARCH_CONTROL == 1) || wxCHECK_VERSION(2, 9, 0)
    Sqrat::DerivedClass<wxSearchCtrl,wxTextCtrl, Sqrat::NoConstructor<wxSearchCtrl> > bwxSearchCtrl(vm,"wxSearchCtrl");
#else
    Sqrat::DerivedClass<wxSearchCtrl,wxTextCtrlBase, Sqrat::NoConstructor<wxSearchCtrl> > bwxSearchCtrl(vm,"wxSearchCtrl");
#endif
    bwxSearchCtrl.Func("ShowCancelButton",&wxSearchCtrl::ShowCancelButton)
#if wxCHECK_VERSION(2, 9, 0)
    .Func("SetDescriptiveText",&wxSearchCtrl::SetDescriptiveText)
    .Func("GetDescriptiveText",&wxSearchCtrl::GetDescriptiveText)
#endif // wxCHECK_VERSION
    .Func("IsSearchButtonVisible",&wxSearchCtrl::IsSearchButtonVisible)
    .Func("IsCancelButtonVisible",&wxSearchCtrl::IsSearchButtonVisible)
    .Func("ShowSearchButton",&wxSearchCtrl::ShowSearchButton);

    Sqrat::RootTable(vm).Bind(_SC("wxSearchCtrl"),bwxSearchCtrl);*/



    /***************************************************************************************************************/
    // wxPickerBase
    /***************************************************************************************************************/
    Sqrat::DerivedClass<wxPickerBase,wxControl, Sqrat::NoConstructor<wxPickerBase> > bwxPickerBase(vm,"wxPickerBase");
    bwxPickerBase.Func("HasTextCtrl",&wxPickerBase::HasTextCtrl)
    .Func("GetTextCtrl",&wxPickerBase::GetTextCtrl);

    Sqrat::RootTable(vm).Bind(_SC("wxPickerBase"),bwxPickerBase);

    /***************************************************************************************************************/
    // wxColourPickerCtrl
    /***************************************************************************************************************/
    Sqrat::DerivedClass<wxColourPickerCtrl,wxPickerBase, Sqrat::NoConstructor<wxColourPickerCtrl> > bwxColourPickerCtrl(vm,"wxColourPickerCtrl");
    bwxColourPickerCtrl.Func("GetColour",&wxColourPickerCtrl::GetColour)
    .Func<void (wxColourPickerCtrl::*) (const wxColour&)>("SetColour",&wxColourPickerCtrl::SetColour);

    Sqrat::RootTable(vm).Bind(_SC("wxColourPickerCtrl"),bwxColourPickerCtrl);


    /***************************************************************************************************************/
    // wxFontPickerCtrl
    /***************************************************************************************************************/
    /*Sqrat::DerivedClass<wxFontPickerCtrl,wxPickerBase, Sqrat::NoConstructor<wxFontPickerCtrl> > bwxFontPickerCtrl(vm,"wxFontPickerCtrl");
    bwxFontPickerCtrl.Func("GetMaxPointSize",&wxFontPickerCtrl::GetMaxPointSize)
    .Func("GetSelectedColour",&wxFontPickerCtrl::GetSelectedColour)
    .Func("GetSelectedFont",&wxFontPickerCtrl::GetSelectedFont)
    .Func("SetMaxPointSize ",&wxFontPickerCtrl::SetMaxPointSize )
    .Func("SetSelectedColour ",&wxFontPickerCtrl::SetSelectedColour )
    .Func("SetSelectedFont",&wxFontPickerCtrl::SetSelectedFont);

    Sqrat::RootTable(vm).Bind(_SC("wxFontPickerCtrl"),bwxFontPickerCtrl);*/


    /***************************************************************************************************************/
    // wxFileDirPickerCtrlBase
    /***************************************************************************************************************/
    Sqrat::DerivedClass<wxFileDirPickerCtrlBase ,wxPickerBase, Sqrat::NoConstructor<wxFileDirPickerCtrlBase> > bwxFileDirPickerCtrlBase(vm,"wxFileDirPickerCtrlBase");
    bwxFileDirPickerCtrlBase.Func("GetPath",&wxFileDirPickerCtrlBase::GetPath)
    .Func("SetPath",&wxFileDirPickerCtrlBase::SetPath );

    Sqrat::RootTable(vm).Bind(_SC("wxFileDirPickerCtrlBase"),bwxFileDirPickerCtrlBase);

    /***************************************************************************************************************/
    // wxDirPickerCtrl
    /***************************************************************************************************************/
    Sqrat::DerivedClass<wxDirPickerCtrl ,wxFileDirPickerCtrlBase, Sqrat::NoConstructor<wxDirPickerCtrl> > bwxDirPickerCtrl(vm,"wxDirPickerCtrl");
    bwxDirPickerCtrl.Func("SetPath",&wxDirPickerCtrl::SetPath)
    .Func("GetPath",&wxDirPickerCtrl::GetPath);

    Sqrat::RootTable(vm).Bind(_SC("wxDirPickerCtrl"),bwxDirPickerCtrl);

    /***************************************************************************************************************/
    // wxFilePickerCtrl
    /***************************************************************************************************************/
    Sqrat::DerivedClass<wxFilePickerCtrl ,wxFileDirPickerCtrlBase, Sqrat::NoConstructor<wxFilePickerCtrl> > bwxFilePickerCtrl(vm,"wxFilePickerCtrl");
    bwxFilePickerCtrl.Func("SetPath",&wxFilePickerCtrl::SetPath )
    .Func("GetPath",&wxFilePickerCtrl::GetPath);

    Sqrat::RootTable(vm).Bind(_SC("wxFilePickerCtrl"),bwxFilePickerCtrl);



    /***************************************************************************************************************/
    // wxSpinButton
    /***************************************************************************************************************/
    Sqrat::DerivedClass<wxSpinButtonBase ,wxControl, Sqrat::NoConstructor<wxSpinButtonBase> > bwxSpinButtonBase(vm,"wxSpinButtonBase");
    Sqrat::RootTable(vm).Bind(_SC("wxSpinButtonBase"),bwxSpinButtonBase);

    Sqrat::DerivedClass<wxSpinButton,wxSpinButtonBase,Sqrat::NoConstructor<wxSpinButton> > bwxSpinButton(vm,"wxSpinButton");
    bwxSpinButton.Func("GetMax",&wxSpinButton::GetMax)
    .Func("GetMin",&wxSpinButton::GetMin)
    .Func("GetValue",&wxSpinButton::GetValue)
    .Func("SetRange",&wxSpinButton::SetRange)
    .Func("SetValue",&wxSpinButton::SetValue);

    Sqrat::RootTable(vm).Bind(_SC("wxSpinCtrl"),bwxSpinButton);

    /***************************************************************************************************************/
    // wxSpinCtrl
    /***************************************************************************************************************/
 #if defined(__WXMSW__) || (__WXMAC__)
    Sqrat::DerivedClass<wxSpinCtrl,wxSpinButton,Sqrat::NoConstructor<wxSpinCtrl> > bwxSpinCtrl(vm,"wxSpinCtrl");
#else
    Sqrat::DerivedClass<wxSpinCtrl,wxControl,Sqrat::NoConstructor<wxSpinCtrl> > bwxSpinCtrl(vm,"wxSpinCtrl");
#endif
    bwxSpinCtrl.Func("GetMax",&wxSpinCtrl::GetMax)
    .Func("GetMin",&wxSpinCtrl::GetMin)
    .Func("GetValue",&wxSpinCtrl::GetValue)
    .Func("SetRange",&wxSpinCtrl::SetRange)
#if wxCHECK_VERSION(2, 9, 0)
    .Func("GetBase",&wxSpinCtrl::GetBase)
    .Func("SetBase",&wxSpinCtrl::SetBase)
#endif // wxCHECK_VERSION
    .Func("SetSelection",&wxSpinCtrl::SetSelection)
    .SquirrelFunc("SetValue",&wxSpinCtrl_SetValue);

    Sqrat::RootTable(vm).Bind(_SC("wxSpinCtrl"),bwxSpinCtrl);


#if wxCHECK_VERSION(2, 9, 0)

#else
    /***************************************************************************************************************/
    // wxTimerBase
    /***************************************************************************************************************/
    Sqrat::Class<wxTimerBase ,Sqrat::NoConstructor<wxTimerBase> > bwxTimerBase(vm,"wxTimerBase");
    Sqrat::RootTable(vm).Bind(_SC("wxTimerBase"),bwxTimerBase);
#endif // wxCHECK_VERSION


    /***************************************************************************************************************/
    // wxTimer
    /***************************************************************************************************************/

#if wxCHECK_VERSION(2, 9, 0)
    Sqrat::Class<wxTimer,Sqrat::NoConstructor<wxTimer> > bwxTimer(vm,"wxTimer");
#else
    Sqrat::DerivedClass<wxTimer,wxTimerBase,Sqrat::NoConstructor<wxTimer> > bwxTimer(vm,"wxTimer");
#endif // wxCHECK_VERSION
    bwxTimer.Func("GetId",&wxTimer::GetId)
    .Func("IsOneShot",&wxTimer::IsOneShot)
    .Func("IsRunning",&wxTimer::IsRunning)
    .Func("Start",&wxTimer::Start)
    //.Func("StartOnce",&wxTimer::StartOnce)
    .Func("Stop",&wxTimer::Stop)
    .Func("GetInterval",&wxTimer::GetInterval);

    Sqrat::RootTable(vm).Bind(_SC("wxTimer"),bwxTimer);


}
}

