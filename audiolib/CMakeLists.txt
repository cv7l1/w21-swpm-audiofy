cmake_minimum_required(VERSION 3.20)
project(audiofy_lib)

set(CMAKE_CXX_STANDARD 17)

add_library(lib_win32 platform/src/win32_audio.cpp platform/src/win32_audio.h platform/src/win32_audio.h platform/src/types.h platform/src/memUtil.h)
target_link_libraries(lib_win32 Mfreadwrite.lib Mfplat.lib mfuuid.lib Pathcch.lib
        Shlwapi.dll)

