#include "GameplayCatalog.h"

#include <Windows.h>

#include <algorithm>
#include <charconv>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <limits>
#include <string_view>
#include <unordered_set>
#include <vector>

namespace
{
	/* Mirror of the publisher's $maximumDamageRatePercent: a rate only one side
	accepts would make Validate and Load disagree about the same document. */
	constexpr std::uint32_t MAXIMUM_DAMAGE_RATE_PERCENT = 100000u;

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
		else
			return false;
		return true;
	}
}

bool LostArk::Server::CGameplayCatalog::Parse_RootMotionSamples(
	const std::string_view packed,
	const std::uint32_t sampleCount,
	const std::uint32_t limitMs,
	std::vector<PLAYER_ROOT_MOTION_SAMPLE>& outSamples)
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
		PLAYER_ROOT_MOTION_SAMPLE sample{};
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
	using SKILL_MAP = decltype(m_Skills);
	using BOSS_MAP = decltype(m_Bosses);
	using PATTERN_MAP = decltype(m_BossPatterns);
	using PLAYER_MAP = decltype(m_Players);
	using DAMAGE_MAP = decltype(m_DamageRatePercentByProfileId);
	struct LOAD_ROLLBACK final
	{
		SKILL_MAP& skills;
		BOSS_MAP& bosses;
		PATTERN_MAP& patterns;
		PLAYER_MAP& players;
		DAMAGE_MAP& damages;
		SKILL_MAP previousSkills;
		BOSS_MAP previousBosses;
		PATTERN_MAP previousPatterns;
		PLAYER_MAP previousPlayers;
		DAMAGE_MAP previousDamages;
		bool committed = false;

		LOAD_ROLLBACK(
			SKILL_MAP& skillTarget,
			BOSS_MAP& bossTarget,
			PATTERN_MAP& patternTarget,
			PLAYER_MAP& playerTarget,
			DAMAGE_MAP& damageTarget)
			: skills(skillTarget)
			, bosses(bossTarget)
			, patterns(patternTarget)
			, players(playerTarget)
			, damages(damageTarget)
			, previousSkills(std::move(skillTarget))
			, previousBosses(std::move(bossTarget))
			, previousPatterns(std::move(patternTarget))
			, previousPlayers(std::move(playerTarget))
			, previousDamages(std::move(damageTarget))
		{
		}

		~LOAD_ROLLBACK()
		{
			if (committed)
				return;
			skills = std::move(previousSkills);
			bosses = std::move(previousBosses);
			patterns = std::move(previousPatterns);
			players = std::move(previousPlayers);
			damages = std::move(previousDamages);
		}
	};
	LOAD_ROLLBACK rollback{
		m_Skills, m_Bosses, m_BossPatterns, m_Players,
		m_DamageRatePercentByProfileId };
	m_Skills.clear();
	m_Bosses.clear();
	m_BossPatterns.clear();
	m_Players.clear();
	m_DamageRatePercentByProfileId.clear();

	const std::filesystem::path dataRoot = Resolve_DataRoot();
	const std::filesystem::path path = dataRoot / L"Gameplay" / L"Gameplay.bootstrap";
	std::ifstream input(path, std::ios::binary);
	if (dataRoot.empty() || !input)
	{
		m_strStatus = "Missing gameplay bootstrap: " + path.string();
		return false;
	}

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
		!ParseNumber(header[1], version) || 10u != version ||
		!ParseNumber(header[2], rowCount) || 0u == rowCount || rowCount > 4096u)
	{
		m_strStatus = "Gameplay bootstrap header is invalid";
		return false;
	}

	for (std::uint32_t row = 0; row < rowCount; ++row)
	{
		if (!std::getline(input, line))
		{
			m_strStatus = "Gameplay bootstrap row is truncated";
			return false;
		}
		StripCarriageReturn(line);
		const std::vector<std::string_view> fields = SplitTabs(line);
		if (!fields.empty() && "DAMAGE" == fields[0])
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
		else if (!fields.empty() && "SKILLSTAGE" == fields[0])
		{
			LostArk::Shared::SKILL_ID ownerSkillId =
				LostArk::Shared::INVALID_SKILL_ID;
			std::uint32_t stageIndex = 0;
			PLAYER_COMBO_STAGE stage{};
			if (7u != fields.size() ||
				!ParseNumber(fields[1], ownerSkillId) ||
				!ParseNumber(fields[2], stageIndex) ||
				!ParseNumber(fields[3], stage.iActionDurationMs) ||
				!ParseNumber(fields[4], stage.iHitTimeMs) ||
				!ParseNumber(fields[5], stage.iInputOpenMs) ||
				!ParseNumber(fields[6], stage.iInputCloseMs) ||
				0u == stage.iActionDurationMs ||
				stage.iHitTimeMs > stage.iActionDurationMs ||
				stage.iInputCloseMs > stage.iActionDurationMs)
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
			std::vector<PLAYER_ROOT_MOTION_SAMPLE> samples;
			if (!Parse_RootMotionSamples(
				fields[3], sampleCount, owner->second.iActionDurationMs, samples))
			{
				return false;
			}
			owner->second.RootMotion = std::move(samples);
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
			std::vector<PLAYER_ROOT_MOTION_SAMPLE> samples;
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
			if (10u != fields.size() || !IsStableId(fields[1]) ||
				!IsStableId(fields[2]) ||
				!ParseNumber(fields[3], boss.iMaximumHp) ||
				!ParseNumber(fields[4], boss.iMaximumHealthBars) ||
				!ParseNumber(fields[5], boss.iAttackPower) ||
				!ParseNumber(fields[6], boss.fCollisionRadius) ||
				!ParseNumber(fields[7], boss.fEngageDistance) ||
				!ParseNumber(fields[8], boss.fMoveSpeed) ||
				!ParseNumber(fields[9], boss.iPhaseTwoHpPercent) ||
				0u == boss.iMaximumHp || 0u == boss.iMaximumHealthBars ||
				boss.iMaximumHealthBars > 1000u || 0u == boss.iAttackPower ||
				!std::isfinite(boss.fCollisionRadius) ||
				boss.fCollisionRadius <= 0.f ||
				!std::isfinite(boss.fEngageDistance) ||
				!std::isfinite(boss.fMoveSpeed) || boss.fEngageDistance <= 0.f ||
				boss.fMoveSpeed <= 0.f || 0u == boss.iPhaseTwoHpPercent ||
				boss.iPhaseTwoHpPercent >= 100u)
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
		else if (!fields.empty() && "PATTERN" == fields[0])
		{
			BOSS_PATTERN_DEFINITION pattern{};
			if (14u != fields.size() || !IsStableId(fields[1]) ||
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
		else if (!fields.empty() && "PATTERNMOTION" == fields[0])
		{
			BOSS_PATTERN_MOTION motion{};
			if (9u != fields.size() || !IsStableId(fields[1]) ||
				!IsStableId(fields[2]) || "LEAP_TO_ANCHOR" != fields[3] ||
				!IsStableId(fields[4]) ||
				!ParseNumber(fields[5], motion.fLandingX) ||
				!ParseNumber(fields[6], motion.fLandingY) ||
				!ParseNumber(fields[7], motion.fLandingZ) ||
				!ParseNumber(fields[8], motion.fApexHeight) ||
				!std::isfinite(motion.fLandingX) ||
				!std::isfinite(motion.fLandingY) ||
				!std::isfinite(motion.fLandingZ) ||
				!std::isfinite(motion.fApexHeight) ||
				motion.fApexHeight <= 0.f || motion.fApexHeight > 200.f)
			{
				m_strStatus = "Boss pattern motion row is invalid";
				return false;
			}
			motion.eKind = BOSS_PATTERN_MOTION_KIND::LEAP_TO_ANCHOR;
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
			if (21u != fields.size() || !IsStableId(fields[1]) ||
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
				("-" != fields[16] && !IsStableId(fields[16])) ||
				!ParseNumber(fields[17], stage.fPushRangeM) ||
				!ParseNumber(fields[18], stage.iPushMs) ||
				!ParseNumber(fields[19], knockdownFlag) ||
				!ParseNumber(fields[20], stage.iDownMs) ||
				0u == stage.iDurationMs ||
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
				(0u == knockdownFlag && 0u != stage.iDownMs))
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
				"-" == fields[16] ? "" : std::string(fields[16]);
			owner->Stages.push_back(std::move(stage));
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
		m_Bosses.empty() || m_BossPatterns.empty() ||
		m_DamageRatePercentByProfileId.empty())
	{
		m_strStatus = "Gameplay bootstrap has trailing rows or missing definitions";
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
	for (const auto& [archetypeId, boss] : m_Bosses)
	{
		(void)archetypeId;
		const auto foundPatterns = m_BossPatterns.find(boss.strEncounterId);
		if (m_BossPatterns.end() == foundPatterns || foundPatterns->second.empty())
		{
			m_strStatus = "Boss encounter has no runtime patterns";
			return false;
		}
		for (const BOSS_PATTERN_DEFINITION& pattern : foundPatterns->second)
		{
			const bool isNormal = BOSS_PATTERN_SELECTION::NORMAL == pattern.eSelection;
			const bool validSelection = isNormal ?
				(pattern.iMinimumHealthBar >= 1u &&
					pattern.iMaximumHealthBar >= pattern.iMinimumHealthBar &&
					pattern.iMaximumHealthBar <= boss.iMaximumHealthBars &&
					0u == pattern.iTriggerHealthBar && 0u == pattern.iTriggerOrder &&
					pattern.iSelectionWeight > 0u &&
					pattern.iMaximumConsecutiveUses > 0u) :
				(0u == pattern.iMinimumHealthBar && 0u == pattern.iMaximumHealthBar &&
					pattern.iTriggerHealthBar >= 1u &&
					pattern.iTriggerHealthBar <= boss.iMaximumHealthBars &&
					pattern.iTriggerOrder > 0u && 0u == pattern.iSelectionWeight &&
					0u == pattern.iMaximumConsecutiveUses);
			if (!validSelection || 0u == pattern.iSourcePrimaryActionId ||
				pattern.Stages.size() != pattern.iExpectedStageCount)
			{
				m_strStatus = "Boss pattern selection or stage count is invalid";
				return false;
			}
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
					validShape = validShape && stage.iHitCount > 0u &&
						(1u == stage.iHitCount ? 0u == stage.iHitIntervalMs :
							stage.iHitIntervalMs > 0u) &&
						static_cast<std::uint64_t>(stage.iHitCount - 1u) *
							stage.iHitIntervalMs < stage.iDurationMs &&
						0u != Find_DamageRatePercent(stage.strDamageProfileId);
				}
				if (!validShape ||
					(stage.bWallContact &&
						(BOSS_PATTERN_STAGE_KIND::ACTIVE != stage.eStageKind ||
						 BOSS_PATTERN_HIT_SHAPE::NONE == stage.eHitShape ||
						 0u == stage.iHitCount)) ||
					std::any_of(pattern.Stages.begin() + stageIndex + 1u,
						pattern.Stages.end(),
						[&stage](const BOSS_PATTERN_STAGE_DEFINITION& other)
						{
							return stage.strStageId == other.strStageId;
						}))
				{
					m_strStatus = "Boss pattern stage hit contract is invalid";
					return false;
				}
			}
		}
	}

	rollback.committed = true;
	m_strStatus = "Loaded gameplay bootstrap";
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

const std::vector<LostArk::Server::BOSS_PATTERN_DEFINITION>*
LostArk::Server::CGameplayCatalog::Find_BossPatterns(
	const std::string& encounterId) const
{
	const auto iter = m_BossPatterns.find(encounterId);
	return m_BossPatterns.end() == iter ? nullptr : &iter->second;
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
