#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>

#include <assert.h>
#include <stdlib.h>
#include <time.h>

#include "LogitechLEDLib.h"

#define SWAP(a, b) { auto tmp = a; a = b; b = tmp; }
#define ARRAY_COUNT(x) (sizeof(x) / sizeof(*(x)))
#define WM_NOTIFY_ICON_MESSAGE (WM_USER + 1)
#define ID_NOTIFY_ICON (WM_USER + 2)
#define WINDOW_CLASS_NAME "LG_G213_LED_CLASS"
#define ID_MENU_CLOSE 0

/* G213
Colors
Supports full RGB and Zones.
Supported Zones
Zone ID Zone Name Zone ID Zone Name
0 Entire Keyboard 3 Right Side
1 Left Side 4 Arrow keys side
2 Center Side 5 Numpad keys side
*/

enum zone_t {
    ZONE_ENTIRE_KEYBOARD = 0,

    ZONE_ITER_START = 1,

    ZONE_LEFT = 1,
    ZONE_CENTER = 2,
    ZONE_RIGHT = 3,
    ZONE_ARROW_KEYS = 4,
    ZONE_NUMPAD = 5,

    ZONE_ITER_END = 6
};

// G213 US layout
unsigned char keymap[256] = {
//  0  1  2  3  4  5  6  7    8  9  a  b  c  d  e  f
    0, 0, 0, 0, 0, 0, 0, 0,   3, 1, 0, 0, 0, 3, 0, 0, // 0
    0, 0, 0, 4, 0, 0, 0, 0,   0, 0, 0, 1, 0, 0, 0, 0, // 1
    2, 4, 4, 4, 4, 4, 4, 4,   4, 0, 0, 0, 4, 4, 4, 0, // 2
    3, 1, 1, 1, 1, 2, 2, 2,   2, 2, 0, 0, 0, 0, 0, 0, // 3
    0, 1, 2, 1, 1, 1, 1, 2,   2, 2, 2, 2, 3, 2, 2, 3, // 4
    3, 1, 1, 1, 2, 2, 2, 1,   1, 2, 1, 1, 3, 3, 0, 0, // 5
    5, 5, 5, 5, 5, 5, 5, 5,   5, 5, 5, 5, 0, 5, 5, 5, // 6
    1, 1, 1, 2, 2, 2, 2, 2,   3, 3, 3, 3, 0, 0, 0, 0, // 7

    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0, // 8
    5, 4, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0, // 9
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 5, 5, 5, // a
    5, 5, 5, 5, 0, 0, 0, 0,   0, 0, 3, 3, 3, 3, 3, 3, // b
    1, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0, // c
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 3, 3, 0, 3, 0, // d
    0, 0, 1, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0, // e
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0, // f
};

struct color_perc_t {
    int red;
    int green;
    int blue;
} colors[] = {
    {100,   0,   0},
    {  0, 100,   0},
    {  0,   0, 100},
    {100, 100,   0},
    {100,   0, 100},
    {  0, 100, 100},
};

inline float rand_perc() {
    return 100.f * (float)rand() / (float)RAND_MAX;
}

void set_random_zone_color(int zone) {
    color_perc_t *color = colors + rand() % ARRAY_COUNT(colors);
    LogiLedSetLightingForTargetZone(LogiLed::Keyboard, zone, color->red, color->green, color->blue);
}

void handle_keypress(DWORD vkey) {
    assert(vkey < 0x100);
    unsigned char zone_id = keymap[vkey & 0xff];
#if 0
    for (unsigned char i = ZONE_ITER_START; i < ZONE_ITER_END; i++) {
        if (zone_id == i) {
            LogiLedSetLightingForTargetZone(LogiLed::Keyboard, i, 100, 100, 100);
        }
        else {
            LogiLedSetLightingForTargetZone(LogiLed::Keyboard, i, 0, 0, 0);
        }
    }
#else
    set_random_zone_color(zone_id);
#endif
}

LRESULT CALLBACK keyboard_proc(int code, WPARAM wparam, LPARAM lparam) {
    if (code == HC_ACTION) {
        KBDLLHOOKSTRUCT *kbd = 0;
        switch (wparam) {
        case WM_KEYDOWN:
            kbd = (KBDLLHOOKSTRUCT*)lparam;
        }
        if (kbd) {
            handle_keypress(kbd->vkCode);
        }
    }
    
    return CallNextHookEx(0, code, wparam, lparam);
}

void add_notify_icon(HWND wnd) {
        NOTIFYICONDATAA nid = {};
        nid.cbSize = sizeof(nid);
        nid.hWnd = wnd;
        nid.uID = ID_NOTIFY_ICON;
        nid.uVersion = NOTIFYICON_VERSION;
        nid.uCallbackMessage = WM_NOTIFY_ICON_MESSAGE;
        nid.hIcon = LoadIcon(NULL, IDI_APPLICATION);
        strcpy_s(nid.szTip, "LG G213 LED");
        nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;

        Shell_NotifyIconA(NIM_ADD, &nid);
}

LRESULT CALLBACK window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    static UINT taskbar_restart;

    switch (msg) {
    case WM_CREATE:
        taskbar_restart = RegisterWindowMessageA("TaskbarCreated");
        add_notify_icon(hwnd);
    break;
    case WM_CLOSE:
        PostQuitMessage(0);
    break;
    case WM_NOTIFY_ICON_MESSAGE:
        switch (lparam) {
        case WM_RBUTTONDOWN:
        {
            HMENU menu = CreatePopupMenu();
            AppendMenuA(menu, MF_STRING, ID_MENU_CLOSE, "Close");
            POINT cursor;
            GetCursorPos(&cursor);

            SetForegroundWindow(hwnd);
            TrackPopupMenu(menu, TPM_LEFTBUTTON | TPM_RIGHTALIGN, cursor.x, cursor.y, 0, hwnd, 0);
            PostMessage(hwnd, WM_NULL, 0, 0);
        } 
        }
    break;
    case WM_COMMAND:
    {
        WORD id = LOWORD(wparam);
        WORD evt = HIWORD(wparam);
        if (id == ID_MENU_CLOSE) {
            PostQuitMessage(0);
        }
    } 
    break;
    default:
        if (msg == taskbar_restart) {
            add_notify_icon(hwnd);
        }
        return DefWindowProcA(hwnd, msg, wparam, lparam);
    }

    return 0;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
    if (LogiLedInit()) {
        LARGE_INTEGER Freq, LastCounter, Counter;
        MSG msg;
        BOOL ret;

        srand((unsigned int)time(0));

        WNDCLASSA wc = {};
        wc.lpfnWndProc = window_proc;
        wc.hInstance = hInstance;
        wc.hbrBackground = (HBRUSH)(COLOR_BACKGROUND);
        wc.lpszClassName = WINDOW_CLASS_NAME;
        RegisterClassA(&wc);

        HWND wnd = CreateWindowA(WINDOW_CLASS_NAME, "LG G213 LED", 0, 0, 0, 0, 0, 0, 0, hInstance, 0);

        LogiLedSetTargetDevice(LOGI_DEVICETYPE_ALL);
        for (int i = ZONE_ITER_START; i < ZONE_ITER_END; i++) {
            set_random_zone_color(i);
        }

        HHOOK hook = SetWindowsHookEx(WH_KEYBOARD_LL, keyboard_proc, 0, 0);
        QueryPerformanceCounter(&Freq);
        QueryPerformanceCounter(&LastCounter);
        QueryPerformanceCounter(&Counter);
        float delta = (float)(Counter.QuadPart - LastCounter.QuadPart) / (float)Freq.QuadPart;
        while ((ret = GetMessageA(&msg, wnd, 0, 0)) != 0) {
            if (ret == -1) {
                OutputDebugStringA("Error on GetMessage\n");
                break;
            }
            else {
                TranslateMessage(&msg);
                DispatchMessageA(&msg);
            }
        }
        UnhookWindowsHookEx(hook);
        LogiLedShutdown();
    }
    else {
        OutputDebugStringA("Could not initialize LED API\n");
        // TODO: Error
    }
    return 0;
}