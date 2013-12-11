/*
 * This file is part of the Code::Blocks IDE and licensed under the GNU Lesser General Public License, version 3
 * http://www.gnu.org/licenses/lgpl-3.0.html
 *
 * $Revision$
 * $Id$
 * $HeadURL$
 */

#include <sdk_precomp.h>
#include "scripting/bindings/sc_plugin.h"
#include <manager.h>
#include <scriptingmanager.h>
#include <wx/menu.h>
#include <scripting/bindings/sq_wx/sq_wx_dialog.h>
#include <wx/fs_mem.h>

#include <map>

namespace ScriptBindings
{

SQInteger CreateWxDialog(HSQUIRRELVM vm)
{
    StackHandler sa(vm);
    HSQOBJECT obj;
    sq_getstackobj(vm,2,&obj);
    Sqrat::Object o(obj,vm);
    cbScriptPlugin* cb_plugin = GetPluginFromObject(sa,obj);

    if(cb_plugin == nullptr)
        return SC_RETURN_FAILED;

    sq_wxDialog* diag = new sq_wxDialog(vm);

    cb_plugin->RegisterWxWindow(diag);
    sa.PushInstance(diag);
    return SC_RETURN_VALUE;
}

SQInteger CreateWxFrame(HSQUIRRELVM vm)
{
    StackHandler sa(vm);
    return sa.ThrowError(_("CreateWxFrame: Not implemented"));
}

cbScriptPlugin::cbScriptPlugin(Sqrat::Object obj) : m_AttachedToMainWindow(false),
    m_resource_counter(0),
    m_menu_manager(true),             // Destroy the menu if this plugin is removed...
    m_object(obj),
    m_script_file(wxEmptyString)

{

    // TODO register the RegisterCBEvent function here.

}

cbScriptPlugin::~cbScriptPlugin()
{
    cb_man_window_list::iterator itr;

    for(itr = m_window_list.begin(); itr != m_window_list.end();++itr)
    {
        (*itr)->Destroy();
    }
    m_window_list.clear();

    Manager::Get()->RemoveAllEventSinksFor(this);
    if(m_AttachedToMainWindow )
    {
        if(this->GetPreviousHandler() != nullptr || this->GetNextHandler() != nullptr)
        {
            Manager::Get()->GetAppWindow()->RemoveEventHandler(this);
            m_AttachedToMainWindow = false;
        }
        else
        {
            // so this is strange.. we have a m_AttachedToMainWindow==true, but no registered event handler...
            //Manager::Get()->GetLogManager()->LogWarning(_("Scripting error: Could not find any EventHandler to remove from the plugin \"") + GetName() + _("\". Please report this to the developer") );
        }
    }

    m_menu_manager.Clear();

    UnloadResources();

}

void cbScriptPlugin::RegisterWxWindow(cb_wxBaseManagedWindowInterface* window)
{
    m_window_list.push_front(window);
}

void cbScriptPlugin::OnMenu(wxMenuEvent &evt)
{
    if(wxGetKeyState(WXK_SHIFT))
    {
        // The sift key is pressed. We should now open the script in an editor window...
        Manager::Get()->GetEditorManager()->Open(GetScriptFile());
        return;
    }
    cb_menu_id_to_idx::iterator itr =  m_menu_to_idx_map.find(evt.GetId());
    if(itr ==m_menu_to_idx_map.end())
    {
        //Wrong menu id
        // could not find any corresponding index
        Manager::Get()->GetLogManager()->LogError(_("Error in cbScriptPlugin::OnMenu: itr == m_menu_to_idx_map.end()"));
        return;
    }

    Sqrat::Function func(m_object,"OnMenuClicked");
    if (!func.IsNull())
    {
        func(itr->second);
        if(Manager::Get()->GetScriptingManager()->DisplayErrors())
        {
            //Error
            Manager::Get()->GetLogManager()->LogError(_("Scripting error in cbScriptPlugin::OnMenu!"));
        }
    }
}

void cbScriptPlugin::OnModulMenu(wxMenuEvent &evt)
{
    if(wxGetKeyState(WXK_SHIFT))
    {
        // The sift key is pressed. We should now open the script in an editor window...
        Manager::Get()->GetEditorManager()->Open(GetScriptFile());
        return;
    }
    cb_menu_id_to_idx::iterator itr =  m_modul_menu_to_idx_map.find(evt.GetId());
    if(itr ==m_modul_menu_to_idx_map.end())
    {
        //Wrong menu id
        // could not find any corresponding index
    }

    Sqrat::Function func(m_object,"OnModuleMenuClicked");
    if (!func.IsNull())
    {
        func(itr->second);
        if(Manager::Get()->GetScriptingManager()->DisplayErrors())
        {
            //Error
        }
    }
}



void cbScriptPlugin::OnCBEvt(CodeBlocksEvent& evt)
{
    cb_evt_func_map::iterator itm = m_cb_evt_map.find(evt.GetEventType());

    if (itm == m_cb_evt_map.end())
        return; // not a registered event?

    Sqrat::Function func(m_object,itm->second.mb_str());
    if (!func.IsNull())
    {
        func(evt);
        Manager::Get()->GetScriptingManager()->DisplayErrors();
    }
    else
    {
        // Could not find the registered event callback
        Manager::Get()->GetLogManager()->LogWarning(_("Scripting error: Could not find event callback \"") + itm->second + _("\" in ") + GetName() );
    }
}


int cbScriptPlugin::RegisterCBEvent(wxEventType evt, wxString func)
{
    cb_evt_func_map::iterator itm = m_cb_evt_map.find(evt);
    if (itm == m_cb_evt_map.end())
    {
        itm = m_cb_evt_map.insert(m_cb_evt_map.end(), std::make_pair(evt, func));
        Manager::Get()->RegisterEventSink(evt, new cbEventFunctor<cbScriptPlugin, CodeBlocksEvent>(this, &cbScriptPlugin::OnCBEvt));
    }
    else
    {
        itm->second = func;
    }
    return 0;
}

int cbScriptPlugin::CreateMenus()
{
    Sqrat::Function func(m_object,"GetMenu");
    if (func.IsNull())
        return 0;       // This plugin does not need menus

    wxArrayString menu_arr;
    Sqrat::SharedPtr<wxArrayString> sh_ret = func.Evaluate<wxArrayString>();
    if(!sh_ret)
    {
        Sqrat::Error::Clear(m_object.GetVM());
        return 0;
    }

    menu_arr = *(sh_ret.Get());
    if(Manager::Get()->GetScriptingManager()->DisplayErrors())
    {
        return 0;
    }

    if (menu_arr.GetCount())
    {
        if(m_AttachedToMainWindow == false)
        {
            Manager::Get()->GetAppWindow()->PushEventHandler(this);
            m_AttachedToMainWindow = true;
        }
        for (size_t i = 0; i < menu_arr.GetCount(); ++i)
        {
            int id = wxNewId();
            id = m_menu_manager.CreateFromString(menu_arr[i], id);
            if(id == 0)
            {
                Manager::Get()->GetLogManager()->LogWarning(_("Could not create menu \"") + menu_arr[i] +_("\" in script ")+ GetName());
                continue;
            }

            wxMenuItem* item = Manager::Get()->GetAppFrame()->GetMenuBar()->FindItem(id);
            if (item)
            {
                item->SetHelp(_("Press SHIFT while clicking this menu item to edit the assigned script in the editor"));

                Connect(id, wxEVT_COMMAND_MENU_SELECTED,
                        (wxObjectEventFunction) (wxEventFunction) (wxCommandEventFunction)
                        &cbScriptPlugin::OnMenu,nullptr,this);

                Manager::Get()->GetLogManager()->Log(_("Registered event for menu \"") +
                                                     menu_arr[i] + _("\" for script ") + GetName() + _(" with id: ") + F(_("%d"),id) );

                m_menu_to_idx_map.insert(m_menu_to_idx_map.end(), std::make_pair(id, i));

            }
        }
    }
    return 0;
}

int cbScriptPlugin::Execute()
{
    Sqrat::Function func(m_object,"Execute");
    if (!func.IsNull())
    {
        func();
        if(Manager::Get()->GetScriptingManager()->DisplayErrors())
        {
            return -1;
        }
        return 1;
    }
    return -2;
}

void cbScriptPlugin::BuildModuleMenu(cb_optional const ModuleType type, cb_optional wxMenu* menu, cb_optional const FileTreeData* data)
{
    Sqrat::Function func(m_object,"GetModuleMenu");
    if (func.IsNull())
        return;       // This plugin does not need menus

    wxArrayString menu_arr;
    Sqrat::SharedPtr<wxArrayString> sh_ret = func.Evaluate<wxArrayString>(type,data);
    if(!sh_ret)
        return;

    menu_arr = *(sh_ret.Get());
    if(Manager::Get()->GetScriptingManager()->DisplayErrors())
    {
        return;
    }

    if (menu_arr.GetCount()==1) // exactly one menu entry
    {
        int id = wxNewId();
        menu->Append(id, menu_arr[0]);

        Connect(id, -1, wxEVT_COMMAND_MENU_SELECTED,
                (wxObjectEventFunction) (wxEventFunction) (wxCommandEventFunction)
                &cbScriptPlugin::OnModulMenu);
        Manager::Get()->GetLogManager()->Log(_("Registered event for menu \"") + menu_arr[0] + _("\" for script ") + GetName());

        m_modul_menu_to_idx_map.insert(m_modul_menu_to_idx_map.end(), std::make_pair(id, 0));

    }
    else if (menu_arr.GetCount()>1) // more menu entries -> create sub-menu
    {
        wxMenu* sub = new wxMenu;
        for (size_t i = 0; i < menu_arr.GetCount(); ++i)
        {
            int id = wxNewId();
            sub->Append(id, menu_arr[i]);
            Connect(id, -1, wxEVT_COMMAND_MENU_SELECTED,
                    (wxObjectEventFunction) (wxEventFunction) (wxCommandEventFunction)
                    &cbScriptPlugin::OnModulMenu);

            Manager::Get()->GetLogManager()->Log(_("Registered event for menu \"") + menu_arr[0] + _("\" for script ") + GetName());

            m_modul_menu_to_idx_map.insert(m_modul_menu_to_idx_map.end(), std::make_pair(id, i));

        }
        menu->Append(-1, GetName() , sub);
    }
}


bool cbScriptPlugin::LoadResource(wxString name)
{
    wxString LoadedResource = Manager::Get()->FindAndLoadResource(name);
    if(LoadedResource == wxEmptyString)
        return false;

    m_LoadedResources.Add(LoadedResource);
    return true;
}

bool cbScriptPlugin::LoadResourceDirect(wxString path)
{
    wxXmlResource::Get()->Load(path);
    m_LoadedResources.Add(path);
    return true;
}

bool cbScriptPlugin::UnloadResources()
{
    bool ret = true;
    for(unsigned int i = 0; i < m_LoadedResources.size();i++)
    {
        ret &= wxXmlResource::Get()->Unload(m_LoadedResources[i]);
        wxString type = m_LoadedResources[i].BeforeFirst(':');
        wxString path = m_LoadedResources[i].AfterFirst(':');
        if(type == _T("memory"))
        {
            wxMemoryFSHandler::RemoveFile(path);
        }
    }
    m_LoadedResources.clear();
    return ret;
}

cbScriptPlugin* GetPluginFromObject(StackHandler& sa,Sqrat::Object obj)
{
    Sqrat::Function func(obj,"GetPluginInfo");
    if (func.IsNull())
    {
        sa.ThrowError(_("GetPluginFromObject: the object is not a plugin (the GetPluginInfo() is missing)") + wxString::FromUTF8(Sqrat::Error::Message(sa.GetVM()).c_str()));
        return nullptr;
    }

    Sqrat::SharedPtr<PluginInfo> sh_ret = func.Evaluate<PluginInfo>();
    if(!sh_ret)
    {
        sa.ThrowError(_("GetPluginFromObject: Could not find: ") + wxString::FromUTF8(Sqrat::Error::Message(sa.GetVM()).c_str()));
        return nullptr;
    }

    PluginInfo info = *(sh_ret.Get());
    cbScriptPlugin *plugin = Manager::Get()->GetScriptingManager()->GetPlugin(info.name);
    if(plugin == nullptr)
    {
        sa.ThrowError(_("GetPluginFromObject: Could not find: ") + info.name + _(" in the registered plugins"));
        return nullptr;
    }
    return plugin;
}


namespace ScriptPluginWrapper
{

/** \defgroup sq_plugin The Squirrel plugin interface
 *  \ingroup Squirrel
 *  \brief Functions to register a plugin an cb events
 *
 */

////////////////////////////////////////////////////////////////////////////////
// register a script plugin (script-bound function)
////////////////////////////////////////////////////////////////////////////////
SQInteger RegisterPlugin(HSQUIRRELVM vm)
{
    // get squirrel object to register from stack
    StackHandler sa(vm);
    HSQOBJECT obj;
    sq_getstackobj(vm,2,&obj);
    Sqrat::Object o(obj,vm);
    //o.AttachToStackObject(2);

    cbScriptPlugin* plugin = new cbScriptPlugin(o);

    Sqrat::Function func(o,"GetPluginInfo");
    // first verify that there is a member function to retrieve the plugin info
    if (func.IsNull())
    {

        delete plugin;
        return sa.ThrowError(_("Not a script plugin!: ") + wxString::FromUTF8(Sqrat::Error::Message(sa.GetVM()).c_str()));
    }


    SQStackInfos si;
    sq_stackinfos(vm,1,&si);
    plugin->SetScriptFile(wxString(si.source,wxConvUTF8));

    // ask for its registration name
    Sqrat::SharedPtr<PluginInfo> sh_ret = func.Evaluate<PluginInfo>();
    if(!sh_ret)
    {
        delete plugin;
        return sa.ThrowError(_("Error on evaluating GetPluginInfo!") + wxString::FromUTF8(Sqrat::Error::Message(sa.GetVM()).c_str()));
    }
    PluginInfo info = *(sh_ret.Get());

    plugin->SetInfo(info);
    Manager::Get()->GetScriptingManager()->RegisterScriptPlugin(plugin->GetName(), plugin);

    // this function returns nothing on the squirrel stack
    return SC_RETURN_OK;
}

bool UnRegisterPlugin(wxString name)
{
    return Manager::Get()->GetScriptingManager()->UnRegisterScriptPlugin(name);
}

////////////////////////////////////////////////////////////////////////////////
// get a script plugin squirrel object (script-bound function)
////////////////////////////////////////////////////////////////////////////////
SQInteger GetPlugin(HSQUIRRELVM v)
{
    StackHandler sa(v);

    // get the script plugin's name
    const wxString& name = *sa.GetInstance<wxString>(2);

    // search for it in the registered script plugins list
    cbScriptPlugin *plugin = Manager::Get()->GetScriptingManager()->GetPlugin(name);
    if(plugin == nullptr)
        return SC_RETURN_OK;

    sa.PushValue<HSQOBJECT>(plugin->GetObject());

    return SC_RETURN_VALUE;
}

////////////////////////////////////////////////////////////////////////////////
// execute a script plugin (script-bound function)
////////////////////////////////////////////////////////////////////////////////
int ExecutePlugin(const wxString& name)
{
    return Manager::Get()->GetScriptingManager()->ExecutePlugin(name);

}

////////////////////////////////////////////////////////////////////////////////
//Register a CB Event handler for this script
////////////////////////////////////////////////////////////////////////////////
SQInteger RegisterCBEvent(HSQUIRRELVM vm)
{
    StackHandler sa(vm);
    if(sa.GetParamCount() < 4)
        return sa.ThrowError(_("RegisterCBEvent: to few parameter"));

    HSQOBJECT obj;
    sq_getstackobj(vm,1,&obj);
    cbScriptPlugin *plugin = GetPluginFromObject(sa,Sqrat::Object(obj,vm));
    if(plugin == nullptr)
    {
        return SQ_ERROR;
    }

    wxEventType type = sa.GetValue<wxEventType>(3);
    wxString func_name = sa.GetValue<wxString>(4);

    plugin->RegisterCBEvent(type,func_name);

    return SC_RETURN_OK;
}

////////////////////////////////////////////////////////////////////////////////
//Search for a scriptfile in the Plugin Resource and compile/run it
////////////////////////////////////////////////////////////////////////////////
SQInteger IncludeScriptFromResource(HSQUIRRELVM vm)

{
    // TODO (bluehazzard#1#): Check if this plugin has the rights to access this resource
    StackHandler sa(vm);
    if(sa.GetParamCount() < 3)
        return sa.ThrowError(_("IncludeScriptFromResource: to few parameter (at least 2: name_of_script,name_of_resource_file)"));
    try
    {
        wxString ResourceFileName = sa.GetValue<wxString>(2);
        wxString ScriptName = sa.GetValue<wxString>(3);
        wxString contents;
        wxString filename = ResourceFileName + ScriptName;

        wxString path = ConfigManager::LocateDataFile(ResourceFileName, sdDataGlobal | sdDataUser);

        int ret = Manager::Get()->GetScriptingManager()->LoadFileFromZip(path,ScriptName,contents);
        if(ret != 0)
        {
            return sa.ThrowError(_("IncludeScriptFromResource: Error loading file"));
        }


        if(!Manager::Get()->GetScriptingManager()->LoadBuffer(contents,filename))
            return sa.ThrowError(_("Error while Loading file: ") + filename );

        return SC_RETURN_OK;
    } catch(CBScriptException &e)
    {

        return sa.ThrowError(_("IncludeScriptFromResource: ")+ e.Message());
    }

}

SQInteger LoadResource(HSQUIRRELVM vm)
{
    StackHandler sa(vm);
    if(sa.GetParamCount() < 3)
        return sa.ThrowError(_("LoadResource: to few parameter (at least 2: plugin_object,name_of_resource_file)"));

    HSQOBJECT obj;
    sq_getstackobj(vm,2,&obj);
    cbScriptPlugin *plugin = GetPluginFromObject(sa,Sqrat::Object(obj,vm));
    if(plugin == nullptr)
        return sa.ThrowError(_("LoadResource: Could not find the plugin from this object"));

    bool result = false;

    try
    {
        wxString name = sa.GetValue<wxString>(3);
        result = plugin->LoadResource(name);

    } catch(CBScriptException &e)
    {
        return sa.ThrowError(_("LoadResource: ")+ e.Message());
    }

    sa.PushValue(result);
    return SC_RETURN_VALUE;
}

SQInteger LoadResourceFromString(HSQUIRRELVM vm)
{
    StackHandler sa(vm);
    if(sa.GetParamCount() < 3)
        return sa.ThrowError(_("LoadResourceFromString: to few parameter (at least 2: plugin_object,resource_string)"));

    HSQOBJECT obj;
    sq_getstackobj(vm,2,&obj);
    cbScriptPlugin *plugin = GetPluginFromObject(sa,Sqrat::Object(obj,vm));
    if(plugin == nullptr)
        return sa.ThrowError(_("LoadResourceFromString: Could not find the plugin from this object"));

    bool result = false;

    try
    {
        wxString resource_string = sa.GetValue<wxString>(3);
        wxString file = plugin->GetName() + wxString::Format(_("_resource_%i"),plugin->GetLoadedResourceCounter());
        wxString memoryFile = _T("memory:") + file;

        plugin->IncLoadedResourceCounter();

        wxMemoryFSHandler::AddFile(file, resource_string.ToUTF8().data(), strlen(resource_string.ToUTF8().data()));

        result = plugin->LoadResourceDirect(memoryFile);

    } catch(CBScriptException &e)
    {
        return sa.ThrowError(_("LoadResourceFromString: ")+ e.Message());
    }

    sa.PushValue(result);
    return SC_RETURN_VALUE;
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

/**
 *  \ingroup sq_plugin
 *  \brief Function bound to squirrel:
 *
 *  ### Plugin management functions bound to squirrel
 *   | Name             | parameter                     | description     | info       |
 *   | :--------------: | :---------------------------: | :--------------:| :---------:|
 *   | ExecutePlugin    |  name                         |  search for a plugin with the _name_ and execute it |   x   |
 *   | GetPlugin        |  name                         |  return the squirrel class of the plugin _name_  |   x   |
 *   | RegisterPlugin   | cbScriptPlugin plugin         |   A instance of the script plugin to be registered  |   x   |
 *   | RegisterCBEvent  | cbScriptPlugin plugin, wxEventType type, wxString function | Register a function with the name _function_ for the _type_ event for the _plugin_ (for ex _this_) |   x   |
 *   | CreateWxDialog   | cbScriptPlugin plugin         | Create a wxDialog |   x   |
 *   | IncludeScriptFromResource  | string resource_name, string file_name  | Load, Compile and run a script from a plugin resource |   x   |
 *   | LoadResource     | plugin,resource_name          | Load a xrc resource from a resource file in the plugin directories |   x   |
 *   | LoadResourceFromString  | plugin, resource_string  | Load a xrc resource from a string |   x   |
 *
 */

////////////////////////////////////////////////////////////////////////////////
// register the script plugin framework
////////////////////////////////////////////////////////////////////////////////
void Register_ScriptPlugin(HSQUIRRELVM vm)
{
    Sqrat::RootTable(vm).Func("ExecutePlugin",&ScriptPluginWrapper::ExecutePlugin);
    Sqrat::RootTable(vm).Func("UnRegisterPlugin",&ScriptPluginWrapper::UnRegisterPlugin);
    Sqrat::RootTable(vm).SquirrelFunc("GetPlugin",&ScriptPluginWrapper::GetPlugin);
    Sqrat::RootTable(vm).SquirrelFunc("RegisterPlugin",&ScriptPluginWrapper::RegisterPlugin);
    Sqrat::RootTable(vm).SquirrelFunc("RegisterCBEvent",&ScriptPluginWrapper::RegisterCBEvent);
    Sqrat::RootTable(vm).SquirrelFunc("CreateWxDialog",&CreateWxDialog);
    Sqrat::RootTable(vm).SquirrelFunc("IncludeScriptFromResource",&ScriptPluginWrapper::IncludeScriptFromResource);
    Sqrat::RootTable(vm).SquirrelFunc("LoadResource",&ScriptPluginWrapper::LoadResource);
    Sqrat::RootTable(vm).SquirrelFunc("LoadResourceFromString",&ScriptPluginWrapper::LoadResourceFromString);

    //Sqrat::RootTable(vm).SquirrelFunc("CreateWxDialog",&ScriptPluginWrapper::CreateWxDialog);

    // load base script plugin

    // WARNING: we CANNOT use ScriptingManager::LoadBuffer() because we have reached here
    // by a call from inside ScriptingManager's constructor. This would cause an infiniteCreateWxDialog
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

