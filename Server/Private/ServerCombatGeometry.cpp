#include "ServerCombatGeometry.h"

#include <algorithm>
#include <cmath>

namespace
{
	constexpr float EPSILON = 0.000001f;

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
