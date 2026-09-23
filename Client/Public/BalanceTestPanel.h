#pragma once

#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

namespace Client
{
// Common numeric authoring is independent of the Valtan pattern draft.
class CBalanceTestPanel final
{
public:
    CBalanceTestPanel();
    ~CBalanceTestPanel();
    void Render(bool& open);
    static void Render_KillBossControl();
    static void Render_CooldownControl();

private:
    struct FIELD { std::string name; double original = 0, value = 0; bool integral = true; std::string sourcePath, sourceArray; };
    struct ROW { std::string id, label; std::vector<FIELD> fields; };
    struct DOCUMENT { std::string path, label; std::vector<ROW> rows; };
    struct JOB;
    bool Reload();
    bool Is_Dirty() const;
    void Start_Job(bool publish);
    void Poll_Job();
    std::vector<DOCUMENT> m_documents;
    std::unique_ptr<JOB> m_job;
    std::size_t m_document = 0, m_row = 0;
    bool m_confirmReload = false;
    std::string m_status;
};
}
