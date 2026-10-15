#pragma once

// 현재 generation을 읽고 동일한 런타임 validator로 숫자 변경 후보를 만들기 위한 catalog 계약이다.
#include "GameplayCatalog.h"
// 조회 entry, 변경 field, revision과 typed 결과를 Shared 계약으로 주고받는다.
#include "Gameplay/BalanceNumericContract.h"
// 저작 정본·설치 bootstrap·활성 포인터의 물리 경로를 준비 결과에 보관한다.
#include <filesystem>
// worker와 룸 스레드가 불변 catalog·snapshot을 함께 참조하는 수명을 유지한다.
#include <memory>
// 후보 bytes와 원본 bytes를 소유해 저장 직전 동일성 검사와 rollback에 사용한다.
#include <string>
// 변경 field 목록과 조회 snapshot의 순서 있는 값을 소유한다.
#include <vector>

namespace LostArk::Server
{
    // PreparePatch가 만드는 값 후보다. ServerApp은 모든 방에 stage한 뒤 이 입력만 저장 worker에 넘긴다.
    // 디스크 저장과 메모리 활성화를 분리하므로 준비 성공 자체가 런타임 적용 성공은 아니다.
    struct SERVER_BALANCE_PREPARED final
    {
        // 같은 변경을 반영한 읽기 전용 catalog와 전/후 숫자 identity다. 원본 active catalog는 변경하지 않는다.
        std::shared_ptr<const CGameplayCatalog> Generation;
        LostArk::Shared::GameplayDataRevision BaseNumericRevision{}, NumericRevision{};
        std::vector<LostArk::Shared::BALANCE_NUMERIC_CHANGE> Changes;
        // 재시작 시 같은 세대를 확인할 bootstrap과 receipt 및 패턴 피해의 저작 위치 연결 정보다.
        std::string BootstrapBytes;
        std::string NumericReceiptBytes;
        std::string SourceBindingsBytes;
        // I/O worker는 여기 보관한 불변 값만 소비한다. 시작 시 읽은 원본과 현재 디스크를 대조해 동시 저장을 거부한다.
        std::string BaseBootstrapBytes;
        std::filesystem::path BootstrapPath, AuthoringDataRoot;
        // Debug의 저장 활성 포인터가 존재할 때만 같은 파일 교체 묶음에 포함한다. 빈 경로는 포인터 교체가 없다는 뜻이다.
        std::filesystem::path RuntimeActivePointerPath;
        std::string RuntimeActivePointerBaseBytes, RuntimeActivePointerBytes;
        // 후보 catalog에서 다시 계산한 조회 snapshot이다. commit 때 catalog와 함께 교체한다.
        std::shared_ptr<const std::vector<LostArk::Shared::BALANCE_NUMERIC_ENTRY>> Entries;
    };

    // ServerApp의 숫자 밸런스 조회·후보 검증·저장을 맡는다. gameplay tick과 전원 승인 순서는 ServerApp이 소유한다.
    // 자체 mutex는 없다. worker는 저장소 복사본을 읽고 CommitPrepared는 RoomThread에서 직렬 호출한다.
    class CServerBalanceNumericStore final
    {
    public:
        // Run 또는 활성 catalog 변경 뒤 호출한다. snapshot과 저작 경로·binding을 읽어 현재 기준을 만든다.
        // 실패 시 status를 남긴다. 경로/binding 멤버는 중간에 갱신되므로 전체 멤버 rollback 함수는 아니다.
        bool Initialize(std::shared_ptr<const CGameplayCatalog> active, std::string& status);
        // RoomThread의 조회 처리에서 요청 sequence와 page에 해당하는 snapshot을 복사한다. 실패는 기존 result를 유지한다.
        bool BuildSnapshot(std::uint32_t sequence, std::uint32_t page, LostArk::Shared::S2C_BALANCE_SNAPSHOT& result,
                           std::string& status) const;
        // 검증 worker가 요청 기준 revision, stable field, 이전 값, 중복·수치 범위를 확인하고 새 catalog를 만든다.
        // 성공 시에만 result를 교체한다. 디스크와 현재 활성 세대는 바꾸지 않는다.
        bool PreparePatch(const LostArk::Shared::C2S_BALANCE_PATCH& request, SERVER_BALANCE_PREPARED& result,
                          LostArk::Shared::BALANCE_APPLY_RESULT& failure, std::string& status) const;
        // 저장 worker가 저작 lock을 잡고 최신 디스크의 기존 값·소유 identity를 다시 검사한 뒤 후보 파일을 교체한다.
        // 실패는 자기 변경을 역순 복구 시도하며 외부 수정은 덮지 않는다. 남은 backup은 복구 근거로 보존한다.
        bool PersistPrepared(const SERVER_BALANCE_PREPARED& prepared, std::string& status) const;
        // ServerApp이 디스크 저장과 모든 방 commit을 끝내는 tick 경계에서 호출한다. 불변 참조와 revision만 교체한다.
        void CommitPrepared(const SERVER_BALANCE_PREPARED& prepared) noexcept;

        // 현재 활성 참조를 차용한다. 장기 보관이 필요하면 shared_ptr 값을 복사한다.
        const std::shared_ptr<const CGameplayCatalog>& Get_ActiveGeneration() const noexcept
        {
            return m_Active;
        }

        // 숫자 snapshot의 기준 identity를 차용한다. 다음 commit 이후 값이 바뀔 수 있다.
        const LostArk::Shared::GameplayDataRevision& Get_NumericRevision() const noexcept
        {
            return m_Revision;
        }

    private:
        // catalog·snapshot·revision은 한 활성 숫자 세대를 구성한다. Prepared가 완성되기 전에는 이를 교체하지 않는다.
        std::shared_ptr<const CGameplayCatalog> m_Active;
        std::shared_ptr<const std::vector<LostArk::Shared::BALANCE_NUMERIC_ENTRY>> m_Entries;
        LostArk::Shared::GameplayDataRevision m_Revision{};

        // Initialize에서 해석한 설치/저작 위치와 그때의 source binding이다. 새 저장 요청의 기준으로 복사한다.
        std::filesystem::path m_BootstrapPath, m_AuthoringDataRoot;
        std::string m_SourceBindingsBytes;
    };
}
