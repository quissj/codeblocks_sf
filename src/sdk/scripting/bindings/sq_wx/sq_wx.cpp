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
#include <scripting/bindings/sc_binding_util.h>

namespace ScriptBindings
{

    long long EmptyFunction()
    {
        return 0;
    }

    SQInteger sc_wxLongLong_tostring(HSQUIRRELVM vm)
    {
        StackHandler sa(vm);
        if(sa.GetParamCount() < 1)
            return sa.ThrowError(_("wxLongLong::_tostring() wrong number of parameter"));
        try
        {
            wxLongLong* inst = sa.GetInstance<wxLongLong>(1);
            sa.PushValue(inst->ToString().ToUTF8().data());
            return SC_RETURN_VALUE;
        } catch (CBScriptException &e)
        {
            return sa.ThrowError(_("wxLongLong::_tostring()")+ e.Message());
        }
    }

    /** \brief Bind wx Types (wxString,wxIntArray etc.) to the vm
     *
     * \param vm HSQUIRRELVM A Squirrel vm to which wx is bound
     *
     */
    void Register_wxGlobals(HSQUIRRELVM vm)
    {

        SQ_WX_binding::bind_wxString(vm);
        bind_wx_util_dialogs(vm);
        bind_wxConstants(vm);

        Sqrat::Class<wxStopWatch> stop_watch(vm,"wxStopWatch");
        stop_watch
        .Func("Pause",&wxStopWatch::Pause)
        .Func("Resume",&wxStopWatch::Resume)
        .Func("Start",&wxStopWatch::Start)
        .Func("Time",&wxStopWatch::Time)
        #if !wxCHECK_VERSION(3, 0, 0)
        .StaticFunc("TimeInMicro",&EmptyFunction);
        #else
        .Func("TimeInMicro",&wxStopWatch::TimeInMicro);
        #endif
        Sqrat::RootTable(vm).Bind("wxStopWatch",stop_watch);

        Sqrat::RootTable(vm).Func("wxLaunchDefaultBrowser",wxLaunchDefaultBrowser);

        #if wxCHECK_VERSION(3,0,0)

        Sqrat::Class<wxLongLongNative> wx_longlong(vm,"wxLongLongNative");
        wx_longlong.Func("ToLong",&wxLongLongNative::ToLong);
        wx_longlong.Func("ToDouble",&wxLongLongNative::ToDouble);
        wx_longlong.Func("GetHi",&wxLongLongNative::GetHi);
        wx_longlong.Func("GetLo",&wxLongLongNative::GetLo);
        wx_longlong.Func("ToString",&wxLongLongNative::ToString);
        wx_longlong.SquirrelFunc("_tostring",&sc_wxLongLong_tostring);
        Sqrat::RootTable(vm).Bind("wxLongLongNative",wx_longlong);
        #endif // wxCHECK_VERSION

        Sqrat::RootTable(vm).Func("wxLaunchDefaultBrowser",     wxLaunchDefaultBrowser);

    }
}
