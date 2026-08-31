#include "GameplayCatalog.h"

#include <Windows.h>
#include <bcrypt.h>

#include <algorithm>
#include <charconv>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <limits>
#include <sstream>
#include <string_view>
#include <unordered_set>
#include <vector>

#pragma comment(lib, "bcrypt.lib")

namespace
{
	/* Mirror of the publisher's $maximumDamageRatePercent: a rate only one side
	accepts would make Validate and Load disagree about the same document. */
	constexpr std::uint32_t MAXIMUM_DAMAGE_RATE_PERCENT = 100000u;
	/* The wire names one plate per bit, so a boss cannot wear more than the
	snapshot can carry. The publisher rejects a larger authored count. */
	constexpr std::size_t MAXIMUM_BOSS_ARMOR_PLATES =
		LostArk::Shared::MAX_WORLD_ENTITY_ARMOR_PLATES;

	bool Calculate_GameplayDataRevision(
		const std::string_view payload,
		LostArk::Shared::GameplayDataRevision& outRevision)
	{
		if (payload.size() > (std::numeric_limits<ULONG>::max)())
			return false;

		BCRYPT_ALG_HANDLE algorithm = nullptr;
		BCRYPT_HASH_HANDLE hash = nullptr;
		DWORD hashObjectSize = 0u;
		DWORD bytesWritten = 0u;
		DWORD hashSize = 0u;
		std::vector<unsigned char> hashObject;
		LostArk::Shared::GameplayDataRevision revision{};
		bool succeeded = false;
		if (0 <= BCryptOpenAlgorithmProvider(
				&algorithm, BCRYPT_SHA256_ALGORITHM, nullptr, 0u) &&
			0 <= BCryptGetProperty(
				algorithm, BCRYPT_OBJECT_LENGTH,
				reinterpret_cast<PUCHAR>(&hashObjectSize),
				sizeof(hashObjectSize), &bytesWritten, 0u) &&
			0 <= BCryptGetProperty(
				algorithm, BCRYPT_HASH_LENGTH,
				reinterpret_cast<PUCHAR>(&hashSize),
				sizeof(hashSize), &bytesWritten, 0u) &&
			hashSize == revision.Bytes.size())
		{
			hashObject.resize(hashObjectSize);
			if (0 <= BCryptCreateHash(
					algorithm, &hash, hashObject.data(), hashObjectSize,
					nullptr, 0u, 0u) &&
				0 <= BCryptHashData(
					hash,
					reinterpret_cast<PUCHAR>(
						const_cast<char*>(payload.data())),
					static_cast<ULONG>(payload.size()), 0u) &&
				0 <= BCryptFinishHash(
					hash, revision.Bytes.data(), hashSize, 0u) &&
				revision.Is_Valid())
			{
				succeeded = true;
			}
		}
		if (nullptr != hash)
			BCryptDestroyHash(hash);
		if (nullptr != algorithm)
			BCryptCloseAlgorithmProvider(algorithm, 0u);
		if (succeeded)
			outRevision = revision;
		return succeeded;
	}

	std::filesystem::path Resolve_DataRoot()
	{
		wchar_t configured[32768]{};
		const DWORD configuredLength = GetEnvironmentVariableW(
			L"LOSTARK_SERVER_DATA_ROOT", configured,
			static_cast<DWORD>(std::size(configured)));
		if (0u != configuredLength && configuredLength < std::size(configured))
			return std::filesystem::path(configured).lexically_normal();

		wchar_t modulePath[32768]{};
		const DWORD moduleLength = GetModuleFileNameW(
			nullptr, modulePath, static_cast<DWORD>(std::size(modulePath)));
		if (0u == moduleLength || moduleLength >= std::size(modulePath))
			return {};
		return std::filesystem::path(modulePath).parent_path().parent_path() /
			L"DataFiles";
	}

	std::vector<std::string_view> SplitTabs(const std::string& line)
	{
		std::vector<std::string_view> fields;
		const std::string_view view(line);
		std::size_t start = 0;
		while (true)
		{
			const std::size_t tab = view.find('\t', start);
			fields.push_back(view.substr(
				start, std::string_view::npos == tab ? tab : tab - start));
			if (std::string_view::npos == tab)
				break;
			start = tab + 1;
		}
		return fields;
	}

	void StripCarriageReturn(std::string& line)
	{
		if (!line.empty() && '\r' == line.back())
			line.pop_back();
	}

	template<typename T>
	bool ParseNumber(const std::string_view value, T& output)
	{
		const auto result = std::from_chars(
			value.data(), value.data() + value.size(), output);
		return std::errc{} == result.ec &&
			result.ptr == value.data() + value.size();
	}

	bool IsStableId(const std::string_view value)
	{
		return !value.empty() && value.size() <= 128u &&
			std::all_of(value.begin(), value.end(), [](const unsigned char character)
			{
				return 0 != std::isalnum(character) || character == '_' ||
					character == '-' || character == '.';
			});
	}

	bool ParseSkillKind(
		const std::string_view value,
		LostArk::Shared::PLAYER_SKILL_KIND& output)
	{
		using LostArk::Shared::PLAYER_SKILL_KIND;
		if ("ACTIVE" == value)
			output = PLAYER_SKILL_KIND::ACTIVE;
		else if ("COMBO" == value)
			output = PLAYER_SKILL_KIND::COMBO;
		else if ("HOLD" == value)
			output = PLAYER_SKILL_KIND::HOLD;
		else if ("COUNTER" == value)
			output = PLAYER_SKILL_KIND::COUNTER;
		else if ("STANDUP" == value)
			output = PLAYER_SKILL_KIND::STANDUP;
		else
			return false;
		return true;
	}

	bool ParseCharacterClass(
		const std::string_view value,
		LostArk::Shared::CHARACTER_CLASS_ID& output)
	{
		using LostArk::Shared::CHARACTER_CLASS_ID;
		if ("LANCE_MASTER" == value)
			output = CHARACTER_CLASS_ID::LANCE_MASTER;
		else if ("GUNSLINGER" == value)
			output = CHARACTER_CLASS_ID::GUNSLINGER;
		else if ("SLAYER" == value)
			output = CHARACTER_CLASS_ID::SLAYER;
		else if ("ARTIST" == value)
			output = CHARACTER_CLASS_ID::ARTIST;
		else if ("DIMENSIONMASTER" == value)
			output = CHARACTER_CLASS_ID::DIMENSIONMASTER;
		else if ("WARLORD" == value)
			output = CHARACTER_CLASS_ID::WARLORD;
		else
			return false;
		return true;
	}

	bool ParseStance(
		const std::string_view value,
		LostArk::Shared::PLAYER_STANCE_ID& output)
	{
		using LostArk::Shared::PLAYER_STANCE_ID;
		if ("NONE" == value)
			output = PLAYER_STANCE_ID::NONE;
		else if ("LANCE_MASTER_LONG_SPEAR" == value)
			output = PLAYER_STANCE_ID::LANCE_MASTER_LONG_SPEAR;
		else if ("LANCE_MASTER_SHORT_SPEAR" == value)
			output = PLAYER_STANCE_ID::LANCE_MASTER_SHORT_SPEAR;
		else if ("WARLORD_NORMAL" == value)
			output = PLAYER_STANCE_ID::WARLORD_NORMAL;
		else if ("WARLORD_DEFENSE" == value)
			output = PLAYER_STANCE_ID::WARLORD_DEFENSE;
		else
			return false;
		return true;
	}

	bool ParseBossPatternSelection(
		const std::string_view value,
		LostArk::Server::BOSS_PATTERN_SELECTION& output)
	{
		using LostArk::Server::BOSS_PATTERN_SELECTION;
		if ("NORMAL" == value)
			output = BOSS_PATTERN_SELECTION::NORMAL;
		else if ("HEALTH_BAR" == value)
			output = BOSS_PATTERN_SELECTION::HEALTH_BAR;
		else if ("AUDITION_ONLY" == value)
			output = BOSS_PATTERN_SELECTION::AUDITION_ONLY;
		else
			return false;
		return true;
	}

	bool ParseBossPatternRotationSelectionMode(
		const std::string_view value,
		LostArk::Server::BOSS_PATTERN_ROTATION_SELECTION_MODE& output)
	{
		using LostArk::Server::BOSS_PATTERN_ROTATION_SELECTION_MODE;
		if ("WEIGHTED_POOL" == value)
			output = BOSS_PATTERN_ROTATION_SELECTION_MODE::WEIGHTED_POOL;
		else if ("ORDERED_INTRO_THEN_WEIGHTED" == value)
			output = BOSS_PATTERN_ROTATION_SELECTION_MODE::
				ORDERED_INTRO_THEN_WEIGHTED;
		else
			return false;
		return true;
	}

	bool ParseBossPatternArmorRequirement(
		const std::string_view value,
		LostArk::Server::BOSS_PATTERN_ARMOR_REQUIREMENT& output)
	{
		using LostArk::Server::BOSS_PATTERN_ARMOR_REQUIREMENT;
		if ("ANY" == value)
			output = BOSS_PATTERN_ARMOR_REQUIREMENT::ANY;
		else if ("ARMORED" == value)
			output = BOSS_PATTERN_ARMOR_REQUIREMENT::ARMORED;
		else if ("STRIPPED" == value)
			output = BOSS_PATTERN_ARMOR_REQUIREMENT::STRIPPED;
		else
			return false;
		return true;
	}

	bool ParseBossPatternPhaseRequirement(
		const std::string_view value,
		LostArk::Server::BOSS_PATTERN_PHASE_REQUIREMENT& output)
	{
		using LostArk::Server::BOSS_PATTERN_PHASE_REQUIREMENT;
		if ("ANY" == value)
			output = BOSS_PATTERN_PHASE_REQUIREMENT::ANY;
		else if ("PHASE_ONE" == value)
			output = BOSS_PATTERN_PHASE_REQUIREMENT::PHASE_ONE;
		else if ("PHASE_TWO" == value)
			output = BOSS_PATTERN_PHASE_REQUIREMENT::PHASE_TWO;
		else
			return false;
		return true;
	}

	bool ParseBossPatternHitShape(
		const std::string_view value,
		LostArk::Server::BOSS_PATTERN_HIT_SHAPE& output)
	{
		using LostArk::Server::BOSS_PATTERN_HIT_SHAPE;
		if ("NONE" == value)
			output = BOSS_PATTERN_HIT_SHAPE::NONE;
		else if ("CIRCLE" == value)
			output = BOSS_PATTERN_HIT_SHAPE::CIRCLE;
		else if ("RING" == value)
			output = BOSS_PATTERN_HIT_SHAPE::RING;
		else if ("CONE" == value)
			output = BOSS_PATTERN_HIT_SHAPE::CONE;
		else if ("BOX" == value)
			output = BOSS_PATTERN_HIT_SHAPE::BOX;
		else if ("CROSS" == value)
			output = BOSS_PATTERN_HIT_SHAPE::CROSS;
		else if ("SIX_DIRECTIONS" == value)
			output = BOSS_PATTERN_HIT_SHAPE::SIX_DIRECTIONS;
		else
			return false;
		return true;
	}

	bool ParseBossPatternPlayerResponse(
		const std::string_view value,
		LostArk::Server::BOSS_PATTERN_PLAYER_RESPONSE& output)
	{
		using LostArk::Server::BOSS_PATTERN_PLAYER_RESPONSE;
		if ("DAMAGE" == value)
			output = BOSS_PATTERN_PLAYER_RESPONSE::DAMAGE;
		else if ("CAPTURE" == value)
			output = BOSS_PATTERN_PLAYER_RESPONSE::CAPTURE;
		else
			return false;
		return true;
	}

	bool ParseBossPatternHitAnchorKind(
		const std::string_view value,
		LostArk::Server::BOSS_PATTERN_HIT_ANCHOR_KIND& output)
	{
		using LostArk::Server::BOSS_PATTERN_HIT_ANCHOR_KIND;
		if ("BOSS_CURRENT" == value)
			output = BOSS_PATTERN_HIT_ANCHOR_KIND::BOSS_CURRENT;
		else if ("STAGE_ORIGIN" == value)
			output = BOSS_PATTERN_HIT_ANCHOR_KIND::STAGE_ORIGIN;
		else
			return false;
		return true;
	}

	bool ParseBossPatternHitActivationKind(
		const std::string_view value,
		LostArk::Server::BOSS_PATTERN_HIT_ACTIVATION_KIND& output)
	{
		using LostArk::Server::BOSS_PATTERN_HIT_ACTIVATION_KIND;
		if ("PULSE_SCHEDULE" == value)
			output = BOSS_PATTERN_HIT_ACTIVATION_KIND::PULSE_SCHEDULE;
		else if ("ACTIVE_WINDOW" == value)
			output = BOSS_PATTERN_HIT_ACTIVATION_KIND::ACTIVE_WINDOW;
		else
			return false;
		return true;
	}

	bool ParsePlayerAttachmentSlot(
		const std::string_view value,
		LostArk::Shared::PLAYER_ATTACHMENT_SLOT& output)
	{
		using LostArk::Shared::PLAYER_ATTACHMENT_SLOT;
		if ("NONE" == value)
			output = PLAYER_ATTACHMENT_SLOT::NONE;
		else if ("BOSS_LEFT_HAND" == value)
			output = PLAYER_ATTACHMENT_SLOT::BOSS_LEFT_HAND;
		else
			return false;
		return true;
	}

	bool ParseBossPatternStageKind(
		const std::string_view value,
		LostArk::Server::BOSS_PATTERN_STAGE_KIND& output)
	{
		using LostArk::Server::BOSS_PATTERN_STAGE_KIND;
		if ("WINDUP" == value)
			output = BOSS_PATTERN_STAGE_KIND::WINDUP;
		else if ("ACTIVE" == value)
			output = BOSS_PATTERN_STAGE_KIND::ACTIVE;
		else if ("RECOVERY" == value)
			output = BOSS_PATTERN_STAGE_KIND::RECOVERY;
		else if ("GROGGY" == value)
			output = BOSS_PATTERN_STAGE_KIND::GROGGY;
		else if ("PART_BREAK" == value)
			output = BOSS_PATTERN_STAGE_KIND::PART_BREAK;
		else
			return false;
		return true;
	}

	bool ParseBossCombatObjectKind(
		const std::string_view value,
		LostArk::Server::BOSS_COMBAT_OBJECT_KIND& output)
	{
		using LostArk::Server::BOSS_COMBAT_OBJECT_KIND;
		if ("FIXED_AREA" == value)
			output = BOSS_COMBAT_OBJECT_KIND::FIXED_AREA;
		else if ("MISSILE" == value)
			output = BOSS_COMBAT_OBJECT_KIND::MISSILE;
		else
			return false;
		return true;
	}

	bool ParseBossCombatObjectOriginPolicy(
		const std::string_view value,
		LostArk::Server::BOSS_COMBAT_OBJECT_ORIGIN_POLICY& output)
	{
		using LostArk::Server::BOSS_COMBAT_OBJECT_ORIGIN_POLICY;
		if ("BOSS_POSITION" == value)
			output = BOSS_COMBAT_OBJECT_ORIGIN_POLICY::BOSS_POSITION;
		else if ("LOCKED_TARGET_UNTIL_FIRST_PULSE" == value)
			output = BOSS_COMBAT_OBJECT_ORIGIN_POLICY::
				LOCKED_TARGET_UNTIL_FIRST_PULSE;
		else if ("LOCKED_TARGET_PER_ALIVE_PLAYER" == value)
			output = BOSS_COMBAT_OBJECT_ORIGIN_POLICY::
				LOCKED_TARGET_PER_ALIVE_PLAYER;
		else
			return false;
		return true;
	}

	bool ParseBossCombatObjectDirectionPolicy(
		const std::string_view value,
		LostArk::Server::BOSS_COMBAT_OBJECT_DIRECTION_POLICY& output)
	{
		using LostArk::Server::BOSS_COMBAT_OBJECT_DIRECTION_POLICY;
		if ("NONE" == value)
			output = BOSS_COMBAT_OBJECT_DIRECTION_POLICY::NONE;
		else if ("PATTERN_FACING_AT_SPAWN" == value)
			output = BOSS_COMBAT_OBJECT_DIRECTION_POLICY::
				PATTERN_FACING_AT_SPAWN;
		else
			return false;
		return true;
	}

	bool ParseBossCombatObjectHitTrigger(
		const std::string_view value,
		LostArk::Server::BOSS_COMBAT_OBJECT_HIT_TRIGGER& output)
	{
		using LostArk::Server::BOSS_COMBAT_OBJECT_HIT_TRIGGER;
		if ("CONTACT" == value)
			output = BOSS_COMBAT_OBJECT_HIT_TRIGGER::CONTACT;
		else if ("TIMED" == value)
			output = BOSS_COMBAT_OBJECT_HIT_TRIGGER::TIMED;
		else
			return false;
		return true;
	}

	bool ParseBossPatternCategory(
		const std::string_view value,
		LostArk::Server::BOSS_PATTERN_CATEGORY& output)
	{
		using LostArk::Server::BOSS_PATTERN_CATEGORY;
		if ("NORMAL" == value)
			output = BOSS_PATTERN_CATEGORY::NORMAL;
		else if ("IMPORTANT" == value)
			output = BOSS_PATTERN_CATEGORY::IMPORTANT;
		else if ("MECHANIC" == value)
			output = BOSS_PATTERN_CATEGORY::MECHANIC;
		else
			return false;
		return true;
	}

	bool ParseBossPatternTargetPolicy(
		const std::string_view value,
		LostArk::Server::BOSS_PATTERN_TARGET_POLICY& output)
	{
		using LostArk::Server::BOSS_PATTERN_TARGET_POLICY;
		if ("NONE" == value)
			output = BOSS_PATTERN_TARGET_POLICY::NONE;
		else if ("NEAREST_EACH_TICK" == value)
			output = BOSS_PATTERN_TARGET_POLICY::NEAREST_EACH_TICK;
		else if ("LOCK_NEAREST_ON_START" == value)
			output = BOSS_PATTERN_TARGET_POLICY::LOCK_NEAREST_ON_START;
		else if ("LOCK_RANDOM_ALIVE_ON_START" == value)
			output = BOSS_PATTERN_TARGET_POLICY::LOCK_RANDOM_ALIVE_ON_START;
		else if ("LOCK_RANDOM_ALIVE_BEHIND_ON_START" == value)
			output =
				BOSS_PATTERN_TARGET_POLICY::LOCK_RANDOM_ALIVE_BEHIND_ON_START;
		else
			return false;
		return true;
	}

	bool ParseBossPatternPartDamagePolicy(
		const std::string_view value,
		LostArk::Server::BOSS_PATTERN_PART_DAMAGE_POLICY& output)
	{
		using LostArk::Server::BOSS_PATTERN_PART_DAMAGE_POLICY;
		if ("NORMAL" == value)
			output = BOSS_PATTERN_PART_DAMAGE_POLICY::NORMAL;
		else if ("DESTROY_FIRST_ELIGIBLE" == value)
			output =
				BOSS_PATTERN_PART_DAMAGE_POLICY::DESTROY_FIRST_ELIGIBLE;
		else
			return false;
		return true;
	}

	bool ParseBossPatternAimPolicy(
		const std::string_view value,
		LostArk::Server::BOSS_PATTERN_AIM_POLICY& output)
	{
		using LostArk::Server::BOSS_PATTERN_AIM_POLICY;
		if ("NONE" == value)
			output = BOSS_PATTERN_AIM_POLICY::NONE;
		else if ("TRACK_TARGET_EACH_TICK" == value)
			output = BOSS_PATTERN_AIM_POLICY::TRACK_TARGET_EACH_TICK;
		else if ("LOCK_FACING_ON_START" == value)
			output = BOSS_PATTERN_AIM_POLICY::LOCK_FACING_ON_START;
		else if ("FACE_MOTION_ANCHOR" == value)
			output = BOSS_PATTERN_AIM_POLICY::FACE_MOTION_ANCHOR;
		else
			return false;
		return true;
	}

	bool ParseBossPatternStageOutcome(
		const std::string_view value,
		LostArk::Server::BOSS_PATTERN_STAGE_OUTCOME& output)
	{
		using LostArk::Server::BOSS_PATTERN_STAGE_OUTCOME;
		if ("TIMEOUT" == value)
			output = BOSS_PATTERN_STAGE_OUTCOME::TIMEOUT;
		else if ("COUNTER_HIT" == value)
			output = BOSS_PATTERN_STAGE_OUTCOME::COUNTER_HIT;
		else if ("STAGGER_BROKEN" == value)
			output = BOSS_PATTERN_STAGE_OUTCOME::STAGGER_BROKEN;
		else if ("WALL_CONTACT" == value)
			output = BOSS_PATTERN_STAGE_OUTCOME::WALL_CONTACT;
		else if ("PART_DESTROYED" == value)
			output = BOSS_PATTERN_STAGE_OUTCOME::PART_DESTROYED;
		else if ("PROP_DESTROYED" == value)
			output = BOSS_PATTERN_STAGE_OUTCOME::PROP_DESTROYED;
		else if ("SUMMON_DEAD" == value)
			output = BOSS_PATTERN_STAGE_OUTCOME::SUMMON_DEAD;
		else if ("ALL_PLAYERS_GRABBED" == value)
			output = BOSS_PATTERN_STAGE_OUTCOME::ALL_PLAYERS_GRABBED;
		else if ("NAVIGATION_BLOCKED" == value)
			output = BOSS_PATTERN_STAGE_OUTCOME::NAVIGATION_BLOCKED;
		else if ("ANY_PLAYER_GRABBED" == value)
			output = BOSS_PATTERN_STAGE_OUTCOME::ANY_PLAYER_GRABBED;
		else
			return false;
		return true;
	}

	bool ParseBossPatternStageActionTrigger(
		const std::string_view value,
		LostArk::Server::BOSS_PATTERN_STAGE_ACTION_TRIGGER& output)
	{
		using LostArk::Server::BOSS_PATTERN_STAGE_ACTION_TRIGGER;
		if ("ENTER" == value)
			output = BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER;
		else if ("EXIT" == value)
			output = BOSS_PATTERN_STAGE_ACTION_TRIGGER::EXIT;
		else
			return false;
		return true;
	}

	bool ParseBossPatternStageActionKind(
		const std::string_view value,
		LostArk::Server::BOSS_PATTERN_STAGE_ACTION_KIND& output)
	{
		using LostArk::Server::BOSS_PATTERN_STAGE_ACTION_KIND;
		if ("SET_BOSS_FLAG" == value)
			output = BOSS_PATTERN_STAGE_ACTION_KIND::SET_BOSS_FLAG;
		else if ("SET_STAGGER_GAUGE" == value)
			output = BOSS_PATTERN_STAGE_ACTION_KIND::SET_STAGGER_GAUGE;
		else if ("SET_SHIELD" == value)
			output = BOSS_PATTERN_STAGE_ACTION_KIND::SET_SHIELD;
		else if ("SET_PLAYER_BIND" == value)
			output = BOSS_PATTERN_STAGE_ACTION_KIND::SET_PLAYER_BIND;
		else if ("SET_PLAYER_SILENCE" == value)
			output = BOSS_PATTERN_STAGE_ACTION_KIND::SET_PLAYER_SILENCE;
		else if ("SET_GAMEPLAY_PHASE" == value)
			output = BOSS_PATTERN_STAGE_ACTION_KIND::SET_GAMEPLAY_PHASE;
		else if ("SPAWN_COMBAT_OBJECT" == value)
			output = BOSS_PATTERN_STAGE_ACTION_KIND::SPAWN_COMBAT_OBJECT;
		else if ("RETARGET_RANDOM_ALIVE" == value)
			output = BOSS_PATTERN_STAGE_ACTION_KIND::RETARGET_RANDOM_ALIVE;
		else if ("RELEASE_GRABBED_PLAYERS" == value)
			output = BOSS_PATTERN_STAGE_ACTION_KIND::RELEASE_GRABBED_PLAYERS;
		else if ("DAMAGE_GRABBED_PLAYERS" == value)
			output = BOSS_PATTERN_STAGE_ACTION_KIND::DAMAGE_GRABBED_PLAYERS;
		else if ("RETURN_TO_ARENA_CENTER" == value)
			output = BOSS_PATTERN_STAGE_ACTION_KIND::RETURN_TO_ARENA_CENTER;
		else if ("EXECUTE_GRABBED_PLAYERS" == value)
			output = BOSS_PATTERN_STAGE_ACTION_KIND::EXECUTE_GRABBED_PLAYERS;
		else
			return false;
		return true;
	}

	bool ParseBossGrabbedReleaseMode(
		const std::string_view value,
		LostArk::Server::BOSS_GRABBED_RELEASE_MODE& output)
	{
		using LostArk::Server::BOSS_GRABBED_RELEASE_MODE;
		if ("NONE" == value)
			output = BOSS_GRABBED_RELEASE_MODE::NONE;
		else if ("HOLD" == value)
			output = BOSS_GRABBED_RELEASE_MODE::HOLD;
		else if ("ARENA_EJECTION" == value)
			output = BOSS_GRABBED_RELEASE_MODE::ARENA_EJECTION;
		else if ("OPPOSITE_KNOCKBACK" == value)
			output = BOSS_GRABBED_RELEASE_MODE::OPPOSITE_KNOCKBACK;
		else
			return false;
		return true;
	}

	bool IsValidBossPatternStageAction(
		const LostArk::Server::BOSS_PATTERN_STAGE_ACTION_TRIGGER trigger,
		const LostArk::Server::BOSS_PATTERN_STAGE_ACTION_KIND kind,
		const std::string_view targetId,
		const std::uint32_t value,
		const std::uint32_t durationMs,
		const LostArk::Server::BOSS_GRABBED_RELEASE_MODE releaseMode,
		const float releaseSpeedMps,
		const float releaseYawOffsetDegrees)
	{
		using LostArk::Server::BOSS_PATTERN_STAGE_ACTION_KIND;
		using LostArk::Server::BOSS_PATTERN_STAGE_ACTION_TRIGGER;
		using LostArk::Server::BOSS_GRABBED_RELEASE_MODE;
		if (!std::isfinite(releaseSpeedMps) || releaseSpeedMps < 0.f ||
			releaseSpeedMps > 50.f || !std::isfinite(releaseYawOffsetDegrees) ||
			std::abs(releaseYawOffsetDegrees) > 180.f)
			return false;
		if (BOSS_PATTERN_STAGE_ACTION_KIND::RELEASE_GRABBED_PLAYERS == kind)
		{
			if ("boss.attachment.left-hand" != targetId || 0u != value)
				return false;
			if (BOSS_GRABBED_RELEASE_MODE::HOLD == releaseMode)
				return 0.f == releaseSpeedMps && 0u == durationMs &&
					0.f == releaseYawOffsetDegrees;
			return (BOSS_GRABBED_RELEASE_MODE::OPPOSITE_KNOCKBACK == releaseMode ||
				BOSS_GRABBED_RELEASE_MODE::ARENA_EJECTION == releaseMode) &&
				releaseSpeedMps > 0.f && durationMs > 0u && durationMs <= 5000u &&
				(BOSS_GRABBED_RELEASE_MODE::ARENA_EJECTION == releaseMode ||
				 0.f == releaseYawOffsetDegrees);
		}
		if (BOSS_PATTERN_STAGE_ACTION_KIND::SET_PLAYER_BIND == kind)
		{
			return BOSS_GRABBED_RELEASE_MODE::NONE == releaseMode &&
				0.f == releaseSpeedMps && 0.f == releaseYawOffsetDegrees &&
				"player.status.bind" == targetId &&
				((BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER == trigger &&
				  10000u == value && durationMs >= 100u && durationMs <= 120000u) ||
				 (BOSS_PATTERN_STAGE_ACTION_TRIGGER::EXIT == trigger &&
				  0u == value && 0u == durationMs));
		}
		if (BOSS_PATTERN_STAGE_ACTION_KIND::SET_PLAYER_SILENCE == kind)
		{
			return BOSS_GRABBED_RELEASE_MODE::NONE == releaseMode &&
				0.f == releaseSpeedMps && 0.f == releaseYawOffsetDegrees &&
				"player.status.silence" == targetId &&
				((BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER == trigger &&
				  1u == value && durationMs >= 100u && durationMs <= 120000u) ||
				 (BOSS_PATTERN_STAGE_ACTION_TRIGGER::EXIT == trigger &&
				  0u == value && 0u == durationMs));
		}
		if (BOSS_GRABBED_RELEASE_MODE::NONE != releaseMode ||
			0.f != releaseSpeedMps || 0.f != releaseYawOffsetDegrees ||
			0u != durationMs)
		{
			return false;
		}
		if (BOSS_PATTERN_STAGE_ACTION_KIND::RETURN_TO_ARENA_CENTER == kind)
		{
			return BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER == trigger &&
				"boss.arena.center" == targetId && 1u == value;
		}
		if (BOSS_PATTERN_STAGE_ACTION_KIND::RETARGET_RANDOM_ALIVE == kind)
		{
			return BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER == trigger &&
				"boss.target.pattern" == targetId && 1u == value;
		}
		if (BOSS_PATTERN_STAGE_ACTION_KIND::DAMAGE_GRABBED_PLAYERS == kind ||
			BOSS_PATTERN_STAGE_ACTION_KIND::EXECUTE_GRABBED_PLAYERS == kind)
		{
			return BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER == trigger &&
				0u == value &&
				(BOSS_PATTERN_STAGE_ACTION_KIND::EXECUTE_GRABBED_PLAYERS == kind ?
					"boss.attachment.left-hand" == targetId :
					0u == targetId.find("damage."));
		}
		const bool isSet = BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER == trigger;
		if ((isSet && 0u == value) || (!isSet && 0u != value))
			return false;
		switch (kind)
		{
		case BOSS_PATTERN_STAGE_ACTION_KIND::SET_BOSS_FLAG:
			return value <= 1u &&
				("boss.flag.groggy" == targetId ||
				 "boss.flag.invulnerable" == targetId ||
				 "boss.flag.counterable" == targetId);
		case BOSS_PATTERN_STAGE_ACTION_KIND::SET_STAGGER_GAUGE:
			return "boss.gauge.stagger" == targetId;
		case BOSS_PATTERN_STAGE_ACTION_KIND::SET_SHIELD:
			return "boss.gauge.shield" == targetId;
		case BOSS_PATTERN_STAGE_ACTION_KIND::SET_PLAYER_BIND:
		case BOSS_PATTERN_STAGE_ACTION_KIND::SET_PLAYER_SILENCE:
			return false;
		case BOSS_PATTERN_STAGE_ACTION_KIND::SET_GAMEPLAY_PHASE:
			return isSet && "boss.phase.gameplay" == targetId &&
				2u == value;
		case BOSS_PATTERN_STAGE_ACTION_KIND::SPAWN_COMBAT_OBJECT:
			return isSet && 1u == value && IsStableId(targetId);
		case BOSS_PATTERN_STAGE_ACTION_KIND::SPAWN_COMBAT_OBJECT_VOLLEY:
			return false;
		case BOSS_PATTERN_STAGE_ACTION_KIND::RETURN_TO_ARENA_CENTER:
		case BOSS_PATTERN_STAGE_ACTION_KIND::RETARGET_RANDOM_ALIVE:
		case BOSS_PATTERN_STAGE_ACTION_KIND::RELEASE_GRABBED_PLAYERS:
		case BOSS_PATTERN_STAGE_ACTION_KIND::DAMAGE_GRABBED_PLAYERS:
		case BOSS_PATTERN_STAGE_ACTION_KIND::EXECUTE_GRABBED_PLAYERS:
			return false;
		}
		return false;
	}

	bool IsStatefulBossPatternStageAction(
		const LostArk::Server::BOSS_PATTERN_STAGE_ACTION_KIND kind)
	{
		return LostArk::Server::BOSS_PATTERN_STAGE_ACTION_KIND::
			SPAWN_COMBAT_OBJECT != kind &&
			LostArk::Server::BOSS_PATTERN_STAGE_ACTION_KIND::
				SPAWN_COMBAT_OBJECT_VOLLEY != kind &&
			LostArk::Server::BOSS_PATTERN_STAGE_ACTION_KIND::
				SET_GAMEPLAY_PHASE != kind &&
			LostArk::Server::BOSS_PATTERN_STAGE_ACTION_KIND::
				RETARGET_RANDOM_ALIVE != kind &&
			LostArk::Server::BOSS_PATTERN_STAGE_ACTION_KIND::
				RELEASE_GRABBED_PLAYERS != kind &&
			LostArk::Server::BOSS_PATTERN_STAGE_ACTION_KIND::
				DAMAGE_GRABBED_PLAYERS != kind &&
			LostArk::Server::BOSS_PATTERN_STAGE_ACTION_KIND::
				EXECUTE_GRABBED_PLAYERS != kind &&
			LostArk::Server::BOSS_PATTERN_STAGE_ACTION_KIND::
				RETURN_TO_ARENA_CENTER != kind;
	}

	bool HasFiniteBossStageGraph(const LostArk::Server::BOSS_PATTERN_DEFINITION& pattern)
	{
		std::vector<std::uint8_t> visited(pattern.Stages.size(), 0u);
		const auto visit = [&](auto&& self, const std::size_t index) -> bool
		{
			if (1u == visited[index]) return false;
			if (2u == visited[index]) return true;
			visited[index] = 1u;
			for (const auto& branch : pattern.Stages[index].Branches)
			{
				if (branch.strNextActionId.empty()) continue;
				const auto next = std::find_if(pattern.Stages.begin(), pattern.Stages.end(),
					[&branch](const auto& stage)
					{ return stage.strActionId == branch.strNextActionId; });
				if (next == pattern.Stages.end() ||
					!self(self, static_cast<std::size_t>(next - pattern.Stages.begin())))
					return false;
			}
			visited[index] = 2u;
			return true;
		};
		for (std::size_t index = 0u; index < pattern.Stages.size(); ++index)
			if (!visit(visit, index)) return false;
		return !pattern.Stages.empty();
	}

	bool ParseBossPhasePolicyKind(
		const std::string_view value,
		LostArk::Server::BOSS_PHASE_POLICY_KIND& output)
	{
		using LostArk::Server::BOSS_PHASE_POLICY_KIND;
		if ("HEALTH_PERCENT_THRESHOLD" == value)
			output = BOSS_PHASE_POLICY_KIND::HEALTH_PERCENT_THRESHOLD;
		else if ("AUTHORED_PATTERN_EVENT" == value)
			output = BOSS_PHASE_POLICY_KIND::AUTHORED_PATTERN_EVENT;
		else
			return false;
		return true;
	}

	std::string BuildBossPatternStageActionStateKey(
		const LostArk::Server::BOSS_PATTERN_STAGE_ACTION_KIND kind,
		const std::string_view targetId)
	{
		return std::to_string(static_cast<std::uint32_t>(kind)) + "\n" +
			std::string(targetId);
	}

	bool ParseBossPartDamageCondition(
		const std::string_view value,
		LostArk::Server::BOSS_PART_DAMAGE_CONDITION& output)
	{
		using LostArk::Server::BOSS_PART_DAMAGE_CONDITION;
		if ("ALWAYS" == value)
			output = BOSS_PART_DAMAGE_CONDITION::ALWAYS;
		else if ("GROGGY_ONLY" == value)
			output = BOSS_PART_DAMAGE_CONDITION::GROGGY_ONLY;
		else
			return false;
		return true;
	}

	bool ParseValtanTimelineArenaState(
		const std::string_view value,
		LostArk::Server::VALTAN_TIMELINE_ARENA_STATE& output)
	{
		using LostArk::Server::VALTAN_TIMELINE_ARENA_STATE;
		if ("FRESH" == value)
			output = VALTAN_TIMELINE_ARENA_STATE::FRESH;
		else if ("ORDINARY_WALLS_GONE" == value)
			output = VALTAN_TIMELINE_ARENA_STATE::ORDINARY_WALLS_GONE;
		else if ("ALL_WALLS_GONE" == value)
			output = VALTAN_TIMELINE_ARENA_STATE::ALL_WALLS_GONE;
		else if ("FLOOR84_GONE" == value)
			output = VALTAN_TIMELINE_ARENA_STATE::FLOOR84_GONE;
		else if ("FLOOR84_AND_30_GONE" == value)
			output = VALTAN_TIMELINE_ARENA_STATE::FLOOR84_AND_30_GONE;
		else
			return false;
		return true;
	}

	std::uint32_t Calculate_ValtanTimelineCommandId(
		const std::string_view rowId)
	{
		std::uint32_t hash = 2166136261u;
		for (const unsigned char character : rowId)
		{
			hash ^= character;
			hash *= 16777619u;
		}
		return hash;
	}

	bool ParseValtanTimelinePropState(
		const std::string_view value,
		LostArk::Server::VALTAN_TIMELINE_PROP_STATE& output)
	{
		using LostArk::Server::VALTAN_TIMELINE_PROP_STATE;
		if ("HIDDEN" == value)
			output = VALTAN_TIMELINE_PROP_STATE::HIDDEN;
		else if ("FOUR_PILLARS_INTACT" == value)
			output = VALTAN_TIMELINE_PROP_STATE::FOUR_PILLARS_INTACT;
		else
			return false;
		return true;
	}

	bool ParseValtanTimelineEntryType(
		const std::string_view value,
		LostArk::Server::VALTAN_TIMELINE_ENTRY_TYPE& output)
	{
		using LostArk::Server::VALTAN_TIMELINE_ENTRY_TYPE;
		if ("MECHANIC" == value)
			output = VALTAN_TIMELINE_ENTRY_TYPE::MECHANIC;
		else if ("NORMAL" == value)
			output = VALTAN_TIMELINE_ENTRY_TYPE::NORMAL;
		else
			return false;
		return true;
	}
}

bool LostArk::Server::CGameplayCatalog::Parse_RootMotionSamples(
	const std::string_view packed,
	const std::uint32_t sampleCount,
	const std::uint32_t limitMs,
	std::vector<ROOT_MOTION_SAMPLE>& outSamples)
{
	outSamples.clear();
	outSamples.reserve(sampleCount);
	std::size_t cursor = 0;
	while (cursor <= packed.size())
	{
		const std::size_t comma = packed.find(',', cursor);
		const std::string_view token{
			packed.data() + cursor,
			(std::string::npos == comma ? packed.size() : comma) - cursor };
		const std::size_t first = token.find(':');
		const std::size_t second = std::string_view::npos == first ?
			std::string_view::npos : token.find(':', first + 1);
		ROOT_MOTION_SAMPLE sample{};
		if (std::string_view::npos == second ||
			!ParseNumber(token.substr(0, first), sample.iTimeMs) ||
			!ParseNumber(token.substr(first + 1, second - first - 1),
				sample.fForward) ||
			!ParseNumber(token.substr(second + 1), sample.fLateral) ||
			!std::isfinite(sample.fForward) ||
			!std::isfinite(sample.fLateral) ||
			sample.iTimeMs > limitMs ||
			(!outSamples.empty() && sample.iTimeMs <= outSamples.back().iTimeMs))
		{
			m_strStatus = "Root motion sample is invalid";
			return false;
		}
		outSamples.push_back(sample);
		if (std::string::npos == comma)
			break;
		cursor = comma + 1;
	}
	if (outSamples.size() != sampleCount)
	{
		m_strStatus = "Root motion sample count does not match";
		return false;
	}
	return true;
}

bool LostArk::Server::CGameplayCatalog::Parse_SkillHits(
	const std::string_view packed,
	const std::uint32_t hitCount,
	const std::uint32_t limitMs,
	std::vector<PLAYER_SKILL_HIT>& outHits)
{
	outHits.clear();
	outHits.reserve(hitCount);
	std::uint32_t subHits = 0;
	std::size_t cursor = 0;
	while (cursor <= packed.size())
	{
		const std::size_t comma = packed.find(',', cursor);
		const std::string_view token{
			packed.data() + cursor,
			(std::string::npos == comma ? packed.size() : comma) - cursor };
		std::string_view fields[13];
		std::size_t fieldCount = 0;
		std::size_t start = 0;
		while (fieldCount < 13)
		{
			const std::size_t colon = token.find(':', start);
			fields[fieldCount++] = token.substr(start,
				std::string_view::npos == colon ? std::string_view::npos : colon - start);
			if (std::string_view::npos == colon)
				break;
			start = colon + 1;
		}
		PLAYER_SKILL_HIT hit{};
		if (13u != fieldCount ||
			!ParseNumber(fields[0], hit.iTimeMs) ||
			!ParseNumber(fields[1], hit.iRepeatCount) ||
			!ParseNumber(fields[2], hit.iRepeatMs) ||
			!Parse_HitShapeExtent(fields + 3, hit) ||
			hit.iTimeMs > limitMs ||
			0u == hit.iRepeatCount || hit.iRepeatCount > 64u ||
			(hit.iRepeatCount > 1u && 0u == hit.iRepeatMs) ||
			static_cast<std::uint64_t>(hit.iTimeMs) +
				static_cast<std::uint64_t>(hit.iRepeatCount - 1u) *
					hit.iRepeatMs > limitMs ||
			(!outHits.empty() && hit.iTimeMs < outHits.back().iTimeMs))
		{
			m_strStatus = "Skill hit shape is invalid";
			return false;
		}
		subHits += hit.iRepeatCount;
		outHits.push_back(hit);
		if (std::string::npos == comma)
			break;
		cursor = comma + 1;
	}
	if (outHits.size() != hitCount || subHits > 64u)
	{
		m_strStatus = "Skill hit shape count does not match";
		return false;
	}
	return true;
}

bool LostArk::Server::CGameplayCatalog::Parse_HitShapeExtent(
	const std::string_view* fields,
	PLAYER_SKILL_HIT& hit)
{
	return ParseNumber(fields[0], hit.iAreaType) &&
		ParseNumber(fields[1], hit.fRange) &&
		ParseNumber(fields[2], hit.fAngleDegrees) &&
		ParseNumber(fields[3], hit.fWidth) &&
		ParseNumber(fields[4], hit.fHeight) &&
		ParseNumber(fields[5], hit.fOffset) &&
		ParseNumber(fields[6], hit.fInner) &&
		ParseNumber(fields[7], hit.iMaxTargets) &&
		ParseNumber(fields[8], hit.iPushMs) &&
		ParseNumber(fields[9], hit.fPushRange) &&
		hit.iPushMs <= 10000u &&
		std::isfinite(hit.fPushRange) &&
		hit.fPushRange >= -50.f && hit.fPushRange <= 50.f &&
		(0u != hit.iPushMs || 0.f == hit.fPushRange) &&
		hit.iAreaType >= 1u && hit.iAreaType <= 3u &&
		std::isfinite(hit.fRange) && hit.fRange > 0.f &&
		std::isfinite(hit.fAngleDegrees) && hit.fAngleDegrees >= 0.f &&
		hit.fAngleDegrees <= 360.f &&
		(3u == hit.iAreaType || 0.f == hit.fAngleDegrees) &&
		std::isfinite(hit.fWidth) && hit.fWidth >= 0.f &&
		(2u == hit.iAreaType) == (hit.fWidth > 0.f) &&
		std::isfinite(hit.fHeight) && hit.fHeight >= 0.f &&
		std::isfinite(hit.fOffset) &&
		std::isfinite(hit.fInner) && hit.fInner >= 0.f &&
		hit.fInner < hit.fRange &&
		(2u != hit.iAreaType || 0.f == hit.fInner) &&
		hit.iMaxTargets <= 64u;
}

bool LostArk::Server::CGameplayCatalog::Parse_SkillProjectile(
	const std::vector<std::string_view>& fields,
	const std::size_t firstField,
	const std::uint32_t limitMs,
	std::vector<PLAYER_SKILL_PROJECTILE>& outProjectiles)
{
	/* index timeMs kind origin offsetForward offsetRight speed minDistance
	maxDistance lifeMs radius hitCount hits */
	std::uint32_t index = 0;
	std::uint32_t origin = 0;
	std::uint32_t hitCount = 0;
	PLAYER_SKILL_PROJECTILE projectile{};
	if (fields.size() != firstField + 13u ||
		!ParseNumber(fields[firstField], index) ||
		!ParseNumber(fields[firstField + 1u], projectile.iTimeMs) ||
		!ParseNumber(fields[firstField + 3u], origin) || origin > 1u ||
		!ParseNumber(fields[firstField + 4u], projectile.fOffsetForward) ||
		!ParseNumber(fields[firstField + 5u], projectile.fOffsetRight) ||
		!ParseNumber(fields[firstField + 6u], projectile.fSpeed) ||
		!ParseNumber(fields[firstField + 7u], projectile.fMinDistance) ||
		!ParseNumber(fields[firstField + 8u], projectile.fMaxDistance) ||
		!ParseNumber(fields[firstField + 9u], projectile.iLifeMs) ||
		!ParseNumber(fields[firstField + 10u], projectile.fRadius) ||
		!ParseNumber(fields[firstField + 11u], hitCount) ||
		!std::isfinite(projectile.fOffsetForward) ||
		projectile.fOffsetForward < -50.f || projectile.fOffsetForward > 50.f ||
		!std::isfinite(projectile.fOffsetRight) ||
		projectile.fOffsetRight < -50.f || projectile.fOffsetRight > 50.f ||
		index != outProjectiles.size() || index >= 8u ||
		projectile.iTimeMs > limitMs ||
		(!outProjectiles.empty() &&
			projectile.iTimeMs < outProjectiles.back().iTimeMs) ||
		0u == projectile.iLifeMs || projectile.iLifeMs > 600000u ||
		!std::isfinite(projectile.fSpeed) || projectile.fSpeed < 0.f ||
		projectile.fSpeed > 1000.f ||
		!std::isfinite(projectile.fMinDistance) || projectile.fMinDistance < 0.f ||
		!std::isfinite(projectile.fMaxDistance) || projectile.fMaxDistance < 0.f ||
		!std::isfinite(projectile.fRadius) || projectile.fRadius < 0.f ||
		hitCount < 1u || hitCount > 16u)
	{
		m_strStatus = "Skill projectile row is invalid";
		return false;
	}
	projectile.eOrigin = 0u == origin ?
		PLAYER_PROJECTILE_ORIGIN::CASTER : PLAYER_PROJECTILE_ORIGIN::AIM;
	const std::string_view kind = fields[firstField + 2u];
	if ("MISSILE" == kind)
		projectile.eKind = PLAYER_PROJECTILE_KIND::MISSILE;
	else if ("FIXAREA" == kind)
		projectile.eKind = PLAYER_PROJECTILE_KIND::FIXAREA;
	else if ("GRENADE" == kind)
		projectile.eKind = PLAYER_PROJECTILE_KIND::GRENADE;
	else if ("TRACE" == kind)
		projectile.eKind = PLAYER_PROJECTILE_KIND::TRACE;
	else
	{
		m_strStatus = "Skill projectile kind is invalid";
		return false;
	}
	/* A fixed area is the one kind that does not move. */
	if ((PLAYER_PROJECTILE_KIND::FIXAREA == projectile.eKind) !=
		(0.f == projectile.fSpeed))
	{
		m_strStatus = "Skill projectile motion is invalid";
		return false;
	}
	const std::string_view packed = fields[firstField + 12u];
	std::size_t cursor = 0;
	bool sawTimed = false;
	while (cursor <= packed.size())
	{
		const std::size_t comma = packed.find(',', cursor);
		const std::string_view token{
			packed.data() + cursor,
			(std::string::npos == comma ? packed.size() : comma) - cursor };
		std::string_view hitFields[14];
		std::size_t fieldCount = 0;
		std::size_t start = 0;
		while (fieldCount < 14)
		{
			const std::size_t colon = token.find(':', start);
			hitFields[fieldCount++] = token.substr(start,
				std::string_view::npos == colon ? std::string_view::npos : colon - start);
			if (std::string_view::npos == colon)
				break;
			start = colon + 1;
		}
		PLAYER_PROJECTILE_HIT hit{};
		std::uint32_t trigger = 0;
		if (14u != fieldCount ||
			!ParseNumber(hitFields[0], trigger) || trigger > 1u ||
			!ParseNumber(hitFields[1], hit.Hit.iTimeMs) ||
			!ParseNumber(hitFields[2], hit.Hit.iRepeatCount) ||
			!ParseNumber(hitFields[3], hit.Hit.iRepeatMs) ||
			!Parse_HitShapeExtent(hitFields + 4, hit.Hit) ||
			hit.Hit.iTimeMs > 600000u ||
			0u == hit.Hit.iRepeatCount || hit.Hit.iRepeatCount > 64u ||
			(hit.Hit.iRepeatCount > 1u && 0u == hit.Hit.iRepeatMs))
		{
			m_strStatus = "Skill projectile hit is invalid";
			return false;
		}
		hit.isContact = 0u == trigger;
		/* contact hits lead at time 0, then timed hits in schedule order */
		if ((hit.isContact && (0u != hit.Hit.iTimeMs || sawTimed)) ||
			(!hit.isContact && !projectile.Hits.empty() &&
				!projectile.Hits.back().isContact &&
				hit.Hit.iTimeMs < projectile.Hits.back().Hit.iTimeMs))
		{
			m_strStatus = "Skill projectile hit order is invalid";
			return false;
		}
		sawTimed = sawTimed || !hit.isContact;
		projectile.Hits.push_back(hit);
		if (std::string::npos == comma)
			break;
		cursor = comma + 1;
	}
	if (projectile.Hits.size() != hitCount)
	{
		m_strStatus = "Skill projectile hit count is invalid";
		return false;
	}
	outProjectiles.push_back(std::move(projectile));
	return true;
}

bool LostArk::Server::CGameplayCatalog::Load()
{
	const std::filesystem::path dataRoot = Resolve_DataRoot();
	if (dataRoot.empty())
	{
		m_strStatus = "Gameplay data root could not be resolved";
		return false;
	}
	return Load_BootstrapPath(
		dataRoot / L"Gameplay" / L"Gameplay.bootstrap", nullptr, nullptr);
}

bool LostArk::Server::CGameplayCatalog::Load_FromBootstrap(
	const std::filesystem::path& bootstrapPath,
	const LostArk::Shared::GameplayDataRevision& expectedBootstrapRevision,
	const LostArk::Shared::GameplayDataRevision& parentRevision)
{
	if (!expectedBootstrapRevision.Is_Valid() || !parentRevision.Is_Valid() ||
		bootstrapPath.empty() || !bootstrapPath.is_absolute())
	{
		m_strStatus = "Staged gameplay bootstrap admission input is invalid";
		return false;
	}

	std::error_code error;
	const std::filesystem::path normalized = bootstrapPath.lexically_normal();
	if (normalized != bootstrapPath)
	{
		m_strStatus = "Staged gameplay bootstrap path is not canonical";
		return false;
	}
	const std::filesystem::file_status linkStatus =
		std::filesystem::symlink_status(normalized, error);
	if (error || !std::filesystem::is_regular_file(linkStatus) ||
		normalized.filename() != L"Gameplay.bootstrap")
	{
		m_strStatus = "Staged gameplay bootstrap is not a regular canonical artifact";
		return false;
	}
	const std::filesystem::path canonical =
		std::filesystem::canonical(normalized, error);
	if (error || canonical != normalized)
	{
		m_strStatus = "Staged gameplay bootstrap path is not canonical";
		return false;
	}

	return Load_BootstrapPath(
		canonical, &expectedBootstrapRevision, &parentRevision);
}

bool LostArk::Server::CGameplayCatalog::Load_BootstrapPath(
	const std::filesystem::path& path,
	const LostArk::Shared::GameplayDataRevision* expectedBootstrapRevision,
	const LostArk::Shared::GameplayDataRevision* parentRevision)
{
	using SKILL_MAP = decltype(m_Skills);
	using BOSS_MAP = decltype(m_Bosses);
	using BOSS_PART_MAP = decltype(m_BossParts);
	using PATTERN_MAP = decltype(m_BossPatterns);
	using BOSS_COMBAT_OBJECT_MAP = decltype(m_BossCombatObjects);
	using INTRO_MAP = decltype(m_IntroPatternIdByEncounter);
	using ROTATION_MAP = decltype(m_BossPatternRotations);
	using SEQUENCE_MAP = decltype(m_BossPatternSequences);
	using TIMELINE_MAP = decltype(m_ValtanTimelines);
	using PLAYER_MAP = decltype(m_Players);
	using DAMAGE_MAP = decltype(m_DamageRatePercentByProfileId);
	struct LOAD_ROLLBACK final
	{
		SKILL_MAP& skills;
		BOSS_MAP& bosses;
		BOSS_PART_MAP& bossParts;
		PATTERN_MAP& patterns;
		BOSS_COMBAT_OBJECT_MAP& bossCombatObjects;
		INTRO_MAP& intros;
		ROTATION_MAP& rotations;
		SEQUENCE_MAP& sequences;
		TIMELINE_MAP& timelines;
		PLAYER_MAP& players;
		DAMAGE_MAP& damages;
		LostArk::Shared::GameplayDataRevision& presentationGenerationId;
		SKILL_MAP previousSkills;
		BOSS_MAP previousBosses;
		BOSS_PART_MAP previousBossParts;
		PATTERN_MAP previousPatterns;
		BOSS_COMBAT_OBJECT_MAP previousBossCombatObjects;
		INTRO_MAP previousIntros;
		ROTATION_MAP previousRotations;
		SEQUENCE_MAP previousSequences;
		TIMELINE_MAP previousTimelines;
		PLAYER_MAP previousPlayers;
		DAMAGE_MAP previousDamages;
		LostArk::Shared::GameplayDataRevision previousPresentationGenerationId{};
		bool committed = false;

		LOAD_ROLLBACK(
			SKILL_MAP& skillTarget,
			BOSS_MAP& bossTarget,
			BOSS_PART_MAP& bossPartTarget,
			PATTERN_MAP& patternTarget,
			BOSS_COMBAT_OBJECT_MAP& bossCombatObjectTarget,
			INTRO_MAP& introTarget,
			ROTATION_MAP& rotationTarget,
			SEQUENCE_MAP& sequenceTarget,
			TIMELINE_MAP& timelineTarget,
			PLAYER_MAP& playerTarget,
			DAMAGE_MAP& damageTarget,
			LostArk::Shared::GameplayDataRevision& presentationGenerationTarget)
			: skills(skillTarget)
			, bosses(bossTarget)
			, bossParts(bossPartTarget)
			, patterns(patternTarget)
			, bossCombatObjects(bossCombatObjectTarget)
			, intros(introTarget)
			, rotations(rotationTarget)
			, sequences(sequenceTarget)
			, timelines(timelineTarget)
			, players(playerTarget)
			, damages(damageTarget)
			, presentationGenerationId(presentationGenerationTarget)
			, previousSkills(std::move(skillTarget))
			, previousBosses(std::move(bossTarget))
			, previousBossParts(std::move(bossPartTarget))
			, previousPatterns(std::move(patternTarget))
			, previousBossCombatObjects(std::move(bossCombatObjectTarget))
			, previousIntros(std::move(introTarget))
			, previousRotations(std::move(rotationTarget))
			, previousSequences(std::move(sequenceTarget))
			, previousTimelines(std::move(timelineTarget))
			, previousPlayers(std::move(playerTarget))
			, previousDamages(std::move(damageTarget))
			, previousPresentationGenerationId(presentationGenerationTarget)
		{
		}

		~LOAD_ROLLBACK()
		{
			if (committed)
				return;
			skills = std::move(previousSkills);
			bosses = std::move(previousBosses);
			bossParts = std::move(previousBossParts);
			patterns = std::move(previousPatterns);
			bossCombatObjects = std::move(previousBossCombatObjects);
			intros = std::move(previousIntros);
			rotations = std::move(previousRotations);
			sequences = std::move(previousSequences);
			timelines = std::move(previousTimelines);
			players = std::move(previousPlayers);
			damages = std::move(previousDamages);
			presentationGenerationId = previousPresentationGenerationId;
		}
	};
	LOAD_ROLLBACK rollback{
		m_Skills, m_Bosses, m_BossParts, m_BossPatterns, m_BossCombatObjects,
		m_IntroPatternIdByEncounter, m_BossPatternRotations,
		m_BossPatternSequences,
		m_ValtanTimelines, m_Players,
		m_DamageRatePercentByProfileId,
		m_ValtanPresentationGenerationId };
	m_Skills.clear();
	m_Bosses.clear();
	m_BossParts.clear();
	m_BossPatterns.clear();
	m_BossCombatObjects.clear();
	m_IntroPatternIdByEncounter.clear();
	m_BossPatternRotations.clear();
	m_BossPatternSequences.clear();
	m_ValtanTimelines.clear();
	m_Players.clear();
	m_DamageRatePercentByProfileId.clear();
	m_ValtanPresentationGenerationId = {};

	std::ifstream bootstrapFile(path, std::ios::binary);
	if (!bootstrapFile)
	{
		m_strStatus = "Missing gameplay bootstrap: " + path.string();
		return false;
	}
	const std::string bootstrapBytes{
		std::istreambuf_iterator<char>{ bootstrapFile },
		std::istreambuf_iterator<char>{} };
	if (bootstrapFile.bad())
	{
		m_strStatus = "Gameplay bootstrap could not be read";
		return false;
	}
	LostArk::Shared::GameplayDataRevision admittedRevision{};
	if (!Calculate_GameplayDataRevision(bootstrapBytes, admittedRevision))
	{
		m_strStatus = "Gameplay bootstrap revision could not be calculated";
		return false;
	}
	if (nullptr != expectedBootstrapRevision &&
		admittedRevision != *expectedBootstrapRevision)
	{
		m_strStatus = "Staged gameplay bootstrap content revision mismatch";
		return false;
	}
	std::istringstream input{ bootstrapBytes };

	std::string line;
	if (!std::getline(input, line))
	{
		m_strStatus = "Gameplay bootstrap is empty";
		return false;
	}
	StripCarriageReturn(line);
	const std::vector<std::string_view> header = SplitTabs(line);
	std::uint32_t version = 0;
	std::uint32_t rowCount = 0;
	if (3u != header.size() || "LOSTARK_GAMEPLAY_BOOTSTRAP" != header[0] ||
		!ParseNumber(header[1], version) ||
		GAMEPLAY_BOOTSTRAP_VERSION != version ||
		!ParseNumber(header[2], rowCount) || 0u == rowCount || rowCount > 4096u)
	{
		m_strStatus = "Gameplay bootstrap header is invalid";
		return false;
	}

	std::unordered_set<LostArk::Shared::SKILL_ID> skillCombatTraitOwners;
	std::unordered_set<LostArk::Shared::SKILL_ID> skillTargetOwners;
	std::unordered_set<std::string> patternPolicyOwners;
	std::unordered_set<std::string> patternStagePartDamageOwners;
	std::unordered_set<std::string> patternStageCounterProxyOwners;
	for (std::uint32_t row = 0; row < rowCount; ++row)
	{
		if (!std::getline(input, line))
		{
			m_strStatus = "Gameplay bootstrap row is truncated";
			return false;
		}
		StripCarriageReturn(line);
		const std::vector<std::string_view> fields = SplitTabs(line);
		if (!fields.empty() && "PATTERNPRESENTATIONGENERATION" == fields[0])
		{
			if (3u != fields.size() || "ENCOUNTER_VALTAN" != fields[1] ||
				m_ValtanPresentationGenerationId.Is_Valid() ||
				!LostArk::Shared::Try_Parse_GameplayDataRevision(
					fields[2], m_ValtanPresentationGenerationId))
			{
				m_strStatus =
					"Valtan presentation generation row is invalid or duplicated";
				return false;
			}
		}
		else if (!fields.empty() && "DAMAGE" == fields[0])
		{
			std::uint32_t ratePercent = 0;
			if (3u != fields.size() || !IsStableId(fields[1]) ||
				!ParseNumber(fields[2], ratePercent) || 0u == ratePercent ||
				ratePercent > MAXIMUM_DAMAGE_RATE_PERCENT ||
				!m_DamageRatePercentByProfileId.emplace(
					std::string(fields[1]), ratePercent).second)
			{
				m_strStatus = "Damage profile row is invalid";
				return false;
			}
		}
		else if (!fields.empty() && "SKILL" == fields[0])
		{
			PLAYER_SKILL_DEFINITION skill{};
			if (16u != fields.size() ||
				!ParseNumber(fields[1], skill.iSkillId) ||
				LostArk::Shared::INVALID_SKILL_ID == skill.iSkillId ||
				!ParseCharacterClass(fields[2], skill.eCharacterClass) ||
				!IsStableId(fields[3]) || !IsStableId(fields[4]) ||
				!ParseNumber(fields[5], skill.iCooldownMs) ||
				!ParseNumber(fields[6], skill.iActionDurationMs) ||
				!ParseNumber(fields[7], skill.iHitTimeMs) ||
				!ParseNumber(fields[8], skill.iResourceCost) ||
				!ParseNumber(fields[9], skill.iIdentityCost) ||
				!ParseNumber(fields[10], skill.fMovementDistance) ||
				!ParseNumber(fields[11], skill.fMaximumRange) ||
				(!fields[12].empty() && !IsStableId(fields[12])) ||
				!ParseSkillKind(fields[13], skill.eSkillKind) ||
				!ParseStance(fields[14], skill.eRequiredStance) ||
				!ParseStance(fields[15], skill.eSetsStance) ||
				(LostArk::Shared::PLAYER_SKILL_KIND::ACTIVE == skill.eSkillKind &&
					0u == skill.iCooldownMs) ||
				0u == skill.iActionDurationMs ||
				/* iHitTimeMs may be 0: a skill can land as the cast starts, so only
					the upper bound below is a real constraint. */
				skill.iHitTimeMs > skill.iActionDurationMs ||
				/* Cost is bounded against the largest class pool after every row is
					read: a single row cannot know which pool applies. */
				!std::isfinite(skill.fMovementDistance) ||
				!std::isfinite(skill.fMaximumRange) ||
				skill.fMovementDistance < 0.f ||
				(fields[12].empty() ?
					(skill.fMaximumRange != 0.f || 0u != skill.iHitTimeMs) :
					skill.fMaximumRange <= 0.f))
			{
				m_strStatus = "Player skill row is invalid";
				return false;
			}
			skill.strInputSlot = fields[3];
			skill.strActionId = fields[4];
			skill.strDamageProfileId = fields[12];
			if (!m_Skills.emplace(skill.iSkillId, std::move(skill)).second)
			{
				m_strStatus = "Duplicate player skill ID";
				return false;
			}
		}
		else if (!fields.empty() && "SKILLCOMBATTRAITS" == fields[0])
		{
			LostArk::Shared::SKILL_ID ownerSkillId =
				LostArk::Shared::INVALID_SKILL_ID;
			std::uint32_t staggerDamage = 0;
			std::uint32_t partDamage = 0;
			std::uint32_t counterPower = 0;
			if (5u != fields.size() ||
				!ParseNumber(fields[1], ownerSkillId) ||
				!ParseNumber(fields[2], staggerDamage) ||
				!ParseNumber(fields[3], partDamage) ||
				!ParseNumber(fields[4], counterPower) ||
				staggerDamage > 1000000u || partDamage > 1000000u ||
				counterPower > 1000000u)
			{
				m_strStatus = "Player skill combat traits row is invalid";
				return false;
			}
			const auto owner = m_Skills.find(ownerSkillId);
			if (m_Skills.end() == owner ||
				!skillCombatTraitOwners.insert(ownerSkillId).second ||
				(0u != counterPower &&
				 LostArk::Shared::PLAYER_SKILL_KIND::COUNTER !=
					owner->second.eSkillKind))
			{
				m_strStatus =
					"Player skill combat traits have no owner or are duplicated";
				return false;
			}
			owner->second.iStaggerDamage = staggerDamage;
			owner->second.iPartDamage = partDamage;
			owner->second.iCounterPower = counterPower;
		}
		else if (!fields.empty() && "SKILLTARGET" == fields[0])
		{
			LostArk::Shared::SKILL_ID ownerSkillId =
				LostArk::Shared::INVALID_SKILL_ID;
			float maximumRange = 0.f;
			std::uint32_t requiresWalkable = 0u;
			if (5u != fields.size() ||
				!ParseNumber(fields[1], ownerSkillId) ||
				"GROUND_POINT" != fields[2] ||
				!ParseNumber(fields[3], maximumRange) ||
				!ParseNumber(fields[4], requiresWalkable) ||
				!std::isfinite(maximumRange) || maximumRange <= 0.f ||
				1u != requiresWalkable)
			{
				m_strStatus = "Player skill target row is invalid";
				return false;
			}
			const auto owner = m_Skills.find(ownerSkillId);
			if (m_Skills.end() == owner ||
				!skillTargetOwners.insert(ownerSkillId).second ||
				LostArk::Shared::PLAYER_SKILL_KIND::ACTIVE !=
					owner->second.eSkillKind ||
				maximumRange != owner->second.fMaximumRange)
			{
				m_strStatus =
					"Player skill target has no owner, is duplicated, or conflicts";
				return false;
			}
			owner->second.eTargetIntent =
				LostArk::Shared::SKILL_TARGET_INTENT_KIND::GROUND_POINT;
			owner->second.fTargetMaximumRange = maximumRange;
			owner->second.requiresWalkableTarget = true;
		}
		else if (!fields.empty() && "SKILLSTAGE" == fields[0])
		{
			LostArk::Shared::SKILL_ID ownerSkillId =
				LostArk::Shared::INVALID_SKILL_ID;
			std::uint32_t stageIndex = 0;
			PLAYER_COMBO_STAGE stage{};
			if (8u != fields.size() ||
				!ParseNumber(fields[1], ownerSkillId) ||
				!ParseNumber(fields[2], stageIndex) ||
				!ParseNumber(fields[3], stage.iActionDurationMs) ||
				!ParseNumber(fields[4], stage.iHitTimeMs) ||
				!ParseNumber(fields[5], stage.iComboAdvanceMs) ||
				!ParseNumber(fields[6], stage.iInputOpenMs) ||
				!ParseNumber(fields[7], stage.iInputCloseMs) ||
				0u == stage.iActionDurationMs ||
				stage.iHitTimeMs > stage.iComboAdvanceMs ||
				stage.iComboAdvanceMs > stage.iActionDurationMs ||
				stage.iInputCloseMs > stage.iActionDurationMs ||
				(0u == stage.iInputCloseMs ? 0u != stage.iInputOpenMs :
					stage.iInputOpenMs >= stage.iInputCloseMs))
			{
				m_strStatus = "Combo stage row is invalid";
				return false;
			}
			const auto owner = m_Skills.find(ownerSkillId);
			// Stages arrive after their skill and in order, so a row that names an
			// unknown skill or skips an index is a corrupt bootstrap, not a
			// tolerable gap.
			if (owner == m_Skills.end() ||
				(LostArk::Shared::PLAYER_SKILL_KIND::COMBO !=
					owner->second.eSkillKind &&
					LostArk::Shared::PLAYER_SKILL_KIND::HOLD !=
						owner->second.eSkillKind &&
					LostArk::Shared::PLAYER_SKILL_KIND::COUNTER !=
						owner->second.eSkillKind) ||
				stageIndex != owner->second.ComboStages.size())
			{
				m_strStatus = "Combo stage does not follow its skill";
				return false;
			}
			owner->second.ComboStages.push_back(stage);
		}
		else if (!fields.empty() && "SKILLROOTMOTION" == fields[0])
		{
			LostArk::Shared::SKILL_ID ownerSkillId =
				LostArk::Shared::INVALID_SKILL_ID;
			std::uint32_t sampleCount = 0;
			if (4u != fields.size() ||
				!ParseNumber(fields[1], ownerSkillId) ||
				!ParseNumber(fields[2], sampleCount) ||
				sampleCount < 2u || sampleCount > 512u)
			{
				m_strStatus = "Root motion row is invalid";
				return false;
			}
			const auto owner = m_Skills.find(ownerSkillId);
			if (owner == m_Skills.end() || !owner->second.RootMotion.empty())
			{
				m_strStatus = "Root motion does not follow its skill";
				return false;
			}
			std::vector<ROOT_MOTION_SAMPLE> samples;
			if (!Parse_RootMotionSamples(
				fields[3], sampleCount, owner->second.iActionDurationMs, samples))
			{
				return false;
			}
			owner->second.RootMotion = std::move(samples);
		}
		else if (!fields.empty() && "PATTERNSTAGEROOTMOTION" == fields[0])
		{
			/* The row sorts after every PATTERNSTAGE of the same encounter, so the
			stage it names must already exist. An index past the end or a second
			curve for one stage is a corrupt bootstrap, not a tolerable gap. */
			std::uint32_t stageIndex = 0;
			std::uint32_t sampleCount = 0;
			if (6u != fields.size() ||
				!ParseNumber(fields[3], stageIndex) ||
				!ParseNumber(fields[4], sampleCount) ||
				sampleCount < 2u || sampleCount > 512u)
			{
				m_strStatus = "Pattern stage root motion row is invalid";
				return false;
			}
			const auto encounter = m_BossPatterns.find(std::string(fields[1]));
			if (encounter == m_BossPatterns.end())
			{
				m_strStatus = "Pattern stage root motion names an unknown encounter";
				return false;
			}
			const auto pattern = std::find_if(
				encounter->second.begin(), encounter->second.end(),
				[&fields](const BOSS_PATTERN_DEFINITION& candidate)
				{ return candidate.strPatternId == fields[2]; });
			if (encounter->second.end() == pattern ||
				stageIndex >= pattern->Stages.size() ||
				BOSS_PATTERN_STAGE_MOTION_KIND::PORTAL_TARGET_RUSH ==
					pattern->Stages[stageIndex].Motion.eKind ||
				!pattern->Stages[stageIndex].Motion.RootMotion.empty())
			{
				m_strStatus = "Pattern stage root motion does not follow its stage";
				return false;
			}
			std::vector<ROOT_MOTION_SAMPLE> samples;
			if (!Parse_RootMotionSamples(
				fields[5], sampleCount,
				pattern->Stages[stageIndex].iDurationMs, samples))
			{
				return false;
			}
			pattern->Stages[stageIndex].Motion.RootMotion = std::move(samples);
		}
		else if (!fields.empty() && "SKILLSTAGEROOTMOTION" == fields[0])
		{
			LostArk::Shared::SKILL_ID ownerSkillId =
				LostArk::Shared::INVALID_SKILL_ID;
			std::uint32_t stageIndex = 0;
			std::uint32_t sampleCount = 0;
			if (5u != fields.size() ||
				!ParseNumber(fields[1], ownerSkillId) ||
				!ParseNumber(fields[2], stageIndex) ||
				!ParseNumber(fields[3], sampleCount) ||
				sampleCount < 2u || sampleCount > 512u)
			{
				m_strStatus = "Root motion row is invalid";
				return false;
			}
			const auto owner = m_Skills.find(ownerSkillId);
			// Stages arrive after their own SKILLCOMBOSTAGE rows, so an index
			// past the end is a corrupt bootstrap rather than a tolerable gap.
			if (owner == m_Skills.end() ||
				stageIndex >= owner->second.ComboStages.size() ||
				!owner->second.ComboStages[stageIndex].RootMotion.empty())
			{
				m_strStatus = "Root motion does not follow its skill";
				return false;
			}
			PLAYER_COMBO_STAGE& stage = owner->second.ComboStages[stageIndex];
			std::vector<ROOT_MOTION_SAMPLE> samples;
			if (!Parse_RootMotionSamples(
				fields[4], sampleCount, stage.iActionDurationMs, samples))
			{
				return false;
			}
			stage.RootMotion = std::move(samples);
		}
		else if (!fields.empty() && "SKILLHIT" == fields[0])
		{
			LostArk::Shared::SKILL_ID ownerSkillId =
				LostArk::Shared::INVALID_SKILL_ID;
			std::uint32_t hitCount = 0;
			if (4u != fields.size() ||
				!ParseNumber(fields[1], ownerSkillId) ||
				!ParseNumber(fields[2], hitCount) ||
				hitCount < 1u || hitCount > 64u)
			{
				m_strStatus = "Skill hit row is invalid";
				return false;
			}
			const auto owner = m_Skills.find(ownerSkillId);
			if (owner == m_Skills.end() || !owner->second.Hits.empty() ||
				owner->second.strDamageProfileId.empty() ||
				LostArk::Shared::PLAYER_SKILL_KIND::ACTIVE !=
					owner->second.eSkillKind)
			{
				m_strStatus = "Skill hit row does not follow its skill";
				return false;
			}
			std::vector<PLAYER_SKILL_HIT> hits;
			if (!Parse_SkillHits(
				fields[3], hitCount, owner->second.iActionDurationMs, hits))
			{
				return false;
			}
			owner->second.Hits = std::move(hits);
		}
		else if (!fields.empty() && "SKILLSTAGEHIT" == fields[0])
		{
			LostArk::Shared::SKILL_ID ownerSkillId =
				LostArk::Shared::INVALID_SKILL_ID;
			std::uint32_t stageIndex = 0;
			std::uint32_t hitCount = 0;
			if (5u != fields.size() ||
				!ParseNumber(fields[1], ownerSkillId) ||
				!ParseNumber(fields[2], stageIndex) ||
				!ParseNumber(fields[3], hitCount) ||
				hitCount < 1u || hitCount > 64u)
			{
				m_strStatus = "Skill stage hit row is invalid";
				return false;
			}
			const auto owner = m_Skills.find(ownerSkillId);
			if (owner == m_Skills.end() ||
				owner->second.strDamageProfileId.empty() ||
				stageIndex >= owner->second.ComboStages.size() ||
				!owner->second.ComboStages[stageIndex].Hits.empty())
			{
				m_strStatus = "Skill stage hit row does not follow its skill";
				return false;
			}
			PLAYER_COMBO_STAGE& stage = owner->second.ComboStages[stageIndex];
			std::vector<PLAYER_SKILL_HIT> hits;
			if (!Parse_SkillHits(
				fields[4], hitCount, stage.iActionDurationMs, hits))
			{
				return false;
			}
			stage.Hits = std::move(hits);
		}
		else if (!fields.empty() && "SKILLPROJ" == fields[0])
		{
			LostArk::Shared::SKILL_ID ownerSkillId =
				LostArk::Shared::INVALID_SKILL_ID;
			if (fields.size() < 2u || !ParseNumber(fields[1], ownerSkillId))
			{
				m_strStatus = "Skill projectile row is invalid";
				return false;
			}
			const auto owner = m_Skills.find(ownerSkillId);
			if (owner == m_Skills.end() ||
				owner->second.strDamageProfileId.empty() ||
				LostArk::Shared::PLAYER_SKILL_KIND::ACTIVE !=
					owner->second.eSkillKind)
			{
				m_strStatus = "Skill projectile row does not follow its skill";
				return false;
			}
			if (!Parse_SkillProjectile(fields, 2u,
				owner->second.iActionDurationMs, owner->second.Projectiles))
			{
				return false;
			}
		}
		else if (!fields.empty() && "SKILLSTAGEPROJ" == fields[0])
		{
			LostArk::Shared::SKILL_ID ownerSkillId =
				LostArk::Shared::INVALID_SKILL_ID;
			std::uint32_t stageIndex = 0;
			if (fields.size() < 3u || !ParseNumber(fields[1], ownerSkillId) ||
				!ParseNumber(fields[2], stageIndex))
			{
				m_strStatus = "Skill stage projectile row is invalid";
				return false;
			}
			const auto owner = m_Skills.find(ownerSkillId);
			if (owner == m_Skills.end() ||
				owner->second.strDamageProfileId.empty() ||
				stageIndex >= owner->second.ComboStages.size())
			{
				m_strStatus = "Skill stage projectile row does not follow its skill";
				return false;
			}
			PLAYER_COMBO_STAGE& stage = owner->second.ComboStages[stageIndex];
			if (!Parse_SkillProjectile(fields, 3u,
				stage.iActionDurationMs, stage.Projectiles))
			{
				return false;
			}
		}
		else if (!fields.empty() && "BOSS" == fields[0])
		{
			BOSS_RUNTIME_PROFILE boss{};
			if (11u != fields.size() || !IsStableId(fields[1]) ||
				!IsStableId(fields[2]) ||
				!ParseNumber(fields[3], boss.iMaximumHp) ||
				!ParseNumber(fields[4], boss.iMaximumHealthBars) ||
				!ParseNumber(fields[5], boss.iAttackPower) ||
				!ParseNumber(fields[6], boss.fCollisionRadius) ||
				!ParseNumber(fields[7], boss.fEngageDistance) ||
				!ParseNumber(fields[8], boss.fMoveSpeed) ||
				!ParseBossPhasePolicyKind(fields[9], boss.PhasePolicy.eKind) ||
				!ParseNumber(fields[10], boss.PhasePolicy.iThresholdPercent) ||
				0u == boss.iMaximumHp || 0u == boss.iMaximumHealthBars ||
				boss.iMaximumHealthBars > 1000u || 0u == boss.iAttackPower ||
				!std::isfinite(boss.fCollisionRadius) ||
				boss.fCollisionRadius <= 0.f ||
				!std::isfinite(boss.fEngageDistance) ||
				!std::isfinite(boss.fMoveSpeed) || boss.fEngageDistance <= 0.f ||
				boss.fMoveSpeed <= 0.f ||
				(BOSS_PHASE_POLICY_KIND::HEALTH_PERCENT_THRESHOLD ==
					boss.PhasePolicy.eKind &&
					(0u == boss.PhasePolicy.iThresholdPercent ||
					 boss.PhasePolicy.iThresholdPercent >= 100u)) ||
				(BOSS_PHASE_POLICY_KIND::AUTHORED_PATTERN_EVENT ==
					boss.PhasePolicy.eKind &&
					0u != boss.PhasePolicy.iThresholdPercent))
			{
				m_strStatus = "Boss profile row is invalid";
				return false;
			}
			boss.strArchetypeId = fields[1];
			boss.strEncounterId = fields[2];
			if (!m_Bosses.emplace(boss.strArchetypeId, std::move(boss)).second)
			{
				m_strStatus = "Duplicate boss archetype ID";
				return false;
			}
		}
		else if (!fields.empty() && "BOSSARMOR" == fields[0])
		{
			BOSS_ARMOR_PLATE plate{};
			if (5u != fields.size() || !IsStableId(fields[1]) ||
				!ParseNumber(fields[2], plate.iPlateIndex) ||
				!ParseNumber(fields[3], plate.iDurability) ||
				!ParseNumber(fields[4], plate.iDefense) ||
				0u == plate.iDurability || 0u == plate.iDefense ||
				plate.iDefense > 10000u)
			{
				m_strStatus = "Boss armour plate row is invalid";
				return false;
			}
			const auto owner = m_Bosses.find(std::string(fields[1]));
			if (m_Bosses.end() == owner)
			{
				m_strStatus = "Boss armour plate has no boss profile";
				return false;
			}
			/* Dense and ordered so the index is a stable slot the client can
			match to its own part order instead of a search. */
			if (plate.iPlateIndex != owner->second.ArmorPlates.size() ||
				owner->second.ArmorPlates.size() >= MAXIMUM_BOSS_ARMOR_PLATES)
			{
				m_strStatus = "Boss armour plate index is out of order";
				return false;
			}
			owner->second.ArmorPlates.push_back(plate);
		}
		else if (!fields.empty() && "BOSSPART" == fields[0])
		{
			BOSS_PART_DEFINITION part{};
			if (7u != fields.size() || !IsStableId(fields[1]) ||
				!IsStableId(fields[2]) ||
				!ParseNumber(fields[3], part.iStateMask) ||
				!ParseNumber(fields[4], part.iMaximumDurability) ||
				!ParseNumber(fields[5], part.iDamageReductionPercent) ||
				!ParseBossPartDamageCondition(fields[6], part.eDamageCondition) ||
				0u == part.iStateMask ||
				0u != (part.iStateMask & (part.iStateMask - 1u)) ||
				0u == part.iMaximumDurability ||
				part.iMaximumDurability > 1000000u ||
				0u == part.iDamageReductionPercent ||
				part.iDamageReductionPercent > 90u ||
				m_Bosses.end() == m_Bosses.find(std::string(fields[1])))
			{
				m_strStatus = "Boss part row is invalid";
				return false;
			}
			part.strBossArchetypeId = fields[1];
			part.strPartId = fields[2];
			auto& parts = m_BossParts[part.strBossArchetypeId];
			if (std::any_of(parts.begin(), parts.end(),
				[&part](const BOSS_PART_DEFINITION& existing)
				{
					return existing.strPartId == part.strPartId ||
						existing.iStateMask == part.iStateMask;
				}))
			{
				m_strStatus = "Duplicate boss part ID or state mask";
				return false;
			}
			parts.push_back(std::move(part));
		}
		else if (!fields.empty() && "BOSSCOMBATOBJECT" == fields[0])
		{
			BOSS_COMBAT_OBJECT_DEFINITION definition{};
			if (15u != fields.size() || !IsStableId(fields[1]) ||
				!IsStableId(fields[2]) || !IsStableId(fields[3]) ||
				!IsStableId(fields[4]) || !IsStableId(fields[5]) ||
				!ParseBossCombatObjectKind(fields[6], definition.eKind) ||
				!ParseBossCombatObjectOriginPolicy(
					fields[7], definition.eOriginPolicy) ||
				!ParseBossCombatObjectDirectionPolicy(
					fields[8], definition.eDirectionPolicy) ||
				!ParseNumber(fields[9], definition.fOffsetForwardM) ||
				!ParseNumber(fields[10], definition.fOffsetRightM) ||
				!ParseNumber(fields[11], definition.fSpeedMps) ||
				!ParseNumber(fields[12], definition.fMaximumDistanceM) ||
				!ParseNumber(fields[13], definition.iLifeMs) ||
				!ParseNumber(fields[14], definition.iExpectedEventCount) ||
				!std::isfinite(definition.fOffsetForwardM) ||
				!std::isfinite(definition.fOffsetRightM) ||
				!std::isfinite(definition.fSpeedMps) ||
				!std::isfinite(definition.fMaximumDistanceM) ||
				std::abs(definition.fOffsetForwardM) > 100.f ||
				std::abs(definition.fOffsetRightM) > 100.f ||
				definition.fSpeedMps < 0.f || definition.fSpeedMps > 1000.f ||
				definition.fMaximumDistanceM < 0.f ||
				definition.fMaximumDistanceM > 1000.f ||
				0u == definition.iLifeMs || definition.iLifeMs > 600000u ||
				0u == definition.iExpectedEventCount ||
				definition.iExpectedEventCount > 16u)
			{
				m_strStatus = "Boss combat object row is invalid";
				return false;
			}
			const bool fixedArea = BOSS_COMBAT_OBJECT_KIND::FIXED_AREA ==
				definition.eKind;
			/* A fixed area is placed on a player, either the boss's single
			pattern target or one per living raider for a volley. */
			const bool lockedOrigin =
				BOSS_COMBAT_OBJECT_ORIGIN_POLICY::
					LOCKED_TARGET_UNTIL_FIRST_PULSE == definition.eOriginPolicy ||
				BOSS_COMBAT_OBJECT_ORIGIN_POLICY::
					LOCKED_TARGET_PER_ALIVE_PLAYER == definition.eOriginPolicy;
			const bool birthPoseOrigin = BOSS_COMBAT_OBJECT_ORIGIN_POLICY::
				BOSS_POSITION == definition.eOriginPolicy;
			const bool validFixedArea = fixedArea && (lockedOrigin || birthPoseOrigin) &&
				BOSS_COMBAT_OBJECT_DIRECTION_POLICY::NONE ==
					definition.eDirectionPolicy &&
				0.f == definition.fOffsetForwardM &&
				0.f == definition.fOffsetRightM && 0.f == definition.fSpeedMps &&
				0.f == definition.fMaximumDistanceM;
			const double maximumTravelM =
				static_cast<double>(definition.fSpeedMps) *
				(static_cast<double>(definition.iLifeMs) / 1000.0);
			const bool validMissile = !fixedArea &&
				BOSS_COMBAT_OBJECT_ORIGIN_POLICY::BOSS_POSITION ==
					definition.eOriginPolicy &&
				BOSS_COMBAT_OBJECT_DIRECTION_POLICY::PATTERN_FACING_AT_SPAWN ==
					definition.eDirectionPolicy &&
				definition.fSpeedMps > 0.f && definition.fMaximumDistanceM > 0.f &&
				maximumTravelM + 0.00001 >= definition.fMaximumDistanceM;
			if (!validFixedArea && !validMissile)
			{
				m_strStatus = "Boss combat object motion contract is invalid";
				return false;
			}
			definition.strEncounterId = fields[1];
			definition.strCombatObjectArchetypeId = fields[2];
			definition.strClientVisualId = fields[3];
			definition.strOwnerPatternId = fields[4];
			definition.strOwnerStageActionId = fields[5];
			if (!m_BossCombatObjects.emplace(
				definition.strCombatObjectArchetypeId,
				std::move(definition)).second)
			{
				m_strStatus = "Duplicate boss combat object archetype ID";
				return false;
			}
		}
		else if (!fields.empty() && "BOSSCOMBATOBJECTHIT" == fields[0])
		{
			BOSS_COMBAT_OBJECT_HIT hit{};
			std::uint32_t hitIndex = 0;
			std::uint32_t knockdown = 0;
			if (20u != fields.size() || !IsStableId(fields[1]) ||
				!IsStableId(fields[2]) || !ParseNumber(fields[3], hitIndex) ||
				!IsStableId(fields[4]) ||
				!ParseBossCombatObjectHitTrigger(fields[5], hit.eTrigger) ||
				!ParseNumber(fields[6], hit.iAtMs) ||
				!ParseNumber(fields[7], hit.iRepeatCount) ||
				!ParseNumber(fields[8], hit.iRepeatIntervalMs) ||
				!ParseBossPatternHitShape(fields[9], hit.eHitShape) ||
				!ParseNumber(fields[10], hit.fHitOuterRadius) ||
				!ParseNumber(fields[11], hit.fHitInnerRadius) ||
				!ParseNumber(fields[12], hit.fHitAngleDegrees) ||
				!ParseNumber(fields[13], hit.fHitLength) ||
				!ParseNumber(fields[14], hit.fHitHalfWidth) ||
				!IsStableId(fields[15]) ||
				!ParseNumber(fields[16], hit.fPushRangeM) ||
				!ParseNumber(fields[17], hit.iPushMs) ||
				!ParseNumber(fields[18], knockdown) || knockdown > 1u ||
				!ParseNumber(fields[19], hit.iDownMs) ||
				0u == hit.iRepeatCount || hit.iRepeatCount > 64u ||
				(1u == hit.iRepeatCount ? 0u != hit.iRepeatIntervalMs :
					0u == hit.iRepeatIntervalMs) ||
				!std::isfinite(hit.fHitOuterRadius) ||
				!std::isfinite(hit.fHitInnerRadius) ||
				!std::isfinite(hit.fHitAngleDegrees) ||
				!std::isfinite(hit.fHitLength) ||
				!std::isfinite(hit.fHitHalfWidth) ||
				!std::isfinite(hit.fPushRangeM) ||
				std::abs(hit.fPushRangeM) > 20.f ||
				((0.f == hit.fPushRangeM) != (0u == hit.iPushMs)) ||
				((0u != knockdown) != (0u != hit.iDownMs)))
			{
				m_strStatus = "Boss combat object hit row is invalid";
				return false;
			}
			const auto owner = m_BossCombatObjects.find(std::string(fields[2]));
			if (m_BossCombatObjects.end() == owner ||
				owner->second.strEncounterId != std::string(fields[1]) ||
				hitIndex != owner->second.Hits.size() ||
				hitIndex >= owner->second.iExpectedEventCount ||
				std::any_of(owner->second.Hits.begin(), owner->second.Hits.end(),
					[&fields](const BOSS_COMBAT_OBJECT_HIT& existing)
					{
						return existing.strHitId == fields[4];
					}))
			{
				m_strStatus = "Boss combat object hit does not follow its owner";
				return false;
			}
			const std::uint64_t lastPulseMs = hit.iAtMs +
				static_cast<std::uint64_t>(hit.iRepeatCount - 1u) *
				hit.iRepeatIntervalMs;
			const bool fixedArea = BOSS_COMBAT_OBJECT_KIND::FIXED_AREA ==
				owner->second.eKind;
			if (lastPulseMs >= owner->second.iLifeMs ||
				(fixedArea && BOSS_COMBAT_OBJECT_HIT_TRIGGER::TIMED !=
					hit.eTrigger) ||
				(!fixedArea && (BOSS_COMBAT_OBJECT_HIT_TRIGGER::CONTACT !=
					hit.eTrigger || 0u != hit.iAtMs)))
			{
				m_strStatus = "Boss combat object hit timing is invalid";
				return false;
			}
			const bool zeroAngleBox = 0.f == hit.fHitAngleDegrees &&
				0.f == hit.fHitLength && 0.f == hit.fHitHalfWidth;
			bool validShape = false;
			switch (hit.eHitShape)
			{
			case BOSS_PATTERN_HIT_SHAPE::CIRCLE:
				validShape = hit.fHitOuterRadius > 0.f &&
					0.f == hit.fHitInnerRadius && zeroAngleBox;
				break;
			case BOSS_PATTERN_HIT_SHAPE::RING:
				validShape = hit.fHitOuterRadius > hit.fHitInnerRadius &&
					hit.fHitInnerRadius > 0.f && zeroAngleBox;
				break;
			case BOSS_PATTERN_HIT_SHAPE::CONE:
				validShape = hit.fHitAngleDegrees > 0.f &&
					hit.fHitAngleDegrees <= 180.f && hit.fHitLength > 0.f &&
					0.f == hit.fHitOuterRadius && 0.f == hit.fHitInnerRadius &&
					0.f == hit.fHitHalfWidth;
				break;
			case BOSS_PATTERN_HIT_SHAPE::BOX:
			case BOSS_PATTERN_HIT_SHAPE::CROSS:
			case BOSS_PATTERN_HIT_SHAPE::SIX_DIRECTIONS:
				validShape = hit.fHitLength > 0.f && hit.fHitHalfWidth > 0.f &&
					0.f == hit.fHitOuterRadius && 0.f == hit.fHitInnerRadius &&
					0.f == hit.fHitAngleDegrees;
				break;
			case BOSS_PATTERN_HIT_SHAPE::NONE:
				break;
			}
			if (!validShape)
			{
				m_strStatus = "Boss combat object hit shape is invalid";
				return false;
			}
			hit.strHitId = fields[4];
			hit.strDamageProfileId = fields[15];
			hit.bKnockdown = 0u != knockdown;
			owner->second.Hits.push_back(std::move(hit));
		}
		else if (!fields.empty() &&
			"BOSSCOMBATOBJECTPRESENTATION" == fields[0])
		{
			BOSS_COMBAT_OBJECT_PRESENTATION_PULSE pulse{};
			std::uint32_t pulseIndex = 0u;
			if (6u != fields.size() || !IsStableId(fields[1]) ||
				!IsStableId(fields[2]) || !ParseNumber(fields[3], pulseIndex) ||
				!IsStableId(fields[4]) || !ParseNumber(fields[5], pulse.iAtMs))
			{
				m_strStatus =
					"Boss combat object presentation row is invalid";
				return false;
			}
			const auto owner = m_BossCombatObjects.find(std::string(fields[2]));
			if (m_BossCombatObjects.end() == owner ||
				owner->second.strEncounterId != std::string(fields[1]) ||
				pulseIndex != owner->second.PresentationPulses.size() ||
				owner->second.Hits.size() + pulseIndex >=
					owner->second.iExpectedEventCount ||
				pulse.iAtMs > owner->second.iLifeMs ||
				std::any_of(
					owner->second.Hits.begin(), owner->second.Hits.end(),
					[&fields](const BOSS_COMBAT_OBJECT_HIT& existing)
					{
						return existing.strHitId == fields[4];
					}) ||
				std::any_of(
					owner->second.PresentationPulses.begin(),
					owner->second.PresentationPulses.end(),
					[&fields](
						const BOSS_COMBAT_OBJECT_PRESENTATION_PULSE& existing)
					{
						return existing.strPresentationEventId == fields[4];
					}))
			{
				m_strStatus =
					"Boss combat object presentation does not follow its owner";
				return false;
			}
			pulse.strPresentationEventId = fields[4];
			owner->second.PresentationPulses.push_back(std::move(pulse));
		}
		else if (!fields.empty() && "PATTERN" == fields[0])
		{
			BOSS_PATTERN_DEFINITION pattern{};
			std::uint32_t invulnerableFlag = 0u;
			if (17u != fields.size() || !IsStableId(fields[1]) ||
				!IsStableId(fields[2]) || !IsStableId(fields[3]) ||
				!ParseBossPatternSelection(fields[4], pattern.eSelection) ||
				!ParseNumber(fields[5], pattern.iMinimumHealthBar) ||
				!ParseNumber(fields[6], pattern.iMaximumHealthBar) ||
				!ParseNumber(fields[7], pattern.iTriggerHealthBar) ||
				!ParseNumber(fields[8], pattern.iTriggerOrder) ||
				!ParseNumber(fields[9], pattern.iSelectionWeight) ||
				!ParseNumber(fields[10], pattern.iMaximumConsecutiveUses) ||
				!ParseNumber(fields[11], pattern.fMinimumRange) ||
				!ParseNumber(fields[12], pattern.fMaximumRange) ||
				!ParseNumber(fields[13], pattern.iExpectedStageCount) ||
				!ParseBossPatternArmorRequirement(
					fields[14], pattern.eArmorRequirement) ||
				!ParseBossPatternPhaseRequirement(
					fields[15], pattern.ePhaseRequirement) ||
				!ParseNumber(fields[16], invulnerableFlag) ||
				invulnerableFlag > 1u ||
				!std::isfinite(pattern.fMinimumRange) ||
				!std::isfinite(pattern.fMaximumRange) ||
				pattern.fMinimumRange < 0.f ||
				pattern.fMaximumRange <= pattern.fMinimumRange ||
				0u == pattern.iExpectedStageCount ||
				pattern.iExpectedStageCount > 64u)
			{
				m_strStatus = "Boss pattern row is invalid";
				return false;
			}
			pattern.bInvulnerableWhileRunning = 1u == invulnerableFlag;
			pattern.strEncounterId = fields[1];
			pattern.strPatternId = fields[2];
			pattern.strActionId = fields[3];
			auto& patterns = m_BossPatterns[pattern.strEncounterId];
			if (std::any_of(patterns.begin(), patterns.end(),
				[&pattern](const BOSS_PATTERN_DEFINITION& existing)
				{
					return existing.strPatternId == pattern.strPatternId ||
						existing.strActionId == pattern.strActionId;
				}))
			{
				m_strStatus = "Duplicate boss pattern or action ID";
				return false;
			}
			patterns.push_back(std::move(pattern));
		}
		else if (!fields.empty() && "PATTERNAUTHORINGMANAGED" == fields[0])
		{
			if (3u != fields.size() || !IsStableId(fields[1]) ||
				!IsStableId(fields[2]))
			{
				m_strStatus = "Boss managed pattern row is invalid";
				return false;
			}
			const auto owners = m_BossPatterns.find(std::string(fields[1]));
			if (m_BossPatterns.end() == owners)
			{
				m_strStatus = "Boss managed pattern encounter is missing";
				return false;
			}
			const auto owner = std::find_if(owners->second.begin(), owners->second.end(),
				[&fields](const BOSS_PATTERN_DEFINITION& pattern)
				{ return pattern.strPatternId == fields[2]; });
			if (owners->second.end() == owner || owner->bAuthoringMasterManaged)
			{
				m_strStatus = "Boss managed pattern is missing or duplicated";
				return false;
			}
			owner->bAuthoringMasterManaged = true;
		}
		else if (!fields.empty() && "PATTERNPOLICY" == fields[0])
		{
			BOSS_PATTERN_CATEGORY category = BOSS_PATTERN_CATEGORY::NORMAL;
			BOSS_PATTERN_TARGET_POLICY targetPolicy =
				BOSS_PATTERN_TARGET_POLICY::NONE;
			BOSS_PATTERN_AIM_POLICY aimPolicy = BOSS_PATTERN_AIM_POLICY::NONE;
			std::uint32_t minimumPhase = 0;
			std::uint32_t maximumPhase = 0;
			if (8u != fields.size() || !IsStableId(fields[1]) ||
				!IsStableId(fields[2]) ||
				!ParseBossPatternCategory(fields[3], category) ||
				!ParseNumber(fields[4], minimumPhase) ||
				!ParseNumber(fields[5], maximumPhase) ||
				!ParseBossPatternTargetPolicy(fields[6], targetPolicy) ||
				!ParseBossPatternAimPolicy(fields[7], aimPolicy) ||
				0u == minimumPhase || maximumPhase < minimumPhase ||
				maximumPhase > 3u)
			{
				m_strStatus = "Boss pattern policy row is invalid";
				return false;
			}
			const auto ownerMap = m_BossPatterns.find(std::string(fields[1]));
			if (m_BossPatterns.end() == ownerMap)
			{
				m_strStatus = "Boss pattern policy has no encounter";
				return false;
			}
			const auto owner = std::find_if(
				ownerMap->second.begin(), ownerMap->second.end(),
				[&fields](const BOSS_PATTERN_DEFINITION& pattern)
				{ return pattern.strPatternId == fields[2]; });
			const std::string ownerKey =
				std::string(fields[1]) + "\n" + std::string(fields[2]);
			const bool targetNone =
				BOSS_PATTERN_TARGET_POLICY::NONE == targetPolicy;
			const bool aimNeedsNoTarget = BOSS_PATTERN_AIM_POLICY::NONE == aimPolicy ||
				BOSS_PATTERN_AIM_POLICY::FACE_MOTION_ANCHOR == aimPolicy;
			const bool targetTracks =
				BOSS_PATTERN_TARGET_POLICY::NEAREST_EACH_TICK == targetPolicy;
			const bool aimTracks =
				BOSS_PATTERN_AIM_POLICY::TRACK_TARGET_EACH_TICK == aimPolicy;
			const bool targetLocks =
				BOSS_PATTERN_TARGET_POLICY::LOCK_NEAREST_ON_START == targetPolicy ||
				BOSS_PATTERN_TARGET_POLICY::LOCK_RANDOM_ALIVE_ON_START == targetPolicy ||
				BOSS_PATTERN_TARGET_POLICY::LOCK_RANDOM_ALIVE_BEHIND_ON_START ==
					targetPolicy;
			const bool aimLocks =
				BOSS_PATTERN_AIM_POLICY::LOCK_FACING_ON_START == aimPolicy;
			const bool policyIsCoherent =
				(targetNone && aimNeedsNoTarget) ||
				(targetTracks && aimTracks) ||
				(targetLocks && (aimLocks || aimTracks));
			if (ownerMap->second.end() == owner ||
				!patternPolicyOwners.insert(ownerKey).second ||
				!policyIsCoherent)
			{
				m_strStatus =
					"Boss pattern policy has no owner, is duplicated, or is incoherent";
				return false;
			}
			owner->eCategory = category;
			owner->eTargetPolicy = targetPolicy;
			owner->eAimPolicy = aimPolicy;
			owner->iMinimumPhase = minimumPhase;
			owner->iMaximumPhase = maximumPhase;
		}
		else if (!fields.empty() && "PATTERNSOURCE" == fields[0])
		{
			std::uint32_t primaryActionId = 0u;
			std::uint32_t shapeCount = 0u;
			std::uint32_t cooldownMs = 0u;
			std::uint32_t cooldownTicks = 0u;
			std::uint32_t rangeUnits = 0u;
			std::uint32_t approachUnits = 0u;
			std::uint32_t turnDegrees = 0u;
			if (10u != fields.size() || !IsStableId(fields[1]) ||
				!IsStableId(fields[2]) ||
				!ParseNumber(fields[3], primaryActionId) ||
				!ParseNumber(fields[4], shapeCount) ||
				!ParseNumber(fields[5], cooldownMs) ||
				!ParseNumber(fields[6], cooldownTicks) ||
				!ParseNumber(fields[7], rangeUnits) ||
				!ParseNumber(fields[8], approachUnits) ||
				!ParseNumber(fields[9], turnDegrees) ||
				0u == primaryActionId || shapeCount > 256u ||
				cooldownMs > 600000u || rangeUnits > 100000u ||
				approachUnits > 100000u || turnDegrees > 360u ||
				cooldownTicks != static_cast<std::uint32_t>(
					(static_cast<std::uint64_t>(cooldownMs) * 30u + 999u) /
					1000u))
			{
				m_strStatus = "Boss pattern source timing row is invalid";
				return false;
			}
			auto ownerMap = m_BossPatterns.find(std::string(fields[1]));
			if (m_BossPatterns.end() == ownerMap)
			{
				m_strStatus = "Boss pattern source timing has no encounter";
				return false;
			}
			const auto owner = std::find_if(
				ownerMap->second.begin(), ownerMap->second.end(),
				[&fields](const BOSS_PATTERN_DEFINITION& pattern)
				{ return pattern.strPatternId == fields[2]; });
			if (ownerMap->second.end() == owner ||
				0u != owner->iSourcePrimaryActionId)
			{
				m_strStatus =
					"Boss pattern source timing has no owner or is duplicated";
				return false;
			}
			owner->iSourcePrimaryActionId = primaryActionId;
			owner->iSourceShapeCount = shapeCount;
			owner->iSourceCooldownMs = cooldownMs;
			owner->iSourceCooldownTicks = cooldownTicks;
			owner->iSourceRangeUnits = rangeUnits;
			owner->iSourceApproachUnits = approachUnits;
			owner->iSourceTurnDegrees = turnDegrees;
		}
		else if (!fields.empty() && "ENCOUNTERINTRO" == fields[0])
		{
			if (3u != fields.size() || !IsStableId(fields[1]) ||
				!IsStableId(fields[2]))
			{
				m_strStatus = "Encounter intro row is invalid";
				return false;
			}
			const std::string introEncounterId(fields[1]);
			if (!m_IntroPatternIdByEncounter.emplace(
				introEncounterId, std::string(fields[2])).second)
			{
				m_strStatus = "Duplicate encounter intro row";
				return false;
			}
		}
		else if (!fields.empty() && "PATTERNFINALE" == fields[0])
		{
			BOSS_PATTERN_FINALE finale{};
			if (fields.size() < 9u || fields.size() > 72u ||
				!IsStableId(fields[1]) ||
				!IsStableId(fields[2]) || "GHOST_PORTAL_LOOP" != fields[3] ||
				!IsStableId(fields[4]) ||
				!ParseNumber(fields[5], finale.fSpawnHalfExtentsX) ||
				!ParseNumber(fields[6], finale.fSpawnHalfExtentsZ) ||
				!ParseNumber(fields[7], finale.iMaximumActiveGhosts) ||
				!std::isfinite(finale.fSpawnHalfExtentsX) ||
				!std::isfinite(finale.fSpawnHalfExtentsZ) ||
				finale.fSpawnHalfExtentsX < 1.f || finale.fSpawnHalfExtentsX > 100.f ||
				finale.fSpawnHalfExtentsZ < 1.f || finale.fSpawnHalfExtentsZ > 100.f ||
				finale.iMaximumActiveGhosts < 1u ||
				finale.iMaximumActiveGhosts > 64u)
			{
				m_strStatus = "Boss finale row is invalid";
				return false;
			}
			std::unordered_set<std::string> childPatternIds;
			for (std::size_t index = 8u; index < fields.size(); ++index)
			{
				if (!IsStableId(fields[index]) ||
					!childPatternIds.emplace(fields[index]).second)
				{
					m_strStatus = "Boss finale child pattern identity is invalid or duplicated";
					return false;
				}
				finale.GhostPatternIds.emplace_back(fields[index]);
			}
			const auto ownerMap = m_BossPatterns.find(std::string(fields[1]));
			if (m_BossPatterns.end() == ownerMap)
			{
				m_strStatus = "Boss finale has no encounter owner";
				return false;
			}
			const auto owner = std::find_if(ownerMap->second.begin(),
				ownerMap->second.end(), [&fields](const BOSS_PATTERN_DEFINITION& p)
				{ return p.strPatternId == fields[2]; });
			if (ownerMap->second.end() == owner ||
				BOSS_PATTERN_FINALE_KIND::NONE != owner->Finale.eKind)
			{
				m_strStatus = "Boss finale owner is missing or duplicated";
				return false;
			}
			finale.eKind = BOSS_PATTERN_FINALE_KIND::GHOST_PORTAL_LOOP;
			finale.strGhostArchetypeId = std::string(fields[4]);
			owner->Finale = std::move(finale);
		}
		else if (!fields.empty() && "PATTERNMOTION" == fields[0])
		{
			BOSS_PATTERN_MOTION motion{};
			if (14u != fields.size() && 15u != fields.size())
			{
				m_strStatus = "Boss pattern motion row is invalid";
				return false;
			}
			const bool leapsToTarget = "LEAP_TO_TARGET" == fields[3];
			if (!IsStableId(fields[1]) ||
				!IsStableId(fields[2]) ||
				("LEAP_TO_ANCHOR" != fields[3] && !leapsToTarget) ||
				!IsStableId(fields[4]) ||
				!ParseNumber(fields[5], motion.fLandingX) ||
				!ParseNumber(fields[6], motion.fLandingY) ||
				!ParseNumber(fields[7], motion.fLandingZ) ||
				!ParseNumber(fields[8], motion.fApexHeight) ||
				!ParseNumber(fields[9], motion.iTravelStageIndex) ||
				!ParseNumber(fields[10], motion.iTakeoffStartMs) ||
				!ParseNumber(fields[11], motion.iTakeoffEndMs) ||
				!ParseNumber(fields[12], motion.iTravelStartMs) ||
				!ParseNumber(fields[13], motion.iTravelEndMs) ||
				!std::isfinite(motion.fLandingX) ||
				!std::isfinite(motion.fLandingY) ||
				!std::isfinite(motion.fLandingZ) ||
				!std::isfinite(motion.fApexHeight) ||
				motion.fApexHeight <= 0.f || motion.fApexHeight > 200.f ||
				0u == motion.iTravelStageIndex ||
				motion.iTakeoffStartMs >= motion.iTakeoffEndMs ||
				motion.iTravelStartMs >= motion.iTravelEndMs)
			{
				m_strStatus = "Boss pattern motion row is invalid";
				return false;
			}
			if (15u == fields.size())
			{
				std::uint32_t moveToAnchor = 0u;
				if (!ParseNumber(fields[14], moveToAnchor) || moveToAnchor > 1u ||
					(1u == moveToAnchor && (leapsToTarget ||
						0u == motion.iTakeoffStartMs)))
				{
					m_strStatus = "Boss pre-takeoff movement is invalid";
					return false;
				}
				motion.bMoveToAnchorBeforeTakeoff = 1u == moveToAnchor;
			}
			motion.eKind = leapsToTarget ?
				BOSS_PATTERN_MOTION_KIND::LEAP_TO_TARGET :
				BOSS_PATTERN_MOTION_KIND::LEAP_TO_ANCHOR;
			motion.strAnchorId = std::string(fields[4]);
			const std::string motionEncounterId(fields[1]);
			auto& motionPatterns = m_BossPatterns[motionEncounterId];
			const auto owner = std::find_if(
				motionPatterns.begin(), motionPatterns.end(),
				[&fields](const BOSS_PATTERN_DEFINITION& candidate)
				{
					return candidate.strPatternId == fields[2];
				});
			if (owner == motionPatterns.end() ||
				BOSS_PATTERN_MOTION_KIND::NONE != owner->Motion.eKind)
			{
				m_strStatus =
					"Boss pattern motion has no owner or is duplicated";
				return false;
			}
			owner->Motion = std::move(motion);
		}
		else if (!fields.empty() && "PATTERNSTAGE" == fields[0])
		{
			BOSS_PATTERN_STAGE_DEFINITION stage{};
			std::uint32_t stageIndex = 0u;
			std::uint32_t knockdownFlag = 0u;
			const bool hasPlayerResponseFields = 24u == fields.size();
			if ((22u != fields.size() && !hasPlayerResponseFields) ||
				!IsStableId(fields[1]) ||
				!IsStableId(fields[2]) ||
				!ParseNumber(fields[3], stageIndex) ||
				!IsStableId(fields[4]) || !IsStableId(fields[5]) ||
				!ParseBossPatternStageKind(fields[6], stage.eStageKind) ||
				!ParseNumber(fields[7], stage.iDurationMs) ||
				!ParseBossPatternHitShape(fields[8], stage.eHitShape) ||
				!ParseNumber(fields[9], stage.fHitOuterRadius) ||
				!ParseNumber(fields[10], stage.fHitInnerRadius) ||
				!ParseNumber(fields[11], stage.fHitAngleDegrees) ||
				!ParseNumber(fields[12], stage.fHitLength) ||
				!ParseNumber(fields[13], stage.fHitHalfWidth) ||
				!ParseNumber(fields[14], stage.iHitCount) ||
				!ParseNumber(fields[15], stage.iHitIntervalMs) ||
				!ParseNumber(fields[16], stage.iHitDelayMs) ||
				("-" != fields[17] && !IsStableId(fields[17])) ||
				!ParseNumber(fields[18], stage.fPushRangeM) ||
				!ParseNumber(fields[19], stage.iPushMs) ||
				!ParseNumber(fields[20], knockdownFlag) ||
				!ParseNumber(fields[21], stage.iDownMs) ||
				(hasPlayerResponseFields &&
				 (!ParseBossPatternPlayerResponse(
					fields[22], stage.ePlayerResponse) ||
				  !ParsePlayerAttachmentSlot(fields[23], stage.eAttachmentSlot))) ||
				0u == stage.iDurationMs ||
				(BOSS_PATTERN_HIT_SHAPE::NONE == stage.eHitShape &&
					0u != stage.iHitDelayMs) ||
				stage.iHitDelayMs >= stage.iDurationMs ||
				!std::isfinite(stage.fHitOuterRadius) ||
				!std::isfinite(stage.fHitInnerRadius) ||
				!std::isfinite(stage.fHitAngleDegrees) ||
				!std::isfinite(stage.fHitLength) ||
				!std::isfinite(stage.fHitHalfWidth) ||
				!std::isfinite(stage.fPushRangeM) ||
				std::fabs(stage.fPushRangeM) > 20.f ||
				knockdownFlag > 1u ||
				(0.f != stage.fPushRangeM && 0u == stage.iPushMs) ||
				(0.f == stage.fPushRangeM && 0u != stage.iPushMs) ||
				(1u == knockdownFlag && 0u == stage.iDownMs) ||
				(0u == knockdownFlag && 0u != stage.iDownMs) ||
				(BOSS_PATTERN_PLAYER_RESPONSE::DAMAGE == stage.ePlayerResponse &&
				 LostArk::Shared::PLAYER_ATTACHMENT_SLOT::NONE !=
					stage.eAttachmentSlot) ||
				(BOSS_PATTERN_PLAYER_RESPONSE::CAPTURE == stage.ePlayerResponse &&
				 (LostArk::Shared::PLAYER_ATTACHMENT_SLOT::BOSS_LEFT_HAND !=
					stage.eAttachmentSlot ||
				  BOSS_PATTERN_HIT_SHAPE::NONE == stage.eHitShape ||
				  0.f != stage.fPushRangeM || 0u != stage.iPushMs ||
				  0u != knockdownFlag || 0u != stage.iDownMs)))
			{
				m_strStatus = "Boss pattern stage row is invalid";
				return false;
			}
			stage.bKnockdown = 1u == knockdownFlag;
			const auto ownerMap = m_BossPatterns.find(std::string(fields[1]));
			if (m_BossPatterns.end() == ownerMap)
			{
				m_strStatus = "Boss pattern stage has no encounter";
				return false;
			}
			const auto owner = std::find_if(
				ownerMap->second.begin(), ownerMap->second.end(),
				[&fields](const BOSS_PATTERN_DEFINITION& pattern)
				{ return pattern.strPatternId == fields[2]; });
			if (ownerMap->second.end() == owner ||
				stageIndex != owner->Stages.size() ||
				stageIndex >= owner->iExpectedStageCount)
			{
				m_strStatus = "Boss pattern stage does not follow its pattern";
				return false;
			}
			stage.strStageId = fields[4];
			stage.strActionId = fields[5];
			stage.strDamageProfileId =
				"-" == fields[17] ? "" : std::string(fields[17]);
			owner->Stages.push_back(std::move(stage));
		}
		else if (!fields.empty() && "PATTERNSTAGEHITOFFSET" == fields[0])
		{
			std::uint32_t hitIndex = 0u;
			std::uint32_t hitOffsetMs = 0u;
			if (6u != fields.size() || !IsStableId(fields[1]) ||
				!IsStableId(fields[2]) || !IsStableId(fields[3]) ||
				!ParseNumber(fields[4], hitIndex) ||
				!ParseNumber(fields[5], hitOffsetMs))
			{
				m_strStatus = "Boss pattern stage hit offset row is invalid";
				return false;
			}
			const auto ownerMap = m_BossPatterns.find(std::string(fields[1]));
			if (m_BossPatterns.end() == ownerMap)
			{
				m_strStatus = "Boss pattern stage hit offset has no encounter";
				return false;
			}
			const auto owner = std::find_if(
				ownerMap->second.begin(), ownerMap->second.end(),
				[&fields](const BOSS_PATTERN_DEFINITION& pattern)
				{ return pattern.strPatternId == fields[2]; });
			if (ownerMap->second.end() == owner)
			{
				m_strStatus = "Boss pattern stage hit offset has no pattern owner";
				return false;
			}
			const auto stage = std::find_if(
				owner->Stages.begin(), owner->Stages.end(),
				[&fields](const BOSS_PATTERN_STAGE_DEFINITION& candidate)
				{ return candidate.strActionId == fields[3]; });
			if (owner->Stages.end() == stage ||
				hitIndex != stage->HitOffsetsMs.size() ||
				hitIndex >= stage->iHitCount ||
				hitOffsetMs >= stage->iDurationMs ||
				(!stage->HitOffsetsMs.empty() &&
				 hitOffsetMs <= stage->HitOffsetsMs.back()))
			{
				m_strStatus =
					"Boss pattern stage hit offsets are not ordered or owned";
				return false;
			}
			stage->HitOffsetsMs.push_back(hitOffsetMs);
		}
		else if (!fields.empty() && "PATTERNSTAGEHITAUTHORITY" == fields[0])
		{
			BOSS_PATTERN_HIT_ANCHOR_KIND anchorKind =
				BOSS_PATTERN_HIT_ANCHOR_KIND::BOSS_CURRENT;
			BOSS_PATTERN_HIT_ACTIVATION_KIND activationKind =
				BOSS_PATTERN_HIT_ACTIVATION_KIND::PULSE_SCHEDULE;
			float forwardOffsetM = 0.f;
			float rightOffsetM = 0.f;
			float yawOffsetDegrees = 0.f;
			std::uint32_t startMs = 0u;
			std::uint32_t lifetimeMs = 0u;
			if (12u != fields.size() || !IsStableId(fields[1]) ||
				!IsStableId(fields[2]) || !IsStableId(fields[3]) ||
				!ParseBossPatternHitAnchorKind(fields[4], anchorKind) ||
				!ParseNumber(fields[5], forwardOffsetM) ||
				!ParseNumber(fields[6], rightOffsetM) ||
				!ParseNumber(fields[7], yawOffsetDegrees) ||
				!ParseBossPatternHitActivationKind(fields[8], activationKind) ||
				!ParseNumber(fields[9], startMs) ||
				!ParseNumber(fields[10], lifetimeMs) ||
				((BOSS_PATTERN_HIT_ACTIVATION_KIND::ACTIVE_WINDOW == activationKind &&
				  "ONCE" != fields[11]) ||
				 (BOSS_PATTERN_HIT_ACTIVATION_KIND::PULSE_SCHEDULE == activationKind &&
				  "NONE" != fields[11])) ||
				!std::isfinite(forwardOffsetM) ||
				!std::isfinite(rightOffsetM) ||
				!std::isfinite(yawOffsetDegrees) ||
				std::fabs(forwardOffsetM) > 1000.f ||
				std::fabs(rightOffsetM) > 1000.f ||
				std::fabs(yawOffsetDegrees) > 360.f)
			{
				m_strStatus = "Boss pattern stage hit authority row is invalid";
				return false;
			}
			const auto ownerMap = m_BossPatterns.find(std::string(fields[1]));
			if (m_BossPatterns.end() == ownerMap)
			{
				m_strStatus = "Boss pattern stage hit authority has no encounter";
				return false;
			}
			const auto owner = std::find_if(
				ownerMap->second.begin(), ownerMap->second.end(),
				[&fields](const BOSS_PATTERN_DEFINITION& pattern)
				{ return pattern.strPatternId == fields[2]; });
			if (ownerMap->second.end() == owner)
			{
				m_strStatus = "Boss pattern stage hit authority has no pattern owner";
				return false;
			}
			const auto stage = std::find_if(
				owner->Stages.begin(), owner->Stages.end(),
				[&fields](const BOSS_PATTERN_STAGE_DEFINITION& candidate)
				{ return candidate.strActionId == fields[3]; });
			if (owner->Stages.end() == stage ||
				BOSS_PATTERN_HIT_SHAPE::NONE == stage->eHitShape ||
				BOSS_PATTERN_HIT_ANCHOR_KIND::BOSS_CURRENT != stage->eHitAnchorKind ||
				0.f != stage->fHitAnchorForwardOffsetM ||
				0.f != stage->fHitAnchorRightOffsetM ||
				0.f != stage->fHitAnchorYawOffsetDegrees ||
				BOSS_PATTERN_HIT_ACTIVATION_KIND::PULSE_SCHEDULE !=
					stage->eHitActivationKind ||
				0u != stage->iHitActivationStartMs ||
				0u != stage->iHitActivationLifetimeMs ||
				(BOSS_PATTERN_HIT_ACTIVATION_KIND::ACTIVE_WINDOW == activationKind &&
				 (0u == lifetimeMs ||
				  static_cast<std::uint64_t>(startMs) + lifetimeMs >
					stage->iDurationMs)))
			{
				m_strStatus =
					"Boss pattern stage hit authority is duplicated or not owned";
				return false;
			}
			stage->eHitAnchorKind = anchorKind;
			stage->fHitAnchorForwardOffsetM = forwardOffsetM;
			stage->fHitAnchorRightOffsetM = rightOffsetM;
			stage->fHitAnchorYawOffsetDegrees = yawOffsetDegrees;
			stage->eHitActivationKind = activationKind;
			stage->iHitActivationStartMs = startMs;
			stage->iHitActivationLifetimeMs = lifetimeMs;
		}
		else if (!fields.empty() && "PATTERNSTAGEPARTDAMAGE" == fields[0])
		{
			BOSS_PATTERN_PART_DAMAGE_POLICY policy =
				BOSS_PATTERN_PART_DAMAGE_POLICY::NORMAL;
			if (5u != fields.size() || !IsStableId(fields[1]) ||
				!IsStableId(fields[2]) || !IsStableId(fields[3]) ||
				!ParseBossPatternPartDamagePolicy(fields[4], policy))
			{
				m_strStatus =
					"Boss pattern stage part-damage row is invalid";
				return false;
			}
			const std::string ownerKey = std::string(fields[1]) + "\n" +
				std::string(fields[2]) + "\n" + std::string(fields[3]);
			const auto ownerMap = m_BossPatterns.find(std::string(fields[1]));
			if (m_BossPatterns.end() == ownerMap ||
				!patternStagePartDamageOwners.insert(ownerKey).second)
			{
				m_strStatus =
					"Boss pattern stage part-damage owner is missing or duplicated";
				return false;
			}
			const auto owner = std::find_if(
				ownerMap->second.begin(), ownerMap->second.end(),
				[&fields](const BOSS_PATTERN_DEFINITION& pattern)
				{ return pattern.strPatternId == fields[2]; });
			if (ownerMap->second.end() == owner)
			{
				m_strStatus =
					"Boss pattern stage part-damage has no pattern owner";
				return false;
			}
			const auto stage = std::find_if(
				owner->Stages.begin(), owner->Stages.end(),
				[&fields](const BOSS_PATTERN_STAGE_DEFINITION& candidate)
				{ return candidate.strActionId == fields[3]; });
			if (owner->Stages.end() == stage)
			{
				m_strStatus =
					"Boss pattern stage part-damage has no stage owner";
				return false;
			}
			stage->ePartDamagePolicy = policy;
		}
		else if (!fields.empty() && "PATTERNSTAGECOUNTERPROXY" == fields[0])
		{
			float forwardOffsetM = 0.f;
			float rightOffsetM = 0.f;
			float radiusM = 0.f;
			if (7u != fields.size() || !IsStableId(fields[1]) ||
				!IsStableId(fields[2]) || !IsStableId(fields[3]) ||
				!ParseNumber(fields[4], forwardOffsetM) ||
				!ParseNumber(fields[5], rightOffsetM) ||
				!ParseNumber(fields[6], radiusM) ||
				!std::isfinite(forwardOffsetM) ||
				!std::isfinite(rightOffsetM) || !std::isfinite(radiusM) ||
				std::fabs(forwardOffsetM) > 20.f ||
				std::fabs(rightOffsetM) > 20.f ||
				radiusM <= 0.f || radiusM > 20.f)
			{
				m_strStatus = "Boss pattern stage counter-proxy row is invalid";
				return false;
			}
			const std::string ownerKey = std::string(fields[1]) + "\n" +
				std::string(fields[2]) + "\n" + std::string(fields[3]);
			const auto ownerMap = m_BossPatterns.find(std::string(fields[1]));
			if (m_BossPatterns.end() == ownerMap ||
				!patternStageCounterProxyOwners.insert(ownerKey).second)
			{
				m_strStatus =
					"Boss pattern stage counter-proxy owner is missing or duplicated";
				return false;
			}
			const auto owner = std::find_if(
				ownerMap->second.begin(), ownerMap->second.end(),
				[&fields](const BOSS_PATTERN_DEFINITION& pattern)
				{ return pattern.strPatternId == fields[2]; });
			if (ownerMap->second.end() == owner)
			{
				m_strStatus =
					"Boss pattern stage counter-proxy has no pattern owner";
				return false;
			}
			const auto stage = std::find_if(
				owner->Stages.begin(), owner->Stages.end(),
				[&fields](const BOSS_PATTERN_STAGE_DEFINITION& candidate)
				{ return candidate.strActionId == fields[3]; });
			if (owner->Stages.end() == stage || stage->bHasCounterProxy)
			{
				m_strStatus =
					"Boss pattern stage counter-proxy has no stage owner or is duplicated";
				return false;
			}
			stage->bHasCounterProxy = true;
			stage->fCounterProxyForwardOffsetM = forwardOffsetM;
			stage->fCounterProxyRightOffsetM = rightOffsetM;
			stage->fCounterProxyRadiusM = radiusM;
		}
		/* Sorted after PATTERN and ahead of its own steps, so the span opens an
		empty list the step rows then fill in order. */
		else if (!fields.empty() && "PATTERNROTATION" == fields[0])
		{
			std::uint32_t fromBar = 0u;
			std::uint32_t toBar = 0u;
			std::uint32_t stepCount = 0u;
			BOSS_PATTERN_ROTATION_SELECTION_MODE selectionMode =
				BOSS_PATTERN_ROTATION_SELECTION_MODE::
					ORDERED_INTRO_THEN_WEIGHTED;
			if (7u != fields.size() || !IsStableId(fields[1]) ||
				!IsStableId(fields[2]) || !ParseNumber(fields[3], fromBar) ||
				!ParseNumber(fields[4], toBar) ||
				!ParseBossPatternRotationSelectionMode(
					fields[5], selectionMode) ||
				!ParseNumber(fields[6], stepCount) ||
				0u == stepCount || stepCount > 32u || fromBar <= toBar)
			{
				m_strStatus = "Boss pattern rotation row is invalid";
				return false;
			}
			std::vector<BOSS_PATTERN_ROTATION_DEFINITION>& rotations =
				m_BossPatternRotations[std::string(fields[1])];
			for (const BOSS_PATTERN_ROTATION_DEFINITION& existing : rotations)
			{
				if (existing.strRotationId == fields[2])
				{
					m_strStatus = "Boss pattern rotation ID is duplicated";
					return false;
				}
			}
			BOSS_PATTERN_ROTATION_DEFINITION staged{};
			staged.strEncounterId = fields[1];
			staged.strRotationId = fields[2];
			staged.eSelectionMode = selectionMode;
			staged.iFromHealthBar = fromBar;
			staged.iToHealthBar = toBar;
			staged.iExpectedStepCount = stepCount;
			rotations.push_back(std::move(staged));
		}
		/* Managed v20 weighted rows own their weight and enabled state. They are
		strictly ordinal, unique, and may only attach to WEIGHTED_POOL. */
		else if (!fields.empty() &&
			"PATTERNROTATIONCANDIDATE" == fields[0])
		{
			std::uint32_t candidateIndex = 0u;
			std::uint32_t selectionWeight = 0u;
			std::uint32_t enabled = 0u;
			if (7u != fields.size() || !IsStableId(fields[1]) ||
				!IsStableId(fields[2]) ||
				!ParseNumber(fields[3], candidateIndex) ||
				!IsStableId(fields[4]) ||
				!ParseNumber(fields[5], selectionWeight) ||
				0u == selectionWeight || selectionWeight > 100000u ||
				!ParseNumber(fields[6], enabled) || enabled > 1u)
			{
				m_strStatus = "Boss pattern rotation candidate row is invalid";
				return false;
			}
			const auto patternMap = m_BossPatterns.find(std::string(fields[1]));
			if (m_BossPatterns.end() == patternMap ||
				patternMap->second.end() == std::find_if(
					patternMap->second.begin(), patternMap->second.end(),
					[&fields](const BOSS_PATTERN_DEFINITION& pattern)
					{
						return pattern.strPatternId == fields[4] &&
							BOSS_PATTERN_SELECTION::NORMAL == pattern.eSelection;
					}))
			{
				m_strStatus =
					"Boss pattern rotation candidate names no normal pattern";
				return false;
			}
			const auto rotationMap =
				m_BossPatternRotations.find(std::string(fields[1]));
			if (m_BossPatternRotations.end() == rotationMap)
			{
				m_strStatus = "Boss pattern rotation candidate has no span";
				return false;
			}
			const auto rotation = std::find_if(
				rotationMap->second.begin(), rotationMap->second.end(),
				[&fields](const BOSS_PATTERN_ROTATION_DEFINITION& candidate)
				{ return candidate.strRotationId == fields[2]; });
			if (rotationMap->second.end() == rotation ||
				BOSS_PATTERN_ROTATION_SELECTION_MODE::WEIGHTED_POOL !=
					rotation->eSelectionMode ||
				candidateIndex != rotation->Candidates.size() ||
				candidateIndex >= rotation->iExpectedStepCount ||
				rotation->Candidates.end() != std::find_if(
					rotation->Candidates.begin(), rotation->Candidates.end(),
					[&fields](const BOSS_PATTERN_ROTATION_CANDIDATE& candidate)
					{ return candidate.strPatternId == fields[4]; }))
			{
				m_strStatus =
					"Boss pattern rotation candidates violate the managed tagged union";
				return false;
			}
			BOSS_PATTERN_ROTATION_CANDIDATE candidate{};
			candidate.strPatternId = fields[4];
			candidate.iSelectionWeight = selectionWeight;
			candidate.bEnabled = 0u != enabled;
			rotation->Candidates.push_back(std::move(candidate));
		}
		/* Sorted after its own span row, so the list it appends to exists. Every
		pattern a step names is already parsed as well. */
		else if (!fields.empty() && "PATTERNROTATIONSTEP" == fields[0])
		{
			std::uint32_t stepIndex = 0u;
			if (5u != fields.size() || !IsStableId(fields[1]) ||
				!IsStableId(fields[2]) || !ParseNumber(fields[3], stepIndex) ||
				!IsStableId(fields[4]))
			{
				m_strStatus = "Boss pattern rotation step row is invalid";
				return false;
			}
			const auto patternMap = m_BossPatterns.find(std::string(fields[1]));
			if (m_BossPatterns.end() == patternMap ||
				patternMap->second.end() == std::find_if(
					patternMap->second.begin(), patternMap->second.end(),
					[&fields](const BOSS_PATTERN_DEFINITION& pattern)
					{
						return pattern.strPatternId == fields[4] &&
							BOSS_PATTERN_SELECTION::NORMAL == pattern.eSelection;
					}))
			{
				m_strStatus = "Boss pattern rotation step names no normal pattern";
				return false;
			}
			const auto rotationMap =
				m_BossPatternRotations.find(std::string(fields[1]));
			if (m_BossPatternRotations.end() == rotationMap)
			{
				m_strStatus = "Boss pattern rotation step has no span";
				return false;
			}
			const auto rotation = std::find_if(
				rotationMap->second.begin(), rotationMap->second.end(),
				[&fields](const BOSS_PATTERN_ROTATION_DEFINITION& candidate)
				{ return candidate.strRotationId == fields[2]; });
			if (rotationMap->second.end() == rotation ||
				BOSS_PATTERN_ROTATION_SELECTION_MODE::
					ORDERED_INTRO_THEN_WEIGHTED != rotation->eSelectionMode ||
				stepIndex != rotation->PatternIds.size() ||
				stepIndex >= rotation->iExpectedStepCount)
			{
				m_strStatus =
					"Boss pattern rotation steps violate the legacy tagged union";
				return false;
			}
			rotation->PatternIds.emplace_back(fields[4]);
		}
		else if (!fields.empty() && "PATTERNROTATIONWINDOW" == fields[0])
		{
			std::uint32_t gameplayPhase = 0u;
			std::uint32_t fromBar = 0u;
			std::uint32_t toBar = 0u;
			std::uint32_t candidateCount = 0u;
			if (9u != fields.size() || !IsStableId(fields[1]) ||
				!IsStableId(fields[2]) || !IsStableId(fields[3]) ||
				!ParseNumber(fields[4], gameplayPhase) ||
				0u == gameplayPhase || gameplayPhase > 3u ||
				!IsStableId(fields[5]) ||
				!ParseNumber(fields[6], fromBar) ||
				!ParseNumber(fields[7], toBar) || fromBar <= toBar ||
				!ParseNumber(fields[8], candidateCount) ||
				0u == candidateCount || candidateCount > 32u)
			{
				m_strStatus = "Boss pattern rotation window row is invalid";
				return false;
			}
			const auto rotationMap =
				m_BossPatternRotations.find(std::string(fields[1]));
			if (m_BossPatternRotations.end() == rotationMap)
			{
				m_strStatus = "Boss pattern rotation window has no span";
				return false;
			}
			const auto rotation = std::find_if(
				rotationMap->second.begin(), rotationMap->second.end(),
				[&fields](const BOSS_PATTERN_ROTATION_DEFINITION& candidate)
				{ return candidate.strRotationId == fields[2]; });
			if (rotationMap->second.end() == rotation ||
				BOSS_PATTERN_ROTATION_SELECTION_MODE::WEIGHTED_POOL !=
					rotation->eSelectionMode || rotation->Window.Is_Defined() ||
				fromBar != rotation->iFromHealthBar ||
				toBar != rotation->iToHealthBar ||
				candidateCount != rotation->iExpectedStepCount)
			{
				m_strStatus =
					"Boss pattern rotation window mismatches its managed span";
				return false;
			}
			for (const BOSS_PATTERN_ROTATION_DEFINITION& existing :
				rotationMap->second)
			{
				if (!existing.Window.Is_Defined()) continue;
				if (existing.Window.strWindowId == fields[3] ||
					existing.Window.strSelectionSetId == fields[5])
				{
					m_strStatus =
						"Boss pattern rotation managed window identity is duplicated";
					return false;
				}
			}
			rotation->Window.strWindowId = fields[3];
			rotation->Window.iGameplayPhase = gameplayPhase;
			rotation->Window.strSelectionSetId = fields[5];
			rotation->Window.iFromHealthBar = fromBar;
			rotation->Window.iToHealthBar = toBar;
			rotation->Window.iExpectedCandidateCount = candidateCount;
		}
		else if (!fields.empty() && "PATTERNSEQUENCE" == fields[0])
		{
			std::uint32_t interStepPursuitMs = 0u;
			std::uint32_t stepCount = 0u;
			if (6u != fields.size() || !IsStableId(fields[1]) ||
				!IsStableId(fields[2]) ||
				"ORDERED_ONCE_THEN_IDLE" != fields[3] ||
				!ParseNumber(fields[4], interStepPursuitMs) ||
				interStepPursuitMs < 100u || interStepPursuitMs > 10000u ||
				!ParseNumber(fields[5], stepCount) ||
				0u == stepCount ||
				stepCount > LostArk::Shared::MAX_VALTAN_PATTERN_FLOW_SLOTS ||
				m_BossPatternSequences.contains(std::string(fields[1])))
			{
				m_strStatus = "Boss pattern sequence row is invalid or duplicated";
				return false;
			}
			BOSS_PATTERN_SEQUENCE_DEFINITION sequence{};
			sequence.strEncounterId = fields[1];
			sequence.strSequenceId = fields[2];
			sequence.eMode =
				BOSS_PATTERN_SEQUENCE_MODE::ORDERED_ONCE_THEN_IDLE;
			sequence.iInterStepPursuitMs = interStepPursuitMs;
			sequence.iInterStepPursuitTicks = static_cast<std::uint32_t>(
				(static_cast<std::uint64_t>(interStepPursuitMs) * 30u + 999u) /
				1000u);
			sequence.iExpectedStepCount = stepCount;
			m_BossPatternSequences.emplace(
				sequence.strEncounterId, std::move(sequence));
		}
		else if (!fields.empty() && "PATTERNSEQUENCESTEP" == fields[0])
		{
			std::uint32_t stepIndex = 0u;
			if (5u != fields.size() || !IsStableId(fields[1]) ||
				!IsStableId(fields[2]) ||
				!ParseNumber(fields[3], stepIndex) || !IsStableId(fields[4]))
			{
				m_strStatus = "Boss pattern sequence step row is invalid";
				return false;
			}
			const auto sequence =
				m_BossPatternSequences.find(std::string(fields[1]));
			const auto patternMap = m_BossPatterns.find(std::string(fields[1]));
			const bool hasPattern = patternMap != m_BossPatterns.end() &&
				patternMap->second.end() != std::find_if(
					patternMap->second.begin(), patternMap->second.end(),
					[&fields](const BOSS_PATTERN_DEFINITION& pattern)
					{
						return pattern.strPatternId == fields[4];
					});
			if (m_BossPatternSequences.end() == sequence ||
				sequence->second.strSequenceId != fields[2] ||
				stepIndex != sequence->second.PatternIds.size() ||
				stepIndex >= sequence->second.iExpectedStepCount ||
				!hasPattern)
			{
				m_strStatus =
					"Boss pattern sequence steps violate their ordered contract";
				return false;
			}
			sequence->second.PatternIds.emplace_back(fields[4]);
		}
		else if (!fields.empty() && "PATTERNSTAGEACTION" == fields[0])
		{
			std::uint32_t actionOrder = 0;
			BOSS_PATTERN_STAGE_ACTION action{};
			const bool hasReleaseFields = 12u == fields.size() ||
				13u == fields.size();
			if ((10u != fields.size() && !hasReleaseFields) ||
				!IsStableId(fields[1]) ||
				!IsStableId(fields[2]) || !IsStableId(fields[3]) ||
				!ParseNumber(fields[4], actionOrder) ||
				!ParseBossPatternStageActionTrigger(fields[5], action.eTrigger) ||
				!ParseBossPatternStageActionKind(fields[6], action.eKind) ||
				!IsStableId(fields[7]) ||
				!ParseNumber(fields[8], action.iValue) ||
				!ParseNumber(fields[9], action.iDurationMs) ||
				(hasReleaseFields &&
				 (!ParseBossGrabbedReleaseMode(
					fields[10], action.eReleaseMode) ||
				  !ParseNumber(fields[11], action.fReleaseSpeedMps) ||
				  (13u == fields.size() &&
				   !ParseNumber(fields[12], action.fReleaseYawOffsetDegrees)))) ||
				!IsValidBossPatternStageAction(
					action.eTrigger, action.eKind, fields[7],
					action.iValue, action.iDurationMs,
					action.eReleaseMode, action.fReleaseSpeedMps,
					action.fReleaseYawOffsetDegrees))
			{
				m_strStatus = "Boss pattern stage action row is invalid";
				return false;
			}
			const auto ownerMap = m_BossPatterns.find(std::string(fields[1]));
			if (m_BossPatterns.end() == ownerMap)
			{
				m_strStatus = "Boss pattern stage action has no encounter";
				return false;
			}
			const auto owner = std::find_if(
				ownerMap->second.begin(), ownerMap->second.end(),
				[&fields](const BOSS_PATTERN_DEFINITION& pattern)
				{ return pattern.strPatternId == fields[2]; });
			if (ownerMap->second.end() == owner)
			{
				m_strStatus = "Boss pattern stage action has no pattern owner";
				return false;
			}
			const auto stage = std::find_if(
				owner->Stages.begin(), owner->Stages.end(),
				[&fields](const BOSS_PATTERN_STAGE_DEFINITION& candidate)
				{ return candidate.strActionId == fields[3]; });
			if (owner->Stages.end() == stage ||
				actionOrder != stage->Actions.size() || actionOrder >= 8u ||
				std::any_of(stage->Actions.begin(), stage->Actions.end(),
					[&action, &fields](const BOSS_PATTERN_STAGE_ACTION& existing)
					{
						return existing.eTrigger == action.eTrigger &&
							existing.eKind == action.eKind &&
							existing.strTargetId == fields[7];
					}))
			{
				m_strStatus =
					"Boss pattern stage action has no stage owner or is duplicated";
				return false;
			}
			action.strTargetId = fields[7];
			stage->Actions.push_back(std::move(action));
		}
		else if (!fields.empty() && "PATTERNSTAGEVOLLEY" == fields[0])
		{
			std::uint32_t actionOrder = 0u;
			std::uint32_t allowOverlap = 0u;
			BOSS_PATTERN_STAGE_ACTION action{};
			action.eKind =
				BOSS_PATTERN_STAGE_ACTION_KIND::SPAWN_COMBAT_OBJECT_VOLLEY;
			if (21u != fields.size() || !IsStableId(fields[1]) ||
				!IsStableId(fields[2]) || !IsStableId(fields[3]) ||
				!ParseNumber(fields[4], actionOrder) ||
				!ParseBossPatternStageActionTrigger(fields[5], action.eTrigger) ||
				BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER != action.eTrigger ||
				!IsStableId(fields[6]) ||
				("PER_ALIVE_PLAYER" != fields[7] &&
				 "BOSS_RELATIVE" != fields[7]) ||
				!ParseNumber(fields[8],
					action.Volley.iCountPerResolvedTarget) ||
				!ParseNumber(fields[10], action.Volley.fRadiusM) ||
				!ParseNumber(fields[11], action.Volley.fStartAngleDegrees) ||
				!ParseNumber(fields[12], action.Volley.fAngleStepDegrees) ||
				!ParseNumber(fields[13], allowOverlap) || allowOverlap > 1u ||
				!ParseNumber(fields[14], action.Volley.iMaximumTotalObjects) ||
				!ParseNumber(fields[15], action.Volley.iSpawnCount) ||
				!ParseNumber(fields[16], action.Volley.iSpawnIntervalMs) ||
				!ParseNumber(fields[17], action.Volley.iArenaRandomCount) ||
				!ParseNumber(fields[18], action.Volley.fArenaRandomRadiusM) ||
				!ParseNumber(fields[19], action.Volley.fArenaHeightToleranceM) ||
				action.Volley.iCountPerResolvedTarget < 1u ||
				action.Volley.iCountPerResolvedTarget > 8u ||
				static_cast<std::uint64_t>(
					action.Volley.iMaximumTotalObjects) <
					static_cast<std::uint64_t>(
						action.Volley.iCountPerResolvedTarget) +
					static_cast<std::uint64_t>(
						action.Volley.iArenaRandomCount) ||
				action.Volley.iMaximumTotalObjects > 64u ||
				action.Volley.iSpawnCount < 1u ||
				action.Volley.iSpawnCount > 8u ||
				(1u == action.Volley.iSpawnCount ?
					0u != action.Volley.iSpawnIntervalMs :
					0u == action.Volley.iSpawnIntervalMs) ||
				action.Volley.iArenaRandomCount > 32u ||
				!std::isfinite(action.Volley.fRadiusM) ||
				action.Volley.fRadiusM < 0.f ||
				!std::isfinite(action.Volley.fStartAngleDegrees) ||
				!std::isfinite(action.Volley.fAngleStepDegrees) ||
				!std::isfinite(action.Volley.fArenaRandomRadiusM) ||
				!std::isfinite(action.Volley.fArenaHeightToleranceM) ||
				action.Volley.fArenaRandomRadiusM < 0.f ||
				action.Volley.fArenaHeightToleranceM < 0.f)
			{
				m_strStatus = "Boss pattern stage volley row is invalid";
				return false;
			}
			action.Volley.ePolicy = "PER_ALIVE_PLAYER" == fields[7] ?
				BOSS_COMBAT_OBJECT_VOLLEY_POLICY::PER_ALIVE_PLAYER :
				BOSS_COMBAT_OBJECT_VOLLEY_POLICY::BOSS_RELATIVE;
			action.Volley.bAllowOverlap = 0u != allowOverlap;
			if ("SINGLE" == fields[9])
				action.Volley.eLayout =
					BOSS_COMBAT_OBJECT_LAYOUT_KIND::SINGLE;
			else if ("RADIAL" == fields[9])
				action.Volley.eLayout =
					BOSS_COMBAT_OBJECT_LAYOUT_KIND::RADIAL;
			else
			{
				m_strStatus = "Boss pattern stage volley layout kind is invalid";
				return false;
			}
			if ("NONE" == fields[20])
				action.Volley.eArenaAnchorPolicy =
					BOSS_COMBAT_OBJECT_ARENA_ANCHOR_POLICY::NONE;
			else if ("BOSS_SPAWN_POSITION" == fields[20])
				action.Volley.eArenaAnchorPolicy =
					BOSS_COMBAT_OBJECT_ARENA_ANCHOR_POLICY::BOSS_SPAWN_POSITION;
			else
			{
				m_strStatus = "Boss pattern stage volley arena anchor is invalid";
				return false;
			}
			const bool isMulti =
				action.Volley.iCountPerResolvedTarget > 1u;
			const bool perAlivePlayer =
				BOSS_COMBAT_OBJECT_VOLLEY_POLICY::PER_ALIVE_PLAYER ==
					action.Volley.ePolicy;
			const bool bossRelative =
				BOSS_COMBAT_OBJECT_VOLLEY_POLICY::BOSS_RELATIVE ==
					action.Volley.ePolicy;
			if ((isMulti &&
				(BOSS_COMBAT_OBJECT_LAYOUT_KIND::RADIAL !=
					action.Volley.eLayout || action.Volley.fRadiusM <= 0.f ||
				 action.Volley.fAngleStepDegrees == 0.f ||
				 action.Volley.bAllowOverlap)) ||
				(!isMulti &&
					(BOSS_COMBAT_OBJECT_LAYOUT_KIND::SINGLE !=
						action.Volley.eLayout ||
					 action.Volley.fRadiusM != 0.f ||
					 action.Volley.fStartAngleDegrees != 0.f ||
					 action.Volley.fAngleStepDegrees != 0.f ||
					 action.Volley.bAllowOverlap)) ||
				(0u == action.Volley.iArenaRandomCount ?
					(BOSS_COMBAT_OBJECT_ARENA_ANCHOR_POLICY::NONE !=
						action.Volley.eArenaAnchorPolicy ||
					 action.Volley.fArenaRandomRadiusM != 0.f ||
					 action.Volley.fArenaHeightToleranceM != 0.f) :
					(BOSS_COMBAT_OBJECT_ARENA_ANCHOR_POLICY::
						BOSS_SPAWN_POSITION !=
						action.Volley.eArenaAnchorPolicy ||
					 action.Volley.fArenaRandomRadiusM <= 0.f ||
					 action.Volley.fArenaHeightToleranceM <= 0.f)) ||
				(perAlivePlayer && 0u == action.Volley.iArenaRandomCount) ||
				(bossRelative &&
					(!isMulti || 1u != action.Volley.iSpawnCount ||
					 0u != action.Volley.iArenaRandomCount)))
			{
				m_strStatus = "Boss pattern stage volley layout is invalid";
				return false;
			}
			action.strTargetId = fields[6];
			action.iValue = action.Volley.iCountPerResolvedTarget;
			const auto ownerMap = m_BossPatterns.find(std::string(fields[1]));
			if (m_BossPatterns.end() == ownerMap)
			{
				m_strStatus = "Boss pattern stage volley has no encounter";
				return false;
			}
			const auto owner = std::find_if(
				ownerMap->second.begin(), ownerMap->second.end(),
				[&fields](const BOSS_PATTERN_DEFINITION& pattern)
				{ return pattern.strPatternId == fields[2]; });
			if (ownerMap->second.end() == owner)
			{
				m_strStatus = "Boss pattern stage volley has no pattern owner";
				return false;
			}
			const auto stage = std::find_if(
				owner->Stages.begin(), owner->Stages.end(),
				[&fields](const BOSS_PATTERN_STAGE_DEFINITION& candidate)
				{ return candidate.strActionId == fields[3]; });
			const std::uint64_t lastSpawnOffsetMs =
				static_cast<std::uint64_t>(action.Volley.iSpawnCount - 1u) *
				action.Volley.iSpawnIntervalMs;
			if (owner->Stages.end() == stage ||
				lastSpawnOffsetMs >= stage->iDurationMs ||
				actionOrder != stage->Actions.size() || actionOrder >= 8u ||
				std::any_of(stage->Actions.begin(), stage->Actions.end(),
					[&action](const BOSS_PATTERN_STAGE_ACTION& existing)
					{
						return existing.eTrigger == action.eTrigger &&
							existing.strTargetId == action.strTargetId;
					}))
			{
				m_strStatus =
					"Boss pattern stage volley owner is missing or duplicated";
				return false;
			}
			stage->Actions.push_back(std::move(action));
		}
		else if (!fields.empty() && "PATTERNSTAGEBRANCH" == fields[0])
		{
			BOSS_PATTERN_STAGE_BRANCH branch{};
			if (6u != fields.size() || !IsStableId(fields[1]) ||
				!IsStableId(fields[2]) || !IsStableId(fields[3]) ||
				!ParseBossPatternStageOutcome(fields[4], branch.eOutcome) ||
				("-" != fields[5] && !IsStableId(fields[5])))
			{
				m_strStatus = "Boss pattern stage branch row is invalid";
				return false;
			}
			const auto ownerMap = m_BossPatterns.find(std::string(fields[1]));
			if (m_BossPatterns.end() == ownerMap)
			{
				m_strStatus = "Boss pattern stage branch has no encounter";
				return false;
			}
			const auto owner = std::find_if(
				ownerMap->second.begin(), ownerMap->second.end(),
				[&fields](const BOSS_PATTERN_DEFINITION& pattern)
				{ return pattern.strPatternId == fields[2]; });
			if (ownerMap->second.end() == owner)
			{
				m_strStatus = "Boss pattern stage branch has no pattern owner";
				return false;
			}
			const auto stage = std::find_if(
				owner->Stages.begin(), owner->Stages.end(),
				[&fields](const BOSS_PATTERN_STAGE_DEFINITION& candidate)
				{ return candidate.strActionId == fields[3]; });
			if (owner->Stages.end() == stage || stage->Branches.size() >= 8u ||
				std::any_of(stage->Branches.begin(), stage->Branches.end(),
					[&branch](const BOSS_PATTERN_STAGE_BRANCH& existing)
					{ return existing.eOutcome == branch.eOutcome; }))
			{
				m_strStatus =
					"Boss pattern stage branch has no stage owner or is duplicated";
				return false;
			}
			branch.strNextActionId =
				"-" == fields[5] ? "" : std::string(fields[5]);
			stage->Branches.push_back(std::move(branch));
		}
		else if (!fields.empty() && "PATTERNSTAGEMOTION" == fields[0])
		{
			BOSS_PATTERN_STAGE_MOTION motion{};
			if ((6u != fields.size() && 8u != fields.size()) ||
				!IsStableId(fields[1]) || !IsStableId(fields[2]) ||
				!IsStableId(fields[3]))
			{
				m_strStatus = "Boss pattern stage motion row is invalid";
				return false;
			}
			if (6u == fields.size() && "FORWARD" == fields[4] &&
				ParseNumber(fields[5], motion.fDistance) &&
				std::isfinite(motion.fDistance) && motion.fDistance > 0.f &&
				motion.fDistance <= 1000.f)
			{
				motion.eKind = BOSS_PATTERN_STAGE_MOTION_KIND::FORWARD;
			}
			else if (8u == fields.size() &&
				"PORTAL_TARGET_RUSH" == fields[4] &&
				ParseNumber(fields[5], motion.iRetargetDelayMs) &&
				ParseNumber(fields[6], motion.fSpeedMps) &&
				ParseNumber(fields[7], motion.fDistance) &&
				std::isfinite(motion.fSpeedMps) &&
				std::isfinite(motion.fDistance) &&
				motion.fSpeedMps > 0.f && motion.fSpeedMps <= 1000.f &&
				motion.fDistance > 0.f && motion.fDistance <= 1000.f)
			{
				motion.eKind = BOSS_PATTERN_STAGE_MOTION_KIND::PORTAL_TARGET_RUSH;
			}
			else if (8u == fields.size() && "PORTAL_CROSS_ARENA" == fields[4] &&
				ParseNumber(fields[5], motion.iCornerIndex) &&
				ParseNumber(fields[6], motion.fHalfExtentsX) &&
				ParseNumber(fields[7], motion.fHalfExtentsZ) &&
				motion.iCornerIndex < 4u && std::isfinite(motion.fHalfExtentsX) &&
				std::isfinite(motion.fHalfExtentsZ) &&
				motion.fHalfExtentsX >= 1.f && motion.fHalfExtentsX <= 100.f &&
				motion.fHalfExtentsZ >= 1.f && motion.fHalfExtentsZ <= 100.f)
			{
				motion.eKind = BOSS_PATTERN_STAGE_MOTION_KIND::PORTAL_CROSS_ARENA;
			}
			else
			{
				m_strStatus = "Boss pattern stage motion policy is invalid";
				return false;
			}
			const auto ownerMap = m_BossPatterns.find(std::string(fields[1]));
			if (m_BossPatterns.end() == ownerMap)
			{
				m_strStatus = "Boss pattern stage motion has no encounter";
				return false;
			}
			const auto owner = std::find_if(
				ownerMap->second.begin(), ownerMap->second.end(),
				[&fields](const BOSS_PATTERN_DEFINITION& pattern)
				{ return pattern.strPatternId == fields[2]; });
			if (ownerMap->second.end() == owner)
			{
				m_strStatus = "Boss pattern stage motion has no pattern owner";
				return false;
			}
			const auto stage = std::find_if(
				owner->Stages.begin(), owner->Stages.end(),
				[&fields](const BOSS_PATTERN_STAGE_DEFINITION& candidate)
				{ return candidate.strActionId == fields[3]; });
			if (owner->Stages.end() == stage ||
				BOSS_PATTERN_STAGE_MOTION_KIND::NONE != stage->Motion.eKind)
			{
				m_strStatus =
					"Boss pattern stage motion has no stage owner or is duplicated";
				return false;
			}
			if (BOSS_PATTERN_STAGE_MOTION_KIND::PORTAL_TARGET_RUSH ==
				motion.eKind)
			{
				const double travelEndMs =
					static_cast<double>(motion.iRetargetDelayMs) +
					static_cast<double>(motion.fDistance) /
					static_cast<double>(motion.fSpeedMps) * 1000.0;
				std::uint32_t expectedOffsetMs = motion.iRetargetDelayMs;
				bool validOffsets = !stage->HitOffsetsMs.empty();
				for (const std::uint32_t offsetMs : stage->HitOffsetsMs)
				{
					validOffsets = validOffsets &&
						offsetMs == expectedOffsetMs &&
						static_cast<double>(offsetMs) < travelEndMs;
					expectedOffsetMs += 50u;
				}
				validOffsets = validOffsets &&
					static_cast<double>(expectedOffsetMs) + 0.000001 >=
						travelEndMs;
				if (travelEndMs > static_cast<double>(stage->iDurationMs) +
						0.000001 || !validOffsets)
				{
					m_strStatus =
						"Boss portal target-rush motion exceeds its stage or has unsafe swept-hit offsets";
					return false;
				}
			}
			stage->Motion = motion;
		}
		else if (!fields.empty() && "PATTERNWALLCONTACT" == fields[0])
		{
			std::uint32_t stageIndex = 0u;
			if (6u != fields.size() || !IsStableId(fields[1]) ||
				!IsStableId(fields[2]) || !ParseNumber(fields[3], stageIndex) ||
				!IsStableId(fields[4]) || !IsStableId(fields[5]))
			{
				m_strStatus = "Boss pattern wall-contact row is invalid";
				return false;
			}
			const auto ownerMap = m_BossPatterns.find(std::string(fields[1]));
			if (m_BossPatterns.end() == ownerMap)
			{
				m_strStatus = "Boss pattern wall-contact has no encounter";
				return false;
			}
			const auto owner = std::find_if(
				ownerMap->second.begin(), ownerMap->second.end(),
				[&fields](const BOSS_PATTERN_DEFINITION& pattern)
				{ return pattern.strPatternId == fields[2]; });
			if (ownerMap->second.end() == owner ||
				stageIndex >= owner->Stages.size())
			{
				m_strStatus = "Boss pattern wall-contact has no stage owner";
				return false;
			}
			BOSS_PATTERN_STAGE_DEFINITION& stage = owner->Stages[stageIndex];
			if (stage.strStageId != fields[4] ||
				stage.strActionId != fields[5] || stage.bWallContact ||
				BOSS_PATTERN_STAGE_KIND::ACTIVE != stage.eStageKind ||
				BOSS_PATTERN_HIT_SHAPE::NONE == stage.eHitShape ||
				0u == stage.iHitCount)
			{
				m_strStatus = "Boss pattern wall-contact join is invalid";
				return false;
			}
			stage.bWallContact = true;
		}
		/* Sorted after PATTERNSTAGE by the publisher, exactly like
		PATTERNWALLCONTACT, so the stage it refines already exists here. */
		else if (!fields.empty() && "PATTERNSTAGECOVERPIERCE" == fields[0])
		{
			std::uint32_t stageIndex = 0u;
			if (6u != fields.size() || !IsStableId(fields[1]) ||
				!IsStableId(fields[2]) || !ParseNumber(fields[3], stageIndex) ||
				!IsStableId(fields[4]) || !IsStableId(fields[5]))
			{
				m_strStatus = "Boss pattern cover-pierce row is invalid";
				return false;
			}
			const auto ownerMap = m_BossPatterns.find(std::string(fields[1]));
			if (m_BossPatterns.end() == ownerMap)
			{
				m_strStatus = "Boss pattern cover-pierce has no encounter";
				return false;
			}
			const auto owner = std::find_if(
				ownerMap->second.begin(), ownerMap->second.end(),
				[&fields](const BOSS_PATTERN_DEFINITION& pattern)
				{ return pattern.strPatternId == fields[2]; });
			if (ownerMap->second.end() == owner ||
				stageIndex >= owner->Stages.size())
			{
				m_strStatus = "Boss pattern cover-pierce has no stage owner";
				return false;
			}
			BOSS_PATTERN_STAGE_DEFINITION& stage = owner->Stages[stageIndex];
			if (stage.strStageId != fields[4] ||
				stage.strActionId != fields[5] || stage.bPiercesCover ||
				BOSS_PATTERN_HIT_SHAPE::NONE == stage.eHitShape ||
				0u == stage.iHitCount)
			{
				m_strStatus = "Boss pattern cover-pierce join is invalid";
				return false;
			}
			stage.bPiercesCover = true;
		}
		/* One row per slot so the row stays a fixed width. The stage collects
		them, and a slot the publisher already gave to another edge cannot arrive
		twice because it validates that across the whole document. */
		else if (!fields.empty() && "PATTERNSTAGEPROPBREAK" == fields[0])
		{
			std::uint32_t stageIndex = 0u;
			if (8u != fields.size() || !IsStableId(fields[1]) ||
				!IsStableId(fields[2]) || !ParseNumber(fields[3], stageIndex) ||
				!IsStableId(fields[4]) || !IsStableId(fields[5]) ||
				!IsStableId(fields[6]) || !IsStableId(fields[7]))
			{
				m_strStatus = "Boss pattern prop-break row is invalid";
				return false;
			}
			const auto ownerMap = m_BossPatterns.find(std::string(fields[1]));
			if (m_BossPatterns.end() == ownerMap)
			{
				m_strStatus = "Boss pattern prop-break has no encounter";
				return false;
			}
			const auto owner = std::find_if(
				ownerMap->second.begin(), ownerMap->second.end(),
				[&fields](const BOSS_PATTERN_DEFINITION& pattern)
				{ return pattern.strPatternId == fields[2]; });
			if (ownerMap->second.end() == owner ||
				stageIndex >= owner->Stages.size())
			{
				m_strStatus = "Boss pattern prop-break has no stage owner";
				return false;
			}
			BOSS_PATTERN_STAGE_DEFINITION& stage = owner->Stages[stageIndex];
			const std::string slotId(fields[7]);
			if (stage.strStageId != fields[4] ||
				stage.strActionId != fields[5] ||
				(!stage.strPropBreakSetId.empty() &&
					stage.strPropBreakSetId != fields[6]) ||
				stage.PropBreakSlotIds.end() != std::find(
					stage.PropBreakSlotIds.begin(),
					stage.PropBreakSlotIds.end(), slotId))
			{
				m_strStatus = "Boss pattern prop-break join is invalid";
				return false;
			}
			stage.strPropBreakSetId = std::string(fields[6]);
			stage.PropBreakSlotIds.push_back(slotId);
		}
		/* Sorted after PATTERNSTAGE by the publisher, exactly like
		PATTERNWALLCONTACT, so the stage it refines already exists here. */
		else if (!fields.empty() && "PATTERNSTAGECHARGE" == fields[0])
		{
			std::uint32_t stageIndex = 0u;
			if (6u != fields.size() || !IsStableId(fields[1]) ||
				!IsStableId(fields[2]) || !ParseNumber(fields[3], stageIndex) ||
				!IsStableId(fields[4]) || !IsStableId(fields[5]))
			{
				m_strStatus = "Boss pattern charge-impact row is invalid";
				return false;
			}
			const auto ownerMap = m_BossPatterns.find(std::string(fields[1]));
			if (m_BossPatterns.end() == ownerMap)
			{
				m_strStatus = "Boss pattern charge-impact has no encounter";
				return false;
			}
			const auto owner = std::find_if(
				ownerMap->second.begin(), ownerMap->second.end(),
				[&fields](const BOSS_PATTERN_DEFINITION& pattern)
				{ return pattern.strPatternId == fields[2]; });
			if (ownerMap->second.end() == owner ||
				stageIndex + 1u >= owner->Stages.size())
			{
				m_strStatus = "Boss pattern charge-impact has no stage owner";
				return false;
			}
			BOSS_PATTERN_STAGE_DEFINITION& stage = owner->Stages[stageIndex];
			/* The authored WALL_CONTACT edge must end this stage in the immediate
			GROGGY stage. The collision/world-destruction transaction publishes the
			outcome only after an exact receiver mutation commits, so allowing an
			intermediate recovery here would delay the visible impact reaction. */
			const auto wallBranch = std::find_if(
				stage.Branches.begin(), stage.Branches.end(),
				[](const BOSS_PATTERN_STAGE_BRANCH& branch)
				{
					return BOSS_PATTERN_STAGE_OUTCOME::WALL_CONTACT ==
						branch.eOutcome;
				});
			const BOSS_PATTERN_STAGE_DEFINITION& impactStage =
				owner->Stages[stageIndex + 1u];
			const bool validImpactKind =
				BOSS_PATTERN_STAGE_KIND::GROGGY == impactStage.eStageKind;
			if (stage.strStageId != fields[4] ||
				stage.strActionId != fields[5] || stage.bChargeImpact ||
				stage.Branches.end() == wallBranch ||
				wallBranch->strNextActionId != impactStage.strActionId ||
				!validImpactKind ||
				owner->fMaximumRange <= 0.f)
			{
				m_strStatus = "Boss pattern charge-impact join is invalid";
				return false;
			}
			stage.bChargeImpact = true;
		}
		else if (!fields.empty() && "VALTANTIMELINE" == fields[0])
		{
			std::uint32_t rowCount = 0u;
			if (4u != fields.size() || !IsStableId(fields[1]) ||
				!IsStableId(fields[2]) ||
				!ParseNumber(fields[3], rowCount) || 0u == rowCount ||
				rowCount > 256u)
			{
				m_strStatus = "Valtan timeline row is invalid";
				return false;
			}
			VALTAN_TIMELINE_DEFINITION definition{};
			definition.strEncounterId = fields[1];
			definition.strTimelineId = fields[2];
			definition.Rows.resize(rowCount);
			if (!m_ValtanTimelines.emplace(
				definition.strEncounterId, std::move(definition)).second)
			{
				m_strStatus = "Duplicate Valtan timeline row";
				return false;
			}
		}
		else if (!fields.empty() && "VALTANTIMELINEROW" == fields[0])
		{
			VALTAN_TIMELINE_ROW timelineRow{};
			std::uint32_t actionCount = 0u;
			if (11u != fields.size() || !IsStableId(fields[1]) ||
				!IsStableId(fields[2]) ||
				!ParseNumber(fields[3], timelineRow.iCommandId) ||
				!ParseNumber(fields[4], timelineRow.iOrdinal) ||
				!IsStableId(fields[5]) ||
				!ParseNumber(fields[6], timelineRow.iSectionHealthBar) ||
				!ParseValtanTimelineEntryType(fields[7], timelineRow.eEntryType) ||
				!ParseValtanTimelineArenaState(fields[8], timelineRow.eArenaState) ||
				!ParseValtanTimelinePropState(fields[9], timelineRow.ePropState) ||
				!ParseNumber(fields[10], actionCount) ||
				0u == timelineRow.iOrdinal ||
				0u == timelineRow.iCommandId ||
				timelineRow.iCommandId !=
					Calculate_ValtanTimelineCommandId(fields[5]) ||
				0u == timelineRow.iSectionHealthBar ||
				timelineRow.iSectionHealthBar > 1000u ||
				0u == actionCount || actionCount > 8u)
			{
				m_strStatus = "Valtan timeline occurrence row is invalid";
				return false;
			}
			const auto owner =
				m_ValtanTimelines.find(std::string(fields[1]));
			if (m_ValtanTimelines.end() == owner ||
				owner->second.strTimelineId != fields[2] ||
				timelineRow.iOrdinal > owner->second.Rows.size())
			{
				m_strStatus = "Valtan timeline occurrence has no owner";
				return false;
			}
			VALTAN_TIMELINE_ROW& slot =
				owner->second.Rows[timelineRow.iOrdinal - 1u];
			if (0u != slot.iOrdinal)
			{
				m_strStatus = "Duplicate Valtan timeline occurrence ordinal";
				return false;
			}
			if (slot.PatternActions.size() > actionCount)
			{
				m_strStatus =
					"Valtan timeline occurrence action count is invalid";
				return false;
			}
			timelineRow.strRowId = fields[5];
			timelineRow.PatternActions = std::move(slot.PatternActions);
			timelineRow.PatternActions.resize(actionCount);
			slot = std::move(timelineRow);
		}
		else if (!fields.empty() && "VALTANTIMELINEPATTERN" == fields[0])
		{
			std::uint32_t ordinal = 0u;
			std::uint32_t actionIndex = 0u;
			VALTAN_TIMELINE_PATTERN_ACTION action{};
			if (7u != fields.size() || !IsStableId(fields[1]) ||
				!IsStableId(fields[2]) || !ParseNumber(fields[3], ordinal) ||
				!ParseNumber(fields[4], actionIndex) ||
				!IsStableId(fields[5]) ||
				!ParseNumber(fields[6], action.iRepeat) ||
				0u == ordinal || 0u == actionIndex || actionIndex > 8u ||
				0u == action.iRepeat || action.iRepeat > 4u)
			{
				m_strStatus = "Valtan timeline pattern action row is invalid";
				return false;
			}
			const auto owner = m_ValtanTimelines.find(std::string(fields[1]));
			if (m_ValtanTimelines.end() == owner ||
				owner->second.strTimelineId != fields[2] ||
				ordinal > owner->second.Rows.size())
			{
				m_strStatus = "Valtan timeline pattern action has no owner";
				return false;
			}
			VALTAN_TIMELINE_ROW& timelineRow = owner->second.Rows[ordinal - 1u];
			if ((0u != timelineRow.iOrdinal &&
				timelineRow.iOrdinal != ordinal) ||
				(0u != timelineRow.iOrdinal &&
				 actionIndex > timelineRow.PatternActions.size()))
			{
				m_strStatus = "Valtan timeline pattern action has no occurrence owner";
				return false;
			}
			if (timelineRow.PatternActions.size() < actionIndex)
				timelineRow.PatternActions.resize(actionIndex);
			VALTAN_TIMELINE_PATTERN_ACTION& slot =
				timelineRow.PatternActions[actionIndex - 1u];
			if (!slot.strPatternId.empty() || 0u != slot.iRepeat)
			{
				m_strStatus = "Duplicate Valtan timeline pattern action index";
				return false;
			}
			action.strPatternId = fields[5];
			slot = std::move(action);
		}
		else if (!fields.empty() && "PLAYER" == fields[0])
		{
			PLAYER_RUNTIME_PROFILE player{};
			if (15u != fields.size() ||
				!ParseCharacterClass(fields[1], player.eCharacterClass) ||
				!ParseNumber(fields[2], player.iMaximumHp) ||
				!ParseNumber(fields[3], player.iMaximumResource) ||
				!ParseNumber(fields[4], player.iResourceRegenPerSecond) ||
				!ParseNumber(fields[5], player.iAttackPower) ||
				!ParseNumber(fields[6], player.iDefense) ||
				!ParseNumber(fields[7], player.fMoveSpeed) ||
				!ParseNumber(fields[8], player.fDefenseStanceMoveSpeedScale) ||
				!ParseNumber(fields[9], player.iMaximumIdentity) ||
				!ParseNumber(fields[10], player.iIdentityRegenPerSecond) ||
				!ParseNumber(fields[11], player.iIdentityDrainPerSecond) ||
				!ParseNumber(fields[12], player.iIdentityStanceSwitchCost) ||
				!ParseNumber(fields[13], player.iIdentityCyclic) ||
				!ParseStance(fields[14], player.eDefaultStance) ||
				0u == player.iMaximumHp || 0u == player.iMaximumResource ||
				0u == player.iResourceRegenPerSecond ||
				player.iResourceRegenPerSecond > player.iMaximumResource ||
				0u == player.iAttackPower || 0u == player.iDefense ||
				!std::isfinite(player.fMoveSpeed) || player.fMoveSpeed <= 0.f ||
				!std::isfinite(player.fDefenseStanceMoveSpeedScale) ||
				player.fDefenseStanceMoveSpeedScale <= 0.f ||
				player.fDefenseStanceMoveSpeedScale > 1.f ||
				player.iIdentityCyclic > 1u ||
				(0u == player.iMaximumIdentity &&
					(0u != player.iIdentityRegenPerSecond ||
						0u != player.iIdentityDrainPerSecond ||
						0u != player.iIdentityStanceSwitchCost ||
						0u != player.iIdentityCyclic)) ||
				/* A cyclic gauge spends itself by wrapping, so it cannot also
				drain or charge a switch -- there would be no single answer for
				what "isHolding" means. A class can also spend identity through a
				skill's own iIdentityCost (Artist's moon/sun orbs), which is not
				known until every SKILL row is read -- checked in the post-load
				pass below, alongside the identity-pool bound on iIdentityCost
				itself. */
				(0u != player.iIdentityCyclic &&
					(0u != player.iIdentityDrainPerSecond ||
						0u != player.iIdentityStanceSwitchCost ||
						0u == player.iIdentityRegenPerSecond)) ||
				player.iIdentityStanceSwitchCost > player.iMaximumIdentity ||
				!m_Players.emplace(player.eCharacterClass, player).second)
			{
				m_strStatus = "Player profile row is invalid";
				return false;
			}
		}
		else
		{
			m_strStatus = "Unknown gameplay bootstrap row kind";
			return false;
		}
	}

	if (std::getline(input, line) || m_Skills.empty() || m_Players.empty() ||
		m_Bosses.empty() || m_BossParts.empty() || m_BossPatterns.empty() ||
		m_BossCombatObjects.empty() ||
		m_ValtanTimelines.empty() ||
		m_DamageRatePercentByProfileId.empty() ||
		!m_ValtanPresentationGenerationId.Is_Valid())
	{
		m_strStatus = "Gameplay bootstrap has trailing rows or missing definitions";
		return false;
	}
	if (skillCombatTraitOwners.size() != m_Skills.size())
	{
		m_strStatus = "Player skill combat traits are incomplete";
		return false;
	}
	/* Rows arrive sorted, so a skill's cost cannot be checked against a pool while
	the skill row is being parsed. The largest pool any class has is the only bound
	that makes a cost payable by somebody. */
	std::uint32_t largestResourcePool = 0;
	std::uint32_t largestIdentityPool = 0;
	for (const auto& [characterClass, player] : m_Players)
	{
		(void)characterClass;
		largestResourcePool =
			(std::max)(largestResourcePool, player.iMaximumResource);
		largestIdentityPool =
			(std::max)(largestIdentityPool, player.iMaximumIdentity);
	}
	for (const auto& [skillId, skill] : m_Skills)
	{
		(void)skillId;
		const bool isCombo = LostArk::Shared::PLAYER_SKILL_KIND::COMBO ==
			skill.eSkillKind;
		const bool validStageCount = isCombo ?
			(skill.ComboStages.size() >= 2u && skill.ComboStages.size() <= 8u) :
			(LostArk::Shared::PLAYER_SKILL_KIND::HOLD == skill.eSkillKind ?
				skill.ComboStages.size() == 3u :
				(LostArk::Shared::PLAYER_SKILL_KIND::COUNTER == skill.eSkillKind ?
					skill.ComboStages.size() == 2u : skill.ComboStages.empty()));
		if (!validStageCount ||
			(isCombo &&
				(skill.ComboStages.back().iComboAdvanceMs !=
					skill.ComboStages.back().iActionDurationMs ||
				 0u != skill.ComboStages.back().iInputOpenMs ||
				 0u != skill.ComboStages.back().iInputCloseMs)))
		{
			m_strStatus = "Player skill stage contract is invalid";
			return false;
		}
		if (isCombo)
		{
			for (std::size_t stageIndex = 0u;
				stageIndex < skill.ComboStages.size(); ++stageIndex)
			{
				const PLAYER_COMBO_STAGE& stage = skill.ComboStages[stageIndex];
				const bool isFinalStage =
					stageIndex + 1u == skill.ComboStages.size();
				const bool isAutomaticStage = !isFinalStage &&
					0u == stage.iInputOpenMs && 0u == stage.iInputCloseMs;
				if ((isAutomaticStage &&
						stage.iComboAdvanceMs != stage.iActionDurationMs) ||
					(!isFinalStage && !isAutomaticStage &&
						(stage.iInputOpenMs >= stage.iInputCloseMs ||
							stage.iInputCloseMs > stage.iActionDurationMs)))
				{
					m_strStatus = "Player combo stage input contract is invalid";
					return false;
				}
				std::uint64_t latestRequiredFireMs = 0u;
				for (const PLAYER_SKILL_HIT& hit : stage.Hits)
				{
					latestRequiredFireMs = (std::max)(latestRequiredFireMs,
						static_cast<std::uint64_t>(hit.iTimeMs) +
						static_cast<std::uint64_t>(hit.iRepeatCount - 1u) *
							hit.iRepeatMs);
				}
				for (const PLAYER_SKILL_PROJECTILE& projectile : stage.Projectiles)
				{
					latestRequiredFireMs = (std::max)(latestRequiredFireMs,
						static_cast<std::uint64_t>(projectile.iTimeMs));
				}
				if (stage.iComboAdvanceMs < latestRequiredFireMs)
				{
					m_strStatus =
						"Player combo boundary precedes a hit or projectile spawn";
					return false;
				}
			}
		}
		if (!skill.strDamageProfileId.empty() &&
			0u == Find_DamageRatePercent(skill.strDamageProfileId))
		{
			m_strStatus = "Player skill references missing damage profile";
			return false;
		}
		if (skill.iResourceCost > largestResourcePool ||
			skill.iIdentityCost > largestIdentityPool)
		{
			m_strStatus = "Player skill costs more than any class can hold";
			return false;
		}
	}
	std::unordered_set<LostArk::Shared::CHARACTER_CLASS_ID> classesWithIdentitySkillCost;
	for (const auto& [skillId, skill] : m_Skills)
	{
		(void)skillId;
		if (0u != skill.iIdentityCost)
			classesWithIdentitySkillCost.insert(skill.eCharacterClass);
	}
	for (const auto& [characterClass, player] : m_Players)
	{
		if (0u != player.iMaximumIdentity &&
			0u == player.iIdentityDrainPerSecond &&
			0u == player.iIdentityStanceSwitchCost &&
			0u == player.iIdentityCyclic &&
			!classesWithIdentitySkillCost.contains(characterClass))
		{
			m_strStatus = "Player identity gauge never spends";
			return false;
		}
	}
	std::unordered_set<std::string> dependentBossArchetypeIds;
	for (const auto& [encounterId, patterns] : m_BossPatterns)
	{
		for (const BOSS_PATTERN_DEFINITION& pattern : patterns)
		{
			if (BOSS_PATTERN_FINALE_KIND::NONE == pattern.Finale.eKind)
				continue;
			const auto ghost = m_Bosses.find(pattern.Finale.strGhostArchetypeId);
			const auto ghostParts = m_BossParts.find(
				pattern.Finale.strGhostArchetypeId);
			if (m_Bosses.end() == ghost ||
				ghost->second.strEncounterId != encounterId ||
				(m_BossParts.end() != ghostParts && !ghostParts->second.empty()) ||
				pattern.bInvulnerableWhileRunning ||
				pattern.Finale.iMaximumActiveGhosts < 1u ||
				pattern.Finale.iMaximumActiveGhosts > 64u ||
				pattern.Finale.GhostPatternIds.empty() ||
				pattern.Finale.GhostPatternIds.size() > 64u)
			{
				m_strStatus = "Boss finale dependent archetype is invalid";
				return false;
			}
			dependentBossArchetypeIds.insert(ghost->first);
			std::unordered_set<std::string> childPatternIds;
			for (const std::string& patternId : pattern.Finale.GhostPatternIds)
			{
				const auto attack = std::find_if(patterns.begin(), patterns.end(),
					[&patternId](const BOSS_PATTERN_DEFINITION& candidate)
					{ return candidate.strPatternId == patternId; });
				if (!childPatternIds.insert(patternId).second ||
					patterns.end() == attack ||
					BOSS_PATTERN_FINALE_KIND::NONE != attack->Finale.eKind)
				{
					m_strStatus = "Boss finale dependent attack is missing or recursive";
					return false;
				}
				if (!HasFiniteBossStageGraph(*attack))
				{
					m_strStatus = "Boss finale dependent attack is not finite";
					return false;
				}
			}
		}
	}
	std::unordered_set<std::string> spawnedBossCombatObjectIds;
	for (const auto& [archetypeId, boss] : m_Bosses)
	{
		const auto foundParts = m_BossParts.find(archetypeId);
		if ((m_BossParts.end() == foundParts || foundParts->second.empty()) &&
			!dependentBossArchetypeIds.contains(archetypeId))
		{
			m_strStatus = "Boss has no part definitions";
			return false;
		}
		std::uint32_t totalDamageReductionPercent = 0u;
		const std::vector<BOSS_PART_DEFINITION> emptyParts;
		const auto& bossParts = m_BossParts.end() == foundParts ?
			emptyParts : foundParts->second;
		for (const BOSS_PART_DEFINITION& part : bossParts)
		{
			if (part.strBossArchetypeId != archetypeId)
			{
				m_strStatus = "Boss part owner is invalid";
				return false;
			}
			totalDamageReductionPercent += part.iDamageReductionPercent;
		}
		if (totalDamageReductionPercent >= 100u)
		{
			m_strStatus = "Boss part damage reduction total is invalid";
			return false;
		}
		if (dependentBossArchetypeIds.contains(archetypeId))
		{
			const bool hasPrimary = std::any_of(m_Bosses.begin(), m_Bosses.end(),
				[&](const auto& entry)
				{
					return entry.second.strEncounterId == boss.strEncounterId &&
						!dependentBossArchetypeIds.contains(entry.first);
				});
			if (!hasPrimary)
			{
				m_strStatus = "Dependent boss encounter has no independent owner";
				return false;
			}
			/* The encounter graph is validated once by its primary profile. */
			continue;
		}
		const auto foundPatterns = m_BossPatterns.find(boss.strEncounterId);
		if (m_BossPatterns.end() == foundPatterns || foundPatterns->second.empty())
		{
			m_strStatus = "Boss encounter has no runtime patterns";
			return false;
		}
		std::size_t gameplayPhaseActionCount = 0u;
		bool hasExactValtanArenaBreakPhaseAction = false;
		bool hasExactValtanHighJumpTypedVolley = false;
		std::size_t valtanHighJumpTypedVolleyCount = 0u;
		std::unordered_set<std::uint64_t> healthMechanicOrderKeys;
		for (const BOSS_PATTERN_DEFINITION& pattern : foundPatterns->second)
		{
			const bool requiresFiniteGraph =
				BOSS_PATTERN_FINALE_KIND::NONE != pattern.Finale.eKind ||
				"VALTAN_TRASH" == pattern.strPatternId ||
				"VALTAN_TRASH_CATCH_IF" == pattern.strPatternId ||
				"VALTAN_TRASH_CATCH_SUCCESS" == pattern.strPatternId ||
				"VALTAN_TRASH_CATCH_FAIL" == pattern.strPatternId;
			if (requiresFiniteGraph && !HasFiniteBossStageGraph(pattern))
			{
				m_strStatus = "Boss bounded pattern stage graph is cyclic or dangling";
				return false;
			}
			const std::string patternPolicyKey =
				pattern.strEncounterId + "\n" + pattern.strPatternId;
			const bool isNormal =
				BOSS_PATTERN_SELECTION::NORMAL == pattern.eSelection;
			const bool isHealthMechanic =
				BOSS_PATTERN_SELECTION::HEALTH_BAR == pattern.eSelection;
			const bool isAuditionOnly =
				BOSS_PATTERN_SELECTION::AUDITION_ONLY == pattern.eSelection;
			const bool validNormalSelection = isNormal &&
				pattern.iMinimumHealthBar >= 1u &&
				pattern.iMaximumHealthBar >= pattern.iMinimumHealthBar &&
				pattern.iMaximumHealthBar <= boss.iMaximumHealthBars &&
				0u == pattern.iTriggerHealthBar && 0u == pattern.iTriggerOrder &&
				pattern.iSelectionWeight > 0u &&
				pattern.iMaximumConsecutiveUses > 0u;
			const bool hasNoHealthWindow =
				0u == pattern.iMinimumHealthBar &&
				0u == pattern.iMaximumHealthBar;
			const bool hasValidHealthWindow =
				pattern.iMinimumHealthBar >= 1u &&
				pattern.iMaximumHealthBar >= pattern.iMinimumHealthBar &&
				pattern.iMaximumHealthBar <= boss.iMaximumHealthBars;
			const bool validHealthSelection = isHealthMechanic &&
				(hasNoHealthWindow || hasValidHealthWindow) &&
				pattern.iTriggerHealthBar >= 1u &&
				pattern.iTriggerHealthBar <= boss.iMaximumHealthBars &&
				pattern.iTriggerOrder > 0u &&
				0u == pattern.iSelectionWeight;
			const bool validAuditionSelection = isAuditionOnly &&
				0u == pattern.iMinimumHealthBar &&
				0u == pattern.iMaximumHealthBar &&
				0u == pattern.iTriggerHealthBar &&
				0u == pattern.iTriggerOrder &&
				0u == pattern.iSelectionWeight &&
				0u == pattern.iMaximumConsecutiveUses &&
				BOSS_PATTERN_CATEGORY::MECHANIC != pattern.eCategory;
			const bool validSelection = validNormalSelection ||
				validHealthSelection || validAuditionSelection;
			if (!validSelection ||
				!patternPolicyOwners.contains(patternPolicyKey) ||
				(BOSS_PATTERN_AIM_POLICY::FACE_MOTION_ANCHOR ==
					pattern.eAimPolicy &&
				 BOSS_PATTERN_MOTION_KIND::LEAP_TO_ANCHOR !=
					pattern.Motion.eKind) ||
				(BOSS_PATTERN_MOTION_KIND::NONE != pattern.Motion.eKind &&
				 (pattern.Stages.empty() ||
				  0u == pattern.Motion.iTravelStageIndex ||
				  pattern.Motion.iTravelStageIndex >= pattern.Stages.size() ||
				  pattern.Motion.iTakeoffStartMs >=
					pattern.Motion.iTakeoffEndMs ||
				  pattern.Motion.iTakeoffEndMs >
					pattern.Stages.front().iDurationMs ||
				  pattern.Motion.iTravelStartMs >=
					pattern.Motion.iTravelEndMs ||
				  pattern.Motion.iTravelEndMs >
					pattern.Stages[pattern.Motion.iTravelStageIndex].iDurationMs)) ||
				0u == pattern.iSourcePrimaryActionId ||
				pattern.Stages.size() != pattern.iExpectedStageCount)
			{
				m_strStatus = "Boss pattern selection or stage count is invalid";
				return false;
			}
			if (BOSS_PATTERN_SELECTION::HEALTH_BAR == pattern.eSelection)
			{
				const std::uint64_t mechanicOrderKey =
					(static_cast<std::uint64_t>(pattern.iTriggerHealthBar) << 32u) |
					static_cast<std::uint64_t>(pattern.iTriggerOrder);
				if (!healthMechanicOrderKeys.insert(mechanicOrderKey).second)
				{
					m_strStatus =
						"Boss health mechanic trigger bar/order pair is duplicated";
					return false;
				}
			}
			std::unordered_set<std::string> activeStageActions;
			for (std::size_t stageIndex = 0u;
				stageIndex < pattern.Stages.size(); ++stageIndex)
			{
				const BOSS_PATTERN_STAGE_DEFINITION& stage =
					pattern.Stages[stageIndex];
				const bool zeroShapeValues =
					0.f == stage.fHitOuterRadius &&
					0.f == stage.fHitInnerRadius &&
					0.f == stage.fHitAngleDegrees &&
					0.f == stage.fHitLength &&
					0.f == stage.fHitHalfWidth;
				bool validShape = false;
				switch (stage.eHitShape)
				{
				case BOSS_PATTERN_HIT_SHAPE::NONE:
					validShape = zeroShapeValues && 0u == stage.iHitCount &&
						0u == stage.iHitIntervalMs &&
						stage.HitOffsetsMs.empty() &&
						stage.strDamageProfileId.empty();
					break;
				case BOSS_PATTERN_HIT_SHAPE::CIRCLE:
					validShape = stage.fHitOuterRadius > 0.f &&
						0.f == stage.fHitInnerRadius &&
						0.f == stage.fHitAngleDegrees &&
						0.f == stage.fHitLength && 0.f == stage.fHitHalfWidth;
					break;
				case BOSS_PATTERN_HIT_SHAPE::RING:
					validShape = stage.fHitOuterRadius > stage.fHitInnerRadius &&
						stage.fHitInnerRadius > 0.f &&
						0.f == stage.fHitAngleDegrees &&
						0.f == stage.fHitLength && 0.f == stage.fHitHalfWidth;
					break;
				case BOSS_PATTERN_HIT_SHAPE::CONE:
					validShape = stage.fHitAngleDegrees > 0.f &&
						stage.fHitAngleDegrees <= 180.f && stage.fHitLength > 0.f &&
						0.f == stage.fHitOuterRadius &&
						0.f == stage.fHitInnerRadius && 0.f == stage.fHitHalfWidth;
					break;
				case BOSS_PATTERN_HIT_SHAPE::BOX:
				case BOSS_PATTERN_HIT_SHAPE::CROSS:
				case BOSS_PATTERN_HIT_SHAPE::SIX_DIRECTIONS:
					validShape = stage.fHitLength > 0.f && stage.fHitHalfWidth > 0.f &&
						0.f == stage.fHitOuterRadius &&
						0.f == stage.fHitInnerRadius &&
						0.f == stage.fHitAngleDegrees;
					break;
				}
				if (BOSS_PATTERN_HIT_SHAPE::NONE != stage.eHitShape)
				{
					const bool hasExplicitHitOffsets =
						!stage.HitOffsetsMs.empty();
					const bool validExplicitHitOffsets =
						hasExplicitHitOffsets &&
						stage.HitOffsetsMs.size() == stage.iHitCount &&
						0u == stage.iHitIntervalMs && 0u == stage.iHitDelayMs &&
						stage.HitOffsetsMs.back() < stage.iDurationMs &&
						std::adjacent_find(
							stage.HitOffsetsMs.begin(), stage.HitOffsetsMs.end(),
							std::greater_equal<std::uint32_t>()) ==
							stage.HitOffsetsMs.end();
					const bool validLegacyHitSchedule =
						!hasExplicitHitOffsets &&
						BOSS_PATTERN_HIT_ACTIVATION_KIND::PULSE_SCHEDULE ==
							stage.eHitActivationKind &&
						(1u == stage.iHitCount ? 0u == stage.iHitIntervalMs :
							stage.iHitIntervalMs > 0u) &&
						static_cast<std::uint64_t>(stage.iHitDelayMs) +
							static_cast<std::uint64_t>(stage.iHitCount - 1u) *
							stage.iHitIntervalMs < stage.iDurationMs;
					const bool validActiveWindow =
						BOSS_PATTERN_HIT_ACTIVATION_KIND::ACTIVE_WINDOW ==
							stage.eHitActivationKind &&
						0u == stage.iHitCount && 0u == stage.iHitIntervalMs &&
						0u == stage.iHitDelayMs && stage.HitOffsetsMs.empty() &&
						stage.iHitActivationLifetimeMs > 0u &&
						static_cast<std::uint64_t>(stage.iHitActivationStartMs) +
							stage.iHitActivationLifetimeMs <= stage.iDurationMs;
					validShape = validShape &&
						((stage.iHitCount > 0u &&
						  BOSS_PATTERN_HIT_ACTIVATION_KIND::PULSE_SCHEDULE ==
							stage.eHitActivationKind &&
						  (validExplicitHitOffsets || validLegacyHitSchedule) &&
						  0u == stage.iHitActivationStartMs &&
						  0u == stage.iHitActivationLifetimeMs) ||
						 validActiveWindow) &&
						0u != Find_DamageRatePercent(stage.strDamageProfileId);
				}
				const bool validHitAnchor =
					std::isfinite(stage.fHitAnchorForwardOffsetM) &&
					std::isfinite(stage.fHitAnchorRightOffsetM) &&
					std::isfinite(stage.fHitAnchorYawOffsetDegrees) &&
					std::fabs(stage.fHitAnchorForwardOffsetM) <= 1000.f &&
					std::fabs(stage.fHitAnchorRightOffsetM) <= 1000.f &&
					std::fabs(stage.fHitAnchorYawOffsetDegrees) <= 360.f &&
					(BOSS_PATTERN_HIT_SHAPE::NONE != stage.eHitShape ||
					 (BOSS_PATTERN_HIT_ANCHOR_KIND::BOSS_CURRENT ==
						stage.eHitAnchorKind &&
					  0.f == stage.fHitAnchorForwardOffsetM &&
					  0.f == stage.fHitAnchorRightOffsetM &&
					  0.f == stage.fHitAnchorYawOffsetDegrees &&
					  BOSS_PATTERN_HIT_ACTIVATION_KIND::PULSE_SCHEDULE ==
						stage.eHitActivationKind &&
					  0u == stage.iHitActivationStartMs &&
					  0u == stage.iHitActivationLifetimeMs));
				const bool validPortalHitAuthority =
					(BOSS_PATTERN_STAGE_MOTION_KIND::PORTAL_TARGET_RUSH !=
						stage.Motion.eKind &&
					 BOSS_PATTERN_STAGE_MOTION_KIND::PORTAL_CROSS_ARENA !=
						stage.Motion.eKind) ||
					(BOSS_PATTERN_HIT_ANCHOR_KIND::BOSS_CURRENT ==
						stage.eHitAnchorKind &&
					 0.f == stage.fHitAnchorForwardOffsetM &&
					 0.f == stage.fHitAnchorRightOffsetM &&
					 0.f == stage.fHitAnchorYawOffsetDegrees &&
					 BOSS_PATTERN_HIT_ACTIVATION_KIND::PULSE_SCHEDULE ==
						stage.eHitActivationKind);
				const bool validPlayerResponse =
					(BOSS_PATTERN_PLAYER_RESPONSE::DAMAGE ==
						stage.ePlayerResponse &&
					 LostArk::Shared::PLAYER_ATTACHMENT_SLOT::NONE ==
						stage.eAttachmentSlot) ||
					(BOSS_PATTERN_PLAYER_RESPONSE::CAPTURE ==
						stage.ePlayerResponse &&
					 LostArk::Shared::PLAYER_ATTACHMENT_SLOT::BOSS_LEFT_HAND ==
						stage.eAttachmentSlot &&
					 BOSS_PATTERN_HIT_SHAPE::NONE != stage.eHitShape &&
						 0.f == stage.fPushRangeM && 0u == stage.iPushMs &&
						 !stage.bKnockdown && 0u == stage.iDownMs);
				const bool validCounterProxy =
					(!stage.bHasCounterProxy &&
					 0.f == stage.fCounterProxyForwardOffsetM &&
					 0.f == stage.fCounterProxyRightOffsetM &&
					 0.f == stage.fCounterProxyRadiusM) ||
					(stage.bHasCounterProxy &&
					 std::isfinite(stage.fCounterProxyForwardOffsetM) &&
					 std::isfinite(stage.fCounterProxyRightOffsetM) &&
					 std::isfinite(stage.fCounterProxyRadiusM) &&
					 std::fabs(stage.fCounterProxyForwardOffsetM) <= 20.f &&
					 std::fabs(stage.fCounterProxyRightOffsetM) <= 20.f &&
					 stage.fCounterProxyRadiusM > 0.f &&
					 stage.fCounterProxyRadiusM <= 20.f);
				if (!validShape || !validHitAnchor || !validPortalHitAuthority ||
					!validPlayerResponse ||
					!validCounterProxy ||
					(stage.bWallContact &&
						(BOSS_PATTERN_STAGE_KIND::ACTIVE != stage.eStageKind ||
						 BOSS_PATTERN_HIT_SHAPE::NONE == stage.eHitShape ||
						 0u == stage.iHitCount)) ||
					std::any_of(pattern.Stages.begin() + stageIndex + 1u,
						pattern.Stages.end(),
						[&stage](const BOSS_PATTERN_STAGE_DEFINITION& other)
						{
							return stage.strStageId == other.strStageId ||
								stage.strActionId == other.strActionId;
						}))
				{
					m_strStatus = "Boss pattern stage hit contract is invalid";
					return false;
				}
				const std::size_t timeoutCount = static_cast<std::size_t>(
					std::count_if(stage.Branches.begin(), stage.Branches.end(),
						[](const BOSS_PATTERN_STAGE_BRANCH& branch)
						{
							return BOSS_PATTERN_STAGE_OUTCOME::TIMEOUT ==
								branch.eOutcome;
						}));
				bool validBranches = 1u == timeoutCount && !stage.Branches.empty();
				for (const BOSS_PATTERN_STAGE_BRANCH& branch : stage.Branches)
				{
					if (!branch.strNextActionId.empty())
					{
						const auto target = std::find_if(
							pattern.Stages.begin(), pattern.Stages.end(),
							[&branch](const BOSS_PATTERN_STAGE_DEFINITION& candidate)
							{
								return candidate.strActionId == branch.strNextActionId;
							});
						validBranches = validBranches &&
							target != pattern.Stages.end() &&
							branch.strNextActionId != stage.strActionId;
					}
					if (BOSS_PATTERN_STAGE_OUTCOME::WALL_CONTACT == branch.eOutcome)
					{
						validBranches = validBranches &&
							BOSS_PATTERN_STAGE_MOTION_KIND::FORWARD ==
								stage.Motion.eKind;
					}
					if (BOSS_PATTERN_STAGE_OUTCOME::NAVIGATION_BLOCKED ==
						branch.eOutcome)
					{
						validBranches = validBranches &&
							BOSS_PATTERN_PLAYER_RESPONSE::CAPTURE == stage.ePlayerResponse &&
							(BOSS_PATTERN_STAGE_MOTION_KIND::FORWARD == stage.Motion.eKind ||
								!stage.Motion.RootMotion.empty());
					}
				}
				bool validActions = true;
				for (const BOSS_PATTERN_STAGE_ACTION& action : stage.Actions)
				{
					if (BOSS_PATTERN_STAGE_ACTION_KIND::
							SPAWN_COMBAT_OBJECT_VOLLEY != action.eKind &&
						!IsValidBossPatternStageAction(
							action.eTrigger, action.eKind, action.strTargetId,
							action.iValue, action.iDurationMs,
							action.eReleaseMode, action.fReleaseSpeedMps,
							action.fReleaseYawOffsetDegrees))
					{
						validActions = false;
						continue;
					}
					if (BOSS_PATTERN_STAGE_ACTION_KIND::SET_GAMEPLAY_PHASE ==
						action.eKind)
					{
						++gameplayPhaseActionCount;
						validActions = validActions &&
							BOSS_PATTERN_SELECTION::HEALTH_BAR == pattern.eSelection &&
							BOSS_PATTERN_CATEGORY::MECHANIC == pattern.eCategory &&
							BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER ==
								action.eTrigger &&
							"boss.phase.gameplay" == action.strTargetId &&
							2u == action.iValue && 0u == action.iDurationMs;
						if ("BOSS_VALTAN" == archetypeId &&
							"ENCOUNTER_VALTAN" == boss.strEncounterId &&
							"VALTAN_ARENA_BREAK_109" == pattern.strPatternId &&
							"IMPACT" == stage.strStageId &&
							"valtan.mechanic.arena-break-109.impact" ==
								stage.strActionId &&
							BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER ==
								action.eTrigger &&
							"boss.phase.gameplay" == action.strTargetId &&
							2u == action.iValue && 0u == action.iDurationMs)
						{
							hasExactValtanArenaBreakPhaseAction = true;
						}
						continue;
					}
					if (BOSS_PATTERN_STAGE_ACTION_KIND::SPAWN_COMBAT_OBJECT ==
							action.eKind ||
						BOSS_PATTERN_STAGE_ACTION_KIND::SPAWN_COMBAT_OBJECT_VOLLEY ==
							action.eKind)
					{
						const auto combatObject = m_BossCombatObjects.find(
							action.strTargetId);
						const bool targetLocksOnStart =
							BOSS_PATTERN_TARGET_POLICY::LOCK_NEAREST_ON_START ==
								pattern.eTargetPolicy ||
							BOSS_PATTERN_TARGET_POLICY::LOCK_RANDOM_ALIVE_ON_START ==
								pattern.eTargetPolicy ||
							BOSS_PATTERN_TARGET_POLICY::
								LOCK_RANDOM_ALIVE_BEHIND_ON_START ==
								pattern.eTargetPolicy;
						const bool fixedArea = combatObject !=
							m_BossCombatObjects.end() &&
							BOSS_COMBAT_OBJECT_KIND::FIXED_AREA ==
								combatObject->second.eKind;
						const bool isVolley =
							BOSS_PATTERN_STAGE_ACTION_KIND::
								SPAWN_COMBAT_OBJECT_VOLLEY == action.eKind;
						if (isVolley && "BOSS_VALTAN" == archetypeId &&
							"ENCOUNTER_VALTAN" == boss.strEncounterId &&
							"VALTAN_HIGH_JUMP" == pattern.strPatternId)
						{
							++valtanHighJumpTypedVolleyCount;
						}
						if (isVolley && "BOSS_VALTAN" == archetypeId &&
							"ENCOUNTER_VALTAN" == boss.strEncounterId &&
							"VALTAN_HIGH_JUMP" == pattern.strPatternId &&
							"AIRBORNE" == stage.strStageId &&
							"valtan.attack.high-jump.airborne" == stage.strActionId &&
							stage.iDurationMs >= 4000u &&
							!stage.Actions.empty() && &action == &stage.Actions.front() &&
							"combatobject.valtan.high-jump.target-axe" ==
								action.strTargetId &&
							BOSS_COMBAT_OBJECT_VOLLEY_POLICY::PER_ALIVE_PLAYER ==
								action.Volley.ePolicy &&
							3u == action.Volley.iSpawnCount &&
							1333u == action.Volley.iSpawnIntervalMs &&
							4u == action.Volley.iArenaRandomCount &&
							14.f == action.Volley.fArenaRandomRadiusM &&
							1.f == action.Volley.fArenaHeightToleranceM &&
							BOSS_COMBAT_OBJECT_ARENA_ANCHOR_POLICY::
								BOSS_SPAWN_POSITION ==
								action.Volley.eArenaAnchorPolicy)
						{
							hasExactValtanHighJumpTypedVolley = true;
						}
						if (m_BossCombatObjects.end() == combatObject ||
							combatObject->second.strEncounterId != pattern.strEncounterId ||
							combatObject->second.strOwnerPatternId != pattern.strPatternId ||
							combatObject->second.strOwnerStageActionId !=
								stage.strActionId ||
							BOSS_PATTERN_HIT_SHAPE::NONE != stage.eHitShape ||
							!spawnedBossCombatObjectIds.insert(
								action.strTargetId).second ||
							(fixedArea && !targetLocksOnStart &&
								BOSS_COMBAT_OBJECT_ORIGIN_POLICY::BOSS_POSITION !=
									combatObject->second.eOriginPolicy) ||
							(isVolley &&
							 ((BOSS_COMBAT_OBJECT_VOLLEY_POLICY::PER_ALIVE_PLAYER ==
									action.Volley.ePolicy &&
								BOSS_COMBAT_OBJECT_ORIGIN_POLICY::
									LOCKED_TARGET_PER_ALIVE_PLAYER !=
									combatObject->second.eOriginPolicy) ||
							  (BOSS_COMBAT_OBJECT_VOLLEY_POLICY::BOSS_RELATIVE ==
									action.Volley.ePolicy &&
								BOSS_COMBAT_OBJECT_ORIGIN_POLICY::BOSS_POSITION !=
									combatObject->second.eOriginPolicy) ||
							  BOSS_COMBAT_OBJECT_VOLLEY_POLICY::NONE ==
									action.Volley.ePolicy)) ||
							(!fixedArea &&
							 BOSS_PATTERN_AIM_POLICY::LOCK_FACING_ON_START !=
								pattern.eAimPolicy))
						{
							m_strStatus =
								"Boss combat object spawn action join is invalid";
							return false;
						}
						continue;
					}
					if (BOSS_PATTERN_STAGE_ACTION_KIND::DAMAGE_GRABBED_PLAYERS ==
						action.eKind && 0u == Find_DamageRatePercent(action.strTargetId))
					{
						m_strStatus = "Boss grabbed-player damage profile is missing";
						return false;
					}
					if ((BOSS_PATTERN_STAGE_ACTION_KIND::DAMAGE_GRABBED_PLAYERS == action.eKind ||
						BOSS_PATTERN_STAGE_ACTION_KIND::EXECUTE_GRABBED_PLAYERS == action.eKind) &&
						(stage.Actions.size() != 1u || stage.eHitShape != BOSS_PATTERN_HIT_SHAPE::NONE))
					{
						m_strStatus = "Boss grabbed-player impact must own its stage transaction";
						return false;
					}
					if (!IsStatefulBossPatternStageAction(action.eKind))
						continue;
					const std::string stateKey =
						BuildBossPatternStageActionStateKey(
							action.eKind, action.strTargetId);
					if (BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER == action.eTrigger)
						validActions = activeStageActions.insert(stateKey).second &&
							validActions;
					else
						validActions = 1u == activeStageActions.erase(stateKey) &&
							validActions;
				}
				const bool hasCounterHit = std::any_of(
					stage.Branches.begin(), stage.Branches.end(),
					[](const BOSS_PATTERN_STAGE_BRANCH& branch)
					{
						return BOSS_PATTERN_STAGE_OUTCOME::COUNTER_HIT ==
							branch.eOutcome;
					});
				const bool hasPartDestroyed = std::any_of(
					stage.Branches.begin(), stage.Branches.end(),
					[](const BOSS_PATTERN_STAGE_BRANCH& branch)
					{
						return BOSS_PATTERN_STAGE_OUTCOME::PART_DESTROYED ==
							branch.eOutcome;
					});
				const bool hasStaggerBroken = std::any_of(
					stage.Branches.begin(), stage.Branches.end(),
					[](const BOSS_PATTERN_STAGE_BRANCH& branch)
					{
						return BOSS_PATTERN_STAGE_OUTCOME::STAGGER_BROKEN ==
							branch.eOutcome;
					});
				const auto hasStageAction = [&stage](
					const BOSS_PATTERN_STAGE_ACTION_TRIGGER trigger,
					const BOSS_PATTERN_STAGE_ACTION_KIND kind,
					const std::string_view targetId)
				{
					return std::any_of(stage.Actions.begin(), stage.Actions.end(),
						[trigger, kind, targetId](
							const BOSS_PATTERN_STAGE_ACTION& action)
						{
							return trigger == action.eTrigger && kind == action.eKind &&
								targetId == action.strTargetId;
						});
				};
				const auto hasBossFlagValue = [&stage](
					const BOSS_PATTERN_STAGE_ACTION_TRIGGER trigger,
					const std::string_view targetId,
					const std::uint32_t value)
				{
					return std::any_of(stage.Actions.begin(), stage.Actions.end(),
						[trigger, targetId, value](
							const BOSS_PATTERN_STAGE_ACTION& action)
						{
							return trigger == action.eTrigger &&
								BOSS_PATTERN_STAGE_ACTION_KIND::SET_BOSS_FLAG ==
									action.eKind &&
								targetId == action.strTargetId &&
								value == action.iValue;
						});
				};
				const auto hasBossFlag = [&stage](const std::string_view targetId)
				{
					return std::any_of(stage.Actions.begin(), stage.Actions.end(),
						[targetId](const BOSS_PATTERN_STAGE_ACTION& action)
						{
							return BOSS_PATTERN_STAGE_ACTION_KIND::SET_BOSS_FLAG ==
									action.eKind && targetId == action.strTargetId;
						});
				};
				const bool hasCounterableFlag = hasBossFlag("boss.flag.counterable");
				const bool hasGroggyFlag = hasBossFlag("boss.flag.groggy");
				if (pattern.bAuthoringMasterManaged && hasCounterHit)
				{
					const auto counterBranch = std::find_if(
						stage.Branches.begin(), stage.Branches.end(),
						[](const BOSS_PATTERN_STAGE_BRANCH& branch)
						{
							return BOSS_PATTERN_STAGE_OUTCOME::COUNTER_HIT ==
								branch.eOutcome;
						});
					const auto counterTarget = counterBranch == stage.Branches.end() ?
						pattern.Stages.end() : std::find_if(
							pattern.Stages.begin(), pattern.Stages.end(),
							[&counterBranch](
								const BOSS_PATTERN_STAGE_DEFINITION& candidate)
							{
								return candidate.strActionId ==
									counterBranch->strNextActionId;
							});
					const auto timeoutBranch = std::find_if(
						stage.Branches.begin(), stage.Branches.end(),
						[](const BOSS_PATTERN_STAGE_BRANCH& branch)
						{
							return BOSS_PATTERN_STAGE_OUTCOME::TIMEOUT ==
								branch.eOutcome;
						});
					const auto timeoutTarget = timeoutBranch == stage.Branches.end() ?
						pattern.Stages.end() : std::find_if(
							pattern.Stages.begin(), pattern.Stages.end(),
							[&timeoutBranch](
								const BOSS_PATTERN_STAGE_DEFINITION& candidate)
							{
								return candidate.strActionId ==
									timeoutBranch->strNextActionId;
							});
					const std::size_t sourceIndex = static_cast<std::size_t>(
						&stage - pattern.Stages.data());
					const bool counterTargetIsForward =
						counterTarget != pattern.Stages.end() &&
						static_cast<std::size_t>(
							counterTarget - pattern.Stages.begin()) > sourceIndex;
					const bool timeoutTargetIsForward =
						timeoutTarget != pattern.Stages.end() &&
						static_cast<std::size_t>(
							timeoutTarget - pattern.Stages.begin()) > sourceIndex;
					const bool counterTargetKindIsSupported =
						counterTarget != pattern.Stages.end() &&
						(BOSS_PATTERN_STAGE_KIND::WINDUP ==
							counterTarget->eStageKind ||
						 BOSS_PATTERN_STAGE_KIND::GROGGY ==
							counterTarget->eStageKind ||
						 BOSS_PATTERN_STAGE_KIND::RECOVERY ==
							counterTarget->eStageKind);
					const bool counterTargetIsGroggy =
						counterTarget != pattern.Stages.end() &&
						BOSS_PATTERN_STAGE_KIND::GROGGY ==
							counterTarget->eStageKind;
					const auto targetHasGroggyAction =
						[&counterTarget, &pattern](
							const BOSS_PATTERN_STAGE_ACTION_TRIGGER trigger,
							const std::uint32_t value)
					{
							return counterTarget != pattern.Stages.end() &&
								std::any_of(
									counterTarget->Actions.begin(),
									counterTarget->Actions.end(),
									[trigger, value](
										const BOSS_PATTERN_STAGE_ACTION& action)
									{
										return trigger == action.eTrigger &&
											BOSS_PATTERN_STAGE_ACTION_KIND::SET_BOSS_FLAG ==
												action.eKind &&
											"boss.flag.groggy" == action.strTargetId &&
											value == action.iValue;
									});
						};
					validBranches = validBranches &&
						BOSS_PATTERN_STAGE_KIND::WINDUP == stage.eStageKind &&
						counterTargetIsForward && timeoutTargetIsForward &&
						counterTargetKindIsSupported;
					validActions = validActions &&
						hasBossFlagValue(
							BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER,
							"boss.flag.counterable", 1u) &&
						hasBossFlagValue(
							BOSS_PATTERN_STAGE_ACTION_TRIGGER::EXIT,
							"boss.flag.counterable", 0u) &&
						(!counterTargetIsGroggy ||
						 (targetHasGroggyAction(
							 BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER, 1u) &&
						  targetHasGroggyAction(
							  BOSS_PATTERN_STAGE_ACTION_TRIGGER::EXIT, 0u)));
				}
				if (pattern.bAuthoringMasterManaged)
				{
					if (!hasCounterHit && hasCounterableFlag)
						validActions = false;
					if (BOSS_PATTERN_STAGE_KIND::GROGGY == stage.eStageKind)
					{
						validActions = validActions &&
							hasBossFlagValue(
								BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER,
								"boss.flag.groggy", 1u) &&
							hasBossFlagValue(
								BOSS_PATTERN_STAGE_ACTION_TRIGGER::EXIT,
								"boss.flag.groggy", 0u);
					}
					else if (hasGroggyFlag)
					{
						validActions = false;
					}
				}
				if ((stage.bHasCounterProxy && !hasCounterHit) ||
					(BOSS_PATTERN_PART_DAMAGE_POLICY::DESTROY_FIRST_ELIGIBLE ==
						stage.ePartDamagePolicy && !hasPartDestroyed))
				{
					validBranches = false;
				}
				if (hasStaggerBroken)
				{
					validActions = validActions &&
						hasStageAction(BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER,
							BOSS_PATTERN_STAGE_ACTION_KIND::SET_STAGGER_GAUGE,
							"boss.gauge.stagger") &&
						hasStageAction(BOSS_PATTERN_STAGE_ACTION_TRIGGER::EXIT,
							BOSS_PATTERN_STAGE_ACTION_KIND::SET_STAGGER_GAUGE,
							"boss.gauge.stagger");
				}
				if (!validBranches || !validActions)
				{
					m_strStatus =
						"Boss pattern stage branch or action contract is invalid";
					return false;
				}
			}
			if (!activeStageActions.empty())
			{
				m_strStatus = "Boss pattern stage action lifetime is not closed";
				return false;
			}
		}
		const bool expectsAuthoredPhaseAction =
			BOSS_PHASE_POLICY_KIND::AUTHORED_PATTERN_EVENT ==
				boss.PhasePolicy.eKind;
		if ((expectsAuthoredPhaseAction && 1u != gameplayPhaseActionCount) ||
			(!expectsAuthoredPhaseAction && 0u != gameplayPhaseActionCount))
		{
			m_strStatus =
				"Boss phase policy and gameplay phase actions differ";
			return false;
		}
		if (expectsAuthoredPhaseAction && "BOSS_VALTAN" == archetypeId &&
			"ENCOUNTER_VALTAN" == boss.strEncounterId &&
			!hasExactValtanArenaBreakPhaseAction)
		{
			m_strStatus =
				"Valtan authored phase action is not the 109 IMPACT ENTER edge";
			return false;
		}
		if (expectsAuthoredPhaseAction && "BOSS_VALTAN" == archetypeId &&
			"ENCOUNTER_VALTAN" == boss.strEncounterId &&
			(1u != valtanHighJumpTypedVolleyCount ||
			 !hasExactValtanHighJumpTypedVolley))
		{
			m_strStatus =
				"Valtan high jump does not own the typed AIRBORNE volley";
			return false;
		}
	}
	std::unordered_set<std::string> bossCombatObjectVisualIds;
	for (const auto& [combatObjectArchetypeId, combatObject] :
		m_BossCombatObjects)
	{
		(void)combatObjectArchetypeId;
		if (combatObject.Hits.size() + combatObject.PresentationPulses.size() !=
				combatObject.iExpectedEventCount ||
			!bossCombatObjectVisualIds.insert(combatObject.strClientVisualId).second)
		{
			m_strStatus = "Boss combat object event count or visual ID is invalid";
			return false;
		}
		for (const BOSS_COMBAT_OBJECT_HIT& hit : combatObject.Hits)
		{
			if (0u == Find_DamageRatePercent(hit.strDamageProfileId))
			{
				m_strStatus =
					"Boss combat object references missing damage profile";
				return false;
			}
		}
	}
	if (spawnedBossCombatObjectIds.size() != m_BossCombatObjects.size())
	{
		m_strStatus = "Boss combat object definitions and spawn actions differ";
		return false;
	}
	std::size_t patternCount = 0u;
	for (const auto& [encounterId, patterns] : m_BossPatterns)
	{
		(void)encounterId;
		patternCount += patterns.size();
	}
	if (patternPolicyOwners.size() != patternCount)
	{
		m_strStatus = "Boss pattern policies are incomplete";
		return false;
	}
	for (const auto& [encounterId, timeline] : m_ValtanTimelines)
	{
		const BOSS_RUNTIME_PROFILE* ownerBoss = nullptr;
		std::size_t ownerBossCount = 0u;
		for (const auto& [archetypeId, boss] : m_Bosses)
		{
			if (boss.strEncounterId != encounterId ||
				dependentBossArchetypeIds.contains(archetypeId))
				continue;
			ownerBoss = &boss;
			++ownerBossCount;
		}
		const auto patterns = m_BossPatterns.find(encounterId);
		if (1u != ownerBossCount || nullptr == ownerBoss ||
			m_BossPatterns.end() == patterns ||
			timeline.strEncounterId != encounterId ||
			timeline.strTimelineId.empty() || timeline.Rows.empty())
		{
			m_strStatus = "Valtan timeline owner contract is invalid";
			return false;
		}
		std::unordered_set<std::string> rowIds;
		std::unordered_set<std::uint32_t> commandIds;
		VALTAN_TIMELINE_ARENA_STATE previousArenaState =
			VALTAN_TIMELINE_ARENA_STATE::FRESH;
		std::uint32_t previousSectionHealthBar = ownerBoss->iMaximumHealthBars;
		for (std::size_t index = 0u; index < timeline.Rows.size(); ++index)
		{
			const VALTAN_TIMELINE_ROW& row = timeline.Rows[index];
			const std::uint8_t arenaRank =
				static_cast<std::uint8_t>(row.eArenaState);
			const std::uint8_t previousArenaRank =
				static_cast<std::uint8_t>(previousArenaState);
			if (row.iOrdinal != index + 1u || !IsStableId(row.strRowId) ||
				!rowIds.insert(row.strRowId).second ||
				0u == row.iCommandId ||
				row.iCommandId !=
					Calculate_ValtanTimelineCommandId(row.strRowId) ||
				!commandIds.insert(row.iCommandId).second ||
				0u == row.iSectionHealthBar ||
				row.iSectionHealthBar > ownerBoss->iMaximumHealthBars ||
				row.iSectionHealthBar > previousSectionHealthBar ||
				row.PatternActions.empty() || row.PatternActions.size() > 8u ||
				arenaRank < previousArenaRank ||
				arenaRank > previousArenaRank + 1u ||
				(0u == index &&
				 (row.iSectionHealthBar != ownerBoss->iMaximumHealthBars ||
				  VALTAN_TIMELINE_ARENA_STATE::FRESH != row.eArenaState)))
			{
				m_strStatus = "Valtan timeline occurrence contract is invalid";
				return false;
			}
			previousArenaState = row.eArenaState;
			previousSectionHealthBar = row.iSectionHealthBar;
			for (const VALTAN_TIMELINE_PATTERN_ACTION& action : row.PatternActions)
			{
				if (!IsStableId(action.strPatternId) || 0u == action.iRepeat ||
					action.iRepeat > 4u ||
					patterns->second.end() == std::find_if(
						patterns->second.begin(), patterns->second.end(),
						[&action](const BOSS_PATTERN_DEFINITION& candidate)
						{
							return candidate.strPatternId == action.strPatternId;
						}))
				{
					m_strStatus = "Valtan timeline pattern join is invalid";
					return false;
				}
			}
		}
	}

	/* The intro is the only automatic selector edge not represented by a
	rotation row. Keep it normal-only so an AUDITION_ONLY stable ID can never be
	promoted into combat by changing ENCOUNTERINTRO alone. */
	for (const auto& [encounterId, introPatternId] :
		m_IntroPatternIdByEncounter)
	{
		const auto patterns = m_BossPatterns.find(encounterId);
		if (m_BossPatterns.end() == patterns ||
			patterns->second.end() == std::find_if(
				patterns->second.begin(), patterns->second.end(),
				[&introPatternId](const BOSS_PATTERN_DEFINITION& pattern)
				{
					return pattern.strPatternId == introPatternId &&
						BOSS_PATTERN_SELECTION::NORMAL == pattern.eSelection;
				}))
		{
			m_strStatus = "Encounter intro names no normal pattern";
			return false;
		}
	}

	for (const auto& [sequenceEncounterId, sequence] :
		m_BossPatternSequences)
	{
		const auto patterns = m_BossPatterns.find(sequenceEncounterId);
		if (m_BossPatterns.end() == patterns ||
			sequence.strEncounterId != sequenceEncounterId ||
			sequence.strSequenceId.empty() ||
			BOSS_PATTERN_SEQUENCE_MODE::ORDERED_ONCE_THEN_IDLE !=
				sequence.eMode ||
			sequence.PatternIds.size() != sequence.iExpectedStepCount)
		{
			m_strStatus = "Boss pattern sequence tagged shape is incomplete";
			return false;
		}
		// Ordinals own occurrences: the same authored pattern may run again.
		for (const std::string& patternId : sequence.PatternIds)
		{
			const auto pattern = std::find_if(
				patterns->second.begin(), patterns->second.end(),
				[&patternId](const BOSS_PATTERN_DEFINITION& candidate)
				{ return candidate.strPatternId == patternId; });
			if (patterns->second.end() == pattern)
			{
				m_strStatus = "Boss pattern sequence contains an unknown step";
				return false;
			}
		}
	}

	for (const auto& [rotationEncounterId, rotations] : m_BossPatternRotations)
	{
		(void)rotationEncounterId;
		for (std::size_t rotationIndex = 0u;
			rotationIndex < rotations.size(); ++rotationIndex)
		{
			const BOSS_PATTERN_ROTATION_DEFINITION& rotation =
				rotations[rotationIndex];
			if (BOSS_PATTERN_ROTATION_SELECTION_MODE::WEIGHTED_POOL ==
				rotation.eSelectionMode)
			{
				if (!rotation.Window.Is_Defined() ||
					!rotation.PatternIds.empty() ||
					rotation.Candidates.size() != rotation.iExpectedStepCount ||
					rotation.Window.iExpectedCandidateCount !=
						rotation.iExpectedStepCount ||
					rotation.Window.iFromHealthBar != rotation.iFromHealthBar ||
					rotation.Window.iToHealthBar != rotation.iToHealthBar)
				{
					m_strStatus =
						"Boss managed weighted rotation tagged shape is incomplete";
					return false;
				}
				std::unordered_set<std::string> uniquePoolIds;
				bool hasEnabledCandidate = false;
				for (const BOSS_PATTERN_ROTATION_CANDIDATE& candidate :
					rotation.Candidates)
				{
					if (!uniquePoolIds.insert(candidate.strPatternId).second)
					{
						m_strStatus =
							"Boss weighted pattern pool contains a duplicate";
						return false;
					}
					hasEnabledCandidate = hasEnabledCandidate || candidate.bEnabled;
				}
				if (!hasEnabledCandidate)
				{
					m_strStatus =
						"Boss managed weighted rotation has no enabled candidate";
					return false;
				}
			}
			else if (rotation.Window.Is_Defined() ||
				!rotation.Candidates.empty() ||
				rotation.PatternIds.size() != rotation.iExpectedStepCount)
			{
				/* Legacy order intentionally preserves duplicate steps. Only the
				tagged shape and exact ordinal count are constrained. */
				m_strStatus =
					"Boss legacy ordered rotation tagged shape is incomplete";
				return false;
			}

			for (std::size_t priorIndex = 0u;
				priorIndex < rotationIndex; ++priorIndex)
			{
				const BOSS_PATTERN_ROTATION_DEFINITION& prior =
					rotations[priorIndex];
				const bool rangesOverlap =
					rotation.iFromHealthBar > prior.iToHealthBar &&
					prior.iFromHealthBar > rotation.iToHealthBar;
				if (!rangesOverlap) continue;
				const bool bothManaged =
					BOSS_PATTERN_ROTATION_SELECTION_MODE::WEIGHTED_POOL ==
						rotation.eSelectionMode &&
					BOSS_PATTERN_ROTATION_SELECTION_MODE::WEIGHTED_POOL ==
						prior.eSelectionMode;
				if (!bothManaged || rotation.Window.iGameplayPhase ==
					prior.Window.iGameplayPhase)
				{
					m_strStatus =
						"Boss pattern rotation spans overlap in one lookup domain";
					return false;
				}
			}
		}
	}

	/* Phase-boundary edits are not an independent HOT_RELOAD domain. Until one
	atomic SET_PHASE_BOUNDARY transaction owns the mechanic and both adjacent
	rotation partitions, direct bootstrap admission must prove the published
	Valtan topology is coherent. */
	const auto valtanRotations =
		m_BossPatternRotations.find("ENCOUNTER_VALTAN");
	const auto valtanSequence =
		m_BossPatternSequences.find("ENCOUNTER_VALTAN");
	const auto valtanPatterns = m_BossPatterns.find("ENCOUNTER_VALTAN");
	const auto valtanBoss = m_Bosses.find("BOSS_VALTAN");
	const bool expectsAuthoredValtanTopology =
		m_Bosses.end() != valtanBoss &&
		"ENCOUNTER_VALTAN" == valtanBoss->second.strEncounterId &&
		BOSS_PHASE_POLICY_KIND::AUTHORED_PATTERN_EVENT ==
			valtanBoss->second.PhasePolicy.eKind;
	if (expectsAuthoredValtanTopology &&
		(m_BossPatternRotations.end() == valtanRotations ||
		 m_BossPatternSequences.end() == valtanSequence ||
		 m_BossPatterns.end() == valtanPatterns))
	{
		/* Missing the exclusive program must not silently revive the legacy
		intro/rotation/global selector, where disabled Product patterns such as
		Dash could become automatic again. */
		m_strStatus =
			"Valtan authored phase topology has no exclusive pattern sequence";
		return false;
	}
	if (m_BossPatternRotations.end() != valtanRotations &&
		m_BossPatterns.end() != valtanPatterns)
	{
		std::uint32_t finalPhaseOneBoundary =
			(std::numeric_limits<std::uint32_t>::max)();
		std::uint32_t firstLegacyFromBar = 0u;
		for (const BOSS_PATTERN_ROTATION_DEFINITION& rotation :
			valtanRotations->second)
		{
			if (BOSS_PATTERN_ROTATION_SELECTION_MODE::WEIGHTED_POOL ==
				rotation.eSelectionMode && 1u == rotation.Window.iGameplayPhase)
			{
				finalPhaseOneBoundary = (std::min)(
					finalPhaseOneBoundary, rotation.Window.iToHealthBar);
			}
			else if (BOSS_PATTERN_ROTATION_SELECTION_MODE::
				ORDERED_INTRO_THEN_WEIGHTED == rotation.eSelectionMode)
			{
				firstLegacyFromBar = (std::max)(
					firstLegacyFromBar, rotation.iFromHealthBar);
			}
		}
		std::uint32_t phaseTransitionBar = 0u;
		std::uint32_t phaseTransitionCount = 0u;
		for (const BOSS_PATTERN_DEFINITION& pattern : valtanPatterns->second)
		{
			if (BOSS_PATTERN_SELECTION::HEALTH_BAR != pattern.eSelection)
				continue;
			for (const BOSS_PATTERN_STAGE_DEFINITION& stage : pattern.Stages)
			{
				for (const BOSS_PATTERN_STAGE_ACTION& action : stage.Actions)
				{
					if (BOSS_PATTERN_STAGE_ACTION_KIND::SET_GAMEPLAY_PHASE !=
						action.eKind || 2u != action.iValue)
					{
						continue;
					}
					phaseTransitionBar = pattern.iTriggerHealthBar;
					++phaseTransitionCount;
				}
			}
		}
		if (1u != phaseTransitionCount ||
			(std::numeric_limits<std::uint32_t>::max)() ==
				finalPhaseOneBoundary || 0u == firstLegacyFromBar ||
			phaseTransitionBar != finalPhaseOneBoundary ||
			phaseTransitionBar != firstLegacyFromBar + 1u)
		{
			m_strStatus =
				"Valtan phase-transition mechanic and rotation topology diverge";
			return false;
		}
	}

	rollback.committed = true;
	m_ActiveRevision = nullptr == parentRevision ?
		admittedRevision : *parentRevision;
	m_strStatus = nullptr == parentRevision ?
		"Loaded gameplay bootstrap" :
		"Loaded staged gameplay parent revision";
	return true;
}

const LostArk::Server::PLAYER_RUNTIME_PROFILE*
LostArk::Server::CGameplayCatalog::Find_Player(
	const LostArk::Shared::CHARACTER_CLASS_ID characterClass) const
{
	const auto iter = m_Players.find(characterClass);
	return m_Players.end() == iter ? nullptr : &iter->second;
}

const LostArk::Server::PLAYER_SKILL_DEFINITION*
LostArk::Server::CGameplayCatalog::Find_Skill(
	const LostArk::Shared::SKILL_ID skillId) const
{
	const auto iter = m_Skills.find(skillId);
	return m_Skills.end() == iter ? nullptr : &iter->second;
}

const LostArk::Server::BOSS_RUNTIME_PROFILE*
LostArk::Server::CGameplayCatalog::Find_Boss(
	const std::string& archetypeId) const
{
	const auto iter = m_Bosses.find(archetypeId);
	return m_Bosses.end() == iter ? nullptr : &iter->second;
}

const std::vector<LostArk::Server::BOSS_PART_DEFINITION>*
LostArk::Server::CGameplayCatalog::Find_BossParts(
	const std::string& archetypeId) const
{
	const auto iter = m_BossParts.find(archetypeId);
	return m_BossParts.end() == iter ? nullptr : &iter->second;
}

const std::vector<LostArk::Server::BOSS_PATTERN_DEFINITION>*
LostArk::Server::CGameplayCatalog::Find_BossPatterns(
	const std::string& encounterId) const
{
	const auto iter = m_BossPatterns.find(encounterId);
	return m_BossPatterns.end() == iter ? nullptr : &iter->second;
}

const LostArk::Server::BOSS_COMBAT_OBJECT_DEFINITION*
LostArk::Server::CGameplayCatalog::Find_BossCombatObject(
	const std::string& combatObjectArchetypeId) const
{
	const auto iter = m_BossCombatObjects.find(combatObjectArchetypeId);
	return m_BossCombatObjects.end() == iter ? nullptr : &iter->second;
}

const LostArk::Server::VALTAN_TIMELINE_DEFINITION*
LostArk::Server::CGameplayCatalog::Find_ValtanTimeline(
	const std::string& encounterId) const
{
	const auto iter = m_ValtanTimelines.find(encounterId);
	return m_ValtanTimelines.end() == iter ? nullptr : &iter->second;
}

const LostArk::Server::VALTAN_TIMELINE_ROW*
LostArk::Server::CGameplayCatalog::Find_ValtanTimelineRow(
	const std::string& encounterId,
	const std::uint32_t commandId) const
{
	if (0u == commandId)
		return nullptr;
	const VALTAN_TIMELINE_DEFINITION* timeline =
		Find_ValtanTimeline(encounterId);
	if (nullptr == timeline)
		return nullptr;
	const auto row = std::find_if(
		timeline->Rows.begin(), timeline->Rows.end(),
		[commandId](const VALTAN_TIMELINE_ROW& candidate)
		{
			return candidate.iCommandId == commandId;
		});
	return timeline->Rows.end() == row ? nullptr : &*row;
}

const LostArk::Server::BOSS_PATTERN_ROTATION_DEFINITION*
LostArk::Server::CGameplayCatalog::Find_BossPatternRotation(
	const std::string& encounterId,
	const std::uint32_t gameplayPhase,
	const std::uint32_t healthBar) const
{
	const auto iter = m_BossPatternRotations.find(encounterId);
	if (m_BossPatternRotations.end() == iter)
		return nullptr;
	for (const BOSS_PATTERN_ROTATION_DEFINITION& rotation : iter->second)
	{
		/* Managed windows are phase-owned. Legacy ORDERED rows predate that
		contract and remain phase-agnostic so the post-109 phase-two script keeps
		its authored order. */
		if (BOSS_PATTERN_ROTATION_SELECTION_MODE::WEIGHTED_POOL ==
			rotation.eSelectionMode && rotation.Window.iGameplayPhase !=
			gameplayPhase)
		{
			continue;
		}
		/* Bars count down. A span owns the stretch at and below its opening
		bar and stops on the bar the next scripted mechanic sits on. */
		if (healthBar <= rotation.iFromHealthBar &&
			healthBar > rotation.iToHealthBar)
		{
			return &rotation;
		}
	}
	return nullptr;
}

const LostArk::Server::BOSS_PATTERN_SEQUENCE_DEFINITION*
LostArk::Server::CGameplayCatalog::Find_BossPatternSequence(
	const std::string& encounterId) const
{
	const auto iter = m_BossPatternSequences.find(encounterId);
	return m_BossPatternSequences.end() == iter ? nullptr : &iter->second;
}

const std::string& LostArk::Server::CGameplayCatalog::Find_IntroPatternId(
	const std::string& encounterId) const
{
	static const std::string EMPTY;
	const auto iter = m_IntroPatternIdByEncounter.find(encounterId);
	return m_IntroPatternIdByEncounter.end() == iter ? EMPTY : iter->second;
}

std::uint32_t LostArk::Server::CGameplayCatalog::Find_DamageRatePercent(
	const std::string& damageProfileId) const
{
	const auto iter = m_DamageRatePercentByProfileId.find(damageProfileId);
	return m_DamageRatePercentByProfileId.end() == iter ? 0u : iter->second;
}

std::uint32_t LostArk::Server::CGameplayCatalog::Resolve_Damage(
	const std::uint32_t attackPower,
	const std::uint32_t damageRatePercent)
{
	if (0u == attackPower || 0u == damageRatePercent)
		return 0u;
	const std::uint64_t scaled =
		(static_cast<std::uint64_t>(attackPower) *
			static_cast<std::uint64_t>(damageRatePercent)) / 100ull;
	return scaled < 1ull ? 1u :
		static_cast<std::uint32_t>((std::min<std::uint64_t>)(
			scaled, (std::numeric_limits<std::uint32_t>::max)()));
}

std::uint32_t LostArk::Server::CGameplayCatalog::Apply_Defense(
	const std::uint32_t rawDamage,
	const std::uint32_t defense)
{
	if (0u == rawDamage)
		return 0u;
	const std::uint64_t mitigated =
		(static_cast<std::uint64_t>(rawDamage) * 100ull) /
		(100ull + static_cast<std::uint64_t>(defense));
	return mitigated < 1ull ? 1u :
		static_cast<std::uint32_t>((std::min<std::uint64_t>)(
			mitigated, (std::numeric_limits<std::uint32_t>::max)()));
}
