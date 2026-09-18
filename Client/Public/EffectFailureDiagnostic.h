#pragma once

#pragma push_macro("new")
#undef new
#include <Windows.h>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <mutex>
#include <string>
#include <string_view>
#include <unordered_map>
#pragma pop_macro("new")

namespace Client
{
// Failure-only diagnostics remain available in the shared Release package.
// No rendering/admission state is changed if logging is unavailable.
inline void Write_EffectFailureDiagnostic(const char* channel, std::string_view detail) noexcept
{
    try
    {
        static std::mutex mutex;
        static unsigned int records = 0u;
        static std::unordered_map<std::string, unsigned long long> repeats;
        const std::lock_guard<std::mutex> lock(mutex);
        if (records >= 1024u) return;
        std::string line(detail.substr(0u, 4096u));
        for (char& value : line) if (value == '\r' || value == '\n') value = ' ';
        const std::string key = std::string(channel) + ':' + line;
        auto repeated = repeats.find(key);
        if (repeated == repeats.end() && repeats.size() < 256u)
            repeated = repeats.emplace(key, 0ULL).first;
        const auto occurrences = repeated == repeats.end() ? 1ULL : ++repeated->second;
        // First and power-of-two repetitions retain a count without per-frame flooding.
        if (occurrences > 1u && (occurrences & (occurrences - 1u)) != 0u) return;
        ++records;
        wchar_t modulePath[32768]{};
        const DWORD length = GetModuleFileNameW(nullptr, modulePath, 32768u);
        std::filesystem::path path = "EffectFailure.user.log";
        if (length > 0u && length < 32768u)
            path = std::filesystem::path(modulePath).parent_path().parent_path().parent_path() /
                L"Default" / L"EffectFailure.user.log";
        std::error_code error;
        const auto bytes = std::filesystem::file_size(path, error);
        const bool rotate = !error && bytes >= 4u * 1024u * 1024u;
        std::ofstream output(path, std::ios::binary | (rotate ? std::ios::trunc : std::ios::app));
        if (!output) return;
        FILETIME fileTime{};
        GetSystemTimeAsFileTime(&fileTime);
        ULARGE_INTEGER ticks{};
        ticks.LowPart = fileTime.dwLowDateTime;
        ticks.HighPart = fileTime.dwHighDateTime;
        const auto utcMs = (ticks.QuadPart - 116444736000000000ULL) / 10000ULL;
        SYSTEMTIME time{};
        GetLocalTime(&time);
        output << std::setfill('0') << std::setw(4) << time.wYear << '-'
            << std::setw(2) << time.wMonth << '-' << std::setw(2) << time.wDay << ' '
            << std::setw(2) << time.wHour << ':' << std::setw(2) << time.wMinute << ':'
            << std::setw(2) << time.wSecond << '.' << std::setw(3) << time.wMilliseconds
            << " utc_ms=" << utcMs << " pid=" << GetCurrentProcessId()
            << " occurrence_count=" << occurrences << " channel=" << channel << ' ' << line << '\n';
    }
    catch (...) { }
}

// Only startup/prepare/spawn scopes use this; no per-frame/particle logging.
struct EFFECT_SLOW_SCOPE_DIAGNOSTIC final
{
    const char* phase;
    std::string_view asset;
    std::string_view group;
    ULONGLONG start = GetTickCount64();
    ~EFFECT_SLOW_SCOPE_DIAGNOSTIC() noexcept
    {
        const auto elapsed = GetTickCount64() - start;
        if (elapsed < 50u) return;
        try
        {
            Write_EffectFailureDiagnostic(phase, "elapsed_ms=" + std::to_string(elapsed) +
                " asset=" + std::string(asset) + " group=" + std::string(group));
        }
        catch (...) { }
    }
};
}
