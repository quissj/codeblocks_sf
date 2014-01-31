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
    #include <wx/string.h>
    #include <globals.h>
#endif
#include <wx/filename.h>
#include <wx/colour.h>

#include "sc_base_types.h"

// FIXME (bluehazzard#1#): Error Handling has to be improved...

namespace ScriptBindings
{
    ///////////////////
    // wxArrayString //
    ///////////////////
    SQInteger wxArrayString_Index(HSQUIRRELVM v)
    {
        CompileTimeAssertion<wxMinimumVersion<2,8>::eval>::Assert();
        StackHandler sa(v);
        try
        {
            wxArrayString& self = *sa.GetInstance<wxArrayString>(1);
            wxString inpstr = *sa.GetInstance<wxString>(2);

            bool chkCase = true;
            bool frmEnd = false;
            if (sa.GetParamCount() >= 3)
                chkCase = sa.GetValue<bool>(3);
            if (sa.GetParamCount() == 4)
                frmEnd = sa.GetValue<bool>(4);
            sa.PushValue<SQInteger>(self.Index(inpstr.c_str(), chkCase, frmEnd));
            return SC_RETURN_VALUE;
        } catch(CBScriptException &e)
        {
            sa.ThrowError(e.Message());
            return SC_RETURN_FAILED;
        }
    }

    //////////////
    // wxColour //
    //////////////
    SQInteger wxColour_OpToString(HSQUIRRELVM v)
    {
        StackHandler sa(v);
        wxColour& self = *sa.GetInstance<wxColour>(1);
        wxString str = wxString::Format(_T("[r=%d, g=%d, b=%d]"), self.Red(), self.Green(), self.Blue());
        sa.PushValue<const SQChar*>(str.mb_str(wxConvUTF8).data());
        return SC_RETURN_VALUE;
    }

    ////////////////
    // wxFileName //
    ////////////////
    SQInteger wxFileName_OpToString(HSQUIRRELVM vm)
    {
        StackHandler sa(vm);
        wxFileName& self = *sa.GetInstance<wxFileName>(1);
        sa.PushValue<const SQChar*>(self.GetFullPath().mb_str(wxConvUTF8));
        return SC_RETURN_VALUE;
    }

    /////////////
    // wxPoint //
    /////////////

    // wxPoint operator==
    SQInteger wxPoint_OpCmp(HSQUIRRELVM v)
    {
        StackHandler sa(v);
        wxPoint& self = *sa.GetInstance<wxPoint>(1);
        wxPoint& other = *sa.GetInstance<wxPoint>(2);
        sa.PushValue<bool>(self==other);
        return SC_RETURN_VALUE;
    }
    SQInteger wxPoint_x(HSQUIRRELVM v)
    {
        StackHandler sa(v);
        wxPoint& self = *sa.GetInstance<wxPoint>(1);
        sa.PushValue<SQInteger>(self.x);
        return SC_RETURN_VALUE;
    }
    SQInteger wxPoint_y(HSQUIRRELVM v)
    {
        StackHandler sa(v);
        wxPoint& self = *sa.GetInstance<wxPoint>(1);
        sa.PushValue<SQInteger>(self.y);
        return SC_RETURN_VALUE;
    }

    //////////////
    // wxString //
    //////////////

    //! Is implemented in separate file
/*
    // the _() function for scripts
    wxString static_(const SQChar* str)
    {
        return wxGetTranslation(cbC2U(str));
    }

    // the _T() function for scripts
    wxString static_T(const SQChar* str)
    {
        return cbC2U(str);
    }

    // wxString operator+
    SQInteger wxString_OpAdd(HSQUIRRELVM v)
    {
        StackHandler sa(v);
        wxString result;
        wxString& str1 = *SqPlus::GetInstance<wxString,false>(v, 1);
        if (sa.GetType(2) == OT_INTEGER)
        {
#ifdef _SQ64
            result.Printf(_T("%s%ld"), str1.c_str(), sa.GetInt(2));
#else
            result.Printf(_T("%s%d"), str1.c_str(), sa.GetInt(2));
#endif
        }
        else if (sa.GetType(2) == OT_FLOAT)
            result.Printf(_T("%s%f"), str1.c_str(), sa.GetFloat(2));
        else if (sa.GetType(2) == OT_USERPOINTER)
            result.Printf(_T("%s%p"), str1.c_str(), sa.GetUserPointer(2));
        else if (sa.GetType(2) == OT_STRING)
            result.Printf(_T("%s%s"), str1.c_str(), cbC2U(sa.GetString(2)).c_str());
        else
            result = str1 + *SqPlus::GetInstance<wxString,false>(v, 2);
        return SqPlus::ReturnCopy(v, result);
    }

    SQInteger wxString_OpCmp(HSQUIRRELVM v)
    {
        StackHandler sa(v);
        wxString& str1 = *SqPlus::GetInstance<wxString,false>(v, 1);
        if (sa.GetType(2) == OT_STRING)
            return sa.Return((SQInteger)str1.Cmp(cbC2U(sa.GetString(2))));
        return sa.Return((SQInteger)str1.Cmp(*SqPlus::GetInstance<wxString,false>(v, 2)));
    }

    SQInteger wxString_OpToString(HSQUIRRELVM v)
    {
        StackHandler sa(v);
        wxString& self = *SqPlus::GetInstance<wxString,false>(v, 1);
        return sa.Return((const SQChar*)self.mb_str(wxConvUTF8));
    }

    SQInteger wxString_AddChar(HSQUIRRELVM v)
    {
        StackHandler sa(v);
        wxString& self = *SqPlus::GetInstance<wxString,false>(v, 1);
        int idx = sa.GetInt(2);
        char tmp[8] = {};
        sprintf(tmp, "%c", idx);
        self += cbC2U(tmp);
        return sa.Return();
    }
    SQInteger wxString_GetChar(HSQUIRRELVM v)
    {
        StackHandler sa(v);
        wxString& self = *SqPlus::GetInstance<wxString,false>(v, 1);
        int idx = sa.GetInt(2);
        return sa.Return((SQInteger)(((const char*)cbU2C(self))[idx]));
    }
    SQInteger wxString_Matches(HSQUIRRELVM v)
    {
        StackHandler sa(v);
        wxString& self = *SqPlus::GetInstance<wxString,false>(v, 1);
        wxString& other = *SqPlus::GetInstance<wxString,false>(v, 2);
        return sa.Return(self.Matches(other));
    }
    SQInteger wxString_AfterFirst(HSQUIRRELVM v)
    {
        StackHandler sa(v);
        wxString& self = *SqPlus::GetInstance<wxString,false>(v, 1);
        SQInteger search_char = static_cast<SQInteger>( sa.GetInt(2) );
        if ( !search_char ) // Probably it's a wxString
        {
            wxString& temp = *SqPlus::GetInstance<wxString,false>(v, 2);
            #if wxCHECK_VERSION(2, 9, 0)
            search_char = static_cast<SQInteger>( temp.GetChar(0).GetValue() );
            #else
            search_char = static_cast<SQInteger>( temp.GetChar(0) );
            #endif
        }
        return SqPlus::ReturnCopy( v, self.AfterFirst( static_cast<wxChar>( search_char ) ) );
    }
    SQInteger wxString_AfterLast(HSQUIRRELVM v)
    {
        StackHandler sa(v);
        wxString& self = *SqPlus::GetInstance<wxString,false>(v, 1);
        SQInteger search_char = static_cast<SQInteger>( sa.GetInt(2) );
        if ( !search_char ) // Probably it's a wxString
        {
            wxString& temp = *SqPlus::GetInstance<wxString,false>(v, 2);
            #if wxCHECK_VERSION(2, 9, 0)
            search_char = static_cast<SQInteger>( temp.GetChar(0).GetValue() );
            #else
            search_char = static_cast<SQInteger>( temp.GetChar(0) );
            #endif
        }
        return SqPlus::ReturnCopy( v, self.AfterLast( static_cast<wxChar>( search_char ) ) );
    }
    SQInteger wxString_BeforeFirst(HSQUIRRELVM v)
    {
        StackHandler sa(v);
        wxString& self = *SqPlus::GetInstance<wxString,false>(v, 1);
        SQInteger search_char = static_cast<SQInteger>( sa.GetInt(2) );
        if ( !search_char ) // Probably it's a wxString
        {
            wxString& temp = *SqPlus::GetInstance<wxString,false>(v, 2);
            #if wxCHECK_VERSION(2, 9, 0)
            search_char = static_cast<SQInteger>( temp.GetChar(0).GetValue() );
            #else
            search_char = static_cast<SQInteger>( temp.GetChar(0) );
            #endif
        }
        return SqPlus::ReturnCopy( v, self.BeforeFirst( static_cast<wxChar>( search_char ) ) );
    }
    SQInteger wxString_BeforeLast(HSQUIRRELVM v)
    {
        StackHandler sa(v);
        wxString& self = *SqPlus::GetInstance<wxString,false>(v, 1);
        SQInteger search_char = static_cast<SQInteger>( sa.GetInt(2) );
        if ( !search_char ) // Probably it's a wxString
        {
            wxString& temp = *SqPlus::GetInstance<wxString,false>(v, 2);
            #if wxCHECK_VERSION(2, 9, 0)
            search_char = static_cast<SQInteger>( temp.GetChar(0).GetValue() );
            #else
            search_char = static_cast<SQInteger>( temp.GetChar(0) );
            #endif
        }
        return SqPlus::ReturnCopy( v, self.BeforeLast( static_cast<wxChar>( search_char ) ) );
    }
    SQInteger wxString_Replace(HSQUIRRELVM v)
    {
        StackHandler sa(v);
        wxString& self = *SqPlus::GetInstance<wxString,false>(v, 1);
        wxString from = *SqPlus::GetInstance<wxString,false>(v, 2);
        wxString to = *SqPlus::GetInstance<wxString,false>(v, 3);
        bool all = true;
        if (sa.GetParamCount() == 4)
            all = sa.GetBool(4);
        return sa.Return((SQInteger)self.Replace(from, to, all));
    }
*/

////////////////////////////////////////////////////////////////////////////////

    void Register_wxTypes(HSQUIRRELVM vm)
    {
        ///////////////////
        // wxArrayString //
        ///////////////////
        Sqrat::Class<wxArrayString> array_string(vm,"wxArrayString");
                array_string.
                //emptyCtor().
                //Ctor<>().
                Func("Add",     &wxArrayString::Add ).
                Func("Clear",   &wxArrayString::Clear ).
                SquirrelFunc("Index",   &wxArrayString_Index).
                Func("GetCount",        &wxArrayString::GetCount)
                // FIXME (bluehazzard#1#): Fix Item in wx2.9
                #if !wxCHECK_VERSION(2, 9, 0) // Strange that this does not work with wx 2.9.x?!
                .Func("Item",           &wxArrayString::Item)
                #endif
                ;
        Sqrat::RootTable(vm).Bind("wxArrayString",array_string);

        //////////////
        // wxColour //
        //////////////
        typedef void(wxColour::*WXC_SET)(const unsigned char, const unsigned char, const unsigned char, const unsigned char);
        Sqrat::Class<wxColour> wx_colour(vm,"wxColour");
                wx_colour.
                Ctor<unsigned char,unsigned char,unsigned char>().
                //emptyCtor().
                SquirrelFunc("_tostring",   &wxColour_OpToString).
                Func("Blue",    &wxColour::Blue).
                Func("Green",   &wxColour::Green).
                Func("Red",     &wxColour::Red).
#if wxVERSION_NUMBER < 2900 || !wxCOLOUR_IS_GDIOBJECT
                Func("IsOk",    &wxColour::IsOk).
#endif
                Func<WXC_SET>("Set",    &wxColour::Set);
        Sqrat::RootTable(vm).Bind("wxColour",wx_colour);

        ////////////////
        // wxFileName //
        ////////////////
        typedef void(wxFileName::*WXFN_ASSIGN_FN)(const wxFileName&);
        typedef void(wxFileName::*WXFN_ASSIGN_STR)(const wxString&, wxPathFormat);
        typedef wxString(wxFileName::*WXFN_GETPATH)(int, wxPathFormat)const;
#if wxCHECK_VERSION(2, 9, 1)
        typedef bool(wxFileName::*WXFN_SETCWD)()const;
#else
        typedef bool(wxFileName::*WXFN_SETCWD)();
#endif
        typedef bool(wxFileName::*WXFN_ISFILEWRITEABLE)()const;

        Sqrat::Class<wxFileName> wx_filename(vm,"wxFileName");
                wx_filename.
                //emptyCtor().
                SquirrelFunc("_tostring",       &wxFileName_OpToString).
                Func<WXFN_ASSIGN_FN>("Assign",  &wxFileName::Assign).
                Func<WXFN_ASSIGN_STR>("Assign", &wxFileName::Assign).
                Func("AssignCwd",       &wxFileName::AssignCwd).
                Func("AssignDir",       &wxFileName::AssignDir).
                Func("AssignHomeDir",   &wxFileName::AssignHomeDir).
                Func("Clear",           &wxFileName::Clear).
                Func("ClearExt",        &wxFileName::ClearExt).
//                Func("GetCwd",&wxFileName::GetCwd).
                Func("GetDirCount",     &wxFileName::GetDirCount).
                Func("GetDirs",         &wxFileName::GetDirs).
                Func("GetExt",          &wxFileName::GetExt).
                Func("GetFullName",     &wxFileName::GetFullName).
                Func("GetFullPath",     &wxFileName::GetFullPath).
                Func("GetLongPath",     &wxFileName::GetLongPath).
                Func("GetName",         &wxFileName::GetName).
                Func<WXFN_GETPATH>("GetPath",&wxFileName::GetPath).
                Func("GetShortPath",    &wxFileName::GetShortPath).
                Func("GetVolume",       &wxFileName::GetVolume).
                Func("HasExt",          &wxFileName::HasExt).
                Func("HasName",         &wxFileName::HasName).
                Func("HasVolume",       &wxFileName::HasVolume).
                Func("InsertDir",       &wxFileName::InsertDir).
                Func("IsAbsolute",      &wxFileName::IsAbsolute).
                Func("IsOk",            &wxFileName::IsOk).
                Func("IsRelative",      &wxFileName::IsRelative).
                Func("IsDir",           &wxFileName::IsDir).
                Func("MakeAbsolute",    &wxFileName::MakeAbsolute).
                Func("MakeRelativeTo",  &wxFileName::MakeRelativeTo).
                Func("Normalize",       &wxFileName::Normalize).
                Func("PrependDir",      &wxFileName::PrependDir).
                Func("RemoveDir",       &wxFileName::RemoveDir).
                Func("RemoveLastDir",   &wxFileName::RemoveLastDir).
                Func("SameAs",          &wxFileName::SameAs).
                Func<WXFN_SETCWD>("SetCwd",&wxFileName::SetCwd).
                Func("SetExt",          &wxFileName::SetExt).
                Func("SetEmptyExt",     &wxFileName::SetEmptyExt).
                Func("SetFullName",     &wxFileName::SetFullName).
                Func("SetName",         &wxFileName::SetName).
                Func("SetVolume",       &wxFileName::SetVolume).
                Func<WXFN_ISFILEWRITEABLE>("IsFileWritable",&wxFileName::IsFileWritable);
        Sqrat::RootTable(vm).Bind("wxFileName",wx_filename);

        /////////////
        // wxPoint //
        /////////////
        Sqrat::Class<wxPoint> wx_point(vm,"wxPoint");
        wx_point.
                //emptyCtor().
            Ctor().
            Ctor<int,int>().
            SquirrelFunc("_cmp",    &wxPoint_OpCmp).
            Var("x",    &wxPoint::x).
            Var("y",    &wxPoint::y);
        Sqrat::RootTable(vm).Bind("wxPoint",wx_point);

        ////////////
        // wxSize //
        ////////////
        typedef void(wxSize::*WXS_SET)(int, int);
        typedef void(wxSize::*WXS_SETH)(int);
        typedef void(wxSize::*WXS_SETW)(int);
        Sqrat::Class<wxSize> wx_size(vm,"wxSize");
        wx_size.
                Ctor().
                Ctor<int,int>().
                Func("GetWidth",    &wxSize::GetWidth).
                Func("GetHeight",   &wxSize::GetHeight).
                Func<WXS_SET>("Set",&wxSize::Set).
                Func<WXS_SETH>("SetHeight", &wxSize::SetHeight).
                Func<WXS_SETW>("SetWidth",  &wxSize::SetWidth);
        Sqrat::RootTable(vm).Bind("wxSize",wx_size);

    }
};
