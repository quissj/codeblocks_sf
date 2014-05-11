/*
 * This file is part of the Code::Blocks IDE and licensed under the GNU Lesser General Public License, version 3
 * http://www.gnu.org/licenses/lgpl-3.0.html
 *
 * $Revision$
 * $Id$
 * $HeadURL$
 */


#include <wx/stopwatch.h>
#include <scripting/bindings/sq_wx/sq_wx.h>
#include <wx/propgrid/xh_propgrid.h>

namespace ScriptBindings
{
    // Is this the best practice????
    //extern void bind_wxString(HSQUIRRELVM vm);

    long long EmptyFunction()
    {
        return 0;
    }

    /** \brief Bind wx Types (wxString,wxIntArray etc.) to the vm
     *
     * \param vm HSQUIRRELVM A Squirrel vm to witch wx is bound
     *
     */
    void bind_wx_types(HSQUIRRELVM vm)
    {
        wxPropertyGridXmlHandler* property_grid_handler = new wxPropertyGridXmlHandler();
        wxXmlResource::Get()->AddHandler(property_grid_handler);

        SQ_WX_binding::bind_wxString(vm);
        bind_wx_util_dialogs(vm);
        bind_wxDialog(vm);
        bind_wxBaseControls(vm);
        bind_wxConstants(vm);

        Sqrat::Class<wxStopWatch> stop_watch(vm,"wxStopWatch");
        stop_watch
        .Func("Pause",&wxStopWatch::Pause)
        .Func("Resume",&wxStopWatch::Resume)
        .Func("Start",&wxStopWatch::Start)
        .Func("Time",&wxStopWatch::Time)
        #if !wxCHECK_VERSION(2, 9, 0) // Only implemented in wx 2.9.x
        .StaticFunc("TimeInMicro",&EmptyFunction);
        #else
        .Func("TimeInMicro",&wxStopWatch::TimeInMicro);
        #endif
        Sqrat::RootTable(vm).Bind("wxStopWatch",stop_watch);
    }
}
