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
	struct ANIMATION_SKILL_CLIP
	{
		std::string strClipName;
		uint32_t iPlayMs = 0u;
		f32_t fPlayRate = 1.f;

		bool operator==(const ANIMATION_SKILL_CLIP&) const = default;
	};

	/* One Server combo stage's clips. A flat "clips" array parses into exactly one
	stage, a nested one into a stage per element, so the JSON shape alone decides
	the stage count and no reader has to infer it from the skill kind. */
	struct ANIMATION_SKILL_STAGE
	{
		std::vector<ANIMATION_SKILL_CLIP> Clips;

		bool operator==(const ANIMATION_SKILL_STAGE&) const = default;
	};

	struct ANIMATION_SKILL_BINDING
	{
		LostArk::Shared::SKILL_ID iSkillId =
			LostArk::Shared::INVALID_SKILL_ID;
		std::vector<ANIMATION_SKILL_STAGE> Stages;
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

	/* Boss actions are not PlayerSkills. Their authored animation document maps a
	stable boss-owned action ID to one clip; gameplay pattern timing remains in
	the encounter document. */
	struct BOSS_PATTERN_ANIMATION_BINDING
	{
		std::string strActionId;
		std::string strClipName;

		bool operator==(const BOSS_PATTERN_ANIMATION_BINDING&) const = default;
	};

	struct BOSS_PATTERN_ANIMATION_BINDING_DOCUMENT
	{
		std::string strBossArchetypeId;
		std::vector<BOSS_PATTERN_ANIMATION_BINDING> Bindings;

		bool operator==(
			const BOSS_PATTERN_ANIMATION_BINDING_DOCUMENT&) const = default;
	};

	class CValtanPatternAnimationBindingDocument final
	{
	public:
		static std::filesystem::path Resolve_Path(
			std::string_view animationAssetId);
		static bool_t Parse_Text(
			std::string_view text,
			BOSS_PATTERN_ANIMATION_BINDING_DOCUMENT& outDocument,
			std::string& outStatus);
		static bool_t Validate(
			const BOSS_PATTERN_ANIMATION_BINDING_DOCUMENT& document,
			std::string_view expectedBossArchetypeId,
			const std::vector<std::string>& availableClips,
			std::string& outStatus);
		static bool_t Load(
			std::string_view animationAssetId,
			std::string_view expectedBossArchetypeId,
			const std::vector<std::string>& availableClips,
			BOSS_PATTERN_ANIMATION_BINDING_DOCUMENT& outDocument,
			std::string& outStatus);
	};

	/* Action-qualified boss Effect mapping.  It deliberately does not duplicate
	   encounter damage/timing authority: one row only joins a replicated boss
	   pattern action to an authored Effect document and its model clip/bone. */
	struct BOSS_PATTERN_EFFECT_BINDING
	{
		std::string strBindingId;
		std::string strPatternId;
		std::string strSemanticStageId;
		std::string strActionId;
		std::string strEffectAssetId;
		std::string strEffectDocument;
		std::string strRuntimeClipName;
		std::string strRuntimeBoneName;
		std::string strProductAdmissionStatus;
		bool_t bProductCatalogMapped = false;
		bool_t bAnimationEventMapped = false;

		bool operator==(const BOSS_PATTERN_EFFECT_BINDING&) const = default;
	};

	struct BOSS_PATTERN_EFFECT_BINDING_DOCUMENT
	{
		std::string strBossArchetypeId;
		std::vector<BOSS_PATTERN_EFFECT_BINDING> Bindings;

		bool operator==(
			const BOSS_PATTERN_EFFECT_BINDING_DOCUMENT&) const = default;
	};

	struct BOSS_PATTERN_EFFECT_TREE_ROW final
	{
		std::string strBossArchetypeId;
		std::string strBindingId;
		std::string strPatternId;
		std::string strSemanticStageId;
		std::string strActionId;
		std::string strEffectAssetId;
		std::string strRuntimeClipName;
		std::string strRuntimeBoneName;
		std::string strProductAdmissionStatus;
		std::filesystem::path Path;
		bool_t bProductCatalogMapped = false;
		bool_t bAnimationEventMapped = false;
	};

	struct BOSS_PATTERN_EFFECT_TREE_STAGE final
	{
		std::vector<BOSS_PATTERN_EFFECT_TREE_ROW> Rows;
	};

	class CValtanPatternEffectBindingDocument final
	{
	public:
		static std::filesystem::path Resolve_Path(
			std::string_view animationAssetId);
		static bool_t Parse_Text(
			std::string_view text,
			BOSS_PATTERN_EFFECT_BINDING_DOCUMENT& outDocument,
			std::string& outStatus);
		static bool_t Validate(
			const BOSS_PATTERN_EFFECT_BINDING_DOCUMENT& document,
			std::string_view expectedBossArchetypeId,
			const std::vector<std::string>& availableClips,
			std::string& outStatus);
		static bool_t Load(
			std::string_view animationAssetId,
			std::string_view expectedBossArchetypeId,
			const std::vector<std::string>& availableClips,
			BOSS_PATTERN_EFFECT_BINDING_DOCUMENT& outDocument,
			std::string& outStatus);
		static bool_t Stage_ValtanEffectToolTree(
			std::string_view text,
			const std::filesystem::path& projectDataRoot,
			BOSS_PATTERN_EFFECT_TREE_STAGE& outStage,
			std::string& outStatus);
	};
}
