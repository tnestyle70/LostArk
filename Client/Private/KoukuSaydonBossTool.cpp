#include "imgui.h"

#include "KoukuSaydonBossTool.h"

#include "DataJson.h"
#include "KoukuSaydonPatternAuditionService.h"
#include "KoukuSaydonCompositionDocument.h"
#include "KoukuSaydonActionWorkbench.h"
#include "KoukuSaydonPresentationAssetService.h"
#include "Level_KakulSaydonArena.h"
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
	constexpr std::uintmax_t MAX_PRODUCT_BYTES = 64u * 1024u * 1024u;
	constexpr std::size_t MAX_PRODUCT_VALUES = 4'000'000u;

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
			const auto* timeline = row.Find("timelinePatternId");
			if (!(timeline ? Has_ExactProperties(row, { "folderId", "gateId", "displayName", "timelinePatternId" }) :
				Has_ExactProperties(row, { "folderId", "gateId", "displayName" })) ||
				(timeline && (!timeline->Is_String() || !Is_StableId(timeline->Get_String()))) ||
				!id || !gate || !name || !Is_StableId(id->Get_String()) || name->Get_String().empty() ||
				!hierarchyIds.insert(id->Get_String()).second ||
				!CKoukuSaydonCompositionDocument::Is_KnownGate(gate->Get_String()))
				return fail("invalid parent identity");
			fullFolders.push_back({ id->Get_String(), gate->Get_String(), name->Get_String(), timeline ? timeline->Get_String() : "" });
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
		if (error || size == 0u)
		{ outStatus = "KoukuSaydon Product index is missing, empty or unreadable: " + path.string() + "; previous list retained."; return false; }
		if (size > MAX_PRODUCT_BYTES)
		{ outStatus = "KoukuSaydon Product index exceeds the 64 MiB limit (" + std::to_string(size) + " bytes); previous list retained."; return false; }
		std::ifstream input(path, std::ios::binary);
		if (!input)
		{ outStatus = "KoukuSaydon Product index could not be opened; previous list retained."; return false; }
		std::string text(static_cast<std::size_t>(size), '\0');
		input.read(text.data(), static_cast<std::streamsize>(size));
		if (!input || input.peek() != std::char_traits<char>::eof())
		{ outStatus = "KoukuSaydon Product index changed while reading; reload the inventory after publishing; previous list retained."; return false; }
		DATA_JSON_VALUE root;
		DATA_JSON_PARSE_LIMITS limits;
		limits.iMaximumBytes = static_cast<std::size_t>(MAX_PRODUCT_BYTES);
		limits.iMaximumValues = MAX_PRODUCT_VALUES;
		if (!CDataJson::Parse(text, root, outStatus, limits))
		{ outStatus = "KoukuSaydon Product index could not be parsed: " + outStatus + "; previous list retained."; return false; }
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
			!patterns || patterns->Get_Array().size() > LostArk::Shared::MAX_VALTAN_PATTERN_FLOW_SLOTS)
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
			const auto* timeline = row.Find("timelinePatternId");
			if (timeline && (!timeline->Is_String() || !Is_StableId(timeline->Get_String())))
			{ outStatus = "Invalid Parent timeline; previous inventory retained."; return false; }
			folders.push_back({id->Get_String(),gate->Get_String(),name->Get_String(),timeline ? timeline->Get_String() : ""});
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

bool Client::CKoukuSaydonBossTool::Prepare_ServerPlay(const std::string_view gateId,
	std::vector<std::string> targets, std::vector<std::string> patternIds,
    std::vector<std::string> bundleIds, std::function<bool(std::string&)> submit, std::string& status,
    std::shared_ptr<const KOUKU_SAYDON_DRAFT_PRODUCT> draft)
{
#ifndef _DEBUG
	if (draft) { status = m_strStatus = "Draft Server Play is available only in Debug builds."; return false; }
#endif
	auto& service = CKoukuSaydonPatternAuditionService::Get();
	service.Update();
	auto* arena = CLevel_KakulSaydonArena::Get_Active();
	const auto& network = CNetworkManager::Get();
	const auto revision = network.Get_GameplayRevisionState().ServerActiveRevision;
	if (m_PlayPreparation || service.Get_Snapshot().Is_InFlight() || service.Get_FlowSnapshot().bActive)
	{ status = m_strStatus = "Finish or stop the pending Server Play before starting another Pattern."; return false; }
	if (!arena || !network.Is_Connected() || !revision.Is_Valid())
	{ status = m_strStatus = "Enter the connected KoukuSaydon arena before Server Play."; return false; }
	if (arena->Is_DebugGatePending() || arena->Get_DebugPlayerController().Is_DebugPlayerPlacementPending())
	{ status = m_strStatus = "Wait for the current Server gate or player placement before Server Play."; return false; }
	const std::size_t gateIndex = gateId == "GATE1" ? 0u : gateId == "GATE2" ? 1u :
		gateId == "GATE3" ? 2u : gateId == "BINGO" ? 8u : CLevel_KakulSaydonArena::NO_ACTIVE_DEBUG_GATE;
	if (gateIndex == CLevel_KakulSaydonArena::NO_ACTIVE_DEBUG_GATE || targets.empty() ||
		std::any_of(targets.begin(), targets.end(), [gateId](const auto& target) {
			const auto actor = CKoukuSaydonCompositionDocument::Resolve_ActorProfileForPlacement(target);
			return actor.empty() || CKoukuSaydonCompositionDocument::Resolve_DefaultPlacementId(gateId, actor) != target;
		}))
	{ status = m_strStatus = "Published Pattern has no supported Gate and boss target."; return false; }
	// Gate preparation reuses the Level's typed spawn/teleport approvals and
	// map/light commit. Pattern selection alone never creates a local boss.
	CKoukuSaydonCompositionDocument saved;
	if (!draft && (!saved.Reload(status) || saved.Get_LastGood().iRevision != m_iSourceRevision))
	{ status = m_strStatus = "Save and publish the selected Composition before preparing its Gate. " + status; return false; }
	PLAY_PREPARATION pending;
	pending.TargetPlacementIds = std::move(targets);
    pending.PatternIds = std::move(patternIds); pending.BundleIds = std::move(bundleIds);
    arena->Debug_ResetCompletePlayPreparation();
	pending.GameplayRevision = revision;
	pending.iSourceRevision = draft ? draft->iSourceRevision : m_iSourceRevision;
    pending.Draft = std::move(draft);
	pending.iWorldGeneration = network.Get_WorldInboundGeneration();
	pending.iPreviousRequestSequence = service.Get_Snapshot().iRequestSequence;
	pending.iGateIndex = gateIndex;
	pending.iDeadlineMilliseconds = GetTickCount64() + 20000u;
	pending.Submit = std::move(submit);
	// Only a typed gate approval from this world and raid epoch can be reused.
	// Full-raid Stop keeps the scene but removes its bosses; slow NPC
	// construction after a real Debug approval still waits without respawning.
	if (!arena->Is_DebugGateApprovedForServerPlay(gateIndex) && !arena->Debug_ActivateGate(gateIndex, status))
	{ m_strStatus = status; return false; }
	pending.iGateGeneration = arena->Get_DebugGateGeneration();
	m_PlayPreparation = std::move(pending);
	status = m_strStatus = "Preparing " + std::string(gateId) +
		" on the Server; playback waits for approved placement and the complete Effect/WORLD dependency closure.";
	return true;
}

bool Client::CKoukuSaydonBossTool::Play_Draft(KOUKU_DRAFT_PLAY_REQUEST request, std::string& status)
{
#ifndef _DEBUG
    status = m_strStatus = "Draft Server Play is available only in Debug builds.";
    return false;
#else
    const auto& network = CNetworkManager::Get();
    if (!network.Is_Connected() || network.Get_WorldInboundGeneration() != request.iWorldGeneration)
    { status = m_strStatus = "The connected world changed. Start draft playback again."; return false; }
    if (request.TargetPlacementIds.empty() || request.strTargetId.empty() || !request.iSourceRevision)
    { status = m_strStatus = "Draft playback has no validated selection."; return false; }
    auto draft = CKoukuSaydonPresentationAssetService::Prepare_DraftProduct(
        request.PresentationJson, request.EncounterJson, request.GameplayRows, request.iSourceRevision, status);
    if (!draft) { m_strStatus = status; return false; }
    const auto revision = network.Get_GameplayRevisionState().ServerActiveRevision;
    const auto target = request.TargetPlacementIds.front();
    const auto id = request.strTargetId, gate = request.strGateId;
    const auto sourceRevision = request.iSourceRevision;
    const bool bundle = request.bBundle;
    const auto marioStage = CKoukuSaydonPatternAuditionService::Get().Get_MarioTestStage();
    const auto marioSeed = CKoukuSaydonPatternAuditionService::Get().Get_MarioTestSeed();
    return Prepare_ServerPlay(gate, std::move(request.TargetPlacementIds),
        std::move(request.PatternIds), std::move(request.BundleIds),
        [id, gate, target, revision, sourceRevision, bundle, marioStage, marioSeed,
            rows = std::move(request.GameplayRows), draft](std::string& reason) {
            if (!CKoukuSaydonPresentationAssetService::Stage_DraftProduct(draft, reason)) return false;
            auto& service = CKoukuSaydonPatternAuditionService::Get();
            service.Set_TargetBoss(target, CKoukuSaydonCompositionDocument::Resolve_BossArchetypeId(target));
            service.Set_MarioTest(marioStage, marioSeed);
            return bundle ? service.Play_DraftBundle(id, gate, revision, sourceRevision, rows, reason) :
                service.Play_DraftSelected(id, revision, sourceRevision, rows, reason);
        }, status, draft);
#endif
}

bool Client::CKoukuSaydonBossTool::Cancel_PlayPreparation(std::string& status)
{
	if (!m_PlayPreparation) return false;
    if (auto* arena = CLevel_KakulSaydonArena::Get_Active()) arena->Debug_ResetCompletePlayPreparation();
	m_PlayPreparation.reset();
	status = m_strStatus = "Pending Server Play cancelled; the submitted gate placement may still finish.";
	return true;
}

void Client::CKoukuSaydonBossTool::Update()
{
	if (!m_PlayPreparation) return;
	const auto fail = [&](const std::string& reason) {
        if (auto* arena = CLevel_KakulSaydonArena::Get_Active()) arena->Debug_ResetCompletePlayPreparation();
		m_PlayPreparation.reset();
		m_strStatus = "Server Play preparation stopped: " + reason;
	};
	auto& pending = *m_PlayPreparation;
	const auto& network = CNetworkManager::Get();
	auto* arena = CLevel_KakulSaydonArena::Get_Active();
	const auto& service = CKoukuSaydonPatternAuditionService::Get();
	if (!arena || !network.Is_Connected() || network.Get_WorldInboundGeneration() != pending.iWorldGeneration)
	{ fail("the connected world changed."); return; }
	if (network.Get_GameplayRevisionState().ServerActiveRevision != pending.GameplayRevision ||
		service.Get_Snapshot().iRequestSequence != pending.iPreviousRequestSequence ||
		service.Get_Snapshot().Is_InFlight() || service.Get_FlowSnapshot().bActive)
	{ fail("another request or gameplay revision replaced this preparation."); return; }
	if (arena->Get_DebugGateGeneration() != pending.iGateGeneration)
	{ fail("another gate activation replaced this preparation."); return; }
	if (GetTickCount64() >= pending.iDeadlineMilliseconds)
    {
        fail(pending.bPreparingResources ? "the complete resource preparation exceeded 20 minutes; no Pattern was started. " + m_strStatus :
            "timed out waiting for the approved Gate and replicated boss. " + arena->Get_DebugGateStatus());
        return;
    }
	if (arena->Is_DebugGatePending()) return;
	if (!arena->Is_DebugGateApprovedForServerPlay(pending.iGateIndex))
	{ fail("the Server gate approval was replaced. " + arena->Get_DebugGateStatus()); return; }
	if (!std::all_of(pending.TargetPlacementIds.begin(), pending.TargetPlacementIds.end(), [&](const auto& target) {
		return arena->Debug_FindArenaBossNpc(CKoukuSaydonCompositionDocument::Resolve_BossArchetypeId(target)) != nullptr;
	})) return;
    if (!pending.bPreparingResources)
    {
        pending.bPreparingResources = true;
        pending.iDeadlineMilliseconds = GetTickCount64() + 20u * 60u * 1000u;
    }
    bool resourcesReady = false;
    std::string preparationStatus;
    if (!arena->Debug_PrepareCompletePlayResources(pending.PatternIds, pending.BundleIds,
        pending.iSourceRevision, resourcesReady, preparationStatus, false, pending.Draft))
    { fail(preparationStatus); return; }
    m_strStatus = std::move(preparationStatus);
    if (!resourcesReady) return;
	// Read into temporary owners so a delayed submission cannot reload an editor
	// draft or silently replace the exact source selected at the original click.
	CKoukuSaydonCompositionDocument saved;
	CKoukuSaydonBossTool published;
	std::string status;
    if (!pending.Draft)
    {
        if (!saved.Reload(status) || !published.Reload(status))
        { fail("saved or published data could not be rechecked. " + status); return; }
        if (saved.Get_LastGood().iRevision != pending.iSourceRevision || published.Get_SourceRevision() != pending.iSourceRevision)
        { fail("the saved or published Composition changed. Start the updated Pattern explicitly."); return; }
    }
	auto submit = std::move(m_PlayPreparation->Submit);
	m_PlayPreparation.reset();
	(void)submit(status);
	m_strStatus = status;
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
		m_strProductLoadError = outStatus;
		m_strStatus = outStatus;
		return false;
	}
	m_strProductLoadError.clear();
	m_ProductFolders=std::move(stagedFolders); m_ProductBundles=std::move(stagedBundles);
	m_ProductPatterns = std::move(stagedPatterns);
	m_PlayAllPatternIds = std::move(stagedPlayAll);
	m_iSourceRevision = stagedSourceRevision;
	m_bHasSavedComposition = true;
	Normalize_Selection();
	if (!m_bFlowDirty)
	{
		std::string flowStatus;
		if (!Load_PatternFlows(flowStatus)) outStatus += "\n" + flowStatus;
	}
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
	const auto revision =
		CNetworkManager::Get().Get_GameplayRevisionState().ServerActiveRevision;
#ifdef _DEBUG
	const auto* arena = CLevel_KakulSaydonArena::Get_Active();
	const bool encoreReplay = m_bReplayGate3OnEncore && arena && arena->Get_ActiveDebugGate() == 8u &&
		pattern->strGateId == "GATE3" && pattern->strActorProfileId == "MN_RPCT_05";
#else
    constexpr bool encoreReplay = false;
#endif
	if (encoreReplay)
	{
		const auto& document = m_FlowDocument.Get_LastGood();
		const auto source = std::find_if(document.Patterns.begin(), document.Patterns.end(),
			[&](const auto& row) { return row.strPatternId == patternId; });
		const bool hasScene = source != document.Patterns.end() && std::any_of(
			source->PresentationOccurrences.begin(), source->PresentationOccurrences.end(), [&](const auto& cue) {
				const auto resource = std::find_if(document.PresentationResources.begin(), document.PresentationResources.end(),
					[&](const auto& row) { return row.strResourceId == cue.strResourceId; });
				return resource != document.PresentationResources.end() && resource->eKind == KOUKU_SAYDON_PRESENTATION_KIND::CAMERA;
			});
		if (source == document.Patterns.end() || !source->WorldOccurrences.empty() ||
			!source->SceneProfileOccurrences.empty() || !source->LogicOccurrences.empty() ||
			!source->SummonOccurrences.empty() || !source->PatternOccurrences.empty() || source->BossMotion || hasScene)
		{
			outStatus = m_strStatus = "This pattern owns a Gate 3 layout or mechanic. Disable Encore reuse to play it in Gate 3.";
			return false;
		}
	}
	const std::string id = pattern->strPatternId;
	const std::string target = encoreReplay ? "boss.kakulsaydon.bingo.saydon" : pattern->strTargetBossPlacementId;
	const auto sourceRevision = m_iSourceRevision;
	const auto marioStage = CKoukuSaydonPatternAuditionService::Get().Get_MarioTestStage();
	const auto marioSeed = CKoukuSaydonPatternAuditionService::Get().Get_MarioTestSeed();
	return Prepare_ServerPlay(encoreReplay ? "BINGO" : pattern->strGateId, {target}, {id}, {},
		[id, target, revision, sourceRevision, marioStage, marioSeed](std::string& status) {
			auto& service = CKoukuSaydonPatternAuditionService::Get();
			service.Set_TargetBoss(target, CKoukuSaydonCompositionDocument::Resolve_BossArchetypeId(target));
			service.Set_MarioTest(marioStage, marioSeed);
			return service.Play_Selected(id, revision, sourceRevision, status);
		}, outStatus);
}

bool Client::CKoukuSaydonBossTool::Play_All(std::string& outStatus)
{
	static const char* gates[] = { "GATE1", "GATE2", "GATE3", "BINGO" };
	return Play_CompositionAll(gates[(std::clamp)(m_iSelectedGate, 0, 3)], outStatus);
}

bool Client::CKoukuSaydonBossTool::Load_PatternFlows(std::string& status)
{
	if (!m_FlowDocument.Reload(status))
	{ m_strFlowLoadError = status; return false; }
	m_strFlowLoadError.clear();
	m_FlowDraft = m_FlowDocument.Get_LastGood().PatternFlows;
	m_bFlowDirty = false;
	m_strSelectedFlowEntryId.clear();
	status = "Loaded saved Pattern Flows.";
	return true;
}

bool Client::CKoukuSaydonBossTool::Request_PublishSavedPatterns(std::string& status)
{
	if (m_bFlowDirty)
	{
		status = m_strStatus = "Save Pattern Flow edits before publishing saved Patterns.";
		return false;
	}
	m_bPublishRequested = true;
	status = m_strStatus = "Preparing the saved Patterns publisher.";
	return true;
}

bool Client::CKoukuSaydonBossTool::Save_PatternFlows(std::string& status)
{
	if (!m_FlowDocument.Has_LastGood())
	{ status = "Load the Composition before saving Pattern Flow."; return false; }
	auto candidate = m_FlowDocument.Get_LastGood();
	candidate.PatternFlows = m_FlowDraft;
	if (!m_FlowDocument.Save_Atomic(candidate, status)) return false;
	m_FlowDraft = m_FlowDocument.Get_LastGood().PatternFlows;
	m_bFlowDirty = false;
	status = "Saved Pattern Flow. Publish Saved Patterns before Complete Play.";
	return true;
}

const Client::KOUKU_SAYDON_COMPOSITION_PATTERN_FLOW*
Client::CKoukuSaydonBossTool::Get_SavedFlow(const std::string_view gateId) const
{
	if (!m_FlowDocument.Has_LastGood()) return nullptr;
	const auto& flows = m_FlowDocument.Get_LastGood().PatternFlows;
	const auto found = std::find_if(flows.begin(), flows.end(),
		[&](const auto& flow) { return flow.strGateId == gateId; });
	return found == flows.end() ? nullptr : &*found;
}

Client::KOUKU_SAYDON_COMPOSITION_PATTERN_FLOW*
Client::CKoukuSaydonBossTool::Find_DraftFlow(const std::string_view gateId)
{
	const auto found = std::find_if(m_FlowDraft.begin(), m_FlowDraft.end(),
		[&](const auto& flow) { return flow.strGateId == gateId; });
	return found == m_FlowDraft.end() ? nullptr : &*found;
}

std::string Client::CKoukuSaydonBossTool::Describe_FlowEntry(
	const KOUKU_SAYDON_COMPOSITION_FLOW_ENTRY& entry, const std::string_view gateId,
	std::string& error) const
{
	error.clear();
	if (entry.strKind == "PATTERN")
	{
		const auto found = std::find_if(m_ProductPatterns.begin(), m_ProductPatterns.end(),
			[&](const auto& pattern) { return pattern.strPatternId == entry.strTargetId; });
		if (found != m_ProductPatterns.end())
		{
			error = found->strGateId != gateId ? "Pattern belongs to another Gate." : found->strLoadError;
			if (error.empty() && found->Stages.empty()) error = "Pattern has no published stages.";
			return found->strDisplayName;
		}
	}
	else if (entry.strKind == "BUNDLE")
	{
		const auto found = std::find_if(m_ProductBundles.begin(), m_ProductBundles.end(),
			[&](const auto& bundle) { return bundle.strBundleId == entry.strTargetId; });
		if (found != m_ProductBundles.end())
		{
			error = found->strGateId != gateId ? "Bundle belongs to another Gate." : found->strLoadError;
			if (error.empty() && found->Members.empty()) error = "Bundle has no published members.";
			return found->strDisplayName;
		}
	}
	error = "Saved target is missing from All Patterns. Publish Saved Patterns or replace this row.";
	return entry.strTargetId;
}

bool Client::CKoukuSaydonBossTool::Prepare_PatternFlow(const std::string_view gateId,
	KOUKU_SAYDON_COMPOSITION_PATTERN_FLOW& flow, std::string& status)
{
	if (!Reload(status)) return false;
	CKoukuSaydonCompositionDocument saved;
	if (!saved.Reload(status)) return false;
	if (saved.Get_LastGood().iRevision != m_iSourceRevision)
	{ status = m_strStatus = "Saved Pattern Flow and published Patterns differ. Publish Saved Patterns before Complete Play."; return false; }
	const auto& flows = saved.Get_LastGood().PatternFlows;
	const auto found = std::find_if(flows.begin(), flows.end(),
		[&](const auto& row) { return row.strGateId == gateId; });
	if (found == flows.end() || found->Entries.empty())
	{ status = m_strStatus = "This Gate has no saved Pattern Flow. Add Pattern or Bundle rows in Boss Tool > Pattern Flow and Save Pattern Flow."; return false; }
	for (const auto& entry : found->Entries)
	{
		std::string error;
		const auto name = Describe_FlowEntry(entry, gateId, error);
		if (!error.empty())
		{ status = m_strStatus = "Pattern Flow '" + name + "': " + error; return false; }
	}
	flow = *found;
	status = "Saved Pattern Flow is ready.";
	return true;
}

bool Client::CKoukuSaydonBossTool::Validate_PatternFlow(const std::string_view gateId, std::string& status)
{
	KOUKU_SAYDON_COMPOSITION_PATTERN_FLOW flow;
	return Prepare_PatternFlow(gateId, flow, status);
}

bool Client::CKoukuSaydonBossTool::Play_PatternFlow(const std::string_view gateId, std::string& status,
    const std::uint32_t expectedSourceRevision)
{
	KOUKU_SAYDON_COMPOSITION_PATTERN_FLOW flow;
	if (!Prepare_PatternFlow(gateId, flow, status)) return false;
    // Prepare reloads both saved flow and published inventory. Check the exact data
    // this submission will consume, including a publish completed after preflight.
    if (expectedSourceRevision != 0u && m_iSourceRevision != expectedSourceRevision)
    {
        status = m_strStatus = "Saved Pattern Flow changed after entry admission. Start the updated Pattern Flow explicitly.";
        return false;
    }
    if (std::any_of(flow.EntryGroups.begin(), flow.EntryGroups.end(), [](const auto& group) { return group.RepeatUntilHealthBars.has_value(); }))
    {
        if (!m_CompletePlayAdmission) { status = m_strStatus = "HP Pattern Flow requires Server Complete Play admission."; return false; }
        return m_CompletePlayAdmission(gateId, status);
    }
    std::vector<KOUKU_SAYDON_PATTERN_FLOW_ENTRY> entries;
	for (const auto& source : flow.Entries)
	{
		KOUKU_SAYDON_PATTERN_FLOW_ENTRY entry;
		entry.strEntryId = source.strEntryId;
		entry.iWaitAfterMs = source.iWaitAfterMs;
		if (source.strKind == "BUNDLE")
		{
			const auto bundle = std::find_if(m_ProductBundles.begin(), m_ProductBundles.end(),
				[&](const auto& row) { return row.strBundleId == source.strTargetId; });
			entry.strBundleId = source.strTargetId;
			entry.strBossPlacementId = bundle->Members.front().strTargetBossPlacementId;
		}
		else
		{
			const auto pattern = std::find_if(m_ProductPatterns.begin(), m_ProductPatterns.end(),
				[&](const auto& row) { return row.strPatternId == source.strTargetId; });
			entry.strPatternId = source.strTargetId;
			entry.strBossPlacementId = pattern->strTargetBossPlacementId;
		}
		entry.strBossArchetypeId = CKoukuSaydonCompositionDocument::Resolve_BossArchetypeId(entry.strBossPlacementId);
		entries.push_back(std::move(entry));
	}
	return Play_LoadedFlow(gateId, entries, status, flow.strLoopStartEntryId);
}

bool Client::CKoukuSaydonBossTool::Play_CompositionAll(const std::string_view gateId, std::string& status)
{
	if (!Reload(status)) return false;
	std::vector<KOUKU_SAYDON_PATTERN_FLOW_ENTRY> entries;
	std::unordered_set<std::string> addedBundles;
	for (const auto& patternId : m_PlayAllPatternIds)
	{
		const auto pattern = std::find_if(m_ProductPatterns.begin(), m_ProductPatterns.end(),
			[&](const auto& row) { return row.strPatternId == patternId && row.strGateId == gateId; });
		if (pattern == m_ProductPatterns.end() || !pattern->strLoadError.empty()) continue;
		bool bundled = false;
		for (const auto& bundle : m_ProductBundles)
		{
			if (bundle.strGateId != gateId || !bundle.strLoadError.empty() || bundle.Members.empty() ||
				!std::any_of(bundle.Members.begin(), bundle.Members.end(),
					[&](const auto& member) { return member.strPatternId == patternId; })) continue;
			bundled = true;
			if (!addedBundles.insert(bundle.strBundleId).second) continue;
			KOUKU_SAYDON_PATTERN_FLOW_ENTRY entry;
			entry.strEntryId = "all.bundle." + std::to_string(entries.size() + 1u);
			entry.strBundleId = bundle.strBundleId;
			entry.strBossPlacementId = bundle.Members.front().strTargetBossPlacementId;
			entry.strBossArchetypeId = CKoukuSaydonCompositionDocument::Resolve_BossArchetypeId(entry.strBossPlacementId);
			entries.push_back(std::move(entry));
		}
		if (bundled) continue;
		KOUKU_SAYDON_PATTERN_FLOW_ENTRY entry;
		entry.strEntryId = "all.pattern." + std::to_string(entries.size() + 1u);
		entry.strPatternId = patternId;
		entry.strBossPlacementId = pattern->strTargetBossPlacementId;
		entry.strBossArchetypeId = CKoukuSaydonCompositionDocument::Resolve_BossArchetypeId(entry.strBossPlacementId);
		entries.push_back(std::move(entry));
	}
	if (entries.empty())
	{ status = m_strStatus = "This Gate has no published Pattern or Bundle to play."; return false; }
	return Play_LoadedFlow(gateId, entries, status);
}

bool Client::CKoukuSaydonBossTool::Play_LoadedFlow(const std::string_view gateId,
	const std::vector<KOUKU_SAYDON_PATTERN_FLOW_ENTRY>& entries, std::string& status, const std::string_view loopStartEntryId)
{
	std::vector<std::string> targets, patternIds, bundleIds;
	for (const auto& entry : entries)
	{
		if (entry.strBundleId.empty()) { targets.push_back(entry.strBossPlacementId); patternIds.push_back(entry.strPatternId); }
		else
		{
            bundleIds.push_back(entry.strBundleId);
			const auto bundle = std::find_if(m_ProductBundles.begin(), m_ProductBundles.end(),
				[&](const auto& item) { return item.strBundleId == entry.strBundleId; });
			if (bundle == m_ProductBundles.end())
			{ status = m_strStatus = "The selected Flow bundle is unavailable."; return false; }
			for (const auto& member : bundle->Members) targets.push_back(member.strTargetBossPlacementId);
		}
	}
	const auto revision = CNetworkManager::Get().Get_GameplayRevisionState().ServerActiveRevision;
	const auto sourceRevision = m_iSourceRevision;
	return Prepare_ServerPlay(gateId, std::move(targets), std::move(patternIds), std::move(bundleIds),
		[gate = std::string(gateId), entries, revision, sourceRevision, loopStart = std::string(loopStartEntryId)](std::string& output) {
			return CKoukuSaydonPatternAuditionService::Get().Play_Flow(gate, entries, revision, sourceRevision, output, loopStart);
		}, status);
}

bool Client::CKoukuSaydonBossTool::Render_SavedPatternFlow(const std::string_view gateId,
	int& selectionKind, std::string& selectedId) const
{
	if (!m_strProductLoadError.empty())
		ImGui::TextWrapped("Inventory load failed: %s", m_strProductLoadError.c_str());
	if (!m_bHasSavedComposition) return false;
	if (!m_strFlowLoadError.empty())
		ImGui::TextWrapped("Pattern Flow load failed: %s", m_strFlowLoadError.c_str());
	if (!m_FlowDocument.Has_LastGood()) return false;
	const auto* flow = Get_SavedFlow(gateId);
	if (!flow || flow->Entries.empty())
	{
		ImGui::TextWrapped("No saved Pattern Flow for this Gate. Open Boss Tool > Pattern Flow to choose the battle order.");
		selectionKind = 0; selectedId.clear();
		return false;
	}
	ImGui::TextUnformatted(flow->strDisplayName.c_str());
	const auto drawEntry = [&](const std::size_t i) {
		const auto& entry = flow->Entries[i];
		std::string error;
		const std::string name = Describe_FlowEntry(entry, gateId, error);
		const int kind = entry.strKind == "BUNDLE" ? 2 : 3;
		const std::string label = std::to_string(i + 1u) + ". [" + entry.strKind + "] " + name +
			(error.empty() ? "" : " [Unavailable]") + "##" + entry.strEntryId;
		if (ImGui::Selectable(label.c_str(), selectionKind == kind && selectedId == entry.strTargetId))
		{ selectionKind = kind; selectedId = entry.strTargetId; }
		if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s\nWait after: %u ms\n%s", entry.strTargetId.c_str(), entry.iWaitAfterMs, error.c_str());
	};
	for (std::size_t i = 0u; i < flow->Entries.size();)
	{
		const auto group = std::find_if(flow->EntryGroups.begin(), flow->EntryGroups.end(),
			[&](const auto& row) { return row.strStartEntryId == flow->Entries[i].strEntryId; });
		if (group == flow->EntryGroups.end()) { drawEntry(i++); continue; }
		const auto last = std::find_if(flow->Entries.begin() + i, flow->Entries.end(),
			[&](const auto& row) { return row.strEntryId == group->strEndEntryId; });
		if (last == flow->Entries.end()) { drawEntry(i++); continue; }
		const std::size_t end = static_cast<std::size_t>(last - flow->Entries.begin()) + 1u;
		const std::string label = group->strDisplayName + (group->RepeatUntilHealthBars ?
			" [Repeat until " + std::to_string(*group->RepeatUntilHealthBars) + " HP bars]" : "") + "##" + group->strGroupId;
		if (ImGui::TreeNodeEx(label.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
		{
			if (group->RepeatUntilHealthBars) ImGui::TextDisabled("Transition after %s completes", group->bTransitionAtGroupEnd ? "the group" : "the current pattern");
			while (i < end) drawEntry(i++);
			ImGui::TreePop();
		}
		else i = end;
	}
	return true;
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
	const std::string target = found->Members.front().strTargetBossPlacementId;
	const std::string id = found->strBundleId, gate = found->strGateId;
	std::vector<std::string> targets;
	for (const auto& member : found->Members) targets.push_back(member.strTargetBossPlacementId);
	const auto revision = CNetworkManager::Get().Get_GameplayRevisionState().ServerActiveRevision;
	const auto sourceRevision = m_iSourceRevision;
	return Prepare_ServerPlay(gate, std::move(targets), {}, {id},
		[id, gate, target, revision, sourceRevision](std::string& output) {
			auto& service = CKoukuSaydonPatternAuditionService::Get();
			service.Set_TargetBoss(target, CKoukuSaydonCompositionDocument::Resolve_BossArchetypeId(target));
			return service.Play_Bundle(id, gate, revision, sourceRevision, output);
		}, status);
}

bool Client::CKoukuSaydonBossTool::Render_PatternTree(
	const std::string_view gateId, int& selectionKind, std::string& selectedId) const
{
	if (!m_strProductLoadError.empty())
		ImGui::TextWrapped("Inventory load failed: %s", m_strProductLoadError.c_str());
	if (!m_bHasSavedComposition) return false;
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
				((selectionKind==1 && selectedId==folder.strFolderId) || (!folder.strTimelinePatternId.empty() && selectionKind==3 && selectedId==folder.strTimelinePatternId) ? ImGuiTreeNodeFlags_Selected:0));
			if (ImGui::IsItemClicked()&&!ImGui::IsItemToggledOpen())
			{
				selectionKind = folder.strTimelinePatternId.empty() ? 1 : 3;
				selectedId = folder.strTimelinePatternId.empty() ? folder.strFolderId : folder.strTimelinePatternId;
			}
			if (ImGui::IsItemHovered() && !folder.strTimelinePatternId.empty())
				if (const auto* timeline = findPattern(folder.strTimelinePatternId)) ImGui::SetTooltip("Parent timeline: %s\n%s", timeline->strPatternId.c_str(), timeline->strLoadError.c_str());
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
					if (pattern.strGateId==gate && pattern.strFolderId==folder.strFolderId && pattern.strPatternId!=folder.strTimelinePatternId) selectPattern(pattern,pattern.strPatternId);
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

void Client::CKoukuSaydonBossTool::Render_PatternFlowEditor(const std::string_view gateId)
{
	if (!m_FlowDocument.Has_LastGood())
	{
		if (ImGui::Button("Load Pattern Flow")) (void)Load_PatternFlows(m_strStatus);
		ImGui::TextWrapped("Pattern Flow could not be loaded. The previous saved Composition is preserved.");
		return;
	}
	if (ImGui::Button("Load Pattern Flow"))
	{
		if (m_bFlowDirty) ImGui::OpenPopup("Discard Pattern Flow edits?");
		else (void)Load_PatternFlows(m_strStatus);
	}
	ImGui::SameLine();
	ImGui::BeginDisabled(!m_bFlowDirty);
	if (ImGui::Button("Save Pattern Flow")) (void)Save_PatternFlows(m_strStatus);
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::BeginDisabled(m_bFlowDirty);
	if (ImGui::Button("Publish Saved Patterns")) (void)Request_PublishSavedPatterns(m_strStatus);
	ImGui::EndDisabled();
	if (ImGui::BeginPopupModal("Discard Pattern Flow edits?", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::TextUnformatted("Load replaces unsaved Pattern Flow edits for every Gate.");
		if (ImGui::Button("Discard and Load"))
		{ (void)Load_PatternFlows(m_strStatus); ImGui::CloseCurrentPopup(); }
		ImGui::SameLine();
		if (ImGui::Button("Keep Editing")) ImGui::CloseCurrentPopup();
		ImGui::EndPopup();
	}
	ImGui::TextDisabled("%s | Source revision %u | Published revision %u", m_bFlowDirty ? "Unsaved Flow" : "Saved Flow",
		m_FlowDocument.Get_LastGood().iRevision, m_iSourceRevision);
	ImGui::TextWrapped("Choose the battle order for this Gate. A Bundle plays every member together; the next row waits for Server completion.");
	auto& service = CKoukuSaydonPatternAuditionService::Get();
	const auto& run = service.Get_FlowSnapshot();
	const auto* saved = Get_SavedFlow(gateId);
	ImGui::BeginDisabled(m_bFlowDirty || !saved || saved->Entries.empty() ||
		m_FlowDocument.Get_LastGood().iRevision != m_iSourceRevision || service.Get_Snapshot().Is_InFlight() || run.bActive || Is_PlayPreparationPending());
	if (ImGui::Button("Play Saved Pattern Flow")) (void)Play_PatternFlow(gateId, m_strStatus);
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::BeginDisabled(!run.bActive && !service.Get_Snapshot().Is_InFlight() && !Is_PlayPreparationPending());
	if (ImGui::Button("Stop Playback") && !Cancel_PlayPreparation(m_strStatus)) (void)service.Stop(m_strStatus);
	ImGui::EndDisabled();
	if (!run.strStatus.empty()) ImGui::TextWrapped("%s", run.strStatus.c_str());
	ImGui::Separator();
	if (ImGui::Button("Add From All Patterns...")) ImGui::OpenPopup("Add From All Patterns");
	if (ImGui::BeginPopup("Add From All Patterns"))
	{
		const char* kinds[] = { "Pattern", "Bundle" };
		ImGui::SetNextItemWidth(180.f);
		ImGui::Combo("Type", &m_iFlowAddKind, kinds, 2);
		ImGui::TextDisabled("%.*s | Click an available row to append it.", static_cast<int>(gateId.size()), gateId.data());
		ImGui::BeginChild("##FlowAvailableRows", ImVec2(530.f, 350.f), ImGuiChildFlags_Borders);
		const auto append = [&](const std::string& targetId, const char* kind)
		{
			auto* flow = Find_DraftFlow(gateId);
			if (!flow)
			{
				KOUKU_SAYDON_COMPOSITION_PATTERN_FLOW created;
				created.strFlowId = "flow." + std::string(gateId);
				created.strGateId = gateId;
				created.strDisplayName = std::string(gateId) + " Pattern Flow";
				m_FlowDraft.push_back(std::move(created));
				flow = &m_FlowDraft.back();
			}
			if (flow->Entries.size() >= 256u)
			{ m_strStatus = "Pattern Flow supports at most 256 rows."; return; }
			std::uint32_t ordinal = 1u;
			std::string entryId;
			do { entryId = flow->strFlowId + ".entry." + std::to_string(ordinal++); }
			while (std::any_of(flow->Entries.begin(), flow->Entries.end(), [&](const auto& row) { return row.strEntryId == entryId; }));
			KOUKU_SAYDON_COMPOSITION_FLOW_ENTRY entry;
			entry.strEntryId = entryId; entry.strKind = kind; entry.strTargetId = targetId;
			flow->Entries.push_back(std::move(entry));
			m_strSelectedFlowEntryId = entryId;
			m_bFlowDirty = true;
			m_strStatus = "Appended " + targetId + ". Save Pattern Flow keeps this order.";
		};
		if (m_iFlowAddKind == 0)
		{
			for (const auto& pattern : m_ProductPatterns)
			{
				if (pattern.strGateId != gateId) continue;
				const bool ready = pattern.strLoadError.empty() && !pattern.Stages.empty();
				ImGui::BeginDisabled(!ready);
				if (ImGui::Selectable((pattern.strDisplayName + (ready ? "" : " [Unavailable]") + "##add." + pattern.strPatternId).c_str()))
					append(pattern.strPatternId, "PATTERN");
				ImGui::EndDisabled();
				if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
					ImGui::SetTooltip("%s\n%s", pattern.strPatternId.c_str(), pattern.strLoadError.c_str());
			}
		}
		else
		{
			for (const auto& bundle : m_ProductBundles)
			{
				if (bundle.strGateId != gateId) continue;
				const bool ready = bundle.strLoadError.empty() && !bundle.Members.empty();
				ImGui::BeginDisabled(!ready);
				if (ImGui::Selectable((bundle.strDisplayName + " [" + std::to_string(bundle.Members.size()) + " actors]" +
					(ready ? "" : " [Unavailable]") + "##add." + bundle.strBundleId).c_str())) append(bundle.strBundleId, "BUNDLE");
				ImGui::EndDisabled();
				if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
					ImGui::SetTooltip("%s\n%s", bundle.strBundleId.c_str(), bundle.strLoadError.c_str());
			}
		}
		ImGui::EndChild();
		if (ImGui::Button("Close")) ImGui::CloseCurrentPopup();
		ImGui::EndPopup();
	}
	auto* flow = Find_DraftFlow(gateId);
	if (!flow || flow->Entries.empty())
	{ ImGui::TextWrapped("The Flow is empty. Add only the Patterns and Bundles used in this Gate's battle."); return; }
	ImGui::SameLine();
	ImGui::Text("%zu rows", flow->Entries.size());
	const auto selected = std::find_if(flow->Entries.begin(), flow->Entries.end(),
		[&](const auto& entry) { return entry.strEntryId == m_strSelectedFlowEntryId; });
	const std::size_t index = static_cast<std::size_t>(selected - flow->Entries.begin());
	const bool hasSelection = index < flow->Entries.size();
	if (ImGui::CollapsingHeader("Flow Groups / HP Repeats"))
	{
		ImGui::TextWrapped("Groups reference the saved rows below. HP repeats run through Server Complete Play and wait for the selected completion boundary.");
		ImGui::BeginDisabled(!hasSelection || flow->EntryGroups.size() >= 64u);
		if (ImGui::Button("Group Selected Row"))
		{
			KOUKU_SAYDON_COMPOSITION_FLOW_GROUP group;
			std::uint32_t ordinal = 1u;
			do { group.strGroupId = flow->strFlowId + ".group." + std::to_string(ordinal++); }
			while (std::any_of(flow->EntryGroups.begin(), flow->EntryGroups.end(), [&](const auto& row) { return row.strGroupId == group.strGroupId; }));
			group.strDisplayName = "Pattern Group";
			group.strStartEntryId = group.strEndEntryId = flow->Entries[index].strEntryId;
			flow->EntryGroups.push_back(std::move(group));
			m_bFlowDirty = true;
		}
		ImGui::EndDisabled();
		for (std::size_t groupIndex = 0u; groupIndex < flow->EntryGroups.size();)
		{
			auto& group = flow->EntryGroups[groupIndex];
			ImGui::PushID(group.strGroupId.c_str());
			bool remove = false;
			if (ImGui::TreeNodeEx(group.strDisplayName.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
			{
				char name[512]{};
				std::copy_n(group.strDisplayName.begin(), (std::min)(group.strDisplayName.size(), sizeof(name) - 1u), name);
				if (ImGui::InputText("Name", name, sizeof(name))) { group.strDisplayName = name; m_bFlowDirty = true; }
				const auto chooseEntry = [&](const char* label, std::string& selectedId) {
					if (!ImGui::BeginCombo(label, selectedId.c_str())) return;
					for (const auto& entry : flow->Entries)
					{
						std::string error;
						const auto title = Describe_FlowEntry(entry, gateId, error) + "##" + entry.strEntryId;
						if (ImGui::Selectable(title.c_str(), entry.strEntryId == selectedId)) { selectedId = entry.strEntryId; m_bFlowDirty = true; }
					}
					ImGui::EndCombo();
				};
				chooseEntry("First row", group.strStartEntryId); chooseEntry("Last row", group.strEndEntryId);
				bool repeat = group.RepeatUntilHealthBars.has_value();
				if (ImGui::Checkbox("Repeat until HP bars", &repeat))
				{ group.RepeatUntilHealthBars = repeat ? std::optional<std::uint32_t>(0u) : std::nullopt; group.bTransitionAtGroupEnd = false; m_bFlowDirty = true; }
				if (repeat)
				{
					int bars = static_cast<int>(*group.RepeatUntilHealthBars);
					if (ImGui::InputInt("HP bars", &bars)) { group.RepeatUntilHealthBars = static_cast<std::uint32_t>((std::clamp)(bars, 0, 1000)); m_bFlowDirty = true; }
					if (ImGui::Checkbox("Finish entire group before transition", &group.bTransitionAtGroupEnd)) m_bFlowDirty = true;
				}
				remove = ImGui::Button("Remove Group");
				ImGui::TreePop();
			}
			ImGui::PopID();
			if (remove) { flow->EntryGroups.erase(flow->EntryGroups.begin() + groupIndex); m_bFlowDirty = true; }
			else ++groupIndex;
		}
		// Store ranges in their visible entry order; validation still rejects overlap or reversed endpoints.
		std::stable_sort(flow->EntryGroups.begin(), flow->EntryGroups.end(), [&](const auto& a, const auto& b) {
			const auto position = [&](const auto& id) { return std::find_if(flow->Entries.begin(), flow->Entries.end(), [&](const auto& row) { return row.strEntryId == id; }); };
			return position(a.strStartEntryId) < position(b.strStartEntryId);
		});
	}
	ImGui::BeginDisabled(!hasSelection || index == 0u);
	if (ImGui::Button("Up")) { std::swap(flow->Entries[index], flow->Entries[index - 1u]); m_bFlowDirty = true; }
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::BeginDisabled(!hasSelection || index + 1u >= flow->Entries.size());
	if (ImGui::Button("Down")) { std::swap(flow->Entries[index], flow->Entries[index + 1u]); m_bFlowDirty = true; }
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::BeginDisabled(!hasSelection);
	if (ImGui::Button("Remove"))
	{
		if (flow->strLoopStartEntryId == flow->Entries[index].strEntryId) flow->strLoopStartEntryId.clear();
		flow->Entries.erase(flow->Entries.begin() + index);
		m_strSelectedFlowEntryId.clear();
		m_bFlowDirty = true;
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::BeginDisabled(!hasSelection || m_strSelectedFlowEntryId.empty());
	if (ImGui::Button("Repeat From Selected")) { flow->strLoopStartEntryId = m_strSelectedFlowEntryId; m_bFlowDirty = true; }
	ImGui::EndDisabled();
	if (!flow->strLoopStartEntryId.empty())
	{
		ImGui::SameLine();
		if (ImGui::Button("Clear Repeat")) { flow->strLoopStartEntryId.clear(); m_bFlowDirty = true; }
	}
	if (ImGui::BeginTable("##KoukuPatternFlowRows", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY,
		ImVec2(0.f, (std::max)(140.f, ImGui::GetContentRegionAvail().y))))
	{
		ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_WidthFixed, 34.f);
		ImGui::TableSetupColumn("Kind", ImGuiTableColumnFlags_WidthFixed, 68.f);
		ImGui::TableSetupColumn("Pattern / Bundle", ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableSetupColumn("Wait after (ms)", ImGuiTableColumnFlags_WidthFixed, 115.f);
		ImGui::TableHeadersRow();
		for (std::size_t i = 0u; i < flow->Entries.size(); ++i)
		{
			auto& entry = flow->Entries[i];
			std::string error;
			const auto name = Describe_FlowEntry(entry, gateId, error);
			ImGui::PushID(entry.strEntryId.c_str());
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0); ImGui::Text("%02zu", i + 1u);
			ImGui::TableSetColumnIndex(1); ImGui::TextUnformatted(entry.strKind.c_str());
			ImGui::TableSetColumnIndex(2);
			if (ImGui::Selectable((name + (entry.strEntryId == flow->strLoopStartEntryId ? " [Repeat Start]" : "") + (error.empty() ? "" : " [Unavailable]")).c_str(), m_strSelectedFlowEntryId == entry.strEntryId))
				m_strSelectedFlowEntryId = entry.strEntryId;
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s\n%s", entry.strTargetId.c_str(), error.c_str());
			ImGui::TableSetColumnIndex(3);
			int wait = static_cast<int>(entry.iWaitAfterMs);
			ImGui::SetNextItemWidth(-1.f);
			if (ImGui::InputInt("##wait", &wait, 0, 0))
			{ entry.iWaitAfterMs = static_cast<std::uint32_t>((std::clamp)(wait, 0, 600000)); m_bFlowDirty = true; }
			ImGui::PopID();
		}
		ImGui::EndTable();
	}
}

void Client::CKoukuSaydonBossTool::Render()
{
	Open();
	ImGui::SetNextWindowSize(ImVec2(900.f, 650.f), ImGuiCond_FirstUseEver);
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
	const bool flowActive = CKoukuSaydonPatternAuditionService::Get().Get_FlowSnapshot().bActive;
	if (ImGui::Button("Reload Published Patterns"))
	{
		std::string status;
		(void)Reload(status);
	}
	ImGui::SameLine();
#ifdef _DEBUG
	if (const auto* arena = CLevel_KakulSaydonArena::Get_Active(); arena && arena->Get_ActiveDebugGate() == 8u)
	{
		ImGui::Checkbox("Reuse Gate 3 attacks on Encore", &m_bReplayGate3OnEncore);
		ImGui::TextDisabled("Select a saved Gate 3 attack and Play Isolated. Layout/mechanic sequences keep their Gate.");
	}
#endif
	const auto* selectedPattern = Find_SelectedPattern();
	const auto* selectedBundle = Find_SelectedBundle();
	const bool ready = selectedBundle ? selectedBundle->strLoadError.empty() && !selectedBundle->Members.empty() :
		selectedPattern && selectedPattern->strLoadError.empty();
	ImGui::BeginDisabled(!m_bHasSavedComposition || !ready || audition.Is_InFlight() || flowActive || Is_PlayPreparationPending());
	if (ImGui::Button(selectedBundle ? "Play Bundle" : "Play Isolated"))
		(void)Play_Selected(m_strStatus);
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::BeginDisabled(
		!m_bHasSavedComposition ||
		m_PlayAllPatternIds.empty() || audition.Is_InFlight() || flowActive || Is_PlayPreparationPending());
	if (ImGui::Button("Composition Play All"))
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
	static const char* gateLabels[] = { "1\xEA\xB4\x80\xEB\xAC\xB8", "2\xEA\xB4\x80\xEB\xAC\xB8", "3\xEA\xB4\x80\xEB\xAC\xB8", "\xEB\xB9\x99\xEA\xB3\xA0" };
	static const char* gates[] = { "GATE1", "GATE2", "GATE3", "BINGO" };
	m_iSelectedGate = (std::clamp)(m_iSelectedGate, 0, 3);
	ImGui::SetNextItemWidth(180.f);
	if (ImGui::Combo("Gate##KoukuBossTree", &m_iSelectedGate, gateLabels, 4))
	{ m_iSelectedInventoryKind = 0; m_strSelectedInventoryId.clear(); m_strSelectedFlowEntryId.clear(); }
	if (ImGui::BeginTabBar("##KoukuBossTabs"))
	{
	if (ImGui::BeginTabItem("Pattern Flow"))
	{
		Render_PatternFlowEditor(gates[m_iSelectedGate]);
		ImGui::EndTabItem();
	}
	if (ImGui::BeginTabItem("All Patterns"))
	{
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
	ImGui::EndTabItem();
	}
	ImGui::EndTabBar();
	}
	ImGui::End();
}
