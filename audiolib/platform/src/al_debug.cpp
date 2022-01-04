//
// Created by Jonathan on 02.12.2021.
//

#include "al_debug.h"
#include "win32_framework.h"
#include <io.h>
#include <fcntl.h>

void Console::setup() {

    AllocConsole();

    HANDLE outHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    if (outHandle == INVALID_HANDLE_VALUE)
    {
        OutputDebugStringW(L"Unable to init logger - can't get console handle");
    }

    DWORD currentMode = 0;
    if (!GetConsoleMode(outHandle, &currentMode))
    {
        OutputDebugStringW(L"Unable to init logger - can't get console mode");
    }
    currentMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;

    if (!SetConsoleMode(outHandle, currentMode))
    {
        OutputDebugStringW(L"Unable to init logger - can't set console mode");
    }
    CONSOLE_FONT_INFOEX fontInfo{ 0 };
    fontInfo.cbSize = sizeof(CONSOLE_FONT_INFOEX);
    fontInfo.nFont = 10;
    fontInfo.dwFontSize.X = 0;
    fontInfo.dwFontSize.Y = 18;
    fontInfo.FontFamily = FF_DONTCARE;
    fontInfo.FontWeight = FW_NORMAL;
    wcscpy_s(fontInfo.FaceName, L"Consolas");
    SetCurrentConsoleFontEx(outHandle, FALSE, &fontInfo);

    //Redirects our std out to the console
    int console = _open_osfhandle((long)outHandle, _O_WTEXT);
    FILE* fp;
    fp = _fdopen(console, "w");
    freopen_s(&fp, "CONOUT$", "w", stdout);
    wprintf(L"\x1b]0;Logging\x07");
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
}
