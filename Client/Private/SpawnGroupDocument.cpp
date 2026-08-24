#include "SpawnGroupDocument.h"

#include "DataJson.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

namespace
{
	using namespace Client;

	constexpr const char_t* SCHEMA = "lostark.world-spawn-groups";
	constexpr uint32_t FORMAT_VERSION = 1;
	constexpr uint32_t MAX_DELAY_MS = 600000;

	bool_t Is_Integer(const DATA_JSON_VALUE* value, const double minimum,
		const double maximum)
	{
		return nullptr != value && value->Is_Number() &&
			std::isfinite(value->Get_Number()) &&
			std::floor(value->Get_Number()) == value->Get_Number() &&
			value->Get_Number() >= minimum && value->Get_Number() <= maximum;
	}

	bool_t Is_ExactObject(const DATA_JSON_VALUE& value,
		const std::initializer_list<const char_t*> keys)
	{
		if (!value.Is_Object() || value.Get_Object().size() != keys.size())
			return false;
		return std::all_of(keys.begin(), keys.end(),
			[&value](const char_t* key) { return nullptr != value.Find(key); });
	}

	bool_t Read_Position(const DATA_JSON_VALUE* value, float3_t& outPosition)
	{
		if (nullptr == value || !value->Is_Array() ||
			3u != value->Get_Array().size())
			return false;
		const auto& array = value->Get_Array();
		if (!array[0].Is_Number() || !array[1].Is_Number() ||
			!array[2].Is_Number())
			return false;
		outPosition = float3_t(static_cast<f32_t>(array[0].Get_Number()),
			static_cast<f32_t>(array[1].Get_Number()),
			static_cast<f32_t>(array[2].Get_Number()));
		return std::isfinite(outPosition.x) && std::isfinite(outPosition.y) &&
			std::isfinite(outPosition.z) && std::abs(outPosition.x) <= 100000.f &&
			std::abs(outPosition.y) <= 100000.f &&
			std::abs(outPosition.z) <= 100000.f;
	}

	bool_t CommitTemporaryFile(const std::filesystem::path& destination,
		const std::filesystem::path& temporary)
	{
		std::error_code existsError;
		if (std::filesystem::exists(destination, existsError) &&
			ReplaceFileW(destination.c_str(), temporary.c_str(), nullptr,
				REPLACEFILE_WRITE_THROUGH, nullptr, nullptr))
			return true;
		return MoveFileExW(temporary.c_str(), destination.c_str(),
			MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
	}
}

bool_t Client::CSpawnGroupDocument::Load(const std::filesystem::path& path,
	const std::string& expectedAreaId, std::string& outStatus)
{
	std::error_code existsError;
	if (!std::filesystem::exists(path, existsError))
	{
		if (existsError)
		{
			outStatus = "Could not inspect spawn group document";
			return false;
		}
		Reset();
		outStatus = "No spawn group document; starting empty";
		return true;
	}

	std::ifstream input(path, std::ios::binary);
	if (!input)
	{
		outStatus = "Could not open spawn group document: " + path.string();
		return false;
	}
	const std::string text((std::istreambuf_iterator<char_t>(input)),
		std::istreambuf_iterator<char_t>());
	DATA_JSON_VALUE root;
	std::string parseError;
	if (!CDataJson::Parse(text, root, parseError) ||
		!Is_ExactObject(root, { "schema", "formatVersion", "areaId",
			"revision", "anchors", "spawnGroups" }))
	{
		outStatus = "Spawn group JSON root is invalid: " + parseError;
		return false;
	}
	const auto* schema = root.Find("schema");
	const auto* version = root.Find("formatVersion");
	const auto* areaId = root.Find("areaId");
	const auto* revision = root.Find("revision");
	const auto* anchors = root.Find("anchors");
	const auto* groups = root.Find("spawnGroups");
	if (nullptr == schema || !schema->Is_String() || schema->Get_String() != SCHEMA ||
		!Is_Integer(version, FORMAT_VERSION, FORMAT_VERSION) ||
		nullptr == areaId || !areaId->Is_String() ||
		areaId->Get_String() != expectedAreaId ||
		!Is_Integer(revision, 1, UINT32_MAX) || nullptr == anchors ||
		!anchors->Is_Array() || anchors->Get_Array().size() > MAX_ANCHOR_COUNT ||
		nullptr == groups || !groups->Is_Array() ||
		groups->Get_Array().size() > MAX_GROUP_COUNT)
	{
		outStatus = "Spawn group JSON header is invalid or belongs to another area";
		return false;
	}

	std::vector<SPAWN_ANCHOR_RECORD> stagedAnchors;
	std::vector<SPAWN_GROUP_RECORD> stagedGroups;
	for (const DATA_JSON_VALUE& value : anchors->Get_Array())
	{
		if (!Is_ExactObject(value, { "anchorId", "position", "yawDegrees" }))
		{
			outStatus = "Spawn anchor has missing or unknown fields";
			return false;
		}
		const auto* id = value.Find("anchorId");
		const auto* yaw = value.Find("yawDegrees");
		SPAWN_ANCHOR_RECORD anchor;
		if (nullptr == id || !id->Is_String() || nullptr == yaw ||
			!yaw->Is_Number() || !Read_Position(value.Find("position"), anchor.position))
		{
			outStatus = "Spawn anchor field type is invalid";
			return false;
		}
		anchor.anchorId = id->Get_String();
		anchor.yawDegrees = static_cast<f32_t>(yaw->Get_Number());
		stagedAnchors.push_back(std::move(anchor));
	}
	for (const DATA_JSON_VALUE& value : groups->Get_Array())
	{
		if (!Is_ExactObject(value, { "spawnGroupId", "requiredCompletedGroupId",
			"maxAlive", "repeatPolicy", "repeatDelayMs", "completionPolicy", "waves" }))
		{
			outStatus = "Spawn group has missing or unknown fields";
			return false;
		}
		const auto* groupId = value.Find("spawnGroupId");
		const auto* prerequisite = value.Find("requiredCompletedGroupId");
		const auto* maxAlive = value.Find("maxAlive");
		const auto* repeat = value.Find("repeatPolicy");
		const auto* repeatDelay = value.Find("repeatDelayMs");
		const auto* completion = value.Find("completionPolicy");
		const auto* waves = value.Find("waves");
		const bool_t repeats = nullptr != repeat && repeat->Is_String() &&
			repeat->Get_String() == "REPEAT";
		if (nullptr == groupId || !groupId->Is_String() || nullptr == prerequisite ||
			(!prerequisite->Is_Null() && !prerequisite->Is_String()) ||
			!Is_Integer(maxAlive, 1, 64) || nullptr == repeat || !repeat->Is_String() ||
			(repeat->Get_String() != "ONCE" && !repeats) ||
			!Is_Integer(repeatDelay, 0, MAX_DELAY_MS) || nullptr == completion ||
			!completion->Is_String() || completion->Get_String() != "ALL_WAVES_CLEARED" ||
			nullptr == waves || !waves->Is_Array() || waves->Get_Array().empty() ||
			waves->Get_Array().size() > MAX_WAVE_COUNT)
		{
			outStatus = "Spawn group header is invalid";
			return false;
		}
		/* The delay belongs to REPEAT alone, so the two have to agree or the
		document means two things at once. */
		if (repeats != (0 != static_cast<uint32_t>(repeatDelay->Get_Number())))
		{
			outStatus = "Spawn group repeat delay contradicts its policy";
			return false;
		}
		SPAWN_GROUP_RECORD group;
		group.spawnGroupId = groupId->Get_String();
		group.requiredCompletedGroupId = prerequisite->Is_String() ?
			prerequisite->Get_String() : std::string{};
		group.maxAlive = static_cast<uint32_t>(maxAlive->Get_Number());
		group.repeats = repeats;
		group.repeatDelayMs = static_cast<uint32_t>(repeatDelay->Get_Number());
		for (const DATA_JSON_VALUE& waveValue : waves->Get_Array())
		{
			if (!Is_ExactObject(waveValue,
				{ "waveId", "startDelayMs", "nextWavePolicy", "nextWaveDelayMs", "entries" }))
			{
				outStatus = "Spawn wave has missing or unknown fields";
				return false;
			}
			const auto* waveId = waveValue.Find("waveId");
			const auto* startDelay = waveValue.Find("startDelayMs");
			const auto* nextPolicy = waveValue.Find("nextWavePolicy");
			const auto* nextDelay = waveValue.Find("nextWaveDelayMs");
			const auto* entries = waveValue.Find("entries");
			const bool_t usesTimer = nullptr != nextPolicy && nextPolicy->Is_String() &&
				nextPolicy->Get_String() == "TIMER";
			if (nullptr == waveId || !waveId->Is_String() ||
				!Is_Integer(startDelay, 0, MAX_DELAY_MS) || nullptr == nextPolicy ||
				!nextPolicy->Is_String() ||
				(nextPolicy->Get_String() != "ALL_DEAD" && !usesTimer) ||
				!Is_Integer(nextDelay, 0, MAX_DELAY_MS) ||
				nullptr == entries || !entries->Is_Array() || entries->Get_Array().empty() ||
				entries->Get_Array().size() > MAX_ENTRY_COUNT)
			{
				outStatus = "Spawn wave header is invalid";
				return false;
			}
			if (usesTimer != (0 != static_cast<uint32_t>(nextDelay->Get_Number())))
			{
				outStatus = "Spawn wave delay contradicts its policy";
				return false;
			}
			SPAWN_WAVE_RECORD wave;
			wave.waveId = waveId->Get_String();
			wave.startDelayMs = static_cast<uint32_t>(startDelay->Get_Number());
			wave.usesTimerNextWave = usesTimer;
			wave.nextWaveDelayMs = static_cast<uint32_t>(nextDelay->Get_Number());
			for (const DATA_JSON_VALUE& entryValue : entries->Get_Array())
			{
				if (!Is_ExactObject(entryValue, { "archetypeId", "count", "anchorId",
					"initialDelayMs", "spawnIntervalMs" }))
				{
					outStatus = "Spawn entry has missing or unknown fields";
					return false;
				}
				const auto* archetypeId = entryValue.Find("archetypeId");
				const auto* count = entryValue.Find("count");
				const auto* anchorId = entryValue.Find("anchorId");
				const auto* initialDelay = entryValue.Find("initialDelayMs");
				const auto* interval = entryValue.Find("spawnIntervalMs");
				if (nullptr == archetypeId || !archetypeId->Is_String() ||
					!Is_Integer(count, 1, MAX_TOTAL_SPAWN_COUNT) || nullptr == anchorId ||
					!anchorId->Is_String() || !Is_Integer(initialDelay, 0, MAX_DELAY_MS) ||
					!Is_Integer(interval, 0, MAX_DELAY_MS))
				{
					outStatus = "Spawn entry field type is invalid";
					return false;
				}
				SPAWN_WAVE_ENTRY_RECORD entry;
				entry.archetypeId = archetypeId->Get_String();
				entry.count = static_cast<uint32_t>(count->Get_Number());
				entry.anchorId = anchorId->Get_String();
				entry.initialDelayMs = static_cast<uint32_t>(initialDelay->Get_Number());
				entry.spawnIntervalMs = static_cast<uint32_t>(interval->Get_Number());
				wave.entries.push_back(std::move(entry));
			}
			group.waves.push_back(std::move(wave));
		}
		stagedGroups.push_back(std::move(group));
	}

	CSpawnGroupDocument staged;
	staged.m_Anchors = std::move(stagedAnchors);
	staged.m_Groups = std::move(stagedGroups);
	staged.m_iRevision = static_cast<uint32_t>(revision->Get_Number());
	if (!staged.Validate(outStatus))
		return false;
	m_Anchors = std::move(staged.m_Anchors);
	m_Groups = std::move(staged.m_Groups);
	m_iRevision = staged.m_iRevision;
	outStatus = "Loaded spawn groups: " + std::to_string(m_Groups.size());
	return true;
}

bool_t Client::CSpawnGroupDocument::Save(const std::filesystem::path& path,
	const std::string& areaId, std::string& outStatus) const
{
	if (areaId.empty() || !Validate(outStatus))
		return false;
	std::vector<SPAWN_ANCHOR_RECORD> anchors = m_Anchors;
	std::vector<SPAWN_GROUP_RECORD> groups = m_Groups;
	std::sort(anchors.begin(), anchors.end(), [](const auto& left, const auto& right)
		{ return left.anchorId < right.anchorId; });
	std::sort(groups.begin(), groups.end(), [](const auto& left, const auto& right)
		{ return left.spawnGroupId < right.spawnGroupId; });

	std::error_code directoryError;
	std::filesystem::create_directories(path.parent_path(), directoryError);
	if (directoryError)
	{
		outStatus = "Could not create spawn group authoring directory";
		return false;
	}
	const std::filesystem::path temporary = path.wstring() + L".tmp";
	std::ofstream output(temporary, std::ios::binary | std::ios::trunc);
	if (!output)
	{
		outStatus = "Could not create spawn group temporary file";
		return false;
	}
	output << std::setprecision(9)
		<< "{\n  \"schema\": \"" << SCHEMA << "\",\n"
		<< "  \"formatVersion\": " << FORMAT_VERSION << ",\n"
		<< "  \"areaId\": \"" << CDataJson::Escape(areaId) << "\",\n"
		<< "  \"revision\": " << m_iRevision << ",\n  \"anchors\": [";
	for (size_t index = 0; index < anchors.size(); ++index)
	{
		const auto& anchor = anchors[index];
		output << (0u == index ? "\n" : ",\n")
			<< "    { \"anchorId\": \"" << CDataJson::Escape(anchor.anchorId)
			<< "\", \"position\": [" << anchor.position.x << ", "
			<< anchor.position.y << ", " << anchor.position.z
			<< "], \"yawDegrees\": " << anchor.yawDegrees << " }";
	}
	output << (anchors.empty() ? "],\n" : "\n  ],\n") << "  \"spawnGroups\": [";
	for (size_t groupIndex = 0; groupIndex < groups.size(); ++groupIndex)
	{
		const auto& group = groups[groupIndex];
		output << (0u == groupIndex ? "\n" : ",\n")
			<< "    {\n      \"spawnGroupId\": \""
			<< CDataJson::Escape(group.spawnGroupId)
			<< "\",\n      \"requiredCompletedGroupId\": ";
		if (group.requiredCompletedGroupId.empty()) output << "null";
		else output << '"' << CDataJson::Escape(group.requiredCompletedGroupId) << '"';
		output << ",\n      \"maxAlive\": " << group.maxAlive
			<< ",\n      \"repeatPolicy\": \""
			<< (group.repeats ? "REPEAT" : "ONCE")
			<< "\",\n      \"repeatDelayMs\": " << group.repeatDelayMs
			<< ",\n      \"completionPolicy\": \"ALL_WAVES_CLEARED\",\n"
			<< "      \"waves\": [";
		for (size_t waveIndex = 0; waveIndex < group.waves.size(); ++waveIndex)
		{
			const auto& wave = group.waves[waveIndex];
			output << (0u == waveIndex ? "\n" : ",\n")
				<< "        {\n          \"waveId\": \""
				<< CDataJson::Escape(wave.waveId)
				<< "\",\n          \"startDelayMs\": " << wave.startDelayMs
				<< ",\n          \"nextWavePolicy\": \""
				<< (wave.usesTimerNextWave ? "TIMER" : "ALL_DEAD")
				<< "\",\n          \"nextWaveDelayMs\": " << wave.nextWaveDelayMs
				<< ",\n          \"entries\": [";
			for (size_t entryIndex = 0; entryIndex < wave.entries.size(); ++entryIndex)
			{
				const auto& entry = wave.entries[entryIndex];
				output << (0u == entryIndex ? "\n" : ",\n")
					<< "            { \"archetypeId\": \""
					<< CDataJson::Escape(entry.archetypeId) << "\", \"count\": "
					<< entry.count << ", \"anchorId\": \""
					<< CDataJson::Escape(entry.anchorId)
					<< "\", \"initialDelayMs\": " << entry.initialDelayMs
					<< ", \"spawnIntervalMs\": " << entry.spawnIntervalMs << " }";
			}
			output << "\n          ]\n        }";
		}
		output << "\n      ]\n    }";
	}
	output << (groups.empty() ? "]\n" : "\n  ]\n") << "}\n";
	output.flush();
	const bool_t writeSucceeded = output.good();
	output.close();
	if (!writeSucceeded || !CommitTemporaryFile(path, temporary))
	{
		std::error_code removeError;
		std::filesystem::remove(temporary, removeError);
		outStatus = "Failed to commit spawn group document atomically";
		return false;
	}
	outStatus = "Saved spawn groups: " + std::to_string(groups.size());
	return true;
}

void Client::CSpawnGroupDocument::Reset()
{
	m_Anchors.clear();
	m_Groups.clear();
	m_iRevision = 1;
}

SPAWN_ANCHOR_RECORD* Client::CSpawnGroupDocument::Find_Anchor(
	const std::string& anchorId)
{
	const auto iter = std::find_if(m_Anchors.begin(), m_Anchors.end(),
		[&](const auto& value) { return value.anchorId == anchorId; });
	return m_Anchors.end() == iter ? nullptr : &*iter;
}

const SPAWN_ANCHOR_RECORD* Client::CSpawnGroupDocument::Find_Anchor(
	const std::string& anchorId) const
{
	const auto iter = std::find_if(m_Anchors.begin(), m_Anchors.end(),
		[&](const auto& value) { return value.anchorId == anchorId; });
	return m_Anchors.end() == iter ? nullptr : &*iter;
}

SPAWN_GROUP_RECORD* Client::CSpawnGroupDocument::Find_Group(
	const std::string& spawnGroupId)
{
	const auto iter = std::find_if(m_Groups.begin(), m_Groups.end(),
		[&](const auto& value) { return value.spawnGroupId == spawnGroupId; });
	return m_Groups.end() == iter ? nullptr : &*iter;
}

const SPAWN_GROUP_RECORD* Client::CSpawnGroupDocument::Find_Group(
	const std::string& spawnGroupId) const
{
	const auto iter = std::find_if(m_Groups.begin(), m_Groups.end(),
		[&](const auto& value) { return value.spawnGroupId == spawnGroupId; });
	return m_Groups.end() == iter ? nullptr : &*iter;
}

bool_t Client::CSpawnGroupDocument::Add_Anchor(const SPAWN_ANCHOR_RECORD& anchor,
	std::string& outStatus)
{
	if (m_Anchors.size() >= MAX_ANCHOR_COUNT || nullptr != Find_Anchor(anchor.anchorId) ||
		!Is_ValidStableId(anchor.anchorId) || !std::isfinite(anchor.position.x) ||
		!std::isfinite(anchor.position.y) || !std::isfinite(anchor.position.z) ||
		!std::isfinite(anchor.yawDegrees))
	{
		outStatus = "Spawn anchor is invalid, duplicate, or over limit";
		return false;
	}
	m_Anchors.push_back(anchor);
	Mark_Edited();
	outStatus = "Added spawn anchor: " + anchor.anchorId;
	return true;
}

bool_t Client::CSpawnGroupDocument::Remove_Anchor(const std::string& anchorId,
	std::string& outStatus)
{
	for (const auto& group : m_Groups)
		for (const auto& wave : group.waves)
			for (const auto& entry : wave.entries)
				if (entry.anchorId == anchorId)
				{
					outStatus = "Spawn anchor is still referenced: " + anchorId;
					return false;
				}
	const auto iter = std::remove_if(m_Anchors.begin(), m_Anchors.end(),
		[&](const auto& value) { return value.anchorId == anchorId; });
	if (m_Anchors.end() == iter)
		return false;
	m_Anchors.erase(iter, m_Anchors.end());
	Mark_Edited();
	outStatus = "Removed spawn anchor: " + anchorId;
	return true;
}

bool_t Client::CSpawnGroupDocument::Add_Group(const SPAWN_GROUP_RECORD& group,
	std::string& outStatus)
{
	if (m_Groups.size() >= MAX_GROUP_COUNT || nullptr != Find_Group(group.spawnGroupId) ||
		!Is_ValidStableId(group.spawnGroupId) || group.maxAlive < 1u ||
		group.maxAlive > 64u || (!group.requiredCompletedGroupId.empty() &&
		nullptr == Find_Group(group.requiredCompletedGroupId)))
	{
		outStatus = "Spawn group is invalid, duplicate, or over limit";
		return false;
	}
	m_Groups.push_back(group);
	Mark_Edited();
	outStatus = "Added spawn group: " + group.spawnGroupId;
	return true;
}

bool_t Client::CSpawnGroupDocument::Remove_Group(const std::string& spawnGroupId,
	std::string& outStatus)
{
	for (const auto& group : m_Groups)
		if (group.requiredCompletedGroupId == spawnGroupId)
		{
			outStatus = "Spawn group is still a prerequisite: " + spawnGroupId;
			return false;
		}
	const auto iter = std::remove_if(m_Groups.begin(), m_Groups.end(),
		[&](const auto& value) { return value.spawnGroupId == spawnGroupId; });
	if (m_Groups.end() == iter)
		return false;
	m_Groups.erase(iter, m_Groups.end());
	Mark_Edited();
	outStatus = "Removed spawn group: " + spawnGroupId;
	return true;
}

bool_t Client::CSpawnGroupDocument::Is_ValidStableId(const std::string& value)
{
	return !value.empty() && value.size() <= 128u &&
		std::all_of(value.begin(), value.end(), [](const unsigned char character)
		{
			return 0 != std::isalnum(character) || character == '_' ||
				character == '-' || character == '.';
		});
}

bool_t Client::CSpawnGroupDocument::Validate(std::string& outStatus) const
{
	if (m_Anchors.size() > MAX_ANCHOR_COUNT || m_Groups.size() > MAX_GROUP_COUNT)
	{
		outStatus = "Spawn group document exceeds its limits";
		return false;
	}
	std::unordered_set<std::string> anchorIds;
	for (const auto& anchor : m_Anchors)
	{
		if (!Is_ValidStableId(anchor.anchorId) ||
			!anchorIds.insert(anchor.anchorId).second ||
			!std::isfinite(anchor.position.x) || !std::isfinite(anchor.position.y) ||
			!std::isfinite(anchor.position.z) || !std::isfinite(anchor.yawDegrees))
		{
			outStatus = "Spawn anchor is invalid or duplicate: " + anchor.anchorId;
			return false;
		}
	}
	std::unordered_set<std::string> groupIds;
	for (const auto& group : m_Groups)
		if (!Is_ValidStableId(group.spawnGroupId) ||
			!groupIds.insert(group.spawnGroupId).second)
		{
			outStatus = "Spawn group ID is invalid or duplicate: " + group.spawnGroupId;
			return false;
		}
	for (const auto& group : m_Groups)
	{
		if (group.maxAlive < 1u || group.maxAlive > 64u || group.waves.empty() ||
			group.waves.size() > MAX_WAVE_COUNT ||
			(!group.requiredCompletedGroupId.empty() &&
				0u == groupIds.count(group.requiredCompletedGroupId)))
		{
			outStatus = "Spawn group references an invalid prerequisite or limit: " +
				group.spawnGroupId;
			return false;
		}
		std::unordered_set<std::string> waveIds;
		uint32_t totalCount = 0;
		for (const auto& wave : group.waves)
		{
			if (!Is_ValidStableId(wave.waveId) || !waveIds.insert(wave.waveId).second ||
				wave.startDelayMs > MAX_DELAY_MS || wave.entries.empty() ||
				wave.entries.size() > MAX_ENTRY_COUNT)
			{
				outStatus = "Spawn wave is invalid or duplicate: " + wave.waveId;
				return false;
			}
			for (const auto& entry : wave.entries)
			{
				if (!Is_ValidStableId(entry.archetypeId) ||
					0u == anchorIds.count(entry.anchorId) || 0u == entry.count ||
					entry.initialDelayMs > MAX_DELAY_MS ||
					entry.spawnIntervalMs > MAX_DELAY_MS ||
					totalCount > MAX_TOTAL_SPAWN_COUNT - entry.count)
				{
					outStatus = "Spawn entry is invalid or references an unknown anchor";
					return false;
				}
				totalCount += entry.count;
			}
		}
	}
	for (const auto& group : m_Groups)
	{
		std::unordered_set<std::string> chain;
		const SPAWN_GROUP_RECORD* cursor = &group;
		while (nullptr != cursor && !cursor->requiredCompletedGroupId.empty())
		{
			if (!chain.insert(cursor->spawnGroupId).second)
			{
				outStatus = "Spawn group prerequisite cycle detected: " + group.spawnGroupId;
				return false;
			}
			cursor = Find_Group(cursor->requiredCompletedGroupId);
		}
	}
	outStatus = "Spawn group document is valid";
	return true;
}
