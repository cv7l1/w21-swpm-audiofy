cmake_minimum_required(VERSION 3.20)
project(audiofy_lib)

set(CMAKE_CXX_STANDARD 20)

add_library(lib_win32 platform/src/types.h platform/src/memUtil.h platform/src/audioPlayer.cpp platform/src/audioPlayer.h platform/src/audioDevice.cpp platform/src/audioDevice.h platform/src/win32_framework.h platform/src/al_error.cpp platform/src/al_error.h)
add_executable(win32_wav_test platform/src/dummyPlayback.cpp platform/src/dummyPlayback.h)
set_target_properties(win32_wav_test PROPERTIES WIN32_EXECUTABLE TRUE)
target_link_libraries(win32_wav_test lib_win32)

target_link_libraries(lib_win32 Mfreadwrite.lib Mfplat.lib mfuuid.lib Pathcch.lib
        Shlwapi.dll)

target_link_libraries(win32_wav_test Mfreadwrite.lib Mfplat.lib mfuuid.lib Pathcch.lib
        Shlwapi.dll)

