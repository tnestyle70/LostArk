#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"
#include "PlayerSkillCatalog.h"

#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace Client
{
	/* Presentation-only mapping. PlayerSkills.json still owns key -> skillId and
	Server timing; this document owns only which authored body clips present an
	approved skill and which clip corresponds to each Server combo stage. */
	struct ANIMATION_SKILL_BINDING
	{
		LostArk::Shared::SKILL_ID iSkillId =
			LostArk::Shared::INVALID_SKILL_ID;
		std::vector<std::string> Clips;
	};

	struct ANIMATION_SKILL_BINDING_DOCUMENT
	{
		std::string strAnimationAssetId;
		LostArk::Shared::CHARACTER_CLASS_ID eCharacterClass =
			LostArk::Shared::CHARACTER_CLASS_ID::END;
		std::vector<ANIMATION_SKILL_BINDING> Bindings;
	};

	class CAnimationSkillBindingDocument final
	{
	public:
		static std::filesystem::path Resolve_Path(
			std::string_view animationAssetId);

		/* Parse does syntax/schema work only. Validate then joins the document to
		PlayerSkills and the current cooked model's stable clip names. Keeping the
		two phases separate lets harnesses prove malformed input and rollback. */
		static bool_t Parse_Text(
			std::string_view text,
			ANIMATION_SKILL_BINDING_DOCUMENT& outDocument,
			std::string& outStatus);
		static bool_t Validate(
			const ANIMATION_SKILL_BINDING_DOCUMENT& document,
			std::string_view expectedAnimationAssetId,
			LostArk::Shared::CHARACTER_CLASS_ID expectedCharacterClass,
			const std::vector<PLAYER_SKILL_DEFINITION>& skills,
			const std::vector<std::string>& availableClips,
			std::string& outStatus);
		static bool_t Load_FromPath(
			const std::filesystem::path& path,
			std::string_view expectedAnimationAssetId,
			LostArk::Shared::CHARACTER_CLASS_ID expectedCharacterClass,
			const std::vector<PLAYER_SKILL_DEFINITION>& skills,
			const std::vector<std::string>& availableClips,
			ANIMATION_SKILL_BINDING_DOCUMENT& outDocument,
			std::string& outStatus);
		static bool_t Load(
			std::string_view animationAssetId,
			LostArk::Shared::CHARACTER_CLASS_ID characterClass,
			const std::vector<PLAYER_SKILL_DEFINITION>& skills,
			const std::vector<std::string>& availableClips,
			ANIMATION_SKILL_BINDING_DOCUMENT& outDocument,
			std::string& outStatus);

		/* sibling temp -> durable flush -> strict reparse/validate -> atomic replace.
		The destination and in-memory caller document survive every failure. */
		static bool_t Save_Atomic(
			const ANIMATION_SKILL_BINDING_DOCUMENT& document,
			std::string_view expectedAnimationAssetId,
			LostArk::Shared::CHARACTER_CLASS_ID expectedCharacterClass,
			const std::vector<PLAYER_SKILL_DEFINITION>& skills,
			const std::vector<std::string>& availableClips,
			std::string& outStatus);
	};
}
