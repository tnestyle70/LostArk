"""쿠크와 발탄의 저작 공격 정의를 검사하고 기본값을 채운다.

판정 도형은 이펙트 렌더 요소의 bounding box와 별개다. 상대 시각의 기준은
쿠크 CombatObject 생성 또는 발탄 stage 시작이며, 실제 판정은 Server가 수행한다.
"""
from __future__ import annotations

import copy
import math
import re


# 쿠크 projector와 발탄 valtan_tuning_pipeline이 함께 사용하는 저작 데이터 검증이다.
# 발탄은 stage 시작, 쿠크 동적 공격은 CombatObject 생성 시각을 시간 기준으로 사용한다.
# 실제 피해 판정이나 파일 저장은 수행하지 않고, 누락 필드를 채운 새 공격 정의를 반환한다.
HIT_DEFAULTS = dict(
    hitId="", trigger="TIMED", atMs=0, endMs=0,
    repeatCount=1, repeatIntervalMs=0, shape="CIRCLE",
    radiusM=1.0, innerRadiusM=0.0, lengthM=0.0, halfWidthM=0.0,
    angleDegrees=0.0, offsetForwardM=0.0, offsetRightM=0.0,
    yawOffsetDegrees=0.0, damageKind="MAX_HP_PERCENT",
    damagePercent=10, damageProfileId="",
)


# 두 보스의 projector가 전달한 목록을 검증하고 기본값을 채운 복사본을 돌려준다.
# 입력 rows는 변경하지 않으며, 중간 행이 실패하면 ValueError로 끝나 부분 결과를 반환하지 않는다.
# lifetime_ms는 허용하는 마지막 상대 시각(ms)이다. 발탄 stage의 끝을 제외하는 규칙은
# 호출자가 duration - 1을 전달해 적용하고, 여기서는 그 경계를 넘는지만 검사한다.
def validate_hits(rows, lifetime_ms=600000):
    if not isinstance(rows, list) or len(rows) > 32:
        raise ValueError("Attack templates require a bounded array of at most 32 hits")
    result, ids = [], set()
    for row in rows:
        if not isinstance(row, dict) or set(row) - (
            HIT_DEFAULTS.keys()
            | {"riseHeightM", "pushMs", "pushRangeM", "forcePush", "pushDirection"}
        ):
            raise ValueError("Attack hit has unknown fields")
        hit = {**HIT_DEFAULTS, **copy.deepcopy(row)}
        identity = hit["hitId"]
        if (
            not isinstance(identity, str)
            or not re.fullmatch(r"[A-Za-z0-9_.-]{1,128}", identity)
            or identity in ids
        ):
            raise ValueError("Attack hit ID must be stable and unique within its template")
        ids.add(identity)

        # Python의 bool은 int의 하위 타입이므로 정확한 type 비교로 true/false 입력을 거부한다.
        for field in ("atMs", "endMs", "repeatCount", "repeatIntervalMs", "damagePercent"):
            maximum = (
                64 if field == "repeatCount"
                else 100 if field == "damagePercent"
                else 600000
            )
            if type(hit[field]) is not int or not 0 <= hit[field] <= maximum:
                raise ValueError("Attack hit integer is invalid: " + field)
        # 거리·도형 크기는 m, 회전은 degree다. 위치 offset과 yaw만 음수를 허용한다.
        for field in (
            "radiusM", "innerRadiusM", "lengthM", "halfWidthM",
            "angleDegrees", "offsetForwardM", "offsetRightM", "yawOffsetDegrees",
        ):
            value = hit[field]
            bound = 360 if field in ("angleDegrees", "yawOffsetDegrees") else 1000
            minimum = -bound if field in ("offsetForwardM", "offsetRightM", "yawOffsetDegrees") else 0
            if type(value) not in (int, float) or not math.isfinite(value) or not minimum <= value <= bound:
                raise ValueError("Attack hit number is invalid: " + field)

        # 34ms는 30Hz 한 tick보다 짧은 반복을 막는다. TIMED는 마지막 예약 시각을,
        # CONTACT는 접촉 판정 구간을 검사하며 CONTACT에 TIMED의 반복 끝 제한을 덧붙이지 않는다.
        if hit["repeatCount"] < 1 or (hit["repeatCount"] > 1 and hit["repeatIntervalMs"] < 34):
            raise ValueError("Attack repetitions need at least one 30 Hz tick between hits")
        last = hit["atMs"] + (hit["repeatCount"] - 1) * hit["repeatIntervalMs"]
        if hit["trigger"] == "TIMED":
            if hit["endMs"] or last > lifetime_ms:
                raise ValueError("Timed attack exceeds the owning lifetime")
        elif hit["trigger"] == "CONTACT":
            if not hit["atMs"] < hit["endMs"] <= lifetime_ms:
                raise ValueError("Contact attack needs a bounded active window")
        else:
            raise ValueError("Attack trigger must be TIMED or CONTACT")

        # 여기서는 판정 도형의 유효한 치수만 확인한다. 실제 위치 변환과 피격 대상 선택은 Server 책임이다.
        shape = hit["shape"]
        if shape == "CIRCLE":
            valid = hit["radiusM"] > 0 and hit["innerRadiusM"] == 0
        elif shape == "RING":
            valid = 0 < hit["innerRadiusM"] < hit["radiusM"]
        elif shape == "BOX":
            valid = hit["lengthM"] > 0 and hit["halfWidthM"] > 0
        elif shape == "CONE":
            valid = (
                hit["lengthM"] > 0
                and 0 < hit["angleDegrees"] <= 360
                and hit["innerRadiusM"] < hit["lengthM"]
            )
        else:
            valid = False
        if not valid:
            raise ValueError("Attack primitive dimensions are invalid")

        # PROFILE은 ID 문법과 percent 배타 조건만 검사한다. 실제 profile 존재 여부는
        # 이 함수가 소유하지 않는 Catalog 참조 검증 단계에서 확인해야 한다.
        kind = hit["damageKind"]
        if kind == "PROFILE":
            valid = (
                isinstance(hit["damageProfileId"], str)
                and re.fullmatch(r"[A-Za-z0-9_.-]{1,128}", hit["damageProfileId"])
                and hit["damagePercent"] == 0
            )
        else:
            valid = hit["damageProfileId"] == "" and (
                (kind == "MAX_HP_PERCENT" and 1 <= hit["damagePercent"] <= 100)
                or (kind == "INSTANT_DEATH" and hit["damagePercent"] == 0)
            )
        if not valid:
            raise ValueError("Attack damage policy is invalid")

        # 공중 이동 높이와 수평 이동 거리가 모두 0이면 시간도 0이어야 한다.
        # 하나라도 이동이 있으면 100~5000ms가 필요하며, 선택 필드의 생략 상태는 그대로 보존한다.
        height, duration = hit.get("riseHeightM", 0), hit.get("pushMs", 0)
        push_range = row.get("pushRangeM", 0)
        if type(push_range) not in (int, float) or not math.isfinite(push_range) or not 0 <= push_range <= 100:
            raise ValueError("Attack horizontal push requires a finite bounded range")
        if (
            type(height) not in (int, float)
            or not math.isfinite(height)
            or not 0 <= height <= 100
            or type(duration) is not int
            or (
                duration != 0 if height == 0 and push_range == 0
                else not 100 <= duration <= 5000
            )
        ):
            raise ValueError("Attack push motion and duration must form a bounded pair")
        if "forcePush" in row and type(row["forcePush"]) is not bool:
            raise ValueError("Attack forcePush must be boolean")
        if row.get("pushDirection", "AWAY_FROM_CONTACT") not in ("AWAY_FROM_CONTACT", "AWAY_FROM_BOSS"):
            raise ValueError("Attack push direction must retain its contact or boss origin")
        if "pushRangeM" in row:
            hit["pushRangeM"] = push_range
        if "riseHeightM" in row or "pushMs" in row:
            hit["riseHeightM"], hit["pushMs"] = height, duration
        result.append(hit)
    return result


# 쿠크 composition 검증이 Logic 종류와 공격 목록의 소유 관계를 확인할 때 호출한다.
# 목록을 검증하되 normalize 결과를 logic에 다시 쓰지 않는다. projector가 실제 인스턴스의
# lifetime으로 validate_hits를 다시 호출해 게시할 목록을 만들며, 이 함수는 파일을 게시하지 않는다.
def validate_logic_hits(logic):
    kind = (
        logic.get("judgementKind") if logic.get("logicType") == "DURATION"
        else logic.get("triggerKind", "")
    )
    allowed = (
        {"fixedHits", "trackingHits", "randomVolleyHits"} if kind == "SHOWTIME_PLAYER_TARGETS"
        else {"projectileHits"} if kind == "PURSUIT_PROJECTILES"
        else {"fixedHits"} if kind in {"ALBION_BLUE_CIRCLE", "ALBION_AIRBORNE"}
        else set()
    )
    keys = {"fixedHits", "trackingHits", "randomVolleyHits", "projectileHits"} & logic.keys()
    if keys - allowed:
        raise ValueError("Only the matching dynamic attack owner can carry hit templates")
    for key in keys - {"randomVolleyHits"}:
        validate_hits(
            logic[key],
            logic.get("effectLifetimeMs", 0) if kind == "ALBION_BLUE_CIRCLE" else 600000,
        )

    # randomVolleyHits의 각 목록은 같은 순서의 시각 occurrence set과 일대일로 대응한다.
    if "randomVolleyHits" in keys:
        rows = logic["randomVolleyHits"]
        if not isinstance(rows, list) or len(rows) != len(logic.get("randomVolleyOccurrenceSets", [])):
            raise ValueError("Random attack templates must match the ordered visual set count")
        for hits in rows:
            validate_hits(hits)

    # 공격을 표시할 owner가 없는 정의를 차단한다. 선택 위치·고정 그룹·추적 occurrence의
    # 실제 ID 연결은 각 projector가 담당하며 여기서는 필요한 필드의 존재 조건을 확인한다.
    if kind == "ALBION_AIRBORNE" and logic.get("fixedHits") and (
        logic.get("airbornePhase") != "SELECT_PLAYER"
        or logic.get("airborneTargetPositionPolicy") != "SELECT"
        or not logic.get("selectedEffectGroupId")
    ):
        raise ValueError("Selected attack lacks its captured position and visual owner")
    if (
        kind not in {"ALBION_BLUE_CIRCLE", "ALBION_AIRBORNE"}
        and logic.get("fixedHits")
        and not logic.get("fixedSelectionGroupId")
    ):
        raise ValueError("Fixed attack lacks its visual owner")
    if logic.get("trackingHits") and not logic.get("trackingPresentationOccurrenceId"):
        raise ValueError("Tracking attack lacks its visual owner")
