#include "WorldGameplayDocument.h"

#include "DataJson.h"

#include <algorithm>
#include <charconv>
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
	constexpr uint32_t FORMAT_VERSION = 4;

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

	bool_t Read_PositiveExtents(
		const DATA_JSON_VALUE* value,
		float3_t& outExtents)
	{
		if (!Read_Position(value, outExtents))
			return false;
		return std::isfinite(outExtents.x) && std::isfinite(outExtents.y) &&
			std::isfinite(outExtents.z) && outExtents.x > 0.f &&
			outExtents.y > 0.f && outExtents.z > 0.f &&
			outExtents.x <= 1000.f && outExtents.y <= 1000.f &&
			outExtents.z <= 1000.f;
	}

	bool_t Is_ValidStableId(const std::string& value, const size_t maximum = 128u)
	{
		return !value.empty() && value.size() <= maximum &&
			std::all_of(value.begin(), value.end(), [](const unsigned char character)
			{
				return 0 != std::isalnum(character) || character == '_' ||
					character == '-' || character == '.';
			});
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
		const DATA_JSON_VALUE* placementId = value.Find("placementId");
		const DATA_JSON_VALUE* kind = value.Find("kind");
		WORLD_GAMEPLAY_PLACEMENT record;
		if (nullptr == placementId || !placementId->Is_String() ||
			nullptr == kind || !kind->Is_String() ||
			!Try_ParseKind(kind->Get_String(), record.eKind))
		{
			outStatus = "Gameplay placement identity or kind is invalid";
			return false;
		}
		const bool_t actorPlacement =
			WORLD_PLACEMENT_KIND::PLAYER_SPAWN == record.eKind ||
			WORLD_PLACEMENT_KIND::NPC == record.eKind ||
			WORLD_PLACEMENT_KIND::BOSS == record.eKind;
		const bool_t triggerPlacement =
			WORLD_PLACEMENT_KIND::TRIGGER_BOX == record.eKind;
		const bool_t collisionPlacement =
			WORLD_PLACEMENT_KIND::COLLISION_BOX == record.eKind;
		const bool_t destroyablePlacement =
			WORLD_PLACEMENT_KIND::DESTROYABLE == record.eKind;
		if ((actorPlacement && !Is_ExactObject(value,
			{ "placementId", "kind", "archetypeId", "encounterId",
			  "position", "yawDegrees", "enabled" })) ||
			(triggerPlacement && !Is_ExactObject(value,
			{ "placementId", "kind", "position", "yawDegrees", "enabled",
			  "halfExtents", "triggerOnce", "events" })) ||
			(collisionPlacement && !Is_ExactObject(value,
			{ "placementId", "kind", "position", "yawDegrees", "enabled",
			  "halfExtents" })) ||
			(destroyablePlacement && !Is_ExactObject(value,
			{ "placementId", "kind", "position", "yawDegrees", "enabled",
			  "deployRuntimePlacementId", "initialState" })))
		{
			outStatus = "Gameplay placement has missing or unknown kind fields";
			return false;
		}
		const DATA_JSON_VALUE* yaw = value.Find("yawDegrees");
		const DATA_JSON_VALUE* enabled = value.Find("enabled");
		if (nullptr == yaw || !yaw->Is_Number() ||
			nullptr == enabled || !enabled->Is_Boolean() ||
			!Read_Position(value.Find("position"), record.position))
		{
			outStatus = "Gameplay placement field type is invalid";
			return false;
		}
		record.placementId = placementId->Get_String();
		record.yawDegrees = static_cast<f32_t>(yaw->Get_Number());
		record.isEnabled = enabled->Get_Boolean();
		if (actorPlacement)
		{
			const DATA_JSON_VALUE* archetypeId = value.Find("archetypeId");
			const DATA_JSON_VALUE* encounterId = value.Find("encounterId");
			if (nullptr == archetypeId ||
				(!archetypeId->Is_Null() && !archetypeId->Is_String()) ||
				nullptr == encounterId ||
				(!encounterId->Is_Null() && !encounterId->Is_String()))
			{
				outStatus = "Gameplay actor reference has an invalid type";
				return false;
			}
			record.archetypeId = archetypeId->Is_String() ?
				archetypeId->Get_String() : std::string{};
			record.encounterId = encounterId->Is_String() ?
				encounterId->Get_String() : std::string{};
		}
		else if (triggerPlacement)
		{
			const DATA_JSON_VALUE* triggerOnce = value.Find("triggerOnce");
			const DATA_JSON_VALUE* events = value.Find("events");
			if (!Read_PositiveExtents(value.Find("halfExtents"), record.halfExtents) ||
				nullptr == triggerOnce || !triggerOnce->Is_Boolean() ||
				nullptr == events || !events->Is_Array() ||
				(record.isEnabled && events->Get_Array().empty()) ||
				events->Get_Array().size() > 32u)
			{
				outStatus = "Gameplay trigger shape or event list is invalid";
				return false;
			}
			record.isTriggerOnce = triggerOnce->Get_Boolean();
			for (const DATA_JSON_VALUE& eventValue : events->Get_Array())
			{
				const DATA_JSON_VALUE* eventType = eventValue.Find("type");
				WORLD_TRIGGER_EVENT event;
				if (nullptr == eventType || !eventType->Is_String() ||
					!Try_ParseTriggerEventKind(eventType->Get_String(), event.eKind))
				{
					outStatus = "Gameplay trigger event identity is invalid";
					return false;
				}
				if (WORLD_TRIGGER_EVENT_KIND::MOVE_PLAYER == event.eKind)
				{
					if (!Is_ExactObject(eventValue,
						{ "type", "targetPosition", "durationSeconds", "arcHeight" }) ||
						!Read_Position(eventValue.Find("targetPosition"), event.targetPosition))
					{
						outStatus = "Gameplay movePlayer event has invalid fields";
						return false;
					}
					const DATA_JSON_VALUE* duration = eventValue.Find("durationSeconds");
					const DATA_JSON_VALUE* arcHeight = eventValue.Find("arcHeight");
					if (nullptr == duration || !duration->Is_Number() ||
						nullptr == arcHeight || !arcHeight->Is_Number())
					{
						outStatus = "Gameplay movePlayer timing is invalid";
						return false;
					}
					event.durationSeconds = static_cast<f32_t>(duration->Get_Number());
					event.arcHeight = static_cast<f32_t>(arcHeight->Get_Number());
				}
				else if (WORLD_TRIGGER_EVENT_KIND::CHANGE_LEVEL == event.eKind)
				{
					if (!Is_ExactObject(eventValue, { "type", "targetWorldId" }))
					{
						outStatus = "Gameplay changeLevel event has invalid fields";
						return false;
					}
					const DATA_JSON_VALUE* targetWorldId =
						eventValue.Find("targetWorldId");
					if (nullptr == targetWorldId || !targetWorldId->Is_String() ||
						!Try_ParseWorldId(
							targetWorldId->Get_String(), event.eTargetWorldId))
					{
						outStatus = "Gameplay changeLevel target world is invalid";
						return false;
					}
				}
				else
				{
					if (!Is_ExactObject(eventValue, { "type", "targetId", "value" }))
					{
						outStatus = "Gameplay trigger event has unknown fields";
						return false;
					}
					const DATA_JSON_VALUE* targetId = eventValue.Find("targetId");
					const DATA_JSON_VALUE* eventValueField = eventValue.Find("value");
					if (nullptr == targetId || !targetId->Is_String())
						return false;
					event.targetId = targetId->Get_String();
					if (WORLD_TRIGGER_EVENT_KIND::SET_CONDITION == event.eKind)
					{
						if (nullptr == eventValueField || !eventValueField->Is_Boolean())
							return false;
						event.conditionValue = eventValueField->Get_Boolean();
					}
					else if (nullptr == eventValueField || !eventValueField->Is_String() ||
						!Try_ParseDestroyableState(eventValueField->Get_String(),
							event.eDestroyableState))
					{
						return false;
					}
				}
				record.triggerEvents.push_back(std::move(event));
			}
		}
		else if (collisionPlacement)
		{
			if (!Read_PositiveExtents(
				value.Find("halfExtents"), record.halfExtents))
			{
				outStatus = "Gameplay collision shape is invalid";
				return false;
			}
		}
		else
		{
			const DATA_JSON_VALUE* deployId = value.Find("deployRuntimePlacementId");
			const DATA_JSON_VALUE* initialState = value.Find("initialState");
			if (nullptr == deployId || !deployId->Is_String() ||
				nullptr == initialState || !initialState->Is_String() ||
				!Try_ParseDestroyableState(initialState->Get_String(), record.eInitialState))
			{
				outStatus = "Gameplay destroyable binding is invalid";
				return false;
			}
			const std::string& deployText = deployId->Get_String();
			const auto parsed = std::from_chars(deployText.data(),
				deployText.data() + deployText.size(), record.deployRuntimePlacementId);
			if (std::errc{} != parsed.ec ||
				parsed.ptr != deployText.data() + deployText.size() ||
				0u == record.deployRuntimePlacementId)
			{
				outStatus = "Gameplay destroyable runtime placement ID is invalid";
				return false;
			}
		}
		if (!Is_Valid(record) || !ids.insert(record.placementId).second)
		{
			outStatus = "Gameplay placement ID is duplicate or invalid: " +
				record.placementId;
			return false;
		}
		staged.push_back(std::move(record));
	}
	for (const WORLD_GAMEPLAY_PLACEMENT& placement : staged)
	{
		if (WORLD_PLACEMENT_KIND::TRIGGER_BOX != placement.eKind)
			continue;
		for (const WORLD_TRIGGER_EVENT& event : placement.triggerEvents)
		{
			if (WORLD_TRIGGER_EVENT_KIND::SET_DESTROYABLE_STATE == event.eKind)
			{
				const auto target = std::find_if(staged.begin(), staged.end(),
					[&](const WORLD_GAMEPLAY_PLACEMENT& value)
					{ return value.placementId == event.targetId; });
				if (staged.end() == target ||
					WORLD_PLACEMENT_KIND::DESTROYABLE != target->eKind)
				{
					outStatus = "Trigger references an unknown destroyable: " + event.targetId;
					return false;
				}
			}
		}
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
	std::unordered_set<uint64_t> deployRuntimeIds;
	for (const WORLD_GAMEPLAY_PLACEMENT& placement : sorted)
	{
		if (WORLD_PLACEMENT_KIND::DESTROYABLE == placement.eKind &&
			!deployRuntimeIds.insert(placement.deployRuntimePlacementId).second)
		{
			outStatus = "Gameplay save rejected duplicate deploy runtime binding";
			return false;
		}
		if (WORLD_PLACEMENT_KIND::TRIGGER_BOX != placement.eKind)
			continue;
		if (placement.isEnabled && placement.triggerEvents.empty())
		{
			outStatus = "Enabled gameplay trigger requires at least one event";
			return false;
		}
		for (const WORLD_TRIGGER_EVENT& event : placement.triggerEvents)
		{
			if (WORLD_TRIGGER_EVENT_KIND::SET_DESTROYABLE_STATE != event.eKind)
				continue;
			const auto target = std::find_if(sorted.begin(), sorted.end(),
				[&](const WORLD_GAMEPLAY_PLACEMENT& value)
				{ return value.placementId == event.targetId; });
			if (sorted.end() == target ||
				WORLD_PLACEMENT_KIND::DESTROYABLE != target->eKind)
			{
				outStatus = "Gameplay trigger target is not a destroyable: " + event.targetId;
				return false;
			}
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
			<< "      \"kind\": \"" << Kind_ToString(record.eKind) << "\",\n";
		const bool_t actorPlacement =
			WORLD_PLACEMENT_KIND::PLAYER_SPAWN == record.eKind ||
			WORLD_PLACEMENT_KIND::NPC == record.eKind ||
			WORLD_PLACEMENT_KIND::BOSS == record.eKind;
		if (actorPlacement)
		{
			output << "      \"archetypeId\": ";
			if (record.archetypeId.empty()) output << "null";
			else output << '"' << CDataJson::Escape(record.archetypeId) << '"';
			output << ",\n      \"encounterId\": ";
			if (record.encounterId.empty()) output << "null";
			else output << '"' << CDataJson::Escape(record.encounterId) << '"';
			output << ",\n";
		}
		output
			<< "      \"position\": [" << record.position.x << ", " <<
				record.position.y << ", " << record.position.z << "],\n"
			<< "      \"yawDegrees\": " << record.yawDegrees << ",\n"
			<< "      \"enabled\": " <<
				(record.isEnabled ? "true" : "false");
		if (WORLD_PLACEMENT_KIND::TRIGGER_BOX == record.eKind ||
			WORLD_PLACEMENT_KIND::COLLISION_BOX == record.eKind)
		{
			output << ",\n      \"halfExtents\": [" << record.halfExtents.x << ", "
				<< record.halfExtents.y << ", " << record.halfExtents.z << "]";
		}
		if (WORLD_PLACEMENT_KIND::TRIGGER_BOX == record.eKind)
		{
			output << ",\n      \"triggerOnce\": "
				<< (record.isTriggerOnce ? "true" : "false")
				<< ",\n      \"events\": [";
			for (size_t eventIndex = 0; eventIndex < record.triggerEvents.size(); ++eventIndex)
			{
				const WORLD_TRIGGER_EVENT& event = record.triggerEvents[eventIndex];
				output << (0u == eventIndex ? "\n" : ",\n")
					<< "        { \"type\": \"" << TriggerEventKind_ToString(event.eKind) << "\"";
				if (WORLD_TRIGGER_EVENT_KIND::MOVE_PLAYER == event.eKind)
				{
					output << ", \"targetPosition\": [" << event.targetPosition.x << ", "
						<< event.targetPosition.y << ", " << event.targetPosition.z << "], "
						<< "\"durationSeconds\": " << event.durationSeconds << ", "
						<< "\"arcHeight\": " << event.arcHeight;
				}
				else if (WORLD_TRIGGER_EVENT_KIND::CHANGE_LEVEL == event.eKind)
				{
					output << ", \"targetWorldId\": \""
						<< WorldId_ToString(event.eTargetWorldId) << '"';
				}
				else
				{
					output << ", \"targetId\": \"" << CDataJson::Escape(event.targetId)
						<< "\", \"value\": ";
					if (WORLD_TRIGGER_EVENT_KIND::SET_CONDITION == event.eKind)
						output << (event.conditionValue ? "true" : "false");
					else
						output << '"' << DestroyableState_ToString(event.eDestroyableState) << '"';
				}
				output << " }";
			}
			output << (record.triggerEvents.empty() ? "]" : "\n      ]");
		}
		else if (WORLD_PLACEMENT_KIND::DESTROYABLE == record.eKind)
		{
			output << ",\n      \"deployRuntimePlacementId\": \""
				<< record.deployRuntimePlacementId << "\",\n"
				<< "      \"initialState\": \""
				<< DestroyableState_ToString(record.eInitialState) << "\"";
		}
		output << "\n    }";
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
	{
		normalized.archetypeId.clear();
		normalized.encounterId.clear();
	}
	if (WORLD_PLACEMENT_KIND::TRIGGER_BOX == normalized.eKind ||
		WORLD_PLACEMENT_KIND::COLLISION_BOX == normalized.eKind ||
		WORLD_PLACEMENT_KIND::DESTROYABLE == normalized.eKind)
	{
		normalized.archetypeId.clear();
		normalized.encounterId.clear();
	}
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
	const bool_t actorPlacement =
		WORLD_PLACEMENT_KIND::PLAYER_SPAWN == placement.eKind ||
		WORLD_PLACEMENT_KIND::NPC == placement.eKind ||
		WORLD_PLACEMENT_KIND::BOSS == placement.eKind;
	const bool_t hasValidActorReference = !actorPlacement ||
		(WORLD_PLACEMENT_KIND::PLAYER_SPAWN == placement.eKind ?
			placement.archetypeId.empty() && placement.encounterId.empty() :
			Is_ValidStableId(placement.archetypeId));
	const bool_t hasValidTriggerEvents =
		(!placement.isEnabled && placement.triggerEvents.empty()) ||
		(!placement.triggerEvents.empty() &&
			placement.triggerEvents.size() <= 32u &&
			std::all_of(placement.triggerEvents.begin(), placement.triggerEvents.end(),
				[](const WORLD_TRIGGER_EVENT& event)
				{
					if (WORLD_TRIGGER_EVENT_KIND::MOVE_PLAYER == event.eKind)
					{
						return std::isfinite(event.targetPosition.x) &&
							std::isfinite(event.targetPosition.y) &&
							std::isfinite(event.targetPosition.z) &&
							std::abs(event.targetPosition.x) <= 100000.f &&
							std::abs(event.targetPosition.y) <= 100000.f &&
							std::abs(event.targetPosition.z) <= 100000.f &&
							std::isfinite(event.durationSeconds) &&
							event.durationSeconds >= 0.05f && event.durationSeconds <= 10.f &&
							std::isfinite(event.arcHeight) &&
							event.arcHeight >= 0.f && event.arcHeight <= 1000.f;
					}
					if (WORLD_TRIGGER_EVENT_KIND::CHANGE_LEVEL == event.eKind)
					{
						return LostArk::Shared::WORLD_ID::BERN ==
								event.eTargetWorldId ||
							LostArk::Shared::WORLD_ID::VALTAN_ARENA ==
								event.eTargetWorldId;
					}
					return WORLD_TRIGGER_EVENT_KIND::END != event.eKind &&
						Is_ValidStableId(event.targetId) &&
						(WORLD_TRIGGER_EVENT_KIND::SET_CONDITION == event.eKind ||
							WORLD_DESTROYABLE_STATE::END != event.eDestroyableState);
				}));
	const bool_t hasValidTrigger =
		WORLD_PLACEMENT_KIND::TRIGGER_BOX != placement.eKind ||
		(std::isfinite(placement.halfExtents.x) &&
			std::isfinite(placement.halfExtents.y) &&
			std::isfinite(placement.halfExtents.z) &&
			placement.halfExtents.x > 0.f && placement.halfExtents.x <= 1000.f &&
			placement.halfExtents.y > 0.f && placement.halfExtents.y <= 1000.f &&
			placement.halfExtents.z > 0.f && placement.halfExtents.z <= 1000.f &&
			hasValidTriggerEvents);
	const bool_t hasValidCollision =
		WORLD_PLACEMENT_KIND::COLLISION_BOX != placement.eKind ||
		(std::isfinite(placement.halfExtents.x) &&
			std::isfinite(placement.halfExtents.y) &&
			std::isfinite(placement.halfExtents.z) &&
			placement.halfExtents.x > 0.f && placement.halfExtents.x <= 1000.f &&
			placement.halfExtents.y > 0.f && placement.halfExtents.y <= 1000.f &&
			placement.halfExtents.z > 0.f && placement.halfExtents.z <= 1000.f &&
			placement.triggerEvents.empty());
	const bool_t hasValidDestroyable =
		WORLD_PLACEMENT_KIND::DESTROYABLE != placement.eKind ||
		(0u != placement.deployRuntimePlacementId &&
			WORLD_DESTROYABLE_STATE::END != placement.eInitialState);
	return WORLD_PLACEMENT_KIND::END != placement.eKind &&
		Is_ValidStableId(placement.placementId) &&
		hasValidActorReference && hasValidTrigger && hasValidCollision &&
		hasValidDestroyable &&
		(!actorPlacement || placement.encounterId.empty() ||
			Is_ValidStableId(placement.encounterId)) &&
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
	case WORLD_PLACEMENT_KIND::TRIGGER_BOX: return "triggerBox";
	case WORLD_PLACEMENT_KIND::COLLISION_BOX: return "collisionBox";
	case WORLD_PLACEMENT_KIND::DESTROYABLE: return "destroyable";
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
	else if ("triggerBox" == value)
		outKind = WORLD_PLACEMENT_KIND::TRIGGER_BOX;
	else if ("collisionBox" == value)
		outKind = WORLD_PLACEMENT_KIND::COLLISION_BOX;
	else if ("destroyable" == value)
		outKind = WORLD_PLACEMENT_KIND::DESTROYABLE;
	else
		return false;
	return true;
}

const char_t* Client::CWorldGameplayDocument::TriggerEventKind_ToString(
	const WORLD_TRIGGER_EVENT_KIND kind)
{
	switch (kind)
	{
	case WORLD_TRIGGER_EVENT_KIND::MOVE_PLAYER: return "movePlayer";
	case WORLD_TRIGGER_EVENT_KIND::CHANGE_LEVEL: return "changeLevel";
	case WORLD_TRIGGER_EVENT_KIND::SET_CONDITION: return "setCondition";
	case WORLD_TRIGGER_EVENT_KIND::SET_DESTROYABLE_STATE: return "setDestroyableState";
	default: return "invalid";
	}
}

bool_t Client::CWorldGameplayDocument::Try_ParseTriggerEventKind(
	const std::string& value, WORLD_TRIGGER_EVENT_KIND& outKind)
{
	if ("movePlayer" == value)
		outKind = WORLD_TRIGGER_EVENT_KIND::MOVE_PLAYER;
	else if ("changeLevel" == value)
		outKind = WORLD_TRIGGER_EVENT_KIND::CHANGE_LEVEL;
	else if ("setCondition" == value)
		outKind = WORLD_TRIGGER_EVENT_KIND::SET_CONDITION;
	else if ("setDestroyableState" == value)
		outKind = WORLD_TRIGGER_EVENT_KIND::SET_DESTROYABLE_STATE;
	else
		return false;
	return true;
}

const char_t* Client::CWorldGameplayDocument::WorldId_ToString(
	const LostArk::Shared::WORLD_ID worldId)
{
	using LostArk::Shared::WORLD_ID;
	switch (worldId)
	{
	case WORLD_ID::BERN: return "BERN";
	case WORLD_ID::VALTAN_ARENA: return "VALTAN_ARENA";
	default: return "invalid";
	}
}

bool_t Client::CWorldGameplayDocument::Try_ParseWorldId(
	const std::string& value,
	LostArk::Shared::WORLD_ID& outWorldId)
{
	using LostArk::Shared::WORLD_ID;
	if ("BERN" == value)
		outWorldId = WORLD_ID::BERN;
	else if ("VALTAN_ARENA" == value)
		outWorldId = WORLD_ID::VALTAN_ARENA;
	else
		return false;
	return true;
}

const char_t* Client::CWorldGameplayDocument::DestroyableState_ToString(
	const WORLD_DESTROYABLE_STATE state)
{
	switch (state)
	{
	case WORLD_DESTROYABLE_STATE::INTACT: return "INTACT";
	case WORLD_DESTROYABLE_STATE::FRACTURED: return "FRACTURED";
	case WORLD_DESTROYABLE_STATE::DESPAWNED: return "DESPAWNED";
	default: return "invalid";
	}
}

bool_t Client::CWorldGameplayDocument::Try_ParseDestroyableState(
	const std::string& value, WORLD_DESTROYABLE_STATE& outState)
{
	if ("INTACT" == value)
		outState = WORLD_DESTROYABLE_STATE::INTACT;
	else if ("FRACTURED" == value)
		outState = WORLD_DESTROYABLE_STATE::FRACTURED;
	else if ("DESPAWNED" == value)
		outState = WORLD_DESTROYABLE_STATE::DESPAWNED;
	else
		return false;
	return true;
}
