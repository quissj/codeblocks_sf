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

    #define CHECK_NULL(n)    if(n==NULL)return

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

        int Populate(Sqrat::Table table);
        Sqrat::Array GetRoot();
        wxTextCtrl* GetLabelEditor();

        Sqrat::Table GetEntry(wxString name);
        Sqrat::Table GetSelectedProperty();
        bool    IsAnyModified()                                 {if(!m_grid) return false; return m_grid->IsAnyModified();};
        void 	SetCellTextColour (const wxColour &col)         {CHECK_NULL(m_grid); m_grid->SetCellTextColour(col);};
        void 	SetColumnCount (int colCount)                   {CHECK_NULL(m_grid); m_grid->SetColumnCount(colCount);};
        void 	SetLineColour (const wxColour &col)             {CHECK_NULL(m_grid); m_grid->SetLineColour(col);};
        void 	SetMarginColour (const wxColour &col)           {CHECK_NULL(m_grid); m_grid->SetMarginColour(col);};
        void    Clear()                                         {CHECK_NULL(m_grid); m_grid->Clear();};
        //void 	SetSelectionBackgroundColour (const wxColour &col){CHECK_NULL(m_grid); m_grid->SetSelectionBackgroundColour(col);};
        //void    SetSelectionTextColour (const wxColour &col)    {CHECK_NULL(m_grid); m_grid->SetSelectionTextColour(col);};


        private:


            /** \brief Fill the table with the attributes of the property
             *
             * \param prop wxPGProperty*
             * \param table Sqrat::Table&
             * \return int 0 if all ok, <0 if error
             *
             * The returned table has the form:
             * \code
             * Table = {
             * name = "Name.of.the.entry.with.all.its.parents",
             * label = "The label of the property",
             * value = "The value as a string",
             * children = {     // Array with subchildren
             * [0] = {          // Index 0 of array is a table like this one
             *          name = "Name.of.the.entry.with.all.its.parents",
             *          label = "The label of the property",
             *          value = "The value as a string"
             *        }
             *  }
             *}
             * \endcode
             */
            int PropertyToSqratTabel(wxPGProperty* prop,Sqrat::Table* table);

            wxPGProperty* CreateEntry(Sqrat::Table &entry,wxString name, wxString label, int type);
            int AddChildren(Sqrat::Table table,wxPGProperty* parent);
            wxPropertyGrid* m_grid;
            HSQUIRRELVM m_vm;

    };
}

#endif // SQ_WX_PROPGRID_H
