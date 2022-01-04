cmake_minimum_required(VERSION 3.20)
project(audiolib_examples1)

set(CMAKE_CXX_STANDARD 20)

add_compile_options(/std:c++latest)

add_executable(audiofy src/main.cpp src/gui/gui_file.cpp src/gui/gui_file.h)
target_include_directories(audiofy PUBLIC include)
target_include_directories(audiofy PUBLIC vorbis)

target_link_directories(audiofy PUBLIC lib)
target_link_libraries(audiofy lib_win32 comsuppw)
target_link_libraries(audiofy Mfreadwrite.lib Mfplat.lib mfuuid.lib Pathcch.lib
        Shlwapi.dll opengl32.lib)
target_link_directories(audiofy PUBLIC lib)
target_link_directories(audiofy PUBLIC lib/glfw/lib-vc2010-64)

target_link_libraries(audiofy imgui_glfw)
target_link_libraries(audiofy glfw3)





add_executable(audiolib_devices main.cpp audiolib_devices/src/example_devices.cpp audiolib_devices/src/example_devices.h src/win32/ay_fileManager.cpp src/win32/ay_fileManager.h src/main.cpp)
set_target_properties(audiolib_devices PROPERTIES WIN32_EXECUTABLE TRUE)
target_include_directories(audiolib_devices PUBLIC include)
target_include_directories(audiolib_devices PUBLIC vorbis)
target_link_directories(audiolib_devices PUBLIC lib)
target_link_libraries(audiolib_devices lib_win32 comsuppw)

add_executable(audiolib_playback decode_playback_buffers/src/playback_example.cpp decode_playback_buffers/src/playback_example.h decode_playback_buffers/src/gui_test.cpp decode_playback_buffers/src/gui_test.h)
set_target_properties(audiolib_playback PROPERTIES WIN32_EXECUTABLE TRUE)

target_include_directories(audiolib_playback PUBLIC include)
target_include_directories(audiolib_playback PUBLIC vorbis)
target_include_directories(audiolib_playback PUBLIC lib/glfw/include)
target_include_directories(audiolib_playback PUBLIC lib/usynergy)
target_include_directories(audiolib_playback PUBLIC include/imgui)

target_link_directories(audiolib_playback  PUBLIC lib)
target_link_libraries(audiolib_playback lib_win32 comsuppw)
target_link_libraries(audiolib_playback  Mfreadwrite.lib Mfplat.lib mfuuid.lib Pathcch.lib
        Shlwapi.dll opengl32.lib)
target_link_directories(audiolib_playback PUBLIC lib)
target_link_directories(audiolib_playback PUBLIC lib/glfw/lib-vc2010-64)

target_link_libraries(audiolib_playback imgui_glfw)
target_link_libraries(audiolib_playback glfw3)

add_executable(audiolib_gui gui_primer/src/gui_example.cpp gui_primer/src/gui_example.h)
set_target_properties(audiolib_gui PROPERTIES WIN32_EXECUTABLE TRUE)
target_include_directories(audiolib_gui PUBLIC include)
target_include_directories(audiolib_gui PUBLIC vorbis)
target_include_directories(audiolib_gui PUBLIC lib/glfw/include)
target_include_directories(audiolib_gui PUBLIC lib/usynergy)
target_include_directories(audiolib_gui PUBLIC include/imgui)

target_link_directories(audiolib_gui PUBLIC lib)
target_link_libraries(audiolib_gui lib_win32 comsuppw)
target_link_libraries(audiolib_gui Mfreadwrite.lib Mfplat.lib mfuuid.lib Pathcch.lib
        Shlwapi.dll opengl32.lib)
target_link_directories(audiolib_gui PUBLIC lib)
target_link_directories(audiolib_gui PUBLIC lib/glfw/lib-vc2010-64)

target_link_libraries(audiolib_gui imgui_glfw)
target_link_libraries(audiolib_gui glfw3)