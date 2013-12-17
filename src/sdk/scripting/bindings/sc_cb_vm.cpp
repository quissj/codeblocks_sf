

#include <globals.h>
#include <cbexception.h>
#include <wx/msgdlg.h>
#include <squirrel.h>
#include <sqstdblob.h>
#include <sqstdio.h>
#include <sqstdmath.h>
#include <sqstdsystem.h>
#include <sqstdstring.h>
#include <sc_cb_vm.h>


namespace ScriptBindings
{

CBScriptException::CBScriptException(wxString msg) : m_message(msg)
{

}

CBScriptException::~CBScriptException()
{

}

wxString CBScriptException::Message()
{
    return m_message;
}


template<> CBsquirrelVMManager* Mgr<CBsquirrelVMManager>::instance = nullptr;
template<> bool  Mgr<CBsquirrelVMManager>::isShutdown = false;

CBsquirrelVMManager::CBsquirrelVMManager()
{

}

CBsquirrelVMManager::~CBsquirrelVMManager()
{

}

void CBsquirrelVMManager::AddVM(CBsquirrelVM* vm)
{
    wxCriticalSectionLocker locker(cs);
    m_map[vm->GetVM()] = vm;
}

CBsquirrelVM* CBsquirrelVMManager::GetVM(HSQUIRRELVM vm)
{
    wxCriticalSectionLocker locker(cs);
    if(m_map.find(vm) == m_map.end())
    {
        // VM not found...
        // Should we throw an exception?
        return nullptr;
    }
    return m_map[vm];
}

void CBsquirrelVMManager::RemoveVM(CBsquirrelVM* vm)
{
    wxCriticalSectionLocker locker(cs);
    VMHashMap::iterator itr = m_map.begin();
    for(;itr != m_map.end();itr++)
    {
        if(itr->second == vm)
        {
            m_map.erase(itr);
        }
    }
}

void CBsquirrelVMManager::RemoveVM(HSQUIRRELVM vm)
{
    wxCriticalSectionLocker locker(cs);
    m_map.erase(vm);
}



CBsquirrelVM::CBsquirrelVM(int initialStackSize,const uint32_t library_to_load) : m_vm(sq_open(initialStackSize))
        , m_rootTable(new Sqrat::RootTable(m_vm))
        , m_script(new Sqrat::Script(m_vm))
        , m_lastErrorMsg()
        , m_shutdwon(false)
{
    //Register VM in the Manager
    CBsquirrelVMManager::Get()->AddVM(this);

    //Register std libs
    sq_pushroottable(m_vm);     // The register functions aspects a Table on the stack

    if(library_to_load & VM_LIB_IO)
        sqstd_register_iolib(m_vm);
    if(library_to_load & VM_LIB_BLOB)
        sqstd_register_bloblib(m_vm);
    if(library_to_load & VM_LIB_MATH)
        sqstd_register_mathlib(m_vm);
    if(library_to_load & VM_LIB_SYST)
        sqstd_register_systemlib(m_vm);
    if(library_to_load & VM_LIB_STR)
        sqstd_register_stringlib(m_vm);
    sq_pop(m_vm, 1);    // Pop the root table

    m_lib_loaded = library_to_load;

    sq_setcompilererrorhandler(m_vm, compilerErrorHandler);

    // FIXME (bluehazzard#1#): temporary set me as default vm
    Sqrat::DefaultVM::Set(m_vm);
}

CBsquirrelVM::~CBsquirrelVM()
{
    if(!m_shutdwon)
        Shutdown();
}

void CBsquirrelVM::Shutdown()
{
    CBsquirrelVMManager::Get()->RemoveVM(m_vm); // Remove this vm from the managed list
    delete m_script;
    delete m_rootTable;
    sq_close(m_vm);
    m_shutdwon = true;
}

void CBsquirrelVM::LoadLibrary(const uint32_t library_to_load)
{
    sq_pushroottable(m_vm); // The register functions aspects a Table on the stack
    if(library_to_load & VM_LIB_IO)
        sqstd_register_iolib(m_vm);
    if(library_to_load & VM_LIB_BLOB)
        sqstd_register_bloblib(m_vm);
    if(library_to_load & VM_LIB_MATH)
        sqstd_register_mathlib(m_vm);
    if(library_to_load & VM_LIB_SYST)
        sqstd_register_systemlib(m_vm);
    if(library_to_load & VM_LIB_STR)
        sqstd_register_stringlib(m_vm);
    sq_pop(m_vm, 1);    // Pop the root table

    m_lib_loaded |= library_to_load;
}

SQInteger CBsquirrelVM::runtimeErrorHandler(HSQUIRRELVM v)
{
    CBsquirrelVM* sq_vm = CBsquirrelVMManager::Get()->GetVM(v);
    if(sq_vm == NULL)
    {
        // Something funny happened.
        // In reality this shouldn't happen never...
        wxMessageBox(_T("Could not find squirrel VM! This is a Program Error. Please report it to the developers!"),_T("Squirrel Error!!"),wxOK|wxICON_ERROR);
        return SQ_ERROR;
    }
    const SQChar *sErr = 0;
    if(sq_gettop(v) >= 1)
    {
        if(SQ_SUCCEEDED(sq_getstring(v, 2, &sErr)))
        {
            sq_vm->m_lastErrorMsg = sErr;
        }
        else
        {
            sq_vm->m_lastErrorMsg = wxString(_("An Unknown RuntimeError Occurred.")).ToUTF8();
        }
    }
    return SQ_ERROR;
}

// Default Error Handler
void CBsquirrelVM::compilerErrorHandler(HSQUIRRELVM v,
                            const SQChar* desc,
                            const SQChar* source,
                            SQInteger line,
                            SQInteger column)
{
    int buffer_size = 128;
    SQChar *tmp_buffer;
    for(;;buffer_size*=2)
    {
        // TODO (bluehazzard#1#): Check if this is UNICODE UTF8 safe
        tmp_buffer = new SQChar [buffer_size];
        int retvalue = snprintf(tmp_buffer,buffer_size, _SC("\nSource: %s\nline: %d\ncolumn:%d\n%s"), source, (int) line, (int) column, desc);
        if(retvalue < buffer_size)
        {
            // Buffersize was large enough
            CBsquirrelVM* sq_vm = CBsquirrelVMManager::Get()->GetVM(v);
            if(sq_vm == NULL)
            {
                // Something funny happened.
                // In reality this shouldn't happen never...
                wxMessageBox(_T("Could not find squirrel VM! This is a Program Error. Please report it to the developers!"),_T("Squirrel Error!!"),wxOK|wxICON_ERROR);
                return;
            }
            sq_vm->m_lastErrorMsg = tmp_buffer;

            delete[] tmp_buffer;
            break;
        }
        // Buffer size was not enough
        delete[] tmp_buffer;
    }
}

void CBsquirrelVM::SetPrintFunc(SQPRINTFUNCTION printFunc, SQPRINTFUNCTION errFunc)
{
    sq_setprintfunc(m_vm, printFunc, errFunc);
}

void CBsquirrelVM::GetPrintFunc(SQPRINTFUNCTION printFunc, SQPRINTFUNCTION errFunc)
{
    printFunc   = sq_getprintfunc(m_vm);
    errFunc     = sq_geterrorfunc(m_vm);
}

void CBsquirrelVM::SetErrorHandler(SQFUNCTION runErr, SQCOMPILERERROR comErr)
{
    sq_newclosure(m_vm, runErr, 0);
    sq_seterrorhandler(m_vm);
    sq_setcompilererrorhandler(m_vm, comErr);
}

CBsquirrelVM::SC_ERROR_STATE CBsquirrelVM::doString(const Sqrat::string& str)
{
    Sqrat::string msg;
    m_lastErrorMsg.clear();
    if(!m_script->CompileString(str, msg))
    {
        if(m_lastErrorMsg.empty())
        {
            m_lastErrorMsg = msg;
        }
        return SC_COMPILE_ERROR;
    }
    if(!m_script->Run(msg))
    {
        if(m_lastErrorMsg.empty())
        {
            m_lastErrorMsg = msg;
        }
        return SC_RUNTIME_ERROR;
    }
    return SC_NO_ERROR;
}

CBsquirrelVM::SC_ERROR_STATE CBsquirrelVM::doFile(const Sqrat::string& file)
{
    Sqrat::string msg;
    m_lastErrorMsg.clear();
    if(!m_script->CompileFile(file, msg))
    {
        if(m_lastErrorMsg.empty())
        {
            m_lastErrorMsg = msg;
        }
        return SC_COMPILE_ERROR;
    }
    if(!m_script->Run(msg))
    {
        if(m_lastErrorMsg.empty())
        {
            m_lastErrorMsg = msg;
        }
        return SC_RUNTIME_ERROR;
    }
    return SC_NO_ERROR;
}


CBsquirrelVM::SC_ERROR_STATE CBsquirrelVM::doString(const wxString str)
{
#if wxCHECK_VERSION(2, 9, 0)
    return doString(str.ToStdString());
#else
    return doString(Sqrat::string(str.mb_str()));
#endif // wxCHECK_VERSION
}

void CBsquirrelVM::SetMeDefault()
{
    if(m_vm == nullptr)
        cbThrow(_("Cant set nullptr as default vm"));
    Sqrat::DefaultVM::Set(m_vm);
}



StackHandler::StackHandler(HSQUIRRELVM vm) : m_vm(vm)
{

}

StackHandler::~StackHandler()
{

}

int StackHandler::GetParamCount()
{
    return sq_gettop(m_vm);
}

SQObjectType StackHandler::GetType(int pos)
{
    return sq_gettype(m_vm,pos);
}

SQInteger StackHandler::ThrowError(SQChar* error)
{
    return sq_throwerror(m_vm,error);
}


SQInteger StackHandler::ThrowError(wxString error)
{

    wxString tmp = _("Stack Handler: ") + error;

    return sq_throwerror(m_vm,tmp.ToUTF8().data());
}

wxString StackHandler::GetError(bool del)
{
    if(!Sqrat::Error::Instance().Occurred(m_vm))
        return wxEmptyString;

    SQStackInfos si;
    char stackinfo[256];
    int stack = 1;
    wxString stack_string(_("Call Stack: \n"));
    wxString tmp;
    while(SQ_SUCCEEDED(sq_stackinfos(m_vm,stack,&si)))
    {
        tmp.Printf(_("%i Function: %s Line: %i\n"),stack,si.funcname,si.line);
        stack_string += tmp;
        stack++;
    }

    stack_string += wxString::FromUTF8(Sqrat::Error::Instance().Message(m_vm).c_str());

    if(del)
        Sqrat::Error::Instance().Clear(m_vm);

    return stack_string;
}

bool StackHandler::HasError()
{
    return Sqrat::Error::Instance().Occurred(m_vm);
}

} // namespace ScriptBindings











