/*
 * This file is part of the Code::Blocks IDE and licensed under the GNU Lesser General Public License, version 3
 * http://www.gnu.org/licenses/lgpl-3.0.html
 *
 * $Revision$
 * $Id$
 * $HeadURL$
 */

#include <sdk_precomp.h>

#ifndef CB_PRECOMP
    #include "scriptingmanager.h"
    #include "cbeditor.h"
    #include "cbexception.h"
    #include "configmanager.h"
    #include "editormanager.h"
    #include "globals.h"
    #include "logmanager.h"
    #include "manager.h"

    #include <settings.h>
    #include <wx/msgdlg.h>
    #include <wx/file.h>
    #include <wx/filename.h>
    #include <wx/regex.h>
#endif

#include "crc32.h"
#include "menuitemsmanager.h"
#include "genericmultilinenotesdlg.h"
// FIXME (bluehazzard#1#): remove if not needed
//#include "sqplus.h"
#include "sqrat.h"
#include "scriptbindings.h"
#include "sc_plugin.h"
#include "sqstdstring.h"
//#include <sc_cb_vm.h>

template<> ScriptingManager* Mgr<ScriptingManager>::instance = nullptr;
template<> bool  Mgr<ScriptingManager>::isShutdown = false;

static wxString s_ScriptErrors;
static wxString capture;


void PrintSquirrelToWxString(wxString &msg,const SQChar * s, va_list& vl)
{
    int buffer_size = 2048;
    SQChar *tmp_buffer;
    for(;;buffer_size*=2)
    {
        // TODO (bluehazzard#1#): Check if this is UNICODE UTF8 safe
        tmp_buffer = new SQChar [buffer_size];
        int retvalue = vsnprintf(tmp_buffer,buffer_size,s,vl);
        if(retvalue < buffer_size)
        {
            // Buffersize was large enough
            msg = cbC2U(tmp_buffer);
            delete[] tmp_buffer;
            break;
        }
        // Buffer size was not enough
        delete[] tmp_buffer;
    }
}

static void ScriptsPrintFunc(HSQUIRRELVM /*v*/, const SQChar * s, ...)
{
    va_list vl;
    va_start(vl,s);
    wxString msg;
    PrintSquirrelToWxString(msg,s,vl);
    va_end(vl);

    s_ScriptErrors << msg;
}

static void CaptureScriptOutput(HSQUIRRELVM /*v*/, const SQChar * s, ...)
{
    va_list vl;
    va_start(vl,s);
    wxString msg;
    PrintSquirrelToWxString(msg,s,vl);
    ::capture.append(msg);
    va_end(vl);
}

static void CaptureScriptErrors(HSQUIRRELVM /*v*/, const SQChar * s, ...)
{
    va_list vl;
    va_start(vl,s);
    wxString msg;
    PrintSquirrelToWxString(msg,s,vl);
    va_end(vl);

    s_ScriptErrors << msg;
    Manager::Get()->GetLogManager()->LogError(_("Script error: ") + msg);
}

BEGIN_EVENT_TABLE(ScriptingManager, wxEvtHandler)
//
END_EVENT_TABLE()

ScriptingManager::ScriptingManager()
    : m_AttachedToMainWindow(false),
    m_MenuItemsManager(false) // not auto-clear
{
    //ctor

    // initialize but don't load the IO lib
    //SquirrelVM::Init((SquirrelInitFlags)(sqifAll & ~sqifIO));
    m_vm = new ScriptBindings::CBsquirrelVM(1024,(ScriptBindings::VM_LIB_ALL & ~ScriptBindings::VM_LIB_IO));

    if (!m_vm->GetVM())
        cbThrow(_T("Can't create scripting engine!"));

    // TODO (bluehazzard#1#): Errors should not be passed to the std output...
    m_vm->SetPrintFunc(ScriptsPrintFunc,CaptureScriptErrors);
    m_vm->SetMeDefault();
    RefreshTrusts();

    // register types
    ScriptBindings::RegisterBindings(m_vm->GetVM());
}

ScriptingManager::~ScriptingManager()
{
    //dtor
    // save trusted scripts set
    ConfigManagerContainer::StringToStringMap myMap;
    int i = 0;
    TrustedScripts::iterator it;
    for (it = m_TrustedScripts.begin(); it != m_TrustedScripts.end(); ++it)
    {
        if (!it->second.permanent)
            continue;
        wxString key = wxString::Format(_T("trust%d"), i++);
        wxString value = wxString::Format(_T("%s?%x"), it->first.c_str(), it->second.crc);
        myMap.insert(myMap.end(), std::make_pair(key, value));
    }
    Manager::Get()->GetConfigManager(_T("security"))->Write(_T("/trusted_scripts"), myMap);

    //SquirrelVM::Shutdown();
}

void ScriptingManager::RegisterScriptFunctions()
{
    // done in scriptbindings.cpp
}

bool ScriptingManager::LoadScript(const wxString& filename)
{
//    wxCriticalSectionLocker c(cs);

    wxLogNull ln; // own error checking implemented -> avoid debug warnings

    wxString fname(filename);
    wxFile f(fname); // try to open
    if (!f.IsOpened())
    {
        bool found = false;

        // check in same dir as currently running script (if any)
        if (!m_CurrentlyRunningScriptFile.IsEmpty())
        {
            fname = wxFileName(m_CurrentlyRunningScriptFile).GetPath() + _T('/') + filename;
            f.Open(fname);
            found = f.IsOpened();
        }

        if (!found)
        {
            // check in standard script dirs
            fname = ConfigManager::LocateDataFile(filename, sdScriptsUser | sdScriptsGlobal);
            f.Open(fname);
            if (!f.IsOpened())
            {
                Manager::Get()->GetLogManager()->DebugLog(_T("Can't open script ") + filename);
                return false;
            }
        }
    }
    // read file
    wxString contents = cbReadFileContents(f);
    m_CurrentlyRunningScriptFile = fname;
    bool ret = LoadBuffer(contents, fname);
    m_CurrentlyRunningScriptFile.Clear();
    return ret;
}

bool ScriptingManager::LoadBuffer(const wxString& buffer, const wxString& debugName)
{
    // includes guard to avoid recursion
    wxString incName = UnixFilename(debugName);
    if (m_IncludeSet.find(incName) != m_IncludeSet.end())
    {
        Manager::Get()->GetLogManager()->LogWarning(F(_T("Ignoring Include(\"%s\") because it would cause recursion..."), incName.wx_str()));
        return true;
    }
    m_IncludeSet.insert(incName);

//    wxCriticalSectionLocker c(cs);

    s_ScriptErrors.Clear();

    // TODO (bluehazzard#1#): THIS IS FUCKING UGLY
    //ScriptBindings::CBsquirrelVM *sq_vm = ScriptBindings::CBsquirrelVMManager::Get()->GetVM(Sqrat::DefaultVM::Get());

    // compile script

    ScriptBindings::CBsquirrelVM::SC_ERROR_STATE ret = m_vm->doString(buffer);
    if(ret == ScriptBindings::CBsquirrelVM::SC_COMPILE_ERROR)
    {
        // A compiling error occurred
        wxString err_msg;
        err_msg << _T("Filename: ") << debugName << _("\nError:") << m_vm->getLastErrorMsg() << _("\nDetails:") <<  s_ScriptErrors;
        cbMessageBox(err_msg, _("Script compile error"), wxICON_ERROR);
        m_IncludeSet.erase(incName);
        return false;
    } else if(ret == ScriptBindings::CBsquirrelVM::SC_RUNTIME_ERROR)
    {
        // A runtime Error occurred
        wxString err_msg;
        err_msg << _T("Filename: ") << debugName << _("\nError:") << m_vm->getLastErrorMsg() << _("\nDetails:") <<  s_ScriptErrors;
        cbMessageBox(err_msg, _("Script run error"), wxICON_ERROR);
        m_IncludeSet.erase(incName);
        return false;
    }

    m_IncludeSet.erase(incName);
    return true;
}


wxString ScriptingManager::LoadBufferRedirectOutput(const wxString& buffer)
{
//    wxCriticalSectionLocker c(cs);

    s_ScriptErrors.Clear();
    ::capture.Clear();

    // FIXME (bluehazzard#1#): Here is a absolute mess with the error handling...

    //ScriptBindings::CBsquirrelVM *sq_vm = ScriptBindings::CBsquirrelVMManager::Get()->GetVM(Sqrat::DefaultVM::Get());

    m_vm->SetPrintFunc(CaptureScriptOutput,CaptureScriptErrors);

    //sq_setprintfunc(SquirrelVM::GetVMPtr(), CaptureScriptOutput);
    bool res = LoadBuffer(buffer);
    //sq_setprintfunc(SquirrelVM::GetVMPtr(), ScriptsPrintFunc);

    return res ? ::capture : (wxString) wxEmptyString;
}

/*wxString ScriptingManager::GetErrorString(Sqrat::Exception* exception, bool clearErrors)
{
    wxString msg;
    if (exception)
        msg.FromUTF8("%s",exception.Message());
    msg << s_ScriptErrors;

    if (clearErrors)
        s_ScriptErrors.Clear();

    return msg;
}*/

wxString ScriptingManager::GetErrorString( bool clearErrors)
{
    ScriptBindings::StackHandler sa(m_vm->GetVM());
    return sa.GetError(clearErrors);
}

bool ScriptingManager::DisplayErrors(wxString error_msg, bool clearErrors)
{

    if(error_msg == wxEmptyString)
        error_msg = GetErrorString(clearErrors);

    if (!error_msg.IsEmpty())
    {
        if (cbMessageBox(_("Script errors have occured...\nPress 'Yes' to see the exact errors."),
                            _("Script errors"),
                            wxICON_ERROR | wxYES_NO | wxNO_DEFAULT) == wxID_YES)
        {
            GenericMultiLineNotesDlg dlg(Manager::Get()->GetAppWindow(),
                                        _("Script errors"),
                                        error_msg,
                                        true);
            dlg.ShowModal();
        }
        return true;
    }
    return false;
}

void ScriptingManager::InjectScriptOutput(const wxString& output)
{
    s_ScriptErrors << output;
}

int ScriptingManager::Configure()
{
    return -1;
}

bool ScriptingManager::RegisterScriptPlugin(const wxString& /*name*/, const wxArrayInt& ids)
{
    // attach this event handler in the main window (one-time run)
    if (!m_AttachedToMainWindow)
    {
        Manager::Get()->GetAppWindow()->PushEventHandler(this);
        m_AttachedToMainWindow = true;
    }

    for (size_t i = 0; i < ids.GetCount(); ++i)
    {
        Connect(ids[i], -1, wxEVT_COMMAND_MENU_SELECTED,
                (wxObjectEventFunction) (wxEventFunction) (wxCommandEventFunction)
                &ScriptingManager::OnScriptPluginMenu);
    }
    return true;
}

bool ScriptingManager::RegisterScriptMenu(const wxString& menuPath, const wxString& scriptOrFunc, bool isFunction)
{
    // attach this event handler in the main window (one-time run)
    if (!m_AttachedToMainWindow)
    {
        Manager::Get()->GetAppWindow()->PushEventHandler(this);
        m_AttachedToMainWindow = true;
    }

    int id = wxNewId();
    id = m_MenuItemsManager.CreateFromString(menuPath, id);
    wxMenuItem* item = Manager::Get()->GetAppFrame()->GetMenuBar()->FindItem(id);
    if (item)
    {
        if (!isFunction)
            item->SetHelp(_("Press SHIFT while clicking this menu item to edit the assigned script in the editor"));

        Connect(id, -1, wxEVT_COMMAND_MENU_SELECTED,
                (wxObjectEventFunction) (wxEventFunction) (wxCommandEventFunction)
                &ScriptingManager::OnScriptMenu);

        MenuBoundScript mbs;
        mbs.scriptOrFunc = scriptOrFunc;
        mbs.isFunc = isFunction;
        m_MenuIDToScript.insert(m_MenuIDToScript.end(), std::make_pair(id, mbs));
        #if wxCHECK_VERSION(2, 9, 0)
        Manager::Get()->GetLogManager()->Log(F(_("Script/function '%s' registered under menu '%s'"), scriptOrFunc.wx_str(), menuPath.wx_str()));
        #else
        Manager::Get()->GetLogManager()->Log(F(_("Script/function '%s' registered under menu '%s'"), scriptOrFunc.c_str(), menuPath.c_str()));
        #endif

        return true;
    }

    Manager::Get()->GetLogManager()->Log(_("Error registering script menu: ") + menuPath);
    return false;
}

bool ScriptingManager::UnRegisterScriptMenu(cb_unused const wxString& menuPath)
{
    // TODO: not implemented
    Manager::Get()->GetLogManager()->DebugLog(_T("ScriptingManager::UnRegisterScriptMenu() not implemented"));
    return false;
}

bool ScriptingManager::UnRegisterAllScriptMenus()
{
    m_MenuItemsManager.Clear();
    return true;
}

bool ScriptingManager::IsScriptTrusted(const wxString& script)
{
    TrustedScripts::iterator it = m_TrustedScripts.find(script);
    if (it == m_TrustedScripts.end())
        return false;
    // check the crc too
    wxUint32 crc = wxCrc32::FromFile(script);
    if (crc == it->second.crc)
        return true;
    cbMessageBox(script + _T("\n\n") + _("The script was marked as \"trusted\" but it has been modified "
                    "since then.\nScript not trusted anymore."),
                _("Warning"), wxICON_WARNING);
    m_TrustedScripts.erase(it);
    return false;
}

bool ScriptingManager::IsCurrentlyRunningScriptTrusted()
{
    return IsScriptTrusted(m_CurrentlyRunningScriptFile);
}

void ScriptingManager::TrustScript(const wxString& script, bool permanently)
{
    // TODO: what should happen when script is empty()?

    TrustedScripts::iterator it = m_TrustedScripts.find(script);
    if (it != m_TrustedScripts.end())
    {
        // already trusted, remove it from the trusts (we recreate the trust below)
        m_TrustedScripts.erase(it);
    }

    TrustedScriptProps props;
    props.permanent = permanently;
    props.crc = wxCrc32::FromFile(script);

    m_TrustedScripts.insert(m_TrustedScripts.end(), std::make_pair(script, props));
}

void ScriptingManager::TrustCurrentlyRunningScript(bool permanently)
{
    TrustScript(m_CurrentlyRunningScriptFile, permanently);
}

bool ScriptingManager::RemoveTrust(const wxString& script)
{
    TrustedScripts::iterator it = m_TrustedScripts.find(script);
    if (it != m_TrustedScripts.end())
    {
        // already trusted, remove it from the trusts (we recreate the trust below)
        m_TrustedScripts.erase(it);
        return true;
    }
    return false;
}

void ScriptingManager::RefreshTrusts()
{
    // reload trusted scripts set
    m_TrustedScripts.clear();
    ConfigManagerContainer::StringToStringMap myMap;
    Manager::Get()->GetConfigManager(_T("security"))->Read(_T("/trusted_scripts"), &myMap);
    ConfigManagerContainer::StringToStringMap::iterator it;
    for (it = myMap.begin(); it != myMap.end(); ++it)
    {
        wxString key = it->second.BeforeFirst(_T('?'));
        wxString value = it->second.AfterFirst(_T('?'));

        TrustedScriptProps props;
        props.permanent = true;
        unsigned long tmp;
        value.ToULong(&tmp, 16);
        props.crc = tmp;
        m_TrustedScripts.insert(m_TrustedScripts.end(), std::make_pair(key, props));
    }
}

const ScriptingManager::TrustedScripts& ScriptingManager::GetTrustedScripts()
{
    return m_TrustedScripts;
}

void ScriptingManager::OnScriptMenu(wxCommandEvent& event)
{
    MenuIDToScript::iterator it = m_MenuIDToScript.find(event.GetId());
    if (it == m_MenuIDToScript.end())
    {
        cbMessageBox(_("No script associated with this menu?!?"), _("Error"), wxICON_ERROR);
        return;
    }

    MenuBoundScript& mbs = it->second;

    // is it a function?
    if (mbs.isFunc)
    {

        Sqrat::Function call_back(Sqrat::RootTable(),mbs.scriptOrFunc.ToUTF8());
        if (call_back.IsNull())
            return;
        call_back();
        // FIXME (bluehazzard#1#): Check the vm...
        ScriptBindings::StackHandler sa(m_vm->GetVM());
        if(sa.HasError())
        {
            DisplayErrors(sa.GetError());
        }
        return;
    }

    // script loading below

    if (wxGetKeyState(WXK_SHIFT))
    {
        wxString script = ConfigManager::LocateDataFile(mbs.scriptOrFunc, sdScriptsUser | sdScriptsGlobal);
        Manager::Get()->GetEditorManager()->Open(script);
        return;
    }


    // run script
    //try
    //{
        if (!LoadScript(mbs.scriptOrFunc))
            cbMessageBox(_("Could not run script: ") + mbs.scriptOrFunc, _("Error"), wxICON_ERROR);
    //}
    //catch (SquirrelError exception)
    //{
    //    DisplayErrors(&exception);
    //}
}

void ScriptingManager::OnScriptPluginMenu(wxCommandEvent& event)
{
    ScriptBindings::ScriptPluginWrapper::OnScriptMenu(event.GetId());
}
