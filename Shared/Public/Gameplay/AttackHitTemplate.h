#pragma once

#include <cmath>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace LostArk::Shared
{
// 공격 한 종류의 판정 정의다. 편집 문서와 Server가 같은 필드·검증 규칙을 사용한다.
// 실행 중 경과 시간·명중 이력은 여기에 저장하지 않으며, Effect의 표시 도형이 판정을 소유하지 않는다.
struct ATTACK_HIT_TEMPLATE final
{
    // ID는 저장·참조에 쓰는 문자열이다. vector에서의 현재 위치와는 별개다.
    std::string strHitId;
    std::string strTrigger = "TIMED";
    std::string strShape = "CIRCLE";
    std::string strDamageKind = "MAX_HP_PERCENT";
    std::string strDamageProfileId;

    // 소비자가 정한 기준(공격 객체 생성 또는 stage 시작)의 시간(ms)이다.
    // TIMED는 예약 시각, CONTACT는 접촉을 허용할 시간 구간을 뜻한다.
    std::uint32_t iAtMs = 0u;
    std::uint32_t iEndMs = 0u;
    std::uint32_t iRepeatCount = 1u;
    std::uint32_t iRepeatIntervalMs = 0u;
    std::uint32_t iDamagePercent = 10u;

    // 판정 도형의 크기(m)와 각도(degree). 실제 월드 배치는 소비자가 기준 위치·방향과 결합한다.
    double fRadiusM = 1.0;
    double fInnerRadiusM = 0.0;
    double fLengthM = 0.0;
    double fHalfWidthM = 0.0;
    double fAngleDegrees = 0.0;
    double fOffsetForwardM = 0.0;
    double fOffsetRightM = 0.0;
    double fYawOffsetDegrees = 0.0;

    // 피격 이동의 높이·거리(m)와 시간(ms). 실제 대상 이동은 Server 소비자가 처리한다.
    double fRiseHeightM = 0.0;
    double fPushRangeM = 0.0;
    std::uint32_t iPushMs = 0u;
    // 미지정은 기존 공통 피격 이동 정책을 유지한다. 명시적인 false와 구분해야 한다.
    std::optional<bool> ForcePush;
    std::string strPushDirection = "AWAY_FROM_CONTACT";
    // Server의 수치 조정 원본을 식별한다. 저작 JSON이나 네트워크 데이터로 직렬화하지 않는다.
    std::string strNumericBalanceId;
    bool operator==(const ATTACK_HIT_TEMPLATE&) const = default;
};

// 아래 검증 함수와 파서가 사용하는 ID 문법 검사다. 참조 대상의 존재·전역 유일성은 검사하지 않는다.
// std::string의 byte 길이를 제한하고 ASCII만 허용하므로 실행 환경의 문자 분류 설정에 의존하지 않는다.
inline bool Is_AttackStableId(const std::string& id)
{
    if (id.empty() || id.size() > 128u)
        return false;

    for (const unsigned char ch : id)
    {
        const bool isLowercase = ch >= 'a' && ch <= 'z';
        const bool isUppercase = ch >= 'A' && ch <= 'Z';
        const bool isDigit = ch >= '0' && ch <= '9';
        const bool isSeparator = ch == '_' || ch == '.' || ch == '-';
        if (!(isLowercase || isUppercase || isDigit || isSeparator))
            return false;
    }
    return true;
}

// 공개 검증 함수의 세부 규칙이다. 모두 입력을 읽기만 하며 실패해도 정의·실행 상태를 바꾸지 않는다.
namespace AttackHitValidationDetail
{
inline bool Validate_Timing(const ATTACK_HIT_TEMPLATE& hit, const std::uint32_t lifetimeMs)
{
    // 600000ms는 저장 계약의 상한이며, 실제 허용 수명은 호출자가 lifetimeMs로 전달한다.
    if (hit.iAtMs > 600000u || hit.iEndMs > 600000u ||
        hit.iRepeatCount == 0u || hit.iRepeatCount > 64u || hit.iRepeatIntervalMs > 600000u)
        return false;

    if (hit.iRepeatCount > 1u && hit.iRepeatIntervalMs < 34u)
        return false;

    if (hit.strTrigger == "TIMED")
    {
        // 횟수가 1 이상임을 확인한 뒤 빼고, 곱셈 전에 64bit로 넓혀 마지막 예약 시각을 구한다.
        const std::uint64_t lastHitMs = hit.iAtMs +
            std::uint64_t(hit.iRepeatCount - 1u) * hit.iRepeatIntervalMs;
        return hit.iEndMs == 0u && lastHitMs <= lifetimeMs;
    }
    if (hit.strTrigger == "CONTACT")
    {
        // 접촉은 활성 구간을 검사한다. TIMED의 마지막 반복 시각 조건을 이 경로에 추가하지 않는다.
        return hit.iAtMs < hit.iEndMs && hit.iEndMs <= lifetimeMs;
    }
    return false;
}

inline bool Validate_Push(const ATTACK_HIT_TEMPLATE& hit)
{
    if (!std::isfinite(hit.fPushRangeM) || hit.fPushRangeM < 0.0 || hit.fPushRangeM > 100.0)
        return false;

    if (hit.strPushDirection != "AWAY_FROM_CONTACT" && hit.strPushDirection != "AWAY_FROM_BOSS")
        return false;

    if (!std::isfinite(hit.fRiseHeightM) || hit.fRiseHeightM < 0.0 || hit.fRiseHeightM > 100.0)
        return false;

    // 이동이 없으면 시간도 0이어야 한다. 수직 또는 수평 이동이 있으면 유효한 이동 시간이 필요하다.
    const bool hasMovement = hit.fRiseHeightM != 0.0 || hit.fPushRangeM != 0.0;
    if (!hasMovement)
        return hit.iPushMs == 0u;

    return hit.iPushMs >= 100u && hit.iPushMs <= 5000u;
}

inline bool Validate_Shape(const ATTACK_HIT_TEMPLATE& hit)
{
    // 선택된 도형에서 쓰지 않는 필드도 유한한 저장 범위 안에 있어야 한다.
    for (const double extentM : {hit.fRadiusM, hit.fInnerRadiusM, hit.fLengthM, hit.fHalfWidthM})
    {
        if (!std::isfinite(extentM) || extentM < 0.0 || extentM > 1000.0)
            return false;
    }
    for (const double offsetM : {hit.fOffsetForwardM, hit.fOffsetRightM})
    {
        if (!std::isfinite(offsetM) || std::abs(offsetM) > 1000.0)
            return false;
    }
    if (!std::isfinite(hit.fAngleDegrees) || hit.fAngleDegrees < 0.0 || hit.fAngleDegrees > 360.0 ||
        !std::isfinite(hit.fYawOffsetDegrees) || std::abs(hit.fYawOffsetDegrees) > 360.0)
        return false;

    // 공통 범위를 통과한 값들이 해당 도형을 실제로 만들 수 있는 조합인지 확인한다.
    if (hit.strShape == "CIRCLE")
        return hit.fRadiusM > 0.0 && hit.fInnerRadiusM == 0.0;
    if (hit.strShape == "RING")
        return hit.fInnerRadiusM > 0.0 && hit.fInnerRadiusM < hit.fRadiusM;
    if (hit.strShape == "BOX")
        return hit.fLengthM > 0.0 && hit.fHalfWidthM > 0.0;
    if (hit.strShape == "CONE")
        return hit.fLengthM > 0.0 && hit.fAngleDegrees > 0.0 && hit.fInnerRadiusM < hit.fLengthM;

    return false;
}

inline bool Validate_Damage(const ATTACK_HIT_TEMPLATE& hit)
{
    if (hit.strDamageKind == "PROFILE")
    {
        // 여기서는 ID 문법만 검사한다. 피해 profile의 실제 존재는 Catalog의 참조 검증 책임이다.
        return Is_AttackStableId(hit.strDamageProfileId) && hit.iDamagePercent == 0u;
    }
    if (!hit.strDamageProfileId.empty())
        return false;

    if (hit.strDamageKind == "MAX_HP_PERCENT")
        return hit.iDamagePercent > 0u && hit.iDamagePercent <= 100u;
    if (hit.strDamageKind == "INSTANT_DEATH")
        return hit.iDamagePercent == 0u;

    return false;
}
}

// Client의 편집 문서 파서·도구와 Server의 Catalog·공격 객체 진입부가 같은 정의를 검사한다.
// true는 데이터 계약을 만족한다는 뜻이다. 이 함수는 공격을 실행하거나 대상의 HP를 변경하지 않는다.
// 실패 시 기존 문서·Catalog를 유지할지는 호출자가 임시 데이터와 commit 경계로 보장한다.
inline bool Validate_AttackHitTemplates(const std::vector<ATTACK_HIT_TEMPLATE>& hits,
    const std::uint32_t lifetimeMs = 600000u)
{
    if (hits.size() > 32u)
        return false;

    for (std::size_t i = 0u; i < hits.size(); ++i)
    {
        const auto& hit = hits[i];
        if (!Is_AttackStableId(hit.strHitId) ||
            !AttackHitValidationDetail::Validate_Timing(hit, lifetimeMs) ||
            !AttackHitValidationDetail::Validate_Push(hit) ||
            !AttackHitValidationDetail::Validate_Shape(hit) ||
            !AttackHitValidationDetail::Validate_Damage(hit))
            return false;

        // 최대 32개의 작은 목록이므로 별도 소유 컨테이너 없이 앞서 본 ID와 비교한다.
        // 이 목록 안의 중복만 거부하며 다른 패턴의 ID까지 전역 검사하지는 않는다.
        for (std::size_t previous = 0u; previous < i; ++previous)
        {
            if (hits[previous].strHitId == hit.strHitId)
                return false;
        }
    }
    // 판정 정의가 없는 빈 목록도 허용한다. 필수 여부는 각 호출자의 상위 계약에서 결정한다.
    return true;
}

// 발탄의 Client 문서와 Server Catalog가 stage의 공통 판정 일정과 개별 도형 정의를 대조한다.
// offsets가 있으면 명시 시각을, 없으면 delay + index * interval을 사용한다. 단위는 모두 ms다.
// contacts가 없는 stage는 기존 stage 판정 경로를 쓰므로 빈 목록은 다른 인자와 무관하게 허용한다.
inline bool Validate_StageAttackContacts(const std::vector<ATTACK_HIT_TEMPLATE>& hits,
    const std::uint32_t lifetimeMs, const std::vector<std::uint32_t>& offsets,
    const std::uint32_t count, const std::uint32_t delayMs, const std::uint32_t intervalMs)
{
    if (hits.empty())
        return true;

    if (lifetimeMs == 0u || hits.size() != count)
        return false;
    if (!offsets.empty() && offsets.size() != count)
        return false;

    // stage 종료 시각 자체에는 새 판정을 예약하지 않는다. 0 검사가 뺄셈보다 먼저 와야 한다.
    if (!Validate_AttackHitTemplates(hits, lifetimeMs - 1u))
        return false;

    for (std::size_t i = 0u; i < hits.size(); ++i)
    {
        const auto& hit = hits[i];
        const std::uint64_t scheduledAtMs = offsets.empty()
            ? std::uint64_t(delayMs) + i * intervalMs
            : offsets[i];

        // 각 contact는 stage 일정의 한 판정 기회에 대응하므로 개별 반복 설정을 허용하지 않는다.
        if (hit.strTrigger != "TIMED" || hit.iEndMs != 0u || hit.iRepeatCount != 1u ||
            hit.iRepeatIntervalMs != 0u || hit.iAtMs != scheduledAtMs)
            return false;
    }
    return true;
}
}
