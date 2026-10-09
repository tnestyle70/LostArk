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
