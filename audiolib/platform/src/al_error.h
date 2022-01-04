//
// Created by Jonathan on 27.11.2021.
//

#ifndef AUDIOFY_LIB_AL_ERROR_H
#define AUDIOFY_LIB_AL_ERROR_H

#include "win32_framework.h"

class Win32Exception : public std::exception {
protected:
    HRESULT hr;
public:
    explicit Win32Exception(HRESULT hr):hr(hr) {}
    HRESULT getErrorCode() const {return this->hr;}
};
class IOError : public Win32Exception {
    const wchar_t* _path;
public:
    IOError(const wchar_t *path, HRESULT hr = 0) : Win32Exception(hr), _path(path) {}
    const wchar_t* getFilePath() {return _path;}
};

inline static void throwIfFailed(HRESULT hr)
{
    if (FAILED(hr))
    {
        throw Win32Exception(hr);
    }
}


#endif //AUDIOFY_LIB_AL_ERROR_H
