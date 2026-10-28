// Loads a DLL into a running process by name (CreateRemoteThread +
// LoadLibraryW), and can lock the workstation. For the lock screen test build.
//
//   inject <process.exe> <dll>   the DLL must be readable by the target, e.g.
//                                inside LockApp's own AC\Temp folder
//   inject lock                  LockWorkStation()

#include <windows.h>
#include <tlhelp32.h>

#include <cstdio>
#include <cwchar>

static DWORD FindPid(const wchar_t* exe) {
    DWORD pid = 0;
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32W pe{sizeof pe};
    for (BOOL ok = Process32FirstW(snap, &pe); ok; ok = Process32NextW(snap, &pe)) {
        if (!_wcsicmp(pe.szExeFile, exe)) pid = pe.th32ProcessID;
    }
    CloseHandle(snap);
    return pid;
}

int wmain(int argc, wchar_t** argv) {
    if (argc == 2 && !wcscmp(argv[1], L"lock")) {
        return LockWorkStation() ? 0 : (printf("LockWorkStation failed: %lu\n", GetLastError()), 1);
    }
    if (argc < 3) {
        printf("usage: inject <process.exe> <dll> | inject lock\n");
        return 2;
    }
    DWORD pid = FindPid(argv[1]);
    if (!pid) {
        printf("%ls is not running\n", argv[1]);
        return 1;
    }
    WCHAR dll[MAX_PATH];
    GetFullPathNameW(argv[2], MAX_PATH, dll, nullptr);
    HANDLE p = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION |
                               PROCESS_VM_WRITE | PROCESS_VM_READ,
                           FALSE, pid);
    if (!p) {
        printf("OpenProcess(%lu) failed: %lu\n", pid, GetLastError());
        return 1;
    }
    size_t bytes = (wcslen(dll) + 1) * sizeof(WCHAR);
    void* mem = VirtualAllocEx(p, nullptr, bytes, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!mem || !WriteProcessMemory(p, mem, dll, bytes, nullptr)) {
        printf("writing the path failed: %lu\n", GetLastError());
        return 1;
    }
    auto load = (LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "LoadLibraryW");
    HANDLE t = CreateRemoteThread(p, nullptr, 0, load, mem, 0, nullptr);
    if (!t) {
        printf("CreateRemoteThread failed: %lu\n", GetLastError());
        return 1;
    }
    DWORD wait = WaitForSingleObject(t, 15000);
    DWORD code = 0;
    GetExitCodeThread(t, &code);
    printf("pid %lu: LoadLibraryW %s (0x%08lX)\n", pid, wait == WAIT_OBJECT_0 ? (code ? "loaded" : "FAILED") : "still waiting",
           code);
    return wait == WAIT_OBJECT_0 && code ? 0 : 1;
}
