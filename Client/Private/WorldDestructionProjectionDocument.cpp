#include "WorldDestructionProjectionDocument.h"

#include "DataJson.h"

#include <algorithm>
#include <cctype>
#include <charconv>
#include <fstream>
#include <initializer_list>
#include <sstream>
#include <unordered_set>

namespace
{
	using namespace Client;

	constexpr const char_t* SCHEMA =
		"lostark.world-destruction-client-projection";
	constexpr uint32_t FORMAT_VERSION = 2u;
	constexpr size_t MAX_GROUP_COUNT = 128u;
	constexpr size_t MAX_MEMBER_COUNT = 4096u;
	constexpr size_t COMBAT_REVISION_HEX_LENGTH = 64u;

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
		if (text.empty() || ('0' == text.front() && text.size() != 1u))
			return false;
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
}

bool_t Client::CWorldDestructionProjectionDocument::Load(
	const std::filesystem::path& path,
	std::string& outStatus)
{
	std::ifstream input(path, std::ios::binary);
	if (path.empty() || !input.is_open())
	{
		outStatus = "World destruction projection is unreadable: " +
			path.string();
		return false;
	}
	std::ostringstream buffer;
	buffer << input.rdbuf();
	if (input.bad())
	{
		outStatus = "World destruction projection read failed: " + path.string();
		return false;
	}
	return Parse_Text(buffer.str(), *this, outStatus);
}

bool_t Client::CWorldDestructionProjectionDocument::Parse_Text(
	const std::string_view text,
	CWorldDestructionProjectionDocument& outDocument,
	std::string& outStatus)
{
	DATA_JSON_VALUE root;
	std::string parseError;
	DATA_JSON_PARSE_LIMITS limits{};
	limits.iMaximumBytes = 1024u * 1024u;
	limits.iMaximumDepth = 12u;
	limits.iMaximumValues = 8192u;
	if (!CDataJson::Parse(text, root, parseError, limits))
	{
		outStatus = "World destruction projection parse failed: " + parseError;
		return false;
	}
	if (!Is_ExactObject(root,
		{ "schema", "formatVersion", "areaId", "combatRuntimeRevision", "groups" }))
	{
		outStatus = "World destruction projection root has unexpected properties";
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
		outStatus = "World destruction projection header is invalid";
		return false;
	}

	const DATA_JSON_VALUE* groups = root.Find("groups");
	if (nullptr == groups || !groups->Is_Array() || groups->Get_Array().empty() ||
		groups->Get_Array().size() > MAX_GROUP_COUNT)
	{
		outStatus = "World destruction projection groups are invalid";
		return false;
	}

	std::vector<WORLD_DESTRUCTION_PROJECTION_GROUP> stagedGroups;
	std::unordered_set<std::string> mutationIds;
	std::unordered_set<uint64_t> placementIds;
	size_t totalMembers = 0u;
	std::string previousGroupId;
	for (const DATA_JSON_VALUE& groupValue : groups->Get_Array())
	{
		if (!Is_ExactObject(groupValue,
			{ "groupId", "mutationId", "memberPlacementIds",
				"suppressionAliasPlacementIds" }))
		{
			outStatus = "World destruction projection group has unexpected properties";
			return false;
		}

		WORLD_DESTRUCTION_PROJECTION_GROUP group;
		if (!Read_String(groupValue, "groupId", group.strGroupId) ||
			!Is_StableId(group.strGroupId) ||
			(!previousGroupId.empty() && !(previousGroupId < group.strGroupId)) ||
			!Read_String(groupValue, "mutationId", group.strMutationId) ||
			!Is_StableId(group.strMutationId) ||
			!mutationIds.insert(group.strMutationId).second)
		{
			outStatus = "World destruction projection group identity is invalid";
			return false;
		}
		previousGroupId = group.strGroupId;

		const DATA_JSON_VALUE* members = groupValue.Find("memberPlacementIds");
		if (nullptr == members || !members->Is_Array() ||
			members->Get_Array().empty() ||
			totalMembers + members->Get_Array().size() > MAX_MEMBER_COUNT)
		{
			outStatus = "World destruction projection members are invalid";
			return false;
		}
		uint64_t previousPlacementId = 0u;
		for (const DATA_JSON_VALUE& memberValue : members->Get_Array())
		{
			uint64_t placementId = 0u;
			if (!memberValue.Is_String() ||
				!Parse_CanonicalPlacementId(memberValue.Get_String(), placementId) ||
				(0u != previousPlacementId && previousPlacementId >= placementId) ||
				!placementIds.insert(placementId).second)
			{
				outStatus =
					"World destruction projection placement identity is invalid";
				return false;
			}
			previousPlacementId = placementId;
			group.MemberPlacementIds.push_back(placementId);
		}
		totalMembers += group.MemberPlacementIds.size();

		const DATA_JSON_VALUE* aliases =
			groupValue.Find("suppressionAliasPlacementIds");
		if (nullptr == aliases || !aliases->Is_Array())
		{
			outStatus =
				"World destruction projection suppression aliases are invalid";
			return false;
		}
		uint64_t previousAliasId = 0u;
		for (const DATA_JSON_VALUE& aliasValue : aliases->Get_Array())
		{
			uint64_t aliasId = 0u;
			if (!aliasValue.Is_String() ||
				!Parse_CanonicalPlacementId(aliasValue.Get_String(), aliasId) ||
				(0u != previousAliasId && previousAliasId >= aliasId) ||
				!std::binary_search(
					group.MemberPlacementIds.begin(),
					group.MemberPlacementIds.end(), aliasId))
			{
				outStatus =
					"World destruction projection suppression alias identity is invalid";
				return false;
			}
			previousAliasId = aliasId;
			group.SuppressionAliasPlacementIds.push_back(aliasId);
		}
		stagedGroups.push_back(std::move(group));
	}

	CWorldDestructionProjectionDocument committed;
	committed.m_strAreaId = std::move(areaId);
	committed.m_strCombatRuntimeRevision = std::move(revision);
	committed.m_Groups = std::move(stagedGroups);
	committed.m_isReady = true;
	outDocument = std::move(committed);
	outStatus = "Loaded world destruction projection: " +
		std::to_string(outDocument.m_Groups.size()) + " groups, " +
		std::to_string(totalMembers) + " placements";
	return true;
}

void Client::CWorldDestructionProjectionDocument::Clear()
{
	m_strAreaId.clear();
	m_strCombatRuntimeRevision.clear();
	m_Groups.clear();
	m_isReady = false;
}

const Client::WORLD_DESTRUCTION_PROJECTION_GROUP*
Client::CWorldDestructionProjectionDocument::Find_Group(
	const std::string_view groupId) const
{
	const auto iter = std::lower_bound(
		m_Groups.begin(), m_Groups.end(), groupId,
		[](const WORLD_DESTRUCTION_PROJECTION_GROUP& group,
			const std::string_view key)
		{
			return group.strGroupId < key;
		});
	return iter != m_Groups.end() && iter->strGroupId == groupId ?
		&*iter : nullptr;
}
