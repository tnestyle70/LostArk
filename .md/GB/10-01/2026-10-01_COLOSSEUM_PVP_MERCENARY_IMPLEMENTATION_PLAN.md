# 콜로세움 PvP 매칭·용병 구현 계획

## G10. 2026-10-06 용병 상황 판단과 확률 스킬 선택

사용자의 현재 요청은 다섯 용병의 고정 스킬 순서와 시간 중심 난사를 없애고,
감지한 위험·거리·체력·공격 기회에 따라 생존과 공격을 선택하는 것이다. ALT_V는 일반 스킬
확률 선택에서 분리하며 기존 마지막 승인 이후 최소900tick과 원본 cooldown을 유지한다.
아래 G00 이후의 고정 순서 설명은 과거 계약이며 이번 G10/G11 적용본이 현재 정본이다.

WintersEngine의 실제 ChampionAISystem/ChampionAIBrain을 읽고 관측/판단 주기 분리,
생존 우선, 진행 중 native combo 보존과 태세 유지 원리를 참고한다. 그 저장소를 수정하거나
research-only InfluenceMap을 이미 연결된 전투 기능으로 간주하지 않는다.

### 변경 파일과 책임

- Server/Public/ColosseumThreatAssessment.h와 Server/Private/ColosseumThreatAssessment.cpp:
  실제 상대 caster hit/ground target/combo, native projectile와 room combat-object를 읽어
  향후 위험을 평가한다. 공용 ServerCombatGeometry와 실제 player body/높이를 사용한다.
  수명·이미 적용한 timed/contact hit·참가자/team/match guard를 반영하고 후보 끝점뿐 아니라
  이동 중의 경로와 도착 시각을 함께 평가한다. 피해나 이동 상태를 변경하지 않는 관측 helper다.
- Server/Public/GameRoom.h: 용병별 난수 상태, 최근 스킬 이력, 유지 표적과 전술·재평가 상태를
  경기 owner에 둔다. 사망/부활은 전술 상태를 정리하되 마지막 ALT_V 승인 시각을 초기화하지 않는다.
- Server/Private/GameRoom_Colosseum.cpp: 다섯 직업을 같은 decision 경로로 통일한다.
  쿨다운·자원·stance·범위·표적을 만족하는 일반 스킬에 상황 점수와 최근 반복 감점을 주어
  가중 무작위 선택한다. 진행 중 native COMBO 입력창은 매 fixed tick 유지한다.
  위험시 안전한 도보/SPACE 후보를 비교하고, 체력·주변 수적 상황과 아군 위치에 따라
  후퇴/거리 유지/공격 표적을 판단한다. 표적·태세 유지 시간으로 잦은 왕복을 막는다.
  실제 명령은 기존 Execute_PlayerMove/Execute_PlayerSkill로 제출하며 pending 등록을
  즉시 시전 성공으로 기록하지 않는다. 피해·CC·스킬 cancel·cooldown 권위는 기존 executor다.
- Server/Private/ServerGameplayContractTests_ColosseumMatch.cpp: 폐기한 ordered-rotation
  assertion을 새 정책에 맞춰 교체하고 다양성·재현성·위험 회피·낮은HP·표적 유지와
  기존 manual COMBO/stance/ALT_V900tick·tick wrap·phase 경계를 함께 검증한다.
- Server/Default/Server.vcxproj와 .filters에 새 H/CPP를 실제 물리 폴더 아래 등록한다.
  필요한 Build domain 분류도 실제 manifest 규칙을 확인한다. Client/Shared packet/schema와
  저작/게시 gameplay 데이터는 이 변경에서 추가하거나 교체하지 않는다.

### 검증과 인도

실제 Server Colosseum match/combat 및 skill-stages 계약검사, 새 위협 helper의 caster/ground/
lingering projectile·경로 위험과 phase/팀 격리 검사를 실행한다. Debug/Release 필요한 제품
빌드와 project/filter XML parse·인코딩·diff 검사를 진행하고 결과만 RESULT에 기록한다.
Client/UI를 자율 실행하지 않으며 실제 전투 체감은 사용자 확인으로 남긴다. 열린 Server/Client가
링크를 점유하면 종료하지 않고 후보 빌드/검증을 먼저 완료한다. 현재 경기에는 재시작 없이
소스 변경이 적용됐다고 설명하지 않는다. 최종 전체 H/CPP는 G11에 현재 파일과 함께 보존한다.


## G11. 현재 반영 H/CPP 전체 코드

G10의 선언과 흐름을 구현한 전체 파일이다. 기존 경기 admission·phase·native executor 계약도 함께 보존한다.
아래 코드는 현재 소스와 LF 정규화 후 일치하는지 검증한다.

### Server/Public/ColosseumThreatAssessment.h

```cpp
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

    // A bounded, immutable observation for tactical choices. This predicts contact;
    // it never advances an action, consumes a hit ledger, or authorizes damage.
    class CColosseumThreatAssessment final
    {
    public:
        struct SAMPLE { float risk = 0.f; float firstImpactSeconds = 1.f; };
        static constexpr float HorizonSeconds = .8f;
        static constexpr std::size_t MaximumThreats = 256u;

        void Observe(const SERVER_PLAYER& observer,
            const std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
            const std::vector<SERVER_COMBAT_OBJECT>& objects,
            const CGameplayCatalog& catalog, std::uint32_t tick);
        // Query a 150 ms occupation window beginning at the proposed arrival.
        [[nodiscard]] SAMPLE At(float x, float y, float z, float arrivalSeconds = 0.f) const;
        // Follow the segment at its travel speed, then occupy its endpoint briefly.
        // Times are relative to Observe; departure advances a cached observation.
        // A zero endpoint occupation joins consecutive motion segments without pauses.
        // Each predicted hit contributes once, even when several path samples meet it.
        [[nodiscard]] float Along(float startX, float startY, float startZ,
            float endX, float endY, float endZ, float travelSeconds,
            float departureSeconds = 0.f, float endpointOccupationSeconds = .15f) const;
        [[nodiscard]] std::size_t ThreatCount() const { return m_Count; }

    private:
        struct THREAT
        {
            SERVER_COMBAT_SHAPE_XZ shape;
            float x = 0.f, y = 0.f, z = 0.f, forwardX = 0.f, forwardZ = 1.f;
            float velocityX = 0.f, velocityZ = 0.f;
            float moveStart = 0.f, moveEnd = HorizonSeconds;
            float begin = 0.f, end = 0.f, height = 1.8f, weight = 1.f;
            bool testHeight = true;
        };
        void Add(THREAT threat);
        [[nodiscard]] bool Intersects(const THREAT& threat, float from, float to,
            float startX, float startY, float startZ, float endX, float endY, float endZ,
            float travelSeconds, float departureSeconds, float& firstImpact) const;
        std::array<THREAT, MaximumThreats> m_Threats{};
        std::size_t m_Count = 0u;
        float m_ProtectedUntil = 0.f;
    };
}
```

### Server/Private/ColosseumThreatAssessment.cpp

```cpp
#include "ColosseumThreatAssessment.h"
#include "ColosseumCombatPolicy.h"
#include "CombatObjectRuntime.h"
#include "Gameplay/WorldCollisionContract.h"

#include <algorithm>
#include <cmath>

using namespace LostArk::Server;
using namespace LostArk::Shared;

namespace
{
    constexpr float TickSeconds = 1.f / 30.f;
    constexpr float OccupationSeconds = .15f;
    constexpr float Pi = 3.14159265358979323846f;

    bool HealthChannel(std::uint32_t kind) { return kind == 0u || kind == 1u; }

    SERVER_COMBAT_SHAPE_XZ ShapeOf(const PLAYER_SKILL_HIT& hit)
    {
        SERVER_COMBAT_SHAPE_XZ shape;
        shape.fOffset = hit.fOffset;
        if (hit.iAreaType == 1u)
        {
            shape.eKind = hit.fInner > 0.f ? SERVER_COMBAT_SHAPE_KIND::RING : SERVER_COMBAT_SHAPE_KIND::CIRCLE;
            shape.fInnerRadius = hit.fInner; shape.fOuterRadius = hit.fRange;
        }
        else if (hit.iAreaType == 2u)
        {
            shape.eKind = SERVER_COMBAT_SHAPE_KIND::FORWARD_BOX;
            shape.fLength = hit.fRange; shape.fHalfWidth = hit.fWidth * .5f;
        }
        else if (hit.iAreaType == 3u)
        {
            shape.eKind = SERVER_COMBAT_SHAPE_KIND::CONE;
            shape.fLength = hit.fRange; shape.fInnerRadius = hit.fInner;
            shape.fAngleDegrees = hit.fAngleDegrees <= 0.f ? 360.f : (std::min)(360.f, hit.fAngleDegrees);
        }
        return shape;
    }

    float RemainingProtection(std::uint32_t end, std::uint32_t tick)
    {
        const auto remaining = static_cast<std::int32_t>(end - tick);
        return end && remaining > 0 ? float(remaining) * TickSeconds : 0.f;
    }
}

void CColosseumThreatAssessment::Add(THREAT threat)
{
    if (!CServerCombatGeometry::Is_Valid(threat.shape) ||
        !std::isfinite(threat.x) || !std::isfinite(threat.y) || !std::isfinite(threat.z) ||
        !std::isfinite(threat.forwardX) || !std::isfinite(threat.forwardZ) ||
        !std::isfinite(threat.velocityX) || !std::isfinite(threat.velocityZ) ||
        !std::isfinite(threat.begin) || !std::isfinite(threat.end) ||
        !std::isfinite(threat.moveStart) || !std::isfinite(threat.moveEnd) ||
        !std::isfinite(threat.height) || threat.height <= 0.f ||
        threat.end < 0.f || threat.begin > HorizonSeconds) return;
    threat.begin = (std::max)({0.f, threat.begin, m_ProtectedUntil});
    threat.end = (std::min)(HorizonSeconds, threat.end);
    if (threat.end < threat.begin) return;
    if (m_Count < MaximumThreats) m_Threats[m_Count++] = threat;
    else
    {
        // A busy emitter must not displace a later-observed imminent enemy hit.
        auto latest = std::max_element(m_Threats.begin(), m_Threats.end(),
            [](const auto& a, const auto& b) { return a.begin < b.begin; });
        if (threat.begin < latest->begin) *latest = threat;
    }
}

void CColosseumThreatAssessment::Observe(const SERVER_PLAYER& observer,
    const std::map<PLAYER_ID, SERVER_PLAYER>& players,
    const std::vector<SERVER_COMBAT_OBJECT>& objects,
    const CGameplayCatalog& catalog, const std::uint32_t tick)
{
    m_Count = 0u;
    m_ProtectedUntil = (std::max)(RemainingProtection(observer.iInvulnerableEndTick, tick),
        RemainingProtection(observer.iTimeStopEndTick, tick));
    const SERVER_COLOSSEUM_COMBAT_CONTEXT context{WORLD_ID::COLOSSEUM,
        observer.iColosseumMatchId, observer.bColosseumCombatActive, {}};
    if (!Is_ColosseumCombatParticipant(context, observer)) return;
    const auto opponent = [&](const SERVER_PLAYER& source) {
        return source.bColosseumCombatActive && Is_ColosseumOpponent(context, source, observer);
    };
    const auto make = [](const PLAYER_SKILL_HIT& hit, float x, float y, float z, float dx, float dz) {
        THREAT result;
        result.shape = ShapeOf(hit); result.x = x; result.y = y; result.z = z;
        result.forwardX = dx; result.forwardZ = dz;
        result.height = hit.fHeight > 0.f ? hit.fHeight : WorldCollision::PLAYER_HALF_EXTENT_Y * 2.f;
        result.weight = hit.iPushMs && hit.fPushRange != 0.f ? 1.35f : 1.f;
        return result;
    };
    for (const auto& [id, source] : players)
    {
        if (!opponent(source)) continue; // Dead/falling casters cannot keep PvP projectiles authoritative.
        // GameRoom skips the native player/action/projectile update during time stop.
        // Room-owned CombatObjects below have their own clock and are not delayed.
        const float sourceDelay = RemainingProtection(source.iTimeStopEndTick, tick);
        const auto addNative = [&](THREAT threat) {
            threat.begin = (std::max)(0.f, threat.begin) + sourceDelay;
            threat.end += sourceDelay;
            threat.moveStart += sourceDelay; threat.moveEnd += sourceDelay;
            Add(threat);
        };
        const auto* skill = source.eAction == PLAYER_ACTION_STATE::SKILL ? catalog.Find_Skill(source.iCurrentSkillId) : nullptr;
        const bool activeDamage = skill && !skill->strDamageProfileId.empty() &&
            (skill->eSkillKind != PLAYER_SKILL_KIND::HOLD || source.iComboStage == 3u) &&
            (skill->eSkillKind != PLAYER_SKILL_KIND::COUNTER || source.iComboStage == 2u);
        if (activeDamage)
        {
            const auto stageIndex = source.iComboStage ? source.iComboStage - 1u : 0u;
            const bool staged = skill->eSkillKind == PLAYER_SKILL_KIND::COMBO ||
                skill->eSkillKind == PLAYER_SKILL_KIND::HOLD || skill->eSkillKind == PLAYER_SKILL_KIND::COUNTER;
            const auto* stage = staged && stageIndex < skill->ComboStages.size() ? &skill->ComboStages[stageIndex] : nullptr;
            const auto& hits = stage ? stage->Hits : skill->Hits;
            const auto& pendingProjectiles = stage ? stage->Projectiles : skill->Projectiles;
            const float x = source.hasSkillTarget ? source.fSkillTargetX : source.fPositionX;
            const float y = source.hasSkillTarget ? source.fSkillTargetY : source.fPositionY;
            const float z = source.hasSkillTarget ? source.fSkillTargetZ : source.fPositionZ;
            std::size_t window = 0u;
            for (const auto& hit : hits)
                for (std::uint32_t repeat = 0u; repeat < hit.iRepeatCount && window < source.iAppliedHitMask.size(); ++repeat, ++window)
                {
                    if (!HealthChannel(hit.iResultKind)) continue;
                    const auto alreadyHit = std::count_if(source.HitWindowTargets.begin(), source.HitWindowTargets.end(),
                        [&](const auto& mark) { return mark.first == window; });
                    if (!source.iAppliedHitMask.none() &&
                        (std::any_of(source.HitWindowTargets.begin(), source.HitWindowTargets.end(),
                            [&](const auto& mark) { return mark.first == window && mark.second == observer.iNetEntityId; }) ||
                         (hit.iMaxTargets && alreadyHit >= hit.iMaxTargets))) continue;
                    auto threat = make(hit, x, y, z, source.fSkillAimDirectionX, source.fSkillAimDirectionZ);
                    const float due = (float(hit.iTimeMs) + float(hit.iRepeatMs) * float(repeat)) * .001f - source.fActionElapsedSeconds;
                    threat.begin = due;
                    threat.end = due + float(hit.iDurationMs) * .001f;
                    // A never-sampled instantaneous/overdue hit still fires on the next fixed tick.
                    if (!source.iAppliedHitMask.test(window)) threat.end = (std::max)(TickSeconds, threat.end);
                    addNative(threat);
                }
            if (hits.empty() && pendingProjectiles.empty() && !source.hasAppliedSkillDamage)
            {
                PLAYER_SKILL_HIT fallback;
                fallback.iAreaType = 1u; fallback.fRange = skill->fMaximumRange;
                auto threat = make(fallback, x, y, z, source.fSkillAimDirectionX, source.fSkillAimDirectionZ);
                threat.begin = (float(stage ? stage->iHitTimeMs : skill->iHitTimeMs) * .001f) - source.fActionElapsedSeconds;
                threat.end = (std::max)(0.f, threat.begin) + TickSeconds;
                addNative(threat);
            }
        }
        // Native missiles and persistent areas continue after the originating action finishes.
        for (const auto& projectile : source.Projectiles)
        {
            const auto* owner = catalog.Find_Skill(projectile.iSkillId);
            if (!owner || projectile.fRemainingSeconds <= 0.f) continue;
            const auto& definitions = projectile.iStageIndex < owner->ComboStages.size() &&
                !owner->ComboStages[projectile.iStageIndex].Projectiles.empty() ?
                owner->ComboStages[projectile.iStageIndex].Projectiles : owner->Projectiles;
            if (projectile.iProjectileIndex >= definitions.size()) continue;
            const float speed = (std::max)(0.f, projectile.fSpeed);
            const float motionEnd = speed > 0.f && projectile.fRemainingDistance >= 0.f ?
                projectile.fRemainingDistance / speed : HorizonSeconds;
            const float life = (std::min)({HorizonSeconds, projectile.fRemainingSeconds,
                speed > 0.f ? motionEnd : HorizonSeconds});
            std::size_t timedIndex = 0u, hitIndex = 0u;
            for (const auto& hit : definitions[projectile.iProjectileIndex].Hits)
            {
                auto threat = make(hit.Hit, projectile.fPositionX, projectile.fPositionY, projectile.fPositionZ,
                    projectile.fDirectionX, projectile.fDirectionZ);
                threat.velocityX = projectile.fDirectionX * speed; threat.velocityZ = projectile.fDirectionZ * speed;
                threat.moveEnd = motionEnd;
                if (hit.isContact)
                {
                    const auto mark = std::find_if(projectile.ContactMarks.begin(), projectile.ContactMarks.end(),
                        [&](const auto& value) { return value.iHitIndex == hitIndex && value.iNetEntityId == observer.iNetEntityId; });
                    const auto count = std::count_if(projectile.ContactMarks.begin(), projectile.ContactMarks.end(),
                        [&](const auto& value) { return value.iHitIndex == hitIndex; });
                    if (HealthChannel(hit.Hit.iResultKind) && hit.Hit.iRepeatCount &&
                        (mark != projectile.ContactMarks.end() ? mark->iAppliedCount < hit.Hit.iRepeatCount :
                         !hit.Hit.iMaxTargets || count < hit.Hit.iMaxTargets))
                    {
                        threat.begin = mark == projectile.ContactMarks.end() ? 0.f : mark->fNextSeconds - projectile.fElapsedSeconds;
                        threat.end = life;
                        addNative(threat);
                    }
                }
                else for (std::uint32_t repeat = 0u; repeat < hit.Hit.iRepeatCount &&
                    timedIndex < projectile.iAppliedTimedMask.size(); ++repeat, ++timedIndex)
                {
                    if (!HealthChannel(hit.Hit.iResultKind) || projectile.iAppliedTimedMask.test(timedIndex)) continue;
                    threat.begin = (std::max)(0.f, (float(hit.Hit.iTimeMs) + float(hit.Hit.iRepeatMs) * float(repeat)) *
                        .001f - projectile.fElapsedSeconds);
                    threat.end = (std::min)(life, threat.begin + TickSeconds);
                    addNative(threat);
                }
                ++hitIndex;
            }
        }
    }
    for (const auto& object : objects)
    {
        if (object.eSourceKind != SERVER_COMBAT_OBJECT_SOURCE_KIND::PLAYER) continue;
        const auto source = std::find_if(players.begin(), players.end(), [&](const auto& value) {
            return value.second.iNetEntityId == object.iSourceNetEntityId;
        });
        if (source == players.end() || !opponent(source->second) || !catalog.Find_Skill(object.iSourceSkillId)) continue;
        const float speed = (std::max)(0.f, object.fSpeedMps);
        const float moveStart = (std::max)(0.f, (float(object.iMovementStartDelayMs) - object.fElapsedMilliseconds) * .001f);
        const float moveEnd = speed > 0.f && object.fRemainingDistanceM >= 0.f && !object.bPersistentLifetime ?
            moveStart + object.fRemainingDistanceM / speed : HorizonSeconds;
        float life = object.bPersistentLifetime ? HorizonSeconds : object.fRemainingMilliseconds * .001f;
        if (object.bExpireOnDistanceEnd && speed > 0.f) life = (std::min)(life, moveEnd);
        if (life <= 0.f) continue;
        std::size_t hitIndex = 0u;
        for (const auto& hit : object.Hits)
        {
            const auto index = hitIndex++;
            if (!HealthChannel(hit.iPlayerResultKind) || hit.RepeatRawDamage.empty()) continue;
            const auto& pose = object.LiveState.CurrentPose;
            const float angle = (pose.fYawDegrees + hit.fYawOffsetDegrees) * Pi / 180.f;
            THREAT threat;
            threat.shape = hit.Shape;
            threat.x = pose.fPositionX + pose.fDirectionX * hit.fOffsetForwardM + pose.fDirectionZ * hit.fOffsetRightM;
            threat.z = pose.fPositionZ + pose.fDirectionZ * hit.fOffsetForwardM - pose.fDirectionX * hit.fOffsetRightM;
            threat.y = pose.fPositionY; threat.forwardX = std::sin(angle); threat.forwardZ = std::cos(angle);
            threat.velocityX = pose.fDirectionX * speed; threat.velocityZ = pose.fDirectionZ * speed;
            threat.moveStart = moveStart; threat.moveEnd = moveEnd;
            // The existing CombatObject PvP adapter tests XZ only; do not invent a Y gate.
            threat.testHeight = false;
            threat.weight = hit.iPushMs && hit.fPushRangeM != 0.f ? 1.35f : 1.f;
            if (hit.eTrigger == SERVER_COMBAT_OBJECT_HIT_TRIGGER::CONTACT || object.bDetonateOnContact)
            {
                const auto mark = std::find_if(object.ContactMarks.begin(), object.ContactMarks.end(), [&](const auto& value) {
                    return value.iHitIndex == index && value.iTargetNetEntityId == observer.iNetEntityId;
                });
                const auto count = std::count_if(object.ContactMarks.begin(), object.ContactMarks.end(),
                    [&](const auto& value) { return value.iHitIndex == index; });
                if ((mark != object.ContactMarks.end() && mark->iAppliedCount >= hit.RepeatRawDamage.size()) ||
                    (mark == object.ContactMarks.end() && hit.Shape.iMaximumTargets && count >= hit.Shape.iMaximumTargets)) continue;
                threat.begin = (std::max)(speed > 0.f ? moveStart : 0.f,
                    (float(hit.iAtMs) - object.fElapsedMilliseconds) * .001f);
                if (mark != object.ContactMarks.end()) threat.begin = (std::max)(threat.begin,
                    (mark->fNextMilliseconds - object.fElapsedMilliseconds) * .001f);
                threat.end = hit.iEndMs ? (std::min)(life, (float(hit.iEndMs) - object.fElapsedMilliseconds) * .001f) : life;
                Add(threat);
            }
            else for (std::size_t repeat = hit.iAppliedTimedCount; repeat < hit.RepeatRawDamage.size() &&
                repeat - hit.iAppliedTimedCount < MaximumThreats; ++repeat)
            {
                if (!object.OwnerHitChain.strTriggerActionId.empty())
                {
                    if (!object.bOwnerHitChainArmed) break;
                    threat.begin = (float(object.iOwnerHitChainDelayMs) + float(hit.iAtMs)) * .001f -
                        float(tick - object.iOwnerHitChainArmedTick) * TickSeconds;
                }
                else threat.begin = (float(hit.iAtMs) + float(hit.iRepeatIntervalMs) * float(repeat) - object.fElapsedMilliseconds) * .001f;
                threat.begin = (std::max)(0.f, threat.begin);
                threat.end = (std::min)(life, threat.begin + TickSeconds);
                Add(threat);
            }
        }
    }
}

bool CColosseumThreatAssessment::Intersects(const THREAT& threat, float from, float to,
    float startX, float startY, float startZ, float endX, float endY, float endZ,
    float travelSeconds, float departureSeconds, float& firstImpact) const
{
    from = (std::max)(from, threat.begin); to = (std::min)(to, threat.end);
    if (to < from) return false;
    const auto pose = [&](float time, float& x, float& y, float& z, float& ox, float& oz) {
        const float alpha = travelSeconds > 0.f ? (std::clamp)((time - departureSeconds) / travelSeconds, 0.f, 1.f) : 1.f;
        x = startX + (endX - startX) * alpha; y = startY + (endY - startY) * alpha; z = startZ + (endZ - startZ) * alpha;
        const float moving = (std::max)(0.f, (std::min)(time, threat.moveEnd) - threat.moveStart);
        ox = threat.x + threat.velocityX * moving; oz = threat.z + threat.velocityZ * moving;
    };
    const float targetSpeed = travelSeconds > 0.f ? std::hypot(endX-startX, endZ-startZ) / travelSeconds : 0.f;
    const float speed = targetSpeed + std::hypot(threat.velocityX, threat.velocityZ);
    const unsigned samples = static_cast<unsigned>((std::clamp)(std::ceil((to-from) *
        (std::max)(60.f, speed / .2f)), 1.f, 128.f));
    float previousX = 0.f, previousZ = 0.f, previousY = 0.f, previousOx = 0.f, previousOz = 0.f;
    float previousTime = from;
    for (unsigned sample = 0u; sample <= samples; ++sample)
    {
        const float time = from + (to-from) * float(sample) / float(samples);
        float x, y, z, ox, oz;
        pose(time, x, y, z, ox, oz);
        const float bottom = y + WorldCollision::PLAYER_CENTER_OFFSET_Y - WorldCollision::PLAYER_HALF_EXTENT_Y;
        const float top = bottom + WorldCollision::PLAYER_HALF_EXTENT_Y * 2.f;
        const bool height = !threat.testHeight || (top >= threat.y && bottom <= threat.y + threat.height);
        if (height && CServerCombatGeometry::Overlaps_Pose(threat.shape, ox, oz, threat.forwardX, threat.forwardZ,
            {x,z,WorldCollision::PLAYER_HALF_EXTENT_X}))
        { firstImpact = time; return true; }
        // Relative swept contact catches a fast circle crossing between fixed samples,
        // including a moving observer crossing an otherwise stationary area.
        if (sample && height && threat.shape.eKind == SERVER_COMBAT_SHAPE_KIND::CIRCLE &&
            (!threat.testHeight || (previousY + WorldCollision::PLAYER_CENTER_OFFSET_Y + WorldCollision::PLAYER_HALF_EXTENT_Y >= threat.y &&
                previousY + WorldCollision::PLAYER_CENTER_OFFSET_Y - WorldCollision::PLAYER_HALF_EXTENT_Y <= threat.y + threat.height)) &&
            CServerCombatGeometry::SweptCircle_Overlaps(
                previousOx + threat.forwardX * threat.shape.fOffset - previousX,
                previousOz + threat.forwardZ * threat.shape.fOffset - previousZ,
                ox + threat.forwardX * threat.shape.fOffset - x,
                oz + threat.forwardZ * threat.shape.fOffset - z,
                threat.shape.fOuterRadius, {0.f,0.f,WorldCollision::PLAYER_HALF_EXTENT_X}))
        { firstImpact = previousTime; return true; }
        previousX=x; previousY=y; previousZ=z; previousOx=ox; previousOz=oz; previousTime=time;
    }
    return false;
}

CColosseumThreatAssessment::SAMPLE CColosseumThreatAssessment::At(float x, float y, float z, float arrivalSeconds) const
{
    SAMPLE result;
    if (!std::isfinite(x) || !std::isfinite(y) || !std::isfinite(z) || !std::isfinite(arrivalSeconds)) return result;
    const float begin = (std::max)(0.f,arrivalSeconds), end = (std::min)(HorizonSeconds,begin+OccupationSeconds);
    for (std::size_t i=0u; i<m_Count; ++i)
    {
        float impact = 1.f;
        if (Intersects(m_Threats[i],begin,end,x,y,z,x,y,z,0.f,0.f,impact))
        { result.risk += m_Threats[i].weight; result.firstImpactSeconds = (std::min)(result.firstImpactSeconds,impact); }
    }
    return result;
}

float CColosseumThreatAssessment::Along(float startX, float startY, float startZ,
    float endX, float endY, float endZ, float travelSeconds,
    float departureSeconds, float endpointOccupationSeconds) const
{
    if (!std::isfinite(startX) || !std::isfinite(startY) || !std::isfinite(startZ) ||
        !std::isfinite(endX) || !std::isfinite(endY) || !std::isfinite(endZ) ||
        !std::isfinite(travelSeconds) || travelSeconds < 0.f ||
        !std::isfinite(departureSeconds) || departureSeconds < 0.f ||
        !std::isfinite(endpointOccupationSeconds) || endpointOccupationSeconds < 0.f) return 0.f;
    const float end = (std::min)(HorizonSeconds,departureSeconds+travelSeconds+endpointOccupationSeconds);
    float risk = 0.f;
    for (std::size_t i=0u; i<m_Count; ++i)
    {
        float impact = 1.f;
        if (Intersects(m_Threats[i],departureSeconds,end,startX,startY,startZ,endX,endY,endZ,travelSeconds,departureSeconds,impact)) risk += m_Threats[i].weight;
    }
    return risk;
}
```

### Server/Public/GameRoom.h

```cpp
#pragma once

#include "RoomCommand.h"
#include <atomic>
#include "ServerPlayer.h"
#include "ServerWorldEntity.h"
#include "WorldBootstrap.h"
#include "GameplayCatalog.h"
#include "ItemCatalog.h"
#include "HonorTitleCatalog.h"
#include "VehicleCatalog.h"
#include "GuideCatalog.h"
#include "ValtanClearRewards.h"
#include "PlayerSkillSystem.h"
#include "CombatObjectRuntime.h"
#include "ColosseumThreatAssessment.h"
#include "ServerNavigation.h"
#include "ServerCollisionSystem.h"
#include "ServerTriggerSystem.h"
#include "SpawnGroupBootstrap.h"
#include "SpawnGroupRuntime.h"
#include "MonsterBrain.h"
#include "NpcBehaviorRuntime.h"
#include "ValtanBrain.h"
#include "KoukuSaydonBrain.h"
#include "KoukuSaydonLogicRuntime.h"
#include "EncounterPropRuntime.h"
#include "EstherSkillSystem.h"
#include "Gameplay/EstherStrikeContract.h"
#include "Gameplay/MaharakaWaterpangContract.h"
#include "WorldDestructionBootstrap.h"
#include "WorldDestructionRuntime.h"
#include "Network/PacketFrame.h"
#include "Network/SessionDiagnostic.h"

#include <cstddef>
#include <cstdint>
#include <deque>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <random>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace LostArk::Server
{
	class CServerGameplayContractRunner;

	class CClientSession;

	/* A room never mutates an admitted gameplay catalog. The facade preserves
	   the established lookup surface while retaining immutable old generations
	   until every replicated occurrence releases its revision pin. */
	class CGameplayCatalogGenerations final
	{
	public:
		static constexpr std::size_t MAX_GENERATION_COUNT = 16u;

		CGameplayCatalogGenerations();
		bool Load();
		bool Initialize(
			const std::shared_ptr<const CGameplayCatalog>& initialGeneration);
		bool Stage(
			std::uint32_t transactionSequence,
			const LostArk::Shared::GameplayDataRevision& baseRevision,
			const std::shared_ptr<const CGameplayCatalog>& candidateGeneration,
			std::string& status);
		bool Commit(std::uint32_t transactionSequence) noexcept;
		bool Stage_NumericBalance(std::uint32_t transactionSequence,
			const std::shared_ptr<const CGameplayCatalog>& candidate, std::string& status);
		bool Commit_NumericBalance(std::uint32_t transactionSequence) noexcept;
		void Abort_NumericBalance(std::uint32_t transactionSequence) noexcept;

		void Abort(std::uint32_t transactionSequence) noexcept;
		void Collect_Garbage(
			const std::vector<LostArk::Shared::GameplayDataRevision>& livePins);

		[[nodiscard]] const CGameplayCatalog* Resolve(
			const LostArk::Shared::GameplayDataRevision& revision) const noexcept;
		[[nodiscard]] const CGameplayCatalog& Active() const noexcept;
		[[nodiscard]] std::shared_ptr<const CGameplayCatalog>
			Get_ActiveGeneration() const noexcept { return m_pActiveGeneration; }
		[[nodiscard]] std::size_t Get_GenerationCount() const noexcept
		{
			return m_Generations.size();
		}
		[[nodiscard]] std::uint16_t Get_ActiveGenerationEpoch() const noexcept
		{
			return m_iActiveGenerationEpoch;
		}

		const PLAYER_SKILL_DEFINITION* Find_Skill(
			LostArk::Shared::SKILL_ID skillId) const;
		const BOSS_RUNTIME_PROFILE* Find_Boss(
			const std::string& archetypeId) const;
		const std::vector<BOSS_PART_DEFINITION>* Find_BossParts(
			const std::string& archetypeId) const;
		const std::vector<BOSS_PATTERN_DEFINITION>* Find_BossPatterns(
			const std::string& encounterId) const;
		const BOSS_COMBAT_OBJECT_DEFINITION* Find_BossCombatObject(
			const std::string& archetypeId) const;
		const VALTAN_TIMELINE_DEFINITION* Find_ValtanTimeline(
			const std::string& encounterId) const;
		const VALTAN_TIMELINE_ROW* Find_ValtanTimelineRow(
			const std::string& encounterId, std::uint32_t commandId) const;
		const BOSS_PATTERN_ROTATION_DEFINITION* Find_BossPatternRotation(
			const std::string& encounterId, std::uint32_t gameplayPhase,
			std::uint32_t healthBar) const;
		const std::string& Find_IntroPatternId(
			const std::string& encounterId) const;
		const PLAYER_RUNTIME_PROFILE* Find_Player(
			LostArk::Shared::CHARACTER_CLASS_ID characterClass) const;
		std::uint32_t Find_DamageRatePercent(
			const std::string& damageProfileId) const;
		[[nodiscard]] const LostArk::Shared::GameplayDataRevision&
			Get_ActiveRevision() const noexcept;
		[[nodiscard]] const std::string& Get_Status() const noexcept;
		operator const CGameplayCatalog&() const noexcept { return Active(); }

	private:
		std::shared_ptr<const CGameplayCatalog> m_pActiveGeneration;
		std::shared_ptr<const CGameplayCatalog> m_pStagedGeneration;
		std::uint32_t m_iStagedTransactionSequence = 0u;
		std::uint16_t m_iActiveGenerationEpoch = 0u;
		std::vector<std::shared_ptr<const CGameplayCatalog>> m_Generations;
		std::vector<std::shared_ptr<const CGameplayCatalog>> m_NumericStagedGenerations;
		std::shared_ptr<const CGameplayCatalog> m_pNumericStagedActive;
		std::uint32_t m_iNumericTransactionSequence = 0u;

		std::string m_strStatus;
	};

	// The last completed outer room-loop iteration, shared by all rooms on the next tick.
	struct SERVER_ROOM_SCHEDULER_METRICS final
	{
		std::uint64_t iSampleUnixMilliseconds = 0u;
		std::uint64_t iPreviousLoopLatenessMicroseconds = 0u;
		std::uint64_t iMaximumLoopLatenessMicroseconds = 0u;
		std::uint64_t iScheduleResetCount = 0u;
	};

	struct SERVER_ROOM_PERFORMANCE_METRICS final
	{
		SERVER_NAVIGATION_PERFORMANCE_METRICS Navigation;
		SERVER_ROOM_SCHEDULER_METRICS Scheduler;
		std::uint64_t iTickCount = 0;
		std::uint64_t iLastTickMicroseconds = 0;
		std::uint64_t iMaximumTickMicroseconds = 0;
		std::size_t iLastIngressDepth = 0;
		std::size_t iIngressHighWatermark = 0;
		std::size_t iLastDrainedCommandCount = 0;
		std::size_t iLastRemainingCommandCount = 0;
		std::size_t iLastCleanupIngressDepth = 0;
		std::size_t iCleanupIngressHighWatermark = 0;
		std::size_t iLastDrainedCleanupCommandCount = 0;
		std::size_t iLastRemainingCleanupCommandCount = 0;
		std::uint64_t iDrainLimitedTickCount = 0;
		std::uint64_t iCoalescedMoveCommandCount = 0;
		std::uint64_t iCoalescedAimCommandCount = 0;
		std::uint64_t iDroppedBestEffortCommandCount = 0;
		std::uint64_t iRejectedReliableCommandCount = 0;
		std::uint64_t iRejectedCleanupCommandCount = 0;
		std::uint64_t iDeduplicatedCleanupCommandCount = 0;
		std::uint64_t iCancelledCommandCountByCleanup = 0;
		std::uint64_t iSnapshotEncodeCount = 0;
		std::uint64_t iSnapshotEncodeFailureCount = 0;
		std::uint64_t iLastSnapshotEncodeMicroseconds = 0;
		std::uint64_t iMaximumSnapshotEncodeMicroseconds = 0;
		std::uint64_t iSnapshotEnqueueBatchCount = 0;
		std::uint64_t iSnapshotRecipientCount = 0;
		std::uint64_t iSnapshotEnqueueFailureCount = 0;
		std::uint64_t iLastSnapshotEnqueueMicroseconds = 0;
		std::uint64_t iMaximumSnapshotEnqueueMicroseconds = 0;
		std::uint64_t iLastMaximumSessionEnqueueMicroseconds = 0;
		std::uint64_t iMaximumSessionEnqueueMicroseconds = 0;
	};

	/* Receive-thread admission is not a bool: a full reliable queue, a room
	   runtime failure, a sealed private arena, and cleanup already in flight
	   require different session policy and diagnostics.  Keep success values
	   explicit too so best-effort shedding remains non-terminal. */
	enum class ROOM_COMMAND_ENQUEUE_RESULT : std::uint8_t
	{
		ACCEPTED,
		DROPPED_BEST_EFFORT,
		DEDUPLICATED_CLEANUP,
		REJECTED_INVALID_COMMAND,
		REJECTED_ROOM_NOT_READY,
		REJECTED_ROOM_SEALED,
		REJECTED_PENDING_CLEANUP,
		REJECTED_RELIABLE_CAPACITY,
		REJECTED_BINDING_MISSING
	};

	[[nodiscard]] constexpr bool Is_AcceptedRoomCommandEnqueueResult(
		const ROOM_COMMAND_ENQUEUE_RESULT result) noexcept
	{
		return ROOM_COMMAND_ENQUEUE_RESULT::ACCEPTED == result ||
			ROOM_COMMAND_ENQUEUE_RESULT::DROPPED_BEST_EFFORT == result ||
			ROOM_COMMAND_ENQUEUE_RESULT::DEDUPLICATED_CLEANUP == result;
	}

	struct SERVER_ROOM_RUNTIME_FAILURE final
	{
		std::uint32_t iServerTick = 0u;
		std::string strSource;
		std::string strDetail;
	};

	class CGameRoom final
	{
		friend class CServerGameplayContractRunner;
		friend int Run_ServerKoukuSupportSurfaceContractTests();
        friend int Run_ServerBingoContractTests();
		friend int Run_ServerCardMazeContractTests();
        friend int Run_ServerKoukuObjectOverlapContractTests();
		friend int Run_ServerVehicleRidingContractTests();
	public:
		explicit CGameRoom(
			LostArk::Shared::WORLD_ID worldId,
			std::shared_ptr<const CGameplayCatalog> initialGameplayGeneration = {},
			const std::atomic_bool* pPreparationCancelled = nullptr);

		bool Enqueue(ROOM_COMMAND command);
		[[nodiscard]] ROOM_COMMAND_ENQUEUE_RESULT Enqueue_Detailed(
			ROOM_COMMAND command);
		[[nodiscard]] std::string Describe_EnqueueResult(
			ROOM_COMMAND_ENQUEUE_RESULT result) const;
		[[nodiscard]] bool Try_GetRuntimeFailure(
			SERVER_ROOM_RUNTIME_FAILURE& outFailure) const;
		void Tick(float fixedDeltaSeconds,
			const SERVER_ROOM_SCHEDULER_METRICS& schedulerMetrics = {});
		// Room-thread only. At most one sampled line is retained until ServerApp writes it.
		[[nodiscard]] std::string Take_PerformanceDiagnostic();
		bool Try_DequeueWorldTransfer(
			SERVER_WORLD_TRANSFER_REQUEST& outTransfer);
		bool Configure_ColosseumMatch(std::uint64_t matchId, const std::vector<SESSION_ID>& sessions);
		void Remove_ColosseumExpectedSession(SESSION_ID sessionId);

		[[nodiscard]] LostArk::Shared::WORLD_ID Get_WorldId() const
		{
			return m_eWorldId;
		}

		[[nodiscard]] bool Is_Ready() const { return m_isReady; }
		[[nodiscard]] const std::string& Get_Status() const
		{
			return m_strStatus;
		}
		/* Room-thread only. Stage is allowed to fail before publication; Commit
		   is a bounded pointer swap after every process room has staged. */
		bool Stage_NumericBalance(std::uint32_t transactionSequence,
			const std::shared_ptr<const CGameplayCatalog>& candidate, std::string& status);
		bool Commit_NumericBalance(std::uint32_t transactionSequence) noexcept;
		void Abort_NumericBalance(std::uint32_t transactionSequence) noexcept;
		bool Stage_GameplayGeneration(
			std::uint32_t transactionSequence,
			const LostArk::Shared::GameplayDataRevision& baseRevision,
			const std::shared_ptr<const CGameplayCatalog>& candidateGeneration,
			std::string& status);
		bool Commit_GameplayGeneration(
			std::uint32_t transactionSequence) noexcept;
		void Abort_GameplayGeneration(
			std::uint32_t transactionSequence) noexcept;
		[[nodiscard]] std::shared_ptr<const CGameplayCatalog>
			Get_ActiveGameplayGeneration() const noexcept
		{
			return m_GameplayCatalog.Get_ActiveGeneration();
		}
		[[nodiscard]] const CGameplayCatalog* Resolve_GameplayGeneration(
			const LostArk::Shared::GameplayDataRevision& revision) const noexcept
		{
			return m_GameplayCatalog.Resolve(revision);
		}
		/* Room-thread only. Decision observability reads the same authoritative
		   brain and immutable selector generation as the Valtan simulation. */
		bool Build_ValtanDecisionTraceResponse(
			const LostArk::Shared::C2S_VALTAN_DECISION_TRACE_QUERY& request,
			LostArk::Shared::S2C_VALTAN_DECISION_TRACE_RESPONSE& outResponse,
			std::string& status) const;
		[[nodiscard]] SERVER_ROOM_PERFORMANCE_METRICS
			Get_PerformanceMetrics() const;

		// Room thread only. A sealed private arena rejects every later command.
		[[nodiscard]] bool Try_SealPrivateArenaForRetirement();
		// Room thread only. Removes the source player before the target room can
		// process its queued ENTER_WORLD and bind the same session again.
		[[nodiscard]] bool Commit_WorldTransferDeparture(SESSION_ID sessionId);
		// Room-thread only, while ServerApp holds its session-binding mutex.
		[[nodiscard]] bool Has_PersonalGuideOwner(const std::shared_ptr<CClientSession>& ownerSession) const;
		bool Transfer_PartyTo(CGameRoom& target,
			const std::vector<SESSION_ID>& leaderFirstSessionIds,
			LostArk::Shared::PARTY_TRANSFER_RESULT& outResult, std::string& status,
			const std::string& raidReturnNpcPlacementId = {},
            const std::string& spawnPlacementOverrideId = {});
		bool Transfer_ColosseumMatchTo(CGameRoom& target,
			const std::vector<SESSION_ID>& orderedSeats, std::uint64_t matchId, std::string& status);
		void Notify_ColosseumTransferResult(bool committed);
		[[nodiscard]] bool Try_SealColosseumForRetirement();
		void Notify_PartyTransferFailure(SESSION_ID sessionId,
			std::uint32_t requestSequence, LostArk::Shared::WORLD_ID targetWorldId,
			LostArk::Shared::PARTY_TRANSFER_RESULT result);

	private:
		void Mark_RuntimeFailure(std::string_view source);
		std::size_t Count_HumanPlayers() const;
		void Initialize_Guide();
		bool Build_GuidePlayer(LostArk::Shared::PLAYER_ID playerId, LostArk::Shared::NET_ENTITY_ID entityId,
			float x, float y, float z, SERVER_PLAYER& outPlayer) const;
		bool Find_GuideLanding(const SERVER_PLAYER& guide, float x, float y, float z, SERVER_NAV_POINT& point,
			const SERVER_PLAYER* anchor = nullptr) const;
		bool Start_Guide(const SERVER_PLAYER& inviter, LostArk::Shared::NET_ENTITY_ID target);
		void Handle_GuideControl(SESSION_ID sessionId, const LostArk::Shared::C2S_GUIDE_CONTROL& request);
		void Broadcast_GuideOwnership();
		void Broadcast_GuideState(const LostArk::Shared::S2C_GUIDE_STATE& message);
		void Update_Guides(float seconds);
		void Update_Colosseum(float seconds);
		void Handle_ColosseumRecruit(SESSION_ID sessionId, const LostArk::Shared::C2S_COLOSSEUM_RECRUIT& request);
		LostArk::Shared::S2C_COLOSSEUM_MATCH_STATE Build_ColosseumState() const;
		void Broadcast_ColosseumState();
        float Predict_GuideContactRisk(const SERVER_PLAYER& guide, float x, float z);
		void Guide_AnchorArrived(const SERVER_PLAYER& anchor, bool localMapTravel = false);
		void Guide_ChatCommand(const SERVER_PLAYER& sender, const std::string& text);
		void Remove_Guide(SESSION_ID ownerSessionId, bool publish = true);
		void Suspend_PersonalGuide(SESSION_ID ownerSessionId);
		void Resume_PersonalGuide(SESSION_ID ownerSessionId, LostArk::Shared::WORLD_ID sourceWorld);
		void Seed_GuideSpaceEntries(SESSION_ID ownerSessionId, const SERVER_PLAYER& anchor);
		void Prune_GuideSpacePrompts(SESSION_ID ownerSessionId);
		void Queue_GuidePrompt(SESSION_ID ownerSessionId, const GUIDE_TRIGGER& trigger);
		void Execute_PlayerMove(SERVER_PLAYER& player, const LostArk::Shared::C2S_MOVE& move);
		bool Execute_PlayerSkill(SERVER_PLAYER& player, const LostArk::Shared::C2S_USE_SKILL& skill);
		struct STAGED_PLAYER_ENTRY final
		{
			std::shared_ptr<CClientSession> pSession;
			SERVER_PLAYER Player;
			std::vector<LostArk::Shared::PACKET_FRAME> Frames;
		};
		bool Stage_PlayerEntry(const std::shared_ptr<CClientSession>& session,
			const LostArk::Shared::C2S_ENTER_WORLD& enterWorld,
			std::span<const STAGED_PLAYER_ENTRY> precedingEntries,
			STAGED_PLAYER_ENTRY& staged,
			LostArk::Shared::SESSION_DIAGNOSTIC_REASON& outReason, std::string& status,
			const std::string& spawnPlacementOverrideId = {},
			const std::vector<LostArk::Shared::INVENTORY_ITEM_SNAPSHOT>&
				carriedInventory = {},
			LostArk::Shared::HONOR_TITLE_ID carriedHonorTitleId =
				LostArk::Shared::INVALID_HONOR_TITLE_ID,
			const std::string& raidReturnNpcPlacementId = {},
			const SERVER_PURSE& carriedPurse = {}, bool hasCarriedCharacterState = false);
		bool Build_PlayerEntryFrames(STAGED_PLAYER_ENTRY& entry,
			std::span<const STAGED_PLAYER_ENTRY> batch, std::string& status);
		void Commit_PlayerEntry(const STAGED_PLAYER_ENTRY& entry);
		void Flush_PartyTransferResults();
		void Handle_Register(const std::shared_ptr<CClientSession>& session);
		bool Join(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_ENTER_WORLD& enterWorld,
			const std::string& spawnPlacementOverrideId = {},
			const std::vector<LostArk::Shared::INVENTORY_ITEM_SNAPSHOT>&
				carriedInventory = {},
			LostArk::Shared::HONOR_TITLE_ID carriedHonorTitleId =
				LostArk::Shared::INVALID_HONOR_TITLE_ID,
			const std::string& raidReturnNpcPlacementId = {},
			const SERVER_PURSE& carriedPurse = {},
			LostArk::Shared::WORLD_ID sourceWorld = LostArk::Shared::WORLD_ID::BERN,
			bool hasCarriedCharacterState = false);
		void Leave(
			SESSION_ID sessionId,
			LostArk::Shared::PLAYER_DESPAWN_REASON reason, bool publishDeparture = true);
		void Close_SessionForBindingFailure(
			SESSION_ID sessionId,
			std::string_view packetName,
			std::string_view validation);
		void Handle_Move(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_MOVE& move);
		[[nodiscard]] bool Is_BufferableComboAction(
			const SERVER_PLAYER& player) const;
		/* A move goal inside the running skill's authored move-cancel window
		ends the action now instead of waiting out the recovery pose. */
		[[nodiscard]] bool Is_MoveCancellableAction(
			const SERVER_PLAYER& player) const;
		[[nodiscard]] bool Commit_MoveGoal(
			SERVER_PLAYER& player, float goalX, float goalZ);
		void Commit_PendingPlayerCommand(
			SERVER_PLAYER& player, std::uint32_t actionStartTick);
		void Handle_UseSkill(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_USE_SKILL& useSkill);
		void Handle_ReleaseSkill(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_RELEASE_SKILL& releaseSkill);
		void Handle_UpdateSkillAim(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_UPDATE_SKILL_AIM& updateSkillAim);
		void Handle_UseEstherSkill(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_USE_ESTHER_SKILL& useEstherSkill);
		/* Debug F1 Esther summon by name: same caster lock and summon timeline as
		the slot path, without the gauge or the world roster. Release ignores it. */
		void Handle_DebugUseEsther(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_DEBUG_USE_ESTHER& request);
		/* The caster the session owns, if it may start an Esther call right now:
		bound, idle, on its feet and not riding. */
		SERVER_PLAYER* Find_EstherCaster(SESSION_ID sessionId, const char* pCommandName);
		/* Queues the summon forward along the aim and locks the caster into
		ESTHER_CAST. The gauge decision is the caller's. */
		void Begin_EstherCall(
			SERVER_PLAYER& caster,
			const ESTHER_ROSTER_ENTRY& rosterEntry,
			float aimX,
			float aimZ);
		void Handle_UseSquareHole(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_USE_SQUAREHOLE& useSquareHole);
		/* The landing of a square hole: the world's disabled "squarehole.<id>" movePlayer
		   row, admitted with the debug-teleport ground/height/collision rules. False when
		   the world has no such row or the landing is not standable for this player. */
		bool Resolve_SquareHoleDestination(
			const SERVER_PLAYER& player,
			std::uint16_t squareHoleId,
			SERVER_NAV_POINT& ground);
		/* The song lock ended: land the player at the destination, or leave them in place. */
		void Finish_SquareHoleSong(SERVER_PLAYER& player);
		bool Spawn_EstherSummon(
			const ESTHER_ROSTER_ENTRY& rosterEntry,
			LostArk::Shared::PLAYER_ID casterPlayerId,
			float positionX,
			float positionY,
			float positionZ,
			float yawDegrees);
		void Update_PendingEstherSummons(float fixedDeltaSeconds);
		void Apply_EstherStrikeHits(SERVER_WORLD_ENTITY& summon, std::uint32_t serverTick);
		void Open_EstherZone(const LostArk::Shared::EstherStrike::ZONE& zone, float positionX, float positionZ, std::uint32_t serverTick);
		void Update_EstherZones(std::uint32_t serverTick);
		void Handle_RevivePlayer(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_REVIVE_PLAYER& revivePlayer);
		/* Debug/Development-build test aid only -- zeroes the caster's own HP and
		sets PLAYER_ACTION_STATE::DEAD so a death-screen tester does not have to
		survive a real hit. Real body is compiled out in Release, matching
		Evaluate_ValtanAudition's convention. */
		void Handle_DebugKillSelf(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_DEBUG_KILL_SELF& debugKillSelf);
		/* Debug-only Character Select audition entry. This stages the ordinary
		Server world-transfer transaction; it never changes a Client level directly. */
		void Handle_DebugEnterKakulSaydonArena(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_DEBUG_ENTER_KAKULSAYDON_ARENA& request);
		/* Debug-only authored waypoint audition. The placement must be a Kakul
		playerSpawn waypoint and Server navigation remains the position authority. */
		void Handle_DebugTeleportToPlacement(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_DEBUG_TELEPORT_TO_PLACEMENT& request);
		void Handle_DebugTeleportToPosition(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_DEBUG_TELEPORT_TO_POSITION& request);
		LostArk::Shared::S2C_DEBUG_TELEPORT_TO_POSITION_RESULT Apply_DebugTeleportToPosition(
			SERVER_PLAYER& player,
			const LostArk::Shared::C2S_DEBUG_TELEPORT_TO_POSITION& request);
		LostArk::Shared::S2C_DEBUG_TELEPORT_TO_POSITION_RESULT Apply_DebugReturnToKoukuStart(
			SERVER_PLAYER& player, std::uint32_t requestSequence);
		void Reset_PlayerForDebugTeleport(SERVER_PLAYER& player);
		void Handle_DebugMarioJump(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_DEBUG_MARIO_JUMP& request);
		LostArk::Shared::S2C_DEBUG_MARIO_JUMP_RESULT Apply_DebugMarioJump(
			SERVER_PLAYER& player,
			const LostArk::Shared::C2S_DEBUG_MARIO_JUMP& request);
		void Handle_MarioMove(SESSION_ID sessionId, const LostArk::Shared::C2S_MARIO_MOVE& request);
		void Handle_MarioReturn(SESSION_ID sessionId, const LostArk::Shared::C2S_MARIO_RETURN& request);
		LostArk::Shared::S2C_MARIO_RETURN_RESULT Apply_MarioReturn(
			SERVER_PLAYER& player, const LostArk::Shared::C2S_MARIO_RETURN& request);
		bool Resolve_MarioReturnDestination(const SERVER_PLAYER& player, SERVER_NAV_POINT& destination) const;
		static void Reset_MarioContactAction(SERVER_PLAYER& player);
		SERVER_TRIGGER_MOVE_ENTRY_RESULT Begin_MarioTriggerMove(
			const WORLD_BOOTSTRAP_PLACEMENT& trigger, SERVER_PLAYER& player,
			std::uint32_t actionStartTick);
		void Update_MarioControlState(SERVER_PLAYER& player);
		std::uint8_t Begin_MarioStageObjects(std::uint8_t stage);
		void Begin_MarioBallChallenge(SERVER_PLAYER& player);
		std::uint8_t Mario_MatchingBallCount(const SERVER_PLAYER& player) const;
		std::uint8_t Mario_MarkerColor(LostArk::Shared::NET_ENTITY_ID targetId) const;
		void Reset_MarioStageObjects(std::uint8_t stage);
		void Cleanup_EmptyMarioStages();
		void Update_MarioMoveGoal(SERVER_PLAYER& player, std::uint32_t updateTick);
		bool Configure_MarioRail(SERVER_PLAYER& player, const std::string& arrivalPlacementId);
		/* Debug F1 clown/player avatar toggle: swaps only the replicated
		madness form of this session's player; Release answers REJECTED_DISABLED. */
		/* Debug F1 bingo check: paints cells and promotes completed lines. The
		board replicates on the world snapshot, so there is no result message. */
		void Handle_DebugBingoFill(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_DEBUG_BINGO_FILL& request);
		/* Debug F1 bingo bomb: marks this session's own player. The bomb
		rides the world snapshot, so there is no result message. */
		void Handle_DebugBingoBomb(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_DEBUG_BINGO_BOMB& request);
		/* Debug F1 bingo hammer: rolls one of the twenty row/column anchors
		and starts the sweep. The hammer rides the world snapshot. */
		void Handle_DebugBingoHammer(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_DEBUG_BINGO_HAMMER& request);
		/* Debug F1 "Normal Monster 1/2" (Kouku Book1/Book2, Valtan Stage_1/Stage_2):
		removes the mapped wave group's live monsters, resets the group and starts it
		over at its authored anchors. Release ignores it; the monsters ride the world
		snapshot, so there is no result message. */
		void Handle_DebugResummonWaveMonsters(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_DEBUG_RESUMMON_WAVE_MONSTERS& request);
		void Handle_DebugSetMadnessForm(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_DEBUG_SET_MADNESS_FORM& request);
		LostArk::Shared::S2C_DEBUG_SET_MADNESS_FORM_RESULT Apply_DebugMadnessForm(
			SERVER_PLAYER& player,
			const LostArk::Shared::C2S_DEBUG_SET_MADNESS_FORM& request);
		/* H key riding toggle for this session's player. The verdict is sent
		back; the ridden vehicle itself rides the world snapshot. */
		void Handle_SetVehicleRiding(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_SET_VEHICLE_RIDING& request);
		LostArk::Shared::S2C_SET_VEHICLE_RIDING_RESULT Apply_SetVehicleRiding(
			SERVER_PLAYER& player,
			const LostArk::Shared::C2S_SET_VEHICLE_RIDING& request);
		/* True while nothing the player is doing forbids a vehicle underneath. */
		bool Can_RideVehicle(const SERVER_PLAYER& player) const;
		/* Title window change for this session's player. The verdict is sent back; the
		worn title itself rides the world snapshot. */
		void Handle_SetHonorTitle(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_SET_HONOR_TITLE& request);
		LostArk::Shared::S2C_SET_HONOR_TITLE_RESULT Apply_SetHonorTitle(
			SERVER_PLAYER& player,
			const LostArk::Shared::C2S_SET_HONOR_TITLE& request);
		/* Metres per second the player walks at: the ridden vehicle's speed, or
		the class speed scaled by its held stance. */
		float Resolve_PlayerMoveSpeed(const SERVER_PLAYER& player) const;
		/* Dismounts every player the world, catalog or current state no longer
		lets ride. Runs once per tick before the snapshot is committed. */
		void Enforce_VehicleRidingState();
		/* Bern voyage ships (EFTable_VoyageShip 8200..8208) sail on the BernSea navigation region. Boarding
		   moves the player to the nearest open sea cell and keeps the pier position; leaving the ship, or any
		   forced dismount, brings the player back to that pier position. Begin returns false when no sea cell
		   lies within reach (the player is not at a harbour). */
		bool Begin_ShipVoyage(SERVER_PLAYER& player, LostArk::Shared::VEHICLE_ID vehicleId);
		void End_ShipVoyage(SERVER_PLAYER& player, const char* reason);
		/* A skill press while mounted. Only a skill of the ridden vehicle starts,
		from an idle mount, off cooldown and with a newer sequence; it faces the
		player's current yaw. */
		bool Try_StartVehicleSkill(
			SERVER_PLAYER& player,
			const LostArk::Shared::C2S_USE_SKILL& command);
		/* Advances a running vehicle skill one fixed tick: authored root motion is
		clamped to walkable ground and collision, and the action ends at its length. */
		void Update_VehicleSkill(SERVER_PLAYER& player, float fixedDeltaSeconds);
		void End_VehicleSkill(SERVER_PLAYER& player);
		/* One quick-slot press while this session's player shows an interaction
		HUD. DANCE answers the open pose window; the other modes only record the
		press until their skills own a Server judgement. */
		void Handle_InteractionSlot(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_INTERACTION_SLOT& request);
		/* Debug F1 "HUD Mode: MARIO / MAZE / Clear": forces one of the two
		modes whose gimmick has no Server trigger yet. Release answers
		REJECTED_DISABLED. */
		void Handle_DebugSetKoukuHudMode(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_DEBUG_SET_KOUKU_HUD_MODE& request);
		LostArk::Shared::S2C_DEBUG_SET_KOUKU_HUD_MODE_RESULT Apply_DebugKoukuHudMode(
			SERVER_PLAYER& player,
			const LostArk::Shared::C2S_DEBUG_SET_KOUKU_HUD_MODE& request);
		/* Every tick: madness maximum from the encounter policy, clown hold
		expiry, and the interaction HUD mode plus slot layout per player. */
		void Update_KoukuPlayerModes(std::uint32_t serverTick);
		void Apply_KoukuGateEntryCard(SERVER_PLAYER& player, const SERVER_WORLD_ENTITY& boss);
		void Handle_ChangeCharacterClass(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_CHANGE_CHARACTER_CLASS& request);
		LostArk::Shared::CHARACTER_CLASS_CHANGE_RESULT Apply_CharacterClassChange(
			SERVER_PLAYER& player,
			const LostArk::Shared::C2S_CHANGE_CHARACTER_CLASS& request);
		void Handle_SpawnWorldEntity(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_SPAWN_WORLD_ENTITY& request);
		/* Debug-only. Moves a live Valtan onto an authored health-bar threshold
		so CValtanBrain judges the crossing itself on a later fixed tick. The
		room never starts a pattern, breaks a wall or plays a cue directly.
		Evaluate owns the decision and the boss mutation and is what the contract
		tests drive; Handle only resolves the session and answers it. */
		LostArk::Shared::VALTAN_AUDITION_RESULT Evaluate_ValtanAudition(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_VALTAN_AUDITION_REQUEST& request,
			std::uint32_t& outCurrentHealthBar);
		void Handle_ValtanAudition(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_VALTAN_AUDITION_REQUEST& request);
		LostArk::Shared::VALTAN_PATTERN_FLOW_RESULT
			Evaluate_ValtanPatternFlowStart(
				SESSION_ID sessionId,
				const LostArk::Shared::C2S_DEBUG_VALTAN_PATTERN_FLOW_START& request,
				std::uint32_t& outRoomFlowEpoch,
				LostArk::Shared::GameplayDataRevision& outPinnedRevision,
				std::string& outReason);
		LostArk::Shared::VALTAN_PATTERN_FLOW_RESULT
			Evaluate_ValtanPatternFlowStopAfterCurrent(
				SESSION_ID sessionId,
				const LostArk::Shared::
					C2S_DEBUG_VALTAN_PATTERN_FLOW_STOP_AFTER_CURRENT& request,
				std::uint32_t& outRoomFlowEpoch,
				LostArk::Shared::GameplayDataRevision& outPinnedRevision,
				std::string& outReason);
		void Handle_ValtanPatternFlowStart(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_DEBUG_VALTAN_PATTERN_FLOW_START& request);
		void Handle_ValtanPatternFlowStopAfterCurrent(
			SESSION_ID sessionId,
			const LostArk::Shared::
				C2S_DEBUG_VALTAN_PATTERN_FLOW_STOP_AFTER_CURRENT& request);
		void Handle_KoukuRaidRequest(SESSION_ID sessionId, const LostArk::Shared::C2S_DEBUG_KOUKUSAYDON_RAID_REQUEST& request);
		bool Is_KoukuRaidInputBlocked() const;
		struct KOUKU_RAID_RUN final
		{
			SESSION_ID iOwnerSessionId = INVALID_SESSION_ID;
			LostArk::Shared::C2S_DEBUG_KOUKUSAYDON_RAID_REQUEST Request;
			LostArk::Shared::S2C_KOUKUSAYDON_RAID_STATE State;
			std::shared_ptr<const CGameplayCatalog> pCatalog;
			std::optional<LostArk::Shared::C2S_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_REQUEST> PriorAuditionRequest;
			std::optional<LostArk::Shared::S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_RESULT> PriorAuditionResult;
			std::optional<LostArk::Shared::S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE> PriorAuditionLifecycle;
			std::vector<LostArk::Shared::PLAYER_ID> PlayerIds;
			std::string strEntryTriggerSequenceId;
			std::set<std::string> CompletedArrivals;
			LostArk::Shared::NET_ENTITY_ID iPrimaryBossId = LostArk::Shared::INVALID_NET_ENTITY_ID;
			std::uint32_t iAuditionRequestSequence = 0u, iAuditionEpoch = 0u, iNextEntryTick = 0u;
			bool bClearCinematic = false, bEntryRunning = false, bGate3CombatEntered = false;
            bool bClearedBossPreparation = false;
            bool bGateVoteEntry = false;
            bool bBingoSpecialRunning = false;
            std::uint32_t iGate3ClearTick = 0u;
		};
		KOUKU_RAID_RUN m_KoukuRaid;
		std::uint32_t m_iNextKoukuRaidEpoch = 1u;
		std::map<SESSION_ID, std::pair<LostArk::Shared::C2S_DEBUG_KOUKUSAYDON_RAID_REQUEST, LostArk::Shared::S2C_KOUKUSAYDON_RAID_STATE>> m_KoukuRaidReceipts;
		bool Is_KoukuRaidRunning() const;
		bool Start_KoukuBingoSpecialPattern(std::uint32_t tick);
		bool Build_KoukuRaidState(LostArk::Shared::S2C_KOUKUSAYDON_RAID_STATE& state) const;
		void Broadcast_KoukuRaidState();
		void Update_KoukuRaid(std::uint32_t tick);
		void Notify_KoukuRaidBossDeath(const SERVER_WORLD_ENTITY& boss, std::uint32_t tick);
		void Stop_KoukuRaid(std::string reason, bool completed = false);
		bool Begin_KoukuRaidPreparation(SESSION_ID sessionId, const LostArk::Shared::C2S_DEBUG_KOUKUSAYDON_RAID_REQUEST& request, std::string& reason, bool clearedGateBoss = false, bool gateVoteEntry = false);
		bool Apply_KoukuRaidReadiness(SESSION_ID sessionId, const LostArk::Shared::C2S_DEBUG_KOUKUSAYDON_RAID_REQUEST& request, std::string& reason);
		bool Begin_KoukuRaidCinematic(const std::string& gateId, bool clear, std::uint32_t tick);
		bool Advance_KoukuRaidGate(std::uint8_t nextGate, bool restart);
		bool Start_KoukuRaidCombat(std::uint32_t tick, const std::string& preflightGateId = {});
        bool Build_KoukuRaidEntryRequest(const KOUKU_RAID_GATE_DEFINITION& gate, std::uint32_t entryIndex,
            LostArk::Shared::C2S_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_REQUEST& request);
		bool Enter_KoukuRaidCombat(std::uint8_t gate, std::uint32_t tick);
		bool Start_KoukuRaidEntry(std::uint32_t tick);
		void Handle_KoukuSaydonDraftChunk(SESSION_ID sessionId,
			const LostArk::Shared::C2S_DEBUG_KOUKUSAYDON_DRAFT_CHUNK& chunk);
		void Handle_KoukuSaydonPatternAudition(
			SESSION_ID sessionId,
			const LostArk::Shared::
				C2S_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_REQUEST& request);
		LostArk::Shared::KOUKUSAYDON_PATTERN_AUDITION_RESULT
			Evaluate_KoukuSaydonPatternAudition(
				SESSION_ID sessionId,
				const LostArk::Shared::
					C2S_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_REQUEST& request,
				LostArk::Shared::
					S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_RESULT& outResult, bool continueRaid = false,
                const std::vector<SERVER_WORLD_ENTITY>* stagedBosses = nullptr);
		SERVER_WORLD_ENTITY* Find_KoukuSaydonAuditionBoss();
		/* The live arena boss a Debug audition scope names: the Gate 1 Kouku or
		a gate boss raised from a disabled placement. Null when that placement
		is not currently spawned. */
		SERVER_WORLD_ENTITY* Find_KoukuSaydonArenaBoss(
			const std::string& placementId,
			const std::string& archetypeId);
		bool Update_KoukuSaydonBoss(
			SERVER_WORLD_ENTITY& boss, std::uint32_t serverTick);
		/* Broadcasts the cues a Logic tick produced, inserts follow-up patterns
		after the running audition slot, and returns true when a window asked
		the running pattern to end now (the brain commits it as COMPLETED). */
		struct KOUKU_PENDING_MECHANIC_TRIGGER final
		{
			LostArk::Shared::NET_ENTITY_ID iBossEntityId = LostArk::Shared::INVALID_NET_ENTITY_ID;
			std::uint32_t iPatternSequence = 0u;
			BOSS_PATTERN_MECHANIC_TRIGGER Trigger;
		};
		std::vector<KOUKU_PENDING_MECHANIC_TRIGGER> m_PendingKoukuMechanicTriggers;
		[[nodiscard]] bool Commit_KoukuAlbionAirborne(SERVER_WORLD_ENTITY& boss,
			const BOSS_PATTERN_DEFINITION& pattern, const BOSS_PATTERN_MECHANIC_TRIGGER& trigger, std::uint32_t serverTick);
		void Commit_KoukuMechanicTriggers(std::uint32_t serverTick);
		void Update_KoukuActorContacts(SERVER_WORLD_ENTITY& actor, const BOSS_PATTERN_DEFINITION& pattern,
			const CGameplayCatalog& product, std::uint32_t serverTick);
		void Update_KoukuPursuitProjectiles(SERVER_WORLD_ENTITY& boss, const BOSS_PATTERN_MECHANIC_TRIGGER& trigger,
			KOUKUSAYDON_PLAYER_TARGET_WINDOW_STATE& window, const CGameplayCatalog& catalog, std::uint32_t serverTick);
		void Update_KoukuPlayerTargets(SERVER_WORLD_ENTITY& boss,
			const BOSS_PATTERN_DEFINITION& pattern, KOUKUSAYDON_LOGIC_LEDGER& ledger,
			const CGameplayCatalog& catalog, std::uint32_t serverTick);
		void Clear_KoukuPlayerTargets(SERVER_WORLD_ENTITY& boss, KOUKUSAYDON_LOGIC_LEDGER& ledger);
		void Update_KoukuRandomVolley(SERVER_WORLD_ENTITY& boss,
			const BOSS_PATTERN_MECHANIC_TRIGGER& trigger, KOUKUSAYDON_PLAYER_TARGET_WINDOW_STATE& window,
			const CGameplayCatalog& catalog, std::uint32_t serverTick, bool hasAlivePlayers);
		void Update_KoukuGazeClones(std::uint32_t serverTick);
		[[nodiscard]] bool Update_KoukuSummonTriggers(SERVER_WORLD_ENTITY& clone,
			const BOSS_PATTERN_DEFINITION& pattern, std::uint32_t serverTick);
		bool Apply_KoukuLogicOutput(
			const KOUKUSAYDON_LOGIC_OUTPUT& output,
			SERVER_WORLD_ENTITY& boss,
			std::uint32_t serverTick);
		enum class KOUKUSAYDON_PATTERN_AUDITION_PHASE : std::uint8_t
		{
			INACTIVE,
			PENDING,
			ACTIVE
		};

		struct KOUKUSAYDON_PATTERN_AUDITION_MEMBER final
		{
			std::string strMemberId;
			LostArk::Shared::NET_ENTITY_ID iBossEntityId = LostArk::Shared::INVALID_NET_ENTITY_ID;
			KOUKUSAYDON_PATTERN_AUDITION_PHASE ePhase = KOUKUSAYDON_PATTERN_AUDITION_PHASE::PENDING;
			std::vector<std::string> PatternIds;
			std::vector<std::uint32_t> TransitionTicks;
			std::size_t iPatternIndex = 0u;
			std::uint32_t iNextStartTick = 0u;
			std::uint32_t iScheduledStartTick = 0u;
			std::uint32_t iPatternSequence = 0u;
			bool bCompleted = false;
			bool bOwnsPlayerMode = false;
			KOUKUSAYDON_LOGIC_LEDGER LogicLedger;
			// The entry root owns its portal across the completion-driven children.
			std::optional<SERVER_WORLD_ENTITY> MarioEntryAnchor;
			std::string strMarioEntryPatternId;
			std::uint32_t iMarioEntryStartTick = 0u;
			std::uint8_t iMarioEntryStage = 0u;
			bool bMarioEntryConsumed = false;
			// Pin the successful entrant, not whichever player is present later.
			bool bMarioSoloReturnRequired = false;
			LostArk::Shared::PLAYER_ID iMarioEntrantPlayerId = 0u;
			SESSION_ID iMarioEntrantSessionId = INVALID_SESSION_ID;
			LostArk::Shared::NET_ENTITY_ID iMarioEntrantNetEntityId = LostArk::Shared::INVALID_NET_ENTITY_ID;
			bool bMarioReturnCompleted = false;
			std::size_t iCompletionChainFirstIndex = 0u;
			std::uint32_t iCompletionChainCount = 0u;
			std::uint32_t iCompletionChainCompleted = 0u;
			std::string strCompletionChainSuccessPatternId;
			bool bCompletionChainStarted = false;
            bool bParentSequenceStarted = false;
            bool bParentSequenceLoops = false;
            std::size_t iParentLoopIndex = 0u;
            std::size_t iParentLastIndex = 0u;
			bool bCompletionChainAwaitingReturn = false;
			bool bCompletionChainSuccessQueued = false;
			std::uint32_t iNextWorldCue = 1u;
			std::unordered_map<std::string, std::string> WorldCueByInstance;
			std::unordered_map<std::string, std::string> WorldCueByOccurrence;
		};
		// Stage completion releases the actor clock, while these occurrence rows retain theirs.
		struct KOUKU_PATTERN_TAIL final
		{
			std::shared_ptr<SERVER_WORLD_ENTITY> pOwner;
			KOUKUSAYDON_PATTERN_AUDITION_MEMBER Member;
			std::uint32_t iEndTick = 0u;
			std::uint32_t iLastUpdateTick = 0u;
		};
		struct KOUKU_SCHEDULED_SUPPORT_SURFACE final
		{
			std::string strMemberId;
			std::uint32_t iStartTick = 0u;
			std::uint32_t iEndTick = 0u;
			SERVER_NAVIGATION_SUPPORT_SURFACE Surface;
		};
		struct KOUKUSAYDON_PATTERN_AUDITION_STATE final
		{
			KOUKUSAYDON_PATTERN_AUDITION_PHASE ePhase = KOUKUSAYDON_PATTERN_AUDITION_PHASE::INACTIVE;
			SESSION_ID iOwnerSessionId = INVALID_SESSION_ID;
			LostArk::Shared::C2S_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_REQUEST Request;
			std::uint32_t iRoomAuditionEpoch = 0u;
			std::uint32_t iCommonStartTick = 0u;
			LostArk::Shared::GameplayDataRevision PinnedGameplayRevision{};
			std::uint32_t iPinnedSourceRevision = 0u;
			/* Global gameplay authority remains PinnedGameplayRevision. The
			   separate Kouku Product source owns pattern/logic rows for this run. */
			std::shared_ptr<const CGameplayCatalog> pProductGeneration;
			std::vector<KOUKUSAYDON_PATTERN_AUDITION_MEMBER> Members;
			std::vector<LostArk::Shared::S2C_WORLD_SEQUENCE_PLAY> WorldPlays;
			std::vector<KOUKU_SCHEDULED_SUPPORT_SURFACE> SupportSchedule;
			std::vector<KOUKU_PATTERN_TAIL> Tails;
		};
		KOUKUSAYDON_PATTERN_AUDITION_MEMBER* Find_KoukuAuditionMember(LostArk::Shared::NET_ENTITY_ID bossId, std::uint32_t patternSequence = 0u);
		KOUKUSAYDON_LOGIC_LEDGER* Active_KoukuPlayerLedger();
		[[nodiscard]] const CGameplayCatalog* Resolve_KoukuProductCatalog() const noexcept;
		void Prepare_KoukuAuditionTick(std::uint32_t serverTick);
		void Update_KoukuPatternTails(std::uint32_t serverTick);
		SERVER_WORLD_ENTITY* Find_KoukuOccurrenceOwner(LostArk::Shared::NET_ENTITY_ID bossId, std::uint32_t patternSequence);
		bool Retain_KoukuPatternTail(KOUKUSAYDON_PATTERN_AUDITION_MEMBER& member,
			const SERVER_WORLD_ENTITY& sourceOwner, const BOSS_PATTERN_DEFINITION& pattern, std::uint32_t serverTick);
        bool Start_KoukuParentSequence(KOUKUSAYDON_PATTERN_AUDITION_MEMBER& member,
            SERVER_WORLD_ENTITY& boss, const BOSS_PATTERN_DEFINITION& pattern, std::uint32_t serverTick);
		bool Start_KoukuCompletionChain(KOUKUSAYDON_PATTERN_AUDITION_MEMBER& member,
			SERVER_WORLD_ENTITY& boss, const BOSS_PATTERN_DEFINITION& pattern, std::uint32_t serverTick);
		void Update_KoukuMarioEntry(KOUKUSAYDON_PATTERN_AUDITION_MEMBER& member, std::uint32_t serverTick);
		void Commit_KoukuMarioEntries();
		bool Commit_KoukuMarioPhasePlayers(SERVER_WORLD_ENTITY& boss, const BOSS_PATTERN_MECHANIC_TRIGGER& trigger, std::uint32_t serverTick);
		void Complete_KoukuMarioReturn(SERVER_PLAYER& player,
			const std::string& sourcePlacementId, std::uint32_t updateTick);
		void Queue_KoukuCompletionChainSuccess(KOUKUSAYDON_PATTERN_AUDITION_MEMBER& member,
			std::uint32_t serverTick);
		struct KOUKU_PENDING_MARIO_ENTRY final { std::string strMemberId; LostArk::Shared::PLAYER_ID iPlayerId; std::uint32_t iRootStartTick; std::uint8_t iStage = 0u; };
		std::vector<KOUKU_PENDING_MARIO_ENTRY> m_PendingKoukuMarioEntries;
		bool Enter_MarioFromPattern(SERVER_PLAYER& player, std::uint8_t stage);
		bool Refresh_KoukuSupportSurfaces(std::uint32_t serverTick);
		bool Build_KoukuBundleState(LostArk::Shared::S2C_KOUKUSAYDON_BUNDLE_STATE& message) const;
		void Broadcast_KoukuBundleState(LostArk::Shared::KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE state);
		void Broadcast_OwnedWorldSequence(const LostArk::Shared::S2C_WORLD_SEQUENCE_PLAY& message);
		void Stop_KoukuWorldOwner(const std::string& memberId = {}, bool finished = false);
		// Survives natural Pattern completion; reset/cancel and HP zero own removal.
		struct KOUKU_DAMAGEABLE_WORLD_CUE final
		{
			LostArk::Shared::S2C_WORLD_SEQUENCE_PLAY Play;
			LostArk::Shared::NET_ENTITY_ID iBodyId = LostArk::Shared::INVALID_NET_ENTITY_ID;
			SESSION_ID iOwnerSessionId = INVALID_SESSION_ID;
			bool bCancelled = false;
			std::uint32_t iNextMadnessTick = 0u;
			BOSS_ENCOUNTER_MADNESS_POLICY MadnessPolicy;
			std::uint8_t iMadnessSource = 0u; // 1 circus ball, 2 odd doll
		};
		std::vector<KOUKU_DAMAGEABLE_WORLD_CUE> m_KoukuDamageableWorldCues;
		std::vector<SERVER_WORLD_ENTITY> m_PendingKoukuWorldBodies;
		bool Stage_KoukuWorldBody(const BOSS_PATTERN_WORLD_COMBAT_BODY& body,
			LostArk::Shared::S2C_WORLD_SEQUENCE_PLAY& play, bool authoredMadness = false);
		void Cancel_KoukuWorldBodies(const std::string& memberId = {}, SESSION_ID ownerSession = INVALID_SESSION_ID);
		void Update_KoukuWorldBodies(std::uint32_t serverTick);

		struct KOUKUSAYDON_PATTERN_AUDITION_RECEIPT final
		{
			LostArk::Shared::
				C2S_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_REQUEST Request;
			LostArk::Shared::
				S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_RESULT Result;
			std::optional<LostArk::Shared::
				S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE>
				LastLifecycle;
		};

		void Queue_KoukuSaydonPatternAuditionLifecycle(
			const std::string& patternId,
			std::uint32_t patternSequence,
			std::uint32_t stageIndex,
			LostArk::Shared::
				KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE state,
			std::string reason = {}, LostArk::Shared::NET_ENTITY_ID bossId = LostArk::Shared::INVALID_NET_ENTITY_ID);
		bool Flush_KoukuSaydonPatternAuditionLifecycle();
		void Clear_KoukuSaydonPatternAudition(bool completed = false, std::string reason = {});
		SERVER_WORLD_ENTITY* Find_AuditionBoss();
		SERVER_WORLD_ENTITY* Find_AuditionBoss(
			const std::string& placementId);
		bool Has_EngagedAuditionPlayer(const SERVER_WORLD_ENTITY& boss) const;
		bool Build_ValtanBossOnlyAuditionReset(
			const SERVER_WORLD_ENTITY& boss,
			std::uint32_t resetTick,
			SERVER_WORLD_ENTITY& outBoss,
			std::string& status);
		bool Reset_ValtanBossOnlyAuditionState(
			SERVER_WORLD_ENTITY& boss,
			std::uint32_t resetTick,
			std::string& status);
		bool Reset_ValtanAuditionState(
			SERVER_WORLD_ENTITY& boss,
			std::uint32_t resetTick,
			std::string& status);
		enum class VALTAN_PATTERN_ID_AUDITION_PHASE : std::uint8_t
		{
			INACTIVE,
			PENDING,
			ACTIVE,
			COMPLETED_HOLD,
			IDLE_HOLD
		};

		struct VALTAN_PATTERN_ID_AUDITION_STATE final
		{
			VALTAN_PATTERN_ID_AUDITION_PHASE ePhase =
				VALTAN_PATTERN_ID_AUDITION_PHASE::INACTIVE;
			SESSION_ID iOwnerSessionId = 0u;
			LostArk::Shared::NET_ENTITY_ID iBossEntityId =
				LostArk::Shared::INVALID_NET_ENTITY_ID;
			std::uint32_t iExpectedPatternSequence = 0u;
			std::uint32_t iRequestSequence = 0u;
			std::uint32_t iRoomAuditionEpoch = 0u;
			std::string strBossPlacementId;
			std::string strPatternId;
			LostArk::Shared::GameplayDataRevision PinnedDefinitionRevision{};
			bool bResetlessContinuation = false;
			bool bReportedWaitingForPlayer = false;
			// A live predecessor has no Client Play request to report a lifecycle for.
			bool bAdoptedLivePredecessor = false;
			// Keep only the current Flow occurrence on its existing ordered Brain path.
			std::optional<BOSS_PATTERN_SEQUENCE_DEFINITION> AdoptedFlowSequence;
		};

		struct VALTAN_NEXT_PATTERN_RESERVATION final
		{
			SESSION_ID iOwnerSessionId = INVALID_SESSION_ID;
			LostArk::Shared::NET_ENTITY_ID iBossEntityId =
				LostArk::Shared::INVALID_NET_ENTITY_ID;
			std::uint32_t iRequestSequence = 0u;
			std::uint32_t iRoomAuditionEpoch = 0u;
			std::uint32_t iPredecessorPatternSequence = 0u;
			std::uint32_t iExpectedPatternSequence = 0u;
			std::string strBossPlacementId;
			std::string strPatternId;
			LostArk::Shared::GameplayDataRevision PinnedDefinitionRevision{};
			bool bReportedWaitingForPlayer = false;
		};

		struct VALTAN_NEXT_PATTERN_COMMAND_RECEIPT final
		{
			LostArk::Shared::C2S_VALTAN_AUDITION_REQUEST Request;
			LostArk::Shared::VALTAN_AUDITION_RESULT Result =
				LostArk::Shared::VALTAN_AUDITION_RESULT::REJECTED_STALE_REQUEST;
			std::uint32_t iCurrentHealthBar = 0u;
		};

		[[nodiscard]] bool Is_ValtanPatternIdAuditionRunning() const noexcept;
		LostArk::Shared::VALTAN_AUDITION_RESULT Evaluate_ValtanNextPatternControl(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_VALTAN_AUDITION_REQUEST& request,
			std::uint32_t& outCurrentHealthBar);
		LostArk::Shared::VALTAN_AUDITION_RESULT Adopt_ValtanLiveNextPattern(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_VALTAN_AUDITION_REQUEST& request,
			SERVER_WORLD_ENTITY& boss);
		void Cancel_ValtanNextPatternReservation(std::string reason);
		void Cancel_ValtanPatternIdAudition(std::string reason);
		void Try_PromoteValtanNextPattern(SERVER_WORLD_ENTITY& boss);
		bool Prepare_ValtanPatternIdAuditionBeforeBrain(SERVER_WORLD_ENTITY& boss);
		bool Refresh_ValtanPatternIdAuditionState();
		void Queue_ValtanAuditionLifecycle(
			SESSION_ID ownerSessionId,
			std::uint32_t requestSequence,
			std::uint32_t roomEpoch,
			std::uint32_t patternSequence,
			const std::string& patternId,
			const LostArk::Shared::GameplayDataRevision& pinnedRevision,
			LostArk::Shared::VALTAN_AUDITION_LIFECYCLE_STATE state,
			std::string reason = {});
		void Queue_ValtanNextPatternLifecycle(
			const VALTAN_NEXT_PATTERN_RESERVATION& reservation,
			LostArk::Shared::VALTAN_AUDITION_LIFECYCLE_STATE state,
			std::string reason = {});
		void Queue_ValtanPatternIdAuditionLifecycle(
			LostArk::Shared::VALTAN_AUDITION_LIFECYCLE_STATE state,
			std::string reason = {});
		bool Flush_ValtanPatternIdAuditionLifecycle();

		enum class VALTAN_PATTERN_FLOW_AUDITION_PHASE : std::uint8_t
		{
			INACTIVE,
			PENDING,
			ACTIVE
		};

		struct VALTAN_PATTERN_FLOW_AUDITION_STATE final
		{
			VALTAN_PATTERN_FLOW_AUDITION_PHASE ePhase =
				VALTAN_PATTERN_FLOW_AUDITION_PHASE::INACTIVE;
			SESSION_ID iOwnerSessionId = INVALID_SESSION_ID;
			LostArk::Shared::PLAYER_ID iOwnerPlayerId =
				LostArk::Shared::INVALID_PLAYER_ID;
			LostArk::Shared::NET_ENTITY_ID iBossEntityId =
				LostArk::Shared::INVALID_NET_ENTITY_ID;
			std::uint32_t iRequestSequence = 0u;
			std::uint32_t iRoomFlowEpoch = 0u;
			std::uint32_t iFirstPatternSequence = 0u;
			std::size_t iStartSlotIndex = 0u;
			std::size_t iReportedSequenceIndex =
				(static_cast<std::size_t>(-1));
			std::uint32_t iReportedPatternSequence = 0u;
			bool bReportedPausedForRevive = false;
			bool bStopAfterCurrent = false;
			std::string strBossPlacementId;
			std::string strFlowId;
			std::string strFlowRevision;
			std::string strStartSlotId;
			std::vector<LostArk::Shared::VALTAN_PATTERN_FLOW_SLOT_WIRE> Slots;
			BOSS_PATTERN_SEQUENCE_DEFINITION Sequence;
			LostArk::Shared::GameplayDataRevision PinnedDefinitionRevision{};
		};

		[[nodiscard]] bool Is_ValtanPatternFlowRunning() const noexcept;
		[[nodiscard]] const BOSS_PATTERN_SEQUENCE_DEFINITION*
			Resolve_ValtanPatternFlowSequence(
				const SERVER_WORLD_ENTITY& boss) const noexcept;
		void Refresh_ValtanPatternFlowState(SERVER_WORLD_ENTITY& boss);
		void Finish_ValtanPatternFlow(
			SERVER_WORLD_ENTITY& boss,
			LostArk::Shared::VALTAN_PATTERN_FLOW_LIFECYCLE_STATE terminalState,
			std::string reason = {});
		void Abort_ValtanPatternFlowForOwner(
			SESSION_ID sessionId,
			std::string reason);
		void Queue_ValtanPatternFlowLifecycle(
			LostArk::Shared::VALTAN_PATTERN_FLOW_LIFECYCLE_STATE state,
			const SERVER_WORLD_ENTITY* boss,
			std::string reason = {});
		bool Flush_ValtanPatternFlowLifecycle();

		enum class VALTAN_TIMELINE_AUDITION_PHASE : std::uint8_t
		{
			INACTIVE,
			WAITING_ENVIRONMENT,
			READY,
			WAITING_PATTERN_START,
			WAITING_PATTERN_FINISH,
			COMPLETED_HOLD,
			FAILED_HOLD
		};

		struct VALTAN_TIMELINE_AUDITION_STATE final
		{
			VALTAN_TIMELINE_AUDITION_PHASE ePhase =
				VALTAN_TIMELINE_AUDITION_PHASE::INACTIVE;
			SESSION_ID iOwnerSessionId = 0;
			LostArk::Shared::PLAYER_ID iOwnerPlayerId =
				LostArk::Shared::INVALID_PLAYER_ID;
			LostArk::Shared::NET_ENTITY_ID iBossEntityId =
				LostArk::Shared::INVALID_NET_ENTITY_ID;
			std::size_t iRowIndex = 0u;
			std::size_t iActionIndex = 0u;
			std::uint32_t iRepeatIndex = 0u;
			std::uint32_t iExpectedPatternSequence = 0u;
			std::uint32_t iEnvironmentDeadlineTick = 0u;
			std::uint32_t iHeldBossHp = 0u;
			std::uint32_t iHeldBossHealthBar = 0u;
			bool bAllowProductPropBreak = false;
			LostArk::Shared::GameplayDataRevision PinnedDefinitionRevision{};
			std::string strExpectedPatternId;
			std::vector<std::string> ExpectedGoneGroupIds;
		};

		/* A page start differs from a one-row timeline audition: it stages the
		already-destroyed arena, releases the real Brain at that page boundary,
		and then leaves the encounter running normally. */
		struct VALTAN_FIGHT_PAGE_START_STATE final
		{
			LostArk::Shared::NET_ENTITY_ID iBossEntityId =
				LostArk::Shared::INVALID_NET_ENTITY_ID;
			std::uint32_t iCommandId = 0u;
			std::uint32_t iEnvironmentDeadlineTick = 0u;
			std::vector<std::string> ExpectedGoneGroupIds;

			bool Is_Active() const noexcept
			{
				return LostArk::Shared::INVALID_NET_ENTITY_ID != iBossEntityId;
			}
		};

		bool Prepare_ValtanTimelineArenaState(
			const CWorldDestructionRuntime& runtime,
			const SERVER_WORLD_ENTITY& boss,
			VALTAN_TIMELINE_ARENA_STATE arenaState,
			std::uint32_t requestTick,
			WORLD_DESTRUCTION_TRANSACTION& outTransaction,
			std::vector<std::string>& outExpectedGoneGroupIds,
			std::string& status) const;
		bool Stage_ValtanTimelineRowStart(
			SESSION_ID sessionId,
			const SERVER_WORLD_ENTITY& boss,
			std::uint32_t commandId,
			std::uint32_t startTick,
			SERVER_PLAYER& outOwner,
			std::string& status) const;
		bool Start_ValtanTimelineRow(
			SESSION_ID sessionId,
			SERVER_WORLD_ENTITY& boss,
			std::uint32_t commandId,
			std::uint32_t startTick,
			std::string& status);
		bool Stop_ValtanTimelineRow(bool resetEncounter = false);
		bool Prepare_ValtanTimelineRowBeforeBrain(
			SERVER_WORLD_ENTITY& boss,
			std::uint32_t updateTick);
		void Restore_ValtanTimelineRowAfterBrain(
			SERVER_WORLD_ENTITY& boss,
			std::uint32_t updateTick);
		bool Start_ValtanFightPage(
			SESSION_ID sessionId,
			SERVER_WORLD_ENTITY& boss,
			std::uint32_t commandId,
			std::uint32_t startTick,
			std::string& status);
		bool Prepare_ValtanFightPageBeforeBrain(
			SERVER_WORLD_ENTITY& boss,
			std::uint32_t updateTick);
		struct VALTAN_DECISION_TRACE_REVISION_STATE final
		{
			LostArk::Shared::NET_ENTITY_ID iBossEntityId =
				LostArk::Shared::INVALID_NET_ENTITY_ID;
			std::string strBossPlacementId;
			std::uint64_t iTraceSequence = 0u;
			LostArk::Shared::GameplayDataRevision DefinitionRevision{};
		};
		// Validates itemId against the loaded catalog and stacks quantity into
		// player.Inventory, capped at maxStack and MAX_INVENTORY_ITEMS distinct
		// stacks. Returns false (no-op) for an unknown item or a full inventory
		// that would need a new stack. Shared by Handle_DebugGiveItem and the
		// Valtan clear-reward grant in the world entity tick loop.
		bool Grant_Item(
			SERVER_PLAYER& player,
			const std::string& itemId,
			std::uint32_t quantity);
		// Debug-only. Validates the item against the loaded catalog and
		// stacks it into the player's inventory, capped at maxStack.
		void Handle_DebugGiveItem(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_DEBUG_GIVE_ITEM& request);
		// Validates the item is owned, is a consumable (iHealPercent > 0), and
		// the player is alive; heals iMaximumHp * iHealPercent / 100, then
		// decrements/removes the stack. HP reaches the Client through the next
		// S2C_WORLD_SNAPSHOT tick like any other HP change; no separate result.
		void Handle_UseItem(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_USE_ITEM& request);
		/* Right-click equip / unequip. Checks the slot kind, the class and bag room,
		   then answers with the whole inventory whether or not anything moved. */
		void Handle_SetEquipment(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_SET_EQUIPMENT& request);
		bool Apply_SetEquipment(SERVER_PLAYER& player,
			const LostArk::Shared::C2S_SET_EQUIPMENT& request) const;
		/* Repair NPC window: every worn part goes back to 100 percent (free for now); the
		   inventory answer carries the repaired percents. */
		void Handle_RepairEquipment(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_REPAIR_EQUIPMENT& request);
		/* After a class change: items bound to another class go back to the bag. */
		bool Unequip_OtherClassItems(SERVER_PLAYER& player) const;
		/* NPC shop basket. The player must stand by that shop NPC; every line must be in
		   its stock, the total price must be covered and the bag must take every line, or
		   nothing changes. Answers with the whole inventory either way. */
		void Handle_BuyItems(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_BUY_ITEMS& request);
		bool Apply_BuyItems(SERVER_PLAYER& player,
			const LostArk::Shared::C2S_BUY_ITEMS& request) const;
		/* One-shot restore of a Client-saved character (inventory, purse, honor title).
		   Bern only, once per fresh entry, and only before any inventory change; the
		   whole request is validated first and a rejected one changes nothing. */
		void Handle_RestoreCharacter(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_RESTORE_CHARACTER& request);
		bool Validate_RestoreCharacter(const SERVER_PLAYER& player,
			const LostArk::Shared::C2S_RESTORE_CHARACTER& request) const;
		void Handle_UpgradeEquipment(SESSION_ID sessionId,
			const LostArk::Shared::C2S_UPGRADE_EQUIPMENT& request);
		void Handle_CaptureCharacter(SESSION_ID sessionId,
			const LostArk::Shared::C2S_CAPTURE_CHARACTER& request);
		// Debug Character Select Arena "되돌리기" -- despawns every world entity the
		// debug spawn buttons created in this room (Broadcast_WorldEntityDespawned per
		// entity) and resets the spawn group runtime so the same groups can be
		// re-activated. CHARACTER_SELECT_ARENA only; no-op reply for anything else.
		void Handle_DespawnAllWorldEntities(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_DESPAWN_ALL_WORLD_ENTITIES& request);
		/* KoukuSaydon arena form of the Debug revert: removes only the entities
		raised from disabled bootstrap placements (the F1 gate buttons) and
		their dependents, keeping the statically enabled Gate 1 Kouku. */
		bool Despawn_KoukuSaydonArenaDebugEntities(bool allArenaBosses = false);
		bool Despawn_KoukuSaydonArenaDebugEntities(bool allArenaBosses, bool preflightOnly);
		// Bern's Valtan-entry confirm window (right-click a guide NPC). Replaces the
		// old automatic changeLevel triggerBox OBB fire: validates the requesting
		// player is still near the named guide NPC world entity, alive, and idle,
		// then stages the same SERVER_WORLD_TRANSFER_REQUEST the trigger used to
		// build. BERN only; no-op for anything else.
		void Handle_ConfirmNpcEntry(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_CONFIRM_NPC_ENTRY& request);
		// Colosseum match queue (BERN only). JOIN is the answer to the Colosseum NPC's offer: the
		// Server re-tests distance/state, collects up to four humans for ten seconds,
		// and reserves acceptance-order parity teams for one atomic
		// admission into a private Colosseum match. Only a committed transfer consumes the queue.
		// LEAVE removes only the requesting session.
		void Handle_ColosseumQueueJoin(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_COLOSSEUM_QUEUE_JOIN& request);
		void Handle_ColosseumQueueLeave(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_COLOSSEUM_QUEUE_LEAVE& request);
		void Send_ColosseumQueueState(
			SESSION_ID sessionId, LostArk::Shared::COLOSSEUM_QUEUE_STATE state);
		void Try_FormColosseumMatch();
		void Broadcast_ColosseumQueueState();
		void Try_StartColosseumEntry();
		void Handle_ColosseumLoadReady(SESSION_ID, const LostArk::Shared::C2S_COLOSSEUM_LOAD_READY&);
		void Handle_ColosseumReturn(SESSION_ID, const LostArk::Shared::C2S_COLOSSEUM_RETURN&);
		void Update_ColosseumMatch(std::uint32_t tick);
		void Broadcast_ColosseumMatchState();
		void Score_ColosseumKills(std::uint32_t tick);
		// The player pressed the key an interact-gated trigger box offered.
		// Names only the box; the trigger system re-tests that this player is
		// still standing in it before anything runs, so a stale or forged
		// request changes nothing.
		void Handle_InteractTrigger(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_INTERACT_TRIGGER& request);
		// Raid Clear screen's "돌아가기" button -- the reverse trip. No proximity
		// or party-leader gating (unlike Handle_ConfirmNpcEntry): any player in
		// a cleared Valtan/Kouku raid can return independently to its recorded entry guide.
		// Direct Lobby entries retain the default guide; NPC entries keep their source ID.
		void Handle_ReturnToBern(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_RETURN_TO_BERN& request);
		// Stages one player's world transfer back to Bern. Shared by the cleared-raid exit
		// button and the gate progress EXIT vote; returns false when nothing was staged.
		bool Stage_ReturnToBern(
			LostArk::Shared::PLAYER_ID playerId,
			std::uint32_t requestSequence);
		/* Same-room-only: request.iTargetNetEntityId must resolve to a real
		   player currently in this room's m_PlayerIdByEntityId. There is no
		   cross-room player identity yet (nickname is display text only, see
		   CLAUDE.md), so an invite naming a player in a different room or a
		   stale/unknown NetEntityId is rejected, not queued. */
		void Handle_PartyInvite(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_PARTY_INVITE& request);
		void Handle_PartyInviteRespond(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_PARTY_INVITE_RESPOND& request);
		void Broadcast_PartyRoster(std::uint32_t partyId);

		/* 파티 레이드 입장 전원 수락 투표. 한 플레이어는 동시에 하나의 열린 proposal에만
		   속한다(propose가 그 불변식을 검사). iProposalId는 이 방에서 발급하는 단조 증가
		   식별자로 pointer/index가 아니다. Voters는 발의 시점 멤버 스냅샷(솔로는 1명),
		   Accepted는 그 부분집합. m_iServerTick이 iDeadlineTick을 넘으면 TIMEOUT으로 닫는다. */
		/* Commander raid gate progress (KoukuSaydon gates). The room marks a gate cleared
		   when its last primary boss dies (Notify_GateBossDeath from the world update),
		   the leader / solo player proposes to move on, members answer, and on
		   ALL_ACCEPTED Advance_Gate despawns the arena, raises the next gate's disabled
		   placements and moves every player to the gate position -- the product path of
		   what the Debug gate buttons do by hand. Implemented in GameRoom_GateProgress.cpp. */
		struct GATE_PROGRESS_STATE
		{
			std::uint8_t iCurrentGate = 0u;     // 1-based, 0 = no gate raised yet
			std::uint8_t iClearedMask = 0u;
			std::uint32_t iProposalId = 0u;     // 0 = no vote open
			std::uint32_t iRaidEpoch = 0u;      // Nonzero pins a vote to its immutable raid run
			LostArk::Shared::GATE_PROGRESS_KIND eKind = LostArk::Shared::GATE_PROGRESS_KIND::ADVANCE;
			std::uint32_t iRequestSequence = 0u;
			LostArk::Shared::PLAYER_ID iProposerId = LostArk::Shared::INVALID_PLAYER_ID;
			std::vector<LostArk::Shared::PLAYER_ID> Voters;
			std::vector<LostArk::Shared::PLAYER_ID> Accepted;
			std::uint32_t iDeadlineTick = 0u;
		};
		void Handle_GateProgressPropose(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_GATE_PROGRESS_PROPOSE& request);
		void Handle_GateProgressRespond(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_GATE_PROGRESS_RESPOND& request);
		void Close_GateProgressVote(LostArk::Shared::GATE_PROGRESS_VOTE_RESULT result);
		void Expire_GateProgressVote();
		bool Complete_GateProgressTransition(const GATE_PROGRESS_STATE& transition);
		void Update_ArenaAssembly();
		bool Complete_ArenaAssembly();
		bool Collect_KoukuEntryParticipants(std::vector<LostArk::Shared::PLAYER_ID>& participants, std::uint32_t& raidEpoch) const;
		bool Collect_ValtanEntryParticipants(std::vector<LostArk::Shared::PLAYER_ID>& participants) const;
		bool Start_ValtanEntry(const std::vector<LostArk::Shared::PLAYER_ID>& expectedParticipants);
		// A primary boss is about to be removed DEAD: clears its gate when it was the last one.
		void Notify_GateBossDeath(const SERVER_WORLD_ENTITY& deadBoss);
		// A gate placement came up (Debug button or Advance_Gate): that gate is now current.
		void Note_GatePlacementRaised(const std::string& placementId);
		bool Advance_Gate(std::uint8_t nextGate);
		bool Advance_Gate(std::uint8_t nextGate, const std::vector<LostArk::Shared::PLAYER_ID>* participants);
		bool Spawn_GatePlacement(const std::string& placementId);
		bool Spawn_GatePlacement(const std::string& placementId, SERVER_WORLD_ENTITY* prepared);
		bool Build_GateProgressState(LostArk::Shared::S2C_GATE_PROGRESS_STATE& message,
			bool bClosed, LostArk::Shared::GATE_PROGRESS_VOTE_RESULT result) const;
		void Broadcast_GateProgressState(
			bool bClosed, LostArk::Shared::GATE_PROGRESS_VOTE_RESULT result,
			LostArk::Shared::GATE_PROGRESS_KIND completedKind = LostArk::Shared::GATE_PROGRESS_KIND::END);
		std::uint8_t Gate_Count() const;
		std::uint8_t Resolve_CurrentKoukuGate() const;
		bool Resolve_KoukuRevivePosition(const SERVER_PLAYER& player, SERVER_NAV_POINT& position, float& yaw) const;
		int Gate_IndexOfPlacement(const std::string& placementId) const;
		/* Raid-clear award input. Every fought primary boss advances its players' fight
		   clock each tick; a dying gate boss hands its ledger to the room, and the clear
		   sends the room ledger to every player and empties it. */
		void Tick_MvpLedgers();
		void Merge_MvpLedger(SERVER_WORLD_ENTITY& boss);
		void Broadcast_RaidMvpResult(std::uint8_t iGate);

		struct RAID_ENTRY_PROPOSAL
		{
			std::uint32_t iProposalId = 0u;
			std::uint32_t iPartyId = 0u;
			std::uint32_t iRequestSequence = 0u;
			LostArk::Shared::RAID_ENTRY_TARGET eTarget =
				LostArk::Shared::RAID_ENTRY_TARGET::VALTAN;
			std::string strNpcPlacementId;
			std::vector<LostArk::Shared::PLAYER_ID> Voters;
			std::vector<LostArk::Shared::PLAYER_ID> Accepted;
			std::uint32_t iDeadlineTick = 0u;
		};

		// 리더/솔로가 입장하기로 발의 -> 대상 전원(솔로는 본인)에게 프롬프트, 투표 개시.
		void Handle_RaidEntryPropose(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_RAID_ENTRY_PROPOSE& request);
		// 개별 수락/거절 반영. 거절이면 즉시 DECLINED 종료, 전원 수락이면 ALL_ACCEPTED 종료.
		void Handle_RaidEntryRespond(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_RAID_ENTRY_RESPOND& request);
		// 투표를 result로 종료해 전원에 통지하고 proposal을 제거한다. ALL_ACCEPTED면
		// Stage_PartyWorldTransfer로 batch 전송을 stage하고, stage 실패면 CANCELLED로 낮춘다.
		void Close_RaidEntryVote(
			RAID_ENTRY_PROPOSAL& proposal,
			LostArk::Shared::RAID_ENTRY_VOTE_RESULT result);
		// 진행/종료 S2C_RAID_ENTRY_VOTE를 proposal의 present voters에게 보낸다.
		void Broadcast_RaidEntryVote(
			const RAID_ENTRY_PROPOSAL& proposal, bool bClosed,
			LostArk::Shared::RAID_ENTRY_VOTE_RESULT result);
		// m_iServerTick 기준 만료 proposal을 TIMEOUT으로 닫는다(tick 루프에서 호출).
		void Expire_RaidEntryProposals();
		// playerId가 voter인 열린 proposal을 CANCELLED로 닫는다(이탈/파티 해산 시).
		void Cancel_RaidEntryProposalsInvolving(LostArk::Shared::PLAYER_ID playerId);
		// 검증된 batch 멤버(front=리더)를 기존 SERVER_WORLD_TRANSFER_REQUEST 경로로 stage.
		// 멤버 unavailable/이미 staged면 false(호출자가 투표를 CANCELLED로 닫는다).
		bool Stage_PartyWorldTransfer(
			const std::vector<LostArk::Shared::PLAYER_ID>& batchMemberIds,
			LostArk::Shared::WORLD_ID targetWorldId,
			std::uint32_t requestSequence, const std::string& raidReturnNpcPlacementId,
			const std::string& spawnPlacementOverrideId = {});
		// player가 알려진 Valtan 입장 guide NPC 근처(proximity)인지 검증한다.
		bool Is_PlayerNearValtanEntryNpc(
			const SERVER_PLAYER& player, const std::string& npcPlacementId) const;
		/* Tells every session in this room that an authored world sequence
		   instance started. Presentation only: the Server keeps no sequence
		   state, so a session that joins later simply misses a played edge.
		   False rejects the action without consuming its trigger or moving players. */
		bool Broadcast_WorldSequencePlay(
			const std::string& instanceId, float playbackSpeed = 1.f,
			float positionOffsetX = 0.f, float positionOffsetY = 0.f, float positionOffsetZ = 0.f,
			std::uint32_t durationMs = 0u, const std::string& targetSequenceInstanceId = {},
			LostArk::Shared::WORLD_SEQUENCE_OPERATION operation = LostArk::Shared::WORLD_SEQUENCE_OPERATION::PLAY);
		void Handle_DebugKillGateBosses(SESSION_ID sessionId, const LostArk::Shared::C2S_DEBUG_KILL_GATE_BOSSES& request);
		LostArk::Shared::DEBUG_KILL_GATE_BOSSES_RESULT Apply_DebugKillGateBosses(
			SESSION_ID sessionId, const LostArk::Shared::C2S_DEBUG_KILL_GATE_BOSSES& request, std::uint8_t& killedCount);
		std::unordered_map<SESSION_ID, std::uint32_t> m_KillGateBossesRequestSequences;
		void Handle_SetCooldownMode(SESSION_ID sessionId, const LostArk::Shared::C2S_SET_COOLDOWN_MODE& request);
		LostArk::Shared::SET_COOLDOWN_MODE_RESULT Apply_SetCooldownMode(
			SESSION_ID sessionId, const LostArk::Shared::C2S_SET_COOLDOWN_MODE& request);
		LostArk::Shared::COOLDOWN_MODE m_eCooldownMode = LostArk::Shared::COOLDOWN_MODE::DEBUG_THREE_SECONDS;
		std::unordered_map<SESSION_ID, std::uint32_t> m_CooldownModeRequestSequences;
		void Handle_DebugWorldPlayback(SESSION_ID sessionId, const LostArk::Shared::C2S_DEBUG_WORLD_PLAYBACK& request);
		std::unordered_map<SESSION_ID, std::uint32_t> m_WorldPlaybackRequestSequences;
		LostArk::Shared::DEBUG_WORLD_PLAYBACK_RESULT Apply_DebugRoomPlayerArrival(
			SESSION_ID sessionId, const LostArk::Shared::C2S_DEBUG_WORLD_PLAYBACK& request);
		LostArk::Shared::DEBUG_TELEPORT_RESULT Validate_DebugTeleportDestination(
			const SERVER_PLAYER& player, const LostArk::Shared::C2S_DEBUG_TELEPORT_TO_POSITION& request,
			SERVER_NAV_POINT& ground, LostArk::Shared::NET_ENTITY_ID ignoredBodyId = LostArk::Shared::INVALID_NET_ENTITY_ID);
		struct ROOM_PLAYER_ARRIVAL_RUN final
		{
			std::uint32_t iEpoch = 0u;
			std::string strRootPatternId;
			std::vector<std::pair<LostArk::Shared::PLAYER_ID, SESSION_ID>> Players;
			std::map<std::string, LostArk::Shared::DEBUG_WORLD_PLAYBACK_RESULT> Occurrences;
		};
		std::unordered_map<SESSION_ID, ROOM_PLAYER_ARRIVAL_RUN> m_RoomPlayerArrivalRuns;
		/* Offers or withdraws one interact-gated box for the one player it
		   concerns. Unlike the sequence broadcast this is never room-wide. */
		void Send_InteractPrompt(const SERVER_INTERACT_PROMPT_EDGE& edge);
		/* The spawn-group activation every trigger path shares: starts a dormant
		   group, restarts a finished one once its monsters are gone, and never
		   stacks a wave on a group that is still running. */
		bool Activate_SpawnGroupFromTrigger(const std::string& spawnGroupId);
		/* What one trigger action does in this room, whether the box fired because
		   the player stepped in (a scripted flow) or pressed G inside it. */
		bool Activate_TriggerTarget(
			WORLD_TRIGGER_ACTION_KIND kind, const std::string& targetId);
		/* Leave() calls this so a disconnecting player does not linger as a
		   ghost roster entry for whoever they partied with. */
		void Remove_FromParty(LostArk::Shared::PLAYER_ID playerId);
		/* Same room-scoped broadcast Broadcast_PartyRoster already uses --
		   every current session in this room receives the relayed line,
		   including the sender (its own head bubble is driven off the same
		   S2C_CHAT the rest of the room gets, not a second local-only path). */
		void Handle_RoomPing(SESSION_ID sessionId, const LostArk::Shared::C2S_ROOM_PING& request);
		void Handle_Chat(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_CHAT& request);

		bool Send_Accepted(
			const std::shared_ptr<CClientSession>& session,
			const SERVER_PLAYER& player);
		bool Send_EnterRejected(
			const std::shared_ptr<CClientSession>& session,
			LostArk::Shared::ENTER_WORLD_REJECTION_REASON reason);
		bool Send_Spawned(
			const std::shared_ptr<CClientSession>& session,
			const SERVER_PLAYER& player);
		static bool Build_WorldEntitySpawnedPayload(
			const SERVER_WORLD_ENTITY& entity,
			std::vector<std::uint8_t>& outPayload);
		bool Send_WorldEntitySpawned(
			const std::shared_ptr<CClientSession>& session,
			const SERVER_WORLD_ENTITY& entity);
		bool Send_WorldEntityDespawned(
			const std::shared_ptr<CClientSession>& session,
			LostArk::Shared::NET_ENTITY_ID netEntityId,
			LostArk::Shared::WORLD_ENTITY_DESPAWN_REASON reason =
				LostArk::Shared::WORLD_ENTITY_DESPAWN_REASON::REMOVED);
		bool Send_CombatObjectSpawned(
			const std::shared_ptr<CClientSession>& session,
			const LostArk::Shared::S2C_COMBAT_OBJECT_SPAWNED& spawned);
		bool Send_WorldEntitySpawnResult(
			const std::shared_ptr<CClientSession>& session,
			const std::string& placementId,
			LostArk::Shared::WORLD_ENTITY_SPAWN_RESULT result,
			LostArk::Shared::NET_ENTITY_ID netEntityId);
		bool Send_ValtanAuditionResult(
			const std::shared_ptr<CClientSession>& session,
			const LostArk::Shared::C2S_VALTAN_AUDITION_REQUEST& request,
			LostArk::Shared::VALTAN_AUDITION_RESULT result,
			std::uint32_t currentHealthBar);
		bool Send_KoukuSaydonPatternAuditionResult(
			const std::shared_ptr<CClientSession>& session,
			const LostArk::Shared::
				S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_RESULT& message);
		bool Send_ValtanPatternFlowResult(
			const std::shared_ptr<CClientSession>& session,
			std::uint32_t commandSequence,
			LostArk::Shared::VALTAN_PATTERN_FLOW_COMMAND command,
			LostArk::Shared::VALTAN_PATTERN_FLOW_RESULT result,
			const std::string& flowId,
			const std::string& flowRevision,
			std::uint32_t roomFlowEpoch,
			const LostArk::Shared::GameplayDataRevision& pinnedRevision,
			const std::string& reason);
		bool Build_RequiredPinnedGameplayRevisions(
			std::vector<LostArk::Shared::GameplayDataRevision>&
				outRevisions) const;
		[[nodiscard]] const CGameplayCatalog* Resolve_ValtanGameplayCatalog(
			const SERVER_WORLD_ENTITY& boss) const noexcept;
		bool Send_CharacterClassChangeResult(
			const std::shared_ptr<CClientSession>& session,
			const LostArk::Shared::C2S_CHANGE_CHARACTER_CLASS& request,
			LostArk::Shared::CHARACTER_CLASS_CHANGE_RESULT result,
			LostArk::Shared::CHARACTER_CLASS_ID activeClass);
		// Single-session send: inventory is per-player state, not room-shared
		// like S2C_WORLD_SNAPSHOT, so it never broadcasts.
		bool Send_InventorySnapshot(
			const std::shared_ptr<CClientSession>& session,
			std::uint32_t requestSequence,
			const SERVER_PLAYER& player);
		bool Send_Despawned(
			const std::shared_ptr<CClientSession>& session,
			LostArk::Shared::NET_ENTITY_ID netEntityId,
			LostArk::Shared::PLAYER_DESPAWN_REASON reason);
		bool Send_WorldDestructionFullSync(
			const std::shared_ptr<CClientSession>& session);
		// Server-owned collision/navigation counters carried by every
		// destruction message so the Debug audition panel never has to infer
		// passage from the replicated wall states.
		LostArk::Shared::WORLD_DESTRUCTION_RUNTIME_DIAGNOSTICS
			Build_WorldDestructionDiagnostics() const;
		void Broadcast_Spawned(
			const SERVER_PLAYER& player,
			SESSION_ID exceptSessionId);
		void Broadcast_Despawned(
			LostArk::Shared::NET_ENTITY_ID netEntityId,
			LostArk::Shared::PLAYER_DESPAWN_REASON reason);
		void Broadcast_WorldEntitySpawned(
			const SERVER_WORLD_ENTITY& entity);
		void Broadcast_WorldEntityDespawned(
			LostArk::Shared::NET_ENTITY_ID netEntityId,
			LostArk::Shared::WORLD_ENTITY_DESPAWN_REASON reason =
				LostArk::Shared::WORLD_ENTITY_DESPAWN_REASON::REMOVED);
		bool Broadcast_WorldDestructionDelta(
			const std::vector<WORLD_DESTRUCTION_STATE_TRANSITION>& transitions,
			const std::vector<LostArk::Shared::WORLD_DESTRUCTION_EVENT_WIRE>&
				liveEvents,
			std::uint32_t serverTick);
		void Broadcast_WorldSnapshot();

		std::shared_ptr<CClientSession> Find_Session(
			SESSION_ID sessionId) const;
		void Rollback_Join(SESSION_ID sessionId);
		[[nodiscard]] bool Is_PlayerAdmissionFull() const;
		const WORLD_BOOTSTRAP_PLACEMENT* Find_AvailablePlayerSpawn() const;
		const WORLD_BOOTSTRAP_PLACEMENT* Find_Placement(
			const std::string& placementId) const;
		bool Build_WorldEntity(
			const WORLD_BOOTSTRAP_PLACEMENT& placement,
			LostArk::Shared::NET_ENTITY_ID netEntityId,
			SERVER_WORLD_ENTITY& outEntity,
			const CGameplayCatalog* definitionCatalog = nullptr,
			LostArk::Shared::NET_ENTITY_ID ownerBossNetEntityId =
				LostArk::Shared::INVALID_NET_ENTITY_ID, std::uint32_t ownerPatternSequence = 0u);
		bool Initialize_WorldEntities();
		bool Reset_ReplayableArenaWhenEmpty();
		bool Reset_ValtanArenaWhenEmpty();
		bool Apply_BossPatternStageActions(
			SERVER_WORLD_ENTITY& boss,
			const std::string& patternId,
			const std::string& actionId,
			BOSS_PATTERN_STAGE_ACTION_TRIGGER trigger,
			std::uint32_t serverTick,
			std::uint32_t spawnWaveOrdinal = 0u,
			bool scheduledSpawnWave = false);
		bool Apply_BossPatternScheduledSpawnWave(
			SERVER_WORLD_ENTITY& boss,
			std::uint32_t serverTick);
		bool Apply_BossPatternStageTransition(
			SERVER_WORLD_ENTITY& boss,
			const std::string& previousPatternId,
			const std::string& previousActionId,
			const std::string& nextPatternId,
			const std::string& nextActionId,
			const LostArk::Shared::GameplayDataRevision&
				previousDefinitionRevision,
			const LostArk::Shared::GameplayDataRevision& nextDefinitionRevision,
			std::uint32_t serverTick);
		bool Stage_BossPatternStageActions(
			const SERVER_WORLD_ENTITY& boss,
			const CGameplayCatalog& catalog,
			const std::string& patternId,
			const std::string& actionId,
			BOSS_PATTERN_STAGE_ACTION_TRIGGER trigger,
			std::uint32_t serverTick,
			SERVER_BOSS_COMBAT_STATE& stagedCombat,
			std::uint8_t& stagedGameplayPhase,
			SERVER_COMBAT_OBJECT_TRANSACTION& combatObjectTransaction,
			std::uint32_t spawnWaveOrdinal = 0u,
			bool scheduledSpawnWave = false);
		/* Runs only after every stage-action preflight transaction commits. These
		actions own player/target state and therefore cannot be staged inside the
		boss-combat or combat-object value transactions above. */
		bool Prepare_GrabbedPlayerImpact(
			const SERVER_WORLD_ENTITY& boss,
			const CGameplayCatalog& catalog,
			const BOSS_PATTERN_STAGE_ACTION& action,
			std::uint32_t serverTick,
			std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& stagedPlayers,
			std::vector<LostArk::Shared::DAMAGE_EVENT>& stagedDamageEvents);
		SERVER_PLAYER* Select_BossRandomAliveTarget(const SERVER_WORLD_ENTITY& boss,
			const std::string& actionId, const std::string& targetId, std::uint32_t serverTick);
		bool Commit_BossPatternPlayerStageActions(
			SERVER_WORLD_ENTITY& boss,
			const CGameplayCatalog& catalog,
			const std::string& patternId,
			const std::string& actionId,
			BOSS_PATTERN_STAGE_ACTION_TRIGGER trigger,
			std::uint32_t serverTick,
			std::uint32_t spawnWaveOrdinal = 0u);
		bool Resolve_ArenaRandomVolleyOrigins(
			const SERVER_WORLD_ENTITY& boss,
			const BOSS_PATTERN_STAGE_ACTION& action,
			const BOSS_COMBAT_OBJECT_DEFINITION& definition,
			std::uint32_t spawnWaveOrdinal,
			std::vector<SERVER_COMBAT_OBJECT_LOCKED_TARGET>& outOrigins,
			float explicitMinimumSpacingM = 0.f, const SERVER_NAV_POINT* anchorOverride = nullptr);
		bool Broadcast_CombatObjectLifecycle();
		void Drain_BossCombatEvents();
		bool Apply_WorldDestructionStageEntry(
			const SERVER_WORLD_ENTITY& boss,
			std::uint32_t serverTick);
		/* Commit the 69 ordinary contact walls and the 30 outer ring walls in one
		transaction, leaving every floor sector INTACT. A floor-collapse bar only
		arrives after the fight has already taken those walls down, so the
		audition for such a bar has to clear them inside the same atomic request
		instead of a second one the boss could start a pattern between. */
		bool Break_EveryWallForAudition(
			const SERVER_WORLD_ENTITY& boss,
			std::uint32_t resetTick,
			std::string& status);
		/* The navigation grid is the ground a boss pattern stride may cross.
		The collision sweep owns wall contact, while the furthest sample the grid
		still owns is what any stride is allowed to reach, so a charge cannot
		leave the floor before its wall contact is evaluated. A start the grid
		already refuses passes through
		unchanged, because refusing it there would strand the boss for good. */
		static void Resolve_NavigableStep(
			const CServerNavigation& navigation,
			float fromX,
			float fromZ,
			float targetX,
			float targetZ,
			float& outX,
			float& outZ);
		/* Raise the pillar slots on the authored stage edge of the pattern that
		owns them. The shatter has no identified product owner yet. */
		bool Apply_EncounterPropStageEntry(
			const SERVER_WORLD_ENTITY& boss,
			std::uint32_t serverTick);
		bool Commit_DueEncounterProps(std::uint32_t serverTick);
		bool Initialize_WorldPickups();
		void Reset_WorldPickups(std::uint32_t serverTick);
		void Update_WorldPickups(std::uint32_t serverTick, bool allowCollection = true);
		void Apply_WorldPickupDestruction(const WORLD_DESTRUCTION_TRANSACTION& transaction,
			std::uint32_t serverTick);
		void Remove_RemainingWorldPickups(std::uint32_t serverTick);
		bool Send_EncounterPropSync(
			const std::shared_ptr<CClientSession>& session);
		void Broadcast_EncounterPropSync();
		/* Break whatever a non-impact boss body physically reached between its
		previous and current position. A charge-impact stage bypasses this generic
		pass and owns one exact swept wall transaction: impact receiver first,
		then the co-located ordinary contact binding. */
		bool Apply_WorldDestructionBodyContact(
			SERVER_WORLD_ENTITY& boss,
			float previousX,
			float previousY,
			float previousZ,
			std::uint32_t serverTick);
		bool Apply_WorldDestructionPatternHitContact(
			SERVER_WORLD_ENTITY& boss,
			std::uint32_t serverTick);
		bool Apply_WorldDestructionContacts(
			SERVER_WORLD_ENTITY& boss,
			const std::vector<std::string>& contactPlacementIds,
			std::uint32_t serverTick);
		bool Apply_WorldDestructionImpact(
			SERVER_WORLD_ENTITY& boss,
			const std::string& receiverPlacementId,
			std::uint32_t serverTick,
			bool& outTriggered);
		bool Commit_WorldDestructionTransaction(
			const WORLD_DESTRUCTION_TRANSACTION& transaction,
			const std::vector<LostArk::Shared::WORLD_DESTRUCTION_EVENT_WIRE>&
				liveEvents,
			std::uint32_t serverTick,
			std::string& status);
		void Invalidate_DynamicNavigationPaths();
		bool Build_WorldDestructionLiveEvents(
			const WORLD_DESTRUCTION_TRANSACTION& transaction,
			const SERVER_WORLD_ENTITY& boss,
			std::vector<LostArk::Shared::WORLD_DESTRUCTION_EVENT_WIRE>&
				liveEvents,
			std::string& status) const;
		bool Commit_DueWorldDestruction(std::uint32_t serverTick);
		bool Activate_Encounter(const std::string& placementId);
		bool Spawn_Monster(
			const std::string& spawnGroupId,
			const SPAWN_GROUP_ENTRY& entry,
			const SPAWN_GROUP_ANCHOR& anchor,
			const MONSTER_RUNTIME_PROFILE& profile,
			std::uint32_t ordinal);
		/* Card maze. The telescope claim deals the suits and raises the
		targets; the MAZE hammer press judges its swing once, at the runtime's
		hit tick, against those targets. */
		bool Spawn_KoukuCardRainSoldiers(LostArk::Shared::NET_ENTITY_ID ownerId, std::uint32_t tick,
			const BOSS_PATTERN_MECHANIC_TRIGGER* tuning = nullptr);
		void Update_KoukuCardRainSoldiers(std::uint32_t tick);
		bool Begin_CardMaze(LostArk::Shared::PLAYER_ID claimantId);
		void Reset_CardMaze();
		void Despawn_CardMazeTargets();
		void Resolve_CardMazeHammerHit(SERVER_PLAYER& player, std::uint32_t updateTick);
		void Resolve_MarioHammerHit(SERVER_PLAYER& player, std::uint32_t updateTick);
		void Update_MarioBombContacts(SERVER_PLAYER& player, std::uint32_t updateTick);
		void Update_MarioBouncingBallContacts(SERVER_PLAYER& player, std::uint32_t updateTick);
		// Rotating cannon jets and the big mokoko waterfall of a live Waterpang match.
		void Update_MaharakaWaterpangHazards(SERVER_PLAYER& player, std::uint32_t updateTick);
        void Handle_MaharakaAITuning(SESSION_ID sessionId, const LostArk::Shared::C2S_MAHARAKA_AI_TUNING& request);
        bool Spawn_MaharakaWaterpangAI();
        void Update_MaharakaWaterpangMatch(std::uint32_t updateTick);
        void Clear_MaharakaWaterpangAI(std::uint32_t keepCount = 0u);
        bool Finish_MaharakaWaterpangMatch();
		// The running Debug forced event first, else the match schedule; false when neither runs.
		bool Sample_MaharakaWaterpangNow(std::uint32_t tick, LostArk::Shared::MAHARAKA_WATERPANG_EVENT_SAMPLE& out) const;
		// The Server's water gun arming rule: replicated as PLAYER_SNAPSHOT.isWaterpangArmed.
		bool Is_MaharakaWaterpangArmed(const SERVER_PLAYER& player) const;
		// Water gun Q/W/E/R cast of an armed body: cooldown, no movement lock, shot scheduled.
		bool Try_StartMaharakaWaterGunSkill(SERVER_PLAYER& player, const LostArk::Shared::C2S_USE_SKILL& command);
		// Flies the scheduled shots and pushes the bodies they strike.
		void Update_MaharakaWaterGunShots(std::uint32_t updateTick);
		// The forced event plus a short tail, so its last push can still leave the deck.
		bool Is_MaharakaWaterpangDebugEventLive(std::uint32_t tick) const;
		// Debug F1 Waterpang pattern button: starts a forced event for the whole room.
		LostArk::Shared::DEBUG_WORLD_PLAYBACK_RESULT Start_MaharakaWaterpangDebugEvent(const std::string& instanceId);
		std::uint8_t Mario_CurseReleasedMask(std::uint8_t stage, std::uint8_t layout) const;
		bool Spawn_CardMazeTarget(const CKoukuCardMazeRuntime::SPAWN_REQUEST& request);
		void Remove_CardMazeTarget(LostArk::Shared::NET_ENTITY_ID id);
		void Update_CardMaze(std::uint32_t tick);
		/* Before the run: raises the clown box for players inside the maze and
		latches its destruction. Clear forgets it and removes a living box. */
		void Update_CardMazeClownBox(std::uint32_t tick);
		void Clear_CardMazeClownBox();
		/* Advances the bingo bomb clock: a mark whose deadline passed is
		planted where its carrier stands, and a carrier that left the room
		takes its mark with it. */
		void Update_KoukuBingo(std::uint32_t tick);
		bool Begin_CardMazeTransfer(SERVER_PLAYER& player, float x, float y, float z,
			std::uint32_t tick, bool leaving);
		std::uint32_t Count_SpawnGroupEntities(
			const std::string& spawnGroupId) const;
		/* 1 unless the player is standing in the stance its identity gauge pays
		for, which is the only thing that changes how fast anyone walks. */
		float Resolve_StanceMoveSpeedScale(const SERVER_PLAYER& player) const;
		/* Hands the living monster and boss bodies to the collision system so this
		tick's player walks and root motion stop at them. */
		void Refresh_PlayerBlockingBodies();
		bool Try_KoukuWalkOffFloor(SERVER_PLAYER& player, float x, float z,
			float fixedDeltaSeconds, std::uint32_t updateTick);
		const WORLD_BOOTSTRAP_PLACEMENT* Resolve_KoukuFallCenter(const SERVER_PLAYER& player) const;
		bool Update_PlayerFall(
			SERVER_PLAYER& player,
			float fixedDeltaSeconds,
			std::uint32_t updateTick);
		void Begin_PlayerFall(
			SERVER_PLAYER& player,
			float fixedDeltaSeconds,
			std::uint32_t updateTick);
		/* Product boss-pattern adapters call these with replicated identities. The
		room owns interruption, fixed-tick fallback motion and release reaction so
		no pattern can leave half of a grabbed player state behind. */
		bool Capture_PlayerAttachment(
			LostArk::Shared::NET_ENTITY_ID playerEntityId,
			LostArk::Shared::NET_ENTITY_ID ownerEntityId,
			LostArk::Shared::PLAYER_ATTACHMENT_SLOT slot,
			std::uint32_t serverTick, std::uint32_t holdEndTick = 0u, std::uint32_t sourcePatternSequence = 0u);
		bool Update_PlayerAttachment(
			SERVER_PLAYER& player,
			std::uint32_t serverTick);
		bool Release_PlayerAttachment(
			SERVER_PLAYER& player,
			LostArk::Shared::NET_ENTITY_ID ownerEntityId,
			float pushRangeM,
			std::uint32_t pushMs,
			bool knockdown,
			std::uint32_t downMs,
			std::uint32_t serverTick);
		std::size_t Release_PlayerAttachments(
			LostArk::Shared::NET_ENTITY_ID ownerEntityId,
			float pushRangeM,
			std::uint32_t pushMs,
			bool knockdown,
			std::uint32_t downMs,
			std::uint32_t serverTick);
		[[nodiscard]] bool Restore_PatternBoundPlayer(SERVER_PLAYER& player);
		void Update_Players(float fixedDeltaSeconds);
		bool Prepare_ArenaEjection(
			SERVER_PLAYER& staged,
			const SERVER_WORLD_ENTITY& boss,
			const BOSS_PATTERN_STAGE_ACTION& action,
			std::uint32_t serverTick);
		bool Resolve_ArenaCenter(
			const SERVER_WORLD_ENTITY& boss,
			SERVER_NAV_POINT& point);
		bool Activate_ValtanGhostPhaseLoop(
			SERVER_WORLD_ENTITY& boss,
			const CGameplayCatalog& catalog);
		bool Begin_ValtanGhostRelocation(
			SERVER_WORLD_ENTITY& boss,
			const CGameplayCatalog& catalog,
			std::uint32_t serverTick);
		bool Update_ValtanGhostPortalScheduler(
			SERVER_WORLD_ENTITY& boss,
			const CGameplayCatalog& catalog,
			std::uint32_t serverTick);
		bool Update_DependentBosses(std::uint32_t serverTick);
		/* Slides a hit player along the armed knockback window, clamped to
		walkable floor and blocking bodies; a wall ends the window early. */
		void Advance_PlayerKnockback(
			SERVER_PLAYER& player, float fixedDeltaSeconds);
		void Update_WorldEntities(float fixedDeltaSeconds);

	private:
		// Best-effort traffic leaves room for gameplay/control commands. LEAVE
		// never shares this bounded queue: disconnect cleanup has its own
		// session-deduplicated priority queue below.
		static constexpr std::size_t MAX_BEST_EFFORT_COMMAND_COUNT = 768u;
		static constexpr std::size_t MAX_RELIABLE_COMMAND_COUNT = 960u;
		static constexpr std::size_t MAX_COMMANDS_DRAINED_PER_TICK = 256u;

		mutable std::mutex m_CommandMutex;
		std::deque<ROOM_COMMAND> m_InboundCommands;
		std::deque<ROOM_COMMAND> m_CleanupCommands;
		std::unordered_set<SESSION_ID> m_QueuedCleanupSessionIds;
		SERVER_ROOM_PERFORMANCE_METRICS m_PerformanceMetrics;
		SERVER_ROOM_PERFORMANCE_METRICS m_LastRoomPerfLogSample;
		std::string m_strPendingPerformanceDiagnostic;
		std::uint64_t m_iLastRoomPerfSnapshotDroppedCount = 0;
		std::uint64_t m_iLastRoomPerfReliableRejectedCount = 0;
		std::uint64_t m_iLastRoomPerfWireSendFailureCount = 0;
		std::size_t m_iLastRoomPerfOutboundHighWatermark = 0u;
		bool m_acceptsCommands = true;
		std::deque<SERVER_WORLD_TRANSFER_REQUEST> m_PendingWorldTransfers;
		/* Bern remembers the ship a session sailed to Maharaka on, so the return trip can put that
		   session back on the same ship at the pier it left from. Bern room only; keyed by session. */
		struct SHIP_RETURN_STATE final
		{
			LostArk::Shared::VEHICLE_ID iVehicleId = LostArk::Shared::INVALID_VEHICLE_ID;
			float fDockX = 0.f;
			float fDockY = 0.f;
			float fDockZ = 0.f;
			float fDockYawDegrees = 0.f;
		};
		std::unordered_map<SESSION_ID, SHIP_RETURN_STATE> m_MaharakaShipReturnBySession;
		void Remember_ShipForWorldTransfer(const SERVER_WORLD_TRANSFER_REQUEST& transfer);
		struct PENDING_ESTHER_SUMMON final
		{
			const ESTHER_ROSTER_ENTRY* pRosterEntry = nullptr;
			LostArk::Shared::PLAYER_ID iCasterPlayerId = LostArk::Shared::INVALID_PLAYER_ID;
			float fPositionX = 0.f;
			float fPositionY = 0.f;
			float fPositionZ = 0.f;
			float fYawDegrees = 0.f;
			float fRemainingSeconds = 0.f;
		};
		std::vector<PENDING_ESTHER_SUMMON> m_PendingEstherSummons;
		struct ESTHER_ZONE_RUNTIME final
		{
			const LostArk::Shared::EstherStrike::ZONE* pZone = nullptr;
			float fPositionX = 0.f;
			float fPositionZ = 0.f;
			std::uint32_t iEndTick = 0u;
			std::uint32_t iNextPulseTick = 0u;
		};
		std::vector<ESTHER_ZONE_RUNTIME> m_EstherZones;

		std::unordered_map<SESSION_ID, std::weak_ptr<CClientSession>> m_Sessions;
		std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER> m_Players;
		struct GUIDE_PROMPT_OCCURRENCE
		{
			std::string PromptId, TriggerId;
			std::size_t NextSegment = 0;
			int Priority = 0;
			bool IsSpaceEnter = false;
			std::vector<std::string> SpaceTriggerIds;
			bool operator==(const GUIDE_PROMPT_OCCURRENCE&) const = default;
		};
		struct GUIDE_RUNTIME
		{
			LostArk::Shared::PLAYER_ID PlayerId = 0, AnchorId = 0;
			std::weak_ptr<CClientSession> OwnerSession;
			LostArk::Shared::NET_ENTITY_ID OwnerNetEntityId = 0;
			bool WaitingForOwner = false, WaitingForShip = false, ReturningOnFoot = false;
			std::string ComboId, PendingComboId, Reason;
			std::size_t ComboStep = 0;
			float ThinkElapsed = 0.f, ComboElapsed = 0.f, StepElapsed = 0.f, FarElapsed = 0.f, HoldElapsed = 0.f, PromptRemaining = 0.f;
			std::uint32_t Sequence = 0;
            std::map<std::string, std::uint32_t> CommandTicks;
			std::uint8_t Action = 0;
            float FollowScore = 0.f, EvadeScore = 0.f, CombatScore = 0.f;
			std::deque<GUIDE_PROMPT_OCCURRENCE> PromptQueue;
			std::map<std::string, std::uint32_t> TriggerTicks;
			std::unordered_set<std::string> InsideBoxes;
			std::map<LostArk::Shared::NET_ENTITY_ID, std::uint32_t> PatternSequences;
		};
		CGuideCatalog m_GuideCatalog;
		// At most one owner binds the one placed Bern guide; no companion clone or party slot.
		std::map<SESSION_ID, GUIDE_RUNTIME> m_PersonalGuides;
		std::uint32_t m_iGuideEventSequence = 0;
		float m_fGuideOwnershipElapsed = 0.f;
		std::unordered_map<SESSION_ID, std::uint32_t> m_GuideControlSequences;
		LostArk::Shared::PLAYER_ID m_iGuideReceptionId = 0;
        LostArk::Shared::PLAYER_ID m_iNextGuidePlayerId = 0x80000000u;
		/* Grants what a started skill buffs, to the caster, the party in this room
		or the entities it targets. */
		void Apply_SkillBuffs(SERVER_PLAYER& caster, std::uint32_t skillId,
			std::uint32_t serverTick);
		std::unordered_map<SESSION_ID, LostArk::Shared::PLAYER_ID>
			m_PlayerIdBySessionId;
		std::unordered_map<LostArk::Shared::NET_ENTITY_ID, LostArk::Shared::PLAYER_ID>
			m_PlayerIdByEntityId;

		/* Same-room party state -- PLAYER_ID is room-local (freshly allocated
		   per room on Join), so this map does not by itself survive a member
		   moving to a different room. 0 means "no party" -- never a real party
		   ID. Invite/accept/join only; leave/kick/leader promotion is a
		   separate follow-up.
		   A party-leader-triggered group Valtan entry (Handle_ConfirmNpcEntry
		   -> Transfer_PartyTo) is the one case
		   that does survive a room change: every member transfers together in
		   one batch and gets re-grouped into a fresh room-local party in the
		   target room, so the party itself is never actually split across two
		   rooms at once. There is still no general cross-room party identity
		   (e.g. inviting or chatting with someone in a different room). */
		std::uint32_t m_iNextPartyId = 1u;
		std::unordered_map<LostArk::Shared::PLAYER_ID, std::uint32_t>
			m_PartyIdByPlayerId;
		std::unordered_map<std::uint32_t, std::vector<LostArk::Shared::PLAYER_ID>>
			m_PartyMembersByPartyId;
		// One pending invite per target at a time; a new invite silently
		// replaces whatever that target's last unanswered invite was.
		std::unordered_map<LostArk::Shared::PLAYER_ID, LostArk::Shared::PLAYER_ID>
			m_PendingPartyInviteByTargetPlayerId;
		// At most one latest failure per present player. A full reliable queue
		// delays the notice instead of disconnecting a rejected source party.
		std::unordered_map<SESSION_ID, LostArk::Shared::S2C_PARTY_TRANSFER_RESULT>
			m_PendingPartyTransferResults;

		// 파티 레이드 입장 투표 상태. struct RAID_ENTRY_PROPOSAL은 위 메서드 선언부에 정의한다.
		std::vector<RAID_ENTRY_PROPOSAL> m_RaidEntryProposals;
		std::uint32_t m_iNextRaidEntryProposalId = 1u;
		// Colosseum match queue: accepted sessions in join order (BERN room only).
		struct COLOSSEUM_QUEUE_ENTRY
		{
			SESSION_ID iSessionId = INVALID_SESSION_ID;
			std::uint32_t iRequestSequence = 0u;
		};
		std::vector<COLOSSEUM_QUEUE_ENTRY> m_ColosseumQueue;
		bool m_bColosseumTransferPending = false;
		std::uint32_t m_iColosseumRetryTick = 0u;
		std::uint64_t m_iColosseumMatchId = 0u;
		LostArk::Shared::COLOSSEUM_MATCH_PHASE m_eColosseumPhase = LostArk::Shared::COLOSSEUM_MATCH_PHASE::RECRUITING;
		std::uint8_t m_iColosseumWinnerTeam = 255u;
		std::uint32_t m_iColosseumRevision = 1u;
		std::array<std::uint32_t, 2> m_ColosseumTeamPartyIds{};
		enum class COLOSSEUM_TACTIC : std::uint8_t { WAIT, ENGAGE, REPOSITION, RETREAT, EVADE, RECOVER };
		struct COLOSSEUM_MERCENARY_RUNTIME final
		{
			float fThinkElapsed = 0.f;
			float fSenseElapsed = 0.f, fDecisionInterval = .2f, fThreatAgeSeconds = 0.f;
			std::uint32_t iSequence = 0u;
			// Match-owned admission clock survives player respawn and tactical resets.
			std::optional<std::uint32_t> iLastAltVAdmissionTick;
			std::uint64_t iRandomState = 0u;
			std::vector<LostArk::Shared::SKILL_ID> AvailableSkills;
			std::array<LostArk::Shared::SKILL_ID, 3> RecentSkills{};
			std::size_t iRecentSkillCursor = 0u;
			LostArk::Shared::SKILL_ID iLastSkillId = LostArk::Shared::INVALID_SKILL_ID;
			LostArk::Shared::NET_ENTITY_ID iTargetEntityId = LostArk::Shared::INVALID_NET_ENTITY_ID;
			std::uint32_t iTargetSelectedTick = 0u, iTacticUntilTick = 0u;
			std::uint32_t iNextAttackTick = 0u, iNextEvadeTick = 0u, iRetreatAllowedTick = 0u;
			std::uint32_t iLastDamageTick = 0u, iObservedHp = 0u;
			COLOSSEUM_TACTIC eTactic = COLOSSEUM_TACTIC::ENGAGE;
			CColosseumThreatAssessment Threats;
			const char* pReason = "Waiting for recruitment";
		};
		std::map<LostArk::Shared::PLAYER_ID, COLOSSEUM_MERCENARY_RUNTIME> m_ColosseumMercenaries;
		std::unordered_map<SESSION_ID, std::uint32_t> m_ColosseumRecruitSequences;
		std::vector<SESSION_ID> m_ColosseumSessions;
		std::uint32_t m_iColosseumPhaseStart = 0u, m_iColosseumPhaseEnd = 0u;
		std::uint32_t m_iColosseumScores[2]{};
		std::uint32_t m_iColosseumKillSequence = 0u;
		std::vector<LostArk::Shared::COLOSSEUM_KILL_EVENT> m_ColosseumRecentKills;
		std::uint32_t m_iColosseumQueueDeadline = 0u, m_iColosseumQueueBroadcastTick = 0u;
		std::array<std::uint8_t, 2> m_ColosseumInitialHumans{};
		GATE_PROGRESS_STATE m_GateProgress;
		std::uint32_t m_iArenaAssemblyStartTick = 0u;
		std::uint32_t m_iArenaAssemblyRaidEpoch = 0u;
		bool m_bArenaAssemblyAttempted = false;
		std::vector<LostArk::Shared::PLAYER_ID> m_ArenaAssemblyParticipants;
		std::vector<SERVER_MVP_LEDGER_ROW> m_GateMvpLedger;
		std::uint32_t m_iNextGateProposalId = 1u;

		LostArk::Shared::WORLD_ID m_eWorldId = LostArk::Shared::WORLD_ID::END;
		CWorldBootstrap m_WorldBootstrap;
		CGameplayCatalogGenerations m_GameplayCatalog;
		std::vector<LostArk::Shared::BALANCE_NUMERIC_ENTRY> m_StagedNumericEntries;
		std::vector<std::pair<std::shared_ptr<const CGameplayCatalog>, std::shared_ptr<const CGameplayCatalog>>>
			m_StagedNumericCatalogRemaps;
		CItemCatalog m_ItemCatalog;
		CVehicleCatalog m_VehicleCatalog;
		CHonorTitleCatalog m_HonorTitleCatalog;
		CValtanClearRewards m_ValtanClearRewards;
		CServerNavigation m_ServerNavigation;
		CServerCollisionSystem m_ServerCollisionSystem;
		CServerTriggerSystem m_ServerTriggerSystem;
        struct MAHARAKA_WATERPANG_AI final
        {
            std::uint32_t iSlot = 0u, iSequence = 0u, iNextThinkTick = 0u, iNextMoveTick = 0u, iNextShotTick = 0u, iSkillSlot = 0u;
        };
        std::map<LostArk::Shared::PLAYER_ID, MAHARAKA_WATERPANG_AI> m_MaharakaWaterpangAI;
        LostArk::Shared::PLAYER_ID m_iNextWaterpangAIPlayerId = 0x90000000u;
        std::uint32_t m_iWaterpangAIRetryTick = 0u;
        LostArk::Shared::MAHARAKA_AI_TUNING m_MaharakaAITuning;
        bool m_bMaharakaAITuningLoaded = false;
        std::string m_strMaharakaAISourceBytes;
		// One room-wide scheduled intro, retained for late join until the room empties.
		std::optional<LostArk::Shared::S2C_WORLD_SEQUENCE_PLAY> m_MaharakaWaterpangIntro;
		// A Waterpang water gun shot from cast to burst; bodies already struck are remembered.
		struct MAHARAKA_WATERGUN_SHOT final
		{
			LostArk::Shared::PLAYER_ID iOwnerId = 0;
			std::uint32_t iSkillId = 0u;
			std::uint32_t iSpawnTick = 0u;
            std::uint32_t iProjectileIndex = 0u;
            LostArk::Shared::COMBAT_OBJECT_ID iVisualObjectId = LostArk::Shared::INVALID_COMBAT_OBJECT_ID;
            LostArk::Shared::NET_ENTITY_ID iSourceNetEntityId = LostArk::Shared::INVALID_NET_ENTITY_ID;
			float fAimDistanceM = 0.f;
			bool bLaunched = false;
			bool bSpent = false;
			float fX = 0.f, fY = 0.f, fZ = 0.f;
			float fDirX = 0.f, fDirZ = 1.f;
			float fTravelM = 0.f;
			float fReachM = 0.f;
			std::vector<LostArk::Shared::PLAYER_ID> Struck;
		};
		std::vector<MAHARAKA_WATERGUN_SHOT> m_MaharakaWaterGunShots;
		// Debug forced waterfall/cannon broadcast; replaced by the next press, kept for late join.
		std::optional<LostArk::Shared::S2C_WORLD_SEQUENCE_PLAY> m_MaharakaWaterpangDebugEvent;
		CSpawnGroupBootstrap m_SpawnGroupBootstrap;
		CSpawnGroupRuntime m_SpawnGroupRuntime;
		std::mt19937 m_MarioLayoutRandom{std::random_device{}()};
		std::mt19937 m_EquipmentUpgradeRandom{std::random_device{}()};
		// Popped source-ball slots per Mario stage (index 1..4), bit = bootstrap slot.
		std::uint16_t m_MarioPoppedBalls[5] = {};
		std::uint8_t m_iNextMarioEntryStage = 1u;
		struct KOUKU_CARD_RAIN_SOLDIER_STATE final
		{
			LostArk::Shared::NET_ENTITY_ID ownerId = LostArk::Shared::INVALID_NET_ENTITY_ID;
			std::uint32_t patternSequence = 0u, expiresAt = 0u;
		};
		std::map<LostArk::Shared::NET_ENTITY_ID, KOUKU_CARD_RAIN_SOLDIER_STATE> m_KoukuCardRainSoldiers;
		CKoukuCardMazeRuntime m_KoukuCardMaze;
		CKoukuBingoRuntime m_KoukuBingo;
        struct KOUKU_BINGO_DURATION final
        {
            LostArk::Shared::NET_ENTITY_ID iOwnerId = LostArk::Shared::INVALID_NET_ENTITY_ID;
            std::uint32_t iPatternSequence = 0u, iEndTick = 0u;
            std::uint32_t iNextBombTick = 0u, iNextHammerTick = 0u, iNextMadnessTick = 0u;
            std::uint32_t iMarkedBombCount = 0u;
            float fHammerHalfForwardM = 0.f, fHammerHalfWidthM = 0.f;
            bool bEncounterOwned = false, bSpecialPatternPending = false;
            bool bLastLineCompletionSucceeded = false;
            bool bLineRewardSinceLastJudgement = false;
            std::uint32_t iLastLineJudgementTick = 0u;
            struct HAMMER { std::int32_t anchor = -1; std::uint32_t startTick = 0u; };
            std::array<HAMMER, 2u> Hammers{};
        } m_KoukuBingoDuration;
        std::uint32_t m_iKoukuBingoBoardEpoch = 0u;
        void Begin_KoukuBingoDuration(const SERVER_WORLD_ENTITY& owner,
            const BOSS_PATTERN_MECHANIC_TRIGGER& trigger, std::uint32_t tick);
        void Stop_KoukuBingoDuration(bool clearBoard);

		std::uint32_t m_iCardMazeMarchStartTick = 0u;
		std::uint32_t m_iCardMazeCycleMs = 0u;
		std::map<LostArk::Shared::PLAYER_ID, std::pair<float, float>> m_CardMazePreviousPositions;
		std::map<LostArk::Shared::PLAYER_ID, std::uint32_t> m_CardMazeContactTicks;
		LostArk::Shared::NET_ENTITY_ID m_iCardMazeClownBoxId = LostArk::Shared::INVALID_NET_ENTITY_ID;
		std::uint32_t m_iCardMazeClownBoxDueTick = 0u;
		bool m_bCardMazeClownBoxDestroyed = false;
		CPlayerSkillSystem m_PlayerSkillSystem;
		CCombatObjectRuntime m_CombatObjectRuntime;
		CMonsterBrain m_MonsterBrain;
		CNpcBehaviorRuntime m_NpcBehaviorRuntime;
		CValtanBrain m_ValtanBrain;
		CKoukuSaydonBrain m_KoukuSaydonBrain;
		std::unique_ptr<CValtanBrain> m_DependentValtanBrain =
			std::make_unique<CValtanBrain>();
		VALTAN_DECISION_TRACE_REVISION_STATE m_ValtanDecisionTraceRevision;
		CEstherSkillSystem m_EstherSkillSystem;
		CWorldDestructionBootstrap m_WorldDestructionBootstrap;
		CWorldDestructionRuntime m_WorldDestructionRuntime;
		struct WORLD_PICKUP_RUNTIME final
		{
			WORLD_PICKUP_DESCRIPTOR Descriptor;
			LostArk::Shared::WORLD_PICKUP_SNAPSHOT Snapshot;
		};
		std::vector<WORLD_PICKUP_RUNTIME> m_WorldPickups;
		std::uint32_t m_iWorldPickupEncounterEpoch = 0u;
		/* The four pillars come back four times in one fight, so they live in a
		reversible prop runtime instead of a one-way destruction group. */
		CEncounterPropRuntime m_EncounterPropRuntime;
		/* Room-authoritative completion latch. The primary Product Valtan death
		   raises it before that entity is reliably despawned; the last-player reset
		   clears it for the next party. */
		bool m_bValtanRaidCleared = false;
		/* Debug audition only: the tick a whole pillar cycle shatters on, and
		the flag the next raise turns into that tick. No product trigger for the
		shatter is identified yet, so nothing else writes these. */
		std::uint32_t m_iPillarAuditionBreakTick = 0u;
		bool m_bPillarAuditionCycleArmed = false;
		std::vector<SERVER_WORLD_ENTITY> m_WorldEntities;
		/* One tick's resolved hits. Cleared at the top of every simulation phase
		and consumed by Broadcast_WorldSnapshot, so an event can only ever ride
		the snapshot of the tick that produced it. */
		std::vector<LostArk::Shared::DAMAGE_EVENT> m_TickDamageEvents;
		/* Damage-text events raised while draining room commands, which happens before
		m_TickDamageEvents is cleared for the tick. Moved in right after that clear so a
		potion heal reaches the same broadcast as a combat hit. */
		std::vector<LostArk::Shared::DAMAGE_EVENT> m_PendingCommandDamageEvents;
		std::vector<LostArk::Shared::BOSS_COMBAT_EVENT>
			m_TickBossCombatEvents;
		std::string m_strStatus;
		SERVER_ROOM_RUNTIME_FAILURE m_RuntimeFailure;
		bool m_isReady = false;

		LostArk::Shared::PLAYER_ID m_iNextPlayerId = 1;
		LostArk::Shared::NET_ENTITY_ID m_iNextNetEntityId = 100;
		std::uint32_t m_iServerTick = 0;
		std::uint64_t m_iNextWorldDestructionEventSequence = 1u;
		std::uint64_t m_iNextBossCombatEventSequence = 1u;
		/* Debug Valtan audition. The armed bar is the one an ARM parked the boss
		above; a CROSS is only honoured for that same bar, so a crossing can
		never span an unknown number of authored thresholds. Both reset with the
		encounter, and the handled sequences reject a resent request instead of
		replaying it. Stable-ID pattern requests have an independent ledger because
		the Effect Tool and the Valtan level own independent sequence counters. */
		std::uint32_t m_iValtanAuditionArmedHealthBar = 0;
		std::unordered_map<SESSION_ID, std::uint32_t>
			m_ValtanAuditionSequenceBySessionId;
		struct VALTAN_PATTERN_ID_COMMAND_RECEIPT final
		{
			LostArk::Shared::C2S_VALTAN_AUDITION_REQUEST Request;
			LostArk::Shared::VALTAN_AUDITION_RESULT Result =
				LostArk::Shared::VALTAN_AUDITION_RESULT::REJECTED_STALE_REQUEST;
			std::uint32_t iCurrentHealthBar = 0u;
			/* A QUEUED receipt must remain reconcilable after its occurrence is no
			   longer the room's current audition. Keep the last authoritative edge
			   so an exact retry cannot loop on a verdict without lifecycle. */
			std::optional<LostArk::Shared::S2C_VALTAN_AUDITION_LIFECYCLE>
				LastLifecycle;
		};
		/* Stable-ID Play/Restart keeps the exact payload and verdict. A retry of
		   one identity replays that verdict; an altered tuple never inherits it. */
		std::unordered_map<SESSION_ID, VALTAN_PATTERN_ID_COMMAND_RECEIPT>
			m_ValtanPatternIdAuditionSequenceBySessionId;
		struct VALTAN_PATTERN_FLOW_COMMAND_RECEIPT final
		{
			std::uint32_t iSequence = 0u;
			std::uint32_t iRoomFlowEpoch = 0u;
			std::string strFlowId;
			std::string strFlowRevision;
			std::string strRequestIdentity;
			LostArk::Shared::GameplayDataRevision PinnedDefinitionRevision{};
			LostArk::Shared::VALTAN_PATTERN_FLOW_RESULT eResult =
				LostArk::Shared::VALTAN_PATTERN_FLOW_RESULT::REJECTED_STALE_FLOW;
			std::string strReason;
			/* Exact Start retries replay the latest authoritative edge for that
			   admitted program. This settles an unconfirmed Client even when the
			   Flow already reached COMPLETED_HOLD; it never starts a second run. */
			std::optional<LostArk::Shared::S2C_DEBUG_VALTAN_PATTERN_FLOW_LIFECYCLE>
				LastLifecycle;
		};
		std::unordered_map<SESSION_ID, VALTAN_PATTERN_FLOW_COMMAND_RECEIPT>
			m_ValtanPatternFlowStartSequenceBySessionId;
		std::unordered_map<SESSION_ID, VALTAN_PATTERN_FLOW_COMMAND_RECEIPT>
			m_ValtanPatternFlowControlSequenceBySessionId;
		struct TARGETED_KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE final
		{
			SESSION_ID iSessionId = INVALID_SESSION_ID;
			LostArk::Shared::
				S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE Message;
		};
		std::uint32_t m_iNextKoukuSaydonPatternAuditionEpoch = 1u;
		struct KOUKU_DRAFT_UPLOAD final
		{
			std::uint32_t iRequestSequence = 0u, iTotalBytes = 0u;
			std::uint64_t iStartedAtMs = 0u;
			LostArk::Shared::GameplayDataRevision RowsRevision{};
			std::string Rows;
		};
		std::unordered_map<SESSION_ID, KOUKU_DRAFT_UPLOAD> m_KoukuDraftUploads;
		KOUKUSAYDON_PATTERN_AUDITION_STATE m_KoukuSaydonPatternAudition;
		std::shared_ptr<const CGameplayCatalog> m_pKoukuPublishedProductGeneration;
		std::unordered_map<SESSION_ID, KOUKUSAYDON_PATTERN_AUDITION_RECEIPT>
			m_KoukuSaydonPatternAuditionReceiptBySessionId;
		std::vector<TARGETED_KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE>
			m_PendingKoukuSaydonPatternAuditionLifecycle;
		struct TARGETED_VALTAN_AUDITION_LIFECYCLE final
		{
			SESSION_ID iSessionId = INVALID_SESSION_ID;
			LostArk::Shared::S2C_VALTAN_AUDITION_LIFECYCLE Message;
		};
		std::uint32_t m_iNextValtanAuditionEpoch = 1u;
		std::vector<TARGETED_VALTAN_AUDITION_LIFECYCLE>
			m_PendingValtanAuditionLifecycle;
		struct TARGETED_VALTAN_PATTERN_FLOW_LIFECYCLE final
		{
			SESSION_ID iSessionId = INVALID_SESSION_ID;
			LostArk::Shared::S2C_DEBUG_VALTAN_PATTERN_FLOW_LIFECYCLE Message;
		};
		std::uint32_t m_iNextValtanPatternFlowEpoch = 1u;
		std::vector<TARGETED_VALTAN_PATTERN_FLOW_LIFECYCLE>
			m_PendingValtanPatternFlowLifecycle;
		VALTAN_PATTERN_ID_AUDITION_STATE m_ValtanPatternIdAudition;
		std::optional<VALTAN_NEXT_PATTERN_RESERVATION> m_ValtanNextPattern;
		std::unordered_map<SESSION_ID, VALTAN_NEXT_PATTERN_COMMAND_RECEIPT>
			m_ValtanNextPatternReceiptBySessionId;
		VALTAN_PATTERN_FLOW_AUDITION_STATE m_ValtanPatternFlowAudition;
		VALTAN_TIMELINE_AUDITION_STATE m_ValtanTimelineAudition;
		VALTAN_FIGHT_PAGE_START_STATE m_ValtanFightPageStart;
	};
}
```

### Server/Private/GameRoom_Colosseum.cpp

```cpp
#include "GameRoom.h"
#include "ClientSession.h"
#include "ColosseumCombatPolicy.h"
#include "ServerCombatGeometry.h"
#include "Network/PacketWriter.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <set>

using namespace LostArk::Server;
using namespace LostArk::Shared;

namespace
{
    constexpr float PI = 3.14159265358979323846f;
    constexpr std::uint32_t MERCENARY_ALT_V_INTERVAL_TICKS = 30u * 30u;
    constexpr std::array<CHARACTER_CLASS_ID, 5> MERCENARY_CLASSES = {
        CHARACTER_CLASS_ID::DIMENSIONMASTER, CHARACTER_CLASS_ID::LANCE_MASTER,
        CHARACTER_CLASS_ID::WARLORD, CHARACTER_CLASS_ID::GUARDIANKNIGHT,
        CHARACTER_CLASS_ID::ARTIST };
    constexpr std::array<const char*, 5> MERCENARY_NAMES = {
        "차원술사 용병", "창술사 용병", "워로드 용병", "가디언나이트 용병", "도화가 용병" };

    bool DeadlineReached(std::uint32_t now, std::uint32_t deadline)
    { return deadline == 0u || static_cast<std::int32_t>(now - deadline) >= 0; }

    // Stable per-match streams make decisions reproducible without synchronizing bots.
    float RandomUnit(std::uint64_t& state)
    {
        if (!state) state = 0x9e3779b97f4a7c15ULL;
        state ^= state >> 12; state ^= state << 25; state ^= state >> 27;
        return static_cast<float>((state * 2685821657736338717ULL) >> 40) / 16777216.f;
    }

    void BuildAvailableMercenarySkills(const CGameplayCatalog& catalog,
        const SERVER_PLAYER& merc, std::vector<SKILL_ID>& skills)
    {
        skills.clear();
        for (const auto& [id, skill] : catalog.Get_Skills())
            if (skill.eCharacterClass == merc.eCharacterClass &&
                (skill.eRequiredStance == PLAYER_STANCE_ID::NONE || skill.eRequiredStance == merc.eStance))
                skills.push_back(id);
        // Catalog iteration order must not change a seeded weighted draw.
        std::sort(skills.begin(), skills.end());
    }

    bool SkillResourcesReady(const SERVER_PLAYER& merc, const PLAYER_SKILL_DEFINITION& skill,
        std::uint32_t tick)
    {
        const auto cooldown = merc.CooldownEndTickBySkillId.find(skill.iSkillId);
        return skill.eCharacterClass == merc.eCharacterClass &&
            (skill.eRequiredStance == PLAYER_STANCE_ID::NONE || skill.eRequiredStance == merc.eStance) &&
            (cooldown == merc.CooldownEndTickBySkillId.end() || DeadlineReached(tick, cooldown->second)) &&
            merc.iCurrentResource >= skill.iResourceCost && merc.iCurrentIdentity >= skill.iIdentityCost &&
            (skill.eSetsStance == PLAYER_STANCE_ID::NONE || DeadlineReached(tick, merc.iStanceSwitchCooldownEndTick));
    }

    float HealthFraction(const SERVER_PLAYER& player)
    { return player.iMaximumHp ? static_cast<float>(player.iCurrentHp) / player.iMaximumHp : 0.f; }

    float SkillReach(const PLAYER_SKILL_DEFINITION& skill)
    { return (std::max)(2.f, skill.fMaximumRange); }

    struct DODGE_MOTION_POINT { float time = 0.f, forward = 0.f, lateral = 0.f; };
    struct DODGE_MOTION
    {
        std::array<DODGE_MOTION_POINT, 181> points{};
        std::size_t count = 1u;
        bool releaseHold = false;
    };

    // Read the same cumulative curve as PlayerSkillSystem::Update. These samples
    // are observations only; the normal skill executor still commits every move.
    std::pair<float, float> SampleDodgeMotion(const std::vector<ROOT_MOTION_SAMPLE>& samples, float seconds)
    {
        if (samples.empty()) return {};
        const float milliseconds = seconds * 1000.f;
        if (milliseconds <= float(samples.front().iTimeMs))
            return {samples.front().fForward, samples.front().fLateral};
        for (std::size_t index = 1u; index < samples.size(); ++index)
        {
            const auto& previous = samples[index - 1u]; const auto& current = samples[index];
            if (milliseconds > float(current.iTimeMs)) continue;
            const float span = float(current.iTimeMs) - float(previous.iTimeMs);
            const float alpha = span <= 0.f ? 0.f : (milliseconds - float(previous.iTimeMs)) / span;
            return {previous.fForward + (current.fForward - previous.fForward) * alpha,
                previous.fLateral + (current.fLateral - previous.fLateral) * alpha};
        }
        return {samples.back().fForward, samples.back().fLateral};
    }

    bool ReadDodgeMotion(const PLAYER_SKILL_DEFINITION& skill, float fixedSeconds, DODGE_MOTION& out)
    {
        const auto* samples = &skill.RootMotion;
        std::uint32_t durationMs = skill.iActionDurationMs;
        out = {};
        if (skill.eSkillKind == PLAYER_SKILL_KIND::HOLD && !skill.ComboStages.empty() &&
            skill.ComboStages.front().iComboAdvanceMs < skill.ComboStages.front().iActionDurationMs)
        {
            // An immediately released branching HOLD completes its start landing,
            // without the duration/route changing with a later tactical release.
            samples = &skill.ComboStages.front().RootMotion;
            durationMs = skill.ComboStages.front().iActionDurationMs;
            out.releaseHold = true;
        }
        else if (skill.eSkillKind != PLAYER_SKILL_KIND::ACTIVE && skill.eSkillKind != PLAYER_SKILL_KIND::STANDUP)
            return false;
        const float duration = float(durationMs) * .001f;
        if (!(fixedSeconds > 0.f) || !(duration > 0.f) || !std::isfinite(skill.fRootMotionScale)) return false;
        float elapsed = 0.f, forward = 0.f, lateral = 0.f;
        while (elapsed < duration)
        {
            if (out.count == out.points.size()) return false;
            elapsed += fixedSeconds;
            float deltaForward = 0.f, deltaLateral = 0.f;
            if (!samples->empty())
            {
                const auto previous = SampleDodgeMotion(*samples, (std::max)(0.f, elapsed - fixedSeconds));
                const auto current = SampleDodgeMotion(*samples, elapsed);
                deltaForward = current.first - previous.first;
                deltaLateral = current.second - previous.second;
            }
            else if (skill.fMovementDistance > 0.f)
                deltaForward = skill.fMovementDistance / duration * fixedSeconds;
            forward += deltaForward * skill.fRootMotionScale;
            lateral += deltaLateral * skill.fRootMotionScale;
            if (!std::isfinite(forward) || !std::isfinite(lateral)) return false;
            out.points[out.count++] = {elapsed, forward, lateral};
        }
        return true;
    }

    S2C_PLAYER_SPAWNED MakeSpawn(const SERVER_PLAYER& player)
    {
        S2C_PLAYER_SPAWNED message;
        message.iPlayerId = player.iPlayerId;
        message.iNetEntityId = player.iNetEntityId;
        message.eCharacterClass = player.eCharacterClass;
        message.eControlKind = player.eControlKind;
        message.strNickName = player.strNickName;
        message.iVoiceType = player.iVoiceType;
        message.strAppearanceJson = player.strAppearanceJson;
        message.fPositionX = player.fPositionX;
        message.fPositionY = player.fPositionY;
        message.fPositionZ = player.fPositionZ;
        message.fYawDegrees = player.fYawDegrees;
        return message;
    }


}

bool CGameRoom::Transfer_ColosseumMatchTo(CGameRoom& target,
    const std::vector<SESSION_ID>& orderedSeats, std::uint64_t matchId, std::string& status)
{
    const auto reject = [&status](const char* reason) { status = reason; return false; };
    if (m_eWorldId != WORLD_ID::BERN || target.m_eWorldId != WORLD_ID::COLOSSEUM ||
        !m_isReady || !target.m_isReady || matchId == 0u || (orderedSeats.empty() || orderedSeats.size() > MAX_COLOSSEUM_MATCH_PLAYERS) ||
        !target.m_Players.empty() || target.m_iColosseumMatchId != 0u ||
        std::set<SESSION_ID>(orderedSeats.begin(), orderedSeats.end()).size() != orderedSeats.size())
        return reject("invalid Colosseum match batch");
    const auto* hpProfile = target.m_GameplayCatalog.Find_Boss("BOSS_VALTAN");
    if (!hpProfile || hpProfile->iMaximumHp == 0u || hpProfile->iMaximumHealthBars != 160u)
        return reject("Colosseum requires the active Valtan 160-bar health profile");
    const auto referenceHp = hpProfile->Get_DamageReferenceHp();
    const auto matchHp = referenceHp / 8u + (referenceHp % 8u != 0u ? 1u : 0u);

    std::vector<STAGED_PLAYER_ENTRY> entries;
    std::vector<PLAYER_ID> departingIds;
    std::vector<NET_ENTITY_ID> departingEntities;
    std::array<std::uint32_t, 2> partyIds{ target.m_iNextPartyId, target.m_iNextPartyId + 1u };
    if (partyIds[0] == 0u || partyIds[1] == 0u ||
        target.m_PartyMembersByPartyId.contains(partyIds[0]) || target.m_PartyMembersByPartyId.contains(partyIds[1]))
        return reject("Colosseum party identity exhausted");
    entries.reserve(4);
    for (std::size_t index = 0; index < orderedSeats.size(); ++index)
    {
        const SESSION_ID sessionId = orderedSeats[index];
        const auto identity = m_PlayerIdBySessionId.find(sessionId);
        if (identity == m_PlayerIdBySessionId.end()) return reject("queued player left Bern");
        const auto& source = m_Players.at(identity->second);
        const auto session = Find_Session(sessionId);
        if (!source.Is_Human() || source.iCurrentHp == 0u || source.eAction != PLAYER_ACTION_STATE::NONE ||
            !session || session->Is_Closing() ||
            std::none_of(m_ColosseumQueue.begin(), m_ColosseumQueue.end(),
                [sessionId](const auto& entry) { return entry.iSessionId == sessionId; }))
            return reject("queued player is no longer available");
        if (const auto party = m_PartyIdByPlayerId.find(source.iPlayerId); party != m_PartyIdByPlayerId.end())
        {
            const auto members = m_PartyMembersByPartyId.find(party->second);
            if (members == m_PartyMembersByPartyId.end() || members->second.size() != 1u)
                return reject("queued player joined another party");
        }
        if (std::any_of(m_RaidEntryProposals.begin(), m_RaidEntryProposals.end(),
            [&source](const auto& vote) { return std::find(vote.Voters.begin(), vote.Voters.end(), source.iPlayerId) != vote.Voters.end(); }))
            return reject("queued player owns an open world-entry vote");
        C2S_ENTER_WORLD enter;
        enter.iProtocolVersion = NETWORK_PROTOCOL_VERSION;
        enter.eWorldId = WORLD_ID::COLOSSEUM;
        enter.eCharacterClass = source.eCharacterClass;
        enter.strNickName = source.strNickName;
        enter.iVoiceType = source.iVoiceType;
        enter.strAppearanceJson = source.strAppearanceJson;
        STAGED_PLAYER_ENTRY entry;
        SESSION_DIAGNOSTIC_REASON reason;
        if (!target.Stage_PlayerEntry(session, enter, entries, entry, reason, status,
            {}, source.Inventory, source.iHonorTitleId, {}, source.Purse, true)) return false;
        // Team spawn is an authored player standing point, not the NPC-approach override.
        const std::string spawnId = std::string("player.spawn.colosseum.team") +
            (index % 2u ? "b.0" : "a.0") + std::to_string(index / 2u + 1u);
        const auto* spawn = target.Find_Placement(spawnId);
        SERVER_NAV_POINT position;
        if (!spawn || !target.m_ServerNavigation.Sample_Position(spawn->fPositionX, spawn->fPositionZ,
            position, spawn->fPositionY) ||
            !target.m_ServerCollisionSystem.Is_PlayerPositionClear(position.x, position.y, position.z, entry.Player.iNetEntityId))
            return reject("Colosseum team spawn failed navigation/collision admission");
        for (const auto& preceding : entries)
            if (std::abs(preceding.Player.fPositionY - position.y) < 1.5f &&
                std::hypot(preceding.Player.fPositionX - position.x, preceding.Player.fPositionZ - position.z) < .75f)
                return reject("Colosseum team spawn overlaps an admitted human");
        auto& player = entry.Player;
        player.Inventory = source.Inventory;
        player.Purse = source.Purse;
        player.bRestoreAvailable = false;
        player.strSpawnPlacementId = spawnId;
        player.fPositionX = position.x; player.fPositionY = position.y; player.fPositionZ = position.z;
        player.fYawDegrees = spawn->fYawDegrees;
        player.iColosseumMatchId = matchId;
        player.iColosseumTeam = static_cast<std::uint8_t>(index % 2u);
        player.iColosseumArrivalIndex = static_cast<std::uint8_t>(index);
        player.fColosseumSpawnX = position.x; player.fColosseumSpawnY = position.y;
        player.fColosseumSpawnZ = position.z; player.fColosseumSpawnYaw = player.fYawDegrees;
        player.bColosseumParticipant = true;
        player.isCombatReady = false;
        player.iColosseumDamageReferenceHp = referenceHp;
        player.iCurrentHp = player.iMaximumHp = matchHp;
        departingIds.push_back(source.iPlayerId);
        departingEntities.push_back(source.iNetEntityId);
        entries.push_back(std::move(entry));
    }

    auto players = target.m_Players;
    auto sessionPlayers = target.m_PlayerIdBySessionId;
    auto entityPlayers = target.m_PlayerIdByEntityId;
    auto sessions = target.m_Sessions;
    auto playerParties = target.m_PartyIdByPlayerId;
    auto parties = target.m_PartyMembersByPartyId;
    auto mercenaries = target.m_ColosseumMercenaries;
    std::vector<SERVER_PLAYER> candidates;
    candidates.reserve(10);
    for (const auto& entry : entries)
    {
        const auto& player = entry.Player;
        players.emplace(player.iPlayerId, player);
        sessionPlayers.emplace(player.iSessionId, player.iPlayerId);
        entityPlayers.emplace(player.iNetEntityId, player.iPlayerId);
        sessions.emplace(player.iSessionId, entry.pSession);
        playerParties.emplace(player.iPlayerId, partyIds[player.iColosseumTeam]);
        parties[partyIds[player.iColosseumTeam]].push_back(player.iPlayerId);
    }
    for (std::uint8_t team = 0u; team < 2u; ++team)
    {
        SERVER_PLAYER anchor;
        const auto human = std::find_if(entries.begin(), entries.end(), [team](const auto& entry) { return entry.Player.iColosseumTeam == team; });
        if (human != entries.end()) anchor = human->Player;
        else
        {
            const auto* spawn = target.Find_Placement(team ? "player.spawn.colosseum.teamb.01" : "player.spawn.colosseum.teama.01");
            if (!spawn) return reject("Colosseum empty-team spawn missing");
            anchor.fPositionX = spawn->fPositionX; anchor.fPositionY = spawn->fPositionY;
            anchor.fPositionZ = spawn->fPositionZ; anchor.fYawDegrees = spawn->fYawDegrees;
        }
        // A solo entrant owns only their own team's recruitment. The opposite
        // team receives four real sessionless mercenaries in this same transaction.
        const bool automaticSoloOpponent = orderedSeats.size() == 1u && human == entries.end();
        auto& teamMembers = parties[partyIds[team]];
        const float yaw = anchor.fYawDegrees * PI / 180.f;
        for (std::size_t slot = 0u; slot < MERCENARY_CLASSES.size(); ++slot)
        {
            const auto* profile = target.m_GameplayCatalog.Find_Player(MERCENARY_CLASSES[slot]);
            if (!profile) return reject("Colosseum mercenary class profile missing");
            SERVER_PLAYER merc;
            merc.eControlKind = PLAYER_CONTROL_KIND::COLOSSEUM_MERCENARY_AI;
            merc.iPlayerId = target.m_iNextPlayerId + static_cast<PLAYER_ID>(orderedSeats.size() + candidates.size());
            merc.iNetEntityId = target.m_iNextNetEntityId + static_cast<NET_ENTITY_ID>(orderedSeats.size() + candidates.size());
            merc.eCharacterClass = MERCENARY_CLASSES[slot];
            merc.strNickName = MERCENARY_NAMES[slot];
            merc.eStance = profile->eDefaultStance;
            merc.iColosseumDamageReferenceHp = referenceHp;
            merc.iCurrentHp = merc.iMaximumHp = matchHp;
            merc.iCurrentResource = merc.iMaximumResource = profile->iMaximumResource;
            merc.iMaximumIdentity = profile->iMaximumIdentity;
            merc.fMoveSpeed = profile->fMoveSpeed;
            CPlayerSkillSystem::Reset_Gauges(merc, target.m_GameplayCatalog);
            merc.iColosseumMatchId = matchId; merc.iColosseumTeam = team;
            // Human-owned teams, including the solo player's team, still recruit manually.
            merc.bColosseumParticipant = automaticSoloOpponent && slot < 4u;
            merc.isCombatReady = false;
            merc.bColosseumReady = true;
            if (merc.bColosseumParticipant)
            {
                merc.iColosseumArrivalIndex = static_cast<std::uint8_t>(team + teamMembers.size() * 2u);
                teamMembers.push_back(merc.iPlayerId);
                playerParties.emplace(merc.iPlayerId, partyIds[team]);
            }
            merc.fYawDegrees = anchor.fYawDegrees;
            const float lateral = (static_cast<float>(slot) - 2.f) * 1.5f;
            const float x = anchor.fPositionX + std::sin(yaw) * 4.f + std::cos(yaw) * lateral;
            const float z = anchor.fPositionZ + std::cos(yaw) * 4.f - std::sin(yaw) * lateral;
            SERVER_NAV_POINT landing;
            bool admitted = false;
            unsigned navRejected = 0u, heightRejected = 0u, collisionRejected = 0u, overlapRejected = 0u, pathRejected = 0u;
            // Same bounded rings and exact navigation/collision admission as
            // Find_GuideLanding, with the complete uncommitted batch included.
            for (unsigned sample = 0u; sample < 49u && !admitted; ++sample)
            {
                const float radius = sample ? .75f + static_cast<float>((sample - 1u) / 12u) * .75f : 0.f;
                const float angle = static_cast<float>(sample % 12u) * PI / 6.f;
                const float candidateX = x + std::sin(angle) * radius;
                const float candidateZ = z + std::cos(angle) * radius;
                const float forward = (candidateX - anchor.fPositionX) * std::sin(yaw) +
                    (candidateZ - anchor.fPositionZ) * std::cos(yaw);
                if (forward < 1.f || std::hypot(candidateX - anchor.fPositionX, candidateZ - anchor.fPositionZ) > 7.5f) continue;
                SERVER_NAV_POINT point;
                if (!target.m_ServerNavigation.Sample_Position(candidateX, candidateZ, point, anchor.fPositionY))
                { ++navRejected; continue; }
                if (std::abs(point.y - anchor.fPositionY) > 1.f) { ++heightRejected; continue; }
                if (!target.m_ServerCollisionSystem.Is_PlayerPositionClear(point.x, point.y, point.z, merc.iNetEntityId))
                { ++collisionRejected; continue; }
                const bool overlap = std::any_of(players.begin(), players.end(), [&point](const auto& value)
                {
                    return std::abs(value.second.fPositionY - point.y) < 1.5f &&
                        std::hypot(value.second.fPositionX - point.x, value.second.fPositionZ - point.z) < .75f;
                });
                if (overlap) { ++overlapRejected; continue; }
                if (!target.m_ServerNavigation.Has_LineOfSight(anchor.fPositionX, anchor.fPositionZ,
                    point.x, point.z, anchor.fPositionY)) { ++pathRejected; continue; }
                landing = point; admitted = true;
            }
            if (!admitted)
            {
                status = "Colosseum mercenary admission team=" + std::to_string(team) + " slot=" + std::to_string(slot) +
                    " desired=" + std::to_string(x) + "," + std::to_string(z) + " nav=" + std::to_string(navRejected) +
                    " height=" + std::to_string(heightRejected) + " collision=" + std::to_string(collisionRejected) +
                    " overlap=" + std::to_string(overlapRejected) + " path=" + std::to_string(pathRejected);
                return false;
            }
            merc.fPositionX = landing.x; merc.fPositionY = landing.y; merc.fPositionZ = landing.z;
            merc.fColosseumSpawnX = landing.x; merc.fColosseumSpawnY = landing.y;
            merc.fColosseumSpawnZ = landing.z; merc.fColosseumSpawnYaw = merc.fYawDegrees;
            players.emplace(merc.iPlayerId, merc);
            entityPlayers.emplace(merc.iNetEntityId, merc.iPlayerId);
            COLOSSEUM_MERCENARY_RUNTIME ai;
            BuildAvailableMercenarySkills(target.m_GameplayCatalog.Active(), merc, ai.AvailableSkills);
            if (ai.AvailableSkills.empty())
            { status = "Colosseum mercenary has no published class skills"; return false; }
            ai.iRandomState = matchId ^ (static_cast<std::uint64_t>(merc.iNetEntityId) * 0x9e3779b97f4a7c15ULL);
            ai.iObservedHp = merc.iCurrentHp;
            mercenaries.emplace(merc.iPlayerId, std::move(ai));
            candidates.push_back(std::move(merc));
        }
    }

    const auto append = [&status](std::vector<PACKET_FRAME>& frames, PACKET_TYPE type, const auto& message)
    {
        CPacketWriter writer;
        if (!Write_Message(writer, message)) { status = "Colosseum admission message encoding failed"; return false; }
        frames.push_back({ type, writer.Get_Buffer() }); return true;
    };
    S2C_COLOSSEUM_MATCH_STATE state;
    state.iMatchId = matchId; state.iRevision = 1u;
    state.ePhase = COLOSSEUM_MATCH_PHASE::LOADING;
    state.iServerTick = target.m_iServerTick; state.iPhaseStartTick = target.m_iServerTick;
    state.iPhaseEndTick = target.m_iServerTick + 3600u;
    for (const auto& [id, player] : players)
    {
        COLOSSEUM_MATCH_PLAYER_STATE row;
        row.iPlayerId = id; row.iNetEntityId = player.iNetEntityId; row.iTeam = player.iColosseumTeam;
        row.bParticipant = player.bColosseumParticipant; row.iArrivalIndex = player.iColosseumArrivalIndex;
        row.bReady = player.bColosseumReady; row.iKills = player.iColosseumKills;
        state.Players.push_back(row);
        if (row.bParticipant) state.Participants.push_back(row);
    }
    state.iExpectedPlayers = static_cast<std::uint8_t>(state.Participants.size());
    S2C_COLOSSEUM_MATCH_FOUND found;
    found.iMatchId = matchId;
    for (const auto& entry : entries)
        found.Participants.push_back({ entry.Player.strNickName, entry.Player.eCharacterClass, entry.Player.iColosseumTeam });
    std::vector<CLIENT_SESSION_RELIABLE_BATCH> outboundBatches;
    for (std::size_t index = 0; index < entries.size(); ++index)
    {
        auto& entry = entries[index];
        if (!target.Build_PlayerEntryFrames(entry, entries, status)) return false;
        std::vector<PACKET_FRAME> frames;
        found.iLocalIndex = static_cast<std::uint8_t>(index);
        if (!append(frames, PACKET_TYPE::S2C_COLOSSEUM_MATCH_FOUND, found)) return false;
        frames.insert(frames.end(), entry.Frames.begin(), entry.Frames.end());
        for (const auto& candidate : candidates)
            if (!append(frames, PACKET_TYPE::S2C_PLAYER_SPAWNED, MakeSpawn(candidate))) return false;
        S2C_PARTY_ROSTER roster;
        for (const auto id : parties.at(partyIds[entry.Player.iColosseumTeam]))
        {
            const auto& member = players.at(id);
            roster.Members.push_back({ member.iNetEntityId, member.strNickName, member.eCharacterClass, member.eControlKind });
        }
        if (!append(frames, PACKET_TYPE::S2C_PARTY_ROSTER, roster) ||
            !append(frames, PACKET_TYPE::S2C_COLOSSEUM_MATCH_STATE, state)) return false;
        outboundBatches.push_back({ entry.pSession, std::move(frames) });
    }
    for (const auto& [id, player] : m_Players)
    {
        if (!player.Is_Human() || std::find(orderedSeats.begin(), orderedSeats.end(), player.iSessionId) != orderedSeats.end()) continue;
        CLIENT_SESSION_RELIABLE_BATCH observer{ Find_Session(player.iSessionId), {} };
        for (const auto entityId : departingEntities)
        {
            S2C_PLAYER_DESPAWNED despawn;
            despawn.iNetEntityId = entityId; despawn.eReason = PLAYER_DESPAWN_REASON::LEVEL_CHANGED;
            if (!append(observer.Frames, PACKET_TYPE::S2C_PLAYER_DESPAWNED, despawn)) return false;
        }
        outboundBatches.push_back(std::move(observer));
    }
    auto expectedSessions = orderedSeats;
    CClientSession::RELIABLE_BATCH_TRANSACTION outbound;
    if (!outbound.Prepare(outboundBatches, status)) return false;
    // All allocations and FIFO capacity checks precede the first source mutation.
    for (const auto playerId : departingIds)
    {
        const auto party = m_PartyIdByPlayerId.find(playerId);
        if (party != m_PartyIdByPlayerId.end())
        {
            m_PartyMembersByPartyId.erase(party->second);
            m_PartyIdByPlayerId.erase(party);
        }
    }
    for (const auto sessionId : orderedSeats) Leave(sessionId, PLAYER_DESPAWN_REASON::LEVEL_CHANGED, false);
    target.m_Players.swap(players); target.m_PlayerIdBySessionId.swap(sessionPlayers);
    target.m_PlayerIdByEntityId.swap(entityPlayers); target.m_Sessions.swap(sessions);
    target.m_PartyIdByPlayerId.swap(playerParties); target.m_PartyMembersByPartyId.swap(parties);
    target.m_ColosseumMercenaries.swap(mercenaries);
    target.m_iColosseumMatchId = matchId; target.m_ColosseumTeamPartyIds = partyIds;
    target.m_ColosseumSessions.swap(expectedSessions);
    target.m_eColosseumPhase = COLOSSEUM_MATCH_PHASE::LOADING;
    target.m_iColosseumPhaseStart = target.m_iServerTick; target.m_iColosseumPhaseEnd = target.m_iServerTick + 3600u;
    for (const auto& entry : entries) ++target.m_ColosseumInitialHumans[entry.Player.iColosseumTeam];
    target.m_iNextPlayerId += static_cast<PLAYER_ID>(target.m_Players.size());
    target.m_iNextNetEntityId += static_cast<NET_ENTITY_ID>(target.m_Players.size()); target.m_iNextPartyId += 2u;
    for (const auto& entry : entries) entry.pSession->Bind_PlayerId(entry.Player.iPlayerId);
    outbound.Commit();
    status = "Colosseum match committed for " + std::to_string(orderedSeats.size()) + " human players";
    return true;
}

void CGameRoom::Notify_ColosseumTransferResult(bool committed)
{
    m_bColosseumTransferPending = false;
    m_iColosseumRetryTick = committed ? 0u : m_iServerTick + 30u;
    if (committed) m_iColosseumQueueDeadline = m_ColosseumQueue.empty() ? 0u : m_iServerTick + 300u;
    for (const auto& entry : m_ColosseumQueue)
        Send_ColosseumQueueState(entry.iSessionId, COLOSSEUM_QUEUE_STATE::WAITING);
}

S2C_COLOSSEUM_MATCH_STATE CGameRoom::Build_ColosseumState() const
{
    S2C_COLOSSEUM_MATCH_STATE state;
    state.iMatchId = m_iColosseumMatchId; state.ePhase = m_eColosseumPhase;
    state.iWinnerTeam = state.iWinningTeam = m_iColosseumWinnerTeam; state.iRevision = m_iColosseumRevision;
    state.iServerTick = m_iServerTick; state.iPhaseStartTick = m_iColosseumPhaseStart; state.iPhaseEndTick = m_iColosseumPhaseEnd;
    state.iLeftScore = m_iColosseumScores[0]; state.iRightScore = m_iColosseumScores[1]; state.RecentKills = m_ColosseumRecentKills;
    for (const auto& [id, player] : m_Players)
        if (player.iColosseumMatchId == m_iColosseumMatchId && player.iColosseumTeam < 2u)
        {
            COLOSSEUM_MATCH_PLAYER_STATE row;
            row.iPlayerId = id; row.iNetEntityId = player.iNetEntityId; row.iTeam = player.iColosseumTeam;
            row.bParticipant = player.bColosseumParticipant; row.iArrivalIndex = player.iColosseumArrivalIndex;
            row.bReady = player.bColosseumReady; row.iKills = player.iColosseumKills;
            state.Players.push_back(row);
            if (row.bParticipant) state.Participants.push_back(row);
        }
    state.iExpectedPlayers = static_cast<std::uint8_t>(state.Participants.size());
    return state;
}

void CGameRoom::Broadcast_ColosseumState()
{
    if (m_iColosseumMatchId == 0u) return;
    CPacketWriter writer;
    if (!Write_Message(writer, Build_ColosseumState())) return;
    for (const auto& [id, player] : m_Players)
        if (player.Is_Human())
            if (const auto session = Find_Session(player.iSessionId); session &&
                !session->Send_Frame(PACKET_TYPE::S2C_COLOSSEUM_MATCH_STATE, writer.Get_Buffer())) session->Request_Close();
}

void CGameRoom::Handle_ColosseumRecruit(SESSION_ID sessionId, const C2S_COLOSSEUM_RECRUIT& request)
{
    if (m_eWorldId != WORLD_ID::COLOSSEUM || m_iColosseumMatchId == 0u || request.iMatchId != m_iColosseumMatchId) return;
    const auto actorId = m_PlayerIdBySessionId.find(sessionId);
    const auto candidateId = m_PlayerIdByEntityId.find(request.iMercenaryNetEntityId);
    if (actorId == m_PlayerIdBySessionId.end() || candidateId == m_PlayerIdByEntityId.end()) return;
    auto& actor = m_Players.at(actorId->second);
    auto& candidate = m_Players.at(candidateId->second);
    auto& sequence = m_ColosseumRecruitSequences[sessionId];
    if (request.iRequestSequence == 0u || (sequence != 0u && static_cast<std::int32_t>(request.iRequestSequence - sequence) <= 0))
    { Broadcast_ColosseumState(); return; }
    sequence = request.iRequestSequence;
    if (m_eColosseumPhase != COLOSSEUM_MATCH_PHASE::RECRUITING || !actor.Is_Human() || actor.iCurrentHp == 0u ||
        !candidate.Is_ColosseumMercenary() || candidate.iColosseumMatchId != m_iColosseumMatchId ||
        actor.iColosseumTeam >= 2u || candidate.iColosseumTeam != actor.iColosseumTeam ||
        candidate.bColosseumParticipant || std::abs(actor.fPositionY - candidate.fPositionY) > 2.f ||
        std::hypot(actor.fPositionX - candidate.fPositionX, actor.fPositionZ - candidate.fPositionZ) > 8.f)
    { Broadcast_ColosseumState(); return; }
    auto& members = m_PartyMembersByPartyId.at(m_ColosseumTeamPartyIds[actor.iColosseumTeam]);
    if (members.size() >= 4u) { Broadcast_ColosseumState(); return; }
    candidate.iColosseumArrivalIndex = static_cast<std::uint8_t>(actor.iColosseumTeam + members.size() * 2u);
    members.push_back(candidate.iPlayerId);
    m_PartyIdByPlayerId.emplace(candidate.iPlayerId, m_ColosseumTeamPartyIds[actor.iColosseumTeam]);
    candidate.bColosseumParticipant = true;
    ++m_iColosseumRevision;
    Try_StartColosseumEntry();
    Broadcast_PartyRoster(m_ColosseumTeamPartyIds[actor.iColosseumTeam]);
    Broadcast_ColosseumState();
}

void CGameRoom::Try_StartColosseumEntry()
{
    if (m_eColosseumPhase != COLOSSEUM_MATCH_PHASE::RECRUITING) return;
    for (const auto partyId : m_ColosseumTeamPartyIds)
    {
        const auto found = m_PartyMembersByPartyId.find(partyId);
        if (found == m_PartyMembersByPartyId.end() || found->second.size() != 4u) return;
    }
    m_eColosseumPhase = COLOSSEUM_MATCH_PHASE::ENTRY_COUNTDOWN;
    m_iColosseumPhaseStart = m_iServerTick; m_iColosseumPhaseEnd = m_iServerTick + 300u;
    ++m_iColosseumRevision;
    for (auto& [id, player] : m_Players)
    {
        player.hasMoveGoal = false; player.MovePath.clear(); player.PendingCommand.Clear();
        player.isCombatReady = player.bColosseumCombatActive = false;
    }
}

bool CGameRoom::Try_SealColosseumForRetirement()
{
    if (m_iColosseumMatchId == 0u || m_eWorldId != WORLD_ID::COLOSSEUM) return false;
    std::scoped_lock lock{ m_CommandMutex };
    if (!m_acceptsCommands) return true;
    if (!m_InboundCommands.empty() || !m_CleanupCommands.empty() || !m_QueuedCleanupSessionIds.empty() ||
        !m_PendingWorldTransfers.empty() || !m_Sessions.empty() || !m_PlayerIdBySessionId.empty() || Count_HumanPlayers() != 0u) return false;
    m_acceptsCommands = false;
    return true;
}

void CGameRoom::Update_Colosseum(float seconds)
{
    if (m_eWorldId == WORLD_ID::BERN)
    {
        if (!m_bColosseumTransferPending && (m_iColosseumRetryTick == 0u ||
            static_cast<std::int32_t>(m_iServerTick - m_iColosseumRetryTick) >= 0)) Try_FormColosseumMatch();
        return;
    }
    if (m_eWorldId != WORLD_ID::COLOSSEUM || m_iColosseumMatchId == 0u || m_eColosseumPhase == COLOSSEUM_MATCH_PHASE::FINISHED) return;
    std::array<unsigned, 2> humans{};
    for (const auto& [id, player] : m_Players)
        if (player.Is_Human() && player.iColosseumTeam < 2u) ++humans[player.iColosseumTeam];
    // A team intentionally created without humans is supported. Losing an admitted
    // human while recruiting aborts the match instead of stranding its allies.
    if ((m_eColosseumPhase == COLOSSEUM_MATCH_PHASE::RECRUITING || m_eColosseumPhase == COLOSSEUM_MATCH_PHASE::LOADING) &&
        (humans[0] < m_ColosseumInitialHumans[0] || humans[1] < m_ColosseumInitialHumans[1]))
    {
        m_eColosseumPhase = COLOSSEUM_MATCH_PHASE::FINISHED;
        m_iColosseumPhaseStart = m_iServerTick; m_iColosseumPhaseEnd = 0u; m_iColosseumWinnerTeam = COLOSSEUM_NO_TEAM;
        for (auto& [id, player] : m_Players)
        {
            player.isCombatReady = player.bColosseumCombatActive = false;
            player.hasMoveGoal = false; player.MovePath.clear(); player.PendingCommand.Clear();
        }
        ++m_iColosseumRevision; Broadcast_ColosseumState(); return;
    }
    if (m_eColosseumPhase != COLOSSEUM_MATCH_PHASE::ACTIVE) return;
    for (auto& [playerId, runtime] : m_ColosseumMercenaries)
    {
        auto actor = m_Players.find(playerId);
        if (actor == m_Players.end()) continue;
        auto& merc = actor->second;
        if (!merc.bColosseumParticipant || merc.iColosseumMatchId != m_iColosseumMatchId) continue;
        if (!merc.iCurrentHp)
        {
            runtime.iTargetEntityId = INVALID_NET_ENTITY_ID;
            runtime.eTactic = COLOSSEUM_TACTIC::WAIT;
            runtime.iObservedHp = 0u; runtime.iNextAttackTick = runtime.iNextEvadeTick = 0u;
            runtime.fSenseElapsed = 1.f; runtime.fThreatAgeSeconds = 0.f;
            runtime.pReason = "Waiting for the server respawn";
            continue;
        }
        if (!runtime.iRandomState)
            runtime.iRandomState = m_iColosseumMatchId ^ (std::uint64_t(merc.iNetEntityId) * 0x9e3779b97f4a7c15ULL);
        if (runtime.iObservedHp && merc.iCurrentHp < runtime.iObservedHp) runtime.iLastDamageTick = m_iServerTick;
        runtime.iObservedHp = merc.iCurrentHp;
        runtime.fThinkElapsed += seconds; runtime.fSenseElapsed += seconds;
        const bool thinking = runtime.fThinkElapsed >= runtime.fDecisionInterval || runtime.AvailableSkills.empty();
        if (thinking)
        {
            runtime.fThinkElapsed = 0.f;
            runtime.fDecisionInterval = .16f + RandomUnit(runtime.iRandomState) * .12f;
            BuildAvailableMercenarySkills(m_GameplayCatalog.Active(), merc, runtime.AvailableSkills);
        }
        // Observations are refreshed at 10 Hz; the cached moving shapes are sampled
        // every fixed tick. Tactical lotteries never run at the simulation frequency.
        if (runtime.fSenseElapsed >= .1f || thinking)
        {
            runtime.Threats.Observe(merc, m_Players, m_CombatObjectRuntime.Get_LiveObjects(),
                m_GameplayCatalog.Active(), m_iServerTick);
            runtime.fSenseElapsed = 0.f;
        }
        const auto immediate = runtime.Threats.At(merc.fPositionX, merc.fPositionY, merc.fPositionZ, runtime.fSenseElapsed);
        const auto approaching = runtime.Threats.At(merc.fPositionX, merc.fPositionY, merc.fPositionZ, runtime.fSenseElapsed + .3f);
        const float risk = (std::max)((std::max)(immediate.risk, approaching.risk),
            runtime.Threats.Along(merc.fPositionX, merc.fPositionY, merc.fPositionZ,
                merc.fPositionX, merc.fPositionY, merc.fPositionZ, .65f, runtime.fSenseElapsed));
        runtime.fThreatAgeSeconds = risk > 0.f ? runtime.fThreatAgeSeconds + seconds : 0.f;
        const auto eligibleEnemy = [&](const SERVER_PLAYER& other)
        {
            return other.bColosseumParticipant && other.iColosseumMatchId == m_iColosseumMatchId &&
                other.iColosseumTeam < 2u && other.iColosseumTeam != merc.iColosseumTeam &&
                other.iCurrentHp && other.isCombatReady && std::abs(other.fPositionY - merc.fPositionY) < 3.f;
        };
        const SERVER_PLAYER* enemy = nullptr;
        for (const auto& [id, other] : m_Players)
            if (other.iNetEntityId == runtime.iTargetEntityId && eligibleEnemy(other)) enemy = &other;
        if (thinking || !enemy)
        {
            const SERVER_PLAYER* best = enemy; float bestScore = -1.f, retainedScore = -1.f;
            for (const auto& [id, other] : m_Players)
            {
                if (!eligibleEnemy(other)) continue;
                const float distance = std::hypot(other.fPositionX - merc.fPositionX, other.fPositionZ - merc.fPositionZ);
                float score = 6.f / (2.f + distance) + (1.f - HealthFraction(other)) * 1.5f;
                if (other.Has_TimeStop(m_iServerTick) || !DeadlineReached(m_iServerTick, other.iInvulnerableEndTick)) score *= .15f;
                if (other.eAction == PLAYER_ACTION_STATE::KNOCKDOWN) score += .7f;
                for (const auto& [allyId, ally] : m_Players)
                {
                    if (!ally.bColosseumParticipant || ally.iColosseumMatchId != m_iColosseumMatchId ||
                        ally.iColosseumTeam != merc.iColosseumTeam || !ally.iCurrentHp || allyId == playerId) continue;
                    // Help a pressured ally and mildly prefer the team's current focus.
                    if (HealthFraction(ally) < .45f &&
                        std::hypot(other.fPositionX - ally.fPositionX, other.fPositionZ - ally.fPositionZ) < 4.f) score += .6f;
                    const auto allyBrain = m_ColosseumMercenaries.find(allyId);
                    if (allyBrain != m_ColosseumMercenaries.end() && allyBrain->second.iTargetEntityId == other.iNetEntityId) score += .15f;
                }
                if (m_ServerNavigation.Is_Loaded() && !m_ServerNavigation.Has_LineOfSight(merc.fPositionX, merc.fPositionZ,
                    other.fPositionX, other.fPositionZ, merc.fPositionY)) score *= .5f;
                if (&other == enemy) retainedScore = score;
                if (score > bestScore) { best = &other; bestScore = score; }
            }
            if (enemy && best != enemy && (bestScore < retainedScore * 1.25f ||
                (m_iServerTick - runtime.iTargetSelectedTick < 30u && bestScore < retainedScore * 1.7f))) best = enemy;
            if (best != enemy || !enemy)
            {
                runtime.iTargetEntityId = best ? best->iNetEntityId : INVALID_NET_ENTITY_ID;
                runtime.iTargetSelectedTick = m_iServerTick;
            }
            enemy = best;
        }
        if (merc.bPatternBound || merc.fKnockbackRemainingSeconds > 0.f || merc.TriggerMove.isActive ||
            merc.Has_TimeStop(m_iServerTick) || merc.eAction == PLAYER_ACTION_STATE::DEAD || merc.eAction == PLAYER_ACTION_STATE::FALLING)
        { runtime.pReason = "Waiting for crowd control to end"; continue; }
        const auto altVReady = [&](const PLAYER_SKILL_DEFINITION& skill)
        {
            return skill.strInputSlot != "ALT_V" || !runtime.iLastAltVAdmissionTick ||
                m_iServerTick - *runtime.iLastAltVAdmissionTick >= MERCENARY_ALT_V_INTERVAL_TICKS;
        };
        const auto trySkill = [&](const PLAYER_SKILL_DEFINITION& skill, float x, float z)
        {
            if (!altVReady(skill) || !SkillResourcesReady(merc, skill, m_iServerTick + 1u)) return false;
            C2S_USE_SKILL command;
            command.iClientSequence = ++runtime.iSequence; command.iSkillId = skill.iSkillId;
            command.eTargetIntent = skill.eTargetIntent; command.fAimX = x; command.fAimZ = z;
            if (!Execute_PlayerSkill(merc, command) || merc.eAction != PLAYER_ACTION_STATE::SKILL ||
                merc.iCurrentSkillId != skill.iSkillId || merc.PendingCommand.eKind != PLAYER_PENDING_COMMAND_KIND::NONE) return false;
            // Only an actual admission changes memory, never a pending COMBO command.
            if (skill.strInputSlot == "ALT_V") runtime.iLastAltVAdmissionTick = m_iServerTick;
            else if (!Is_DodgeSkill(skill) && skill.eSkillKind != PLAYER_SKILL_KIND::STANDUP)
            {
                runtime.iLastSkillId = skill.iSkillId;
                runtime.RecentSkills[runtime.iRecentSkillCursor++ % runtime.RecentSkills.size()] = skill.iSkillId;
            }
            return true;
        };
        const auto moveTo = [&](const SERVER_NAV_POINT& point)
        {
            if (merc.eAction != PLAYER_ACTION_STATE::NONE && !Is_MoveCancellableAction(merc)) return false;
            if (merc.hasMoveGoal && std::hypot(merc.fMoveGoalX - point.x, merc.fMoveGoalZ - point.z) < .05f) return true;
            C2S_MOVE move; move.iClientSequence = ++runtime.iSequence; move.fGoalX = point.x; move.fGoalZ = point.z;
            Execute_PlayerMove(merc, move);
            return merc.hasMoveGoal && merc.PendingCommand.eKind == PLAYER_PENDING_COMMAND_KIND::NONE &&
                std::hypot(merc.fMoveGoalX - point.x, merc.fMoveGoalZ - point.z) < .05f;
        };
        const float hp = HealthFraction(merc);
        unsigned nearbyEnemies = 0u, nearbyAllies = 0u;
        for (const auto& [id, other] : m_Players)
        {
            if (id == playerId || !other.bColosseumParticipant || other.iColosseumMatchId != m_iColosseumMatchId || !other.iCurrentHp) continue;
            if (std::hypot(other.fPositionX - merc.fPositionX, other.fPositionZ - merc.fPositionZ) > 6.f) continue;
            if (other.iColosseumTeam == merc.iColosseumTeam) ++nearbyAllies; else ++nearbyEnemies;
        }
        const float nearest = enemy ? std::hypot(enemy->fPositionX - merc.fPositionX, enemy->fPositionZ - merc.fPositionZ) : 0.f;
        // Range preference follows the currently available attacks, not a class-name script.
        float preferredRange = 2.f, reachSum = 0.f; unsigned reachCount = 0u;
        for (const auto id : runtime.AvailableSkills)
            if (const auto* skill = m_GameplayCatalog.Find_Skill(id); skill && skill->strInputSlot != "ALT_V" &&
                !Is_DodgeSkill(*skill) && skill->eSkillKind != PLAYER_SKILL_KIND::STANDUP &&
                SkillResourcesReady(merc, *skill, m_iServerTick + 1u))
            { reachSum += std::clamp(SkillReach(*skill) * .65f, 1.5f, 7.f); ++reachCount; }
        if (reachCount) preferredRange = reachSum / reachCount;
        const auto choosePosition = [&](bool escape, bool retreat, float radius, float speed, SERVER_NAV_POINT& out)
        {
            bool found = false; float bestScore = (std::numeric_limits<float>::max)();
            const auto scorePoint = [&](const SERVER_NAV_POINT& point)
            {
                const float distance = std::hypot(point.x - merc.fPositionX, point.z - merc.fPositionZ);
                const float travel = distance / (std::max)(speed, 1.f);
                const float endRisk = runtime.Threats.At(point.x, point.y, point.z, runtime.fSenseElapsed + travel).risk;
                const float pathRisk = runtime.Threats.Along(merc.fPositionX, merc.fPositionY, merc.fPositionZ,
                    point.x, point.y, point.z, travel, runtime.fSenseElapsed);
                float score = endRisk * 10.f + pathRisk * 4.f;
                float closestEnemy = 100.f, closestAlly = 100.f;
                for (const auto& [id, other] : m_Players)
                {
                    if (id == playerId || !other.bColosseumParticipant || other.iColosseumMatchId != m_iColosseumMatchId || !other.iCurrentHp) continue;
                    const float gap = std::hypot(point.x - other.fPositionX, point.z - other.fPositionZ);
                    if (other.iColosseumTeam == merc.iColosseumTeam) closestAlly = (std::min)(closestAlly, gap);
                    else closestEnemy = (std::min)(closestEnemy, gap);
                    if (gap < 1.4f) score += (1.4f - gap) * 2.f;
                }
                if (retreat) score -= (std::min)(closestEnemy, 8.f) * .8f;
                else if (enemy) score += std::abs(std::hypot(point.x - enemy->fPositionX, point.z - enemy->fPositionZ) - preferredRange) * (escape ? .08f : .4f);
                if (retreat && closestAlly < 100.f) score += (std::max)(0.f, closestAlly - 4.f) * .12f;
                if (merc.hasMoveGoal && std::hypot(point.x - merc.fMoveGoalX, point.z - merc.fMoveGoalZ) < .6f) score -= .35f;
                return std::pair{score, endRisk};
            };
            const float stay = scorePoint({merc.fPositionX, merc.fPositionY, merc.fPositionZ}).first;
            for (unsigned i = 0u; i < 25u; ++i)
            {
                if (i == 24u && !merc.hasMoveGoal) continue;
                const float angle = float(i % 12u) * PI / 6.f;
                const float length = i < 12u ? radius : radius * .5f;
                const float x = i == 24u ? merc.fMoveGoalX : merc.fPositionX + std::sin(angle) * length;
                const float z = i == 24u ? merc.fMoveGoalZ : merc.fPositionZ + std::cos(angle) * length;
                SERVER_NAV_POINT point;
                if (!Find_GuideLanding(merc, x, merc.fPositionY, z, point) ||
                    !m_ServerNavigation.Has_LineOfSight(merc.fPositionX, merc.fPositionZ, point.x, point.z, merc.fPositionY)) continue;
                const auto [score, endRisk] = scorePoint(point);
                if (escape && endRisk >= risk) continue;
                if (score >= bestScore || (!escape && score >= stay - .15f)) continue;
                // Leaving the current hit may cross its boundary, but a second
                // dangerous zone must not be preferred just for a safe endpoint.
                if (escape && score > stay + 1.f) continue;
                out = point; bestScore = score; found = true;
            }
            return found;
        };
        const auto chooseDodge = [&](const PLAYER_SKILL_DEFINITION& skill, SERVER_NAV_POINT& aim, bool& releaseHold)
        {
            DODGE_MOTION motion;
            if (!ReadDodgeMotion(skill, seconds, motion)) return false;
            bool found = false; float bestScore = (std::numeric_limits<float>::max)();
            const float stayRisk = runtime.Threats.Along(merc.fPositionX, merc.fPositionY, merc.fPositionZ,
                merc.fPositionX, merc.fPositionY, merc.fPositionZ, .65f, runtime.fSenseElapsed);
            for (unsigned direction = 0u; direction < 12u; ++direction)
            {
                const float angle = float(direction) * PI / 6.f;
                const float forwardX = std::sin(angle), forwardZ = std::cos(angle);
                SERVER_PLAYER pose;
                pose.iMarioStage = merc.iMarioStage;
                pose.fPositionX = merc.fPositionX; pose.fPositionY = merc.fPositionY; pose.fPositionZ = merc.fPositionZ;
                bool valid = true; float pathRisk = 0.f;
                for (std::size_t step = 1u; step < motion.count; ++step)
                {
                    const auto& previous = motion.points[step - 1u]; const auto& current = motion.points[step];
                    const float deltaForward = current.forward - previous.forward;
                    const float deltaLateral = current.lateral - previous.lateral;
                    SERVER_NAV_POINT next{pose.fPositionX + forwardX * deltaForward + forwardZ * deltaLateral,
                        pose.fPositionY, pose.fPositionZ + forwardZ * deltaForward - forwardX * deltaLateral};
                    if (deltaForward != 0.f || deltaLateral != 0.f)
                    {
                        bool clamped = false;
                        if (m_ServerNavigation.Is_Loaded())
                            CPlayerSkillSystem::Clamp_StepToWalkable(m_ServerNavigation,
                                pose.fPositionX, pose.fPositionZ, next.x, next.z, next, clamped, pose.fPositionY);
                        // Reject an altered route, instead of inventing where a
                        // native wall clamp or body slide would eventually land.
                        if (clamped) { valid = false; break; }
                        float x = next.x, y = next.y, z = next.z; bool blocked = false;
                        if (!m_ServerCollisionSystem.Resolve_PlayerMove(pose, next.x, next.y, next.z, x, y, z, blocked) ||
                            blocked || std::abs(x-next.x) > .001f || std::abs(y-next.y) > .001f || std::abs(z-next.z) > .001f)
                        { valid = false; break; }
                    }
                    pathRisk = (std::max)(pathRisk, runtime.Threats.Along(pose.fPositionX, pose.fPositionY, pose.fPositionZ,
                        next.x, next.y, next.z, current.time-previous.time, runtime.fSenseElapsed+previous.time, 0.f));
                    pose.fPositionX = next.x; pose.fPositionY = next.y; pose.fPositionZ = next.z;
                }
                if (!valid || std::hypot(pose.fPositionX-merc.fPositionX, pose.fPositionZ-merc.fPositionZ) < .25f) continue;
                const float endRisk = runtime.Threats.At(pose.fPositionX, pose.fPositionY, pose.fPositionZ,
                    runtime.fSenseElapsed+motion.points[motion.count-1u].time).risk;
                if (risk > 0.f && (endRisk >= risk || pathRisk > stayRisk)) continue;
                float score = endRisk * 10.f + pathRisk * 4.f;
                if (enemy) score += std::abs(std::hypot(pose.fPositionX-enemy->fPositionX,
                    pose.fPositionZ-enemy->fPositionZ)-preferredRange) * .08f;
                for (const auto& [id, other] : m_Players)
                {
                    if (id == playerId || !other.bColosseumParticipant || other.iColosseumMatchId != m_iColosseumMatchId || !other.iCurrentHp) continue;
                    const float gap = std::hypot(pose.fPositionX-other.fPositionX, pose.fPositionZ-other.fPositionZ);
                    if (gap < 1.4f) score += (1.4f-gap) * 2.f;
                }
                if (score >= bestScore) continue;
                // Aim selects a direction only. It is deliberately distinct from
                // the predicted landing; native motion never caps at aim distance.
                aim = {merc.fPositionX+forwardX, merc.fPositionY, merc.fPositionZ+forwardZ};
                releaseHold = motion.releaseHold; bestScore = score; found = true;
            }
            return found;
        };
        if (merc.eAction == PLAYER_ACTION_STATE::KNOCKDOWN)
        {
            if (thinking)
                for (const auto id : runtime.AvailableSkills)
                    if (const auto* skill = m_GameplayCatalog.Find_Skill(id); skill && skill->eSkillKind == PLAYER_SKILL_KIND::STANDUP)
                    {
                        SERVER_NAV_POINT goal{merc.fPositionX, merc.fPositionY, merc.fPositionZ};
                        bool releaseHold = false;
                        (void)chooseDodge(*skill, goal, releaseHold);
                        if (trySkill(*skill, goal.x, goal.z))
                        { runtime.eTactic = COLOSSEUM_TACTIC::RECOVER; runtime.pReason = "Stand-up toward a safer position"; break; }
                    }
            continue;
        }
        const auto* running = merc.eAction == PLAYER_ACTION_STATE::SKILL ? m_GameplayCatalog.Find_Skill(merc.iCurrentSkillId) : nullptr;
        if (risk > 0.f && runtime.fThreatAgeSeconds >= .065f && DeadlineReached(m_iServerTick, runtime.iNextEvadeTick))
        {
            bool escaped = false;
            // The COMBO executor buffers different skills before checking cancel
            // admission. Preserve that boundary; a queued SPACE is not a dodge.
            if (!running || running->eSkillKind != PLAYER_SKILL_KIND::COMBO)
                for (const auto id : runtime.AvailableSkills)
                    if (const auto* skill = m_GameplayCatalog.Find_Skill(id); skill && Is_DodgeSkill(*skill) &&
                        SkillResourcesReady(merc, *skill, m_iServerTick + 1u))
                    {
                        SERVER_NAV_POINT aim; bool releaseHold = false;
                        if (chooseDodge(*skill, aim, releaseHold) && trySkill(*skill, aim.x, aim.z))
                        {
                            if (releaseHold)
                            {
                                C2S_RELEASE_SKILL release; release.iClientSequence = ++runtime.iSequence; release.iSkillId = skill->iSkillId;
                                m_PlayerSkillSystem.Release(merc, release, m_GameplayCatalog);
                            }
                            escaped = true; runtime.pReason = "Dodge admitted along its authored motion route"; break;
                        }
                    }
            if (!escaped)
            {
                SERVER_NAV_POINT safe;
                if (choosePosition(true, false, 3.f, merc.fMoveSpeed, safe) && moveTo(safe))
                { escaped = true; runtime.pReason = "Walking out of a predicted attack"; }
            }
            runtime.iNextEvadeTick = m_iServerTick + (escaped ? 6u : 3u);
            if (escaped)
            { runtime.eTactic = COLOSSEUM_TACTIC::EVADE; runtime.iTacticUntilTick = m_iServerTick + 9u; continue; }
        }
        // Native staged inputs stay tick-accurate even while the tactical clock sleeps.
        if (running && running->eSkillKind == PLAYER_SKILL_KIND::COMBO && enemy &&
            !merc.hasBufferedComboInput && merc.iComboStage > 0u && merc.iComboStage < running->ComboStages.size())
        {
            const auto& stage = running->ComboStages[merc.iComboStage - 1u];
            const float nowMs = merc.fActionElapsedSeconds * 1000.f;
            if (stage.iInputCloseMs && nowMs >= stage.iInputOpenMs && nowMs <= stage.iInputCloseMs)
            {
                C2S_USE_SKILL continuation;
                continuation.iClientSequence = ++runtime.iSequence; continuation.iSkillId = running->iSkillId;
                continuation.eTargetIntent = running->eTargetIntent;
                continuation.fAimX = enemy->fPositionX; continuation.fAimZ = enemy->fPositionZ;
                (void)Execute_PlayerSkill(merc, continuation);
                if (merc.hasBufferedComboInput) runtime.pReason = "Buffered the current skill continuation";
            }
        }
        if (running && running->eSkillKind == PLAYER_SKILL_KIND::HOLD && !merc.hasReleasedHold &&
            merc.fActionElapsedSeconds >= (risk > 0.f ? .25f : 1.f))
        {
            C2S_RELEASE_SKILL release; release.iClientSequence = ++runtime.iSequence; release.iSkillId = running->iSkillId;
            m_PlayerSkillSystem.Release(merc, release, m_GameplayCatalog);
        }
        if (!thinking) continue;
        if (!enemy) { runtime.eTactic = COLOSSEUM_TACTIC::WAIT; runtime.pReason = "No living opponent in this match"; continue; }
        if (runtime.eTactic == COLOSSEUM_TACTIC::EVADE && !DeadlineReached(m_iServerTick, runtime.iTacticUntilTick)) continue;
        const bool hurtRecently = runtime.iLastDamageTick && m_iServerTick - runtime.iLastDamageTick < 45u;
        if (runtime.eTactic == COLOSSEUM_TACTIC::RETREAT && DeadlineReached(m_iServerTick, runtime.iTacticUntilTick))
        {
            // PvP offers no passive full heal: retreat buys space, then counterattack.
            runtime.eTactic = COLOSSEUM_TACTIC::ENGAGE;
            runtime.iRetreatAllowedTick = m_iServerTick + 60u;
        }
        else if (runtime.eTactic != COLOSSEUM_TACTIC::RETREAT && DeadlineReached(m_iServerTick, runtime.iRetreatAllowedTick) &&
            ((hp < .3f && (nearbyEnemies || hurtRecently)) || (hp < .55f && nearbyEnemies > nearbyAllies + 1u)))
        { runtime.eTactic = COLOSSEUM_TACTIC::RETREAT; runtime.iTacticUntilTick = m_iServerTick + 45u; }
        if (runtime.eTactic == COLOSSEUM_TACTIC::RETREAT)
        {
            SERVER_NAV_POINT safe;
            if (choosePosition(false, true, 4.f, merc.fMoveSpeed, safe) && moveTo(safe))
            { runtime.pReason = "Regrouping away from pressure before re-engaging"; continue; }
        }
        if (merc.eAction != PLAYER_ACTION_STATE::NONE)
        { runtime.pReason = "Preserving the admitted skill and its native input windows"; continue; }
        if (risk > 0.f)
        { runtime.pReason = "Threat remains; withholding a new attack commitment"; continue; }
        if (!DeadlineReached(m_iServerTick, runtime.iNextAttackTick)) continue;
        struct CHOICE { const PLAYER_SKILL_DEFINITION* skill; float weight, x, z; };
        std::vector<CHOICE> choices;
        const PLAYER_SKILL_DEFINITION* awakening = nullptr;
        for (const auto id : runtime.AvailableSkills)
        {
            const auto* skill = m_GameplayCatalog.Find_Skill(id);
            if (!skill || Is_DodgeSkill(*skill) || skill->eSkillKind == PLAYER_SKILL_KIND::STANDUP ||
                !SkillResourcesReady(merc, *skill, m_iServerTick + 1u) || nearest > SkillReach(*skill)) continue;
            if (skill->strInputSlot == "ALT_V") { if (altVReady(*skill)) awakening = skill; continue; }
            float weight = skill->strInputSlot == "LMB" ? .45f : 1.f;
            const float castSeconds = (std::max)(.15f, skill->iActionDurationMs * .001f);
            weight *= 1.f / (1.f + castSeconds * static_cast<float>(nearbyEnemies) * .25f);
            if (enemy->eAction == PLAYER_ACTION_STATE::KNOCKDOWN) weight *= 1.f + (std::min)(castSeconds, 2.f);
            if (HealthFraction(*enemy) < .3f) weight *= 1.f + 1.f / castSeconds;
            if (skill->eSetsStance != PLAYER_STANCE_ID::NONE) weight *= reachCount <= 2u ? 2.f : .35f;
            if (const auto* buffs = m_GameplayCatalog.Active().Find_SkillBuffs(id))
                for (const auto& buff : *buffs)
                    if (buff.eTarget != CGameplayCatalog::SKILL_BUFF_TARGET::ENEMY)
                    {
                        const bool active = std::any_of(merc.ActiveBuffs.begin(), merc.ActiveBuffs.end(),
                            [&](const auto& row) { return row.iBuffId == buff.iBuffId; });
                        weight *= active ? .25f : (buff.iShieldPercentOfMaxHp && hp < .6f ? 2.5f : 1.2f);
                    }
            if (!DeadlineReached(m_iServerTick, enemy->iInvulnerableEndTick) || enemy->Has_TimeStop(m_iServerTick)) weight *= .05f;
            if (id == runtime.iLastSkillId) weight *= .12f;
            else if (std::find(runtime.RecentSkills.begin(), runtime.RecentSkills.end(), id) != runtime.RecentSkills.end()) weight *= .5f;
            float x = enemy->fPositionX, z = enemy->fPositionZ;
            if (enemy->hasMoveGoal)
            {
                const float dx = enemy->fMoveGoalX - x, dz = enemy->fMoveGoalZ - z;
                const float length = std::hypot(dx, dz);
                const float lead = (std::min)(length, enemy->fMoveSpeed * std::clamp(skill->iHitTimeMs * .001f, .1f, .35f));
                if (length > .01f) { x += dx * lead / length; z += dz * lead / length; }
            }
            choices.push_back({skill, (std::max)(.01f, weight), x, z});
        }
        bool started = false;
        // Awakening has its own tactical opportunity and 30-second admission gate.
        if (awakening && (choices.empty() || nearbyEnemies >= 2u || HealthFraction(*enemy) < .4f) &&
            trySkill(*awakening, enemy->fPositionX, enemy->fPositionZ))
        { runtime.pReason = "Awakening admitted at a tactical opportunity"; started = true; }
        while (!started && !choices.empty())
        {
            float total = 0.f; for (const auto& choice : choices) total += choice.weight;
            float draw = RandomUnit(runtime.iRandomState) * total;
            std::size_t index = choices.size() - 1u;
            for (std::size_t i = 0u; i < choices.size(); ++i)
                if ((draw -= choices[i].weight) <= 0.f) { index = i; break; }
            const auto selected = choices[index];
            started = trySkill(*selected.skill, selected.x, selected.z);
            choices.erase(choices.begin() + index);
            if (started) runtime.pReason = "Weighted skill choice matched range, resources and pressure";
        }
        if (started)
        {
            runtime.eTactic = COLOSSEUM_TACTIC::ENGAGE;
            // The admitted action owns its real duration, including early HOLD
            // release and interruption. Never wait for a nominal duration afterward.
            runtime.iNextAttackTick = m_iServerTick + 3u +
                static_cast<std::uint32_t>(RandomUnit(runtime.iRandomState) * 6.f);
            continue;
        }
        SERVER_NAV_POINT position;
        if (choosePosition(false, false, 3.f, merc.fMoveSpeed, position) && moveTo(position))
        { runtime.eTactic = COLOSSEUM_TACTIC::REPOSITION; runtime.pReason = "Repositioning for a useful attack range"; }
        else { runtime.eTactic = COLOSSEUM_TACTIC::WAIT; runtime.pReason = "Holding position while useful skills recover"; }
    }
}
```

### Server/Private/ServerGameplayContractTests_ColosseumMatch.cpp

```cpp
#include "ServerGameplayContractTests_Runner.h"
#include "ServerApp.h"
#include "ClientSession.h"
#include "ColosseumCombatPolicy.h"
#include "ColosseumThreatAssessment.h"
#include "Network/PacketWriter.h"
#include "Network/PacketReader.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <memory>
#include <set>
#include <string>
#include <vector>

using namespace LostArk::Server;
using namespace LostArk::Shared;

int CServerGameplayContractRunner::Run_ColosseumMatchContracts()
{
    TESTS tests;
    auto app = std::make_unique<CServerApp>();
    auto source = std::make_shared<CGameRoom>(WORLD_ID::BERN);
    tests.Require(source->Is_Ready(), "Colosseum match fixture loads real Bern generation");
    if (!source->Is_Ready()) return 1;
    app->m_pActiveGameplayGeneration = source->Get_ActiveGameplayGeneration();
    app->m_SharedGameRooms.emplace(WORLD_ID::BERN, source);
    std::vector<std::shared_ptr<CClientSession>> sessions;
    const auto drain = [&]()
    {
        for (const auto& session : sessions)
        {
            session->m_OutboundFrames.clear(); session->m_iQueuedOutboundBytes = 0u;
            session->m_OutboundMetrics.iCurrentQueuedByteCount = 0u;
            session->m_OutboundMetrics.iCurrentQueuedFrameCount = 0u;
        }
    };
    for (unsigned i = 0; i < 8u; ++i)
    {
        const SESSION_ID id = 991000u + i;
        auto session = std::make_shared<CClientSession>(id, INVALID_SOCKET,
            CClientSession::FRAME_HANDLER{}, CClientSession::CLOSED_HANDLER{});
        session->m_isSendRunning.store(true);
        source->Handle_Register(session);
        C2S_ENTER_WORLD enter;
        enter.eWorldId = WORLD_ID::BERN;
        enter.eCharacterClass = i < 4u ? CHARACTER_CLASS_ID::GUARDIANKNIGHT : CHARACTER_CLASS_ID::LANCE_MASTER;
        enter.strNickName = "Colosseum" + std::to_string(i);
        const bool joined = source->Join(id, enter, "npc.bern.25184_1.2");
        tests.Require(joined, "Human session admitted through existing Bern admission");
        if (!joined) return 1;
        sessions.push_back(session);
        app->m_Sessions.emplace(id, session);
        app->m_GameplayBindingBySessionId.emplace(id,
            CServerApp::SESSION_GAMEPLAY_BINDING{ WORLD_ID::BERN, INVALID_SESSION_ID, source });
        auto& player = source->m_Players.at(session->Get_PlayerId());
        player.Inventory.clear(); player.Purse.iSilver = 77u + i;
        drain();
    }
    for (unsigned count = 1u; count <= 4u; ++count)
    {
        source->m_iServerTick = 1000u; source->m_iColosseumQueueDeadline = 0u;
        source->m_ColosseumQueue.clear(); source->m_PendingWorldTransfers.clear(); source->m_bColosseumTransferPending = false;
        for (unsigned i = 0u; i < count; ++i) source->m_ColosseumQueue.push_back({ sessions[i]->Get_SessionId(), i + 1u });
        source->Try_FormColosseumMatch();
        tests.Require(source->m_iColosseumQueueDeadline == 1300u && source->m_PendingWorldTransfers.empty(),
            "One through four humans open exactly a ten-second collection window");
        source->m_iServerTick = 1299u; source->Try_FormColosseumMatch();
        tests.Require(source->m_PendingWorldTransfers.empty(), "No early match before the shared collection deadline");
        source->m_iServerTick = 1300u; source->Try_FormColosseumMatch();
        tests.Require(source->m_PendingWorldTransfers.size() == 1u &&
            source->m_PendingWorldTransfers.front().PartyBatchSessionIds.size() == count && source->m_ColosseumQueue.size() == count,
            "One through four humans reserve the exact present count transactionally at the deadline");
    }
    source->m_ColosseumQueue.clear(); source->m_PendingWorldTransfers.clear(); source->m_bColosseumTransferPending = false;
    source->m_iServerTick = 0u; source->m_iColosseumQueueDeadline = 0u;
    drain();
    for (unsigned count = 1u; count <= 4u; ++count)
    {
        auto smallSource = std::make_shared<CGameRoom>(WORLD_ID::BERN, app->m_pActiveGameplayGeneration);
        auto smallTarget = std::make_shared<CGameRoom>(WORLD_ID::COLOSSEUM, app->m_pActiveGameplayGeneration);
        std::vector<std::shared_ptr<CClientSession>> peers;
        std::vector<SESSION_ID> ordered;
        bool admitted = smallSource->Is_Ready() && smallTarget->Is_Ready();
        for (unsigned i = 0u; i < count && admitted; ++i)
        {
            const SESSION_ID id = 998000u + count * 10u + i;
            auto peer = std::make_shared<CClientSession>(id, INVALID_SOCKET,
                CClientSession::FRAME_HANDLER{}, CClientSession::CLOSED_HANDLER{});
            peer->m_isSendRunning.store(true); smallSource->Handle_Register(peer);
            C2S_ENTER_WORLD enter;
            enter.eWorldId = WORLD_ID::BERN; enter.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
            enter.strNickName = "Flexible" + std::to_string(i);
            admitted = smallSource->Join(id, enter, "npc.bern.25184_1.2");
            if (admitted) smallSource->m_ColosseumQueue.push_back({id, i + 1u});
            peers.push_back(peer); ordered.push_back(id);
        }
        std::string status;
        const bool committed = admitted && smallSource->Transfer_ColosseumMatchTo(*smallTarget, ordered, 9000u + count, status);
        tests.Require(committed, "One through four humans each commit through the real atomic Colosseum admission");
        if (!committed) std::cout << "Flexible admission count=" << count << " status=" << status << '\n';
        if (committed)
        {
            tests.Require(smallTarget->Count_HumanPlayers() == count && smallTarget->m_Players.size() == count + 10u,
                "Flexible admission keeps exactly the admitted humans and ten sessionless mercenaries");
            for (std::size_t i = 0; i < ordered.size(); ++i)
            {
                const auto& p = smallTarget->m_Players.at(smallTarget->m_PlayerIdBySessionId.at(ordered[i]));
                tests.Require(p.iColosseumTeam == i % 2u && p.iColosseumArrivalIndex == i,
                    "Flexible admission preserves the accepted order and parity team");
                smallTarget->Handle_ColosseumLoadReady(ordered[i], {smallTarget->m_iColosseumMatchId});
            }
            const auto autoSelected = std::count_if(smallTarget->m_Players.begin(), smallTarget->m_Players.end(),
                [](const auto& p) { return p.second.Is_ColosseumMercenary() && p.second.bColosseumParticipant; });
            tests.Require(autoSelected == (count == 1u ? 4 : 0),
                "Only solo admission automatically selects four opposite-team mercenaries");
            tests.Require(std::all_of(smallTarget->m_Players.begin(), smallTarget->m_Players.end(), [&](const auto& entry) {
                const auto& p = entry.second;
                if (!p.Is_ColosseumMercenary()) return true;
                if (p.iSessionId != INVALID_SESSION_ID) return false;
                if (!p.bColosseumParticipant) return !smallTarget->m_PartyIdByPlayerId.contains(p.iPlayerId);
                const auto party = smallTarget->m_PartyIdByPlayerId.find(p.iPlayerId);
                return count == 1u && p.iColosseumTeam == 1u && p.iColosseumArrivalIndex < 8u &&
                    p.iColosseumArrivalIndex % 2u == p.iColosseumTeam && party != smallTarget->m_PartyIdByPlayerId.end() &&
                    party->second == smallTarget->m_ColosseumTeamPartyIds[1];
            }), "Automatic opponents use real stable combat seats and party membership without fake sessions");
            const auto initialState = smallTarget->Build_ColosseumState();
            CPacketWriter initialWriter;
            tests.Require(initialState.Participants.size() == count + (count == 1u ? 4u : 0u) &&
                Write_Message(initialWriter, initialState),
                "Admission publishes only human participants and the explicit solo opponent team");
            const auto advance = [&](const std::uint32_t tick) {
                smallTarget->m_iServerTick = tick;
                smallTarget->Update_ColosseumMatch(tick);
            };
            advance(1u);
            tests.Require(smallTarget->m_eColosseumPhase == COLOSSEUM_MATCH_PHASE::RECRUITING,
                "Small matches wait for manual recruitment on each human-owned team");
            std::array<SESSION_ID, 2> recruiter{};
            for (const auto id : ordered)
            {
                const auto& player = smallTarget->m_Players.at(smallTarget->m_PlayerIdBySessionId.at(id));
                recruiter[player.iColosseumTeam] = id;
            }
            for (std::uint8_t team = 0u; team < 2u; ++team)
            {
                if (!recruiter[team]) continue;
                std::uint32_t sequence = 1u;
                for (const auto& [id, candidate] : smallTarget->m_Players)
                {
                    if (!candidate.Is_ColosseumMercenary() || candidate.iColosseumTeam != team) continue;
                    smallTarget->Handle_ColosseumRecruit(recruiter[team], {sequence++, smallTarget->m_iColosseumMatchId, candidate.iNetEntityId});
                }
            }
            const auto state = smallTarget->Build_ColosseumState();
            CPacketWriter writer;
            tests.Require(state.ePhase == COLOSSEUM_MATCH_PHASE::ENTRY_COUNTDOWN && state.Participants.size() == 8u &&
                state.iExpectedPlayers == 8u && state.iPhaseEndTick - state.iPhaseStartTick == 300u && Write_Message(writer, state) &&
                smallTarget->m_PartyMembersByPartyId.at(smallTarget->m_ColosseumTeamPartyIds[0]).size() == 4u &&
                smallTarget->m_PartyMembersByPartyId.at(smallTarget->m_ColosseumTeamPartyIds[1]).size() == 4u,
                "One through four humans reach two real four-person parties after explicit allied recruitment");
            const auto entryDeadline = smallTarget->m_iColosseumPhaseEnd;
            advance(entryDeadline - 1u);
            tests.Require(smallTarget->m_eColosseumPhase == COLOSSEUM_MATCH_PHASE::ENTRY_COUNTDOWN &&
                std::none_of(smallTarget->m_Players.begin(), smallTarget->m_Players.end(), [](const auto& p) { return p.second.bColosseumCombatActive; }),
                "Small matches cannot activate combat before the full ten-second entry countdown");
            advance(entryDeadline);
            tests.Require(smallTarget->m_eColosseumPhase == COLOSSEUM_MATCH_PHASE::INTRO &&
                smallTarget->m_iColosseumPhaseEnd == entryDeadline + 258u,
                "Small matches enter the real shared 8.6-second introduction");
            advance(smallTarget->m_iColosseumPhaseEnd);
            tests.Require(smallTarget->m_eColosseumPhase == COLOSSEUM_MATCH_PHASE::COUNTDOWN &&
                smallTarget->m_iColosseumPhaseEnd - smallTarget->m_iColosseumPhaseStart == 300u,
                "Small introductions advance to the real ten-second gate countdown");
            advance(smallTarget->m_iColosseumPhaseEnd);
            tests.Require(smallTarget->m_eColosseumPhase == COLOSSEUM_MATCH_PHASE::ACTIVE &&
                smallTarget->m_iColosseumPhaseEnd - smallTarget->m_iColosseumPhaseStart == 3600u &&
                std::count_if(smallTarget->m_Players.begin(), smallTarget->m_Players.end(), [](const auto& p) { return p.second.bColosseumCombatActive; }) == 8,
                "One through four humans activate eight combatants for exactly 120 seconds");
            smallTarget->Handle_ColosseumReturn(ordered.front(), {smallTarget->m_iColosseumMatchId});
            tests.Require(smallTarget->m_PendingWorldTransfers.empty(), "The typed result return rejects an active small match");
            const auto combatDeadline = smallTarget->m_iColosseumPhaseEnd;
            const std::uint8_t winningTeam = count == 2u ? 1u : 0u;
            smallTarget->m_iColosseumScores[winningTeam] = 2u;
            smallTarget->m_iColosseumScores[1u - winningTeam] = 1u;
            advance(combatDeadline - 1u);
            tests.Require(smallTarget->m_eColosseumPhase == COLOSSEUM_MATCH_PHASE::ACTIVE,
                "Small matches preserve combat until the full match deadline");
            advance(combatDeadline);
            const auto finished = smallTarget->Build_ColosseumState();
            CPacketWriter resultWriter;
            tests.Require(finished.ePhase == COLOSSEUM_MATCH_PHASE::FINISHED && finished.iWinningTeam == winningTeam &&
                std::count_if(finished.Participants.begin(), finished.Participants.end(), [winningTeam](const auto& row) { return row.iTeam == winningTeam; }) == 4 &&
                std::none_of(smallTarget->m_Players.begin(), smallTarget->m_Players.end(), [](const auto& p) { return p.second.isCombatReady; }) &&
                Write_Message(resultWriter, finished),
                "Small matches finish with an encodable four-person winning roster and no combat authority");

            // The fixture owns an isolated app binding so the result button's typed
            // command reaches the same atomic transfer used by the production Server.
            auto returnApp = std::make_unique<CServerApp>();
            returnApp->m_pActiveGameplayGeneration = app->m_pActiveGameplayGeneration;
            returnApp->m_SharedGameRooms.emplace(WORLD_ID::BERN, smallSource);
            returnApp->m_ColosseumMatches.emplace(smallTarget->m_iColosseumMatchId, smallTarget);
            for (const auto& peer : peers)
            {
                returnApp->m_Sessions.emplace(peer->Get_SessionId(), peer);
                returnApp->m_GameplayBindingBySessionId.emplace(peer->Get_SessionId(),
                    CServerApp::SESSION_GAMEPLAY_BINDING{WORLD_ID::COLOSSEUM, INVALID_SESSION_ID, smallTarget});
            }
            smallTarget->Handle_ColosseumReturn(ordered.front(), {smallTarget->m_iColosseumMatchId + 1u});
            tests.Require(smallTarget->m_PendingWorldTransfers.empty(), "The typed result return rejects another match identity");
            for (std::size_t index = 0; index < ordered.size(); ++index)
            {
                for (const auto& peer : peers)
                {
                    peer->m_OutboundFrames.clear(); peer->m_iQueuedOutboundBytes = 0u;
                    peer->m_OutboundMetrics.iCurrentQueuedByteCount = 0u;
                    peer->m_OutboundMetrics.iCurrentQueuedFrameCount = 0u;
                }
                const auto sessionId = ordered[index];
                smallTarget->Handle_ColosseumReturn(sessionId, {smallTarget->m_iColosseumMatchId});
                SERVER_WORLD_TRANSFER_REQUEST back;
                CServerApp::SESSION_WORLD_TRANSFER_FAILURE failure;
                const bool returned = smallTarget->Try_DequeueWorldTransfer(back) &&
                    returnApp->Transfer_SessionWorld(smallTarget, back, failure);
                tests.Require(returned, "Every small-match human completes the typed result return to Bern");
                if (!returned) { std::cout << "Flexible return count=" << count << " status=" << failure.strContext << '\n'; continue; }
                const auto& restored = smallSource->m_Players.at(smallSource->m_PlayerIdBySessionId.at(sessionId));
                const auto* profile = smallSource->m_GameplayCatalog.Find_Player(restored.eCharacterClass);
                tests.Require(profile && restored.eCharacterClass == CHARACTER_CLASS_ID::LANCE_MASTER &&
                    restored.strNickName == "Flexible" + std::to_string(index) &&
                    restored.iCurrentHp == profile->iMaximumHp && restored.iMaximumHp == profile->iMaximumHp &&
                    restored.iColosseumMatchId == 0u && !restored.bColosseumParticipant &&
                    !smallSource->m_PartyIdByPlayerId.contains(restored.iPlayerId) &&
                    returnApp->m_GameplayBindingBySessionId.at(sessionId).pSimulation == smallSource,
                    "Typed return preserves identity, restores normal health and clears match-party authority");
            }
            tests.Require(smallSource->Count_HumanPlayers() == count && smallTarget->Count_HumanPlayers() == 0u &&
                std::none_of(smallSource->m_Players.begin(), smallSource->m_Players.end(), [](const auto& p) { return p.second.Is_ColosseumMercenary(); }),
                "All small-match humans return while mercenaries stay out of Bern");
            std::cout << "Flexible lifecycle count=" << count << " autoOpponents=" << autoSelected << " returned=" << smallSource->Count_HumanPlayers() << '\n';
        }
        for (const auto& peer : peers) peer->m_isSendRunning.store(false);
    }
    for (unsigned i = 0; i < 3u; ++i) source->m_ColosseumQueue.push_back({ sessions[i]->Get_SessionId(), i + 1u });
    source->Try_FormColosseumMatch();
    tests.Require(source->m_PendingWorldTransfers.empty() && source->m_ColosseumQueue.size() == 3u,
        "Both configurations keep three humans waiting");
    source->m_ColosseumQueue.push_back({ sessions[3]->Get_SessionId(), 4u });
    source->Try_FormColosseumMatch();
    tests.Require(source->m_PendingWorldTransfers.empty(), "Even four humans wait for the same ten-second collection deadline");
    source->m_iServerTick = source->m_iColosseumQueueDeadline;
    source->Try_FormColosseumMatch();
    SERVER_WORLD_TRANSFER_REQUEST transfer;
    const bool staged = source->Try_DequeueWorldTransfer(transfer);
    tests.Require(staged && transfer.bColosseumMatch && transfer.PartyBatchSessionIds.size() == 4u &&
        source->m_ColosseumQueue.size() == 4u, "Deadline reserves one ordered immutable batch without erasing the queue");
    if (!staged) return 1;
    std::atomic_bool cancelBeforeLoad{ true };
    auto cancelledRoom = std::make_unique<CGameRoom>(WORLD_ID::COLOSSEUM, app->m_pActiveGameplayGeneration, &cancelBeforeLoad);
    tests.Require(!cancelledRoom->Is_Ready() && cancelledRoom->Get_Status() == "Room preparation cancelled",
        "Cooperative cancellation rejects construction before the first loader stage");
    // Synchronous preparation is explicit test setup only. Product admission must
    // consume a completed worker result and never load a room inside the transfer.
    app->m_PreparedColosseumRoom = std::make_shared<CGameRoom>(WORLD_ID::COLOSSEUM, app->m_pActiveGameplayGeneration);
    tests.Require(app->m_PreparedColosseumRoom->Is_Ready(), "Transfer fixture owns an already prepared room");
    // Exhaust one reliable FIFO after valid sessions and source bindings were staged.
    sessions[2]->m_iQueuedOutboundBytes = CClientSession::MAX_OUTBOUND_BYTE_COUNT;
    CServerApp::SESSION_WORLD_TRANSFER_FAILURE failure;
    tests.Require(!app->Transfer_SessionWorld(source, transfer, failure) &&
        failure.strContext.find("byte capacity is exhausted") != std::string::npos && source->Count_HumanPlayers() == 8u &&
        app->m_ColosseumMatches.empty() && source->m_ColosseumQueue.size() == 4u &&
        std::all_of(sessions.begin(), sessions.end(), [&](const auto& session) {
            return app->m_GameplayBindingBySessionId.at(session->Get_SessionId()).pSimulation == source;
        }), "Reliable capacity rejection preserves all four sources, queue and bindings");
    drain();
    const bool committed = app->Transfer_SessionWorld(source, transfer, failure);
    tests.Require(committed, "Real four-human navigation/profile/reliable transaction commits");
    if (!committed) { std::cout << failure.strContext << '\n'; return 1; }
    source->Notify_ColosseumTransferResult(true);
    app->m_PreparedColosseumRoom.reset();
    auto room = app->m_ColosseumMatches.begin()->second;
    tests.Require(source->Count_HumanPlayers() == 4u && source->m_ColosseumQueue.empty() &&
        room->m_Players.size() == 14u && room->Count_HumanPlayers() == 4u && room->m_ColosseumMercenaries.size() == 10u,
        "Commit creates one private four-human room and ten sessionless class candidates");
    for (const auto& [id, merc] : room->m_Players)
        if (merc.Is_ColosseumMercenary())
            std::cout << "Mercenary admitted team=" << static_cast<unsigned>(merc.iColosseumTeam)
                << " class=" << static_cast<unsigned>(merc.eCharacterClass) << " position="
                << merc.fPositionX << ',' << merc.fPositionY << ',' << merc.fPositionZ << '\n';
    const auto state = room->Build_ColosseumState();
    tests.Require(state.Players.size() == 14u && state.ePhase == COLOSSEUM_MATCH_PHASE::LOADING &&
        std::count_if(state.Players.begin(), state.Players.end(), [](const auto& p) { return p.bParticipant; }) == 4,
        "Persistent initial state marks only four humans as participants");
    const auto expectedReferenceHp = source->m_GameplayCatalog.Find_Boss("BOSS_VALTAN")->Get_DamageReferenceHp();
    const auto expectedMatchHp = expectedReferenceHp / 8u + (expectedReferenceHp % 8u ? 1u : 0u);
    tests.Require(expectedReferenceHp == 741285439u && expectedMatchHp == 92660680u &&
        source->m_GameplayCatalog.Find_Boss("BOSS_VALTAN")->iMaximumHp == 2100000000u,
        "New Colosseum admission preserves its original HP and damage scale when raid Valtan HP increases to 2.1 billion");
    tests.Require(std::all_of(room->m_Players.begin(), room->m_Players.end(), [expectedMatchHp, expectedReferenceHp](const auto& value) {
        return value.second.iMaximumHp == expectedMatchHp && value.second.iColosseumDamageReferenceHp == expectedReferenceHp && !value.second.isCombatReady &&
            (!value.second.Is_Human() || (value.second.Inventory.empty() && value.second.Purse.iSilver >= 77u));
    }), "Match has 20 Valtan HP bars with the full immutable damage reference and preserves empty inventory/purse");
    for (const auto& [id, ai] : room->m_ColosseumMercenaries)
    {
        const auto& merc = room->m_Players.at(id);
        std::vector<std::string> slots;
        bool valid = !ai.AvailableSkills.empty();
        for (const auto skillId : ai.AvailableSkills)
        {
            const auto* skill = room->m_GameplayCatalog.Find_Skill(skillId);
            valid = valid && skill &&
                skill->eCharacterClass == merc.eCharacterClass &&
                (skill->eRequiredStance == PLAYER_STANCE_ID::NONE || skill->eRequiredStance == merc.eStance);
            if (skill) slots.push_back(skill->strInputSlot);
        }
        tests.Require(valid, "Every mercenary stages actual class/stance bindings without a fixed Guide combo");
        {
            std::vector<SKILL_ID> allAvailable;
            for (const auto& [skillId, skill] : room->m_GameplayCatalog.Active().Get_Skills())
                if (skill.eCharacterClass == merc.eCharacterClass &&
                    (skill.eRequiredStance == PLAYER_STANCE_ID::NONE || skill.eRequiredStance == merc.eStance)) allAvailable.push_back(skillId);
            std::sort(allAvailable.begin(), allAvailable.end());
            auto admittedSkills = ai.AvailableSkills;
            std::sort(admittedSkills.begin(), admittedSkills.end());
            tests.Require(admittedSkills == allAvailable &&
                std::find(slots.begin(), slots.end(), "LMB") != slots.end() &&
                std::find(slots.begin(), slots.end(), "SPACE") != slots.end(),
                "All classes, including DimensionMaster, include every current published skill, LMB and SPACE");
        }
        std::cout << "Mercenary available class=" << static_cast<unsigned>(merc.eCharacterClass) << " slots=";
        for (const auto& slot : slots) std::cout << slot << ' ';
        std::cout << '\n';
    }
    tests.Require(room->m_Players.at(room->m_PlayerIdBySessionId.at(sessions[0]->Get_SessionId())).iColosseumTeam == 0u &&
        room->m_Players.at(room->m_PlayerIdBySessionId.at(sessions[1]->Get_SessionId())).iColosseumTeam == 1u &&
        room->m_Players.at(room->m_PlayerIdBySessionId.at(sessions[2]->Get_SessionId())).iColosseumTeam == 0u,
        "Acceptance order fixes teams 1/3 versus 2/4");
    for (unsigned i = 0u; i < 4u; ++i) room->Handle_ColosseumLoadReady(sessions[i]->Get_SessionId(), { room->m_iColosseumMatchId });
    room->Update_ColosseumMatch(1u);
    tests.Require(room->m_eColosseumPhase == COLOSSEUM_MATCH_PHASE::RECRUITING,
        "All human presentation readiness opens mercenary selection without combat authority");
    std::array<SESSION_ID, 2> recruiters{};
    std::array<std::vector<NET_ENTITY_ID>, 2> candidates;
    for (auto& [id, player] : room->m_Players)
        if (player.Is_Human()) recruiters[player.iColosseumTeam] = player.iSessionId;
        else candidates[player.iColosseumTeam].push_back(player.iNetEntityId);
    C2S_COLOSSEUM_RECRUIT request{ 1u, room->m_iColosseumMatchId, candidates[1][0] };
    room->Handle_ColosseumRecruit(recruiters[0], request);
    tests.Require(!room->m_Players.at(room->m_PlayerIdByEntityId.at(candidates[1][0])).bColosseumParticipant,
        "Recruit rejects the other team's candidate");
    for (std::uint8_t team = 0; team < 2u; ++team)
    {
        request.iRequestSequence = 2u; request.iMercenaryNetEntityId = candidates[team][0];
        room->Handle_ColosseumRecruit(recruiters[team], request);
        room->Handle_ColosseumRecruit(recruiters[team], request);
        tests.Require(room->m_PartyMembersByPartyId.at(room->m_ColosseumTeamPartyIds[team]).size() == 3u,
            "Duplicate recruit selects one mercenary exactly once");
        request.iRequestSequence = 3u; request.iMercenaryNetEntityId = candidates[team][1];
        room->Handle_ColosseumRecruit(recruiters[team], request);
    }
    tests.Require(room->m_eColosseumPhase == COLOSSEUM_MATCH_PHASE::ENTRY_COUNTDOWN &&
        room->m_iColosseumPhaseEnd - room->m_iColosseumPhaseStart == 300u &&
        std::count_if(room->m_Players.begin(), room->m_Players.end(), [](const auto& p) { return p.second.bColosseumParticipant; }) == 8,
        "Two selected mercenaries per team prepare eight participants and a ten-second entry countdown");
    const auto entryDeadline = room->m_iColosseumPhaseEnd;
    room->Update_ColosseumMatch(entryDeadline - 1u);
    tests.Require(room->m_eColosseumPhase == COLOSSEUM_MATCH_PHASE::ENTRY_COUNTDOWN &&
        std::none_of(room->m_Players.begin(), room->m_Players.end(), [](const auto& p) { return p.second.bColosseumCombatActive; }),
        "Entry countdown blocks all damage before the deadline");
    room->Update_ColosseumMatch(entryDeadline);
    tests.Require(room->m_eColosseumPhase == COLOSSEUM_MATCH_PHASE::INTRO && room->m_iColosseumPhaseEnd == entryDeadline + 258u,
        "Ten-second entry transitions to one shared 8.6-second lineup clock");
    room->Update_ColosseumMatch(room->m_iColosseumPhaseEnd);
    tests.Require(room->m_eColosseumPhase == COLOSSEUM_MATCH_PHASE::COUNTDOWN,
        "Lineup completion starts the original gate countdown");
    room->Update_ColosseumMatch(room->m_iColosseumPhaseEnd);
    tests.Require(room->m_eColosseumPhase == COLOSSEUM_MATCH_PHASE::ACTIVE,
        "Gate countdown completion enables eight participant combat");
    request.iRequestSequence = 4u; request.iMercenaryNetEntityId = candidates[0][2];
    room->Handle_ColosseumRecruit(recruiters[0], request);
    tests.Require(!room->m_Players.at(room->m_PlayerIdByEntityId.at(candidates[0][2])).bColosseumParticipant,
        "A third mercenary remains inert after the match starts");
    const auto before = room->m_Players.at(room->m_PlayerIdByEntityId.at(candidates[0][2]));
    room->Update_Colosseum(.25f);
    const auto& after = room->m_Players.at(before.iPlayerId);
    tests.Require(!after.hasMoveGoal && after.eAction == PLAYER_ACTION_STATE::NONE &&
        after.fPositionX == before.fPositionX && after.fPositionZ == before.fPositionZ,
        "Unselected candidate cannot acquire movement or a skill");
    tests.Require(std::any_of(room->m_Players.begin(), room->m_Players.end(), [](const auto& p) {
        return p.second.Is_ColosseumMercenary() && p.second.bColosseumParticipant &&
            (p.second.hasMoveGoal || p.second.eAction == PLAYER_ACTION_STATE::SKILL);
    }), "Selected mercenary targets the opposing team through the shared executor");
    for (unsigned i = 4; i < 8u; ++i) source->m_ColosseumQueue.push_back({ sessions[i]->Get_SessionId(), i + 1u });
    source->Try_FormColosseumMatch();
    source->m_iServerTick = source->m_iColosseumQueueDeadline;
    source->Try_FormColosseumMatch();
    SERVER_WORLD_TRANSFER_REQUEST second;
    const bool dequeued = source->Try_DequeueWorldTransfer(second);
    const auto beginTime = std::chrono::steady_clock::now();
    const bool preparing = dequeued && app->Begin_ColosseumPreparation(source, second);
    const auto launchMs = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - beginTime).count();
    tests.Require(preparing && app->m_ColosseumPreparationWorker.valid() && source->Count_HumanPlayers() == 4u &&
        app->m_ColosseumMatches.size() == 1u && source->m_ColosseumQueue.size() == 4u,
        "Async preparation retains four source humans, queue and authority until ready");
    tests.Require(!app->Begin_ColosseumPreparation(source, second), "Only one preparation worker can own the queued batch");
    std::cout << "Colosseum preparation launchMs=" << launchMs << '\n';
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::minutes(2);
    while (app->m_ColosseumPreparationWorker.valid() && std::chrono::steady_clock::now() < deadline)
    {
        app->Advance_ColosseumPreparation();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    const bool next = !app->m_ColosseumPreparationWorker.valid() && app->m_ColosseumMatches.size() == 2u;
    tests.Require(next && app->m_ColosseumMatches.size() == 2u,
        "The next four humans receive another private simulation");
    if (next)
    {
        const auto other = app->m_ColosseumMatches.rbegin()->second;
        tests.Require(other != room && other->m_iColosseumMatchId != room->m_iColosseumMatchId &&
            other->m_eColosseumPhase == COLOSSEUM_MATCH_PHASE::LOADING,
            "Match authority and recruitment state are isolated despite reused room-local entity IDs");
    }
    // An unresolved result must never block the tick or publish prepared authority.
    if (!app->m_ColosseumPreparationWorker.valid())
    {
        std::promise<CServerApp::COLOSSEUM_PREPARATION_RESULT> delayed;
        app->m_ColosseumPreparationWorker = delayed.get_future();
        app->m_ColosseumPreparationSource = source;
        app->m_ColosseumPreparationTransfer = second;
        app->m_ColosseumPreparationCancelled = std::make_shared<std::atomic_bool>(false);
        app->Advance_ColosseumPreparation();
        tests.Require(app->m_ColosseumPreparationWorker.valid() && app->m_ColosseumPreparationCancelled->load() &&
            app->m_ColosseumMatches.size() == 2u,
            "Pending future poll returns and cancels when its source binding has changed");
        delayed.set_value({});
        app->Advance_ColosseumPreparation();
        tests.Require(!app->m_ColosseumPreparationWorker.valid() && app->m_ColosseumMatches.size() == 2u,
            "A cancelled completion cannot create or rebind a match");
    }
    const auto defeatedSession = recruiters[1];
    auto& defeated = room->m_Players.at(room->m_PlayerIdBySessionId.at(defeatedSession));
    defeated.iCurrentHp = 0u; defeated.eAction = PLAYER_ACTION_STATE::DEAD;
    room->Handle_UseSquareHole(defeatedSession, C2S_USE_SQUAREHOLE{ 90u, 1u });
    tests.Require(room->m_PendingWorldTransfers.empty(), "An ACTIVE defeated human cannot bypass the arena through a square hole");
    for (auto& [id, player] : room->m_Players)
        if (player.iColosseumTeam == 1u && player.bColosseumParticipant) player.iCurrentHp = 0u;
    room->Update_Colosseum(.1f);
    tests.Require(room->m_eColosseumPhase == COLOSSEUM_MATCH_PHASE::ACTIVE, "A team death does not truncate the timed respawning match");
    room->m_iColosseumScores[0] = 1u;
    room->Update_ColosseumMatch(room->m_iColosseumPhaseEnd);
    tests.Require(room->m_eColosseumPhase == COLOSSEUM_MATCH_PHASE::FINISHED && room->m_iColosseumWinnerTeam == 0u &&
        std::none_of(room->m_Players.begin(), room->m_Players.end(), [](const auto& p) { return p.second.isCombatReady; }),
        "The 120-second deadline determines the score winner and closes every damage authority");
    drain();
    room->Handle_UseSquareHole(defeatedSession, C2S_USE_SQUAREHOLE{ 91u, 1u });
    SERVER_WORLD_TRANSFER_REQUEST returnToBern;
    const bool returned = room->Try_DequeueWorldTransfer(returnToBern) &&
        app->Transfer_SessionWorld(room, returnToBern, failure);
    tests.Require(returned, "A FINISHED defeated human returns through the normal atomic Bern transfer");
    if (returned)
    {
        const auto& human = source->m_Players.at(source->m_PlayerIdBySessionId.at(defeatedSession));
        const auto* profile = source->m_GameplayCatalog.Find_Player(human.eCharacterClass);
        std::cout << "Returned Guardian normal HP=" << human.iMaximumHp << " activeProfile=" << (profile ? profile->iMaximumHp : 0u) << '\n';
        tests.Require(profile && human.eCharacterClass == CHARACTER_CLASS_ID::GUARDIANKNIGHT &&
            human.iCurrentHp == profile->iMaximumHp && human.iMaximumHp == profile->iMaximumHp &&
            human.eAction == PLAYER_ACTION_STATE::NONE && human.iColosseumMatchId == 0u && human.iColosseumDamageReferenceHp == 0u && !human.bColosseumParticipant &&
            !source->m_PartyIdByPlayerId.contains(human.iPlayerId) &&
            std::none_of(source->m_Players.begin(), source->m_Players.end(), [](const auto& value) { return value.second.Is_ColosseumMercenary(); }),
            "Bern restores the actual normal Guardian profile HP and carries no match, mercenary or automatic team party authority");
    }
    // Drive the real AI decision and shared movement executor; malformed geometry
    // would take the attack branch instead of choosing a lower-risk navigation goal.
    for (unsigned shapeKind = 0; shapeKind < 5u; ++shapeKind)
    {
        auto catalog = std::make_shared<CGameplayCatalog>(source->m_GameplayCatalog.Active());
        auto* skill = const_cast<PLAYER_SKILL_DEFINITION*>(catalog->Find_Skill(34040u));
        tests.Require(skill != nullptr, "Evade fixture owns a mutable copy of a published caster skill");
        if (!skill) continue;
        skill->Hits.clear(); skill->ComboStages.clear();
        PLAYER_SKILL_HIT hit;
        hit.iAreaType = shapeKind < 2u ? 1u : shapeKind == 2u ? 2u : 3u;
        hit.fRange = 1.5f; hit.fWidth = shapeKind == 2u ? 3.f : 0.f;
        hit.fInner = shapeKind == 1u ? .2f : 0.f;
        hit.fAngleDegrees = shapeKind == 3u ? 60.f : 0.f;
        hit.iTimeMs = 100u;
        skill->Hits.push_back(hit);
        auto hazardRoom = std::make_unique<CGameRoom>(WORLD_ID::COLOSSEUM, catalog);
        SERVER_NAV_POINT landing;
        const bool standable = hazardRoom->Is_Ready() &&
            hazardRoom->m_ServerNavigation.Sample_Position(-20.4f, -2.6f, landing, 12.22f);
        tests.Require(standable, "Evade fixture uses the actual Colosseum navigation surface");
        if (!standable) continue;
        hazardRoom->m_iColosseumMatchId = 900u;
        hazardRoom->m_eColosseumPhase = COLOSSEUM_MATCH_PHASE::ACTIVE;
        const auto* profile = catalog->Find_Player(CHARACTER_CLASS_ID::LANCE_MASTER);
        SERVER_PLAYER merc;
        merc.iPlayerId = 1u; merc.iNetEntityId = 101u;
        merc.eControlKind = PLAYER_CONTROL_KIND::COLOSSEUM_MERCENARY_AI;
        merc.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
        merc.eStance = profile->eDefaultStance;
        merc.iCurrentHp = merc.iMaximumHp = expectedMatchHp;
        merc.iColosseumDamageReferenceHp = expectedReferenceHp;
        merc.iCurrentResource = merc.iMaximumResource = profile->iMaximumResource;
        merc.fMoveSpeed = profile->fMoveSpeed;
        merc.iColosseumMatchId = 900u; merc.iColosseumTeam = 0u;
        merc.bColosseumParticipant = true; merc.isCombatReady = true; merc.bColosseumCombatActive = true;
        merc.CooldownEndTickBySkillId[34020u] = 100000u; // Existing navigation fallback when SPACE is unavailable.
        merc.fPositionX = landing.x; merc.fPositionY = landing.y; merc.fPositionZ = landing.z;
        auto caster = merc;
        caster.iPlayerId = 2u; caster.iNetEntityId = 102u; caster.iColosseumTeam = 1u;
        caster.eControlKind = PLAYER_CONTROL_KIND::HUMAN;
        caster.eAction = PLAYER_ACTION_STATE::SKILL; caster.iCurrentSkillId = skill->iSkillId;
        caster.fYawDegrees = 0.f;
        hazardRoom->m_Players.emplace(merc.iPlayerId, merc);
        hazardRoom->m_Players.emplace(caster.iPlayerId, caster);
        hazardRoom->m_ColosseumMercenaries.emplace(merc.iPlayerId, CGameRoom::COLOSSEUM_MERCENARY_RUNTIME{});
        hazardRoom->Update_Colosseum(.25f);
        const auto& evading = hazardRoom->m_Players.at(merc.iPlayerId);
        tests.Require(evading.hasMoveGoal && evading.eAction == PLAYER_ACTION_STATE::NONE &&
            std::hypot(evading.fMoveGoalX - landing.x, evading.fMoveGoalZ - landing.z) > .5f &&
            hazardRoom->m_ColosseumMercenaries.at(merc.iPlayerId).Threats.At(evading.fMoveGoalX, evading.fPositionY, evading.fMoveGoalZ).risk == 0.f,
            "Circle/ring/box/cone/full-cone future hit selects an actual navigation escape instead of attacking");
        if (shapeKind == 0u)
        {
            auto& dodgeActor = hazardRoom->m_Players.at(merc.iPlayerId);
            dodgeActor = merc; dodgeActor.CooldownEndTickBySkillId.erase(34020u);
            hazardRoom->m_ColosseumMercenaries.at(merc.iPlayerId) = {};
            hazardRoom->Update_Colosseum(.25f);
            const auto* admittedDodge = catalog->Find_Skill(dodgeActor.iCurrentSkillId);
            tests.Require(admittedDodge && Is_DodgeSkill(*admittedDodge) && dodgeActor.eAction == PLAYER_ACTION_STATE::SKILL,
                "A non-DimensionMaster uses available SPACE toward the validated escape instead of suppressing dodge skills");
            // Real published manual/automatic chains, through the actual room AI
            // input and native skill Update. No stage is advanced by the test.
            const std::array<SKILL_ID, 10> chainSkills{ 17080u, 34160u, 34140u, 31210u, 49110u, 17080u, 17000u, 34010u, 31000u, 49000u };
            for (std::size_t chainIndex = 0u; chainIndex < chainSkills.size(); ++chainIndex)
            {
                auto* chain = const_cast<PLAYER_SKILL_DEFINITION*>(catalog->Find_Skill(chainSkills[chainIndex]));
                const auto* chainProfile = chain ? catalog->Find_Player(chain->eCharacterClass) : nullptr;
                tests.Require(chain && chainProfile && chain->ComboStages.size() >= 2u,
                    "Native continuation fixture owns the published COMBO stage contract");
                if (!chain || !chainProfile || chain->ComboStages.size() < 2u) continue;
                const auto originalOpen = chain->ComboStages.front().iInputOpenMs;
                const auto originalClose = chain->ComboStages.front().iInputCloseMs;
                if (chainIndex == 5u)
                {
                    // 260..290ms is missed by the 200ms tactical clock but contains
                    // a real 30 Hz fixed tick. Keep the native executor unchanged.
                    chain->ComboStages.front().iInputOpenMs = 260u;
                    chain->ComboStages.front().iInputCloseMs = 290u;
                }
                SERVER_PLAYER actor = merc;
                actor.eCharacterClass = chain->eCharacterClass; actor.eStance = chainProfile->eDefaultStance;
                actor.iCurrentResource = actor.iMaximumResource = chainProfile->iMaximumResource;
                actor.iMaximumIdentity = chainProfile->iMaximumIdentity;
                actor.fMoveSpeed = chainProfile->fMoveSpeed;
                CPlayerSkillSystem::Reset_Gauges(actor, hazardRoom->m_GameplayCatalog);
                auto& controlled = hazardRoom->m_Players.at(merc.iPlayerId);
                controlled = actor;
                auto& target = hazardRoom->m_Players.at(caster.iPlayerId);
                target.eAction = PLAYER_ACTION_STATE::NONE; target.iCurrentSkillId = INVALID_SKILL_ID;
                target.fPositionX = landing.x; target.fPositionY = landing.y; target.fPositionZ = landing.z + 1.f;
                auto& chainAi = hazardRoom->m_ColosseumMercenaries.at(merc.iPlayerId);
                chainAi = {};
                const auto admittedChain = std::find_if(room->m_ColosseumMercenaries.begin(), room->m_ColosseumMercenaries.end(),
                    [&](const auto& value) { return room->m_Players.at(value.first).eCharacterClass == chain->eCharacterClass; });
                tests.Require(admittedChain != room->m_ColosseumMercenaries.end() && !admittedChain->second.AvailableSkills.empty(),
                    "Native continuation class has admitted available skill bindings");
                if (admittedChain == room->m_ColosseumMercenaries.end() || admittedChain->second.AvailableSkills.empty()) continue;
                SKILL_ID nextSkill = INVALID_SKILL_ID;
                for (const auto& [id, definition] : catalog->Get_Skills())
                    if (definition.eCharacterClass == actor.eCharacterClass)
                    {
                        controlled.CooldownEndTickBySkillId[id] = hazardRoom->m_iServerTick + 100000u;
                        if (definition.strInputSlot == "Q" &&
                            (definition.eRequiredStance == PLAYER_STANCE_ID::NONE || definition.eRequiredStance == actor.eStance)) nextSkill = id;
                    }
                controlled.CooldownEndTickBySkillId.erase(chain->iSkillId);
                chainAi.AvailableSkills = admittedChain->second.AvailableSkills;
                hazardRoom->Update_Colosseum(.25f);
                const auto lastAfterStart = chainAi.iLastSkillId;
                const auto recentAfterStart = chainAi.RecentSkills;
                const auto recentCursorAfterStart = chainAi.iRecentSkillCursor;
                const bool chainStarted = controlled.eAction == PLAYER_ACTION_STATE::SKILL && controlled.iCurrentSkillId == chain->iSkillId;
                const auto paidResource = controlled.iCurrentResource;
                const auto paidCooldown = controlled.CooldownEndTickBySkillId[chain->iSkillId];
                std::size_t maximumStage = controlled.iComboStage;
                unsigned buffered = 0u;
                bool ordered = chainStarted, inputWindowsOnly = true;
                std::vector<SERVER_WORLD_ENTITY> emptyWorld;
                std::vector<DAMAGE_EVENT> damage;
                for (unsigned tick = 0u; chainStarted && tick < 900u && controlled.eAction != PLAYER_ACTION_STATE::NONE; ++tick)
                {
                    const auto previousSequence = controlled.iLastSkillSequence;
                    const auto previousStage = controlled.iComboStage;
                    const float beforeMs = controlled.fActionElapsedSeconds * 1000.f;
                    hazardRoom->Update_Colosseum(1.f / 30.f);
                    if (controlled.iLastSkillSequence != previousSequence)
                    {
                        ++buffered;
                        const auto& stage = chain->ComboStages[previousStage - 1u];
                        inputWindowsOnly = inputWindowsOnly && stage.iInputCloseMs > 0u &&
                            beforeMs >= stage.iInputOpenMs && beforeMs <= stage.iInputCloseMs && controlled.hasBufferedComboInput;
                    }
                    ordered = ordered && chainAi.iLastSkillId == lastAfterStart && chainAi.RecentSkills == recentAfterStart &&
                        chainAi.iRecentSkillCursor == recentCursorAfterStart && controlled.iCurrentSkillId == chain->iSkillId &&
                        controlled.PendingCommand.eKind == PLAYER_PENDING_COMMAND_KIND::NONE;
                    hazardRoom->m_PlayerSkillSystem.Update(controlled, emptyWorld, hazardRoom->m_GameplayCatalog,
                        &hazardRoom->m_ServerNavigation, nullptr, 1.f / 30.f, ++hazardRoom->m_iServerTick, damage);
                    maximumStage = (std::max)(maximumStage, static_cast<std::size_t>(controlled.iComboStage));
                }
                const auto expectedBuffers = std::count_if(chain->ComboStages.begin(), chain->ComboStages.end() - 1,
                    [](const auto& stage) { return stage.iInputCloseMs > 0u; });
                tests.Require(chainStarted && ordered && inputWindowsOnly && controlled.eAction == PLAYER_ACTION_STATE::NONE &&
                    maximumStage == chain->ComboStages.size() && buffered == expectedBuffers &&
                    controlled.iCurrentResource == paidResource && controlled.CooldownEndTickBySkillId[chain->iSkillId] == paidCooldown,
                    "Every native stage completes; only manual windows buffer once, with no extra cost, cooldown or next-slot approval");
                std::cout << "Mercenary native chain skill=" << chain->iSkillId << " narrow=" << (chainIndex == 5u)
                    << " stages=" << maximumStage << '/' << chain->ComboStages.size() << " buffers=" << buffered << '/' << expectedBuffers << '\n';
                controlled.fPositionX = landing.x; controlled.fPositionY = landing.y; controlled.fPositionZ = landing.z;
                controlled.CooldownEndTickBySkillId[chain->iSkillId] = hazardRoom->m_iServerTick + 100000u;
                controlled.CooldownEndTickBySkillId.erase(nextSkill);
                chainAi.fThinkElapsed = 1.f;
                hazardRoom->Update_Colosseum(.25f);
                tests.Require(controlled.eAction == PLAYER_ACTION_STATE::SKILL && controlled.iCurrentSkillId == nextSkill,
                    "Only the completed native COMBO opens another available skill on the following decision");
                chain->ComboStages.front().iInputOpenMs = originalOpen;
                chain->ComboStages.front().iInputCloseMs = originalClose;
            }
            // Fresh actors isolate new-cast admission while preserving each random
            // stream and recent-skill memory; normal simulation advances these clocks.
            for (const auto classId : { CHARACTER_CLASS_ID::DIMENSIONMASTER, CHARACTER_CLASS_ID::LANCE_MASTER,
                CHARACTER_CLASS_ID::WARLORD, CHARACTER_CLASS_ID::GUARDIANKNIGHT, CHARACTER_CLASS_ID::ARTIST })
            {
                auto& actor = hazardRoom->m_Players.at(merc.iPlayerId);
                auto& selection = hazardRoom->m_ColosseumMercenaries.at(merc.iPlayerId);
                const auto* actorProfile = catalog->Find_Player(classId);
                auto fresh = merc;
                fresh.eCharacterClass = classId; fresh.eStance = actorProfile->eDefaultStance;
                fresh.iCurrentResource = fresh.iMaximumResource = actorProfile->iMaximumResource;
                fresh.iMaximumIdentity = actorProfile->iMaximumIdentity;
                fresh.fMoveSpeed = actorProfile->fMoveSpeed; fresh.CooldownEndTickBySkillId.clear();
                CPlayerSkillSystem::Reset_Gauges(fresh, hazardRoom->m_GameplayCatalog);
                auto& target = hazardRoom->m_Players.at(caster.iPlayerId);
                target = caster; target.eAction = PLAYER_ACTION_STATE::NONE; target.iCurrentSkillId = INVALID_SKILL_ID;
                target.fPositionZ = landing.z + 1.f;
                const auto trace = [&](const std::uint64_t seed)
                {
                    selection = {}; selection.iRandomState = seed;
                    std::vector<SKILL_ID> result;
                    bool valid = true;
                    for (unsigned sample = 0u; sample < 32u; ++sample)
                    {
                        hazardRoom->m_iServerTick = 10000u + sample * 120u;
                        actor = fresh; selection.iNextAttackTick = 0u; selection.fThinkElapsed = 1.f;
                        hazardRoom->Update_Colosseum(.25f);
                        const auto* running = catalog->Find_Skill(actor.iCurrentSkillId);
                        valid = valid && actor.eAction == PLAYER_ACTION_STATE::SKILL && running &&
                            running->eCharacterClass == classId && !Is_DodgeSkill(*running) &&
                            running->eSkillKind != PLAYER_SKILL_KIND::STANDUP && running->strInputSlot != "ALT_V" &&
                            (running->eRequiredStance == PLAYER_STANCE_ID::NONE || running->eRequiredStance == fresh.eStance) &&
                            selection.iLastSkillId == actor.iCurrentSkillId;
                        result.push_back(actor.iCurrentSkillId);
                    }
                    tests.Require(valid, "Weighted draws admit only actual ready class/stance attacks and commit successful-cast memory");
                    return result;
                };
                const auto first = trace(0x123456789abcdefu);
                const auto replay = trace(0x123456789abcdefu);
                const auto different = trace(0x987654321abcdefu);
                tests.Require(first == replay && first != different && std::set<SKILL_ID>(first.begin(), first.end()).size() >= 3u,
                    "Every class has reproducible seeded choices and multiple different ready skills instead of a fixed rotation");
                actor = fresh; actor.bPatternBound = true;
                selection.iNextAttackTick = 0u; selection.fThinkElapsed = 1.f;
                const auto previousLast = selection.iLastSkillId;
                const auto previousRecent = selection.RecentSkills;
                const auto previousCursor = selection.iRecentSkillCursor;
                hazardRoom->Update_Colosseum(.25f);
                tests.Require(actor.eAction == PLAYER_ACTION_STATE::NONE && selection.iLastSkillId == previousLast &&
                    selection.RecentSkills == previousRecent && selection.iRecentSkillCursor == previousCursor,
                    "Crowd control admits no new skill and preserves the successful-cast history");
                actor = fresh;
                for (const auto& [id, definition] : catalog->Get_Skills())
                    if (definition.eCharacterClass == classId) actor.CooldownEndTickBySkillId[id] = hazardRoom->m_iServerTick + 10000u;
                selection.iNextAttackTick = 0u; selection.fThinkElapsed = 1.f;
                hazardRoom->Update_Colosseum(.25f);
                tests.Require(actor.eAction == PLAYER_ACTION_STATE::NONE && selection.iLastSkillId == previousLast &&
                    selection.RecentSkills == previousRecent && selection.iRecentSkillCursor == previousCursor,
                    "Unavailable skills do not consume successful-cast memory or fabricate an attack");
                if (classId == CHARACTER_CLASS_ID::GUARDIANKNIGHT)
                {
                    actor = fresh; actor.iEmberOrbs = 0u; selection = {};
                    const PLAYER_SKILL_DEFINITION* expression = nullptr;
                    for (const auto& [id, definition] : catalog->Get_Skills())
                        if (definition.eCharacterClass == classId && definition.iEmberCost > 0u &&
                            (definition.eRequiredStance == PLAYER_STANCE_ID::NONE || definition.eRequiredStance == actor.eStance))
                        { expression = &definition; break; }
                    tests.Require(expression != nullptr, "Guardian fixture resolves a native human-form ember expression skill");
                    if (expression)
                    {
                        for (const auto& [id, definition] : catalog->Get_Skills())
                            if (definition.eCharacterClass == classId && id != expression->iSkillId)
                                actor.CooldownEndTickBySkillId[id] = hazardRoom->m_iServerTick + 10000u;
                        auto native = actor;
                        C2S_USE_SKILL request; request.iClientSequence = 1u; request.iSkillId = expression->iSkillId;
                        request.eTargetIntent = expression->eTargetIntent; request.fAimX = target.fPositionX; request.fAimZ = target.fPositionZ;
                        const bool nativeAccepted = hazardRoom->m_PlayerSkillSystem.Try_Start(native, request, *catalog,
                            hazardRoom->m_iServerTick + 1u, &hazardRoom->m_ServerNavigation);
                        hazardRoom->Update_Colosseum(.25f);
                        tests.Require(nativeAccepted && actor.eAction == PLAYER_ACTION_STATE::SKILL &&
                            actor.iCurrentSkillId == expression->iSkillId && actor.iEmberOrbs == 0u &&
                            selection.iLastSkillId == expression->iSkillId,
                            "Guardian AI preserves native zero-ember expression admission instead of adding a stricter resource gate");
                    }
                }
                actor = fresh; actor.eAction = PLAYER_ACTION_STATE::KNOCKDOWN;
                actor.iKnockdownEndTick = hazardRoom->m_iServerTick + 300u; selection = {};
                hazardRoom->Update_Colosseum(.25f);
                const auto* standup = catalog->Find_Skill(actor.iCurrentSkillId);
                const bool hasPublishedStandup = std::any_of(catalog->Get_Skills().begin(), catalog->Get_Skills().end(),
                    [&](const auto& entry) { return entry.second.eCharacterClass == classId &&
                        entry.second.eSkillKind == PLAYER_SKILL_KIND::STANDUP; });
                if (classId == CHARACTER_CLASS_ID::GUARDIANKNIGHT)
                    tests.Require(!hasPublishedStandup && !standup && actor.eAction == PLAYER_ACTION_STATE::KNOCKDOWN &&
                        actor.iKnockdownEndTick == hazardRoom->m_iServerTick + 300u &&
                        actor.PendingCommand.eKind == PLAYER_PENDING_COMMAND_KIND::NONE,
                        "Guardian has no published stand-up and waits for native knockdown recovery without fabricating a skill");
                else
                    tests.Require(hasPublishedStandup && standup && standup->eSkillKind == PLAYER_SKILL_KIND::STANDUP &&
                        actor.eAction == PLAYER_ACTION_STATE::SKILL && actor.iKnockdownEndTick == 0u &&
                        selection.eTactic == CGameRoom::COLOSSEUM_TACTIC::RECOVER,
                        "All four classes with a published stand-up, including DimensionMaster, use native knockdown recovery admission");
                actor = fresh; target = caster; selection = {};
                hazardRoom->Update_Colosseum(.25f);
                const auto* dodge = catalog->Find_Skill(actor.iCurrentSkillId);
                tests.Require(dodge && Is_DodgeSkill(*dodge) && actor.eAction == PLAYER_ACTION_STATE::SKILL &&
                    selection.eTactic == CGameRoom::COLOSSEUM_TACTIC::EVADE,
                    "Every class including DimensionMaster uses native SPACE against an imminent enemy hit");
                std::vector<SERVER_WORLD_ENTITY> dodgeWorld;
                std::vector<DAMAGE_EVENT> dodgeDamage;
                for (unsigned tick = 0u; tick < 300u && actor.eAction != PLAYER_ACTION_STATE::NONE; ++tick)
                    hazardRoom->m_PlayerSkillSystem.Update(actor, dodgeWorld, hazardRoom->m_GameplayCatalog,
                        &hazardRoom->m_ServerNavigation, nullptr, 1.f / 30.f, ++hazardRoom->m_iServerTick, dodgeDamage);
                tests.Require(dodge && actor.eAction == PLAYER_ACTION_STATE::NONE &&
                    selection.Threats.At(actor.fPositionX, actor.fPositionY, actor.fPositionZ).risk == 0.f &&
                    std::hypot(actor.fPositionX - landing.x, actor.fPositionZ - landing.z) > 1.5f,
                    "Native SPACE root motion completes at an actually safe position for every class, including abrupt and backward-tail curves");
            }
            // A soft repetition penalty lowers likelihood; it deliberately permits
            // a repeated cast when it remains the only usable option.
            {
                auto& actor = hazardRoom->m_Players.at(merc.iPlayerId);
                auto& selection = hazardRoom->m_ColosseumMercenaries.at(merc.iPlayerId);
                auto& target = hazardRoom->m_Players.at(caster.iPlayerId);
                target = caster; target.eAction = PLAYER_ACTION_STATE::NONE; target.iCurrentSkillId = INVALID_SKILL_ID;
                target.fPositionZ = landing.z + 1.f;
                const auto countQ = [&](const bool penalized)
                {
                    unsigned count = 0u;
                    for (unsigned sample = 1u; sample <= 128u; ++sample)
                    {
                        hazardRoom->m_iServerTick = 20000u;
                        actor = merc; selection = {}; selection.iRandomState = 0x123456789abcdefu * sample;
                        for (const auto& [id, definition] : catalog->Get_Skills())
                            if (definition.eCharacterClass == actor.eCharacterClass && id != 34040u && id != 34090u)
                                actor.CooldownEndTickBySkillId[id] = 30000u;
                        if (penalized) { selection.iLastSkillId = 34040u; selection.RecentSkills.fill(34040u); }
                        hazardRoom->Update_Colosseum(.25f);
                        if (actor.iCurrentSkillId == 34040u) ++count;
                    }
                    return count;
                };
                const auto unpenalized = countQ(false), penalized = countQ(true);
                tests.Require(unpenalized > 20u && penalized < unpenalized / 2u,
                    "Recent successful skills receive a measurable soft penalty in otherwise identical weighted draws");
                actor = merc; selection = {}; selection.iLastSkillId = 34040u; selection.RecentSkills.fill(34040u);
                for (const auto& [id, definition] : catalog->Get_Skills())
                    if (definition.eCharacterClass == actor.eCharacterClass && id != 34040u)
                        actor.CooldownEndTickBySkillId[id] = 30000u;
                hazardRoom->Update_Colosseum(.25f);
                tests.Require(actor.iCurrentSkillId == 34040u && actor.eAction == PLAYER_ACTION_STATE::SKILL,
                    "A recent skill remains usable when all alternatives are unavailable");
            }
            // Drive both real AI selectors against the published ALT_V binding.
            // Player resets below isolate command admission; the production
            // death/respawn call must preserve the match-owned interval.
            for (const auto classId : { CHARACTER_CLASS_ID::DIMENSIONMASTER, CHARACTER_CLASS_ID::LANCE_MASTER,
                CHARACTER_CLASS_ID::WARLORD, CHARACTER_CLASS_ID::GUARDIANKNIGHT, CHARACTER_CLASS_ID::ARTIST })
            {
                const auto* actorProfile = catalog->Find_Player(classId);
                const PLAYER_SKILL_DEFINITION* awakening = nullptr;
                for (const auto& [id, definition] : catalog->Get_Skills())
                    if (definition.eCharacterClass == classId && definition.strInputSlot == "ALT_V") awakening = &definition;
                tests.Require(actorProfile && awakening && awakening->eSkillKind == PLAYER_SKILL_KIND::ACTIVE,
                    "Every Colosseum mercenary resolves its actual published ALT_V active skill");
                if (!actorProfile || !awakening) continue;
                auto& actor = hazardRoom->m_Players.at(merc.iPlayerId);
                auto& interval = hazardRoom->m_ColosseumMercenaries.at(merc.iPlayerId);
                auto& target = hazardRoom->m_Players.at(caster.iPlayerId);
                target.eAction = PLAYER_ACTION_STATE::NONE; target.iCurrentSkillId = INVALID_SKILL_ID;
                target.fPositionX = landing.x; target.fPositionY = landing.y; target.fPositionZ = landing.z + 1.f;
                const auto blockOtherSkills = [&](SERVER_PLAYER& player)
                {
                    for (const auto& [id, definition] : catalog->Get_Skills())
                        if (definition.eCharacterClass == classId && id != awakening->iSkillId)
                            player.CooldownEndTickBySkillId[id] = hazardRoom->m_iServerTick + 100000u;
                };
                const auto freshActor = [&]()
                {
                    auto player = merc;
                    player.eCharacterClass = classId; player.eStance = actorProfile->eDefaultStance;
                    player.iCurrentResource = player.iMaximumResource = actorProfile->iMaximumResource;
                    player.iMaximumIdentity = actorProfile->iMaximumIdentity; player.fMoveSpeed = actorProfile->fMoveSpeed;
                    player.fColosseumSpawnX = landing.x; player.fColosseumSpawnY = landing.y;
                    player.fColosseumSpawnZ = landing.z; player.fColosseumSpawnYaw = player.fYawDegrees;
                    player.CooldownEndTickBySkillId.clear();
                    CPlayerSkillSystem::Reset_Gauges(player, hazardRoom->m_GameplayCatalog);
                    blockOtherSkills(player);
                    return player;
                };
                const auto decide = [&]()
                {
                    interval.fThinkElapsed = 1.f; interval.iNextAttackTick = 0u;
                    hazardRoom->Update_Colosseum(.25f);
                };
                constexpr std::uint32_t firstTick = 200000u;
                hazardRoom->m_iServerTick = firstTick;
                hazardRoom->m_iColosseumPhaseEnd = firstTick + 3600u;
                hazardRoom->m_eColosseumPhase = COLOSSEUM_MATCH_PHASE::ACTIVE;
                actor = freshActor(); interval = {}; interval.AvailableSkills = { awakening->iSkillId };
                actor.CooldownEndTickBySkillId[awakening->iSkillId] = firstTick + 1200u;
                decide();
                tests.Require(actor.eAction == PLAYER_ACTION_STATE::NONE && !interval.iLastAltVAdmissionTick,
                    "A native cooldown rejection does not consume the mercenary ALT_V interval");
                actor.CooldownEndTickBySkillId.erase(awakening->iSkillId);
                decide();
                tests.Require(actor.eAction == PLAYER_ACTION_STATE::SKILL && actor.iCurrentSkillId == awakening->iSkillId &&
                    interval.iLastAltVAdmissionTick == firstTick,
                    "An accepted ALT_V starts that mercenary's thirty-second admission interval");

                actor.iCurrentHp = 0u; actor.eAction = PLAYER_ACTION_STATE::DEAD;
                actor.iColosseumRespawnTick = firstTick + 90u;
                hazardRoom->m_iServerTick = firstTick + 90u;
                hazardRoom->Update_ColosseumMatch(hazardRoom->m_iServerTick);
                tests.Require(actor.iCurrentHp == expectedMatchHp && actor.iMaximumHp == expectedMatchHp &&
                    actor.CooldownEndTickBySkillId.empty() && interval.iLastAltVAdmissionTick == firstTick,
                    "Actual arena respawn restores twenty-bar HP and clears native cooldowns while preserving the ALT_V interval");
                blockOtherSkills(actor);
                decide();
                tests.Require(actor.eAction == PLAYER_ACTION_STATE::NONE && interval.iLastAltVAdmissionTick == firstTick,
                    "Respawn cannot authorize another mercenary ALT_V before thirty seconds");
                hazardRoom->m_iServerTick = firstTick + 899u; actor = freshActor();
                decide();
                tests.Require(actor.eAction == PLAYER_ACTION_STATE::NONE && interval.iLastAltVAdmissionTick == firstTick,
                    "The same mercenary ALT_V stays blocked at the 899-tick boundary");
                hazardRoom->m_iServerTick = firstTick + 900u; actor = freshActor();
                decide();
                tests.Require(actor.eAction == PLAYER_ACTION_STATE::SKILL && actor.iCurrentSkillId == awakening->iSkillId &&
                    interval.iLastAltVAdmissionTick == firstTick + 900u,
                    "The same mercenary ALT_V is admitted at 900 ticks when its native cooldown permits");

                hazardRoom->m_iServerTick = firstTick + 1800u; actor = freshActor();
                actor.CooldownEndTickBySkillId[awakening->iSkillId] = firstTick + 2700u;
                decide();
                tests.Require(actor.eAction == PLAYER_ACTION_STATE::NONE &&
                    actor.CooldownEndTickBySkillId.at(awakening->iSkillId) == firstTick + 2700u &&
                    interval.iLastAltVAdmissionTick == firstTick + 900u,
                    "An existing longer native cooldown remains unchanged after the thirty-second minimum expires");
                auto peer = freshActor(); peer.iPlayerId = 3u; peer.iNetEntityId = 103u;
                hazardRoom->m_Players.emplace(peer.iPlayerId, peer);
                auto& peerInterval = hazardRoom->m_ColosseumMercenaries[peer.iPlayerId];
                peerInterval.AvailableSkills = { awakening->iSkillId };
                decide();
                tests.Require(hazardRoom->m_Players.at(peer.iPlayerId).iCurrentSkillId == awakening->iSkillId &&
                    peerInterval.iLastAltVAdmissionTick == firstTick + 1800u && interval.iLastAltVAdmissionTick == firstTick + 900u,
                    "A different mercenary owns an independent ALT_V interval");
                hazardRoom->m_ColosseumMercenaries.erase(peer.iPlayerId); hazardRoom->m_Players.erase(peer.iPlayerId);
                for (const auto phase : { COLOSSEUM_MATCH_PHASE::RECRUITING, COLOSSEUM_MATCH_PHASE::FINISHED })
                {
                    hazardRoom->m_eColosseumPhase = phase;
                    decide();
                    tests.Require(interval.iLastAltVAdmissionTick == firstTick + 900u,
                        "Recruitment and match completion do not erase a mercenary's accepted ALT_V clock");
                }
                hazardRoom->m_eColosseumPhase = COLOSSEUM_MATCH_PHASE::ACTIVE;

                const auto beforeWrap = (std::numeric_limits<std::uint32_t>::max)() - 400u;
                hazardRoom->m_iServerTick = beforeWrap; actor = freshActor();
                interval = {}; interval.AvailableSkills = { awakening->iSkillId };
                decide();
                tests.Require(interval.iLastAltVAdmissionTick == beforeWrap && actor.iCurrentSkillId == awakening->iSkillId,
                    "The mercenary ALT_V interval records an accepted cast near server tick wrap");
                hazardRoom->m_iServerTick = beforeWrap + 899u; actor = freshActor();
                decide();
                tests.Require(actor.eAction == PLAYER_ACTION_STATE::NONE && interval.iLastAltVAdmissionTick == beforeWrap,
                    "Unsigned elapsed ticks keep ALT_V blocked across wrap before thirty seconds");
                hazardRoom->m_iServerTick = beforeWrap + 900u; actor = freshActor();
                decide();
                tests.Require(actor.iCurrentSkillId == awakening->iSkillId &&
                    interval.iLastAltVAdmissionTick == beforeWrap + 900u,
                    "ALT_V becomes eligible exactly at the thirty-second interval across tick wrap");
            }
            // The normal skill executor owns stance changes and stand-up too.
            // Neither a second stance runtime nor a synthetic direct swap is used.
            {
                auto& controlled = hazardRoom->m_Players.at(merc.iPlayerId);
                auto& selection = hazardRoom->m_ColosseumMercenaries.at(merc.iPlayerId);
                controlled = merc; selection = {};
                CPlayerSkillSystem::Reset_Gauges(controlled, hazardRoom->m_GameplayCatalog);
                for (const auto& [id, definition] : catalog->Get_Skills())
                    if (definition.eCharacterClass == CHARACTER_CLASS_ID::LANCE_MASTER)
                        controlled.CooldownEndTickBySkillId[id] = hazardRoom->m_iServerTick + 100000u;
                controlled.CooldownEndTickBySkillId.erase(34000u);
                hazardRoom->Update_Colosseum(.25f);
                tests.Require(controlled.iCurrentSkillId == 34000u, "Available-skill selection admits the real published LanceMaster Z stance skill");
                std::vector<SERVER_WORLD_ENTITY> emptyWorld;
                std::vector<DAMAGE_EVENT> damage;
                for (unsigned tick = 0u; tick < 150u && controlled.eAction != PLAYER_ACTION_STATE::NONE; ++tick)
                    hazardRoom->m_PlayerSkillSystem.Update(controlled, emptyWorld, hazardRoom->m_GameplayCatalog,
                        &hazardRoom->m_ServerNavigation, nullptr, 1.f / 30.f, ++hazardRoom->m_iServerTick, damage);
                tests.Require(controlled.eStance == PLAYER_STANCE_ID::LANCE_MASTER_SHORT_SPEAR,
                    "The native action commits the stance before the next AI decision");
                controlled.CooldownEndTickBySkillId.erase(34510u);
                selection.iNextAttackTick = 0u; selection.fThinkElapsed = 1.f;
                hazardRoom->Update_Colosseum(.25f);
                tests.Require(controlled.iCurrentSkillId == 34510u,
                    "The native stance exposes its ready LMB through the refreshed class bindings");
                for (unsigned tick = 0u; tick < 900u && controlled.eAction != PLAYER_ACTION_STATE::NONE; ++tick)
                {
                    hazardRoom->Update_Colosseum(1.f / 30.f);
                    hazardRoom->m_PlayerSkillSystem.Update(controlled, emptyWorld, hazardRoom->m_GameplayCatalog,
                        &hazardRoom->m_ServerNavigation, nullptr, 1.f / 30.f, ++hazardRoom->m_iServerTick, damage);
                }
                controlled.fPositionX = landing.x; controlled.fPositionY = landing.y; controlled.fPositionZ = landing.z;
                controlled.CooldownEndTickBySkillId[34510u] = hazardRoom->m_iServerTick + 100000u;
                controlled.CooldownEndTickBySkillId.erase(34540u);
                selection.iNextAttackTick = 0u; selection.fThinkElapsed = 1.f;
                hazardRoom->Update_Colosseum(.25f);
                tests.Require(controlled.iCurrentSkillId == 34540u &&
                    std::find(selection.AvailableSkills.begin(), selection.AvailableSkills.end(), 34540u) != selection.AvailableSkills.end() &&
                    std::find(selection.AvailableSkills.begin(), selection.AvailableSkills.end(), 34040u) == selection.AvailableSkills.end(),
                    "Selection refreshes actual active-catalog bindings after a native stance change");
                controlled = merc; controlled.eAction = PLAYER_ACTION_STATE::KNOCKDOWN;
                controlled.iKnockdownEndTick = hazardRoom->m_iServerTick + 300u; selection = {};
                hazardRoom->Update_Colosseum(.25f);
                const auto* standup = catalog->Find_Skill(controlled.iCurrentSkillId);
                tests.Require(standup && standup->eSkillKind == PLAYER_SKILL_KIND::STANDUP &&
                    controlled.eAction == PLAYER_ACTION_STATE::SKILL && controlled.iKnockdownEndTick == 0u,
                    "A non-DimensionMaster mercenary uses the existing stand-up executor only while knocked down");
            }
        }
    }
    // Exercise the observation against state committed by the real skill executor,
    // then drive the room selector with that same lingering ground hazard.
    {
        auto catalog = std::make_shared<CGameplayCatalog>(source->m_GameplayCatalog.Active());
        auto tactical = std::make_unique<CGameRoom>(WORLD_ID::COLOSSEUM, catalog);
        SERVER_NAV_POINT landing;
        const bool ready = tactical->Is_Ready() && tactical->m_ServerNavigation.Sample_Position(-20.4f, -2.6f, landing, 12.22f);
        tests.Require(ready, "Tactical fixtures load the actual Colosseum navigation and native skill catalog");
        if (ready)
        {
            tactical->m_iColosseumMatchId = 901u; tactical->m_eColosseumPhase = COLOSSEUM_MATCH_PHASE::ACTIVE;
            tactical->m_iServerTick = 100u;
            const auto* profile = catalog->Find_Player(CHARACTER_CLASS_ID::LANCE_MASTER);
            SERVER_PLAYER fresh;
            fresh.iPlayerId = 1u; fresh.iNetEntityId = 101u;
            fresh.eControlKind = PLAYER_CONTROL_KIND::COLOSSEUM_MERCENARY_AI;
            fresh.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER; fresh.eStance = profile->eDefaultStance;
            fresh.iCurrentHp = fresh.iMaximumHp = expectedMatchHp; fresh.iColosseumDamageReferenceHp = expectedReferenceHp;
            fresh.iCurrentResource = fresh.iMaximumResource = profile->iMaximumResource;
            fresh.fMoveSpeed = profile->fMoveSpeed; fresh.iColosseumMatchId = 901u;
            fresh.iColosseumTeam = 0u; fresh.bColosseumParticipant = true;
            fresh.isCombatReady = fresh.bColosseumCombatActive = true;
            fresh.fPositionX = landing.x; fresh.fPositionY = landing.y; fresh.fPositionZ = landing.z;
            auto enemy = fresh;
            enemy.iPlayerId = 2u; enemy.iNetEntityId = 102u; enemy.iColosseumTeam = 1u;
            enemy.eControlKind = PLAYER_CONTROL_KIND::HUMAN; enemy.fPositionZ += 4.f;
            tactical->m_Players.emplace(1u, fresh); tactical->m_Players.emplace(2u, enemy);
            auto& actor = tactical->m_Players.at(1u);
            auto& caster = tactical->m_Players.at(2u);
            auto& ai = tactical->m_ColosseumMercenaries[1u];
            auto* skill = const_cast<PLAYER_SKILL_DEFINITION*>(catalog->Find_Skill(34040u));
            skill->eSkillKind = PLAYER_SKILL_KIND::ACTIVE; skill->ComboStages.clear(); skill->RootMotion.clear();
            skill->Hits.clear(); skill->Projectiles.clear(); skill->iActionDurationMs = 100u;
            skill->iCooldownMs = 0u; skill->iResourceCost = 0u;
            skill->eTargetIntent = SKILL_TARGET_INTENT_KIND::GROUND_POINT;
            skill->fTargetMaximumRange = skill->fMaximumRange = 12.f; skill->requiresWalkableTarget = true;
            PLAYER_SKILL_HIT hit; hit.iResultKind = 1u; hit.iTimeMs = 80u; hit.iAreaType = 1u; hit.fRange = .8f;
            skill->Hits.push_back(hit);
            C2S_USE_SKILL command; command.iClientSequence = 1u; command.iSkillId = skill->iSkillId;
            command.eTargetIntent = SKILL_TARGET_INTENT_KIND::GROUND_POINT; command.fAimX = landing.x; command.fAimZ = landing.z;
            const bool groundCast = tactical->m_PlayerSkillSystem.Try_Start(caster, command, *catalog, tactical->m_iServerTick,
                &tactical->m_ServerNavigation);
            CColosseumThreatAssessment observed;
            const std::vector<SERVER_COMBAT_OBJECT> noObjects;
            observed.Observe(actor, tactical->m_Players, noObjects, *catalog, tactical->m_iServerTick);
            tests.Require(groundCast && caster.hasSkillTarget && observed.At(landing.x, landing.y, landing.z).risk > 0.f &&
                observed.At(caster.fPositionX, caster.fPositionY, caster.fPositionZ).risk == 0.f &&
                observed.At(landing.x, landing.y + 5.f, landing.z).risk == 0.f,
                "Native ground-target admission places predicted damage at the committed aim point with the actual height guard");
            caster = enemy; skill->Hits.clear();
            PLAYER_SKILL_PROJECTILE area; area.eKind = PLAYER_PROJECTILE_KIND::FIXAREA;
            area.eOrigin = PLAYER_PROJECTILE_ORIGIN::AIM; area.fMaxDistance = 12.f; area.iLifeMs = 1000u;
            PLAYER_PROJECTILE_HIT contact; contact.isContact = true; contact.Hit = hit; contact.Hit.iTimeMs = 0u;
            area.Hits.push_back(contact); skill->Projectiles.push_back(area);
            const bool areaCast = tactical->m_PlayerSkillSystem.Try_Start(caster, command, *catalog, tactical->m_iServerTick,
                &tactical->m_ServerNavigation);
            std::vector<SERVER_WORLD_ENTITY> emptyWorld; std::vector<DAMAGE_EVENT> damage;
            for (unsigned tick = 0u; tick < 5u; ++tick)
                tactical->m_PlayerSkillSystem.Update(caster, emptyWorld, tactical->m_GameplayCatalog,
                    &tactical->m_ServerNavigation, nullptr, 1.f / 30.f, ++tactical->m_iServerTick, damage);
            observed.Observe(actor, tactical->m_Players, noObjects, *catalog, tactical->m_iServerTick);
            tests.Require(areaCast && caster.eAction == PLAYER_ACTION_STATE::NONE && !caster.Projectiles.empty() &&
                observed.At(landing.x, landing.y, landing.z).risk > 0.f,
                "A projectile spawned by the native executor remains a threat after its caster action finishes");
            tests.Require(observed.At(landing.x - 3.f, landing.y, landing.z).risk == 0.f &&
                observed.At(landing.x + 3.f, landing.y, landing.z).risk == 0.f &&
                observed.Along(landing.x - 3.f, landing.y, landing.z, landing.x + 3.f, landing.y, landing.z, .6f) > 0.f &&
                observed.Along(landing.x - 3.f, landing.y, landing.z + 3.f, landing.x + 3.f, landing.y, landing.z + 3.f, .6f) == 0.f,
                "Route assessment detects crossing a lingering hit even when both endpoints are safe and accepts a clear detour");
            actor.CooldownEndTickBySkillId[34020u] = 10000u;
            tactical->Update_Colosseum(.25f);
            tests.Require(actor.hasMoveGoal && actor.eAction == PLAYER_ACTION_STATE::NONE &&
                ai.eTactic == CGameRoom::COLOSSEUM_TACTIC::EVADE &&
                observed.At(actor.fMoveGoalX, actor.fPositionY, actor.fMoveGoalZ, .4f).risk == 0.f,
                "The actual AI walks to a safe navigation goal for an active lingering area when SPACE is cooling down");
            if (!caster.Projectiles.empty())
            {
                caster.Projectiles.front().ContactMarks.push_back({actor.iNetEntityId, 0u, 1u, 0.f});
                observed.Observe(actor, tactical->m_Players, noObjects, *catalog, tactical->m_iServerTick);
                tests.Require(observed.At(landing.x, landing.y, landing.z).risk == 0.f,
                    "Consumed per-target contact hits are not predicted as fresh damage");
                caster.Projectiles.front().ContactMarks.clear();
            }
            caster.iCurrentHp = 0u;
            observed.Observe(actor, tactical->m_Players, noObjects, *catalog, tactical->m_iServerTick);
            tests.Require(observed.ThreatCount() == 0u, "A dead projectile owner is excluded by the same PvP source guard as damage");
            caster = enemy; caster.fPositionZ = landing.z + 1.f; actor = fresh; ai = {};
            auto challenger = caster; challenger.iPlayerId = 3u; challenger.iNetEntityId = 103u; challenger.fPositionZ += .1f;
            tactical->m_Players.emplace(3u, challenger);
            const auto decide = [&]()
            {
                ai.fThinkElapsed = 1.f; ai.iNextAttackTick = 0u;
                for (const auto& [id, definition] : catalog->Get_Skills())
                    if (definition.eCharacterClass == actor.eCharacterClass)
                        actor.CooldownEndTickBySkillId[id] = tactical->m_iServerTick + 10000u;
                tactical->Update_Colosseum(.25f);
            };
            decide();
            tests.Require(ai.iTargetEntityId == caster.iNetEntityId, "Target scoring selects the closest comparable living opponent");
            tactical->m_Players.at(3u).fPositionZ = landing.z + .9f; tactical->m_iServerTick += 6u;
            decide();
            tests.Require(ai.iTargetEntityId == caster.iNetEntityId, "Target hysteresis retains an opponent through a small short-lived score change");
            caster.iCurrentHp = 0u; tactical->m_iServerTick += 6u; decide();
            tests.Require(ai.iTargetEntityId == challenger.iNetEntityId, "A dead target is replaced immediately by another eligible opponent");
            tactical->m_Players.at(3u).iColosseumMatchId = 999u; decide();
            tests.Require(ai.iTargetEntityId == INVALID_NET_ENTITY_ID && ai.eTactic == CGameRoom::COLOSSEUM_TACTIC::WAIT,
                "Other matches are excluded from target selection even when their players are nearby");
            tactical->m_Players.erase(3u); caster = enemy; caster.fPositionZ = landing.z + 2.f;
            actor = fresh; actor.iCurrentHp = actor.iMaximumHp / 5u; ai = {}; ai.iObservedHp = fresh.iMaximumHp;
            tactical->m_iServerTick = 1000u; decide();
            const auto retreatUntil = ai.iTacticUntilTick;
            tests.Require(ai.eTactic == CGameRoom::COLOSSEUM_TACTIC::RETREAT && actor.hasMoveGoal && retreatUntil > 1000u,
                "Low health under nearby pressure enters a bounded retreat through the real movement executor");
            caster.fPositionZ = landing.z + 10.f; tactical->m_iServerTick = 1006u; decide();
            tests.Require(ai.eTactic == CGameRoom::COLOSSEUM_TACTIC::RETREAT && ai.iTacticUntilTick == retreatUntil,
                "Retreat hysteresis holds its existing deadline when immediate pressure disappears");
            tactical->m_iServerTick = retreatUntil + 1u; decide();
            tests.Require(ai.eTactic != CGameRoom::COLOSSEUM_TACTIC::RETREAT && actor.iCurrentHp == actor.iMaximumHp / 5u &&
                ai.iRetreatAllowedTick > tactical->m_iServerTick,
                "Retreat re-engages after its deadline without waiting for unavailable passive healing and cannot immediately retrigger");
            // Full room ticks exercise simultaneous decisions, movement, native
            // skill damage, hit reactions and score/respawn guards together.
            auto battle = std::make_unique<CGameRoom>(WORLD_ID::COLOSSEUM,
                std::make_shared<CGameplayCatalog>(source->m_GameplayCatalog.Active()));
            battle->m_iColosseumMatchId = 902u; battle->m_eColosseumPhase = COLOSSEUM_MATCH_PHASE::ACTIVE;
            battle->m_iServerTick = 100u; battle->m_iColosseumPhaseEnd = 10000u;
            const std::array<CHARACTER_CLASS_ID, 6> classes{ CHARACTER_CLASS_ID::DIMENSIONMASTER,
                CHARACTER_CLASS_ID::LANCE_MASTER, CHARACTER_CLASS_ID::WARLORD,
                CHARACTER_CLASS_ID::GUARDIANKNIGHT, CHARACTER_CLASS_ID::ARTIST, CHARACTER_CLASS_ID::LANCE_MASTER };
            std::map<PLAYER_ID, std::array<float, 2>> initialPositions;
            std::map<PLAYER_ID, std::set<SKILL_ID>> casts;
            for (std::size_t i = 0u; i < classes.size(); ++i)
            {
                auto player = fresh;
                player.iPlayerId = static_cast<PLAYER_ID>(i + 1u); player.iNetEntityId = static_cast<NET_ENTITY_ID>(201u + i);
                player.iColosseumMatchId = 902u; player.iColosseumTeam = static_cast<std::uint8_t>(i % 2u);
                const auto* memberProfile = battle->m_GameplayCatalog.Find_Player(classes[i]);
                player.eCharacterClass = classes[i]; player.eStance = memberProfile->eDefaultStance;
                player.iCurrentResource = player.iMaximumResource = memberProfile->iMaximumResource;
                player.iMaximumIdentity = memberProfile->iMaximumIdentity; player.fMoveSpeed = memberProfile->fMoveSpeed;
                CPlayerSkillSystem::Reset_Gauges(player, battle->m_GameplayCatalog);
                SERVER_NAV_POINT position;
                const float angle = static_cast<float>(i) * 1.04719755f;
                if (battle->m_ServerNavigation.Sample_Position(landing.x + std::cos(angle) * 1.5f,
                    landing.z + std::sin(angle) * 1.5f, position, landing.y))
                { player.fPositionX = position.x; player.fPositionY = position.y; player.fPositionZ = position.z; }
                player.fColosseumSpawnX = player.fPositionX; player.fColosseumSpawnY = player.fPositionY;
                player.fColosseumSpawnZ = player.fPositionZ;
                initialPositions[player.iPlayerId] = { player.fPositionX, player.fPositionZ };
                battle->m_PlayerIdByEntityId[player.iNetEntityId] = player.iPlayerId;
                battle->m_Players.emplace(player.iPlayerId, player);
                battle->m_ColosseumMercenaries[player.iPlayerId].iRandomState = 0x123456789abcdefu * (i + 1u);
            }
            std::size_t actualDamageEvents = 0u;
            bool moved = false, lostHp = false;
            for (unsigned tick = 0u; tick < 360u; ++tick)
            {
                battle->Tick(1.f / 30.f);
                actualDamageEvents += battle->m_TickDamageEvents.size();
                for (const auto& [id, player] : battle->m_Players)
                {
                    const auto& initial = initialPositions.at(id);
                    moved = moved || std::hypot(player.fPositionX - initial[0], player.fPositionZ - initial[1]) > .5f;
                    lostHp = lostHp || player.iCurrentHp < player.iMaximumHp;
                    if (player.eAction == PLAYER_ACTION_STATE::SKILL) casts[id].insert(player.iCurrentSkillId);
                }
            }
            const auto variedActors = std::count_if(casts.begin(), casts.end(), [](const auto& entry) { return entry.second.size() >= 2u; });
            tests.Require(battle->m_iServerTick == 460u && actualDamageEvents > 0u && moved && lostHp && variedActors >= 2,
                "Twelve seconds of full room ticks produce real PvP damage, HP loss, movement and varied skill use by multiple mercenaries");
            std::cout << "Mercenary battle ticks=" << (battle->m_iServerTick - 100u) << " damage=" << actualDamageEvents
                << " moved=" << moved << " lostHp=" << lostHp << " variedActors=" << variedActors << '\n';
        }
    }
    auto preview = std::make_unique<CGameRoom>(WORLD_ID::COLOSSEUM);
    preview->Update_Colosseum(.5f);
    tests.Require(preview->m_iColosseumMatchId == 0u && preview->m_ColosseumMercenaries.empty(),
        "Direct Colosseum preview gains no PvP authority or candidates");
    // Numeric balance uses its real parser and room stage/commit. A running
    // match pins health at admission; ordinary raid players still migrate by ratio.
    std::map<PLAYER_ID, std::array<std::uint32_t, 3>> pinnedHealth;
    for (auto& [id, player] : room->m_Players)
    {
        if (player.iCurrentHp) player.iCurrentHp = player.iMaximumHp / 2u;
        pinnedHealth.emplace(id, std::array<std::uint32_t, 3>{ player.iCurrentHp, player.iMaximumHp, player.iColosseumDamageReferenceHp });
    }
    std::uint32_t numericSequence = 1800u;
    for (const auto phase : { COLOSSEUM_MATCH_PHASE::RECRUITING, COLOSSEUM_MATCH_PHASE::ACTIVE, COLOSSEUM_MATCH_PHASE::FINISHED })
    {
        room->m_eColosseumPhase = phase;
        const auto base = room->Get_ActiveGameplayGeneration();
        const auto* classProfile = base->Find_Player(CHARACTER_CLASS_ID::LANCE_MASTER);
        const auto* bossProfile = base->Find_Boss("BOSS_VALTAN");
        const std::vector<BALANCE_NUMERIC_CHANGE> changes{
            { BALANCE_DOMAIN::PLAYER, "LANCE_MASTER", "maximumHp", double(classProfile->iMaximumHp), double(classProfile->iMaximumHp + 37u) },
            { BALANCE_DOMAIN::BOSS, "BOSS_VALTAN", "maximumHp", double(bossProfile->iMaximumHp), double(bossProfile->iMaximumHp + 1000u) }
        };
        auto candidate = std::make_shared<CGameplayCatalog>();
        std::string bytes, status;
        const bool parsed = candidate->Load_NumericBalancePatch(*base, changes, bytes);
        const bool committed = parsed && room->Stage_NumericBalance(++numericSequence, candidate, status) &&
            room->Commit_NumericBalance(numericSequence);
        tests.Require(committed && std::all_of(room->m_Players.begin(), room->m_Players.end(), [&](const auto& value) {
            const auto health = pinnedHealth.at(value.first);
            return value.second.iCurrentHp == health[0] && value.second.iMaximumHp == health[1] &&
                value.second.iColosseumDamageReferenceHp == health[2];
        }), "Numeric reload pins arena HP and full damage reference for humans, selected mercenaries and idle candidates in every phase");
        if (phase == COLOSSEUM_MATCH_PHASE::RECRUITING && parsed)
        {
            auto raid = std::make_unique<CGameRoom>(WORLD_ID::VALTAN_ARENA, base);
            SERVER_PLAYER player;
            player.iPlayerId = 9001u; player.iNetEntityId = 9002u;
            player.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
            player.iMaximumHp = classProfile->iMaximumHp; player.iCurrentHp = classProfile->iMaximumHp / 2u;
            const auto oldHp = player.iCurrentHp;
            raid->m_Players.emplace(player.iPlayerId, player);
            const auto newMaximum = candidate->Find_Player(CHARACTER_CLASS_ID::LANCE_MASTER)->iMaximumHp;
            const auto newCurrent = static_cast<std::uint32_t>((static_cast<std::uint64_t>(oldHp) * newMaximum + classProfile->iMaximumHp / 2u) / classProfile->iMaximumHp);
            const bool raidCommitted = raid->Is_Ready() && raid->Stage_NumericBalance(1900u, candidate, status) && raid->Commit_NumericBalance(1900u);
            tests.Require(raidCommitted && raid->m_Players.at(player.iPlayerId).iMaximumHp == newMaximum &&
                raid->m_Players.at(player.iPlayerId).iCurrentHp == newCurrent,
                "The real Valtan room keeps normal class-HP ratio migration after the same numeric reload");
        }
    }
    for (const auto& session : sessions) session->m_isSendRunning.store(false);
    std::cout << "Colosseum match contract failures=" << tests.failures << '\n';
    return tests.failures ? 1 : 0;
}

int CServerGameplayContractRunner::Run_ColosseumMatch()
{
    int result = 1;
    const auto execute = [](void* output)
    {
        std::cout << std::unitbuf;
        *static_cast<int*>(output) = Run_ColosseumMatchContracts();
    };
    return ServerGameplayContractDetail::Run_WithContractWorkerStack(execute, &result) ? result : 1;
}
```

작성일: 2026-10-01. 기준은 현재 소스와 `COLOSSEUM.worldbootstrap`이며 기존 콜로세움 월드맵 PLAN/RESULT를 이어간다.

## 목표와 현재 확인

Bern 큐의 인간 네 명을 무작위 2 대 2로 나누고, 각 팀이 다섯 직업 후보 중 두 용병을 선택하여 4 대 4 경기를 한다. Debug와 Release의 입장 인원은 모두 네 명이다. 팀별 인간 둘은 자동 파티가 되며 선택된 용병 둘을 같은 roster에 넣는다. 양 팀 모집 완료 후 ACTIVE, 한 팀 참가자 전멸 후 FINISHED다. 기존 레이드와 직접 콜로세움 저작 입장은 영향을 받지 않는다.

작업 착수 당시 `GameRoom_Inventory.cpp`는 Debug 한 명/Release 네 명이며 큐를 먼저 지운 뒤 MATCH_FOUND와 개별 이동을 송신한다. `ServerApp.cpp`는 콜로세움을 shared room으로 받으며 `SERVER_PLAYER`에는 팀/경기 식별자가 없다. 따라서 당시 loading 화면의 팀 표시는 전투 권한이나 경기 격리를 보장하지 않는다. 기존 `Transfer_PartyTo`의 Stage_PlayerEntry/RELIABLE_BATCH_TRANSACTION을 같은 기준으로 재사용한다.

## 변경 파일과 데이터 흐름

`ServerPlayer.h`에 match ID/team/participant와 immutable `iColosseumDamageReferenceHp`를 추가한다. `RoomCommand.h`, `GameRoom.cpp`, `ServerApp.cpp`는 typed recruit를 room thread로 전달한다. `GameRoom_Inventory.cpp`는 큐 네 명을 하나의 Colosseum batch로 예약하고 성공할 때만 큐를 제거한다. `ServerTriggerSystem.h`는 이 서버 내부 batch 표식을 전달한다.

새 `GameRoom_Colosseum.cpp`가 전원 입장 staging, 팀 파티, 열 명 후보, 모집, 경기 상태와 AI를 소유한다. `ServerApp.h/.cpp`는 match별 private room 수명과 tick/revision/numeric update/shutdown을 관리한다. `GameRoom_PlayerSimulation.cpp`는 해당 경기의 player pointer span과 ACTIVE 권한을 PlayerSkillSystem에 전달한다. 새 cpp는 `Server.vcxproj` 및 `.filters`에 기존 필터를 보존하여 등록한다.

Shared wire와 Client는 별도 담당자가 `C2S_COLOSSEUM_RECRUIT`(120), `S2C_COLOSSEUM_MATCH_STATE`(121), `COLOSSEUM_MERCENARY_AI`를 연결한다. Shared 담당자가 root와 조율하여 protocol 130을 한 번 변경한다. 스킬 피해·CC는 별도 담당 `ColosseumCombatPolicy.h`/PlayerSkillSystem이 동일한 match/team guard를 소비한다.

## G00 — 네 명 입장 transaction과 경기 격리

큐에는 살아 있고 이동 가능한 인간만 유지한다. 한 immutable 네 명 batch를 예약하며 다른 transfer와 중복하지 않는다. 새 private room에 source 플레이어의 인벤토리·purse·칭호·내구도를 보존하여 전부 stage한다. authored teama/teamb playerSpawn과 navigation/collision을 검증한다. 기존 MATCH_FOUND, ENTER_ACCEPTED, 전체 spawn, 팀 roster, 지속 match state를 reliable queue에 일괄 예약한다. 한 명이라도 실패하면 원래 방/파티/큐/바인딩을 유지하며 재시도할 수 있다. 송신 예약 성공 이후 source departure와 target container swap 및 app binding을 commit한다.

새 경기의 CGameRoom 생성자는 WorldBootstrap, item/vehicle/honor/rewards, navigation, collision과 entity를 읽으므로 RoomThread나 sessions mutex 안에서 실행하지 않는다. `Begin_ColosseumPreparation`은 기존 numeric worker와 같은 단일 `std::async`에 immutable active generation과 취소 플래그만 전달한다. worker는 app/session/player 컨테이너를 캡처하지 않는다. RoomThread는 0ms future poll만 하고 완료 뒤 generation pointer, 인간 네 명 binding, 현재 queue/party/vote를 다시 확인하여 입장을 commit한다. 준비 중 다른 매치가 worker를 덮어쓰지 않으며 source는 Bern에 남는다.

CGameRoom의 선택적 cancellation 인수는 기본 null이므로 기존 레이드 생성 계약은 유지한다. worker 준비는 각 로드 단계 사이에서 취소를 확인하고 실패한 객체는 worker에서 버린다. shutdown은 RoomThread 정지 후 취소를 요청하고 최대 30초를 기다리며, I/O가 응답하지 않으면 기존 numeric worker와 동일한 ERROR_TIMEOUT process fail-fast를 사용한다. `TerminateThread`는 사용하지 않는다. worker 완료 시 실제 constructor 시간을 로그에 남기며 이 값은 raid tick 시간이나 GPU 성능으로 설명하지 않는다.

`m_ColosseumMatches`는 shared preview와 별도로 소유한다. direct Lobby 입장은 기존 shared room으로 유지하며 match ID가 없으므로 PvP 권한과 40줄 HP를 주지 않는다. private room은 binding이 없고 cleanup까지 소비된 뒤 폐기한다. revision/numeric transaction은 private 경기까지 현재 room 집합으로 검증한다.

## G01 — 모집과 전투 상태

각 팀 입장점 앞에 차원술사·창술사·워로드·가디언나이트·도화가를 기존 SERVER_PLAYER/character class spawn으로 만든다. 후보 중심의 고정 가로열은 실제 navgrid 밖으로 나갈 수 있으므로 기존 Guide landing과 같은 49개/반경 3m 이하 ring을 탐색한다. 전방 1m 이상, 입장점에서 7.5m 이하, 높이차 1m 이하, 기존 collision, staged 인간·용병 0.75m 분리와 실제 navigation LOS를 모두 통과한 위치만 확정한다. 원래 배치 후보 중 z=-4.1/-5.6 네 좌표가 현재 navgrid에서 walkable=0인 것을 실측했으며 허용 지형이나 collider를 넓히지 않는다. 가짜 socket이나 session을 만들지 않는다. 미선택 후보는 participant=false이며 공격과 피격 대상이 아니다. recruit는 요청자 match/team, phase, sequence, 후보 소유 팀, 생존, 거리와 선택수 둘을 검증한다. 기존 선택 재요청은 중복 선발하지 않는다. 선택 후 roster와 지속 state를 모든 인간에게 복제한다. 인간만 party leader가 될 수 있게 인간 둘 뒤에 용병을 붙인다.

사용자의 후속 변경에 따라 전투 HP는 현재 active BOSS_VALTAN profile HP의 1/4을 올림하고 전용 표시는 40줄이다. `iColosseumDamageReferenceHp`에는 원래 160줄 full HP를 별도로 보관하여 기존 피해량을 유지한다. 현재 설치 active BOSS_VALTAN 741285439를 소비하면 경기 HP는 185321360이며 피해 기준은 741285439다. source BossProfiles의 600000이나 이 예시 숫자를 코드에 하드코딩하지 않는다. 경기의 인간·선택 용병·미선택 후보는 입장에서 두 값을 확정하고 모집/진행/종료 중 Balance Tool numeric reload에도 현재 HP 비율, maxHP와 full reference를 보존한다. 변경한 class/boss 수치는 다음 신규 경기 입장부터 사용한다. Bern 귀환은 fresh staged player의 일반 class HP와 reference=0을 사용한다. 기존 레이드의 class HP 비율 migration과 boss 데이터는 바꾸지 않는다. 양 팀 두 용병 선택 완료 후 ACTIVE로 바꾸고, 살아 있는 participant가 없는 팀이 생기면 FINISHED로 바꾼다. 모집 중 인간 이탈은 경기 종료로 처리하여 영원히 모집에 머물지 않는다.

## G02 — 직업별 기존 스킬·이동·회피 소비

선택된 용병만 상대 팀 participant를 탐색하며 전술 판단은 0.2초 주기로 수행한다. 사용자의 최종 교정에 따라 차원술사만 Guide Combat 첫 authored combo인 W→A→S→D→F→V→T→ALT_V를 고정 순서로 사용하고 LMB를 금지한다. 이 순서는 후보 staging에서 active class/stance/slot을 resolve하고, 다음 한 스킬의 승인·종료를 기다린다. 사용 불가 단계 대기 5초와 전체 45초 timeout은 차원술사에만 적용한다.

창술사·워로드·가디언나이트·도화가는 현재 class/stance의 실제 binding을 LMB→Q→W→E→R→A→S→D→F→T→V→ALT_V→Z→X 순서로 순환한다. 무작위 선택은 없으며 없는 슬롯과 쿨다운·자원·상태로 거절된 스킬은 건너뛴다. 승인한 스킬이 종료되면 그 다음부터 찾고, 나머지가 전부 사용 불가하면 LMB로 돌아온다. 각 5Hz 판단에서 저장 vector의 용량을 재사용해 현재 목록을 갱신한다. 스탠스 변경으로 목록 길이가 달라질 때는 숫자 인덱스 대신 다음 입력 슬롯의 위치를 보존한다. Z/X도 published skill ID로 같은 Execute_PlayerSkill을 사용하고 native Commit_StanceChange 후 새로운 binding을 소비한다. SPACE는 아래 위험 회피에서만, STANDUP은 KNOCKDOWN에서만 사용하며 일반 공격 순환에서는 제외한다. 별도 stance나 인간·보스 executor는 만들지 않는다.

실행 중 COMBO는 매 Server fixed tick에 현재 published input window 안에서 같은 skill command를 한 번 buffer한다. 차원술사의 LMB는 이 경로에서도 금지하지만 나머지 네 직업 LMB의 BA continuation은 허용한다. 자동 단계와 이미 buffered인 단계에는 입력하지 않으며 executor의 false 반환을 신규 승인이나 다음 스킬 선택으로 해석하지 않는다. action이 NONE이 된 뒤에만 다른 스킬을 선택한다. 실행 중 HOLD는 기존 release로 끝낸다.

이동은 Execute_PlayerMove와 Server navigation/collision을 사용한다. 적 caster의 실제 향후 hit shape를 ServerCombatGeometry로 확인하고 위험도가 낮은 navigation landing으로 회피한다. 차원술사 외 네 직업은 그 방향으로 기존 executor의 SPACE를 먼저 시도하고 거절되면 navigation 회피를 사용한다. COMBO 중 다른 스킬은 공용 경계가 가용성 검사 전에 예약하므로, 진행 중 COMBO에서는 SPACE를 예약하지 않고 기존 navigation 회피를 유지한다. 임의 teleport, 별도 damage 식, 별도 이동 simulation을 만들지 않는다. active roster의 context를 PlayerSkillSystem Update에 전달하여 동일 collider/contact ledger에서 PvP를 판정한다.

## G03 — 실제 PvP 스킬 피해와 CC

별도 전투 담당자는 기존 캐스터 shape/window, projectile contact/timed, fallback 네 hit 경로에 상대 팀 player body(XZ와 높이)를 연결한다. 기존 ledger와 maxTargets를 유지한다. 전체 cast 피해는 ceil(iColosseumDamageReferenceHp × authored bars / 160)이며 도화가 T만 1/5를 적용한 뒤 subhit으로 분배한다. 40줄 maxHP로부터 피해 기준을 곱셈 복원하지 않으며 명시 reference가 없으면 bar 피해를 거절한다. 다른 피해는 기존 buff/spread/crit/defense와 공용 HP commit을 사용한다. 별도 incoming ±10% 산식은 추가하지 않는다.

ALT_V는 16m/1.5s, V는 5.1m/2.161s이며 아레나 외 탈출 허용은 false다. 일반 authored push는 유지한다. 워로드 원본 enemy stun 4초는 LANDED일 때만 기존 ActiveBuffs/knockdown 경로로 적용한다. 가디언 Z에는 현재 hit/범위 정의가 없어 전장 전체 debuff를 임의 생성하지 않는다. null PvP context의 보스 레이드 경로는 그대로다.

## 검증

변경 파일 diff와 인코딩/개행, vcxproj/filter XML parse, git diff --check를 확인한다. 네 명/팀2명, 실패 시 원본 보존, 서로 다른 두 match의 격리, 열 후보 중 두 명만 선발, 반대팀/원거리/중복 모집 거절, ACTIVE 전후 damage guard, 경기 종료/퇴장, 기존 shared preview/레이드 비적용을 계약 테스트로 확인한다. 추가로 실제 async 준비 동안 source 보존, 단일 worker 중복 거절, 미완료 future의 비차단 poll, source binding 변경 후 취소된 completion의 commit 거절을 검증한다. FINISHED의 사망 인간은 기존 square-hole transaction으로 Bern에 돌아와 일반 class HP와 무소속 상태를 받으며 ACTIVE 사망자의 조기 귀환은 차단한다. 회피는 원/고리/박스/부채꼴/360도부채꼴 다섯 경우에 실제 Update_Colosseum과 이동 executor가 안전한 navigation goal을 고르는지 검증한다. 실제 admission의 열 후보 skill 목록이 published class/stance/slot과 대응하는지 검증한다. 차원술사만 고정 순서와 LMB 금지를, 나머지 네 직업은 현재 모든 스킬과 LMB 포함을 확인한다. 실제 Update_Colosseum으로 차원술사 다음 스킬 cooldown 대기, 승인 후 action 종료 대기, CC 후 다음 순서 재개, 단계/전체 timeout을 확인한다. 워로드 E, 창술사 R/A, 도화가 R, 가디언 W의 실제 native Update로 모든 stage 도달·정상 종료·수동 window당 buffer 1회·자동 stage 무입력·추가 비용/쿨다운 없음·다음 슬롯 대기를 검사한다. 별도 좁은 260~290ms window도 실제 fixed tick에서 검사하여 5Hz think에 종속되지 않음을 확인한다. 나머지 네 직업의 native LMB 모든 단계, 전부 준비된 상태의 실제 LMB→Q→W 순환, 창술사 Z→짧아진 short stance 목록→새 LMB→Q binding, KNOCKDOWN→STANDUP 및 위험 방향 SPACE/사용 불가 시 navigation fallback을 검사한다. 첫 경기 인간을 가디언으로 구성하여 사망 종료 뒤 Bern 귀환의 일반 active profile HP(현재 132000) 복구도 검증한다. 실제 numeric parser/stage/commit으로 세 경기 phase의 HP/reference 보존과 Valtan room 일반 class HP 비율 migration을 대조한다. 컴파일과 Product build는 root 단일 runner만 수행한다. GPU와 Client 화면은 에이전트가 조작하지 않으며 최종 UI/행동 확인은 사용자 검증으로 구분한다.

## G09. 콜로세움 넉백·용병 ALT_V·체력 조정

2026-10-01 사용자는 UI 수정과 함께 콜로세움 넉백을 기존10%로 줄이고, 각 용병 ALT_V를
최소30초 간격으로 제한하며 참가자 체력을 절반으로 줄이도록 요청했다.

`ColosseumCombatPolicy.h`의10분의1 비율을 `PlayerSkillSystem.cpp`의 기존 PvP 적중 adapter에서
소비한다. authored push/pull과 V/ALT_V 특례를 결정한 뒤 signed 거리와 이동 시간을10분의1로
만든다. 시간은1ms 이상 올림하며 push가 없는 스킬은 그대로 둔다. V는0.51m/217ms,
ALT_V는1.6m/150ms다. 공용 레이드 reaction, 별도 stun/down·착지 회복·면역·경계 검사는 유지한다.
기존 combat 계약에서 네 hit 경로, 양방향 push/pull·무넉백·아군/범위/무적 차단과 raid parity를 확인한다.

`GameRoom_Colosseum.cpp`의 입장 HP는 ceil(active BOSS_VALTAN maximumHp/8)로 정한다.
현재40줄185321360에서20줄92660680으로 줄고, 기존160줄 스킬 피해 reference는 유지한다.
일반 직업 HP132000과 실제 보스 profile은 바꾸지 않는다. 기존 match fixture의 입장·부활·귀환·
numeric reload 기대값을20줄 기준으로 대조한다.

`GameRoom.h`의 room-owned 용병 상태에 마지막 ALT_V 승인 tick을 optional로 보관한다.
용병의 두 선택 경로는 이전 승인에서900tick(30Hz,30초)이 지나야 다음 ALT_V를 제출한다.
성공한 실제 승인만 기록하고 각 용병이 독립적으로 소유한다. 죽음·부활·rotation 재시작에서
이 기록을 지우지 않는다. 더 긴 기존 스킬 cooldown·자원·상태 검증도 계속 통과해야 한다.
인간 ALT_V 입력과 다른 world의 AI는 수정하지 않는다. 기존 테스트에899/900tick 경계,
실패한 승인·부활·독립 용병·긴 cooldown·tick wrap을 추가한다.

기존 C++ 파일만 수정하므로 프로젝트/filter 항목 추가는 없다. 현재 dirty인1인 상대 자동 용병과
입장·이동 수정은 보존한다. 최소 compile, 기존 combat/match 계약과 정상 Product Build 결과를
분리해 기록한다. 실제 Client 다인 화면·체감은 사용자 확인 범위다.
이름표의 복제 HP 비율 표시도 최대20줄로 맞추며 일반 월드 HP 표시는 유지한다.
