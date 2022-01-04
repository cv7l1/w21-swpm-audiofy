//
// Created by Jonathan on 28.11.2021.
//

#ifndef AUDIOFY_LIB_AL_UTIL_H
#define AUDIOFY_LIB_AL_UTIL_H

#include "win32_framework.h"
#include "al_error.h"

#define SAFE_RELEASE(punk)  \
if ((punk) != NULL)  \
{ (punk)->Release(); (punk) = NULL; }


class ProcPtr {
public:
    explicit ProcPtr(FARPROC ptr) : _ptr(ptr) {}
    template <typename T, typename = std::enable_if_t<std::is_function_v<T>>>
    operator T* () const {
        return reinterpret_cast<T *>(_ptr);
    }

private:
    FARPROC _ptr;
};

class DllHelper {
public:
    explicit DllHelper(_In_z_ const wchar_t* libPath) {
        _module = LoadLibraryW(libPath);
        if(_module == INVALID_HANDLE_VALUE) {
            throw Win32Exception(GetLastError());
        }
    }
    explicit DllHelper() : _module(nullptr) {}

    ~ DllHelper() { FreeLibrary(_module);}

    ProcPtr operator[] (_In_z_ const char* proc_name) const {
        return ProcPtr(GetProcAddress(_module, proc_name));
    }
    static HMODULE _parent_module;

private:
    HMODULE _module;

};


#endif //AUDIOFY_LIB_AL_UTIL_H
