#pragma once

namespace LostArk::Shared::CombatCollision
{
	// Server 피해/이동 후보와 Client 표현이 공유하는 XZ 겹침 검사다. 높이·HP·대상 선택은 호출자가 맡는다.
	// 좌표와 반경은 같은 m 단위이며 접촉 경계에는 아래 허용 오차를 더한다.
	inline constexpr float CONTACT_EPSILON = 0.0001f;

	struct BODY_CIRCLE_XZ final
	{
		float fCenterX = 0.f;
		float fCenterZ = 0.f;
		float fRadius = 0.f;
	};

	struct CIRCLE_XZ final
	{
		float fCenterX = 0.f;
		float fCenterZ = 0.f;
		float fRadius = 0.f;
	};

	// 중심 좌표가 유한하고 반경이 양수여야 한다. 반경 0의 점은 유효한 몸체 원으로 취급하지 않는다.
	[[nodiscard]] bool Is_Valid(const BODY_CIRCLE_XZ& circle) noexcept;
	[[nodiscard]] bool Is_Valid(const CIRCLE_XZ& circle) noexcept;

	[[nodiscard]] bool Circles_Overlap(
		const CIRCLE_XZ& circle,
		const BODY_CIRCLE_XZ& target) noexcept;
	[[nodiscard]] bool Circles_Overlap(
		const BODY_CIRCLE_XZ& left,
		const BODY_CIRCLE_XZ& right) noexcept;

	// 대상 원이 도넛 띠와 닿는지 검사한다. 안쪽 반경 0과 inner==outer도 이 기하 API는 허용한다.
	[[nodiscard]] bool Circle_IntersectsRing(
		const BODY_CIRCLE_XZ& target,
		float centerX,
		float centerZ,
		float innerRadius,
		float outerRadius) noexcept;

	// 원점을 뒤쪽 중앙으로 두고 forward 0~length, right -halfWidth~halfWidth인 상자를 검사한다.
	// 아래 방향을 쓰는 함수는 유효한 0 아닌 forward를 내부에서 정규화한다.
	[[nodiscard]] bool Circle_IntersectsForwardBox(
		const BODY_CIRCLE_XZ& target,
		float originX,
		float originZ,
		float forwardX,
		float forwardZ,
		float length,
		float halfWidth) noexcept;

	// 중심에서 앞뒤·좌우로 뻗은 두 직사각형의 합집합이다. 길이와 폭 입력은 각각 반크기다.
	[[nodiscard]] bool Circle_IntersectsCross(
		const BODY_CIRCLE_XZ& target,
		float centerX,
		float centerZ,
		float forwardX,
		float forwardZ,
		float halfLength,
		float halfWidth) noexcept;

	// 중심을 지나는 0/60/120 degree 방향의 세 띠를 합쳐 여섯 갈래를 만든다.
	[[nodiscard]] bool Circle_IntersectsSixDirections(
		const BODY_CIRCLE_XZ& target,
		float centerX,
		float centerZ,
		float forwardX,
		float forwardZ,
		float halfLength,
		float halfWidth) noexcept;

	// 공격 원점에서 대상까지의 선분이 엄폐 원과 닿는지 검사한다. 양 끝점의 원 내부 여부만 보는 검사는 아니다.
	// 엄폐물 옆의 대상을 잘못 보호하지 않도록 가장 가까운 선분 지점까지의 거리를 사용한다.
	[[nodiscard]] bool Segment_IntersectsCircle(
		float startX,
		float startZ,
		float endX,
		float endZ,
		const CIRCLE_XZ& circle) noexcept;

	// angleDegrees는 반각이 아닌 전체 각도다. 대상 중심뿐 아니라 몸체 원이 경계에 걸치는 경우도 검사한다.
	[[nodiscard]] bool Circle_IntersectsCone(
		const BODY_CIRCLE_XZ& target,
		float originX,
		float originZ,
		float forwardX,
		float forwardZ,
		float length,
		float angleDegrees) noexcept;

	// 지역 X는 오른쪽, Z는 전방이다. 단위 부채꼴의 각도를 먼저 정한 뒤 X/Z 반경으로 타원을 만든다.
	// reverse는 같은 타원에서 나머지 각도 영역을 선택한다. 반전 전 0도는 빈 영역, 360도는 전체 타원이다.
	// 대상 몸체의 원 반경은 타원 좌표로 찌그러뜨리지 않고 실제 m 거리로 비교한다.
	[[nodiscard]] bool Circle_IntersectsEllipticSector(
		const BODY_CIRCLE_XZ& target,
		float originX,
		float originZ,
		float forwardX,
		float forwardZ,
		float radiusX,
		float radiusZ,
		float angleDegrees,
		bool reverse = false) noexcept;
}
