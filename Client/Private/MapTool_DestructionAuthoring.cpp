#include "imgui.h"
#include "MapTool_Internal.h"
#include "GameInstance.h"
#include "DeployPropObject.h"
#include "DestructionSimulationController.h"
#include "Model.h"
#include "RuntimeAssetRoot.h"
#include "Trigger_Box.h"
#include <algorithm>
#include <array>
#include <cctype>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iterator>
#include <limits>
#include <map>
#include <sstream>
#include <system_error>
#include <unordered_map>
#include <unordered_set>




void Client::CMapTool::Update_DestructionSimulation(
	const f32_t fTimeDelta,
	const bool_t isMapAuthoringLevel)
{
	if (nullptr == m_pDestructionSimulationController)
		return;
	if (m_bDestructionSimulationClearRequested)
	{
		m_pDestructionSimulationController->Clear();
		m_bDestructionSimulationClearRequested = false;
	}

	const bool_t isAuditionVisible = isMapAuthoringLevel && m_bOpen &&
		TOOL_MODE::WORLD_DESTRUCTION == m_eToolMode;
	if (!isAuditionVisible)
	{
		if (m_pDestructionSimulationController->Get_Runtime().Is_Staged() ||
			m_pDestructionSimulationController->Has_PendingCommand())
		{
			/* Leaving the audition must release its exclusive debug clock and
			   restore every DeployProp preview seam. Clear also discards a stage
			   command latched by the preceding Render frame. */
			m_pDestructionSimulationController->Clear();
		}
		return;
	}

	m_pDestructionSimulationController->Update(
		(std::max)(0.f, fTimeDelta));
	m_pDestructionSimulationController->Post_Physics_Update();

	const DESTRUCTION_SIMULATION_CONTROLLER_SNAPSHOT& snapshot =
		m_pDestructionSimulationController->Get_Snapshot();
	if (isAuditionVisible && m_bDestructionSimulationLoop &&
		DESTRUCTION_SIMULATION_PLAYBACK_STATE::FINISHED == snapshot.eState &&
		!m_pDestructionSimulationController->Has_PendingCommand())
	{
		m_pDestructionSimulationController->Request_Reset();
		m_pDestructionSimulationController->Request_Play();
	}
}

bool_t Client::CMapTool::Reload_DestructionAuthoring()
{
	if (m_bDestructionSimulationElementDraftDirty)
	{
		m_DestructionStatus =
			"Apply or Revert the debris Detail draft before reloading";
		m_DestructionSimulationStatus = m_DestructionStatus;
		return false;
	}

	const EDITOR_AREA_DESCRIPTOR* descriptor = Get_ActiveEditorArea();
	if (nullptr == descriptor || descriptor->encounterReference.empty() ||
		descriptor->worldEventsDocument.empty())
	{
		m_DestructionStatus =
			"Active Area has no complete destruction authoring contract";
		return false;
	}

	CEncounterPatternReference stagedEncounter;
	std::string encounterStatus;
	if (!stagedEncounter.Load(
		descriptor->encounterReference, encounterStatus))
	{
		m_EncounterReferenceStatus = encounterStatus;
		m_DestructionStatus = encounterStatus;
		return false;
	}

	CWorldDestructionDocument stagedDestruction;
	std::error_code inspectError;
	const bool_t hasDocument = std::filesystem::is_regular_file(
		descriptor->worldEventsDocument, inspectError);
	if (IsFileInspectionFailure(inspectError))
	{
		m_DestructionStatus = "World events document is unreadable";
		return false;
	}

	std::string destructionStatus;
	if (!hasDocument)
	{
		stagedDestruction.Reset_Empty();
		destructionStatus =
			"No world events document yet. An empty draft was staged.";
	}
	else if (!stagedDestruction.Load(
		descriptor->worldEventsDocument,
		m_Catalog.Get_AreaId(),
		"ENCOUNTER_VALTAN",
		destructionStatus))
	{
		m_DestructionStatus = destructionStatus;
		return false;
	}

	std::string validationStatus;
	if (!Validate_DestructionExternalReferences(
		stagedDestruction,
		m_DeployRuntime,
		m_RuntimeBlockerDocument,
		m_WorldGameplayDocument,
		stagedEncounter,
		validationStatus))
	{
		m_EncounterReferenceStatus = encounterStatus;
		m_DestructionStatus = validationStatus;
		return false;
	}
	const uint64_t previousPlacementId = m_iSelectedDeployPlacementId;
	const std::string previousGroupId = m_SelectedDestructionGroupId;
	const std::string previousProfileId =
		m_SelectedDestructionSimulationProfileId;
	Restore_DestructionPreview();
	m_bDestructionPickArmed = false;
	m_bDestructionAddMemberArmed = false;
	m_bDestructionNewSettingArmed = false;
	m_bDestructionTimelinePlaying = false;
	m_fDestructionTimelineMs = 0.f;
	m_SelectedDestructionGroupId.clear();
	m_SelectedDestructionBindingId.clear();
	m_SelectedDestructionPatternId.clear();
	m_SelectedDestructionStageId.clear();
	m_EncounterReference = std::move(stagedEncounter);
	m_DestructionDocument = std::move(stagedDestruction);
	m_EncounterReferenceStatus = encounterStatus;

	bool_t selectionRestored = false;
	if (0u != previousPlacementId)
	{
		selectionRestored = Select_DestructionWall(
			previousPlacementId, "authoring reload");
	}
	if (!selectionRestored && !previousGroupId.empty())
	{
		const DESTRUCTION_GROUP* previousGroup =
			m_DestructionDocument.Find_Group(previousGroupId);
		if (nullptr != previousGroup)
		{
			m_SelectedDestructionGroupId = previousGroup->groupId;
			selectionRestored = true;
		}
	}

	const DESTRUCTION_SIMULATION_PROFILE* previousProfile =
		previousProfileId.empty() ? nullptr :
		m_DestructionSimulationDocument.Find_Profile(previousProfileId);
	if (nullptr != previousProfile &&
		previousProfile->groupId == m_SelectedDestructionGroupId)
	{
		Select_DestructionSimulationProfile(previousProfile->profileId);
	}
	else if (selectionRestored &&
		m_SelectedDestructionSimulationProfileId.empty())
	{
		const auto matchingProfile = std::find_if(
			m_DestructionSimulationDocument.Get_Profiles().begin(),
			m_DestructionSimulationDocument.Get_Profiles().end(),
			[this](const DESTRUCTION_SIMULATION_PROFILE& profile)
			{
				return profile.groupId == m_SelectedDestructionGroupId;
			});
		if (matchingProfile !=
			m_DestructionSimulationDocument.Get_Profiles().end())
		{
			Select_DestructionSimulationProfile(matchingProfile->profileId);
		}
	}
	if (selectionRestored)
		Sync_DestructionDraftFromSelection();
	Refresh_DestructionHighlight();
	m_DestructionStatus = destructionStatus + " " + validationStatus;
	return true;
}

bool_t Client::CMapTool::Ensure_DestructionDebrisAuthoringPrototypes()
{
	if (m_iAuthoringLevelIndex >= ETOUI(LEVEL::END) ||
		nullptr == m_pDevice || nullptr == m_pContext)
	{
		m_Status =
			"PROJECT_AUTHORED debris model admission is unavailable";
		return false;
	}

	const auto& specs =
		CDestructionSimulationRuntime::Get_ProjectAuthoredDebrisModelSpecs();
	if (specs.empty())
	{
		m_Status = "PROJECT_AUTHORED debris model recipe is empty";
		return false;
	}

	std::unordered_set<wstring_t> uniqueTags;
	vector<const DESTRUCTION_SIMULATION_DEBRIS_MODEL_SPEC*> genericSpecs;
	vector<string> exactSourceAssetIds;
	for (const DESTRUCTION_SIMULATION_DEBRIS_MODEL_SPEC& spec : specs)
	{
		if (spec.prototypeTag.empty() || spec.assetId.empty() ||
			!std::isfinite(spec.fUniformScale) || spec.fUniformScale <= 0.f ||
			!std::isfinite(spec.vSourceLocalPivotMeters.x) ||
			!std::isfinite(spec.vSourceLocalPivotMeters.y) ||
			!std::isfinite(spec.vSourceLocalPivotMeters.z) ||
			!uniqueTags.emplace(spec.prototypeTag).second)
		{
			m_Status =
				"PROJECT_AUTHORED debris model recipe is invalid or duplicated";
			return false;
		}
		if (spec.sourceDeployAssetId.empty())
		{
			genericSpecs.push_back(&spec);
		}
		else if (std::find(
			exactSourceAssetIds.begin(), exactSourceAssetIds.end(),
			spec.sourceDeployAssetId) == exactSourceAssetIds.end())
		{
			exactSourceAssetIds.push_back(spec.sourceDeployAssetId);
		}
	}
	if (genericSpecs.empty())
	{
		m_Status = "PROJECT_AUTHORED generic debris recipe is empty";
		return false;
	}

	auto admitBatch = [this](
		const vector<const DESTRUCTION_SIMULATION_DEBRIS_MODEL_SPEC*>& batch,
		const bool_t required,
		const string& label,
		bool_t& outReady,
		string& outWarning)
	{
		outReady = false;
		vector<pair<wstring_t, unique_ptr<CPrototype>>> stagedPrototypes;
		vector<pair<wstring_t, std::filesystem::path>> stagedFingerprints;
		stagedPrototypes.reserve(batch.size());
		stagedFingerprints.reserve(batch.size());
		for (const DESTRUCTION_SIMULATION_DEBRIS_MODEL_SPEC* spec : batch)
		{
			const std::filesystem::path modelPath =
				CRuntimeAssetRoot::Resolve(spec->assetId).lexically_normal();
			std::error_code modelError;
			if (modelPath.empty() || !std::filesystem::is_regular_file(
				modelPath, modelError) || modelError)
			{
				const string message = label + " missing " + spec->assetId;
				if (required)
				{
					m_Status = message;
					return false;
				}
				outWarning = message;
				return true;
			}

			const shared_ptr<CModel> existing = dynamic_pointer_cast<CModel>(
				CGameInstance::Get().Clone_Prototype(
					m_iAuthoringLevelIndex, spec->prototypeTag));
			if (nullptr != existing)
			{
				const auto fingerprint =
					m_PrototypeModelPaths.find(spec->prototypeTag);
				if (fingerprint == m_PrototypeModelPaths.end() ||
					fingerprint->second.lexically_normal() != modelPath)
				{
					m_Status =
						"PROJECT_AUTHORED debris prototype tag collision: " +
						spec->assetId;
					return false;
				}
				continue;
			}

			const f32_t modelScale = 0.01f * spec->fUniformScale;
			auto model = CModel::Create(
				m_pDevice,
				m_pContext,
				MODEL::NONANIM,
				modelPath.string().c_str(),
				XMMatrixScaling(modelScale, modelScale, modelScale));
			if (nullptr == model)
			{
				const string message = label + " decode failed " + spec->assetId;
				if (required)
				{
					m_Status = message;
					return false;
				}
				outWarning = message;
				return true;
			}

			unique_ptr<CPrototype> prototype = std::move(model);
			stagedPrototypes.emplace_back(
				spec->prototypeTag, std::move(prototype));
			stagedFingerprints.emplace_back(spec->prototypeTag, modelPath);
		}

		if (!stagedPrototypes.empty() && FAILED(
			CGameInstance::Get().Add_Prototypes(
				m_iAuthoringLevelIndex, std::move(stagedPrototypes))))
		{
			const string message = label + " prototype batch commit failed";
			if (required)
			{
				m_Status = message;
				return false;
			}
			outWarning = message;
			return true;
		}
		for (const auto& [prototypeTag, modelPath] : stagedFingerprints)
			m_PrototypeModelPaths.emplace(prototypeTag, modelPath);
		outReady = true;
		return true;
	};

	bool_t genericReady = false;
	string warning;
	if (!admitBatch(
		genericSpecs, true, "Generic debris", genericReady, warning) ||
		!genericReady)
	{
		return false;
	}
	vector<string> unavailableExactRecipes;
	for (const string& sourceAssetId : exactSourceAssetIds)
	{
		vector<const DESTRUCTION_SIMULATION_DEBRIS_MODEL_SPEC*> exactSpecs;
		for (const DESTRUCTION_SIMULATION_DEBRIS_MODEL_SPEC& spec : specs)
		{
			if (spec.sourceDeployAssetId == sourceAssetId)
				exactSpecs.push_back(&spec);
		}
		bool_t exactReady = false;
		warning.clear();
		if (!admitBatch(
			exactSpecs, false, "Exact debris " + sourceAssetId,
			exactReady, warning))
		{
			return false;
		}
		if (!exactReady)
			unavailableExactRecipes.push_back(warning);
	}

	m_Status = "PROJECT_AUTHORED generic debris models ready";
	if (unavailableExactRecipes.empty())
	{
		m_Status += "; exact wall recipes ready";
	}
	else
	{
		m_Status += "; exact wall recipe fallback: " +
			unavailableExactRecipes.front();
	}

	return true;
}

std::filesystem::path Client::CMapTool::Get_WorldDestructionPath() const
{
	const EDITOR_AREA_DESCRIPTOR* descriptor = Get_ActiveEditorArea();
	return nullptr != descriptor ? descriptor->worldEventsDocument :
		std::filesystem::path{};
}

bool_t Client::CMapTool::Load_WorldDestruction()
{
	const std::filesystem::path path = Get_WorldDestructionPath();
	if (path.empty())
	{
		m_DestructionStatus = "This Area has no world destruction document";
		return false;
	}
	/* The document only exists after the first Save, so a missing file is a
	   normal starting state and not a read failure. */
	std::error_code inspectError;
	if (!std::filesystem::is_regular_file(path, inspectError))
	{
		m_DestructionStatus = IsFileInspectionFailure(inspectError) ?
			"World events document is unreadable" :
			"No world events document yet. Create a group and press Save.";
		return false;
	}
	CWorldDestructionDocument staged;
	std::string status;
	if (!staged.Load(path, m_Catalog.Get_AreaId(), "ENCOUNTER_VALTAN", status))
	{
		m_DestructionStatus = status;
		return false;
	}
	if (!Validate_DestructionExternalReferences(
		staged,
		m_DeployRuntime,
		m_RuntimeBlockerDocument,
		m_WorldGameplayDocument,
		m_EncounterReference,
		status))
	{
		m_DestructionStatus = status;
		return false;
	}
	if (m_DestructionSimulationDocument.Is_Ready() &&
		!m_DestructionSimulationDocument.Validate_GroupReferences(
			staged, status))
	{
		m_DestructionStatus = status;
		return false;
	}
	if (nullptr != m_pDestructionSimulationController)
		m_pDestructionSimulationController->Clear();
	m_bDestructionSimulationClearRequested = false;
	const std::string previousGroupId = m_SelectedDestructionGroupId;
	const std::string previousProfileId =
		m_SelectedDestructionSimulationProfileId;
	m_DestructionDocument = std::move(staged);
	m_SelectedDestructionBindingId.clear();
	m_SelectedDestructionStageId.clear();
	Reset_DestructionSimulationUI();
	m_bDestructionSimulationClearRequested = false;
	const DESTRUCTION_GROUP* selectedGroup = previousGroupId.empty() ? nullptr :
		m_DestructionDocument.Find_Group(previousGroupId);
	const DESTRUCTION_SIMULATION_PROFILE* selectedProfile =
		previousProfileId.empty() ? nullptr :
		m_DestructionSimulationDocument.Find_Profile(previousProfileId);
	if (nullptr != selectedProfile && nullptr != selectedGroup &&
		selectedProfile->groupId != selectedGroup->groupId)
	{
		selectedProfile = nullptr;
	}
	if (nullptr == selectedProfile && nullptr != selectedGroup)
	{
		const auto found = std::find_if(
			m_DestructionSimulationDocument.Get_Profiles().begin(),
			m_DestructionSimulationDocument.Get_Profiles().end(),
			[selectedGroup](const DESTRUCTION_SIMULATION_PROFILE& profile)
			{
				return profile.groupId == selectedGroup->groupId;
			});
		if (found != m_DestructionSimulationDocument.Get_Profiles().end())
			selectedProfile = &*found;
	}

	if (nullptr != selectedGroup)
		m_SelectedDestructionGroupId = selectedGroup->groupId;
	else if (nullptr == selectedProfile &&
		!m_DestructionSimulationDocument.Get_Profiles().empty())
	{
		selectedProfile =
			&m_DestructionSimulationDocument.Get_Profiles().front();
	}
	else if (nullptr == selectedProfile)
		m_SelectedDestructionGroupId.clear();

	if (nullptr != selectedProfile)
		Select_DestructionSimulationProfile(selectedProfile->profileId);
	Refresh_DestructionHighlight();
	m_DestructionStatus = status;
	return true;
}

bool_t Client::CMapTool::Save_WorldDestruction()
{
	return Save_DestructionAuthoringPair();
}

bool_t Client::CMapTool::Save_DestructionAuthoringPair()
{
	if (m_bDestructionSimulationElementDraftDirty)
	{
		const std::string status =
			"Apply or Revert the debris Detail draft before saving";
		m_DestructionStatus = status;
		m_DestructionSimulationStatus = status;
		return false;
	}
	if (m_bWorldGameplayDirty || m_NavigationDocument.Is_Dirty() ||
		m_RuntimeBlockerDocument.Is_Dirty())
	{
		m_DestructionStatus =
			"Save World Gameplay and Navigation first, or use Save All";
		return false;
	}
	std::string status;
	if (!Validate_DestructionExternalReferences(
		m_DestructionDocument,
		m_DeployRuntime,
		m_RuntimeBlockerDocument,
		m_WorldGameplayDocument,
		m_EncounterReference,
		status))
	{
		m_DestructionStatus = status;
		return false;
	}
	const EDITOR_AREA_DESCRIPTOR* descriptor = Get_ActiveEditorArea();
	const std::filesystem::path destructionPath =
		Get_WorldDestructionPath();
	const std::filesystem::path simulationPath =
		Get_DestructionSimulationPath();
	if (nullptr == descriptor || destructionPath.empty() ||
		simulationPath.empty() || !m_DestructionDocument.Is_Ready() ||
		!m_DestructionSimulationDocument.Is_Ready())
	{
		const std::string status =
			"Destruction pair save requires a ready Valtan Area";
		m_DestructionStatus = status;
		m_DestructionSimulationStatus = status;
		return false;
	}

	if (!CDestructionSimulationDocument::Save_AuthoringPair(
		m_DestructionDocument, destructionPath,
		m_DestructionSimulationDocument, simulationPath,
		descriptor->areaId, "ENCOUNTER_VALTAN", status))
	{
		m_DestructionStatus = status;
		m_DestructionSimulationStatus = status;
		return false;
	}

	m_DestructionDocument.Clear_Dirty();
	m_DestructionSimulationDocument.Clear_Dirty();
	m_DestructionStatus = status;
	m_DestructionSimulationStatus = status;
	return true;
}

std::filesystem::path Client::CMapTool::Get_DestructionSimulationPath() const
{
	const EDITOR_AREA_DESCRIPTOR* descriptor = Get_ActiveEditorArea();
	return nullptr != descriptor ? descriptor->destructionSimulationDocument :
		std::filesystem::path{};
}

bool_t Client::CMapTool::Load_DestructionSimulation()
{
	if (m_bDestructionSimulationElementDraftDirty)
	{
		m_DestructionSimulationStatus =
			"Apply or Revert the debris Detail draft before reloading";
		return false;
	}

	const std::string selectedGroupId = m_SelectedDestructionGroupId;
	const std::string selectedProfileId =
		m_SelectedDestructionSimulationProfileId;

	const EDITOR_AREA_DESCRIPTOR* descriptor = Get_ActiveEditorArea();
	const std::filesystem::path path = Get_DestructionSimulationPath();
	if (nullptr == descriptor || path.empty())
	{
		m_DestructionSimulationStatus =
			"This Area has no destruction simulation document";
		return false;
	}

	CDestructionSimulationDocument staged;
	std::error_code inspectError;
	const bool_t exists = std::filesystem::is_regular_file(path, inspectError);
	if (IsFileInspectionFailure(inspectError))
	{
		m_DestructionSimulationStatus =
			"Destruction simulation document is unreadable";
		return false;
	}

	std::string status;
	if (!exists)
	{
		staged.Reset_Empty();
		status = "No simulation document yet; create a default profile";
	}
	else if (!staged.Load(path, descriptor->areaId, status))
	{
		m_DestructionSimulationStatus = status;
		return false;
	}
	if (m_DestructionDocument.Is_Ready() &&
		!staged.Validate_GroupReferences(m_DestructionDocument, status))
	{
		m_DestructionSimulationStatus = status;
		return false;
	}
	if (nullptr != m_pDestructionSimulationController)
		m_pDestructionSimulationController->Clear();
	m_bDestructionSimulationClearRequested = false;
	m_DestructionSimulationDocument = std::move(staged);
	Reset_DestructionSimulationUI();
	const DESTRUCTION_GROUP* selectedGroup = selectedGroupId.empty() ? nullptr :
		m_DestructionDocument.Find_Group(selectedGroupId);
	const DESTRUCTION_SIMULATION_PROFILE* selectedProfile =
		selectedProfileId.empty() ? nullptr :
		m_DestructionSimulationDocument.Find_Profile(selectedProfileId);
	if (nullptr != selectedProfile && nullptr != selectedGroup &&
		selectedProfile->groupId != selectedGroup->groupId)
	{
		selectedProfile = nullptr;
	}
	if (nullptr == selectedProfile && nullptr != selectedGroup)
	{
		const auto found = std::find_if(
			m_DestructionSimulationDocument.Get_Profiles().begin(),
			m_DestructionSimulationDocument.Get_Profiles().end(),
			[selectedGroup](const DESTRUCTION_SIMULATION_PROFILE& profile)
			{
				return profile.groupId == selectedGroup->groupId;
			});
		if (found != m_DestructionSimulationDocument.Get_Profiles().end())
			selectedProfile = &*found;
	}

	if (nullptr != selectedGroup)
		m_SelectedDestructionGroupId = selectedGroup->groupId;
	else if (nullptr == selectedProfile &&
		!m_DestructionSimulationDocument.Get_Profiles().empty())
	{
		selectedProfile =
			&m_DestructionSimulationDocument.Get_Profiles().front();
	}
	else
		m_SelectedDestructionGroupId.clear();

	if (nullptr != selectedProfile)
		Select_DestructionSimulationProfile(selectedProfile->profileId);
	else if (nullptr != selectedGroup)
		status += "; selected group has no simulation profile; create default";
	Refresh_DestructionHighlight();
	m_DestructionSimulationStatus = status;
	return true;
}

bool_t Client::CMapTool::Save_DestructionSimulation()
{
	return Save_DestructionAuthoringPair();
}

const DESTRUCTION_SIMULATION_PROFILE*
Client::CMapTool::Get_SelectedDestructionSimulationProfile() const
{
	return m_SelectedDestructionSimulationProfileId.empty() ? nullptr :
		m_DestructionSimulationDocument.Find_Profile(
			m_SelectedDestructionSimulationProfileId);
}

void Client::CMapTool::Reset_DestructionSimulationUI()
{
	m_SelectedDestructionSimulationProfileId.clear();
	m_SelectedDestructionSimulationElementId.clear();
	m_SelectedDestructionSimulationFragmentId.clear();
	m_DestructionSimulationElementDraft.reset();
	m_bDestructionSimulationElementDraftDirty = false;
	m_DestructionSimulationReceiverId[0] = '\0';
	m_bDestructionSimulationClearRequested = true;
}

void Client::CMapTool::Select_DestructionSimulationProfile(
	const std::string& profileId)
{
	if (m_bDestructionSimulationElementDraftDirty &&
		profileId != m_SelectedDestructionSimulationProfileId)
	{
		m_DestructionSimulationStatus =
			"Apply or Revert the debris Detail draft before changing profile";
		return;
	}
	const DESTRUCTION_SIMULATION_PROFILE* profile =
		m_DestructionSimulationDocument.Find_Profile(profileId);
	if (nullptr == profile)
	{
		m_DestructionSimulationStatus =
			"Selected destruction simulation profile is missing";
		return;
	}

	const bool_t profileChanged =
		m_SelectedDestructionSimulationProfileId != profile->profileId;
	m_SelectedDestructionSimulationProfileId = profile->profileId;
	m_SelectedDestructionGroupId = profile->groupId;
	m_SelectedDestructionSimulationElementId.clear();
	m_SelectedDestructionSimulationFragmentId.clear();
	m_DestructionSimulationElementDraft.reset();
	m_bDestructionSimulationElementDraftDirty = false;
	m_DestructionSimulationReceiverId[0] = '\0';
	if (!profile->Elements.empty())
		Select_DestructionSimulationElement(profile->Elements.front().elementId);
	if (nullptr != m_pDestructionSimulationController)
	{
		const bool_t hasRuntime =
			m_pDestructionSimulationController->Get_Runtime().Is_Staged() ||
			m_pDestructionSimulationController->Has_PendingCommand();
		if (profileChanged && hasRuntime)
		{
			m_bDestructionSimulationClearRequested = true;
		}
		else if (hasRuntime)
		{
			m_pDestructionSimulationController->Request_Reset();
		}
	}
	m_DestructionSimulationStatus = "Selected simulation profile: " +
		profile->profileId;
}

void Client::CMapTool::Select_DestructionSimulationElement(
	const std::string& elementId)
{
	if (m_bDestructionSimulationElementDraftDirty &&
		elementId != m_SelectedDestructionSimulationElementId)
	{
		m_DestructionSimulationStatus =
			"Apply or Revert the debris Detail draft before changing element";
		return;
	}
	const DESTRUCTION_SIMULATION_ELEMENT* element =
		m_DestructionSimulationDocument.Find_Element(
			m_SelectedDestructionSimulationProfileId, elementId);
	if (nullptr == element)
	{
		m_DestructionSimulationStatus =
			"Selected debris element is missing";
		return;
	}

	m_SelectedDestructionSimulationElementId = element->elementId;
	m_SelectedDestructionSimulationFragmentId.clear();
	m_DestructionSimulationElementDraft = *element;
	m_bDestructionSimulationElementDraftDirty = false;
	strncpy_s(m_DestructionSimulationReceiverId,
		element->Trigger.receiverCollisionId.c_str(), _TRUNCATE);

	if (nullptr != m_pDestructionSimulationController)
	{
		const DESTRUCTION_SIMULATION_SCOPE scope =
			m_pDestructionSimulationController->Get_Snapshot().eScope;
		if (DESTRUCTION_SIMULATION_SCOPE::SOLO_SELECTED == scope ||
			DESTRUCTION_SIMULATION_SCOPE::SOLO_FRAGMENT == scope)
		{
			const DESTRUCTION_SIMULATION_PROFILE* selectedProfile =
				Get_SelectedDestructionSimulationProfile();
			if (nullptr != selectedProfile &&
				AreEmittersAuthoredAsOneWall(*selectedProfile))
			{
				m_pDestructionSimulationController->Request_SetScope(
					DESTRUCTION_SIMULATION_SCOPE::ALL_DEBRIS);
			}
			else
			{
				m_pDestructionSimulationController->Request_SetScope(
					DESTRUCTION_SIMULATION_SCOPE::SOLO_SELECTED,
					element->elementId);
			}
		}
	}
}

void Client::CMapTool::Select_DestructionSimulationFragment(
	const std::string& elementId,
	const std::string& fragmentId)
{
	if (nullptr == m_pDestructionSimulationController || fragmentId.empty())
	{
		m_DestructionSimulationStatus =
			"Stage a destruction profile before selecting a fragment";
		return;
	}
	const DESTRUCTION_SIMULATION_FRAME& frame =
		m_pDestructionSimulationController->Get_Runtime().Get_Frame();
	if (frame.profileId != m_SelectedDestructionSimulationProfileId)
	{
		m_DestructionSimulationStatus =
			"Selected fragment is not in the active staged profile";
		return;
	}
	const auto runtimeElement = std::find_if(
		frame.Elements.begin(), frame.Elements.end(),
		[&elementId](const DESTRUCTION_SIMULATION_ELEMENT_FRAME& value)
		{
			return value.elementId == elementId;
		});
	if (runtimeElement == frame.Elements.end())
	{
		m_DestructionSimulationStatus =
			"Selected fragment emitter is missing from the runtime frame";
		return;
	}
	const auto fragment = std::find_if(
		runtimeElement->Fragments.begin(), runtimeElement->Fragments.end(),
		[&fragmentId](const DESTRUCTION_SIMULATION_FRAGMENT_FRAME& value)
		{
			return value.fragmentId == fragmentId;
		});
	if (fragment == runtimeElement->Fragments.end())
	{
		m_DestructionSimulationStatus =
			"Selected fragment is missing from the runtime frame";
		return;
	}

	const bool_t keepSoloFragment =
		DESTRUCTION_SIMULATION_SCOPE::SOLO_FRAGMENT ==
			m_pDestructionSimulationController->Get_Snapshot().eScope;
	if (m_SelectedDestructionSimulationElementId != elementId)
		Select_DestructionSimulationElement(elementId);
	if (m_SelectedDestructionSimulationElementId != elementId)
		return;
	m_SelectedDestructionSimulationFragmentId = fragment->fragmentId;
	if (keepSoloFragment)
	{
		m_pDestructionSimulationController->Request_SetScope(
			DESTRUCTION_SIMULATION_SCOPE::SOLO_FRAGMENT,
			fragment->fragmentId);
	}
	m_DestructionSimulationStatus = "Selected mesh fragment: " +
		fragment->fragmentId;
}

bool_t Client::CMapTool::Create_DefaultDestructionSimulationProfile()
{
	const DESTRUCTION_GROUP* group = m_SelectedDestructionGroupId.empty() ?
		nullptr :
		m_DestructionDocument.Find_Group(m_SelectedDestructionGroupId);
	if (nullptr == group)
	{
		m_DestructionSimulationStatus =
			"Select a non-empty destruction group before creating a profile";
		return false;
	}
	const auto existingProfile = std::find_if(
		m_DestructionSimulationDocument.Get_Profiles().begin(),
		m_DestructionSimulationDocument.Get_Profiles().end(),
		[group](const DESTRUCTION_SIMULATION_PROFILE& profile)
		{
			return profile.profileId == group->groupId + ".preview";
		});
	if (existingProfile != m_DestructionSimulationDocument.Get_Profiles().end())
	{
		Select_DestructionSimulationProfile(existingProfile->profileId);
		m_DestructionSimulationStatus =
			"Selected the existing simulation profile for group " +
			group->groupId;
		return true;
	}

	DESTRUCTION_SIMULATION_PROFILE profile;
	std::string status;
	if (!CDestructionSimulationDocument::Create_DefaultForGroup(
		*group, m_DeployRuntime, profile, status) ||
		!m_DestructionSimulationDocument.Add_Profile(profile, status))
	{
		m_DestructionSimulationStatus = status;
		return false;
	}
	Select_DestructionSimulationProfile(profile.profileId);
	m_DestructionSimulationStatus = status;
	return true;
}

bool_t Client::CMapTool::Modify_DestructionGroupMember(
	const uint64_t placementId,
	const bool_t addMember)
{
	if (m_bDestructionSimulationElementDraftDirty)
	{
		m_DestructionStatus =
			"Apply or Revert the debris Detail draft before editing group members";
		return false;
	}
	if (0u == placementId || m_SelectedDestructionGroupId.empty())
	{
		m_DestructionStatus = "A selected group and Deploy placement are required";
		return false;
	}

	CWorldDestructionDocument stagedDestruction = m_DestructionDocument;
	CDestructionSimulationDocument stagedSimulation =
		m_DestructionSimulationDocument;
	std::string status;
	const bool_t changed = addMember ? stagedDestruction.Add_Member(
		m_SelectedDestructionGroupId, placementId, status) :
		stagedDestruction.Remove_Member(
			m_SelectedDestructionGroupId, placementId);
	if (!changed)
	{
		m_DestructionStatus = status.empty() ?
			"Destruction group member change was rejected" : status;
		return false;
	}

	const DESTRUCTION_GROUP* stagedGroup = stagedDestruction.Find_Group(
		m_SelectedDestructionGroupId);
	if (nullptr == stagedGroup ||
		!stagedSimulation.Synchronize_Group(
			*stagedGroup, m_DeployRuntime, status) ||
		!stagedSimulation.Validate_GroupReferences(
			stagedDestruction, status))
	{
		m_DestructionStatus = status.empty() ?
			"Destruction group/simulation transaction was rejected" : status;
		return false;
	}

	const std::string selectedGroupId = m_SelectedDestructionGroupId;
	const std::string selectedProfileId =
		m_SelectedDestructionSimulationProfileId;
	if (nullptr != m_pDestructionSimulationController)
		m_pDestructionSimulationController->Clear();
	m_DestructionDocument = std::move(stagedDestruction);
	m_DestructionSimulationDocument = std::move(stagedSimulation);
	Reset_DestructionSimulationUI();
	m_bDestructionSimulationClearRequested = false;
	m_SelectedDestructionGroupId = selectedGroupId;
	const DESTRUCTION_SIMULATION_PROFILE* profile =
		m_DestructionSimulationDocument.Find_Profile(selectedProfileId);
	if (nullptr == profile)
	{
		const auto found = std::find_if(
			m_DestructionSimulationDocument.Get_Profiles().begin(),
			m_DestructionSimulationDocument.Get_Profiles().end(),
			[&selectedGroupId](const DESTRUCTION_SIMULATION_PROFILE& candidate)
			{
				return candidate.groupId == selectedGroupId;
			});
		if (found != m_DestructionSimulationDocument.Get_Profiles().end())
			profile = &*found;
	}
	if (nullptr != profile)
		Select_DestructionSimulationProfile(profile->profileId);
	Refresh_DestructionHighlight();
	m_DestructionStatus = addMember ?
		"Added group member and synchronized simulation elements" :
		"Removed group member and synchronized simulation elements";
	return true;
}

bool_t Client::CMapTool::Request_StageDestructionSimulation(
	const DESTRUCTION_SIMULATION_PROFILE& profile,
	const bool_t preserveSampleTime,
	const bool_t playAfterStage)
{
	if (!m_bDestructionDebrisPrototypesReady)
	{
		m_DestructionSimulationStatus = m_DestructionDebrisPrototypeStatus;
		return false;
	}
	if (nullptr == m_pDestructionSimulationController ||
		m_iAuthoringLevelIndex >= ETOUI(LEVEL::END) ||
		profile.groupId != m_SelectedDestructionGroupId ||
		nullptr == m_DestructionDocument.Find_Group(profile.groupId))
	{
		m_DestructionSimulationStatus =
			"Stage requires the selected profile and destruction group to match";
		return false;
	}

	std::string validationStatus;
	if (!CDestructionSimulationDocument::Validate_Profile(
		profile, validationStatus))
	{
		m_DestructionSimulationStatus = validationStatus;
		return false;
	}

	const DESTRUCTION_SIMULATION_CONTROLLER_SNAPSHOT previous =
		m_pDestructionSimulationController->Get_Snapshot();
	/* Stage_Profile replaces the staged actors through the controller-owned
	   runtime, so it supersedes a UI-only clear requested by profile selection. */
	m_bDestructionSimulationClearRequested = false;
	m_pDestructionSimulationController->Request_StageProfile(
		profile,
		m_SelectedDestructionGroupId,
		m_DestructionDocument,
		m_DeployRuntime,
		m_iAuthoringLevelIndex);
	if (DESTRUCTION_SIMULATION_SCOPE::SOLO_FRAGMENT == previous.eScope &&
		!m_SelectedDestructionSimulationFragmentId.empty())
	{
		m_pDestructionSimulationController->Request_SetScope(
			DESTRUCTION_SIMULATION_SCOPE::SOLO_FRAGMENT,
			m_SelectedDestructionSimulationFragmentId);
	}
	else if (DESTRUCTION_SIMULATION_SCOPE::SOLO_SELECTED == previous.eScope &&
		!m_SelectedDestructionSimulationElementId.empty())
	{
		m_pDestructionSimulationController->Request_SetScope(
			DESTRUCTION_SIMULATION_SCOPE::SOLO_SELECTED,
			m_SelectedDestructionSimulationElementId);
	}
	else
	{
		m_pDestructionSimulationController->Request_SetScope(
			DESTRUCTION_SIMULATION_SCOPE::ALL_DEBRIS);
	}
	if (preserveSampleTime)
		m_pDestructionSimulationController->Request_Seek(
			(std::min)(previous.fSampleTimeSeconds, profile.fDurationSeconds));
	if (playAfterStage)
		m_pDestructionSimulationController->Request_Play();
	m_DestructionSimulationStatus = playAfterStage ?
		"Simulation stage + play requested: " + profile.profileId :
		"Simulation staged paused at 0 s; press Play: " + profile.profileId;
	return true;
}

bool_t Client::CMapTool::Stage_DestructionElementDraftPreview()
{
	const DESTRUCTION_SIMULATION_PROFILE* activeProfile =
		Get_SelectedDestructionSimulationProfile();
	if (nullptr == activeProfile ||
		!m_DestructionSimulationElementDraft.has_value())
	{
		return false;
	}

	DESTRUCTION_SIMULATION_PROFILE staged = *activeProfile;
	const auto element = std::find_if(staged.Elements.begin(),
		staged.Elements.end(),
		[this](const DESTRUCTION_SIMULATION_ELEMENT& value)
		{
			return value.elementId ==
				m_DestructionSimulationElementDraft->elementId;
		});
	if (element == staged.Elements.end())
		return false;
	*element = *m_DestructionSimulationElementDraft;

	const bool_t wasPlaying = nullptr != m_pDestructionSimulationController &&
		DESTRUCTION_SIMULATION_PLAYBACK_STATE::PLAYING ==
			m_pDestructionSimulationController->Get_Snapshot().eState;
	return Request_StageDestructionSimulation(staged, true, wasPlaying);
}

bool_t Client::CMapTool::Validate_DestructionExternalReferences(
	const CWorldDestructionDocument& destruction,
	const CDeployPropRuntime& deployRuntime,
	const CNavRuntimeBlockerDocument& blockers,
	const CWorldGameplayDocument& worldGameplay,
	const CEncounterPatternReference& encounter,
	std::string& outStatus) const
{
	const bool_t hasEnabledBinding = std::any_of(
		destruction.Get_Bindings().begin(),
		destruction.Get_Bindings().end(),
		[](const DESTRUCTION_BINDING& binding)
		{
			return binding.isEnabled;
		});
	if (hasEnabledBinding)
	{
		if (!encounter.Is_Ready() ||
			encounter.Get_EncounterId() != "ENCOUNTER_VALTAN" ||
			encounter.Get_BossArchetypeId() != "BOSS_VALTAN")
		{
			outStatus =
				"Save blocked: enabled break settings require the Valtan encounter";
			return false;
		}

		size_t matchingBossCount = 0u;
		for (const WORLD_GAMEPLAY_PLACEMENT& placement :
			worldGameplay.Get_Placements())
		{
			if (WORLD_PLACEMENT_KIND::BOSS == placement.eKind &&
				placement.archetypeId == encounter.Get_BossArchetypeId() &&
				placement.encounterId == encounter.Get_EncounterId())
			{
				++matchingBossCount;
			}
		}
		if (1u != matchingBossCount)
		{
			outStatus =
				"Save blocked: enabled break settings require exactly one "
				"BOSS_VALTAN placement bound to ENCOUNTER_VALTAN";
			return false;
		}
	}

	std::vector<std::string> bindingSemanticKeys;
	std::vector<std::pair<std::string, std::string>> navigationOwners;
	for (const DESTRUCTION_GROUP& group : destruction.Get_Groups())
	{
		for (const uint64_t placementId : group.memberPlacementIds)
		{
			const auto entry = std::find_if(
				deployRuntime.Get_Entries().begin(),
				deployRuntime.Get_Entries().end(),
				[placementId](const DEPLOY_RUNTIME_ENTRY& value)
				{
					return value.placement.runtimePlacementId == placementId;
				});
			if (deployRuntime.Get_Entries().end() == entry ||
				!entry->placement.destructible)
			{
				outStatus = "Save blocked: group " + group.groupId +
					" references an unknown or non-destructible DeployProp " +
					std::to_string(placementId);
				return false;
			}
		}

		for (const std::string& regionId : group.navigationRegionIds)
		{
			const auto claimed = std::find_if(navigationOwners.begin(),
				navigationOwners.end(), [&regionId](const auto& value)
				{
					return value.first == regionId;
				});
			if (navigationOwners.end() != claimed &&
				claimed->second != group.groupId)
			{
				outStatus = "Save blocked: navigation region " + regionId +
					" is already owned by group " + claimed->second;
				return false;
			}
			if (navigationOwners.end() == claimed)
				navigationOwners.push_back({ regionId, group.groupId });

			bool_t found = false;
			for (size_t index = 0u;
				index < blockers.Get_RegionCount(); ++index)
			{
				const NAV_RUNTIME_BLOCKER_REGION* region =
					blockers.Get_Region(index);
				if (nullptr != region && region->id == regionId)
				{
					found = true;
					break;
				}
			}
			if (!found)
			{
				outStatus = "Save blocked: group " + group.groupId +
					" references an unknown navigation region " + regionId;
				return false;
			}
		}
	}

	for (const DESTRUCTION_BINDING& binding :
		destruction.Get_Bindings())
	{
		const DESTRUCTION_MUTATION* mutation =
			destruction.Find_Mutation(binding.mutationId);
		const DESTRUCTION_GROUP* group = nullptr == mutation ? nullptr :
			destruction.Find_Group(mutation->groupId);
		if (nullptr == mutation || nullptr == group)
		{
			outStatus = "Save blocked: binding " + binding.bindingId +
				" has a missing mutation or group";
			return false;
		}
		/* A wall that never crosses a base-walkable cell owns no blocker to
		open when it breaks, so an empty navigation closure is authored, not
		missing. Publish-ValtanWorldDestruction.ps1 accepts zero or one
		region and demanding one here blocks the Area on data the publisher
		already admits. The declared regions are still checked below. */
		const bool requiresNavigation = binding.isEnabled &&
			DESTRUCTION_TRIGGER_KIND::COLLISION_IMPACT == binding.eTriggerKind;
		if (binding.isEnabled && group->memberPlacementIds.empty())
		{
			outStatus = "Save blocked: enabled binding " + binding.bindingId +
				" has no wall members";
			return false;
		}
		if (requiresNavigation)
		{
			for (const std::string& regionId : group->navigationRegionIds)
			{
				bool_t hasCells = false;
				for (size_t index = 0u; index < blockers.Get_RegionCount(); ++index)
				{
					const NAV_RUNTIME_BLOCKER_REGION* region = blockers.Get_Region(index);
					if (nullptr != region && region->id == regionId)
					{
						hasCells = 0u != blockers.Get_RegionCellCount(index);
						break;
					}
				}
				if (!hasCells)
				{
					outStatus = "Save blocked: enabled binding " +
						binding.bindingId + " uses an empty navigation region " +
						regionId;
					return false;
				}
			}
		}

		/* A contact break names no pattern and no stage on purpose, so it has
		no encounter tuple to resolve. Publish-ValtanWorldDestruction.ps1 skips
		the same check for it, and demanding one here blocks the Area on data
		the publisher accepts. */
		if (DESTRUCTION_TRIGGER_KIND::COLLIDER_CONTACT != binding.eTriggerKind)
		{
			const ENCOUNTER_PATTERN_REFERENCE* pattern =
				encounter.Find_Pattern(binding.patternId);
			if (nullptr == pattern)
			{
				outStatus = "Save blocked: binding " + binding.bindingId +
					" references an unknown Valtan pattern " + binding.patternId;
				return false;
			}
			const auto stage = std::find_if(pattern->stages.begin(),
				pattern->stages.end(),
				[&binding](const ENCOUNTER_STAGE_REFERENCE& value)
				{
					return value.stageId == binding.stageId;
				});
			if (pattern->stages.end() == stage ||
				(DESTRUCTION_TRIGGER_KIND::STAGE_TIME == binding.eTriggerKind &&
					binding.iOffsetMs > stage->iDurationMs))
			{
				outStatus = "Save blocked: binding " + binding.bindingId +
					" has an unknown stage or an out-of-range time";
				return false;
			}
		}
		if (DESTRUCTION_TRIGGER_KIND::COLLISION_IMPACT ==
				binding.eTriggerKind ||
			DESTRUCTION_TRIGGER_KIND::COLLIDER_CONTACT ==
				binding.eTriggerKind)
		{
			const WORLD_GAMEPLAY_PLACEMENT* receiver =
				worldGameplay.Find(binding.receiverCollisionId);
			if (nullptr == receiver ||
				WORLD_PLACEMENT_KIND::COLLISION_BOX != receiver->eKind ||
				(binding.isEnabled && !receiver->isEnabled))
			{
				outStatus = "Save blocked: binding " + binding.bindingId +
					" references an unknown Collision Box " +
					binding.receiverCollisionId;
				return false;
			}
		}

		const std::string semanticKey = group->groupId + "|" +
			binding.patternId + "|" + binding.stageId + "|" +
			CWorldDestructionDocument::TriggerKind_ToString(
				binding.eTriggerKind) + "|" +
			std::to_string(binding.iOffsetMs) + "|" +
			binding.receiverCollisionId;
		if (bindingSemanticKeys.end() != std::find(
			bindingSemanticKeys.begin(), bindingSemanticKeys.end(), semanticKey))
		{
			outStatus = "Save blocked: duplicate semantic break setting " +
				binding.bindingId;
			return false;
		}
		bindingSemanticKeys.push_back(semanticKey);
	}

	outStatus = "World destruction external references validated";
	return true;
}

bool_t Client::CMapTool::Validate_CurrentDestructionReferences(
	std::string& outStatus) const
{
	if (!m_DestructionDocument.Is_Ready())
	{
		outStatus = "World destruction document is not active";
		return true;
	}
	return Validate_DestructionExternalReferences(
		m_DestructionDocument,
		m_DeployRuntime,
		m_RuntimeBlockerDocument,
		m_WorldGameplayDocument,
		m_EncounterReference,
		outStatus);
}

bool_t Client::CMapTool::Select_DestructionWall(
	const uint64_t runtimePlacementId,
	const char_t* source)
{
	if (m_bDestructionSimulationElementDraftDirty)
	{
		m_DestructionStatus =
			"Apply or Revert the debris Detail draft before selecting another wall";
		return false;
	}

	Restore_DestructionPreview();
	const auto entry = std::find_if(
		m_DeployRuntime.Get_Entries().begin(),
		m_DeployRuntime.Get_Entries().end(),
		[runtimePlacementId](const DEPLOY_RUNTIME_ENTRY& value)
		{
			return value.placement.runtimePlacementId == runtimePlacementId;
		});
	if (m_DeployRuntime.Get_Entries().end() == entry)
	{
		m_DestructionStatus = "Wall selection failed: unknown placement " +
			std::to_string(runtimePlacementId);
		return false;
	}
	if (!entry->placement.destructible)
	{
		m_DestructionStatus =
			"Wall selection rejected: this DeployProp is not destructible";
		return false;
	}

	if (m_bDestructionAddMemberArmed)
	{
		const DESTRUCTION_GROUP* target =
			m_SelectedDestructionGroupId.empty() ? nullptr :
			m_DestructionDocument.Find_Group(m_SelectedDestructionGroupId);
		if (nullptr == target)
		{
			m_bDestructionAddMemberArmed = false;
			m_DestructionStatus =
				"Add wall cancelled: the target group no longer exists";
			return false;
		}

		const DESTRUCTION_GROUP* owner =
			m_DestructionDocument.Find_GroupOfMember(runtimePlacementId);
		if (nullptr != owner && owner->groupId != target->groupId)
		{
			m_DestructionStatus = "Add wall failed: it already belongs to " +
				owner->groupId;
			return false;
		}
		if (nullptr == owner)
		{
			if (!Modify_DestructionGroupMember(runtimePlacementId, true))
				return false;
		}
		m_bDestructionAddMemberArmed = false;
	}

	m_iSelectedDeployPlacementId = runtimePlacementId;
	const DESTRUCTION_GROUP* owner =
		m_DestructionDocument.Find_GroupOfMember(runtimePlacementId);
	if (nullptr != owner)
	{
		m_SelectedDestructionGroupId = owner->groupId;
		const auto profile = std::find_if(
			m_DestructionSimulationDocument.Get_Profiles().begin(),
			m_DestructionSimulationDocument.Get_Profiles().end(),
			[owner](const DESTRUCTION_SIMULATION_PROFILE& value)
			{
				return value.groupId == owner->groupId;
			});
		if (profile != m_DestructionSimulationDocument.Get_Profiles().end())
			Select_DestructionSimulationProfile(profile->profileId);
		else
		{
			Reset_DestructionSimulationUI();
			m_SelectedDestructionGroupId = owner->groupId;
			m_DestructionSimulationStatus =
				"Selected wall group has no simulation profile; create default";
		}
	}
	else if (!m_bDestructionAdvancedMode)
	{
		m_SelectedDestructionGroupId.clear();
		Reset_DestructionSimulationUI();
		m_DestructionSimulationStatus =
			"Selected wall is not assigned to a destruction group";
	}

	Sync_DestructionDraftFromSelection();
	if (!Refresh_DestructionHighlight())
	{
		m_DestructionStatus =
			"Wall selected and its group data was updated, but the authoring outline could not be created";
		m_bDestructionPickArmed = false;
		return true;
	}

	m_bDestructionPickArmed = false;
	m_DestructionStatus = "Selected wall " +
		std::to_string(runtimePlacementId) + " | " +
		entry->placement.assetId + " | " +
		(nullptr != source ? source : "unknown source");
	return true;
}

void Client::CMapTool::Sync_DestructionDraftFromSelection()
{
	m_SelectedDestructionBindingId.clear();
	m_bDestructionNewSettingArmed = false;
	if (0u == m_iSelectedDeployPlacementId)
		return;

	const DESTRUCTION_GROUP* owner =
		m_DestructionDocument.Find_GroupOfMember(
			m_iSelectedDeployPlacementId);
	if (nullptr == owner)
	{
		m_SelectedDestructionPatternId.clear();
		m_SelectedDestructionStageId.clear();
		m_iDestructionTriggerKind = 1;
		m_iDestructionOffsetMs = 0;
		m_iDestructionBreakingMs = 1900;
		m_bDestructionBindingEnabled = false;
		m_DestructionReceiverId[0] = '\0';
		m_fDestructionTimelineMs = 0.f;
		m_bDestructionTimelinePlaying = false;
		m_bDestructionNewSettingArmed = true;
		return;
	}
	m_SelectedDestructionGroupId = owner->groupId;

	const DESTRUCTION_BINDING* onlyBinding = nullptr;
	size_t bindingCount = 0u;
	for (const DESTRUCTION_BINDING& binding :
		m_DestructionDocument.Get_Bindings())
	{
		const DESTRUCTION_MUTATION* mutation =
			m_DestructionDocument.Find_Mutation(binding.mutationId);
		if (nullptr != mutation && mutation->groupId == owner->groupId &&
			WORLD_DESTROYABLE_STATE::FRACTURED == mutation->eTargetState)
		{
			onlyBinding = &binding;
			++bindingCount;
		}
	}
	if (1u == bindingCount && nullptr != onlyBinding)
		Load_DestructionDraftFromBinding(*onlyBinding);
	else
	{
		m_SelectedDestructionPatternId.clear();
		m_SelectedDestructionStageId.clear();
		m_iDestructionTriggerKind = 1;
		m_iDestructionOffsetMs = 0;
		m_iDestructionBreakingMs = 1900;
		m_bDestructionBindingEnabled = false;
		m_DestructionReceiverId[0] = '\0';
		m_fDestructionTimelineMs = 0.f;
		m_bDestructionTimelinePlaying = false;
		m_bDestructionNewSettingArmed = 0u == bindingCount;
	}
}

void Client::CMapTool::Load_DestructionDraftFromBinding(
	const DESTRUCTION_BINDING& binding)
{
	m_SelectedDestructionBindingId = binding.bindingId;
	m_bDestructionNewSettingArmed = false;
	m_SelectedDestructionPatternId = binding.patternId;
	m_SelectedDestructionStageId = binding.stageId;
	/* A contact binding has no pattern and no stage, so the apply path below
	already refuses to write it. Give it an index of its own anyway: showing it
	as COLLISION_IMPACT would claim the wall breaks on a scheduled impact when
	it actually breaks on collider contact. */
	m_iDestructionTriggerKind =
		DESTRUCTION_TRIGGER_KIND::STAGE_ENTER == binding.eTriggerKind ? 0 :
		DESTRUCTION_TRIGGER_KIND::STAGE_TIME == binding.eTriggerKind ? 1 :
		DESTRUCTION_TRIGGER_KIND::STAGE_EXIT == binding.eTriggerKind ? 2 :
		DESTRUCTION_TRIGGER_KIND::COLLISION_IMPACT == binding.eTriggerKind ? 3 : 4;
	m_iDestructionOffsetMs = static_cast<int32_t>(binding.iOffsetMs);
	m_bDestructionBindingEnabled = binding.isEnabled;
	strncpy_s(m_DestructionReceiverId, binding.receiverCollisionId.c_str(),
		_TRUNCATE);

	const DESTRUCTION_MUTATION* mutation =
		m_DestructionDocument.Find_Mutation(binding.mutationId);
	if (nullptr != mutation)
		m_iDestructionBreakingMs =
			static_cast<int32_t>(mutation->iBreakingDurationMs);

	const ENCOUNTER_STAGE_REFERENCE* stage =
		Find_SelectedDestructionStage();
	if (nullptr != stage)
	{
		m_fDestructionTimelineMs = static_cast<f32_t>(stage->iStartOffsetMs);
		if (DESTRUCTION_TRIGGER_KIND::STAGE_TIME == binding.eTriggerKind)
			m_fDestructionTimelineMs += static_cast<f32_t>(binding.iOffsetMs);
		else if (DESTRUCTION_TRIGGER_KIND::STAGE_EXIT == binding.eTriggerKind)
			m_fDestructionTimelineMs += static_cast<f32_t>(stage->iDurationMs);
	}
}

const Client::ENCOUNTER_STAGE_REFERENCE*
Client::CMapTool::Find_SelectedDestructionStage() const
{
	const ENCOUNTER_PATTERN_REFERENCE* pattern =
		m_SelectedDestructionPatternId.empty() ? nullptr :
		m_EncounterReference.Find_Pattern(m_SelectedDestructionPatternId);
	if (nullptr == pattern)
		return nullptr;
	const auto stage = std::find_if(pattern->stages.begin(), pattern->stages.end(),
		[this](const ENCOUNTER_STAGE_REFERENCE& value)
		{
			return value.stageId == m_SelectedDestructionStageId;
		});
	return pattern->stages.end() == stage ? nullptr : &(*stage);
}

void Client::CMapTool::Use_DestructionTimelineTime()
{
	const ENCOUNTER_PATTERN_REFERENCE* pattern =
		m_SelectedDestructionPatternId.empty() ? nullptr :
		m_EncounterReference.Find_Pattern(m_SelectedDestructionPatternId);
	if (nullptr == pattern || pattern->stages.empty())
	{
		m_DestructionStatus = "Choose a pattern before using the playhead";
		return;
	}

	const f32_t clamped = (std::clamp)(m_fDestructionTimelineMs, 0.f,
		static_cast<f32_t>(pattern->iTotalDurationMs));
	const ENCOUNTER_STAGE_REFERENCE* selected = &pattern->stages.back();
	for (const ENCOUNTER_STAGE_REFERENCE& stage : pattern->stages)
	{
		if (clamped < static_cast<f32_t>(
			stage.iStartOffsetMs + stage.iDurationMs))
		{
			selected = &stage;
			break;
		}
	}

	m_SelectedDestructionStageId = selected->stageId;
	m_iDestructionTriggerKind = 1;
	m_iDestructionOffsetMs = static_cast<int32_t>((std::clamp)(
		clamped - static_cast<f32_t>(selected->iStartOffsetMs),
		0.f, static_cast<f32_t>(selected->iDurationMs)));
	m_DestructionStatus = "Break time copied from the pattern playhead";
}

bool_t Client::CMapTool::Apply_SimpleDestructionAuthoring()
{
	const auto entry = std::find_if(
		m_DeployRuntime.Get_Entries().begin(),
		m_DeployRuntime.Get_Entries().end(),
		[this](const DEPLOY_RUNTIME_ENTRY& value)
		{
			return value.placement.runtimePlacementId ==
				m_iSelectedDeployPlacementId;
		});
	if (m_DeployRuntime.Get_Entries().end() == entry ||
		!entry->placement.destructible)
	{
		m_DestructionStatus = "Choose a destructible wall first";
		return false;
	}

	const ENCOUNTER_PATTERN_REFERENCE* pattern =
		m_SelectedDestructionPatternId.empty() ? nullptr :
		m_EncounterReference.Find_Pattern(m_SelectedDestructionPatternId);
	const ENCOUNTER_STAGE_REFERENCE* stage =
		Find_SelectedDestructionStage();
	if (nullptr == pattern || nullptr == stage)
	{
		m_DestructionStatus = "Choose a valid Valtan pattern and stage";
		return false;
	}

	const DESTRUCTION_TRIGGER_KIND trigger =
		0 == m_iDestructionTriggerKind ?
			DESTRUCTION_TRIGGER_KIND::STAGE_ENTER :
		1 == m_iDestructionTriggerKind ?
			DESTRUCTION_TRIGGER_KIND::STAGE_TIME :
		2 == m_iDestructionTriggerKind ?
			DESTRUCTION_TRIGGER_KIND::STAGE_EXIT :
			DESTRUCTION_TRIGGER_KIND::COLLISION_IMPACT;
	const uint32_t offsetMs =
		DESTRUCTION_TRIGGER_KIND::STAGE_TIME == trigger ?
			static_cast<uint32_t>((std::max)(0, m_iDestructionOffsetMs)) : 0u;
	if (offsetMs > stage->iDurationMs)
	{
		m_DestructionStatus = "Break time exceeds the selected stage duration";
		return false;
	}

	std::string receiverId;
	if (DESTRUCTION_TRIGGER_KIND::COLLISION_IMPACT == trigger)
	{
		receiverId = m_DestructionReceiverId;
		const WORLD_GAMEPLAY_PLACEMENT* receiver =
			m_WorldGameplayDocument.Find(receiverId);
		if (nullptr == receiver ||
			WORLD_PLACEMENT_KIND::COLLISION_BOX != receiver->eKind)
		{
			m_DestructionStatus =
				"Choose an existing Collision Box for impact destruction";
			return false;
		}
	}
	if (m_iDestructionBreakingMs < 0 ||
		m_iDestructionBreakingMs >
			static_cast<int32_t>(CWorldDestructionDocument::MAX_DURATION_MS))
	{
		m_DestructionStatus = "Breaking duration is outside the supported range";
		return false;
	}

	CWorldDestructionDocument staged = m_DestructionDocument;
	std::string status;
	const uint64_t wallId = entry->placement.runtimePlacementId;
	const DESTRUCTION_GROUP* owner = staged.Find_GroupOfMember(wallId);
	std::string groupId;
	if (nullptr == owner)
	{
		groupId = "destroyable.group.valtan.deploy." + std::to_string(wallId);
		if (!staged.Add_Group(groupId, status) ||
			!staged.Add_Member(groupId, wallId, status))
		{
			m_DestructionStatus = status;
			return false;
		}
	}
	else
	{
		groupId = owner->groupId;
	}

	const std::string groupHash = ToStableHex(HashStableAuthoringId(groupId));
	std::string mutationId;
	const DESTRUCTION_BINDING* selectedBinding =
		m_SelectedDestructionBindingId.empty() ? nullptr :
		staged.Find_Binding(m_SelectedDestructionBindingId);
	if (nullptr != selectedBinding)
	{
		const DESTRUCTION_MUTATION* selectedMutation =
			staged.Find_Mutation(selectedBinding->mutationId);
		if (nullptr != selectedMutation && selectedMutation->groupId == groupId &&
			WORLD_DESTROYABLE_STATE::FRACTURED ==
				selectedMutation->eTargetState)
			mutationId = selectedMutation->mutationId;
		else
		{
			m_DestructionStatus =
				"Easy Wall Editor can only edit FRACTURED wall settings";
			return false;
		}
	}
	if (mutationId.empty())
	{
		for (const DESTRUCTION_MUTATION& mutation : staged.Get_Mutations())
		{
			if (mutation.groupId != groupId ||
				WORLD_DESTROYABLE_STATE::FRACTURED != mutation.eTargetState)
			{
				continue;
			}
			if (!mutationId.empty())
			{
				m_DestructionStatus =
					"This group has multiple fracture mutations. Choose one in Advanced mode.";
				return false;
			}
			mutationId = mutation.mutationId;
		}
	}
	if (mutationId.empty())
	{
		mutationId = "mutation.valtan.group." + groupHash + ".fracture";
		DESTRUCTION_MUTATION mutation;
		mutation.mutationId = mutationId;
		mutation.groupId = groupId;
		mutation.eTargetState = WORLD_DESTROYABLE_STATE::FRACTURED;
		mutation.iBreakingDurationMs =
			static_cast<uint32_t>(m_iDestructionBreakingMs);
		if (!staged.Add_Mutation(mutation, status))
		{
			m_DestructionStatus = status;
			return false;
		}
	}
	else
	{
		const DESTRUCTION_MUTATION* current = staged.Find_Mutation(mutationId);
		if (nullptr == current)
		{
			m_DestructionStatus = "Selected fracture mutation is missing";
			return false;
		}
		DESTRUCTION_MUTATION updated = *current;
		updated.iBreakingDurationMs =
			static_cast<uint32_t>(m_iDestructionBreakingMs);
		if (!staged.Update_Mutation(updated, status))
		{
			m_DestructionStatus = status;
			return false;
		}
	}

	auto hasSameSemantic = [&staged, &groupId, &pattern, &stage, trigger,
		offsetMs, &receiverId](const DESTRUCTION_BINDING& value)
	{
		const DESTRUCTION_MUTATION* valueMutation =
			staged.Find_Mutation(value.mutationId);
		return nullptr != valueMutation && valueMutation->groupId == groupId &&
			value.patternId == pattern->patternId &&
			value.stageId == stage->stageId &&
			value.eTriggerKind == trigger &&
			value.iOffsetMs == offsetMs &&
			value.receiverCollisionId == receiverId;
	};
	std::vector<const DESTRUCTION_BINDING*> semanticMatches;
	for (const DESTRUCTION_BINDING& value : staged.Get_Bindings())
	{
		if (hasSameSemantic(value))
			semanticMatches.push_back(&value);
	}

	std::string bindingId;
	if (nullptr != selectedBinding && selectedBinding->mutationId == mutationId)
	{
		for (const DESTRUCTION_BINDING* match : semanticMatches)
		{
			if (match->bindingId != selectedBinding->bindingId)
			{
				m_DestructionStatus =
					"This group already has the same break setting";
				return false;
			}
		}
		bindingId = selectedBinding->bindingId;
	}
	else if (semanticMatches.size() > 1u)
	{
		m_DestructionStatus =
			"Duplicate break settings already exist. Resolve them in Advanced mode.";
		return false;
	}
	else if (1u == semanticMatches.size())
	{
		bindingId = semanticMatches.front()->bindingId;
	}
	else
	{
		const std::string semantic = pattern->patternId + "|" + stage->stageId +
			"|" + CWorldDestructionDocument::TriggerKind_ToString(trigger) +
			"|" + std::to_string(offsetMs) + "|" + receiverId;
		bindingId = "binding.valtan.group." + groupHash + "." +
			ToStableHex(HashStableAuthoringId(semantic));
	}

	DESTRUCTION_BINDING binding;
	binding.bindingId = bindingId;
	binding.mutationId = mutationId;
	binding.patternId = pattern->patternId;
	binding.stageId = stage->stageId;
	binding.eTriggerKind = trigger;
	binding.iOffsetMs = offsetMs;
	binding.receiverCollisionId = receiverId;
	binding.isEnabled = m_bDestructionBindingEnabled;
	const DESTRUCTION_BINDING* existing = staged.Find_Binding(bindingId);
	if (nullptr != existing &&
		(nullptr == selectedBinding ||
			existing->bindingId != selectedBinding->bindingId) &&
		!hasSameSemantic(*existing))
	{
		m_DestructionStatus =
			"Generated break setting ID collides with a different setting";
		return false;
	}
	const bool_t bindingApplied = nullptr == existing ?
		staged.Add_Binding(binding, status) :
		staged.Update_Binding(binding, status);
	if (!bindingApplied)
	{
		m_DestructionStatus = status;
		return false;
	}

	m_DestructionDocument = std::move(staged);
	m_SelectedDestructionGroupId = groupId;
	m_SelectedDestructionBindingId = bindingId;
	m_bDestructionNewSettingArmed = false;
	strncpy_s(m_DestructionGroupId, groupId.c_str(), _TRUNCATE);
	strncpy_s(m_DestructionMutationId, mutationId.c_str(), _TRUNCATE);
	strncpy_s(m_DestructionBindingId, bindingId.c_str(), _TRUNCATE);
	Refresh_DestructionHighlight();
	m_DestructionStatus = "Wall break setting staged. Press Save to write Data.";
	return true;
}

bool_t Client::CMapTool::Refresh_DestructionHighlight()
{
	Remove_WorldTriggerBoxes(m_DestructionHighlightBoxes);
	if (m_iAuthoringLevelIndex >= ETOUI(LEVEL::END))
		return false;

	/* Outline every member of the selected group, plus the single picked prop
	   while it has no group yet. The wire box is authoring presentation only. */
	std::vector<uint64_t> targets;
	const DESTRUCTION_GROUP* group = m_SelectedDestructionGroupId.empty() ?
		nullptr :
		m_DestructionDocument.Find_Group(m_SelectedDestructionGroupId);
	if (nullptr != group)
		targets = group->memberPlacementIds;
	if (0u != m_iSelectedDeployPlacementId &&
		targets.end() == std::find(targets.begin(), targets.end(),
			m_iSelectedDeployPlacementId))
	{
		targets.push_back(m_iSelectedDeployPlacementId);
	}

	vector<TRIGGER_BOX_ENTRY> staged;
	for (const uint64_t placementId : targets)
	{
		const shared_ptr<CDeployPropObject> prop =
			m_DeployRuntime.Find(placementId);
		float3_t center{};
		float3_t halfExtents{};
		if (nullptr == prop || !prop->Get_WorldBounds(center, halfExtents))
			continue;

		CTrigger_Box::TRIGGER_BOX_DESC desc{};
		desc.placementId = std::to_string(placementId);
		desc.position = center;
		desc.halfExtents = halfExtents;
		desc.yawDegrees = 0.f;
		desc.isEnabled = true;
		desc.isCollisionBox = false;
		shared_ptr<CGameObject> gameObject;
		if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(
			m_iAuthoringLevelIndex,
			TEXT("Prototype_GameObject_TriggerBox"),
			m_iAuthoringLevelIndex,
			TEXT("Layer_TriggerBoxes"),
			&desc,
			&gameObject)))
		{
			Remove_WorldTriggerBoxes(staged);
			return false;
		}
		shared_ptr<CTrigger_Box> box =
			dynamic_pointer_cast<CTrigger_Box>(gameObject);
		if (nullptr == box)
		{
			CGameInstance::Get().Remove_GameObject_from_Layer(
				m_iAuthoringLevelIndex,
				TEXT("Layer_TriggerBoxes"),
				gameObject);
			Remove_WorldTriggerBoxes(staged);
			return false;
		}
		box->Set_Selected(placementId == m_iSelectedDeployPlacementId);
		box->Set_AuthoringVisible(
			m_bOpen && TOOL_MODE::WORLD_DESTRUCTION == m_eToolMode);
		staged.push_back({ desc.placementId, std::move(box) });
	}
	m_DestructionHighlightBoxes = std::move(staged);
	return true;
}

void Client::CMapTool::Apply_DestructionPreview(const DEPLOY_PROP_STATE state)
{
	Restore_DestructionPreview();
	std::vector<uint64_t> targets;
	const DESTRUCTION_GROUP* group = m_SelectedDestructionGroupId.empty() ?
		nullptr :
		m_DestructionDocument.Find_Group(m_SelectedDestructionGroupId);
	if (nullptr != group)
		targets = group->memberPlacementIds;
	else if (0u != m_iSelectedDeployPlacementId)
		targets.push_back(m_iSelectedDeployPlacementId);
	if (targets.empty())
	{
		m_DestructionStatus = "Select a group or a wall before previewing";
		return;
	}

	/* Presentation only. The Server owns the real state and nothing here is
	   written to a document, so the preview never marks the document dirty. */
	size_t applied = 0u;
	for (const uint64_t placementId : targets)
	{
		const shared_ptr<CDeployPropObject> prop =
			m_DeployRuntime.Find(placementId);
		if (nullptr == prop)
			continue;
		m_DestructionPreviewPreviousStates.push_back(
			{ placementId, prop->Get_State() });
		if (m_DeployRuntime.Set_State(placementId, state))
			++applied;
	}
	m_DestructionStatus = "Preview applied to " + std::to_string(applied) +
		" of " + std::to_string(targets.size()) + " props";
	Refresh_DestructionHighlight();
}

void Client::CMapTool::Restore_DestructionPreview()
{
	for (const auto& previous : m_DestructionPreviewPreviousStates)
		m_DeployRuntime.Set_State(previous.first, previous.second);
	m_DestructionPreviewPreviousStates.clear();
}
