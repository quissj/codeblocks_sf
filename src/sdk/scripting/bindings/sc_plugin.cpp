/*
 * This file is part of the Code::Blocks IDE and licensed under the GNU Lesser General Public License, version 3
 * http://www.gnu.org/licenses/lgpl-3.0.html
 *
 * $Revision$
 * $Id$
 * $HeadURL$
 */

#include <sdk_precomp.h>
#include "sc_plugin.h"
#include <manager.h>
#include <scriptingmanager.h>
#include <wx/menu.h>

#include <map>

namespace ScriptBindings {
namespace ScriptPluginWrapper {

// struct and map for mapping script plugins to menu callbacks
struct MenuCallback
{
    Sqrat::Object object;
    int menuIndex;
};
typedef std::map<int, MenuCallback> ModuleMenuCallbacks;
ModuleMenuCallbacks s_MenuCallbacks;

// master list of registered script plugins
typedef std::map<wxString, Sqrat::Object> ScriptPlugins;
ScriptPlugins s_ScriptPlugins;

// list of registered script plugins menubar items
typedef std::map<wxString, MenuItemsManager> ScriptPluginsMenus;
ScriptPluginsMenus s_ScriptPluginsMenus;

////////////////////////////////////////////////////////////////////////////////
// ask the script plugin what menus to add in the menubar
// and return an integer array of the menu IDs
////////////////////////////////////////////////////////////////////////////////
wxArrayInt CreateMenu(const wxString& name)
{
    wxArrayInt ret;
    StackHandler sa(Sqrat::DefaultVM::Get());

    ScriptPlugins::iterator it = s_ScriptPlugins.find(name);
    if (it == s_ScriptPlugins.end())
        return ret;
    Sqrat::Object& o = it->second;

    ScriptPluginsMenus::iterator itm = s_ScriptPluginsMenus.find(name);
    if (itm == s_ScriptPluginsMenus.end())
    {
        itm = s_ScriptPluginsMenus.insert(s_ScriptPluginsMenus.end(), std::make_pair(name, MenuItemsManager(false)));
    }
    MenuItemsManager& mi = itm->second;

    Sqrat::Function func(o,"GetMenu");
    if (func.IsNull())
        return ret;

    wxArrayString arr;
    arr = func.Evaluate<wxArrayString>();
    if(Manager::Get()->GetScriptingManager()->DisplayErrors())
    {
        return ret;
    }

    if (arr.GetCount())
    {
        for (size_t i = 0; i < arr.GetCount(); ++i)
        {
            int id = wxNewId();
            id = mi.CreateFromString(arr[i], id);

            ret.Add(id);

            MenuCallback callback;
            callback.object = it->second;
            callback.menuIndex = i;

            ModuleMenuCallbacks::iterator mmcIt = s_MenuCallbacks.find(id);
            if (mmcIt == s_MenuCallbacks.end())
                s_MenuCallbacks.insert(s_MenuCallbacks.end(), std::make_pair(id, callback));
            else
            {
                s_MenuCallbacks.erase(mmcIt);
                s_MenuCallbacks.insert(s_MenuCallbacks.end(), std::make_pair(id, callback));
            }
        }
    }

    return ret;
}

////////////////////////////////////////////////////////////////////////////////
// ask the script plugin what items to add in the context menu
// and return an integer array of the menu IDs
////////////////////////////////////////////////////////////////////////////////
wxArrayInt CreateModuleMenu(const ModuleType typ, wxMenu* menu, const FileTreeData* data)
{
    wxArrayInt ret;
    StackHandler sa(Sqrat::DefaultVM::Get());

    ScriptPlugins::iterator it;
    for (it = s_ScriptPlugins.begin(); it != s_ScriptPlugins.end(); ++it)
    {
        Sqrat::Object& o = it->second;
        Sqrat::Function func(o,"GetModuleMenu");
        if (func.IsNull())
            continue;

        wxArrayString arr;
        arr = func.Evaluate<wxArrayString>(typ, data);
        if(Manager::Get()->GetScriptingManager()->DisplayErrors())
        {
            continue;
        }


        if (arr.GetCount()==1) // exactly one menu entry
        {
            int id = wxNewId();
            menu->Append(id, arr[0]);
            ret.Add(id);

            MenuCallback callback;
            callback.object = it->second;
            callback.menuIndex = 0;
            s_MenuCallbacks.insert(s_MenuCallbacks.end(), std::make_pair(id, callback));
        }
        else if (arr.GetCount()>1) // more menu entries -> create sub-menu
        {
            wxMenu* sub = new wxMenu;
            for (size_t i = 0; i < arr.GetCount(); ++i)
            {
                int id = wxNewId();
                sub->Append(id, arr[i]);

                ret.Add(id);

                MenuCallback callback;
                callback.object = it->second;
                callback.menuIndex = i;
                s_MenuCallbacks.insert(s_MenuCallbacks.end(), std::make_pair(id, callback));
            }
            menu->Append(-1, it->first, sub);
        }
    }
    return ret;
}

////////////////////////////////////////////////////////////////////////////////
// callback for script plugins menubar entries
////////////////////////////////////////////////////////////////////////////////
void OnScriptMenu(int id)
{
    ModuleMenuCallbacks::iterator it;
    StackHandler sa(Sqrat::DefaultVM::Get());

    it = s_MenuCallbacks.find(id);

    if (it != s_MenuCallbacks.end())
    {
        MenuCallback& callback = it->second;

        Sqrat::Function func(callback.object,"OnMenuClicked");
        if (!func.IsNull())
        {
            func(callback.menuIndex);
            if(Manager::Get()->GetScriptingManager()->DisplayErrors())
            {
                //cbMessageBox(_("In OnScriptMenu:\n") + sa.GetError(), _("Script error:"), wxICON_ERROR);
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
// callback for script plugins context menu entries
////////////////////////////////////////////////////////////////////////////////
void OnScriptModuleMenu(int id)
{
    StackHandler sa(Sqrat::DefaultVM::Get());
    ModuleMenuCallbacks::iterator it;
    it = s_MenuCallbacks.find(id);
    if (it != s_MenuCallbacks.end())
    {
        MenuCallback& callback = it->second;
        Sqrat::Function func(callback.object,"OnModuleMenuClicked");
        if (!func.IsNull())
        {
            func(callback.menuIndex);
            if(Manager::Get()->GetScriptingManager()->DisplayErrors())
            {
                //cbMessageBox(_("In OnScriptModuleMenu:\n") + sa.GetError(), _("Script error"), wxICON_ERROR);
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
// register a script plugin (script-bound function)
////////////////////////////////////////////////////////////////////////////////
SQInteger RegisterPlugin(HSQUIRRELVM vm)
{
    // get squirrel object to register from stack
    HSQOBJECT obj;
    sq_getstackobj(vm,2,&obj);
    Sqrat::Object o(obj,vm);
    //o.AttachToStackObject(2);

    Sqrat::Function func(o,"GetPluginInfo");

    // first verify that there is a member function to retrieve the plugin info
    if (func.IsNull())
        return sq_throwerror(vm, "Not a script plugin!");

    // ask for its registration name
    PluginInfo info = func.Evaluate<PluginInfo>();
    wxString s = info.name;

    // look if a script plugin with the same name already exists
    ScriptPlugins::iterator it = s_ScriptPlugins.find(s);
    if (it != s_ScriptPlugins.end())
    {
        // already exists; release the old one
        s_ScriptPlugins.erase(it);
        Manager::Get()->GetLogManager()->Log(_("Script plugin unregistered: ") + s);
    }

    // finally, register this script plugin
    it = s_ScriptPlugins.insert(s_ScriptPlugins.end(), std::make_pair(s, o));
    Manager::Get()->GetLogManager()->Log(_("Script plugin registered: ") + s);

    Manager::Get()->GetScriptingManager()->RegisterScriptPlugin(s, CreateMenu(s));

    // this function returns nothing on the squirrel stack
    return SC_RETURN_OK;
}

////////////////////////////////////////////////////////////////////////////////
// get a script plugin squirrel object (script-bound function)
////////////////////////////////////////////////////////////////////////////////
SQInteger GetPlugin(HSQUIRRELVM v)
{
    StackHandler sa(v);

    // get the script plugin's name
    //const wxString& name = *SqPlus::GetInstance<wxString,false>(v, 2);
    const wxString& name = *sa.GetInstance<wxString>(2);

    // search for it in the registered script plugins list
    ScriptPlugins::iterator it = s_ScriptPlugins.find(name);
    if (it != s_ScriptPlugins.end())
    {
        // found; return the squirrel object
        sa.PushValue<HSQOBJECT>(it->second.GetObject());
        return SC_RETURN_VALUE;
    }

    // not found; return nothing
    return SC_RETURN_OK;
}

////////////////////////////////////////////////////////////////////////////////
// execute a script plugin (script-bound function)
////////////////////////////////////////////////////////////////////////////////
int ExecutePlugin(const wxString& name)
{
    StackHandler sa(Sqrat::DefaultVM::Get());
    // look for script plugin
    ScriptPlugins::iterator it = s_ScriptPlugins.find(name);
    if (it != s_ScriptPlugins.end())
    {
        // found; execute it
        Sqrat::Object& o = it->second;
        Sqrat::Function func(o,"Execute");
        if (!func.IsNull())
        {
            func();
            if(Manager::Get()->GetScriptingManager()->DisplayErrors())
            {
                //cbMessageBox(sa.GetError(), _("Script error"), wxICON_ERROR);
            }
        }
    }
    return -1;
}

}; // namespace ScriptPluginWrapper

// base script plugin class
const char* s_cbScriptPlugin =
    "class cbScriptPlugin\n"
    "{\n"
    "    info = PluginInfo();\n"
    "    constructor()\n"
    "    {\n"
    "        info.name = _T(\"cbScriptPlugin\");\n"
    "        info.title = _T(\"Base script plugin\");\n"
    "        info.version = _T(\"0.1a\");\n"
    "        info.license = _T(\"GPL\");\n"
    "    }\n"
    "    function GetPluginInfo()\n"
    "    {\n"
    "        return info;\n"
    "    }\n"
    "    function GetMenu()\n"
    "    {\n"
    "        return wxArrayString();\n"
    "    }\n"
    "    function GetModuleMenu(who,data)\n"
    "    {\n"
    "        return wxArrayString();\n"
    "    }\n"
    "    function Execute()\n"
    "    {\n"
    "        LogDebug(info.name + _T(\"::Run() : not implemented\"));\n"
    "        return -1;\n"
    "    }\n"
    "    function OnMenuClicked(index)\n"
    "    {\n"
    "        LogDebug(info.name + _T(\": menu clicked: \") + index);\n"
    "    }\n"
    "    function OnModuleMenuClicked(index)\n"
    "    {\n"
    "        LogDebug(info.name + _T(\": module menu clicked: \") + index);\n"
    "    }\n"
    "}\n";

////////////////////////////////////////////////////////////////////////////////
// register the script plugin framework
////////////////////////////////////////////////////////////////////////////////
void Register_ScriptPlugin(HSQUIRRELVM vm)
{
    Sqrat::RootTable(vm).Func("ExecutePlugin",&ScriptPluginWrapper::ExecutePlugin);
    Sqrat::RootTable(vm).SquirrelFunc("GetPlugin",&ScriptPluginWrapper::GetPlugin);
    Sqrat::RootTable(vm).SquirrelFunc("RegisterPlugin",&ScriptPluginWrapper::RegisterPlugin);

    // load base script plugin

    // WARNING: we CANNOT use ScriptingManager::LoadBuffer() because we have reached here
    // by a call from inside ScriptingManager's constructor. This would cause an infinite
    // loop and the app would die with a stack overflow. We got to load the script manually...
    // we also have to disable the printfunc for a while

    SQPRINTFUNCTION oldPrintFunc = sq_getprintfunc(vm);
    SQPRINTFUNCTION oldErrorFunc = sq_geterrorfunc(vm);
    sq_setprintfunc(vm, 0,0);

    // compile and run script
    StackHandler sa(vm);
    Sqrat::Script script(vm);

    //cript = SquirrelVM::CompileBuffer(s_cbScriptPlugin, "cbScriptPlugin");
    //SquirrelVM::RunScript(script);
    script.CompileString(s_cbScriptPlugin,"PluginBaseScript (in source code)");
    script.Run();
    if(sa.HasError()/*Manager::Get()->GetScriptingManager()->DisplayErrors()*/)
    {
        // TODO (bluehazzard#1#): DisplayErrors from ScriptingManager causes a infinite loop
        cbMessageBox(wxString::Format(_("Failed to register script plugins framework.\n\n")) + sa.GetError(),
                    _("Script compile error"),
                    wxICON_ERROR);
    }

    // restore the printfunc
    sq_setprintfunc(vm, oldPrintFunc,oldErrorFunc);
}

}; // namespace ScriptBindings

