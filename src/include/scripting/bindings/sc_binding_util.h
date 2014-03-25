#ifndef SC_BINDING_UTIL_H
#define SC_BINDING_UTIL_H


// helper macros to bind constants
#define BIND_INT_CONSTANT(a) Sqrat::ConstTable(vm).Const(_SC(#a), a)
#define BIND_INT_CONSTANT_NAMED(a,n) Sqrat::ConstTable(vm).Const(_SC(n), a)
// NOTE (bluehazzard#1#): This can break the API, but the old API was wrong, because constants should be constants and don't get modified...
#define BIND_WXSTR_CONSTANT_NAMED(a,n) Sqrat::ConstTable(vm).Const(_SC(n),a.ToUTF8())

#define SC_RETURN_FAILED    -1
#define SC_RETURN_OK        -0
#define SC_RETURN_VALUE      1


#endif // SC_BINDING_UTIL_H
