//
// Created by Jonathan on 09.12.2021.
//

#include "al_core_util.h"
#include "al_error.h"

void al_core_util::setLibResourcePath(wchar_t *path) {
    bool result = SetCurrentDirectoryW(path);
    if(!result) {throw IOError(path);}
}
