//
// Created by Jonathan on 30.12.2021.
//
#include "imgui.h"
#include "gui/components/fileDialog.h"
#include "imgui_plot.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <d3d11.h>
#include <tchar.h>
#include <implot.h>
#include "al_debug.h"
#include "gui/components/equalizer.h"
#include "gui/components/leveling.h"
#include "gui/components/controlElements.h"
#include "gui/components/plot.h"
#include "gui/components/mix.h"
#include "gui/components/cut.h"
#include "gui/GuiMain.h"
#include "Application.h"
#include "win32_framework.h"
#include "win32/ay_fileManager.h"
#include "gui/components/projectFileListComponent.h"
#include "SoundTouchDLL.h"
#include "gui/components/DeviceListComponent.h"
#include "gui/components/toolbar.h"
#include <iostream>
#include <cstdio>
#include <chrono>
#include <thread>

// Data
static ID3D11Device*            g_pd3dDevice = NULL;
static ID3D11DeviceContext*     g_pd3dDeviceContext = NULL;
static IDXGISwapChain*          g_pSwapChain = NULL;
static ID3D11RenderTargetView*  g_mainRenderTargetView = NULL;

// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
//DEBUG!
int setup(GUIWin32Context* context) {
    CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    Console::setup();

    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, _T("ImGui Example"), NULL };
    ::RegisterClassEx(&wc);
    HWND hwnd = ::CreateWindow(wc.lpszClassName, _T("Audiofy"), WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, NULL, NULL, wc.hInstance, NULL);

    // Initialize Direct3D
    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClass(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    // Show the window
    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    ImGui::StyleColorsLight();
    ImGui::StyleColorsClassic();
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    *context = GUIWin32Context {
        hwnd,
        wc,
        clear_color
    };
    return 0;
}

//Run some tests to ensure, that the application is in a working state
bool SanityCheck(bool debug) {
    if(debug) {
        al_ErrorInfo("Running sanity checks...");
    }
    //Check if necessary .dlls exist
    if(!PathFileExistsW(L"vorbisfile.dll")) {
        al_ErrorCritical("vorbisfile.dll not found, ensure that .dll is in the same location as the .exe");
        MessageBoxW(nullptr, L"Unable to load vorbis library, the program may be in an unstable state", nullptr, MB_OK);
    }
    return true;
}

void setupGUI(AudioContext& context) {
    al_ErrorInfo("Setting up GUI");
    AudioDeviceManager* deviceManager = new AudioDeviceManager;
    auto mixer = new MixerComponent(&context);
    GuiMain::AddComponent(new ControlElements(&context, mixer));
    GuiMain::AddComponent(new Toolbar(&context, deviceManager));
    GuiMain::AddComponent(new MixerComponent(&context));

    //Application::decoder.decodeAudioFile(audioFile2, buffer2);

    auto plot = new WaveformPlot(&context);

    GuiMain::AddComponent(plot);
    al_ErrorInfo("Setting up GUI done");

}

// Main code
int WinMain(  _In_ HINSTANCE hInstance,
              _In_ HINSTANCE hPrevInstance,
              _In_ LPSTR     lpCmdLine,
              _In_ int       nShowCmd)
{

      
    GUIWin32Context context;
    
    auto result = setup(&context);
    if(result != 0) {
        MessageBoxW(nullptr, L"Unable to create gui", nullptr, MB_OK);
        return 1;
    }
    SanityCheck(true);
    // Main loop
    bool done = false;

	AudioContext audioContext;
	setupGUI(audioContext);
    u32 timeID = 1;
    SetTimer(context.hwnd, timeID, 1000, nullptr);

    using clock = std::chrono::steady_clock;

    auto next_frame = clock::now();
    while (!done)
    {
        MSG msg;
        while (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
            if (msg.message == WM_TIMER) {
                if (msg.wParam == timeID) {
                    audioContext.onMetronomeTick();
                    SetTimer(nullptr, timeID, 1000, nullptr);
                }
            }
        }
        if (done)
            break;
        next_frame += std::chrono::milliseconds(33); // 5Hz

        // Start the Dear ImGui frame
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        GuiMain::Show();
        showCut(&audioContext);
        showEqualizer();
        showLeveling();

        // Rendering
        ImGui::Render();
        const float clear_color_with_alpha[4] = { context.clear_color.x * context.clear_color.w, context.clear_color.y * context.clear_color.w, context.clear_color.z * context.clear_color.w, context.clear_color.w };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        g_pSwapChain->Present(1, 0); // Present with vsync
        //g_pSwapChain->Present(0, 0); // Present without vsync
        std::this_thread::sleep_until(next_frame);
    }

    // Cleanup
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    ImPlot::DestroyContext();
    CleanupDeviceD3D();
    ::DestroyWindow(context.hwnd);
    ::UnregisterClass(context.windowClass.lpszClassName, context.windowClass.hInstance);

    return 0;
}

// Helper functions

bool CreateDeviceD3D(HWND hWnd)
{
    // Setup swap chain
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    if (D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext) != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = NULL; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = NULL; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
}

void CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget()
{
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = NULL; }
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Win32 message handler

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
        case WM_SIZE:
            if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
            {
                CleanupRenderTarget();
                g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
                CreateRenderTarget();
            }
            return 0;
            case WM_SYSCOMMAND:
                if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
                    return 0;
                break;
                case WM_DESTROY:
                    ::PostQuitMessage(0);
                    return 0;
    }
    return ::DefWindowProc(hWnd, msg, wParam, lParam);
}

