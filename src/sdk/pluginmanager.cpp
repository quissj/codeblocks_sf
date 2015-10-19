/*
 * This file is part of the Code::Blocks IDE and licensed under the GNU Lesser General Public License, version 3
 * http://www.gnu.org/licenses/lgpl-3.0.html
 *
 * $Revision$
 * $Id$
 * $HeadURL$
 */

#include "sdk_precomp.h"

#ifndef CB_PRECOMP
#include <wx/dir.h>
#include <wx/filesys.h>
#include <wx/intl.h>
#include <wx/menu.h>
#include <wx/string.h>

#include "pluginmanager.h"
#include "cbexception.h"
#include "cbplugin.h"
#include "infowindow.h"
#include "logmanager.h"
#include "macrosmanager.h"
#include "manager.h"
#include "editormanager.h"
#include "configmanager.h"
#include "personalitymanager.h"
#include "scriptingmanager.h"
#include "globals.h"
#include "sdk_events.h"
#endif

#include <wx/dynlib.h>
#include <wx/filesys.h>
#include <wx/progdlg.h>
#include <wx/utils.h>
#include <wx/filename.h>

#include <wx/wfstream.h>
#include <wx/zipstrm.h>
#include <wx/txtstrm.h>

#include "filefilters.h"
#include "tinyxml/tinyxml.h"

#include "annoyingdialog.h"
#include "pluginsconfigurationdlg.h"

#include "scripting/bindings/sc_plugin.h"

template<> PluginManager* Mgr<PluginManager>::instance = nullptr;
template<> bool  Mgr<PluginManager>::isShutdown = false;

inline void VersionStringToNumbers(const wxString& version, long* major, long* minor, long* release)
{
    wxString majorS = version.BeforeFirst(_T('.')); // 6.3.2 -> 6
    wxString minorS = version.AfterFirst(_T('.')); // 6.3.2 -> 3.2
    wxString releaseS = version.AfterLast(_T('.')); // 6.3.2 -> 2
    minorS = minorS.BeforeFirst(_T('.')); // 3.2 -> 3
    if (major)
        majorS.ToLong(major);
    if (minor)
        minorS.ToLong(minor);
    if (release)
        releaseS.ToLong(release);
}

// returns -1 if new is less then old, 0 if equal and 1 if new is greater than old
inline int CompareVersions(const wxString& new_version, const wxString& old_version)
{
    long new_major, new_minor, new_release;
    long old_major, old_minor, old_release;

    VersionStringToNumbers(new_version, &new_major, &new_minor, &new_release);
    VersionStringToNumbers(old_version, &old_major, &old_minor, &old_release);

#define SIGN(a) (a>0?1:(a<0?-1:0))
    int result = 0;
    result += SIGN(new_major - old_major) << 2;
    result += SIGN(new_minor - old_minor) << 1;
    result += SIGN(new_release - old_release) << 0;
#undef SIGN

    if (result < 0) return -1;
    else if (result > 0) return 1;
    return 0;
}

namespace LibLoader
{
struct RefCountedLib
{
    RefCountedLib() : lib(nullptr), ref(0) {}
    wxDynamicLibrary* lib;
    int ref;
};
typedef std::map<wxString, RefCountedLib> Libs;
Libs s_Libs;

inline wxDynamicLibrary* LoadLibrary(const wxString& filename)
{
    Libs::iterator it = s_Libs.find(filename);
    if (it != s_Libs.end())
    {
        // existing lib./codeblocks
        it->second.ref++;
        return it->second.lib;
    }
    // new lib
    it = s_Libs.insert(s_Libs.end(), std::make_pair(filename, RefCountedLib()));
    it->second.lib = new wxDynamicLibrary;
    it->second.ref = 1;
    it->second.lib->Load(filename);
    return it->second.lib;
}

inline void RemoveLibrary(wxDynamicLibrary* lib)
{
    Libs::iterator it;
    for (it = s_Libs.begin(); it != s_Libs.end(); ++it)
    {
        RefCountedLib& rcl = it->second;
        if (rcl.lib == lib)
        {
            // found
            rcl.ref--;
            if (rcl.ref == 0)
            {
                // only delete the lib if not shutting down
                // if we are shutting down, it will be deleted automatically
                if (!Manager::IsAppShuttingDown())
                    delete rcl.lib;
                s_Libs.erase(it);
            }
            return;
        }
    }
    // if we reached here, it's a lib that was not handled by us
    // (or had wrong refcounting)
}

inline void Cleanup()
{
    Libs::iterator it;
    for (it = s_Libs.begin(); it != s_Libs.end(); ++it)
    {
        RefCountedLib& rcl = it->second;
        // only delete the lib if not shutting down
        // if we are shutting down, it will be deleted automatically
        if (!Manager::IsAppShuttingDown())
            delete rcl.lib;
    }
    s_Libs.clear();
}
};

//static
bool PluginManager::s_SafeMode = false;

BEGIN_EVENT_TABLE(PluginManager, wxEvtHandler)
//
END_EVENT_TABLE()

// class constructor
PluginManager::PluginManager()
    : m_pCurrentlyLoadingLib(nullptr),
      m_pCurrentlyLoadingManifestDoc(nullptr)
{
    Manager::Get()->GetAppWindow()->PushEventHandler(this);
}

// class destructor
PluginManager::~PluginManager()
{
    UnloadAllPlugins();
}

void PluginManager::CreateMenu(cb_unused wxMenuBar* menuBar)
{
}

void PluginManager::ReleaseMenu(cb_unused wxMenuBar* menuBar)
{
}

bool PluginManager::AttachPlugin(cbPlugin* plugin, bool ignoreSafeMode)
{
    if (!plugin)
        return false;
    if (plugin->IsAttached())
        return true;

    if (!s_SafeMode || ignoreSafeMode)
        plugin->Attach();
    return true;
}

bool PluginManager::DetachPlugin(cbPlugin* plugin)
{
    if (!plugin)
        return false;
    if (!plugin->IsAttached())
        return true;

    Manager::Get()->RemoveAllEventSinksFor(plugin);
    plugin->Release(Manager::IsAppShuttingDown());
    return true;
}

bool PluginManager::InstallScriptPlugin(const wxString& actualName,InstallInfo& info, bool forAllUsers, bool askForConfirmation)
{
    if(actualName.IsEmpty())
    {
        Manager::Get()->GetLogManager()->LogWarning(_T("PluginManager::InstallScriptPlugin: pluginName.IsEmpty()"));
        return false;
    }


    // Now we can start to install the plugin
    Manager::Get()->GetLogManager()->DebugLog(wxString(_T("Install Plugin:")) + info.name );


    wxString pluginDir;
    wxString basename = info.name;
    if (forAllUsers)
    {
        pluginDir = ConfigManager::GetFolder(sdPluginsGlobal);
    }
    else
    {
        pluginDir = ConfigManager::GetFolder(sdPluginsUser);
    }

    wxString localName = basename + wxString(_(".splugin"));

    wxString pluginFilename = UnixFilename(pluginDir + _T('/') + localName);

    pd.Update(1, _T("Load install script"));
    Manager::Get()->GetLogManager()->DebugLog(_T("Load install script"));

    // extract plugin from bundle
    if (!ExtractFile(actualName,
                     localName,
                     pluginFilename))
    {
        return false;
    }

    return true;

}

bool PluginManager::UninstallScriptPlugin(const wxString& pluginName, bool removeFiles)
{
    ScriptBindings::cbScriptPlugin* plugin = Manager::Get()->GetScriptingManager()->GetPlugin(pluginName);
    if(plugin == nullptr)
    {
        // plugin not found
        return false;
    }
    wxString pluginFilename = plugin->GetScriptFile();


    wxFileName fname(pluginFilename);
    wxString resourceFilename = fname.GetName() + _T(".zip");
    wxString settingsOnFilename = fname.GetName() + _T(".png");
    wxString settingsOffFilename = fname.GetName() + _T("-off.png");
    resourceFilename = ConfigManager::LocateDataFile(resourceFilename, sdDataGlobal | sdDataUser);
    settingsOnFilename = ConfigManager::LocateDataFile(_T("images/settings/") + settingsOnFilename, sdDataGlobal | sdDataUser);
    settingsOffFilename = ConfigManager::LocateDataFile(_T("images/settings/") + settingsOffFilename, sdDataGlobal | sdDataUser);

    wxArrayString extraFiles;
    ReadExtraFilesFromManifestFile(pluginName,extraFiles);
    for (size_t n = 0; n < extraFiles.GetCount(); ++n)
    {
        extraFiles[n] = ConfigManager::LocateDataFile(extraFiles[n], sdDataGlobal | sdDataUser);
    }
    // Check the rights
    if (wxFileExists(pluginFilename) && !wxFile::Access(pluginFilename, wxFile::write))
    {
        // no write-access; abort
        cbMessageBox(_("You don't have the needed privileges to uninstall this plugin.\n"
                       "Ask your administrator to uninstall this plugin for you..."),
                     _("Warning"), wxICON_WARNING);
        return false;
    }

    wxProgressDialog pd(wxString::Format(_("Uninstalling %s"), pluginName.c_str()),
                        _T("A description wide enough for the dialog ;)"), 3);

    Manager::Get()->GetScriptingManager()->UnRegisterScriptPlugin(pluginName);

    if (!removeFiles)
        return true;


    if (!pluginFilename.IsEmpty())
    {
        // Check if the file is in the plugins directory of c::b
        // if not ask the user if we should delete it
        wxString global_plugin_folder = ConfigManager::GetFolder(sdPluginsGlobal);
        wxString local_plugin_folder = ConfigManager::GetFolder(sdPluginsUser);
        if( !(pluginFilename.StartsWith(global_plugin_folder) ||
            pluginFilename.StartsWith(local_plugin_folder)))
        {
            // The plugin is not in the specific plugins folder, so we ask the user if should remove it
            if(wxMessageBox(_T("The plugin you want to uninstall does not seems to be in a Code::Blocks plugins folder.\nShould the file be deleted?"),
                         _T("Should the plugin file be deleted?"),
                         wxYES_NO|wxICON_INFORMATION) == wxNO)
            {
                // The user don't want that we delete the file, so we have
                // nothing to do here
                return true;
            }
        }


        if (wxRemoveFile(pluginFilename))
        {
//            Manager::Get()->GetLogManager()->DebugLog(F(_T("Plugin file removed")));
            if (!resourceFilename.IsEmpty())
            {
                if (!wxRemoveFile(resourceFilename))
                    Manager::Get()->GetLogManager()->LogWarning(_T("Failed to remove plugin resources: ") + resourceFilename);
            }
            if (!settingsOnFilename.IsEmpty() && wxFileExists(settingsOnFilename))
            {
                if (!wxRemoveFile(settingsOnFilename))
                    Manager::Get()->GetLogManager()->LogWarning(_T("Failed to remove icon for \"Settings\" dialog: ") + settingsOnFilename);
            }
            if (!settingsOffFilename.IsEmpty() && wxFileExists(settingsOffFilename))
            {
                if (!wxRemoveFile(settingsOffFilename))
                    Manager::Get()->GetLogManager()->LogWarning(_T("Failed to remove icon for \"Settings\" dialog: ") + settingsOffFilename);
            }
            for (size_t i = 0; i < extraFiles.GetCount(); ++i)
            {
                if (!extraFiles[i].IsEmpty() && wxFileExists(extraFiles[i]))
                {
                    if (!wxRemoveFile(extraFiles[i]))
                        Manager::Get()->GetLogManager()->LogWarning(_T("Failed to remove extra file: ") + extraFiles[i]);
                }
            }
            return true;
        }
        else
        {
            Manager::Get()->GetLogManager()->LogWarning(_T("Failed to remove plugin file: ") + pluginFilename);
            cbMessageBox(_("Plugin could not be completely uninstalled because its files could not be removed.\n\n"
                           "This can happen if the plugin's file is in-use like, for "
                           "example, when the same plugin file provides more than one "
                           "plugin.\n"
                           "In this case either uninstall all other plugins "
                           "which are provided by the same file, or remove it yourself "
                           "(manually) when you shut down Code::Blocks.\n"
                           "The files that could not be deleted are:\n\n") +
                         pluginFilename + _T('\n') +
                         resourceFilename + _T('\n') +
                         settingsOnFilename + _T('\n') +
                         settingsOffFilename,
                         _("Warning"), wxICON_WARNING);
            return false;
        }
    }
    return false;

}



/* File structure of a .cbplugin archive:
* plugin_name.cbplugin     --
* plugin_name.cbplugin/plugin_name.dll     -- plugin binary in a binary plugin
* plugin_name.cbplugin/plugin_name.splugin -- plugin script in a script plugin
* plugin_name.cbplugin/plugin_name.zip     -- plugin resources
* plugin_name.cbplugin/plugin_name.zip/manifest.xml                            -- plugin manifest file
* plugin_name.cbplugin/plugin_name.zip/install/install.xml                     -- plugin install information file (needed for uninstall)
* plugin_name.cbplugin/plugin_name.zip/install/(*files needed by install.xml  )-- all resource files needed by the install process
* plugin_name.cbplugin/plugin_name.zip(*files needed by plugin  )              -- all resource files needed by the plugin
*/

bool PluginManager::InstallPlugin(const wxString& pluginName, bool forAllUsers, bool askForConfirmation)
{
    if (pluginName.IsEmpty())
    {
        Manager::Get()->GetLogManager()->LogWarning(_T("luginManager::InstallPlugin: Plugin name is empty"));
        return false;
    }

    // pluginName is the name with path to the .cbplugin File


    // The plugin name can probably contain GET_USER_DATA_DIR etc.
    wxString actualName = pluginName;
    Manager::Get()->GetMacrosManager()->ReplaceMacros(actualName);
    // actualName is the path to the .plugin file, cleared from all macros

    /* To install a plugin:
     1) Load "install/install.xml" from the plugin and check if it is a script
        or a binary plugin
     2) Load the Install script
     3) Check if there is a plugin with the same name installed
     3) Run the pre install function (function pre_install() )
        from the install script and check the return value
     4) Extract the resources (images and extra files)
     5) Extract the binary plugin or the script
     6) Run the post install script function
    */
    InstallInfo install_info;
    install_info.PluginSourcePath = wxFileName(actualName);
    // First read The "install/install.xml" file
    int ret = ReadInstallInfo(actualName,&install_info);
    if(ret != 0)
    {
        // If there is no install.xml file then it is a old plugin and for sure a binary version
        install_info.type = wxT("BINARY");
    }

    cb::shared_ptr<ScriptBindings::CBSquirrelThread> sandbox; // Sandbox for the install script

// TODO (bluehazzard#1#): Check every step of the installation, and if a error occurred, delete all installed files
    INSTALL_STATUS status = STATUS_INSTALL;     // Track the current installation state


    // The plugin file name can be combined with the version:
    // "PluginName-2.4.5.cbplugin"
    if(install_info.BaseName.IsEmpty())
    {
        // If there is no install.xml to get version and Name
        // we get the name and version from the file name itself
        wxString version;
        install_info.BaseName = install_info.PluginSourcePath.GetName();
        if (install_info.BaseName.Contains(_T('-')))
        {
            // Separate version and name
            version = install_info.BaseName.AfterFirst(_T('-'));
            install_info.BaseName = install_info.BaseName.BeforeFirst(_T('-'));
        }
    }

    // Create the remaining file Names
    install_info.ResourceFileName = install_info.BaseName + _T(".zip");
    install_info.settingsOnName = install_info.BaseName + _T(".png");
    install_info.settingsOffName = install_info.BaseName + _T("-off.png");

    wxProgressDialog pd(_("Installing: ") + install_info.BaseName, _T("A description wide enough for the dialog ;)"), 10);

    //wxString pluginFilename = UnixFilename(pluginDir + _T('/') + localName);

    if(install_info.type = wxT("SCRIPT"))
    {
        // if plugin with the same name exists, ask to uninstall first
        if(Manager::Get()->GetScriptingManager()->GetPlugin(install_info.BaseName) != nullptr)
        {
            if (askForConfirmation)
            {
                wxString msg = _("A plugin with the same name is already registered.\n");

                if (cbMessageBox(msg + _T('\n') +
                                 _("If you want to proceed, the installed plugin will be "
                                   "uninstalled first.\n"
                                   "Do you want to proceed?"),
                                 _("Confirmation"), wxICON_QUESTION | wxYES_NO) == wxID_NO)
                {
                    return false;
                }
            }
            if (!UninstallScriptPlugin(install_info.BaseName,true))
                return false;
        }
    }
    else    // It is a binary plugin
    {
        // if plugin with the same name exists, ask to uninstall first
        cbPlugin* existingPlugin = FindPluginByName(install_info.BaseName);
        if (existingPlugin)
        {
            if (askForConfirmation)
            {
                wxString msg = _("A plugin with the same name is already installed.\n");
                if (!version.IsEmpty())
                {
                    const PluginInfo* existingInfo = GetPluginInfo(existingPlugin);
                    if (CompareVersions(version, existingInfo->version) < 0)
                    {
                        msg = _("The plugin you are trying to install, is older "
                                "than the one currently installed.");
                    }
                }

                if (cbMessageBox(msg + _T('\n') +
                                 _("If you want to proceed, the installed plugin will be "
                                   "uninstalled first.\n"
                                   "Do you want to proceed?"),
                                 _("Confirmation"), wxICON_QUESTION | wxYES_NO) == wxID_NO)
                {
                    return false;
                }
            }
            if (!UninstallPlugin(existingPlugin))
                return false;
        }
    }


    pd.Update(1, _T("Load install script"));
    Manager::Get()->GetLogManager()->DebugLog(_T("Load install script"));

    if(install_info.InstallScript != wxEmptyString)
    {
        // If the install.xml contains a install script load and it
        wxString content;
        wxString path = install_info.PluginSourcePath() + wxT("#zip:") + install_info.ResourceFileName +  wxT("#zip:install/") + install_info.InstallScript;
        if(Manager::Get()->GetScriptingManager()->LoadFileFromFSPath(path,content) != 0)
        {
            Manager::Get()->GetLogManager()->DebugLog(_T("Failed to load install script ")+ install_info.InstallScript + _(" from plugin") );;
            return false;
        }

        // we run the install script in a sandbox
        sandbox = cb::shared_ptr<ScriptBindings::CBSquirrelThread>(Manager::Get()->GetScriptingManager()->CreateSandbox());
        if(sandbox.get() == nullptr)
        {
            Manager::Get()->GetLogManager()->LogError(_T("Failed to create a script sandbox..."));
            return false;
        }

        ScriptBindings::SC_ERROR_STATE state = sandbox->doString(content,_("Install script: ")+pluginFilename);
        if(state != ScriptBindings::SC_NO_ERROR)
        {
            Manager::Get()->GetLogManager()->LogError(_T("Failed to run install script: "));
            wxString error = sandbox->GetVM()->getLastErrorMsg();
            Manager::Get()->GetLogManager()->LogError(_T("Script error: ")+ error);
            Manager::Get()->GetScriptingManager()->DisplayErrors(error,false);
            return false;
        }

    }

    pd.Update(2,_T("Run pre Install script"));
    if(sandbox.get() != nullptr)
    {
        // Run the pre_install function
        Sqrat::Function pre_install_function(sandbox->GetVM()->GetRootTable(),"pre_install");
        if(pre_install_function.IsNull())
        {
            Manager::Get()->GetLogManager()->DebugLog(_T("No pre_install function found..."));
        }
        else
        {
            Sqrat::SharedPtr<int> sh_ret = pre_install_function.Evaluate<int>(static_cast<int>(status));
            if(!sh_ret)
            {
                Manager::Get()->GetLogManager()->LogError(_T("Failed to run install script: "));
                wxString error = sandbox->GetVM()->getLastErrorMsg();
                Manager::Get()->GetLogManager()->LogError(_T("Script error: ")+ error);
                Manager::Get()->GetScriptingManager()->DisplayErrors(error,false);
                return false;
            }

            int ret = *sh_ret.Get();
            status = static_cast<INSTALL_STATUS>(ret);
        }
    }

    if(status == STATUS_ERROR)
    {
        Manager::Get()->GetLogManager()->DebugLog(_T("A error occurred while installing the plugin"));
        return false;
    } else if(status == STATUS_USER_ABORT)
    {
        Manager::Get()->GetLogManager()->DebugLog(_T("User aborted the installation of the plugin"));
        return false;
    }


    // Extract all files
    // The function updates the process dialog
    // TODO (bluehazzard#1#): Improve the UI (update Process dialog)
    //pd.Update(2,_("Extract plugin resource"));
    ExtractResourceFiles(pd,install_info,forAllUsers);


    pd.Update(5, _("Extracting extra Files"));
    // extract extra files
    wxArrayString extraFiles;
    ReadExtraFilesFromManifestFile(localName, extraFiles);
    for (size_t i = 0; i < extraFiles.GetCount(); ++i)
    {
        ExtractFile(actualName,
                    extraFiles[i],
                    resourceDir + _T("/") + extraFiles[i],
                    false);
        pd.Pulse();
    }

    // Now extract the binary or the script
    pd.Update(6, _("Install the plugin binary"));
    bool success = false;
    if(install_info.type == wxT("SCRIPT"))
    {
        success = InstallScriptPlugin(pluginName,&install_info,forAllUsers,askForConfirmation);
    }
    else
    {
        success = InstallBinaryPlugin(pluginName,forAllUsers,askForConfirmation);
    }

    if(!success)
    {
        // There was a failure in coping the binary/ script
        // TODO (bluehazzard#1#): Remove all until now copied files...
        return false;
    }


    pd.Update(7,_T("Run post Install script"));
    if(sandbox.get() != nullptr)
    {
        // Run the pre_install function
        Sqrat::Function pre_install_function(sandbox->GetVM()->GetRootTable(),"post_install");
        if(pre_install_function.IsNull())
        {
            Manager::Get()->GetLogManager()->DebugLog(_T("No post_install function found..."));
        }
        else
        {
            Sqrat::SharedPtr<int> sh_ret = pre_install_function.Evaluate<int>(static_cast<int>(status));
            if(!sh_ret)
            {
                Manager::Get()->GetLogManager()->LogError(_T("Failed to run install script: "));
                wxString error = sandbox->GetVM()->getLastErrorMsg();
                Manager::Get()->GetLogManager()->LogError(_T("Script error: ")+ error);
                Manager::Get()->GetScriptingManager()->DisplayErrors(error,false);
                return false;
            }

            int ret = *sh_ret.Get();
            status = static_cast<INSTALL_STATUS>(ret);
        }
    }

    if(status != STATUS_INSTALL)
    {
        if(status == STATUS_ERROR)
            Manager::Get()->GetLogManager()->DebugLog(_T("A error occurred while installing the plugin"));
        else if(status == STATUS_USER_ABORT)
            Manager::Get()->GetLogManager()->DebugLog(_T("User aborted the installation of the plugin"));

        pd.Pulse(_("Installation is halted..."));

        if(cbMessageBox(_("The installation could not be finished. Should we delete the already installed files?"),
                        _("Installation was not successful"),wxICON_EXCLAMATION|wxYES_NO) == wxID_YES)
        {
            pd.Pulse(_("Removing installed files..."));
            bool success = true;
            //success &= wxRemoveFile(pluginFilename);
            //success &= wxRemoveFile(resourceDir + _T('/') + resourceName);
            //success &= wxRemoveFile(resourceDir + _T("/images/settings/") + settingsOnName);
            //success &= wxRemoveFile(resourceDir + _T("/images/settings/") + settingsOffName);
            for (size_t i = 0; i < extraFiles.GetCount(); ++i)
                success &= wxRemoveFile(resourceDir + _T("/") + extraFiles[i]);
            if(!success)
            {
                 Manager::Get()->GetLogManager()->DebugLog(_T("Not all files could be deleted properly"));
            }
        }

        return false;
    }

    // Load the new plugins
    pd.Update(8, _("Updating menus and toolbars"));
    //    Manager::Get()->GetLogManager()->DebugLog(F(_T("Loading plugin...")));
    ScanForPlugins(pluginDir);
    LoadAllPlugins();
    if(install_info.type == wxT("SCRIPT"))
    {
        ScriptBindings::cbScriptPlugin* plugin = Manager::Get()->GetScriptingManager()->GetPlugin(pluginName);
        if(plugin != nullptr)
        {
            // plugin not found
            Manager::Get()->GetLogManager()->DebugLog(_T("Failed to install script Plugin"));
            return false;
        }
    }
    else
    {
        cbPlugin* plugin = FindPluginByFileName(pluginFilename);
        const PluginInfo* info = GetPluginInfo(plugin);
        if (!plugin || !info)
        {
            Manager::Get()->GetLogManager()->DebugLog(_T("Failed"));
            return false;
        }
        //    Manager::Get()->GetLogManager()->DebugLog(F(_T("Succeeded")));
    }

    // inform app to update menus and toolbars
    pd.Update(9, _("Updating menus and toolbars"));
    CodeBlocksEvent evt(cbEVT_PLUGIN_INSTALLED);
    evt.SetPlugin(plugin);
    Manager::Get()->ProcessEvent(evt);
//    Manager::Get()->GetLogManager()->DebugLog(F(_T("Menus updated")));

    pd.Update(10, _("FINISH"));


//// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

}

bool PluginManager::ExtractResourceFiles(wxProgressDialog& pd,InstallInfo info,bool forAllUsers)
{
    wxString pluginDir;
    wxString resourceDir;
    if (forAllUsers)
    {
        pluginDir = ConfigManager::GetFolder(sdPluginsGlobal);
        resourceDir = ConfigManager::GetFolder(sdDataGlobal);
    }
    else
    {
        pluginDir = ConfigManager::GetFolder(sdPluginsUser);
        resourceDir = ConfigManager::GetFolder(sdDataUser);
    }

    pd.Update(3, _("Extracting plugin resources"));

    // extract resources from bundle
    if (!ExtractFile(info.FullInstallPath.GetFullPath(),
                     info.ResourceFileName,
                     resourceDir + _T('/') + info.ResourceFileName))
    {
        // TODO (bluehazzard#1#): This is a critical error, we should handle it
        Manager::Get()->GetLogManager()->DebugWarning(wxString(_T("Error on extracting "))+ info.ResourceFileName);
    }

    pd.Update(4, _("Extracting plugin icons for \"Settings\" dialog"));

    if(!ExtractFile(info.FullInstallPath.GetFullPath(),
                info.settingsOnName,
                resourceDir + _T("/images/settings/") + info.settingsOnName,
                false))
    {
         Manager::Get()->GetLogManager()->DebugWarning(wxString(_T("Error on extracting "))+ info.settingsOnName);

    if(!ExtractFile(info.FullInstallPath.GetFullPath(),
                info.settingsOffName,
                resourceDir + _T("/images/settings/") + info.settingsOffName,
                false))
    {
        Manager::Get()->GetLogManager()->DebugWarning(wxString(_T("Error on extracting "))+ info.settingsOffName);
    }
//    Manager::Get()->GetLogManager()->DebugLog(F(_T("Extracted resources")));

}

bool PluginManager::InstallBinaryPlugin(const wxString& actualName,InstallInfo& info, bool forAllUsers, bool askForConfirmation)
{
    if(pluginName.IsEmpty())
    {
        Manager::Get()->GetLogManager()->LogWarning(_T("PluginManager::InstallScriptPlugin: pluginName.IsEmpty()"));
        return false;
    }

    if (!platform::windows && resourceName.StartsWith(_T("lib")))
        resourceName.Remove(0, 3);
    if (!platform::windows && settingsOnName.StartsWith(_T("lib")))
        settingsOnName.Remove(0, 3);
    if (!platform::windows && settingsOffName.StartsWith(_T("lib")))
        settingsOffName.Remove(0, 3);
    wxString pluginFilename = UnixFilename(pluginDir + _T('/') + localName);
//    Manager::Get()->GetLogManager()->DebugLog(F(_T("Plugin filename: ") + pluginFilename));
//    Manager::Get()->GetLogManager()->DebugLog(F(_T("Plugin resources: ") + ConfigManager::GetDataFolder() + _T('/') + resourceName));

    // extract plugin from bundle
    if (!ExtractFile(actualName,
                     localName,
                     pluginFilename))
        return false;
//    Manager::Get()->GetLogManager()->DebugLog(F(_T("Extracted plugin")));
    return true;
}

bool PluginManager::UninstallPlugin(cbPlugin* plugin, bool removeFiles)
{
    if (!plugin)
        return false;

    wxString title;
    wxString pluginFilename;
    wxString resourceFilename;
    wxString settingsOnFilename;
    wxString settingsOffFilename;
    wxArrayString extrafiles;

    // find the plugin element
    for (size_t i = 0; i < m_Plugins.GetCount(); ++i)
    {
        PluginElement* elem = m_Plugins[i];
        if (elem && elem->plugin == plugin)
        {
            // got it
            title = elem->info.title;
            pluginFilename = elem->fileName;
            // now get the resource name
            wxFileName fname(pluginFilename);
            resourceFilename = fname.GetName() + _T(".zip");
            settingsOnFilename = fname.GetName() + _T(".png");
            settingsOffFilename = fname.GetName() + _T("-off.png");
            if (!platform::windows && resourceFilename.StartsWith(_T("lib")))
                resourceFilename.Remove(0, 3);
            if (!platform::windows && settingsOnFilename.StartsWith(_T("lib")))
                settingsOnFilename.Remove(0, 3);
            if (!platform::windows && settingsOffFilename.StartsWith(_T("lib")))
                settingsOffFilename.Remove(0, 3);
            resourceFilename = ConfigManager::LocateDataFile(resourceFilename, sdDataGlobal | sdDataUser);
            settingsOnFilename = ConfigManager::LocateDataFile(_T("images/settings/") + settingsOnFilename, sdDataGlobal | sdDataUser);
            settingsOffFilename = ConfigManager::LocateDataFile(_T("images/settings/") + settingsOffFilename, sdDataGlobal | sdDataUser);

            ReadExtraFilesFromManifestFile(resourceFilename, extrafiles);
            for (size_t n = 0; n < extrafiles.GetCount(); ++n)
            {
                extrafiles[n] = ConfigManager::LocateDataFile(extrafiles[n], sdDataGlobal | sdDataUser);
            }
            break;
        }
    }

    if (wxFileExists(pluginFilename) && !wxFile::Access(pluginFilename, wxFile::write))
    {
        // no write-access; abort
        cbMessageBox(_("You don't have the needed privileges to uninstall this plugin.\n"
                       "Ask your administrator to uninstall this plugin for you..."),
                     _("Warning"), wxICON_WARNING);
        return false;
    }

//    Manager::Get()->GetLogManager()->DebugLog(F(_T("UninstallPlugin:")));
//    Manager::Get()->GetLogManager()->DebugLog(F(_T("Plugin filename: ") + pluginFilename));
//    Manager::Get()->GetLogManager()->DebugLog(F(_T("Plugin resources: ") + resourceFilename));

    wxProgressDialog pd(wxString::Format(_("Uninstalling %s"), title.c_str()),
                        _T("A description wide enough for the dialog ;)"), 3);

    pd.Update(1, _("Detaching plugin"));
    DetachPlugin(plugin);
//    Manager::Get()->GetLogManager()->DebugLog(F(_T("Plugin released")));

    pd.Update(2, _("Updating menus and toolbars"));
    CodeBlocksEvent event(cbEVT_PLUGIN_UNINSTALLED);
    event.SetPlugin(plugin);
    Manager::Get()->ProcessEvent(event);
//    Manager::Get()->GetLogManager()->DebugLog(F(_T("Menus updated")));

    pd.Update(3, _("Unloading plugin"));
    UnloadPlugin(plugin);
//    Manager::Get()->GetLogManager()->DebugLog(F(_T("Plugin unloaded")));

    if (!removeFiles)
        return true;

    // under linux, if the progress dialog is still visible and updated
    // causes a crash because it re-enters gtk_main_iteration() calling
    // eventually OnUpdateUI() in the config dialog, which in turn references
    // an invalid plugin...
//    pd.Update(4, _("Removing files"));

    if (!pluginFilename.IsEmpty())
    {
        if (wxRemoveFile(pluginFilename))
        {
//            Manager::Get()->GetLogManager()->DebugLog(F(_T("Plugin file removed")));
            if (!resourceFilename.IsEmpty())
            {
                if (!wxRemoveFile(resourceFilename))
                    Manager::Get()->GetLogManager()->LogWarning(_T("Failed to remove plugin resources: ") + resourceFilename);
            }
            if (!settingsOnFilename.IsEmpty() && wxFileExists(settingsOnFilename))
            {
                if (!wxRemoveFile(settingsOnFilename))
                    Manager::Get()->GetLogManager()->LogWarning(_T("Failed to remove icon for \"Settings\" dialog: ") + settingsOnFilename);
            }
            if (!settingsOffFilename.IsEmpty() && wxFileExists(settingsOffFilename))
            {
                if (!wxRemoveFile(settingsOffFilename))
                    Manager::Get()->GetLogManager()->LogWarning(_T("Failed to remove icon for \"Settings\" dialog: ") + settingsOffFilename);
            }
            for (size_t i = 0; i < extrafiles.GetCount(); ++i)
            {
                if (!extrafiles[i].IsEmpty() && wxFileExists(extrafiles[i]))
                {
                    if (!wxRemoveFile(extrafiles[i]))
                        Manager::Get()->GetLogManager()->LogWarning(_T("Failed to remove extra file: ") + extrafiles[i]);
                }
            }
            return true;
        }
        else
        {
            Manager::Get()->GetLogManager()->LogWarning(_T("Failed to remove plugin file: ") + pluginFilename);
            cbMessageBox(_("Plugin could not be completely uninstalled because its files could not be removed.\n\n"
                           "This can happen if the plugin's file is in-use like, for "
                           "example, when the same plugin file provides more than one "
                           "plugin.\n"
                           "In this case either uninstall all other plugins "
                           "which are provided by the same file, or remove it yourself "
                           "(manually) when you shut down Code::Blocks.\n"
                           "The files that could not be deleted are:\n\n") +
                         pluginFilename + _T('\n') +
                         resourceFilename + _T('\n') +
                         settingsOnFilename + _T('\n') +
                         settingsOffFilename,
                         _("Warning"), wxICON_WARNING);
            return false;
        }
    }
    return false;
}

bool PluginManager::ExportPlugin(cbPlugin* plugin, const wxString& filename)
{
    if (!plugin)
        return false;

    wxArrayString sourcefiles;
    wxArrayString extrafiles;
    wxArrayString extrafilesdest;
    wxFileName fname;
    wxString resourceFilename;

    // find the plugin element
    for (size_t i = 0; i < m_Plugins.GetCount(); ++i)
    {
        PluginElement* elem = m_Plugins[i];
        if (elem && elem->plugin == plugin)
        {
            // got it

            // plugin file
            sourcefiles.Add(elem->fileName);
            fname.Assign(elem->fileName);

            // now get the resource zip filename
            resourceFilename = fname.GetName() + _T(".zip");
            if (!platform::windows && resourceFilename.StartsWith(_T("lib")))
                resourceFilename.Remove(0, 3);
            resourceFilename = ConfigManager::LocateDataFile(resourceFilename, sdDataGlobal | sdDataUser);
            sourcefiles.Add(resourceFilename);

            // the highlighted icon the plugin may have for its "settings" page
            resourceFilename = fname.GetName() + _T(".png");
            if (!platform::windows && resourceFilename.StartsWith(_T("lib")))
                resourceFilename.Remove(0, 3);
            resourceFilename.Prepend(_T("images/settings/"));
            resourceFilename = ConfigManager::LocateDataFile(resourceFilename, sdDataGlobal | sdDataUser);
            if (!resourceFilename.IsEmpty())
                sourcefiles.Add(resourceFilename);

            // the non-highlighted icon the plugin may have for its "settings" page
            resourceFilename = fname.GetName() + _T("-off.png");
            if (!platform::windows && resourceFilename.StartsWith(_T("lib")))
                resourceFilename.Remove(0, 3);
            resourceFilename.Prepend(_T("images/settings/"));
            resourceFilename = ConfigManager::LocateDataFile(resourceFilename, sdDataGlobal | sdDataUser);
            if (!resourceFilename.IsEmpty())
                sourcefiles.Add(resourceFilename);

            // export extra files
            resourceFilename = fname.GetName() + _T(".zip");
            if (!platform::windows && resourceFilename.StartsWith(_T("lib")))
                resourceFilename.Remove(0, 3);
            ReadExtraFilesFromManifestFile(resourceFilename, extrafilesdest);
            for (size_t n = 0; n < extrafilesdest.GetCount(); ++n)
            {
                extrafiles.Add(ConfigManager::LocateDataFile(extrafilesdest[n], sdDataGlobal | sdDataUser));
            }

            break;
        }
    }

    if (wxFileExists(filename))
    {
        if (!wxFile::Access(filename, wxFile::write))
        {
            cbMessageBox(wxString::Format(_("%s is in use.\nAborting..."), filename.c_str()),
                         _("Warning"), wxICON_WARNING);
            return false;
        }
    }

//    Manager::Get()->GetLogManager()->DebugLog(F(_T("Creating archive: ") + filename));
    wxFileOutputStream out(filename);
    wxZipOutputStream zip(out, 9); // max compression
    for (size_t i = 0; i < sourcefiles.GetCount(); ++i)
    {
        if (sourcefiles[i].IsEmpty())
            continue;

        wxFileInputStream in(sourcefiles[i]);
        zip.PutNextEntry(wxFileName(sourcefiles[i]).GetFullName());
        zip << in;
    }
    for (size_t i = 0; i < extrafiles.GetCount(); ++i)
    {
        if (extrafiles[i].IsEmpty() || extrafilesdest[i].IsEmpty())
            continue;

        wxFileInputStream in(extrafiles[i]);

        zip.PutNextEntry(extrafilesdest[i]);
        zip << in;
    }
    zip.SetComment(_T("This is a redistributable plugin for the Code::Blocks IDE.\n"
                      "See http://www.codeblocks.org for details..."));

    return true;
}

bool PluginManager::ExtractFile(const wxString& bundlename,
                                const wxString& src_filename,
                                const wxString& dst_filename,
                                bool isMandatory)
{
//    Manager::Get()->GetLogManager()->DebugLog(F(_T("ExtractFile:")));
//    Manager::Get()->GetLogManager()->DebugLog(F(_T("Plugin filename: ") + bundlename));
//    Manager::Get()->GetLogManager()->DebugLog(F(_T("Source filename: ") + src_filename));
//    Manager::Get()->GetLogManager()->DebugLog(F(_T("Destination filename: ") + dst_filename));

    // check if the destination file already exists
    if (wxFileExists(dst_filename) && !wxFile::Access(dst_filename, wxFile::write))
    {
//        Manager::Get()->GetLogManager()->DebugLog(F(_T("Destination file in use")));
        cbMessageBox(_("The destination file is in use.\nAborting..."), _("Warning"), wxICON_WARNING);
        return false;
    }

    // make sure destination dir exists
    CreateDirRecursively(wxFileName(dst_filename).GetPath(wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR));

    // actually extract file
//    Manager::Get()->GetLogManager()->DebugLog(F(_T("Extracting...")));
    wxFileSystem* fs = new wxFileSystem;
    wxFSFile* f = fs->OpenFile(bundlename + _T("#zip:") + src_filename);
    if (f)
    {
        // open output file for writing
        wxFile output(dst_filename, wxFile::write);
        if (!output.IsOpened())
        {
//            Manager::Get()->GetLogManager()->DebugLog(F(_T("Can't open destination file for writing")));
            wxString msg = wxString::Format(_("Can't open destination file '%s' for writing..."),
                                            dst_filename.c_str());
            cbMessageBox(msg, _("Error"), wxICON_ERROR);
            delete f;
            delete fs;
            return false;
        }

        // copy file
        wxInputStream* is = f->GetStream();
        char tmp[1025] = {};
        while (!is->Eof() && is->CanRead())
        {
            memset(tmp, 0, sizeof(tmp));
            is->Read(tmp, sizeof(tmp) - 1);
            output.Write(tmp, is->LastRead());
        }
        delete f;
//        Manager::Get()->GetLogManager()->DebugLog(F(_T("Extracted")));
    }
    else
    {
//        Manager::Get()->GetLogManager()->DebugLog(F(_T("File not found in plugin")));
        if (isMandatory)
        {
            wxString msg = wxString::Format(_("File '%s' not found in plugin '%s'"),
                                            src_filename.c_str(), bundlename.c_str());
            cbMessageBox(msg, _("Error"), wxICON_ERROR);
            delete fs;
            return false;
        }
    }
    delete fs;
    return true;
}

void PluginManager::RegisterPlugin(const wxString& name,
                                   CreatePluginProc createProc,
                                   FreePluginProc freeProc,
                                   PluginSDKVersionProc versionProc)
{
    // sanity checks
    if (name.IsEmpty() || !createProc || !freeProc || !versionProc)
        return;

    // first check to see it's not already loaded
    if (FindPluginByName(name))
        return; // yes, already loaded

    // read manifest file for plugin
    PluginInfo info;
    if (!ReadManifestFile(m_CurrentlyLoadingFilename, name, &info) ||
            info.name.IsEmpty())
    {
        Manager::Get()->GetLogManager()->LogError(_T("Invalid manifest file for: ") + name);
        return;
    }

    // now get the SDK version number (extra check)
    int major;
    int minor;
    int release;
    versionProc(&major, &minor, &release);
    if (major != PLUGIN_SDK_VERSION_MAJOR ||
            minor != PLUGIN_SDK_VERSION_MINOR ||
            release != PLUGIN_SDK_VERSION_RELEASE)
    {
        // wrong version: in this case, inform the user...
        wxString fmt;
        fmt.Printf(_("SDK version mismatch for %s (%d.%d.%d). Expecting %d.%d.%d"),
                   name.c_str(),
                   major,
                   minor,
                   release,
                   PLUGIN_SDK_VERSION_MAJOR,
                   PLUGIN_SDK_VERSION_MINOR,
                   PLUGIN_SDK_VERSION_RELEASE);
        Manager::Get()->GetLogManager()->LogError(fmt);
        return;
    }

    // all done
    // add this plugin in the temporary registration vector to be loaded
    // by LoadPlugin() (which triggered the call to this function).
    PluginRegistration pr;
    pr.name = name;
    pr.createProc = createProc;
    pr.freeProc = freeProc;
    pr.versionProc = versionProc;
    pr.info = info;
    m_RegisteredPlugins.push_back(pr);
}

int PluginManager::ReadInstallInfo(const wxString& pluginFilename,
                                   InstallInfo* infoOut)
{
    if(infoOut == nullptr)
        return -1;

    // The pluginFilename parameter contains the path to the plugin...
    wxString contents;
    // if we want to access the plugin resource zip file we need the actual plugin name
    wxString basename = wxFileName(pluginFilename).GetName();

    wxFileSystem* fs = new wxFileSystem;
    // Search for the install.xml in the plugin resource zip file within the install archive
    wxFSFile* f = fs->OpenFile(pluginFilename + _T("#zip:")+ basename + _T(".zip#zip:install/install.xml"));
    if (f)
    {
        wxInputStream* is = f->GetStream();
        char tmp[1024] = {};
        while (!is->Eof() && is->CanRead())
        {
            memset(tmp, 0, sizeof(tmp));
            is->Read(tmp, sizeof(tmp) - 1);
            contents << cbC2U((const char*)tmp);
        }
        delete f;
    }
    else
    {
        Manager::Get()->GetLogManager()->LogError(_T("Could not read install.xml in: ") + pluginFilename);
        delete fs;
        return -2;
    }
    delete fs;

    // We have now loaded the content of the install.xml file in "contents"

    /*
    * The install.xml file structure:
    *
    *
    *<?xml version="1.0" standalone=no>
    *<CodeBlocks_plugin_install_file>
    *	<Plugin type="SCRIPT" name="PluginName" version_major=1 version_minor=0>
    *       <install_script file="script_file">
    *           // Here you can add the name of a squirrel script within the .cbplugin archive
    *           // This script can define the function:
    *           // pre_install(info):
    *           //  This function is called before the installation.
    *           //  Possible return values:
    *           //     *  0: all is ok, and the installation can continue
    *           //     * -1: a internal error occurred and the installation may be aborted
    *           //     * -2: the user aborted the installation
    *           //
    *           // post_install(info)
    *           //  This function is called after the installation was completed
    *           //  info.status shows the status of the installation:
    *           //     *  0: all is ok, and the installation was completed without an error
    *           //     * -1: a internal error occurred and the installation was aborted
    *           //     * -2: the user aborted the installation
    *           //
    *           // pre_uninstall(info)
    *           //  This function is called before the the plugin gets uninstalled.
    *           //  Possible return values:
    *           //     *  0: all is ok, and the uninstallation can continue
    *           //     * -1: a internal error occurred and the uninstallation may be aborted
    *           //     * -2: the user aborted the uninstallation
    *           //
    *           // post_uninstall(info)
    *           //  This function is called after the uninstallation was completed
    *           //     *  0: all is ok, and the installation was completed without an error
    *           //     * -1: a internal error occurred and the installation was aborted
    *           //     * -2: the user aborted the installation
    *   </Plugin>
    *</CodeBlocks_plugin_install_file>
    *
    *  The Attribute "type" can either be "SCRIPT" or "BINARY"
    *
    */

    cb::shared_ptr<TiXmlDocument> xml_doc(new TiXmlDocument());

    //TiXmlDocument xml_doc = new TiXmlDocument();

    if(!xml_doc->Parse(cbU2C(contents)))
    {
        Manager::Get()->GetLogManager()->LogError(_T("Plugin install.xml could not be parsed: ") + pluginFilename);
        return -2;
    }

    TiXmlElement* root = xml_doc->FirstChildElement("CodeBlocks_plugin_install_file");
    if(!root)
    {
        Manager::Get()->GetLogManager()->LogError(_T("Plugin install file not valid (no root element found) for: ") + pluginFilename);
        return -3;
    }

    TiXmlElement* plugin = root->FirstChildElement("Plugin");
    if(!plugin)
    {
        Manager::Get()->GetLogManager()->LogError(_T("Plugin install file not valid (no \"Plugin\" element found) for: ") + pluginFilename);
        return -3;
    }

    const char* type = plugin->Attribute("type");
    if(!type)
    {
        Manager::Get()->GetLogManager()->LogError(_T("Plugin install file not valid (no \"type\" attribute in \"Plugin\" element found) for: ") + pluginFilename);
        return -3;
    }
    infoOut->type = wxString::FromUTF8(type);

    const char* name = plugin->Attribute("name");
    if(!type)
    {
        Manager::Get()->GetLogManager()->LogError(_T("Plugin install file not valid (no \"name\" attribute in \"Plugin\" element found) for: ") + pluginFilename);
        return -3;
    }
    infoOut->BaseName = wxString::FromUTF8(name);

    infoOut->version_major = 0;
    if(plugin->QueryIntAttribute("version_major",&(infoOut->version_major)) != TIXML_SUCCESS)
    {
        Manager::Get()->GetLogManager()->LogError(_T("Plugin install file not valid (no or wrong \"version_major\" attribute in \"Plugin\" element found) for: ") + pluginFilename);
    }

    infoOut->version_minor = 0;
    if(plugin->QueryIntAttribute("version_minor",&(infoOut->version_minor)) != TIXML_SUCCESS)
    {
        Manager::Get()->GetLogManager()->LogError(_T("Plugin install file not valid (no or wrong \"version_minor\" attribute in \"Plugin\" element found) for: ") + pluginFilename);
    }


    TiXmlElement* install_script = plugin->FirstChildElement("install_script");
    if(install_script)
    {
        const char* ac_install_script = install_script->Attribute("file");
        if(ac_install_script)
            infoOut->InstallScript = wxString::FromUTF8(ac_install_script);
        else
 // TODO (bluehazzard#1#): If no file what should we do then?
           infoOut->InstallScript.Clear();
    }


    return 0;
}

bool PluginManager::ReadManifestFile(const wxString& pluginFilename,
                                     const wxString& pluginName,
                                     PluginInfo* infoOut)
{
    if (!m_pCurrentlyLoadingManifestDoc)
    {
        // find and load plugin's resource file
        // (pluginFilename contains no path info)
        wxFileName fname(pluginFilename);
        fname.SetExt(_T("zip"));
        wxString actual = fname.GetFullName();

        // remove 'lib' prefix from plugin name (if any)
        if (!platform::windows && actual.StartsWith(_T("lib")))
            actual.Remove(0, 3);

        actual = ConfigManager::LocateDataFile(actual, sdPluginsUser | sdDataUser | sdPluginsGlobal | sdDataGlobal);
        if (actual.IsEmpty())
        {
            Manager::Get()->GetLogManager()->LogError(_T("Plugin resource not found: ") + fname.GetFullName());
            return false; // not found
        }

        // load XML from ZIP
        wxString contents;
        wxFileSystem* fs = new wxFileSystem;
        wxFSFile* f = fs->OpenFile(actual + _T("#zip:manifest.xml"));
        if (f)
        {
            wxInputStream* is = f->GetStream();
            char tmp[1024] = {};
            while (!is->Eof() && is->CanRead())
            {
                memset(tmp, 0, sizeof(tmp));
                is->Read(tmp, sizeof(tmp) - 1);
                contents << cbC2U((const char*)tmp);
            }
            delete f;
        }
        else
        {
            Manager::Get()->GetLogManager()->LogError(_T("No plugin manifest file in resource: ") + actual);
            delete fs;
            return false;
        }
        delete fs;

        // actually load XML document
        m_pCurrentlyLoadingManifestDoc = new TiXmlDocument;
        if (!m_pCurrentlyLoadingManifestDoc->Parse(cbU2C(contents)))
        {
            Manager::Get()->GetLogManager()->LogError(_T("Plugin manifest could not be parsed: ") + actual);
            return false;
        }
    }

    TiXmlElement* root = m_pCurrentlyLoadingManifestDoc->FirstChildElement("CodeBlocks_plugin_manifest_file");
    if (!root)
    {
        Manager::Get()->GetLogManager()->LogError(_T("Plugin resource file not valid (no root element found) for: ") + pluginFilename);
        return false;
    }

    TiXmlElement* version = root->FirstChildElement("SdkVersion");
    if (!version)
    {
        Manager::Get()->GetLogManager()->LogError(_T("Plugin resource file not valid (no SdkVersion element found) for: ") + pluginFilename);
        return false;
    }

    // check version
//    int major;
//    int minor;
//    int release;
//    if (version->QueryIntAttribute("major", &major) != TIXML_SUCCESS)
//        major = 0;
//    if (version->QueryIntAttribute("minor", &minor) != TIXML_SUCCESS)
//        minor = 0;
//    if (version->QueryIntAttribute("release", &release) != TIXML_SUCCESS)
//        release = 0;
//
//    if (major != PLUGIN_SDK_VERSION_MAJOR ||
//        minor != PLUGIN_SDK_VERSION_MINOR ||
//        release != PLUGIN_SDK_VERSION_RELEASE)
//    {
//        // wrong version: in this case, inform the user...
//        wxString fmt;
//        fmt.Printf(_("SDK version mismatch for %s (%d.%d.%d). Expecting %d.%d.%d"),
//                    pluginName.c_str(),
//                    major,
//                    minor,
//                    release,
//                    PLUGIN_SDK_VERSION_MAJOR,
//                    PLUGIN_SDK_VERSION_MINOR,
//                    PLUGIN_SDK_VERSION_RELEASE);
//        Manager::Get()->GetLogManager()->LogError(fmt);
//        return false;
//    }

    // if no plugin name specified, we 're done here (successfully)
    if (pluginName.IsEmpty() || !infoOut)
        return true;

    TiXmlElement* plugin = root->FirstChildElement("Plugin");
    while (plugin)
    {
        const char* name = plugin->Attribute("name");
        if (name && cbC2U(name) == pluginName)
        {
            infoOut->name = pluginName;
            TiXmlElement* value = plugin->FirstChildElement("Value");
            while (value)
            {
                if (value->Attribute("title"))
                    infoOut->title = cbC2U(value->Attribute("title"));
                if (value->Attribute("version"))
                    infoOut->version = cbC2U(value->Attribute("version"));
                if (value->Attribute("description"))
                    infoOut->description = cbC2U(value->Attribute("description"));
                if (value->Attribute("author"))
                    infoOut->author = cbC2U(value->Attribute("author"));
                if (value->Attribute("authorEmail"))
                    infoOut->authorEmail = cbC2U(value->Attribute("authorEmail"));
                if (value->Attribute("authorWebsite"))
                    infoOut->authorWebsite = cbC2U(value->Attribute("authorWebsite"));
                if (value->Attribute("thanksTo"))
                    infoOut->thanksTo = cbC2U(value->Attribute("thanksTo"));
                if (value->Attribute("license"))
                    infoOut->license = cbC2U(value->Attribute("license"));

                value = value->NextSiblingElement("Value");
            }
            break;
        }

        plugin = plugin->NextSiblingElement("Plugin");
    }

    return true;
}

void PluginManager::ReadExtraFilesFromManifestFile(const wxString& pluginFilename,
        wxArrayString& extraFiles)
{
    extraFiles.Clear();

    // find and load plugin's resource file
    // (pluginFilename contains no path info)
    wxFileName fname(pluginFilename);
    fname.SetExt(_T("zip"));
    wxString actual = fname.GetFullName();

    // remove 'lib' prefix from plugin name (if any)
    if (!platform::windows && actual.StartsWith(_T("lib")))
        actual.Remove(0, 3);

    actual = ConfigManager::LocateDataFile(actual, sdPluginsUser | sdDataUser | sdPluginsGlobal | sdDataGlobal);
    if (actual.IsEmpty())
    {
        Manager::Get()->GetLogManager()->LogError(_T("Plugin resource not found: ") + fname.GetFullName());
        return; // not found
    }

    // load XML from ZIP
    wxString contents;
    wxFileSystem* fs = new wxFileSystem;
    wxFSFile* f = fs->OpenFile(actual + _T("#zip:manifest.xml"));
    if (f)
    {
        wxInputStream* is = f->GetStream();
        char tmp[1024] = {};
        while (!is->Eof() && is->CanRead())
        {
            memset(tmp, 0, sizeof(tmp));
            is->Read(tmp, sizeof(tmp) - 1);
            contents << cbC2U((const char*)tmp);
        }
        delete f;
    }
    else
    {
        Manager::Get()->GetLogManager()->LogError(_T("No plugin manifest file in resource: ") + actual);
        delete fs;
        return;
    }
    delete fs;

    // actually load XML document
    TiXmlDocument doc;
    if (!doc.Parse(cbU2C(contents)))
        return;

    TiXmlElement* root = doc.FirstChildElement("CodeBlocks_plugin_manifest_file");
    if (!root)
        return;

    TiXmlElement* extra = root->FirstChildElement("Extra");
    while (extra)
    {
        const char* file = extra->Attribute("file");
        if (file && *file)
        {
            extraFiles.Add(cbC2U(file));
        }

        extra = extra->NextSiblingElement("Extra");
    }
}

int PluginManager::ScanForScriptPlugins(const wxString& path)
{
    static const wxString PluginMask = _T("*.splugin");

    int count = 0;
    if (!wxDirExists(path))
        return count;

    wxDir dir(path);

    if (!dir.IsOpened())
        return count;

    wxString filename;
    wxString failed;
    bool ok = dir.GetFirst(&filename, PluginMask, wxDIR_FILES);
    while(ok)
    {
        if(LoadScriptPlugin(filename))
        {
            count++;
        }
        else
        {
            Manager::Get()->GetLogManager()->LogWarning(wxString(_T("Failed to load script plugin:")) + filename);
            InfoWindow::Display(_("Warning"),wxString(_T("Failed to load script plugin:")) + filename,15000, 3000);
        }
        ok = dir.GetNext(&filename);
    }

    Manager::Get()->GetLogManager()->Log(F(_("Loaded %d script plugins"), count));

    return count;
}

int PluginManager::ScanForPlugins(const wxString& path)
{
    static const wxString PluginsMask = platform::windows                      ? _T("*.dll")
                                        : (platform::darwin || platform::macosx) ? _T("*.dylib")
                                        :                                          _T("*.so");
    int count = 0;
    if (!wxDirExists(path))
        return count;
    wxDir dir(path);

    if (!dir.IsOpened())
        return count;

    bool batch = Manager::IsBatchBuild();
    wxArrayString bbplugins;
    if (batch)
    {
        ConfigManager *bbcfg = Manager::Get()->GetConfigManager(_T("plugins"));
        bbplugins = bbcfg->ReadArrayString(_T("/batch_build_plugins"));
        if (!bbplugins.GetCount())
        {
            // defaults
            if      (platform::windows)
                bbplugins.Add(_T("compiler.dll"));
            else if (platform::darwin || platform::macosx)
                bbplugins.Add(_T("libcompiler.dylib"));
            else
                bbplugins.Add(_T("libcompiler.so"));
        }
    }

    wxString filename;
    wxString failed;
    bool ok = dir.GetFirst(&filename, PluginsMask, wxDIR_FILES);
    while (ok)
    {
        if (batch)
        {
            // for batch builds, we will load only those plugins that the
            // user has set (default only compiler.dll)
            bool matched = false;
            for (size_t i = 0; i < bbplugins.GetCount(); ++i)
            {
                if (bbplugins[i] == filename)
                {
                    matched = true;
                    break;
                }
            }
            if (!matched)
            {
                ok = dir.GetNext(&filename);
                continue;
            }
        }

        // load manifest
        m_pCurrentlyLoadingManifestDoc = nullptr;
        if (ReadManifestFile(filename))
        {
            if (LoadPlugin(path + wxFILE_SEP_PATH + filename))
                ++count;
            else
                failed << _T('\n') << filename;
        }
        if (m_pCurrentlyLoadingManifestDoc)
        {
            delete m_pCurrentlyLoadingManifestDoc;
            m_pCurrentlyLoadingManifestDoc = nullptr;
        }
        ok = dir.GetNext(&filename);
    }
    Manager::Get()->GetLogManager()->Log(F(_("Loaded %d plugins"), count));
    if (!failed.IsEmpty())
    {
        InfoWindow::Display(_("Warning"),
                            _("One or more plugins were not loaded.\n"
                              "This usually happens when a plugin is built for\n"
                              "a different version of the Code::Blocks SDK.\n"
                              "Check the application log for more info.\n\n"
                              "List of failed plugins:\n") + failed,
                            15000, 3000);
    }

    ScanForScriptPlugins(path);

    return count;
}

bool PluginManager::LoadScriptPlugin(const wxString& pluginName)
{
    wxFileName file(pluginName);
    if(file.GetExt() != _("splugin"))
        return false;

    wxString path = ConfigManager::LocateDataFile(pluginName, sdPluginsGlobal | sdPluginsUser);

    return Manager::Get()->GetScriptingManager()->LoadScript(path);
}

bool PluginManager::LoadPlugin(const wxString& pluginName)
{
    // clear registration temporary vector
    m_RegisteredPlugins.clear();

    // load library
    m_CurrentlyLoadingFilename = pluginName;
    m_pCurrentlyLoadingLib = LibLoader::LoadLibrary(pluginName);
    if (!m_pCurrentlyLoadingLib->IsLoaded())
    {
        Manager::Get()->GetLogManager()->LogError(F(_T("%s: not loaded (missing symbols?)"), pluginName.wx_str()));
        LibLoader::RemoveLibrary(m_pCurrentlyLoadingLib);
        m_pCurrentlyLoadingLib = nullptr;
        m_CurrentlyLoadingFilename.Clear();
        return false;
    }

    // by now, the library has loaded and its global variables are initialized.
    // this means it has already called RegisterPlugin()
    // now we can actually create the plugin(s) instance(s) :)
    // try to load the plugin(s)

    std::vector<PluginRegistration>::iterator it;
    for (it = m_RegisteredPlugins.begin(); it != m_RegisteredPlugins.end(); ++it)
    {
        PluginRegistration& pr = *it;
        cbPlugin* plug = nullptr;
        try
        {
            plug = pr.createProc();
        }
        catch (cbException& exception)
        {
            exception.ShowErrorMessage(false);
            continue;
        }

        // all done; add it to our list
        PluginElement* plugElem = new PluginElement;
        plugElem->fileName = m_CurrentlyLoadingFilename;
        plugElem->info = pr.info;
        plugElem->library = m_pCurrentlyLoadingLib;
        plugElem->freeProc = pr.freeProc;
        plugElem->plugin = plug;
        m_Plugins.Add(plugElem);

        SetupLocaleDomain(pr.name);

        Manager::Get()->GetLogManager()->DebugLog(F(_T("%s: loaded"), pr.name.wx_str()));
    }

    if (m_RegisteredPlugins.empty())
    {
        // no plugins loaded from this library, but it's not an error
        LibLoader::RemoveLibrary(m_pCurrentlyLoadingLib);
    }
    m_pCurrentlyLoadingLib = nullptr;
    m_CurrentlyLoadingFilename.Clear();
    return true;
}

void PluginManager::LoadAllPlugins()
{
    // check if a plugin crashed the app last time
    wxString probPlugin = Manager::Get()->GetConfigManager(_T("plugins"))->Read(_T("/try_to_activate"), wxEmptyString);
    if (!probPlugin.IsEmpty())
    {
        wxString msg;
        msg.Printf(_("Plugin \"%s\" failed to load last time Code::Blocks was executed.\n"
                     "Do you want to disable this plugin from loading?"), probPlugin.c_str());
        if (cbMessageBox(msg, _("Warning"), wxICON_WARNING | wxYES_NO) == wxID_NO)
            probPlugin = _T("");
    }

    PluginElement* elem = nullptr;
    for (unsigned int i = 0; i < m_Plugins.GetCount(); ++i)
    {
        elem = m_Plugins[i];
        cbPlugin* plug = elem->plugin;
        if (!plug || plug->IsAttached())
            continue;

        // do not load it if the user has explicitly asked not to...
        wxString baseKey;
        baseKey << _T("/") << elem->info.name;
        bool loadIt = Manager::Get()->GetConfigManager(_T("plugins"))->ReadBool(baseKey, true);

        // if we have a problematic plugin, check if this is it
        if (loadIt && !probPlugin.IsEmpty())
        {
            loadIt = elem->info.title != probPlugin;
            // if this is the problematic plugin, don't load it
            if (!loadIt)
                Manager::Get()->GetConfigManager(_T("plugins"))->Write(baseKey, false);
        }

        if (loadIt)
        {
            Manager::Get()->GetConfigManager(_T("plugins"))->Write(_T("/try_to_activate"), elem->info.title);
            Manager::Get()->GetLogManager()->Log(elem->info.name);
            try
            {
                AttachPlugin(plug);
                Manager::Get()->GetConfigManager(_T("plugins"))->Write(_T("/try_to_activate"), wxEmptyString, false);
            }
            catch (cbException& exception)
            {
                Manager::Get()->GetLogManager()->Log(_T("[failed]"));
                exception.ShowErrorMessage(false);

                wxString msg;
                msg.Printf(_("Plugin \"%s\" failed to load...\n"
                             "Do you want to disable this plugin from loading next time?"), elem->info.title.c_str());
                if (cbMessageBox(msg, _("Warning"), wxICON_WARNING | wxYES_NO) == wxID_YES)
                    Manager::Get()->GetConfigManager(_T("plugins"))->Write(baseKey, false);
            }
        }
    }
    Manager::Get()->GetConfigManager(_T("plugins"))->Write(_T("/try_to_activate"), wxEmptyString, false);
}

void PluginManager::UnloadAllPlugins()
{
//    Manager::Get()->GetLogManager()->DebugLog("Count %d", m_Plugins.GetCount());

    while (m_Plugins.GetCount())
    {
        UnloadPlugin(m_Plugins[0]->plugin);
    }
    m_Plugins.Clear();
    LibLoader::Cleanup();
}

void PluginManager::UnloadPlugin(cbPlugin* plugin)
{
    if (!plugin)
        return;

    // detach plugin if needed
    DetachPlugin(plugin);

    // find plugin element
    for (unsigned int i = 0; i < m_Plugins.GetCount(); ++i)
    {
        PluginElement* plugElem = m_Plugins[i];
        if (plugElem->plugin == plugin)
        {
            // found
            // free plugin
            if (plugElem->freeProc)
                plugElem->freeProc(plugin);
            else
                delete plugin; // try to delete it ourselves...
            // remove lib
            LibLoader::RemoveLibrary(plugElem->library);
            // and delete plugin element
            delete plugElem;
            m_Plugins.RemoveAt(i);

            break;
        }
    }
}

PluginElement* PluginManager::FindElementByName(const wxString& pluginName)
{
    for (unsigned int i = 0; i < m_Plugins.GetCount(); ++i)
    {
        PluginElement* plugElem = m_Plugins[i];
        if (plugElem->info.name == pluginName)
            return plugElem;
    }

    return nullptr;
}

cbPlugin* PluginManager::FindPluginByName(const wxString& pluginName)
{
    for (unsigned int i = 0; i < m_Plugins.GetCount(); ++i)
    {
        PluginElement* plugElem = m_Plugins[i];
        if (plugElem->info.name == pluginName)
            return plugElem->plugin;
    }

    return nullptr;
}

cbPlugin* PluginManager::FindPluginByFileName(const wxString& pluginFileName)
{
    for (unsigned int i = 0; i < m_Plugins.GetCount(); ++i)
    {
        PluginElement* plugElem = m_Plugins[i];
        if (plugElem->fileName == pluginFileName)
            return plugElem->plugin;
    }

    return nullptr;
}

const PluginInfo* PluginManager::GetPluginInfo(const wxString& pluginName)
{
    PluginElement* plugElem = FindElementByName(pluginName);
    if (plugElem && plugElem->info.name == pluginName)
        return &plugElem->info;

    return nullptr;
}

const PluginInfo* PluginManager::GetPluginInfo(cbPlugin* plugin)
{
    for (unsigned int i = 0; i < m_Plugins.GetCount(); ++i)
    {
        PluginElement* plugElem = m_Plugins[i];
        if (plugElem->plugin == plugin)
            return &plugElem->info;
    }

    return nullptr;
}

int PluginManager::ExecutePlugin(const wxString& pluginName)
{
    PluginElement* elem = FindElementByName(pluginName);
    cbPlugin* plug = elem ? elem->plugin : nullptr;
    if (plug)
    {
        if (plug->GetType() != ptTool)
        {
            Manager::Get()->GetLogManager()->LogError(F(_T("Plugin %s is not a tool to have Execute() method!"), elem->info.name.wx_str()));
        }
        else
        {
            try
            {
                return ((cbToolPlugin*)plug)->Execute();
            }
            catch (cbException& exception)
            {
                exception.ShowErrorMessage(false);
            }
        }
    }
    else
    {
        Manager::Get()->GetLogManager()->LogError(F(_T("No plugin registered by this name: %s"), pluginName.wx_str()));
    }
    return 0;
}

inline int SortByConfigurationPriority(cbPlugin** first, cbPlugin** second)
{
    return (*first)->GetConfigurationPriority() - (*second)->GetConfigurationPriority();
}

void PluginManager::GetConfigurationPanels(int group, wxWindow* parent, ConfigurationPanelsArray& arrayToFill)
{
    // build an array of Plugins* because we need to order it by configuration priority
    PluginsArray arr;
    for (unsigned int i = 0; i < m_Plugins.GetCount(); ++i)
    {
        cbPlugin* plug = m_Plugins[i]->plugin;
        // all check are done here
        if (plug && plug->IsAttached() && (plug->GetConfigurationGroup() & group))
            arr.Add(plug);
    }

    // sort the array
    arr.Sort(SortByConfigurationPriority);

    // now enumerate the array and fill the supplied configurations panel array
    arrayToFill.Clear();
    for (unsigned int i = 0; i < arr.GetCount(); ++i)
    {
        cbPlugin* plug = arr[i];
        cbConfigurationPanel* pnl = plug->GetConfigurationPanel(parent);
        if (pnl)
            arrayToFill.Add(pnl);
    }
}

void PluginManager::GetProjectConfigurationPanels(wxWindow* parent, cbProject* project, ConfigurationPanelsArray& arrayToFill)
{
    for (unsigned int i = 0; i < m_Plugins.GetCount(); ++i)
    {
        cbPlugin* plug = m_Plugins[i]->plugin;
        if (plug && plug->IsAttached())
        {
            cbConfigurationPanel* pnl = plug->GetProjectConfigurationPanel(parent, project);
            if (pnl)
                arrayToFill.Add(pnl);
        }
    }
}

PluginsArray PluginManager::GetToolOffers()
{
    return GetOffersFor(ptTool);
}

PluginsArray PluginManager::GetMimeOffers()
{
    return GetOffersFor(ptMime);
}

PluginsArray PluginManager::GetCompilerOffers()
{
    return GetOffersFor(ptCompiler);
}

PluginsArray PluginManager::GetDebuggerOffers()
{
    return GetOffersFor(ptDebugger);
}

PluginsArray PluginManager::GetCodeCompletionOffers()
{
    return GetOffersFor(ptCodeCompletion);
}

PluginsArray PluginManager::GetSmartIndentOffers()
{
    return GetOffersFor(ptSmartIndent);
}

PluginsArray PluginManager::GetOffersFor(PluginType type)
{
    PluginsArray arr;

    // special case for MIME plugins
    // we 'll add the default MIME handler, last in the returned array
    cbPlugin* dflt = nullptr;

    for (unsigned int i = 0; i < m_Plugins.GetCount(); ++i)
    {
        cbPlugin* plug = m_Plugins[i]->plugin;
        if (plug && plug->IsAttached() && plug->GetType() == type)
        {
            if (type == ptMime)
            {
                // default MIME handler?
                if (((cbMimePlugin*)plug)->HandlesEverything())
                    dflt = plug;
                else
                    arr.Add(plug);
            }
            else
                arr.Add(plug);
        }
    }

    // add default MIME handler last
    if (dflt)
        arr.Add(dflt);

    return arr;
}

void PluginManager::AskPluginsForModuleMenu(const ModuleType type, wxMenu* menu, const FileTreeData* data)
{
    for (unsigned int i = 0; i < m_Plugins.GetCount(); ++i)
    {
        cbPlugin* plug = m_Plugins[i]->plugin;
        if (plug && plug->IsAttached())
        {
            try
            {
                plug->BuildModuleMenu(type, menu, data);
            }
            catch (cbException& exception)
            {
                exception.ShowErrorMessage(false);
            }
        }
    }

    Manager::Get()->GetScriptingManager()->CreateModuleMenu(type,menu,data);

}


cbMimePlugin* PluginManager::GetMIMEHandlerForFile(const wxString& filename)
{
    PluginsArray mimes = GetMimeOffers();
    for (unsigned int i = 0; i < mimes.GetCount(); ++i)
    {
        cbMimePlugin* plugin = (cbMimePlugin*)mimes[i];
        if (plugin && plugin->CanHandleFile(filename))
            return plugin;
    }
    return nullptr;
}

int PluginManager::Configure()
{
    PluginsConfigurationDlg dlg(Manager::Get()->GetAppWindow());
    PlaceWindow(&dlg);
    return dlg.ShowModal();
}

void PluginManager::SetupLocaleDomain(const wxString& DomainName)
{
    int catalogNum=Manager::Get()->GetConfigManager(_T("app"))->ReadInt(_T("/locale/catalogNum"),(int)0);
    int i = 1;
    for (; i <= catalogNum; ++i)
    {
        wxString catalogName=Manager::Get()->GetConfigManager(_T("app"))->Read(wxString::Format(_T("/locale/Domain%d"), i), wxEmptyString);
        if (catalogName.Cmp(DomainName) == 0)
            break;
    }
    if (i > catalogNum)
    {
        ++catalogNum;
        Manager::Get()->GetConfigManager(_T("app"))->Write(_T("/locale/catalogNum"), (int)catalogNum);
        Manager::Get()->GetConfigManager(_T("app"))->Write(wxString::Format(_T("/locale/Domain%d"), i), DomainName);
    }
}

void PluginManager::NotifyPlugins(CodeBlocksEvent& event)
{
    Manager::Get()->ProcessEvent(event);
}

void PluginManager::NotifyPlugins(CodeBlocksDockEvent& event)
{
    Manager::Get()->ProcessEvent(event);
}

void PluginManager::NotifyPlugins(CodeBlocksLayoutEvent& event)
{
    Manager::Get()->ProcessEvent(event);
}

