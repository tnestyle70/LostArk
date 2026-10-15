#pragma once

#include "ServerCombatGeometry.h"
#include "Network/NetworkIds.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <map>
#include <vector>

namespace LostArk::Server
{
    struct SERVER_PLAYER;
    struct SERVER_COMBAT_OBJECT;
    class CGameplayCatalog;

    // 전술 선택을 위해 제한된 미래 접촉을 관측한다. 공격 시계/피격 ledger를 진행하거나 피해를 승인하지 않는다.

    class CColosseumThreatAssessment final
    {
    public:
        // risk는 예상 HP 피해량/확률이 아니라 접촉한 위협 가중치의 합이다. 첫 접촉 시각은 Observe 기준 초이며 미접촉 기본값은 1초다.
        struct SAMPLE
        {
            float risk = 0.f;
            float firstImpactSeconds = 1.f;
        };

        static constexpr float HorizonSeconds = .8f;
        static constexpr std::size_t MaximumThreats = 256u;

        // GameRoom 콜로세움 용병 판단이 현재 상대와 생성된 공격을 읽어 0.8초 관측을 다시 만든다.
        // 입력은 차용하고 예측 도형 값만 배열에 복사한다. 무효 참가자는 기존 관측을 비운 상태로 끝난다.
        void Observe(const SERVER_PLAYER& observer,
                     const std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
                     const std::vector<SERVER_COMBAT_OBJECT>& objects,
                     const CGameplayCatalog& catalog,
                     std::uint32_t tick);
        // 도착 예정 시각부터 150ms 동안 그 위치를 점유하는 위험을 조회한다. 도착 시각은 Observe 기준 초다.
        [[nodiscard]] SAMPLE At(float x, float y, float z, float arrivalSeconds = 0.f) const;
        // 선분 이동 뒤 종점을 잠시 점유하는 경로를 조회한다. 출발 시각은 마지막 Observe를 기준으로 한다.
        // 출발 지연으로 캐시된 관측의 경과 시간을 반영한다. 함수가 관측 배열 자체를 진행시키지는 않는다.
        // 종점 점유가 0이면 연속 경로 조각을 별도 대기 없이 평가할 수 있다.
        // 위협 한 개는 여러 경로 표본과 만나도 이 호출에서 한 번만 더한다.
        [[nodiscard]] float Along(float startX,
                                  float startY,
                                  float startZ,
                                  float endX,
                                  float endY,
                                  float endZ,
                                  float travelSeconds,
                                  float departureSeconds = 0.f,
                                  float endpointOccupationSeconds = .15f) const;

        // 현재 저장한 위협 수다. 관측되지 않은 공격이 없다는 보장은 아니며 최대 256개로 제한된다.
        [[nodiscard]] std::size_t ThreatCount() const
        {
            return m_Count;
        }

    private:
        // Observe 기준 초 단위 접촉/이동 구간과 월드 m 좌표·m/s 속도다. 관측 뒤 방향/속도는 고정한 근사 예측이다.
        struct THREAT
        {
            SERVER_COMBAT_SHAPE_XZ shape;
            float x = 0.f,
                y = 0.f,
                z = 0.f,
                forwardX = 0.f,
                forwardZ = 1.f;
            float velocityX = 0.f,
                velocityZ = 0.f;
            float moveStart = 0.f,
                moveEnd = HorizonSeconds;
            float begin = 0.f,
                end = 0.f,
                height = 1.8f,
                weight = 1.f;
            bool testHeight = true;
        };

        // 유효 구간을 보호 시간과 예측 한계로 자른다. 배열이 차면 시작이 가장 늦은 항목보다 빠른 위협만 교체한다.
        void Add(THREAT threat);
        // 경로와 위협의 겹친 시간 구간을 표본화한다. true일 때만 firstImpact를 쓰며 원형 sweep 값은 정확한 충돌시각이 아닌 구간 시작이다.
        [[nodiscard]] bool Intersects(const THREAT& threat,
                                      float from,
                                      float to,
                                      float startX,
                                      float startY,
                                      float startZ,
                                      float endX,
                                      float endY,
                                      float endZ,
                                      float travelSeconds,
                                      float departureSeconds,
                                      float& firstImpact) const;
        // 배열은 이 관측 객체가 소유한다. At/Along은 읽기 전용이며 Observe와의 동시 접근을 내부에서 동기화하지 않는다.
        std::array<THREAT, MaximumThreats> m_Threats{};
        std::size_t m_Count = 0u;
        float m_ProtectedUntil = 0.f;
    };
}
