#include "imgui.h"

#include "KoukuSaydonBossTool.h"

#include "DataJson.h"
#include "KoukuSaydonPatternAuditionService.h"
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

	bool Load_ProductIndex(
		std::vector<Client::CKoukuSaydonBossTool::PRODUCT_PATTERN>& outPatterns,
		std::vector<std::string>& outPlayAll,
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
		outPatterns = std::move(staged);
		outPlayAll = std::move(playOrder);
		outSourceRevision = parsedRevision;
		outStatus = "Loaded " + std::to_string(outPatterns.size()) + " Product patterns; " +
			std::to_string(errors) + " invalid patterns isolated." + (validOrder ? "" : " Play All unavailable.");
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
	std::uint32_t stagedSourceRevision = 0u;
	if (!Load_ProductIndex(
			stagedPatterns, stagedPlayAll, stagedSourceRevision, outStatus))
	{
		m_strStatus = outStatus;
		return false;
	}
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
	const auto found = std::find_if(
		m_ProductPatterns.begin(), m_ProductPatterns.end(),
		[this](const PRODUCT_PATTERN& pattern)
		{
			return pattern.strPatternId == m_strSelectedPatternId;
		});
	return found == m_ProductPatterns.end() ? nullptr : &*found;
}

void Client::CKoukuSaydonBossTool::Normalize_Selection()
{
	if (nullptr != Find_SelectedPattern())
		return;
	m_strSelectedPatternId.clear();
	if (!m_ProductPatterns.empty())
		m_strSelectedPatternId = m_ProductPatterns.front().strPatternId;
}

bool Client::CKoukuSaydonBossTool::Play_Selected(std::string& outStatus)
{
	const PRODUCT_PATTERN* const pattern =
		Find_SelectedPattern();
	if (nullptr == pattern || !pattern->strLoadError.empty())
	{
		outStatus = "Select one valid saved PRODUCT pattern first.";
		return false;
	}
	const auto& revision =
		CNetworkManager::Get().Get_GameplayRevisionState().ServerActiveRevision;
	return CKoukuSaydonPatternAuditionService::Get().Play_Selected(
		pattern->strPatternId, revision, m_iSourceRevision, outStatus);
}

bool Client::CKoukuSaydonBossTool::Play_All(std::string& outStatus)
{
	if (!m_bHasSavedComposition ||
		m_PlayAllPatternIds.empty())
	{
		outStatus = "The saved composition has no PRODUCT Play All order.";
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
			"Saved authoring and generated KoukuSaydon Product revisions differ; Publish before Server Play.";
		m_strStatus = outStatus;
		return false;
	}
	const auto found = std::find_if(
		m_ProductPatterns.begin(), m_ProductPatterns.end(),
		[patternId](const PRODUCT_PATTERN& pattern)
		{
			return pattern.strPatternId == patternId;
		});
	if (found == m_ProductPatterns.end())
	{
		outStatus =
			"The selected Workbench Pattern is not in the saved PRODUCT inventory.";
		m_strStatus = outStatus;
		return false;
	}
	m_strSelectedPatternId = found->strPatternId;
	const bool played = Play_Selected(outStatus);
	m_strStatus = outStatus;
	return played;
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
	if (ImGui::Button("Reload Saved Product"))
	{
		std::string status;
		(void)Reload(status);
	}
	ImGui::SameLine();
	ImGui::BeginDisabled(
		!m_bHasSavedComposition || nullptr == Find_SelectedPattern() ||
		!Find_SelectedPattern()->strLoadError.empty() ||
		audition.Is_InFlight());
	if (ImGui::Button("Play Isolated"))
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
				"Server playback is active for a different local Product source revision; reload/restart before trusting Live markers.");
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
		ImGui::TableSetupColumn("PRODUCT Patterns", ImGuiTableColumnFlags_WidthFixed,
			300.f);
		ImGui::TableSetupColumn("Server Playback", ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		for (const PRODUCT_PATTERN& pattern : m_ProductPatterns)
		{
			ImGui::PushID(pattern.strPatternId.c_str());
			const bool selected =
				pattern.strPatternId == m_strSelectedPatternId;
			const std::string label = pattern.strDisplayName + (pattern.strLoadError.empty() ? "" : " [Error]");
			if (ImGui::Selectable(label.c_str(), selected))
				m_strSelectedPatternId = pattern.strPatternId;
			if (audition.Is_Live(pattern.strPatternId, m_iSourceRevision))
			{
				ImGui::SameLine();
				ImGui::TextColored(LIVE_COLOR, "[Live]");
			}
			ImGui::TextDisabled("%s", pattern.strPatternId.c_str());
			ImGui::PopID();
		}

		ImGui::TableSetColumnIndex(1);
		const PRODUCT_PATTERN* const selected =
			Find_SelectedPattern();
		if (nullptr == selected)
		{
			ImGui::TextDisabled("Select one PRODUCT pattern.");
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
