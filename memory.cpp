#include "memory.hpp"

#include <windows.h>
#include <tlhelp32.h>

#include <algorithm>
#include <vector>


c_memory::c_memory(const std::string& process_name) {
    m_pid = find_pid(process_name);
    if (!m_pid) return;

    m_process_handle = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, FALSE, m_pid);
    if (!m_process_handle) return;

    m_base_address = find_module_base(process_name);
}

c_memory::~c_memory() {
    if (m_process_handle) CloseHandle(m_process_handle);
}

bool c_memory::valid() const {
    return m_process_handle != nullptr;
}

std::string c_memory::find_username() {
    SYSTEM_INFO sys_info;
    GetSystemInfo(&sys_info);

    uintptr_t addr = (uintptr_t)sys_info.lpMinimumApplicationAddress;
    uintptr_t max_addr = (uintptr_t)sys_info.lpMaximumApplicationAddress;

    std::string pattern_str = ("\"username\":\"");
    const char* pattern = pattern_str.c_str();
    size_t pattern_len = strlen(pattern);

    while (addr < max_addr) {
        MEMORY_BASIC_INFORMATION mbi;
        if (VirtualQueryEx(m_process_handle, (LPCVOID)addr, &mbi, sizeof(mbi)) == 0) {
            addr += sys_info.dwPageSize;
            continue;
        }

        if (mbi.State == MEM_COMMIT && (mbi.Protect & PAGE_READWRITE)) {
            size_t size = min(mbi.RegionSize, (SIZE_T)1024 * 1024 * 10);
            std::vector<char> buffer(size);
            SIZE_T bytes_read;

            if (ReadProcessMemory(m_process_handle, mbi.BaseAddress, buffer.data(), size, &bytes_read)) {
                for (size_t i = 0; i < bytes_read - pattern_len; i++) {
                    if (memcmp(&buffer[i], pattern, pattern_len) == 0) {
                        const char* start = &buffer[i] + pattern_len;
                        const char* end = start;

                        while (end < &buffer[bytes_read] && *end && *end != '"') end++;

                        if (end > start) {
                            size_t len = end - start;
                            if (len > 2 && len < 64) {
                                std::string username(start, len);
                                bool valid_name = true;
                                for (char c : username) {
                                    if (c < 0x20 || c > 0x7E) { valid_name = false; break; }
                                }
                                std::string discord_str = ("discord");
                                bool has_discord = username.find(discord_str) == std::string::npos;
                                if (valid_name && has_discord) {
                                    pattern_str.clear();
                                    return username;
                                }
                            }
                        }
                    }
                }
            }
        }

        addr += mbi.RegionSize;
    }

    pattern_str.clear();
    return "";
}

DWORD c_memory::find_pid(const std::string& name) {
    PROCESSENTRY32W entry{ sizeof(entry) };
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) return 0;

    std::wstring wide(name.begin(), name.end());

    if (Process32FirstW(snap, &entry)) {
        do {
            if (_wcsicmp(entry.szExeFile, wide.c_str()) == 0) {
                DWORD pid = entry.th32ProcessID;
                CloseHandle(snap);
                return pid;
            }
        } while (Process32NextW(snap, &entry));
    }
    CloseHandle(snap);
    return 0;
}

uintptr_t c_memory::find_module_base(const std::string& name) {
    MODULEENTRY32W entry{ sizeof(entry) };
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, m_pid);
    if (snap == INVALID_HANDLE_VALUE) return 0;

    std::wstring wide(name.begin(), name.end());

    if (Module32FirstW(snap, &entry)) {
        do {
            if (_wcsicmp(entry.szModule, wide.c_str()) == 0) {
                uintptr_t base = (uintptr_t)entry.modBaseAddr;
                CloseHandle(snap);
                return base;
            }
        } while (Module32NextW(snap, &entry));
    }
    CloseHandle(snap);
    return 0;
}
