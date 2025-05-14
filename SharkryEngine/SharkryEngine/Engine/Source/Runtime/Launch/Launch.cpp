#include "Core/HAL/PlatformType.h"
#include "Core/Container/String.h"
#include "EngineLoop.h"
#include <windows.h>
#include <shellapi.h>
#include <shlwapi.h> // PathRemoveFileSpecW
#pragma comment(lib, "shlwapi.lib")


FEngineLoop GEngineLoop;
#ifdef _DEBUG_VIEWER
FWString GViewerFilePath;
std::vector<std::wstring> AnimAssetList;
#endif

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    UNREFERENCED_PARAMETER(nShowCmd);

    // ⬇ 현재 EXE 위치로 작업 디렉토리 변경
    WCHAR exePath[MAX_PATH];
    GetModuleFileNameW(NULL, exePath, MAX_PATH);
    PathRemoveFileSpecW(exePath);
    SetCurrentDirectoryW(exePath);

#ifdef _DEBUG_VIEWER
    int argc;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (argv && argc >= 2)
    {
        GViewerFilePath = FWString(argv[1]);
    }
    if (argc >= 3) 
    {
        FWString AnimListArg = FWString(argv[2]);
        AnimAssetList = SplitWString(AnimListArg, L';');
    }
    LocalFree(argv);
#endif

    GEngineLoop.Init(hInstance);
    GEngineLoop.Tick();
    GEngineLoop.Exit();

    return 0;
}

