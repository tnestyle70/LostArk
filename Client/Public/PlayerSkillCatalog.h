#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"
#include "Network/PacketMessages.h"

#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>

namespace Client
{
	struct PLAYER_SKILL_TARGET_PREVIEW
	{
		std::string strAssetId;
		float fDiameter = 0.f;
		float4_t vValidTint = { 1.f, 1.f, 1.f, 1.f };
		float4_t vInvalidTint = { 1.f, 0.f, 0.f, 1.f };
		/* Kept visible to diagnostics so runtime-resource identity is never
		 reported as source-extracted evidence. Both usages are PROJECT_TUNED. */
		std::string strAssetIdentityBasis;
		std::string strUsageBasis;
		std::string strSourceEvidence;
	};
	/* One row of Data/Balance/PlayerSkills.json with its damage rate already
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
		/* Optional admitted presentation asset. Empty means this skill has no
		Effect fallback; animation cues may still provide timed Effects. */
		std::string strEffectId;
		std::uint32_t iCooldownMs = 0;
		/* Percent of the caster's attack power, as authored: display damage is
		attackPower x rate / 100 and only the server resolves the real number. */
		std::uint32_t iDamageRatePercent = 0;
		/* Read-only combat traits. The Client may present them, but only the
		Server uses them to resolve boss stagger, part damage, or counters. */
		std::uint32_t iStaggerDamage = 0;
		std::uint32_t iPartDamage = 0;
		std::uint32_t iCounterPower = 0;
		/* Official CostMp so the HUD can show a skill the server would refuse
		to pay for. The server keeps its own copy from the bootstrap. */
		std::uint32_t iResourceCost = 0;
		float fMaximumRange = 0.f;
		LostArk::Shared::SKILL_TARGET_INTENT_KIND eTargetIntent =
			LostArk::Shared::SKILL_TARGET_INTENT_KIND::AIM_POINT;
		float fTargetMaximumRange = 0.f;
		bool requiresWalkableTarget = false;
		PLAYER_SKILL_TARGET_PREVIEW RangePreview;
		PLAYER_SKILL_TARGET_PREVIEW TargetPreview;
		LostArk::Shared::PLAYER_SKILL_KIND eSkillKind =
			LostArk::Shared::PLAYER_SKILL_KIND::ACTIVE;
		LostArk::Shared::PLAYER_STANCE_ID eRequiredStance =
			LostArk::Shared::PLAYER_STANCE_ID::NONE;
		LostArk::Shared::PLAYER_STANCE_ID eSetsStance =
			LostArk::Shared::PLAYER_STANCE_ID::NONE;
		/* Presentation authoring maps one COMBO clip to each Server-owned stage.
		ACTIVE skills therefore carry zero and COMBO skills carry 2..8. */
		std::size_t iComboStageCount = 0;
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
		document, so a caller matches on "Q" or "ALT_V" rather than a key code.
		While the replicated action is KNOCKDOWN the slot resolves to its STANDUP
		skill instead of the normal one, mirroring the server's admission rule. */
		static const PLAYER_SKILL_DEFINITION* Find_BySlot(
			LostArk::Shared::CHARACTER_CLASS_ID characterClass,
			const std::string& inputSlot,
			LostArk::Shared::PLAYER_STANCE_ID stance =
				LostArk::Shared::PLAYER_STANCE_ID::NONE,
			bool isKnockedDown = false);

		static const PLAYER_SKILL_DEFINITION* Find_ById(
			LostArk::Shared::SKILL_ID skillId);
	};
}
