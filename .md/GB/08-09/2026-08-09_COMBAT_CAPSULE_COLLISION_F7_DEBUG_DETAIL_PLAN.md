# 전투 Capsule Collider, Valtan Pattern, Knockback, Arena Spawn과 F7 Debug 상세 코드 계획서

## 0. 문서 사용법

이 문서는 구현 시 오른쪽 창에 두고 각 G의 내부 순서를 반영하는 코드 계약이다. 기존 G00~G15 foundation에 이후
요구가 G16~G23으로 확장되었으므로 실제 G 간 적용 순서는 문서 끝 `최종 적용 순서`를 따른다. 새 H/CPP의 public contract와
핵심 algorithm은 전체 형태로 적고, 수백 줄인 기존 catalog/publisher/GameRoom/MainApp에는 교체할 struct,
함수 또는 insertion block을 정확히 적는다. 구현 중 실제 baseline signature가 달라졌다면 임의로 맞추지 않고
먼저 이 문서의 대응 G를 갱신한다.

이 문서의 `HIT_VOLUME`은 gameplay authority다. player skill shape v1은 lossless migration용
`LEGACY_XZ_RANGE`와 reviewed `CAPSULE` 두 종류다. G16 이후의 Valtan event는 기존
`CIRCLE/RING/CONE/BOX/CROSS`를 lossless target-point mode로 옮긴 뒤 reviewed capsule-aware shape와 weapon track을
추가한다. 어느 쪽도 Effect Mesh bounds debug, Client `CCollider`, PhysX capsule로 대체하지 않는다.

## G00. Shared capsule geometry 계약

### 대상

- 새 파일 `Shared/Public/Gameplay/CombatCollisionContract.h`
- 새 파일 `Shared/Private/CombatCollisionContract.cpp`
- `Shared/Default/Shared.vcxproj`
- `Shared/Default/Shared.vcxproj.filters`

### `CombatCollisionContract.h` 전체 코드

```cpp
#pragma once

#include <cstdint>

namespace LostArk::Shared::CombatCollision
{
	inline constexpr std::uint32_t FIXED_TICK_HZ = 30u;
	inline constexpr float CONTACT_EPSILON = 0.0001f;
	inline constexpr float MAXIMUM_RADIUS = 32.f;
	inline constexpr float MAXIMUM_SEGMENT_LENGTH = 64.f;
	inline constexpr std::uint32_t MAXIMUM_STAGES_PER_SKILL = 8u;
	inline constexpr std::uint32_t MAXIMUM_HIT_EVENTS_PER_STAGE = 16u;
	inline constexpr std::uint32_t MAXIMUM_REPEATS_PER_EVENT = 8u;
	inline constexpr std::uint32_t MAXIMUM_TARGETS_PER_REPEAT = 16u;
	inline constexpr std::uint32_t MAXIMUM_HIT_APPLICATIONS_PER_STAGE = 2048u;

	enum class COMBAT_HIT_VOLUME_KIND : std::uint8_t
	{
		LEGACY_XZ_RANGE,
		CAPSULE,
		END
	};

	struct COMBAT_BODY_CAPSULE final
	{
		float fRadius = 0.f;
		float fCylinderHalfHeight = 0.f;
		float fCenterOffsetY = 0.f;
	};

	struct COMBAT_CAPSULE final
	{
		float fStartX = 0.f;
		float fStartY = 0.f;
		float fStartZ = 0.f;
		float fEndX = 0.f;
		float fEndY = 0.f;
		float fEndZ = 0.f;
		float fRadius = 0.f;
	};

	struct COMBAT_AABB final
	{
		float fMinimumX = 0.f;
		float fMinimumY = 0.f;
		float fMinimumZ = 0.f;
		float fMaximumX = 0.f;
		float fMaximumY = 0.f;
		float fMaximumZ = 0.f;
	};

	struct COMBAT_LOCAL_HIT_VOLUME final
	{
		COMBAT_HIT_VOLUME_KIND eKind = COMBAT_HIT_VOLUME_KIND::END;
		float fLegacyXzRange = 0.f;
		COMBAT_CAPSULE LocalCapsule;
	};

	bool Is_Valid(const COMBAT_BODY_CAPSULE& body);
	bool Is_Valid(const COMBAT_CAPSULE& capsule);
	bool Is_Valid(const COMBAT_LOCAL_HIT_VOLUME& volume);

	COMBAT_CAPSULE Build_BodyCapsule(
		float rootX,
		float rootY,
		float rootZ,
		const COMBAT_BODY_CAPSULE& body);

	COMBAT_CAPSULE Transform_LocalCapsule(
		const COMBAT_CAPSULE& localCapsule,
		float rootX,
		float rootY,
		float rootZ,
		float yawDegrees);

	COMBAT_AABB Build_Aabb(const COMBAT_CAPSULE& capsule);
	bool Overlaps(const COMBAT_AABB& left, const COMBAT_AABB& right);
	float DistanceSquared(const COMBAT_CAPSULE& left, const COMBAT_CAPSULE& right);
	bool Intersects(const COMBAT_CAPSULE& left, const COMBAT_CAPSULE& right);
}
```

### `CombatCollisionContract.cpp` 전체 코드

```cpp
#include "Gameplay/CombatCollisionContract.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace
{
	struct VECTOR3 final
	{
		float x = 0.f;
		float y = 0.f;
		float z = 0.f;
	};

	bool IsFinite(const float value)
	{
		return std::isfinite(value);
	}

	VECTOR3 Subtract(const VECTOR3& left, const VECTOR3& right)
	{
		return { left.x - right.x, left.y - right.y, left.z - right.z };
	}

	VECTOR3 AddScaled(const VECTOR3& point, const VECTOR3& direction, const float scale)
	{
		return {
			point.x + direction.x * scale,
			point.y + direction.y * scale,
			point.z + direction.z * scale,
		};
	}

	float Dot(const VECTOR3& left, const VECTOR3& right)
	{
		return left.x * right.x + left.y * right.y + left.z * right.z;
	}

	float LengthSquared(const VECTOR3& value)
	{
		return Dot(value, value);
	}

	float Clamp01(const float value)
	{
		return (std::max)(0.f, (std::min)(1.f, value));
	}

	VECTOR3 StartOf(const LostArk::Shared::CombatCollision::COMBAT_CAPSULE& capsule)
	{
		return { capsule.fStartX, capsule.fStartY, capsule.fStartZ };
	}

	VECTOR3 EndOf(const LostArk::Shared::CombatCollision::COMBAT_CAPSULE& capsule)
	{
		return { capsule.fEndX, capsule.fEndY, capsule.fEndZ };
	}
}

bool LostArk::Shared::CombatCollision::Is_Valid(const COMBAT_BODY_CAPSULE& body)
{
	return IsFinite(body.fRadius) &&
		IsFinite(body.fCylinderHalfHeight) &&
		IsFinite(body.fCenterOffsetY) &&
		body.fRadius > 0.f && body.fRadius <= MAXIMUM_RADIUS &&
		body.fCylinderHalfHeight >= 0.f &&
		body.fCylinderHalfHeight * 2.f <= MAXIMUM_SEGMENT_LENGTH &&
		body.fCenterOffsetY >= body.fRadius + body.fCylinderHalfHeight &&
		body.fCenterOffsetY <= MAXIMUM_SEGMENT_LENGTH;
}

bool LostArk::Shared::CombatCollision::Is_Valid(const COMBAT_CAPSULE& capsule)
{
	if (!IsFinite(capsule.fStartX) || !IsFinite(capsule.fStartY) ||
		!IsFinite(capsule.fStartZ) || !IsFinite(capsule.fEndX) ||
		!IsFinite(capsule.fEndY) || !IsFinite(capsule.fEndZ) ||
		!IsFinite(capsule.fRadius) || capsule.fRadius <= 0.f ||
		capsule.fRadius > MAXIMUM_RADIUS)
	{
		return false;
	}
	const VECTOR3 segment = Subtract(EndOf(capsule), StartOf(capsule));
	return LengthSquared(segment) <=
		MAXIMUM_SEGMENT_LENGTH * MAXIMUM_SEGMENT_LENGTH;
}

bool LostArk::Shared::CombatCollision::Is_Valid(
	const COMBAT_LOCAL_HIT_VOLUME& volume)
{
	if (COMBAT_HIT_VOLUME_KIND::LEGACY_XZ_RANGE == volume.eKind)
	{
		return IsFinite(volume.fLegacyXzRange) &&
			volume.fLegacyXzRange > 0.f &&
			volume.fLegacyXzRange <= MAXIMUM_RADIUS &&
			0.f == volume.LocalCapsule.fStartX &&
			0.f == volume.LocalCapsule.fStartY &&
			0.f == volume.LocalCapsule.fStartZ &&
			0.f == volume.LocalCapsule.fEndX &&
			0.f == volume.LocalCapsule.fEndY &&
			0.f == volume.LocalCapsule.fEndZ &&
			0.f == volume.LocalCapsule.fRadius;
	}
	if (COMBAT_HIT_VOLUME_KIND::CAPSULE == volume.eKind)
	{
		return 0.f == volume.fLegacyXzRange &&
			Is_Valid(volume.LocalCapsule);
	}
	return false;
}

LostArk::Shared::CombatCollision::COMBAT_CAPSULE
LostArk::Shared::CombatCollision::Build_BodyCapsule(
	const float rootX,
	const float rootY,
	const float rootZ,
	const COMBAT_BODY_CAPSULE& body)
{
	COMBAT_CAPSULE capsule{};
	if (!Is_Valid(body) || !IsFinite(rootX) || !IsFinite(rootY) || !IsFinite(rootZ))
		return capsule;
	const float centerY = rootY + body.fCenterOffsetY;
	capsule.fStartX = rootX;
	capsule.fStartY = centerY - body.fCylinderHalfHeight;
	capsule.fStartZ = rootZ;
	capsule.fEndX = rootX;
	capsule.fEndY = centerY + body.fCylinderHalfHeight;
	capsule.fEndZ = rootZ;
	capsule.fRadius = body.fRadius;
	return capsule;
}

LostArk::Shared::CombatCollision::COMBAT_CAPSULE
LostArk::Shared::CombatCollision::Transform_LocalCapsule(
	const COMBAT_CAPSULE& localCapsule,
	const float rootX,
	const float rootY,
	const float rootZ,
	const float yawDegrees)
{
	COMBAT_CAPSULE result{};
	if (!Is_Valid(localCapsule) || !IsFinite(rootX) || !IsFinite(rootY) ||
		!IsFinite(rootZ) || !IsFinite(yawDegrees))
	{
		return result;
	}
	constexpr float DEGREES_TO_RADIANS = 0.01745329251994329577f;
	const float radians = yawDegrees * DEGREES_TO_RADIANS;
	const float sine = std::sin(radians);
	const float cosine = std::cos(radians);
	auto transformPoint = [&](const float x, const float y, const float z,
		float& outX, float& outY, float& outZ)
	{
		outX = rootX + x * cosine + z * sine;
		outY = rootY + y;
		outZ = rootZ - x * sine + z * cosine;
	};
	transformPoint(localCapsule.fStartX, localCapsule.fStartY,
		localCapsule.fStartZ, result.fStartX, result.fStartY, result.fStartZ);
	transformPoint(localCapsule.fEndX, localCapsule.fEndY,
		localCapsule.fEndZ, result.fEndX, result.fEndY, result.fEndZ);
	result.fRadius = localCapsule.fRadius;
	return result;
}

LostArk::Shared::CombatCollision::COMBAT_AABB
LostArk::Shared::CombatCollision::Build_Aabb(const COMBAT_CAPSULE& capsule)
{
	COMBAT_AABB bounds{};
	if (!Is_Valid(capsule))
		return bounds;
	bounds.fMinimumX = (std::min)(capsule.fStartX, capsule.fEndX) - capsule.fRadius;
	bounds.fMinimumY = (std::min)(capsule.fStartY, capsule.fEndY) - capsule.fRadius;
	bounds.fMinimumZ = (std::min)(capsule.fStartZ, capsule.fEndZ) - capsule.fRadius;
	bounds.fMaximumX = (std::max)(capsule.fStartX, capsule.fEndX) + capsule.fRadius;
	bounds.fMaximumY = (std::max)(capsule.fStartY, capsule.fEndY) + capsule.fRadius;
	bounds.fMaximumZ = (std::max)(capsule.fStartZ, capsule.fEndZ) + capsule.fRadius;
	return bounds;
}

bool LostArk::Shared::CombatCollision::Overlaps(
	const COMBAT_AABB& left,
	const COMBAT_AABB& right)
{
	return left.fMinimumX <= right.fMaximumX + CONTACT_EPSILON &&
		left.fMaximumX + CONTACT_EPSILON >= right.fMinimumX &&
		left.fMinimumY <= right.fMaximumY + CONTACT_EPSILON &&
		left.fMaximumY + CONTACT_EPSILON >= right.fMinimumY &&
		left.fMinimumZ <= right.fMaximumZ + CONTACT_EPSILON &&
		left.fMaximumZ + CONTACT_EPSILON >= right.fMinimumZ;
}

float LostArk::Shared::CombatCollision::DistanceSquared(
	const COMBAT_CAPSULE& left,
	const COMBAT_CAPSULE& right)
{
	if (!Is_Valid(left) || !Is_Valid(right))
		return (std::numeric_limits<float>::infinity)();
	const VECTOR3 p1 = StartOf(left);
	const VECTOR3 q1 = EndOf(left);
	const VECTOR3 p2 = StartOf(right);
	const VECTOR3 q2 = EndOf(right);
	const VECTOR3 d1 = Subtract(q1, p1);
	const VECTOR3 d2 = Subtract(q2, p2);
	const VECTOR3 r = Subtract(p1, p2);
	const float a = Dot(d1, d1);
	const float e = Dot(d2, d2);
	const float f = Dot(d2, r);
	float s = 0.f;
	float t = 0.f;
	const float epsilonSquared = CONTACT_EPSILON * CONTACT_EPSILON;
	if (a <= epsilonSquared && e <= epsilonSquared)
		return LengthSquared(r);
	if (a <= epsilonSquared)
	{
		t = Clamp01(f / e);
	}
	else
	{
		const float c = Dot(d1, r);
		if (e <= epsilonSquared)
		{
			s = Clamp01(-c / a);
		}
		else
		{
			const float b = Dot(d1, d2);
			const float denominator = a * e - b * b;
			if (std::abs(denominator) > epsilonSquared)
				s = Clamp01((b * f - c * e) / denominator);
			t = (b * s + f) / e;
			if (t < 0.f)
			{
				t = 0.f;
				s = Clamp01(-c / a);
			}
			else if (t > 1.f)
			{
				t = 1.f;
				s = Clamp01((b - c) / a);
			}
		}
	}
	const VECTOR3 closestLeft = AddScaled(p1, d1, s);
	const VECTOR3 closestRight = AddScaled(p2, d2, t);
	return LengthSquared(Subtract(closestLeft, closestRight));
}

bool LostArk::Shared::CombatCollision::Intersects(
	const COMBAT_CAPSULE& left,
	const COMBAT_CAPSULE& right)
{
	if (!Is_Valid(left) || !Is_Valid(right))
		return false;
	if (!Overlaps(Build_Aabb(left), Build_Aabb(right)))
		return false;
	const float combinedRadius = left.fRadius + right.fRadius;
	const float inclusiveRadius = combinedRadius + CONTACT_EPSILON;
	return DistanceSquared(left, right) <=
		inclusiveRadius * inclusiveRadius;
}
```

`Is_Valid(COMBAT_LOCAL_HIT_VOLUME)`은 kind별 exact union validation을 한다. `LEGACY_XZ_RANGE`는 finite
`0 < range <= MAXIMUM_RADIUS`와 zero/default capsule을 요구하고, `CAPSULE`은 range 0과 valid capsule을 요구한다.
두 payload가 동시에 채워진 ambiguous row는 실패한다. squared distance와 비교할 때는 길이 epsilon을 제곱식 밖에
더하지 않고 반드시 `(radiusSum + CONTACT_EPSILON)^2`를 사용한다.

### 검증

별도 collision harness project를 만들지 않는다. `Server/Private/ServerGameplayContractTests.cpp`의 pure geometry
section이 Shared 함수만 직접 호출해 GEO01~GEO11을 실행한다. Server/Engine type을 geometry 입력으로 쓰지 않으며
Shared Debug/Release build 뒤 Server `--contract-test` Debug/Release에서 같은 case를 고정한다.

### 새 파일 `Shared/Public/Network/NetworkTickContract.h` 전체 코드

```cpp
#pragma once

#include <cstdint>
#include <limits>

namespace LostArk::Shared
{
	[[nodiscard]] constexpr std::uint32_t NextNonZeroTick(
		const std::uint32_t tick)
	{
		return 0u == tick ||
			tick == (std::numeric_limits<std::uint32_t>::max)() ?
			1u : tick + 1u;
	}

	[[nodiscard]] constexpr std::uint32_t NextNonZeroCounter(
		const std::uint32_t value)
	{
		return NextNonZeroTick(value);
	}

	[[nodiscard]] constexpr bool Try_AdvanceTickSkippingZero(
		const std::uint32_t startTick,
		const std::uint32_t distance,
		std::uint32_t& outTick)
	{
		outTick = 0u;
		if (0u == startTick ||
			distance > static_cast<std::uint32_t>(
				(std::numeric_limits<std::int32_t>::max)()))
		{
			return false;
		}

		const std::uint64_t nonZeroDomain =
			static_cast<std::uint64_t>(
				(std::numeric_limits<std::uint32_t>::max)());
		const std::uint64_t zeroBased =
			static_cast<std::uint64_t>(startTick) - 1ull;
		outTick = static_cast<std::uint32_t>(
			((zeroBased + distance) % nonZeroDomain) + 1ull);
		return true;
	}

	[[nodiscard]] constexpr bool Try_GetForwardTickDistanceSkippingZero(
		const std::uint32_t startTick,
		const std::uint32_t currentTick,
		std::uint32_t& outDistance)
	{
		outDistance = 0u;
		if (0u == startTick || 0u == currentTick)
			return false;
		if (startTick == currentTick)
			return true;

		const std::uint32_t rawWrappedDistance = currentTick - startTick;
		if (rawWrappedDistance >
			static_cast<std::uint32_t>((std::numeric_limits<std::int32_t>::max)()))
		{
			return false;
		}

		outDistance = currentTick > startTick ?
			currentTick - startTick :
			(std::numeric_limits<std::uint32_t>::max)() - startTick + currentTick;
		return true;
	}
}
```

이 helper가 Server stage age/repeat ledger, Client active hit/6-tick ghost, 기존
`CActionPresentationTimeline`의 age 계산을 모두 대체한다. raw `current-start`, `current>=start`, 각 시스템의 별도
`NextTick`은 남기지 않는다. fixture는 advance `(MAX-1,+1)=MAX`, `(MAX-1,+2)=1`, `(MAX,+1)=1`과 distance
`(MAX-1 -> MAX)=1`, `(MAX-1 -> 1)=2`, `(MAX -> 1)=1`, `(1 -> MAX)=future/false`, zero input=false,
distance `INT32_MAX+1` advance=false를 고정한다.

소비 규칙은 구분한다. action/stage age 계산은 equal tick의 `true + distance 0`을 허용한다. 반면
`CClientReplication`의 snapshot ordering guard는 helper 성공뿐 아니라 `distance > 0`을 요구해 equal/duplicate
snapshot을 계속 거부한다. wrap forward snapshot은 위 skipping-zero distance가 positive일 때만 승인한다.

## G01. Gameplay authoring struct와 bootstrap row 계약

### `GameplayCatalog.h`에 추가할 enum/struct 전체

```cpp
enum class PLAYER_HIT_ANCHOR_POLICY : std::uint8_t
{
	ACTION_ROOT_AT_WINDOW_OPEN,
	END
};

enum class PLAYER_DAMAGE_APPLICATION_POLICY : std::uint8_t
{
	NONE,
	ONCE_PER_STAGE,
	PER_EVENT_REPEAT,
	END
};

struct PLAYER_HIT_VOLUME_EVENT final
{
	std::string strEventId;
	std::string strDamageProfileId;
	std::string strReactionProfileId;
	std::uint32_t iStartTickOffset = 0;
	std::uint32_t iEndTickOffset = 0;
	std::uint32_t iRepeatCount = 0;
	std::uint32_t iRepeatIntervalTicks = 0;
	std::uint32_t iMaximumTargets = 0;
	PLAYER_HIT_ANCHOR_POLICY eAnchorPolicy =
		PLAYER_HIT_ANCHOR_POLICY::END;
	LostArk::Shared::CombatCollision::COMBAT_LOCAL_HIT_VOLUME LocalVolume;
};

struct PLAYER_STAGE_DAMAGE_BUDGET final
{
	bool usesSkillDefaultDamageProfile = false;
	std::uint32_t iMaximumTargets = 0;
};

struct PLAYER_COMBAT_STAGE final
{
	std::uint32_t iStageIndex = 0;
	std::uint32_t iDurationMs = 0;
	std::uint32_t iInputOpenMs = 0;
	std::uint32_t iInputCloseMs = 0;
	bool hasInputWindow = false;
	PLAYER_DAMAGE_APPLICATION_POLICY eDamageApplicationPolicy =
		PLAYER_DAMAGE_APPLICATION_POLICY::END;
	PLAYER_STAGE_DAMAGE_BUDGET DamageBudget;
	std::vector<PLAYER_ROOT_MOTION_SAMPLE> RootMotion;
	std::vector<PLAYER_HIT_VOLUME_EVENT> HitEvents;
};

struct PLAYER_COMBAT_TIMELINE final
{
	std::string strTimelineId;
	LostArk::Shared::SKILL_ID iSkillId =
		LostArk::Shared::INVALID_SKILL_ID;
	std::vector<PLAYER_COMBAT_STAGE> Stages;
};
```

### Timeline authoring JSON formatVersion 1 exact schema

Publisher는 다음 exact-property set을 사용한다. 표에 없는 field는 모두 거부한다.

| node | exact fields |
|---|---|
| root | `schema`, `formatVersion`, `characterClass`, `fixedTickHz`, `timelines` |
| timeline | `timelineId`, `skillId`, `stages` |
| stage | `stageIndex`, `durationMs`, `inputWindow`, `rootMotion`, `damageApplicationPolicy`, `damageBudget`, `events` |
| input window object | `openMs`, `closeMs` |
| root-motion sample | `timeMs`, `forward`, `lateral` |
| common event | `eventId`, `kind`, `startMs`, `endMs`, `anchorPolicy`, `shape`, `repeatCount`, `repeatIntervalMs`, `reactionProfileId` |
| ONCE budget | `damageProfileSource`, `maximumTargets` |
| PER-event additions | event의 `damageProfileId`, `maximumTargets` |
| legacy shape | `kind`, `range` |
| capsule shape | `kind`, `localStart`, `localEnd`, `radius` |

root는 `schema="lostark.player-combat-timelines"`, `formatVersion=1`, `fixedTickHz=30`을 요구한다. `inputWindow`는
`null` 또는 exact object이며 inclusive `0 <= openMs <= closeMs <= durationMs`다. `rootMotion`은 empty 또는
strictly increasing `timeMs`이고 모든 sample time이 stage duration 안에 있어야 한다. vector는 finite float 3개,
root-motion forward/lateral은 finite float다.

damage union은 다음과 같다.

```json
{
  "damageApplicationPolicy": "NONE",
  "damageBudget": null,
  "events": []
}
```

```json
{
  "damageApplicationPolicy": "ONCE_PER_STAGE",
  "damageBudget": {
    "damageProfileSource": "SKILL_DEFAULT",
    "maximumTargets": 1
  },
  "events": [
    {
      "eventId": "hit.legacy",
      "kind": "HIT_VOLUME",
      "startMs": 300,
      "endMs": 334,
      "anchorPolicy": "ACTION_ROOT_AT_WINDOW_OPEN",
      "shape": { "kind": "LEGACY_XZ_RANGE", "range": 4.5 },
      "repeatCount": 1,
      "repeatIntervalMs": 0,
      "reactionProfileId": ""
    }
  ]
}
```

```json
{
  "damageApplicationPolicy": "PER_EVENT_REPEAT",
  "damageBudget": null,
  "events": [
    {
      "eventId": "hit.01",
      "kind": "HIT_VOLUME",
      "startMs": 250,
      "endMs": 284,
      "anchorPolicy": "ACTION_ROOT_AT_WINDOW_OPEN",
      "shape": {
        "kind": "CAPSULE",
        "localStart": [0.0, 0.9, 0.4],
        "localEnd": [0.0, 0.9, 2.4],
        "radius": 0.8
      },
      "repeatCount": 1,
      "repeatIntervalMs": 0,
      "damageProfileId": "damage.player.example.hit01",
      "maximumTargets": 4,
      "reactionProfileId": "reaction.monster.player-skill.light"
    }
  ]
}
```

`NONE`은 event 0개만 허용한다. `ONCE_PER_STAGE` event에는 `damageProfileId/maximumTargets`가 없어야 하고,
`PER_EVENT_REPEAT`의 모든 event에는 두 field가 있어야 한다. 모든 hit event는 `reactionProfileId` property를
가지며 빈 문자열은 damage-only, non-empty는 `ReactionProfiles.json`의 exact ID를 뜻한다. `eventId`는 timeline 안에서 unique, start/end는
`0 <= startMs < endMs <= durationMs`, anchor는 v1에서 `ACTION_ROOT_AT_WINDOW_OPEN`만 허용한다. repeat 1이면
interval 0, repeat가 2 이상이면 positive interval이고 양자화 뒤 window가 겹치거나 duration을 넘으면 거부한다.
shape는 legacy/capsule exact union이며 capsule과 legacy payload를 함께 둘 수 없다. schema v1에는 compound가 없다.

`PLAYER_SKILL_DEFINITION`에서는 다음 필드를 제거한다.

```cpp
std::uint32_t iActionDurationMs;
std::uint32_t iHitTimeMs;
float fMovementDistance;
float fMaximumRange;
std::vector<PLAYER_COMBO_STAGE> ComboStages;
std::vector<PLAYER_ROOT_MOTION_SAMPLE> RootMotion;
```

`PlayerSkills.json` formatVersion 3 root exact set은 `schema`, `formatVersion`, `skills`다. 각 skill row exact set은
다음 13개다.

```text
skillId, characterClass, inputSlot, displayName, actionId, skillKind,
cooldownMs, resourceCost, serverDamageProfileId, effectId,
requiredStance, setsStance, combatTimelineId
```

`effectId`와 stance/damage ID는 빈 문자열을 명시적으로 허용하되 property 생략/null은 허용하지 않는다.
`combatTimelineId`는 non-empty이고 six timeline document 중 정확히 하나를 참조해야 한다. 제거된 timing/range/
movement/combo property가 남거나 새 row가 timeline 없이 저장되면 publisher와 Balance Tool reload가 모두 실패한다.

다음 필드를 추가한다.

```cpp
std::string strCombatTimelineId;
```

현재 `strEffectId`/JSON `effectId`는 진행 중인 effect restoration의 presentation fallback이므로 이 collider
slice에서 제거하지 않는다. Server parser는 지금처럼 해당 field를 runtime damage definition으로 소비하지 않는다.
`strDamageProfileId`/JSON `serverDamageProfileId`도 current single-budget damage 정본과 Client HUD를 보존하기 위해
남긴다. `ONCE_PER_STAGE`는 이 skill default만 사용하며 timeline event가 같은 ID를 복제하지 않는다.
`PER_EVENT_REPEAT` stage만 event profile을 가진다. schema v1은 한 skill의 damage-bearing stage들이
`ONCE_PER_STAGE`와 `PER_EVENT_REPEAT`를 섞지 못하게 한다. PER-event skill은 default profile이 empty여야 하고 Client
HUD summary는 event profile 합계에서 derive한다. `NONE` stage는 event/budget이 모두 empty이며 guard/stance/no-damage
stage를 표현한다.

one-shot converter의 legacy damage-capable 분기는 다음 표가 정본이다. skill-level damage profile이 있다는 이유로
모든 stage에 event를 만들지 않는다.

| `skillKind` | damage-capable stage | `NONE` stage |
|---|---|---|
| `ACTIVE` | `serverDamageProfileId`가 non-empty인 stage 0 | profile-empty stage 0 |
| `COMBO` | 모든 stage | 없음 |
| `HOLD` | stage index 2 | stage index 0, 1 |
| `COUNTER` | stage index 1 | stage index 0 |

COMBO/HOLD/COUNTER의 profile이 비어 있거나 실제 stage count가 위 계약과 다르면 migration을 중단한다. `NONE`은
`damagePolicy=NONE`, empty budget, `hitEvents=[]`이며 legacy first-fire 값도 `null`이다. G8 admission 전에 frozen
v2 source를 pure legacy로 변환한 118-stage converter golden은 각 row를
`optional<uint32_t> firstFireOffset`, damage policy, event count의 tuple로 비교한다. 이로써 HOLD charge와 COUNTER
guard가 damage event를 얻는 회귀를 막는다. final product graph는 reviewed allowlist stage를 이 tuple 비교에서
제외하고 별도 trace/capsule golden으로 검증한다.

`PLAYER_RUNTIME_PROFILE`, `BOSS_RUNTIME_PROFILE`, monster spawn profile에는 다음 필드를 추가한다.

```cpp
LostArk::Shared::CombatCollision::COMBAT_BODY_CAPSULE CombatBodyCapsule;
```

`CGameplayCatalog`에는 다음 lookup을 추가한다.

```cpp
const PLAYER_COMBAT_TIMELINE* Find_CombatTimeline(
	std::string_view timelineId) const;
const PLAYER_COMBAT_TIMELINE* Find_CombatTimeline(
	LostArk::Shared::SKILL_ID skillId) const;
```

두 lookup은 서로 다른 definition 복사본을 만들지 않는다. `timelineId` map이 storage이고 skill map은 pointer가
아닌 timeline ID를 보조 index로 가진다. load commit 뒤 reference invalidation이 없도록 immutable catalog 전체를
한 번에 교체한다.

catalog와 publisher는 Shared public 상한을 동일하게 검사한다.

```text
stages per skill <= 8
hit events per stage <= 16
repeats per event <= 8
targets per repeat <= 16
ONCE_PER_STAGE: eventCount * repeatCount * stage.maximumTargets <= 2048
PER_EVENT_REPEAT: sum(events.repeatCount * events.maximumTargets) <= 2048 per stage
```

상한을 넘는 문서는 stage하지 않는다.

### Bootstrap v5 row

```text
LOSTARK_GAMEPLAY_BOOTSTRAP\t5\t<RUNTIME_REVISION>\t<ROW_COUNT>
PLAYER\t<CLASS>\t<HP>\t<RESOURCE>\t<REGEN>\t<ATTACK>\t<DEFENSE>\t<MOVE>\t<STANCE>\t<RADIUS>\t<CYLINDER_HALF_HEIGHT>\t<CENTER_OFFSET_Y>\t<RECEIVED_KNOCKBACK_SCALE>\t<REACTION_IMMUNITY>
BOSS\t<ARCHETYPE>\t<ENCOUNTER>\t<HP>\t<BARS>\t<ATTACK>\t<RADIUS>\t<CYLINDER_HALF_HEIGHT>\t<CENTER_OFFSET_Y>\t<ENGAGE>\t<MOVE>\t<PHASE2_PERCENT>\t<RECEIVED_KNOCKBACK_SCALE>\t<REACTION_IMMUNITY>
SKILL\t<SKILL_ID>\t<CLASS>\t<INPUT_SLOT>\t<ACTION_ID>\t<DAMAGE_PROFILE_OR_EMPTY>\t<COOLDOWN_MS>\t<RESOURCE_COST>\t<SKILL_KIND>\t<REQUIRED_STANCE>\t<SETS_STANCE>\t<TIMELINE_ID>
TIMELINE\t<TIMELINE_ID>\t<SKILL_ID>\t<STAGE_COUNT>
STAGE\t<TIMELINE_ID>\t<STAGE_INDEX>\t<DURATION_MS>\t<HAS_INPUT_WINDOW>\t<INPUT_OPEN_MS>\t<INPUT_CLOSE_MS>\t<DAMAGE_POLICY>\t<USES_SKILL_DEFAULT_DAMAGE>\t<STAGE_MAX_TARGETS>\t<HIT_EVENT_COUNT>
ROOTMOTION\t<TIMELINE_ID>\t<STAGE_INDEX>\t<SAMPLE_INDEX>\t<TIME_MS>\t<FORWARD>\t<LATERAL>
HITEVENT\t<TIMELINE_ID>\t<STAGE_INDEX>\t<EVENT_ID>\t<START_OFFSET>\t<END_OFFSET>\t<REPEAT_COUNT>\t<REPEAT_INTERVAL_TICKS>\t<ANCHOR_POLICY>\t<SHAPE_KIND>\t<LEGACY_XZ_RANGE>\t<START_X>\t<START_Y>\t<START_Z>\t<END_X>\t<END_Y>\t<END_Z>\t<CAPSULE_RADIUS>\t<EVENT_MAX_TARGETS>\t<EVENT_DAMAGE_PROFILE_ID>\t<REACTION_PROFILE_ID_OR_EMPTY>
```

`ONCE_PER_STAGE`는 `SKILL` row의 damage profile과 `STAGE` row의 target cap을 적용한다. 해당 `HITEVENT` row의
event target/profile 칸은 empty/0이어야 한다. `PER_EVENT_REPEAT`는 반대로 `SKILL` damage profile과 stage cap이
empty/0이고 각 `HITEVENT`가 target cap/profile을 소유한다. `NONE`은 stage cap/event count가 0이다. publisher와
catalog가 이 exclusive union과 skill-level mixed-policy 금지를 검증한다.

### Tick 양자화 함수

publisher와 Server fixture가 같은 식을 사용한다.

```powershell
function Convert-MillisecondsToEvaluationOffset([uint32]$Milliseconds) {
    # Mirrors static_cast<float>(std::chrono::duration<double>{1.0 / 30.0}.count()).
    [single]$fixedDelta = [single](1.0 / 30.0)
    [single]$threshold = [single](
        ([single]$Milliseconds) * ([single]0.001))
    [single]$elapsed = [single]0
    for ([uint32]$offset = 0; $offset -lt 1000000; $offset++) {
        # The explicit cast rounds after every add exactly where C++ float does.
        $elapsed = [single]($elapsed + $fixedDelta)
        if ($elapsed -ge $threshold) { return $offset }
    }
    throw "Legacy float32 evaluation offset exceeded the public bound."
}

function Convert-MillisecondsToExclusiveEndOffset(
    [uint32]$Milliseconds,
    [uint32]$StartOffset
) {
    $candidate = Convert-MillisecondsToEvaluationOffset $Milliseconds
    $minimum = [uint64]$StartOffset + [uint64]1
    if ([uint64]$candidate -lt $minimum) { return [uint32]$minimum }
    return [uint32]$candidate
}

function Convert-MillisecondsToIntervalTicks([uint32]$Milliseconds) {
    if ([uint32]0 -eq $Milliseconds) { return [uint32]0 }
    return [uint32][Math]::Ceiling(([uint64]$Milliseconds * 30.0) / 1000.0)
}
```

end tick은 exclusive다. repeat ordinal `n`의 window는
`[start + n * interval, end + n * interval)`이다.

첫 accepted action은 `actionStartTick`과 같은 room Update에서 age 0으로 평가된다. 기존 코드는 그 Update에서
`float fActionElapsedSeconds += 1.f/30.f`를 먼저 수행하고 float threshold와 비교한다. float 누적 rounding 때문에
단순 `ceil(ms*30/1000)-1`은 1500/1600/1700/2200 ms 같은 current stage에서 한 tick 어긋난다. 위 converter는
step, threshold multiplication, 매 add를 모두 float32로 round한다. 대응 C++ fixture는 current Server와 같은
`static_cast<float>(std::chrono::duration<double>{1.0 / 30.0}.count())`로 step을 만들고, 그 step의 IEEE-754 bit
pattern부터 frozen pre-admission 118 stage의 `optional<first-fire offset>`까지 PowerShell 결과와 byte-for-byte
비교한다. `NONE` stage는 converter를 호출하지 않고 null을 기록한다.

`repeatCount=1`은 `repeatIntervalMs=0`/interval 0만 허용한다. `repeatCount>1`은 positive interval을 duration
delta 식으로 양자화하고 `intervalTicks >= endOffset - startOffset`을 요구해 repeat window overlap을 거부한다.
`stageEvaluationTickCount=Convert-MillisecondsToEvaluationOffset(durationMs)+1`로 두고 마지막 repeat의 exclusive
end가 이 count를 넘으면 실패한다. duration threshold를 처음 통과한 final evaluation tick도 current code처럼
damage를 먼저 평가한 뒤 stage를 끝낸다.

stage duration/input window와 root-motion sample은 authoring ms를 그대로 bootstrap에 보존한다. 기존
`fActionElapsedSeconds`, inclusive input-window 비교, `Sample_RootMotion` interpolation을 유지하므로 hit window처럼
tick으로 반올림하지 않는다.

### Exact legacy range conversion

위 표에서 damage-capable인 current single-range damage row만 3D point capsule로 바꾸지 않고 다음 explicit
shape로 변환한다. `NONE` row에는 아래 event 자체를 만들지 않는다.

```text
shape.kind = LEGACY_XZ_RANGE
shape.range = legacy maximumRange
startOffset = Convert-MillisecondsToEvaluationOffset(legacy hitTimeMs)
endOffset = startOffset + 1
repeatCount = 1
maximumTargets = 1
damageApplicationPolicy = ONCE_PER_STAGE
damageProfileSource = SKILL_DEFAULT
```

Server branch는 caster root와 target root의 XZ squared distance를 `maximumRange + targetBody.radius`의 제곱과
비교한다. Y, capsule segment height를 사용하지 않으므로 tall Valtan/Lugaru까지 기존 target set과 수학적으로 같다.
별도 damage loop가 아니라 `CCombatCollisionSystem::Query_WorldTargets` 안의 shape branch다. vertical separation은
reviewed `CAPSULE` event에만 적용한다.

### Root-motion source migration

converter는 다음 네 current source를 읽어 대응 class timeline `rootMotion` array로 합친다.

```text
Data/Animation/RootMotion/Artist.rootmotion.json
Data/Animation/RootMotion/DimensionMaster.rootmotion.json
Data/Animation/RootMotion/LanceMaster.rootmotion.json
Data/Animation/RootMotion/Warlord.rootmotion.json
```

skill/stage/sample order와 `timeMs/forward/lateral` 값을 보존하고, timeline commit과 같은 변경에서 네 old source를
삭제한다. publisher는 old root-motion file과 timeline `rootMotion`이 동시에 있으면 duplicate authority로 실패한다.
Gunslinger/Slayer는 complete timeline을 가지되 실제 source sample이 없으면 empty array를 명시한다.

stage에 authored sample이 없고 legacy `movementDistance>0`이면 converter가 다음 두 sample을 만든다.

```text
(timeMs=0, forward=0, lateral=0)
(timeMs=stage.durationMs, forward=legacy movementDistance, lateral=0)
```

이는 current linear fallback `movementDistance / durationSeconds * fixedDeltaSeconds`와 같은 누적 이동을 만들며,
30 Hz step별 root position parity fixture가 통과해야 legacy field를 제거한다. authored sample이 있으면 이 synthetic
pair를 추가하지 않는다.

### Legacy field consumer migration

`PlayerSkills.json`에서 timing/combo field를 제거하기 전에 아래 소비자를 한 inventory로 묶어 모두 이관한다.

| 소비자 | 현재 직접 읽는 값 | 변경 후 source |
|---|---|---|
| `CPlayerSkillCatalog` | `comboStages` count | `combatTimelineId -> stages.size()` |
| `BalanceTool` | duration/hit/range/move/combo edit/save | 제거, timeline summary read-only |
| `Animation_Tool`/binding validation | catalog의 combo count | catalog가 join한 timeline stage count |
| Effect Tool | catalog skill labels/stage count | 변경된 catalog 그대로 소비 |
| Python effect component/materializer | raw `comboStages` | six timeline parser의 stage rows |
| provenance export/update | legacy field path | Data/Combat timeline field path |
| ProjectAudit/EffectToolFinal | raw stage array | timeline join 결과 |

`Client::PLAYER_SKILL_DEFINITION`에는 다음 필드를 추가한다.

```cpp
std::string strCombatTimelineId;
std::size_t iComboStageCount = 0;
```

`CPlayerSkillCatalog::Load`는 다음 staged graph를 한 번에 읽는다.

```text
DamageProfiles.json
PlayerSkills.json formatVersion 3
Data/Combat/Timelines/Player six class documents formatVersion 1
```

parse 순서는 `damage -> timeline documents -> skill rows -> reference validation -> staged vector commit`이다.
timeline의 class/skill ID가 skill row와 다르거나 duplicate/missing이면 기존 `g_Skills`를 유지하고 false를 반환한다.
ACTIVE는 stage 1개, COMBO는 2~8, HOLD는 3, COUNTER는 contract가 요구하는 stage 수를 검증한다. 저장 문서에는
`iComboStageCount`를 다시 쓰지 않는다.

기존 `iDamageRatePercent`는 ONCE-mode skill에서 `serverDamageProfileId`를 resolve한다. PER-event-mode skill은
default profile이 empty인 대신 모든 damaging event profile의 rate를 checked-add해 HUD summary를 만든다. overflow,
mixed damage mode, unknown event profile이면 staged catalog 전체를 commit하지 않는다. 이 summary는 Client UI용이며
Server damage 횟수/target 판정에 사용하지 않는다.

`BalanceTool::SKILL_EDIT`에서는 다음 member와 대응 UI/validation/writer를 제거한다.

```cpp
std::uint32_t actionDurationMs;
std::uint32_t hitTimeMs;
float movementDistance;
float maximumRange;
std::vector<COMBO_STAGE_EDIT> comboStages;
```

다음을 추가한다.

```cpp
std::string combatTimelineId;
std::uint32_t timelineStageCount = 0;
```

Balance Save는 `combatTimelineId`를 그대로 보존하고 timeline 문서를 쓰지 않는다. collider/timing 변경은 이
수직 슬라이스에서 JSON authoring + publisher로만 수행하며, Animation Tool Combat Timeline editor는 별도 UI
slice가 닫히기 전까지 read-only summary다.

Python pipeline에는 공용 helper
`Tools/GameplayPipeline/player_combat_timelines.py`를 추가한다. helper는 six file을 exact schema로 load하고
`skillId -> ordered stages` immutable map을 반환한다. effect pipeline script마다 간이 parser를 복제하지 않는다.
대응 Python test는 duplicate/missing/wrong-class/stage-gap을 고정한다.

## G02. Actor profile data 변경

### Player profile row

여섯 class 모두 다음 exact baseline을 가진다.

```json
{
"combatBodyCapsule": {
  "radius": 0.45,
  "cylinderHalfHeight": 0.45,
  "centerOffsetY": 0.9
}
}
```

### Monster rows

```json
{
  "MONSTER_VALTAN_PADD_01": [0.55, 0.55, 1.10],
  "MONSTER_VALTAN_SJFC_00_4": [0.60, 0.60, 1.20],
  "MONSTER_VALTAN_0019_05": [0.65, 0.65, 1.30],
  "MINIBOSS_LUGARU": [1.35, 1.35, 2.70]
}
```

배열 표기는 이 계획서의 `(radius, cylinderHalfHeight, centerOffsetY)` 요약이며 실제 profile JSON에는 named
`combatBodyCapsule` object로 저장한다.

### Valtan row

```json
{
"combatBodyCapsule": {
  "radius": 3.0,
  "cylinderHalfHeight": 3.0,
  "centerOffsetY": 6.0
}
}
```

기존 top-level `collisionRadius`는 같은 의미를 중복 소유하므로 schema bump와 함께 제거한다. monster AI의
attack reach는 `CombatBodyCapsule.fRadius`, 기존 boss/player skill reach code는 새 overlap 결과를 소비한다.
provenance receipt는 publisher의 field sync 단계에서 세 새 field를 `PROJECT_TUNED`로 기록한다.

### Balance Tool profile schema migration

`PlayerProfiles.json`은 formatVersion 3, `MonsterProfiles.json`은 formatVersion 2,
`BossProfiles.json`은 formatVersion 4로 올린다. monster/boss top-level `collisionRadius`는 새 exact
`combatBodyCapsule` object로 이관한 뒤 제거한다. 같은 G에서
`CBalanceTool::PLAYER_EDIT`와 `BOSS_EDIT`에 다음 값을 추가하고, reload의 exact-property set, staged read,
finite/positive validation, writer를 모두 바꾼다.

```cpp
float combatBodyRadius = 0.f;
float combatBodyCylinderHalfHeight = 0.f;
float combatBodyCenterOffsetY = 0.f;
```

player/boss row의 `combatBodyCapsule`은 정확히 `radius`, `cylinderHalfHeight`, `centerOffsetY` 세 field만 허용한다.
`BOSS_EDIT::collisionRadius`와 기존 `Collision radius` writer/UI는 제거하고 위 capsule radius를 표시한다. 나머지
두 필드도 같은 profile object 안에서 편집하거나 최소한 read-only로 표시하면서 writer가 원값을 그대로 보존해야
한다. Player row 역시 새 object를 모르는 exact parser 때문에 reload가 실패하거나 Save가 body를 소실하지 않도록
동시에 이관한다. reload-save-reload harness는 여섯 player와 Valtan의 세 값, formatVersion, JSON exact set이
동일함을 검사하고 partial object, extra field, NaN/zero/negative 값에서 기존 `m_players`/`m_bosses`를 유지하며
Save를 차단한다.

이 삭제와 같은 G에서 `Server/Private/MonsterBrain.cpp`의 chase/attack reach, `GameRoom.cpp` spawn copy,
`ServerGameplayContractTests.cpp` fixture를 모두 `CombatBodyCapsule.fRadius`로 이관한다. 기존 horizontal monster AI
결과가 달라지면 schema migration 실패로 본다. `SERVER_WORLD_ENTITY::fCollisionRadius`만 먼저 삭제해 build를 깨는
중간 commit은 만들지 않는다.

### Spawn bootstrap v2 profile row

```text
LOSTARK_SPAWN_GROUP_BOOTSTRAP\t2\t<WORLD_ID>\t<AREA_ID>\t<DOCUMENT_REVISION>\t<RUNTIME_REVISION>\t<ANCHOR_COUNT>\t<GROUP_COUNT>\t<PROFILE_COUNT>
PROFILE\t<ARCHETYPE_ID>\t<HP>\t<ATTACK>\t<DEFENSE>\t<RADIUS>\t<CYLINDER_HALF_HEIGHT>\t<CENTER_OFFSET_Y>\t<ENGAGE>\t<MOVE>\t<ATTACK_RANGE>\t<WINDUP_MS>\t<ACTIVE_MS>\t<RECOVERY_MS>\t<DEAD_DESPAWN_MS>\t<RECEIVED_KNOCKBACK_SCALE>\t<REACTION_IMMUNITY>\t<ATTACK_HIT_EVENT_ID>\t<ATTACK_REACTION_PROFILE_ID_OR_EMPTY>
```

v1의 `WORLD_ID/AREA_ID/DOCUMENT_REVISION/ANCHOR_COUNT/GROUP_COUNT/PROFILE_COUNT` 순서는 유지하고
`RUNTIME_REVISION`만 document revision 뒤에 추가한다. world bootstrap도 v6 field를 재배치하지 않고 runtime
revision 하나만 추가해 formatVersion 7을 사용한다.

```text
LOSTARK_WORLD_BOOTSTRAP\t7\t<WORLD_ID>\t<AREA_ID>\t<DOCUMENT_REVISION>\t<RUNTIME_REVISION>\t<ROW_COUNT>
```

### Runtime-set atomic publish

`Publish-BalanceRuntimeSet.ps1`을 product publish의 유일한 entry로 사용한다. staged promote target은 다음 열다섯
파일이다. Bern은 현재 navigation publisher 대상이 아니므로 존재하지 않는 Bern nav artifact를 invent하지 않는다.

```text
Server/Bin/DataFiles/Gameplay/Gameplay.bootstrap
Server/Bin/DataFiles/World/BERN.worldbootstrap
Server/Bin/DataFiles/World/VALTAN_ARENA.worldbootstrap
Server/Bin/DataFiles/World/VALTAN_ARENA.spawngroupsbootstrap
Server/Bin/DataFiles/World/CHARACTER_SELECT_ARENA.spawngroupsbootstrap
Server/Bin/DataFiles/World/TRAINING_GROUND.worldbootstrap
Server/Bin/DataFiles/World/CHARACTER_SELECT_ARENA.worldbootstrap
Server/Bin/DataFiles/Navigation/LV_LUT_HEARTRB_ED.navgrid
Server/Bin/DataFiles/Navigation/LV_LOBBY_CLASSSELECT_SL00.navgrid
Server/Bin/DataFiles/Navigation/LV_DEV_TRAINING_GROUND.navgrid
Client/Bin/DataFiles/Navigation/LV_LUT_HEARTRB_ED.navgrid
Client/Bin/DataFiles/Navigation/LV_LOBBY_CLASSSELECT_SL00.navgrid
Client/Bin/DataFiles/Navigation/LV_DEV_TRAINING_GROUND.navgrid
Client/Bin/DataFiles/Gameplay/CombatColliders.debug.json
Server/Bin/DataFiles/RuntimeSet.balance.manifest
```

script parameter는 repository-relative `ServerOutputRoot='Server/Bin/DataFiles'`와
`ClientOutputRoot='Client/Bin/DataFiles'`를 분리해 받는다. 둘 다 resolved path가 repository 안인지 검사한다.
failure injection/rollback 목록에는 두 root의 destination을 같은 ordered transaction으로 넣는다.

현재 world count를 코드에 암묵적으로 복제하지 않고 publisher가 반환한 staged manifest를 orchestrator가 읽는다.
각 gameplay/world/navigation publisher는 artifact뿐 아니라 exact
`CANONICAL_INPUT_RECEIPT{relativePath,schemaId,formatVersion,sha256}` row를 반환한다. orchestrator는 canonical relative path로
union하며 동일 path의 `(schemaId,formatVersion,sha256)`가 모두 byte-identical이면 한 row로 deduplicate하고 하나라도
다르면 conflicting dependency로 실패한다. 그 뒤 unexpected/missing required path를 거부하고 relative path ordinal
sort로 `(path, schema, version, source hash)`를
직렬화한다. `combatRuntimeRevision`은 이 receipt bytes의 SHA-256로 먼저 계산해 self-hash cycle을 만들지 않는다.

required receipt inventory는 Balance 5문서와 official provenance receipt, final player skill timeline 여섯 문서,
`ValtanEncounter.json`, `ReactionProfiles.json`, `ValtanHitTracks.json`과 그 source receipt인
`Valtan.weapontracks.json`, actor catalog 네 문서(Character/Boss/Npc/Monster),
네 Area `Gameplay.world.json`, Valtan/Character Select 두 `SpawnGroups.world.json`, 그리고 navigation publisher의 Valtan
source+paint+blockers, Character Select source+paint, Training Ground uniform grid JSON이다. G01에서 timeline으로 흡수 후
삭제되는 RootMotion 네 파일은 final receipt에 넣지 않는다. dependency table에 없는 optional/reference file을 revision
input에 암묵적으로 섞지 않는다. manifest에는 그 full lowercase-hex
revision, role, output-root-relative destination, 최종 artifact SHA-256가 deterministic order로 들어간다. manifest는
Server가 읽기 쉬운 line-based generated runtime contract이며 JSON이 아니다.

```text
LOSTARK_BALANCE_RUNTIME_SET_MANIFEST\t1\t<RUNTIME_REVISION>\t<ARTIFACT_COUNT>
ARTIFACT\tSERVER_REQUIRED\t<Gameplay/Gameplay.bootstrap>\t<SHA256>
ARTIFACT\tSERVER_REQUIRED\t<World/BERN.worldbootstrap>\t<SHA256>
...
ARTIFACT\tCLIENT_RUNTIME\t<Navigation/LV_LOBBY_CLASSSELECT_SL00.navgrid>\t<SHA256>
ARTIFACT\tCLIENT_DEBUG\t<Gameplay/CombatColliders.debug.json>\t<SHA256>
```

promotion 전에 모든 staged file을 strict reload하고 revision/hash를 비교한다. manifest는 마지막 단일 replace로
promote한다. promotion index 1부터 마지막까지 failure injection을 반복해 reverse-order rollback을 검증한다.
기존 target list에서 누락된 Valtan spawn-group bootstrap, 새 Character Select Arena spawn-group bootstrap과 Server/Client
navigation grid 세 쌍을 반드시 포함한다. navigation publisher는 이 orchestration에서는 destination을 직접 replace하지
않고 여섯 staged artifact와 receipt만 반환한다. Bern/Training expected-absent spawn artifact tombstone은 manifest artifact
row가 아니지만 ordered mutation/rollback slot에 각각 들어간다. 정확한 mutation 순서는 14 artifact promote, 최대 2
tombstone, manifest-last 1회다. catchable failure injection은 최대 17 mutation index의 reverse rollback을 순회한다.
별도 child-process kill은 artifact/tombstone/manifest-last 각 경계에서 강제 종료해 old manifest + mixed output이 Server
startup에서 fail closed되는지 확인한다. durable publisher journal을 이번 slice에 암묵 도입하거나 old set 자동 복구를
주장하지 않는다. 다음 정상 orchestrator 실행은 canonical input에서 모든 artifact를 다시 stage/promote하고 manifest를
마지막에 교체해 한 complete new set으로 heal한다.

`CServerApp::Run`은 room과 socket을 만들기 전에 manifest를 strict parse한다. exact header/row count, role,
lowercase 64-hex revision/hash, duplicate destination, absolute/drive-qualified/`..` path를 검증하고, Server DataFiles
root 아래 정확히 열 개의 `SERVER_REQUIRED` artifact bytes를 Windows CNG SHA-256로 대조한다. `CLIENT_RUNTIME` 세
row와 `CLIENT_DEBUG` 한 row는 문법, role별 exact count와 destination uniqueness inventory에는 포함하지만 Server가
Client root 파일을 열지는 않는다. old manifest + partially promoted new
artifact이면 hash/revision 검증이 실패하므로 process crash가 manifest-last 직전에 발생해도 room ready가 되지
않는다. manifest 검증이 성공한 뒤에만 승인 revision을 모든 `CGameRoom` 생성자에 주입한다.

orchestrator는 same 64-hex revision을 gameplay/world/spawn-group bootstrap header와 Client debug JSON에 stamp한다.
`CGameplayCatalog`, `CWorldBootstrap`, `CSpawnGroupBootstrap` loader는 자신의 header revision을 expose하고 각 room은
이를 manifest 승인 revision에 직접 비교한다. gameplay/world는 항상 필수다. Valtan과 Character Select Arena는
spawn-group artifact가 필수다. 현재 exact manifest inventory에서 Bern/Training Ground는
`Has_RuntimeArtifact()==false`만 정상이다. 예상 밖 Bern/Training spawn-group 파일은 hash inventory 밖 입력이므로
revision이 같아도 거부한다. mismatch, Valtan/Character Select artifact 부재, Bern/Training unexpected artifact면
room ready를 거부하고 Join packet을 보내지 않는다.

`Server.vcxproj`의 pre-build는 기존 separate `Publish-GameplayBalance.ps1`, `Publish-WorldGameplay.ps1`,
`Publish-ServerNavigation.ps1 -Mode Publish` 세 호출을 모두 제거하고 `Publish-BalanceRuntimeSet.ps1 -Mode Publish` 한 번만
호출한다. individual publisher는 orchestrator가 repository-relative staging root를 넘긴 staged-emitter mode에서만
사용하고 product destination을 직접 replace하지 않는다. Project XML audit은 orchestrator 외 `-Mode Publish` literal이
남아 있으면 실패한다.

`Publish-BalanceRuntimeSet.ps1 -Mode Validate`는 동일 staged graph를 만들고 strict reload까지만 수행하며 destination
mtime/hash를 바꾸지 않는다. 새 `Test-BalanceRuntimeSetRollback.ps1`은 temporary destination에서 각 catchable promotion
index 실패의 이전 revision 복구와 child-process kill 뒤 Server fail-closed/full republish heal을 분리해 확인한다.

## G03. Server runtime state

### 새 파일 `Server/Public/BalanceRuntimeSetManifest.h` 전체 public contract

```cpp
#pragma once

#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace LostArk::Server
{
	class CBalanceRuntimeSetManifest final
	{
	public:
		[[nodiscard]] bool Load_AndVerify(
			const std::filesystem::path& serverDataRoot);
		[[nodiscard]] const std::string& Get_RuntimeRevision() const
		{
			return m_strRuntimeRevision;
		}
		[[nodiscard]] const std::string& Get_Status() const
		{
			return m_strStatus;
		}
		[[nodiscard]] bool Contains_ServerRequiredArtifact(
			std::string_view relativeDestination) const;

	private:
		struct ARTIFACT final
		{
			std::string strRole;
			std::string strRelativeDestination;
			std::string strSha256;
		};

		std::string m_strRuntimeRevision;
		std::vector<ARTIFACT> m_Artifacts;
		std::string m_strStatus;
	};
}
```

`Server/Private/BalanceRuntimeSetManifest.cpp`는 다음 순서를 하나의 staged load로 구현한다.

1. `<serverDataRoot>/RuntimeSet.balance.manifest`를 binary/text exact line parser로 읽는다.
2. header version/count와 각 4-column row를 검증하고 blank/short/trailing row를 거부한다.
3. destination이 forward-slash relative path인지, absolute/drive/`.`/`..`가 없는지 검사한다. uniqueness key는
   `(resolved role root, relative destination)`이다. Server와 Client root의 같은 `Navigation/...` path는 허용하지만
   같은 role root 안 duplicate는 거부한다.
4. role은 `SERVER_REQUIRED`, `CLIENT_RUNTIME`, `CLIENT_DEBUG`만 허용하고 revision/hash는 lowercase 64-hex만 허용한다.
5. `SERVER_REQUIRED` destination을 canonical Server DataFiles root 아래로 resolve하고 Windows CNG
   `BCRYPT_SHA256_ALGORITHM`으로 실제 bytes를 hash한다. `bcrypt.lib`를 `Server.vcxproj`에 명시한다.
6. gameplay 1개, 지원 world 4개, Valtan/Character Select spawn-group 2개, Server navigation 3개,
   Client navigation 3개와 Client debug 1개의 exact inventory를 검사한다.
7. 모든 검증 뒤에만 revision/artifact vector/status를 member에 commit한다. 실패하면 이전 member를 유지한다.

`CServerApp::Run`은 이 loader를 room 생성보다 먼저 실행하고 실패하면 listener를 열지 않은 채 exit code 1로
종료한다. 성공하면 `CGameRoom(worldId, manifest.Get_RuntimeRevision())`으로 승인 revision을 주입한다. room은
gameplay/world header를 항상 승인 revision과 비교한다. `CSpawnGroupBootstrap`에는
`Has_RuntimeArtifact()`를 추가한다. Valtan과 Character Select Arena는 `true`가 필수이고, Bern/Training Ground는
`false`만 허용한다. Bern/Training이 `true`이면 manifest 밖 unexpected artifact로 실패한다. 이 규칙은
`No spawn groups` 상태와 revision 0을 승인 revision처럼 비교하지 않으며, 향후 다른 world가 spawn group을 얻을 때
manifest inventory와 room policy를 같은 version에서 확장하도록 강제한다.

### 새 파일 `Server/Public/PlayerSkillHitRuntime.h` 전체 public contract

```cpp
#pragma once

#include "Gameplay/CombatCollisionContract.h"
#include "Network/NetworkIds.h"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace LostArk::Server
{
	struct SERVER_HIT_REPEAT_RUNTIME final
	{
		std::size_t iEventIndex = 0;
		std::uint32_t iRepeatOrdinal = 0;
		std::uint32_t iWindowStartOffset = 0;
		std::uint32_t iWindowEndOffset = 0;
		bool hasCapturedAnchor = false;
		LostArk::Shared::CombatCollision::COMBAT_HIT_VOLUME_KIND eShapeKind =
			LostArk::Shared::CombatCollision::COMBAT_HIT_VOLUME_KIND::END;
		float fLegacyAnchorRootX = 0.f;
		float fLegacyAnchorRootZ = 0.f;
		float fLegacyXzRange = 0.f;
		LostArk::Shared::CombatCollision::COMBAT_CAPSULE WorldCapsule;
		std::vector<LostArk::Shared::NET_ENTITY_ID> HitTargetIds;
	};

	struct SERVER_PLAYER_SKILL_HIT_RUNTIME final
	{
		std::uint32_t iActionStartTick = 0;
		std::uint32_t iStageIndex = 0;
		bool hasConsumedStageDamageBudget = false;
		bool hasEvaluatedFirstDamageWindow = false;
		std::vector<SERVER_HIT_REPEAT_RUNTIME> Repeats;

		void Reset();
		void Begin_Stage(std::uint32_t actionStartTick, std::uint32_t stageIndex);
};
}
```

`Begin_Stage`는 authored repeat 총수를 먼저 checked-add로 계산하고
`MAXIMUM_HIT_EVENTS_PER_STAGE * MAXIMUM_REPEATS_PER_EVENT` 이하일 때만 `reserve`한다. `HitTargetIds`도 policy에
따른 stage/event `maximumTargets`까지만 reserve/push한다. runtime이 catalog validation을 신뢰해 무제한 growth하지 않고 push
직전에도 public cap을 다시 검사한다. cap 위반은 action을 cancel하고 Server diagnostic failure로 처리한다.

### `ServerPlayer.h`

다음을 제거한다.

```cpp
bool hasAppliedSkillDamage = false;
```

다음을 추가한다.

```cpp
LostArk::Shared::CombatCollision::COMBAT_BODY_CAPSULE CombatBodyCapsule;
SERVER_PLAYER_SKILL_HIT_RUNTIME SkillHitRuntime;
```

### `ServerWorldEntity.h`

다음을 제거한다.

```cpp
float fCollisionRadius = 0.f;
```

다음을 추가한다.

```cpp
LostArk::Shared::CombatCollision::COMBAT_BODY_CAPSULE CombatBodyCapsule;
```

## G04. Server combat collision query

### 새 파일 `Server/Public/CombatCollisionSystem.h` 전체 코드

```cpp
#pragma once

#include "Gameplay/CombatCollisionContract.h"
#include "Network/NetworkIds.h"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace LostArk::Server
{
	struct SERVER_WORLD_ENTITY;

	struct SERVER_COMBAT_HIT_VOLUME final
	{
		LostArk::Shared::CombatCollision::COMBAT_HIT_VOLUME_KIND eKind =
			LostArk::Shared::CombatCollision::COMBAT_HIT_VOLUME_KIND::END;
		float fLegacyAnchorRootX = 0.f;
		float fLegacyAnchorRootZ = 0.f;
		float fLegacyXzRange = 0.f;
		LostArk::Shared::CombatCollision::COMBAT_CAPSULE WorldCapsule;
	};

	struct COMBAT_COLLISION_CANDIDATE final
	{
		LostArk::Shared::NET_ENTITY_ID iNetEntityId =
			LostArk::Shared::INVALID_NET_ENTITY_ID;
		std::size_t iWorldEntityIndex = 0;
		float fDistanceSquared = 0.f;
	};

	class CCombatCollisionSystem final
	{
	public:
		bool Query_WorldTargets(
			const SERVER_COMBAT_HIT_VOLUME& hitVolume,
			const std::vector<SERVER_WORLD_ENTITY>& worldEntities,
			const std::vector<LostArk::Shared::NET_ENTITY_ID>& excludedTargetIds,
			std::uint32_t remainingTargetLimit,
			std::vector<COMBAT_COLLISION_CANDIDATE>& outCandidates) const;
	};
}
```

### `CombatCollisionSystem.cpp` algorithm

함수는 다음 순서를 그대로 구현한다.

1. `outCandidates.clear()`.
2. invalid/ambiguous hit volume 또는 `remainingTargetLimit==0`이면 false.
3. world entity를 순회한다.
4. kind가 BOSS/MONSTER가 아니거나 DEAD/HP 0/invalid ID이거나 ID가 `excludedTargetIds`에 이미 있으면 skip.
5. runtime body spec이 invalid면 `outCandidates.clear()` 후 전체 query를 false로 실패시킨다. invalid actor를 없는
   target처럼 숨기거나 앞에서 모은 partial candidate를 반환하지 않는다.
6. `LEGACY_XZ_RANGE`는 target root XZ squared distance를
   `(hitVolume.fLegacyXzRange + body.fRadius)^2`와 비교한다. 현재 코드와 같이 별도 epsilon, Y, body segment를
   사용하지 않는다.
7. `CAPSULE`은 runtime body spec으로 `Build_BodyCapsule`하고 AABB broad phase와 Shared narrow phase를 호출한다.
8. legacy candidate distance는 root XZ squared distance, capsule candidate distance는
   `CombatCollision::DistanceSquared(hitCapsule, bodyCapsule)`이다. midpoint 거리를 사용하지 않는다.
9. `distanceSquared`, `NetEntityId` 순 stable sort.
10. `remainingTargetLimit`보다 길면 resize.
11. 성공 시 true를 반환한다. 후보 0개도 정상 성공이다.

정렬 comparator exact block:

```cpp
std::sort(outCandidates.begin(), outCandidates.end(),
	[](const COMBAT_COLLISION_CANDIDATE& left,
		const COMBAT_COLLISION_CANDIDATE& right)
	{
		if (left.fDistanceSquared != right.fDistanceSquared)
			return left.fDistanceSquared < right.fDistanceSquared;
		return left.iNetEntityId < right.iNetEntityId;
	});
```

## G05. `CPlayerSkillSystem` 교체 계약

### Header signature

```cpp
void Update(
	SERVER_PLAYER& player,
	std::vector<SERVER_WORLD_ENTITY>& worldEntities,
	const CGameplayCatalog& catalog,
	const CServerNavigation* navigation,
	const CCombatCollisionSystem& combatCollision,
	CCombatDisplacementSystem& displacement,
	float fixedDeltaSeconds,
	std::uint32_t serverTick,
	std::vector<LostArk::Shared::DAMAGE_EVENT>& outDamageEvents) const;
```

### `Try_Start` 변경

`CPlayerSkillSystem`에 다음 command-preflight helper를 추가한다. 내부 `Validate_StartIdentity`를 `Try_Start`와 공유해
sequence/skill/class/HP/finite aim/stance 검증을 두 벌로 만들지 않는다.

```cpp
bool Try_ConsumeForcedMotionBlockedStartSequence(
	SERVER_PLAYER& player,
	const LostArk::Shared::C2S_USE_SKILL& command,
	const CGameplayCatalog& catalog) const;
```

precondition은 `player.ForcedMotion.bActive`다. valid strict-forward command면 `iLastSkillSequence`만 기록하고 true,
invalid/stale면 아무 것도 바꾸지 않고 false다. resource/cooldown/action/stance/path는 어느 경우에도 바꾸지 않는다.
Client가 USE/RELEASE에 같은 `m_iNextActionSequence`를 쓰므로 Server도 기존 `iLastSkillSequence`를 두 packet의 공통
action-input stream으로 사용한다. `Release`는 같은 strict-forward guard를 검증·기록한 뒤 forced-motion/action guard를
평가하고 별도 release counter를 만들지 않는다.

- skill admission 성공 뒤 `catalog.Find_CombatTimeline(skill->strCombatTimelineId)`가 null이면 시작을 거부한다.
- `iComboStage`는 timeline stage count와 skill kind로 결정한다.
- `SkillHitRuntime.Begin_Stage(player.iActionStartTick, 0u)`를 호출한다.
- 기존 `hasAppliedSkillDamage=false` 대입은 모두 제거한다.

anonymous `IsInsideComboWindow`는 skill의 removed `ComboStages`를 받지 않는다. 다음 입력으로 교체한다.

```cpp
bool IsInsideComboWindow(
	const PLAYER_COMBAT_TIMELINE& timeline,
	const SERVER_PLAYER& player)
{
	if (0u == player.iComboStage ||
		player.iComboStage > timeline.Stages.size())
	{
		return false;
	}
	const PLAYER_COMBAT_STAGE& stage =
		timeline.Stages[player.iComboStage - 1u];
	if (!stage.hasInputWindow)
		return false;
	const float elapsedMs = player.fActionElapsedSeconds * 1000.f;
	return elapsedMs >= static_cast<float>(stage.iInputOpenMs) &&
		elapsedMs <= static_cast<float>(stage.iInputCloseMs);
}
```

input window는 hit window와 달리 기존 `fActionElapsedSeconds`와 inclusive ms contract를 그대로 유지한다.
publisher가 임의 tick rounding을 하지 않는다.

`Try_Counter`도 `catalog.Find_CombatTimeline(skillId)`에서 stage 0을 얻고, stage count 2와
`hasInputWindow/inputOpenMs/inputCloseMs`를 검사한다. counter 성공 뒤
`newActionStartTick=NextNonZeroTick(serverTick)`으로 두고 `SkillHitRuntime.Begin_Stage(newActionStartTick, 1u)`를 호출한다.
`Release`는 removed stage struct를 읽지 않으므로 skill kind validation을 유지한다.

### `Update` 단계

기존 regen/admission/root-motion/damage/stage transition을 다음 meaning unit으로 분리한다.

```cpp
const PLAYER_COMBAT_STAGE* Resolve_CurrentStage(
	const SERVER_PLAYER& player,
	const CGameplayCatalog& catalog);

void Apply_StageRootMotion(
	SERVER_PLAYER& player,
	const PLAYER_COMBAT_STAGE& stage,
	const CServerNavigation* navigation,
	float fixedDeltaSeconds);

bool Evaluate_HitWindows(
	SERVER_PLAYER& player,
	std::vector<SERVER_WORLD_ENTITY>& worldEntities,
	const PLAYER_SKILL_DEFINITION& skill,
	const PLAYER_COMBAT_STAGE& stage,
	const CGameplayCatalog& catalog,
	const CCombatCollisionSystem& combatCollision,
	CCombatDisplacementSystem& displacement,
	std::uint32_t serverTick,
	std::vector<LostArk::Shared::DAMAGE_EVENT>& outDamageEvents);

void Finish_OrAdvanceStage(
	SERVER_PLAYER& player,
	const PLAYER_SKILL_DEFINITION& skill,
	const PLAYER_COMBAT_STAGE& stage,
	std::uint32_t serverTick);
```

### Hit window 평가 exact 순서

```text
if !Try_GetForwardTickDistanceSkippingZero(player.iActionStartTick, serverTick, stageElapsedTick):
  return success with no window
for eventIndex in authored order
  for repeatOrdinal in [0, repeatCount)
    repeatStart = event.start + repeatOrdinal * event.interval
    repeatEnd   = event.end   + repeatOrdinal * event.interval
    if stageElapsedTick is outside [repeatStart, repeatEnd): continue
    find-or-create bounded runtime row(eventIndex, repeatOrdinal)
    if anchor not captured:
      if LEGACY_XZ_RANGE: capture current player root XZ and range
      if CAPSULE: WorldCapsule = Transform_LocalCapsule(event.LocalVolume.LocalCapsule, current player root/yaw)
      mark captured
    mark hasEvaluatedFirstDamageWindow
    if stage policy ONCE_PER_STAGE and budget consumed: continue damage query
    resolve maximumTargets and damageProfileId from stage budget or event exclusive union
    if repeat.HitTargetIds.size >= maximumTargets: continue
    remainingTargetLimit = maximumTargets - repeat.HitTargetIds.size
    Query_WorldTargets(captured hit volume, repeat.HitTargetIds, remainingTargetLimit)
    resolve/apply damage in stable candidate order
    if event.reactionProfileId is non-empty and target survived: queue reaction from the same resolved hit
    append target ID after authoritative damage resolution succeeds, including a valid zero-damage application
    for ONCE_PER_STAGE, consume budget after the first non-empty resolved candidate batch
```

`maximumTargets`는 tick당 cap이 아니라 repeat 전체의 unique-target cap이다. 이미 맞은 ID를 query 결과를 자른 뒤
버리면 더 먼 신규 후보를 놓치므로, exclusion은 scan/sort/resize 전에 적용한다. multi-tick window에서 첫 tick 1명,
다음 tick 신규 2명이 들어오고 cap이 2면 두 번째 tick에는 remaining 1명만 반환·적용한다. 정상 입력에서
`HitTargetIds` cap 초과는 발생할 수 없으며 발생하면 runtime corruption으로 action을 cancel한다.

`DAMAGE_EVENT` capacity가 가득 차도 HP와 death는 먼저 적용한다. event push만 생략하고 Debug output에 overflow
counter를 올린다.

### Stage reset

다음 모든 edge에서 `SkillHitRuntime.Reset()` 또는 `Begin_Stage()`를 호출한다.

- new skill accepted
- combo/hold/counter stage advanced
- action duration complete
- action cancelled
- player death/revive
- room leave/disconnect

combo advance 조건의 기존 `hasAppliedSkillDamage`는 아래로 교체한다.

```cpp
const bool cancelsIntoNextStage =
	!isHold && hasNextStage &&
	player.SkillHitRuntime.hasEvaluatedFirstDamageWindow;
```

이는 target이 없어도 첫 damage window를 평가한 뒤 buffered combo가 다음 stage로 넘어가는 기존 의미를 보존한다.

Shared `NextNonZeroTick(tick)`은 `UINT32_MAX` 다음을 1로 두고 0을 invalid sentinel로 유지한다. 새 skill은 GameRoom이 넘긴
`updateTick`에서 즉시 age 0으로 평가한다. `Finish_OrAdvanceStage`가 combo/hold/counter의 새 stage를 만들 때는
`iActionStartTick=NextNonZeroTick(serverTick)`, `fActionElapsedSeconds=0`, `Begin_Stage(nextTick, nextStage)`로 예약하고 같은
Update에서 새 stage를 재평가하지 않는다. 다음 Update가 정확히 age 0을 평가하므로 0 ms hit가 사라지지 않는다.
duration/root motion은 현재처럼 elapsed seconds를 먼저 증가시키고 ms threshold/interpolation을 적용한다. stage
age, repeat lifetime, ledger expiry 어디에서도 raw unsigned subtraction을 다시 쓰지 않는다.

## G06. Client debug runtime JSON

### Enter-approved runtime revision

`Shared/Public/Network/PacketMessages.h`의 `S2C_ENTER_ACCEPTED`에 다음 필드를 추가하고
`Shared/Public/Network/PacketType.h`의 current version 12를 final version 13으로 한 번만 bump한다. G19의 reaction,
occurrence, activation packet도 같은 v13 변경 단위에 포함하고 중간 version을 만들지 않는다.

```cpp
inline constexpr std::size_t MAX_COMBAT_RUNTIME_REVISION_BYTES = 64u;

struct S2C_ENTER_ACCEPTED
{
	std::uint16_t iProtocolVersion = NETWORK_PROTOCOL_VERSION;
	WORLD_ID eWorldId = WORLD_ID::BERN;
	PLAYER_ID iPlayerId = INVALID_PLAYER_ID;
	NET_ENTITY_ID iNetEntityId = INVALID_NET_ENTITY_ID;
	std::string strCombatRuntimeRevision;
};
```

writer/reader는 revision이 정확히 64자의 lowercase hexadecimal인지 검증한다. reader는 local temporary를 모두
검증한 뒤 message에 commit해 truncated/non-hex/wrong-length 입력에서 destination을 유지한다. `CGameRoom::Join`은
room ready 때 검증한 runtime-set manifest revision을 싣는다. `CNetworkManager`는 accepted connection revision을
보관하고 `std::string_view Get_CombatRuntimeRevision() const`로 노출하며 disconnect/world handoff reset에서 비운다.
`C2S_USE_SKILL` 구조에는 아무 필드도 추가하지 않는다.

### 출력 위치

`Client/Bin/DataFiles/Gameplay/CombatColliders.debug.json`

### 전체 top-level schema

```json
{
  "schema": "lostark.combat-collider-debug-runtime",
  "formatVersion": 1,
  "sourceRevision": "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef",
  "fixedTickHz": 30,
  "playerBodies": [
    {
      "characterClass": "LANCE_MASTER",
      "radius": 0.45,
      "cylinderHalfHeight": 0.45,
      "centerOffsetY": 0.9
    }
  ],
  "worldBodies": [
    {
      "kind": "MONSTER",
      "archetypeId": "MONSTER_VALTAN_PADD_01",
      "radius": 0.55,
      "cylinderHalfHeight": 0.55,
      "centerOffsetY": 1.1
    }
  ],
  "timelines": [
    {
      "skillId": 999999,
      "stages": [
        {
          "stageIndex": 0,
          "stageEvaluationTickCount": 55,
          "allowedNextStageIndices": [],
          "events": [
            {
              "eventId": "hit.legacy",
              "startTickOffset": 8,
              "endTickOffset": 9,
              "repeatCount": 1,
              "repeatIntervalTicks": 0,
              "anchorPolicy": "ACTION_ROOT_AT_WINDOW_OPEN",
              "shape": { "kind": "LEGACY_XZ_RANGE", "range": 4.5 }
            },
            {
              "eventId": "hit.01",
              "startTickOffset": 7,
              "endTickOffset": 8,
              "repeatCount": 1,
              "repeatIntervalTicks": 0,
              "anchorPolicy": "ACTION_ROOT_AT_WINDOW_OPEN",
              "shape": {
                "kind": "CAPSULE",
                "localStart": [0.0, 0.9, 0.4],
                "localEnd": [0.0, 0.9, 2.4],
                "radius": 0.8
              }
            }
          ]
        }
      ]
    }
  ],
  "bossEvents": [
    {
      "bossArchetypeId": "BOSS_VALTAN",
      "hitEventId": "dash-charge.contact.01",
      "anchorPolicy": "OWNER_ROOT_FOLLOW",
      "anchorLocalOffset": [0.0, 0.0, 0.0],
      "targetExtentPolicy": "HURT_CAPSULE_FOOTPRINT_XZ",
      "trackId": "",
      "shape": {
        "kind": "FORWARD_BOX",
        "localCenter": [0.0, 0.0, 0.0],
        "localYawDegrees": 0.0,
        "forwardStart": 0.0,
        "forwardEnd": 3.5,
        "halfWidth": 2.5
      }
    }
  ],
  "bossHitTracks": [
    {
      "trackId": "valtan.weapon.swing.reviewed.v1",
      "samples": [
        {
          "tickOffset": 0,
          "localStart": [0.0, 1.8, 0.6],
          "localEnd": [0.0, 1.8, 1.8],
          "radius": 0.7
        }
      ]
    }
  ]
}
```

exact-property set은 root의 아홉 field, player body의 네 field, world body의 다섯 field, timeline의 두 field,
stage의 네 field, event의 일곱 field다. shape는 `{kind,range}` legacy 또는
`{kind,localStart,localEnd,radius}` capsule exclusive union이다. `anchorPolicy`는 v1에서
`ACTION_ROOT_AT_WINDOW_OPEN`만 허용한다. `playerBodies` key는 characterClass, `worldBodies` key는
`(kind, archetypeId)`이고 `kind`는 `MONSTER/BOSS`만 허용한다. timeline row는 Server bootstrap의 quantized hit
event와 carryover에 필요한 stage evaluation count/allowed transition만 표현한다. damage rate/profile/target cap,
HP, defense, model/effect path는 포함하지 않는다.

boss event exact fields는 `bossArchetypeId,hitEventId,anchorPolicy,anchorLocalOffset,targetExtentPolicy,trackId,shape`다.
non-track shape union은 G16과 같고 top-level `trackId`는 empty string이다. track shape는 정확히
`{"kind":"CAPSULE_TRACK"}`이며 top-level `trackId`가 non-empty/existing이어야 한다. shape와 top-level에 track ID를
동시에 저장하는 ambiguous form은 거부한다. boss track은 `trackId,samples`, sample은
`tickOffset,localStart,localEnd,radius` exact set이다. damage,
reaction profile, effect/clip/bone path와 source hash는 debug runtime에 넣지 않는다. occurrence snapshot이 event ID를
제공하고 debug catalog는 shape/track만 제공한다.

위 `skillId=999999` row는 두 shape union의 wire-format만 한 곳에서 보이는 schema fixture이며 product catalog에
publish하는 gameplay fixture가 아니다. 실제 DimensionMaster 2050210 runtime row는 hittrace-admitted
`hit.01~04` capsule 네 개만 가지며 legacy+capsule 혼합 예시로 사용하지 않는다.

writer ordering은 player class enum, world `(kind,archetypeId)`, timeline skillId/stage/eventId,
boss `(archetypeId,hitEventId)`, track ID/sample tick 순이며 parser는
duplicate/out-of-order row, unknown enum/ID, non-finite/invalid body/shape, offset `end<=start`, repeat overflow,
stageEvaluationTickCount 밖 event, invalid next-stage index, extra/null/missing field를 거부하고 기존 catalog를 유지한다.
`sourceRevision`은 publisher와 ProjectAudit가 generated output parity를 확인하고 Client가 enter-approved remote
revision과 비교하는 full SHA-256다. mismatch면 debug catalog status를 unavailable로 바꾸지만 gameplay socket,
snapshot, character presentation은 유지한다.

### `CombatColliderDebugCatalog.h` public contract

```cpp
#pragma once

#include "Gameplay/CombatCollisionContract.h"
#include "Gameplay/BossCombatContract.h"
#include "Network/PacketMessages.h"

#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

namespace Client
{
#ifdef _DEBUG
	enum class COMBAT_DEBUG_ANCHOR_POLICY : std::uint8_t
	{
		ACTION_ROOT_AT_WINDOW_OPEN,
		END
	};

	struct COMBAT_DEBUG_HIT_EVENT final
	{
		std::string strEventId;
		std::uint32_t iStartTickOffset = 0;
		std::uint32_t iEndTickOffset = 0;
		std::uint32_t iRepeatCount = 0;
		std::uint32_t iRepeatIntervalTicks = 0;
		COMBAT_DEBUG_ANCHOR_POLICY eAnchorPolicy =
			COMBAT_DEBUG_ANCHOR_POLICY::END;
		LostArk::Shared::CombatCollision::COMBAT_LOCAL_HIT_VOLUME LocalVolume;
	};

	struct COMBAT_DEBUG_STAGE final
	{
		std::uint32_t iStageIndex = 0;
		std::uint32_t iStageEvaluationTickCount = 0;
		std::vector<std::uint32_t> AllowedNextStageIndices;
		std::vector<COMBAT_DEBUG_HIT_EVENT> HitEvents;
	};

	struct COMBAT_DEBUG_BOSS_EVENT final
	{
		std::string strBossArchetypeId;
		std::string strHitEventId;
		LostArk::Shared::BossCombat::HIT_ANCHOR_POLICY eAnchorPolicy =
			LostArk::Shared::BossCombat::HIT_ANCHOR_POLICY::END;
		float fAnchorLocalOffsetX = 0.f;
		float fAnchorLocalOffsetY = 0.f;
		float fAnchorLocalOffsetZ = 0.f;
		LostArk::Shared::BossCombat::TARGET_EXTENT_POLICY eTargetExtentPolicy =
			LostArk::Shared::BossCombat::TARGET_EXTENT_POLICY::END;
		std::string strTrackId;
		LostArk::Shared::BossCombat::LOCAL_HIT_SHAPE LocalShape;
	};

	struct COMBAT_DEBUG_BOSS_TRACK final
	{
		std::string strTrackId;
		std::vector<std::pair<std::uint32_t,
			LostArk::Shared::CombatCollision::COMBAT_CAPSULE>> Samples;
	};

	class CCombatColliderDebugCatalog final
	{
	public:
		bool Load_Runtime(std::string& outStatus);
		const LostArk::Shared::CombatCollision::COMBAT_BODY_CAPSULE*
			Find_PlayerBody(LostArk::Shared::CHARACTER_CLASS_ID characterClass) const;
		const LostArk::Shared::CombatCollision::COMBAT_BODY_CAPSULE*
			Find_WorldBody(
				LostArk::Shared::WORLD_ENTITY_KIND kind,
				std::string_view archetypeId) const;
		const COMBAT_DEBUG_STAGE* Find_Stage(
			LostArk::Shared::SKILL_ID skillId,
			std::uint32_t stageIndex) const;
		const COMBAT_DEBUG_BOSS_EVENT* Find_BossEvent(
			std::string_view bossArchetypeId,
			std::string_view hitEventId) const;
		const COMBAT_DEBUG_BOSS_TRACK* Find_BossTrack(
			std::string_view trackId) const;
		std::string_view Get_SourceRevision() const;
	};
#endif
}
```

내부 storage key는 stable class/archetype/skill-stage tuple이다. pointer/tag/vector index를 외부 저장 ID로 쓰지 않는다.

## G07. Client debug view model

### Shape와 owner enum

```cpp
enum class COMBAT_DEBUG_SHAPE_ROLE : std::uint8_t
{
	LOCAL_PLAYER_HURT,
	REMOTE_PLAYER_HURT,
	MONSTER_HURT,
	BOSS_HURT,
	PLAYER_HIT,
	BOSS_HIT_PENDING,
	BOSS_HIT_ACTIVE,
	BOSS_HIT_GHOST,
	PLAYER_HIT_GHOST,
	DAMAGE_TARGET
};

enum class COMBAT_DEBUG_GEOMETRY_KIND : std::uint8_t
{
	LEGACY_XZ_RING,
	CAPSULE,
	CIRCLE,
	RING,
	CONE,
	ORIENTED_BOX,
	CROSS,
	CAPSULE_TRACK_SAMPLE,
	END
};

struct COMBAT_DEBUG_SHAPE final
{
	LostArk::Shared::NET_ENTITY_ID iOwnerNetEntityId =
		LostArk::Shared::INVALID_NET_ENTITY_ID;
	std::string strHitEventId;
	COMBAT_DEBUG_SHAPE_ROLE eRole = COMBAT_DEBUG_SHAPE_ROLE::MONSTER_HURT;
	COMBAT_DEBUG_GEOMETRY_KIND eKind =
		COMBAT_DEBUG_GEOMETRY_KIND::END;
	float fLegacyAnchorRootX = 0.f;
	float fLegacyAnchorRootY = 0.f;
	float fLegacyAnchorRootZ = 0.f;
	float fLegacyXzRange = 0.f;
	LostArk::Shared::CombatCollision::COMBAT_CAPSULE Capsule;
	LostArk::Shared::BossCombat::LOCAL_HIT_SHAPE BossShape;
	LostArk::Shared::BossCombat::WORLD_HIT_ANCHOR BossAnchor;
	std::uint32_t iGhostExpireTick = 0;
};
```

### `CCombatColliderDebugViewModel` public API

```cpp
class CCombatColliderDebugViewModel final
{
public:
	static CCombatColliderDebugViewModel& Get();

	bool Initialize(const CCombatColliderDebugCatalog* catalog, std::string& outStatus);
	void Apply_PlayerSpawn(const LostArk::Shared::S2C_PLAYER_SPAWNED& spawned);
	void Apply_PlayerDespawn(const LostArk::Shared::S2C_PLAYER_DESPAWNED& despawned);
	void Apply_WorldSpawn(const LostArk::Shared::S2C_WORLD_ENTITY_SPAWNED& spawned);
	void Apply_WorldDespawn(const LostArk::Shared::S2C_WORLD_ENTITY_DESPAWNED& despawned);
	void Apply_Snapshot(
		const LostArk::Shared::S2C_WORLD_SNAPSHOT& snapshot,
		LostArk::Shared::NET_ENTITY_ID localPlayerEntityId,
		std::string_view acceptedCombatRuntimeRevision);
	void Reset();

	const std::vector<COMBAT_DEBUG_SHAPE>& Get_FrameShapes() const;
	std::string_view Get_Status() const;
};
```

`Initialize(nullptr, status)`는 Debug overlay unavailable을 나타내는 정상 성공 경로다. catalog pointer를 null로
commit하고 모든 entity/window/ghost를 비운 뒤 `Apply_*`와 `Get_FrameShapes()`가 no-op/empty를 반환하게 한다.
따라서 catalog missing/corrupt여도 `CClientReplication` hook은 null dereference나 gameplay failure를 만들지 않는다.
non-null catalog의 revision/graph 검증 실패만 Initialize false이며 이 경우에도 이전 state를 commit하지 않는다.

### `ClientReplication.cpp` hook 위치

각 기존 적용이 성공한 뒤 `_DEBUG`에서만 다음을 호출한다.

```cpp
CCombatColliderDebugViewModel::Get().Apply_PlayerSpawn(spawned);
CCombatColliderDebugViewModel::Get().Apply_PlayerDespawn(despawned);
CCombatColliderDebugViewModel::Get().Apply_WorldSpawn(spawned);
CCombatColliderDebugViewModel::Get().Apply_WorldDespawn(despawned);
CCombatColliderDebugViewModel::Get().Apply_Snapshot(
	snapshot,
	CNetworkManager::Get().Get_LocalEntityId(),
	CNetworkManager::Get().Get_CombatRuntimeRevision());
```

`Reset_World()` 마지막에는 다음을 호출한다.

```cpp
#ifdef _DEBUG
CCombatColliderDebugViewModel::Get().Reset();
#endif
```

spawn hook은 packet identity/class/archetype 검증 직후, presentation GameObject/Prototype 생성보다 먼저 실행한다.
presentation 생성이 실패해도 debug identity는 남기고 `presentationMissing` status를 기록하며, despawn packet에서
항상 제거한다. GameObject 성공 return 뒤에만 hook을 두어 collider가 presentation 성공 여부에 종속되게 하지 않는다.

view model은 repeat의 exact `windowStartTick` snapshot을 관측했을 때만 anchor를 capture한다. 첫 관측 tick이 이미
window start를 지났다면 현재 transform으로 늦게 근사하지 않고 그 repeat draw를 생략하며 diagnostic counter를
증가시킨다. TCP snapshot 순서가 정상인 제품 경로에서는 exact start tick을 관측하고, late join/invalid stream은
잘못된 capsule을 그리는 대신 fail closed한다.

snapshot stage mapping은 Server와 동일하게 ACTIVE의 `iComboStage=0`을 timeline `stageIndex=0`으로,
COMBO/HOLD/COUNTER의 1-based `iComboStage`를 `stageIndex=iComboStage-1`로 바꾼다. out-of-range stage는 fallback하지
않고 해당 hit draw를 생략한다.

accepted revision이 local catalog `sourceRevision`과 다르거나 empty면 body/hit frame 생성을 모두 중단하고
`runtime revision mismatch`를 status에 남긴다. gameplay replication은 계속한다.
`Try_GetForwardTickDistanceSkippingZero(actionStartTick, snapshotTick, age)`가 false인 future start는 pending으로
처리한다. `CActionPresentationTimeline`은 이 경우 visual age를 0으로 clamp하지만, F7 hit volume은 helper가 age를
반환하기 전에는 만들지 않는다.

active hit shape가 종료되면 동일 geometry/identity를 `PLAYER_HIT_GHOST`로 바꾸고 Shared forward-distance 기준 6 server tick
동안 bounded history에 둔다. active yellow와 ghost orange는 동시에 같은 event/repeat로 중복 제출하지 않는다.
history는 player당 `MAXIMUM_HIT_EVENTS_PER_STAGE * MAXIMUM_REPEATS_PER_EVENT`를 넘지 않고 oldest-first로 제거한다.
ghost는 `Query_WorldTargets`, damage ledger, target highlight 입력으로 절대 사용하지 않는다.

snapshot commit 순서는 `validate incoming -> transition carryover using previous -> commit incoming -> build current`
으로 고정한다. Server가 hit을 평가한 같은 Update에서 stage/`NONE`을 snapshot하면 Client가 old active frame을 받지
못하므로, carryover가 previous action/stage를 incoming `serverTick`과 incoming root/yaw로 한 번 평가한다.

- same skill의 Server-valid combo/hold/counter stage advance이고 incoming `actionStartTick`이
  `NextNonZeroTick(incoming.serverTick)`이면 old active event를 ghost로 기록한다.
- incoming action이 `NONE`, player가 alive이고 previous age가 authored `stageEvaluationTickCount`의 normal completion
  boundary에 도달했다면 final-tick old active event를 ghost로 기록한다.
- incoming `DEAD`, HP 0, early completion boundary 이전 cancel, despawn/disconnect/level reset, skill identity 교체에는
  carryover ghost를 만들지 않는다.
- multi-tick event anchor가 이전 frame에서 이미 capture되었으면 그것을 사용한다. carryover tick이 정확한 window
  open이면 incoming root/yaw로 capture한다. 그보다 늦었고 기존 anchor가 없으면 `late anchor unavailable`로 생략한다.
- `(NetEntityId, actionStartTick, stageIndex, eventId, repeatOrdinal)` key로 이미 active/ghost가 있었으면 중복하지 않는다.

`stageEvaluationTickCount`와 Server-valid stage transition graph는 debug catalog가 canonical timeline에서 함께
생성하며 Client가 skill kind를 추측해 별도 전이 규칙을 만들지 않는다.

## G08. Engine wire capsule draw

### `DebugDraw.h` 추가 선언

```cpp
void XM_CALLCONV DrawCapsule(
	DirectX::PrimitiveBatch<DirectX::VertexPositionColor>* batch,
	DirectX::FXMVECTOR pointA,
	DirectX::FXMVECTOR pointB,
	float radius,
	DirectX::GXMVECTOR color = DirectX::Colors::White);
```

### draw 규칙

- segment가 0에 가까우면 기존 sphere draw와 같은 세 orthogonal ring을 그린다.
- 유효 segment면 axis와 평행하지 않은 helper axis를 골라 normalized basis 두 개를 만든다.
- endpoint마다 axis에 수직인 32-segment ring을 그린다.
- ring의 0/90/180/270도 네 점을 직선으로 연결한다.
- 각 cap은 axis가 포함된 두 평면에서 hemisphere arc를 그린다.
- radius가 finite positive가 아니면 draw call을 내지 않는다.
- helper는 visualization만 소유하고 intersection 함수를 만들지 않는다.
- debug `LEGACY_XZ_RING`은 새 3D helper를 만들지 않고 기존 `DrawRing`으로 root Y의 XZ circle을 그린다. target body
  radius는 별도 hurt capsule로 보이므로 공격 ring 자체를 임의 확장하지 않는다.

## G09. Client renderer

### `CombatColliderDebugRenderer.h` public contract

```cpp
#pragma once

#include "Component.h"
#include "CombatColliderDebugViewModel.h"

namespace Client
{
#ifdef _DEBUG
	class CCombatColliderDebugRenderer final : public Engine::CComponent
	{
	private:
		CCombatColliderDebugRenderer(
			ComPtr<ID3D11Device> device,
			ComPtr<ID3D11DeviceContext> context);

	public:
		virtual ~CCombatColliderDebugRenderer();
		virtual HRESULT Initialize_Prototype() override;
		virtual HRESULT Initialize(void* argument) override;
		virtual HRESULT Render() override;
		virtual shared_ptr<Engine::CPrototype> Clone(void* argument) override;

		void Set_FrameShapes(
			const std::vector<COMBAT_DEBUG_SHAPE>& shapes);

		static shared_ptr<CCombatColliderDebugRenderer> Create(
			ComPtr<ID3D11Device> device,
			ComPtr<ID3D11DeviceContext> context);

	private:
		std::vector<COMBAT_DEBUG_SHAPE> m_FrameShapes;
		shared_ptr<PrimitiveBatch<VertexPositionColor>> m_pBatch;
		shared_ptr<BasicEffect> m_pEffect;
		ComPtr<ID3D11InputLayout> m_pInputLayout;
	};
#endif
}
```

`Render()`는 view/projection을 현재 GameInstance에서 읽고 한 번 `Begin/End` 사이에 모든 shape를 그린다.
kind가 `CAPSULE`이면 `DrawCapsule`, `LEGACY_XZ_RING`이면 `DrawRing`을 호출한다. role-to-color는 renderer 한
곳에서만 결정하고 `PLAYER_HIT_GHOST`는 dim orange다. `Set_FrameShapes`는 frame snapshot을 복사하며 GameObject pointer를
저장하지 않는다.

renderer switch harness는 final enum의 모든 drawable kind를 한 번씩 통과시키고 `END`는 draw zero/diagnostic 처리한다.

## G10. MainApp F7 integration

### `MainApp.h` `_DEBUG` member 추가

```cpp
CCombatColliderDebugCatalog m_CombatColliderDebugCatalog;
shared_ptr<CCombatColliderDebugRenderer> m_pCombatColliderDebugRenderer;
bool_t m_bF7Down = false;
bool_t m_bCombatColliderDebugVisible = false;
string m_strCombatColliderDebugStatus;
```

forward declaration과 include는 실제 complete-type 요구에 맞춰 배치한다. value member인 catalog는 header include가
필요하고 renderer는 forward declaration으로 충분하다.

### `ReadyDebugTools()` 추가 block

```cpp
const bool_t colliderCatalogLoaded =
	m_CombatColliderDebugCatalog.Load_Runtime(
		m_strCombatColliderDebugStatus);
if (!colliderCatalogLoaded)
	m_strCombatColliderDebugStatus =
		"Combat collider overlay unavailable: " +
		m_strCombatColliderDebugStatus;
if (!CCombatColliderDebugViewModel::Get().Initialize(
		colliderCatalogLoaded ? &m_CombatColliderDebugCatalog : nullptr,
		m_strCombatColliderDebugStatus))
	return E_FAIL;
m_pCombatColliderDebugRenderer =
	CCombatColliderDebugRenderer::Create(m_pDevice, m_pContext);
if (nullptr == m_pCombatColliderDebugRenderer)
	return E_FAIL;
```

catalog load 실패가 Debug Client start를 막지는 않는다. renderer/device 생성 실패는 Debug infrastructure 실패이므로
start를 막는다.

### `UpdateDebugToolShortcut()` 전체 replacement body

```cpp
void CMainApp::UpdateDebugToolShortcut()
{
	const bool_t windowFocused =
		IsWindowOwnedByCurrentProcess(GetForegroundWindow());
	const bool_t f1Down = windowFocused &&
		0 != (GetAsyncKeyState(VK_F1) & 0x8000);
	const bool_t f7Down = windowFocused &&
		0 != (GetAsyncKeyState(VK_F7) & 0x8000);
	if (f1Down && !m_bF1Down)
		m_bDeveloperToolsVisible = !m_bDeveloperToolsVisible;
	if (f7Down && !m_bF7Down)
		m_bCombatColliderDebugVisible =
			!m_bCombatColliderDebugVisible;
	m_bF1Down = f1Down;
	m_bF7Down = f7Down;
}
```

### `Update()` submit block

`CGameInstance::Get().Update_Engine`, effect synchronization/update 뒤, level transition 적용 전 `_DEBUG` block에
추가한다.

```cpp
if (m_bCombatColliderDebugVisible &&
	nullptr != m_pCombatColliderDebugRenderer)
{
	m_pCombatColliderDebugRenderer->Set_FrameShapes(
		CCombatColliderDebugViewModel::Get().Get_FrameShapes());
	CGameInstance::Get().Add_DebugComponent(
		m_pCombatColliderDebugRenderer);
}
```

F7 off에서는 `Set_FrameShapes`와 debug queue submit을 모두 하지 않는다. view model은 snapshot state만 보관하며
per-frame JSON read를 하지 않는다.

### Developer Tools status

F1 diagnostics에 아래 read-only text만 추가한다.

```cpp
ImGui::Text("F7 Combat Colliders: %s",
	m_bCombatColliderDebugVisible ? "ON" : "OFF");
ImGui::TextWrapped("%s", m_strCombatColliderDebugStatus.c_str());
```

## G11. Effect traceability contract

### `FourClassStandaloneMesh.hittrace.json` schema

```json
{
  "schema": "lostark.four-class-standalone-mesh-hit-trace",
  "formatVersion": 1,
  "sources": [
    {
      "sourceId": "source.dimensionmaster.2050210",
      "characterClass": "DIMENSIONMASTER",
      "skillId": 2050210,
      "admissionStatus": "ADMITTED",
      "materializationDocument": "Data/Effects/AuthoredCorrections/DimensionMaster/effect.dimensionmaster.skill.2050210.authored-baseline.materialization.json",
      "materializationSha256": "4766f1e255d519912de3427ef3fc9eed8897c4a4a191aed2656813ecf0eaa72a",
      "expectedMaterializationStatus": "preserveExisting",
      "skillBindingDocument": "Data/Animation/Authored/DimensionMaster/DimensionMaster.skillbindings.json",
      "skillBindingSha256": "d1d4be0a00ac80a2847843ecb5f740dc62a23ea66be79bd095bfc282ccc563dc",
      "animeventDocument": "Data/Animation/Authored/DimensionMaster/DimensionMaster.animevents",
      "animeventSha256": "4da4baa6ec4cc88c46e3ac28e5c70cb4fc70848bcb6eeafec4002acd1ff3ce5b",
      "bindingStageIndex": 0,
      "bindingClipId": "pc_sp_m_00_sk_sk_willowrend",
      "cue": {
        "kind": "EFFECT",
        "startMs": 0,
        "payload": "effect.dimensionmaster.skill.2050210.authored-baseline",
        "effectRef": "asset",
        "anchor": "root",
        "follow": "follow",
        "stop": "natural"
      },
      "effectAssetId": "effect.dimensionmaster.skill.2050210.authored-baseline",
      "effectDocumentVersion": 12,
      "effectDocumentSha256": "c08a3d48d68bb5ca6c95a61cc5b7ee1bf4b3b0ecd033f9fe5219ae07d1122c26"
    },
    {
      "sourceId": "source.lancemaster.34010",
      "characterClass": "LANCE_MASTER",
      "skillId": 34010,
      "admissionStatus": "BLOCKED",
      "materializationDocument": "Data/Effects/AuthoredCorrections/LanceMaster/effect.lancemaster.skill.34010.materialization.json",
      "materializationSha256": "ae82c2e1e4dfe4be22de3ed7db0416332ef6f22705fbc2cb9a8a76375bfa90e2",
      "expectedMaterializationStatus": "blocked"
    },
    {
      "sourceId": "source.artist.31000",
      "characterClass": "ARTIST",
      "skillId": 31000,
      "admissionStatus": "BLOCKED",
      "materializationDocument": "Data/Effects/AuthoredCorrections/Artist/effect.artist.skill.31000.materialization.json",
      "materializationSha256": "63268c976fd9a41edc3a95a6ca788952d99dfcbb9f51cbaea0ae2dbe037b8cd9",
      "expectedMaterializationStatus": "blocked"
    },
    {
      "sourceId": "source.warlord.17000",
      "characterClass": "WARLORD",
      "skillId": 17000,
      "admissionStatus": "BLOCKED",
      "materializationDocument": "Data/Effects/AuthoredCorrections/Warlord/effect.warlord.skill.17000.materialization.json",
      "materializationSha256": "5f981efdc26fd3766a47e23abec47863e95090dd8b62f5fb314c5e52e943b380",
      "expectedMaterializationStatus": "blocked"
    }
  ],
  "links": [
    {
      "sourceId": "source.dimensionmaster.2050210",
      "stageIndex": 0,
      "hitEventId": "hit.01",
      "occurrenceId": "authored.baseline.a.hit01",
      "carrierElementId": "authored.baseline.a.hit01.body",
      "effectDelayMs": 250,
      "actionRelativeEffectStartMs": 250,
      "reviewedWindowStartOffset": 7,
      "timingDeltaTicks": 0,
      "reviewToleranceTicks": 0,
      "anchorReview": "AUTHORED_ROOT_SNAPSHOT"
    },
    {
      "sourceId": "source.dimensionmaster.2050210",
      "stageIndex": 0,
      "hitEventId": "hit.02",
      "occurrenceId": "authored.baseline.a.hit02",
      "carrierElementId": "authored.baseline.a.hit02.body",
      "effectDelayMs": 600,
      "actionRelativeEffectStartMs": 600,
      "reviewedWindowStartOffset": 17,
      "timingDeltaTicks": 0,
      "reviewToleranceTicks": 0,
      "anchorReview": "AUTHORED_ROOT_SNAPSHOT"
    },
    {
      "sourceId": "source.dimensionmaster.2050210",
      "stageIndex": 0,
      "hitEventId": "hit.03",
      "occurrenceId": "authored.baseline.a.hit03",
      "carrierElementId": "authored.baseline.a.hit03.body",
      "effectDelayMs": 900,
      "actionRelativeEffectStartMs": 900,
      "reviewedWindowStartOffset": 26,
      "timingDeltaTicks": 0,
      "reviewToleranceTicks": 0,
      "anchorReview": "AUTHORED_ROOT_SNAPSHOT"
    },
    {
      "sourceId": "source.dimensionmaster.2050210",
      "stageIndex": 0,
      "hitEventId": "hit.04",
      "occurrenceId": "authored.baseline.a.hit04",
      "carrierElementId": "authored.baseline.a.hit04.body",
      "effectDelayMs": 1300,
      "actionRelativeEffectStartMs": 1300,
      "reviewedWindowStartOffset": 38,
      "timingDeltaTicks": 0,
      "reviewToleranceTicks": 0,
      "anchorReview": "AUTHORED_ROOT_SNAPSHOT"
    }
  ]
}
```

위 hash/status는 계획 작성 시점의 current source를 실측한 값이다. G0에서 effect 세션 merge 후 다시 계산하고
문서가 바뀌었다면 trace source receipt를 같은 commit에서 갱신한다. source row는 exact discriminated union이다.
`ADMITTED`는 binding/animevent path/hash/stage/clip/cue와 effect asset/version/hash가 필수이고 non-empty link가
있어야 한다. `BLOCKED`는 이 admitted-only field와 link를 가질 수 없고 materialization `status=blocked`와
non-empty blocker array를 audit한다. cue object exact set은 `kind,startMs,payload,effectRef,anchor,follow,stop`이다.
combat `hitEventId`는 이 hittrace가 effect occurrence를 찾아가는 단방향 join key다. Effect JSON,
correction/materialization manifest, `.animevents`에는 gameplay ID를 역으로 추가하지 않는다.

timing audit은 binding stage가 resolve한 clip에서 위 exact tuple과 같은 EFFECT cue가 정확히 하나인지 확인한다.
`actionRelativeEffectStartMs = cue.startMs + effectDelayMs`를 checked-add한 뒤
`Convert-MillisecondsToEvaluationOffset(actionRelativeEffectStartMs)`로 양자화하고 timeline event의
`startTickOffset`과 비교한다. `timingDeltaTicks = abs(effectOffset-startTickOffset)`이며
stored `reviewedWindowStartOffset`은 timeline 값과 같아야 한다. formatVersion 1의 tolerance는 0 tick으로 고정하므로
`timingDeltaTicks <= reviewToleranceTicks == 0`만 통과한다. anchor audit은 source occurrence가 root snapshot이고
follow=false인지, outer cue가 `anchor=root/follow=follow`인지 함께 검사한다. 이 조합은 action root를 cue부터
추종하다 occurrence open에서 snapshot하므로 gameplay `ACTION_ROOT_AT_WINDOW_OPEN`과 일치한다.

### Audit 조건

- class/skill/stage/hit event가 canonical timeline에 존재한다.
- skillbinding stage가 exact clip 하나를 resolve하고 animevents에 exact cue tuple이 하나만 존재한다.
- cue payload가 source effectAssetId와 같고 cue start/anchor/follow/stop이 source receipt와 같다.
- effect asset이 product catalog에 admitted 상태다.
- occurrence와 carrier가 정확히 하나 존재한다.
- carrier kind는 Mesh다.
- carrier가 echo/flow/rim/afterimage/sprite role이 아니다.
- content hash가 current source receipt와 일치한다.
- quantized `(cue start + effect delay)`와 hit window start가 같은 action stage, 같은 tick이며 stored
  delta/tolerance가 0이다.
- Server/bootstrap/debug runtime에는 EffectAssetId가 들어가지 않는다.

default audit은 DimensionMaster의 current `ADMITTED` source와 네 occurrence/carrier link를 모두 strict 검증하고
Lance Master/Artist/Warlord의 명시적 `BLOCKED` source를 허용한다. blocked source가 link/reviewed capsule을
참조하면 실패한다. 각 effect 세션이 product admission을 끝낸 뒤에는
timeline+hittrace를 같은 commit에서 갱신하고 `Test-FourClassCombatHitTrace.ps1 -RequireAllAdmitted`를 실행한다.
따라서 세 class의 현재 복원 blocker가 Shared/Server/F7 foundation 완료를 막지 않는다.

## G12. Project와 data 등록

### Shared project

- `CombatCollisionContract.cpp`는 `ClCompile`.
- `Gameplay/CombatCollisionContract.h`, `Network/NetworkTickContract.h`는 `ClInclude`.
- 기존 physical folder filter에만 추가하고 다른 filter를 이동하지 않는다.

### Server project

- `BalanceRuntimeSetManifest.cpp`, `CombatCollisionSystem.cpp`, `PlayerSkillHitRuntime.cpp`는 `ClCompile`.
- 대응 세 header는 `ClInclude`이며 기존 physical folder filter에만 등록한다.
- Debug/Release x64 linker `AdditionalDependencies`에 `bcrypt.lib;%(AdditionalDependencies)`를 추가해 기존 library를
  덮어쓰지 않는다.

### Client project

- debug catalog/view model/renderer CPP/H를 등록한다.
- player timeline 여섯 개, reaction/Valtan combat·presentation authoring과 trace JSON은 Client 프로젝트
  `96.DataFiles` 아래 `None`으로만 노출한다.
- `Client/Bin/DataFiles/Gameplay/CombatColliders.debug.json`은 generated runtime이며 `Content` item으로 만들지 않는다.

### 새 C++ 인코딩

모든 새 H/CPP는 UTF-8 BOM 없음. 기존 H/CPP를 수정할 때는 파일별 현재 encoding을 감지해 유지한다.

## G13. Harness exact cases

### Shared geometry cases

```text
GEO01 same vertical capsules -> intersect
GEO02 separated by radius sum + epsilon -> miss
GEO03 exact tangent -> intersect
GEO04 horizontal hit capsule grazing vertical body -> intersect
GEO05 same XZ but separated Y -> miss
GEO06 zero-length hit segment with positive radius -> valid sphere-equivalent
GEO07 NaN endpoint -> invalid
GEO08 radius 0 or > maximum -> invalid
GEO09 yaw 90 local +Z -> world +X
GEO10 capsule tangent uses (radiusSum + lengthEpsilon)^2
GEO11 ambiguous legacy-range + capsule payload -> invalid
```

### Publisher/catalog cases

```text
PUB01 six files, 90 skills, 118 stages -> pass
PUB02 missing one skill timeline -> fail
PUB03 duplicate timelineId/eventId -> fail
PUB04 stageIndex gap or binding count mismatch -> fail
PUB05 end<=start before quantization -> fail
PUB06 last repeat exceeds duration -> fail
PUB07 missing damage/body reference -> fail
PUB08 invalid Client destination promotion -> both outputs rollback
PUB09 runtime-set Validate -> destination hash/mtime unchanged
PUB10 promotion failure at every artifact index -> all artifacts/manifest rollback
PUB11 BalanceTool player/boss reload-save-reload -> capsule fields/version/exact set preserved
PUB12 partial/extra/NaN/zero actor capsule -> staged editor state retained, Save blocked
CAT01 truncated/trailing v5 bootstrap -> fail and old catalog retained
CAT02 v4 bootstrap -> fail; no fallback
MAN00 identical shared canonical-input receipt rows deduplicate; same path with schema/version/hash conflict -> fail
MAN01 manifest + ten SERVER_REQUIRED + three CLIENT_RUNTIME + one CLIENT_DEBUG rows -> hashes/revision pass before room construction
MAN02 old manifest + any promoted new server artifact -> Server start fails before listener
MAN03 missing/bad-hash/duplicate/path-escape manifest row -> Server start fails
ROOM01 Bern/Training spawn-group artifact absent -> ready when gameplay/world match approved revision
ROOM02 Valtan or Character Select spawn-group absent -> not ready
ROOM03 Bern/Training unexpected spawn-group present, even matching revision -> not ready
```

### Server damage cases

```text
HIT00 legacy XZ tall Valtan/Lugaru target set -> current formula parity
HIT01 tick before window -> 0 damage
HIT02 first active tick tangent monster -> 1 damage event
HIT03 same target next active tick -> no duplicate
HIT04 second target inside maxTargets=1 -> stable nearest/ID tie-break
HIT05 Valtan body tangent -> hit
HIT06 vertical separation -> miss
HIT07 target enters later during multi-tick window -> one hit
HIT08 repeat ordinal 2 -> exactly second allowed hit
HIT09 ONCE_PER_STAGE four windows -> stage total one application
HIT10 PER_EVENT_REPEAT four windows -> exactly authored applications
HIT11 action cancel/death/despawn -> ledger cleared
HIT12 combo stage edge -> new stage ledger, old event cannot fire
HIT13 NPC/self/dead target -> excluded
HIT14 damage event cap -> HP still authoritative
HIT15 0/33/34/300/1500/1600/1700/2200ms float32 conversion -> current first evaluation tick parity
HIT16 combo/counter stage 0ms event -> next Update age 0, not missed
HIT17 MAX-1->MAX age1, MAX-1->1 age2, MAX->1 age1 -> window/ledger remains ordered
HIT18 monster chase/attack radius migration -> current AI reach parity
HIT19 frozen pre-admission 118 stages optional old first offset + policy + event count -> converter golden parity
HIT20 HOLD stage 0/1 and COUNTER stage 0 -> NONE, zero hit events, zero damage
HIT21 final graph legacy stages excluding reviewed allowlist -> frozen tuple parity
HIT22 reviewed allowlist (2050210,0) -> hit.01~04 offsets 7/17/26/38, CAPSULE, ONCE stage total one damage
```

### Client debug cases

```text
DBG01 F7 key-down edge -> one toggle
DBG02 held F7 -> no repeat toggle
DBG03 other process focus -> no toggle
DBG04 Release build -> no F7 state/poll/renderer
DBG05 player/world spawn + snapshot -> correct capsule count
DBG06 active hit window -> legacy ring or reviewed capsule present by shape kind
DBG07 outside window -> active hit shape absent
DBG08 despawn/disconnect/level leave -> 0 stale shapes
DBG09 bad debug catalog/hash -> Initialize(nullptr), all replication hooks no-op, overlay unavailable, gameplay untouched
DBG10 one-tick active yellow -> six-tick dim-orange ghost -> removed
DBG11 future actionStartTick -> presentation age 0, no premature hit draw
DBG12 presentation spawn failure -> stable debug identity + diagnostic remains
DBG13 accepted/local revision mismatch -> F7 unavailable, gameplay untouched
DBG14 buffered combo hit tick -> incoming future next-stage snapshot -> old event six-tick ghost
DBG15 final-duration hit tick -> incoming NONE snapshot -> old event six-tick ghost
DBG16 death/early cancel/reset transition -> no carryover ghost
DBG17 equal snapshot tick -> rejected; MAX->1 wrap-forward snapshot -> accepted
```

### Network protocol cases

```text
NET01 S2C_ENTER_ACCEPTED 64-lowercase-hex revision -> roundtrip
NET02 empty/non-hex/wrong-length revision -> writer/reader reject
NET03 truncated/trailing accepted payload -> reject, destination unchanged
NET04 C2S_USE_SKILL field inventory -> skill/aim intent only
```

### Four-class trace cases

```text
TRACE01 DM binding stage 0 -> willowrend clip -> unique authored-baseline EFFECT cue -> four carrier links pass
TRACE02 skillbinding/animevents/effect/materialization hash drift -> fail
TRACE03 cue missing/duplicate/payload/start/anchor/follow/stop drift -> fail
TRACE04 cueStartMs + occurrenceDelayMs quantized offset differs from timeline start -> fail
TRACE05 blocked source has admitted-only field/link or reviewed capsule -> fail
TRACE06 decorative/non-Mesh carrier or source follow policy mismatch -> fail
```

## G14. Public 문서와 audit replacement

`AGENTS.md`, `CLAUDE.md`, `TEAM_GAMEPLAY_INTERFACE_HANDBOOK.md`,
`UNIFIED_DATA_MANAGEMENT_ARCHITECTURE.md`, `ANIMATION_TOOL_OWNER_HANDOFF.md`,
`BALANCE_TOOL_OWNER_HANDOFF.md`의 공식 key/data ownership 문구를 같은 변경에서 통일한다.

```text
공식 전역 기능키는 Debug Developer Tools의 F1, follow/free camera 전환의 F6,
read-only combat collider overlay의 F7뿐이다.
F2~F5와 F8~F12로 level, map, profile, authoring 또는 gameplay 상태를 바꾸지 않는다.
free camera에서는 gameplay command 입력을 보내지 않는다.
F7은 _DEBUG Client presentation만 바꾸고 Server command/collision/damage 권위에는 영향을 주지 않는다.
```

ProjectAudit은 다음 literal/semantic 조건을 검사한다.

- `VK_F7`은 `_DEBUG` MainApp shortcut과 collider overlay 이름 근처에서만 존재한다.
- F7 branch 안에 `Send_`, `Change_Level`, `Apply_LevelRequest`, profile save/reload가 없다.
- `C2S_USE_SKILL`에 target/damage/collider field가 없다.
- `Data/Effects`와 `.animevents` schema에 radius/damage/maximumTargets가 없다.
- combat timeline six-file coverage와 actor body coverage가 완전하다.
- Server/Client runtime source revision이 같다.
- enter-approved revision이 exact 64-hex이고 Client mismatch는 F7만 disable한다.
- default four-class trace는 declared `BLOCKED`를 허용하고 strict admission은 explicit switch에서만 요구한다.

## G15. 적용과 검증 순서

```text
1. effect/HDR/network 세션 merge commit 확인 후 sibling clean worktree 생성
2. 두 PLAN만 같은 경로로 복사하고 SHA-256 일치/current worktree branch-index 불변 확인
3. Shared geometry + harness
4. Data migration + publisher validate/rollback tests
5. Server catalog/body admission
6. Server hit runtime + contract tests
7. Enter-approved runtime revision protocol + NetworkProtocolHarness
8. Debug runtime mirror/view model/renderer + one-tick ghost
9. F7 public key migration + Client frontend tests
10. admitted four-class hit trace; blocked class는 strict gate 전까지 legacy 유지
11. Debug full regression
12. Release full regression
13. Server --contract-test
14. ProjectAudit
15. git diff --check
```

실행 관찰은 자동 이미지 캡처 없이 structured diagnostic와 사용자 직접 화면 확인으로만 한다.

## G16. Valtan attack event와 motion authoring

### 대상

- `Data/Encounters/Valtan/ValtanEncounter.json` formatVersion 4
- 새 `Data/Combat/ReactionProfiles.json`
- 새 `Data/Combat/ValtanHitTracks.json`
- `Client/Public/BalanceTool.h`, `Client/Private/BalanceTool.cpp`
- `Tools/GameplayPipeline/Publish-GameplayBalance.ps1`
- `Server/Public/GameplayCatalog.h`, `Server/Private/GameplayCatalog.cpp`
- `Server/Public/ServerWorldEntity.h`
- `Server/Public/ValtanBrain.h`, `Server/Private/ValtanBrain.cpp`
- `Server/Public/GameRoom.h`, `Server/Private/GameRoom.cpp`
- 새 `Shared/Public/Gameplay/BossCombatContract.h`
- 새 `Shared/Private/BossCombatContract.cpp`

### `BossCombatContract.h` 전체 public contract

```cpp
#pragma once

#include "Gameplay/CombatCollisionContract.h"

#include <cstdint>

namespace LostArk::Shared::BossCombat
{
	inline constexpr std::uint32_t MAXIMUM_BOSS_STAGES_PER_PATTERN = 8u;
	inline constexpr std::uint32_t MAXIMUM_BOSS_HIT_EVENTS_PER_STAGE = 16u;
	inline constexpr std::uint32_t MAXIMUM_BOSS_TARGETS_PER_EVENT = 32u;
	inline constexpr std::uint32_t MAXIMUM_ACTIVE_BOSS_OCCURRENCES = 64u;
	inline constexpr std::uint32_t MAXIMUM_TRACK_SAMPLES = 256u;
	// Current Valtan v3 authoring contains 100-unit wipe radii. The v4
	// lossless migration therefore needs a reviewed public bound above 100.
	inline constexpr float MAXIMUM_BOSS_SHAPE_REACH = 128.f;

	enum class HIT_ANCHOR_POLICY : std::uint8_t
	{
		OWNER_ROOT_AT_EVENT_OPEN,
		OWNER_ROOT_FOLLOW,
		WORLD_POINT_AT_EVENT_OPEN,
		TARGET_POINT_AT_EVENT_OPEN,
		AUTHORED_ROOT_LOCAL_TRACK,
		END
	};

	enum class TARGET_EXTENT_POLICY : std::uint8_t
	{
		LEGACY_ROOT_POINT_XZ,
		HURT_CAPSULE_FOOTPRINT_XZ,
		FULL_HURT_CAPSULE_3D,
		END
	};

	enum class HIT_SHAPE_KIND : std::uint8_t
	{
		CIRCLE,
		RING,
		CONE,
		FORWARD_BOX,
		CROSS,
		CAPSULE,
		CAPSULE_TRACK,
		END
	};

	enum class STAGE_MOTION_KIND : std::uint8_t
	{
		NONE,
		FORWARD_DISTANCE,
		TARGET_SNAPSHOT_DISTANCE,
		END
	};

	struct LOCAL_HIT_SHAPE final
	{
		HIT_SHAPE_KIND eKind = HIT_SHAPE_KIND::END;
		float fLocalCenterX = 0.f;
		float fLocalCenterY = 0.f;
		float fLocalCenterZ = 0.f;
		float fLocalYawDegrees = 0.f;
		float fInnerRadius = 0.f;
		float fOuterRadius = 0.f;
		float fAngleDegrees = 0.f;
		float fLength = 0.f;
		float fForwardStart = 0.f;
		float fForwardEnd = 0.f;
		float fHalfWidth = 0.f;
		CombatCollision::COMBAT_CAPSULE LocalCapsule;
	};

	struct STAGE_MOTION final
	{
		STAGE_MOTION_KIND eKind = STAGE_MOTION_KIND::END;
		float fDistance = 0.f;
		std::uint32_t iDurationTickCount = 0;
		bool bStopOnStaticCollision = false;
		bool bStopOnNavigationFailure = false;
	};

	struct WORLD_HIT_ANCHOR final
	{
		float fX = 0.f;
		float fY = 0.f;
		float fZ = 0.f;
		float fYawDegrees = 0.f;
	};

	bool Is_Valid(const LOCAL_HIT_SHAPE& shape);
	bool Is_Valid(const STAGE_MOTION& motion);

	bool Contains_LegacyRootPointXz(
		const LOCAL_HIT_SHAPE& localShape,
		const WORLD_HIT_ANCHOR& anchor,
		float targetRootX,
		float targetRootZ);

	bool Overlaps_HurtFootprintXz(
		const LOCAL_HIT_SHAPE& localShape,
		const WORLD_HIT_ANCHOR& anchor,
		float targetRootX,
		float targetRootZ,
		float targetRadius);

	bool Overlaps_FullHurtCapsule(
		const LOCAL_HIT_SHAPE& localShape,
		const WORLD_HIT_ANCHOR& anchor,
		const CombatCollision::COMBAT_CAPSULE& targetBody);
}
```

`LOCAL_HIT_SHAPE`는 kind별 exact union으로 검증한다. `CIRCLE/RING/CONE/FORWARD_BOX/CROSS`는 XZ analytic
shape이고 `CAPSULE`은 full 3D primitive다. `CAPSULE_TRACK`은 event row의 `trackId`가 별도 reviewed track을
resolve하므로 inline capsule payload가 zero여야 한다. `LEGACY_ROOT_POINT_XZ`는 현재
`CValtanBrain::ContainsPatternHit`의 root-point 식을 그대로 사용한다. reviewed footprint는 target body radius로
shape region을 Minkowski 확장하고, reviewed weapon capsule은 full capsule-capsule overlap을 사용한다.
`FORWARD_DISTANCE`는 stage-open authoritative yaw를 lock하고 authored distance를 이동한다.
`TARGET_SNAPSHOT_DISTANCE`는 stage-open eligible target point로 XZ direction을 lock한 뒤 실제 target distance와
maximumDistance 중 작은 값을 이동한다. target이 없거나 zero direction이면 stage motion만 cancel하고 pattern
transition은 계속한다.

Valtan stage tick 순서는 `tick-age 계산 -> due anchor capture(pre-motion pose) -> stage motion sweep/commit ->
owner-follow/track shape를 post-motion pose로 구성 -> active event overlap -> existing Try_Counter gate ->
damage/reaction -> occurrence snapshot`으로
고정한다. root/world/target-open anchor는 capture 뒤 boss가 움직여도 고정되고 owner-follow/track만 same-tick moved root를
사용한다.

### `ValtanEncounter.json` v4 exact stage/event schema

root/pattern의 기존 field는 유지한다. stage exact set은 다음으로 교체한다.

```text
stageId, actionId, stageKind, durationMs, motion, hitEvents
```

event exact set은 다음과 같다.

```text
hitEventId, anchorCaptureMs, startMs, endMs, anchorPolicy, anchorLocalOffset,
targetExtentPolicy, shape, maximumTargets, maximumHitsPerTarget,
damageProfileId, reactionProfileId, presentationTraceId
```

모든 nullable 대용 field는 property를 생략하지 않고 빈 문자열을 쓴다. `presentationTraceId`는 audit/Client
presentation join용이며 Server bootstrap에는 쓰지 않는다. 한 stage의 예시는 다음과 같다.

```json
{
  "stageId": "CHARGE",
  "actionId": "valtan.attack.dash-charge.active",
  "stageKind": "ACTIVE",
  "durationMs": 500,
  "motion": {
    "kind": "FORWARD_DISTANCE",
    "distance": 10.0,
    "durationMs": 500,
    "stopOnStaticCollision": true,
    "stopOnNavigationFailure": true
  },
  "hitEvents": [
    {
      "hitEventId": "dash-charge.contact.01",
      "anchorCaptureMs": 0,
      "startMs": 0,
      "endMs": 500,
      "anchorPolicy": "OWNER_ROOT_FOLLOW",
      "anchorLocalOffset": [0.0, 0.0, 0.0],
      "targetExtentPolicy": "HURT_CAPSULE_FOOTPRINT_XZ",
      "shape": {
        "kind": "FORWARD_BOX",
        "localCenter": [0.0, 0.0, 0.0],
        "localYawDegrees": 0.0,
        "forwardStart": 0.0,
        "forwardEnd": 3.5,
        "halfWidth": 2.5
      },
      "maximumTargets": 4,
      "maximumHitsPerTarget": 1,
      "damageProfileId": "damage.valtan.dash-charge",
      "reactionProfileId": "reaction.player.valtan.charge",
      "presentationTraceId": "presentation.valtan.dash-charge.contact"
    }
  ]
}
```

shape exact unions:

```json
[
  { "kind": "CIRCLE", "localCenter": [0,0,0], "outerRadius": 7.0 },
  { "kind": "RING", "localCenter": [0,0,0], "innerRadius": 7.0, "outerRadius": 14.0 },
  { "kind": "CONE", "localCenter": [0,0,0], "localYawDegrees": 0, "angleDegrees": 120, "length": 8.0 },
  { "kind": "FORWARD_BOX", "localCenter": [0,0,0], "localYawDegrees": 0, "forwardStart": 0, "forwardEnd": 10, "halfWidth": 2.5 },
  { "kind": "CROSS", "localCenter": [0,0,0], "localYawDegrees": 0, "length": 10, "halfWidth": 1.8 },
  { "kind": "CAPSULE", "localStart": [0,1,0], "localEnd": [0,1,4], "radius": 1.2 },
  { "kind": "CAPSULE_TRACK", "trackId": "valtan.weapon.swing.reviewed.v1" }
]
```

`targetExtentPolicy × shape.kind` allowed matrix는 다음 exact set이다.

| target extent | allowed shape |
|---|---|
| `LEGACY_ROOT_POINT_XZ` | `CIRCLE`, `RING`, `CONE`, `FORWARD_BOX`, `CROSS` |
| `HURT_CAPSULE_FOOTPRINT_XZ` | 위 analytic 다섯 shape와 static `CAPSULE`의 XZ projection |
| `FULL_HURT_CAPSULE_3D` | `CAPSULE`, `CAPSULE_TRACK` |

analytic shape에는 authored vertical extent가 없으므로 `FULL_HURT_CAPSULE_3D`를 허용하지 않는다. reviewed track은 sample
capsule의 3D overlap만 사용하며 legacy point/footprint로 강등하지 않는다. `Is_Valid`, JSON publisher, bootstrap loader와
debug catalog가 같은 matrix를 사용하고 다른 조합은 전체 stage 전 invalid다.

motion exact unions:

```json
[
  { "kind": "NONE" },
  { "kind": "FORWARD_DISTANCE", "distance": 10, "durationMs": 500, "stopOnStaticCollision": true, "stopOnNavigationFailure": true },
  { "kind": "TARGET_SNAPSHOT_DISTANCE", "maximumDistance": 12, "durationMs": 600, "stopOnStaticCollision": true, "stopOnNavigationFailure": true }
]
```

`0 <= anchorCaptureMs <= startMs < endMs <= durationMs`, `1 <= maximumTargets <= 32`,
`maximumHitsPerTarget=1`을 v4 first cut에서 요구한다. empty hit stage는 `hitEvents=[]`이고 `motion`은 독립적으로
존재할 수 있다. event ID는 encounter 전체에서 unique다. `TARGET_POINT_AT_EVENT_OPEN`은 capture 시점의 eligible
target root에 `anchorLocalOffset`을 더해 저장하고 target이 없으면 그 event만 cancel한다.
`WORLD_POINT_AT_EVENT_OPEN`은 `anchorLocalOffset`을 capture 시점 boss root/yaw로 world 변환해 고정한다.
owner-root 정책도 같은 offset을 root local로 적용한다. `OWNER_ROOT_FOLLOW`은 매 tick current authoritative boss root/yaw를
쓴다. `AUTHORED_ROOT_LOCAL_TRACK`도 event/stage tick의 reviewed sample을 current authoritative owner root/yaw로
world 변환한다. Client animation/root motion이나 bone pose가 Server transform을 대신 소유하지 않는다.
`AUTHORED_ROOT_LOCAL_TRACK`은 `shape.kind=CAPSULE_TRACK`과 non-empty trackId를 요구하고, 다른 anchor policy는
CAPSULE_TRACK을 가질 수 없다. decoder는 JSON shape의 trackId를 catalog event의 `strTrackId`로 분리하고 Shared
primitive payload는 zero/default로 유지한다.

### v3에서 v4로의 lossless converter

- `hitShape=NONE` stage는 `hitEvents=[]`, `motion={kind:NONE}`이다.
- damage stage의 `hitCount=N`은 `N`개의 explicit event로 펼친다. event ID는
  `<lower-pattern-id>.<lower-stage-id>.legacy.<01..N>`으로 persisted해 encounter-global unique를 보장한다.
- ordinal `n`의 start offset은 기존 float32 threshold에 맞춰
  `Convert-MillisecondsToEvaluationOffset(n * hitIntervalMs)`로 개별 계산한다. converted interval을 단순 곱하지
  않는다.
- authoring `startMs=n*hitIntervalMs`, `endMs=min(durationMs,startMs+1)`로 만들고 publisher는 migrated receipt가
  표시된 event의 exclusive end를 `startOffset+1`로 고정한다. `startMs>=durationMs`인 old row는 migration을
  거부한다.
- 각 migrated event는 one-evaluation-tick window, `OWNER_ROOT_AT_EVENT_OPEN`,
  zero `anchorLocalOffset`, `LEGACY_ROOT_POINT_XZ`, `maximumTargets=32`, `maximumHitsPerTarget=1`, empty reaction이다.
- shape 수치는 현재 outer/inner/angle/length/halfWidth를 그대로 옮긴다.
- 모든 v3 stage motion은 `NONE`이다. 실제 이동 charge는 reviewed allowlist에서만 motion과 follow event로 교체한다.

frozen v3→v4 fixture는 31 pattern의 stage count/action ID/duration과 각 pulse의 old first-fire tick, hit target set,
damage profile/적용 횟수를 비교한다. final graph에서는 reviewed allowlist만 별도 golden으로 뺀다.
counter-capable player가 overlap하면 current `CPlayerSkillSystem::Try_Counter` 결과와 target별 damage suppression 순서를
그대로 보존한다. reaction은 counter가 성공하면 queue하지 않는다. 현재 코드는 counter 성공으로 Valtan pattern 자체를
interrupt하지 않으므로 boss pattern/stage는 계속된다. pattern interrupt는 별도 reviewed gameplay change 없이는 추가하지
않는다.

### Gameplay bootstrap v5 추가 row

기존 `PATTERNSTAGE` inline shape row는 제거하고 다음 exact row로 교체한다.

```text
PATTERNSTAGE\t<ENCOUNTER>\t<PATTERN>\t<STAGE_INDEX>\t<STAGE_ID>\t<ACTION_ID>\t<STAGE_KIND>\t<DURATION_MS>\t<MOTION_KIND>\t<HIT_EVENT_COUNT>
BOSSMOTION\t<ENCOUNTER>\t<PATTERN>\t<STAGE_INDEX>\t<KIND>\t<DISTANCE>\t<DURATION_TICKS>\t<STOP_STATIC>\t<STOP_NAV>
BOSSHITEVENT\t<ENCOUNTER>\t<PATTERN>\t<STAGE_INDEX>\t<EVENT_ID>\t<CAPTURE_OFFSET>\t<START_OFFSET>\t<END_OFFSET>\t<ANCHOR_POLICY>\t<ANCHOR_OFFSET_X>\t<ANCHOR_OFFSET_Y>\t<ANCHOR_OFFSET_Z>\t<TARGET_EXTENT_POLICY>\t<SHAPE_KIND>\t<TRACK_ID_OR_EMPTY>\t<CENTER_X>\t<CENTER_Y>\t<CENTER_Z>\t<YAW>\t<INNER_RADIUS>\t<OUTER_RADIUS>\t<ANGLE>\t<LENGTH>\t<FORWARD_START>\t<FORWARD_END>\t<HALF_WIDTH>\t<CAPSULE_START_X>\t<CAPSULE_START_Y>\t<CAPSULE_START_Z>\t<CAPSULE_END_X>\t<CAPSULE_END_Y>\t<CAPSULE_END_Z>\t<CAPSULE_RADIUS>\t<MAX_TARGETS>\t<MAX_HITS_PER_TARGET>\t<DAMAGE_PROFILE>\t<REACTION_PROFILE_OR_EMPTY>
BOSSHITTRACK\t<TRACK_ID>\t<SAMPLE_COUNT>\t<SOURCE_SHA256>
BOSSHITTRACKSAMPLE\t<TRACK_ID>\t<TICK_OFFSET>\t<START_X>\t<START_Y>\t<START_Z>\t<END_X>\t<END_Y>\t<END_Z>\t<RADIUS>
```

catalog loader는 pattern→stage→event/track reference graph 전체를 parse→validate→stage한 뒤 한 번에 commit한다.
v4 bootstrap이나 old inline `PATTERNSTAGE`를 fallback으로 읽지 않는다.

### Server boss motion/occurrence runtime

`ServerWorldEntity.h`에 다음 bounded runtime state를 추가한다.

```cpp
struct SERVER_BOSS_HIT_TARGET_STATE final
{
	LostArk::Shared::NET_ENTITY_ID iTargetNetEntityId =
		LostArk::Shared::INVALID_NET_ENTITY_ID;
	std::uint32_t iAppliedHitCount = 0;
};

struct SERVER_BOSS_HIT_OCCURRENCE final
{
	std::string strHitEventId;
	std::uint32_t iPatternSequence = 0;
	std::uint32_t iStageIndex = 0;
	std::uint32_t iAnchorCaptureTick = 0;
	std::uint32_t iStartDelayTickCount = 0;
	std::uint32_t iDurationTickCount = 0;
	LostArk::Shared::BossCombat::HIT_ANCHOR_POLICY eAnchorPolicy =
		LostArk::Shared::BossCombat::HIT_ANCHOR_POLICY::END;
	LostArk::Shared::BossCombat::WORLD_HIT_ANCHOR CapturedAnchor;
	LostArk::Shared::NET_ENTITY_ID iLockedTargetNetEntityId =
		LostArk::Shared::INVALID_NET_ENTITY_ID;
	std::vector<SERVER_BOSS_HIT_TARGET_STATE> TargetStates;
};

struct SERVER_BOSS_MOTION_RUNTIME final
{
	bool bActive = false;
	bool bStoppedByWorld = false;
	std::uint32_t iPatternSequence = 0;
	std::uint32_t iStageIndex = 0;
	std::uint32_t iStartTick = 0;
	std::uint32_t iDurationTickCount = 0;
	std::uint32_t iAppliedStepCount = 0;
	LostArk::Shared::BossCombat::STAGE_MOTION_KIND eKind =
		LostArk::Shared::BossCombat::STAGE_MOTION_KIND::END;
	LostArk::Shared::NET_ENTITY_ID iLockedTargetNetEntityId =
		LostArk::Shared::INVALID_NET_ENTITY_ID;
	float fStartX = 0.f;
	float fStartY = 0.f;
	float fStartZ = 0.f;
	float fLockedDirectionX = 0.f;
	float fLockedDirectionZ = 0.f;
	float fTotalDistance = 0.f;
};

SERVER_BOSS_MOTION_RUNTIME BossMotionRuntime;
std::vector<SERVER_BOSS_HIT_OCCURRENCE> BossHitOccurrences;
```

stage evaluation age 0에서 motion snapshot을 먼저 만들고 event capture offset 0인 occurrence를 stable event order로
만든다. initial `BeginPattern`은 current tick age 0을 즉시 평가한다. 이전 stage 종료에서 다음 stage로 전환할 때는
`iStartTick=NextNonZeroTick(serverTick)`으로 예약하고 그 tick 전에는 motion/occurrence를 만들지 않으며 다음 Update의 age 0에서
처음 평가한다. 이후 매 tick `captureOffset` crossing에서 occurrence를 생성한다. target/world anchor가 target을 요구하면 기존 `iTargetEntityId`가
eligible일 때 그 ID를 유지하고, 아니면 eligible 후보를 `(distanceSquared,NetEntityId)`로 정렬해 첫 ID를 고른다. chosen
target ID와 world point/direction을 runtime에 저장하고 active 중 다시 고르지 않는다.

motion age는 shared zero-skipping helper로 계산한다. desired cumulative distance는
`totalDistance * min(age+1,duration)/duration`, tick displacement는 desired minus already-applied cumulative다. stage-open
locked direction으로 Server static collision/navigation을 통과해 root를 commit한 뒤 같은 tick `OWNER_ROOT_FOLLOW`
occurrence를 새 root에서 평가한다. collision/nav stop은 `bStoppedByWorld=true`로 terminal 처리하고 재시도하지 않는다.

occurrence는 pending/active/expired를 capture tick + delay + duration으로 derive한다. first cut은
`maximumHitsPerTarget==1`만 publish하고 legacy multi-pulse는 pulse마다 distinct event ID로 migration한다. active tick마다
이미 `TargetStates`에 있는 ID를 query에서 제외하고 `remaining=maximumTargets-TargetStates.size()`만 받는다. candidate
ordering distance는 shape kind와 무관하게 evaluation anchor root/point에서 target root까지의 XZ squared distance로
고정하고 그 뒤 NetEntityId를 쓴다. boolean overlap helper에 존재하지 않는 per-shape distance를 추측하지 않는다.
counter 성공도 target state를 추가해 다음 tick 재타격하지 않는다. event-lifetime 신규 entrant까지 합쳐
maximumTargets를 넘지 않는다.
snapshot `CombatOccurrences`는 이 Server vector의 pending/active row만 projection한다. stage transition, action cancel,
boss death/despawn/reset은 motion과 occurrence/target ledger를 모두 clear한다. legacy parity가 통과한 뒤 단일
`iAppliedPatternHitCount` 경로는 제거하고 두 runtime을 병행하지 않는다.

## G17. Valtan Animation/Effect presentation join

### 소유권과 대상

- 새 `Data/Animation/Authored/Valtan/Valtan.patternbindings.json`
- 새 `Data/Animation/Authored/Valtan/Valtan.weapontracks.json`
- 새 `Data/Animation/Authored/Valtan/ValtanCombatPresentation.trace.json`
- 새 `Data/Effects/Bindings/Valtan.effectbindings.json`
- 새 `Data/Animation/Authored/CombatReactions/CombatReactionBindings.json`
- 새 `Client/Public/BossPatternPresentationDocument.h`
- 새 `Client/Private/BossPatternPresentationDocument.cpp`
- 새 `Client/Public/CombatReactionPresentationCatalog.h`
- 새 `Client/Private/CombatReactionPresentationCatalog.cpp`
- `Client/Public/Animation_Tool.h`, `Client/Private/Animation_Tool.cpp`
- effect 세션 merge 뒤 `Client/Public/Effect_Tool.h`, `Client/Private/Effect_Tool.cpp`
- effect 세션 merge 뒤 `Client/Public/Effect_DocumentRenderer.h`, `Client/Private/Effect_DocumentRenderer.cpp`
- `Client/Public/Valtan.h`, `Client/Private/Valtan.cpp`
- effect 세션 merge 뒤 `Effect_PresentationService.*`와 새 `IEffectAnchorProvider`

Animation Tool은 pattern action→실제 clip, presentation cue와 bone 측정 seed를 소유한다. Balance Tool은
`ValtanHitTracks.json`의 reviewed gameplay primitive를 소유한다. Effect Tool은 effect asset resource/timeline/local
transform을 소유한다. 세 문서는 stable action/stage/event/trace ID와 hash receipt로만 join한다.

### `Valtan.patternbindings.json` exact schema

```json
{
  "schema": "lostark.valtan-pattern-bindings",
  "formatVersion": 1,
  "actorAssetId": "Boss_Valtan",
  "fixedTickHz": 30,
  "bindings": [
    {
      "actionId": "valtan.attack.dash-charge.active",
      "clips": [
        {
          "sequenceIndex": 0,
          "clipId": "att_battle_4_02",
          "startOffsetMs": 0,
          "playRate": 1.0,
          "loop": false
        }
      ],
      "cues": [
        {
          "presentationCueId": "cue.valtan.dash-charge.contact",
          "startMs": 0,
          "effectBindingId": "effect-binding.valtan.dash-charge.contact"
        }
      ]
    }
  ]
}
```

root exact fields는 `schema,formatVersion,actorAssetId,fixedTickHz,bindings`다. binding은
`actionId,clips,cues`, clip은 위 다섯 field, cue는 정확히 `presentationCueId,startMs,effectBindingId`다. pattern stage action ID는 정확히 한
binding을 가져야 한다. clip sequenceIndex는 0부터 contiguous, clip/model에 실제 존재, finite positive playRate,
trimmed total duration이 encounter stage presentation 범위를 덮어야 한다. missing/corrupt binding은 해당 Client action
표현만 generic BossCatalog fallback으로 격리하며 Server spawn/pattern을 막지 않는다.

Animation Tool은 cue identity와 action-relative `startMs`만 저장한다. effect asset, anchor/local transform/follow/stop과
runtime trigger policy는 Effect Tool 단일 writer의 다음 문서가 소유한다.

```json
{
  "schema": "lostark.valtan-effect-bindings",
  "formatVersion": 1,
  "actorAssetId": "Boss_Valtan",
  "bindings": [
    {
      "effectBindingId": "effect-binding.valtan.dash-charge.contact",
      "effectAssetId": "effect.valtan.dash-charge.contact",
      "runtimeTriggerPolicy": "SERVER_OCCURRENCE",
      "triggerPhase": "ACTIVE_START",
      "playbackRate": 1.0,
      "anchorKind": "SERVER_EVENT_ANCHOR",
      "anchorName": "",
      "localPosition": [0.0, 0.0, 2.5],
      "localYawDegrees": 0.0,
      "follow": true,
      "stopPolicy": "OCCURRENCE_END"
    }
  ]
}
```

root exact fields는 `schema,formatVersion,actorAssetId,bindings`, binding은 위 열한 field exact set이다. key
`effectBindingId`는 unique하며 Animation cue는 정확히 한 Effect binding을 참조한다. `SERVER_OCCURRENCE`는 trace-linked
combat cue에만, `ANIMATION_CUE`는 decorative cue에만 허용한다. finite `(0,16]` `playbackRate`는 Effect binding이
소유하며 action clip playRate를 암묵 상속하지 않는다. `SERVER_OCCURRENCE`의 `triggerPhase`는 `CAPTURE` 또는
`ACTIVE_START`, `ANIMATION_CUE`는 JSON null만 허용하며
animation cue의 quantized start는 각각 encounter event capture offset 또는 active start offset과 tolerance 안에서
일치해야 한다. 두 Tool은 서로의 문서를 save하지 않는다.

`CValtan::Apply_NetworkState`는 `strActionId`, pattern sequence, stage index, action start tick과 current server tick을
받아 binding을 resolve한다. action identity가 바뀌면 clip sequence를 시작하고 동일 identity snapshot이면
`ForwardTickDistanceSkippingZero`로 presentation age를 seek/resync한다. Client local dt만 누적해 Server stage와
영구 drift시키지 않는다.

### `Valtan.weapontracks.json` 측정 seed

```json
{
  "schema": "lostark.valtan-animation-weapon-tracks",
  "formatVersion": 1,
  "actorAssetId": "Boss_Valtan",
  "tracks": [
    {
      "sourceTrackId": "valtan.weapon.swing.seed.v1",
      "actionId": "valtan.attack.swing.active",
      "clipId": "att_battle_1_02",
      "boneName": "b_wp_r_01",
      "modelSha256": "<64-lowercase-hex>",
      "bindingDocumentSha256": "<64-lowercase-hex>",
      "bindingSequenceIndex": 0,
      "bindingStartOffsetMs": 0,
      "bindingPlayRate": 1.0,
      "measurementModelScale": 0.0001,
      "measurementModelYawDegrees": -90.0,
      "sampleHz": 30,
      "samples": [
        { "tickOffset": 0, "rootLocalPosition": [0.0, 1.8, 1.2] },
        { "tickOffset": 1, "rootLocalPosition": [0.8, 1.7, 1.8] }
      ]
    }
  ]
}
```

이 문서는 Animation Tool이 actual clip/bone matrix에서 측정한 point track이며 Server는 읽지 않는다. sample은 finite,
tick offset strictly increasing, sample count <= 256이다. collider radius나 damage가 없다. 현재 pipeline에 canonical
per-clip content hash가 없으므로 존재하지 않는 `clipSha256` 계약을 만들지 않는다. 대신 model content hash, exact clip ID,
binding document hash와 sequence/startOffset/playRate/sampleHz/model-scale/model-yaw tuple이 측정 timebase의 정본이다.

Animation Tool bake는 action sample wall seconds에서 앞 sequence clip의
`trimmedClipDurationSeconds/playRate`를 순서대로 빼 현재 clip을 고른다. 현재 clip local wall age에 대해
`clipSeconds=startOffsetMs/1000.f + localWallAgeSeconds*playRate`, model track position은
`clipSeconds*CModel::Get_AnimationTickPerSecond(animIndex)`다. clip ID는 bounded animation index scan에서
`Get_AnimationName(index)`와 exact unique match로 resolve하고 missing/duplicate를 거부한다. 같은 index로
`Set_Animation`/`Get_AnimationTickPerSecond`/`Set_AnimTrackPosition`을 호출한 뒤 model/bone combined matrix update를 정확히
한 번 수행하고 그 다음 `b_wp_r_01`을 body visual-root composition으로 읽는다. sequence boundary, non-1 playRate,
non-zero startOffset과 30 Hz sample tick의 golden을 둔다.

### `ValtanHitTracks.json` reviewed gameplay track

```json
{
  "schema": "lostark.valtan-reviewed-hit-tracks",
  "formatVersion": 1,
  "fixedTickHz": 30,
  "tracks": [
    {
      "trackId": "valtan.weapon.swing.reviewed.v1",
      "sourceTrackId": "valtan.weapon.swing.seed.v1",
      "sourceDocumentSha256": "<64-lowercase-hex>",
      "sourceModelSha256": "<64-lowercase-hex>",
      "sourceBindingDocumentSha256": "<64-lowercase-hex>",
      "sourceClipId": "att_battle_1_02",
      "sourceBindingActionId": "valtan.attack.swing.active",
      "sourceBindingSequenceIndex": 0,
      "sourceBindingStartOffsetMs": 0,
      "sourceBindingPlayRate": 1.0,
      "sourceMeasurementModelScale": 0.0001,
      "sourceMeasurementModelYawDegrees": -90.0,
      "sourceSampleHz": 30,
      "sourceBoneName": "b_wp_r_01",
      "reviewStatus": "APPROVED",
      "samples": [
        {
          "tickOffset": 0,
          "localStart": [0.0, 1.8, 0.6],
          "localEnd": [0.0, 1.8, 1.8],
          "radius": 0.7
        }
      ]
    }
  ]
}
```

Balance Tool의 `Import Animation Seed`는 source document를 read-only로 stage하고 capsule seed를 보여준 뒤 사용자가
승인한 수치만 이 gameplay 문서에 저장한다. seed document, model content, binding document 또는 exact
`clipId/sequenceIndex/startOffsetMs/playRate/sampleHz/modelScale/modelYaw` tuple이 drift하면 `APPROVED` track publish를
실패시킨다.
Animation Tool이 이 파일을 직접 save하지 않는다.

sample `tickOffset`은 binding action/stage start 기준 0-based offset이다. `CAPSULE_TRACK` event의 모든 active tick을
strictly increasing sample 범위가 덮어야 한다. 이전/현재 capsule 두 개의 단순 union을 continuous sweep으로 가장하지
않고 다음 bounded deterministic conservative subdivision을 Shared helper 하나로 고정한다.

```cpp
inline constexpr std::uint32_t MAXIMUM_TRACK_SWEEP_SUBDIVISIONS = 16u;

bool Swept_CapsuleTrackIntersects(
	const CombatCollision::COMBAT_CAPSULE& previous,
	const CombatCollision::COMBAT_CAPSULE& current,
	const CombatCollision::COMBAT_CAPSULE& targetWorld);
```

각 substep의 허용 endpoint 이동은 `min(previous.radius,current.radius) * 0.5f`다. 두 endpoint 중 큰 이동량을 그 값으로
나눈 ceil이 subdivision 수이며 1..16을 벗어나면 publisher가 reviewed track을 거부한다. 각 substep은 보간된 blade
capsule과 start-endpoint 이동 connector capsule, end-endpoint 이동 connector capsule 세 개를 target capsule과 검사한다.
회전하는 선분의 정확한 연속충돌을 주장하지 않으며 이 공개 오차/상한 안의 보수적 판정만 지원한다. sample 간격,
radius 또는 binding timebase가 상한을 넘으면 runtime clamp로 누락시키지 않고 authoring을 더 촘촘히 bake하게 한다.

### presentation trace

```json
{
  "schema": "lostark.valtan-combat-presentation-trace",
  "formatVersion": 1,
  "links": [
    {
      "presentationTraceId": "presentation.valtan.dash-charge.contact",
      "patternId": "VALTAN_DASH_CHARGE",
      "stageId": "CHARGE",
      "hitEventId": "dash-charge.contact.01",
      "bindingActionId": "valtan.attack.dash-charge.active",
      "presentationCueId": "cue.valtan.dash-charge.contact",
      "effectBindingId": "effect-binding.valtan.dash-charge.contact",
      "effectAssetId": "effect.valtan.dash-charge.contact",
      "triggerPhase": "ACTIVE_START",
      "timingToleranceTicks": 1,
      "anchorReview": "OWNER_ROOT_FOLLOW"
    }
  ]
}
```

trace audit은 encounter event, exact binding/cue/effect asset, trigger phase, quantized trigger tick과 anchor 정책을
검증한다. `CAPTURE`는 anchor capture tick, `ACTIVE_START`는 event start tick과 비교한다. trace와 cue는
presentation join이고 Server bootstrap에는 들어가지 않는다. missing effect는 cue만 격리한다.

### Combat reaction presentation binding

Animation Tool은 player class와 monster archetype의 실제 model clip을 골라 다음 Client presentation 문서를 저장한다.

```json
{
  "schema": "lostark.combat-reaction-presentation-bindings",
  "formatVersion": 1,
  "bindings": [
    {
      "presentationReactionId": "reaction.player.knockdown.heavy",
      "targetKind": "PLAYER_CLASS",
      "targetId": "LANCE_MASTER",
      "clipId": "<tool-selected-existing-clip-id>",
      "playRate": 1.0,
      "startOffsetMs": 0,
      "returnPolicy": "LATEST_SERVER_ACTION"
    },
    {
      "presentationReactionId": "reaction.monster.player-skill.light",
      "targetKind": "MONSTER_ARCHETYPE",
      "targetId": "MONSTER_VALTAN_PADD_01",
      "clipId": "<tool-selected-existing-clip-id>",
      "playRate": 1.0,
      "startOffsetMs": 0,
      "returnPolicy": "LATEST_SERVER_ACTION"
    }
  ]
}
```

angle-bracket clip value는 wire-format placeholder가 아니라 Animation Tool에서 해당 model의 실제 clip 선택을 저장하는
자리다. publisher 대상 제품 문서에는 이 literal을 허용하지 않는다. exact fields는 root
`schema,formatVersion,bindings`, row의 위 일곱 개다. key `(presentationReactionId,targetKind,targetId)`는 unique이고
clip/model join, finite positive playRate, valid start offset을 strict 검증한다. Server는 이 문서를 읽지 않는다.

`CCombatReactionPresentationCatalog`는 parse→validate→stage→commit 후 다음 lookup을 제공한다.

```cpp
const COMBAT_REACTION_PRESENTATION_BINDING* Find(
	std::string_view presentationReactionId,
	REACTION_PRESENTATION_TARGET_KIND targetKind,
	std::string_view targetId) const;
```

`CClientReplication` accepted snapshot에서 reaction sequence가 바뀌면 local player/remote player의 `CCharacter`에는
class ID, monster `CNpc`에는 archetype ID와 presentationReactionId를 typed callback으로 전달한다. reaction clip이
Server duration 동안 presentation priority를 가지며 끝나면 가장 최근 snapshot action clip으로 복귀한다. duplicate
sequence는 replay하지 않는다. missing binding/clip은 diagnostic 후 기존 action pose를 유지하고 authoritative
forced-motion root는 계속 반영한다. Valtan은 `ALL` reaction immune이므로 별도 hit flash만 허용하고 root reaction clip
coverage의 필수 대상이 아니다.

late snapshot은 `Try_GetForwardTickDistanceSkippingZero(startTick,currentServerTick)`로 age를 구한다. `age>=duration`이면
reaction clip을 시작하지 않고 latest Server action pose를 유지한다. 그 전이면 model sample position을
`startOffsetMs/1000.f + (age/30.f)*playRate`로 seek하고 priority wall lifetime은 `(duration-age)/30.f`만 남긴다. Client
local dt로 frame 0부터 다시 시작하거나 playback rate를 wall lifetime에 두 번 곱하지 않는다. wall/nav block 등으로
Server가 motion을 일찍 끝내 다음 accepted snapshot이 `Reaction=NONE`이면 scheduled end를 기다리지 않고 reaction priority를
즉시 clear해 latest Server action으로 복귀한다.

`fPresentationArcHeight`는 authoritative actor root, navigation, hurt collider에 더하지 않는다. accepted Server tick age로
`u=clamp(age/duration,0,1)`, `visualOffsetY=4*arcHeight*u*(1-u)`를 계산한다. `CCharacter`는 gameplay world와 별도
`m_ReactionPresentationParentMatrix = Translation(0,visualOffsetY,0) * authoritativeWorld`를 두고 Body와 모든 skinned
equipment/socket part의 `pParentMatrix`가 이 presentation parent를 함께 참조하게 한다. `CNpc`는 render/model world에만
같은 offset parent를 합성한다. gameplay world, collider/navigation, explicit Server effect anchor에는 적용하지 않는다.
local dt를 누적해 포물선을 적분하지 않는다. `Reaction=NONE`, higher-priority replacement, death, despawn, disconnect와
level clear에서 offset과 presentation parent를 즉시 authoritative world로 되돌린다. late snapshot은 같은 식으로 중간
offset에서 시작하고 early Server stop은 다음 accepted `NONE`에서 즉시 0이 된다.

제품 coverage는 nonfatal runtime fallback과 별개로 publisher/ProjectAudit에서 fail closed한다. Valtan/monster→player로
참조되는 모든 presentationReactionId는 여섯 `PLAYER_CLASS` binding을 모두 가져야 한다. player skill→monster로
참조되는 모든 presentationReactionId는 `MonsterCatalog.json`의 네 supported combat archetype binding을 모두 가져야
한다. selected clip은 각 target model에 실제 존재해야 하고 angle-bracket/example literal, empty clip, generic silent
fallback은 제품 publish에서 거부한다.

### Effect anchor provider

새 interface는 전체 public contract를 다음으로 고정한다.

```cpp
#pragma once

#include "Network/NetworkIds.h"

#include <DirectXMath.h>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>

class IEffectAnchorProvider
{
public:
	virtual ~IEffectAnchorProvider() = default;
	virtual bool Try_GetEffectRootWorld(
		DirectX::XMFLOAT4X4& outWorld) const = 0;
	virtual bool Try_GetEffectBoneWorld(
		std::string_view boneName,
		DirectX::XMFLOAT4X4& outWorld) const = 0;
};

enum class EFFECT_RUNTIME_ANCHOR_KIND : std::uint8_t
{
	OWNER_ROOT,
	OWNER_BONE,
	EXPLICIT_SERVER_WORLD,
	END
};

enum class EFFECT_RUNTIME_TRIGGER_POLICY : std::uint8_t
{
	ANIMATION_CUE,
	SERVER_OCCURRENCE,
	END
};

enum class EFFECT_RUNTIME_TRIGGER_PHASE : std::uint8_t
{
	NONE,
	CAPTURE,
	ACTIVE_START,
	END
};

enum class EFFECT_RUNTIME_STOP_POLICY : std::uint8_t
{
	ACTION_END,
	OCCURRENCE_END,
	NATURAL_END,
	END
};

struct EFFECT_OCCURRENCE_KEY final
{
	LostArk::Shared::NET_ENTITY_ID iSourceNetEntityId =
		LostArk::Shared::INVALID_NET_ENTITY_ID;
	std::uint32_t iPatternSequence = 0;
	std::uint32_t iStageIndex = 0;
	std::string strHitEventId;
};

struct EFFECT_RUNTIME_ANCHOR final
{
	EFFECT_RUNTIME_ANCHOR_KIND eKind = EFFECT_RUNTIME_ANCHOR_KIND::END;
	std::weak_ptr<IEffectAnchorProvider> pOwner;
	std::string strBoneName;
	DirectX::XMFLOAT4X4 ExplicitWorld{};
};

struct EFFECT_RUNTIME_SPAWN_DESC final
{
	EFFECT_RUNTIME_TRIGGER_POLICY eTriggerPolicy =
		EFFECT_RUNTIME_TRIGGER_POLICY::END;
	EFFECT_RUNTIME_TRIGGER_PHASE eTriggerPhase =
		EFFECT_RUNTIME_TRIGGER_PHASE::END;
	EFFECT_RUNTIME_STOP_POLICY eStopPolicy =
		EFFECT_RUNTIME_STOP_POLICY::END;
	EFFECT_RUNTIME_ANCHOR Anchor;
	EFFECT_OCCURRENCE_KEY OccurrenceKey;
	std::string strEffectBindingId;
	std::string strEffectAssetId;
	std::string strActionId;
	std::string strCueInstanceId;
	std::uint32_t iActionStartTick = 0;
	DirectX::XMFLOAT4X4 LocalTransform{};
	bool bFollow = false;
	float fPlaybackRate = 1.f;
	float fRequestedDurationSeconds = 0.f;
	float fInitialSampleTimeSeconds = 0.f;
	float fRemainingWallLifetimeSeconds = 0.f;
};
```

`CCharacter`와 `CValtan`이 provider를 구현한다. `b_wp_r_01`은 Client effect/trail anchor로만 사용한다.
`CValtan::Try_GetEffectBoneWorld`는 bone을 actor root에 바로 곱하지 않는다. `CBody_Valtan`의 model combined bone matrix에
현재 body visual-root local transform(Y -90도와 model scale tuple)을 곱하고, 다시 parent actor world를 곱하는 기존
`CPart_Equipment`의 `b_wp_r_01` composition을 그대로 호출/공유한다. Animation weapon seed의 measurement scale/yaw도 이
tuple을 소비하며 장비 socket world와 effect provider world가 epsilon 안에서 같은지 harness로 고정한다.
`TARGET/WORLD_POINT_AT_EVENT_OPEN` effect는 Server occurrence snapshot의 `ExplicitWorld`를 사용하고 Client target/current
boss pose로 재계산하지 않는다. 이 변경은 현재 effect 세션이 merge된 뒤 clean sibling worktree에서만 적용한다.

`ACTIVE_EFFECT` lifecycle은 anchor union과 같은 변경에서 닫는다. owner root/bone provider 만료와 level/disconnect teardown은
모든 policy에 적용하는 안전 정리 edge다. 그 밖의 정상 terminal은 exact policy로 나눈다.

- `OCCURRENCE_END`: full occurrence key가 accepted snapshot에서 사라지거나 event가 cancel될 때 stop한다.
- `ACTION_END`: source action ID/start tick identity가 바뀌거나 action이 끝날 때 stop한다.
- `NATURAL_END`: occurrence/action 종료로 강제 stop하지 않고 document wall lifetime만 소비한다.

explicit Server world는 weak owner 대신 copied matrix와 `EFFECT_OCCURRENCE_KEY`를 소유한다. active effect dedup key는
`(EFFECT_OCCURRENCE_KEY,effectBindingId,triggerPhase)`이므로 같은 snapshot은 replay하지 않되 한 hit event에 명시된 서로
다른 VFX binding은 각각 재생한다. `NATURAL_END` effect가 occurrence보다 먼저 끝나도 반복 snapshot으로 다시 spawn하지
않도록 consumed-phase ledger를 occurrence disappearance까지 유지한다. `ACTION_END` animation cue ledger도 action identity
교체까지 유지한다. trace publisher는 `(pattern,stage,hitEventId,effectBindingId,triggerPhase)` unique와 binding 존재를
강제한다.

trace-linked combat effect의 제품 trigger owner는 `SERVER_OCCURRENCE` edge 하나뿐이다. 같은 Animation cue는
preview/audit timebase로 남고 runtime binding이 `SERVER_OCCURRENCE`면 animation edge에서 spawn하지 않는다. combat
trace가 없는 decorative cue만 `ANIMATION_CUE`로 재생한다. activation catalog 또는 Valtan spawn admission 때 trace-linked
effect asset/prototype을 batch prewarm하고 missing asset은 presentation diagnostic으로 격리하되 Server spawn과 damage를
롤백하지 않는다. Effect Tool selector에는 playback-only Valtan을 명시적으로 preview 대상으로 허용하지만 playable
character로 승격시키지 않으며 collider/damage save UI는 추가하지 않는다.

`SERVER_OCCURRENCE` spawn tick은 Effect binding의 `triggerPhase`로 고른다. `CAPTURE`는 anchor capture tick,
`ACTIVE_START`는 capture tick을 start-delay만큼 zero-skip advance한 tick이다. snapshot을 늦게 받은 Client는 shared
forward-distance로 trigger age를 구해 `fInitialSampleTimeSeconds=(age/30.f)*playbackRate`로 seek한다.
`fRemainingWallLifetimeSeconds`는 playback sample time과 분리한다. `CAPTURE`의 wall lifetime tick은 overflow-checked
`startDelayTickCount + durationTickCount`, `ACTIVE_START`는 `durationTickCount`이며 각 trigger-relative age를 뺀다.
`CEffectDocumentRenderer::PREPARED_DOCUMENT`에 catalog stage 때 `CEffectPlayback::Get_DurationSeconds()`와 같은 validated
`fDocumentSampleDurationSeconds` metadata를 복사하고 `CEffectObject`/presentation service에 read-only로 전달한다. service가
Effect JSON을 다시 읽거나 duration을 별도 계산하지 않는다. 이 prepared sample duration을
`documentSampleDurationSeconds`, binding 값을 `playbackRate`라 할 때
natural remaining wall time은
`max(0,(documentSampleDurationSeconds-fInitialSampleTimeSeconds)/playbackRate)`다. `NATURAL_END`는 이 값만,
`OCCURRENCE_END`는 이 값과 위 occurrence remaining의 min, `ACTION_END`는 이 값과 action-binding clip sequence의
remaining wall time min을 descriptor deadline으로 쓴다. occurrence duration을 `NATURAL_END` lifetime으로 재사용하지 않는다.
remaining이 0이면 spawn하지 않는다. `fRequestedDurationSeconds`는 prepared document의 natural remaining,
`fRemainingWallLifetimeSeconds`는 policy-resolved deadline이다. `LocalTransform`, follow/stop, duration, playback rate와 기존
action/cue identity는 Effect binding을 resolve한 full descriptor로 service에 전달하며 Client가 current target pose로 Server
anchor를 재계산하지 않는다.

descriptor validation은 trigger-policy discriminated union이다. `ANIMATION_CUE`는 non-expired owner,
non-zero `iActionStartTick`, action ID와 cue instance를 요구하고 dedup key를 `(owner identity,actionStartTick,cueInstance)`로
만들며 `eTriggerPhase=NONE`만 허용한다. `SERVER_OCCURRENCE`는 full occurrence key와 effect binding ID,
`CAPTURE|ACTIVE_START` phase를 요구하며 action fields는 audit/stop용일 뿐 dedup 정본이 아니다. `ACTIVE_EFFECT`와
consumed ledger도 `eTriggerPhase`를 저장한다. 두 policy가 서로의 fallback key를 사용하지 않는다.
stop-policy validation은 trigger-policy와 별도의 discriminated condition이다. `ACTION_END`면 어느 trigger policy든
non-empty bounded `strActionId`와 non-zero `iActionStartTick`이 필수이고 source의 accepted action identity를 descriptor에
복사한다. `SERVER_OCCURRENCE + ACTION_END`가 empty/0이면 fallback terminal을 추측하지 않고 그 presentation spawn만
거부한다. `OCCURRENCE_END`는 full occurrence key, `NATURAL_END`는 validated prepared document duration을 요구한다.

Effect binding `SERVER_EVENT_ANCHOR` mapping도 exact union이다. occurrence policy가 `OWNER_ROOT_FOLLOW`면 source
`CValtan` provider의 runtime `OWNER_ROOT`와 `follow=true`로 resolve한다. `OWNER_ROOT_AT_EVENT_OPEN`,
`WORLD_POINT_AT_EVENT_OPEN`, `TARGET_POINT_AT_EVENT_OPEN`은 captured matrix를 `EXPLICIT_SERVER_WORLD`, `follow=false`로
resolve한다. `AUTHORED_ROOT_LOCAL_TRACK`은 `SERVER_EVENT_ANCHOR` effect binding을 거부한다. weapon trail처럼 bone을
따라야 하는 visual은 explicit authoring `anchorKind=OWNER_BONE`, `anchorName=b_wp_r_01`, `ANIMATION_CUE` 또는 reviewed
occurrence binding 정책을 사용한다. fixed ground effect를 current owner pose로 다시 계산하거나 owner-follow effect를
capture matrix에 고정하지 않는다.

## G18. Reaction profile과 Server forced motion

### `CombatReactionContract.h` 전체 public contract

```cpp
#pragma once

#include "Network/NetworkIds.h"

#include <cstdint>
#include <string>

namespace LostArk::Shared::CombatReaction
{
	inline constexpr float MAXIMUM_REACTION_DISTANCE = 32.f;
	inline constexpr std::uint32_t MAXIMUM_REACTION_DURATION_TICKS = 180u;

	enum class REACTION_KIND : std::uint8_t
	{
		NONE,
		STAGGER,
		KNOCKBACK,
		KNOCKDOWN,
		END
	};

	enum class DIRECTION_POLICY : std::uint8_t
	{
		SOURCE_TO_TARGET,
		ATTACK_FORWARD,
		HIT_VOLUME_FORWARD,
		END
	};

	enum class INTERRUPT_POLICY : std::uint8_t
	{
		CANCEL_ACTION,
		END
	};

	enum class BLOCKED_POLICY : std::uint8_t
	{
		STOP_AT_FIRST_BLOCK,
		END
	};

	enum class REACTION_IMMUNITY : std::uint8_t
	{
		NONE,
		DISPLACEMENT,
		ALL,
		END
	};

	struct PROFILE final
	{
		std::string strReactionProfileId;
		REACTION_KIND eKind = REACTION_KIND::END;
		DIRECTION_POLICY eDirectionPolicy = DIRECTION_POLICY::END;
		float fDistance = 0.f;
		std::uint32_t iDurationTickCount = 0;
		float fPresentationArcHeight = 0.f;
		INTERRUPT_POLICY eInterruptPolicy = INTERRUPT_POLICY::END;
		BLOCKED_POLICY eBlockedPolicy = BLOCKED_POLICY::END;
		std::string strPresentationReactionId;
	};

	struct SNAPSHOT final
	{
		REACTION_KIND eKind = REACTION_KIND::NONE;
		std::uint32_t iSequence = 0;
		std::uint32_t iStartTick = 0;
		std::uint32_t iDurationTickCount = 0;
		float fDirectionX = 0.f;
		float fDirectionZ = 0.f;
		float fPresentationArcHeight = 0.f;
		std::string strPresentationReactionId;
	};

	bool Is_Valid(const PROFILE& profile);
	bool Is_Valid(const SNAPSHOT& snapshot);
int Priority(REACTION_KIND kind);
}
```

새 `Shared/Private/CombatReactionContract.cpp`는 finite/range/exclusive enum 검증, active/inactive snapshot union과
`KNOCKDOWN > KNOCKBACK > STAGGER > NONE` priority를 구현한다. presentation ID의 bounded UTF-8 검증은 packet
codec과 publisher가 담당한다.

`NONE`은 snapshot inactive sentinel이고 profile에서는 금지한다. `DISPLACEMENT`는 translation만 막고 명시적
STAGGER/interrupt를 허용할 archetype에만 사용한다. `ALL`은 reaction admission과 action/path cancel을 모두 거부하되
damage와 별도 hit-flash event는 그대로 허용한다. Valtan first-cut 기본값은 `ALL`, 일반 monster/player는 `NONE`이다.
inactive snapshot은 empty presentation ID, active snapshot은 admitted profile의 bounded non-empty
`presentationReactionId`를 가진다. Client는 kind만으로 clip을 추측하지 않는다.

### `ReactionProfiles.json` exact schema

```json
{
  "schema": "lostark.combat-reaction-profiles",
  "formatVersion": 1,
  "fixedTickHz": 30,
  "profiles": [
    {
      "reactionProfileId": "reaction.player.valtan.charge",
      "reactionKind": "KNOCKDOWN",
      "directionPolicy": "ATTACK_FORWARD",
      "distance": 6.0,
      "durationMs": 450,
      "presentationArcHeight": 0.35,
      "interruptPolicy": "CANCEL_ACTION",
      "blockedPolicy": "STOP_AT_FIRST_BLOCK",
      "presentationReactionId": "reaction.player.knockdown.heavy"
    },
    {
      "reactionProfileId": "reaction.player.valtan.heavy",
      "reactionKind": "KNOCKBACK",
      "directionPolicy": "ATTACK_FORWARD",
      "distance": 3.0,
      "durationMs": 300,
      "presentationArcHeight": 0.2,
      "interruptPolicy": "CANCEL_ACTION",
      "blockedPolicy": "STOP_AT_FIRST_BLOCK",
      "presentationReactionId": "reaction.player.knockback.heavy"
    },
    {
      "reactionProfileId": "reaction.player.monster.basic",
      "reactionKind": "KNOCKBACK",
      "directionPolicy": "SOURCE_TO_TARGET",
      "distance": 1.0,
      "durationMs": 200,
      "presentationArcHeight": 0.1,
      "interruptPolicy": "CANCEL_ACTION",
      "blockedPolicy": "STOP_AT_FIRST_BLOCK",
      "presentationReactionId": "reaction.player.knockback.light"
    },
    {
      "reactionProfileId": "reaction.monster.player-skill.light",
      "reactionKind": "KNOCKBACK",
      "directionPolicy": "SOURCE_TO_TARGET",
      "distance": 1.25,
      "durationMs": 180,
      "presentationArcHeight": 0.0,
      "interruptPolicy": "CANCEL_ACTION",
      "blockedPolicy": "STOP_AT_FIRST_BLOCK",
      "presentationReactionId": "reaction.monster.player-skill.light"
    }
  ]
}
```

duration은 `Convert-MillisecondsToIntervalTicks`로 positive tick count가 되고 public cap 이하여야 한다. distance와 arc는
finite/non-negative이며 STAGGER는 distance 0, KNOCKBACK/KNOCKDOWN은 positive distance다. ID unique, exact-property,
unknown enum/duplicate/NaN은 전체 publish를 실패시킨다.

pure legacy converter는 player/Valtan event의 reaction ID를 empty로 두고 frozen target/damage/tick parity를 먼저 증명한다.
그 다음 final authoring graph에 다음 `PROJECT_TUNED` overlay를 명시적으로 적용하고 provenance receipt도 같은 변경에서
갱신한다.

- 모든 damage-capable player legacy event는 `reaction.monster.player-skill.light`를 참조한다. actor profile의 Valtan
  `ALL` immunity 때문에 boss pattern은 cancel되지 않고 일반 monster만 반응한다.
- pure v3→v4 Valtan damage event는 `reaction.player.valtan.heavy`, reviewed dash-charge contact는
  `reaction.player.valtan.charge`를 참조한다.
- 네 MonsterProfiles row는 모두 `attackReactionProfileId=reaction.player.monster.basic`을 사용하고 stable hit ID는 각각
  `monster.valtan-padd-01.basic-attack.contact`, `monster.valtan-sjfc-00-4.basic-attack.contact`,
  `monster.valtan-0019-05.basic-attack.contact`, `monster.lugaru.basic-attack.contact`다.

위 overlay allowlist를 final golden에서 별도 비교한다. pure migration golden은 reaction 전 상태만 lossless라고 부르고,
final KB gameplay change를 legacy parity라고 오기하지 않는다. referenced profile마다 target presentation coverage가 여섯
class/네 monster archetype에 완전해야 publish/audit가 통과한다.

PlayerProfiles/MonsterProfiles/BossProfiles 각 actor row에는 다음 exact object를 추가한다.

```json
{
"combatReaction": {
  "receivedKnockbackScale": 1.0,
  "immunity": "NONE"
}
}
```

여섯 player profile과 regular monster `MONSTER_VALTAN_PADD_01`, `MONSTER_VALTAN_SJFC_00_4`,
`MONSTER_VALTAN_0019_05`는 scale 1.0/immunity `NONE`, `MINIBOSS_LUGARU`는 PROJECT_TUNED scale 0.35/immunity
`NONE`, Valtan은 scale 0/immunity `ALL`이다. 따라서 player skill 적중이 Valtan pattern/stage/occurrence를 cancel하지 않고
damage와 hit flash만 남긴다. scale은 finite `0..4`. MonsterProfiles에는 공격 적중 시 사용할
`attackHitEventId`와 `attackReactionProfileId`를 exact field로 추가한다. hit event ID는 archetype별 stable/non-empty/
unique/96-byte 이하이며 예시는 `monster.valtan-padd-01.basic-attack.contact`다. reaction ID 빈 문자열이면 현재
damage-only 공격을 뜻한다.
PlayerSkills timeline과 Valtan event의 `reactionProfileId`가 이 catalog를 join한다.
이 field는 G02의 아직 미배포 final `PlayerProfiles v3 / MonsterProfiles v2 / BossProfiles v4`에 함께 넣고 중간
schema version을 한 번 더 만들지 않는다. Balance Tool은 현재 실제 editor가 있는 PLAYER/BOSS staged read/write/exact
set만 보존하고 새 Monster editor를 만들지 않는다. MonsterProfiles v2 exact parse/write/join은 publisher/schema test가
소유한다.

Gameplay bootstrap v5에는 다음 row를 추가한다.

```text
REACTION\t<ID>\t<KIND>\t<DIRECTION_POLICY>\t<DISTANCE>\t<DURATION_TICKS>\t<PRESENTATION_ARC_HEIGHT>\t<INTERRUPT_POLICY>\t<BLOCKED_POLICY>\t<PRESENTATION_REACTION_ID>
```

`PLAYER/BOSS/PROFILE` actor rows에는 received scale/immunity를 append한다. old bootstrap reader는 남기지 않는다.

### `BossHealthContract.h` 전체 코드

```cpp
#pragma once

#include <algorithm>
#include <cstdint>

namespace LostArk::Shared::BossHealth
{
	inline constexpr std::uint32_t MAXIMUM_HEALTH_BARS = 1000u;

	inline std::uint32_t Compute_HealthBarCount(
		const std::uint32_t currentHp,
		const std::uint32_t maximumHp,
		const std::uint32_t maximumBars)
	{
		if (maximumHp == 0 || maximumBars == 0 ||
			maximumBars > MAXIMUM_HEALTH_BARS || currentHp == 0)
		{
			return 0u;
		}
		const std::uint64_t clampedHp = static_cast<std::uint64_t>(
			(std::min)(currentHp, maximumHp));
		const std::uint64_t numerator =
			clampedHp * static_cast<std::uint64_t>(maximumBars);
		return static_cast<std::uint32_t>(
			(numerator + static_cast<std::uint64_t>(maximumHp) - 1u) /
			static_cast<std::uint64_t>(maximumHp));
	}
}
```

Server Valtan threshold, `CCombatHUDViewModel`, Balance Tool preview가 이 helper를 사용한다. Valtan max 160에서 HP가
48750이면 130, HP 30000이면 80이다. `x130`은 phase 1의 one-shot threshold/current stack이고 max/phase가 아니다.

### 새 `Server/Public/CombatDisplacementSystem.h` 전체 public contract

```cpp
#pragma once

#include "Gameplay/CombatReactionContract.h"
#include "Network/NetworkIds.h"

#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace LostArk::Server
{
struct SERVER_PLAYER;
struct SERVER_WORLD_ENTITY;
class CServerCollisionSystem;
class CServerNavigation;
class CGameplayCatalog;

struct SERVER_REACTION_REQUEST final
{
	LostArk::Shared::NET_ENTITY_ID iSourceNetEntityId =
		LostArk::Shared::INVALID_NET_ENTITY_ID;
	LostArk::Shared::NET_ENTITY_ID iTargetNetEntityId =
		LostArk::Shared::INVALID_NET_ENTITY_ID;
	std::string strHitEventId;
	std::string strReactionProfileId;
	float fSourceX = 0.f;
	float fSourceZ = 0.f;
	float fAttackForwardX = 0.f;
	float fAttackForwardZ = 0.f;
	float fHitVolumeForwardX = 0.f;
	float fHitVolumeForwardZ = 0.f;
};

struct SERVER_FORCED_MOTION final
{
	bool bActive = false;
	std::uint32_t iSequence = 0;
	std::uint32_t iStartTick = 0;
	std::uint32_t iDurationTickCount = 0;
	std::uint32_t iAppliedStepCount = 0;
	std::string strReactionProfileId;
	std::string strPresentationReactionId;
	LostArk::Shared::CombatReaction::REACTION_KIND eKind =
		LostArk::Shared::CombatReaction::REACTION_KIND::NONE;
	float fDirectionX = 0.f;
	float fDirectionZ = 0.f;
	float fTotalDistance = 0.f;
	float fPresentationArcHeight = 0.f;
};

class CCombatDisplacementSystem final
{
public:
	void Begin_Tick();
	bool Queue_Reaction(
		const SERVER_REACTION_REQUEST& request,
		const CGameplayCatalog& catalog);

	void Admit_QueuedReactions(
		std::uint32_t serverTick,
		const CGameplayCatalog& catalog,
		std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
		std::vector<SERVER_WORLD_ENTITY>& entities);

	void Update_ForcedMotions(
		std::uint32_t serverTick,
		CServerCollisionSystem& collision,
		CServerNavigation& navigation,
		std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
		std::vector<SERVER_WORLD_ENTITY>& entities);

	bool Has_PendingReaction(
		LostArk::Shared::NET_ENTITY_ID targetId) const;
	void Clear_Player(SERVER_PLAYER& player);
	void Clear_WorldEntity(SERVER_WORLD_ENTITY& entity);
	void Clear_All();

private:
	std::vector<SERVER_REACTION_REQUEST> m_QueuedRequests;
};
}
```

profile lookup은 `Queue_Reaction`/`Admit_QueuedReactions`의 immutable `CGameplayCatalog&` 하나로 고정하고 전역 singleton이나
선택적 constructor 경로를 만들지 않는다. queue는 raw hit request를 모두 쌓지 않고 target NetEntityId별 best candidate
하나로 즉시 coalesce한다. comparator는 resolved reaction priority desc, source NetEntityId asc, hitEventId asc다.
`MAXIMUM_REACTION_TARGETS_PER_BARRIER=MAX_WORLD_SNAPSHOT_ENTITIES(256)`이며 room init에서 vector를 reserve한다. second
player-target barrier는 실제 player cap 32 안이다. valid room actor 수가 이 cap을 넘지 않으므로 presentation damage-event
cap을 gameplay reaction cap으로 재사용하지 않는다. same target에 더 낮은 후보가 와도 damage는 이미 적용되고 reaction
candidate만 교체/폐기된다. invalid profile/target 또는 impossible unique-target overflow는 room invariant failure로 처리해
snapshot 진행을 중단하며 정상 gameplay에서 임의 reaction drop으로 계속하지 않는다.
`Has_PendingReaction`은 queue만 검사한다. active truth는 actor의 `ForcedMotion.bActive` 하나이며 system이 두 번째 active-ID
set을 복제하지 않는다.

`ServerPlayer.h`와 `ServerWorldEntity.h`는 각각 다음 exact state를 소유한다.

```cpp
SERVER_FORCED_MOTION ForcedMotion;
std::uint32_t iLastReactionSequence = 0;
```

admission은 `NextNonZeroCounter(iLastReactionSequence)`를 새 motion sequence로 commit한다. active motion 종료는
`ForcedMotion`을 inactive sentinel로 만들되 last sequence를 유지한다. death, despawn, room leave/reset은 actor reference를
받는 `Clear_Player`/`Clear_WorldEntity`로 motion을 reset하고 해당 NetEntityId의 pending request를 함께 제거한다. 새 actor
runtime은 last sequence 0에서 시작한다.

`CGameRoom`은 process-global이 아닌 room-local owner 하나를 가진다.

```cpp
CCombatDisplacementSystem m_CombatDisplacementSystem;
```

fixed tick 순서는 `Begin_Tick` → player skill updates(같은 instance 전달) → 첫
`Admit_QueuedReactions` barrier(world-entity target의 action/path cancel 확정) → Valtan/monster hit resolver(같은 instance에
queue) → 두 번째 admission barrier(player target 확정) → `Update_ForcedMotions` → snapshot이다. admission은 처리한 queue를
비워 다음 barrier와 섞지 않는다. 이미 active motion이거나 첫 barrier에서 새 reaction이 admitted된 monster는 같은 tick
AI action advance/hit evaluation 전체를 건너뛰므로 맞은 뒤 공격을 한 번 더 실행하지 않는다. Valtan `ALL` immunity는
첫 barrier에서 cancel되지 않는다. 현재 target-kind graph에 PvP/world-to-world hit가 없으므로 두 barrier가 같은 target의
cross-phase priority를 분할하지 않는다. 세 resolver가 별도 displacement instance나 immediate transform mutation을 만들지
않는다.

brain signatures도 같은 owner를 명시적으로 받는다.

```cpp
void CValtanBrain::Update(
	SERVER_WORLD_ENTITY& boss,
	std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
	const CGameplayCatalog& catalog,
	const CServerNavigation& navigation,
	CServerCollisionSystem& worldCollision,
	const CCombatCollisionSystem& combatCollision,
	CCombatDisplacementSystem& displacement,
	float fixedDeltaSeconds,
	std::uint32_t serverTick,
	std::vector<LostArk::Shared::DAMAGE_EVENT>& outDamageEvents) const;

void CMonsterBrain::Update(
	SERVER_WORLD_ENTITY& monster,
	std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
	const CGameplayCatalog& catalog,
	const CServerNavigation& navigation,
	const CCombatCollisionSystem& combatCollision,
	CCombatDisplacementSystem& displacement,
	float fixedDeltaSeconds,
	std::uint32_t serverTick,
	std::vector<LostArk::Shared::DAMAGE_EVENT>& outDamageEvents) const;
```

둘은 accepted hit에서 `Queue_Reaction(request,catalog)`만 호출하고 transform을 직접 밀지 않는다. GameRoom call site도
room member 하나를 위 parameter로 전달한다.

### admission과 tick 순서

1. player skill/monster/Valtan hit resolver는 damage와 같은 exactly-once result에서 reaction request를 queue한다.
2. lethal damage는 target을 DEAD로 만든 뒤 request를 버린다.
3. target별 후보를 `reaction priority desc, source NetEntityId asc, hitEventId asc`로 정렬해 하나만 고른다.
4. direction policy를 resolve하고 zero vector이면 attack forward, 그것도 invalid면 request를 거부한다.
5. target scale/immunity를 적용한다. `ALL`이면 reaction과 cancel을 모두 건너뛴다. target에 active motion이 있으면 새
   reaction priority가 active kind보다 엄격히 높을 때만 sequence를 증가시켜 replace하고, 같거나 낮으면 damage만 유지한
   채 reaction request를 버린다. admitted reaction만 profile의
   interrupt policy에 따라 current action/path를 cancel한다. player가 `TRIGGER_MOVE` 중이면 `TriggerMove={}`와 action을
   같은 admission에서 clear한 뒤 forced motion을 시작한다.
6. pending/active forced motion 동안 player input/skill start, `CServerTriggerSystem::Update_PlayerMotion`, trigger
   `Begin_MovePlayer`와 monster AI locomotion/action evaluation을 모두 보류한다. 이 때문에 시작하지 못한 trigger entry는
   once-consumed로 기록하지 않고 reaction 종료 뒤 정상 재평가한다.
7. 각 tick desired cumulative distance를 `totalDistance * completedSteps / durationTicks`로 계산하고 이전 cumulative와의
   차만큼 sweep한다. 마지막 tick rounding remainder도 Server가 소유한다.
8. static collision 또는 nav projection 실패 시 마지막 안전 root에서 motion을 끝낸다.

GameRoom은 player-skill hit 수집 직후 첫 admission barrier를 실행해 같은 tick `CMonsterBrain`의 locomotion뿐 아니라
action advance와 ACTIVE hit 평가도 취소한다. world attack 평가 뒤 두 번째 barrier를 실행하고 forced motion을 update한
후 snapshot을 만든다.

network command는 fixed update보다 먼저 처리되므로 input guard 위치도 고정한다. `Handle_Move`는 현재 payload/strict-forward
sequence 검증과 `iLastMoveSequence` 기록 뒤, path/goal mutation 전에 `player.ForcedMotion.bActive`면 return한다.
`Handle_UseSkill`은 active면 위 `Try_ConsumeForcedMotionBlockedStartSequence`만 호출하고 즉시 return한다. inactive일 때만
Character Select resource refill과 기존 `Try_Start`를 순서대로 호출한다. `Try_Start`도 defense-in-depth로 active state를
받으면 같은 helper를 거쳐 false를 반환한다. `Release`는 USE와 공통 `iLastSkillSequence`를 strict-forward로 소비한 뒤
active forced motion이면 `hasReleasedHold`를 바꾸지 않고 return한다. 이 순서로 cooldown/resource/action/stance/path는
blocked command에서 하나도 바뀌지 않고 같은 sequence가 forced motion 종료 직후 다시 살아나지 않는다.

`CServerCollisionSystem`에는 다음 API를 추가한다.

```cpp
bool Resolve_ForcedBodyDisplacement(
	const LostArk::Shared::CombatCollision::COMBAT_BODY_CAPSULE& body,
	float fromX, float fromY, float fromZ,
	float desiredX, float desiredY, float desiredZ,
	float& outSafeX, float& outSafeY, float& outSafeZ) const;
```

vertical body capsule을 static oriented collisionBox local space로 옮기고 box를 radius/vertical extent만큼 확장한 뒤
root segment를 sweep한다. navigation projection과 collision 둘 중 하나라도 실패하면 partial unsafe pose를 commit하지
않는다. player normal movement OBB path는 이번 G에서 교체하지 않는다.

Client PhysX `AddForce/AddImpulse`는 authoritative player/monster root에 호출하지 않는다. 이후 분리 가능한 도끼 파편,
ragdoll secondary body, dust 같은 presentation object에만 별도 G로 사용할 수 있다.

## G19. Shared protocol: reaction, occurrence와 world activation

### Packet type 추가/교체

기존 Character Select Valtan 전용 `C2S_SPAWN_WORLD_ENTITY` request/result는 제거하고 다음 typed packet으로 교체한다.

```cpp
enum class WORLD_SET_ACTIVATION_RESULT_CODE : std::uint8_t
{
	ACTIVATED,
	ALREADY_ACTIVE,
	REJECTED,
	END
};

enum class WORLD_SET_ACTIVATION_REJECT_REASON : std::uint8_t
{
	NONE,
	UNAUTHORIZED_SESSION,
	WRONG_WORLD,
	REVISION_MISMATCH,
	STALE_OR_REPLAYED_SEQUENCE,
	UNKNOWN_ACTIVATION_ID,
	INVALID_RESOURCE_STATE,
	NAVIGATION_REJECTED,
	CAPACITY_EXCEEDED,
	INTERNAL_STAGE_FAILURE,
	END
};

enum class WORLD_ACTIVATION_RESOURCE_STATE : std::uint8_t
{
	DORMANT,
	PARTIAL,
	ACTIVE,
	COMPLETED,
	END
};

struct WORLD_SET_ACTIVATION_OUTCOME final
{
	WORLD_SET_ACTIVATION_RESULT_CODE eResult =
		WORLD_SET_ACTIVATION_RESULT_CODE::END;
	WORLD_SET_ACTIVATION_REJECT_REASON eRejectReason =
		WORLD_SET_ACTIVATION_REJECT_REASON::NONE;
};

struct C2S_ACTIVATE_WORLD_SET final
{
	std::uint32_t iClientSequence = 0;
	std::string strActivationId;
};

struct S2C_WORLD_SET_ACTIVATION_RESULT final
{
	std::uint32_t iClientSequence = 0;
	std::string strActivationId;
	WORLD_SET_ACTIVATION_OUTCOME Outcome;
	std::string strRuntimeRevision;
};

struct WORLD_ACTIVATION_CONTROL final
{
	std::string strActivationId;
	std::string strDisplayName;
	WORLD_ACTIVATION_RESOURCE_STATE eState =
		WORLD_ACTIVATION_RESOURCE_STATE::END;
};

struct S2C_WORLD_ACTIVATION_CATALOG final
{
	std::string strRuntimeRevision;
	std::vector<WORLD_ACTIVATION_CONTROL> Controls;
};
```

`ACTIVATED/ALREADY_ACTIVE`는 reason `NONE`, `REJECTED`는 non-NONE exact enum을 요구한다. aggregate control은
referenced resource가 모두 dormant면 `DORMANT`, dormant가 하나라도 있고 나머지가 active/completed면 `PARTIAL`,
dormant가 없고 active가 하나라도 있으면 `ACTIVE`, 전부 terminal이면 `COMPLETED`다. 따라서 ACTIVE+COMPLETED 혼합은
`ACTIVE`다. `PARTIAL` control은 누를 수 있어 missing resource만 추가한다. ID/display name은 각각
96 UTF-8 bytes 이하, controls는 16 이하, ID unique/sorted다. catalog는 enter 승인 뒤 해당
room의 controls와 current state를 모두 보내며 Character Select 버튼은 이것만 소비한다. request는 catalog ID를
echo하지만 Server는 current room bootstrap set으로 다시 resolve한다.

### boss occurrence snapshot

```cpp
struct WORLD_COMBAT_OCCURRENCE_SNAPSHOT final
{
	NET_ENTITY_ID iSourceNetEntityId = INVALID_NET_ENTITY_ID;
	std::uint32_t iPatternSequence = 0;
	std::uint32_t iStageIndex = 0;
	std::string strHitEventId;
	std::uint32_t iAnchorCaptureTick = 0;
	std::uint32_t iStartDelayTickCount = 0;
	std::uint32_t iDurationTickCount = 0;
	LostArk::Shared::BossCombat::HIT_ANCHOR_POLICY eAnchorPolicy =
		LostArk::Shared::BossCombat::HIT_ANCHOR_POLICY::END;
	float fAnchorX = 0.f;
	float fAnchorY = 0.f;
	float fAnchorZ = 0.f;
	float fAnchorYawDegrees = 0.f;
};
```

room snapshot에 `CombatOccurrences` vector를 추가하고 cap 64를 writer/reader/Server 모두 검사한다. capture 뒤 종료 전의
pending/active occurrence만 보낸다. owner-follow도 identity/ticks는 보내되 Client는 source entity current pose를
사용한다. world/target-open은 captured anchor를 그대로 사용한다. action cancel/death/stage transition은 occurrence를
즉시 제거하고 Client가 false ghost를 만들지 않게 한다.

wire에는 wrap-ambiguous absolute end tick을 싣지 않는다. occurrence start는
`Try_AdvanceTickSkippingZero(iAnchorCaptureTick, iStartDelayTickCount,outStartTick)` 성공값, reaction/occurrence age는
`Try_GetForwardTickDistanceSkippingZero(startTick, currentTick)`로만 계산한다. active 조건은
`age < iDurationTickCount`다. duration과 delay는 public bounded count이며 raw unsigned subtraction, `<` tick 비교 또는
zero tick을 통과하는 `start + duration`을 codec/Client/Server에서 사용하지 않는다.

### player/world snapshot reaction과 boss bars

기존 `DAMAGE_EVENT`에는 hit-flash dedup에 필요한 Server edge를 다음처럼 추가한다.

```cpp
std::uint32_t iPresentationSequence = 0;
NET_ENTITY_ID iSourceNetEntityId = INVALID_NET_ENTITY_ID;
std::string strHitEventId;
```

`CGameRoom`은 `std::uint32_t m_iLastDamagePresentationSequence=0`을 room lifetime 동안 reset하지 않는다. 모든
PlayerSkill/Valtan/Monster damage producer는 sequence 0 event를 authoritative append order로 한 tick vector에 넣고,
GameRoom이 두 reaction barrier 뒤 snapshot 직전에 각 row를 `NextNonZeroCounter`로 stamp한다. producer가 서로 sequence를
발급하지 않는다. Client dedup key는 `iPresentationSequence` 하나이며 disconnect/level leave에서 ledger를 clear한다. manual room
generation reset에도 Server sequence는 계속 증가한다. target NetEntityId로 `CValtan`/`CCharacter`/`CNpc` hit flash를 route한다.
source action transform이나 damage authority를 Client에서 재구성하지 않는다. output cap 때문에 event push가 생략돼도
HP/death 권위는 유지하며 flash만 생략된다. codec은 transmitted sequence가 non-zero이고 snapshot 안 unique인지,
source/target ID와 hit event ID가 bounded/valid인지 검사한다.

`PLAYER_SNAPSHOT`과 `WORLD_ENTITY_SNAPSHOT`에 다음 exact field를 추가한다.

```cpp
LostArk::Shared::CombatReaction::SNAPSHOT Reaction;
```

`WORLD_ENTITY_SNAPSHOT`에는 다음 field도 추가한다.

```cpp
std::uint32_t iMaximumHealthBars = 0;
```

boss는 profile의 1..1000, non-boss는 0이다. current bar는 보내지 않고 Shared helper로 계산한다. reaction inactive는
all-zero `NONE`와 empty presentation ID; active는 non-zero sequence/start/duration, normalized finite XZ direction과
96-byte 이하 stable presentation reaction ID다. position은 계속 snapshot의
authoritative root다.

room snapshot에는 bounded activation state도 추가한다.

```cpp
struct WORLD_ACTIVATION_STATE_SNAPSHOT final
{
	std::string strActivationId;
	WORLD_ACTIVATION_RESOURCE_STATE eState =
		WORLD_ACTIVATION_RESOURCE_STATE::END;
	std::uint32_t iRoomGeneration = 0;
};
```

cap은 16이다. late join은 catalog full state를 먼저 받고 이후 snapshot state로 갱신한다. equal/out-of-order snapshot은
기존 strict tick guard로 거부한다.

### packet validation

- unknown enum, zero/invalid NetEntityId와 non-finite anchor/direction/yaw를 거부한다. duration tick count는 `1..cap`,
  start-delay tick count는 capture와 active start가 같은 정상 event를 위해 `0..cap`을 허용한다.
- reaction/occurrence active-age와 expiry는 Shared zero-skipping forward-distance helper만 사용한다.
- occurrence ID/set ID는 bounded UTF-8이며 embedded NUL, duplicate, empty를 거부한다.
- writer/reader는 vector count를 먼저 cap 검사하고 destination을 stage한 뒤 exact payload end에서만 commit한다.
- truncated/trailing packet은 기존 destination을 유지한다.
- `C2S_USE_SKILL`은 여전히 skill/aim intent-only이며 target/hit/reaction field를 추가하지 않는다.
- runtime revision은 기존 enter-approved 64 lowercase hex와 같아야 한다.

NetworkProtocolHarness는 Debug/Release에서 byte-size golden 대신 field-by-field roundtrip과 exact end offset을 함께
검사한다. 최종 `NETWORK_PROTOCOL_VERSION=13`이며 protocol 변경과 Server/Client consumer를 같은 G에서 build한다.

## G20. Map Tool Character Select placement와 activation-set authoring

### 대상

- 네 Area의 `Data/Worlds/<AreaId>/Gameplay.world.json`
- 새 `Data/Worlds/LV_LOBBY_CLASSSELECT_SL00/SpawnGroups.world.json`
- `Client/Public/WorldGameplayDocument.h`, `Client/Private/WorldGameplayDocument.cpp`
- `Client/Public/MapTool.h`, `Client/Private/MapTool.cpp`
- 새 `Client/Public/AreaAuthoringTransaction.h`, `Client/Private/AreaAuthoringTransaction.cpp`
- `Tools/WorldPipeline/Publish-WorldGameplay.ps1`
- `Tools/NavigationPipeline/Publish-ServerNavigation.ps1`
- `Tools/GameplayPipeline/Publish-BalanceRuntimeSet.ps1`

### `Gameplay.world.json` formatVersion 5

root exact set을 다음으로 올린다.

```text
schema, formatVersion, areaId, revision, placements, manualActivationSets
```

Bern/Valtan/Training도 `manualActivationSets: []`를 명시한다. Character Select row는 다음 세 set을 가진다.

이 schema bump에서 authoring revision은 현재 값에서 정확히 한 번만 올린다: Bern `13→14`, Valtan `561→562`,
Character Select `3→4`, Training Ground `2→3`. 구현 중 save retry마다 revision을 추가 증가시키지 않는다.

```json
{
"manualActivationSets": [
  {
    "activationId": "arena.character-select.spawn.monsters",
    "displayName": "Spawn Monsters",
    "activationMode": "IMMEDIATE_ATOMIC",
    "resetPolicy": "RESET_WHEN_ROOM_EMPTY",
    "spawnGroupIds": ["spawn.character-select.audition"],
    "bossPlacementIds": []
  },
  {
    "activationId": "arena.character-select.spawn.valtan",
    "displayName": "Spawn Valtan",
    "activationMode": "IMMEDIATE_ATOMIC",
    "resetPolicy": "RESET_WHEN_ROOM_EMPTY",
    "spawnGroupIds": [],
    "bossPlacementIds": ["boss.valtan.character-select.lazy"]
  },
  {
    "activationId": "arena.character-select.spawn.all",
    "displayName": "Spawn All",
    "activationMode": "IMMEDIATE_ATOMIC",
    "resetPolicy": "RESET_WHEN_ROOM_EMPTY",
    "spawnGroupIds": ["spawn.character-select.audition"],
    "bossPlacementIds": ["boss.valtan.character-select.lazy"]
  }
]
}
```

set exact fields는 위 여섯 개다. ID는 Area unique, refs는 same Area unique, boss ref는 disabled boss placement만,
group ref는 existing group만 허용한다. 공통 root set count는 0..16이며 Character Select는 exact 3, 다른 세 Area는
exact 0이다. activationId/displayName은 non-empty 96 UTF-8 bytes 이하이고 line-bootstrap 금지문자(tab/CR/LF/NUL)를 거부한다. set별 group/boss ref는 각각 16 이하이고 둘의 union은
non-empty다. set끼리 resource overlap은 허용하되 duplicate spawn을 막는 Server resource ledger가 필수다.
first cut은 manual set resource와 같은 Area trigger action의 `activateSpawnGroup`/`activateEncounter` target 교집합을 publisher가
거부한다. trigger activation을 manual generation ledger 밖에서 먼저 실행한 뒤 버튼으로 중복 commit/reset하는 두 권위를
허용하지 않는다.
`IMMEDIATE_ATOMIC`의 group은 requiredCompletedGroupId null, repeatPolicy ONCE, one wave,
start/initial/interval 0, `totalCount == maxAlive`, bounded total이어야 한다.

### Character Select `SpawnGroups.world.json`

새 파일은 기존 formatVersion 1 exact schema를 사용한다. 실제 위치는 구현자가 숫자로 추측하지 않고 Development
Map Editor의 map surface pick으로 저작한다. 정확한 개별 위치가 필요하면 monster 하나마다 stable anchor와
entry `count=1`을 둔다. 하나의 anchor에 count>1을 겹쳐 놓지 않는다.

```json
{
  "schema": "lostark.world-spawn-groups",
  "formatVersion": 1,
  "areaId": "LV_LOBBY_CLASSSELECT_SL00",
  "revision": 1,
  "anchors": [],
  "spawnGroups": [
    {
      "spawnGroupId": "spawn.character-select.audition",
      "requiredCompletedGroupId": null,
      "maxAlive": 1,
      "repeatPolicy": "ONCE",
      "completionPolicy": "ALL_WAVES_CLEARED",
      "waves": [
        {
          "waveId": "wave.immediate",
          "startDelayMs": 0,
          "nextWavePolicy": "ALL_DEAD",
          "entries": []
        }
      ]
    }
  ]
}
```

위 empty anchor/entry와 maxAlive 1은 schema 예시일 뿐 제품 publish 대상이 아니다. 구현 시 Map Tool에서 하나 이상의
실제 anchor/entry를 저장하고 `maxAlive=totalCount`로 맞춘 뒤에만 publisher가 통과한다.

### Map Tool API와 UI state

`WorldGameplayDocument.h`와 `MapTool.h`에 다음 authoring contract/member/function을 추가한다.

```cpp
struct MANUAL_ACTIVATION_SET final
{
	std::string strActivationId;
	std::string strDisplayName;
	MANUAL_ACTIVATION_MODE eActivationMode = MANUAL_ACTIVATION_MODE::END;
	MANUAL_ACTIVATION_RESET_POLICY eResetPolicy =
		MANUAL_ACTIVATION_RESET_POLICY::END;
	std::vector<std::string> SpawnGroupIds;
	std::vector<std::string> BossPlacementIds;
};

enum class GAMEPLAY_PICK_MODE : std::uint8_t
{
	NONE,
	CREATE_PLACEMENT,
	RELOCATE_SELECTED_PLACEMENT,
	CREATE_SPAWN_ANCHOR,
	END
};

bool Begin_RelocateSelectedGameplayPlacement();
bool Commit_GameplayPick(const MAP_SURFACE_HIT& hit);
void Cancel_GameplayPick();
bool Add_ManualActivationSet();
bool Remove_SelectedManualActivationSet();
bool Validate_ManualActivationSets(std::string& outReason) const;
bool Save_GameplayWorldAuthoringTransaction(std::string& outReason);
```

`CWorldGameplayDocument`는 `std::vector<MANUAL_ACTIVATION_SET>`를 staged document state로 소유하고 const getter,
Map Tool-only mutable draft API, `Mark_Edited`, exact parse/write/validate를 제공한다. stable ID로 선택하며 vector index를
저장 ID로 노출하지 않는다. Area transaction layer set은 Gameplay, policy에 따른 present/absent SpawnGroups와 해당 Area의
navigation authoring(`.navsource/.navpaint/.navblockers` 또는 uniform `.navgrid.json`) 전부다.
Gameplay/SpawnGroups/Navigation 개별 Save는 이 transaction으로 수렴해 서로 다른 disk revision을 검증하는 우회 경로를
제거한다. 기존 `Save_AllAuthoring`/`Save and Continue`가 visual Map placements 또는 destruction/simulation pair까지 dirty인
경우 이번 transaction과 범위를 섞지 않고 어떤 파일도 쓰기 전에 typed diagnostic으로 중단한다. 사용자가 그 독립
authoring domain을 먼저 저장/정리한 뒤 gameplay-world transaction을 다시 실행한다. Bern/Training은 explicit
`SpawnGroups ABSENT` state를 사용하며 빈 optional source를 생성하지 않는다.

현재 `Render_DestructionGroupEditor`의 `Save World Events`와 simulation panel의 `Save Simulations`는
`Save_AllAuthoring`을 호출하지 않고 기존 atomic `Save_DestructionAuthoringPair` typed wrapper로 직접 reroute한다.
visual placement는 기존 dedicated Save Placements를 유지한다. unsaved Area-switch popup은 mixed domain이면 아무 것도
저장하지 않고 domain 목록/이유를 보여 준 뒤 cancel로 editor에 돌아가 각 dedicated save를 실행할 수 있게 한다. 이 두
버튼이 자기 dirty state 때문에 영구적으로 Save All reject되는 경로를 남기지 않는다.

Map Tool은 계속 Debug `LEVEL::DEVELOPMENT`의 Map Editor workspace에서만 열린다. Area selector로
`LV_LOBBY_CLASSSELECT_SL00` 또는 `LV_LUT_HEARTRB_ED`를 고른 뒤 기존
`boss.valtan.character-select.lazy`/`boss.valtan.center`를 선택하고 `Relocate Selected Gameplay Placement`를 누르면
다음 valid map hit 한 번으로 staged position을 바꾼다. Save 전까지 runtime/authoring file은 바뀌지 않는다.
Esc/Area 변경/selection 삭제는 pick을 cancel한다.

transaction은 모든 dirty in-memory gameplay/spawn/navigation draft를 sibling temp overlay에 serialize하고, unchanged
layer도 current disk bytes/hash로 같은 overlay inventory에 넣는다. 각 문서를 strict reload한 뒤 stable ref/nav projection을
cross-validate한다. optional-absent Area는 `SpawnGroups=ABSENT` marker를 만들고 source가 실제로 absent인지 검증한다. 이어
temp overlay를 입력으로 world/navigation publisher Validate를 실행한다. 그 다음 layer별 old bytes/hash/ABSENT와 new
temp/hash/ABSENT를 담은 recovery journal을 fsync하고 존재하는 destination만 ordered replace한다. 어느 replace, publisher
또는 reload가 실패하면 backup bytes/absence로 전체 layer set을 복원하며 in-memory
revision/selection/dirty flag도 save 전 값으로 유지한다. startup에서 incomplete journal을 발견하면 hash와 ABSENT marker로
old/new state를 판별해 old layer set으로 recover한 뒤에만 편집을 허용한다. replace 성공과 strict reload 뒤 journal을
committed로 표시하고 backup/temp를 정리한다. pending nav paint/blocker가 있는데 old disk nav로 anchor를 검증하거나,
pair를 commit한 뒤 별도 nav save를 실행하는 순서는 금지한다.

새 pure transaction의 public contract는 다음으로 고정한다. `RelativeDestination`은 authoring root 기준 canonical relative
path이고 absolute/drive-qualified/`..`를 거부한다. role은 Area policy대로 exact-once이고 canonical
`RelativeDestination`은 role과 무관하게 전체 request에서 unique여야 한다. `Bytes`와 `strSha256`은 `PRESENT`일 때만
non-empty/64-lowercase-hex이고 서로 일치해야 하며, `ABSENT`는 둘 다 비어 있어야 한다. request는 Area policy가 요구하는
layer를 정확히 한 번씩 가지며 caller의 draft를 참조하지 않고 byte snapshot을 소유한다.

```cpp
#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <string>
#include <vector>

namespace Client
{
enum class AREA_AUTHORING_LAYER_ROLE : std::uint8_t
{
	GAMEPLAY,
	SPAWN_GROUPS,
	NAV_SOURCE,
	NAV_PAINT,
	NAV_BLOCKERS,
	NAV_UNIFORM_GRID,
	END
};

enum class AREA_AUTHORING_LAYER_PRESENCE : std::uint8_t
{
	PRESENT,
	ABSENT,
	END
};

struct AREA_AUTHORING_LAYER_IMAGE final
{
	AREA_AUTHORING_LAYER_ROLE eRole = AREA_AUTHORING_LAYER_ROLE::END;
	std::filesystem::path RelativeDestination;
	AREA_AUTHORING_LAYER_PRESENCE ePresence =
		AREA_AUTHORING_LAYER_PRESENCE::END;
	std::vector<std::uint8_t> Bytes;
	std::string strSha256;
};

using AREA_AUTHORING_VALIDATE_CALLBACK = std::function<bool(
	const std::filesystem::path& overlayRoot,
	std::string& outReason)>;

enum class AREA_AUTHORING_FAILURE_POINT : std::uint8_t
{
	NONE,
	AFTER_JOURNAL_FLUSH,
	AFTER_LAYER_MUTATION,
	BEFORE_COMMIT_MARKER,
	END
};

struct AREA_AUTHORING_FAILURE_INJECTION final
{
	AREA_AUTHORING_FAILURE_POINT eForwardPoint =
		AREA_AUTHORING_FAILURE_POINT::NONE;
	std::size_t iForwardMutationOrdinal = 0;
	bool bFailRollbackMutation = false;
	std::size_t iRollbackMutationOrdinal = 0;
	bool bLeaveIncompleteJournalForRecovery = false;
};

struct AREA_AUTHORING_TRANSACTION_REQUEST final
{
	std::string strAreaId;
	std::filesystem::path AuthoringRoot;
	std::filesystem::path JournalRoot;
	std::vector<AREA_AUTHORING_LAYER_IMAGE> DesiredLayers;
	AREA_AUTHORING_VALIDATE_CALLBACK ValidateOverlay;
	AREA_AUTHORING_FAILURE_INJECTION FailureInjectionForHarness;
};

enum class AREA_AUTHORING_TRANSACTION_STATUS : std::uint8_t
{
	COMMITTED,
	ROLLED_BACK,
	RECOVERED_OLD_SET,
	NO_RECOVERY_NEEDED,
	INJECTED_CRASH_PENDING_RECOVERY,
	RECOVERY_REQUIRED,
	FAILED_WITHOUT_MUTATION,
	END
};

struct AREA_AUTHORING_TRANSACTION_RESULT final
{
	AREA_AUTHORING_TRANSACTION_STATUS eStatus =
		AREA_AUTHORING_TRANSACTION_STATUS::END;
	std::size_t iMutationCount = 0;
	std::vector<AREA_AUTHORING_LAYER_IMAGE> CommittedLayers;
};

class CAreaAuthoringTransaction final
{
public:
	bool Commit(
		const AREA_AUTHORING_TRANSACTION_REQUEST& request,
		AREA_AUTHORING_TRANSACTION_RESULT& outResult,
		std::string& outReason) const;

	bool Recover_Incomplete(
		const std::filesystem::path& authoringRoot,
		const std::filesystem::path& journalRoot,
		AREA_AUTHORING_TRANSACTION_RESULT& outResult,
		std::string& outReason) const;
};
}
```

`FailureInjectionForHarness`는 `LOSTARK_CLIENT_FRONTEND_HARNESS` build에서만 non-default 값을 admit하고 제품 Client는
non-default request를 거부한다. forward mutation failure 뒤 `bFailRollbackMutation`과 ordinal이 old-state restore/flush의
exact failure를 주입해 `RECOVERY_REQUIRED`를 재현한다. `AreaAuthoringTransaction.cpp`가 journal v1 codec, SHA-256, backup/temp sidecar,
Windows `CreateFileW`/`WriteFile`/`FlushFileBuffers`와 write-through replace를 단독 소유한다. 순서는 old/new sidecar
write+flush → exact journal write+flush → ordered destination mutation → destination hash strict reload → commit marker
write+flush다. ordinary injected/real failure는 같은 call에서 old bytes/ABSENT를 전부 복원해 `ROLLED_BACK`을 반환한다.
crash injection만 incomplete journal을 의도적으로 남기며 다음 `Recover_Incomplete`가 old set 복원과 hash 검증을 끝내기
전에는 Map Tool load/save를 허용하지 않는다. callback 실패는 journal/destination mutation 전
`FAILED_WITHOUT_MUTATION`이고, callback은 overlay만 읽으며 live destination을 고치지 않는다.
rollback write/replace/flush 자체가 실패하면 journal/sidecar를 보존하고 `RECOVERY_REQUIRED`로 fail closed해 editor를 즉시
disable한다. same-process 또는 다음 startup `Recover_Incomplete`가 old set을 완전히 복원하기 전에는 다른 save/load를
허용하지 않는다. `Commit`의 bool은 `COMMITTED`일 때만 true, `Recover_Incomplete`는 `RECOVERED_OLD_SET` 또는
`NO_RECOVERY_NEEDED`일 때만 true다.

filesystem journal/replace/recovery는 MapTool UI에 숨기지 않고 pure `CAreaAuthoringTransaction`이 소유한다. MapTool의
`Save_GameplayWorldAuthoringTransaction`은 draft adapter와 publisher Validate callback만 제공한다. 이 CPP를
ClientFrontendHarness project에 직접 등록해 temp root에서 각 layer replace failure와 process-crash journal recovery를
실행하며 full MapTool/Level dependency를 harness에 링크하지 않는다.

### publisher와 navigation join

`Publish-WorldGameplay.ps1`과 Server navigation publisher는 manual set이 참조하는 모든 spawn anchor를 nav grid에
projection하고 authored Y와 projected Y tolerance, finite yaw를 검사한다. 지금처럼 player/boss placement만 nav
검증하고 spawn anchor를 빠뜨리지 않는다. `MAP_LOAD_SCOPE`는 Client `CLevelRegistry` 소유이므로 Server bootstrap/
preflight 입력으로 복제하지 않는다. 현재 ClientFrontendHarness는 `LevelRegistry.cpp` dependency graph를 링크하지
않으므로 scope 검증을 거기에 허위로 추가하지 않는다. ProjectAudit의 descriptor/presentation coverage 검사와 full Client
Character Select Server Arena smoke가 map-scope/lazy-load를 검증한다.

baseline actor + set이 한 번에 추가하는 boss/monster + room public maximum player count가 Shared world entity/snapshot cap을
넘지 않아야 한다. profile/catalog/presentation archetype도 모두 join한다. Valtan/Character Select required SpawnGroups
source가 누락되면 publish를 실패시키고 old runtime set을 보존한다. Bern/Training처럼 expected-absent world의 stale
generated spawn artifact만 staged tombstone/delete와 rollback 목록에 넣는다.

world bootstrap v7에는 다음 row를 추가한다.

```text
MANUALSET\t<ACTIVATION_ID>\t<DISPLAY_NAME>\t<IMMEDIATE_ATOMIC>\t<RESET_WHEN_ROOM_EMPTY>\t<GROUP_REF_COUNT>\t<BOSS_REF_COUNT>
MANUALGROUP\t<ACTIVATION_ID>\t<SPAWN_GROUP_ID>
MANUALBOSS\t<ACTIVATION_ID>\t<BOSS_PLACEMENT_ID>
```

v7은 앞 G의 runtime revision까지 합친 아직 미배포 final version이므로 중간 v7/v8 두 번 bump하지 않는다. Valtan과
Character Select spawn-group bootstrap은 runtime-set manifest의 `SERVER_REQUIRED` row다. Bern/Training에 예상 밖
spawn artifact가 있으면 revision과 무관하게 room ready를 거부한다.

## G21. Server atomic world-set activation과 existing monster AI

### World bootstrap/runtime definitions

```cpp
enum class MANUAL_ACTIVATION_MODE : std::uint8_t
{
	IMMEDIATE_ATOMIC,
	END
};

enum class MANUAL_RESET_POLICY : std::uint8_t
{
	RESET_WHEN_ROOM_EMPTY,
	END
};

struct MANUAL_ACTIVATION_SET_DEFINITION final
{
	std::string strActivationId;
	std::string strDisplayName;
	MANUAL_ACTIVATION_MODE eMode = MANUAL_ACTIVATION_MODE::END;
	MANUAL_RESET_POLICY eResetPolicy = MANUAL_RESET_POLICY::END;
	std::vector<std::string> SpawnGroupIds;
	std::vector<std::string> BossPlacementIds;
};
```

`CWorldBootstrap`는 Area별 immutable set map을 expose한다. set은 pointer/index가 아니라 stable ID로 resolve한다.

### `CSpawnGroupRuntime` staging contract

```cpp
struct STAGED_MONSTER_SPAWN final
{
	std::string strSpawnGroupId;
	std::string strArchetypeId;
	std::string strAnchorId;
	float fX = 0.f;
	float fY = 0.f;
	float fZ = 0.f;
	float fYawDegrees = 0.f;
	SERVER_WORLD_ENTITY Entity;
};

struct STAGED_IMMEDIATE_GROUP_RUNTIME_COMMIT final
{
	std::string strSpawnGroupId;
	std::uint32_t iRoomGeneration = 0;
	std::size_t iWaveIndex = 0;
	std::uint64_t iElapsedMs = 0;
	std::vector<std::uint32_t> SpawnedByEntry;
};

struct STAGED_IMMEDIATE_GROUP_ACTIVATION final
{
	STAGED_IMMEDIATE_GROUP_RUNTIME_COMMIT RuntimeCommit;
	std::vector<STAGED_MONSTER_SPAWN> Monsters;
};

enum class ACTIVATION_STAGE_FAILURE : std::uint8_t
{
	NONE,
	INVALID_RESOURCE_STATE,
	NAVIGATION_REJECTED,
	CAPACITY_EXCEEDED,
	INTERNAL_STAGE_FAILURE,
	END
};

enum class MANUAL_RESOURCE_RUNTIME_STATE : std::uint8_t
{
	DORMANT,
	ACTIVE,
	COMPLETED,
	END
};

class CSpawnGroupRuntime final
{
public:
	ACTIVATION_STAGE_FAILURE Stage_ImmediateRuntimeCommit(
		std::string_view spawnGroupId,
		std::uint32_t roomGeneration,
		const std::vector<std::uint32_t>& stagedCountByEntry,
		STAGED_IMMEDIATE_GROUP_RUNTIME_COMMIT& outStaged,
		std::string& outReason) const;

	void Commit_ImmediateActivation(
		STAGED_IMMEDIATE_GROUP_RUNTIME_COMMIT&& staged) noexcept;

	MANUAL_RESOURCE_RUNTIME_STATE Query_ManualResourceState(
		std::string_view spawnGroupId,
		std::uint32_t roomGeneration) const;
	void Reset_ManualResource(
		std::string_view spawnGroupId,
		std::uint32_t roomGeneration);
};
```

GameRoom의 `Stage_ImmediateGroupEntities`가 immutable `CSpawnGroupBootstrap`, `CGameplayCatalog`, `CServerNavigation`,
reserved first NetEntityId와 current capacity를 받아 anchor resolve/nav projection/profile-body join 및 완전한
`SERVER_WORLD_ENTITY` payload를 만든다. 성공한 entry별 count를 `Stage_ImmediateRuntimeCommit`에 넘겨 current definition의
authored total과 exact 비교한다. 따라서 `CSpawnGroupRuntime`이 catalog/nav/entity builder를 숨겨 추측하지 않는다.
staged token은 non-zero room generation, one-wave index 0, elapsed 0, entry별 authored total과 같은
`SpawnedByEntry`를 포함한다. `Monsters` entity payload와 runtime metadata는 분리되어 GameRoom이 entities를
`m_WorldEntities`로 move한 뒤 metadata만 `Commit_ImmediateActivation`으로 넘긴다.
`Commit`은 validation/allocation을 하지 않는 `noexcept` token move이며 group을 RUNNING으로 만들고 normal `Update`가
같은 entry를 다시 spawn하지 못하게 한다. 여러 group/entity/ledger vector capacity를 모두 checked reserve한 뒤에만
aggregate commit에 들어간다.

runtime group의 stored generation 0은 명시적 unbound `DORMANT`다. `Stage_ImmediateRuntimeCommit`은 current non-zero
generation과 stored 0 또는 exact match만 admit하고 staged token/commit이 current generation을 기록한다.
`Reset_ManualResource(groupId,currentGeneration)`은 exact bound generation에서만 runtime cursor/count/state를 초기화한 뒤
stored generation을 다시 0으로 만든다. 따라서 최초 generation 1과 last-player reset 뒤 다음 generation 모두 DORMANT에서
재활성화할 수 있다. stored non-zero generation mismatch와 unknown group은 invalid fallback을 정상 DORMANT로 가장하지 않고
`END`를 반환한다.
RUNNING은 ACTIVE, all-waves-cleared는 COMPLETED로 projection한다. boss resource는 matching manual-origin entity가 present
and alive면 ACTIVE, present DEAD 또는 terminal ledger면 COMPLETED, origin이 없으면 DORMANT다. activation/group completion/
boss death/reset마다 이 source state에서 모든 overlapping set control을 다시 derive한다.

### GameRoom activation state

```cpp
enum class ACTIVATION_RESOURCE_KIND : std::uint8_t
{
	SPAWN_GROUP,
	BOSS_PLACEMENT,
	END
};

struct ACTIVATION_RESOURCE_KEY final
{
	ACTIVATION_RESOURCE_KIND eKind = ACTIVATION_RESOURCE_KIND::END;
	std::string strResourceId;
};

struct STAGED_WORLD_SET_ACTIVATION final
{
	std::string strActivationId;
	std::uint32_t iRoomGeneration = 0;
	std::vector<STAGED_IMMEDIATE_GROUP_ACTIVATION> Groups;
	std::vector<SERVER_WORLD_ENTITY> Bosses;
	std::vector<ACTIVATION_RESOURCE_KEY> NewlyActivatedResources;
};
```

GameRoom private API는 다음 의미 단위로 나눈다.

```cpp
WORLD_SET_ACTIVATION_OUTCOME Try_ActivateWorldSet(
	SESSION_ID sessionId,
	std::uint32_t clientSequence,
	std::string_view activationId);

ACTIVATION_STAGE_FAILURE Stage_WorldSetActivation(
	const MANUAL_ACTIVATION_SET_DEFINITION& definition,
	STAGED_WORLD_SET_ACTIVATION& outStaged,
	std::string& outReason);

void Commit_WorldSetActivation(
	STAGED_WORLD_SET_ACTIVATION&& staged) noexcept;

void Reset_ManualArenaStateWhenRoomEmpty();
void Broadcast_ActivationCatalog(SESSION_ID sessionId) const;
```

### transaction exact order

1. authenticated session, player in this room, accepted runtime revision, Character Select Server Arena를 검증한다.
2. sequence replay guard와 bounded activation ID를 검증한다.
3. set을 current room bootstrap에서 resolve한다.
4. referenced group/placement를 resource ledger와 runtime entity identity로 `DORMANT` 또는
   `ACTIVE_OR_COMPLETED`로 분류한다.
5. 전부 active/completed면 mutation 없이 `ALREADY_ACTIVE`를 반환한다.
6. missing resource 전체에 대해 same Area/type/disabled boss/immediate group/profile/body/nav/height/entity cap/
   snapshot cap/NetEntityId headroom을 preflight한다. staged post-commit full snapshot, 각 spawn frame, activation catalog/result를
   real protocol codec의 scratch writer로 serialize해 `MAX_PACKET_BYTES` 이하인지도 mutation 전에 확인한다. 또한 proposed
   world actor count에 `MAX_PLAYERS_PER_ROOM` player rows까지 채우고 각 class/nickname/reaction/string field, damage 64,
   boss occurrence 64, activation state 16과 모든 bounded ID를 각각 public max length로 채운 conservative future snapshot을
   serialize해 recurring worst case가 64 KiB를 넘지 않아야 한다. current connected player 수만 사용해 late join 용량을
   낙관하지 않는다.
7. contiguous entity IDs를 reserve하되 failure 시 global next ID를 소비하지 않는다. commit 성공 시에만 next ID를
   advance한다.
8. world entity vector/group runtime/resource ledger capacity를 모두 reserve한다. per-set mutable ledger는 없다.
9. boss와 monster staged entity를 stable `(resource kind, resource ID, anchor/entry order)`로 commit한다.
10. spawn broadcasts와 result는 preflight에서 만든 validated payload plan을 commit 뒤에만 queue한다. unexpected I/O
    failure는 disconnect/resnapshot recovery를 사용하지만 canonical frame oversize는 commit 전 거부되어야 한다.

stage failure는 text를 parse하지 않고 `ACTIVATION_STAGE_FAILURE`를 exact protocol reject reason으로 total mapping한다.
public result는 exact reject enum을 보내고 Server diagnostic에는 `outReason`의 상세 원인을 보존한다. 성공 outcome은
reason NONE, reject outcome은 non-NONE이어야 한다.

Monsters 뒤 All이면 group은 no-op이고 boss만 stage한다. Valtan 뒤 All이면 boss는 no-op이고 group만 stage한다.
All 동시 요청은 room command queue에서 하나만 commit한다. set ID가 달라도 resource ledger가 duplicate를 막는다.

control state는 per-set mutable truth로 저장하지 않고 매번 referenced resource ledger에서 derive한다. 하나라도
DORMANT면 `PARTIAL`(전부 dormant면 `DORMANT`), dormant가 없고 하나라도 live ACTIVE면 `ACTIVE`, 전부 terminal이면
`COMPLETED`다. activation commit, group completion, boss death, room reset 때 영향받는 모든 set의 derived state를
snapshot/catalog에 갱신한다. 따라서 Monsters 뒤 All은 PARTIAL로 남아 눌릴 수 있다.

entity에는 manual origin을 다음처럼 기록한다.

```cpp
std::uint32_t iManualRoomGeneration = 0;
ACTIVATION_RESOURCE_KIND eManualOriginKind = ACTIVATION_RESOURCE_KIND::END;
std::string strManualOriginResourceId;
```

kind와 ID tuple이 origin 정본이다. spawnGroup ID와 boss placement ID의 text가 우연히 같아도 reset/death/completion
lookup이 섞이지 않는다.

`CGameRoom`은 다음 상태를 명시적으로 소유한다.

```cpp
std::uint32_t m_iManualRoomGeneration = 1u;
```

0은 invalid sentinel이다. room construction은 1, 마지막 player leave/disconnect/transfer로 manual state cleanup을 완전히
commit한 직후 Shared `NetworkTickContract.h::NextNonZeroCounter`로 한 번 증가한다. 빈 room의 다음 첫 join은 이미 증가한
generation을 사용한다.

queued room command는 enqueue 시 generation을 캡처하고 execute 시 current와 다르면 mutation 없이 reject한다. wrap은
zero를 건너뛰며 staged token, `RUNTIME_GROUP`, entity origin, resource ledger, activation catalog/snapshot이 같은 generation을
사용한다. `Reset_ManualResource`는 runtime group의 stored generation과 정확히 일치할 때만 reset하고 0/unbound로
되돌린 뒤 room generation을 증가시킨다.

`SERVER_PLAYER`에는 `iLastWorldActivationSequence=0`을 추가하고 accepted/rejected activation request 모두 strict-forward
sequence guard 뒤 latest를 기록한다. enter/connection마다 새 player runtime이므로 guard도 reset된다. old
`Request_ValtanSpawn` timeout/deque/member, `C2S_SPAWN_WORLD_ENTITY` sender/reader/result consumer는 typed activation 경로와
같은 change에서 전부 제거한다.

마지막 player가 leave/disconnect/transfer하면 `RESET_WHEN_ROOM_EMPTY` resource-origin entity를 despawn broadcast한 뒤
group/resource ledger를 reset한다. control state는 resource ledger에서 다시 derive한다. baseline placement/nav/collision/
trigger는 유지하고 NetEntityId는 단조 증가한다.
다른 player가 하나라도 남아 있으면 reset하지 않는다. late join에는 current entities와 activation catalog state를
replay한다. monster/Valtan이 죽어도 room이 비기 전에는 resource를 completed로 유지해 버튼으로 재소환하지 않는다.

### existing monster AI 확장

새 `CMonster` class/vector는 만들지 않는다. `CGameRoom::m_WorldEntities`의 MONSTER row마다 기존
`CMonsterBrain::Update`를 호출한다. 공용 target predicate는 두 CPP의 anonymous helper로 복제하지 않고 기존
`Server/Public/ServerPlayer.h`의 `namespace LostArk::Server`에 inline으로 추가해 두 Brain이 같은 함수를 include/호출한다.

```cpp
bool Is_CombatTargetEligible(const SERVER_PLAYER& player)
{
	return player.iCurrentHp > 0 &&
		player.isCombatReady &&
		PLAYER_ACTION_STATE::DEAD != player.eAction;
}
```

same-room은 container 자체로 보장하되 helper call site에서 명시한다. monster/Valtan 모두 `ServerPlayer.h`의 같은
predicate를 사용하며 AI01/AI02는 두 Brain을 각각 통과시킨다.
Character Select player는 current contract처럼 enter 직후 combat-ready다.

AI 순서는 유지한다.

```text
no eligible target/outside engage -> IDLE
inside engage/outside attack -> CHASE + Server navigation
inside attack -> WINDUP -> ACTIVE -> RECOVERY -> IDLE
HP zero -> DEAD -> deadDespawnMs 뒤 MONSTER만 despawn
```

forced motion pending/active monster는 target selection과 path move를 건너뛰고 reaction 종료 뒤 IDLE부터 다시 시작한다.
attack ACTIVE hit은 current legacy XZ reach를 보존하되 player body radius를 적용하는 reviewed change와 reaction profile을
별도 golden으로 검증한다. current monster attack의 `CPlayerSkillSystem::Try_Counter` gate도 damage/reaction 전에
유지하고 counter 성공 시 reaction을 queue하지 않는다. DEAD corpse는 group alive-count에는 기존처럼 despawn 전까지 남지만 damage target/F7 hurt
volume에서는 즉시 제외한다. Valtan DEAD는 자동 despawn하지 않고 room reset까지 present다.

## G22. Client Character Select controls, Valtan mirror, HUD와 F7

### 대상

- `Client/{Public,Private}/Level_CharacterSelect.*`, `Client/Public/WorldEntityCommandSink.h`,
  `Client/Public/NetworkWorldEntityCommandSink.h`, `Client/Private/NetworkWorldEntityCommandSink.cpp`, `NetworkManager.*`
- `Client/{Public,Private}/ClientReplication.*`
- `Client/{Public,Private}/Character.*`, `Npc.*`, `Valtan.*`
- `Client/{Public,Private}/CombatReactionPresentationCatalog.*`
- `Client/{Public,Private}/CombatHUDViewModel.*`, `HUDRuntimeView.*`
- `Client/{Public,Private}/CombatColliderDebug*`, `MainApp.*`
- effect 세션 merge 뒤 `Effect_PresentationService.*`와 `IEffectAnchorProvider.h`

### typed command sink

기존 파일 이름은 유지해 project churn을 줄이되 그 안의 `IWorldEntityCommandSink`/`CNetworkWorldEntityCommandSink`와
Valtan-placement 전용 request를 다음 interface/class로 교체한다. 별도 두 번째 sink 파일은 만들지 않는다.

```cpp
class IWorldActivationCommandSink
{
public:
	virtual ~IWorldActivationCommandSink() = default;
	virtual bool Request_ActivateWorldSet(
		std::string_view activationId) = 0;
};
```

`CNetworkWorldActivationCommandSink`만 제품 구현이다. sink는 sequence를 발급하고 packet을 보내지만 ID를 해석하거나
로컬 spawn하지 않는다. Preview mode에는 sink가 없고 버튼도 보이지 않는다.

sink는 accepted Server Arena handoff로 생성될 때 `m_iNextActivationSequence=1`과 bounded pending map을 초기화한다.
request는 ID/frame을 먼저 validate/build하고 pending capacity를 reserve한 뒤 현재 non-zero sequence와 activation ID를
pending에 넣고 network enqueue를 시도한다. enqueue 성공 여부와 무관하게 sequence는 `NextNonZeroCounter`로 소비한다.
enqueue 실패면 방금 pending row를 즉시 erase하고 typed local status를 반환해 button이 다시 활성화되며 local spawn은
하지 않는다. MAX 다음은 1이며 0을 전송하지 않는다. result는 sequence+activation ID가 pending row와 둘 다 일치할 때만 소비한다. disconnect, Preview
전환, level leave에서 pending/counter를 clear하고 다음 accepted handoff가 다시 1에서 시작한다. Server strict-forward
helper도 MAX-1→MAX→1을 허용하고 duplicate/0/stale을 거부한다.

`CLevel_CharacterSelect` Server Arena panel은 `S2C_WORLD_ACTIVATION_CATALOG`의 controls를 stable order로 그린다.
display name은 packet catalog 값을 쓰고 click은 activation ID만 sink에 보낸다. state ACTIVE/COMPLETED 또는 request
pending이면 disable한다. REJECTED는 status reason/result만 보이고 local retry timer나 fallback spawn을 만들지 않는다.

### `CValtan` presentation mirror

```cpp
bool m_isNetworkPresent = false;
bool m_isAlivePresentation = false;
bool m_isPatternActivePresentation = false;
std::uint32_t m_iReactionSequence = 0;
```

spawn construction 성공 후 present=true, snapshot마다 alive=`hp>0 && action!=DEAD`, pattern active=
`action==PATTERN_ACTIVE`를 mirror한다. 이 flag를 Server hit/HUD authority로 사용하지 않는다. despawn/object release에서
전부 clear한다. Valtan은 default `ALL` reaction immune이라 reaction sequence를 만들지 않는다. 기존/확장
`DAMAGE_EVENT`의 target NetEntityId로 route하고 room-lifetime `iPresentationSequence` 단일 key로 authored hit flash를
deduplicate 재생한다.
PhysX impulse로 root를 적분하지 않는다.

Valtan action clip은 G17 binding과 `(actionId,patternSequence,stageIndex,actionStartTick)` identity로 resolve한다. coarse
BossCatalog windup/active/recovery clip은 missing-binding presentation fallback만 남긴다.

### `CCombatHUDViewModel` public state

```cpp
struct HUD_BOSS_STATE final
{
	bool isValid = false;
	bool hasBoss = false;
	bool isAlive = false;
	bool isPatternActive = false;
	LostArk::Shared::NET_ENTITY_ID iNetEntityId =
		LostArk::Shared::INVALID_NET_ENTITY_ID;
	std::string strArchetypeId;
	std::string strDisplayName;
	std::uint32_t iCurrentHp = 0;
	std::uint32_t iMaximumHp = 0;
	std::uint32_t iMaximumHealthBars = 0;
	std::uint32_t iCurrentHealthBar = 0;
	std::uint32_t iPhase = 0;
	WORLD_ENTITY_ACTION eAction = WORLD_ENTITY_ACTION::IDLE;
	std::string strActionId;
	std::string strPatternId;
	std::uint32_t iPatternSequence = 0;
	std::uint32_t iPatternStageIndex = 0;
};

class CCombatHUDViewModel final
{
public:
	void Apply_Boss(
		const std::string& archetypeId,
		const WORLD_ENTITY_SNAPSHOT& snapshot);
	void Clear_Boss(LostArk::Shared::NET_ENTITY_ID netEntityId);
	void Reset_RuntimeState();
	const HUD_BOSS_STATE& Get_Boss() const;
};
```

`CClientReplication`의 world-entity record는 validated spawn identity(kind, archetypeId, encounterId, NetEntityId)를
GameObject pointer와 분리해 먼저 commit한다. lazy presentation pointer는 optional이며 prototype/model load 실패가 identity
record를 rollback하지 않는다. HUD, snapshot/despawn routing과 occurrence join은 identity record를 사용하고 rendering만
optional pointer를 사용한다.

committed identity record가 BOSS일 때 그 record의 archetypeId와 snapshot을 함께
기존 `Apply_Boss`에 전달한다. ViewModel은 non-empty archetype, valid max HP, `1..1000` maximum bars를 요구하고
기존처럼 staged `BossProfiles.json` lookup에서 displayName을 resolve한다. BossCatalog에 HUD 이름 field를 중복 추가하지
않는다. Shared helper로 current bar를 계산한다. first snapshot에서 hasBoss=true. DEAD snapshot은 hasBoss=true/isAlive=false/current bar 0을 유지해 `x0`을
보인다. matching despawn, room reset, disconnect/level leave에서 clear한다. mismatching entity despawn은 current boss를
지우지 않는다. `isValid`/presentation pointer를 alive 의미로 재사용하지 않는다.

기존 caller 호환을 위해 `Apply_Boss`, `Get_Boss`, `Reset_RuntimeState` 이름은 유지하고 matching entity만 지우는
`Clear_Boss`만 추가한다. BalanceTool/MainApp/Development/CharacterSelect/ClientReplication caller가 불필요한 rename으로
깨지지 않게 한다. 기존 `strActionId`, `strPatternId`, `iPatternSequence`, `iPatternStageIndex`도 snapshot에서 계속 채우며
Balance Tool live pattern join을 보존한다. 위 block에 생략된 `Get`, player/damage API와 private profile/cache member는
현 public/private contract를 그대로 유지하는 additive 변경이다.

상단 fallback text는 다음 state만 소비한다.

```text
발탄  x160
발탄  x130
발탄  x0
```

130은 phase 1일 수 있고 phase 2는 current 80 threshold다. UI 담당자가 제품 gauge를 구현하면
`HUDRuntimeView/CUIObject`가 같은 ViewModel을 읽고 fallback text만 제거한다. `_DEBUG` F7 ViewModel을 제품 HUD에
사용하지 않는다.

### ClientReplication hook 순서

1. spawned BOSS identity를 stage/commit하고 presentation lazy-load는 optional child state로 시도한다.
2. first snapshot 전에는 HUD/F7 hurt body를 활성화하지 않는다.
3. snapshot strict-forward 검증 뒤 presentation, HUD, reaction, occurrence, activation state를 같은 accepted snapshot으로
   갱신한다.
4. HP0/DEAD면 hurt collider는 즉시 suppress하고 HUD는 x0을 유지한다.
5. despawn은 presentation remove 뒤 matching HUD/occurrence/F7 state를 idempotent clear한다.
6. disconnect/level leave는 activation catalog, occurrences, reaction, HUD를 전부 clear한다.

player reaction은 `CCharacter::Apply_NetworkReaction`, monster reaction은 `CNpc::Apply_NetworkReaction` typed callback으로
전달한다. 두 callback은 sequence edge, Server start tick/duration과 presentation binding만 소비하고 gameplay action/HP/root를
결정하지 않는다. root는 기존 snapshot interpolation/correction을 따른다.

### F7 boss shape 확장

G07에서 확정한 final `COMBAT_DEBUG_GEOMETRY_KIND`
`LEGACY_XZ_RING,CAPSULE,CIRCLE,RING,CONE,ORIENTED_BOX,CROSS,CAPSULE_TRACK_SAMPLE,END`를 그대로 재사용한다.
G22에서 두 번째 enum을 선언하거나 player geometry 값을 교체하지 않는다.

view model은 accepted runtime revision의 debug catalog와 Server occurrence identity/anchor를 join한다. 상태/색은 다음과
같다.

```text
hurt body alive                 green
pending captured occurrence     cyan, dim
active hit occurrence           yellow
recent active occurrence ghost  orange, six ticks
actual overlap diagnostic       red, one tick
dead/despawn/cancelled           not drawn
```

owner-follow는 accepted entity snapshot pose, world/target-open은 occurrence captured anchor, track은 same tick offset의
reviewed sample을 사용한다. Server occurrence가 없는 Client animation/effect frame에서 collider를 임의 생성하지 않는다.
future platform/ground visual이 아직 없으면 event admission 자체를 `BLOCKED`로 두고 F7만 가짜 shape를 만들지 않는다.

Engine debug draw에는 ring/cone/box/cross line helper를 추가하거나 existing line/ring primitive를 compose한다. Debug
renderer만 사용하고 Release에는 catalog/parser/state/key polling이 없다.

### activation/effect failure boundary

- Client monster/Valtan lazy-load 실패는 diagnostic 후 해당 presentation만 격리한다. Server entity/HP/activation은
  rollback하지 않는다.
- publisher/ProjectAudit가 activation archetype의 catalog/resource/prototype coverage를 사전에 검사한다.
- missing pattern clip/effect는 Server combat을 유지한다.
- invalid debug catalog는 F7 unavailable/no-op이며 product buttons/HUD/network gameplay는 계속한다.

## G23. 추가 project 등록, harness와 적용 순서

### project/filter 등록

Shared:

- `Gameplay/BossCombatContract.h`, `Gameplay/CombatReactionContract.h`, `Gameplay/BossHealthContract.h`를 `ClInclude`.
- `BossCombatContract.cpp`, `CombatReactionContract.cpp`를 `ClCompile`.
- physical folder와 같은 existing filter에만 넣고 다른 항목을 이동하지 않는다.

Server:

- `CombatDisplacementSystem.cpp`를 `ClCompile`, header를 `ClInclude`.
- G03의 manifest, G04 hit runtime 파일과 함께 `Server.vcxproj/.filters`에 모두 등록한다.
- Engine/PhysX library를 Server linker에 추가하지 않는다. manifest SHA-256용 `bcrypt.lib`만 기존 AdditionalDependencies를
  보존해 추가한다.

Client:

- `BossPatternPresentationDocument.cpp/.h`, `CombatReactionPresentationCatalog.cpp/.h`, `IEffectAnchorProvider.h`와
  `AreaAuthoringTransaction.cpp/.h`, 필요한 activation sink CPP/H를
  `Client.vcxproj/.filters`에 등록한다.
- `AreaAuthoringTransaction.cpp`와 pure dependencies를 `ClientFrontendHarness.vcxproj/.filters`에도 등록한다.
- Windows CNG SHA-256을 쓰므로 Client와 ClientFrontendHarness의 Debug/Release `AdditionalDependencies`에 `bcrypt.lib`를
  기존 값 보존 방식으로 추가하고 project XML audit으로 두 consumer를 검사한다.
- `LOSTARK_CLIENT_FRONTEND_HARNESS`를 ClientFrontendHarness Debug/Release `PreprocessorDefinitions`에 inherited definition을
  보존해 추가하고 제품 Client에는 정의하지 않는다. failure injection API는 이 macro에서만 실행 가능하다.
- 신규 Data authoring JSON은 `96.DataFiles` 아래 `None`만 사용한다.
- generated bootstrap/debug runtime은 project `Content` item으로 추가하지 않는다.

모든 새 C++는 UTF-8 BOM 없음, 기존 C++ 수정은 현재 encoding을 유지한다.

### publisher/schema exact cases

```text
VALPUB01 v3 31-pattern converter -> stage/action/duration/pulse tick/target/damage parity
VALPUB02 unknown anchor/extent/shape/motion/reaction -> fail, old runtime set retained
VALPUB03 capture>start, start>=end, end>duration -> fail
VALPUB04 duplicate event/track, non-monotonic sample, NaN/radius cap -> fail; legacy reach 100 accepted, >128 rejected
VALPUB05 source seed/model/binding document/clip ID/sequence/startOffset/playRate/sampleHz/bone drift on APPROVED track -> fail
VALPUB06 presentation trace drift -> presentation audit fail, Server root-based event remains valid
REACT01 malformed version/enum/ID/duration/scale/immunity -> fail
REACT02 missing player/monster/boss body-reaction profile join -> fail
REACT03 final PROJECT_TUNED overlay -> every damage player event, all four monster attacks and Valtan heavy/charge IDs exact
REACT04 actor response matrix -> six players/three regular=1.0, Lugaru=0.35, Valtan=0+ALL
WORLDPUB01 three Character Select sets and overlapping resources -> pass
WORLDPUB02 unknown/cross-Area/enabled boss/duplicate ref -> fail
WORLDPUB02A manual resource also referenced by activateSpawnGroup/activateEncounter trigger -> fail
WORLDPUB03 IMMEDIATE group prerequisite/multi-wave/nonzero delay/count>maxAlive -> fail
WORLDPUB04 every activation spawn anchor nav/height -> pass
WORLDPUB04A Character Select MAP_LOAD_SCOPE/presentation coverage -> ProjectAudit + full Client smoke pass
WORLDPUB05 expected-absent Bern/Training stale spawn artifact -> staged delete; injected failure restores old file
WORLDPUB05A required Valtan/CharacterSelect SpawnGroups source deletion -> publish fail, old runtime set retained
MAN04 gameplay + four world + two spawn + Server/Client nav grids + Client debug artifact hashes -> pass
MAPSAVE01 gameplay + required SpawnGroups + dirty nav layers -> one overlay validation and atomic commit
MAPSAVE02 Bern/Training SpawnGroups ABSENT -> no empty source created; Gameplay/nav commit succeeds
MAPSAVE03 strict reload/cross-join/publisher callback failure -> zero destination mutation, draft revision/dirty preserved
MAPSAVE04 inject failure after every ordered layer mutation -> every byte/ABSENT state rolled back, draft state preserved
MAPSAVE05 crash after journal/layer/last-layer before marker -> startup recovery restores exact old set before editor enable
MAPSAVE06 committed journal/new-set startup -> no rollback; temp/backup cleanup is idempotent
MAPSAVE07 duplicate role, missing required role, cross-role same destination or escaped path -> reject, zero mutation
MAPSAVE08 visual Map placement or destruction/simulation dirty with Save All/Continue -> reject before every write
MAPSAVE09 rollback write/flush failure -> RECOVERY_REQUIRED, editor disabled until exact old-set recovery
MAPSAVE10 destruction World Events/Simulations buttons route to destruction pair; mixed Area switch remains cancelable
```

Runtime-set artifact inventory는 gameplay 1, world 4, required spawn 2, Server navigation 3, Client navigation 3,
Client debug 1의 artifact row 14개와 manifest 1개다. 추가 authoring은 Gameplay.bootstrap/World bootstrap 안에
publish되므로 그 밖의 destination 수를 늘리지 않는다.

### Shared geometry/reaction cases

```text
BOSS_GEO01 CIRCLE/RING legacy root-point exact boundary parity
BOSS_GEO02 CONE yaw 0/90/180/270 legacy parity
BOSS_GEO03 FORWARD_BOX/CROSS legacy parity
BOSS_GEO04 footprint tangent inclusive; epsilon outside miss
BOSS_GEO05 reviewed full weapon capsule tangent/vertical separation
BOSS_GEO06 track previous-current sample sweep prevents tunneling
BOSS_GEO07 invalid/ambiguous shape union rejected
BOSS_GEO08 extent×shape allowed matrix complete; analytic+FULL and track+footprint rejected
HEALTH01 60000/60000/160 -> 160
HEALTH02 48750/60000/160 -> 130 and phase remains 1
HEALTH03 30000/60000/160 -> 80 and phase 2 is independent
HEALTH04 zero/dead -> 0; invalid max/bars -> 0
REACTION01 deterministic priority/tie order
REACTION02 direction zero fallback/rejection
```

### Server pattern/forced-motion cases

```text
VAL01 before capture -> no occurrence/no hit
VAL02 captured pending ground event -> fixed world anchor survives boss/target movement
VAL03 active half-open event -> exactly one hit per target
VAL04 stage cancel/death -> occurrence removed, no later hit
VAL05 owner-follow charge -> motion commit then collider uses same-tick root
VAL06 charge wall/nav stop -> safe pose, event identity remains deterministic
VAL07 migrated all pulses -> old first-fire tick/count/damage target parity
VAL07A migrated legacy event with 5+ eligible players -> all current-room targets preserved up to cap 32
VAL08 reviewed hurt footprint/capsule -> body tangent in, outside miss
VAL09 current counter window overlap -> counter succeeds, damage/reaction suppressed, pattern transition parity
VAL10 initial pattern age0 captures on current tick; transitioned stage age0 starts only at NextNonZeroTick
VAL11 eligible target tie -> distance then NetEntityId; captured target ID remains fixed for motion/event lifetime
VAL12 multi-tick new entrants -> event ledger exclusion + remaining cap, never exceeds maximumTargets
KB01 player skill pushes regular monster, Valtan displacement immune
KB02 Valtan/monster hit pushes player and cancels current action
KB03 same target simultaneous requests -> priority/source/event deterministic winner
KB04 lethal damage -> no forced motion
KB05 wall/nav stop -> last safe root, no penetration
KB06 forced monster skips chase/path until reaction end then IDLE
KB07 cancel/death/despawn/room reset -> reaction state cleared
KB08 active reaction: strictly higher priority replaces; equal/lower reaction rejected while damage remains
KB09 nav/wall early stop -> next NONE clears reaction clip and presentation arc immediately
KB10 arc late seek uses Server tick parabola on Body/Model local only; replacement/death/despawn clears to zero
KB11 active/pending reaction blocks trigger Begin/Update; admitted hit clears TRIGGER_MOVE payload before displacement
KB12 active reaction MOVE/USE_SKILL consumes valid sequence but changes no path/resource/cooldown/action; RELEASE is no-op;
     USE/RELEASE common action stream rejects cross-type stale/replay sequence
AI01 no eligible player IDLE; eligible player CHASE; in range WINDUP/ACTIVE/RECOVERY
AI02 HP-only but not combat-ready player is excluded by new shared predicate
AI03 DEAD corpse excluded from hit/F7 but alive-count parity retained until despawn
AI04 monster attack counter gate -> current success/damage suppression parity, reaction zero
```

### activation Server cases

```text
ACT01 Spawn Monsters -> all authored entries committed same tick, stable order
ACT02 Spawn Valtan -> disabled placement committed once at authored transform
ACT03 Spawn All from dormant -> boss+monsters all-or-none
ACT04 Monsters then All -> only Valtan added, duplicate monster zero
ACT05 Valtan then All -> only monsters added, duplicate boss zero
ACT06 concurrent same/different set request -> resource ledger prevents duplicate
ACT07 invalid nav/profile/body/entity cap/NetEntityId headroom -> zero partial mutation
ACT07A staged current and MAX_PLAYERS_PER_ROOM late-join + max-damage/max-occurrence/max-reaction future frames at
       MAX_PACKET_BYTES boundary pass;
       +1 byte -> CAPACITY_EXCEEDED, zero mutation
ACT08 wrong world/session/revision/ID/sequence -> REJECTED
ACT09 late join -> entity + control/state replay
ACT10 one player leaves while another remains -> no reset
ACT11 last player leaves -> manual entities despawn, resource ledger/group reset, baseline retained
ACT12 next room generation -> set works again, NetEntityId remains monotonic
ACT13 killed/completed resource before room empty -> ALREADY_ACTIVE, no manual respawn
```

### Network/Client cases

```text
NET05 activation request/result/catalog roundtrip and caps
NET06 reaction snapshot roundtrip/invalid enum/non-finite/truncated/trailing
NET07 combat occurrence capture/start-delay/duration/anchor roundtrip, zero-skip wrap and cap 64
NET08 maximumHealthBars boss/non-boss validation
NET09 activation state cap/duplicate ID/generation validation
NET10 player/Valtan/monster DAMAGE_EVENT source/target/sequence/hitEventId roundtrip and exact end offset;
      zero/duplicate sequence, invalid source, empty/oversize/NUL ID, truncated/trailing reject with destination unchanged;
      duplicate snapshot does not replay Client hit flash
UI01 Preview mode -> no activation controls/no boss HUD
UI02 Server Arena catalog -> exact three buttons; pending/active disable
UI02A packet build/enqueue failure or pending-cap rejection -> pending row erased, no local spawn; retry uses next nonzero sequence
UI03 spawn first snapshot -> HUD x160; HP48750 -> x130 phase1; HP30000 -> x80 phase2
UI04 DEAD -> x0 remains; matching despawn/reset/disconnect -> clear
UI05 unrelated despawn -> current boss HUD preserved
UI06 CCharacter/CNpc reaction sequence edge once; duplicate snapshot no replay; root follows Server
UI06A missing reaction binding/clip -> diagnostic, action pose fallback, forced root movement preserved
UI06B late reaction snapshot -> startOffset + age*playRate seek, remaining wall priority only; expired snapshot no replay
UI06C presentation arc uses Server age on Body/Model local only; NONE/replacement/death/despawn clears immediately
EFX01 CAPTURE and ACTIVE_START bindings spawn at their exact occurrence phase tick only
EFX02 late CAPTURE/ACTIVE_START uses playback-rate sample seek and phase-specific remaining wall lifetime
EFX03 duplicate snapshot same occurrence/binding/phase does not replay; distinct binding on same occurrence both spawn
EFX04 OCCURRENCE_END, ACTION_END, NATURAL_END stop at only their declared terminal; teardown always clears;
      SERVER_OCCURRENCE+ACTION_END empty action ID/start tick rejects
EFX05 NATURAL_END finishing early keeps consumed ledger until occurrence disappears
EFX06 owner-follow, captured explicit-world and b_wp_r_01 body-visual-root anchor composition match contract
EFX07 prewarm/binding/asset failure isolates presentation; Server spawn/damage/activation remains committed
DBG18 pending/active/ghost boss shapes use occurrence anchor and reviewed catalog
DBG19 DEAD/despawn/cancel -> boss hurt/hit shapes zero
DBG20 missing effect/binding/debug catalog -> gameplay/buttons/HUD authority preserved
```

### ProjectAudit/public 문서

다음 public 계약을 같은 implementation commit에서 갱신한다.

- `AGENTS.md`, `CLAUDE.md`
- `.md/TEAM/TEAM_GAMEPLAY_INTERFACE_HANDBOOK.md`
- `.md/TEAM/AREA_DATA_LAYER_GUIDE.md`
- `.md/TEAM/ANIMATION_TOOL_OWNER_HANDOFF.md`
- `.md/TEAM/BALANCE_TOOL_OWNER_HANDOFF.md`

ProjectAudit은 다음을 추가 검사한다.

- gameplay collider/damage/reaction field가 Effect JSON/Valtan animevent에 없다.
- Server source가 model/clip/bone/effect path나 PhysX를 읽지 않는다.
- Animation weapon seed와 reviewed gameplay track의 단방향 hash join이 유효하다.
- Valtan/monster→player reaction ID마다 여섯 class, player→monster reaction ID마다 네 supported combat archetype의
  실제 clip binding이 완전하다.
- Character Select activation set 세 개와 required spawn artifact가 complete하다.
- 제품에 `CMonster`/`vector<CMonster>`가 새로 생기지 않았다.
- buttons는 typed activation sink만 사용하고 packet/socket/local spawn을 직접 호출하지 않는다.
- boss HUD는 `CCombatHUDViewModel`만 소비하고 `_DEBUG` collider view model을 읽지 않는다.
- Valtan max bar 160과 current 130/phase 1 독립 계약이 유지된다.

### 최종 적용 순서

```text
1. 현재 effect/HDR/network/map 세션이 모두 commit/merge됐는지 확인
2. current main에서 sibling clean worktree와 codex/combat-capsule-f7-debug branch 생성
3. 두 PLAN을 SHA-256 확인 후 sibling에 복사/commit 대상으로 추가
4. G00~G05 player body/skill authority, 해당 base G12 project 등록과 기존 harness 완료
5. G16 Valtan v3→v4 pure-legacy migration/BossCombatContract와 해당 G12/G23 project 등록 + parity harness
6. G18 reaction catalog/body fields/Shared helpers/room-owned displacement system과 해당 project 등록
7. G20 Map Tool data/schema/navigation/runtime-set publisher + real Character Select placement authoring
8. G19 protocol v13 reaction/occurrence/activation packet + NetworkProtocolHarness
9. G21 atomic activation/room reset/existing monster AI + Server contract tests
10. G17 action binding/Effect binding/weapon seed/reviewed track/Valtan presentation bridge와 신규 Client file 등록
11. G06~G11 Client debug runtime/view/renderer/F7/revision/effect trace와 project 등록을 boss contract 위에 구현
12. G22 Character Select buttons, CCharacter/CNpc reaction, Valtan mirror, HUD와 boss F7 연결
13. G12와 G23의 Shared/Server/Client project/filter/Data 등록 누락·중복 audit 완료
14. G13/G23 publisher/schema/rollback script tests와 G14 public key/data ownership 문서 반영
15. Engine x64 Debug/Release build
16. UpdateLib.bat Debug/Release
17. Shared + NetworkProtocolHarness x64 Debug/Release build와 harness 실행
18. Server x64 Debug/Release build와 `Server.exe --contract-test`
19. Client x64 Debug/Release build
20. ClientFrontendHarness와 structured manual Server+Client smoke
21. G15/G23 final ProjectAudit, JSON/XML parse, project XML audit, git diff --check
22. RESULT에 자동/수동/미완료를 분리하고 하나의 검증 단위 commit/push
```

manual smoke는 이미지 캡처/비교 없이 entity ID, activation/resource ID, occurrence tick/anchor, reaction sequence/root,
HUD state를 structured diagnostics로 대조하고 최종 화면은 사용자가 직접 확인한다.
