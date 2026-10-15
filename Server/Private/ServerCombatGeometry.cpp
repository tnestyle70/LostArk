#include "ServerCombatGeometry.h"

#include <algorithm>
#include <cmath>

namespace
{
	// 거리 제곱이 이 값 이하인 선분은 한 점으로 취급해 0에 가까운 값으로 나누지 않는다.
	constexpr float EPSILON = 0.000001f;

	// SweptCircle_Overlaps가 사용하는 점과 선분 사이의 최소 거리 제곱(m²)이다.
	// 내적 투영 비율을 0~1로 제한하여 무한 직선이 아닌 실제 이동 구간의 최근접점을 구한다.
	float DistanceSquaredToSegment(
		const float pointX,
		const float pointZ,
		const float startX,
		const float startZ,
		const float endX,
		const float endZ)
	{
		const float segmentX = endX - startX;
		const float segmentZ = endZ - startZ;
		const float lengthSquared = segmentX * segmentX + segmentZ * segmentZ;
		if (lengthSquared <= EPSILON)
		{
			const float deltaX = pointX - startX;
			const float deltaZ = pointZ - startZ;
			return deltaX * deltaX + deltaZ * deltaZ;
		}
		const float projection = (std::clamp)(
			((pointX - startX) * segmentX + (pointZ - startZ) * segmentZ) /
				lengthSquared,
			0.f,
			1.f);
		const float nearestX = startX + segmentX * projection;
		const float nearestZ = startZ + segmentZ * projection;
		const float deltaX = pointX - nearestX;
		const float deltaZ = pointZ - nearestZ;
		return deltaX * deltaX + deltaZ * deltaZ;
	}
}

// 생성/판정 경계가 사용하는 도형 계약 검사다. 모든 float 필드의 유한성을 먼저 검사한다.
// 종류별 크기 관계와 미사용 필드의 0을 검사하고 END/알 수 없는 enum은 false다.
bool LostArk::Server::CServerCombatGeometry::Is_Valid(
	const SERVER_COMBAT_SHAPE_XZ& shape) noexcept
{
	if (!std::isfinite(shape.fOffset) ||
		!std::isfinite(shape.fOuterRadius) ||
		!std::isfinite(shape.fInnerRadius) ||
		!std::isfinite(shape.fLength) ||
		!std::isfinite(shape.fHalfWidth) ||
		!std::isfinite(shape.fAngleDegrees))
	{
		return false;
	}
	switch (shape.eKind)
	{
	case SERVER_COMBAT_SHAPE_KIND::CIRCLE:
		return shape.fOuterRadius > 0.f && 0.f == shape.fInnerRadius &&
			0.f == shape.fLength && 0.f == shape.fHalfWidth &&
			0.f == shape.fAngleDegrees;
	case SERVER_COMBAT_SHAPE_KIND::RING:
		return shape.fInnerRadius > 0.f &&
			shape.fOuterRadius > shape.fInnerRadius &&
			0.f == shape.fLength && 0.f == shape.fHalfWidth &&
			0.f == shape.fAngleDegrees;
	case SERVER_COMBAT_SHAPE_KIND::FORWARD_BOX:
		return shape.fLength > 0.f && shape.fHalfWidth > 0.f &&
			0.f == shape.fOuterRadius && 0.f == shape.fInnerRadius &&
			0.f == shape.fAngleDegrees;
	case SERVER_COMBAT_SHAPE_KIND::CONE:
		return shape.fLength > 0.f && shape.fAngleDegrees > 0.f &&
			shape.fAngleDegrees <= 360.f && 0.f == shape.fOuterRadius &&
			shape.fInnerRadius >= 0.f && shape.fInnerRadius < shape.fLength &&
			0.f == shape.fHalfWidth;
	default:
		return false;
	}
}

// 도형과 pose의 유효 수치를 검사한 뒤 전방 offset을 적용해 공용 XZ 교차 함수로 전달한다.
// 상태를 바꾸거나 HP를 적용하지 않는다. forward의 정규화는 이 함수에서 하지 않는다.
bool LostArk::Server::CServerCombatGeometry::Overlaps_Pose(
	const SERVER_COMBAT_SHAPE_XZ& shape,
	const float originX,
	const float originZ,
	const float forwardX,
	const float forwardZ,
	const LostArk::Shared::CombatCollision::BODY_CIRCLE_XZ& target)
{
	using namespace LostArk::Shared::CombatCollision;
	if (!Is_Valid(shape) || !std::isfinite(originX) ||
		!std::isfinite(originZ) || !std::isfinite(forwardX) ||
		!std::isfinite(forwardZ))
	{
		return false;
	}
	const float shapeX = originX + forwardX * shape.fOffset;
	const float shapeZ = originZ + forwardZ * shape.fOffset;
	switch (shape.eKind)
	{
	case SERVER_COMBAT_SHAPE_KIND::CIRCLE:
		return Circles_Overlap(
			CIRCLE_XZ{ shapeX, shapeZ, shape.fOuterRadius }, target);
	case SERVER_COMBAT_SHAPE_KIND::RING:
		return Circle_IntersectsRing(
			target, shapeX, shapeZ,
			shape.fInnerRadius, shape.fOuterRadius);
	case SERVER_COMBAT_SHAPE_KIND::FORWARD_BOX:
		return Circle_IntersectsForwardBox(
			target, shapeX, shapeZ, forwardX, forwardZ,
			shape.fLength, shape.fHalfWidth);
	case SERVER_COMBAT_SHAPE_KIND::CONE:
	{
		// 안쪽 반경이 있으면 ring 조건을 먼저 적용한다.
		// 각도 360 이상은 방향 검사를 생략하고, 그보다 작으면 부채꼴 조건도 동시에 만족해야 한다.
		const bool inRing = shape.fInnerRadius > 0.f ?
			Circle_IntersectsRing(
				target, shapeX, shapeZ,
				shape.fInnerRadius, shape.fLength) :
			Circles_Overlap(CIRCLE_XZ{ shapeX, shapeZ, shape.fLength }, target);
		return inRing && (shape.fAngleDegrees >= 360.f ||
			Circle_IntersectsCone(
				target, shapeX, shapeZ, forwardX, forwardZ,
				shape.fLength, shape.fAngleDegrees));
	}
	default:
		return false;
	}
}

// combat object와 위협 예측이 두 고정 시점 사이의 원 이동을 검사한다. 좌표/반경 단위는 m다.
// 대상 중심에서 이동 선분까지의 거리와 두 반경 합을 제곱으로 비교하므로 제곱근을 계산하지 않는다.
// 접촉 경계는 포함하며 공격 반경은 양수, 대상 반경은 0 이상이어야 한다.
bool LostArk::Server::CServerCombatGeometry::SweptCircle_Overlaps(
	const float startX,
	const float startZ,
	const float endX,
	const float endZ,
	const float contactRadius,
	const LostArk::Shared::CombatCollision::BODY_CIRCLE_XZ& target)
{
	if (!std::isfinite(startX) || !std::isfinite(startZ) ||
		!std::isfinite(endX) || !std::isfinite(endZ) ||
		!std::isfinite(contactRadius) || contactRadius <= 0.f ||
		!std::isfinite(target.fCenterX) || !std::isfinite(target.fCenterZ) ||
		!std::isfinite(target.fRadius) || target.fRadius < 0.f)
	{
		return false;
	}
	const float combinedRadius = contactRadius + target.fRadius;
	return DistanceSquaredToSegment(
		target.fCenterX, target.fCenterZ, startX, startZ, endX, endZ) <=
		combinedRadius * combinedRadius;
}
