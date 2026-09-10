#include "imgui.h"

#include "KoukuSaydonBossTool.h"

#include "DataJson.h"
#include "KoukuSaydonPatternAuditionService.h"
#include "KoukuSaydonCompositionDocument.h"
#include "NetworkManager.h"
#include "ProjectDataRoot.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <string_view>
#include <unordered_set>

namespace
{
	constexpr ImVec4 LIVE_COLOR{ 0.3f, 1.f, 0.45f, 1.f };
	constexpr std::string_view PRODUCT_SCHEMA = "lostark.encounter-profile";
	constexpr std::uint32_t PRODUCT_VERSION = 4u;
	constexpr std::string_view ENCOUNTER_ID = "ENCOUNTER_KAKULSAYDON_G1";
	constexpr std::string_view BOSS_ARCHETYPE_ID =
		"BOSS_KAKULSAYDON_G1_KOUKU";
	constexpr std::uintmax_t MAX_PRODUCT_BYTES = 8u * 1024u * 1024u;

	const Client::DATA_JSON_VALUE* Required(
		const Client::DATA_JSON_VALUE& object,
		const char* const name,
		const Client::DATA_JSON_TYPE type)
	{
		const Client::DATA_JSON_VALUE* const value = object.Find(name);
		return nullptr != value && value->Get_Type() == type ? value : nullptr;
	}

	bool Has_ExactProperties(
		const Client::DATA_JSON_VALUE& object,
		const std::initializer_list<std::string_view> names)
	{
		if (!object.Is_Object() || object.Get_Object().size() != names.size())
			return false;
		return std::all_of(names.begin(), names.end(),
			[&object](const std::string_view name)
			{
				return nullptr != object.Find(name);
			});
	}

	bool Parse_U32(
		const Client::DATA_JSON_VALUE& value,
		const std::uint32_t maximum,
		std::uint32_t& out)
	{
		if (!value.Is_Number() || value.Was_FloatingPointToken())
			return false;
		const double number = value.Get_Number();
		if (!std::isfinite(number) || number < 0.0 ||
			number > static_cast<double>(maximum) || std::floor(number) != number)
		{
			return false;
		}
		out = static_cast<std::uint32_t>(number);
		return true;
	}

	bool Is_StableId(const std::string_view value)
	{
		return !value.empty() && value.size() <= 128u && value != "." &&
			value != ".." &&
			std::all_of(value.begin(), value.end(), [](const unsigned char c)
			{
				return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
					(c >= '0' && c <= '9') || c == '_' || c == '-' || c == '.';
			});
	}

	bool Apply_PatternInventory(
		const Client::DATA_JSON_VALUE& root,
		std::vector<Client::CKoukuSaydonBossTool::PRODUCT_PATTERN>& patterns,
		std::vector<Client::CKoukuSaydonBossTool::PRODUCT_FOLDER>& folders,
		std::vector<Client::CKoukuSaydonBossTool::PRODUCT_BUNDLE>& bundles,
		std::string& status)
	{
		using namespace Client;
		const auto* inventory = root.Find("patternInventory");
		if (!inventory) return true; // Legacy published Products contain only executable rows.
		const auto fail = [&](const std::string& reason) {
			status = "Invalid Boss Patterns inventory: " + reason + "; previous inventory retained.";
			return false;
		};
		const auto* patternRows = Required(*inventory, "patterns", DATA_JSON_TYPE::ARRAY);
		const auto* folderRows = Required(*inventory, "folders", DATA_JSON_TYPE::ARRAY);
		const auto* bundleRows = Required(*inventory, "bundles", DATA_JSON_TYPE::ARRAY);
		if (!Has_ExactProperties(*inventory, { "folders", "patterns", "bundles" }) ||
			!patternRows || !folderRows || !bundleRows || patternRows->Get_Array().size() > 4096u ||
			folderRows->Get_Array().size() > 4096u || bundleRows->Get_Array().size() > 4096u)
			return fail("invalid or oversized hierarchy");

		std::vector<CKoukuSaydonBossTool::PRODUCT_FOLDER> fullFolders;
		std::vector<CKoukuSaydonBossTool::PRODUCT_PATTERN> fullPatterns;
		std::vector<CKoukuSaydonBossTool::PRODUCT_BUNDLE> fullBundles;
		std::unordered_set<std::string> hierarchyIds, patternIds;
		for (const auto& row : folderRows->Get_Array())
		{
			const auto* id = Required(row, "folderId", DATA_JSON_TYPE::STRING);
			const auto* gate = Required(row, "gateId", DATA_JSON_TYPE::STRING);
			const auto* name = Required(row, "displayName", DATA_JSON_TYPE::STRING);
			if (!Has_ExactProperties(row, { "folderId", "gateId", "displayName" }) ||
				!id || !gate || !name || !Is_StableId(id->Get_String()) || name->Get_String().empty() ||
				!hierarchyIds.insert(id->Get_String()).second ||
				!CKoukuSaydonCompositionDocument::Is_KnownGate(gate->Get_String()))
				return fail("invalid parent identity");
			fullFolders.push_back({ id->Get_String(), gate->Get_String(), name->Get_String() });
		}
		const auto hasParent = [&](const std::string& id, const std::string& gate) {
			return std::any_of(fullFolders.begin(), fullFolders.end(), [&](const auto& folder) {
				return folder.strFolderId == id && folder.strGateId == gate;
			});
		};
		for (const auto& folder : folders)
		{
			if (!std::any_of(fullFolders.begin(), fullFolders.end(), [&](const auto& row) {
				return row.strFolderId == folder.strFolderId && row.strGateId == folder.strGateId &&
					row.strDisplayName == folder.strDisplayName;
			})) return fail("executable parent metadata differs from the tree");
		}
		std::size_t readyPatterns = 0u, readyBundles = 0u;
		for (const auto& row : patternRows->Get_Array())
		{
			const auto* id = Required(row, "patternId", DATA_JSON_TYPE::STRING);
			const auto* name = Required(row, "displayName", DATA_JSON_TYPE::STRING);
			const auto* category = Required(row, "category", DATA_JSON_TYPE::STRING);
			const auto* gate = Required(row, "gateId", DATA_JSON_TYPE::STRING);
			const auto* actor = Required(row, "actorProfileId", DATA_JSON_TYPE::STRING);
			const auto* target = Required(row, "targetBossPlacementId", DATA_JSON_TYPE::STRING);
			const auto* reason = Required(row, "unavailableReason", DATA_JSON_TYPE::STRING);
			const auto* folder = row.Find("folderId");
			const bool exact = folder ? Has_ExactProperties(row,
				{ "patternId", "displayName", "category", "gateId", "actorProfileId", "targetBossPlacementId", "folderId", "unavailableReason" }) :
				Has_ExactProperties(row,
				{ "patternId", "displayName", "category", "gateId", "actorProfileId", "targetBossPlacementId", "unavailableReason" });
			if (!exact || !id || !name || !category || !gate || !actor || !target || !reason ||
				!Is_StableId(id->Get_String()) || !patternIds.insert(id->Get_String()).second ||
				name->Get_String().empty() || (category->Get_String() != "NORMAL" && category->Get_String() != "MECHANIC") ||
				!CKoukuSaydonCompositionDocument::Is_KnownGate(gate->Get_String()) ||
				!Is_StableId(actor->Get_String()) || !Is_StableId(target->Get_String()) ||
				(folder && (!folder->Is_String() || !hasParent(folder->Get_String(), gate->Get_String()))))
				return fail("invalid pattern identity or parent");
			CKoukuSaydonBossTool::PRODUCT_PATTERN pattern;
			pattern.strPatternId = id->Get_String();
			pattern.strDisplayName = name->Get_String();
			pattern.strCategory = category->Get_String();
			pattern.strGateId = gate->Get_String();
			pattern.strActorProfileId = actor->Get_String();
			pattern.strTargetBossPlacementId = target->Get_String();
			pattern.strFolderId = folder ? folder->Get_String() : "";
			pattern.strLoadError = reason->Get_String();
			const auto executable = std::find_if(patterns.begin(), patterns.end(), [&](const auto& item) {
				return item.strPatternId == pattern.strPatternId;
			});
			if (pattern.strLoadError.empty())
			{
				if (executable == patterns.end() || !executable->strLoadError.empty() || executable->Stages.empty() ||
					executable->strDisplayName != pattern.strDisplayName || executable->strCategory != pattern.strCategory ||
					executable->strGateId != pattern.strGateId || executable->strFolderId != pattern.strFolderId ||
					executable->strActorProfileId != pattern.strActorProfileId ||
					executable->strTargetBossPlacementId != pattern.strTargetBossPlacementId)
					return fail("ready pattern metadata differs from executable pattern " + pattern.strPatternId);
				pattern.Stages = executable->Stages;
				++readyPatterns;
			}
			else if (executable != patterns.end()) return fail("unavailable pattern is executable: " + pattern.strPatternId);
			fullPatterns.push_back(std::move(pattern));
		}
		if (readyPatterns != patterns.size()) return fail("executable patterns are missing from the tree");
		for (const auto& row : bundleRows->Get_Array())
		{
			const auto* id = Required(row, "bundleId", DATA_JSON_TYPE::STRING);
			const auto* folder = Required(row, "folderId", DATA_JSON_TYPE::STRING);
			const auto* gate = Required(row, "gateId", DATA_JSON_TYPE::STRING);
			const auto* name = Required(row, "displayName", DATA_JSON_TYPE::STRING);
			const auto* members = Required(row, "members", DATA_JSON_TYPE::ARRAY);
			const auto* reason = Required(row, "unavailableReason", DATA_JSON_TYPE::STRING);
			if (!Has_ExactProperties(row, { "bundleId", "folderId", "gateId", "displayName", "members", "unavailableReason" }) ||
				!id || !folder || !gate || !name || !members || !reason || !Is_StableId(id->Get_String()) ||
				!hierarchyIds.insert(id->Get_String()).second || name->Get_String().empty() ||
				members->Get_Array().size() > 8u || !hasParent(folder->Get_String(), gate->Get_String()))
				return fail("invalid bundle identity or parent");
			CKoukuSaydonBossTool::PRODUCT_BUNDLE bundle;
			bundle.strBundleId = id->Get_String();
			bundle.strFolderId = folder->Get_String();
			bundle.strGateId = gate->Get_String();
			bundle.strDisplayName = name->Get_String();
			bundle.strLoadError = reason->Get_String();
			std::unordered_set<std::string> memberIds, memberPatterns;
			for (const auto& member : members->Get_Array())
			{
				const auto* memberId = Required(member, "memberId", DATA_JSON_TYPE::STRING);
				const auto* patternId = Required(member, "patternId", DATA_JSON_TYPE::STRING);
				const auto* offset = Required(member, "startOffsetMs", DATA_JSON_TYPE::NUMBER);
				CKoukuSaydonBossTool::PRODUCT_BUNDLE_MEMBER item;
				if (!Has_ExactProperties(member, { "memberId", "patternId", "startOffsetMs" }) ||
					!memberId || !patternId || !offset || !Is_StableId(memberId->Get_String()) ||
					!memberIds.insert(memberId->Get_String()).second || !memberPatterns.insert(patternId->Get_String()).second ||
					!Parse_U32(*offset, 600000u, item.iStartOffsetMs))
					return fail("invalid bundle member: " + bundle.strBundleId);
				const auto pattern = std::find_if(fullPatterns.begin(), fullPatterns.end(), [&](const auto& item) {
					return item.strPatternId == patternId->Get_String();
				});
				if (pattern == fullPatterns.end() || pattern->strGateId != bundle.strGateId)
					return fail("bundle child is missing or belongs to another Gate: " + bundle.strBundleId);
				item.strMemberId = memberId->Get_String();
				item.strPatternId = patternId->Get_String();
				item.strTargetBossPlacementId = pattern->strTargetBossPlacementId;
				item.strActorProfileId = pattern->strActorProfileId;
				bundle.Members.push_back(std::move(item));
			}
			const auto executable = std::find_if(bundles.begin(), bundles.end(), [&](const auto& item) {
				return item.strBundleId == bundle.strBundleId;
			});
			if (bundle.strLoadError.empty())
			{
				if (executable == bundles.end() || !executable->strLoadError.empty() || executable->Members.empty() ||
					executable->strDisplayName != bundle.strDisplayName || executable->strFolderId != bundle.strFolderId ||
					executable->strGateId != bundle.strGateId || executable->Members.size() != bundle.Members.size())
					return fail("ready bundle metadata differs from executable bundle " + bundle.strBundleId);
				for (std::size_t i = 0u; i < bundle.Members.size(); ++i)
				{
					const auto& source = bundle.Members[i];
					const auto& targetMember = executable->Members[i];
					if (source.strMemberId != targetMember.strMemberId || source.strPatternId != targetMember.strPatternId ||
						source.iStartOffsetMs != targetMember.iStartOffsetMs || source.strActorProfileId != targetMember.strActorProfileId ||
						source.strTargetBossPlacementId != targetMember.strTargetBossPlacementId)
						return fail("ready bundle membership differs from executable bundle " + bundle.strBundleId);
				}
				bundle.iDurationMs = executable->iDurationMs;
				++readyBundles;
			}
			else if (executable != bundles.end()) return fail("unavailable bundle is executable: " + bundle.strBundleId);
			fullBundles.push_back(std::move(bundle));
		}
		if (readyBundles != bundles.size()) return fail("executable bundles are missing from the tree");
		folders = std::move(fullFolders);
		patterns = std::move(fullPatterns);
		bundles = std::move(fullBundles);
		return true;
	}

	bool Load_ProductIndex(
		std::vector<Client::CKoukuSaydonBossTool::PRODUCT_PATTERN>& outPatterns,
		std::vector<std::string>& outPlayAll,
		std::vector<Client::CKoukuSaydonBossTool::PRODUCT_FOLDER>& outFolders,
		std::vector<Client::CKoukuSaydonBossTool::PRODUCT_BUNDLE>& outBundles,
		std::uint32_t& outSourceRevision,
		std::string& outStatus)
	{
		using namespace Client;
		const auto path = CProjectDataRoot::Resolve(L"Encounters/KoukuSaydon/KoukuSaydonEncounter.json");
		std::error_code error;
		const auto size = std::filesystem::file_size(path, error);
		if (error || size == 0u || size > MAX_PRODUCT_BYTES)
		{ outStatus = "KoukuSaydon Product index is missing or oversized; previous list retained."; return false; }
		std::ifstream input(path, std::ios::binary);
		const std::string text{std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
		DATA_JSON_VALUE root;
		if (!input || !CDataJson::Parse(text, root, outStatus)) return false;
		const auto* schema = Required(root, "schema", DATA_JSON_TYPE::STRING);
		const auto* version = Required(root, "formatVersion", DATA_JSON_TYPE::NUMBER);
		const auto* encounter = Required(root, "encounterId", DATA_JSON_TYPE::STRING);
		const auto* boss = Required(root, "bossArchetypeId", DATA_JSON_TYPE::STRING);
		const auto* revision = Required(root, "sourceRevision", DATA_JSON_TYPE::NUMBER);
		const auto* patterns = Required(root, "patterns", DATA_JSON_TYPE::ARRAY);
		std::uint32_t parsedVersion = 0u, parsedRevision = 0u;
		if (!schema || schema->Get_String() != PRODUCT_SCHEMA || !version ||
			!Parse_U32(*version, PRODUCT_VERSION, parsedVersion) || parsedVersion != PRODUCT_VERSION ||
			!encounter || encounter->Get_String() != ENCOUNTER_ID || !boss || boss->Get_String() != BOSS_ARCHETYPE_ID ||
			!revision || !Parse_U32(*revision, UINT32_MAX, parsedRevision) || parsedRevision == 0u ||
			!patterns || patterns->Get_Array().size() > 64u)
		{ outStatus = "KoukuSaydon Product header is invalid; previous list retained."; return false; }
		std::vector<CKoukuSaydonBossTool::PRODUCT_PATTERN> staged;
		std::unordered_set<std::string> ids;
		std::size_t errors = 0u;
		for (const auto& value : patterns->Get_Array())
		{
			CKoukuSaydonBossTool::PRODUCT_PATTERN pattern;
			const auto* id = Required(value, "patternId", DATA_JSON_TYPE::STRING);
			const auto* name = Required(value, "displayName", DATA_JSON_TYPE::STRING);
			const auto* category = Required(value, "category", DATA_JSON_TYPE::STRING);
			const auto* stages = Required(value, "stages", DATA_JSON_TYPE::ARRAY);
			pattern.strPatternId = id ? id->Get_String() : "";
			pattern.strDisplayName = name ? name->Get_String() : "Invalid pattern";
			pattern.strCategory = category ? category->Get_String() : "";
			if (const auto* folder = value.Find("folderId"))
			{
				if (!folder->Is_String() || !Is_StableId(folder->Get_String()))
					pattern.strLoadError = "Pattern parent identity is invalid.";
				else pattern.strFolderId = folder->Get_String();
			}
			if (const auto* gate = value.Find("gateId"))
			{
				const auto* target = Required(value, "targetBossPlacementId", DATA_JSON_TYPE::STRING);
				const auto* actor = Required(value, "actorProfileId", DATA_JSON_TYPE::STRING);
				if (!gate->Is_String() || !CKoukuSaydonCompositionDocument::Is_KnownGate(gate->Get_String()) || !target || !actor ||
					CKoukuSaydonCompositionDocument::Resolve_DefaultPlacementId(gate->Get_String(), actor->Get_String()) != target->Get_String() ||
					CKoukuSaydonCompositionDocument::Resolve_ActorProfileForPlacement(target->Get_String()) != actor->Get_String())
					pattern.strLoadError = "Pattern Gate or target actor binding is invalid.";
				else { pattern.strGateId=gate->Get_String(); pattern.strTargetBossPlacementId=target->Get_String(); pattern.strActorProfileId=actor->Get_String(); }
			}
			if (!id || !Is_StableId(pattern.strPatternId) || !ids.insert(pattern.strPatternId).second ||
				!stages || stages->Get_Array().empty() || stages->Get_Array().size() > 64u)
				pattern.strLoadError = "Pattern identity or stages are invalid.";
			if (stages && pattern.strLoadError.empty())
			{
				std::unordered_set<std::string> stageIds;
				for (const auto& stageValue : stages->Get_Array())
				{
					const auto* stageId = Required(stageValue, "stageId", DATA_JSON_TYPE::STRING);
					const auto* actionId = Required(stageValue, "actionId", DATA_JSON_TYPE::STRING);
					const auto* kind = Required(stageValue, "stageKind", DATA_JSON_TYPE::STRING);
					const auto* duration = Required(stageValue, "durationMs", DATA_JSON_TYPE::NUMBER);
					CKoukuSaydonBossTool::PRODUCT_STAGE stage;
					if (!stageId || !Is_StableId(stageId->Get_String()) || !stageIds.insert(stageId->Get_String()).second ||
						!actionId || !Is_StableId(actionId->Get_String()) || !kind || !duration ||
						!Parse_U32(*duration, 600000u, stage.iDurationMs) || stage.iDurationMs == 0u)
					{ pattern.strLoadError = "A stage has invalid ID or timing."; break; }
					stage.strStageId = stageId->Get_String();
					stage.strActionId = actionId->Get_String();
					stage.strStageKind = kind->Get_String();
					pattern.Stages.push_back(std::move(stage));
				}
			}
			if (!pattern.strLoadError.empty()) ++errors;
			staged.push_back(std::move(pattern));
		}
		std::vector<std::string> playOrder;
		const auto* order = Required(root, "playAllPatternIds", DATA_JSON_TYPE::ARRAY);
		bool validOrder = errors == 0u && order && order->Get_Array().size() == staged.size();
		if (validOrder)
		{
			for (std::size_t i = 0u; i < staged.size(); ++i)
			{
				const auto& id = order->Get_Array()[i];
				if (!id.Is_String() || id.Get_String() != staged[i].strPatternId) { validOrder = false; break; }
				playOrder.push_back(id.Get_String());
			}
		}
		if (!validOrder) playOrder.clear();
		std::vector<CKoukuSaydonBossTool::PRODUCT_FOLDER> folders;
		std::vector<CKoukuSaydonBossTool::PRODUCT_BUNDLE> bundles;
		const auto* folderRows = root.Find("folders"); const auto* bundleRows = root.Find("bundles");
		if ((folderRows && !folderRows->Is_Array()) || (bundleRows && !bundleRows->Is_Array()))
		{ outStatus="Invalid Product hierarchy; previous inventory retained."; return false; }
		std::unordered_set<std::string> hierarchyIds;
		if (folderRows) for (const auto& row : folderRows->Get_Array())
		{
			const auto* id=Required(row,"folderId",DATA_JSON_TYPE::STRING); const auto* gate=Required(row,"gateId",DATA_JSON_TYPE::STRING); const auto* name=Required(row,"displayName",DATA_JSON_TYPE::STRING);
			if (!id || !gate || !name || !Is_StableId(id->Get_String()) || !hierarchyIds.insert(id->Get_String()).second || !CKoukuSaydonCompositionDocument::Is_KnownGate(gate->Get_String()))
			{ outStatus="Invalid Product parent identity; previous inventory retained."; return false; }
			folders.push_back({id->Get_String(),gate->Get_String(),name->Get_String()});
		}
		for (auto& pattern : staged)
		{
			if (pattern.strFolderId.empty() || std::any_of(folders.begin(), folders.end(),
				[&](const auto& folder) { return folder.strFolderId == pattern.strFolderId && folder.strGateId == pattern.strGateId; })) continue;
			if (pattern.strLoadError.empty()) ++errors;
			pattern.strLoadError = "Pattern parent is missing or belongs to another Gate.";
			validOrder = false;
			playOrder.clear();
		}
		if (bundleRows) for (const auto& row : bundleRows->Get_Array())
		{
			CKoukuSaydonBossTool::PRODUCT_BUNDLE bundle;
			const auto* id=Required(row,"bundleId",DATA_JSON_TYPE::STRING); const auto* folder=Required(row,"folderId",DATA_JSON_TYPE::STRING);
			const auto* gate=Required(row,"gateId",DATA_JSON_TYPE::STRING); const auto* name=Required(row,"displayName",DATA_JSON_TYPE::STRING);
			const auto* duration=Required(row,"durationMs",DATA_JSON_TYPE::NUMBER); const auto* members=Required(row,"members",DATA_JSON_TYPE::ARRAY);
			if (!id || !folder || !gate || !name || !duration || !members || !Is_StableId(id->Get_String()) || !hierarchyIds.insert(id->Get_String()).second)
			{ outStatus="Invalid Product bundle identity; previous inventory retained."; return false; }
			bundle.strBundleId=id->Get_String(); bundle.strFolderId=folder->Get_String(); bundle.strGateId=gate->Get_String(); bundle.strDisplayName=name->Get_String();
			if (!Parse_U32(*duration,600000,bundle.iDurationMs) || !bundle.iDurationMs || members->Get_Array().empty() || members->Get_Array().size()>4 ||
				!std::any_of(folders.begin(),folders.end(),[&](const auto& f){ return f.strFolderId==bundle.strFolderId && f.strGateId==bundle.strGateId; })) bundle.strLoadError="Bundle parent, Gate or lifetime is invalid.";
			std::unordered_set<std::string> actors, memberIds;
			for (const auto& member : members->Get_Array())
			{
				CKoukuSaydonBossTool::PRODUCT_BUNDLE_MEMBER item;
				const auto* memberId=Required(member,"memberId",DATA_JSON_TYPE::STRING); const auto* patternId=Required(member,"patternId",DATA_JSON_TYPE::STRING);
				const auto* target=Required(member,"targetBossPlacementId",DATA_JSON_TYPE::STRING); const auto* actor=Required(member,"actorProfileId",DATA_JSON_TYPE::STRING); const auto* offset=Required(member,"startOffsetMs",DATA_JSON_TYPE::NUMBER);
				if (!memberId || !patternId || !target || !actor || !offset || !Parse_U32(*offset,600000,item.iStartOffsetMs)) { bundle.strLoadError="Invalid bundle member."; continue; }
				item.strMemberId=memberId->Get_String(); item.strPatternId=patternId->Get_String(); item.strTargetBossPlacementId=target->Get_String(); item.strActorProfileId=actor->Get_String();
				const auto p=std::find_if(staged.begin(),staged.end(),[&](const auto& x){return x.strPatternId==item.strPatternId;});
				if (!Is_StableId(item.strMemberId) || !memberIds.insert(item.strMemberId).second || !actors.insert(item.strTargetBossPlacementId).second || p==staged.end() ||
					(p!=staged.end() && (!p->strLoadError.empty() || p->strGateId!=bundle.strGateId || p->strTargetBossPlacementId!=item.strTargetBossPlacementId || p->strActorProfileId!=item.strActorProfileId)))
					bundle.strLoadError="Bundle child, Gate or actor reference is invalid.";
				bundle.Members.push_back(std::move(item));
			}
			bundles.push_back(std::move(bundle));
		}
		if (!Apply_PatternInventory(root, staged, folders, bundles, outStatus)) return false;
		errors = std::count_if(staged.begin(), staged.end(), [](const auto& pattern) { return !pattern.strLoadError.empty(); });
		const auto unavailableBundles = std::count_if(bundles.begin(), bundles.end(), [](const auto& bundle) { return !bundle.strLoadError.empty(); });
		outFolders=std::move(folders); outBundles=std::move(bundles);
		outPatterns = std::move(staged);
		outPlayAll = std::move(playOrder);
		outSourceRevision = parsedRevision;
		outStatus = "Loaded Boss Patterns tree: " + std::to_string(outPatterns.size()) + " patterns / " +
			std::to_string(outFolders.size()) + " parents / " + std::to_string(outBundles.size()) + " bundles; " +
			std::to_string(errors) + " patterns and " + std::to_string(unavailableBundles) +
			" bundles unavailable." + (validOrder ? "" : " Play All unavailable.");
		return true;
	}

	std::uint32_t Pattern_DurationMs(
		const Client::CKoukuSaydonBossTool::PRODUCT_PATTERN& pattern)
	{
		std::uint64_t total = 0u;
		for (const Client::CKoukuSaydonBossTool::PRODUCT_STAGE& stage :
			pattern.Stages)
			total += stage.iDurationMs;
		return total > (std::numeric_limits<std::uint32_t>::max)() ?
			(std::numeric_limits<std::uint32_t>::max)() :
			static_cast<std::uint32_t>(total);
	}
}

void Client::CKoukuSaydonBossTool::Open()
{
	m_bOpen = true;
	if (!m_bLoadAttempted)
	{
		std::string status;
		(void)Reload(status);
	}
}

bool Client::CKoukuSaydonBossTool::Reload(std::string& outStatus)
{
	m_bLoadAttempted = true;
	std::vector<PRODUCT_PATTERN> stagedPatterns;
	std::vector<std::string> stagedPlayAll;
	std::vector<PRODUCT_FOLDER> stagedFolders;
	std::vector<PRODUCT_BUNDLE> stagedBundles;
	std::uint32_t stagedSourceRevision = 0u;
	if (!Load_ProductIndex(
			stagedPatterns, stagedPlayAll, stagedFolders, stagedBundles, stagedSourceRevision, outStatus))
	{
		m_strStatus = outStatus;
		return false;
	}
	m_ProductFolders=std::move(stagedFolders); m_ProductBundles=std::move(stagedBundles);
	m_ProductPatterns = std::move(stagedPatterns);
	m_PlayAllPatternIds = std::move(stagedPlayAll);
	m_iSourceRevision = stagedSourceRevision;
	m_bHasSavedComposition = true;
	Normalize_Selection();
	m_strStatus = outStatus;
	outStatus = m_strStatus;
	return true;
}

const Client::CKoukuSaydonBossTool::PRODUCT_PATTERN*
Client::CKoukuSaydonBossTool::Find_SelectedPattern() const
{
	if (m_iSelectedInventoryKind != 3) return nullptr;
	const auto found = std::find_if(
		m_ProductPatterns.begin(), m_ProductPatterns.end(),
		[this](const PRODUCT_PATTERN& pattern)
		{
			return pattern.strPatternId == m_strSelectedInventoryId;
		});
	return found == m_ProductPatterns.end() ? nullptr : &*found;
}

const Client::CKoukuSaydonBossTool::PRODUCT_BUNDLE*
Client::CKoukuSaydonBossTool::Find_SelectedBundle() const
{
	if (m_iSelectedInventoryKind != 2) return nullptr;
	const auto found = std::find_if(m_ProductBundles.begin(), m_ProductBundles.end(),
		[this](const auto& row) { return row.strBundleId == m_strSelectedInventoryId; });
	return found == m_ProductBundles.end() ? nullptr : &*found;
}

void Client::CKoukuSaydonBossTool::Normalize_Selection()
{
	if (Find_SelectedPattern() || Find_SelectedBundle() ||
		(m_iSelectedInventoryKind == 1 && std::any_of(m_ProductFolders.begin(), m_ProductFolders.end(),
			[this](const auto& row) { return row.strFolderId == m_strSelectedInventoryId; }))) return;
	m_iSelectedInventoryKind = 0;
	m_strSelectedInventoryId.clear();
}

bool Client::CKoukuSaydonBossTool::Play_Selected(std::string& outStatus)
{
	const std::string stableId = m_strSelectedInventoryId;
	return m_iSelectedInventoryKind == 2 ? Play_SavedBundleById(stableId, outStatus) :
		Play_SavedPatternById(stableId, outStatus);
}

bool Client::CKoukuSaydonBossTool::Play_LoadedPatternById(const std::string_view patternId, std::string& outStatus)
{
	const auto found = std::find_if(m_ProductPatterns.begin(), m_ProductPatterns.end(),
		[&](const auto& row) { return row.strPatternId == patternId; });
	const PRODUCT_PATTERN* const pattern = found == m_ProductPatterns.end() ? nullptr : &*found;
	if (nullptr == pattern || !pattern->strLoadError.empty())
	{
		outStatus = m_strStatus = "Pattern '" + std::string(patternId) +
			"' is not ready in published revision " + std::to_string(m_iSourceRevision) +
			". " + (pattern ? pattern->strLoadError : "Use Publish All Patterns in Composition to synchronize the tree.");
		return false;
	}
	m_iSelectedInventoryKind = 3;
	m_strSelectedInventoryId = pattern->strPatternId;
	const auto& revision =
		CNetworkManager::Get().Get_GameplayRevisionState().ServerActiveRevision;
	if (!pattern->strTargetBossPlacementId.empty())
		CKoukuSaydonPatternAuditionService::Get().Set_TargetBoss(pattern->strTargetBossPlacementId,
			CKoukuSaydonCompositionDocument::Resolve_BossArchetypeId(pattern->strTargetBossPlacementId));
	const bool played = CKoukuSaydonPatternAuditionService::Get().Play_Selected(
		pattern->strPatternId, revision, m_iSourceRevision, outStatus);
	m_strStatus = outStatus;
	return played;
}

bool Client::CKoukuSaydonBossTool::Play_All(std::string& outStatus)
{
	if (!Reload(outStatus)) return false;
	if (!m_bHasSavedComposition ||
		m_PlayAllPatternIds.empty())
	{
		outStatus = "The published composition has no executable Play All order.";
		return false;
	}
	const auto& revision =
		CNetworkManager::Get().Get_GameplayRevisionState().ServerActiveRevision;
	return CKoukuSaydonPatternAuditionService::Get().Play_All(
		revision, m_iSourceRevision, outStatus);
}

bool Client::CKoukuSaydonBossTool::Play_PatternById(
	const std::string_view patternId,
	const std::uint32_t expectedSourceRevision,
	std::string& outStatus)
{
	if (!Reload(outStatus))
		return false;
	if (0u == expectedSourceRevision ||
		m_iSourceRevision != expectedSourceRevision)
	{
		outStatus =
			"Saved authoring and published KoukuSaydon revisions differ; use Publish All Patterns before Server Play.";
		m_strStatus = outStatus;
		return false;
	}
	return Play_LoadedPatternById(patternId, outStatus);
}

bool Client::CKoukuSaydonBossTool::Play_SavedPatternById(const std::string_view patternId, std::string& status)
{
	const std::string stableId(patternId);
	return Reload(status) && Play_LoadedPatternById(stableId, status);
}

bool Client::CKoukuSaydonBossTool::Play_SavedBundleById(const std::string_view bundleId, std::string& status)
{
	const std::string stableId(bundleId);
	return Reload(status) && Play_LoadedBundleById(stableId, status);
}

bool Client::CKoukuSaydonBossTool::Play_BundleById(const std::string_view bundleId,
	const std::uint32_t expectedSourceRevision, std::string& status)
{
	if (!Reload(status)) return false;
	if (!expectedSourceRevision || expectedSourceRevision!=m_iSourceRevision)
	{ status=m_strStatus="Saved bundle and published revisions differ. Use Publish All Patterns before Complete Play."; return false; }
	return Play_LoadedBundleById(bundleId, status);
}

bool Client::CKoukuSaydonBossTool::Play_LoadedBundleById(const std::string_view bundleId, std::string& status)
{
	const auto found=std::find_if(m_ProductBundles.begin(),m_ProductBundles.end(),[&](const auto& b){return b.strBundleId==bundleId;});
	if (found==m_ProductBundles.end() || !found->strLoadError.empty() || found->Members.empty())
	{ status=m_strStatus="Bundle '" + std::string(bundleId) + "' is not ready in published revision " +
		std::to_string(m_iSourceRevision) + ". " + (found == m_ProductBundles.end() ?
		"Use Publish All Patterns in Composition to synchronize the tree." : found->strLoadError); return false; }
	m_iSelectedInventoryKind = 2;
	m_strSelectedInventoryId = found->strBundleId;
	const auto& target=found->Members.front().strTargetBossPlacementId;
	auto& service=CKoukuSaydonPatternAuditionService::Get();
	service.Set_TargetBoss(target,CKoukuSaydonCompositionDocument::Resolve_BossArchetypeId(target));
	const auto& revision=CNetworkManager::Get().Get_GameplayRevisionState().ServerActiveRevision;
	const bool played=service.Play_Bundle(found->strBundleId,found->strGateId,revision,m_iSourceRevision,status);
	m_strStatus=status; return played;
}

bool Client::CKoukuSaydonBossTool::Render_PatternTree(
	const std::string_view gateId, int& selectionKind, std::string& selectedId) const
{
	const std::string gate(gateId);
	const auto& patterns = m_ProductPatterns;
	const auto& folders = m_ProductFolders;
	const auto& bundles = m_ProductBundles;
	const auto& audition = CKoukuSaydonPatternAuditionService::Get().Get_Snapshot();
	const auto findPattern = [&](const std::string& id) -> const CKoukuSaydonBossTool::PRODUCT_PATTERN* {
		auto it=std::find_if(patterns.begin(),patterns.end(),[&](const auto& p){return p.strPatternId==id;}); return it==patterns.end()?nullptr:&*it; };
	const bool selectionExists = selectionKind == 1 ?
		std::any_of(folders.begin(), folders.end(), [&](const auto& row) { return row.strFolderId == selectedId && row.strGateId == gate; }) :
		selectionKind == 2 ?
		std::any_of(bundles.begin(), bundles.end(), [&](const auto& row) { return row.strBundleId == selectedId && row.strGateId == gate; }) :
		selectionKind == 3 &&
		std::any_of(patterns.begin(), patterns.end(), [&](const auto& row) { return row.strPatternId == selectedId && row.strGateId == gate; });
	if (!selectionExists) { selectionKind = 0; selectedId.clear(); }
	const auto selectPattern = [&](const CKoukuSaydonBossTool::PRODUCT_PATTERN& pattern, const std::string& uiId) {
		if (ImGui::Selectable((pattern.strDisplayName+(pattern.strLoadError.empty()?"":" [Unavailable]")+"##"+uiId).c_str(),selectionKind==3 && selectedId==pattern.strPatternId))
		{ selectionKind=3; selectedId=pattern.strPatternId; }
		if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s\n%s\n%s",pattern.strPatternId.c_str(),pattern.strTargetBossPlacementId.c_str(),pattern.strLoadError.c_str());
		if (audition.Is_Live(pattern.strPatternId, m_iSourceRevision))
		{ ImGui::SameLine(); ImGui::TextColored(LIVE_COLOR, "[Live]"); }
	};
	bool gateHasProduct=false;
	{
		for (const auto& folder : folders)
		{
			if (folder.strGateId!=gate) continue;
			gateHasProduct=true;
			const bool open=ImGui::TreeNodeEx((folder.strDisplayName+" [Parent]##"+folder.strFolderId).c_str(),ImGuiTreeNodeFlags_DefaultOpen|ImGuiTreeNodeFlags_OpenOnArrow|
				(selectionKind==1 && selectedId==folder.strFolderId?ImGuiTreeNodeFlags_Selected:0));
			if (ImGui::IsItemClicked()&&!ImGui::IsItemToggledOpen()) { selectionKind=1; selectedId=folder.strFolderId; }
			if (open)
			{
				for (const auto& bundle : bundles)
				{
					if (bundle.strFolderId!=folder.strFolderId || bundle.strGateId!=gate) continue;
					const bool bundleOpen=ImGui::TreeNodeEx((bundle.strDisplayName+" ["+std::to_string(bundle.Members.size())+" actors]"+(bundle.strLoadError.empty()?"":" [Unavailable]")+"##"+bundle.strBundleId).c_str(),ImGuiTreeNodeFlags_DefaultOpen|ImGuiTreeNodeFlags_OpenOnArrow|
						(selectionKind==2 && selectedId==bundle.strBundleId?ImGuiTreeNodeFlags_Selected:0));
					if (ImGui::IsItemClicked()&&!ImGui::IsItemToggledOpen()) { selectionKind=2; selectedId=bundle.strBundleId; }
					if (ImGui::IsItemHovered() && !bundle.strLoadError.empty()) ImGui::SetTooltip("%s",bundle.strLoadError.c_str());
					if (bundleOpen)
					{
						for (const auto& member : bundle.Members)
							if (const auto* pattern=findPattern(member.strPatternId)) selectPattern(*pattern,member.strMemberId);
							else ImGui::TextDisabled("Missing: %s",member.strPatternId.c_str());
						ImGui::TreePop();
					}
				}
				for (const auto& pattern : patterns)
					if (pattern.strGateId==gate && pattern.strFolderId==folder.strFolderId) selectPattern(pattern,pattern.strPatternId);
				ImGui::TreePop();
			}
		}
		for (const auto& pattern : patterns)
		{
			if (pattern.strGateId!=gate) continue;
			gateHasProduct=true;
			const bool linked=std::any_of(bundles.begin(),bundles.end(),[&](const auto& b){return std::any_of(b.Members.begin(),b.Members.end(),[&](const auto& m){return m.strPatternId==pattern.strPatternId;});});
			const bool parented=std::any_of(folders.begin(),folders.end(),[&](const auto& folder){return folder.strFolderId==pattern.strFolderId && folder.strGateId==gate;});
			if (!linked && !parented) selectPattern(pattern,pattern.strPatternId);
		}
		if (!gateHasProduct) ImGui::TextDisabled("No published tree for this Gate. Use Publish All Patterns in Composition.");
	}
	return gateHasProduct;
}

void Client::CKoukuSaydonBossTool::Render()
{
	Open();
	ImGui::SetNextWindowSize(ImVec2(760.f, 520.f), ImGuiCond_FirstUseEver);
	if (!ImGui::Begin(
		"KoukuSaydon Boss Tool###KoukuSaydonBossTool", &m_bOpen))
	{
		ImGui::End();
		return;
	}

	const KOUKU_SAYDON_PATTERN_AUDITION_SNAPSHOT& audition =
		CKoukuSaydonPatternAuditionService::Get().Get_Snapshot();
	const bool exactLiveProduct = audition.Is_Live(
		audition.strLivePatternId, m_iSourceRevision);
	if (ImGui::Button("Reload Published Patterns"))
	{
		std::string status;
		(void)Reload(status);
	}
	ImGui::SameLine();
	const auto* selectedPattern = Find_SelectedPattern();
	const auto* selectedBundle = Find_SelectedBundle();
	const bool ready = selectedBundle ? selectedBundle->strLoadError.empty() && !selectedBundle->Members.empty() :
		selectedPattern && selectedPattern->strLoadError.empty();
	ImGui::BeginDisabled(!m_bHasSavedComposition || !ready || audition.Is_InFlight());
	if (ImGui::Button(selectedBundle ? "Play Bundle" : "Play Isolated"))
		(void)Play_Selected(m_strStatus);
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::BeginDisabled(
		!m_bHasSavedComposition ||
		m_PlayAllPatternIds.empty() || audition.Is_InFlight());
	if (ImGui::Button("Start Full Pattern"))
		(void)Play_All(m_strStatus);
	ImGui::EndDisabled();

	ImGui::SameLine();
	ImGui::TextDisabled(
		"Server: %s",
		Describe_KoukuSaydonPatternAuditionState(audition.eState));
	if (!audition.strStatus.empty())
	{
		if (exactLiveProduct)
			ImGui::TextColored(LIVE_COLOR, "%s", audition.strStatus.c_str());
		else if (KOUKU_SAYDON_PATTERN_AUDITION_STATE::ACTIVE == audition.eState)
			ImGui::TextWrapped(
				"This replay keeps its original Product revision. Stop Server Play, then start Complete Play to use the newly published revision.");
		else
			ImGui::TextWrapped("%s", audition.strStatus.c_str());
	}
	if (!m_strStatus.empty())
		ImGui::TextWrapped("%s", m_strStatus.c_str());
	if (m_bHasSavedComposition)
		ImGui::TextDisabled("Product source revision: %u", m_iSourceRevision);

	if (!m_bHasSavedComposition)
	{
		ImGui::Separator();
		ImGui::TextWrapped(
			"No admitted saved composition is available. Repair it in the "
			"KoukuSaydon Action Workbench and reload this panel.");
		ImGui::End();
		return;
	}

	ImGui::Separator();
	if (ImGui::BeginTable(
		"##KoukuBossSplit", 2,
		ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerV,
		ImVec2(0.f, -1.f)))
	{
		ImGui::TableSetupColumn("Boss Patterns", ImGuiTableColumnFlags_WidthFixed,
			300.f);
		ImGui::TableSetupColumn("Server Playback", ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		static const char* gateLabels[] = { "1\xEA\xB4\x80\xEB\xAC\xB8", "2\xEA\xB4\x80\xEB\xAC\xB8", "3\xEA\xB4\x80\xEB\xAC\xB8", "\xEB\xB9\x99\xEA\xB3\xA0" };
		static const char* gates[] = { "GATE1", "GATE2", "GATE3", "BINGO" };
		m_iSelectedGate = (std::clamp)(m_iSelectedGate, 0, 3);
		if (ImGui::Combo("Gate##KoukuBossTree", &m_iSelectedGate, gateLabels, 4))
		{ m_iSelectedInventoryKind = 0; m_strSelectedInventoryId.clear(); }
		(void)Render_PatternTree(gates[m_iSelectedGate], m_iSelectedInventoryKind, m_strSelectedInventoryId);

		ImGui::TableSetColumnIndex(1);
		const PRODUCT_PATTERN* const selected =
			Find_SelectedPattern();
		const PRODUCT_BUNDLE* const bundle = Find_SelectedBundle();
		if (bundle)
		{
			ImGui::Text("%s", bundle->strDisplayName.c_str());
			if (!bundle->strLoadError.empty()) ImGui::TextWrapped("%s", bundle->strLoadError.c_str());
			ImGui::TextDisabled("%s", bundle->strBundleId.c_str());
			for (const auto& member : bundle->Members)
				ImGui::BulletText("%s | %s | %u ms", member.strPatternId.c_str(),
					member.strTargetBossPlacementId.c_str(), member.iStartOffsetMs);
		}
		else if (nullptr == selected)
		{
			ImGui::TextDisabled("Select a bundle or child Pattern. Parent folders organize the tree.");
		}
		else
		{
			ImGui::Text("%s", selected->strDisplayName.c_str());
			if (!selected->strLoadError.empty()) ImGui::TextWrapped("%s", selected->strLoadError.c_str());
			ImGui::TextDisabled("%s", selected->strPatternId.c_str());
			ImGui::Text("%zu stages / %.3f sec",
				selected->Stages.size(),
				static_cast<double>(Pattern_DurationMs(*selected)) / 1000.0);
			if (audition.Is_Live(
				selected->strPatternId, m_iSourceRevision))
			{
				ImGui::TextColored(
					LIVE_COLOR, "[Live] sequence %u / stage %u",
					audition.iPatternSequence, audition.iStageIndex + 1u);
			}
			ImGui::SeparatorText("Stages");
			for (std::size_t index = 0u; index < selected->Stages.size(); ++index)
			{
				const PRODUCT_STAGE& stage =
					selected->Stages[index];
				const bool liveStage = audition.Is_Live(
					selected->strPatternId, m_iSourceRevision) &&
					audition.iStageIndex == index;
				if (liveStage)
					ImGui::TextColored(LIVE_COLOR, "[Live]");
				else
					ImGui::TextDisabled("%02zu", index + 1u);
				ImGui::SameLine();
				ImGui::Text("%s  %s  %u ms",
					stage.strStageId.c_str(), stage.strStageKind.c_str(),
					stage.iDurationMs);
			}
		}
		ImGui::EndTable();
	}
	ImGui::End();
}
