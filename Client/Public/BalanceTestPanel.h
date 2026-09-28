#pragma once

#include "Network/PacketMessages.h"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace Client
{
class CNetworkPlayerCommandSink;
// Common numeric authoring is independent of the Valtan pattern draft.
class CBalanceTestPanel final
{
public:
    CBalanceTestPanel();
    ~CBalanceTestPanel();
    void Update();
    void Render(bool& open);
    static void Render_KillBossControl();
    static void Render_CooldownControl();

private:
    struct FIELD { std::string name; double original = 0, value = 0; bool integral = true; };
    struct ROW { std::string id, label, damageProfileId; bool isAltV = false; std::vector<FIELD> fields; };
    struct DOCUMENT { LostArk::Shared::BALANCE_DOMAIN domain{}; std::string label; std::vector<ROW> rows; };
    bool Reload();
    bool Is_Dirty() const;
    void Save_AndApply();
    void Install_Snapshot(const LostArk::Shared::GameplayDataRevision& revision,
        const std::vector<LostArk::Shared::BALANCE_NUMERIC_ENTRY>& entries);
    std::vector<DOCUMENT> m_documents;
    std::unique_ptr<CNetworkPlayerCommandSink> m_sink;
    LostArk::Shared::GameplayDataRevision m_revision{}, m_observedRevision{}, m_appliedRevision{};
    std::uint32_t m_sequence = 0u, m_pending = 0u;
    std::uint64_t m_submittedAt = 0u;
    std::size_t m_document = 0, m_row = 0;
    bool m_confirmReload = false;
    char m_filter[192]{};
    int m_skillFilter = 0;
    std::string m_status;
};
}
