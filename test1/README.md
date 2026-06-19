# Void Key Logger

![C++](https://img.shields.io/badge/Language-C++-blue.svg)
![Platform](https://img.shields.io/badge/Platform-Windows-lightgrey.svg)
![API](https://img.shields.io/badge/API-Win32-0078D7.svg)
![Status](https://img.shields.io/badge/Status-Active-brightgreen.svg)

A lightweight, stealth-mode Windows keylogger written in C++ using the Windows Low-Level Keyboard Hook API (`WH_KEYBOARD_LL`). Designed for authorized penetration testing, security research, and educational purposes.

```
            _     _   _              _                             
__   _____ (_) __| | | | _____ _   _| | ___   __ _  __ _  ___ _ __ 
\ \ / / _ \| |/ _` | | |/ / _ \ | | | |/ _ \ / _` |/ _` |/ _ \ '__|
 \ V / (_) | | (_| | |   <  __/ |_| | | (_) | (_| | (_| |  __/ |   
  \_/ \___/|_|\__,_| |_|\_\___|\__, |_|\___/ \__, |\__, |\___|_|   
                               |___/         |___/ |___/           
```

---

## Table of Contents

- [Features](#features)
- [Architecture](#architecture)
- [Configuration](#configuration)
- [Output Formats](#output-formats)
- [Building](#building)
- [Usage](#usage)
- [Log File Format](#log-file-format)
- [Stealth Mechanisms](#stealth-mechanisms)
- [Detection & Bypass Considerations](#detection--bypass-considerations)
- [Use Cases](#use-cases)
- [Disclaimer](#disclaimer)
- [License](#license)

---

## Features

- **Global Keyboard Hooking** — Captures all keystrokes system-wide via `WH_KEYBOARD_LL`
- **Multi-Format Output** — Supports named keys, decimal codes, and hex codes
- **Per-Hour Log Rotation** — Automatically creates a new log file every hour with timestamped filenames
- **Active Window Tracking** — Logs which application window the user is interacting with
- **Shift/CapsLock Awareness** — Correctly handles uppercase/lowercase input
- **Console Visualization** — Real-time keystroke display with color-coded output
- **Boot Wait Mode** — Optionally delays capture until system boot completes
- **Stealth Mode** — Hides console window and optionally frees the console entirely
- **Mouse Event Filter** — Optionally ignore mouse clicks (1=left, 2=right)

---

## Architecture

```
┌─────────────────────────────────────┐
│            main()                   │
│  ┌─ Banner Display ───────────────┐ │
│  └────────────────────────────────┘ │
│  ┌─ Stealth() ────────────────────┐ │
│  │  Hides console window          │ │
│  └────────────────────────────────┘ │
│  ┌─ BootCheck ────────────────────┐ │
│  │  (optional bootwait)           │ │
│  └────────────────────────────────┘ │
│  ┌─ SetHook() ────────────────────┐ │
│  │  SetWindowsHookEx(...)         │ │
│  └────────────────────────────────┘ │
│  ┌─ Message Loop ─────────────────┐ │
│  │  while(GetMessage())           │ │
│  │  ┌──────────────────────────┐  │ │
│  │  │  HookCallback()          │  │ │
│  │  │  ┌────────────────────┐  │  │ │
│  │  │  │  Save()            │  │  │ │
│  │  │  │  ├─ Window Track   │  │  │ │
│  │  │  │  ├─ Key Mapping    │  │  │ │
│  │  │  │  ├─ Console Output │  │  │ │
│  │  │  │  └─ File Output    │  │  │ │
│  │  │  └────────────────────┘  │  │ │
│  │  └──────────────────────────┘  │ │
│  └────────────────────────────────┘ │
└─────────────────────────────────────┘
```

### Component Breakdown

| Component | Description |
|-----------|-------------|
| `HookCallback()` | Low-level keyboard hook procedure. Fires on every `WM_KEYDOWN` event. |
| `Save()` | Core logging function — maps virtual key codes to human-readable output, tracks active windows, and writes to file + console. |
| `Stealth()` | Hides the console window or calls `FreeConsole()` to remove it entirely. |
| `SetHook()` / `ReleaseHook()` | Install and uninstall the global keyboard hook. |
| `IsSystemBooting()` | Checks `GetSystemMetrics(SM_GETSYSTEMMETRICS)` to determine if the system is still initializing. |
| `LogWithColor()` | Helper for colorized console output using `SetConsoleTextAttribute()`. |

---

## Configuration

All configuration is handled via preprocessor `#define` directives at the top of `main.cpp`:

```cpp
#define invisible     // visible / invisible
#define bootwait      // bootwait / nowait
#define FORMAT 0      // 0 = names, 10 = decimal, 16 = hex
#define mouseignore   // comment out to track mouse clicks
```

### Visibility

| Option | Effect |
|--------|--------|
| `#define invisible` | Hides console window (stealth mode) |
| `#define visible` | Keeps console window visible (debug mode) |

### Boot Behavior

| Option | Effect |
|--------|--------|
| `#define bootwait` | Pauses logging until system boot completes (10s polling) |
| `#define nowait` | Starts logging immediately |

### Output Format

| Value | Output | Example |
|-------|--------|---------|
| `0` | Named keys | `[ENTER]`, `_` (space) |
| `10` | Decimal codes | `[13]`, `[32]` |
| `16` | Hex codes | `[D]`, `[20]` |

### Mouse Filter

| Directive | Effect |
|-----------|--------|
| `#define mouseignore` | Ignores mouse click events (default) |
| Comment it out | Logs mouse button presses (1 = left, 2 = right) |

---

## Output Formats

### Format 0 — Named Keys (Default)

```
[Window: Notepad - at 2026-06-19T14:23:45]
Hello_world[ENTER]
[Window: Chrome - at 2026-06-19T14:24:01]
www[.].example[.].com[ENTER]
```

### Format 10 — Decimal Codes

```
[Window: Notepad - at 2026-06-19T14:23:45]
[72][101][108][108][111][95][119][111][114][108][100][13]
```

### Format 16 — Hex Codes

```
[Window: Notepad - at 2026-06-19T14:23:45]
[48][65][6c][6c][6f][5f][77][6f][72][6c][64][d]
```

---

## Building

### Prerequisites

- Windows Vista or later (required for `WH_KEYBOARD_LL`)
- Microsoft Visual Studio 2019+ or MinGW-w64
- CMake (optional)

### Visual Studio (Command Line)

```powershell
cl /EHsc /Fe:VoidKeyLogger.exe main.cpp user32.lib
```

### Visual Studio (IDE)

1. Create a new **Console Application** project
2. Add `main.cpp`
3. Build → Build Solution (Ctrl+Shift+B)

### MinGW-w64

```bash
x86_64-w64-mingw32-g++ -static -o VoidKeyLogger.exe main.cpp -luser32 -lgdi32
```

### CMake

```cmake
cmake_minimum_required(VERSION 3.10)
project(VoidKeyLogger)
add_executable(VoidKeyLogger main.cpp)
target_link_libraries(VoidKeyLogger user32 gdi32)
```

```bash
mkdir build && cd build
cmake .. -G "MinGW Makefiles"
make
```

---

## Usage

```bash
VoidKeyLogger.exe
```

No command-line arguments required. The logger will:

1. Display the ASCII banner (if `visible` mode)
2. Optionally wait for system boot (if `bootwait` mode)
3. Install the global keyboard hook
4. Begin logging to timestamped `.log` files in the current directory
5. Run until terminated (Ctrl+C, taskkill, or system shutdown)

### Stopping

```bash
taskkill /IM VoidKeyLogger.exe /F
```

Or use Process Explorer / Task Manager.

---

## Log File Format

Files are named with the timestamp of creation:

```
2026-06-19__14-23-45.log
2026-06-19__15-00-00.log
```

A new file is created **every hour** (when `tm_hour` changes). Each log entry contains:

```
[Window: <window_title> - at <ISO_timestamp>]
<keystroke_data>
```

---

## Stealth Mechanisms

### Console Hiding (`invisible` mode)

Two layers of concealment:

1. `ShowWindow(FindWindowA("ConsoleWindowClass", NULL), 0)` — Hides the console window
2. `FreeConsole()` — Detaches the process from its console entirely (stronger stealth)

For a **visible/debug** mode, define `visible` instead, which calls `ShowWindow(..., 1)`.

### Boot Wait (`bootwait` mode)

```cpp
while (IsSystemBooting()) { Sleep(10000); }
```

Uses `GetSystemMetrics(0x2004)` (undocumented — `SM_GETSYSTEMMETRICS`) to detect if the system is still initializing. This reduces the chance of detection during early boot security software loading.

---

## Detection & Bypass Considerations

This tool is **intentionally straightforward** for educational and authorized testing purposes. It uses known techniques with well-documented signatures:

| Detection Vector | Description |
|-----------------|-------------|
| `WH_KEYBOARD_LL` hook | EDRs/AVs commonly flag global hooks |
| `FreeConsole()` call | Indicator of stealth intent |
| `GetWindowTextA` API | Window enumeration pattern |
| File writes to `.log` | Suspicious file extension in certain contexts |
| `SetWindowsHookEx` with `NULL` module | DLL injection-less hook (runs in calling process) |

### Bypass Suggestions (For Authorized Testing)

*Compile with custom entry point obfuscation*
*Integrate with a process hollowing or reflective loader*
*Encrypt log output before writing to disk*
*Use named pipes or network sockets instead of file I/O*
*Modify hook technique to use `GetAsyncKeyState` polling instead*

---

## Use Cases

- **Red Team Engagements** — Credential harvesting, user behavior profiling
- **Security Audits** — Testing DLP (Data Loss Prevention) solutions
- **Incident Response Training** — Understanding keylogger behavior for forensic analysis
- **Malware Analysis Education** — Studying hook-based keylogging techniques
- **Parental Controls Research** — Understanding keystroke capture mechanisms

---

## Disclaimer

> **This software is provided for authorized security testing and educational purposes only.** Unauthorized use of keyloggers may violate local, state, and federal laws. The authors assume no liability for misuse. You are responsible for complying with all applicable laws and obtaining proper authorization before deploying this tool.

---

## License

MIT License — See [LICENSE](LICENSE) for details.

---

## Acknowledgments

- Windows Low-Level Keyboard Hooks (MSDN)
- Win32 API documentation
- The security research community

---

*Built for the field. Use with authorization.*