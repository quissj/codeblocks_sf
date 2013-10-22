/*
 * This file is part of the Code::Blocks IDE and licensed under the GNU Lesser General Public License, version 3
 * http://www.gnu.org/licenses/lgpl-3.0.html
 *
 * $Revision$
 * $Id$
 * $HeadURL$
 */

 #include <sq_wx/sq_wx.h>
 #include <squirrel.h>
 #include <sqrat.h>

namespace ScriptBindings
{
    // Is this the best practice????
    //extern void bind_wxString(HSQUIRRELVM vm);

    /** \brief Bind wx Types (wxString,wxIntArray etc.) to the vm
     *
     * \param vm HSQUIRRELVM A Squirrel vm to witch wx is bound
     *
     */
    void bind_wx_types(HSQUIRRELVM vm)
    {
        SQ_WX_binding::bind_wxString(vm);
        bind_wx_util_dialogs(vm);
    }
}
