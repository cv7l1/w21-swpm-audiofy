// Dear ImGui: standalone example application for GLFW + OpenGL 3, using programmable pipeline
// (GLFW is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal graphics context creation, etc.)
// If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#include <math.h> 
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif
#include <GLFW/glfw3.h> // Will drag system OpenGL headers
#define M_PI 3.141592654

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of testing and compatibility with old VS compilers.
// To link with VS2010-era libraries, VS2015+ requires linking with legacy_stdio_definitions.lib, which we do using this pragma.
// Your own project should not be affected, as you are likely to link with a newer binary of GLFW that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

int main(int, char**)
{
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
                counter++;
            ImGui::SameLine();
            if (ImGui::Button("Pause"))                          
                counter++;
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

