#pragma once

#include <windows.h>

#include <cstdint>
#include <string>

class c_memory {
public:
    explicit c_memory(const std::string& process_name);
    ~c_memory();

    bool valid() const;
    std::string find_username();

private:
    DWORD     m_pid{};
    HANDLE    m_process_handle{};
    uintptr_t m_base_address{};

    DWORD find_pid(const std::string& name);
    uintptr_t find_module_base(const std::string& name);
};
