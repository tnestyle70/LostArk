#include "WorldDestructionDebrisPresentationDocument.h"

#include "DataJson.h"
#include "WorldDestructionProjectionDocument.h"

#include <algorithm>
#include <cctype>
#include <cfloat>
#include <charconv>
#include <cmath>
#include <fstream>
#include <initializer_list>
#include <sstream>
#include <unordered_set>

namespace
{
	using namespace Client;

	constexpr const char_t* SCHEMA =
		"lostark.world-destruction-debris-presentation";
	constexpr uint32_t FORMAT_VERSION = 1u;
	constexpr size_t MAX_PROFILE_COUNT = 128u;
	constexpr size_t MAX_EMITTER_COUNT = 4096u;
	constexpr size_t MAX_PROFILE_EMITTER_COUNT = 256u;
	constexpr size_t COMBAT_REVISION_HEX_LENGTH = 64u;
	constexpr f32_t MAX_ABSOLUTE_OFFSET = 1000.f;
	constexpr f32_t MAX_SPEED_METERS_PER_SECOND = 250.f;
	constexpr f32_t MAX_GRAVITY_SCALE = 10.f;
	constexpr f32_t MAX_LIFETIME_SECONDS = 60.f;
	constexpr f32_t DIRECTION_LENGTH_TOLERANCE = 0.001f;

	bool_t Is_ExactObject(
		const DATA_JSON_VALUE& value,
		const std::initializer_list<const char_t*> keys)
	{
		if (!value.Is_Object() || value.Get_Object().size() != keys.size())
			return false;
		return std::all_of(keys.begin(), keys.end(),
			[&value](const char_t* key) { return nullptr != value.Find(key); });
	}

	bool_t Read_String(
		const DATA_JSON_VALUE& parent,
		const char_t* key,
		std::string& outValue)
	{
		const DATA_JSON_VALUE* value = parent.Find(key);
		if (nullptr == value || !value->Is_String() || value->Get_String().empty())
			return false;
		outValue = value->Get_String();
		return true;
	}

	bool_t Read_Number(
		const DATA_JSON_VALUE& parent,
		const char_t* key,
		f32_t& outValue)
	{
		const DATA_JSON_VALUE* value = parent.Find(key);
		if (nullptr == value || !value->Is_Number() ||
			!std::isfinite(value->Get_Number()) ||
			value->Get_Number() < -static_cast<double>(FLT_MAX) ||
			value->Get_Number() > static_cast<double>(FLT_MAX))
		{
			return false;
		}
		outValue = static_cast<f32_t>(value->Get_Number());
		return std::isfinite(outValue);
	}

	bool_t Read_Float3(
		const DATA_JSON_VALUE& parent,
		const char_t* key,
		float3_t& outValue)
	{
		const DATA_JSON_VALUE* value = parent.Find(key);
		if (nullptr == value || !value->Is_Array() ||
			3u != value->Get_Array().size())
		{
			return false;
		}
		for (const DATA_JSON_VALUE& component : value->Get_Array())
		{
			if (!component.Is_Number() ||
				!std::isfinite(component.Get_Number()) ||
				component.Get_Number() < -static_cast<double>(FLT_MAX) ||
				component.Get_Number() > static_cast<double>(FLT_MAX))
			{
				return false;
			}
		}
		outValue = {
			static_cast<f32_t>(value->Get_Array()[0u].Get_Number()),
			static_cast<f32_t>(value->Get_Array()[1u].Get_Number()),
			static_cast<f32_t>(value->Get_Array()[2u].Get_Number())
		};
		return std::isfinite(outValue.x) && std::isfinite(outValue.y) &&
			std::isfinite(outValue.z);
	}

	bool_t Read_FormatVersion(const DATA_JSON_VALUE& root)
	{
		const DATA_JSON_VALUE* value = root.Find("formatVersion");
		return nullptr != value && value->Is_Number() &&
			value->Get_Number() == static_cast<double>(FORMAT_VERSION);
	}

	bool_t Is_StableId(const std::string_view value)
	{
		return !value.empty() && value.size() <= 128u &&
			std::all_of(value.begin(), value.end(), [](const unsigned char character)
			{
				return 0 != std::isalnum(character) || character == '_' ||
					character == '-' || character == '.';
			});
	}

	bool_t Is_Revision(const std::string_view value)
	{
		if (COMBAT_REVISION_HEX_LENGTH != value.size())
			return false;
		bool_t hasNonZero = false;
		for (const unsigned char character : value)
		{
			if (!((character >= '0' && character <= '9') ||
				(character >= 'a' && character <= 'f')))
			{
				return false;
			}
			hasNonZero = hasNonZero || character != '0';
		}
		return hasNonZero;
	}

	bool_t Parse_CanonicalPlacementId(
		const std::string_view text,
		uint64_t& outValue)
	{
		if (text.empty() || text.size() > 20u ||
			('0' == text.front() && text.size() != 1u))
		{
			return false;
		}
		uint64_t value = 0u;
		const auto result = std::from_chars(
			text.data(), text.data() + text.size(), value, 10);
		if (result.ec != std::errc{} || result.ptr != text.data() + text.size() ||
			0u == value)
		{
			return false;
		}
		outValue = value;
		return true;
	}

	bool_t Is_ValidEmitterVectors(
		const WORLD_DESTRUCTION_DEBRIS_EMITTER& emitter)
	{
		if (std::abs(emitter.vSpawnOffset.x) > MAX_ABSOLUTE_OFFSET ||
			std::abs(emitter.vSpawnOffset.y) > MAX_ABSOLUTE_OFFSET ||
			std::abs(emitter.vSpawnOffset.z) > MAX_ABSOLUTE_OFFSET)
		{
			return false;
		}
		const f32_t directionLength = std::sqrt(
			emitter.vDirection.x * emitter.vDirection.x +
			emitter.vDirection.y * emitter.vDirection.y +
			emitter.vDirection.z * emitter.vDirection.z);
		return std::isfinite(directionLength) &&
			std::abs(directionLength - 1.f) <= DIRECTION_LENGTH_TOLERANCE;
	}
}

bool_t Client::CWorldDestructionDebrisPresentationDocument::Load(
	const std::filesystem::path& path,
	std::string& outStatus)
{
	std::ifstream input(path, std::ios::binary);
	if (path.empty() || !input.is_open())
	{
		outStatus = "World destruction debris presentation is unreadable: " +
			path.string();
		return false;
	}
	std::ostringstream buffer;
	buffer << input.rdbuf();
	if (input.bad())
	{
		outStatus = "World destruction debris presentation read failed: " +
			path.string();
		return false;
	}
	return Parse_Text(buffer.str(), *this, outStatus);
}

bool_t Client::CWorldDestructionDebrisPresentationDocument::Parse_Text(
	const std::string_view text,
	CWorldDestructionDebrisPresentationDocument& outDocument,
	std::string& outStatus)
{
	DATA_JSON_VALUE root;
	std::string parseError;
	DATA_JSON_PARSE_LIMITS limits{};
	limits.iMaximumBytes = 2u * 1024u * 1024u;
	limits.iMaximumDepth = 12u;
	limits.iMaximumValues = 65536u;
	if (!CDataJson::Parse(text, root, parseError, limits))
	{
		outStatus = "World destruction debris presentation parse failed: " +
			parseError;
		return false;
	}
	if (!Is_ExactObject(root,
		{ "schema", "formatVersion", "areaId", "combatRuntimeRevision", "profiles" }))
	{
		outStatus = "World destruction debris presentation root has unexpected properties";
		return false;
	}

	std::string schema;
	std::string areaId;
	std::string revision;
	if (!Read_String(root, "schema", schema) || SCHEMA != schema ||
		!Read_FormatVersion(root) || !Read_String(root, "areaId", areaId) ||
		!Is_StableId(areaId) ||
		!Read_String(root, "combatRuntimeRevision", revision) ||
		!Is_Revision(revision))
	{
		outStatus = "World destruction debris presentation header is invalid";
		return false;
	}

	const DATA_JSON_VALUE* profiles = root.Find("profiles");
	if (nullptr == profiles || !profiles->Is_Array() ||
		profiles->Get_Array().empty() ||
		profiles->Get_Array().size() > MAX_PROFILE_COUNT)
	{
		outStatus = "World destruction debris presentation profiles are invalid";
		return false;
	}

	std::vector<WORLD_DESTRUCTION_DEBRIS_PROFILE> stagedProfiles;
	std::unordered_set<std::string> mutationIds;
	std::unordered_set<std::string> bindingIds;
	std::unordered_set<uint64_t> projectedPlacementIds;
	size_t totalEmitters = 0u;
	std::string previousGroupId;
	for (const DATA_JSON_VALUE& profileValue : profiles->Get_Array())
	{
		if (!Is_ExactObject(profileValue,
			{ "groupId", "mutationId", "bindingId", "emitters" }))
		{
			outStatus = "World destruction debris profile has unexpected properties";
			return false;
		}

		WORLD_DESTRUCTION_DEBRIS_PROFILE profile;
		if (!Read_String(profileValue, "groupId", profile.strGroupId) ||
			!Is_StableId(profile.strGroupId) ||
			(!previousGroupId.empty() && !(previousGroupId < profile.strGroupId)) ||
			!Read_String(profileValue, "mutationId", profile.strMutationId) ||
			!Is_StableId(profile.strMutationId) ||
			!mutationIds.insert(profile.strMutationId).second ||
			!Read_String(profileValue, "bindingId", profile.strBindingId) ||
			!Is_StableId(profile.strBindingId) ||
			!bindingIds.insert(profile.strBindingId).second)
		{
			outStatus = "World destruction debris profile identity is invalid";
			return false;
		}
		previousGroupId = profile.strGroupId;

		const DATA_JSON_VALUE* emitters = profileValue.Find("emitters");
		if (nullptr == emitters || !emitters->Is_Array() ||
			emitters->Get_Array().empty() ||
			emitters->Get_Array().size() > MAX_PROFILE_EMITTER_COUNT ||
			totalEmitters + emitters->Get_Array().size() > MAX_EMITTER_COUNT)
		{
			outStatus = "World destruction debris emitters are invalid";
			return false;
		}

		uint64_t previousSourcePlacementId = 0u;
		for (const DATA_JSON_VALUE& emitterValue : emitters->Get_Array())
		{
			if (!Is_ExactObject(emitterValue,
				{ "sourceRuntimePlacementId", "suppressionAliasPlacementIds",
				  "spawnOffset", "direction", "speedMetersPerSecond",
				  "gravityScale", "lifetimeSeconds" }))
			{
				outStatus = "World destruction debris emitter has unexpected properties";
				return false;
			}

			WORLD_DESTRUCTION_DEBRIS_EMITTER emitter;
			std::string sourcePlacementId;
			if (!Read_String(emitterValue, "sourceRuntimePlacementId",
				sourcePlacementId) ||
				!Parse_CanonicalPlacementId(sourcePlacementId,
					emitter.iSourceRuntimePlacementId) ||
				(0u != previousSourcePlacementId &&
					previousSourcePlacementId >= emitter.iSourceRuntimePlacementId) ||
				!projectedPlacementIds.insert(
					emitter.iSourceRuntimePlacementId).second)
			{
				outStatus = "World destruction debris source placement is invalid";
				return false;
			}
			previousSourcePlacementId = emitter.iSourceRuntimePlacementId;

			const DATA_JSON_VALUE* aliases =
				emitterValue.Find("suppressionAliasPlacementIds");
			if (nullptr == aliases || !aliases->Is_Array() ||
				aliases->Get_Array().size() > MAX_PROFILE_EMITTER_COUNT)
			{
				outStatus = "World destruction debris suppression aliases are invalid";
				return false;
			}
			uint64_t previousAliasPlacementId = 0u;
			for (const DATA_JSON_VALUE& aliasValue : aliases->Get_Array())
			{
				uint64_t aliasPlacementId = 0u;
				if (!aliasValue.Is_String() ||
					!Parse_CanonicalPlacementId(aliasValue.Get_String(),
						aliasPlacementId) ||
					(0u != previousAliasPlacementId &&
						previousAliasPlacementId >= aliasPlacementId) ||
					!projectedPlacementIds.insert(aliasPlacementId).second)
				{
					outStatus = "World destruction debris suppression alias is invalid";
					return false;
				}
				previousAliasPlacementId = aliasPlacementId;
				emitter.SuppressionAliasPlacementIds.push_back(aliasPlacementId);
			}

			if (!Read_Float3(emitterValue, "spawnOffset", emitter.vSpawnOffset) ||
				!Read_Float3(emitterValue, "direction", emitter.vDirection) ||
				!Read_Number(emitterValue, "speedMetersPerSecond",
					emitter.fSpeedMetersPerSecond) ||
				!Read_Number(emitterValue, "gravityScale",
					emitter.fGravityScale) ||
				!Read_Number(emitterValue, "lifetimeSeconds",
					emitter.fLifetimeSeconds) ||
				!Is_ValidEmitterVectors(emitter) ||
				emitter.fSpeedMetersPerSecond < 0.f ||
				emitter.fSpeedMetersPerSecond > MAX_SPEED_METERS_PER_SECOND ||
				emitter.fGravityScale < 0.f ||
				emitter.fGravityScale > MAX_GRAVITY_SCALE ||
				emitter.fLifetimeSeconds <= 0.f ||
				emitter.fLifetimeSeconds > MAX_LIFETIME_SECONDS)
			{
				outStatus = "World destruction debris emitter physics values are invalid";
				return false;
			}

			profile.Emitters.push_back(std::move(emitter));
		}
		totalEmitters += profile.Emitters.size();
		stagedProfiles.push_back(std::move(profile));
	}

	CWorldDestructionDebrisPresentationDocument committed;
	committed.m_strAreaId = std::move(areaId);
	committed.m_strCombatRuntimeRevision = std::move(revision);
	committed.m_Profiles = std::move(stagedProfiles);
	committed.m_isReady = true;
	outDocument = std::move(committed);
	outStatus = "Loaded world destruction debris presentation: " +
		std::to_string(outDocument.m_Profiles.size()) + " profiles, " +
		std::to_string(totalEmitters) + " emitters, " +
		std::to_string(projectedPlacementIds.size()) + " placements";
	return true;
}

bool_t Client::CWorldDestructionDebrisPresentationDocument::Validate_Against(
	const CWorldDestructionProjectionDocument& projection,
	std::string& outStatus) const
{
	if (!m_isReady || !projection.Is_Ready() ||
		m_strAreaId != projection.Get_AreaId() ||
		m_strCombatRuntimeRevision != projection.Get_CombatRuntimeRevision() ||
		m_Profiles.size() != projection.Get_Groups().size())
	{
		outStatus = "World destruction debris presentation/projection header mismatch";
		return false;
	}

	for (size_t profileIndex = 0u; profileIndex < m_Profiles.size(); ++profileIndex)
	{
		const WORLD_DESTRUCTION_DEBRIS_PROFILE& profile =
			m_Profiles[profileIndex];
		const WORLD_DESTRUCTION_PROJECTION_GROUP& group =
			projection.Get_Groups()[profileIndex];
		if (profile.strGroupId != group.strGroupId ||
			profile.strMutationId != group.strMutationId)
		{
			outStatus = "World destruction debris presentation group mismatch: " +
				profile.strGroupId;
			return false;
		}

		std::vector<uint64_t> projectedMembers;
		std::vector<uint64_t> suppressionAliases;
		for (const WORLD_DESTRUCTION_DEBRIS_EMITTER& emitter : profile.Emitters)
		{
			projectedMembers.push_back(emitter.iSourceRuntimePlacementId);
			projectedMembers.insert(projectedMembers.end(),
				emitter.SuppressionAliasPlacementIds.begin(),
				emitter.SuppressionAliasPlacementIds.end());
			suppressionAliases.insert(suppressionAliases.end(),
				emitter.SuppressionAliasPlacementIds.begin(),
				emitter.SuppressionAliasPlacementIds.end());
		}
		std::sort(projectedMembers.begin(), projectedMembers.end());
		std::sort(suppressionAliases.begin(), suppressionAliases.end());
		if (projectedMembers != group.MemberPlacementIds ||
			suppressionAliases != group.SuppressionAliasPlacementIds)
		{
			outStatus = "World destruction debris presentation coverage/alias mismatch: " +
				profile.strGroupId;
			return false;
		}
	}

	outStatus = "Validated world destruction debris presentation against " +
		std::to_string(m_Profiles.size()) + " projection groups";
	return true;
}

void Client::CWorldDestructionDebrisPresentationDocument::Clear()
{
	m_strAreaId.clear();
	m_strCombatRuntimeRevision.clear();
	m_Profiles.clear();
	m_isReady = false;
}

const Client::WORLD_DESTRUCTION_DEBRIS_PROFILE*
Client::CWorldDestructionDebrisPresentationDocument::Find_Group(
	const std::string_view groupId) const
{
	const auto iter = std::lower_bound(
		m_Profiles.begin(), m_Profiles.end(), groupId,
		[](const WORLD_DESTRUCTION_DEBRIS_PROFILE& profile,
			const std::string_view key)
		{
			return profile.strGroupId < key;
		});
	return iter != m_Profiles.end() && iter->strGroupId == groupId ?
		&*iter : nullptr;
}
