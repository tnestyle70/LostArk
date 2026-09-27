#include "GameRoom.h"
#include "GameRoom_Internal.h"
#include "Gameplay/WorldCollisionContract.h"
#include <algorithm>
#include <cmath>

using namespace LostArk::Server;
using namespace LostArk::Shared;

float CGameRoom::Predict_GuideContactRisk(const SERVER_PLAYER& guide, float x, float z)
{
    SERVER_PLAYER probe;
    probe.iPlayerId = guide.iPlayerId;
    probe.iNetEntityId = guide.iNetEntityId;
    probe.fPositionX = x; probe.fPositionY = guide.fPositionY; probe.fPositionZ = z;
    const auto horizon = static_cast<std::uint32_t>(m_GuideCatalog.HorizonSeconds * 1000.f);
    float risk = 0.f;
    const auto* product = Resolve_KoukuProductCatalog();
    const auto* catalog = product ? product : &m_GameplayCatalog.Active();
    for (const auto& boss : m_WorldEntities)
    {
        if (!boss.iCurrentHp || boss.strPatternId.empty()) continue;
        const auto* patterns = catalog->Find_BossPatterns(boss.strEncounterId);
        if (!patterns) continue;
        const auto pattern = std::find_if(patterns->begin(), patterns->end(), [&](const auto& row) {
            return row.strPatternId == boss.strPatternId;
        });
        if (pattern != patterns->end())
            risk += CValtanBrain::Predict_ContactRisk(boss, *pattern, probe, m_iServerTick, horizon);
    }
    if (product)
    {
        const auto sample = [&](const auto& member, const SERVER_WORLD_ENTITY* owner) {
            if (!owner || !member.LogicLedger.Is_Active()) return 0.f;
            std::string status;
            const auto* pattern = CKoukuSaydonBrain::Find_AnimationOnlyPattern(
                *product, member.LogicLedger.strPatternId, status);
            return pattern ? CKoukuSaydonLogicRuntime::Predict_ContactRisk(
                *owner, *pattern, member.LogicLedger, probe, m_iServerTick, horizon) : 0.f;
        };
        for (const auto& member : m_KoukuSaydonPatternAudition.Members)
            risk += sample(member, Find_KoukuOccurrenceOwner(member.iBossEntityId, member.iPatternSequence));
        for (const auto& tail : m_KoukuSaydonPatternAudition.Tails)
            risk += sample(tail.Member, tail.pOwner.get());
    }
    // The persistent bingo hammer is a room hazard, independent of pattern clocks.
    for (const auto& hammer : m_KoukuBingoDuration.Hammers)
    {
        if (hammer.anchor < 0 || !hammer.startTick) continue;
        const float age = static_cast<float>(m_iServerTick - hammer.startTick) * 1000.f / GameRoomDetail::SERVER_TICK_HZ;
        const float begin = float(KOUKU_BINGO_HAMMER_WARNING_MS + KOUKU_BINGO_HAMMER_DESCEND_MS);
        if (age + horizon < begin || age >= begin + KOUKU_BINGO_HAMMER_SWEEP_MS) continue;
        const auto path = Kouku_BingoHammerPath(hammer.anchor);
        const float dx = path.fEndX - path.fStartX, dz = path.fEndZ - path.fStartZ;
        const float length = std::hypot(dx, dz);
        if (length <= 0.f) continue;
        const float start = std::clamp((age - begin) / KOUKU_BINGO_HAMMER_SWEEP_MS, 0.f, 1.f);
        const float end = std::clamp((age + horizon - begin) / KOUKU_BINGO_HAMMER_SWEEP_MS, 0.f, 1.f);
        const float fx = dx / length, fz = dz / length;
        if (CombatCollision::Circle_IntersectsForwardBox(
            {x, z, WorldCollision::PLAYER_HALF_EXTENT_X + WorldCollision::CONTACT_MARGIN},
            path.fStartX + dx * start - fx * m_KoukuBingoDuration.fHammerHalfForwardM,
            path.fStartZ + dz * start - fz * m_KoukuBingoDuration.fHammerHalfForwardM,
            fx, fz, 2.f * m_KoukuBingoDuration.fHammerHalfForwardM + length * (end - start),
            m_KoukuBingoDuration.fHammerHalfWidthM)) risk += 5.f;
    }
    return risk;
}
