#ifndef SQ_WX_PROPGRID_H
#define SQ_WX_PROPGRID_H

#include <wx/propgrid/propgrid.h>
#include <scripting/bindings/sq_wx/sq_wx.h>
#include <scripting/sqrat.h>

namespace ScriptBindings
{
    SQInteger sq_wx_propgrid_wrapper_constructor(HSQUIRRELVM vm);


    enum PROPERTY_TYPE
    {
        P_TYPE_STRING,
        P_TYPE_INT,
        P_TYPE_BOOL,
        P_TYPE_SELECTION,
        P_TYPE_COLOR,
        P_TYPE_LABEL
    };

    class sq_wx_propgrid_wrapper
    {
        friend SQInteger sq_wx_propgrid_wrapper_constructor(HSQUIRRELVM vm);
        public:


        sq_wx_propgrid_wrapper() : m_grid(NULL)
        {

        };

        sq_wx_propgrid_wrapper(wxPropertyGrid *grid) : m_grid(grid)
        {

        };
        virtual ~sq_wx_propgrid_wrapper()   {};

        void Populate(Sqrat::Table table);
        Sqrat::Table GetRoot();
        wxTextCtrl* GetLabelEditor();
        Sqrat::Table GetEntry(wxString name);
        Sqrat::Table GetSelectedProperty();

        private:
            int PropertyToSqratTabel(wxPGProperty* prop,Sqrat::Table& table);

            wxPGProperty* CreateEntry(Sqrat::Table entry,wxString name, wxString label, int type);
            void AddChildren(Sqrat::Table table,wxPGProperty* parent);
            wxPropertyGrid* m_grid;
            HSQUIRRELVM m_vm;

    };
}

#endif // SQ_WX_PROPGRID_H
