/*
 * This file is part of the Code::Blocks IDE and licensed under the GNU Lesser General Public License, version 3
 * http://www.gnu.org/licenses/lgpl-3.0.html
 *
 * $Revision$
 * $Id$
 * $HeadURL$
 */

 #include <squirrel.h>
 #include <sqrat.h>
 #include <sq_wx/sq_wx.h>
 #include <wx/numdlg.h>
 #include <wx/colordlg.h>
 #include <wx/textdlg.h>
 #include <wx/filedlg.h>
 #include <sc_cb_vm.h>
 #include <sc_binding_util.h>


/** \defgroup sq_dialogs Squirrel User dialogs
 *  \ingroup Squirrel
 *  \brief Useful dialogs for interacting with user
 */

namespace ScriptBindings
{
    SQInteger wx_GetColourFromUser(HSQUIRRELVM vm)
    {
        StackHandler sa(vm);
        const wxColour& c = sa.GetParamCount() == 2 ? *sa.GetInstance<wxColour>(2) : *wxBLACK;
        sa.PushInstanceCopy<wxColour>(wxGetColourFromUser(Manager::Get()->GetAppWindow(),c));
        return SC_RETURN_VALUE;
    }

    long wx_GetNumberFromUser(const wxString& message, const wxString& prompt, const wxString& caption, long value)
    {
        return wxGetNumberFromUser(message, prompt, caption, value);
    }
    wxString wx_GetPasswordFromUser(const wxString& message, const wxString& caption, const wxString& default_value)
    {
        return wxGetPasswordFromUser(message, caption, default_value);
    }
    wxString wx_GetTextFromUser(const wxString& message, const wxString& caption, const wxString& default_value)
    {
        return wxGetTextFromUser(message, caption, default_value);
    }
    wxArrayString wx_GetFileFromUser(const wxString& message, const wxString& caption, const wxString& default_value,const wxString& wildcart,long style)
    {
        wxFileDialog* filedlg;
        filedlg = new wxFileDialog(nullptr,message,default_value,wxEmptyString,wildcart,style);
        wxArrayString ret;
        if(filedlg->ShowModal() == wxID_OK)
        {
            filedlg->GetPaths(ret);
        }
        return ret;
    }


    void bind_wx_util_dialogs(HSQUIRRELVM vm)
    {
        /** \ingroup sq_dialogs
         *### wxGetColourFromUser(value)
         *
         *  - __value__     A default value [wxColor]
         *
         *  This function displays an dialog  with the possibility to ask a color from the user
         *
         *  - __return__ The Value entered by the user or _value_
         */
        /** \ingroup sq_dialogs
         *### wxGetNumberFromUser(message,prompt,caption,value)
         *
         *  - __message__   Message to inform the user [wxString]
         *  - __prompt__    ... [wxString]
         *  - __caption__   The caption for the Dialog [wxString]
         *  - __value__     A default value [int]
         *
         *  This function displays an dialog  with the possibility to ask a number from the user
         *
         *  - __return__ The Value entered by the user or _value_
         */

        /** \ingroup sq_dialogs
         *### wxGetPasswordFromUser(message,prompt,caption,value)
         *
         *  - __message__   Message to inform the user [wxString]
         *  - __prompt__    ... [wxString]
         *  - __caption__   The caption for the Dialog [wxString]
         *  - __value__     A default value [wxString]
         *
         *  This function displays an dialog  with the possibility to ask for a password
         *
         *  - __return__ The Value entered by the user or _value_
         */

        /** \ingroup sq_dialogs
         *### wxGetTextFromUser(key,default_value)
         *
         *  - __message__   Message to inform the user [wxString]
         *  - __prompt__    ... [wxString]
         *  - __caption__   The caption for the Dialog [wxString]
         *  - __value__     A default value [wxString]
         *
         *  This function displays an dialog  with the possibility to ask for a text from the user
         *
         *  - __return__ The Value entered by the user or _value_
         */

         /** \ingroup sq_dialogs
         *### wxGetFileFromUser(message, caption, default_value,wildcart,style)
         *
         *  - __message__        Message to inform the user [wxString]
         *  - __caption__        ... [wxString]
         *  - __default_value__  The Default file/path [wxString]
         *  - __wildcart__       The wildcart to filter the files [wxString]
         *  - __style__          The styles: wxFD_DEFAULT_STYLE, wxFD_OPEN, wxFD_SAVE, wxFD_OVERWRITE_PROMPT, wxFD_FILE_MUST_EXIST, wxFD_MULTIPLE, wxFD_CHANGE_DIR, wxFD_PREVIEW
         *
         *
         *  This function displays an dialog  with the possibility to ask for one or multiple files to be saved or loaded
         *
         *  - __return__ A wxArrayString with the given files (empty if the user canceled the dialog)
         */


        Sqrat::RootTable()
        .SquirrelFunc(_SC("wxGetColourFromUser"),&wx_GetColourFromUser)
        .Func(_SC("wxGetNumberFromUser"),&wx_GetNumberFromUser)
        .Func(_SC("wxGetPasswordFromUser"),&wx_GetPasswordFromUser)
        .Func(_SC("wxGetTextFromUser"),&wx_GetTextFromUser)
        .Func(_SC("wxGetFileFromUser"),&wx_GetFileFromUser);
    }
}
