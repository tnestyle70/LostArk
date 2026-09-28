#pragma once

#include "GameplayCatalog.h"
#include "Gameplay/BalanceNumericContract.h"
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

namespace LostArk::Server
{
    struct SERVER_BALANCE_PREPARED final
    {
        std::shared_ptr<const CGameplayCatalog> Generation;
        LostArk::Shared::GameplayDataRevision BaseNumericRevision{}, NumericRevision{};
        std::vector<LostArk::Shared::BALANCE_NUMERIC_CHANGE> Changes;
        std::string BootstrapBytes;
        std::string NumericReceiptBytes;
        std::string SourceBindingsBytes;
        // Everything consumed by the I/O worker is immutable and owned here.
        std::string BaseBootstrapBytes;
        std::filesystem::path BootstrapPath, AuthoringDataRoot;
        std::filesystem::path RuntimeActivePointerPath;
        std::string RuntimeActivePointerBaseBytes, RuntimeActivePointerBytes;
        std::shared_ptr<const std::vector<LostArk::Shared::BALANCE_NUMERIC_ENTRY>> Entries;
    };

    class CServerBalanceNumericStore final
    {
    public:
        bool Initialize(std::shared_ptr<const CGameplayCatalog> active, std::string& status);
        bool BuildSnapshot(std::uint32_t sequence, std::uint32_t page,
            LostArk::Shared::S2C_BALANCE_SNAPSHOT& result, std::string& status) const;
        bool PreparePatch(const LostArk::Shared::C2S_BALANCE_PATCH& request,
            SERVER_BALANCE_PREPARED& result, LostArk::Shared::BALANCE_APPLY_RESULT& failure,
            std::string& status) const;
        bool PersistPrepared(const SERVER_BALANCE_PREPARED& prepared, std::string& status) const;
        void CommitPrepared(const SERVER_BALANCE_PREPARED& prepared) noexcept;
        const std::shared_ptr<const CGameplayCatalog>& Get_ActiveGeneration() const noexcept { return m_Active; }
        const LostArk::Shared::GameplayDataRevision& Get_NumericRevision() const noexcept { return m_Revision; }

    private:
        std::shared_ptr<const CGameplayCatalog> m_Active;
        std::shared_ptr<const std::vector<LostArk::Shared::BALANCE_NUMERIC_ENTRY>> m_Entries;
        LostArk::Shared::GameplayDataRevision m_Revision{};
        std::filesystem::path m_BootstrapPath, m_AuthoringDataRoot;
        std::string m_SourceBindingsBytes;
    };
}
