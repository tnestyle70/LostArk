#include "imgui.h"
#include "Animation_Tool_Internal.h"
#include "BalanceTool.h"
#include "ValtanBossTool.h"
#include "AnimationTargetService.h"
#include "Character.h"
#include "DataJson.h"
#include "Effect_RuntimeAuthority.h"
#include "MainApp.h"
#include "Model.h"
#include "ProjectDataRoot.h"
#include "SoundCueCatalog.h"
#include <charconv>
#include <algorithm>
#include <array>
#include <cerrno>
#include <cfloat>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <io.h>
#include <iomanip>
#include <iterator>
#include <limits>
#include <map>
#include <sstream>
#include <span>
#include <system_error>
#include <tuple>
#include <unordered_map>
#include <unordered_set>




void Client::CAnimation_Tool::
Invalidate_ValtanPatternCreateExactSourceSelection()
{
	m_bValtanPatternCreateExactSourceSelection = false;
	m_iValtanPatternCreateSourceActionId = -1;
	m_iValtanPatternCreateSourceSequenceIndex = -1;
	m_strValtanPatternCreateValidatedRequestSha256.clear();
}

std::filesystem::path Client::CAnimation_Tool::Get_CustomChainFilePath() const
{
	const CUSTOM_CHAIN_PROFILE* pProfile =
		Find_CustomChainProfile(m_AssetName);
	if (nullptr == pProfile)
		return std::filesystem::path{};
	return CProjectDataRoot::Resolve(
		std::filesystem::path(L"Valtan") / pProfile->pFileName);
}

bool_t Client::CAnimation_Tool::Read_CustomChainDocument(
	const std::filesystem::path& source,
	std::vector<CUSTOM_CHAIN_ENTRY>& Out,
	std::string& strOutError) const
{
	FILE* file = nullptr;
	if (0 != _wfopen_s(&file, source.c_str(), L"rb") || nullptr == file)
	{
		strOutError = "could not open " + source.filename().string();
		return false;
	}
	std::string text;
	char_t buffer[4096]{};
	size_t read = 0u;
	while (0u < (read = fread(buffer, 1u, sizeof(buffer), file)))
		text.append(buffer, read);
	fclose(file);
	return Parse_CustomChainDocument(text, Out, strOutError);
}

bool_t Client::CAnimation_Tool::Parse_CustomChainDocument(
	const std::string& text,
	std::vector<CUSTOM_CHAIN_ENTRY>& Out,
	std::string& strOutError) const
{
	DATA_JSON_VALUE root;
	std::string error;
	if (!CDataJson::Parse(text, root, error) || !root.Is_Object())
	{
		strOutError = "document is unreadable: " + error;
		return false;
	}
	const DATA_JSON_VALUE* pSchema = root.Find("schema");
	const DATA_JSON_VALUE* pVersion = root.Find("formatVersion");
	const DATA_JSON_VALUE* pBoss = root.Find("bossArchetypeId");
	const DATA_JSON_VALUE* pEncounter = root.Find("encounterId");
	const DATA_JSON_VALUE* pChains = root.Find("chains");
	if (5u != root.Get_Object().size() ||
		nullptr == pSchema || !pSchema->Is_String() ||
		pSchema->Get_String() != "lostark.valtan-pattern-presentation-debug" ||
		nullptr == pVersion || !pVersion->Is_Number() ||
		1.0 != pVersion->Get_Number() ||
		nullptr == pBoss || !pBoss->Is_String() ||
		pBoss->Get_String() != "BOSS_VALTAN" ||
		nullptr == pEncounter || !pEncounter->Is_String() ||
		pEncounter->Get_String() != "ENCOUNTER_VALTAN" ||
		nullptr == pChains || !pChains->Is_Array())
	{
		strOutError = "document header/version/owner or chain array is invalid";
		return false;
	}

	/* Every row is strict and the result is staged first.  Skipping one broken
	   chain/occurrence would turn a malformed owner into a valid-looking partial
	   library and a later Save would permanently erase the skipped data. */
	std::vector<CUSTOM_CHAIN_ENTRY> staged;
	std::unordered_set<std::string> chainIds;
	std::unordered_set<std::string> occurrenceIds;
	for (size_t iChain = 0u; iChain < pChains->Get_Array().size(); ++iChain)
	{
		const DATA_JSON_VALUE& Chain = pChains->Get_Array()[iChain];
		if (!Chain.Is_Object() || 4u != Chain.Get_Object().size())
		{
			strOutError = "chain[" + std::to_string(iChain) +
				"] is not an exact object";
			return false;
		}
		const DATA_JSON_VALUE* pId = Chain.Find("chainId");
		const DATA_JSON_VALUE* pPattern = Chain.Find("targetPatternId");
		const DATA_JSON_VALUE* pStage = Chain.Find("targetStageId");
		const DATA_JSON_VALUE* pAnimation = Chain.Find("animation");
		if (nullptr == pId || !pId->Is_String() || pId->Get_String().empty() ||
			!chainIds.insert(pId->Get_String()).second ||
			nullptr == pPattern || !pPattern->Is_String() ||
			nullptr == pStage || !pStage->Is_String() ||
			nullptr == pAnimation || !pAnimation->Is_Object() ||
			3u != pAnimation->Get_Object().size())
		{
			strOutError = "chain[" + std::to_string(iChain) +
				"] has an invalid/duplicate identity, target, or animation";
			return false;
		}
		const DATA_JSON_VALUE* pEndPolicy = pAnimation->Find("endPolicy");
		const DATA_JSON_VALUE* pRepeatCount = pAnimation->Find("repeatCount");
		const DATA_JSON_VALUE* pOccurrences = pAnimation->Find("occurrences");
		if (nullptr == pEndPolicy || !pEndPolicy->Is_String() ||
			(pEndPolicy->Get_String() != "EXACT" &&
			 pEndPolicy->Get_String() != "NATIVE_CLIP_LENGTHS") ||
			nullptr == pRepeatCount || !pRepeatCount->Is_Number() ||
			!std::isfinite(pRepeatCount->Get_Number()) ||
			std::floor(pRepeatCount->Get_Number()) != pRepeatCount->Get_Number() ||
			nullptr == pOccurrences || !pOccurrences->Is_Array() ||
			pOccurrences->Get_Array().empty() ||
			pOccurrences->Get_Array().size() > 64u ||
			pRepeatCount->Get_Number() !=
				static_cast<double>(pOccurrences->Get_Array().size()))
		{
			strOutError = "chain[" + std::to_string(iChain) +
				"] has an invalid end policy, repeat count, or occurrence array";
			return false;
		}

		CUSTOM_CHAIN_ENTRY Entry;
		Entry.chainId = pId->Get_String();
		Entry.targetPatternId = pPattern->Get_String();
		Entry.targetStageId = pStage->Get_String();
		for (size_t iOccurrence = 0u;
			iOccurrence < pOccurrences->Get_Array().size(); ++iOccurrence)
		{
			const DATA_JSON_VALUE& Occurrence =
				pOccurrences->Get_Array()[iOccurrence];
			if (!Occurrence.Is_Object() || 7u != Occurrence.Get_Object().size())
			{
				strOutError = "chain[" + std::to_string(iChain) +
					"].occurrence[" + std::to_string(iOccurrence) +
					"] is not an exact object";
				return false;
			}
			const DATA_JSON_VALUE* pOccurrenceId =
				Occurrence.Find("clipOccurrenceId");
			const DATA_JSON_VALUE* pClip = Occurrence.Find("clip");
			const DATA_JSON_VALUE* pMappingBasis =
				Occurrence.Find("mappingBasis");
			const DATA_JSON_VALUE* pSourceStart =
				Occurrence.Find("sourceStartMs");
			const DATA_JSON_VALUE* pPlayMs = Occurrence.Find("playMs");
			const DATA_JSON_VALUE* pPlayRate = Occurrence.Find("playRate");
			const DATA_JSON_VALUE* pRepeatUntilStageEnd =
				Occurrence.Find("repeatUntilStageEnd");
			if (nullptr == pOccurrenceId || !pOccurrenceId->Is_String() ||
				pOccurrenceId->Get_String().empty() ||
				!occurrenceIds.insert(pOccurrenceId->Get_String()).second ||
				nullptr == pClip || !pClip->Is_String() ||
				pClip->Get_String().empty() ||
				nullptr == pMappingBasis || !pMappingBasis->Is_String() ||
				pMappingBasis->Get_String() != "PROJECT_AUTHORED" ||
				nullptr == pSourceStart || !pSourceStart->Is_Number() ||
				!std::isfinite(pSourceStart->Get_Number()) ||
				std::floor(pSourceStart->Get_Number()) != pSourceStart->Get_Number() ||
				pSourceStart->Get_Number() < 0.0 ||
				nullptr == pPlayMs || !pPlayMs->Is_Number() ||
				!std::isfinite(pPlayMs->Get_Number()) ||
				std::floor(pPlayMs->Get_Number()) != pPlayMs->Get_Number() ||
				pPlayMs->Get_Number() < 0.0 ||
				pPlayMs->Get_Number() > static_cast<double>(
					(std::numeric_limits<int32_t>::max)()) ||
				nullptr == pPlayRate || !pPlayRate->Is_Number() ||
				!std::isfinite(pPlayRate->Get_Number()) ||
				pPlayRate->Get_Number() <= 0.0 ||
				nullptr == pRepeatUntilStageEnd ||
				!pRepeatUntilStageEnd->Is_Boolean())
			{
				strOutError = "chain[" + std::to_string(iChain) +
					"].occurrence[" + std::to_string(iOccurrence) +
					"] has invalid identity, clip, timing, rate, or loop fields";
				return false;
			}
			CUSTOM_CHAIN_STEP Step;
			Step.clipName = pClip->Get_String();
			Step.fDurationSeconds =
				static_cast<f32_t>(pPlayMs->Get_Number()) * 0.001f;
			Entry.steps.push_back(std::move(Step));
		}
		staged.push_back(std::move(Entry));
	}

	Out = std::move(staged);
	strOutError.clear();
	return true;
}

bool_t Client::CAnimation_Tool::Load_CustomChainLibrary()
{
	const std::filesystem::path source = Get_CustomChainFilePath();
	if (source.empty())
	{
		m_CustomChainLibrary.clear();
		m_strCustomChainStatus =
			"Custom chains are not admitted for this target.";
		return false;
	}
	std::error_code existsError;
	if (!std::filesystem::exists(source, existsError) || existsError)
	{
		/* Nothing saved yet is the normal first state, not a failure. */
		m_CustomChainLibrary.clear();
		m_strCustomChainStatus.clear();
		return true;
	}

	std::vector<CUSTOM_CHAIN_ENTRY> staged;
	std::string error;
	if (!Read_CustomChainDocument(source, staged, error))
	{
		m_strCustomChainStatus =
			"Debug chain file rejected; saved chains preserved: " + error;
		return false;
	}

	m_CustomChainLibrary = std::move(staged);
	m_strCustomChainStatus = "Loaded " +
		std::to_string(m_CustomChainLibrary.size()) + " saved chains.";
	return true;
}

bool_t Client::CAnimation_Tool::Save_CustomChainLibrary()
{
	const std::filesystem::path destination = Get_CustomChainFilePath();
	const CUSTOM_CHAIN_PROFILE* pProfile =
		Find_CustomChainProfile(m_AssetName);
	if (destination.empty() || nullptr == pProfile)
	{
		m_strCustomChainStatus =
			"Save rejected: custom chains are not admitted for this target.";
		return false;
	}
	SCOPED_VALTAN_PATTERN_TRANSACTION_LOCK TransactionLock;
	std::string strLockError;
	if (!TransactionLock.Try_Acquire(
			CProjectDataRoot::Get().parent_path(), strLockError))
	{
		m_strCustomChainStatus =
			"Save rejected before mutation: " + strLockError +
			". The in-memory chain and previous source bytes were preserved.";
		return false;
	}
	std::error_code directoryError;
	std::filesystem::create_directories(
		destination.parent_path(), directoryError);
	if (directoryError)
	{
		m_strCustomChainStatus =
			"Save failed to create the Data directory: " +
			directoryError.message();
		return false;
	}

	std::filesystem::path temporary = destination;
	temporary += L".tmp";
	std::error_code removeError;
	std::filesystem::remove(temporary, removeError);

	FILE* file = nullptr;
	if (0 != _wfopen_s(&file, temporary.c_str(), L"wb") || nullptr == file)
	{
		m_strCustomChainStatus =
			"Save failed to open a temporary file; previous chains preserved.";
		return false;
	}

	bool_t bWritten = 0 <= fprintf(file,
		"{\n"
		"  \"schema\": \"lostark.valtan-pattern-presentation-debug\",\n"
		"  \"formatVersion\": 1,\n"
		"  \"bossArchetypeId\": \"BOSS_VALTAN\",\n"
		"  \"encounterId\": \"ENCOUNTER_VALTAN\",\n"
		"  \"chains\": [\n");
	for (size_t iChain = 0u; iChain < m_CustomChainLibrary.size(); ++iChain)
	{
		const CUSTOM_CHAIN_ENTRY& Entry = m_CustomChainLibrary[iChain];
		/* A step left at zero has no authored wall, so the chain cannot claim
		   it fills a Server stage exactly. The distinction is recorded rather
		   than guessed away, because merging it is a decision, not a rename. */
		bool_t bEveryStepAuthored = true;
		for (const CUSTOM_CHAIN_STEP& Step : Entry.steps)
		{
			if (Step.fDurationSeconds <= 0.f)
				bEveryStepAuthored = false;
		}
		bWritten = bWritten && 0 <= fprintf(file,
			"    {\n"
			"      \"chainId\": \"%s\",\n"
			"      \"targetPatternId\": \"%s\",\n"
			"      \"targetStageId\": \"%s\",\n"
			"      \"animation\": {\n"
			"        \"endPolicy\": \"%s\",\n"
			"        \"repeatCount\": %zu,\n"
			"        \"occurrences\": [\n",
			CDataJson::Escape(Entry.chainId).c_str(),
			CDataJson::Escape(Entry.targetPatternId).c_str(),
			CDataJson::Escape(Entry.targetStageId).c_str(),
			bEveryStepAuthored ? "EXACT" : "NATIVE_CLIP_LENGTHS",
			Entry.steps.size());
		for (size_t iStep = 0u; iStep < Entry.steps.size(); ++iStep)
		{
			const CUSTOM_CHAIN_STEP& Step = Entry.steps[iStep];
			const int32_t iPlayMs = static_cast<int32_t>(
				std::lround(Step.fDurationSeconds * 1000.f));
			bWritten = bWritten && 0 <= fprintf(file,
				"          {\n"
				"            \"clipOccurrenceId\": \"%s.%s.clip.%02zu\",\n"
				"            \"clip\": \"%s\",\n"
				"            \"mappingBasis\": \"PROJECT_AUTHORED\",\n"
				"            \"sourceStartMs\": 0,\n"
				"            \"playMs\": %d,\n"
				"            \"playRate\": 1.0,\n"
				"            \"repeatUntilStageEnd\": false\n"
				"          }%s\n",
				pProfile->pOccurrencePrefix,
				CDataJson::Escape(Entry.chainId).c_str(),
				iStep + 1u,
				CDataJson::Escape(Step.clipName).c_str(),
				iPlayMs < 0 ? 0 : iPlayMs,
				iStep + 1u == Entry.steps.size() ? "" : ",");
		}
		bWritten = bWritten && 0 <= fprintf(file,
			"        ]\n"
			"      }\n"
			"    }%s\n",
			iChain + 1u == m_CustomChainLibrary.size() ? "" : ",");
	}
	bWritten = bWritten && 0 <= fprintf(file, "  ]\n}\n");
	const bool_t bClosed = 0 == fclose(file);

	if (!bWritten || !bClosed)
	{
		std::error_code cleanupError;
		std::filesystem::remove(temporary, cleanupError);
		m_strCustomChainStatus =
			"Save failed while writing; previous chains preserved.";
		return false;
	}

	if (FALSE == MoveFileExW(temporary.c_str(), destination.c_str(),
		MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
	{
		const DWORD iReplaceError = GetLastError();
		std::error_code cleanupError;
		std::filesystem::remove(temporary, cleanupError);
		m_strCustomChainStatus =
			"Save failed to atomically replace the file; previous chains "
			"preserved. Win32 error " + std::to_string(iReplaceError) + ".";
		return false;
	}

	m_strCustomChainStatus = "Saved " +
		std::to_string(m_CustomChainLibrary.size()) + " chains to " +
		destination.filename().string() + ".";
	return true;
}

void Client::CAnimation_Tool::Render_ValtanPatternCreatePanel()
{
	ImGui::SeparatorText("Create New Pattern");
	ImGui::TextWrapped(
		"Promote one reviewed Animation Intake chain into a new audition-only "
		"Valtan pattern. Validate performs the full staged transaction without "
		"writing. Apply is enabled only for those exact validated request bytes.");
	ImGui::TextDisabled(
		"Defaults: MANUAL_SERVER_AUDITION / AUDITION_ONLY; hit, motion and "
		"special gameplay logic remain NONE until authored by their typed owners.");

	const bool_t bBusy = nullptr != m_hValtanPatternCreateProcess;
	const auto InvalidateValidation = [this]()
	{
		m_strValtanPatternCreateValidatedRequestSha256.clear();
	};
	ImGui::BeginDisabled(bBusy);
	const char_t* const SourceKinds[] = {
		"Current assembled chain", "Saved intake chain" };
	ImGui::SetNextItemWidth(240.f);
	if (ImGui::Combo(
		"Intake source", &m_iValtanPatternCreateSourceKind,
		SourceKinds, static_cast<int32_t>(std::size(SourceKinds))))
	{
		InvalidateValidation();
	}
	m_iValtanPatternCreateSourceKind = std::clamp(
		m_iValtanPatternCreateSourceKind, 0, 1);

	if (1 == m_iValtanPatternCreateSourceKind)
	{
		if (m_CustomChainLibrary.empty())
		{
			ImGui::TextColored(
				ImVec4(1.f, 0.6f, 0.25f, 1.f),
				"No saved intake chain is available. Save or Reload Animation Intake first.");
		}
		else
		{
			m_iValtanPatternCreateSavedIndex = std::clamp(
				m_iValtanPatternCreateSavedIndex, 0,
				static_cast<int32_t>(m_CustomChainLibrary.size() - 1u));
			const CUSTOM_CHAIN_ENTRY& Selected = m_CustomChainLibrary[
				static_cast<size_t>(m_iValtanPatternCreateSavedIndex)];
			ImGui::SetNextItemWidth(360.f);
			if (ImGui::BeginCombo(
				"Saved intake chain", Selected.chainId.c_str()))
			{
				for (size_t iChain = 0u;
					iChain < m_CustomChainLibrary.size(); ++iChain)
				{
					const bool_t bSelected = iChain == static_cast<size_t>(
						m_iValtanPatternCreateSavedIndex);
					if (ImGui::Selectable(
						m_CustomChainLibrary[iChain].chainId.c_str(), bSelected))
					{
						m_iValtanPatternCreateSavedIndex =
							static_cast<int32_t>(iChain);
						InvalidateValidation();
					}
					if (bSelected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
			ImGui::TextDisabled(
				"Selected: %s | %zu clips",
				Selected.chainId.c_str(), Selected.steps.size());
		}
	}
	else
	{
		ImGui::TextDisabled(
			"Current assembled chain: %s | %zu clips",
			'\0' == m_CustomChainId[0] ? "<name required>" :
				m_CustomChainId,
			m_CustomChainSteps.size());
	}

	ImGui::SetNextItemWidth(360.f);
	if (ImGui::InputTextWithHint(
		"Stable patternId", "VALTAN_MY_NEW_PATTERN",
		m_ValtanPatternCreatePatternId,
		sizeof(m_ValtanPatternCreatePatternId)))
	{
		InvalidateValidation();
	}
	ImGui::SetNextItemWidth(360.f);
	if (ImGui::InputTextWithHint(
		"Display name", "Korean or English authoring label",
		m_ValtanPatternCreateDisplayName,
		sizeof(m_ValtanPatternCreateDisplayName)))
	{
		InvalidateValidation();
	}
	ImGui::SetNextItemWidth(200.f);
	if (ImGui::SliderInt(
		"Authoring phase", &m_iValtanPatternCreateAuthoringPhase, 1, 3))
	{
		InvalidateValidation();
	}
	ImGui::SetNextItemWidth(300.f);
	if (ImGui::Combo(
		"Target policy", &m_iValtanPatternCreateTargetPolicy,
		VALTAN_PATTERN_CREATE_TARGET_POLICIES.data(),
		static_cast<int32_t>(VALTAN_PATTERN_CREATE_TARGET_POLICIES.size())))
	{
		InvalidateValidation();
	}
	ImGui::SetNextItemWidth(300.f);
	if (ImGui::Combo(
		"Aim policy", &m_iValtanPatternCreateAimPolicy,
		VALTAN_PATTERN_CREATE_AIM_POLICIES.data(),
		static_cast<int32_t>(VALTAN_PATTERN_CREATE_AIM_POLICIES.size())))
	{
		InvalidateValidation();
	}
	ImGui::EndDisabled();

	if ((0 == m_iValtanPatternCreateTargetPolicy) !=
		(0 == m_iValtanPatternCreateAimPolicy))
	{
		ImGui::TextColored(
			ImVec4(1.f, 0.6f, 0.25f, 1.f),
			"Target and Aim must both be NONE or both select a lock policy.");
	}
	const bool_t bBalanceDirty = nullptr == m_pBalanceTool ||
		m_pBalanceTool->Is_ValtanDraftDirty();
	const bool_t bOtherValtanOwnerDirty = Is_ValtanDocumentDirty();
	const bool_t bMutationAdmitted =
		Can_MutateValtanView(m_eValtanPatternMasterAdmission);
	if (bBalanceDirty || bOtherValtanOwnerDirty)
	{
		ImGui::TextColored(
			ImVec4(1.f, 0.6f, 0.25f, 1.f),
			"Apply blocked: save or discard every Balance / Valtan Animation-Sound owner draft first. Validate remains read-only.");
	}
	if (!bMutationAdmitted)
	{
		ImGui::TextColored(
			ImVec4(1.f, 0.6f, 0.25f, 1.f),
			"Create blocked: Pattern data is %s. Load the current data before saving.",
			ValtanPatternMasterAdmissionLabel());
	}

	ImGui::BeginDisabled(bBusy);
	if (ImGui::Button("Validate Create Request"))
		(void)Start_ValtanPatternCreateCommand(false);
	ImGui::SameLine();
	ImGui::BeginDisabled(
		m_strValtanPatternCreateValidatedRequestSha256.empty() ||
		bBalanceDirty || bOtherValtanOwnerDirty || !bMutationAdmitted);
	if (ImGui::Button("Apply Create Pattern"))
		(void)Start_ValtanPatternCreateCommand(true);
	ImGui::EndDisabled();
	ImGui::EndDisabled();

	if (!m_strValtanPatternCreateValidatedRequestSha256.empty())
	{
		ImGui::TextColored(
			ImVec4(0.35f, 0.9f, 0.45f, 1.f),
			"VALIDATED request SHA-256: %s",
			m_strValtanPatternCreateValidatedRequestSha256.c_str());
	}
	if (!m_strValtanPatternCreateStatus.empty())
		ImGui::TextWrapped("%s", m_strValtanPatternCreateStatus.c_str());
	if (!m_ValtanPatternCreateRequestPath.empty())
	{
		ImGui::TextDisabled(
			"Request file (preserved): %s",
			m_ValtanPatternCreateRequestPath.string().c_str());
	}
	if (!m_ValtanPatternCreateDiagnosticPath.empty())
	{
		ImGui::TextDisabled(
			"Diagnostic file (preserved): %s",
			m_ValtanPatternCreateDiagnosticPath.string().c_str());
	}
	if (m_bValtanPatternCreateHasExitCode)
		ImGui::TextDisabled(
			"Last exit code: %u", m_iValtanPatternCreateExitCode);
	if (!m_strValtanPatternCreateDiagnostic.empty() &&
		ImGui::CollapsingHeader("Create process diagnostic"))
	{
		ImGui::TextWrapped("%s", m_strValtanPatternCreateDiagnostic.c_str());
	}
}

bool_t Client::CAnimation_Tool::Build_ValtanPatternCreateRequest(
	std::string& strOutRequest,
	std::string& strOutError) const
{
	strOutRequest.clear();
	strOutError.clear();
	const std::string strPatternId = m_ValtanPatternCreatePatternId;
	const std::string strDisplayName = m_ValtanPatternCreateDisplayName;
	if (!Is_StablePatternAuthoringId(strPatternId))
	{
		strOutError =
			"patternId must be a 1..160 character stable ID [A-Za-z0-9_.-].";
		return false;
	}
	const size_t iDisplayFirst = strDisplayName.find_first_not_of(" \t\r\n");
	const size_t iDisplayLast = strDisplayName.find_last_not_of(" \t\r\n");
	if (strDisplayName.empty() || 0u != iDisplayFirst ||
		iDisplayLast != strDisplayName.size() - 1u)
	{
		strOutError = "Display name must be non-empty and trimmed.";
		return false;
	}
	if (m_iValtanPatternCreateAuthoringPhase < 1 ||
		m_iValtanPatternCreateAuthoringPhase > 3 ||
		m_iValtanPatternCreateTargetPolicy < 0 ||
		static_cast<size_t>(m_iValtanPatternCreateTargetPolicy) >=
			VALTAN_PATTERN_CREATE_TARGET_POLICIES.size() ||
		m_iValtanPatternCreateAimPolicy < 0 ||
		static_cast<size_t>(m_iValtanPatternCreateAimPolicy) >=
			VALTAN_PATTERN_CREATE_AIM_POLICIES.size())
	{
		strOutError = "Create New Pattern policy or phase selection is invalid.";
		return false;
	}
	if ((0 == m_iValtanPatternCreateTargetPolicy) !=
		(0 == m_iValtanPatternCreateAimPolicy))
	{
		strOutError =
			"Target and Aim must both be NONE or both select a lock policy.";
		return false;
	}

	/* Create Pattern is a Valtan data transaction and must remain usable from
	   the data-only Composition Workbench before an Arena Clone/model has
	   populated m_AssetName.  Its backend owns this one fixed intake source. */
	const std::filesystem::path SourcePath = CProjectDataRoot::Resolve(
		std::filesystem::path(L"Valtan") /
		L"Valtan.presentation.debug.json");
	std::string strSourceBytes;
	if (SourcePath.empty())
	{
		strOutError =
			"Could not resolve Data/Valtan/Valtan.presentation.debug.json for Create Pattern.";
		return false;
	}
	if (!Read_BoundedFile(
		SourcePath, VALTAN_PATTERN_CREATE_MAX_DIAGNOSTIC_BYTES,
		strSourceBytes, strOutError))
	{
		strOutError = "Could not read the exact Animation Intake source: " +
			strOutError;
		return false;
	}
	const std::string strSourceSha256 =
		CEffectRuntimeAuthorityCodec::Compute_Sha256Hex(strSourceBytes);
	if (!Is_LowerSha256(strSourceSha256))
	{
		strOutError = "Could not compute the Animation Intake source SHA-256.";
		return false;
	}

	std::ostringstream Request;
	Request << R"json({
  "schema": "lostark.valtan-animation-pattern-create-request",
  "formatVersion": 1,
  "expectedSourceSha256": ")json" << strSourceSha256 << R"json(",
  "patternId": ")json" << CDataJson::Escape(strPatternId) << R"json(",
  "displayName": ")json" << CDataJson::Escape(strDisplayName) << R"json(",
  "authoringPhase": )json" << m_iValtanPatternCreateAuthoringPhase << R"json(,
  "targetPolicy": ")json" << VALTAN_PATTERN_CREATE_TARGET_POLICIES[
		static_cast<size_t>(m_iValtanPatternCreateTargetPolicy)] << R"json(",
  "aimPolicy": ")json" << VALTAN_PATTERN_CREATE_AIM_POLICIES[
		static_cast<size_t>(m_iValtanPatternCreateAimPolicy)] << R"json(",
  "intakeChain": )json";

	if (1 == m_iValtanPatternCreateSourceKind)
	{
		if (m_CustomChainLibrary.empty() ||
			m_iValtanPatternCreateSavedIndex < 0 ||
			static_cast<size_t>(m_iValtanPatternCreateSavedIndex) >=
				m_CustomChainLibrary.size())
		{
			strOutError = "Select one saved intake chain.";
			return false;
		}
		const CUSTOM_CHAIN_ENTRY& Entry = m_CustomChainLibrary[
			static_cast<size_t>(m_iValtanPatternCreateSavedIndex)];
		if (!Is_StablePatternAuthoringId(Entry.chainId))
		{
			strOutError = "The selected saved chain has an invalid stable ID.";
			return false;
		}
		Request << R"json({
    "selectionKind": "SAVED_INTAKE_CHAIN",
    "sourceChainId": ")json" << CDataJson::Escape(Entry.chainId) << R"json("
  }
})json";
	}
	else
	{
		const std::string strChainId = m_CustomChainId;
		if (!Is_StablePatternAuthoringId(strChainId))
		{
			strOutError =
				"Current assembled chain needs a stable chain name before validation.";
			return false;
		}
		if (m_CustomChainSteps.empty() || m_CustomChainSteps.size() > 64u)
		{
			strOutError = "Current assembled chain must contain 1..64 clips.";
			return false;
		}
		Request << R"json({
    "selectionKind": "CURRENT_CHAIN",
)json";
		if (m_bValtanPatternCreateExactSourceSelection)
		{
			if (m_iValtanPatternCreateSourceActionId <= 0 ||
				m_iValtanPatternCreateSourceSequenceIndex < 0 ||
				m_iValtanPatternCreateSourceSequenceIndex > 4096)
			{
				strOutError =
					"The staged exact Animation source identity is outside the canonical action/sequence range.";
				return false;
			}
			Request << R"json(    "sourceActionId": )json" <<
				m_iValtanPatternCreateSourceActionId << R"json(,
    "sourceSequenceIndex": )json" <<
				m_iValtanPatternCreateSourceSequenceIndex << R"json(,
)json";
		}
		Request << R"json(    "chain": {
      "chainId": ")json" << CDataJson::Escape(strChainId) << R"json(",
      "targetPatternId": "",
      "targetStageId": "",
      "animation": {
        "endPolicy": "NATIVE_CLIP_LENGTHS",
        "repeatCount": )json" << m_CustomChainSteps.size() << R"json(,
        "occurrences": [
)json";
		for (size_t iStep = 0u; iStep < m_CustomChainSteps.size(); ++iStep)
		{
			const CUSTOM_CHAIN_STEP& Step = m_CustomChainSteps[iStep];
			if (!Is_StablePatternAuthoringId(Step.clipName) ||
				!std::isfinite(Step.fDurationSeconds) ||
				Step.fDurationSeconds < 0.f ||
				Step.fDurationSeconds * 1000.0 >
					static_cast<double>((std::numeric_limits<int32_t>::max)()))
			{
				strOutError =
					"Current assembled chain contains an invalid clip or duration.";
				return false;
			}
			const int32_t iPlayMs = static_cast<int32_t>(
				std::lround(Step.fDurationSeconds * 1000.f));
			std::ostringstream OccurrenceId;
			OccurrenceId << "valtan.debug." << strChainId << ".clip." <<
				std::setw(2) << std::setfill('0') << iStep + 1u;
			if (!Is_StablePatternAuthoringId(OccurrenceId.str()))
			{
				strOutError =
					"The current chain name is too long for stable clip occurrence IDs.";
				return false;
			}
			Request << R"json(          {
            "clipOccurrenceId": ")json" <<
				CDataJson::Escape(OccurrenceId.str()) << R"json(",
            "clip": ")json" << CDataJson::Escape(Step.clipName) << R"json(",
            "mappingBasis": "PROJECT_AUTHORED",
            "sourceStartMs": 0,
            "playMs": )json" << iPlayMs << R"json(,
            "playRate": 1.0,
            "repeatUntilStageEnd": false
          })json" << (iStep + 1u == m_CustomChainSteps.size() ? "\n" : ",\n");
		}
		Request << R"json(        ]
      }
    }
  }
})json";
	}

	strOutRequest = Request.str();
	DATA_JSON_VALUE Verification;
	std::string strParseError;
	if (!CDataJson::Parse(strOutRequest, Verification, strParseError) ||
		!Verification.Is_Object())
	{
		strOutRequest.clear();
		strOutError =
			"The strict Create request could not self-parse: " + strParseError;
		return false;
	}
	return true;
}

bool_t Client::CAnimation_Tool::Start_ValtanPatternCreateCommand(
	const bool_t bApply)
{
	if (nullptr != m_hValtanPatternCreateProcess)
	{
		m_strValtanPatternCreateStatus =
			"A Create New Pattern Validate/Apply process is already running.";
		return false;
	}
	if (bApply && (nullptr == m_pBalanceTool ||
		m_pBalanceTool->Is_ValtanDraftDirty() || Is_ValtanDocumentDirty()))
	{
		m_strValtanPatternCreateStatus =
			"Apply rejected before mutation: save or discard every Balance / Valtan Animation-Sound owner draft first.";
		return false;
	}
	if (bApply && !Can_MutateValtanView(m_eValtanPatternMasterAdmission))
	{
		m_strValtanPatternCreateStatus =
			"Create stopped before writing: Pattern data is " +
			std::string(ValtanPatternMasterAdmissionLabel()) +
			". Load the current data before saving.";
		return false;
	}

	std::string strRequest;
	std::string strError;
	if (!Build_ValtanPatternCreateRequest(strRequest, strError))
	{
		m_strValtanPatternCreateStatus =
			"Create New Pattern request rejected: " + strError;
		return false;
	}
	const std::string strRequestSha256 =
		CEffectRuntimeAuthorityCodec::Compute_Sha256Hex(strRequest);
	if (!Is_LowerSha256(strRequestSha256))
	{
		m_strValtanPatternCreateStatus =
			"Create New Pattern request SHA-256 could not be computed.";
		return false;
	}
	if (bApply && strRequestSha256 !=
		m_strValtanPatternCreateValidatedRequestSha256)
	{
		m_strValtanPatternCreateValidatedRequestSha256.clear();
		m_strValtanPatternCreateStatus =
			"Apply rejected: the source or authoring fields changed after Validate. Validate the exact request again.";
		return false;
	}

	std::error_code Error;
	const std::filesystem::path ProjectRoot =
		std::filesystem::weakly_canonical(
			CProjectDataRoot::Get().parent_path(), Error);
	const std::filesystem::path Script = ProjectRoot / L"Tools" /
		L"ValtanPipeline" / L"promote_valtan_animation_chains.py";
	if (Error || ProjectRoot.empty() ||
		!std::filesystem::is_directory(ProjectRoot, Error) || Error ||
		!std::filesystem::is_regular_file(Script, Error) || Error)
	{
		m_strValtanPatternCreateStatus =
			"The fixed Create New Pattern backend could not be resolved; no process was started.";
		return false;
	}
	std::filesystem::path Python;
	if (!Resolve_PythonExecutable(Python, strError))
	{
		m_strValtanPatternCreateStatus = strError;
		return false;
	}
	const std::filesystem::path TemporaryRoot =
		std::filesystem::temp_directory_path(Error);
	if (Error || TemporaryRoot.empty())
	{
		m_strValtanPatternCreateStatus =
			"The Create New Pattern diagnostic directory could not be resolved.";
		return false;
	}
	if (0u == ++m_iValtanPatternCreateCommandSequence)
		++m_iValtanPatternCreateCommandSequence;
	const std::wstring strStem = L"LostArk.ValtanCreatePattern." +
		std::to_wstring(GetCurrentProcessId()) + L"." +
		std::to_wstring(GetTickCount64()) + L"." +
		std::to_wstring(m_iValtanPatternCreateCommandSequence) +
		(bApply ? L".Apply" : L".Validate");
	m_ValtanPatternCreateRequestPath =
		TemporaryRoot / (strStem + L".request.json");
	m_ValtanPatternCreateDiagnosticPath =
		TemporaryRoot / (strStem + L".diagnostic.log");

	{
		std::ofstream RequestFile(
			m_ValtanPatternCreateRequestPath,
			std::ios::binary | std::ios::out | std::ios::trunc);
		RequestFile.write(
			strRequest.data(), static_cast<std::streamsize>(strRequest.size()));
		RequestFile.flush();
		if (!RequestFile)
		{
			m_strValtanPatternCreateStatus =
				"Could not write the preserved Create New Pattern request file.";
			return false;
		}
	}

	SECURITY_ATTRIBUTES Security{};
	Security.nLength = sizeof(Security);
	Security.bInheritHandle = TRUE;
	const HANDLE Output = CreateFileW(
		m_ValtanPatternCreateDiagnosticPath.c_str(), GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_DELETE, &Security, CREATE_NEW,
		FILE_ATTRIBUTE_NORMAL, nullptr);
	if (INVALID_HANDLE_VALUE == Output)
	{
		m_strValtanPatternCreateStatus =
			"Could not create the preserved Create diagnostic file (Win32 " +
			std::to_string(GetLastError()) + ").";
		return false;
	}
	const HANDLE Input = CreateFileW(
		L"NUL", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
		&Security, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (INVALID_HANDLE_VALUE == Input)
	{
		const DWORD iError = GetLastError();
		CloseHandle(Output);
		m_strValtanPatternCreateStatus =
			"Could not create the Create process input handle (Win32 " +
			std::to_string(iError) + ").";
		return false;
	}

	/* User-authored fields live only in the JSON request.  The executable,
	   script, mode, repository and request paths below are fixed/canonical;
	   lpApplicationName pins python.exe and no shell interprets the command. */
	std::wstring Command = L"\"" + Python.wstring() + L"\" \"" +
		Script.wstring() + L"\" --repo-root \"" + ProjectRoot.wstring() +
		L"\" --mode " + (bApply ? L"Apply" : L"Validate") +
		L" --request-file \"" +
		m_ValtanPatternCreateRequestPath.wstring() + L"\"";
	std::vector<wchar_t> MutableCommand(Command.begin(), Command.end());
	MutableCommand.push_back(L'\0');
	STARTUPINFOW Startup{};
	Startup.cb = sizeof(Startup);
	Startup.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
	Startup.wShowWindow = SW_HIDE;
	Startup.hStdInput = Input;
	Startup.hStdOutput = Output;
	Startup.hStdError = Output;
	PROCESS_INFORMATION Process{};
	const BOOL bCreated = CreateProcessW(
		Python.c_str(), MutableCommand.data(), nullptr, nullptr, TRUE,
		CREATE_NO_WINDOW, nullptr, ProjectRoot.c_str(), &Startup, &Process);
	const DWORD iCreateError = bCreated ? ERROR_SUCCESS : GetLastError();
	CloseHandle(Input);
	CloseHandle(Output);
	if (!bCreated)
	{
		m_strValtanPatternCreateStatus =
			"Could not start the Create New Pattern backend (Win32 " +
			std::to_string(iCreateError) + "). Request and diagnostic files were preserved.";
		return false;
	}
	CloseHandle(Process.hThread);
	m_hValtanPatternCreateProcess = Process.hProcess;
	m_strValtanPatternCreateActiveRequestSha256 = strRequestSha256;
	m_strValtanPatternCreateActivePatternId = m_ValtanPatternCreatePatternId;
	m_bValtanPatternCreateActiveApply = bApply;
	m_bValtanPatternCreateHasExitCode = false;
	m_iValtanPatternCreateExitCode = 0u;
	m_iValtanPatternCreateStartedAtMilliseconds = GetTickCount64();
	m_strValtanPatternCreateDiagnostic.clear();
	if (!bApply)
		m_strValtanPatternCreateValidatedRequestSha256.clear();
	m_strValtanPatternCreateStatus = std::string(
		bApply ? "Apply Create Pattern" : "Validate Create Request") +
		" is running. Its request and diagnostic files are preserved.";
	return true;
}

bool_t Client::CAnimation_Tool::Parse_ValtanPatternCreateResult(
	const std::string& strDiagnostic,
	std::string& strOutError) const
{
	DATA_JSON_VALUE Root;
	if (!CDataJson::Parse(strDiagnostic, Root, strOutError) ||
		!Root.Is_Object())
	{
		strOutError = "Create backend result is not strict JSON: " + strOutError;
		return false;
	}
	constexpr std::array<std::string_view, 11u> RequiredKeys = {
		"schema", "formatVersion", "mode", "patternId", "sourceChainId",
		"admissionState", "selectionMode", "sourceSha256", "patternCount",
		"stageCount", "projectedArtifactCount" };
	if (Root.Get_Object().size() != RequiredKeys.size() ||
		!std::all_of(RequiredKeys.begin(), RequiredKeys.end(),
			[&Root](const std::string_view strKey)
			{
				return Root.Get_Object().contains(std::string(strKey));
			}))
	{
		strOutError = "Create backend result properties do not match v1.";
		return false;
	}
	const auto String = [&Root](const std::string_view strKey) ->
		const DATA_JSON_VALUE*
	{
		const DATA_JSON_VALUE* const pValue = Root.Find(strKey);
		return nullptr != pValue && pValue->Is_String() ? pValue : nullptr;
	};
	const auto UnsignedInteger = [&Root](
		const std::string_view strKey, uint64_t& iOut) -> bool_t
	{
		const DATA_JSON_VALUE* const pValue = Root.Find(strKey);
		if (nullptr == pValue || !pValue->Is_Number() ||
			pValue->Was_FloatingPointToken() ||
			!std::isfinite(pValue->Get_Number()) ||
			pValue->Get_Number() < 0.0 ||
			pValue->Get_Number() >
				static_cast<double>((std::numeric_limits<uint32_t>::max)()))
		{
			return false;
		}
		iOut = static_cast<uint64_t>(pValue->Get_Number());
		return static_cast<double>(iOut) == pValue->Get_Number();
	};
	const DATA_JSON_VALUE* const pSchema = String("schema");
	const DATA_JSON_VALUE* const pMode = String("mode");
	const DATA_JSON_VALUE* const pPatternId = String("patternId");
	const DATA_JSON_VALUE* const pSourceChainId = String("sourceChainId");
	const DATA_JSON_VALUE* const pAdmission = String("admissionState");
	const DATA_JSON_VALUE* const pSelection = String("selectionMode");
	const DATA_JSON_VALUE* const pSourceSha = String("sourceSha256");
	uint64_t iVersion = 0u;
	uint64_t iPatternCount = 0u;
	uint64_t iStageCount = 0u;
	uint64_t iProjectedCount = 0u;
	if (nullptr == pSchema || nullptr == pMode || nullptr == pPatternId ||
		nullptr == pSourceChainId || nullptr == pAdmission ||
		nullptr == pSelection || nullptr == pSourceSha ||
		!UnsignedInteger("formatVersion", iVersion) || 1u != iVersion ||
		!UnsignedInteger("patternCount", iPatternCount) ||
		!UnsignedInteger("stageCount", iStageCount) ||
		!UnsignedInteger("projectedArtifactCount", iProjectedCount) ||
		0u == iPatternCount || 0u == iStageCount || 0u == iProjectedCount ||
		pSchema->Get_String() !=
			"lostark.valtan-animation-pattern-create-result" ||
		pMode->Get_String() !=
			(m_bValtanPatternCreateActiveApply ? "Apply" : "Validate") ||
		pPatternId->Get_String() != m_strValtanPatternCreateActivePatternId ||
		!Is_StablePatternAuthoringId(pSourceChainId->Get_String()) ||
		pAdmission->Get_String() != "MANUAL_SERVER_AUDITION" ||
		pSelection->Get_String() != "AUDITION_ONLY" ||
		!Is_LowerSha256(pSourceSha->Get_String()))
	{
		strOutError =
			"Create backend result identity, mode or safe-default contract is invalid.";
		return false;
	}
	return true;
}

void Client::CAnimation_Tool::Poll_ValtanPatternCreateCommand()
{
	if (nullptr == m_hValtanPatternCreateProcess)
		return;
	const HANDLE Process = static_cast<HANDLE>(m_hValtanPatternCreateProcess);
	const DWORD iWait = WaitForSingleObject(Process, 0u);
	if (WAIT_TIMEOUT == iWait)
	{
		if (GetTickCount64() - m_iValtanPatternCreateStartedAtMilliseconds >=
			VALTAN_PATTERN_CREATE_TIMEOUT_MILLISECONDS &&
			std::string::npos == m_strValtanPatternCreateStatus.find(
				"still running after"))
		{
			m_strValtanPatternCreateStatus =
				"Create New Pattern is still running after 120 seconds. It was not terminated because the all-or-nothing transaction must finish or roll back; request and diagnostic files remain available.";
		}
		return;
	}
	if (WAIT_OBJECT_0 != iWait)
	{
		m_strValtanPatternCreateStatus =
			"Create New Pattern process observation failed (Win32 " +
			std::to_string(GetLastError()) +
			"). The child was not terminated and its files remain preserved.";
		return;
	}

	DWORD iExitCode = 1u;
	const bool_t bExitKnown = FALSE != GetExitCodeProcess(Process, &iExitCode);
	CloseHandle(Process);
	m_hValtanPatternCreateProcess = nullptr;
	m_bValtanPatternCreateHasExitCode = bExitKnown;
	m_iValtanPatternCreateExitCode = iExitCode;
	std::string strReadError;
	if (!Read_BoundedFile(
			m_ValtanPatternCreateDiagnosticPath,
			VALTAN_PATTERN_CREATE_MAX_DIAGNOSTIC_BYTES,
			m_strValtanPatternCreateDiagnostic, strReadError))
	{
		m_strValtanPatternCreateStatus =
			"Create New Pattern exited, but its bounded diagnostic could not be read: " +
			strReadError + ". Files remain preserved.";
		return;
	}
	if (!bExitKnown)
	{
		m_strValtanPatternCreateStatus =
			"Create New Pattern exited without a readable exit code. No success is assumed; inspect the preserved diagnostic.";
		return;
	}
	if (0u != iExitCode)
	{
		if (!m_bValtanPatternCreateActiveApply)
			m_strValtanPatternCreateValidatedRequestSha256.clear();
		m_strValtanPatternCreateStatus = std::string(
			m_bValtanPatternCreateActiveApply ?
				"Apply Create Pattern" : "Validate Create Request") +
			" failed with exit code " + std::to_string(iExitCode) +
			". No success is assumed; inspect the preserved diagnostic.";
		return;
	}

	std::string strParseError;
	if (!Parse_ValtanPatternCreateResult(
			m_strValtanPatternCreateDiagnostic, strParseError))
	{
		if (!m_bValtanPatternCreateActiveApply)
			m_strValtanPatternCreateValidatedRequestSha256.clear();
		m_strValtanPatternCreateStatus =
			"Create New Pattern returned exit code 0 but its typed result was rejected: " +
			strParseError + ". No success is assumed.";
		return;
	}
	if (!m_bValtanPatternCreateActiveApply)
	{
		m_strValtanPatternCreateValidatedRequestSha256 =
			m_strValtanPatternCreateActiveRequestSha256;
		m_strValtanPatternCreateStatus =
			"Create New Pattern Validate passed. Apply is unlocked only for this exact request SHA-256.";
		return;
	}

	m_strValtanPatternCreateValidatedRequestSha256.clear();
	std::string strBalanceReloadStatus =
		"Balance source reload is unavailable.";
	const bool_t bBalanceReloaded = nullptr != m_pBalanceTool &&
		m_pBalanceTool->Reload_ValtanSource(strBalanceReloadStatus);
	m_bCustomChainLibraryLoadAttempted = true;
	const bool_t bIntakeReloaded = bBalanceReloaded && Load_CustomChainLibrary();
	const std::string strIntakeStatus = bBalanceReloaded ?
		m_strCustomChainStatus :
		"Intake reload was not attempted because joined source/Product admission failed.";
	const bool_t bAnimationReloaded = bBalanceReloaded &&
		Reload_ValtanPatternMaster();
	const std::string strAnimationStatus = bBalanceReloaded ?
		m_strValtanPatternMasterStatus :
		"Anim joined-master reload was not attempted because joined source/Product admission failed.";
	std::string strBossStatus =
		bAnimationReloaded ? "Boss canonical graph reload is unavailable." :
			"Boss canonical graph reload was not attempted because Anim reload failed.";
	const bool_t bBossReloaded = bAnimationReloaded &&
		nullptr != m_pValtanBossTool &&
		m_pValtanBossTool->Reload_CanonicalGraph(strBossStatus);
	const bool_t bAnimationAdmitted = bAnimationReloaded &&
		Can_MutateValtanView(m_eValtanPatternMasterAdmission);
	bool_t bSelected = false;
	if (bAnimationAdmitted)
	{
		const std::vector<const VALTAN_PATTERN_VIEW*> Patterns =
			Collect_ValtanPatternMasterPatterns();
		for (size_t iPattern = 0u; iPattern < Patterns.size(); ++iPattern)
		{
			const VALTAN_PATTERN_VIEW* const pPattern = Patterns[iPattern];
			if (nullptr == pPattern || pPattern->strPatternId !=
				m_strValtanPatternCreateActivePatternId)
			{
				continue;
			}
			m_iValtanPatternMasterSelected = static_cast<int32_t>(iPattern);
			m_strValtanWorkbenchPatternId = pPattern->strPatternId;
			m_strValtanWorkbenchStageId = pPattern->Stages.empty() ?
				std::string{} : pPattern->Stages.front().strStageId;
			m_eValtanWorkbenchSelection =
				VALTAN_WORKBENCH_SELECTION_KIND::PATTERN;
			m_eValtanWorkbenchDetailOwner =
				VALTAN_WORKBENCH_DETAIL_OWNER::ANIMATION;
			m_bValtanWorkbenchFocusDetailRequested = true;
			bSelected = true;
			break;
		}
	}
#ifdef _DEBUG
	if (bSelected)
	{
		if (CMainApp* const pApp = CMainApp::Get_Active())
			(void)pApp->Debug_SelectCompletePlayPattern(
				m_strValtanPatternCreateActivePatternId);
	}
#endif
	const bool_t bReloadClosureAdmitted =
		bBalanceReloaded && bIntakeReloaded &&
		bAnimationAdmitted && bBossReloaded && bSelected;
	if (bReloadClosureAdmitted)
	{
		m_strValtanCompositionPatternCreatedId =
			m_strValtanPatternCreateActivePatternId;
		m_bValtanCompositionPatternCreatedPending = true;
	}
	m_strValtanPatternCreateStatus =
		std::string(bReloadClosureAdmitted ?
			"Apply completed the Data source/Product transaction and local canonical reload for " :
			"Apply committed the Data source transaction, but the local Product/canonical reload is INCOMPLETE for ") +
		m_strValtanPatternCreateActivePatternId + ". Balance source reload: " +
		(bBalanceReloaded ? "PASS" : "REJECTED") + " (" +
		strBalanceReloadStatus +
		"). Apply transaction source/Product closure: PASS (validated before exit 0). Intake reload: " +
		(bIntakeReloaded ? "PASS" : "REJECTED") + " (" + strIntakeStatus +
		"). Anim joined-master admission/selection: " +
		(bAnimationAdmitted && bSelected ? "PASS" : "REJECTED") + " (" +
		strAnimationStatus + "). Boss canonical graph/inventory reload: " +
		(bBossReloaded ? "PASS" : "REJECTED") + " (" + strBossStatus + ").";
	if (bReloadClosureAdmitted)
	{
		m_strValtanPatternCreateStatus +=
			" Data Pattern creation is complete. The Create transaction does not republish Gameplay.bootstrap or restart the active Server; run the Server + Client Product build and restart Server before product entry/playback.";
	}
}

void Client::CAnimation_Tool::Render_ValtanCustomChainWindow(
	const shared_ptr<Engine::CModel>& pModel)
{
	const CUSTOM_CHAIN_PROFILE* pProfile =
		Find_CustomChainProfile(m_AssetName);
	if (nullptr == pProfile)
	{
		m_bShowValtanCustomChainWindow = false;
		return;
	}

	if (!ImGui::Begin(
		pProfile->pWindowTitle, &m_bShowValtanCustomChainWindow))
	{
		ImGui::End();
		return;
	}

	ImGui::TextWrapped(
		"Assemble a chain by hand out of the model's own clips. Seconds carry "
		"the same meaning the source cuts do: 0 plays the clip's native length, "
		"a shorter value cuts it, a longer one loops it until the step is "
		"filled. Save Animation Intake writes the review source only; it does "
		"not silently replace Server gameplay or Product presentation.");

	const bool_t bBusy =
		m_bValtanPatternPreviewPlaying || m_bValtanPatternMasterPlaying ||
		nullptr != m_hValtanPatternCreateProcess;

	ImGui::BeginDisabled(bBusy || m_CustomChainSteps.empty());
	if (ImGui::Button("Play Chain"))
	{
		/* The transport that owns step, progress, pause and speed already
		   lives in the source reference window, so playback opens it. */
		if (Start_ValtanCustomChainPreview(pModel))
			m_bShowValtanSourceReferenceWindow = true;
	}
	ImGui::SameLine();
	if (ImGui::Button("Clear Steps"))
	{
		m_CustomChainSteps.clear();
		Invalidate_ValtanPatternCreateExactSourceSelection();
	}
	ImGui::EndDisabled();

	ImGui::SetNextItemWidth(160.f);
	ImGui::SliderFloat(
		"Blend##customchainblend", &m_fPreviewBlendSeconds, 0.f, 0.5f, "%.3f s");
	if (ImGui::IsItemHovered())
	{
		ImGui::SetTooltip(
			"Preview-only cross-fade when a step changes clip. 0 matches the "
			"product Valtan, which does not blend yet; 0.12 matches the value "
			"CCharacter already uses.");
	}

	if (!m_bCustomChainLibraryLoadAttempted)
	{
		m_bCustomChainLibraryLoadAttempted = true;
		Load_CustomChainLibrary();
	}

	ImGui::SeparatorText("Saved Animation Intake");
	ImGui::TextDisabled("%s", pProfile->pFileLabel);
	ImGui::BeginDisabled(bBusy);
	ImGui::SetNextItemWidth(160.f);
	if (ImGui::InputTextWithHint("##chainid", "chain name",
		m_CustomChainId, sizeof(m_CustomChainId)))
	{
		Invalidate_ValtanPatternCreateExactSourceSelection();
	}
	ImGui::SameLine();
	ImGui::SetNextItemWidth(160.f);
	ImGui::InputTextWithHint("##chaintargetpattern", "target patternId",
		m_CustomChainTargetPatternId, sizeof(m_CustomChainTargetPatternId));
	ImGui::SameLine();
	ImGui::SetNextItemWidth(120.f);
	ImGui::InputTextWithHint("##chaintargetstage", "target stageId",
		m_CustomChainTargetStageId, sizeof(m_CustomChainTargetStageId));

	if (ImGui::Button("Save Animation Intake"))
	{
		if ('\0' == m_CustomChainId[0])
		{
			m_strCustomChainStatus = "Save rejected: name the chain first.";
		}
		else if (m_CustomChainSteps.empty())
		{
			m_strCustomChainStatus = "Save rejected: the chain owns no step.";
		}
		else
		{
			CUSTOM_CHAIN_ENTRY Entry;
			Entry.chainId = m_CustomChainId;
			Entry.targetPatternId = m_CustomChainTargetPatternId;
			Entry.targetStageId = m_CustomChainTargetStageId;
			Entry.steps = m_CustomChainSteps;
			/* Same name replaces in place so re-saving a chain being tuned
			   does not grow a pile of near-identical entries. */
			const std::vector<CUSTOM_CHAIN_ENTRY> Previous =
				m_CustomChainLibrary;
			bool_t bReplaced = false;
			for (CUSTOM_CHAIN_ENTRY& Existing : m_CustomChainLibrary)
			{
				if (Existing.chainId != Entry.chainId)
					continue;
				Existing = Entry;
				bReplaced = true;
				break;
			}
			if (!bReplaced)
				m_CustomChainLibrary.push_back(std::move(Entry));
			if (!Save_CustomChainLibrary())
				m_CustomChainLibrary = Previous;
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("Reload File"))
	{
		Load_CustomChainLibrary();
	}
	ImGui::EndDisabled();

	if (!m_strCustomChainStatus.empty())
		ImGui::TextWrapped("%s", m_strCustomChainStatus.c_str());

	if (!m_CustomChainLibrary.empty())
	{
		size_t iDeleteEntry = m_CustomChainLibrary.size();
		ImGui::BeginDisabled(bBusy);
		for (size_t iEntry = 0u; iEntry < m_CustomChainLibrary.size(); ++iEntry)
		{
			const CUSTOM_CHAIN_ENTRY& Entry = m_CustomChainLibrary[iEntry];
			ImGui::PushID(static_cast<int32_t>(iEntry) + 20000);
			if (ImGui::SmallButton("Load"))
			{
				m_CustomChainSteps = Entry.steps;
				Invalidate_ValtanPatternCreateExactSourceSelection();
				snprintf(m_CustomChainId, sizeof(m_CustomChainId), "%s",
					Entry.chainId.c_str());
				snprintf(m_CustomChainTargetPatternId,
					sizeof(m_CustomChainTargetPatternId), "%s",
					Entry.targetPatternId.c_str());
				snprintf(m_CustomChainTargetStageId,
					sizeof(m_CustomChainTargetStageId), "%s",
					Entry.targetStageId.c_str());
				m_strCustomChainStatus =
					"Loaded chain " + Entry.chainId + " into the steps.";
			}
			ImGui::SameLine();
			if (ImGui::SmallButton("Delete"))
				iDeleteEntry = iEntry;
			ImGui::SameLine();
			ImGui::Text("%s  |  %s / %s  |  %zu steps",
				Entry.chainId.c_str(),
				Entry.targetPatternId.empty() ? "-" :
					Entry.targetPatternId.c_str(),
				Entry.targetStageId.empty() ? "-" :
					Entry.targetStageId.c_str(),
				Entry.steps.size());
			ImGui::PopID();
		}
		ImGui::EndDisabled();
		if (iDeleteEntry < m_CustomChainLibrary.size())
		{
			const std::vector<CUSTOM_CHAIN_ENTRY> Previous =
				m_CustomChainLibrary;
			m_CustomChainLibrary.erase(
				m_CustomChainLibrary.begin() +
				static_cast<std::ptrdiff_t>(iDeleteEntry));
			if (!Save_CustomChainLibrary())
				m_CustomChainLibrary = Previous;
		}
	}

	Render_ValtanPatternCreatePanel();

	ImGui::SeparatorText("Steps");
	if (m_CustomChainSteps.empty())
	{
		ImGui::TextDisabled("No step yet. Add clips from the list below.");
	}
	else
	{
		f32_t fTotalSeconds = 0.f;
		size_t iRemove = m_CustomChainSteps.size();
		size_t iMoveUp = m_CustomChainSteps.size();
		size_t iMoveDown = m_CustomChainSteps.size();
		ImGui::BeginDisabled(bBusy);
		for (size_t iStep = 0u; iStep < m_CustomChainSteps.size(); ++iStep)
		{
			CUSTOM_CHAIN_STEP& Step = m_CustomChainSteps[iStep];
			ImGui::PushID(static_cast<int32_t>(iStep));
			ImGui::Text("%2zu", iStep + 1u);
			ImGui::SameLine();
			ImGui::SetNextItemWidth(80.f);
			if (ImGui::InputFloat(
					"##seconds", &Step.fDurationSeconds, 0.f, 0.f, "%.3f"))
			{
				Invalidate_ValtanPatternCreateExactSourceSelection();
			}
			if (!std::isfinite(Step.fDurationSeconds) ||
				Step.fDurationSeconds < 0.f)
			{
				Step.fDurationSeconds = 0.f;
			}
			ImGui::SameLine();
			if (ImGui::SmallButton("^"))
				iMoveUp = iStep;
			ImGui::SameLine();
			if (ImGui::SmallButton("v"))
				iMoveDown = iStep;
			ImGui::SameLine();
			if (ImGui::SmallButton("x"))
				iRemove = iStep;
			ImGui::SameLine();
			ImGui::TextUnformatted(Step.clipName.c_str());
			fTotalSeconds += Step.fDurationSeconds;
			ImGui::PopID();
		}
		ImGui::EndDisabled();
		ImGui::Text(
			"%zu steps, %.3f s of authored length (steps left at 0 add their "
			"native length on top).",
			m_CustomChainSteps.size(), fTotalSeconds);

		/* The list is edited after the row loop so the vector never moves while
		   ImGui is still drawing from it. */
		if (iRemove < m_CustomChainSteps.size())
		{
			m_CustomChainSteps.erase(
				m_CustomChainSteps.begin() +
					static_cast<std::ptrdiff_t>(iRemove));
			Invalidate_ValtanPatternCreateExactSourceSelection();
		}
		else if (iMoveUp > 0u && iMoveUp < m_CustomChainSteps.size())
		{
			std::swap(
				m_CustomChainSteps[iMoveUp - 1u], m_CustomChainSteps[iMoveUp]);
			Invalidate_ValtanPatternCreateExactSourceSelection();
		}
		else if (iMoveDown + 1u < m_CustomChainSteps.size())
		{
			std::swap(
				m_CustomChainSteps[iMoveDown], m_CustomChainSteps[iMoveDown + 1u]);
			Invalidate_ValtanPatternCreateExactSourceSelection();
		}
	}

	ImGui::SeparatorText("Clips");
	ImGui::TextDisabled(
		"Only %s* clips are listed: both Valtan bodies share that vocabulary, "
		"so a chain authored here plays on either.",
		CUSTOM_CHAIN_CLIP_PREFIX);
	ImGui::SetNextItemWidth(-1.f);
	ImGui::InputTextWithHint(
		"##customchainfilter",
		"filter by clip name",
		m_CustomChainFilter,
		sizeof(m_CustomChainFilter));

	ImGui::BeginDisabled(bBusy);
	if (ImGui::BeginChild(
		"##customchainclips",
		ImVec2(0.f, 260.f),
		ImGuiChildFlags_Borders,
		ImGuiWindowFlags_NoScrollWithMouse))
	{
		for (uint32_t iAnimation = 0u;
			iAnimation < pModel->Get_NumAnimations(); ++iAnimation)
		{
			const char_t* pName = pModel->Get_AnimationName(iAnimation);
			if (nullptr == pName ||
				!Contains_NoCase(pName, m_CustomChainFilter))
			{
				continue;
			}
			/* The ghost body carries its own rpbf_02.ao_* clips on top of the
			   shared vocabulary. They play, but the product body has no clip of
			   that name, so authoring one into this shared document would break
			   the promotion that pins the product model. */
			if (0 != std::strncmp(
				pName,
				CUSTOM_CHAIN_CLIP_PREFIX,
				std::strlen(CUSTOM_CHAIN_CLIP_PREFIX)))
			{
				continue;
			}
			ImGui::PushID(static_cast<int32_t>(iAnimation));
			if (ImGui::SmallButton("+"))
			{
				CUSTOM_CHAIN_STEP Step;
				Step.clipName = pName;
				m_CustomChainSteps.push_back(std::move(Step));
				Invalidate_ValtanPatternCreateExactSourceSelection();
			}
			ImGui::SameLine();
			ImGui::TextUnformatted(pName);
			ImGui::PopID();
		}
	}
	ImGui::EndChild();
	ImGui::EndDisabled();

	ImGui::End();
}

bool_t Client::CAnimation_Tool::Start_ValtanCustomChainPreview(
	const shared_ptr<Engine::CModel>& pModel)
{
	if (nullptr == pModel || CAnimationTargetService::Resolve_Model() != pModel)
	{
		m_strValtanPatternPreviewStatus =
			"Custom chain start rejected because the animation target changed.";
		return false;
	}
	if (m_CustomChainSteps.empty())
	{
		m_strValtanPatternPreviewStatus =
			"Custom chain start rejected because it owns no step.";
		return false;
	}

	std::vector<VALTAN_PATTERN_PREVIEW_PLAY_ITEM> staged;
	staged.reserve(m_CustomChainSteps.size());
	const uint32_t iStepCount =
		static_cast<uint32_t>(m_CustomChainSteps.size());
	for (uint32_t iStep = 0u; iStep < iStepCount; ++iStep)
	{
		const CUSTOM_CHAIN_STEP& Step = m_CustomChainSteps[iStep];
		VALTAN_PATTERN_PREVIEW_PLAY_ITEM item;
		item.strPatternLabel = "Custom chain";
		item.iSequenceIndex = -1;
		item.iSequenceRepeatNumber = 1u;
		item.iSequenceRepeatCount = 1u;
		item.iSourceStepNumber = iStep + 1u;
		item.iSourceStepCount = iStepCount;
		item.strSequenceName = "Custom chain";
		item.strSequenceMode = "CUSTOM";
		item.strClipName = Step.clipName;
		item.iStepNumber = iStep + 1u;
		item.iStepCount = iStepCount;
		/* A non-positive or non-finite entry means the animator has not chosen
		   a length yet, which the playlist already reads as the native one. */
		item.fAuthoredDurationSeconds =
			std::isfinite(Step.fDurationSeconds) && Step.fDurationSeconds > 0.f ?
				Step.fDurationSeconds : 0.f;
		staged.push_back(std::move(item));
	}

	if (m_bValtanPatternMasterPlaying)
	{
		Reset_ValtanPatternMasterPreviewState(
			"Valtan Pattern Master yielded to the custom chain.");
	}

	m_ValtanPatternPreviewPlaylist = std::move(staged);
	m_iValtanPatternPreviewItem = 0u;
	m_bValtanPatternPreviewPlaying = true;
	m_bValtanPatternPreviewPaused = false;
	m_fValtanPatternPreviewElapsedSeconds = 0.f;
	m_ValtanPatternPreviewModel = pModel;
	m_iValtanPatternPreviewTargetGeneration =
		CAnimationTargetService::Resolve_TargetGeneration();
	m_iValtanSequenceSelected = -1;
	m_fValtanPatternHitTimelineBaseSeconds = 0.f;
	m_strValtanPatternPreviewStatus = "Playing the custom chain: " +
		std::to_string(iStepCount) + " steps.";
	return Activate_ValtanPatternPreviewItem(pModel);
}
