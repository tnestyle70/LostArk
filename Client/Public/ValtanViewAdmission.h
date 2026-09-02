#pragma once

#include "Engine_Defines.h"

#include <cstdint>

NS_BEGIN(Client)

/* One state model for every Tool view built from the joined Valtan Product.
   Display and mutation are deliberately different capabilities: a pinned
   last-good generation remains inspectable after reload failure, while every
   save/play/restart path requires a freshly ADMITTED generation. */
enum class VALTAN_VIEW_ADMISSION : uint8_t
{
	UNLOADED,
	ADMITTED,
	STALE_PRESERVED,
	REJECTED,
	END
};

inline constexpr bool_t Can_DisplayValtanView(
	const VALTAN_VIEW_ADMISSION eAdmission) noexcept
{
	return VALTAN_VIEW_ADMISSION::ADMITTED == eAdmission ||
		VALTAN_VIEW_ADMISSION::STALE_PRESERVED == eAdmission;
}

inline constexpr bool_t Can_MutateValtanView(
	const VALTAN_VIEW_ADMISSION eAdmission) noexcept
{
	return VALTAN_VIEW_ADMISSION::ADMITTED == eAdmission;
}

NS_END
