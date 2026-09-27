#pragma once
#include "DataJson.h"
#include <filesystem>

namespace Client
{
// Source-only editing. Save and Publish run the same strict Guide domain validator.
class CGuideAIDocument final
{
public:
    ~CGuideAIDocument();
    bool Load();
    bool Start_Save();
    bool Start_Publish();
    void Poll();
    bool Is_Busy() const { return m_Process != nullptr; }
    bool Is_Loaded() const { return m_Loaded; }
    bool Is_Dirty() const;
    uint64_t Published_Revision() const { return m_PublishedRevision; }
    DATA_JSON_VALUE& Draft() { return m_Draft; }
    const DATA_JSON_VALUE& Draft() const { return m_Draft; }
    const std::string& Status() const { return m_Status; }
    void Set_Status(std::string value) { m_Status = std::move(value); }
    static std::string Serialize(const DATA_JSON_VALUE& value);
    static bool Read(const std::filesystem::path& path, DATA_JSON_VALUE& value, std::string& status);
private:
    bool Start(const wchar_t* mode);
    bool Read_Source(DATA_JSON_VALUE& candidate);
    void Read_PublishedRevision();
    DATA_JSON_VALUE m_Baseline, m_Draft, m_LoadCandidate;
    bool m_Loaded = false;
    uint64_t m_PublishedRevision = 0;
    HANDLE m_Process = nullptr;
    std::filesystem::path m_LogPath, m_BaselinePath, m_DraftPath;
    std::wstring m_Mode;
    std::string m_Status;
};
namespace GuideJson
{
using J = DATA_JSON_VALUE;
const J& Field(const J& value, const char* name);
std::string String(const J& value, const char* name);
double Number(const J& value, const char* name, double fallback = 0.);
bool Boolean(const J& value, const char* name, bool fallback = false);
void Set(J& value, const char* name, J field);
}
}
