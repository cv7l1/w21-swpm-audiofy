cmake_minimum_required(VERSION 3.20)
project(audiofy)
set(CMAKE_CXX_STANDARD 20)
add_compile_options(/std:c++latest)

file(GLOB_RECURSE imgui_SRC "src/gui/imgui/*.cpp")
add_library(imgui_win32_dx11 ${imgui_SRC})
target_include_directories(imgui_win32_dx11 PUBLIC include/imgui)


add_executable(audiofy src/main.cpp src/win32/ay_fileManager.cpp src/audio/AudioWorkspace.cpp src/audio/AudioWorkspace.h
        src/gui/components/controlElements.h src/gui/components/cut.h src/gui/components/equalizer.h src/gui/components/fileDialog.h src/gui/components/leveling.h src/gui/components/mix.h src/gui/components/plot.h)

target_include_directories(audiofy PUBLIC include)
target_include_directories(audiofy PUBLIC include/vorbis)

target_link_directories(audiofy PUBLIC lib)
target_link_libraries(audiofy lib_win32 comsuppw)
target_link_libraries(audiofy Mfreadwrite.lib Mfplat.lib mfuuid.lib Pathcch.lib
        Shlwapi.dll opengl32.lib)
target_link_directories(audiofy PUBLIC lib)
target_link_directories(audiofy PUBLIC lib/glfw/lib-vc2010-64)

target_link_libraries(audiofy imgui_win32_dx11)
target_link_libraries(audiofy D3D11)
set_target_properties(audiofy PROPERTIES WIN32_EXECUTABLE TRUE)
