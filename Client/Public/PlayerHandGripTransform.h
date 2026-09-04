#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <cmath>

NS_BEGIN(Client)

/* Authored CAPTURE hit value in metres. No Client runtime composes it onto a
   hand bone: the grabbed player's world transform is the Server attachment
   anchor replicated in PLAYER_SNAPSHOT and written by
   CCharacter::Update_NetworkTransform. The struct remains the typed shape that
   Balance Tool, Composition Detail, the encounter reference parser and the
   gameplay publisher validate. */
struct PLAYER_HAND_GRIP_LOCAL_OFFSET final
{
	f32_t fForwardM = 0.f;
	f32_t fUpM = 0.f;
	f32_t fRightM = 0.f;

	bool operator==(const PLAYER_HAND_GRIP_LOCAL_OFFSET&) const = default;
};

class CPlayerHandGripTransform final
{
public:
	static constexpr f32_t MAX_GRIP_OFFSET_COMPONENT_M = 10.f;

	static bool_t Is_ValidGripLocalOffset(
		const PLAYER_HAND_GRIP_LOCAL_OFFSET& gripLocalOffset)
	{
		return Is_ValidGripComponent(gripLocalOffset.fForwardM) &&
			Is_ValidGripComponent(gripLocalOffset.fUpM) &&
			Is_ValidGripComponent(gripLocalOffset.fRightM);
	}

private:
	static bool_t Is_ValidGripComponent(const f32_t value)
	{
		return std::isfinite(value) &&
			std::abs(value) <= MAX_GRIP_OFFSET_COMPONENT_M;
	}
};

NS_END
