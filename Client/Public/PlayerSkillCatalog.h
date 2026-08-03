#pragma once

#include "Client_Defines.h"
#include "Network/PacketMessages.h"

#include <cstdint>
#include <string>
#include <vector>

namespace Client
{
	/* One row of Data/Balance/PlayerSkills.json with its damage amount already
	resolved through Data/Balance/DamageProfiles.json.

	Both the combat HUD and the input controller need this, so the parse lives
	here instead of inside either consumer. The definition says what a skill is;
	which quick slot a class has it on is the inputSlot field, and that pairing
	moves to a loadout document once the skill window exists. */
	struct PLAYER_SKILL_DEFINITION
	{
		LostArk::Shared::SKILL_ID iSkillId = LostArk::Shared::INVALID_SKILL_ID;
		LostArk::Shared::CHARACTER_CLASS_ID eCharacterClass =
			LostArk::Shared::CHARACTER_CLASS_ID::END;
		std::string strInputSlot;
		std::string strDisplayName;
		std::string strActionId;
		std::uint32_t iCooldownMs = 0;
		std::uint32_t iDamage = 0;
	};

	class CPlayerSkillCatalog final
	{
	public:
		/* Reads both balance documents. On failure the previously loaded set is
		kept and outStatus explains why, so a bad edit cannot empty a catalog that
		a running level is already reading. */
		static bool Load(std::string& outStatus);

		static const std::vector<PLAYER_SKILL_DEFINITION>& Get_Skills();

		/* The skill a class has on one quick slot, or nullptr when that class
		leaves the slot empty. Slot names are the inputSlot strings in the balance
		document, so a caller matches on "Q" or "ALT_V" rather than a key code. */
		static const PLAYER_SKILL_DEFINITION* Find_BySlot(
			LostArk::Shared::CHARACTER_CLASS_ID characterClass,
			const std::string& inputSlot);

		static const PLAYER_SKILL_DEFINITION* Find_ById(
			LostArk::Shared::SKILL_ID skillId);
	};
}
