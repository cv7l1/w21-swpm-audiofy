cmake_minimum_required(VERSION 3.20)
project(audiofy)
add_compile_options(/std:c++latest)

file(GLOB_RECURSE imgui_SRC "audiofy/src/gui/imgui/*.cpp")
file(GLOB_RECURSE audiolib_SRC "audiolib/platform/src/*.cpp")
file(GLOB_RECURSE audiofy_SRC "audiofy/src/*.cpp")
add_library(imgui_win32_dx11_lib ${imgui_SRC}  "audiofy/src/soundprocessor/effects/Volume.h"   "audiofy/src/soundprocessor/effects/Pitch.h" "audiofy/src/soundprocessor/effects/Tempo.h")
add_library(audiofy_lib ${audiolib_SRC}  "audiofy/src/soundprocessor/effects/Volume.h"   "audiofy/src/soundprocessor/effects/Pitch.h" "audiofy/src/soundprocessor/effects/Tempo.h")

target_include_directories(audiofy_lib PUBLIC audiolib/vorbis)

target_link_libraries(audiofy_lib Mfreadwrite.lib Mfplat.lib mfuuid.lib Pathcch.lib
        Shlwapi.dll)

target_include_directories(imgui_win32_dx11_lib PUBLIC include/imgui)


#add_executable(audiofy src/Application.cpp src/win32/ay_fileManager.cpp src/audio/AudioWorkspace.cpp src/audio/AudioWorkspace.h
#        src/gui/components/controlElements.h src/gui/components/cut.h src/gui/components/equalizer.h src/gui/components/fileDialog.h src/gui/components/leveling.h src/gui/components/mix.h src/gui/components/plot.h src/gui/components/projectFileListComponent.cpp src/gui/components/projectFileListComponent.h src/gui/GuiMain.cpp src/gui/GuiMain.h src/gui/components/IComponent.cpp src/gui/components/IComponent.h src/Application.h src/gui/components/fileDialog.cpp src/gui/components/plot.cpp src/gui/components/mix.cpp src/soundprocessor/SoundProcessor.cpp src/gui/components/DeviceListComponent.cpp src/gui/components/DeviceListComponent.h)
add_executable(audiofy ${audiofy_SRC}  "audiofy/src/soundprocessor/effects/Volume.h"   "audiofy/src/soundprocessor/effects/Pitch.h" "audiofy/src/soundprocessor/effects/Tempo.h")

#target_include_directories(audiofy PUBLIC include)
target_include_directories(audiofy PUBLIC audiolib/platform/src)
target_include_directories(audiofy PUBLIC audiofy/include/vorbis)
target_include_directories(audiofy PUBLIC audiofy/include/imgui)
target_include_directories(imgui_win32_dx11_lib PUBLIC audiofy/include/imgui)
target_include_directories(audiofy PUBLIC audiofy/include/soundtouch)

target_link_directories(audiofy PUBLIC lib)
target_link_libraries(audiofy comsuppw)
target_link_libraries(audiofy Mfreadwrite.lib Mfplat.lib mfuuid.lib Pathcch.lib
        Shlwapi.dll opengl32.lib)
target_link_directories(audiofy PUBLIC audiofy/lib)

target_link_libraries(audiofy imgui_win32_dx11_lib)
target_link_libraries(audiofy SoundTouchDLL_x64)
target_link_libraries(audiofy D3D11)
target_link_libraries(audiofy audiofy_lib)
set_target_properties(audiofy PROPERTIES WIN32_EXECUTABLE TRUE)
