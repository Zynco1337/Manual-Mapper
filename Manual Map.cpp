
#include <windows.h>
#include <tlhelp32.h>
#include <commdlg.h>
#include <string>
#include <iostream>


void ConsoleColor(WORD attributes) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), attributes);
}

void SetConsoleWindowSize(int width, int height) {
    COORD bufferSize = { static_cast<SHORT>(width), static_cast<SHORT>(height) };
    SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), bufferSize);

    SMALL_RECT windowSize = { 0, 0, static_cast<SHORT>(width - 1), static_cast<SHORT>(height - 1) };
    SetConsoleWindowInfo(GetStdHandle(STD_OUTPUT_HANDLE), TRUE, &windowSize);
}


void CenterConsoleWindow() {
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    HWND consoleWindow = GetConsoleWindow();

    RECT consoleRect;
    GetWindowRect(consoleWindow, &consoleRect);
    int consoleWidth = consoleRect.right - consoleRect.left;
    int consoleHeight = consoleRect.bottom - consoleRect.top;

    int posX = (screenWidth - consoleWidth) / 2;
    int posY = (screenHeight - consoleHeight) / 2;

    SetWindowPos(consoleWindow, HWND_TOP, posX, posY, 0, 0, SWP_NOSIZE);

    LONG style = GetWindowLong(consoleWindow, GWL_EXSTYLE);
    SetWindowLong(consoleWindow, GWL_EXSTYLE, style | WS_EX_LAYERED);
    SetLayeredWindowAttributes(consoleWindow, 0, (BYTE)(255 * 0.9), LWA_ALPHA);
}

DWORD GetProcessIdByName(const std::wstring& processName) {
    DWORD processId = 0;
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32W pe32;
        pe32.dwSize = sizeof(PROCESSENTRY32W);
        if (Process32FirstW(hSnapshot, &pe32)) {
            do {
                if (processName == pe32.szExeFile) {
                    processId = pe32.th32ProcessID;
                    break;
                }
            } while (Process32NextW(hSnapshot, &pe32));
        }
        CloseHandle(hSnapshot);
    }
    return processId;
}


std::wstring OpenFileDialog() {
    OPENFILENAMEW ofn;
    wchar_t filePath[MAX_PATH] = { 0 };

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFilter = L"DLL Files\0*.dll\0All Files\0*.*\0";
    ofn.lpstrFile = filePath;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
    ConsoleColor(FOREGROUND_BLUE | FOREGROUND_INTENSITY);
    ofn.lpstrTitle = L"Select a DLL to Inject";

    if (GetOpenFileNameW(&ofn)) {
        return std::wstring(filePath);
    }
    return L"";
}

bool InjectDLL(DWORD processId, const std::wstring& dllPath) {
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
    if (!hProcess) {
        system("cls");
        ConsoleColor(FOREGROUND_RED | FOREGROUND_INTENSITY);
        std::wcout << L"Failed to open process.\n";
        std::wcout << L"Closing In 5 Seconds.\n";
        Sleep(5000);
        return false;
    }

    void* pRemoteMemory = VirtualAllocEx(hProcess, NULL, dllPath.size() * sizeof(wchar_t), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!pRemoteMemory) {
        CloseHandle(hProcess);
        ConsoleColor(FOREGROUND_RED | FOREGROUND_INTENSITY);
        system("cls");
        std::wcout << L"Failed to allocate memory.\n";
        std::wcout << L"Closing In 5 Seconds.\n";
        Sleep(5000);
        return false;
    }

    if (!WriteProcessMemory(hProcess, pRemoteMemory, dllPath.c_str(), dllPath.size() * sizeof(wchar_t), NULL)) {
        VirtualFreeEx(hProcess, pRemoteMemory, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        system("cls");
        ConsoleColor(FOREGROUND_RED | FOREGROUND_INTENSITY);
        std::wcout << L"Failed to write memory.\n";
        std::wcout << L"Closing In 5 Seconds.\n";
        Sleep(5000);
        return false;
    }

    HMODULE hKernel32 = GetModuleHandleW(L"Kernel32.dll");
    if (!hKernel32) {
        VirtualFreeEx(hProcess, pRemoteMemory, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        system("cls");
        ConsoleColor(FOREGROUND_RED | FOREGROUND_INTENSITY);
        std::wcout << L"Failed to get Kernel32 handle.\n";
        std::wcout << L"Closing In 5 Seconds.\n";
        Sleep(5000);
        return false;
    }

    LPVOID pLoadLibraryW = (LPVOID)GetProcAddress(hKernel32, "LoadLibraryW");
    if (!pLoadLibraryW) {
        VirtualFreeEx(hProcess, pRemoteMemory, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        system("cls");
        ConsoleColor(FOREGROUND_RED | FOREGROUND_INTENSITY);
        std::wcout << L"Failed to get LoadLibraryW address.\n";
        std::wcout << L"Closing In 5 Seconds.\n";
        Sleep(5000);
        return false;
    }

    HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)pLoadLibraryW, pRemoteMemory, 0, NULL);
    if (!hThread) {
        VirtualFreeEx(hProcess, pRemoteMemory, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        system("cls");
        ConsoleColor(FOREGROUND_RED | FOREGROUND_INTENSITY);
        std::wcout << L"Failed to create remote thread.\n";
        std::wcout << L"Closing In 5 Seconds.\n";
        Sleep(5000);
        return false;
    }

    WaitForSingleObject(hThread, INFINITE);

    VirtualFreeEx(hProcess, pRemoteMemory, 0, MEM_RELEASE);
    CloseHandle(hThread);
    CloseHandle(hProcess);
    system("cls");
    ConsoleColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    std::wcout << L"Injection successful!\n";
    std::wcout << L"Closing In 5 Seconds.\n";
    Sleep(5000);
    return 1;
}

int main() {
    SetConsoleTitleW(L"SixFive Manual Mapper");
    CenterConsoleWindow();
    SetConsoleWindowSize(70, 35);
    std::wstring dllPath = OpenFileDialog();
    ConsoleColor(FOREGROUND_RED | FOREGROUND_INTENSITY);
    if (dllPath.empty()) {
        system("cls");
        std::wcout << L"No DLL selected.\n";
        std::wcout << L"Closing In 5 Seconds.\n";
        Sleep(5000);
        return 1;
    }

    std::wstring processName;
    ConsoleColor(FOREGROUND_BLUE | FOREGROUND_INTENSITY);
    std::wcout << L"Enter the target process name (e.g., notepad.exe): ";
    std::wcin >> processName;
    DWORD processId = GetProcessIdByName(processName);
    ConsoleColor(FOREGROUND_RED | FOREGROUND_INTENSITY);
    if (!processId) {
        system("cls");
        std::wcout << L"Process not found.\n";
        std::wcout << L"Closing In 5 Seconds.\n";
        Sleep(5000);
        return 1;
    }

    if (!InjectDLL(processId, dllPath)) {
        system("cls");
        std::wcout << L"Injection failed.\n";
        std::wcout << L"Closing In 5 Seconds.\n";
        Sleep(5000);
        return 1;
    }

    return 0;
}
