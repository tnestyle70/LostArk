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

	/* One XZ hit primitive independent of who spawned it. Player skill data and
	   boss combat-object data are converted into this shape at spawn time, so the
	   room-owned object loop never needs to know a skill/stage/catalog layout. */
	struct SERVER_COMBAT_SHAPE_XZ final
	{
		SERVER_COMBAT_SHAPE_KIND eKind = SERVER_COMBAT_SHAPE_KIND::END;
		float fOffset = 0.f;
		float fOuterRadius = 0.f;
		float fInnerRadius = 0.f;
		float fLength = 0.f;
		float fHalfWidth = 0.f;
		float fAngleDegrees = 0.f;
		std::uint32_t iMaximumTargets = 0u;
	};

	class CServerCombatGeometry final
	{
	public:
		[[nodiscard]] static bool Is_Valid(
			const SERVER_COMBAT_SHAPE_XZ& shape) noexcept;
		[[nodiscard]] static bool Overlaps_Pose(
			const SERVER_COMBAT_SHAPE_XZ& shape,
			float originX,
			float originZ,
			float forwardX,
			float forwardZ,
			const LostArk::Shared::CombatCollision::BODY_CIRCLE_XZ& target);
		/* A moving circular contact is a capsule in XZ. Expanding the target
		   circle by the contact radius and testing the travelled segment prevents
		   a fast wave from stepping over a player between fixed ticks. */
		[[nodiscard]] static bool SweptCircle_Overlaps(
			float startX,
			float startZ,
			float endX,
			float endZ,
			float contactRadius,
			const LostArk::Shared::CombatCollision::BODY_CIRCLE_XZ& target);
	};
}
