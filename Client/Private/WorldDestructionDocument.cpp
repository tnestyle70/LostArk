#include "WorldDestructionDocument.h"

#include "DataJson.h"

#include <algorithm>
#include <charconv>
#include <cmath>
#include <fstream>
#include <initializer_list>
#include <sstream>
#include <unordered_set>

namespace
{
	using namespace Client;

	constexpr const char_t* SCHEMA = "lostark.world-destruction-events";
	constexpr uint32_t FORMAT_VERSION = 1u;
	/* Every relationship in this document is reconstructed by the team from
	   observed original data, so the whole file carries one provenance value
	   instead of pretending individual rows are extracted truth. */
	constexpr const char_t* PROVENANCE = "PROJECT_TUNED";

	bool_t Read_TextFile(
		const std::filesystem::path& path,
		std::string& outText)
	{
		std::ifstream input(path, std::ios::binary);
		if (!input.is_open())
			return false;
		std::ostringstream buffer;
		buffer << input.rdbuf();
		if (input.bad())
			return false;
		outText = buffer.str();
		return true;
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

	bool_t Read_String(
		const DATA_JSON_VALUE& parent,
		const char_t* key,
		const bool_t allowEmpty,
		std::string& outValue)
	{
		const DATA_JSON_VALUE* value = parent.Find(key);
		if (nullptr == value || !value->Is_String() ||
			(!allowEmpty && value->Get_String().empty()))
		{
			return false;
		}
		outValue = value->Get_String();
		return true;
	}

	bool_t Read_Unsigned(
		const DATA_JSON_VALUE& parent,
		const char_t* key,
		const uint32_t maximum,
		uint32_t& outValue)
	{
		const DATA_JSON_VALUE* value = parent.Find(key);
		if (nullptr == value || !value->Is_Number())
			return false;
		const double number = value->Get_Number();
		if (!std::isfinite(number) || std::floor(number) != number ||
			number < 0.0 || number > static_cast<double>(maximum))
		{
			return false;
		}
		outValue = static_cast<uint32_t>(number);
		return true;
	}

	bool_t Read_Bool(
		const DATA_JSON_VALUE& parent,
		const char_t* key,
		bool_t& outValue)
	{
		const DATA_JSON_VALUE* value = parent.Find(key);
		if (nullptr == value || !value->Is_Boolean())
			return false;
		outValue = value->Get_Boolean();
		return true;
	}

	/* 64-bit deploy identities lose precision as JSON numbers, so they are
	   stored and parsed as decimal strings. */
	bool_t Parse_Uint64(const std::string& text, uint64_t& outValue)
	{
		if (text.empty() || text.size() > 20u ||
			!std::all_of(text.begin(), text.end(),
				[](const char_t character)
				{
					return character >= '0' && character <= '9';
				}))
		{
			return false;
		}
		uint64_t parsed = 0u;
		const auto result = std::from_chars(
			text.data(), text.data() + text.size(), parsed);
		if (result.ec != std::errc{} ||
			result.ptr != text.data() + text.size())
		{
			return false;
		}
		outValue = parsed;
		return true;
	}

	bool_t Commit_TemporaryFile(
		const std::filesystem::path& destination,
		const std::filesystem::path& temporary)
	{
		std::error_code existsError;
		if (std::filesystem::exists(destination, existsError) &&
			!existsError && ReplaceFileW(
				destination.c_str(), temporary.c_str(), nullptr,
				REPLACEFILE_WRITE_THROUGH, nullptr, nullptr))
		{
			return true;
		}
		return FALSE != MoveFileExW(
			temporary.c_str(), destination.c_str(),
			MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
	}
}

bool_t Client::CWorldDestructionDocument::Is_StableId(
	const std::string& value)
{
	if (value.empty() || value.size() > 128u)
		return false;
	return std::all_of(value.begin(), value.end(),
		[](const char_t character)
		{
			return (character >= 'a' && character <= 'z') ||
				(character >= 'A' && character <= 'Z') ||
				(character >= '0' && character <= '9') ||
				'.' == character || '_' == character || '-' == character;
		});
}

const char_t* Client::CWorldDestructionDocument::NavPolarity_ToString(
	const DESTRUCTION_NAV_POLARITY polarity)
{
	switch (polarity)
	{
	case DESTRUCTION_NAV_POLARITY::BLOCK_WHILE_INTACT:
		return "BLOCK_WHILE_INTACT";
	case DESTRUCTION_NAV_POLARITY::BLOCK_WHILE_FRACTURED:
		return "BLOCK_WHILE_FRACTURED";
	default:
		return "";
	}
}

bool_t Client::CWorldDestructionDocument::Try_ParseNavPolarity(
	const std::string& value,
	DESTRUCTION_NAV_POLARITY& outPolarity)
{
	if ("BLOCK_WHILE_INTACT" == value)
		outPolarity = DESTRUCTION_NAV_POLARITY::BLOCK_WHILE_INTACT;
	else if ("BLOCK_WHILE_FRACTURED" == value)
		outPolarity = DESTRUCTION_NAV_POLARITY::BLOCK_WHILE_FRACTURED;
	else
		return false;
	return true;
}

const char_t* Client::CWorldDestructionDocument::TriggerKind_ToString(
	const DESTRUCTION_TRIGGER_KIND kind)
{
	switch (kind)
	{
	case DESTRUCTION_TRIGGER_KIND::STAGE_ENTER:
		return "STAGE_ENTER";
	case DESTRUCTION_TRIGGER_KIND::STAGE_TIME:
		return "STAGE_TIME";
	case DESTRUCTION_TRIGGER_KIND::STAGE_EXIT:
		return "STAGE_EXIT";
	case DESTRUCTION_TRIGGER_KIND::COLLISION_IMPACT:
		return "COLLISION_IMPACT";
	default:
		return "";
	}
}

bool_t Client::CWorldDestructionDocument::Try_ParseTriggerKind(
	const std::string& value,
	DESTRUCTION_TRIGGER_KIND& outKind)
{
	if ("STAGE_ENTER" == value)
		outKind = DESTRUCTION_TRIGGER_KIND::STAGE_ENTER;
	else if ("STAGE_TIME" == value)
		outKind = DESTRUCTION_TRIGGER_KIND::STAGE_TIME;
	else if ("STAGE_EXIT" == value)
		outKind = DESTRUCTION_TRIGGER_KIND::STAGE_EXIT;
	else if ("COLLISION_IMPACT" == value)
		outKind = DESTRUCTION_TRIGGER_KIND::COLLISION_IMPACT;
	else
		return false;
	return true;
}

void Client::CWorldDestructionDocument::Reset_Empty()
{
	m_Groups.clear();
	m_Mutations.clear();
	m_Bindings.clear();
	m_isReady = true;
	m_isDirty = false;
}

void Client::CWorldDestructionDocument::Clear()
{
	m_Groups.clear();
	m_Mutations.clear();
	m_Bindings.clear();
	m_isReady = false;
	m_isDirty = false;
}

bool_t Client::CWorldDestructionDocument::Load(
	const std::filesystem::path& path,
	const std::string& expectedAreaId,
	const std::string& expectedEncounterId,
	std::string& outStatus)
{
	std::string text;
	if (path.empty() || !Read_TextFile(path, text))
	{
		outStatus = "World destruction document is unreadable: " +
			path.string();
		return false;
	}

	std::string parseError;
	DATA_JSON_VALUE root;
	if (!CDataJson::Parse(text, root, parseError))
	{
		outStatus = "World destruction parse failed: " + parseError;
		return false;
	}
	if (!Is_ExactObject(root, {
			"schema", "formatVersion", "areaId", "encounterId", "provenance",
			"groups", "mutations", "bindings" }))
	{
		outStatus = "World destruction root has unexpected properties";
		return false;
	}

	std::string schema;
	std::string areaId;
	std::string encounterId;
	std::string provenance;
	uint32_t formatVersion = 0u;
	if (!Read_String(root, "schema", false, schema) || SCHEMA != schema ||
		!Read_Unsigned(root, "formatVersion", FORMAT_VERSION, formatVersion) ||
		FORMAT_VERSION != formatVersion ||
		!Read_String(root, "areaId", false, areaId) ||
		areaId != expectedAreaId ||
		!Read_String(root, "encounterId", false, encounterId) ||
		encounterId != expectedEncounterId ||
		!Read_String(root, "provenance", false, provenance) ||
		PROVENANCE != provenance)
	{
		outStatus = "World destruction header does not match this Area";
		return false;
	}

	const DATA_JSON_VALUE* groups = root.Find("groups");
	const DATA_JSON_VALUE* mutations = root.Find("mutations");
	const DATA_JSON_VALUE* bindings = root.Find("bindings");
	if (nullptr == groups || !groups->Is_Array() ||
		groups->Get_Array().size() > MAX_GROUP_COUNT ||
		nullptr == mutations || !mutations->Is_Array() ||
		mutations->Get_Array().size() > MAX_MUTATION_COUNT ||
		nullptr == bindings || !bindings->Is_Array() ||
		bindings->Get_Array().size() > MAX_BINDING_COUNT)
	{
		outStatus = "World destruction arrays are invalid or over limit";
		return false;
	}

	std::vector<DESTRUCTION_GROUP> stagedGroups;
	std::vector<DESTRUCTION_MUTATION> stagedMutations;
	std::vector<DESTRUCTION_BINDING> stagedBindings;
	stagedGroups.reserve(groups->Get_Array().size());
	stagedMutations.reserve(mutations->Get_Array().size());
	stagedBindings.reserve(bindings->Get_Array().size());

	for (const DATA_JSON_VALUE& entry : groups->Get_Array())
	{
		if (!Is_ExactObject(entry, {
				"groupId", "memberPlacementIds", "navigationRegionIds",
				"navPolarity", "initialState" }))
		{
			outStatus = "World destruction group has unexpected properties";
			return false;
		}

		DESTRUCTION_GROUP group;
		std::string polarity;
		std::string initialState;
		if (!Read_String(entry, "groupId", false, group.groupId) ||
			!Is_StableId(group.groupId) ||
			!Read_String(entry, "navPolarity", false, polarity) ||
			!Try_ParseNavPolarity(polarity, group.eNavPolarity) ||
			!Read_String(entry, "initialState", false, initialState) ||
			!CWorldGameplayDocument::Try_ParseDestroyableState(
				initialState, group.eInitialState))
		{
			outStatus = "World destruction group field is invalid";
			return false;
		}

		const DATA_JSON_VALUE* members = entry.Find("memberPlacementIds");
		const DATA_JSON_VALUE* regions = entry.Find("navigationRegionIds");
		if (nullptr == members || !members->Is_Array() ||
			members->Get_Array().size() > MAX_MEMBER_COUNT ||
			nullptr == regions || !regions->Is_Array() ||
			regions->Get_Array().size() > MAX_REGION_REF_COUNT)
		{
			outStatus = "World destruction group array is invalid: " +
				group.groupId;
			return false;
		}
		for (const DATA_JSON_VALUE& member : members->Get_Array())
		{
			uint64_t placementId = 0u;
			if (!member.Is_String() ||
				!Parse_Uint64(member.Get_String(), placementId) ||
				0u == placementId)
			{
				outStatus = "World destruction member ID is invalid: " +
					group.groupId;
				return false;
			}
			group.memberPlacementIds.push_back(placementId);
		}
		for (const DATA_JSON_VALUE& region : regions->Get_Array())
		{
			if (!region.Is_String() || !Is_StableId(region.Get_String()))
			{
				outStatus = "World destruction region ID is invalid: " +
					group.groupId;
				return false;
			}
			group.navigationRegionIds.push_back(region.Get_String());
		}
		stagedGroups.push_back(std::move(group));
	}

	for (const DATA_JSON_VALUE& entry : mutations->Get_Array())
	{
		if (!Is_ExactObject(entry, {
				"mutationId", "groupId", "targetState",
				"breakingDurationMs" }))
		{
			outStatus = "World destruction mutation has unexpected properties";
			return false;
		}
		DESTRUCTION_MUTATION mutation;
		std::string targetState;
		if (!Read_String(entry, "mutationId", false, mutation.mutationId) ||
			!Is_StableId(mutation.mutationId) ||
			!Read_String(entry, "groupId", false, mutation.groupId) ||
			!Is_StableId(mutation.groupId) ||
			!Read_String(entry, "targetState", false, targetState) ||
			!CWorldGameplayDocument::Try_ParseDestroyableState(
				targetState, mutation.eTargetState) ||
			!Read_Unsigned(entry, "breakingDurationMs", MAX_DURATION_MS,
				mutation.iBreakingDurationMs))
		{
			outStatus = "World destruction mutation field is invalid";
			return false;
		}
		stagedMutations.push_back(std::move(mutation));
	}

	for (const DATA_JSON_VALUE& entry : bindings->Get_Array())
	{
		if (!Is_ExactObject(entry, {
				"bindingId", "mutationId", "patternId", "stageId",
				"triggerKind", "offsetMs", "receiverCollisionId",
				"enabled" }))
		{
			outStatus = "World destruction binding has unexpected properties";
			return false;
		}
		DESTRUCTION_BINDING binding;
		std::string triggerKind;
		if (!Read_String(entry, "bindingId", false, binding.bindingId) ||
			!Is_StableId(binding.bindingId) ||
			!Read_String(entry, "mutationId", false, binding.mutationId) ||
			!Is_StableId(binding.mutationId) ||
			!Read_String(entry, "patternId", false, binding.patternId) ||
			!Is_StableId(binding.patternId) ||
			!Read_String(entry, "stageId", false, binding.stageId) ||
			!Is_StableId(binding.stageId) ||
			!Read_String(entry, "triggerKind", false, triggerKind) ||
			!Try_ParseTriggerKind(triggerKind, binding.eTriggerKind) ||
			!Read_Unsigned(entry, "offsetMs", MAX_DURATION_MS,
				binding.iOffsetMs) ||
			!Read_String(entry, "receiverCollisionId", true,
				binding.receiverCollisionId) ||
			!Read_Bool(entry, "enabled", binding.isEnabled))
		{
			outStatus = "World destruction binding field is invalid";
			return false;
		}
		if (!binding.receiverCollisionId.empty() &&
			!Is_StableId(binding.receiverCollisionId))
		{
			outStatus = "World destruction receiver ID is invalid: " +
				binding.bindingId;
			return false;
		}
		stagedBindings.push_back(std::move(binding));
	}

	CWorldDestructionDocument staged;
	staged.m_Groups = std::move(stagedGroups);
	staged.m_Mutations = std::move(stagedMutations);
	staged.m_Bindings = std::move(stagedBindings);
	if (!staged.Validate_Graph(outStatus))
		return false;

	m_Groups = std::move(staged.m_Groups);
	m_Mutations = std::move(staged.m_Mutations);
	m_Bindings = std::move(staged.m_Bindings);
	m_isReady = true;
	m_isDirty = false;
	outStatus = "Loaded world destruction events: " +
		std::to_string(m_Groups.size()) + " groups, " +
		std::to_string(m_Mutations.size()) + " mutations, " +
		std::to_string(m_Bindings.size()) + " bindings";
	return true;
}

bool_t Client::CWorldDestructionDocument::Save(
	const std::filesystem::path& path,
	const std::string& areaId,
	const std::string& encounterId,
	std::string& outStatus) const
{
	if (path.empty() || areaId.empty() || encounterId.empty())
	{
		outStatus = "World destruction save header is invalid";
		return false;
	}
	if (!Validate_Graph(outStatus))
		return false;

	std::error_code directoryError;
	std::filesystem::create_directories(path.parent_path(), directoryError);
	if (directoryError)
	{
		outStatus = "Could not create world destruction directory";
		return false;
	}

	const std::filesystem::path temporary = path.wstring() + L".tmp";
	std::ofstream output(temporary, std::ios::binary | std::ios::trunc);
	if (!output)
	{
		outStatus = "Could not create world destruction temporary file";
		return false;
	}

	output << "{\n"
		<< "  \"schema\": \"" << SCHEMA << "\",\n"
		<< "  \"formatVersion\": " << FORMAT_VERSION << ",\n"
		<< "  \"areaId\": \"" << CDataJson::Escape(areaId) << "\",\n"
		<< "  \"encounterId\": \"" << CDataJson::Escape(encounterId)
		<< "\",\n"
		<< "  \"provenance\": \"" << PROVENANCE << "\",\n"
		<< "  \"groups\": [";
	for (size_t index = 0; index < m_Groups.size(); ++index)
	{
		const DESTRUCTION_GROUP& group = m_Groups[index];
		output << (0u == index ? "\n" : ",\n")
			<< "    {\n"
			<< "      \"groupId\": \"" << CDataJson::Escape(group.groupId)
			<< "\",\n"
			<< "      \"memberPlacementIds\": [";
		for (size_t member = 0; member < group.memberPlacementIds.size();
			++member)
		{
			output << (0u == member ? "" : ", ") << '"'
				<< group.memberPlacementIds[member] << '"';
		}
		output << "],\n      \"navigationRegionIds\": [";
		for (size_t region = 0; region < group.navigationRegionIds.size();
			++region)
		{
			output << (0u == region ? "" : ", ") << '"'
				<< CDataJson::Escape(group.navigationRegionIds[region])
				<< '"';
		}
		output << "],\n"
			<< "      \"navPolarity\": \""
			<< NavPolarity_ToString(group.eNavPolarity) << "\",\n"
			<< "      \"initialState\": \""
			<< CWorldGameplayDocument::DestroyableState_ToString(
				group.eInitialState)
			<< "\"\n    }";
	}
	output << (m_Groups.empty() ? "],\n" : "\n  ],\n")
		<< "  \"mutations\": [";
	for (size_t index = 0; index < m_Mutations.size(); ++index)
	{
		const DESTRUCTION_MUTATION& mutation = m_Mutations[index];
		output << (0u == index ? "\n" : ",\n")
			<< "    { \"mutationId\": \""
			<< CDataJson::Escape(mutation.mutationId) << "\", \"groupId\": \""
			<< CDataJson::Escape(mutation.groupId) << "\", \"targetState\": \""
			<< CWorldGameplayDocument::DestroyableState_ToString(
				mutation.eTargetState)
			<< "\", \"breakingDurationMs\": " << mutation.iBreakingDurationMs
			<< " }";
	}
	output << (m_Mutations.empty() ? "],\n" : "\n  ],\n")
		<< "  \"bindings\": [";
	for (size_t index = 0; index < m_Bindings.size(); ++index)
	{
		const DESTRUCTION_BINDING& binding = m_Bindings[index];
		output << (0u == index ? "\n" : ",\n")
			<< "    {\n"
			<< "      \"bindingId\": \""
			<< CDataJson::Escape(binding.bindingId) << "\",\n"
			<< "      \"mutationId\": \""
			<< CDataJson::Escape(binding.mutationId) << "\",\n"
			<< "      \"patternId\": \""
			<< CDataJson::Escape(binding.patternId) << "\",\n"
			<< "      \"stageId\": \""
			<< CDataJson::Escape(binding.stageId) << "\",\n"
			<< "      \"triggerKind\": \""
			<< TriggerKind_ToString(binding.eTriggerKind) << "\",\n"
			<< "      \"offsetMs\": " << binding.iOffsetMs << ",\n"
			<< "      \"receiverCollisionId\": \""
			<< CDataJson::Escape(binding.receiverCollisionId) << "\",\n"
			<< "      \"enabled\": "
			<< (binding.isEnabled ? "true" : "false") << "\n    }";
	}
	output << (m_Bindings.empty() ? "]\n" : "\n  ]\n") << "}\n";
	output.flush();
	const bool_t writeSucceeded = output.good();
	output.close();

	/* Reparse the staged file before it replaces the destination so a broken
	   writer can never leave an unreadable authoring document behind. */
	CWorldDestructionDocument verify;
	std::string verifyStatus;
	if (!writeSucceeded ||
		!verify.Load(temporary, areaId, encounterId, verifyStatus))
	{
		std::error_code removeError;
		std::filesystem::remove(temporary, removeError);
		outStatus = writeSucceeded ?
			"World destruction reload check failed: " + verifyStatus :
			"World destruction write failed";
		return false;
	}
	if (!Commit_TemporaryFile(path, temporary))
	{
		std::error_code removeError;
		std::filesystem::remove(temporary, removeError);
		outStatus = "World destruction atomic replace failed";
		return false;
	}

	outStatus = "Saved world destruction events: " +
		std::to_string(m_Groups.size()) + " groups, " +
		std::to_string(m_Bindings.size()) + " bindings";
	return true;
}

bool_t Client::CWorldDestructionDocument::Validate_Graph(
	std::string& outStatus) const
{
	std::unordered_set<std::string> groupIds;
	std::unordered_set<uint64_t> memberIds;
	for (const DESTRUCTION_GROUP& group : m_Groups)
	{
		if (!Is_StableId(group.groupId) ||
			!groupIds.insert(group.groupId).second)
		{
			outStatus = "Duplicate or invalid destruction group: " +
				group.groupId;
			return false;
		}
		if (DESTRUCTION_NAV_POLARITY::END == group.eNavPolarity ||
			WORLD_DESTROYABLE_STATE::END == group.eInitialState)
		{
			outStatus = "Destruction group has an unset enum: " + group.groupId;
			return false;
		}
		std::unordered_set<std::string> regionIds;
		for (const std::string& regionId : group.navigationRegionIds)
		{
			if (!Is_StableId(regionId) || !regionIds.insert(regionId).second)
			{
				outStatus = "Duplicate or invalid navigation region in " +
					group.groupId;
				return false;
			}
		}
		/* A deploy prop may belong to exactly one group, otherwise two groups
		   could drive the same wall to different states in the same tick. */
		for (const uint64_t placementId : group.memberPlacementIds)
		{
			if (0u == placementId || !memberIds.insert(placementId).second)
			{
				outStatus = "Deploy placement is in more than one group: " +
					std::to_string(placementId);
				return false;
			}
		}
	}

	std::unordered_set<std::string> mutationIds;
	for (const DESTRUCTION_MUTATION& mutation : m_Mutations)
	{
		if (!Is_StableId(mutation.mutationId) ||
			!mutationIds.insert(mutation.mutationId).second)
		{
			outStatus = "Duplicate or invalid destruction mutation: " +
				mutation.mutationId;
			return false;
		}
		if (WORLD_DESTROYABLE_STATE::END == mutation.eTargetState ||
			WORLD_DESTROYABLE_STATE::INTACT == mutation.eTargetState)
		{
			outStatus = "Mutation must target FRACTURED or DESPAWNED: " +
				mutation.mutationId;
			return false;
		}
		if (0u == groupIds.count(mutation.groupId))
		{
			outStatus = "Mutation references an unknown group: " +
				mutation.mutationId;
			return false;
		}
	}

	std::unordered_set<std::string> bindingIds;
	for (const DESTRUCTION_BINDING& binding : m_Bindings)
	{
		if (!Is_StableId(binding.bindingId) ||
			!bindingIds.insert(binding.bindingId).second)
		{
			outStatus = "Duplicate or invalid destruction binding: " +
				binding.bindingId;
			return false;
		}
		if (DESTRUCTION_TRIGGER_KIND::END == binding.eTriggerKind)
		{
			outStatus = "Binding has an unset trigger kind: " +
				binding.bindingId;
			return false;
		}
		if (0u == mutationIds.count(binding.mutationId))
		{
			outStatus = "Binding references an unknown mutation: " +
				binding.bindingId;
			return false;
		}
		const bool_t needsReceiver =
			DESTRUCTION_TRIGGER_KIND::COLLISION_IMPACT == binding.eTriggerKind;
		if (needsReceiver == binding.receiverCollisionId.empty())
		{
			outStatus = needsReceiver ?
				"COLLISION_IMPACT binding needs a receiver: " +
					binding.bindingId :
				"Only COLLISION_IMPACT may set a receiver: " +
					binding.bindingId;
			return false;
		}
		if (DESTRUCTION_TRIGGER_KIND::STAGE_TIME != binding.eTriggerKind &&
			0u != binding.iOffsetMs)
		{
			outStatus = "Only STAGE_TIME may set offsetMs: " +
				binding.bindingId;
			return false;
		}
		/* An enabled binding is a request the Server will have to honour, so a
		   draft without its target resolved stays disabled instead of being
		   silently accepted. */
		if (binding.isEnabled && binding.patternId.empty())
		{
			outStatus = "Enabled binding needs a pattern: " +
				binding.bindingId;
			return false;
		}
	}
	return true;
}

const Client::DESTRUCTION_GROUP*
Client::CWorldDestructionDocument::Find_Group(
	const std::string& groupId) const
{
	const auto iter = std::find_if(m_Groups.begin(), m_Groups.end(),
		[&groupId](const DESTRUCTION_GROUP& group)
		{
			return group.groupId == groupId;
		});
	return m_Groups.end() == iter ? nullptr : &(*iter);
}

Client::DESTRUCTION_GROUP*
Client::CWorldDestructionDocument::Find_GroupInternal(
	const std::string& groupId)
{
	const auto iter = std::find_if(m_Groups.begin(), m_Groups.end(),
		[&groupId](const DESTRUCTION_GROUP& group)
		{
			return group.groupId == groupId;
		});
	return m_Groups.end() == iter ? nullptr : &(*iter);
}

const Client::DESTRUCTION_MUTATION*
Client::CWorldDestructionDocument::Find_Mutation(
	const std::string& mutationId) const
{
	const auto iter = std::find_if(m_Mutations.begin(), m_Mutations.end(),
		[&mutationId](const DESTRUCTION_MUTATION& mutation)
		{
			return mutation.mutationId == mutationId;
		});
	return m_Mutations.end() == iter ? nullptr : &(*iter);
}

const Client::DESTRUCTION_BINDING*
Client::CWorldDestructionDocument::Find_Binding(
	const std::string& bindingId) const
{
	const auto iter = std::find_if(m_Bindings.begin(), m_Bindings.end(),
		[&bindingId](const DESTRUCTION_BINDING& binding)
		{
			return binding.bindingId == bindingId;
		});
	return m_Bindings.end() == iter ? nullptr : &(*iter);
}

const Client::DESTRUCTION_GROUP*
Client::CWorldDestructionDocument::Find_GroupOfMember(
	const uint64_t runtimePlacementId) const
{
	for (const DESTRUCTION_GROUP& group : m_Groups)
	{
		if (group.memberPlacementIds.end() != std::find(
			group.memberPlacementIds.begin(),
			group.memberPlacementIds.end(),
			runtimePlacementId))
		{
			return &group;
		}
	}
	return nullptr;
}

bool_t Client::CWorldDestructionDocument::Add_Group(
	const std::string& groupId,
	std::string& outStatus)
{
	if (!Is_StableId(groupId) || m_Groups.size() >= MAX_GROUP_COUNT ||
		nullptr != Find_Group(groupId))
	{
		outStatus = "Group ID is invalid, duplicate, or over limit";
		return false;
	}
	DESTRUCTION_GROUP group;
	group.groupId = groupId;
	m_Groups.push_back(std::move(group));
	m_isReady = true;
	m_isDirty = true;
	outStatus = "Added destruction group: " + groupId;
	return true;
}

bool_t Client::CWorldDestructionDocument::Remove_Group(
	const std::string& groupId,
	std::string& outStatus)
{
	const auto referencing = std::find_if(
		m_Mutations.begin(), m_Mutations.end(),
		[&groupId](const DESTRUCTION_MUTATION& mutation)
		{
			return mutation.groupId == groupId;
		});
	if (m_Mutations.end() != referencing)
	{
		outStatus = "Remove the mutations that target this group first: " +
			referencing->mutationId;
		return false;
	}
	const auto iter = std::find_if(m_Groups.begin(), m_Groups.end(),
		[&groupId](const DESTRUCTION_GROUP& group)
		{
			return group.groupId == groupId;
		});
	if (m_Groups.end() == iter)
	{
		outStatus = "Unknown destruction group: " + groupId;
		return false;
	}
	m_Groups.erase(iter);
	m_isDirty = true;
	outStatus = "Removed destruction group: " + groupId;
	return true;
}

bool_t Client::CWorldDestructionDocument::Add_Member(
	const std::string& groupId,
	const uint64_t runtimePlacementId,
	std::string& outStatus)
{
	if (0u == runtimePlacementId)
	{
		outStatus = "Deploy placement ID is invalid";
		return false;
	}
	const DESTRUCTION_GROUP* owner = Find_GroupOfMember(runtimePlacementId);
	if (nullptr != owner)
	{
		outStatus = "Deploy placement already belongs to " + owner->groupId;
		return false;
	}
	DESTRUCTION_GROUP* group = Find_GroupInternal(groupId);
	if (nullptr == group ||
		group->memberPlacementIds.size() >= MAX_MEMBER_COUNT)
	{
		outStatus = "Unknown group or member limit reached: " + groupId;
		return false;
	}
	group->memberPlacementIds.push_back(runtimePlacementId);
	m_isDirty = true;
	outStatus = "Added member " + std::to_string(runtimePlacementId) +
		" to " + groupId;
	return true;
}

bool_t Client::CWorldDestructionDocument::Remove_Member(
	const std::string& groupId,
	const uint64_t runtimePlacementId)
{
	DESTRUCTION_GROUP* group = Find_GroupInternal(groupId);
	if (nullptr == group)
		return false;
	const auto iter = std::find(group->memberPlacementIds.begin(),
		group->memberPlacementIds.end(), runtimePlacementId);
	if (group->memberPlacementIds.end() == iter)
		return false;
	group->memberPlacementIds.erase(iter);
	m_isDirty = true;
	return true;
}

bool_t Client::CWorldDestructionDocument::Set_NavPolarity(
	const std::string& groupId,
	const DESTRUCTION_NAV_POLARITY polarity)
{
	DESTRUCTION_GROUP* group = Find_GroupInternal(groupId);
	if (nullptr == group || DESTRUCTION_NAV_POLARITY::END == polarity)
		return false;
	if (group->eNavPolarity == polarity)
		return true;
	group->eNavPolarity = polarity;
	m_isDirty = true;
	return true;
}

bool_t Client::CWorldDestructionDocument::Set_InitialState(
	const std::string& groupId,
	const WORLD_DESTROYABLE_STATE state)
{
	DESTRUCTION_GROUP* group = Find_GroupInternal(groupId);
	if (nullptr == group || WORLD_DESTROYABLE_STATE::END == state)
		return false;
	if (group->eInitialState == state)
		return true;
	group->eInitialState = state;
	m_isDirty = true;
	return true;
}

bool_t Client::CWorldDestructionDocument::Add_NavigationRegion(
	const std::string& groupId,
	const std::string& regionId,
	std::string& outStatus)
{
	DESTRUCTION_GROUP* group = Find_GroupInternal(groupId);
	if (nullptr == group || !Is_StableId(regionId) ||
		group->navigationRegionIds.size() >= MAX_REGION_REF_COUNT)
	{
		outStatus = "Unknown group, invalid region ID, or limit reached";
		return false;
	}
	if (group->navigationRegionIds.end() != std::find(
		group->navigationRegionIds.begin(),
		group->navigationRegionIds.end(), regionId))
	{
		outStatus = "Region is already linked: " + regionId;
		return false;
	}
	group->navigationRegionIds.push_back(regionId);
	m_isDirty = true;
	outStatus = "Linked navigation region: " + regionId;
	return true;
}

bool_t Client::CWorldDestructionDocument::Remove_NavigationRegion(
	const std::string& groupId,
	const std::string& regionId)
{
	DESTRUCTION_GROUP* group = Find_GroupInternal(groupId);
	if (nullptr == group)
		return false;
	const auto iter = std::find(group->navigationRegionIds.begin(),
		group->navigationRegionIds.end(), regionId);
	if (group->navigationRegionIds.end() == iter)
		return false;
	group->navigationRegionIds.erase(iter);
	m_isDirty = true;
	return true;
}

bool_t Client::CWorldDestructionDocument::Add_Mutation(
	const DESTRUCTION_MUTATION& mutation,
	std::string& outStatus)
{
	if (m_Mutations.size() >= MAX_MUTATION_COUNT ||
		!Is_StableId(mutation.mutationId) ||
		nullptr != Find_Mutation(mutation.mutationId) ||
		nullptr == Find_Group(mutation.groupId) ||
		mutation.iBreakingDurationMs > MAX_DURATION_MS ||
		(WORLD_DESTROYABLE_STATE::FRACTURED != mutation.eTargetState &&
			WORLD_DESTROYABLE_STATE::DESPAWNED != mutation.eTargetState))
	{
		outStatus = "Mutation is invalid, duplicate, or over limit";
		return false;
	}
	m_Mutations.push_back(mutation);
	m_isDirty = true;
	outStatus = "Added mutation: " + mutation.mutationId;
	return true;
}

bool_t Client::CWorldDestructionDocument::Update_Mutation(
	const DESTRUCTION_MUTATION& mutation,
	std::string& outStatus)
{
	const auto iter = std::find_if(m_Mutations.begin(), m_Mutations.end(),
		[&mutation](const DESTRUCTION_MUTATION& value)
		{
			return value.mutationId == mutation.mutationId;
		});
	if (m_Mutations.end() == iter ||
		nullptr == Find_Group(mutation.groupId) ||
		mutation.iBreakingDurationMs > MAX_DURATION_MS ||
		(WORLD_DESTROYABLE_STATE::FRACTURED != mutation.eTargetState &&
			WORLD_DESTROYABLE_STATE::DESPAWNED != mutation.eTargetState))
	{
		outStatus = "Mutation update is invalid";
		return false;
	}
	*iter = mutation;
	m_isDirty = true;
	outStatus = "Updated mutation: " + mutation.mutationId;
	return true;
}

bool_t Client::CWorldDestructionDocument::Remove_Mutation(
	const std::string& mutationId,
	std::string& outStatus)
{
	const auto referencing = std::find_if(
		m_Bindings.begin(), m_Bindings.end(),
		[&mutationId](const DESTRUCTION_BINDING& binding)
		{
			return binding.mutationId == mutationId;
		});
	if (m_Bindings.end() != referencing)
	{
		outStatus = "Remove the bindings that use this mutation first: " +
			referencing->bindingId;
		return false;
	}
	const auto iter = std::find_if(m_Mutations.begin(), m_Mutations.end(),
		[&mutationId](const DESTRUCTION_MUTATION& mutation)
		{
			return mutation.mutationId == mutationId;
		});
	if (m_Mutations.end() == iter)
	{
		outStatus = "Unknown mutation: " + mutationId;
		return false;
	}
	m_Mutations.erase(iter);
	m_isDirty = true;
	outStatus = "Removed mutation: " + mutationId;
	return true;
}

bool_t Client::CWorldDestructionDocument::Add_Binding(
	const DESTRUCTION_BINDING& binding,
	std::string& outStatus)
{
	if (m_Bindings.size() >= MAX_BINDING_COUNT ||
		!Is_StableId(binding.bindingId) ||
		nullptr != Find_Binding(binding.bindingId) ||
		nullptr == Find_Mutation(binding.mutationId))
	{
		outStatus = "Binding is invalid, duplicate, or over limit";
		return false;
	}
	std::vector<DESTRUCTION_BINDING> staged = m_Bindings;
	staged.push_back(binding);
	CWorldDestructionDocument probe;
	probe.m_Groups = m_Groups;
	probe.m_Mutations = m_Mutations;
	probe.m_Bindings = std::move(staged);
	if (!probe.Validate_Graph(outStatus))
		return false;
	m_Bindings = std::move(probe.m_Bindings);
	m_isDirty = true;
	outStatus = "Added binding: " + binding.bindingId;
	return true;
}

bool_t Client::CWorldDestructionDocument::Update_Binding(
	const DESTRUCTION_BINDING& binding,
	std::string& outStatus)
{
	const auto iter = std::find_if(m_Bindings.begin(), m_Bindings.end(),
		[&binding](const DESTRUCTION_BINDING& value)
		{
			return value.bindingId == binding.bindingId;
		});
	if (m_Bindings.end() == iter ||
		nullptr == Find_Mutation(binding.mutationId))
	{
		outStatus = "Binding update is invalid";
		return false;
	}
	CWorldDestructionDocument probe;
	probe.m_Groups = m_Groups;
	probe.m_Mutations = m_Mutations;
	probe.m_Bindings = m_Bindings;
	probe.m_Bindings[static_cast<size_t>(
		std::distance(m_Bindings.begin(), iter))] = binding;
	if (!probe.Validate_Graph(outStatus))
		return false;
	m_Bindings = std::move(probe.m_Bindings);
	m_isDirty = true;
	outStatus = "Updated binding: " + binding.bindingId;
	return true;
}

bool_t Client::CWorldDestructionDocument::Remove_Binding(
	const std::string& bindingId)
{
	const auto iter = std::find_if(m_Bindings.begin(), m_Bindings.end(),
		[&bindingId](const DESTRUCTION_BINDING& binding)
		{
			return binding.bindingId == bindingId;
		});
	if (m_Bindings.end() == iter)
		return false;
	m_Bindings.erase(iter);
	m_isDirty = true;
	return true;
}
