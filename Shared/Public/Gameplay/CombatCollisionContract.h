#pragma once

namespace LostArk::Shared::CombatCollision
{
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

	[[nodiscard]] bool Is_Valid(const BODY_CIRCLE_XZ& circle) noexcept;
	[[nodiscard]] bool Is_Valid(const CIRCLE_XZ& circle) noexcept;

	[[nodiscard]] bool Circles_Overlap(
		const CIRCLE_XZ& circle,
		const BODY_CIRCLE_XZ& target) noexcept;
	[[nodiscard]] bool Circles_Overlap(
		const BODY_CIRCLE_XZ& left,
		const BODY_CIRCLE_XZ& right) noexcept;

	[[nodiscard]] bool Circle_IntersectsRing(
		const BODY_CIRCLE_XZ& target,
		float centerX,
		float centerZ,
		float innerRadius,
		float outerRadius) noexcept;

	[[nodiscard]] bool Circle_IntersectsForwardBox(
		const BODY_CIRCLE_XZ& target,
		float originX,
		float originZ,
		float forwardX,
		float forwardZ,
		float length,
		float halfWidth) noexcept;

	[[nodiscard]] bool Circle_IntersectsCross(
		const BODY_CIRCLE_XZ& target,
		float centerX,
		float centerZ,
		float forwardX,
		float forwardZ,
		float halfLength,
		float halfWidth) noexcept;

	// Three centered strips at 0, 60 and 120 degrees form six radial arms.
	[[nodiscard]] bool Circle_IntersectsSixDirections(
		const BODY_CIRCLE_XZ& target,
		float centerX,
		float centerZ,
		float forwardX,
		float forwardZ,
		float halfLength,
		float halfWidth) noexcept;

	// angleDegrees is the cone's full angle, not its half angle.
	[[nodiscard]] bool Circle_IntersectsCone(
		const BODY_CIRCLE_XZ& target,
		float originX,
		float originZ,
		float forwardX,
		float forwardZ,
		float length,
		float angleDegrees) noexcept;
}
