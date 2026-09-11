#include "Gameplay/CombatCollisionContract.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>

namespace
{
	using LostArk::Shared::CombatCollision::BODY_CIRCLE_XZ;
	using LostArk::Shared::CombatCollision::CIRCLE_XZ;

	constexpr double DIRECTION_EPSILON_SQUARED = 0.000000000001;
	constexpr double PI = 3.14159265358979323846;
	constexpr double DEGREES_TO_RADIANS = PI / 180.0;

	struct VECTOR2 final
	{
		double x = 0.0;
		double z = 0.0;
	};

	bool IsFinite(const float value) noexcept
	{
		return std::isfinite(value);
	}

	bool IsPositiveFinite(const float value) noexcept
	{
		return IsFinite(value) && value > 0.f;
	}

	template<typename TCircle>

	bool IsValidCircle(const TCircle& circle) noexcept
	{
		return IsFinite(circle.fCenterX) &&
			IsFinite(circle.fCenterZ) &&
			IsPositiveFinite(circle.fRadius);
	}

	double LengthSquared(const VECTOR2& value) noexcept
	{
		return value.x * value.x + value.z * value.z;
	}

	double DistanceSquared(const VECTOR2& left, const VECTOR2& right) noexcept
	{
		return LengthSquared({ left.x - right.x, left.z - right.z });
	}

	bool NormalizeDirection(
		const float forwardX,
		const float forwardZ,
		VECTOR2& outForward,
		VECTOR2& outRight) noexcept
	{
		if (!IsFinite(forwardX) || !IsFinite(forwardZ))
			return false;
		const VECTOR2 forward{ forwardX, forwardZ };
		const double lengthSquared = LengthSquared(forward);
		if (!std::isfinite(lengthSquared) ||
			lengthSquared <= DIRECTION_EPSILON_SQUARED)
		{
			return false;
		}
		const double inverseLength = 1.0 / std::sqrt(lengthSquared);
		outForward = { forward.x * inverseLength, forward.z * inverseLength };
		outRight = { outForward.z, -outForward.x };
		return true;
	}

	bool WithinInclusiveRadius(
		const double distanceSquared,
		const double radius) noexcept
	{
		const double inclusiveRadius = radius +
			LostArk::Shared::CombatCollision::CONTACT_EPSILON;
		return distanceSquared <= inclusiveRadius * inclusiveRadius;
	}

	double DistanceSquaredToLocalRectangle(
		const double localForward,
		const double localRight,
		const double minimumForward,
		const double maximumForward,
		const double minimumRight,
		const double maximumRight) noexcept
	{
		const double closestForward = std::clamp(
			localForward, minimumForward, maximumForward);
		const double closestRight = std::clamp(
			localRight, minimumRight, maximumRight);
		const double deltaForward = localForward - closestForward;
		const double deltaRight = localRight - closestRight;
		return deltaForward * deltaForward + deltaRight * deltaRight;
	}

	double DistanceSquaredToSegment(
		const VECTOR2& point,
		const VECTOR2& start,
		const VECTOR2& end) noexcept
	{
		const VECTOR2 segment{ end.x - start.x, end.z - start.z };
		const double segmentLengthSquared = LengthSquared(segment);
		if (segmentLengthSquared <= DIRECTION_EPSILON_SQUARED)
			return DistanceSquared(point, start);
		const VECTOR2 fromStart{ point.x - start.x, point.z - start.z };
		const double projection = std::clamp(
			(fromStart.x * segment.x + fromStart.z * segment.z) /
				segmentLengthSquared,
			0.0,
			1.0);
		const VECTOR2 closest{
			start.x + segment.x * projection,
			start.z + segment.z * projection
		};
		return DistanceSquared(point, closest);
	}

	using POLYNOMIAL = std::array<double, 5>;
	using POLYNOMIAL_ROOTS = std::array<double, 4>;

	double EvaluatePolynomial(const POLYNOMIAL& coefficients, int degree,
		const double value) noexcept
	{
		double result = coefficients[degree];
		while (degree > 0)
			result = result * value + coefficients[--degree];
		return result;
	}

	// A polynomial is monotone between consecutive roots of its derivative.
	// Recursing down to degree one isolates every real quartic root, including
	// double roots, without sampling an arc or allocating during a combat tick.
	int FindPolynomialRoots(const POLYNOMIAL& coefficients, int degree,
		const double low, const double high, POLYNOMIAL_ROOTS& roots) noexcept
	{
		while (degree > 0 && coefficients[degree] == 0.0)
			--degree;
		if (degree == 0)
			return 0;
		if (degree == 1)
		{
			const double root = -coefficients[0] / coefficients[1];
			if (root < low || root > high)
				return 0;
			roots[0] = root;
			return 1;
		}
		POLYNOMIAL derivative{};
		for (int i = 1; i <= degree; ++i)
			derivative[i - 1] = coefficients[i] * i;
		POLYNOMIAL_ROOTS criticalPoints{};
		const int criticalCount = FindPolynomialRoots(
			derivative, degree - 1, low, high, criticalPoints);
		int count = 0;
		const auto append = [&](const double root)
		{
			if (count < degree && (count == 0 ||
				std::abs(roots[count - 1] - root) > 1.e-12))
				roots[count++] = root;
		};
		double left = low;
		double leftValue = EvaluatePolynomial(coefficients, degree, left);
		if (std::abs(leftValue) <= 1.e-14)
			append(left);
		for (int i = 0; i <= criticalCount; ++i)
		{
			const double right = i == criticalCount ? high : criticalPoints[i];
			const double rightValue = EvaluatePolynomial(coefficients, degree, right);
			if ((leftValue < 0.0 && rightValue > 0.0) ||
				(leftValue > 0.0 && rightValue < 0.0))
			{
				double a = left, b = right, valueA = leftValue;
				for (int iteration = 0; iteration < 48; ++iteration)
				{
					const double midpoint = (a + b) * 0.5;
					const double value = EvaluatePolynomial(coefficients, degree, midpoint);
					if ((valueA < 0.0) == (value < 0.0))
					{
						a = midpoint;
						valueA = value;
					}
					else
						b = midpoint;
				}
				append((a + b) * 0.5);
			}
			if (std::abs(rightValue) <= 1.e-14)
				append(right);
			left = right;
			leftValue = rightValue;
		}
		return count;
	}

	double DistanceSquaredToEllipseArc(const VECTOR2& point,
		const double radiusX, const double radiusZ,
		const double startAngle, const double endAngle) noexcept
	{
		// Reflect each quadrant into [0, pi/2], so t = tan(theta/2) stays
		// in [0,1]. Distance extrema on (a sin(theta), b cos(theta)) satisfy
		// a*x*t^4 + 2*(b*z-a^2+b^2)*t^3 + 2*(a^2-b^2+b*z)*t-a*x=0.
		const double ax = radiusX * point.x;
		const double bz = radiusZ * point.z;
		const double difference = radiusX * radiusX - radiusZ * radiusZ;
		POLYNOMIAL coefficients{
			-ax, 2.0 * (difference + bz), 0.0,
			2.0 * (bz - difference), ax };
		double scale = 0.0;
		for (const double value : coefficients)
			scale = (std::max)(scale, std::abs(value));
		if (scale > 0.0)
			for (double& value : coefficients)
				value /= scale;
		const double low = std::tan(startAngle * 0.5);
		const double high = std::tan(endAngle * 0.5);
		const auto distanceAt = [&](const double t)
		{
			const double divisor = 1.0 + t * t;
			return DistanceSquared(point,
				{ radiusX * 2.0 * t / divisor, radiusZ * (1.0 - t * t) / divisor });
		};
		double minimum = (std::min)(distanceAt(low), distanceAt(high));
		POLYNOMIAL_ROOTS roots{};
		const int count = FindPolynomialRoots(coefficients, 4, low, high, roots);
		for (int i = 0; i < count; ++i)
			minimum = (std::min)(minimum, distanceAt(roots[i]));
		return minimum;
	}

	bool BodiesOverlap(
		const double leftX,
		const double leftZ,
		const double leftRadius,
		const BODY_CIRCLE_XZ& right) noexcept
	{
		const VECTOR2 delta{
			static_cast<double>(right.fCenterX) - leftX,
			static_cast<double>(right.fCenterZ) - leftZ
		};
		return WithinInclusiveRadius(
			LengthSquared(delta), leftRadius + right.fRadius);
	}
}

bool LostArk::Shared::CombatCollision::Is_Valid(
	const BODY_CIRCLE_XZ& circle) noexcept
{
	return IsValidCircle(circle);
}

bool LostArk::Shared::CombatCollision::Is_Valid(
	const CIRCLE_XZ& circle) noexcept
{
	return IsValidCircle(circle);
}

bool LostArk::Shared::CombatCollision::Circles_Overlap(
	const CIRCLE_XZ& circle,
	const BODY_CIRCLE_XZ& target) noexcept
{
	return Is_Valid(circle) && Is_Valid(target) && BodiesOverlap(
		circle.fCenterX, circle.fCenterZ, circle.fRadius, target);
}

bool LostArk::Shared::CombatCollision::Circles_Overlap(
	const BODY_CIRCLE_XZ& left,
	const BODY_CIRCLE_XZ& right) noexcept
{
	return Is_Valid(left) && Is_Valid(right) && BodiesOverlap(
		left.fCenterX, left.fCenterZ, left.fRadius, right);
}

bool LostArk::Shared::CombatCollision::Circle_IntersectsRing(
	const BODY_CIRCLE_XZ& target,
	const float centerX,
	const float centerZ,
	const float innerRadius,
	const float outerRadius) noexcept
{
	if (!Is_Valid(target) || !IsFinite(centerX) || !IsFinite(centerZ) ||
		!IsFinite(innerRadius) || !IsPositiveFinite(outerRadius) ||
		innerRadius < 0.f || innerRadius > outerRadius)
	{
		return false;
	}
	const VECTOR2 delta{
		static_cast<double>(target.fCenterX) - centerX,
		static_cast<double>(target.fCenterZ) - centerZ
	};
	const double distance = std::sqrt(LengthSquared(delta));
	const double epsilon = CONTACT_EPSILON;
	const double targetRadius = target.fRadius;
	const double inner = innerRadius;
	const double outer = outerRadius;
	return distance <= outer + targetRadius + epsilon &&
		distance + targetRadius + epsilon >= inner;
}

bool LostArk::Shared::CombatCollision::Circle_IntersectsForwardBox(
	const BODY_CIRCLE_XZ& target,
	const float originX,
	const float originZ,
	const float forwardX,
	const float forwardZ,
	const float length,
	const float halfWidth) noexcept
{
	VECTOR2 forward{};
	VECTOR2 right{};
	if (!Is_Valid(target) || !IsFinite(originX) || !IsFinite(originZ) ||
		!IsPositiveFinite(length) || !IsPositiveFinite(halfWidth) ||
		!NormalizeDirection(forwardX, forwardZ, forward, right))
	{
		return false;
	}
	const VECTOR2 delta{
		static_cast<double>(target.fCenterX) - originX,
		static_cast<double>(target.fCenterZ) - originZ
	};
	const double localForward = delta.x * forward.x + delta.z * forward.z;
	const double localRight = delta.x * right.x + delta.z * right.z;
	return WithinInclusiveRadius(
		DistanceSquaredToLocalRectangle(
			localForward, localRight, 0.0, length, -halfWidth, halfWidth),
		target.fRadius);
}

bool LostArk::Shared::CombatCollision::Circle_IntersectsCross(
	const BODY_CIRCLE_XZ& target,
	const float centerX,
	const float centerZ,
	const float forwardX,
	const float forwardZ,
	const float halfLength,
	const float halfWidth) noexcept
{
	VECTOR2 forward{};
	VECTOR2 right{};
	if (!Is_Valid(target) || !IsFinite(centerX) || !IsFinite(centerZ) ||
		!IsPositiveFinite(halfLength) || !IsPositiveFinite(halfWidth) ||
		!NormalizeDirection(forwardX, forwardZ, forward, right))
	{
		return false;
	}
	const VECTOR2 delta{
		static_cast<double>(target.fCenterX) - centerX,
		static_cast<double>(target.fCenterZ) - centerZ
	};
	const double localForward = delta.x * forward.x + delta.z * forward.z;
	const double localRight = delta.x * right.x + delta.z * right.z;
	const double longitudinalDistanceSquared = DistanceSquaredToLocalRectangle(
		localForward,
		localRight,
		-halfLength,
		halfLength,
		-halfWidth,
		halfWidth);
	const double lateralDistanceSquared = DistanceSquaredToLocalRectangle(
		localForward,
		localRight,
		-halfWidth,
		halfWidth,
		-halfLength,
		halfLength);
	return WithinInclusiveRadius(
		(std::min)(longitudinalDistanceSquared, lateralDistanceSquared),
		target.fRadius);
}

bool LostArk::Shared::CombatCollision::Circle_IntersectsSixDirections(
	const BODY_CIRCLE_XZ& target,
	const float centerX,
	const float centerZ,
	const float forwardX,
	const float forwardZ,
	const float halfLength,
	const float halfWidth) noexcept
{
	VECTOR2 forward{};
	VECTOR2 right{};
	if (!Is_Valid(target) || !IsFinite(centerX) || !IsFinite(centerZ) ||
		!IsPositiveFinite(halfLength) || !IsPositiveFinite(halfWidth) ||
		!NormalizeDirection(forwardX, forwardZ, forward, right))
	{
		return false;
	}
	const VECTOR2 delta{
		static_cast<double>(target.fCenterX) - centerX,
		static_cast<double>(target.fCenterZ) - centerZ
	};
	const double localForward = delta.x * forward.x + delta.z * forward.z;
	const double localRight = delta.x * right.x + delta.z * right.z;
	constexpr double COSINE_60 = 0.5;
	constexpr double SINE_60 = 0.86602540378443864676;
	constexpr std::array<double, 3u> STRIP_COSINES{
		1.0, COSINE_60, COSINE_60
	};
	constexpr std::array<double, 3u> STRIP_SINES{
		0.0, SINE_60, -SINE_60
	};
	double minimumDistanceSquared =
		(std::numeric_limits<double>::max)();
	for (std::size_t strip = 0u; strip < STRIP_COSINES.size(); ++strip)
	{
		const double along = localForward * STRIP_COSINES[strip] +
			localRight * STRIP_SINES[strip];
		const double across = -localForward * STRIP_SINES[strip] +
			localRight * STRIP_COSINES[strip];
		minimumDistanceSquared = (std::min)(
			minimumDistanceSquared,
			DistanceSquaredToLocalRectangle(
				along, across,
				-halfLength, halfLength, -halfWidth, halfWidth));
	}
	return WithinInclusiveRadius(minimumDistanceSquared, target.fRadius);
}

bool LostArk::Shared::CombatCollision::Segment_IntersectsCircle(
	const float startX,
	const float startZ,
	const float endX,
	const float endZ,
	const CIRCLE_XZ& circle) noexcept
{
	if (!IsValidCircle(circle) || !IsFinite(startX) || !IsFinite(startZ) ||
		!IsFinite(endX) || !IsFinite(endZ))
	{
		return false;
	}
	return WithinInclusiveRadius(
		DistanceSquaredToSegment(
			VECTOR2{ circle.fCenterX, circle.fCenterZ },
			VECTOR2{ startX, startZ },
			VECTOR2{ endX, endZ }),
		circle.fRadius);
}

bool LostArk::Shared::CombatCollision::Circle_IntersectsCone(
	const BODY_CIRCLE_XZ& target,
	const float originX,
	const float originZ,
	const float forwardX,
	const float forwardZ,
	const float length,
	const float angleDegrees) noexcept
{
	VECTOR2 forward{};
	VECTOR2 right{};
	if (!Is_Valid(target) || !IsFinite(originX) || !IsFinite(originZ) ||
		!IsPositiveFinite(length) || !IsPositiveFinite(angleDegrees) ||
		angleDegrees > 360.f ||
		!NormalizeDirection(forwardX, forwardZ, forward, right))
	{
		return false;
	}
	const VECTOR2 origin{ originX, originZ };
	const VECTOR2 center{ target.fCenterX, target.fCenterZ };
	const VECTOR2 delta{ center.x - origin.x, center.z - origin.z };
	const double distanceSquared = LengthSquared(delta);
	const double distance = std::sqrt(distanceSquared);
	if (angleDegrees == 360.f)
	{
		return WithinInclusiveRadius(distanceSquared, length + target.fRadius);
	}

	const double halfAngle = angleDegrees * 0.5 * DEGREES_TO_RADIANS;
	const double cosine = std::cos(halfAngle);
	const double sine = std::sin(halfAngle);
	const double localForward = delta.x * forward.x + delta.z * forward.z;
	const bool directionInside = distance <= CONTACT_EPSILON ||
		localForward / distance >= cosine;
	if (directionInside && distance <= length + CONTACT_EPSILON)
		return true;

	const VECTOR2 positiveBoundary{
		forward.x * cosine + right.x * sine,
		forward.z * cosine + right.z * sine
	};
	const VECTOR2 negativeBoundary{
		forward.x * cosine - right.x * sine,
		forward.z * cosine - right.z * sine
	};
	const VECTOR2 positiveEnd{
		origin.x + positiveBoundary.x * length,
		origin.z + positiveBoundary.z * length
	};
	const VECTOR2 negativeEnd{
		origin.x + negativeBoundary.x * length,
		origin.z + negativeBoundary.z * length
	};
	double minimumDistanceSquared = (std::min)(
		DistanceSquaredToSegment(center, origin, positiveEnd),
		DistanceSquaredToSegment(center, origin, negativeEnd));
	if (directionInside)
	{
		const double radialDistance = std::abs(distance - length);
		minimumDistanceSquared = (std::min)(
			minimumDistanceSquared, radialDistance * radialDistance);
	}
	return WithinInclusiveRadius(minimumDistanceSquared, target.fRadius);
}

bool LostArk::Shared::CombatCollision::Circle_IntersectsEllipticSector(
	const BODY_CIRCLE_XZ& target,
	const float originX, const float originZ,
	const float forwardX, const float forwardZ,
	const float radiusX, const float radiusZ,
	const float angleDegrees, const bool reverse) noexcept
{
	VECTOR2 forward{}, right{};
	if (!Is_Valid(target) || !IsFinite(originX) || !IsFinite(originZ) ||
		!IsPositiveFinite(radiusX) || !IsPositiveFinite(radiusZ) ||
		!IsFinite(angleDegrees) || angleDegrees < 0.f || angleDegrees > 360.f ||
		!NormalizeDirection(forwardX, forwardZ, forward, right))
		return false;
	const double angle = reverse ? 360.0 - angleDegrees : angleDegrees;
	if (angle == 0.0)
		return false;
	const VECTOR2 delta{
		static_cast<double>(target.fCenterX) - originX,
		static_cast<double>(target.fCenterZ) - originZ };
	if (!WithinInclusiveRadius(LengthSquared(delta),
		(std::max)(radiusX, radiusZ) + static_cast<double>(target.fRadius)))
		return false;
	const double directionSign = reverse ? -1.0 : 1.0;
	const VECTOR2 point{
		(delta.x * right.x + delta.z * right.z) * directionSign,
		(delta.x * forward.x + delta.z * forward.z) * directionSign };
	const double halfAngle = angle * 0.5 * DEGREES_TO_RADIANS;
	const VECTOR2 normalized{ point.x / radiusX, point.z / radiusZ };
	if (LengthSquared(normalized) <= 1.0 &&
		std::abs(std::atan2(normalized.x, normalized.z)) <= halfAngle)
		return true;

	double minimum = std::numeric_limits<double>::infinity();
	if (angle < 360.0)
	{
		const double x = radiusX * std::sin(halfAngle);
		const double z = radiusZ * std::cos(halfAngle);
		minimum = (std::min)(
			DistanceSquaredToSegment(point, {}, { x, z }),
			DistanceSquaredToSegment(point, {}, { -x, z }));
	}
	// The body remains a circle in metres. Scaling its radius in normalized
	// ellipse space would produce false hits along the narrow axis.
	for (const double xSign : { -1.0, 1.0 })
	{
		minimum = (std::min)(minimum, DistanceSquaredToEllipseArc(
			{ point.x * xSign, point.z }, radiusX, radiusZ,
			0.0, (std::min)(halfAngle, PI * 0.5)));
		if (halfAngle > PI * 0.5)
			minimum = (std::min)(minimum, DistanceSquaredToEllipseArc(
				{ point.x * xSign, -point.z }, radiusX, radiusZ,
				PI - halfAngle, PI * 0.5));
	}
	return WithinInclusiveRadius(minimum, target.fRadius);
}
