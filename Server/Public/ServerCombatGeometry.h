#pragma once

#include "Gameplay/CombatCollisionContract.h"

#include <cstdint>

namespace LostArk::Server
{
	enum class SERVER_COMBAT_SHAPE_KIND : std::uint8_t
	{
		CIRCLE,
		RING,
		FORWARD_BOX,
		CONE,
		END
	};

	// 공격 주체와 독립된 XZ 판정 도형이다. 플레이어 스킬/보스 combat object 또는 stage contact를 이 형식으로 옮긴다.
	// 도형 판정 함수는 Catalog의 개별 스킬/stage 구조를 알 필요가 없으며 HP와 대상 선택은 소비자가 맡는다.
	struct SERVER_COMBAT_SHAPE_XZ final
	{
		SERVER_COMBAT_SHAPE_KIND eKind = SERVER_COMBAT_SHAPE_KIND::END;
		// 원점에서 전방으로 이동한 거리(m)다. 방향 입력이 단위 벡터라는 사용 계약에서 저작 거리와 일치한다.
		float fOffset = 0.f;
		// 거리 필드는 m, fAngleDegrees는 전체 부채꼴 각도(degree)다. 사용하지 않는 도형 필드는 0이어야 한다.
		float fOuterRadius = 0.f;
		float fInnerRadius = 0.f;
		float fLength = 0.f;
		float fHalfWidth = 0.f;
		float fAngleDegrees = 0.f;
		// 대상 수 제한은 호출자가 소비한다. 아래 도형 유효성/겹침 검사 자체는 이 값을 적용하지 않는다.
		std::uint32_t iMaximumTargets = 0u;
	};

	class CServerCombatGeometry final
	{
	public:
		// 생성/도형 소비 경로가 유한 수치와 종류별 필수/미사용 필드를 검사한다. 외부 참조나 최대 대상 수는 검사하지 않는다.
		[[nodiscard]] static bool Is_Valid(
			const SERVER_COMBAT_SHAPE_XZ& shape) noexcept;
		// combat object, Valtan contact, 위협 예측이 주어진 위치/전방과 대상 몸체 원의 교차를 조회한다.
		// 입력은 바꾸지 않는다. 이 함수는 전방 벡터를 정규화하지 않으므로 호출자가 올바른 전방을 제공한다.
		[[nodiscard]] static bool Overlaps_Pose(
			const SERVER_COMBAT_SHAPE_XZ& shape,
			float originX,
			float originZ,
			float forwardX,
			float forwardZ,
			const LostArk::Shared::CombatCollision::BODY_CIRCLE_XZ& target);
		// 움직이는 원의 두 위치를 선분으로 연결하고 대상 반경에 공격 반경을 더해 검사한다.
		// 그 사이에 있는 대상을 끝 위치 하나만 검사해서 놓치지 않도록 이동 구간을 포함한다.
		[[nodiscard]] static bool SweptCircle_Overlaps(
			float startX,
			float startZ,
			float endX,
			float endZ,
			float contactRadius,
			const LostArk::Shared::CombatCollision::BODY_CIRCLE_XZ& target);
	};
}
