//
// Created by Jonathan on 14.12.2021.
//

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "win32_framework.h"
#include "al_player.h"
#include "al_file.h"
#include "al_debug.h"
#include "al_error.h"

#include <stdio.h>
#include <math.h>
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif
#include <GLFW/glfw3.h> // Will drag system OpenGL headers
#define M_PI 3.141592654
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

i16 sineWave(double time, double freq, double amplitude) {
    i16 result;
    double tpc = 44100 / freq;
    double cycles = time / tpc;
    double rad = M_PI * 2 * cycles;
    i16 _amp = 32767 * amplitude;
    result = _amp  * sin(rad);
    return result;
}

std::vector<i16> createSineWaveBuffer(size_t len) {
    std::vector<i16> vec;

    for(int i = 0; i<len - 1; i+=2) {
        //This should give us a middle C4 (for reference: https://www.youtube.com/watch?v=t8ZE1Mlkg2s)
        i16 currentSample = sineWave(i, 261.6, 0.9);
        vec.push_back(currentSample / 3);
        vec.push_back(currentSample / 3);
    }
    return vec;
}

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

int WINAPI WinMain(_In_ HINSTANCE hInstance,
                   _In_ HINSTANCE hPrevInstance,
                   _In_ PSTR lpCmdLine,
                   _In_ INT nChmdShow) {
    //Setup Audiolib
    CoInitializeEx(nullptr, COINIT_MULTITHREADED);

    // Setup window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100
const char* glsl_version = "#version 100";
glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
    // GL 3.2 + GLSL 150
const char* glsl_version = "#version 150";
glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
// GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

// Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(1250, 760, "Audiofy", NULL, NULL);
    if (window == NULL)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

// Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

// Setup Dear ImGui style
    ImGui::StyleColorsDark();
//ImGui::StyleColorsClassic();

// Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

// Load Fonts
// - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
// - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
// - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
// - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
// - Read 'docs/FONTS.md' for more instructions and details.
// - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
//io.Fonts->AddFontDefault();
//io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
//io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
//io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
//io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
//ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
//IM_ASSERT(font != NULL);
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

/*
 * This will initialize the console logger. Only for debug purposes
 */
    Console::setup();

/*This will initialize the audio device manager, through which we can recieve information
 * about all recognized audio devices.
 */
    AudioDeviceManager audioDeviceManager;

/*
 * This will return the audio device set as 'default' for a specific role.
 */
    auto defaultDevice = audioDeviceManager.getDefaultDevice(AudioDeviceRole::Playback);

/* Note that getDefaultDevice() may return a nullptr in which case no default device could be found
 * This usually means, that there are no device available for the given role.
 * This is not an error and does not throw an exception. This case should be handled by the user
 */
    if(!defaultDevice) {
        al_ErrorWarn("There is no usable speaker currently plugged in :(");
    }

/*
 * The default device may have been unplugged or disabled in some other way. This is something
 * that the user of the library should always watch out for and handle
 */
    u32 deviceState = defaultDevice->getCurrentDeviceState();
    if(deviceState == DEVICE_STATE_ACTIVE) {
        al_ErrorInfo("Device ready and working! :)");
    } else {
        al_ErrorWarn("Device is not plugged in");
    }

/* This will initialize our audio player. We can optionally pass a playback device
 * but we don't have to. If we don't, it will use the default device or throw
 * an exception if no valid device was found
 * We can also tell the audio player to use debug functionality, which enables
 * some asserts and debug output. By default, this is set so false. Please note,
 * that this may impact performance in a negative way.
 * We may also want to specify which audio format the player is going to use by default.
 * If nothing is specified, it's going to use 44.1khz, 16 bit samples and 2 channels.
 * When the player is destroyed, all buffers currently playing will be stopped
 * and no futher buffers can be submitted.
 * Please ensure that only ONE audioPlayer is active during runtime.
 * If you want to construct multiple audio players (which you really shouldn't...)
 * please make sure that all other instances have been destroyed.
 */
    AudioPlayer audioPlayer(true);
    audioPlayer.setErrorCallback([]() { al_ErrorWarn("Critical error, restarting the audio player");});
    auto defaultAudioFormat = AudioFormatInfo<i16>::PCMDefault();
    u32 bufferSize = defaultAudioFormat.sampleRate * sizeof(i16) * 2;
    AudioPlayBuffer sineAudioBuffer(defaultAudioFormat, 0);

    sineAudioBuffer.setPlayFullBuffer(true);

    auto sinePCMAudioBuffer = createSineWaveBuffer(bufferSize);
    sineAudioBuffer.getRawData() = sinePCMAudioBuffer;
    sineAudioBuffer.setLoop(true);

    audioPlayer.playAudioBuffer(sineAudioBuffer, false);

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::SetNextWindowPos(ImVec2(0, 0));
            ImGui::SetNextWindowSize(ImVec2(198, 88));
            ImGui::Begin("Import your audio files");

            ImGui::SliderFloat("Bitrate", &f, 0.0f, 1.0f);

            if (ImGui::Button("Import"))
                counter++;
            ImGui::SameLine();
            ImGui::Text("Files Imported = %d", counter);

            ImGui::End();
        }
        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::SetNextWindowPos(ImVec2(0, 88));
            ImGui::SetNextWindowSize(ImVec2(198, 111));
            ImGui::Begin("Export your audio file");


            ImGui::SliderFloat("Bitrate", &f, 0.5f, 1.0f);

            static char name[128] = "";
            ImGui::Text("Filename: "); ImGui::SameLine(); ImGui::InputText("", name, IM_ARRAYSIZE(name));
            if (ImGui::Button("Export"))
                counter++;

            ImGui::End();
        }

        {
            ImGui::SetNextWindowPos(ImVec2(198, 0));
            ImGui::SetNextWindowSize(ImVec2(707, 664));
            ImGui::Begin("Plot");


            ImGui::End();
        }

        {
            static float f = 1.0f;
            static int counter = 0;

            ImGui::SetNextWindowPos(ImVec2(0, 663));
            ImGui::SetNextWindowSize(ImVec2(398, 98));
            ImGui::Begin("Control Elements");

            ImGui::SliderFloat("Playback Speed", &f, 0.3f, 3.0f);

            if (ImGui::Button("Play"))
                audioPlayer.play();
            counter++;
            ImGui::SameLine();
            if (ImGui::Button("Pause"))
                audioPlayer.pause();
            ImGui::SameLine();
            if (ImGui::Button("Stop"))
                counter++;
            ImGui::SameLine();
            if (ImGui::Button("Record"))
                counter++;
            ImGui::SameLine();
            if (ImGui::Button("Export"))
                counter++;

            ImGui::End();
        }

        {
            ImGui::SetNextWindowPos(ImVec2(904, 0));
            ImGui::SetNextWindowSize(ImVec2(363, 401));
            ImGui::Begin("MIX");

            ImGui::End();
        }
        {
            static float f = 1.0f;
            static int counter = 0;

            ImGui::SetNextWindowPos(ImVec2(904, 401));
            ImGui::SetNextWindowSize(ImVec2(363, 263));
            ImGui::Begin("Equalizer");

            ImGui::SliderFloat("32", &f, 0.3f, 3.0f);
            ImGui::SliderFloat("64", &f, 0.3f, 3.0f);
            ImGui::SliderFloat("125", &f, 0.3f, 3.0f);
            ImGui::SliderFloat("250", &f, 0.3f, 3.0f);
            ImGui::SliderFloat("500", &f, 0.3f, 3.0f);
            ImGui::SliderFloat("1k", &f, 0.3f, 3.0f);
            ImGui::SliderFloat("2k", &f, 0.3f, 3.0f);
            ImGui::SliderFloat("4k", &f, 0.3f, 3.0f);
            ImGui::SliderFloat("8k", &f, 0.3f, 3.0f);
            ImGui::SliderFloat("16k", &f, 0.3f, 3.0f);
            ImGui::SameLine();
            if (ImGui::Button("apply"))
                counter++;

            ImGui::End();
        }
        {
            static float f = 1.0f;
            static int counter = 0;

            ImGui::SetNextWindowPos(ImVec2(395, 663));
            ImGui::SetNextWindowSize(ImVec2(363, 100));
            ImGui::Begin("Selection");

            static char name[128] = "";
            ImGui::Text("Start: "); ImGui::SameLine(); ImGui::InputText("", name, IM_ARRAYSIZE(name));
            static char name2[128] = "";
            ImGui::Text("Stop: "); ImGui::SameLine(); ImGui::InputText("", name, IM_ARRAYSIZE(name2));
            if (ImGui::Button("Cut"))
                counter++;

            ImGui::End();
        }
        {
            static float f = 1.0f;
            static int counter = 0;

            ImGui::SetNextWindowPos(ImVec2(756, 663));
            ImGui::SetNextWindowSize(ImVec2(522, 100));
            ImGui::Begin("Leveling");

            ImGui::SliderFloat("Level", &f, 0.3f, 3.0f);
            ImGui::SameLine();
            if (ImGui::Button("Mute###mute1"))
                counter++;

            ImGui::SliderFloat("Level", &f, 0.3f, 3.0f);
            ImGui::SameLine();
            if (ImGui::Button("Mute###mute2"))
                counter++;

            ImGui::End();
        }

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
