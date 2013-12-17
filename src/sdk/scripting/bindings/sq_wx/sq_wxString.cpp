/*
 * This file is part of the Code::Blocks IDE and licensed under the GNU Lesser General Public License, version 3
 * http://www.gnu.org/licenses/lgpl-3.0.html
 *
 * $Revision$
 * $Id$
 * $HeadURL$
 */

 #include <wx/wx.h>
 #include <wx/string.h>
 #include <sqrat.h>
 #include "sc_binding_util.h"
 #include <sq_wx/sq_wx.h>
 #include <sc_cb_vm.h>

 // We need at least version 2.8.5
 #if !wxCHECK_VERSION(2, 8, 5)
    #error Minimal wxWidgets version is 2.8.5
#endif

namespace ScriptBindings
{
namespace SQ_WX_binding
{


/** \brief  Converts a wxString to a sqrat/squirrel string.
 *          Takes care of the UNICODE conversion. Scripts use internally UTF8, wxString uses the system specified Encoding
 *
 * \param input wxString Input String
 * \return Sqrat::string A squirrel string (char)
 *
 */
Sqrat::string cWxToStdString(wxString input)
{
    return Sqrat::string( input.ToUTF8());
}

/** \brief Converts a squirrel String to a wxString.
 *          Takes care of the UNICODE conversion
 *
 * \param input SQChar* Pointer to the inout string
 * \return wxString     A wxString object
 *
 */
wxString StdToWxString(SQChar* input)
{
    return wxString::FromUTF8(input);
}

/** \brief Converts a squirrel String to a translated wxString.
 *          Takes care of the UNICODE conversion
 *
 * \param input SQChar* Pointer to the inout string
 * \return wxString     A wxString object
 *
 */
wxString StdToWxStringTranslated(SQChar* input)
{
    return wxGetTranslation(wxString::FromUTF8(input));
}

/** \brief The constructor function which squirrel uses for creating a wxWidgets object from various other types
 *
 * \param vm HSQUIRRELVM
 * \return SQInteger
 *
 */
static SQInteger wxString_constructor(HSQUIRRELVM vm)
{
    if (sq_gettop(vm) == 1) {    //Empty constructor....
        wxString* instance = new wxString();
        sq_setinstanceup(vm, 1, instance);
        sq_setreleasehook(vm, 1, &Sqrat::DefaultAllocator<wxString>::Delete);
        return SC_RETURN_OK;
    } else if (sq_gettop(vm) == 2) { // 1 Parameter
        // Lets check which type it is
         //copy Constructor?
        Sqrat::Var<const wxString&> copy(vm, 2);
        if (!Sqrat::Error::Instance().Occurred(vm)) {
            wxString* instance = new wxString(copy.value);
            sq_setinstanceup(vm, 1, instance);
            sq_setreleasehook(vm, 1, &Sqrat::DefaultAllocator<wxString>::Delete);
            return SC_RETURN_OK;
        }
        // it was not the copy ctr
        // lets test if it is a squirrel string
        Sqrat::Error::Instance().Clear(vm);
        Sqrat::Var<SQChar*> char_arr(vm, 2);
        if (!Sqrat::Error::Instance().Occurred(vm)) {
            wxString* instance = new wxString(wxString::FromUTF8(char_arr.value));
            sq_setinstanceup(vm, 1, instance);
            sq_setreleasehook(vm, 1, &Sqrat::DefaultAllocator<wxString>::Delete);
            return SC_RETURN_OK;
        }
        // Wrong ctr parameter
        Sqrat::Error::Instance().Clear(vm);
        return sq_throwerror(vm, Sqrat::Error::FormatTypeError(vm, 2, Sqrat::ClassType<wxString>::ClassName(vm) + _SC("|SQChar*")).c_str());
    }

    return sq_throwerror(vm, _SC("wrong number of parameters"));
}

/** \brief The operator+ for wxString in squirrel
 *
 * \param vm HSQUIRRELVM
 * \return SQInteger
 *
 */
static SQInteger wxString_add(HSQUIRRELVM vm)
{

    if (sq_gettop(vm) == 1) {    //no parameter to add
        sq_throwerror(vm, _SC("wxString add wrong number of parameters"));
    } else if (sq_gettop(vm) == 2) { // 1 Parameter
        // Lets check which type it is

        //First get the "this"
        wxString* instance = Sqrat::ClassType<wxString>::GetInstance(vm, 1);
        if(instance == NULL)
        {
            return sq_throwerror(vm, _SC("have no base"));
        }

        wxString ret(*instance); // create the return value

        // Test if we add wxString
        Sqrat::Var<const wxString&> copy(vm, 2);
        if (!Sqrat::Error::Instance().Occurred(vm)) {
            ret.Append(copy.value);
            Sqrat::Var<wxString>::push(vm, ret);
            return SC_RETURN_VALUE;
        }
        // Test if we add squirrel string
        Sqrat::Error::Instance().Clear(vm);
        Sqrat::Var<SQChar*> char_arr(vm, 2);
        if (!Sqrat::Error::Instance().Occurred(vm)) {
            ret.Append(wxString::FromUTF8(char_arr.value));
            Sqrat::Var<wxString>::push(vm, ret);
            return SC_RETURN_VALUE;
        }
        // Test if we add a int (this probably will never happen, because squirrel transforms it to a string?)
        Sqrat::Error::Instance().Clear(vm);
        Sqrat::Var<SQInteger> int_val(vm, 2);
        if (!Sqrat::Error::Instance().Occurred(vm)) {
            ret.Append(wxString::Format(wxT("%d"),int_val.value));
            Sqrat::Var<wxString>::push(vm, ret);
            return SC_RETURN_VALUE;
        }
        // Test if we add a floating point number (this probably will never happen, because squirrel transforms it to a string?)
        Sqrat::Error::Instance().Clear(vm);
        Sqrat::Var<SQFloat> float_val(vm, 2);
        if (!Sqrat::Error::Instance().Occurred(vm)) {
            ret.Append(wxString::Format(wxT("%f"),int_val.value));
            Sqrat::Var<wxString>::push(vm, ret);
            return SC_RETURN_VALUE;
        }
        // Wrong type of parameter
        return sq_throwerror(vm, Sqrat::Error::FormatTypeError(vm, 2, Sqrat::ClassType<wxString>::ClassName(vm) + _SC("|SQChar*|int|float")).c_str());
    }
    return sq_throwerror(vm, _SC("wrong number of parameters"));
}

/** \brief Compare to strings. This function is only used for > and <. For == you have to use a function like wxString::Compare()
 *
 * \param vm HSQUIRRELVM
 * \return SQInteger
 *
 */
static SQInteger wxString_cmp(HSQUIRRELVM vm)
{
    int ret = 0;
    if (sq_gettop(vm) < 2) {
        return sq_throwerror(vm, _SC("wxString cmp (<,>) wrong number of parameters"));
    }
    //First get the "this"
    wxString* lhs = Sqrat::ClassType<wxString>::GetInstance(vm, 1);
    wxString rhs;
    if(lhs == NULL)
    {
        return sq_throwerror(vm, _SC("have no base"));
    }
    Sqrat::Var<const wxString&> str_val(vm, 2);
    if (!Sqrat::Error::Instance().Occurred(vm)) {
        ret = lhs->CompareTo(str_val.value);
        Sqrat::Var<int>::push(vm, ret);
        return SC_RETURN_VALUE;
    }
    Sqrat::Error::Instance().Clear(vm);
    Sqrat::Var<SQChar*> char_arr(vm, 2);
    if (!Sqrat::Error::Instance().Occurred(vm)) {
        ret = lhs->CompareTo(wxString::Format(wxT("%s"),char_arr.value));
        Sqrat::Var<int>::push(vm, ret);
        return SC_RETURN_VALUE;
    }
    return sq_throwerror(vm, _SC("wrong number of parameters"));
}

static SQInteger wxString_Replace(HSQUIRRELVM vm)
{
    if (sq_gettop(vm) < 3) {
        return sq_throwerror(vm, _SC("wxString::Replace wrong number of parameters"));
    }
    //First get the "this"
    wxString* self = Sqrat::ClassType<wxString>::GetInstance(vm, 1);
    if(self == NULL) {
        return sq_throwerror(vm, _SC("have no base"));
    }

    bool all = false;
    Sqrat::Var<wxString&> old_str(vm,2);
    Sqrat::Var<wxString&> new_str(vm,3);
    if(Sqrat::Error::Instance().Occurred(vm)) {
        return sq_throwerror(vm, Sqrat::Error::FormatTypeError(vm, 2, Sqrat::ClassType<wxString>::ClassName(vm)).c_str());
    }

    if(sq_gettop(vm) == 4)
    {
        Sqrat::Var<bool> all_val(vm,4);
        if(Sqrat::Error::Instance().Occurred(vm)) {
            return sq_throwerror(vm, Sqrat::Error::FormatTypeError(vm, 2, _SC("bool")).c_str());
        }

        all = all_val.value;
    }
    Sqrat::Var<int>::push(vm, self->Replace(old_str.value, new_str.value, all));
    return SC_RETURN_VALUE;
}

SQInteger GetWxStringFromVM(HSQUIRRELVM vm,SQInteger stack_pos,wxString* str)
{
    StackHandler sa(vm);
    wxString tmp;
    Sqrat::Var<SQChar*> search_char_arr(vm,stack_pos);
    if(!Sqrat::Error::Instance().Occurred(vm)) {
        // !! This only works if the value is given with "" but not with '' because of the string delimiter '\0'
        // FIXME (bluehazzard#1#): Insert a type of check if it is a SQChar* with a correct delimiter, because we use FromUTF8 to get a UNICODE character....
        *str = wxString::FromUTF8(search_char_arr.value);    // Get the SQChar array with the UNICODE value and convert it to a wxString
    } else {
        //Sqrat::Var<wxString&> search_char_str(vm,2);
        *str = *sa.GetInstance<wxString>(stack_pos);
        if(Sqrat::Error::Instance().Occurred(vm)) {
            return sq_throwerror(vm, Sqrat::Error::FormatTypeError(vm, 2, Sqrat::ClassType<wxString>::ClassName(vm) +  _SC("|SQChar*")).c_str());
        }
        //*str = search_char_str.value;
    }
    //*str = tmp;
    return SC_RETURN_OK;
}

SQInteger wxString_AfterFirst(HSQUIRRELVM vm)
{
    if (sq_gettop(vm) < 2) {
        return sq_throwerror(vm, _SC("wxString::AfterFirst wrong number of parameters"));
    }
    //First get the "this"
    wxString* self = Sqrat::ClassType<wxString>::GetInstance(vm, 1);
    if(self == NULL) {
        return sq_throwerror(vm, _SC("have no base"));
    }
    wxString search_char;
    SQInteger result = GetWxStringFromVM(vm,2,&search_char);
    if(SQ_FAILED(result))
        return result;
    /*
    Sqrat::Var<*SQChar> search_char_arr(vm,2);
    if(!Sqrat::Error::Instance().Occurred(vm)) {
        // !! This only works if the value is given with "" but not with '' because of the string delimiter '\0'
        // FIXME (bluehazzard#1#): Insert a type of check if it is a SQChar* with a correct delimiter, because we use FromUTF8 to get a UNICODE character....
        search_char.FromUTF8(search_char_arr.value);    // Get the SQChar array with the UNICODE value and convert it to a wxString
    } else {
        Sqrat::Var<wxString&> search_char_str(vm,2);
        if(Sqrat::Error::Instance().Occurred(vm)) {
            return sq_throwerror(vm, Sqrat::Error::FormatTypeError(vm, 2, Sqrat::ClassType<wxString>::ClassName(vm)).c_str() + "|SQChar*");
        }
        search_char = search_char_str.value;
    }*/
    Sqrat::Var<wxString>::push(vm, self->AfterFirst(search_char[0]));
    return SC_RETURN_VALUE;
}

SQInteger wxString_AfterLast(HSQUIRRELVM vm)
{
    if (sq_gettop(vm) < 2) {
        return sq_throwerror(vm, _SC("wxString::AfterLast: wrong number of parameters"));
    }
    //First get the "this"
    wxString* self = Sqrat::ClassType<wxString>::GetInstance(vm, 1);
    if(self == NULL) {
        return sq_throwerror(vm, _SC("wxString::AfterLast: have no base"));
    }
    wxString search_char;
    SQInteger result = GetWxStringFromVM(vm,2,&search_char);
    if(SQ_FAILED(result))
        return result;

    Sqrat::Var<wxString>::push(vm, self->AfterLast(search_char[0]));
    return SC_RETURN_VALUE;
}

SQInteger wxString_BeforeFirst(HSQUIRRELVM vm)
{
    StackHandler sa(vm);
    if (sa.GetParamCount() < 2) {
        return sa.ThrowError(_("wxString::BeforeFirst: wrong number of parameters"));
    }
    //First get the "this"
    wxString* self = Sqrat::ClassType<wxString>::GetInstance(vm, 1);
    if(self == NULL) {
        return sa.ThrowError(_("wxString::BeforeFirst: have no base"));
    }
    wxString search_char;
    SQInteger result = GetWxStringFromVM(vm,2,&search_char);
    if(SQ_FAILED(result))
        return result;

    //Sqrat::Var<wxString>::push(vm, self->BeforeFirst(search_char[0]));
    sa.PushInstanceCopy<wxString>(self->BeforeFirst(search_char[0]));
    return SC_RETURN_VALUE;
}

SQInteger wxString_BeforeLast(HSQUIRRELVM vm)
{
    StackHandler sa(vm);

    if (sa.GetParamCount() < 2) {
        return sa.ThrowError(_("wxString::BeforeLast: wrong number of parameters"));
    }
    //First get the "this"
    wxString* self = sa.GetInstance<wxString>(1);
    if(self == NULL) {
        return sa.ThrowError(_("wxString::BeforeLast: have no base"));
    }
    wxString search_char;
    SQInteger result = GetWxStringFromVM(vm,2,&search_char);
    if(SQ_FAILED(result))
        return result;
    sa.PushInstanceCopy<wxString>(self->BeforeLast(search_char[0]));
    return SC_RETURN_VALUE;
}


SQInteger wxString_Matches(HSQUIRRELVM v)
{
    StackHandler sa(v);
    wxString& self = *sa.GetInstance<wxString>(1);
    wxString other;
    if(GetWxStringFromVM(v,2,&other) != SC_RETURN_OK)
        return SC_RETURN_FAILED;

    sa.PushValue<SQInteger>(self.Matches(other));
    return SC_RETURN_VALUE;
}


void bind_wxString(HSQUIRRELVM vm)
{
    using namespace Sqrat;

    Class<wxString> bwxString(vm,"wxString");
    bwxString.SquirrelFunc("constructor",&wxString_constructor)
    .SquirrelFunc(_SC("_cmp"),&wxString_cmp)
    .SquirrelFunc("_add", &wxString_add)
    .GlobalFunc(_SC("_tostring"),&cWxToStdString)

    .Func<wxString& (wxString::*)(const wxString&)>("Append",&wxString::Append)
    .Func("IsEmpty",&wxString::IsEmpty)
    .Func("Length", &wxString::Len)
    .Func("length", &wxString::Len)
    .Func("len",    &wxString::Len)
    .Func("size",   &wxString::Len)
    .Func("Lower",  &wxString::Lower)
    .Func("LowerCase",  &wxString::LowerCase)
    .Func("MakeLower",  &wxString::MakeLower)
    .Func("Upper",  &wxString::Upper)
    .Func("UpperCase",  &wxString::UpperCase)
    .Func("MakeUpper",  &wxString::MakeUpper)
    .Func("Mid",    &wxString::Mid)
    .Func<wxString& (wxString::*) (size_t pos, size_t len)>("Remove",    &wxString::Remove)
    .Func("RemoveLast",    &wxString::RemoveLast)
    .SquirrelFunc("Replace", &wxString_Replace)
    .SquirrelFunc("AfterFirst", &wxString_AfterFirst)
    .SquirrelFunc("AfterLast", &wxString_AfterLast)
    .SquirrelFunc("BeforeFirst", &wxString_BeforeFirst)
    .SquirrelFunc("BeforeLast", &wxString_BeforeLast)
    .Func("Right",   &wxString::Right)
    // TODO (bluehazzard#1#): In wx2.9 this is wxString not wxChar
    .SquirrelFunc("Matches",&wxString_Matches);


// TODO (bluehazzard#1#): Still to implement:  Not that easy with UTF8...
//* wxString::AddChar
//* wxString::GetChar



    RootTable(vm).Bind(_SC("wxString"),bwxString);
    //RootTable(vm).Func<wxString (*) (SQChar *)>(_SC("_T"),&StdToWxString);
    RootTable(vm).Func(_SC("_T"),&StdToWxString);
    RootTable(vm).Func<wxString (*) (SQChar *)>(_SC("wxT"),&StdToWxString);
    RootTable(vm).Func<wxString (*) (SQChar *)>(_SC("_"),&StdToWxStringTranslated);
}


} // namespace SQ_WX_binding

} // namespace ScriptBinding
