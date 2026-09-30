#pragma once
#ifdef _DEBUG
#include "Network/PacketMessages.h"
#include <chrono>
#include <memory>
#include <string>

namespace Client
{
class IPlayerCommandSink;

class CMaharakaAITool final
{
public:
    explicit CMaharakaAITool(std::shared_ptr<IPlayerCommandSink> sink);
    void Set_Active(bool active);
    void Open();
    void Update();
    void Render();
    bool Is_Open() const { return m_open; }
    bool Consume_InteractionRequest();

private:
    bool Is_Dirty() const;
    bool Is_Stale() const;
    void Submit(LostArk::Shared::MAHARAKA_AI_OPERATION operation);
    void Rebase_Draft();

    std::shared_ptr<IPlayerCommandSink> m_sink;
    LostArk::Shared::MAHARAKA_AI_TUNING m_server{}, m_base{}, m_draft{};
    LostArk::Shared::MAHARAKA_AI_OPERATION m_operation = LostArk::Shared::MAHARAKA_AI_OPERATION::GET;
    bool m_open = false, m_active = false, m_interaction = false;
    bool m_hasServer = false, m_hasDraft = false;
    std::uint32_t m_sequence = 0u, m_pending = 0u;
    std::chrono::steady_clock::time_point m_submittedAt{};
    std::string m_status = "Enter Maharaka to read the Server's active AI settings.";
};
}
#endif
