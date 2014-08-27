
#include <scripting/bindings/sq_wx/sq_wx_propgrid.h>
#include <scripting/bindings/sq_wx/sq_wx_type_handler.h>

#define PROPGRID_LABEL_NAME     "name"
#define PROPGRID_LABEL_LABEL    "label"
#define PROPGRID_LABEL_VALUE    "value"
#define PROPGRID_LABEL_TYPE     "type"
#define PROPGRID_LABEL_CHILDREN     "children"
#define PROPGRID_LABEL_STYLE        "style"
#define PROPGRID_LABEL_SELECTION    "selection"

namespace ScriptBindings
{
    int sq_wx_propgrid_wrapper::Populate(Sqrat::Table table)
    {
        Sqrat::Table::iterator itr;
        StackHandler sa(table.GetVM());
        while(table.Next(itr))
        {
            HSQOBJECT key = itr.getKey();


            const SQChar *tmp_key_name = sq_objtostring(&key);
            wxString  name = wxString::FromUTF8(tmp_key_name);

            Sqrat::Table entry(itr.getValue(),table.GetVM());

            int ret = 0;
            if(entry.HasKey(PROPGRID_LABEL_NAME))
                ret = entry.GetValue<wxString>(PROPGRID_LABEL_NAME,name);

            if(ret < 0)
                return sa.ThrowError(wxString::Format(_("Could not find attribute \"%s\" to populate the property grid"),PROPGRID_LABEL_NAME));;


            wxString label;
            ret = entry.GetValue<wxString>(PROPGRID_LABEL_LABEL,label);
            if(ret < 0)
                return sa.ThrowError(wxString::Format(_("Could not find attribute \"%s\" to populate the property grid"),PROPGRID_LABEL_LABEL));

            int type = 0;
            ret = entry.GetValue<int>(PROPGRID_LABEL_TYPE,type);
            if(ret < 0)
                return sa.ThrowError(wxString::Format(_("Could not find attribute \"%s\" to populate the property grid"),PROPGRID_LABEL_TYPE));


            wxPGProperty* parent = NULL;
            parent = CreateEntry(entry,name,label,type);
            if(parent == NULL)
                return sa.ThrowError(wxString::Format(_("Could not create property with Type: %d"),type));

            parent = m_grid->Append(parent);

            Sqrat::Table children;
            ret = entry.GetValue<Sqrat::Table>(PROPGRID_LABEL_CHILDREN,children);
            if(ret != -1)
            {
                ret = AddChildren(children,parent);
                if (ret < 0)
                    return ret;
            }
        }

        return 0;
    }

    wxPGProperty* sq_wx_propgrid_wrapper::CreateEntry(Sqrat::Table entry,wxString name, wxString label, int type)
    {
        int style = 0;
        int ret = 0;
        ret = entry.GetValue<int>(PROPGRID_LABEL_STYLE,style);
        wxPGProperty* prop = NULL;
        switch(type)
        {
            case P_TYPE_BOOL:
                {
                    bool value = false;
                    ret = entry.GetValue<bool>(PROPGRID_LABEL_VALUE,value);
                    prop = new wxBoolProperty(label,name,value);
                    if(style & 0x01)
                       prop->SetAttribute(wxPG_BOOL_USE_CHECKBOX,true);

                }
                break;
            case P_TYPE_STRING:
                {
                    wxString value;
                    ret = entry.GetValue<wxString>(PROPGRID_LABEL_VALUE,value);
                    prop = new wxStringProperty(label,name,value);
                    //if(style & 0x01)
                        //prop->SetAttribute(wxTE_PASSWORD,true);

                }
                break;
            case P_TYPE_INT:
                {
                    int value;
                    ret = entry.GetValue<int>(PROPGRID_LABEL_VALUE,value);
                    prop = new wxIntProperty(label,name,value);

                }
                break;
            case P_TYPE_SELECTION:
                {
                    wxString value;
                    Sqrat::Array selection(entry.GetVM());
                    ret = entry.GetValue<wxString>(PROPGRID_LABEL_VALUE,value);
                    ret = entry.GetValue<Sqrat::Array>(PROPGRID_LABEL_SELECTION,selection);
                    size_t size = selection.GetSize();
                    wxArrayString arr;
                    for(size_t i = 0; i < size;i++)
                    {
                        wxString tmp;
                        selection.GetElement(i,tmp);
                        arr.Add(tmp);
                    }
                    prop = new wxEnumProperty(label,name,arr);
                }
                break;
            case P_TYPE_LABEL:
                {
                    wxString value;
                    ret = entry.GetValue<wxString>(PROPGRID_LABEL_VALUE,value);
                    prop = new wxPropertyCategory(label,name);

                }
                break;
            default:
            {
                    return NULL;
            }

        }
        return prop;
    }

    int sq_wx_propgrid_wrapper::AddChildren(Sqrat::Table table,wxPGProperty* parent)
    {
        if(parent == NULL)
            return -1;

        StackHandler sa(table.GetVM());

        Sqrat::Table::iterator itr;
        while(table.Next(itr))
        {
            HSQOBJECT key = itr.getKey();
            const SQChar *tmp_key_name = sq_objtostring(&key);
            wxString name = wxString::FromUTF8(tmp_key_name);

            Sqrat::Table entry(itr.getValue(),table.GetVM());

            int ret = 0;
            if(entry.HasKey(PROPGRID_LABEL_NAME))
                ret = entry.GetValue<wxString>(PROPGRID_LABEL_NAME,name);
            if(ret < 0)
                return sa.ThrowError(wxString::Format(_("Could not find attribute \"%s\" to populate the property grid"),PROPGRID_LABEL_NAME));

            wxString label;
            ret = entry.GetValue<wxString>(PROPGRID_LABEL_LABEL,label);
            if(ret < 0)
                return sa.ThrowError(wxString::Format(_("Could not find attribute \"%s\" to populate the property grid"),PROPGRID_LABEL_LABEL));

            int type = 0;
            ret = entry.GetValue<int>(PROPGRID_LABEL_TYPE,type);
            if(ret < 0)
                return sa.ThrowError(wxString::Format(_("Could not find attribute \"%s\" to populate the property grid"),PROPGRID_LABEL_TYPE));

            wxPGProperty* prop = NULL;
            wxPGProperty* parent_parent = NULL;
            prop = CreateEntry(entry,name,label,type);
            if(prop == NULL)
            {
                StackHandler sa(table.GetVM());
                return sa.ThrowError(wxString::Format(_("Could not create property with Type: %d"),type));

            }
            parent_parent = m_grid->AppendIn(parent,prop);
            Sqrat::Table children;
            ret = entry.GetValue<Sqrat::Table>(PROPGRID_LABEL_CHILDREN,children);
            if(ret != -1)
            {
                ret = AddChildren(children,parent_parent);
                if(ret < 0)
                    return ret;
            }
        }
        return 0;
    }

    Sqrat::Table sq_wx_propgrid_wrapper::GetEntry(wxString name)
    {
        Sqrat::Table entry(m_vm);
        PropertyToSqratTabel(m_grid->GetRoot()->GetPropertyByName(name),entry);
        return entry;
    }

    Sqrat::Table sq_wx_propgrid_wrapper::GetRoot()
    {
        Sqrat::Table root(m_vm);
        PropertyToSqratTabel(m_grid->GetRoot(),root);
        return root;

    }

    wxTextCtrl* sq_wx_propgrid_wrapper::GetLabelEditor()
    {
        return m_grid->GetLabelEditor();
    }

    Sqrat::Table sq_wx_propgrid_wrapper::GetSelectedProperty()
    {
        Sqrat::Table entry(m_vm);
        PropertyToSqratTabel(m_grid->GetSelectedProperty(),entry);
        return entry;
    }

    int sq_wx_propgrid_wrapper::PropertyToSqratTabel(wxPGProperty* prop,Sqrat::Table& table)
    {
        if(prop == NULL)
            return -1;

        table.SetValue("name",Sqrat::string(prop->GetBaseName().ToUTF8()));
        table.SetValue("label",Sqrat::string(prop->GetLabel().ToUTF8()));
        table.SetValue("value",Sqrat::string(prop->GetValueString().ToUTF8()));
        for(size_t i = 0; i < prop->GetChildCount(); i++)
        {
            Sqrat::Table tmp_child(m_vm);
            if(PropertyToSqratTabel(prop->Item(i),tmp_child) < 0)
                return -2;
            SQChar *name = NULL;
            tmp_child.GetValue("name",name);
            table.SetValue(name,tmp_child);
        }
    }

    SQInteger sq_wx_propgrid_wrapper_constructor(HSQUIRRELVM vm)
    {
        StackHandler sa(vm);
        try
        {
        if (sa.GetParamCount() == 1)      //Empty constructor....
        {
            /*sq_wx_propgrid_wrapper* instance = new wxString();
            sq_setinstanceup(vm, 1, instance);
            sq_setreleasehook(vm, 1, &Sqrat::DefaultAllocator<wxString>::Delete);
            return SC_RETURN_OK;*/
            return sa.ThrowError(_("sq_wx_propgrid_wrapper constructor: We need a property grid as parameter"));;
        }
        else if (sa.GetParamCount() == 2)     // 1 Parameter
        {
            wxPropertyGrid* prop_grid =  sa.GetInstance<wxPropertyGrid>(2);
            if(prop_grid == NULL)
                return sa.ThrowError(_("sq_wx_propgrid_wrapper constructor: property grid pointer was NULL"));

            sq_wx_propgrid_wrapper* instance = new sq_wx_propgrid_wrapper(prop_grid);
            instance->m_vm = vm;
            sq_setinstanceup(vm, 1, instance);
            sq_setreleasehook(vm, 1, &Sqrat::DefaultAllocator<sq_wx_propgrid_wrapper>::Delete);
            return SC_RETURN_OK;
        }
        }
        catch(CBScriptException &e)
        {
            return sa.ThrowError(e.Message());
        }

        return sa.ThrowError(_("wrong number of parameters"));
    }

}


