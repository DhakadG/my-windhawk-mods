// ==WindhawkMod==
// @id              spicetify-guardian
// @name            Spicetify Guardian
// @description     Keeps Spicetify alive across Spotify updates - detects the moment Spotify wipes it, checks compatibility, and re-applies. Plus a tray dashboard for every other Spicetify chore.
// @version         1.0.0
// @author          lost_husky
// @github          https://github.com/DhakadG
// @donateUrl       https://ko-fi.com/losthusky_
// @license         MIT
// @include         windhawk.exe
// @compilerOptions -DWIN32_LEAN_AND_MEAN -ladvapi32 -lcomctl32 -lgdi32 -lole32 -lshell32 -lshlwapi -luser32 -lversion -lwinhttp
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
# Spicetify Guardian

Spotify updates itself, and every time it does it overwrites `xpui.spa` and your
Spicetify install disappears. You notice when the theme is gone, then you go and
type `spicetify update` again.

This mod does that for you, the moment it happens, and refuses to do it when
doing it would be a bad idea.

## Why another Spicetify auto-updater

The existing ones run a fixed command chain at every login. That works, but:

- they re-patch an install that was already fine,
- they cannot react to an update that lands while you are using Spotify,
- they have no idea whether Spicetify actually supports the Spotify build you
  just received, and
- several of them run `clear backup apply` unconditionally, which - if
  Spicetify happens to still be applied - backs up the *patched* files as if
  they were pristine and quietly corrupts your backup.

This one checks first, every time, and only acts when there is something to fix.

## How it detects the breakage

No polling of version numbers, no guessing.

When Spicetify is applied, `Spotify\Apps\xpui` is a **folder**. When Spotify
updates, it writes `xpui.spa` back as a **file** and deletes the folder. That
inversion is the signal - it is exact, instant, needs no network, and cannot be
fooled by a stale config file.

The mod watches that one folder and reacts within seconds.

## The compatibility gate

Every `spicetify/cli` release publishes the Spotify range it was tested against,
in the release notes:

```
## Compatibility
- Spotify for **Windows/Microsoft Store**: `1.2.14` -> `1.2.93`
```

The mod reads that from GitHub and sorts your situation into one of three:

| Verdict | Meaning | What happens |
| --- | --- | --- |
| **Supported** | Inside the published range | Repairs silently |
| **Untested** | Newer than the published max | Repairs, and says so |
| **Too old** | Older than the published min | Refuses, tells you to update Spotify |

**Untested is the normal state for most of any given month.** Spotify ships
faster than Spicetify cuts releases, and the published range records what was
tested, not the ceiling of what works. So it is a warning, not a wall.

If you would rather it never touch an untested combination, turn on **Strict
mode** in the dashboard. It will then hold off and point you at the downgrade
guide instead.

## Safety

Everything that could go wrong, and what stops it:

- **Never repairs an install that is already fine.** The corrupt-your-backup
  footgun above is gated on the applied-state check.
- **Never races you.** If a `spicetify` process is already running - because you
  are in a terminal doing it by hand - it backs off.
- **Never loses your config.** `config-xpui.ini` is snapshotted before every
  destructive operation. The dashboard restores any of the last 30.
- **Never kills your music mid-song.** Every check runs *before* Spotify is
  touched. Only once a repair is definitely going ahead is Spotify closed - and
  closed politely with `WM_CLOSE`, not killed, so it saves its own state. It is
  restarted afterwards.
- **Never loops.** Three consecutive failures on the same Spotify build and it
  goes quiet until the version changes or you intervene.
- **Never leaves Spotify broken.** If a repair fails, Spotify is restored to
  stock so it still launches, un-themed.

## The tray icon

Left-click opens the dashboard. Right-click gives you repair, restart Spotify,
pause, and the Spotify-update blocking toggles.

The icon dims when automatic repair is paused, and turns amber when Spicetify is
missing.

### Dashboard

- **Status** - versions, applied state, tested range, last run, pause countdown
- **Actions** - repair, update, restore to stock, install/remove Spicetify and
  Marketplace, block/unblock Spotify updates
- **Config** - tick extensions and custom apps on and off, switch theme
- **Health** - the quiet breakages: config entries pointing at files that no
  longer exist, Marketplace installed without its placeholder theme, the same
  extension filename in both extension folders
- **Log** - what it did and why

## Pausing

Right-click the tray icon to pause for 1, 3 or 7 days, or until the next Spotify
update. Useful when you are mid-way through changing something and do not want
anything re-applying underneath you.

## Blocking Spotify updates instead

If you would rather Spotify simply never updated, the dashboard can block it -
either via `spicetify spotify-updates block`, or by denying write access to
Spotify's `Update` folder (the method from the r/spicetify guide, which is the
one that actually holds).

Note that blocking and auto-repair solve the same problem from opposite ends,
and blocking means no Spotify security fixes until you unblock.

## Command-line equivalents

Everything here is also available as standalone PowerShell scripts, for machines
without Windhawk:

https://github.com/DhakadG/spicetify-guardian
*/
// ==/WindhawkModReadme==

// ==WindhawkModSettings==
/*
- autoRepair: true
  $name: Repair automatically
  $description: >-
    Re-apply Spicetify as soon as a Spotify update removes it. Turn this off to
    make the mod detect and notify only, leaving the repair to you from the tray
    icon.
- notifications: true
  $name: Show notifications
  $description: >-
    Notify when a repair happens, is held off, or fails. A check that finds
    nothing wrong is always silent.
- spicetifyPath: ""
  $name: Path to spicetify.exe
  $description: >-
    Leave empty to detect it automatically (PATH, then the two locations the
    official installer uses). Only set this if you installed Spicetify somewhere
    unusual.
*/
// ==/WindhawkModSettings==

// Everything else deliberately lives in the dashboard rather than here.
// Windhawk settings are read-only to a mod, so anything the mod itself needs to
// change - pause state, strict mode, the attempt counter - cannot live here. It
// is persisted to %LOCALAPPDATA%\SpicetifyGuardian\state.json instead, which is
// the same file the PowerShell scripts use, so the two stay in agreement.

#include <windows.h>
#include <aclapi.h>
#include <commctrl.h>
// WIN32_LEAN_AND_MEAN (set in @compilerOptions) keeps windows.h from pulling in
// shellapi.h, so ShellExecuteEx / SHFileOperation / Shell_NotifyIcon need it
// explicitly.
#include <shellapi.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <winhttp.h>

#include <algorithm>
#include <string>
#include <vector>

// =====================================================================
// Constants
// =====================================================================

static constexpr wchar_t kAppName[]     = L"Spicetify Guardian";
static constexpr wchar_t kStateSubdir[] = L"SpicetifyGuardian";

static constexpr UINT WM_APP_TRAY        = WM_APP + 1;
static constexpr UINT WM_APP_STATE_DIRTY = WM_APP + 2;   // worker -> UI: refresh
static constexpr UINT WM_APP_NOTIFY      = WM_APP + 3;   // worker -> UI: balloon

static constexpr UINT kTrayIconId = 1;

// Spotify writes its Apps folder in bursts during an update. Repairing halfway
// through would fight it, so wait for quiet before acting.
static constexpr DWORD kDebounceMs = 15000;

// Backstop in case a directory notification is ever missed.
static constexpr DWORD kPollMs = 15 * 60 * 1000;

// The published compatibility range changes only when Spicetify cuts a release.
// Six hours keeps us responsive while staying far inside GitHub's 60 req/hr
// unauthenticated limit.
static constexpr long long kCompatCacheSeconds = 6 * 60 * 60;

static constexpr int kMaxAttemptsPerVersion = 3;
static constexpr int kMaxSnapshots          = 30;

static constexpr wchar_t kDowngradeGuide[] =
    L"https://spicetify.app/docs/faq#can-i-use-an-older-version-of-spotify";

// Tasks the worker thread can be asked to perform.
enum class Task {
    Repair,
    RepairForced,
    UpdateCli,
    RestoreStock,
    BackupApply,
    RestartSpotify,
    BlockUpdates,
    UnblockUpdates,
    InstallMarketplace,
    RemoveMarketplace,
    InstallSpicetify,
    UninstallSpicetify,
    RefreshCompat,
    ApplyConfig,
};

// =====================================================================
// Small helpers
// =====================================================================

static std::wstring Trim(const std::wstring& s) {
    size_t b = s.find_first_not_of(L" \t\r\n");
    if (b == std::wstring::npos) {
        return L"";
    }
    size_t e = s.find_last_not_of(L" \t\r\n");
    return s.substr(b, e - b + 1);
}

static std::wstring EnvPath(const wchar_t* var, const wchar_t* tail) {
    wchar_t buf[MAX_PATH];
    DWORD n = GetEnvironmentVariableW(var, buf, ARRAYSIZE(buf));
    if (n == 0 || n >= ARRAYSIZE(buf)) {
        return L"";
    }
    std::wstring out = buf;
    if (tail && *tail) {
        if (!out.empty() && out.back() != L'\\') {
            out += L'\\';
        }
        out += tail;
    }
    return out;
}

static bool FileExists(const std::wstring& p) {
    DWORD a = GetFileAttributesW(p.c_str());
    return a != INVALID_FILE_ATTRIBUTES && !(a & FILE_ATTRIBUTE_DIRECTORY);
}

static bool DirExists(const std::wstring& p) {
    DWORD a = GetFileAttributesW(p.c_str());
    return a != INVALID_FILE_ATTRIBUTES && (a & FILE_ATTRIBUTE_DIRECTORY);
}

static long long UnixNow() {
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    ULARGE_INTEGER u{};
    u.LowPart = ft.dwLowDateTime;
    u.HighPart = ft.dwHighDateTime;
    // FILETIME is 100ns ticks since 1601; 11644473600 is the offset to 1970.
    return (long long)(u.QuadPart / 10000000ULL) - 11644473600LL;
}

static std::string ToUtf8(const std::wstring& w) {
    if (w.empty()) {
        return "";
    }
    int n = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(), nullptr, 0, nullptr, nullptr);
    std::string s((size_t)n, '\0');
    WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(), s.data(), n, nullptr, nullptr);
    return s;
}

static std::wstring FromUtf8(const std::string& s) {
    if (s.empty()) {
        return L"";
    }
    int n = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), nullptr, 0);
    std::wstring w((size_t)n, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), w.data(), n);
    return w;
}

static std::vector<std::wstring> Split(const std::wstring& s, wchar_t sep) {
    std::vector<std::wstring> out;
    size_t start = 0;
    while (start <= s.size()) {
        size_t p = s.find(sep, start);
        if (p == std::wstring::npos) {
            std::wstring piece = Trim(s.substr(start));
            if (!piece.empty()) {
                out.push_back(piece);
            }
            break;
        }
        std::wstring piece = Trim(s.substr(start, p - start));
        if (!piece.empty()) {
            out.push_back(piece);
        }
        start = p + 1;
    }
    return out;
}

// Strip the ANSI colour codes spicetify emits, which otherwise land in the log
// and the dashboard as escape gibberish.
static std::wstring StripAnsi(const std::wstring& s) {
    std::wstring out;
    out.reserve(s.size());
    for (size_t i = 0; i < s.size(); i++) {
        if (s[i] == L'\x1b' && i + 1 < s.size() && s[i + 1] == L'[') {
            size_t j = i + 2;
            while (j < s.size() && s[j] != L'm') {
                j++;
            }
            i = (j < s.size()) ? j : s.size() - 1;
            continue;
        }
        out += s[i];
    }
    return out;
}

// =====================================================================
// State directory - shared with the PowerShell scripts
// =====================================================================

static std::wstring StateDir() {
    return EnvPath(L"LOCALAPPDATA", kStateSubdir);
}
static std::wstring SnapshotDir() {
    return StateDir() + L"\\snapshots";
}
static std::wstring StateFile() {
    return StateDir() + L"\\state.json";
}
static std::wstring CompatCacheFile() {
    return StateDir() + L"\\compat-cache.json";
}
static std::wstring LogFile() {
    return StateDir() + L"\\guardian.log";
}

static void EnsureStateDir() {
    std::wstring d = StateDir();
    if (!d.empty()) {
        SHCreateDirectoryExW(nullptr, d.c_str(), nullptr);
        SHCreateDirectoryExW(nullptr, SnapshotDir().c_str(), nullptr);
    }
}

static CRITICAL_SECTION g_logLock;

// The log is shared with the scripts, so it uses their exact line format:
//   [yyyy-MM-dd HH:mm:ss] [LEVEL] message
static void GuardianLog(const wchar_t* level, const std::wstring& msg) {
    Wh_Log(L"[%s] %s", level, msg.c_str());

    EnsureStateDir();
    std::wstring path = LogFile();
    if (path.empty()) {
        return;
    }

    SYSTEMTIME st;
    GetLocalTime(&st);
    wchar_t stamp[64];
    swprintf_s(stamp, L"[%04d-%02d-%02d %02d:%02d:%02d] [%s] ", st.wYear, st.wMonth, st.wDay,
               st.wHour, st.wMinute, st.wSecond, level);

    std::string line = ToUtf8(std::wstring(stamp) + StripAnsi(msg) + L"\r\n");

    EnterCriticalSection(&g_logLock);
    HANDLE h = CreateFileW(path.c_str(), FILE_APPEND_DATA, FILE_SHARE_READ | FILE_SHARE_WRITE,
                           nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h != INVALID_HANDLE_VALUE) {
        DWORD written = 0;
        WriteFile(h, line.data(), (DWORD)line.size(), &written, nullptr);
        CloseHandle(h);
    }
    LeaveCriticalSection(&g_logLock);
}

static void LogF(const wchar_t* level, const wchar_t* fmt, ...) {
    wchar_t buf[2048];
    va_list ap;
    va_start(ap, fmt);
    _vsnwprintf_s(buf, ARRAYSIZE(buf), _TRUNCATE, fmt, ap);
    va_end(ap);
    GuardianLog(level, buf);
}

// =====================================================================
// File IO
// =====================================================================

static std::wstring ReadTextFile(const std::wstring& path) {
    HANDLE h = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr,
                           OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) {
        return L"";
    }

    LARGE_INTEGER size{};
    if (!GetFileSizeEx(h, &size) || size.QuadPart <= 0 || size.QuadPart > (32 << 20)) {
        CloseHandle(h);
        return L"";
    }

    std::string buf((size_t)size.QuadPart, '\0');
    DWORD read = 0;
    BOOL ok = ReadFile(h, buf.data(), (DWORD)buf.size(), &read, nullptr);
    CloseHandle(h);
    if (!ok) {
        return L"";
    }
    buf.resize(read);

    // Skip a UTF-8 BOM; PowerShell's Set-Content -Encoding UTF8 writes one on
    // Windows PowerShell 5.1.
    if (buf.size() >= 3 && (unsigned char)buf[0] == 0xEF && (unsigned char)buf[1] == 0xBB &&
        (unsigned char)buf[2] == 0xBF) {
        buf.erase(0, 3);
    }
    return FromUtf8(buf);
}

static bool WriteTextFile(const std::wstring& path, const std::wstring& text) {
    std::string utf8 = ToUtf8(text);
    HANDLE h = CreateFileW(path.c_str(), GENERIC_WRITE, FILE_SHARE_READ, nullptr, CREATE_ALWAYS,
                           FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) {
        return false;
    }
    DWORD written = 0;
    BOOL ok = WriteFile(h, utf8.data(), (DWORD)utf8.size(), &written, nullptr);
    CloseHandle(h);
    return ok && written == utf8.size();
}

// =====================================================================
// Flat JSON  (state.json / compat-cache.json only)
// =====================================================================
//
// These two files are written by the PowerShell side with ConvertTo-Json and
// are flat objects of strings, numbers and booleans. A full JSON parser would
// be a dependency for no gain, so this reads exactly that shape and nothing
// more. Anything unrecognised falls back to the caller's default.

static bool JsonFindValue(const std::wstring& json, const std::wstring& key, std::wstring* out) {
    std::wstring needle = L"\"" + key + L"\"";
    size_t p = json.find(needle);
    if (p == std::wstring::npos) {
        return false;
    }
    p = json.find(L':', p + needle.size());
    if (p == std::wstring::npos) {
        return false;
    }
    p++;
    while (p < json.size() && iswspace(json[p])) {
        p++;
    }
    if (p >= json.size()) {
        return false;
    }

    if (json[p] == L'"') {
        p++;
        std::wstring v;
        while (p < json.size() && json[p] != L'"') {
            if (json[p] == L'\\' && p + 1 < json.size()) {
                p++;
                switch (json[p]) {
                    case L'n': v += L'\n'; break;
                    case L'r': v += L'\r'; break;
                    case L't': v += L'\t'; break;
                    default:   v += json[p]; break;
                }
            } else {
                v += json[p];
            }
            p++;
        }
        *out = v;
        return true;
    }

    size_t e = json.find_first_of(L",}\r\n", p);
    if (e == std::wstring::npos) {
        e = json.size();
    }
    *out = Trim(json.substr(p, e - p));
    return true;
}

static std::wstring JsonStr(const std::wstring& json, const wchar_t* key, const wchar_t* def = L"") {
    std::wstring v;
    return JsonFindValue(json, key, &v) ? v : def;
}

static long long JsonInt(const std::wstring& json, const wchar_t* key, long long def = 0) {
    std::wstring v;
    if (!JsonFindValue(json, key, &v) || v.empty() || v == L"null") {
        return def;
    }
    return _wtoi64(v.c_str());
}

static bool JsonBool(const std::wstring& json, const wchar_t* key, bool def = false) {
    std::wstring v;
    if (!JsonFindValue(json, key, &v)) {
        return def;
    }
    return _wcsicmp(v.c_str(), L"true") == 0 || v == L"1";
}

static std::wstring JsonEscape(const std::wstring& s) {
    std::wstring o;
    for (wchar_t c : s) {
        switch (c) {
            case L'"':  o += L"\\\""; break;
            case L'\\': o += L"\\\\"; break;
            case L'\n': o += L"\\n";  break;
            case L'\r': o += L"\\r";  break;
            case L'\t': o += L"\\t";  break;
            default:    o += c;       break;
        }
    }
    return o;
}

// =====================================================================
// Persistent state
// =====================================================================

struct GuardianState {
    long long pauseUntil = 0;        // 0 none, -1 until next Spotify version
    std::wstring pauseAtVersion;
    bool strictMode = false;
    std::wstring lastSpotifyVersion;
    int attempts = 0;
    std::wstring lastRunUtc;
    std::wstring lastRunResult;
    std::wstring lastRunDetail;
};

static CRITICAL_SECTION g_stateLock;
static GuardianState g_state;

static void LoadState() {
    std::wstring json = ReadTextFile(StateFile());
    EnterCriticalSection(&g_stateLock);
    if (!json.empty()) {
        g_state.pauseUntil         = JsonInt(json, L"pauseUntil", 0);
        g_state.pauseAtVersion     = JsonStr(json, L"pauseAtVersion");
        g_state.strictMode         = JsonBool(json, L"strictMode", false);
        g_state.lastSpotifyVersion = JsonStr(json, L"lastSpotifyVersion");
        g_state.attempts           = (int)JsonInt(json, L"attempts", 0);
        g_state.lastRunUtc         = JsonStr(json, L"lastRunUtc");
        g_state.lastRunResult      = JsonStr(json, L"lastRunResult");
        g_state.lastRunDetail      = JsonStr(json, L"lastRunDetail");
    }
    LeaveCriticalSection(&g_stateLock);
}

static void SaveState() {
    EnsureStateDir();

    EnterCriticalSection(&g_stateLock);
    GuardianState s = g_state;
    LeaveCriticalSection(&g_stateLock);

    // Key names and casing must match what the PowerShell side writes, or the
    // two halves stop seeing each other's pause and strict-mode settings.
    std::wstring json = L"{\r\n";
    json += L"  \"pauseUntil\": " + std::to_wstring(s.pauseUntil) + L",\r\n";
    json += L"  \"pauseAtVersion\": \"" + JsonEscape(s.pauseAtVersion) + L"\",\r\n";
    json += std::wstring(L"  \"strictMode\": ") + (s.strictMode ? L"true" : L"false") + L",\r\n";
    json += L"  \"lastSpotifyVersion\": \"" + JsonEscape(s.lastSpotifyVersion) + L"\",\r\n";
    json += L"  \"attempts\": " + std::to_wstring(s.attempts) + L",\r\n";
    json += L"  \"lastRunUtc\": \"" + JsonEscape(s.lastRunUtc) + L"\",\r\n";
    json += L"  \"lastRunResult\": \"" + JsonEscape(s.lastRunResult) + L"\",\r\n";
    json += L"  \"lastRunDetail\": \"" + JsonEscape(s.lastRunDetail) + L"\"\r\n";
    json += L"}\r\n";

    WriteTextFile(StateFile(), json);
}

static std::wstring UtcNowIso() {
    SYSTEMTIME st;
    GetSystemTime(&st);
    wchar_t buf[64];
    swprintf_s(buf, L"%04d-%02d-%02dT%02d:%02d:%02d.0000000Z", st.wYear, st.wMonth, st.wDay,
               st.wHour, st.wMinute, st.wSecond);
    return buf;
}

// =====================================================================
// Mod settings
// =====================================================================

struct ModSettings {
    bool autoRepair = true;
    bool notifications = true;
    std::wstring spicetifyPath;
};

static CRITICAL_SECTION g_settingsLock;
static ModSettings g_settings;

static std::wstring GetSettingStr(const wchar_t* name) {
    PCWSTR raw = Wh_GetStringSetting(name);
    std::wstring out = raw;   // Wh_GetStringSetting returns L"", never NULL
    Wh_FreeStringSetting(raw);
    return out;
}

static void LoadModSettings() {
    ModSettings s;
    s.autoRepair = Wh_GetIntSetting(L"autoRepair") != 0;
    s.notifications = Wh_GetIntSetting(L"notifications") != 0;
    s.spicetifyPath = Trim(GetSettingStr(L"spicetifyPath"));

    EnterCriticalSection(&g_settingsLock);
    g_settings = s;
    LeaveCriticalSection(&g_settingsLock);
}

static ModSettings Settings() {
    EnterCriticalSection(&g_settingsLock);
    ModSettings s = g_settings;
    LeaveCriticalSection(&g_settingsLock);
    return s;
}

// =====================================================================
// INI reading  (config-xpui.ini)
// =====================================================================

static std::wstring IniGet(const std::wstring& text,
                           const std::wstring& section,
                           const std::wstring& key) {
    std::wstring cur;
    size_t pos = 0;

    while (pos <= text.size()) {
        size_t eol = text.find(L'\n', pos);
        std::wstring line = Trim(text.substr(pos, (eol == std::wstring::npos) ? std::wstring::npos
                                                                              : eol - pos));
        if (eol == std::wstring::npos) {
            pos = text.size() + 1;
        } else {
            pos = eol + 1;
        }

        if (line.empty() || line[0] == L';' || line[0] == L'#') {
            continue;
        }
        if (line.front() == L'[' && line.back() == L']') {
            cur = line.substr(1, line.size() - 2);
            continue;
        }
        if (cur != section) {
            continue;
        }

        size_t eq = line.find(L'=');
        if (eq == std::wstring::npos) {
            continue;
        }
        if (Trim(line.substr(0, eq)) == key) {
            return Trim(line.substr(eq + 1));
        }
    }
    return L"";
}

// =====================================================================
// Discovery
// =====================================================================

static std::wstring FindSpicetifyExe() {
    ModSettings s = Settings();
    if (!s.spicetifyPath.empty() && FileExists(s.spicetifyPath)) {
        return s.spicetifyPath;
    }

    // The two locations the official installer uses, then PATH. PATH is checked
    // last because a stale entry left by an old uninstall is a real thing.
    std::wstring candidates[] = {
        EnvPath(L"LOCALAPPDATA", L"spicetify\\spicetify.exe"),
        EnvPath(L"APPDATA", L"spicetify\\spicetify.exe"),
    };
    for (const auto& c : candidates) {
        if (!c.empty() && FileExists(c)) {
            return c;
        }
    }

    wchar_t found[MAX_PATH] = {};
    if (SearchPathW(nullptr, L"spicetify.exe", nullptr, ARRAYSIZE(found), found, nullptr)) {
        return found;
    }
    return L"";
}

static std::wstring SpicetifyConfigPath() {
    std::wstring p = EnvPath(L"SPICETIFY_CONFIG", L"config-xpui.ini");
    if (!p.empty() && FileExists(p)) {
        return p;
    }
    p = EnvPath(L"APPDATA", L"spicetify\\config-xpui.ini");
    return FileExists(p) ? p : L"";
}

struct SpotifyPaths {
    std::wstring root;
    std::wstring exe;
    std::wstring appsDir;
    std::wstring updateDir;
    bool valid = false;
};

static SpotifyPaths FindSpotify() {
    SpotifyPaths sp;

    // Spicetify's own config wins: the user may have pointed it at a
    // non-default install, and that is the one Spicetify will patch.
    std::wstring cfg = SpicetifyConfigPath();
    if (!cfg.empty()) {
        std::wstring fromCfg = IniGet(ReadTextFile(cfg), L"Setting", L"spotify_path");
        if (!fromCfg.empty() && FileExists(fromCfg + L"\\Spotify.exe")) {
            sp.root = fromCfg;
        }
    }

    if (sp.root.empty()) {
        std::wstring candidates[] = {
            EnvPath(L"APPDATA", L"Spotify"),
            EnvPath(L"LOCALAPPDATA", L"Spotify"),
            EnvPath(L"ProgramFiles", L"Spotify"),
        };
        for (const auto& c : candidates) {
            if (!c.empty() && FileExists(c + L"\\Spotify.exe")) {
                sp.root = c;
                break;
            }
        }
    }

    if (sp.root.empty()) {
        return sp;
    }

    sp.exe = sp.root + L"\\Spotify.exe";
    sp.appsDir = sp.root + L"\\Apps";
    sp.updateDir = EnvPath(L"LOCALAPPDATA", L"Spotify\\Update");
    sp.valid = true;
    return sp;
}

static std::wstring FileVersionOf(const std::wstring& path) {
    DWORD dummy = 0;
    DWORD size = GetFileVersionInfoSizeW(path.c_str(), &dummy);
    if (size == 0) {
        return L"";
    }

    std::vector<BYTE> buf(size);
    if (!GetFileVersionInfoW(path.c_str(), 0, size, buf.data())) {
        return L"";
    }

    VS_FIXEDFILEINFO* ffi = nullptr;
    UINT len = 0;
    if (!VerQueryValueW(buf.data(), L"\\", (LPVOID*)&ffi, &len) || !ffi) {
        return L"";
    }

    wchar_t out[64];
    swprintf_s(out, L"%u.%u.%u.%u", HIWORD(ffi->dwFileVersionMS), LOWORD(ffi->dwFileVersionMS),
               HIWORD(ffi->dwFileVersionLS), LOWORD(ffi->dwFileVersionLS));
    return out;
}

// Compare only the first three components. Spotify reports "1.2.96.518" and
// Spicetify's notes say "1.2.93"; the build number is meaningless here.
struct Ver3 {
    int a = -1, b = -1, c = -1;
    bool ok() const { return a >= 0; }
};

static Ver3 ParseVer3(const std::wstring& s) {
    Ver3 v;
    if (swscanf_s(s.c_str(), L"%d.%d.%d", &v.a, &v.b, &v.c) != 3) {
        v.a = -1;
    }
    return v;
}

static int CompareVer3(const Ver3& x, const Ver3& y) {
    if (x.a != y.a) return x.a < y.a ? -1 : 1;
    if (x.b != y.b) return x.b < y.b ? -1 : 1;
    if (x.c != y.c) return x.c < y.c ? -1 : 1;
    return 0;
}

// =====================================================================
// Running a process and capturing its output
// =====================================================================

struct RunResult {
    DWORD exitCode = (DWORD)-1;
    std::wstring output;
    bool success = false;
};

static RunResult RunCapture(const std::wstring& exe,
                            const std::wstring& args,
                            DWORD timeoutMs = 300000) {
    RunResult r;

    SECURITY_ATTRIBUTES sa{};
    sa.nLength = sizeof(sa);
    sa.bInheritHandle = TRUE;

    HANDLE readPipe = nullptr;
    HANDLE writePipe = nullptr;
    if (!CreatePipe(&readPipe, &writePipe, &sa, 0)) {
        r.output = L"CreatePipe failed";
        return r;
    }
    SetHandleInformation(readPipe, HANDLE_FLAG_INHERIT, 0);

    STARTUPINFOW si{};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;
    si.hStdOutput = writePipe;
    si.hStdError = writePipe;
    si.hStdInput = nullptr;

    std::wstring cmd = L"\"" + exe + L"\" " + args;
    std::vector<wchar_t> mutableCmd(cmd.begin(), cmd.end());
    mutableCmd.push_back(L'\0');

    PROCESS_INFORMATION pi{};
    BOOL ok = CreateProcessW(nullptr, mutableCmd.data(), nullptr, nullptr, TRUE,
                             CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi);
    CloseHandle(writePipe);

    if (!ok) {
        CloseHandle(readPipe);
        r.output = L"CreateProcess failed";
        return r;
    }

    // Read to EOF first. Waiting on the process before draining the pipe
    // deadlocks as soon as the child writes more than the pipe buffer, which
    // spicetify's progress output comfortably does.
    std::string raw;
    char buf[4096];
    DWORD read = 0;
    while (ReadFile(readPipe, buf, sizeof(buf), &read, nullptr) && read > 0) {
        raw.append(buf, read);
    }
    CloseHandle(readPipe);

    if (WaitForSingleObject(pi.hProcess, timeoutMs) == WAIT_TIMEOUT) {
        TerminateProcess(pi.hProcess, 1);
        r.output = L"Timed out";
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
        return r;
    }

    GetExitCodeProcess(pi.hProcess, &r.exitCode);
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);

    r.output = StripAnsi(FromUtf8(raw));
    r.success = (r.exitCode == 0);
    return r;
}

static RunResult RunSpicetify(const std::wstring& args) {
    std::wstring exe = FindSpicetifyExe();
    if (exe.empty()) {
        RunResult r;
        r.output = L"spicetify.exe not found";
        return r;
    }

    LogF(L"ACTION", L"> spicetify %s", args.c_str());
    RunResult r = RunCapture(exe, args);

    // Echo each line into the shared log so the dashboard's Log tab and the
    // scripts' log show the same thing.
    size_t start = 0;
    while (start < r.output.size()) {
        size_t eol = r.output.find(L'\n', start);
        std::wstring line = Trim(r.output.substr(
            start, (eol == std::wstring::npos) ? std::wstring::npos : eol - start));
        if (!line.empty()) {
            GuardianLog(L"INFO", L"  " + line);
        }
        if (eol == std::wstring::npos) {
            break;
        }
        start = eol + 1;
    }
    return r;
}

static std::wstring GetSpicetifyVersion() {
    std::wstring exe = FindSpicetifyExe();
    if (exe.empty()) {
        return L"";
    }
    RunResult r = RunCapture(exe, L"-v", 15000);
    std::wstring out = Trim(r.output);

    // `spicetify -v` prints just the number, but be tolerant of a banner.
    for (size_t i = 0; i < out.size(); i++) {
        if (iswdigit(out[i])) {
            size_t j = i;
            while (j < out.size() && (iswdigit(out[j]) || out[j] == L'.')) {
                j++;
            }
            std::wstring cand = out.substr(i, j - i);
            if (ParseVer3(cand).ok()) {
                return cand;
            }
            i = j;
        }
    }
    return L"";
}

// =====================================================================
// Status
// =====================================================================

enum class Health { Applied, Wiped, StaleBackup, NotInstalled, NoSpotify };

struct Status {
    Health health = Health::NoSpotify;
    bool applied = false;
    bool spotifyInstalled = false;
    bool spotifyRunning = false;
    bool spicetifyInstalled = false;
    bool backupMatches = false;
    bool marketplaceInstalled = false;
    bool updatesBlocked = false;
    std::wstring spotifyVersion;
    std::wstring spicetifyVersion;
    std::wstring backupVersion;
    std::wstring backupWith;
    std::wstring theme;
    std::wstring colorScheme;
    std::vector<std::wstring> extensions;
    std::vector<std::wstring> customApps;
    SpotifyPaths paths;
};

static bool IsProcessRunning(const wchar_t* imageName);
static bool AreUpdatesBlocked();

// The core detection. When Spicetify is applied, Apps\xpui is a FOLDER and
// xpui.spa is gone; when Spotify updates it puts xpui.spa back as a FILE and
// deletes the folder. Exact, instant, no network.
static bool TestSpicetifyApplied(const SpotifyPaths& sp) {
    if (!sp.valid) {
        return false;
    }
    bool dir = DirExists(sp.appsDir + L"\\xpui");
    bool spa = FileExists(sp.appsDir + L"\\xpui.spa");
    return dir && !spa;
}

static Status GetStatus(bool includeVersions = true) {
    Status s;
    s.paths = FindSpotify();
    s.spotifyInstalled = s.paths.valid;

    if (s.paths.valid) {
        s.spotifyVersion = FileVersionOf(s.paths.exe);
        s.applied = TestSpicetifyApplied(s.paths);
    }
    s.spotifyRunning = IsProcessRunning(L"Spotify.exe");

    std::wstring exe = FindSpicetifyExe();
    s.spicetifyInstalled = !exe.empty();
    if (s.spicetifyInstalled && includeVersions) {
        s.spicetifyVersion = GetSpicetifyVersion();
    }

    std::wstring cfgPath = SpicetifyConfigPath();
    if (!cfgPath.empty()) {
        std::wstring ini = ReadTextFile(cfgPath);
        s.backupVersion = IniGet(ini, L"Backup", L"version");
        s.backupWith    = IniGet(ini, L"Backup", L"with");
        s.theme         = IniGet(ini, L"Setting", L"current_theme");
        s.colorScheme   = IniGet(ini, L"Setting", L"color_scheme");
        s.extensions    = Split(IniGet(ini, L"AdditionalOptions", L"extensions"), L'|');
        s.customApps    = Split(IniGet(ini, L"AdditionalOptions", L"custom_apps"), L'|');
    }

    for (const auto& a : s.customApps) {
        if (_wcsicmp(a.c_str(), L"marketplace") == 0) {
            s.marketplaceInstalled = true;
            break;
        }
    }

    s.updatesBlocked = AreUpdatesBlocked();

    // The backup records Spotify's full build string ("1.2.96.518.g366879e1"),
    // which begins with the exe's FileVersion. A prefix mismatch means the
    // patch on disk was built against a different Spotify.
    if (!s.backupVersion.empty() && !s.spotifyVersion.empty()) {
        s.backupMatches = s.backupVersion.compare(0, s.spotifyVersion.size(), s.spotifyVersion) == 0;
    }

    if (!s.spotifyInstalled) {
        s.health = Health::NoSpotify;
    } else if (!s.spicetifyInstalled) {
        s.health = Health::NotInstalled;
    } else if (!s.applied) {
        s.health = Health::Wiped;
    } else if (!s.backupMatches && !s.backupVersion.empty()) {
        s.health = Health::StaleBackup;
    } else {
        s.health = Health::Applied;
    }

    return s;
}

// =====================================================================
// Process helpers
// =====================================================================

#include <tlhelp32.h>

static bool IsProcessRunning(const wchar_t* imageName) {
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) {
        return false;
    }

    PROCESSENTRY32W pe{};
    pe.dwSize = sizeof(pe);
    bool found = false;
    if (Process32FirstW(snap, &pe)) {
        do {
            if (_wcsicmp(pe.szExeFile, imageName) == 0) {
                found = true;
                break;
            }
        } while (Process32NextW(snap, &pe));
    }
    CloseHandle(snap);
    return found;
}

static std::vector<DWORD> GetProcessIds(const wchar_t* imageName) {
    std::vector<DWORD> ids;
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) {
        return ids;
    }

    PROCESSENTRY32W pe{};
    pe.dwSize = sizeof(pe);
    if (Process32FirstW(snap, &pe)) {
        do {
            if (_wcsicmp(pe.szExeFile, imageName) == 0) {
                ids.push_back(pe.th32ProcessID);
            }
        } while (Process32NextW(snap, &pe));
    }
    CloseHandle(snap);
    return ids;
}

struct CloseEnumCtx {
    const std::vector<DWORD>* pids;
    int posted;
};

static BOOL CALLBACK CloseWindowsProc(HWND hWnd, LPARAM lParam) {
    auto* ctx = (CloseEnumCtx*)lParam;

    DWORD pid = 0;
    GetWindowThreadProcessId(hWnd, &pid);
    if (pid == 0) {
        return TRUE;
    }
    if (std::find(ctx->pids->begin(), ctx->pids->end(), pid) == ctx->pids->end()) {
        return TRUE;
    }
    // Only top-level visible windows; Spotify has a swarm of hidden helpers and
    // posting WM_CLOSE to those achieves nothing.
    if (!IsWindowVisible(hWnd) || GetWindow(hWnd, GW_OWNER) != nullptr) {
        return TRUE;
    }

    PostMessageW(hWnd, WM_CLOSE, 0, 0);
    ctx->posted++;
    return TRUE;
}

// Close Spotify the polite way. WM_CLOSE lets it flush playback position, queue
// and cache; a kill loses all of that. Only escalate if it will not go.
// Returns true if Spotify was running and is now stopped, i.e. the caller owns
// restarting it.
static bool StopSpotifyGracefully(DWORD timeoutMs = 10000) {
    std::vector<DWORD> pids = GetProcessIds(L"Spotify.exe");
    if (pids.empty()) {
        return false;
    }

    LogF(L"ACTION", L"Closing Spotify (%d process(es))...", (int)pids.size());

    CloseEnumCtx ctx{&pids, 0};
    EnumWindows(CloseWindowsProc, (LPARAM)&ctx);

    ULONGLONG deadline = GetTickCount64() + timeoutMs;
    while (GetTickCount64() < deadline) {
        Sleep(250);
        if (!IsProcessRunning(L"Spotify.exe")) {
            GuardianLog(L"INFO", L"Spotify closed cleanly.");
            return true;
        }
    }

    GuardianLog(L"WARN", L"Spotify did not close in time; terminating.");
    for (DWORD pid : GetProcessIds(L"Spotify.exe")) {
        HANDLE h = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
        if (h) {
            TerminateProcess(h, 0);
            CloseHandle(h);
        }
    }
    Sleep(500);
    return true;
}

static bool StartSpotify() {
    SpotifyPaths sp = FindSpotify();
    if (!sp.valid) {
        GuardianLog(L"ERROR", L"Cannot start Spotify: executable not found.");
        return false;
    }

    SHELLEXECUTEINFOW ei{};
    ei.cbSize = sizeof(ei);
    ei.fMask = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_FLAG_NO_UI;
    ei.lpFile = sp.exe.c_str();
    ei.lpDirectory = sp.root.c_str();
    ei.nShow = SW_SHOWNORMAL;

    if (!ShellExecuteExW(&ei)) {
        LogF(L"ERROR", L"Could not start Spotify (error %lu).", GetLastError());
        return false;
    }
    if (ei.hProcess) {
        CloseHandle(ei.hProcess);
    }
    GuardianLog(L"ACTION", L"Spotify restarted.");
    return true;
}

// =====================================================================
// Snapshots
// =====================================================================

static void PruneSnapshots() {
    std::vector<std::wstring> names;
    WIN32_FIND_DATAW fd{};
    HANDLE h = FindFirstFileW((SnapshotDir() + L"\\config-xpui-*.ini").c_str(), &fd);
    if (h == INVALID_HANDLE_VALUE) {
        return;
    }
    do {
        if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            names.push_back(fd.cFileName);
        }
    } while (FindNextFileW(h, &fd));
    FindClose(h);

    // Sorted by NAME: CopyFile preserves the source timestamp, so every
    // snapshot of an unchanged config shares an mtime. The yyyyMMdd-HHmmss in
    // the filename is the real capture time and sorts correctly as text.
    std::sort(names.begin(), names.end(), std::greater<std::wstring>());
    for (size_t i = kMaxSnapshots; i < names.size(); i++) {
        DeleteFileW((SnapshotDir() + L"\\" + names[i]).c_str());
    }
}

static bool SnapshotConfig(const wchar_t* reason) {
    std::wstring cfg = SpicetifyConfigPath();
    if (cfg.empty()) {
        return false;
    }
    EnsureStateDir();

    SYSTEMTIME st;
    GetLocalTime(&st);
    wchar_t name[128];
    swprintf_s(name, L"config-xpui-%04d%02d%02d-%02d%02d%02d-%s.ini", st.wYear, st.wMonth, st.wDay,
               st.wHour, st.wMinute, st.wSecond, reason);

    std::wstring dest = SnapshotDir() + L"\\" + name;
    if (!CopyFileW(cfg.c_str(), dest.c_str(), FALSE)) {
        LogF(L"ERROR", L"Snapshot failed (error %lu).", GetLastError());
        return false;
    }

    LogF(L"INFO", L"Config snapshot saved: %s", name);
    PruneSnapshots();
    return true;
}

static std::vector<std::wstring> ListSnapshots() {
    std::vector<std::wstring> names;
    WIN32_FIND_DATAW fd{};
    HANDLE h = FindFirstFileW((SnapshotDir() + L"\\config-xpui-*.ini").c_str(), &fd);
    if (h == INVALID_HANDLE_VALUE) {
        return names;
    }
    do {
        if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            names.push_back(fd.cFileName);
        }
    } while (FindNextFileW(h, &fd));
    FindClose(h);

    std::sort(names.begin(), names.end(), std::greater<std::wstring>());
    return names;
}

// =====================================================================
// Compatibility gate
// =====================================================================

struct Compat {
    bool available = false;
    std::wstring min;
    std::wstring max;
    std::wstring tag;
    std::wstring latestTag;
};

enum class Tier { Supported, Untested, TooOld, Unknown };

struct Verdict {
    Tier tier = Tier::Unknown;
    bool shouldProceed = true;
    bool upgradeFirst = false;
    Compat compat;
    std::wstring message;
};

static CRITICAL_SECTION g_compatLock;
static Verdict g_lastVerdict;

// HTTPS GET with a User-Agent. Wh_GetUrlContent cannot set headers and the
// GitHub API rejects requests without one, so this goes direct to WinHTTP.
static bool HttpGet(const wchar_t* host, const wchar_t* path, std::string* body) {
    bool ok = false;
    HINTERNET ses = WinHttpOpen(L"SpicetifyGuardian/1.0",
                                WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY,
                                WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!ses) {
        return false;
    }
    WinHttpSetTimeouts(ses, 8000, 8000, 15000, 20000);

    HINTERNET con = WinHttpConnect(ses, host, INTERNET_DEFAULT_HTTPS_PORT, 0);
    HINTERNET req = con ? WinHttpOpenRequest(con, L"GET", path, nullptr, WINHTTP_NO_REFERER,
                                             WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE)
                        : nullptr;

    if (req) {
        static const wchar_t kHeaders[] =
            L"Accept: application/vnd.github+json\r\n"
            L"X-GitHub-Api-Version: 2022-11-28\r\n";

        if (WinHttpSendRequest(req, kHeaders, (DWORD)-1L, WINHTTP_NO_REQUEST_DATA, 0, 0, 0) &&
            WinHttpReceiveResponse(req, nullptr)) {
            DWORD status = 0;
            DWORD len = sizeof(status);
            WinHttpQueryHeaders(req, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                                WINHTTP_HEADER_NAME_BY_INDEX, &status, &len, WINHTTP_NO_HEADER_INDEX);

            if (status == 200) {
                DWORD avail = 0;
                while (WinHttpQueryDataAvailable(req, &avail) && avail > 0) {
                    size_t prev = body->size();
                    body->resize(prev + avail);
                    DWORD read = 0;
                    if (!WinHttpReadData(req, body->data() + prev, avail, &read)) {
                        break;
                    }
                    body->resize(prev + read);
                    // Guard against an unbounded response.
                    if (body->size() > (4 << 20)) {
                        break;
                    }
                }
                ok = !body->empty();
            } else {
                LogF(L"WARN", L"GitHub API returned HTTP %lu.", status);
            }
        }
    }

    if (req) WinHttpCloseHandle(req);
    if (con) WinHttpCloseHandle(con);
    WinHttpCloseHandle(ses);
    return ok;
}

// Pull `- Spotify for **Windows/Microsoft Store**: `1.2.14` -> `1.2.93`` out of
// a release body. This block is the only machine-readable compatibility
// statement the Spicetify project publishes.
static bool ParseCompatBlock(const std::wstring& body, std::wstring* minOut, std::wstring* maxOut) {
    size_t p = body.find(L"Spotify for **Windows");
    if (p == std::wstring::npos) {
        return false;
    }
    size_t eol = body.find(L'\n', p);
    std::wstring line = body.substr(p, (eol == std::wstring::npos) ? std::wstring::npos : eol - p);

    std::vector<std::wstring> ticks;
    size_t q = 0;
    while (true) {
        size_t a = line.find(L'`', q);
        if (a == std::wstring::npos) break;
        size_t b = line.find(L'`', a + 1);
        if (b == std::wstring::npos) break;
        ticks.push_back(line.substr(a + 1, b - a - 1));
        q = b + 1;
    }

    if (ticks.size() < 2) {
        return false;
    }
    *minOut = ticks[0];
    *maxOut = ticks[1];
    return ParseVer3(*minOut).ok() && ParseVer3(*maxOut).ok();
}

static bool LoadCompatCache(const std::wstring& forVersion, Compat* out) {
    std::wstring json = ReadTextFile(CompatCacheFile());
    if (json.empty()) {
        return false;
    }
    if (UnixNow() - JsonInt(json, L"fetchedAt", 0) >= kCompatCacheSeconds) {
        return false;
    }
    if (JsonStr(json, L"forVersion") != forVersion) {
        return false;
    }

    out->available = true;
    out->min = JsonStr(json, L"min");
    out->max = JsonStr(json, L"max");
    out->tag = JsonStr(json, L"tag");
    out->latestTag = JsonStr(json, L"latestTag");
    return !out->min.empty() && !out->max.empty();
}

static void SaveCompatCache(const std::wstring& forVersion, const Compat& c) {
    EnsureStateDir();
    std::wstring json = L"{\r\n";
    json += L"  \"fetchedAt\": " + std::to_wstring(UnixNow()) + L",\r\n";
    json += L"  \"forVersion\": \"" + JsonEscape(forVersion) + L"\",\r\n";
    json += L"  \"min\": \"" + JsonEscape(c.min) + L"\",\r\n";
    json += L"  \"max\": \"" + JsonEscape(c.max) + L"\",\r\n";
    json += L"  \"tag\": \"" + JsonEscape(c.tag) + L"\",\r\n";
    json += L"  \"latestTag\": \"" + JsonEscape(c.latestTag) + L"\"\r\n";
    json += L"}\r\n";
    WriteTextFile(CompatCacheFile(), json);
}

static Compat FetchCompat(const std::wstring& spicetifyVersion, bool force) {
    Compat c;
    if (!force && LoadCompatCache(spicetifyVersion, &c)) {
        return c;
    }
    c = Compat{};

    std::string raw;
    if (!HttpGet(L"api.github.com", L"/repos/spicetify/cli/releases/latest", &raw)) {
        GuardianLog(L"WARN", L"Compatibility check unavailable (no response from GitHub).");
        return c;
    }

    std::wstring json = FromUtf8(raw);
    c.latestTag = JsonStr(json, L"tag_name");
    std::wstring body = JsonStr(json, L"body");
    std::wstring tag = c.latestTag;

    // If the installed CLI is older than latest, ITS release notes describe the
    // range that actually applies right now.
    std::wstring latestNoV = c.latestTag;
    if (!latestNoV.empty() && (latestNoV[0] == L'v' || latestNoV[0] == L'V')) {
        latestNoV.erase(0, 1);
    }
    if (!spicetifyVersion.empty() && latestNoV != spicetifyVersion) {
        std::string ownRaw;
        std::wstring path = L"/repos/spicetify/cli/releases/tags/v" + spicetifyVersion;
        if (HttpGet(L"api.github.com", path.c_str(), &ownRaw)) {
            std::wstring ownJson = FromUtf8(ownRaw);
            std::wstring ownBody = JsonStr(ownJson, L"body");
            if (!ownBody.empty()) {
                body = ownBody;
                tag = JsonStr(ownJson, L"tag_name");
            }
        }
    }

    if (ParseCompatBlock(body, &c.min, &c.max)) {
        c.available = true;
        c.tag = tag;
        SaveCompatCache(spicetifyVersion, c);
    } else {
        GuardianLog(L"WARN", L"Could not parse the compatibility block from the release notes.");
    }
    return c;
}

static Verdict EvaluateCompat(const std::wstring& spotifyVersion,
                              const std::wstring& spicetifyVersion,
                              bool strict,
                              bool force) {
    Verdict v;
    v.compat = FetchCompat(spicetifyVersion, force);

    std::wstring latestNoV = v.compat.latestTag;
    if (!latestNoV.empty() && (latestNoV[0] == L'v' || latestNoV[0] == L'V')) {
        latestNoV.erase(0, 1);
    }
    v.upgradeFirst = !latestNoV.empty() && !spicetifyVersion.empty() &&
                     latestNoV != spicetifyVersion;

    if (!v.compat.available) {
        v.tier = Tier::Unknown;
        v.shouldProceed = true;
        v.message = L"Compatibility could not be verified; proceeding anyway.";
        return v;
    }

    Ver3 have = ParseVer3(spotifyVersion);
    Ver3 lo = ParseVer3(v.compat.min);
    Ver3 hi = ParseVer3(v.compat.max);
    if (!have.ok() || !lo.ok() || !hi.ok()) {
        v.message = L"Compatibility could not be verified; proceeding anyway.";
        return v;
    }

    wchar_t buf[768];

    if (CompareVer3(have, lo) < 0) {
        v.tier = Tier::TooOld;
        v.shouldProceed = false;
        swprintf_s(buf,
                   L"Spotify %s is older than the oldest build Spicetify %s supports (%s). "
                   L"Update Spotify, then repair.",
                   spotifyVersion.c_str(), spicetifyVersion.c_str(), v.compat.min.c_str());
        v.message = buf;
    } else if (CompareVer3(have, hi) > 0) {
        v.tier = Tier::Untested;
        if (strict) {
            v.shouldProceed = false;
            swprintf_s(buf,
                       L"Strict mode: holding off. Spotify %s is beyond Spicetify %s's tested "
                       L"range (%s -> %s). Wait for a new Spicetify release, downgrade Spotify, "
                       L"or turn strict mode off.",
                       spotifyVersion.c_str(), spicetifyVersion.c_str(), v.compat.min.c_str(),
                       v.compat.max.c_str());
        } else {
            swprintf_s(buf,
                       L"Spotify %s is newer than Spicetify %s's tested maximum (%s). This "
                       L"usually still works - the published range lags reality.",
                       spotifyVersion.c_str(), spicetifyVersion.c_str(), v.compat.max.c_str());
        }
        v.message = buf;
    } else {
        v.tier = Tier::Supported;
        swprintf_s(buf, L"Spotify %s is within Spicetify %s's tested range (%s -> %s).",
                   spotifyVersion.c_str(), spicetifyVersion.c_str(), v.compat.min.c_str(),
                   v.compat.max.c_str());
        v.message = buf;
    }

    return v;
}

// =====================================================================
// Pause
// =====================================================================

struct PauseInfo {
    bool paused = false;
    std::wstring reason;
};

static PauseInfo GetPauseInfo(const std::wstring& spotifyVersion) {
    EnterCriticalSection(&g_stateLock);
    long long until = g_state.pauseUntil;
    std::wstring atVersion = g_state.pauseAtVersion;
    LeaveCriticalSection(&g_stateLock);

    PauseInfo p;

    if (until == -1) {
        if (!atVersion.empty() && !spotifyVersion.empty() && atVersion != spotifyVersion) {
            p.reason = L"Spotify version changed; pause expired.";
            return p;
        }
        p.paused = true;
        p.reason = L"Paused until the next Spotify update.";
        return p;
    }

    long long now = UnixNow();
    if (until > now) {
        long long left = until - now;
        wchar_t buf[128];
        swprintf_s(buf, L"Paused for another %lldd %lldh %lldm.", left / 86400,
                   (left % 86400) / 3600, (left % 3600) / 60);
        p.paused = true;
        p.reason = buf;
    }
    return p;
}

static void SetPause(int days) {
    EnterCriticalSection(&g_stateLock);
    if (days == 0) {
        g_state.pauseUntil = 0;
        g_state.pauseAtVersion.clear();
    } else if (days < 0) {
        g_state.pauseUntil = -1;
        g_state.pauseAtVersion = FileVersionOf(FindSpotify().exe);
    } else {
        g_state.pauseUntil = UnixNow() + (long long)days * 86400;
        g_state.pauseAtVersion.clear();
    }
    LeaveCriticalSection(&g_stateLock);

    SaveState();

    if (days == 0) {
        GuardianLog(L"ACTION", L"Automatic repair resumed.");
    } else if (days < 0) {
        GuardianLog(L"ACTION", L"Paused until the next Spotify update.");
    } else {
        LogF(L"ACTION", L"Paused for %d day(s).", days);
    }
}

// =====================================================================
// The repair
// =====================================================================

enum class RepairAction { None, NoOp, Blocked, Repair };

struct RepairResult {
    RepairAction action = RepairAction::None;
    bool success = false;
    std::wstring reason;
};

// Order matters, and it is deliberate: every check runs BEFORE Spotify is
// touched, so a run that is going to be refused never interrupts playback.
static RepairResult DoRepair(bool forced) {
    RepairResult r;

    Status st = GetStatus();

    // --- 1. prerequisites -------------------------------------------------
    if (!st.spotifyInstalled) {
        r.reason = L"Spotify is not installed.";
        GuardianLog(L"ERROR", r.reason);
        return r;
    }
    if (!st.spicetifyInstalled) {
        r.reason = L"Spicetify is not installed.";
        GuardianLog(L"ERROR", r.reason);
        return r;
    }

    // --- 2. already healthy -----------------------------------------------
    // This gate is what stops the corrupt-your-backup footgun: running
    // `clear backup apply` over an install that is still patched would back up
    // the PATCHED files as if they were pristine.
    if (st.applied && st.backupMatches) {
        r.action = RepairAction::NoOp;
        r.success = true;
        r.reason = L"Spicetify is already applied to Spotify " + st.spotifyVersion +
                   L". Nothing to do.";
        GuardianLog(L"INFO", r.reason);
        return r;
    }

    bool needsRestoreFirst = st.applied && !st.backupMatches;

    // --- 3. concurrency ---------------------------------------------------
    if (IsProcessRunning(L"spicetify.exe")) {
        r.reason = L"A spicetify process is already running; not interfering.";
        GuardianLog(L"WARN", r.reason);
        return r;
    }

    // --- 4. pause ---------------------------------------------------------
    if (!forced) {
        PauseInfo p = GetPauseInfo(st.spotifyVersion);
        if (p.paused) {
            r.reason = p.reason;
            GuardianLog(L"INFO", L"Skipping repair. " + p.reason);
            return r;
        }
    }

    // --- 5. attempt budget ------------------------------------------------
    EnterCriticalSection(&g_stateLock);
    bool sameVersion = (g_state.lastSpotifyVersion == st.spotifyVersion);
    int attempts = g_state.attempts;
    bool strict = g_state.strictMode;
    LeaveCriticalSection(&g_stateLock);

    if (!forced && sameVersion && attempts >= kMaxAttemptsPerVersion) {
        wchar_t buf[512];
        swprintf_s(buf,
                   L"Already failed %d times on Spotify %s. Not retrying automatically - "
                   L"use Repair now from the tray menu to override.",
                   attempts, st.spotifyVersion.c_str());
        r.reason = buf;
        GuardianLog(L"WARN", r.reason);
        return r;
    }

    // --- 6. compatibility -------------------------------------------------
    Verdict v = EvaluateCompat(st.spotifyVersion, st.spicetifyVersion, strict, false);
    EnterCriticalSection(&g_compatLock);
    g_lastVerdict = v;
    LeaveCriticalSection(&g_compatLock);

    GuardianLog(v.tier == Tier::Supported ? L"INFO" : L"WARN", v.message);

    if (!v.shouldProceed) {
        r.action = RepairAction::Blocked;
        r.reason = v.message;

        EnterCriticalSection(&g_stateLock);
        g_state.lastRunUtc = UtcNowIso();
        g_state.lastRunResult = L"Blocked";
        g_state.lastRunDetail = v.message;
        LeaveCriticalSection(&g_stateLock);
        SaveState();
        return r;
    }

    // --- 7. snapshot ------------------------------------------------------
    SnapshotConfig(L"repair");

    // --- 8. close Spotify (all checks are behind us) ----------------------
    bool weClosedSpotify = StopSpotifyGracefully();

    // --- 9. repair --------------------------------------------------------
    r.action = RepairAction::Repair;

    if (needsRestoreFirst) {
        GuardianLog(L"WARN",
                    L"Patched against a different Spotify build; restoring to stock first.");
        RunSpicetify(L"restore");
    }

    // `spicetify update` self-heals: when it finds itself already up to date it
    // runs clear -> backup -> apply, which is exactly the repair. Falling back
    // to that chain covers the offline case.
    RunResult run = RunSpicetify(L"update");
    if (!run.success) {
        GuardianLog(L"WARN", L"update failed; falling back to clear/backup/apply.");
        run = RunSpicetify(L"clear backup apply -n");
    }

    // --- 10. verify -------------------------------------------------------
    Sleep(500);
    Status after = GetStatus();

    if (after.applied && run.success) {
        r.success = true;
        r.reason = L"Spicetify re-applied to Spotify " + after.spotifyVersion + L".";
        GuardianLog(L"ACTION", r.reason);

        EnterCriticalSection(&g_stateLock);
        g_state.attempts = 0;
        g_state.lastRunResult = L"Success";
        LeaveCriticalSection(&g_stateLock);
    } else {
        wchar_t buf[256];
        swprintf_s(buf, L"Repair failed (exit %lu).", run.exitCode);
        r.reason = buf;
        GuardianLog(L"ERROR", r.reason);

        EnterCriticalSection(&g_stateLock);
        g_state.attempts++;
        g_state.lastRunResult = L"Failed";
        LeaveCriticalSection(&g_stateLock);

        // Leave the user with a Spotify that launches, even if un-themed.
        if (!after.applied) {
            GuardianLog(L"WARN", L"Restoring Spotify to stock so it still runs.");
            RunSpicetify(L"restore");
        }
        LogF(L"WARN",
             L"If this keeps happening, Spicetify may not support Spotify %s yet. "
             L"Downgrade guidance: %s",
             after.spotifyVersion.c_str(), kDowngradeGuide);
    }

    EnterCriticalSection(&g_stateLock);
    g_state.lastSpotifyVersion = after.spotifyVersion;
    g_state.lastRunUtc = UtcNowIso();
    g_state.lastRunDetail = r.reason + L" " + v.message;
    LeaveCriticalSection(&g_stateLock);
    SaveState();

    // --- 11. restart ------------------------------------------------------
    if (weClosedSpotify) {
        StartSpotify();
    }

    return r;
}

// =====================================================================
// Spotify update blocking
// =====================================================================

static bool RunIcacls(const std::wstring& args) {
    wchar_t sys[MAX_PATH];
    if (!GetSystemDirectoryW(sys, ARRAYSIZE(sys))) {
        return false;
    }
    std::wstring exe = std::wstring(sys) + L"\\icacls.exe";
    RunResult r = RunCapture(exe, args, 30000);
    if (!r.success) {
        LogF(L"ERROR", L"icacls failed (exit %lu): %s", r.exitCode, Trim(r.output).c_str());
    }
    return r.success;
}

static std::wstring CurrentUserForAcl() {
    wchar_t user[256] = {};
    DWORD n = ARRAYSIZE(user);
    if (!GetUserNameW(user, &n)) {
        return L"";
    }
    std::wstring domain = EnvPath(L"USERDOMAIN", nullptr);
    return domain.empty() ? std::wstring(user) : domain + L"\\" + user;
}

static void BlockSpotifyUpdates() {
    // Method 1: Spicetify's own supported mechanism.
    RunSpicetify(L"spotify-updates block");

    // Method 2: deny write on the Update folder. Community reports say the
    // patch alone does not hold on every build, and this one does. Read access
    // is deliberately left intact - denying it makes Spotify log errors on
    // every launch.
    SpotifyPaths sp = FindSpotify();
    std::wstring dir = sp.updateDir;
    if (dir.empty()) {
        return;
    }

    if (DirExists(dir)) {
        // Clear anything already staged, including a half-downloaded installer.
        std::wstring from = dir;
        from.push_back(L'\0');   // SHFileOperation needs a double-null list
        SHFILEOPSTRUCTW op{};
        op.wFunc = FO_DELETE;
        op.pFrom = from.c_str();
        op.fFlags = FOF_NO_UI | FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT;
        SHFileOperationW(&op);
    }
    SHCreateDirectoryExW(nullptr, dir.c_str(), nullptr);

    std::wstring me = CurrentUserForAcl();
    if (me.empty()) {
        GuardianLog(L"ERROR", L"Could not determine the current user for the ACL rule.");
        return;
    }

    if (RunIcacls(L"\"" + dir + L"\" /deny \"" + me + L":(OI)(CI)(W,D)\"")) {
        LogF(L"ACTION", L"Denied write/delete on %s for %s", dir.c_str(), me.c_str());
    }
}

static void UnblockSpotifyUpdates() {
    RunSpicetify(L"spotify-updates unblock");

    SpotifyPaths sp = FindSpotify();
    if (sp.updateDir.empty() || !DirExists(sp.updateDir)) {
        return;
    }

    std::wstring me = CurrentUserForAcl();
    if (me.empty()) {
        return;
    }

    // /remove:d strips deny entries only, leaving allow rules and inherited
    // permissions exactly as they were.
    if (RunIcacls(L"\"" + sp.updateDir + L"\" /remove:d \"" + me + L"\"")) {
        LogF(L"ACTION", L"Removed deny rules on %s", sp.updateDir.c_str());
    }
}

// Read the DACL directly rather than shelling out to icacls. This is called on
// every status refresh, and a process spawn per refresh would be absurd.
static bool AreUpdatesBlocked() {
    SpotifyPaths sp = FindSpotify();
    if (sp.updateDir.empty() || !DirExists(sp.updateDir)) {
        return false;
    }

    PACL dacl = nullptr;
    PSECURITY_DESCRIPTOR sd = nullptr;
    if (GetNamedSecurityInfoW(sp.updateDir.c_str(), SE_FILE_OBJECT, DACL_SECURITY_INFORMATION,
                              nullptr, nullptr, &dacl, nullptr, &sd) != ERROR_SUCCESS) {
        return false;
    }

    bool denied = false;
    if (dacl) {
        for (WORD i = 0; i < dacl->AceCount && !denied; i++) {
            LPVOID ace = nullptr;
            if (!GetAce(dacl, i, &ace)) {
                continue;
            }
            auto* header = (ACE_HEADER*)ace;
            // Any deny ACE carrying write or delete is the block, whoever it
            // names - the ACL method targets the current user, but a user may
            // have applied it against a group instead.
            if (header->AceType == ACCESS_DENIED_ACE_TYPE) {
                auto* denyAce = (ACCESS_DENIED_ACE*)ace;
                if (denyAce->Mask & (FILE_GENERIC_WRITE | DELETE | GENERIC_WRITE | GENERIC_ALL)) {
                    denied = true;
                }
            }
        }
    }

    if (sd) {
        LocalFree(sd);
    }
    return denied;
}

// =====================================================================
// Marketplace / install helpers
// =====================================================================

// Runs a PowerShell one-liner. Used only for the two official installers, which
// are published as .ps1 by the Spicetify project and have no CLI equivalent.
static RunResult RunPowerShell(const std::wstring& command) {
    std::wstring ps = EnvPath(L"SystemRoot", L"System32\\WindowsPowerShell\\v1.0\\powershell.exe");
    std::wstring args = L"-NoProfile -ExecutionPolicy Bypass -NonInteractive -Command \"" +
                        command + L"\"";
    return RunCapture(ps, args, 600000);
}

static void InstallSpicetify() {
    GuardianLog(L"ACTION", L"Installing Spicetify from the official installer.");
    RunResult r = RunPowerShell(
        L"$ErrorActionPreference='Stop'; "
        L"iwr -useb 'https://raw.githubusercontent.com/spicetify/cli/main/install.ps1' | iex");
    if (!r.success) {
        LogF(L"ERROR", L"Spicetify install failed (exit %lu).", r.exitCode);
        return;
    }
    if (FindSpicetifyExe().empty()) {
        GuardianLog(L"ERROR", L"Installer finished but spicetify.exe was not found.");
        return;
    }
    bool wasRunning = StopSpotifyGracefully();
    RunSpicetify(L"backup apply -n");
    if (wasRunning) {
        StartSpotify();
    }
    GuardianLog(L"ACTION", L"Spicetify installed and applied.");
}

static void UninstallSpicetify() {
    // Restore FIRST, while the tool that can un-patch Spotify still exists.
    SnapshotConfig(L"uninstall");
    bool wasRunning = StopSpotifyGracefully();

    RunResult r = RunSpicetify(L"restore");
    if (!r.success) {
        GuardianLog(L"ERROR",
                    L"`spicetify restore` failed. Stopping rather than deleting the only tool "
                    L"that can un-patch Spotify.");
        if (wasRunning) {
            StartSpotify();
        }
        return;
    }

    std::wstring exe = FindSpicetifyExe();
    if (!exe.empty()) {
        std::wstring dir = exe.substr(0, exe.find_last_of(L'\\'));
        // Guard the path: never recursively delete something that is not
        // recognisably a spicetify install directory.
        size_t leaf = dir.find_last_of(L'\\');
        std::wstring leafName = (leaf == std::wstring::npos) ? dir : dir.substr(leaf + 1);
        if (_wcsicmp(leafName.c_str(), L"spicetify") == 0) {
            std::wstring from = dir;
            from.push_back(L'\0');
            SHFILEOPSTRUCTW op{};
            op.wFunc = FO_DELETE;
            op.pFrom = from.c_str();
            op.fFlags = FOF_NO_UI | FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT;
            if (SHFileOperationW(&op) == 0) {
                LogF(L"ACTION", L"Removed %s", dir.c_str());
            }
        } else {
            LogF(L"WARN", L"Skipping deletion of unexpected install dir: %s", dir.c_str());
        }
    }

    GuardianLog(L"ACTION", L"Spicetify uninstalled. Your config and themes were kept.");
    if (wasRunning) {
        StartSpotify();
    }
}

static void InstallMarketplace() {
    SnapshotConfig(L"marketplace-install");
    GuardianLog(L"ACTION", L"Installing Spicetify Marketplace.");

    RunResult r = RunPowerShell(
        L"$ErrorActionPreference='Stop'; "
        L"iwr -useb 'https://raw.githubusercontent.com/spicetify/marketplace/main/resources/"
        L"install.ps1' | iex");
    if (!r.success) {
        LogF(L"ERROR", L"Marketplace install failed (exit %lu).", r.exitCode);
        return;
    }

    // Marketplace needs a placeholder theme to exist and be active, or themes
    // installed through it silently never apply. The official installer sets
    // this up, but a later `config current_theme something-else` quietly breaks
    // it again and nothing tells you.
    std::wstring themeDir = EnvPath(L"APPDATA", L"spicetify\\Themes\\marketplace");
    if (!themeDir.empty() && !DirExists(themeDir)) {
        SHCreateDirectoryExW(nullptr, themeDir.c_str(), nullptr);
        WriteTextFile(themeDir + L"\\color.ini", L"[Base]\r\n");
        GuardianLog(L"ACTION", L"Created the missing Marketplace placeholder theme.");
    }

    Status st = GetStatus(false);
    if (!st.marketplaceInstalled) {
        RunSpicetify(L"config custom_apps marketplace");
    }
    RunSpicetify(L"config inject_css 1 replace_colors 1");
    RunSpicetify(L"config current_theme marketplace");

    bool wasRunning = StopSpotifyGracefully();
    RunSpicetify(L"apply -n");
    if (wasRunning) {
        StartSpotify();
    }
    GuardianLog(L"ACTION", L"Marketplace installed.");
}

static void RemoveMarketplace() {
    SnapshotConfig(L"marketplace-remove");

    Status st = GetStatus(false);
    // Clear the theme BEFORE deleting anything, so apply never references a
    // folder that is on its way out.
    if (_wcsicmp(st.theme.c_str(), L"marketplace") == 0) {
        RunSpicetify(L"config current_theme \" \"");
    }
    // The trailing '-' is Spicetify's syntax for removing an array entry.
    RunSpicetify(L"config custom_apps marketplace-");

    std::wstring appDir = EnvPath(L"APPDATA", L"spicetify\\CustomApps\\marketplace");
    if (!appDir.empty() && DirExists(appDir)) {
        std::wstring from = appDir;
        from.push_back(L'\0');
        SHFILEOPSTRUCTW op{};
        op.wFunc = FO_DELETE;
        op.pFrom = from.c_str();
        op.fFlags = FOF_NO_UI | FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT;
        SHFileOperationW(&op);
    }

    bool wasRunning = StopSpotifyGracefully();
    RunSpicetify(L"apply -n");
    if (wasRunning) {
        StartSpotify();
    }
    GuardianLog(L"ACTION",
                L"Marketplace removed. Themes and extensions installed through it were kept.");
}

// =====================================================================
// Health checks
// =====================================================================

struct HealthIssue {
    int severity;   // 0 info, 1 warning, 2 error
    std::wstring title;
    std::wstring detail;
    std::wstring fix;
};

static std::vector<std::wstring> ListFiles(const std::wstring& dir) {
    std::vector<std::wstring> names;
    if (dir.empty() || !DirExists(dir)) {
        return names;
    }
    WIN32_FIND_DATAW fd{};
    HANDLE h = FindFirstFileW((dir + L"\\*").c_str(), &fd);
    if (h == INVALID_HANDLE_VALUE) {
        return names;
    }
    do {
        if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            names.push_back(fd.cFileName);
        }
    } while (FindNextFileW(h, &fd));
    FindClose(h);
    return names;
}

static std::vector<HealthIssue> GetHealthIssues(const Status& st) {
    std::vector<HealthIssue> issues;

    // Extensions resolve from two folders and BOTH are legitimate: the one next
    // to spicetify.exe holds the CLI's bundled extensions (shuffle+, trashbin,
    // keyboardShortcut...), the one in the config dir holds yours. Their mere
    // coexistence is normal and must not be reported.
    //
    // What IS worth flagging is the same filename in both, where which copy
    // wins is not obvious and editing the wrong one looks like "my change did
    // nothing".
    std::wstring cfgExt = EnvPath(L"APPDATA", L"spicetify\\Extensions");
    std::wstring localExt = EnvPath(L"LOCALAPPDATA", L"spicetify\\Extensions");
    std::vector<std::wstring> cfgNames = ListFiles(cfgExt);
    std::vector<std::wstring> localNames = ListFiles(localExt);

    std::wstring clashes;
    for (const auto& n : cfgNames) {
        for (const auto& m : localNames) {
            if (_wcsicmp(n.c_str(), m.c_str()) == 0) {
                if (!clashes.empty()) {
                    clashes += L", ";
                }
                clashes += n;
            }
        }
    }
    if (!clashes.empty()) {
        issues.push_back({1, L"Same extension filename in both extension folders",
                          clashes + L" exists in both extension folders. Only one copy is "
                                    L"loaded, so edits to the other silently do nothing.",
                          L"Delete or rename whichever copy you are not maintaining."});
    }

    // Config entries pointing at files that no longer exist.
    std::wstring missingExt;
    for (const auto& e : st.extensions) {
        if (!FileExists(cfgExt + L"\\" + e) && !FileExists(localExt + L"\\" + e)) {
            if (!missingExt.empty()) {
                missingExt += L", ";
            }
            missingExt += e;
        }
    }
    if (!missingExt.empty()) {
        issues.push_back({1, L"Config lists extensions that are not installed",
                          L"Missing: " + missingExt,
                          L"Untick them on the Config tab, or reinstall them."});
    }

    std::wstring missingApps;
    std::wstring cfgApps = EnvPath(L"APPDATA", L"spicetify\\CustomApps");
    std::wstring localApps = EnvPath(L"LOCALAPPDATA", L"spicetify\\CustomApps");
    for (const auto& a : st.customApps) {
        if (!DirExists(cfgApps + L"\\" + a) && !DirExists(localApps + L"\\" + a)) {
            if (!missingApps.empty()) {
                missingApps += L", ";
            }
            missingApps += a;
        }
    }
    if (!missingApps.empty()) {
        issues.push_back({1, L"Config lists custom apps that are not installed",
                          L"Missing: " + missingApps,
                          L"Untick them on the Config tab, or reinstall them."});
    }

    // Marketplace without its placeholder theme - themes silently stop applying.
    if (st.marketplaceInstalled) {
        std::wstring mpTheme = EnvPath(L"APPDATA", L"spicetify\\Themes\\marketplace");
        if (!DirExists(mpTheme)) {
            issues.push_back({2, L"Marketplace installed without its placeholder theme",
                              L"Marketplace is enabled but its theme folder does not exist. "
                              L"Themes installed from Marketplace will not apply.",
                              L"Actions tab -> Install / repair Marketplace."});
        } else if (_wcsicmp(st.theme.c_str(), L"marketplace") != 0) {
            issues.push_back({0, L"Marketplace installed but not the active theme",
                              L"current_theme is '" + st.theme +
                                  L"'. Marketplace theme installs need current_theme = "
                                  L"marketplace.",
                              L"Config tab -> set the theme to marketplace."});
        }
    }

    if (st.health == Health::StaleBackup) {
        issues.push_back({2, L"Backup does not match the installed Spotify",
                          L"Backup was taken from " + st.backupVersion + L" but Spotify is " +
                              st.spotifyVersion + L".",
                          L"Run a repair; it restores to stock and rebuilds the backup."});
    }

    std::wstring exe = FindSpicetifyExe();
    if (!exe.empty() && FileExists(exe + L".old")) {
        issues.push_back({0, L"Leftover spicetify.exe.old",
                          exe + L".old is left over from a previous upgrade.",
                          L"Safe to delete."});
    }

    // Blocking updates and repairing after them solve the same problem from
    // opposite ends. Running both is harmless but pointless, and it is easy to
    // forget blocking is on and then wonder why Spotify never updates.
    if (st.updatesBlocked && Settings().autoRepair) {
        issues.push_back({0, L"Spotify updates blocked and automatic repair both on",
                          L"Spotify cannot update, so there is nothing left for automatic "
                          L"repair to fix. You are also not receiving Spotify security fixes.",
                          L"Pick one: unblock updates and let repair handle them, or leave "
                          L"blocking on and turn automatic repair off in the mod settings."});
    }

    return issues;
}

// =====================================================================
// Worker thread
// =====================================================================

static HANDLE g_workerThread = nullptr;
static HANDLE g_stopEvent = nullptr;
static HANDLE g_workEvent = nullptr;
static HWND g_hTrayWnd = nullptr;
static HWND g_hDashWnd = nullptr;

static CRITICAL_SECTION g_queueLock;
static std::vector<Task> g_taskQueue;

static CRITICAL_SECTION g_statusLock;
static Status g_status;
static std::vector<HealthIssue> g_issues;
static bool g_busy = false;
static std::wstring g_busyLabel;

// Config-tab edits are queued as spicetify argument strings and applied
// together, so ticking three extensions is one `apply` rather than three.
static std::vector<std::wstring> g_pendingConfigArgs;

static void QueueTask(Task t) {
    EnterCriticalSection(&g_queueLock);
    g_taskQueue.push_back(t);
    LeaveCriticalSection(&g_queueLock);
    SetEvent(g_workEvent);
}

static void SetBusy(bool busy, const wchar_t* label) {
    EnterCriticalSection(&g_statusLock);
    g_busy = busy;
    g_busyLabel = label ? label : L"";
    LeaveCriticalSection(&g_statusLock);

    if (g_hTrayWnd) {
        PostMessageW(g_hTrayWnd, WM_APP_STATE_DIRTY, 0, 0);
    }
}

static void RefreshStatus() {
    Status s = GetStatus();
    std::vector<HealthIssue> issues = GetHealthIssues(s);

    EnterCriticalSection(&g_statusLock);
    g_status = s;
    g_issues = issues;
    LeaveCriticalSection(&g_statusLock);

    if (g_hTrayWnd) {
        PostMessageW(g_hTrayWnd, WM_APP_STATE_DIRTY, 0, 0);
    }
}

static Status SnapshotStatus() {
    EnterCriticalSection(&g_statusLock);
    Status s = g_status;
    LeaveCriticalSection(&g_statusLock);
    return s;
}

// Notification text is handed to the UI thread as a heap string; the tray
// window owns it from that point.
static void Notify(const wchar_t* title, const std::wstring& text, DWORD icon) {
    if (!Settings().notifications || !g_hTrayWnd) {
        return;
    }
    auto* payload = new std::pair<std::wstring, std::wstring>(title, text);
    PostMessageW(g_hTrayWnd, WM_APP_NOTIFY, (WPARAM)icon, (LPARAM)payload);
}

static void RunRepairTask(bool forced, bool automatic) {
    SetBusy(true, L"Repairing");
    RepairResult r = DoRepair(forced);
    SetBusy(false, nullptr);
    RefreshStatus();

    switch (r.action) {
        case RepairAction::Repair:
            if (r.success) {
                Notify(kAppName, r.reason, NIIF_INFO);
            } else {
                Notify(kAppName,
                       r.reason + L" Spotify still works, but is un-themed. See the log.",
                       NIIF_ERROR);
            }
            break;
        case RepairAction::Blocked:
            Notify(kAppName, r.reason, NIIF_WARNING);
            break;
        case RepairAction::NoOp:
            // An automatic run that finds nothing wrong is silent by design.
            if (!automatic) {
                Notify(kAppName, r.reason, NIIF_INFO);
            }
            break;
        default:
            if (!automatic && !r.reason.empty()) {
                Notify(kAppName, r.reason, NIIF_WARNING);
            }
            break;
    }
}

static void HandleTask(Task t) {
    switch (t) {
        case Task::Repair:
            RunRepairTask(false, false);
            break;

        case Task::RepairForced:
            RunRepairTask(true, false);
            break;

        case Task::UpdateCli: {
            SetBusy(true, L"Updating Spicetify");
            SnapshotConfig(L"update");
            bool wasRunning = StopSpotifyGracefully();
            RunResult r = RunSpicetify(L"update");
            if (wasRunning) {
                StartSpotify();
            }
            SetBusy(false, nullptr);
            RefreshStatus();
            Notify(kAppName,
                   r.success ? L"Spicetify updated." : L"Spicetify update failed. See the log.",
                   r.success ? NIIF_INFO : NIIF_ERROR);
            break;
        }

        case Task::RestoreStock: {
            SetBusy(true, L"Restoring Spotify");
            SnapshotConfig(L"manual-restore");
            bool wasRunning = StopSpotifyGracefully();
            RunResult r = RunSpicetify(L"restore");
            if (wasRunning) {
                StartSpotify();
            }
            SetBusy(false, nullptr);
            RefreshStatus();
            Notify(kAppName,
                   r.success ? L"Spotify restored to stock." : L"Restore failed. See the log.",
                   r.success ? NIIF_INFO : NIIF_ERROR);
            break;
        }

        case Task::BackupApply: {
            // Guarded for the same reason DoRepair is: backing up a patched
            // install writes a corrupt backup.
            Status st = GetStatus();
            if (st.applied) {
                Notify(kAppName,
                       L"Spicetify is already applied. Use Restore to stock first if you want "
                       L"to rebuild the backup.",
                       NIIF_WARNING);
                break;
            }
            SetBusy(true, L"Applying");
            SnapshotConfig(L"backup-apply");
            bool wasRunning = StopSpotifyGracefully();
            RunResult r = RunSpicetify(L"backup apply -n");
            if (wasRunning) {
                StartSpotify();
            }
            SetBusy(false, nullptr);
            RefreshStatus();
            Notify(kAppName, r.success ? L"Spicetify applied." : L"Apply failed. See the log.",
                   r.success ? NIIF_INFO : NIIF_ERROR);
            break;
        }

        case Task::RestartSpotify: {
            SetBusy(true, L"Restarting Spotify");
            bool was = StopSpotifyGracefully();
            if (was) {
                StartSpotify();
            } else {
                StartSpotify();
            }
            SetBusy(false, nullptr);
            RefreshStatus();
            break;
        }

        case Task::BlockUpdates:
            SetBusy(true, L"Blocking Spotify updates");
            BlockSpotifyUpdates();
            SetBusy(false, nullptr);
            RefreshStatus();
            Notify(kAppName,
                   L"Spotify updates blocked. You will not receive Spotify updates - including "
                   L"security fixes - until you unblock.",
                   NIIF_INFO);
            break;

        case Task::UnblockUpdates:
            SetBusy(true, L"Unblocking Spotify updates");
            UnblockSpotifyUpdates();
            SetBusy(false, nullptr);
            RefreshStatus();
            Notify(kAppName, L"Spotify updates enabled again.", NIIF_INFO);
            break;

        case Task::InstallMarketplace:
            SetBusy(true, L"Installing Marketplace");
            InstallMarketplace();
            SetBusy(false, nullptr);
            RefreshStatus();
            Notify(kAppName, L"Marketplace installed.", NIIF_INFO);
            break;

        case Task::RemoveMarketplace:
            SetBusy(true, L"Removing Marketplace");
            RemoveMarketplace();
            SetBusy(false, nullptr);
            RefreshStatus();
            Notify(kAppName, L"Marketplace removed.", NIIF_INFO);
            break;

        case Task::InstallSpicetify:
            SetBusy(true, L"Installing Spicetify");
            InstallSpicetify();
            SetBusy(false, nullptr);
            RefreshStatus();
            break;

        case Task::UninstallSpicetify:
            SetBusy(true, L"Uninstalling Spicetify");
            UninstallSpicetify();
            SetBusy(false, nullptr);
            RefreshStatus();
            break;

        case Task::RefreshCompat: {
            SetBusy(true, L"Checking compatibility");
            Status st = GetStatus();
            EnterCriticalSection(&g_stateLock);
            bool strict = g_state.strictMode;
            LeaveCriticalSection(&g_stateLock);

            Verdict v = EvaluateCompat(st.spotifyVersion, st.spicetifyVersion, strict, true);
            EnterCriticalSection(&g_compatLock);
            g_lastVerdict = v;
            LeaveCriticalSection(&g_compatLock);

            SetBusy(false, nullptr);
            RefreshStatus();
            break;
        }

        case Task::ApplyConfig: {
            std::vector<std::wstring> args;
            EnterCriticalSection(&g_queueLock);
            args.swap(g_pendingConfigArgs);
            LeaveCriticalSection(&g_queueLock);

            if (args.empty()) {
                break;
            }
            SetBusy(true, L"Applying config");
            SnapshotConfig(L"config-edit");
            for (const auto& a : args) {
                RunSpicetify(a);
            }
            bool wasRunning = StopSpotifyGracefully();
            RunSpicetify(L"apply -n");
            if (wasRunning) {
                StartSpotify();
            }
            SetBusy(false, nullptr);
            RefreshStatus();
            break;
        }
    }
}

// Watches Spotify's Apps folder. Spotify writes xpui.spa back as a file when it
// updates, which is precisely the moment Spicetify stopped being applied - so a
// change notification there is the trigger. Debounced, because Spotify writes
// in bursts and repairing mid-update would fight it.
static DWORD WINAPI WorkerThreadProc(LPVOID) {
    RefreshStatus();

    HANDLE hDir = INVALID_HANDLE_VALUE;
    OVERLAPPED ov{};
    HANDLE hDirEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    std::vector<BYTE> buf(8192);
    bool watching = false;

    ULONGLONG pendingSince = 0;
    ULONGLONG lastPoll = GetTickCount64();

    auto startWatch = [&]() {
        Status st = SnapshotStatus();
        if (!st.paths.valid || !DirExists(st.paths.appsDir)) {
            return;
        }
        hDir = CreateFileW(st.paths.appsDir.c_str(), FILE_LIST_DIRECTORY,
                           FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr,
                           OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
                           nullptr);
        if (hDir == INVALID_HANDLE_VALUE) {
            LogF(L"WARN", L"Could not watch %s (error %lu); falling back to polling.",
                 st.paths.appsDir.c_str(), GetLastError());
            return;
        }
        ZeroMemory(&ov, sizeof(ov));
        ov.hEvent = hDirEvent;
        if (ReadDirectoryChangesW(hDir, buf.data(), (DWORD)buf.size(), FALSE,
                                  FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME |
                                      FILE_NOTIFY_CHANGE_LAST_WRITE,
                                  nullptr, &ov, nullptr)) {
            watching = true;
            LogF(L"INFO", L"Watching %s", st.paths.appsDir.c_str());
        } else {
            CloseHandle(hDir);
            hDir = INVALID_HANDLE_VALUE;
        }
    };

    startWatch();

    HANDLE waits[3] = {g_stopEvent, g_workEvent, hDirEvent};

    while (true) {
        DWORD n = watching ? 3 : 2;
        DWORD w = WaitForMultipleObjects(n, waits, FALSE, 5000);

        if (w == WAIT_OBJECT_0) {
            break;   // stop
        }

        if (w == WAIT_OBJECT_0 + 1) {
            std::vector<Task> tasks;
            EnterCriticalSection(&g_queueLock);
            tasks.swap(g_taskQueue);
            LeaveCriticalSection(&g_queueLock);
            for (Task t : tasks) {
                HandleTask(t);
            }
            continue;
        }

        if (watching && w == WAIT_OBJECT_0 + 2) {
            ResetEvent(hDirEvent);
            DWORD bytes = 0;
            GetOverlappedResult(hDir, &ov, &bytes, FALSE);

            pendingSince = GetTickCount64();

            ZeroMemory(&ov, sizeof(ov));
            ov.hEvent = hDirEvent;
            if (!ReadDirectoryChangesW(hDir, buf.data(), (DWORD)buf.size(), FALSE,
                                       FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME |
                                           FILE_NOTIFY_CHANGE_LAST_WRITE,
                                       nullptr, &ov, nullptr)) {
                CloseHandle(hDir);
                hDir = INVALID_HANDLE_VALUE;
                watching = false;
            }
            continue;
        }

        // Timeout tick: handle the debounce and the backstop poll.
        ULONGLONG now = GetTickCount64();

        if (!watching && now - lastPoll > 60000) {
            startWatch();
            waits[2] = hDirEvent;
        }

        bool due = false;
        if (pendingSince && now - pendingSince >= kDebounceMs) {
            pendingSince = 0;
            due = true;
            GuardianLog(L"INFO", L"Apps folder changed - checking.");
        } else if (now - lastPoll >= kPollMs) {
            lastPoll = now;
            due = true;
        }

        if (!due) {
            continue;
        }

        RefreshStatus();
        Status st = SnapshotStatus();
        lastPoll = now;

        if (st.health == Health::Wiped || st.health == Health::StaleBackup) {
            if (Settings().autoRepair) {
                RunRepairTask(false, true);
            } else {
                Notify(kAppName,
                       L"Spotify updated and removed Spicetify. Automatic repair is off - "
                       L"right-click the tray icon to repair.",
                       NIIF_WARNING);
            }
        }
    }

    if (hDir != INVALID_HANDLE_VALUE) {
        CancelIo(hDir);
        CloseHandle(hDir);
    }
    if (hDirEvent) {
        CloseHandle(hDirEvent);
    }
    return 0;
}

// =====================================================================
// Tray icon
// =====================================================================

// Drawn rather than shipped as a resource: a Windhawk mod is a single .cpp with
// nowhere to put an .ico. A filled circle with a slice out of it reads as a
// record at 16px, and the colour carries the state.
static HICON MakeTrayIcon(int state) {   // 0 ok, 1 paused, 2 broken
    int sz = GetSystemMetrics(SM_CXSMICON);
    if (sz <= 0) {
        sz = 16;
    }

    HDC screen = GetDC(nullptr);
    HDC dc = CreateCompatibleDC(screen);

    BITMAPINFO bi{};
    bi.bmiHeader.biSize = sizeof(bi.bmiHeader);
    bi.bmiHeader.biWidth = sz;
    bi.bmiHeader.biHeight = -sz;   // top-down
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 32;
    bi.bmiHeader.biCompression = BI_RGB;

    void* bits = nullptr;
    HBITMAP colour = CreateDIBSection(dc, &bi, DIB_RGB_COLORS, &bits, nullptr, 0);
    HGDIOBJ oldBmp = SelectObject(dc, colour);

    if (bits) {
        ZeroMemory(bits, (size_t)sz * sz * 4);
    }

    COLORREF fill = (state == 0)   ? RGB(30, 215, 96)     // Spotify green
                    : (state == 1) ? RGB(130, 130, 130)   // paused: grey
                                   : RGB(240, 170, 40);   // broken: amber

    HBRUSH brush = CreateSolidBrush(fill);
    HGDIOBJ oldBrush = SelectObject(dc, brush);
    HGDIOBJ oldPen = SelectObject(dc, GetStockObject(NULL_PEN));
    Ellipse(dc, 0, 0, sz + 1, sz + 1);

    // Punch a hole so it reads as a disc rather than a dot.
    HBRUSH hole = CreateSolidBrush(RGB(0, 0, 0));
    SelectObject(dc, hole);
    int inset = (sz * 3) / 8;
    Ellipse(dc, inset, inset, sz - inset + 1, sz - inset + 1);

    SelectObject(dc, oldPen);
    SelectObject(dc, oldBrush);
    DeleteObject(brush);
    DeleteObject(hole);

    // CreateDIBSection gives zeroed (fully transparent) alpha; set alpha on the
    // pixels we actually painted, and clear it inside the hole.
    if (bits) {
        auto* px = (DWORD*)bits;
        double c = (sz - 1) / 2.0;
        double rOuter = sz / 2.0;
        double rInner = (sz / 2.0) - inset;
        for (int y = 0; y < sz; y++) {
            for (int x = 0; x < sz; x++) {
                double dx = x - c;
                double dy = y - c;
                double d = dx * dx + dy * dy;
                bool inDisc = d <= rOuter * rOuter;
                bool inHole = d <= rInner * rInner;
                DWORD& p = px[y * sz + x];
                if (inDisc && !inHole) {
                    p = (0xFFu << 24) | (GetRValue(fill) << 16) | (GetGValue(fill) << 8) |
                        GetBValue(fill);
                } else {
                    p = 0;
                }
            }
        }
    }

    SelectObject(dc, oldBmp);

    std::vector<BYTE> maskBits(((size_t)((sz + 15) / 16) * 2) * sz, 0);
    HBITMAP mask = CreateBitmap(sz, sz, 1, 1, maskBits.data());

    ICONINFO ii{};
    ii.fIcon = TRUE;
    ii.hbmColor = colour;
    ii.hbmMask = mask;
    HICON icon = CreateIconIndirect(&ii);

    DeleteObject(colour);
    DeleteObject(mask);
    DeleteDC(dc);
    ReleaseDC(nullptr, screen);
    return icon;
}

static UINT g_taskbarCreatedMsg = 0;

static int TrayIconState() {
    Status st = SnapshotStatus();
    PauseInfo p = GetPauseInfo(st.spotifyVersion);
    if (st.health == Health::Wiped || st.health == Health::StaleBackup) {
        return 2;
    }
    if (p.paused || !Settings().autoRepair) {
        return 1;
    }
    return 0;
}

static void UpdateTrayIcon(bool add) {
    if (!g_hTrayWnd) {
        return;
    }

    Status st = SnapshotStatus();
    PauseInfo p = GetPauseInfo(st.spotifyVersion);

    std::wstring tip = kAppName;
    switch (st.health) {
        case Health::Applied:      tip += L" - Spicetify applied"; break;
        case Health::Wiped:        tip += L" - Spicetify REMOVED"; break;
        case Health::StaleBackup:  tip += L" - backup out of date"; break;
        case Health::NotInstalled: tip += L" - Spicetify not installed"; break;
        case Health::NoSpotify:    tip += L" - Spotify not found"; break;
    }
    if (p.paused) {
        tip += L" (paused)";
    }

    EnterCriticalSection(&g_statusLock);
    bool busy = g_busy;
    std::wstring busyLabel = g_busyLabel;
    LeaveCriticalSection(&g_statusLock);
    if (busy) {
        tip = std::wstring(kAppName) + L" - " + busyLabel + L"...";
    }

    NOTIFYICONDATAW nid{};
    nid.cbSize = sizeof(nid);
    nid.hWnd = g_hTrayWnd;
    nid.uID = kTrayIconId;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP | NIF_SHOWTIP;
    nid.uCallbackMessage = WM_APP_TRAY;
    nid.hIcon = MakeTrayIcon(TrayIconState());
    wcsncpy_s(nid.szTip, tip.c_str(), _TRUNCATE);

    BOOL ok = Shell_NotifyIconW(add ? NIM_ADD : NIM_MODIFY, &nid);
    if (ok && add) {
        NOTIFYICONDATAW ver{};
        ver.cbSize = sizeof(ver);
        ver.hWnd = g_hTrayWnd;
        ver.uID = kTrayIconId;
        ver.uVersion = NOTIFYICON_VERSION_4;
        Shell_NotifyIconW(NIM_SETVERSION, &ver);
    }

    if (nid.hIcon) {
        DestroyIcon(nid.hIcon);
    }
}

static void ShowBalloon(const std::wstring& title, const std::wstring& text, DWORD icon) {
    if (!g_hTrayWnd) {
        return;
    }
    NOTIFYICONDATAW nid{};
    nid.cbSize = sizeof(nid);
    nid.hWnd = g_hTrayWnd;
    nid.uID = kTrayIconId;
    nid.uFlags = NIF_INFO;
    nid.dwInfoFlags = icon;
    wcsncpy_s(nid.szInfoTitle, title.c_str(), _TRUNCATE);
    wcsncpy_s(nid.szInfo, text.c_str(), _TRUNCATE);
    nid.uTimeout = 10000;
    Shell_NotifyIconW(NIM_MODIFY, &nid);
}

// Menu command ids.
enum {
    IDM_DASHBOARD = 100,
    IDM_REPAIR,
    IDM_REPAIR_FORCED,
    IDM_RESTART_SPOTIFY,
    IDM_PAUSE_1,
    IDM_PAUSE_3,
    IDM_PAUSE_7,
    IDM_PAUSE_NEXT,
    IDM_PAUSE_OFF,
    IDM_BLOCK,
    IDM_UNBLOCK,
    IDM_OPEN_LOG,
    IDM_OPEN_CONFIG,
    IDM_STRICT_TOGGLE,
};

static void ShowDashboard();

static void ShowTrayMenu() {
    Status st = SnapshotStatus();
    PauseInfo p = GetPauseInfo(st.spotifyVersion);

    EnterCriticalSection(&g_statusLock);
    bool busy = g_busy;
    LeaveCriticalSection(&g_statusLock);

    HMENU menu = CreatePopupMenu();

    std::wstring header;
    switch (st.health) {
        case Health::Applied:      header = L"Spicetify applied and healthy"; break;
        case Health::Wiped:        header = L"Spicetify REMOVED - repair needed"; break;
        case Health::StaleBackup:  header = L"Backup does not match Spotify"; break;
        case Health::NotInstalled: header = L"Spicetify not installed"; break;
        case Health::NoSpotify:    header = L"Spotify not found"; break;
    }
    AppendMenuW(menu, MF_STRING | MF_DISABLED, 0, header.c_str());
    if (p.paused) {
        AppendMenuW(menu, MF_STRING | MF_DISABLED, 0, p.reason.c_str());
    }
    AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);

    UINT busyFlag = busy ? MF_GRAYED : MF_ENABLED;
    AppendMenuW(menu, MF_STRING | busyFlag, IDM_REPAIR, L"&Repair now");
    AppendMenuW(menu, MF_STRING | busyFlag, IDM_RESTART_SPOTIFY, L"Re&start Spotify");
    AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);

    HMENU pause = CreatePopupMenu();
    AppendMenuW(pause, MF_STRING, IDM_PAUSE_1, L"For &1 day");
    AppendMenuW(pause, MF_STRING, IDM_PAUSE_3, L"For &3 days");
    AppendMenuW(pause, MF_STRING, IDM_PAUSE_7, L"For &7 days");
    AppendMenuW(pause, MF_STRING, IDM_PAUSE_NEXT, L"Until the &next Spotify update");
    AppendMenuW(pause, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(pause, MF_STRING | (p.paused ? MF_ENABLED : MF_GRAYED), IDM_PAUSE_OFF,
                L"&Resume now");
    AppendMenuW(menu, MF_POPUP, (UINT_PTR)pause, L"&Pause automatic repair");

    HMENU updates = CreatePopupMenu();
    AppendMenuW(updates, MF_STRING | busyFlag, IDM_BLOCK, L"&Block Spotify updates");
    AppendMenuW(updates, MF_STRING | busyFlag, IDM_UNBLOCK, L"&Unblock Spotify updates");
    AppendMenuW(menu, MF_POPUP, (UINT_PTR)updates, L"Spotify &updates");

    EnterCriticalSection(&g_stateLock);
    bool strict = g_state.strictMode;
    LeaveCriticalSection(&g_stateLock);
    AppendMenuW(menu, MF_STRING | (strict ? MF_CHECKED : MF_UNCHECKED), IDM_STRICT_TOGGLE,
                L"S&trict compatibility mode");

    AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(menu, MF_STRING, IDM_DASHBOARD, L"&Dashboard...");
    AppendMenuW(menu, MF_STRING, IDM_OPEN_LOG, L"Open &log folder");
    AppendMenuW(menu, MF_STRING, IDM_OPEN_CONFIG, L"Open Spicetify &config folder");

    POINT pt;
    GetCursorPos(&pt);
    // Required so the menu dismisses when the user clicks elsewhere.
    SetForegroundWindow(g_hTrayWnd);
    TrackPopupMenu(menu, TPM_RIGHTBUTTON | TPM_BOTTOMALIGN, pt.x, pt.y, 0, g_hTrayWnd, nullptr);
    PostMessageW(g_hTrayWnd, WM_NULL, 0, 0);
    DestroyMenu(menu);
}

static void OpenFolder(const std::wstring& path) {
    if (!path.empty()) {
        ShellExecuteW(nullptr, L"open", path.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
    }
}

static LRESULT CALLBACK TrayWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == g_taskbarCreatedMsg && g_taskbarCreatedMsg) {
        UpdateTrayIcon(true);
        return 0;
    }

    switch (msg) {
        case WM_APP_TRAY: {
            UINT event = LOWORD(lParam);
            if (event == WM_LBUTTONUP) {
                ShowDashboard();
            } else if (event == WM_RBUTTONUP || event == WM_CONTEXTMENU) {
                ShowTrayMenu();
            }
            return 0;
        }

        case WM_APP_STATE_DIRTY:
            UpdateTrayIcon(false);
            if (g_hDashWnd) {
                PostMessageW(g_hDashWnd, WM_APP_STATE_DIRTY, 0, 0);
            }
            return 0;

        case WM_APP_NOTIFY: {
            auto* payload = (std::pair<std::wstring, std::wstring>*)lParam;
            if (payload) {
                ShowBalloon(payload->first, payload->second, (DWORD)wParam);
                delete payload;
            }
            return 0;
        }

        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDM_DASHBOARD:       ShowDashboard(); break;
                case IDM_REPAIR:          QueueTask(Task::RepairForced); break;
                case IDM_RESTART_SPOTIFY: QueueTask(Task::RestartSpotify); break;
                case IDM_PAUSE_1:         SetPause(1); UpdateTrayIcon(false); break;
                case IDM_PAUSE_3:         SetPause(3); UpdateTrayIcon(false); break;
                case IDM_PAUSE_7:         SetPause(7); UpdateTrayIcon(false); break;
                case IDM_PAUSE_NEXT:      SetPause(-1); UpdateTrayIcon(false); break;
                case IDM_PAUSE_OFF:       SetPause(0); UpdateTrayIcon(false); break;
                case IDM_BLOCK:           QueueTask(Task::BlockUpdates); break;
                case IDM_UNBLOCK:         QueueTask(Task::UnblockUpdates); break;
                case IDM_OPEN_LOG:        OpenFolder(StateDir()); break;
                case IDM_OPEN_CONFIG: {
                    std::wstring cfg = SpicetifyConfigPath();
                    if (!cfg.empty()) {
                        OpenFolder(cfg.substr(0, cfg.find_last_of(L'\\')));
                    }
                    break;
                }
                case IDM_STRICT_TOGGLE: {
                    EnterCriticalSection(&g_stateLock);
                    g_state.strictMode = !g_state.strictMode;
                    bool now = g_state.strictMode;
                    LeaveCriticalSection(&g_stateLock);
                    SaveState();
                    LogF(L"ACTION", L"Strict compatibility mode %s.", now ? L"ON" : L"off");
                    if (g_hDashWnd) {
                        PostMessageW(g_hDashWnd, WM_APP_STATE_DIRTY, 0, 0);
                    }
                    break;
                }
            }
            return 0;
    }
    return DefWindowProcW(hWnd, msg, wParam, lParam);
}

// =====================================================================
// Dashboard
// =====================================================================
//
// Plain Win32 with a tab control. The settings page is deliberately almost
// empty - a Windhawk mod cannot write its own settings, so anything the mod
// changes at runtime has to live here and persist to state.json instead.

enum {
    IDC_TAB = 1000,
    IDC_STATUS_TEXT,
    IDC_LOG_EDIT,
    IDC_EXT_LIST,
    IDC_APP_LIST,
    IDC_THEME_COMBO,
    IDC_HEALTH_LIST,
    IDC_SNAP_LIST,

    IDC_BTN_FIRST = 1100,
    IDC_BTN_REPAIR = IDC_BTN_FIRST,
    IDC_BTN_UPDATE,
    IDC_BTN_RESTORE,
    IDC_BTN_APPLY,
    IDC_BTN_RESTART,
    IDC_BTN_BLOCK,
    IDC_BTN_UNBLOCK,
    IDC_BTN_MP_INSTALL,
    IDC_BTN_MP_REMOVE,
    IDC_BTN_SP_INSTALL,
    IDC_BTN_SP_UNINSTALL,
    IDC_BTN_COMPAT,
    IDC_BTN_STRICT,
    IDC_BTN_CONFIG_APPLY,
    IDC_BTN_SNAP_RESTORE,
    IDC_BTN_SNAP_NEW,
    IDC_BTN_LAST,
};

static constexpr int kTabCount = 5;
static const wchar_t* kTabNames[kTabCount] = {L"Status", L"Actions", L"Config", L"Health", L"Log"};

struct DashState {
    HWND tab = nullptr;
    HWND statusText = nullptr;
    HWND logEdit = nullptr;
    HWND extList = nullptr;
    HWND appList = nullptr;
    HWND themeCombo = nullptr;
    HWND healthList = nullptr;
    HWND snapList = nullptr;
    HWND buttons[IDC_BTN_LAST - IDC_BTN_FIRST] = {};
    HFONT font = nullptr;
    int activeTab = 0;
};

static DashState* DashOf(HWND h) {
    return (DashState*)GetWindowLongPtrW(h, GWLP_USERDATA);
}

static HWND MakeButton(HWND parent, int id, const wchar_t* text, HFONT font) {
    HWND h = CreateWindowExW(0, L"BUTTON", text, WS_CHILD | BS_PUSHBUTTON | WS_TABSTOP, 0, 0, 0, 0,
                             parent, (HMENU)(INT_PTR)id, nullptr, nullptr);
    SendMessageW(h, WM_SETFONT, (WPARAM)font, TRUE);
    return h;
}

static std::wstring BuildStatusText() {
    Status st = SnapshotStatus();
    PauseInfo p = GetPauseInfo(st.spotifyVersion);

    EnterCriticalSection(&g_compatLock);
    Verdict v = g_lastVerdict;
    LeaveCriticalSection(&g_compatLock);

    EnterCriticalSection(&g_stateLock);
    GuardianState s = g_state;
    LeaveCriticalSection(&g_stateLock);

    std::wstring t;
    auto line = [&](const wchar_t* label, const std::wstring& value) {
        wchar_t buf[1024];
        swprintf_s(buf, L"%-24s%s\r\n", label, value.c_str());
        t += buf;
    };

    switch (st.health) {
        case Health::Applied:      t += L"Spicetify is applied and healthy.\r\n\r\n"; break;
        case Health::Wiped:        t += L"SPICETIFY HAS BEEN REMOVED - a repair is needed.\r\n\r\n"; break;
        case Health::StaleBackup:  t += L"The backup does not match the installed Spotify.\r\n\r\n"; break;
        case Health::NotInstalled: t += L"Spicetify is not installed.\r\n\r\n"; break;
        case Health::NoSpotify:    t += L"Spotify was not found.\r\n\r\n"; break;
    }

    line(L"Spotify version", st.spotifyVersion.empty() ? L"not found" : st.spotifyVersion);
    line(L"Spotify running", st.spotifyRunning ? L"yes" : L"no");
    line(L"Spotify folder", st.paths.valid ? st.paths.root : L"-");
    line(L"Spicetify version", st.spicetifyVersion.empty() ? L"not found" : st.spicetifyVersion);
    line(L"Applied", st.applied ? L"yes" : L"NO");
    line(L"Backup built from", st.backupVersion.empty() ? L"(none)" : st.backupVersion);
    line(L"Backup matches", st.backupMatches ? L"yes" : L"no");
    line(L"Theme", st.theme.empty() ? L"(none)" : st.theme);
    line(L"Marketplace", st.marketplaceInstalled ? L"installed" : L"not installed");
    line(L"Spotify updates", st.updatesBlocked ? L"BLOCKED" : L"allowed");

    t += L"\r\n";
    line(L"Automatic repair", Settings().autoRepair
                                  ? (p.paused ? L"PAUSED - " + p.reason : std::wstring(L"active"))
                                  : std::wstring(L"off (see mod settings)"));
    line(L"Strict mode", s.strictMode ? L"on" : L"off");
    if (!s.lastRunUtc.empty()) {
        line(L"Last run", s.lastRunUtc);
        line(L"Last result", s.lastRunResult);
    }
    if (s.attempts > 0) {
        line(L"Consecutive failures", std::to_wstring(s.attempts));
    }

    t += L"\r\n";
    if (v.compat.available) {
        line(L"Tested Spotify range", v.compat.min + L" -> " + v.compat.max +
                                          L"   (declared by " + v.compat.tag + L")");
        const wchar_t* tierName = L"unknown";
        switch (v.tier) {
            case Tier::Supported: tierName = L"Supported"; break;
            case Tier::Untested:  tierName = L"Untested (newer than tested max)"; break;
            case Tier::TooOld:    tierName = L"Too old"; break;
            case Tier::Unknown:   tierName = L"Unknown"; break;
        }
        line(L"Verdict", tierName);
        if (v.upgradeFirst) {
            line(L"Newer Spicetify", v.compat.latestTag + L" is available");
        }
        t += L"\r\n" + v.message + L"\r\n";
    } else {
        t += L"Compatibility not checked yet - use 'Check compatibility' on the Actions tab.\r\n";
    }

    return t;
}

static void RefreshConfigTab(DashState* d) {
    Status st = SnapshotStatus();

    // Extensions: everything on disk in either folder, ticked if the config
    // lists it. Both folders are legitimate - one holds Spicetify's bundled
    // extensions, the other yours.
    ListView_DeleteAllItems(d->extList);
    std::vector<std::wstring> extFiles;
    for (const auto& dir : {EnvPath(L"APPDATA", L"spicetify\\Extensions"),
                            EnvPath(L"LOCALAPPDATA", L"spicetify\\Extensions")}) {
        for (const auto& f : ListFiles(dir)) {
            if (std::find(extFiles.begin(), extFiles.end(), f) == extFiles.end()) {
                extFiles.push_back(f);
            }
        }
    }
    // Config entries whose file has vanished still need a row, or they cannot
    // be unticked from here.
    for (const auto& e : st.extensions) {
        if (std::find(extFiles.begin(), extFiles.end(), e) == extFiles.end()) {
            extFiles.push_back(e);
        }
    }
    std::sort(extFiles.begin(), extFiles.end());

    int row = 0;
    for (const auto& f : extFiles) {
        LVITEMW it{};
        it.mask = LVIF_TEXT;
        it.iItem = row;
        it.pszText = (LPWSTR)f.c_str();
        ListView_InsertItem(d->extList, &it);
        bool on = std::find(st.extensions.begin(), st.extensions.end(), f) != st.extensions.end();
        ListView_SetCheckState(d->extList, row, on);
        row++;
    }

    ListView_DeleteAllItems(d->appList);
    std::vector<std::wstring> appDirs;
    for (const auto& dir : {EnvPath(L"APPDATA", L"spicetify\\CustomApps"),
                            EnvPath(L"LOCALAPPDATA", L"spicetify\\CustomApps")}) {
        if (dir.empty() || !DirExists(dir)) {
            continue;
        }
        WIN32_FIND_DATAW fd{};
        HANDLE h = FindFirstFileW((dir + L"\\*").c_str(), &fd);
        if (h == INVALID_HANDLE_VALUE) {
            continue;
        }
        do {
            if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && fd.cFileName[0] != L'.') {
                std::wstring n = fd.cFileName;
                if (std::find(appDirs.begin(), appDirs.end(), n) == appDirs.end()) {
                    appDirs.push_back(n);
                }
            }
        } while (FindNextFileW(h, &fd));
        FindClose(h);
    }
    for (const auto& a : st.customApps) {
        if (std::find(appDirs.begin(), appDirs.end(), a) == appDirs.end()) {
            appDirs.push_back(a);
        }
    }
    std::sort(appDirs.begin(), appDirs.end());

    row = 0;
    for (const auto& a : appDirs) {
        LVITEMW it{};
        it.mask = LVIF_TEXT;
        it.iItem = row;
        it.pszText = (LPWSTR)a.c_str();
        ListView_InsertItem(d->appList, &it);
        bool on = std::find(st.customApps.begin(), st.customApps.end(), a) != st.customApps.end();
        ListView_SetCheckState(d->appList, row, on);
        row++;
    }

    SendMessageW(d->themeCombo, CB_RESETCONTENT, 0, 0);
    SendMessageW(d->themeCombo, CB_ADDSTRING, 0, (LPARAM)L"(no theme)");
    int sel = 0;
    int idx = 1;
    std::wstring themesDir = EnvPath(L"APPDATA", L"spicetify\\Themes");
    if (DirExists(themesDir)) {
        WIN32_FIND_DATAW fd{};
        HANDLE h = FindFirstFileW((themesDir + L"\\*").c_str(), &fd);
        if (h != INVALID_HANDLE_VALUE) {
            do {
                if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && fd.cFileName[0] != L'.') {
                    SendMessageW(d->themeCombo, CB_ADDSTRING, 0, (LPARAM)fd.cFileName);
                    if (_wcsicmp(fd.cFileName, st.theme.c_str()) == 0) {
                        sel = idx;
                    }
                    idx++;
                }
            } while (FindNextFileW(h, &fd));
            FindClose(h);
        }
    }
    SendMessageW(d->themeCombo, CB_SETCURSEL, sel, 0);
}

static void RefreshHealthTab(DashState* d) {
    EnterCriticalSection(&g_statusLock);
    std::vector<HealthIssue> issues = g_issues;
    LeaveCriticalSection(&g_statusLock);

    ListView_DeleteAllItems(d->healthList);

    if (issues.empty()) {
        LVITEMW it{};
        it.mask = LVIF_TEXT;
        it.iItem = 0;
        it.pszText = (LPWSTR)L"OK";
        ListView_InsertItem(d->healthList, &it);
        ListView_SetItemText(d->healthList, 0, 1, (LPWSTR)L"No problems found");
        ListView_SetItemText(d->healthList, 0, 2, (LPWSTR)L"");
        return;
    }

    int row = 0;
    for (const auto& i : issues) {
        const wchar_t* sev = (i.severity == 2) ? L"Error" : (i.severity == 1) ? L"Warning" : L"Info";
        LVITEMW it{};
        it.mask = LVIF_TEXT;
        it.iItem = row;
        it.pszText = (LPWSTR)sev;
        ListView_InsertItem(d->healthList, &it);
        std::wstring what = i.title + L" - " + i.detail;
        ListView_SetItemText(d->healthList, row, 1, (LPWSTR)what.c_str());
        ListView_SetItemText(d->healthList, row, 2, (LPWSTR)i.fix.c_str());
        row++;
    }
}

static void RefreshLogTab(DashState* d) {
    std::wstring log = ReadTextFile(LogFile());

    // Only the tail: the log grows for the lifetime of the install and an edit
    // control does not need megabytes of it.
    constexpr size_t kMaxChars = 60000;
    if (log.size() > kMaxChars) {
        size_t cut = log.find(L'\n', log.size() - kMaxChars);
        log = L"[... earlier entries in guardian.log ...]\r\n" +
              log.substr(cut == std::wstring::npos ? log.size() - kMaxChars : cut + 1);
    }

    SetWindowTextW(d->logEdit, log.c_str());
    int len = GetWindowTextLengthW(d->logEdit);
    SendMessageW(d->logEdit, EM_SETSEL, len, len);
    SendMessageW(d->logEdit, EM_SCROLLCARET, 0, 0);
}

static void RefreshSnapshotList(DashState* d) {
    SendMessageW(d->snapList, LB_RESETCONTENT, 0, 0);
    for (const auto& n : ListSnapshots()) {
        SendMessageW(d->snapList, LB_ADDSTRING, 0, (LPARAM)n.c_str());
    }
}

static void ShowTabControls(DashState* d) {
    int t = d->activeTab;

    ShowWindow(d->statusText, t == 0 ? SW_SHOW : SW_HIDE);
    ShowWindow(d->extList,    t == 2 ? SW_SHOW : SW_HIDE);
    ShowWindow(d->appList,    t == 2 ? SW_SHOW : SW_HIDE);
    ShowWindow(d->themeCombo, t == 2 ? SW_SHOW : SW_HIDE);
    ShowWindow(d->healthList, t == 3 ? SW_SHOW : SW_HIDE);
    ShowWindow(d->logEdit,    t == 4 ? SW_SHOW : SW_HIDE);
    ShowWindow(d->snapList,   t == 1 ? SW_SHOW : SW_HIDE);

    for (int i = 0; i < IDC_BTN_LAST - IDC_BTN_FIRST; i++) {
        int id = IDC_BTN_FIRST + i;
        bool visible = false;
        if (t == 1) {
            visible = (id != IDC_BTN_CONFIG_APPLY);
        } else if (t == 2) {
            visible = (id == IDC_BTN_CONFIG_APPLY);
        }
        if (d->buttons[i]) {
            ShowWindow(d->buttons[i], visible ? SW_SHOW : SW_HIDE);
        }
    }
}

static void LayoutDashboard(HWND hWnd) {
    DashState* d = DashOf(hWnd);
    if (!d) {
        return;
    }

    RECT rc;
    GetClientRect(hWnd, &rc);
    const int pad = 10;
    int w = rc.right - rc.left;
    int h = rc.bottom - rc.top;

    MoveWindow(d->tab, pad, pad, w - 2 * pad, h - 2 * pad, TRUE);

    RECT page = {pad, pad, w - pad, h - pad};
    TabCtrl_AdjustRect(d->tab, FALSE, &page);
    int px = page.left + 6;
    int py = page.top + 6;
    int pw = page.right - page.left - 12;
    int ph = page.bottom - page.top - 12;

    MoveWindow(d->statusText, px, py, pw, ph, TRUE);
    MoveWindow(d->logEdit, px, py, pw, ph, TRUE);
    MoveWindow(d->healthList, px, py, pw, ph, TRUE);

    // Config tab: two checkbox lists side by side, theme combo underneath.
    int half = (pw - 10) / 2;
    MoveWindow(d->extList, px, py, half, ph - 70, TRUE);
    MoveWindow(d->appList, px + half + 10, py, pw - half - 10, ph - 70, TRUE);
    MoveWindow(d->themeCombo, px, py + ph - 58, half, 200, TRUE);
    if (d->buttons[IDC_BTN_CONFIG_APPLY - IDC_BTN_FIRST]) {
        MoveWindow(d->buttons[IDC_BTN_CONFIG_APPLY - IDC_BTN_FIRST], px + half + 10, py + ph - 58,
                   pw - half - 10, 28, TRUE);
    }

    // Actions tab: a grid of buttons, snapshot list below.
    const int bw = 180;
    const int bh = 30;
    const int gap = 8;
    int cols = std::max(1, (pw + gap) / (bw + gap));
    int i = 0;
    for (int id = IDC_BTN_FIRST; id < IDC_BTN_LAST; id++) {
        if (id == IDC_BTN_CONFIG_APPLY) {
            continue;
        }
        HWND b = d->buttons[id - IDC_BTN_FIRST];
        if (!b) {
            continue;
        }
        int col = i % cols;
        int rowIdx = i / cols;
        MoveWindow(b, px + col * (bw + gap), py + rowIdx * (bh + gap), bw, bh, TRUE);
        i++;
    }
    int rows = (i + cols - 1) / cols;
    int listTop = py + rows * (bh + gap) + 10;
    MoveWindow(d->snapList, px, listTop, pw, std::max(60, py + ph - listTop), TRUE);
}

static void RefreshDashboard(HWND hWnd) {
    DashState* d = DashOf(hWnd);
    if (!d) {
        return;
    }

    SetWindowTextW(d->statusText, BuildStatusText().c_str());
    RefreshConfigTab(d);
    RefreshHealthTab(d);
    RefreshLogTab(d);
    RefreshSnapshotList(d);

    EnterCriticalSection(&g_statusLock);
    bool busy = g_busy;
    LeaveCriticalSection(&g_statusLock);
    for (int i = 0; i < IDC_BTN_LAST - IDC_BTN_FIRST; i++) {
        if (d->buttons[i]) {
            EnableWindow(d->buttons[i], !busy);
        }
    }
}

// Turn the tick states back into `spicetify config` arguments. Only the
// differences are emitted, so an unchanged config produces no commands at all.
static void QueueConfigChanges(DashState* d) {
    Status st = SnapshotStatus();
    std::vector<std::wstring> args;

    auto diffList = [&](HWND list, const std::vector<std::wstring>& current,
                        const wchar_t* key) {
        int n = ListView_GetItemCount(list);
        for (int i = 0; i < n; i++) {
            wchar_t name[MAX_PATH] = {};
            ListView_GetItemText(list, i, 0, name, ARRAYSIZE(name));
            bool ticked = ListView_GetCheckState(list, i) != 0;
            bool inConfig = std::find(current.begin(), current.end(), name) != current.end();

            if (ticked && !inConfig) {
                args.push_back(std::wstring(L"config ") + key + L" " + name);
            } else if (!ticked && inConfig) {
                // Trailing '-' is Spicetify's remove-from-array syntax.
                args.push_back(std::wstring(L"config ") + key + L" " + name + L"-");
            }
        }
    };

    diffList(d->extList, st.extensions, L"extensions");
    diffList(d->appList, st.customApps, L"custom_apps");

    int sel = (int)SendMessageW(d->themeCombo, CB_GETCURSEL, 0, 0);
    if (sel >= 0) {
        wchar_t theme[MAX_PATH] = {};
        SendMessageW(d->themeCombo, CB_GETLBTEXT, sel, (LPARAM)theme);
        std::wstring want = (sel == 0) ? L" " : theme;
        std::wstring have = st.theme.empty() ? L" " : st.theme;
        if (want != have) {
            args.push_back(L"config current_theme \"" + want + L"\"");
        }
    }

    if (args.empty()) {
        MessageBoxW(g_hDashWnd, L"Nothing to change.", kAppName, MB_OK | MB_ICONINFORMATION);
        return;
    }

    std::wstring preview = L"Apply these changes?\r\n\r\n";
    for (const auto& a : args) {
        preview += L"  spicetify " + a + L"\r\n";
    }
    preview += L"\r\nSpotify will be closed and restarted.";

    if (MessageBoxW(g_hDashWnd, preview.c_str(), kAppName, MB_OKCANCEL | MB_ICONQUESTION) != IDOK) {
        return;
    }

    EnterCriticalSection(&g_queueLock);
    g_pendingConfigArgs = args;
    LeaveCriticalSection(&g_queueLock);
    QueueTask(Task::ApplyConfig);
}

static void RestoreSelectedSnapshot(DashState* d) {
    int sel = (int)SendMessageW(d->snapList, LB_GETCURSEL, 0, 0);
    if (sel == LB_ERR) {
        MessageBoxW(g_hDashWnd, L"Select a snapshot first.", kAppName, MB_OK | MB_ICONINFORMATION);
        return;
    }

    int len = (int)SendMessageW(d->snapList, LB_GETTEXTLEN, sel, 0);
    std::wstring name((size_t)len, L'\0');
    SendMessageW(d->snapList, LB_GETTEXT, sel, (LPARAM)name.data());

    std::wstring msg = L"Restore " + name + L" over your current config-xpui.ini?\r\n\r\n"
                       L"The config being replaced is snapshotted first, so this is undoable.";
    if (MessageBoxW(g_hDashWnd, msg.c_str(), kAppName, MB_OKCANCEL | MB_ICONQUESTION) != IDOK) {
        return;
    }

    SnapshotConfig(L"pre-restore");
    std::wstring cfg = SpicetifyConfigPath();
    if (cfg.empty()) {
        return;
    }
    if (CopyFileW((SnapshotDir() + L"\\" + name).c_str(), cfg.c_str(), FALSE)) {
        LogF(L"ACTION", L"Config restored from %s", name.c_str());
        QueueTask(Task::ApplyConfig);   // no pending args: just re-applies
        RefreshDashboard(g_hDashWnd);
    } else {
        MessageBoxW(g_hDashWnd, L"Could not restore that snapshot.", kAppName, MB_OK | MB_ICONERROR);
    }
}

static LRESULT CALLBACK DashWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    DashState* d = DashOf(hWnd);

    switch (msg) {
        case WM_CREATE: {
            d = new DashState();
            SetWindowLongPtrW(hWnd, GWLP_USERDATA, (LONG_PTR)d);

            NONCLIENTMETRICSW ncm{};
            ncm.cbSize = sizeof(ncm);
            SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0);
            d->font = CreateFontIndirectW(&ncm.lfMessageFont);

            HFONT mono = CreateFontW(-12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                     OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                                     FIXED_PITCH | FF_MODERN, L"Consolas");

            d->tab = CreateWindowExW(0, WC_TABCONTROLW, L"",
                                     WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, 0, 0, 0, 0, hWnd,
                                     (HMENU)IDC_TAB, nullptr, nullptr);
            SendMessageW(d->tab, WM_SETFONT, (WPARAM)d->font, TRUE);
            for (int i = 0; i < kTabCount; i++) {
                TCITEMW ti{};
                ti.mask = TCIF_TEXT;
                ti.pszText = (LPWSTR)kTabNames[i];
                TabCtrl_InsertItem(d->tab, i, &ti);
            }

            d->statusText = CreateWindowExW(
                0, L"EDIT", L"",
                WS_CHILD | WS_VSCROLL | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL, 0, 0, 0, 0,
                hWnd, (HMENU)IDC_STATUS_TEXT, nullptr, nullptr);
            SendMessageW(d->statusText, WM_SETFONT, (WPARAM)mono, TRUE);

            d->logEdit = CreateWindowExW(
                0, L"EDIT", L"",
                WS_CHILD | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE | ES_READONLY, 0, 0, 0, 0, hWnd,
                (HMENU)IDC_LOG_EDIT, nullptr, nullptr);
            SendMessageW(d->logEdit, WM_SETFONT, (WPARAM)mono, TRUE);

            auto makeList = [&](int id, bool checkboxes) {
                HWND l = CreateWindowExW(WS_EX_CLIENTEDGE, WC_LISTVIEWW, L"",
                                         WS_CHILD | LVS_REPORT | LVS_SINGLESEL, 0, 0, 0, 0, hWnd,
                                         (HMENU)(INT_PTR)id, nullptr, nullptr);
                DWORD ex = LVS_EX_FULLROWSELECT;
                if (checkboxes) {
                    ex |= LVS_EX_CHECKBOXES;
                }
                ListView_SetExtendedListViewStyle(l, ex);
                SendMessageW(l, WM_SETFONT, (WPARAM)d->font, TRUE);
                return l;
            };

            d->extList = makeList(IDC_EXT_LIST, true);
            LVCOLUMNW c{};
            c.mask = LVCF_TEXT | LVCF_WIDTH;
            c.cx = 300;
            c.pszText = (LPWSTR)L"Extensions";
            ListView_InsertColumn(d->extList, 0, &c);

            d->appList = makeList(IDC_APP_LIST, true);
            c.pszText = (LPWSTR)L"Custom apps";
            ListView_InsertColumn(d->appList, 0, &c);

            d->healthList = makeList(IDC_HEALTH_LIST, false);
            c.cx = 80;
            c.pszText = (LPWSTR)L"Severity";
            ListView_InsertColumn(d->healthList, 0, &c);
            c.cx = 520;
            c.pszText = (LPWSTR)L"Finding";
            ListView_InsertColumn(d->healthList, 1, &c);
            c.cx = 320;
            c.pszText = (LPWSTR)L"Fix";
            ListView_InsertColumn(d->healthList, 2, &c);

            d->themeCombo = CreateWindowExW(0, L"COMBOBOX", L"",
                                            WS_CHILD | WS_VSCROLL | CBS_DROPDOWNLIST, 0, 0, 0, 0,
                                            hWnd, (HMENU)IDC_THEME_COMBO, nullptr, nullptr);
            SendMessageW(d->themeCombo, WM_SETFONT, (WPARAM)d->font, TRUE);

            d->snapList = CreateWindowExW(WS_EX_CLIENTEDGE, L"LISTBOX", L"",
                                          WS_CHILD | WS_VSCROLL | LBS_NOTIFY, 0, 0, 0, 0, hWnd,
                                          (HMENU)IDC_SNAP_LIST, nullptr, nullptr);
            SendMessageW(d->snapList, WM_SETFONT, (WPARAM)mono, TRUE);

            struct {
                int id;
                const wchar_t* text;
            } kButtons[] = {
                {IDC_BTN_REPAIR,       L"Repair now"},
                {IDC_BTN_UPDATE,       L"Update Spicetify"},
                {IDC_BTN_COMPAT,       L"Check compatibility"},
                {IDC_BTN_RESTART,      L"Restart Spotify"},
                {IDC_BTN_APPLY,        L"Backup && apply"},
                {IDC_BTN_RESTORE,      L"Restore to stock"},
                {IDC_BTN_BLOCK,        L"Block Spotify updates"},
                {IDC_BTN_UNBLOCK,      L"Unblock Spotify updates"},
                {IDC_BTN_MP_INSTALL,   L"Install / repair Marketplace"},
                {IDC_BTN_MP_REMOVE,    L"Remove Marketplace"},
                {IDC_BTN_SP_INSTALL,   L"Install Spicetify"},
                {IDC_BTN_SP_UNINSTALL, L"Uninstall Spicetify"},
                {IDC_BTN_STRICT,       L"Toggle strict mode"},
                {IDC_BTN_SNAP_NEW,     L"Snapshot config now"},
                {IDC_BTN_SNAP_RESTORE, L"Restore selected snapshot"},
                {IDC_BTN_CONFIG_APPLY, L"Apply config changes"},
            };
            for (const auto& b : kButtons) {
                d->buttons[b.id - IDC_BTN_FIRST] = MakeButton(hWnd, b.id, b.text, d->font);
            }

            ShowTabControls(d);
            return 0;
        }

        case WM_SIZE:
            LayoutDashboard(hWnd);
            return 0;

        case WM_GETMINMAXINFO: {
            auto* mmi = (MINMAXINFO*)lParam;
            mmi->ptMinTrackSize.x = 760;
            mmi->ptMinTrackSize.y = 520;
            return 0;
        }

        case WM_NOTIFY: {
            auto* nh = (NMHDR*)lParam;
            if (nh->idFrom == IDC_TAB && nh->code == TCN_SELCHANGE) {
                d->activeTab = TabCtrl_GetCurSel(d->tab);
                ShowTabControls(d);
                LayoutDashboard(hWnd);
                InvalidateRect(hWnd, nullptr, TRUE);
            }
            return 0;
        }

        case WM_APP_STATE_DIRTY:
            RefreshDashboard(hWnd);
            return 0;

        case WM_COMMAND: {
            int id = LOWORD(wParam);
            switch (id) {
                case IDC_BTN_REPAIR:       QueueTask(Task::RepairForced); break;
                case IDC_BTN_UPDATE:       QueueTask(Task::UpdateCli); break;
                case IDC_BTN_COMPAT:       QueueTask(Task::RefreshCompat); break;
                case IDC_BTN_RESTART:      QueueTask(Task::RestartSpotify); break;
                case IDC_BTN_APPLY:        QueueTask(Task::BackupApply); break;

                case IDC_BTN_RESTORE:
                    if (MessageBoxW(hWnd,
                                    L"Un-patch Spotify and return it to stock?\r\n\r\n"
                                    L"Your themes, extensions and config are kept.",
                                    kAppName, MB_OKCANCEL | MB_ICONQUESTION) == IDOK) {
                        QueueTask(Task::RestoreStock);
                    }
                    break;

                case IDC_BTN_BLOCK:
                    if (MessageBoxW(hWnd,
                                    L"Block Spotify from updating?\r\n\r\n"
                                    L"You will stop receiving Spotify updates, including "
                                    L"security fixes, until you unblock.\r\n\r\n"
                                    L"This also makes automatic repair redundant - there will "
                                    L"be no updates left to break Spicetify.",
                                    kAppName, MB_OKCANCEL | MB_ICONWARNING) == IDOK) {
                        QueueTask(Task::BlockUpdates);
                    }
                    break;

                case IDC_BTN_UNBLOCK:      QueueTask(Task::UnblockUpdates); break;
                case IDC_BTN_MP_INSTALL:   QueueTask(Task::InstallMarketplace); break;

                case IDC_BTN_MP_REMOVE:
                    if (MessageBoxW(hWnd,
                                    L"Remove Marketplace?\r\n\r\n"
                                    L"Themes and extensions you installed through it are kept - "
                                    L"only the browser UI goes.",
                                    kAppName, MB_OKCANCEL | MB_ICONQUESTION) == IDOK) {
                        QueueTask(Task::RemoveMarketplace);
                    }
                    break;

                case IDC_BTN_SP_INSTALL:
                    if (MessageBoxW(hWnd,
                                    L"Download and run the official Spicetify installer from\r\n"
                                    L"raw.githubusercontent.com/spicetify/cli ?",
                                    kAppName, MB_OKCANCEL | MB_ICONQUESTION) == IDOK) {
                        QueueTask(Task::InstallSpicetify);
                    }
                    break;

                case IDC_BTN_SP_UNINSTALL:
                    if (MessageBoxW(hWnd,
                                    L"Restore Spotify to stock and remove the Spicetify CLI?\r\n\r\n"
                                    L"Your themes, extensions and custom apps are kept.",
                                    kAppName, MB_OKCANCEL | MB_ICONWARNING) == IDOK) {
                        QueueTask(Task::UninstallSpicetify);
                    }
                    break;

                case IDC_BTN_STRICT: {
                    EnterCriticalSection(&g_stateLock);
                    g_state.strictMode = !g_state.strictMode;
                    bool now = g_state.strictMode;
                    LeaveCriticalSection(&g_stateLock);
                    SaveState();
                    LogF(L"ACTION", L"Strict compatibility mode %s.", now ? L"ON" : L"off");
                    RefreshDashboard(hWnd);
                    break;
                }

                case IDC_BTN_SNAP_NEW:
                    SnapshotConfig(L"manual");
                    RefreshSnapshotList(d);
                    break;

                case IDC_BTN_SNAP_RESTORE: RestoreSelectedSnapshot(d); break;
                case IDC_BTN_CONFIG_APPLY: QueueConfigChanges(d); break;
            }
            return 0;
        }

        case WM_CLOSE:
            DestroyWindow(hWnd);
            return 0;

        case WM_DESTROY:
            if (d) {
                if (d->font) {
                    DeleteObject(d->font);
                }
                delete d;
                SetWindowLongPtrW(hWnd, GWLP_USERDATA, 0);
            }
            g_hDashWnd = nullptr;
            return 0;
    }

    return DefWindowProcW(hWnd, msg, wParam, lParam);
}

static void ShowDashboard() {
    if (g_hDashWnd && IsWindow(g_hDashWnd)) {
        ShowWindow(g_hDashWnd, SW_RESTORE);
        SetForegroundWindow(g_hDashWnd);
        RefreshDashboard(g_hDashWnd);
        return;
    }

    static const wchar_t kClass[] = L"SpicetifyGuardianDashboard";
    static bool registered = false;
    if (!registered) {
        WNDCLASSEXW wc{};
        wc.cbSize = sizeof(wc);
        wc.lpfnWndProc = DashWndProc;
        wc.hInstance = GetModuleHandleW(nullptr);
        wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
        wc.lpszClassName = kClass;
        RegisterClassExW(&wc);
        registered = true;
    }

    int w = 900;
    int h = 620;
    int x = (GetSystemMetrics(SM_CXSCREEN) - w) / 2;
    int y = (GetSystemMetrics(SM_CYSCREEN) - h) / 2;

    g_hDashWnd = CreateWindowExW(0, kClass, kAppName, WS_OVERLAPPEDWINDOW, x, y, w, h, nullptr,
                                 nullptr, GetModuleHandleW(nullptr), nullptr);
    if (!g_hDashWnd) {
        return;
    }

    HICON icon = MakeTrayIcon(TrayIconState());
    SendMessageW(g_hDashWnd, WM_SETICON, ICON_SMALL, (LPARAM)icon);

    RefreshDashboard(g_hDashWnd);
    LayoutDashboard(g_hDashWnd);
    ShowWindow(g_hDashWnd, SW_SHOW);
    SetForegroundWindow(g_hDashWnd);
}

// =====================================================================
// Tool mod entry points
// =====================================================================

BOOL WhTool_ModInit();
void WhTool_ModSettingsChanged();
void WhTool_ModUninit();

BOOL WhTool_ModInit() {
    InitializeCriticalSection(&g_logLock);
    InitializeCriticalSection(&g_stateLock);
    InitializeCriticalSection(&g_settingsLock);
    InitializeCriticalSection(&g_statusLock);
    InitializeCriticalSection(&g_queueLock);
    InitializeCriticalSection(&g_compatLock);

    EnsureStateDir();
    LoadModSettings();
    LoadState();

    INITCOMMONCONTROLSEX icc{};
    icc.dwSize = sizeof(icc);
    icc.dwICC = ICC_TAB_CLASSES | ICC_LISTVIEW_CLASSES | ICC_STANDARD_CLASSES;
    InitCommonControlsEx(&icc);

    g_taskbarCreatedMsg = RegisterWindowMessageW(L"TaskbarCreated");

    static const wchar_t kTrayClass[] = L"SpicetifyGuardianTray";
    WNDCLASSW wc{};
    wc.lpfnWndProc = TrayWndProc;
    wc.hInstance = GetModuleHandleW(nullptr);
    wc.lpszClassName = kTrayClass;
    RegisterClassW(&wc);

    g_hTrayWnd = CreateWindowExW(WS_EX_TOOLWINDOW, kTrayClass, nullptr, WS_POPUP, 0, 0, 0, 0,
                                 nullptr, nullptr, wc.hInstance, nullptr);
    if (!g_hTrayWnd) {
        Wh_Log(L"Could not create the tray window");
        return FALSE;
    }

    g_stopEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    g_workEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);

    UpdateTrayIcon(true);

    g_workerThread = CreateThread(nullptr, 0, WorkerThreadProc, nullptr, 0, nullptr);
    if (!g_workerThread) {
        Wh_Log(L"Could not start the worker thread");
        return FALSE;
    }

    GuardianLog(L"INFO", L"Spicetify Guardian started.");
    return TRUE;
}

void WhTool_ModSettingsChanged() {
    LoadModSettings();
    if (g_hTrayWnd) {
        PostMessageW(g_hTrayWnd, WM_APP_STATE_DIRTY, 0, 0);
    }
}

void WhTool_ModUninit() {
    if (g_stopEvent) {
        SetEvent(g_stopEvent);
    }
    if (g_workerThread) {
        // Generous: a repair in flight may be waiting on spicetify.
        if (WaitForSingleObject(g_workerThread, 30000) == WAIT_TIMEOUT) {
            Wh_Log(L"Worker thread did not stop in time");
        }
        CloseHandle(g_workerThread);
        g_workerThread = nullptr;
    }

    if (g_hDashWnd && IsWindow(g_hDashWnd)) {
        DestroyWindow(g_hDashWnd);
    }

    if (g_hTrayWnd) {
        NOTIFYICONDATAW nid{};
        nid.cbSize = sizeof(nid);
        nid.hWnd = g_hTrayWnd;
        nid.uID = kTrayIconId;
        Shell_NotifyIconW(NIM_DELETE, &nid);
        DestroyWindow(g_hTrayWnd);
        g_hTrayWnd = nullptr;
    }

    if (g_stopEvent) {
        CloseHandle(g_stopEvent);
        g_stopEvent = nullptr;
    }
    if (g_workEvent) {
        CloseHandle(g_workEvent);
        g_workEvent = nullptr;
    }

    GuardianLog(L"INFO", L"Spicetify Guardian stopped.");

    DeleteCriticalSection(&g_compatLock);
    DeleteCriticalSection(&g_queueLock);
    DeleteCriticalSection(&g_statusLock);
    DeleteCriticalSection(&g_settingsLock);
    DeleteCriticalSection(&g_stateLock);
    DeleteCriticalSection(&g_logLock);
}

////////////////////////////////////////////////////////////////////////////////
// Windhawk tool mod implementation for mods which don't need to inject to other
// processes or hook other functions. Context:
// https://github.com/ramensoftware/windhawk/wiki/Mods-as-tools:-Running-mods-in-a-dedicated-process
//
// The mod will load and run in a dedicated windhawk.exe process.
//
// Paste the code below as part of the mod code, and use these callbacks:
// * WhTool_ModInit
// * WhTool_ModSettingsChanged
// * WhTool_ModUninit
//
// Currently, other callbacks are not supported.

bool g_isToolModProcessLauncher;
HANDLE g_toolModProcessMutex;

void WINAPI EntryPoint_Hook() {
    Wh_Log(L">");
    ExitThread(0);
}

BOOL Wh_ModInit() {
    DWORD sessionId;
    if (ProcessIdToSessionId(GetCurrentProcessId(), &sessionId) && sessionId == 0) {
        return FALSE;
    }

    bool isExcluded = false;
    bool isToolModProcess = false;
    bool isCurrentToolModProcess = false;
    int argc;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLine(), &argc);
    if (!argv) {
        Wh_Log(L"CommandLineToArgvW failed");
        return FALSE;
    }

    for (int i = 1; i < argc; i++) {
        if (wcscmp(argv[i], L"-service") == 0 || wcscmp(argv[i], L"-service-start") == 0 ||
            wcscmp(argv[i], L"-service-stop") == 0) {
            isExcluded = true;
            break;
        }
    }

    for (int i = 1; i < argc - 1; i++) {
        if (wcscmp(argv[i], L"-tool-mod") == 0) {
            isToolModProcess = true;
            if (wcscmp(argv[i + 1], WH_MOD_ID) == 0) {
                isCurrentToolModProcess = true;
            }
            break;
        }
    }

    LocalFree(argv);

    if (isExcluded) {
        return FALSE;
    }

    if (isCurrentToolModProcess) {
        g_toolModProcessMutex = CreateMutex(nullptr, TRUE, L"windhawk-tool-mod_" WH_MOD_ID);
        if (!g_toolModProcessMutex) {
            Wh_Log(L"CreateMutex failed");
            ExitProcess(1);
        }

        if (GetLastError() == ERROR_ALREADY_EXISTS) {
            Wh_Log(L"Tool mod already running (%s)", WH_MOD_ID);
            ExitProcess(1);
        }

        if (!WhTool_ModInit()) {
            ExitProcess(1);
        }

        IMAGE_DOS_HEADER* dosHeader = (IMAGE_DOS_HEADER*)GetModuleHandle(nullptr);
        IMAGE_NT_HEADERS* ntHeaders =
            (IMAGE_NT_HEADERS*)((BYTE*)dosHeader + dosHeader->e_lfanew);

        DWORD entryPointRVA = ntHeaders->OptionalHeader.AddressOfEntryPoint;
        void* entryPoint = (BYTE*)dosHeader + entryPointRVA;

        Wh_SetFunctionHook(entryPoint, (void*)EntryPoint_Hook, nullptr);
        return TRUE;
    }

    if (isToolModProcess) {
        return FALSE;
    }

    g_isToolModProcessLauncher = true;
    return TRUE;
}

void Wh_ModAfterInit() {
    if (!g_isToolModProcessLauncher) {
        return;
    }

    WCHAR currentProcessPath[MAX_PATH];
    switch (GetModuleFileName(nullptr, currentProcessPath, ARRAYSIZE(currentProcessPath))) {
        case 0:
        case ARRAYSIZE(currentProcessPath):
            Wh_Log(L"GetModuleFileName failed");
            return;
    }

    WCHAR commandLine[MAX_PATH + 2 +
                      (sizeof(L" -tool-mod \"" WH_MOD_ID "\"") / sizeof(WCHAR)) - 1];
    swprintf_s(commandLine, L"\"%s\" -tool-mod \"%s\"", currentProcessPath, WH_MOD_ID);

    HMODULE kernelModule = GetModuleHandle(L"kernelbase.dll");
    if (!kernelModule) {
        kernelModule = GetModuleHandle(L"kernel32.dll");
        if (!kernelModule) {
            Wh_Log(L"No kernelbase.dll/kernel32.dll");
            return;
        }
    }

    using CreateProcessInternalW_t =
        BOOL(WINAPI*)(HANDLE hUserToken, LPCWSTR lpApplicationName, LPWSTR lpCommandLine,
                      LPSECURITY_ATTRIBUTES lpProcessAttributes,
                      LPSECURITY_ATTRIBUTES lpThreadAttributes, WINBOOL bInheritHandles,
                      DWORD dwCreationFlags, LPVOID lpEnvironment, LPCWSTR lpCurrentDirectory,
                      LPSTARTUPINFOW lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation,
                      PHANDLE hRestrictedUserToken);
    CreateProcessInternalW_t pCreateProcessInternalW =
        (CreateProcessInternalW_t)GetProcAddress(kernelModule, "CreateProcessInternalW");
    if (!pCreateProcessInternalW) {
        Wh_Log(L"No CreateProcessInternalW");
        return;
    }

    STARTUPINFO si{
        .cb = sizeof(STARTUPINFO),
        .dwFlags = STARTF_FORCEOFFFEEDBACK,
    };
    PROCESS_INFORMATION pi;
    if (!pCreateProcessInternalW(nullptr, currentProcessPath, commandLine, nullptr, nullptr, FALSE,
                                 NORMAL_PRIORITY_CLASS, nullptr, nullptr, &si, &pi, nullptr)) {
        Wh_Log(L"CreateProcess failed");
        return;
    }

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}

void Wh_ModSettingsChanged() {
    if (g_isToolModProcessLauncher) {
        return;
    }

    WhTool_ModSettingsChanged();
}

void Wh_ModUninit() {
    if (g_isToolModProcessLauncher) {
        return;
    }

    WhTool_ModUninit();
    ExitProcess(0);
}
