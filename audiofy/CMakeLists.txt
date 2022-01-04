cmake_minimum_required(VERSION 3.20)
project(audiofy)
set(CMAKE_CXX_STANDARD 20)
add_compile_options(/std:c++latest)

add_executable(audiofy src/main.cpp src/gui/gui_file.cpp src/win32/ay_fileManager.cpp)
target_include_directories(audiofy PUBLIC include)
target_include_directories(audiofy PUBLIC include/vorbis)

target_link_directories(audiofy PUBLIC lib)
target_link_libraries(audiofy lib_win32 comsuppw)
target_link_libraries(audiofy Mfreadwrite.lib Mfplat.lib mfuuid.lib Pathcch.lib
        Shlwapi.dll opengl32.lib)
target_link_directories(audiofy PUBLIC lib)
target_link_directories(audiofy PUBLIC lib/glfw/lib-vc2010-64)

target_link_libraries(audiofy imgui_glfw)
target_link_libraries(audiofy glfw3)
set_target_properties(audiofy PROPERTIES WIN32_EXECUTABLE TRUE)
