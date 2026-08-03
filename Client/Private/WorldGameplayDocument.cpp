#include "WorldGameplayDocument.h"

#include "DataJson.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <unordered_set>

namespace
{
	using namespace Client;

	constexpr const char_t* SCHEMA = "lostark.world-gameplay";
	constexpr uint32_t FORMAT_VERSION = 1;

	bool_t Is_IntegerNumber(const DATA_JSON_VALUE* value)
	{
		return nullptr != value && value->Is_Number() &&
			std::isfinite(value->Get_Number()) &&
			std::floor(value->Get_Number()) == value->Get_Number();
	}

	bool_t Is_ExactObject(
		const DATA_JSON_VALUE& value,
		const std::initializer_list<const char_t*> keys)
	{
		if (!value.Is_Object() || value.Get_Object().size() != keys.size())
			return false;
		return std::all_of(keys.begin(), keys.end(),
			[&value](const char_t* key)
			{
				return nullptr != value.Find(key);
			});
	}

	bool_t Read_Position(
		const DATA_JSON_VALUE* value,
		float3_t& outPosition)
	{
		if (nullptr == value || !value->Is_Array() ||
			3u != value->Get_Array().size())
		{
			return false;
		}

		const auto& values = value->Get_Array();
		if (!values[0].Is_Number() || !values[1].Is_Number() ||
			!values[2].Is_Number())
		{
			return false;
		}
		outPosition = float3_t(
			static_cast<f32_t>(values[0].Get_Number()),
			static_cast<f32_t>(values[1].Get_Number()),
			static_cast<f32_t>(values[2].Get_Number()));
		return true;
	}

	bool_t CommitTemporaryFile(
		const std::filesystem::path& destination,
		const std::filesystem::path& temporary)
	{
		std::error_code existsError;
		if (std::filesystem::exists(destination, existsError) &&
			ReplaceFileW(
				destination.c_str(), temporary.c_str(), nullptr,
				REPLACEFILE_WRITE_THROUGH, nullptr, nullptr))
		{
			return true;
		}
		return MoveFileExW(
			temporary.c_str(), destination.c_str(),
			MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
	}
}

bool_t Client::CWorldGameplayDocument::Load(
	const std::filesystem::path& path,
	const std::string& expectedAreaId,
	std::string& outStatus)
{
	std::error_code existsError;
	if (!std::filesystem::exists(path, existsError))
	{
		m_Placements.clear();
		m_iRevision = 1;
		outStatus = "No gameplay document; starting empty";
		return !existsError;
	}

	std::ifstream input(path, std::ios::binary);
	if (!input)
	{
		outStatus = "Could not open gameplay document: " + path.string();
		return false;
	}
	const std::string text(
		(std::istreambuf_iterator<char_t>(input)),
		std::istreambuf_iterator<char_t>());

	DATA_JSON_VALUE root;
	std::string parseError;
	if (!CDataJson::Parse(text, root, parseError) ||
		!Is_ExactObject(root,
			{ "schema", "formatVersion", "areaId", "revision", "placements" }))
	{
		outStatus = "Gameplay JSON root is invalid: " + parseError;
		return false;
	}

	const DATA_JSON_VALUE* schema = root.Find("schema");
	const DATA_JSON_VALUE* version = root.Find("formatVersion");
	const DATA_JSON_VALUE* areaId = root.Find("areaId");
	const DATA_JSON_VALUE* revision = root.Find("revision");
	const DATA_JSON_VALUE* placements = root.Find("placements");
	if (nullptr == schema || !schema->Is_String() ||
		schema->Get_String() != SCHEMA ||
		!Is_IntegerNumber(version) ||
		version->Get_Number() != FORMAT_VERSION ||
		nullptr == areaId || !areaId->Is_String() ||
		areaId->Get_String() != expectedAreaId ||
		!Is_IntegerNumber(revision) || revision->Get_Number() < 1.0 ||
		revision->Get_Number() > UINT32_MAX ||
		nullptr == placements || !placements->Is_Array() ||
		placements->Get_Array().size() > MAX_PLACEMENT_COUNT)
	{
		outStatus = "Gameplay JSON header is invalid or belongs to another area";
		return false;
	}

	std::vector<WORLD_GAMEPLAY_PLACEMENT> staged;
	std::unordered_set<std::string> ids;
	staged.reserve(placements->Get_Array().size());
	for (const DATA_JSON_VALUE& value : placements->Get_Array())
	{
		if (!Is_ExactObject(value,
			{ "placementId", "kind", "archetypeId", "encounterId",
			  "position", "yawDegrees", "enabled" }))
		{
			outStatus = "Gameplay placement object has missing or unknown fields";
			return false;
		}

		const DATA_JSON_VALUE* placementId = value.Find("placementId");
		const DATA_JSON_VALUE* kind = value.Find("kind");
		const DATA_JSON_VALUE* archetypeId = value.Find("archetypeId");
		const DATA_JSON_VALUE* encounterId = value.Find("encounterId");
		const DATA_JSON_VALUE* yaw = value.Find("yawDegrees");
		const DATA_JSON_VALUE* enabled = value.Find("enabled");

		WORLD_GAMEPLAY_PLACEMENT record;
		if (nullptr == placementId || !placementId->Is_String() ||
			nullptr == kind || !kind->Is_String() ||
			nullptr == archetypeId ||
			(!archetypeId->Is_Null() && !archetypeId->Is_String()) ||
			nullptr == encounterId ||
			(!encounterId->Is_Null() && !encounterId->Is_String()) ||
			nullptr == yaw || !yaw->Is_Number() ||
			nullptr == enabled || !enabled->Is_Boolean() ||
			!Try_ParseKind(kind->Get_String(), record.eKind) ||
			!Read_Position(value.Find("position"), record.position))
		{
			outStatus = "Gameplay placement field type is invalid";
			return false;
		}

		record.placementId = placementId->Get_String();
		record.archetypeId = archetypeId->Is_String() ?
			archetypeId->Get_String() : std::string{};
		record.encounterId = encounterId->Is_String() ?
			encounterId->Get_String() : std::string{};
		record.yawDegrees = static_cast<f32_t>(yaw->Get_Number());
		record.isEnabled = enabled->Get_Boolean();
		if (!Is_Valid(record) || !ids.insert(record.placementId).second)
		{
			outStatus = "Gameplay placement ID is duplicate or invalid: " +
				record.placementId;
			return false;
		}
		staged.push_back(std::move(record));
	}

	m_Placements = std::move(staged);
	m_iRevision = static_cast<uint32_t>(revision->Get_Number());
	outStatus = "Loaded gameplay placements: " +
		std::to_string(m_Placements.size());
	return true;
}

bool_t Client::CWorldGameplayDocument::Save(
	const std::filesystem::path& path,
	const std::string& areaId,
	std::string& outStatus) const
{
	if (areaId.empty() || m_Placements.size() > MAX_PLACEMENT_COUNT)
	{
		outStatus = "Gameplay save header is invalid";
		return false;
	}

	std::vector<WORLD_GAMEPLAY_PLACEMENT> sorted = m_Placements;
	std::sort(sorted.begin(), sorted.end(),
		[](const auto& left, const auto& right)
		{
			return left.placementId < right.placementId;
		});
	for (size_t index = 0; index < sorted.size(); ++index)
	{
		if (!Is_Valid(sorted[index]) ||
			(0u != index &&
				sorted[index - 1u].placementId == sorted[index].placementId))
		{
			outStatus = "Gameplay save rejected invalid or duplicate placement";
			return false;
		}
	}

	std::error_code directoryError;
	std::filesystem::create_directories(path.parent_path(), directoryError);
	if (directoryError)
	{
		outStatus = "Could not create gameplay authoring directory";
		return false;
	}

	const std::filesystem::path temporary = path.wstring() + L".tmp";
	std::ofstream output(temporary, std::ios::binary | std::ios::trunc);
	if (!output)
	{
		outStatus = "Could not create gameplay temporary file";
		return false;
	}

	output << "{\n"
		<< "  \"schema\": \"" << SCHEMA << "\",\n"
		<< "  \"formatVersion\": " << FORMAT_VERSION << ",\n"
		<< "  \"areaId\": \"" << CDataJson::Escape(areaId) << "\",\n"
		<< "  \"revision\": " << m_iRevision << ",\n"
		<< "  \"placements\": [";
	output << std::setprecision(9);
	for (size_t index = 0; index < sorted.size(); ++index)
	{
		const WORLD_GAMEPLAY_PLACEMENT& record = sorted[index];
		output << (0u == index ? "\n" : ",\n")
			<< "    {\n"
			<< "      \"placementId\": \"" <<
				CDataJson::Escape(record.placementId) << "\",\n"
			<< "      \"kind\": \"" << Kind_ToString(record.eKind) << "\",\n"
			<< "      \"archetypeId\": ";
		if (record.archetypeId.empty())
			output << "null";
		else
			output << '"' << CDataJson::Escape(record.archetypeId) << '"';
		output << ",\n"
			<< "      \"encounterId\": ";
		if (record.encounterId.empty())
			output << "null";
		else
			output << '"' << CDataJson::Escape(record.encounterId) << '"';
		output << ",\n"
			<< "      \"position\": [" << record.position.x << ", " <<
				record.position.y << ", " << record.position.z << "],\n"
			<< "      \"yawDegrees\": " << record.yawDegrees << ",\n"
			<< "      \"enabled\": " <<
				(record.isEnabled ? "true" : "false") << "\n"
			<< "    }";
	}
	output << (sorted.empty() ? "]\n" : "\n  ]\n") << "}\n";
	output.flush();
	const bool_t writeSucceeded = output.good();
	output.close();
	if (!writeSucceeded || !CommitTemporaryFile(path, temporary))
	{
		std::error_code removeError;
		std::filesystem::remove(temporary, removeError);
		outStatus = "Failed to commit gameplay document atomically";
		return false;
	}

	outStatus = "Saved gameplay placements: " + std::to_string(sorted.size());
	return true;
}

bool_t Client::CWorldGameplayDocument::Add(
	const WORLD_GAMEPLAY_PLACEMENT& placement,
	std::string& outStatus)
{
	WORLD_GAMEPLAY_PLACEMENT normalized = placement;
	if (WORLD_PLACEMENT_KIND::PLAYER_SPAWN == normalized.eKind)
		normalized.archetypeId.clear();
	if (m_Placements.size() >= MAX_PLACEMENT_COUNT ||
		!Is_Valid(normalized) || nullptr != Find(normalized.placementId))
	{
		outStatus = "Gameplay placement is invalid, duplicate, or over limit";
		return false;
	}
	m_Placements.push_back(std::move(normalized));
	Mark_Edited();
	outStatus = "Added gameplay placement: " + placement.placementId;
	return true;
}

bool_t Client::CWorldGameplayDocument::Remove(
	const std::string& placementId)
{
	const auto iter = std::find_if(
		m_Placements.begin(), m_Placements.end(),
		[&placementId](const auto& placement)
		{
			return placement.placementId == placementId;
		});
	if (m_Placements.end() == iter)
		return false;
	m_Placements.erase(iter);
	Mark_Edited();
	return true;
}

WORLD_GAMEPLAY_PLACEMENT* Client::CWorldGameplayDocument::Find(
	const std::string& placementId)
{
	const auto iter = std::find_if(
		m_Placements.begin(), m_Placements.end(),
		[&placementId](const auto& placement)
		{
			return placement.placementId == placementId;
		});
	return m_Placements.end() == iter ? nullptr : &*iter;
}

const WORLD_GAMEPLAY_PLACEMENT* Client::CWorldGameplayDocument::Find(
	const std::string& placementId) const
{
	const auto iter = std::find_if(
		m_Placements.begin(), m_Placements.end(),
		[&placementId](const auto& placement)
		{
			return placement.placementId == placementId;
		});
	return m_Placements.end() == iter ? nullptr : &*iter;
}

bool_t Client::CWorldGameplayDocument::Is_Valid(
	const WORLD_GAMEPLAY_PLACEMENT& placement)
{
	const auto validStableId = [](const std::string& value, const size_t maximum)
	{
		return !value.empty() && value.size() <= maximum &&
			std::all_of(value.begin(), value.end(), [](const unsigned char character)
			{
				return 0 != std::isalnum(character) || character == '_' ||
					character == '-' || character == '.';
			});
	};
	const bool_t hasValidArchetype =
		WORLD_PLACEMENT_KIND::PLAYER_SPAWN == placement.eKind ?
			placement.archetypeId.empty() :
			validStableId(placement.archetypeId, 128u);
	return WORLD_PLACEMENT_KIND::END != placement.eKind &&
		validStableId(placement.placementId, 128u) &&
		hasValidArchetype &&
		(placement.encounterId.empty() ||
			validStableId(placement.encounterId, 128u)) &&
		std::isfinite(placement.position.x) &&
		std::isfinite(placement.position.y) &&
		std::isfinite(placement.position.z) &&
		std::isfinite(placement.yawDegrees) &&
		std::abs(placement.position.x) <= 100000.f &&
		std::abs(placement.position.y) <= 100000.f &&
		std::abs(placement.position.z) <= 100000.f &&
		std::abs(placement.yawDegrees) <= 360000.f;
}

const char_t* Client::CWorldGameplayDocument::Kind_ToString(
	const WORLD_PLACEMENT_KIND kind)
{
	switch (kind)
	{
	case WORLD_PLACEMENT_KIND::PLAYER_SPAWN: return "playerSpawn";
	case WORLD_PLACEMENT_KIND::NPC: return "npc";
	case WORLD_PLACEMENT_KIND::BOSS: return "boss";
	default: return "invalid";
	}
}

bool_t Client::CWorldGameplayDocument::Try_ParseKind(
	const std::string& value,
	WORLD_PLACEMENT_KIND& outKind)
{
	if ("playerSpawn" == value)
		outKind = WORLD_PLACEMENT_KIND::PLAYER_SPAWN;
	else if ("npc" == value)
		outKind = WORLD_PLACEMENT_KIND::NPC;
	else if ("boss" == value)
		outKind = WORLD_PLACEMENT_KIND::BOSS;
	else
		return false;
	return true;
}
