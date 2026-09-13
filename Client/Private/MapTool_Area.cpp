#include "imgui.h"
#include "MapTool_Internal.h"
#include "WorldSequenceToolPanel.h"
#include "Camera_Free.h"
#include "DataJson.h"
#include "GameInstance.h"
#include "MapEditorWorkspaceService.h"
#include "MapAssetPreview.h"
#include "DestructionSimulationController.h"
#include "Model.h"
#include "ProjectDataRoot.h"
#include "RuntimeAssetRoot.h"
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




bool_t Client::CMapTool::Ensure_AuthoringPrototypes()
{
	return Ensure_AuthoringPrototypes(m_Catalog);
}

bool_t Client::CMapTool::Admit_AuthoringPrototype(
	const MAP_ASSET_ENTRY& asset)
{
	/* This tool owns every map prototype it admits into the authoring level,
	   so its own fingerprint already answers the identity question. Probing
	   the engine instead deep clones an admitted model only to drop it, which
	   an Area the size of Bern would pay for a thousand times per switch. */
	const auto fingerprint = m_PrototypeModelPaths.find(asset.prototypeTag);
	if (fingerprint != m_PrototypeModelPaths.end())
	{
		if (fingerprint->second.lexically_normal() !=
			asset.resolvedModelPath.lexically_normal())
		{
			m_Status = "Prototype tag already belongs to another model: " +
				asset.id;
			return false;
		}
		return true;
	}

	const shared_ptr<CModel> existing =
		dynamic_pointer_cast<CModel>(
			CGameInstance::Get().Clone_Prototype(
				m_iAuthoringLevelIndex,
				asset.prototypeTag));
	if (nullptr != existing)
	{
		m_Status = "Prototype tag already belongs to another model: " +
			asset.id;
		return false;
	}

	Engine::MODEL_ASSET_LOAD_DESC loadDesc;
	loadDesc.assetRoot = CRuntimeAssetRoot::Get();
	loadDesc.meshPath = asset.resolvedModelPath;
	loadDesc.materialOverrides = asset.materialOverrides;
	auto model = CModel::Create(
		m_pDevice,
		m_pContext,
		MODEL::NONANIM,
		loadDesc,
		XMMatrixScaling(0.01f, 0.01f, 0.01f));
	if (nullptr == model ||
		FAILED(CGameInstance::Get().Add_Prototype(
			m_iAuthoringLevelIndex,
			asset.prototypeTag,
			std::move(model))))
	{
		m_Status = "Map authoring model admission failed: " + asset.id;
		return false;
	}
	m_PrototypeModelPaths.emplace(
		asset.prototypeTag,
		asset.resolvedModelPath.lexically_normal());
	return true;
}

bool_t Client::CMapTool::Ensure_AuthoringPrototypes(
	const CMapAssetCatalog& catalog)
{
	if (!catalog.Is_Ready() ||
		m_iAuthoringLevelIndex >= ETOUI(LEVEL::END) ||
		nullptr == m_pDevice || nullptr == m_pContext)
	{
		m_Status = "Map authoring prototype admission is unavailable";
		return false;
	}

	for (const MAP_ASSET_ENTRY& asset : catalog.Get_Entries())
	{
		if (!Admit_AuthoringPrototype(asset))
			return false;
	}

	m_Status = "Map authoring admitted " +
		std::to_string(catalog.Get_Entries().size()) +
		" model prototypes";
	return true;
}

bool_t Client::CMapTool::Ensure_DeployAuthoringPrototypes(
	const CDeployPropCatalog& catalog)
{
	if (!catalog.Is_Ready() ||
		m_iAuthoringLevelIndex >= ETOUI(LEVEL::END) ||
		nullptr == m_pDevice || nullptr == m_pContext)
	{
		m_Status = "DeployProp authoring prototype admission is unavailable";
		return false;
	}

	const matrix_t modelTransform =
		XMMatrixScaling(0.01f, 0.01f, 0.01f);
	const auto admitModel = [this, &modelTransform](
		const std::wstring& prototypeTag,
		const std::filesystem::path& modelPath,
		const MODEL modelKind,
		const std::string& assetId)
	{
		const shared_ptr<CModel> existing = dynamic_pointer_cast<CModel>(
			CGameInstance::Get().Clone_Prototype(
				m_iAuthoringLevelIndex, prototypeTag));
		if (nullptr != existing)
		{
			const auto fingerprint = m_PrototypeModelPaths.find(prototypeTag);
			return fingerprint != m_PrototypeModelPaths.end() &&
				fingerprint->second.lexically_normal() ==
					modelPath.lexically_normal();
		}

		auto model = CModel::Create(
			m_pDevice,
			m_pContext,
			modelKind,
			modelPath.string().c_str(),
			modelTransform);
		if (nullptr == model || FAILED(CGameInstance::Get().Add_Prototype(
			m_iAuthoringLevelIndex,
			prototypeTag,
			std::move(model))))
		{
			m_Status = "DeployProp model admission failed: " + assetId;
			return false;
		}
		m_PrototypeModelPaths.emplace(
			prototypeTag, modelPath.lexically_normal());
		return true;
	};

	for (const DEPLOY_PROP_ASSET_ENTRY& asset : catalog.Get_Assets())
	{
		const MODEL modelKind =
			DEPLOY_PROP_MODEL_KIND::ANIM == asset.kind ?
			MODEL::ANIM : MODEL::NONANIM;
		if (!admitModel(
			asset.intactPrototypeTag,
			asset.intactResolvedPath,
			modelKind,
			asset.id))
		{
			return false;
		}
		if (DEPLOY_PROP_MODEL_KIND::STATIC == asset.kind &&
			!asset.fracturedPrototypeTag.empty() &&
			!admitModel(
				asset.fracturedPrototypeTag,
				asset.fracturedResolvedPath,
				MODEL::NONANIM,
				asset.id))
		{
			return false;
		}
	}
	return true;
}

bool_t Client::CMapTool::Load_EditorAreaRegistry()
{
	const std::filesystem::path path =
		CProjectDataRoot::Resolve(L"Maps/MapCatalog.json");
	std::string text;
	std::string parseError;
	DATA_JSON_VALUE root;
	if (path.empty() || !ReadTextFile(path, text) ||
		!CDataJson::Parse(text, root, parseError) || !root.Is_Object())
	{
		m_Status = "Map editor catalog parse failed: " + parseError;
		return false;
	}

	const DATA_JSON_VALUE* schema = root.Find("schema");
	const DATA_JSON_VALUE* version = root.Find("formatVersion");
	const DATA_JSON_VALUE* areas = root.Find("areas");
	if (nullptr == schema || !schema->Is_String() ||
		schema->Get_String() != "lostark.map-catalog" ||
		nullptr == version || !version->Is_Number() ||
		version->Get_Number() != 1.0 ||
		nullptr == areas || !areas->Is_Array())
	{
		m_Status = "Map editor catalog header is invalid";
		return false;
	}

	const std::array<std::pair<const char_t*, const char_t*>, 5> targets =
	{{
		{ "LV_LOBBY_CLASSSELECT_SL00", "Character Select" },
		{ "LV_BER_BERNCASTLE", "Bern" },
		{ "LV_LUT_HEARTRB_ED", "Valtan" },
		{ "LV_LUT_MIDNIGHTC_ED", "KoukuSaydon / MidnightC ED" },
		{ "LV_SHS_RCARENA_D", "Training Map" },
	}};
	std::vector<EDITOR_AREA_DESCRIPTOR> staged;
	staged.reserve(targets.size());
	for (const auto& target : targets)
	{
		const DATA_JSON_VALUE* selected = nullptr;
		for (const DATA_JSON_VALUE& candidate : areas->Get_Array())
		{
			const DATA_JSON_VALUE* id = candidate.Find("id");
			if (candidate.Is_Object() && nullptr != id && id->Is_String() &&
				id->Get_String() == target.first)
			{
				if (nullptr != selected)
				{
					m_Status = "Duplicate MapCatalog area: " +
						std::string(target.first);
					return false;
				}
				selected = &candidate;
			}
		}
		if (nullptr == selected)
		{
			m_Status = "MapCatalog area is missing: " +
				std::string(target.first);
			return false;
		}

		EDITOR_AREA_DESCRIPTOR descriptor;
		descriptor.areaId = target.first;
		descriptor.label = target.second;
		std::string sourceCatalog;
		std::string sourcePlacements;
		if (!ReadRequiredString(*selected, "sourceCatalog", sourceCatalog) ||
			!ReadRequiredString(*selected, "sourcePlacements", sourcePlacements))
		{
			m_Status = "MapCatalog authoring source is missing: " +
				descriptor.areaId;
			return false;
		}
		descriptor.sourceCatalog = ResolveDataCatalogPath(sourceCatalog);
		descriptor.sourcePlacements =
			ResolveDataCatalogPath(sourcePlacements);
		const DATA_JSON_VALUE* sourceMaterials = selected->Find("sourceMaterials");
		const DATA_JSON_VALUE* runtimeMaterials = selected->Find("materials");
		if ((nullptr == sourceMaterials) != (nullptr == runtimeMaterials))
		{
			m_Status = "MapCatalog material source/runtime pair is incomplete: " +
				descriptor.areaId;
			return false;
		}
		if (nullptr != sourceMaterials)
		{
			const std::string expectedSource = "Data/Maps/Authoring/" +
				descriptor.areaId + "/" + descriptor.areaId + ".mapmaterials.json";
			const std::string expectedRuntime = "Client/Bin/DataFiles/Map/" +
				descriptor.areaId + ".mapmaterials.json";
			if (!sourceMaterials->Is_String() || !runtimeMaterials->Is_String() ||
				sourceMaterials->Get_String() != expectedSource ||
				runtimeMaterials->Get_String() != expectedRuntime)
			{
				m_Status = "MapCatalog material paths are not canonical: " +
					descriptor.areaId;
				return false;
			}
			descriptor.sourceMaterials = ResolveDataCatalogPath(expectedSource);
			if (descriptor.sourceMaterials.empty())
			{
				m_Status = "MapCatalog material path escapes Data root: " +
					descriptor.areaId;
				return false;
			}
		}
		const DATA_JSON_VALUE* sourceLights = selected->Find("sourceLights");
		if (nullptr != sourceLights)
		{
			if (!sourceLights->Is_String() ||
				sourceLights->Get_String().empty())
			{
				m_Status = "MapCatalog light authoring source is invalid: " +
					descriptor.areaId;
				return false;
			}
			descriptor.sourceLights = ResolveDataCatalogPath(
				sourceLights->Get_String());
			if (descriptor.sourceLights.empty())
			{
				m_Status = "MapCatalog light path escapes Data root: " +
					descriptor.areaId;
				return false;
			}
		}
		const DATA_JSON_VALUE* sourceDeployCatalog =
			selected->Find("sourceDeployCatalog");
		const DATA_JSON_VALUE* sourceDeployPlacements =
			selected->Find("sourceDeployPlacements");
		if ((nullptr == sourceDeployCatalog) !=
			(nullptr == sourceDeployPlacements) ||
			(nullptr != sourceDeployCatalog &&
				(!sourceDeployCatalog->Is_String() ||
					sourceDeployCatalog->Get_String().empty() ||
					!sourceDeployPlacements->Is_String() ||
					sourceDeployPlacements->Get_String().empty())))
		{
			m_Status = "MapCatalog deploy authoring pair is invalid: " +
				descriptor.areaId;
			return false;
		}
		if (nullptr != sourceDeployCatalog)
		{
			descriptor.sourceDeployCatalog = ResolveDataCatalogPath(
				sourceDeployCatalog->Get_String());
			descriptor.sourceDeployPlacements = ResolveDataCatalogPath(
				sourceDeployPlacements->Get_String());
			if (descriptor.sourceDeployCatalog.empty() ||
				descriptor.sourceDeployPlacements.empty())
			{
				m_Status = "MapCatalog deploy path escapes Data root: " +
					descriptor.areaId;
				return false;
			}
		}

		/* Camera shots are an optional Client presentation layer. The document
		   is named per Area and is simply absent where none is authored. */
		descriptor.cameraShotDocument = CProjectDataRoot::Resolve(
			std::filesystem::path("Maps") / "Authoring" / descriptor.areaId /
			(descriptor.areaId + ".camerashots.json"));

		if (descriptor.areaId == "LV_LOBBY_CLASSSELECT_SL00" ||
			descriptor.areaId == "LV_BER_BERNCASTLE" ||
			descriptor.areaId == "LV_LUT_HEARTRB_ED" ||
			descriptor.areaId == "LV_LUT_MIDNIGHTC_ED")
		{
			std::string source;
			std::string paint;
			if (!ReadRequiredString(*selected, "navigationSource", source) ||
				!ReadRequiredString(*selected, "navigationPaint", paint))
			{
				m_Status = "MapCatalog navigation source is missing: " +
					descriptor.areaId;
				return false;
			}
			descriptor.navigationSource = ResolveDataCatalogPath(source);
			descriptor.navigationPaint = ResolveDataCatalogPath(paint);
			descriptor.navigationPolicy =
				EDITOR_NAVIGATION_POLICY::SOURCE_PAINT;
			descriptor.allowNavigationBootstrap =
				descriptor.areaId == "LV_BER_BERNCASTLE";
		}
		if (descriptor.areaId == "LV_LUT_HEARTRB_ED")
		{
			if (descriptor.sourceLights.empty())
			{
				m_Status = "Valtan source light presentation is missing";
				return false;
			}
			std::string blockers;
			if (!ReadRequiredString(*selected, "navigationBlockers", blockers))
			{
				m_Status = "Valtan navigation blocker source is missing";
				return false;
			}
			descriptor.navigationBlockers = ResolveDataCatalogPath(blockers);
			descriptor.navigationPolicy =
				EDITOR_NAVIGATION_POLICY::SOURCE_PAINT_BLOCKERS;
			/* The editor area registry already declares the explicit areas
			   the Map Tool opens, so the destruction reference path is
			   declared the same way instead of adding a field to the shared
			   MapCatalog schema. Render_DestructionEncounterSource cross
			   checks the loaded encounterId against the boss placement of
			   this Area. */
			descriptor.encounterReference = CProjectDataRoot::Resolve(
				L"Encounters/Valtan/ValtanEncounter.json");
			descriptor.worldEventsDocument = CProjectDataRoot::Resolve(
				L"Encounters/Valtan/ValtanWorldEvents.json");
			descriptor.destructionSimulationDocument = CProjectDataRoot::Resolve(
				L"Maps/Authoring/LV_LUT_HEARTRB_ED/"
				L"LV_LUT_HEARTRB_ED.destructionsimulation.json");
			if (descriptor.encounterReference.empty() ||
				descriptor.worldEventsDocument.empty() ||
				descriptor.destructionSimulationDocument.empty())
			{
				m_Status = "Valtan encounter reference path is invalid";
				return false;
			}
		}

		if (descriptor.areaId == "LV_LOBBY_CLASSSELECT_SL00" ||
			descriptor.areaId == "LV_BER_BERNCASTLE" ||
			descriptor.areaId == "LV_LUT_HEARTRB_ED" ||
			descriptor.areaId == "LV_LUT_MIDNIGHTC_ED")
		{
			std::string gameplay;
			if (!ReadRequiredString(*selected, "gameplayDocument", gameplay))
			{
				m_Status = "MapCatalog gameplay document is missing: " +
					descriptor.areaId;
				return false;
			}
			descriptor.gameplayDocument = ResolveDataCatalogPath(gameplay);
			descriptor.gameplayPolicy =
				EDITOR_GAMEPLAY_POLICY::REQUIRED;
		}

		if (descriptor.sourceCatalog.empty() ||
			descriptor.sourcePlacements.empty() ||
			((!descriptor.sourceDeployCatalog.empty() ||
				!descriptor.sourceDeployPlacements.empty()) &&
				(descriptor.sourceDeployCatalog.empty() ||
					descriptor.sourceDeployPlacements.empty())) ||
			(EDITOR_NAVIGATION_POLICY::NONE != descriptor.navigationPolicy &&
				(descriptor.navigationSource.empty() ||
					descriptor.navigationPaint.empty())) ||
			(EDITOR_GAMEPLAY_POLICY::REQUIRED == descriptor.gameplayPolicy &&
				descriptor.gameplayDocument.empty()))
		{
			m_Status = "MapCatalog path escapes Data root: " +
				descriptor.areaId;
			return false;
		}
		staged.push_back(std::move(descriptor));
	}

	m_EditorAreas = std::move(staged);
	return true;
}

const Client::CMapTool::EDITOR_AREA_DESCRIPTOR*
Client::CMapTool::Get_ActiveEditorArea() const
{
	return m_iActiveEditorArea < m_EditorAreas.size() ?
		&m_EditorAreas[m_iActiveEditorArea] : nullptr;
}

bool_t Client::CMapTool::Begin_EditorAreaSwitch(const size_t descriptorIndex)
{
	if (descriptorIndex >= m_EditorAreas.size() ||
		m_iAuthoringLevelIndex != ETOUI(LEVEL::DEVELOPMENT) ||
		!CMapEditorWorkspaceService::Is_Active() ||
		nullptr == m_pDevice || nullptr == m_pContext)
	{
		m_Status = "Map editor Area switch is unavailable";
		return false;
	}
	if (nullptr != m_pWorldSequenceToolPanel && m_Catalog.Is_Ready())
	{
		m_pWorldSequenceToolPanel->Stop_AndRestore(
			m_iAuthoringLevelIndex, m_Catalog, m_Placements,
			m_DeployRuntime);
		if (m_pWorldSequenceToolPanel->Is_PreviewActive())
		{
			m_Status = m_pWorldSequenceToolPanel->Get_Status();
			return false;
		}
	}

	const EDITOR_AREA_DESCRIPTOR& descriptor =
		m_EditorAreas[descriptorIndex];
	const std::filesystem::path worldSequencePath =
		descriptor.sourcePlacements.parent_path() /
		std::filesystem::path(descriptor.areaId + ".worldsequences.json");
	std::string recoveryStatus;
	SCOPED_AUTHORING_SAVE_LOCK authoringLock;
	if (!authoringLock.Acquire(worldSequencePath, recoveryStatus) ||
		!RecoverAuthoringTransactionUnderLock(
		descriptor.sourcePlacements, worldSequencePath, recoveryStatus))
	{
		m_Status = recoveryStatus;
		return false;
	}
	std::error_code error;
	if (!std::filesystem::is_regular_file(descriptor.sourceCatalog, error) ||
		error ||
		!std::filesystem::is_regular_file(descriptor.sourcePlacements, error) ||
		error)
	{
		m_Status = "Required authoring map source is missing: " +
			descriptor.areaId;
		return false;
	}

	/* Only the model catalog is read here so the admission can start. Every
	   other authoring document still loads inside the one atomic
	   Switch_EditorArea transaction that runs once the prototypes exist. */
	CMapAssetCatalog stagedCatalog;
	if (!stagedCatalog.Load_Source(
		descriptor.sourceCatalog,
		descriptor.sourcePlacements,
		descriptor.areaId,
		descriptor.sourceMaterials))
	{
		m_Status = stagedCatalog.Get_Status();
		return false;
	}

	m_EditorAreaPreload.iDescriptorIndex = descriptorIndex;
	m_EditorAreaPreload.Catalog = std::move(stagedCatalog);
	m_EditorAreaPreload.iNextEntry = 0;
	Report_EditorAreaPreloadProgress();
	return true;
}

void Client::CMapTool::Report_EditorAreaPreloadProgress()
{
	if (!m_EditorAreaPreload.Is_Active())
		return;

	m_Status = "Preparing " +
		m_EditorAreas[m_EditorAreaPreload.iDescriptorIndex].label + ": " +
		std::to_string(m_EditorAreaPreload.iNextEntry) + " / " +
		std::to_string(m_EditorAreaPreload.Catalog.Get_Entries().size()) +
		" model prototypes";
}

void Client::CMapTool::Update_EditorAreaPreload()
{
	if (!m_EditorAreaPreload.Is_Active())
		return;

	if (m_iAuthoringLevelIndex != ETOUI(LEVEL::DEVELOPMENT) ||
		!CMapEditorWorkspaceService::Is_Active() ||
		m_EditorAreaPreload.iDescriptorIndex >= m_EditorAreas.size())
	{
		m_EditorAreaPreload = {};
		m_Status = "Map editor Area switch was cancelled";
		return;
	}

	const std::vector<MAP_ASSET_ENTRY>& entries =
		m_EditorAreaPreload.Catalog.Get_Entries();
	const auto admissionStart = std::chrono::steady_clock::now();
	while (m_EditorAreaPreload.iNextEntry < entries.size())
	{
		if (!Admit_AuthoringPrototype(
			entries[m_EditorAreaPreload.iNextEntry]))
		{
			/* The failing asset already named itself in the status and no
			   Area state was replaced, so the editor keeps the loaded Area. */
			m_EditorAreaPreload = {};
			return;
		}
		++m_EditorAreaPreload.iNextEntry;
		if (std::chrono::steady_clock::now() - admissionStart >=
			EDITOR_AREA_ADMISSION_FRAME_BUDGET)
		{
			break;
		}
	}

	if (m_EditorAreaPreload.iNextEntry < entries.size())
	{
		Report_EditorAreaPreloadProgress();
		return;
	}

	const size_t iCommitIndex = m_EditorAreaPreload.iDescriptorIndex;
	m_EditorAreaPreload = {};
	Switch_EditorArea(iCommitIndex);
}

bool_t Client::CMapTool::Switch_EditorArea(const size_t descriptorIndex)
{
	if (descriptorIndex >= m_EditorAreas.size() ||
		m_iAuthoringLevelIndex != ETOUI(LEVEL::DEVELOPMENT) ||
		!CMapEditorWorkspaceService::Is_Active())
	{
		m_Status = "Map editor Area switch is unavailable";
		return false;
	}

	const EDITOR_AREA_DESCRIPTOR& descriptor =
		m_EditorAreas[descriptorIndex];
	const std::filesystem::path worldSequencePath =
		descriptor.sourcePlacements.parent_path() /
		std::filesystem::path(descriptor.areaId + ".worldsequences.json");
	std::string recoveryStatus;
	SCOPED_AUTHORING_SAVE_LOCK authoringLock;
	if (!authoringLock.Acquire(worldSequencePath, recoveryStatus) ||
		!RecoverAuthoringTransactionUnderLock(
		descriptor.sourcePlacements, worldSequencePath, recoveryStatus))
	{
		m_Status = recoveryStatus;
		return false;
	}
	std::error_code error;
	if (!std::filesystem::is_regular_file(descriptor.sourceCatalog, error) ||
		error ||
		!std::filesystem::is_regular_file(descriptor.sourcePlacements, error) ||
		error)
	{
		m_Status = "Required authoring map source is missing: " +
			descriptor.areaId;
		return false;
	}

	CMapAssetCatalog stagedCatalog;
	if (!stagedCatalog.Load_Source(
		descriptor.sourceCatalog,
		descriptor.sourcePlacements,
		descriptor.areaId,
		descriptor.sourceMaterials))
	{
		m_Status = stagedCatalog.Get_Status();
		return false;
	}

	std::vector<MAP_PLACEMENT_RECORD> records;
	std::string stagedStatus;
	if (!CMapPlacementDocument::Read(
		descriptor.sourcePlacements,
		stagedCatalog,
		records,
		stagedStatus))
	{
		m_Status = stagedStatus;
		return false;
	}
	/* The sequence document reference-validates Deploy animation tracks, so
	   it is loaded once this Area's Deploy runtime has been staged below. */
	auto stagedWorldSequencePanel =
		std::make_unique<CWorldSequenceToolPanel>();

	CWorldGameplayDocument stagedWorld;
	CSpawnGroupDocument stagedSpawnGroups;
	if (EDITOR_GAMEPLAY_POLICY::REQUIRED == descriptor.gameplayPolicy)
	{
		error.clear();
		if (!std::filesystem::is_regular_file(
			descriptor.gameplayDocument, error) || error ||
			!stagedWorld.Load(
				descriptor.gameplayDocument,
				descriptor.areaId,
				stagedStatus))
		{
			m_Status = error ?
				"Could not inspect required gameplay document" : stagedStatus;
			return false;
		}
		const std::filesystem::path spawnGroupsPath =
			descriptor.gameplayDocument.parent_path() / L"SpawnGroups.world.json";
		if (!stagedSpawnGroups.Load(
			spawnGroupsPath, descriptor.areaId, stagedStatus))
		{
			m_Status = stagedStatus;
			return false;
		}
	}

	CNavGridPaintDocument stagedNavigation;
	CNavRuntimeBlockerDocument stagedBlockers;
	bool_t navigationLoaded = false;
	if (EDITOR_NAVIGATION_POLICY::NONE != descriptor.navigationPolicy)
	{
		error.clear();
		const bool_t hasSource = std::filesystem::is_regular_file(
			descriptor.navigationSource, error);
		if (IsFileInspectionFailure(error) ||
			(!hasSource && !descriptor.allowNavigationBootstrap))
		{
			m_Status = "Required navigation source is missing: " +
				descriptor.areaId;
			return false;
		}
		if (hasSource)
		{
			if (!stagedNavigation.Load(
				descriptor.navigationSource,
				descriptor.navigationPaint,
				stagedStatus) ||
				stagedNavigation.Get_Desc().areaId != descriptor.areaId ||
				!stagedBlockers.Load(
					descriptor.navigationBlockers,
					stagedNavigation.Get_Desc(),
					stagedStatus))
			{
				m_Status = stagedStatus;
				return false;
			}
			navigationLoaded = true;
		}
	}

	/* Pure data document: an Area without a world events path simply has no
	   destruction authoring, a missing file for Valtan starts empty so the
	   first Save can create it, and a corrupt file fails the switch like the
	   other authoring documents do. */
	CWorldDestructionDocument stagedDestruction;
	CEncounterPatternReference stagedEncounterReference;
	std::string stagedEncounterStatus;
	if (!descriptor.worldEventsDocument.empty())
	{
		if (descriptor.encounterReference.empty() ||
			!stagedEncounterReference.Load(
				descriptor.encounterReference, stagedEncounterStatus))
		{
			m_Status = descriptor.encounterReference.empty() ?
				"World destruction requires an encounter reference" :
				stagedEncounterStatus;
			return false;
		}
		std::error_code destructionError;
		const bool_t hasDocument = std::filesystem::is_regular_file(
			descriptor.worldEventsDocument, destructionError);
		if (IsFileInspectionFailure(destructionError))
		{
			m_Status = "World destruction document is unreadable: " +
				descriptor.areaId;
			return false;
		}
		std::string destructionStatus;
		if (!hasDocument)
		{
			stagedDestruction.Reset_Empty();
		}
		else if (!stagedDestruction.Load(
			descriptor.worldEventsDocument,
			descriptor.areaId,
			"ENCOUNTER_VALTAN",
			destructionStatus))
		{
			m_Status = destructionStatus;
			return false;
		}
	}

	CDestructionSimulationDocument stagedSimulation;
	if (!descriptor.destructionSimulationDocument.empty())
	{
		std::error_code simulationError;
		const bool_t hasSimulation = std::filesystem::is_regular_file(
			descriptor.destructionSimulationDocument, simulationError);
		if (IsFileInspectionFailure(simulationError))
		{
			m_Status = "Destruction simulation document is unreadable: " +
				descriptor.areaId;
			return false;
		}
		std::string simulationStatus;
		if (!hasSimulation)
		{
			stagedSimulation.Reset_Empty();
		}
		else if (!stagedSimulation.Load(
			descriptor.destructionSimulationDocument,
			descriptor.areaId,
			simulationStatus))
		{
			m_Status = simulationStatus;
			return false;
		}
	}
	if (stagedSimulation.Is_Ready() && stagedDestruction.Is_Ready() &&
		!stagedSimulation.Validate_GroupReferences(
			stagedDestruction, stagedStatus))
	{
		m_Status = stagedStatus;
		return false;
	}
	shared_ptr<CMapLightPresentationRuntime> stagedMapLightPresentation;
	if (!descriptor.sourceLights.empty())
	{
		stagedMapLightPresentation =
			make_shared<CMapLightPresentationRuntime>();
		if (!stagedMapLightPresentation->Load(
			descriptor.sourceLights, descriptor.areaId))
		{
			m_Status = stagedMapLightPresentation->Get_Status();
			return false;
		}
	}

	/* Read-only here: a rejected shot document must never block the Area, it
	   only leaves the list empty with a reported reason. */
	(void)Load_CameraShots(descriptor);

	if (!Ensure_AuthoringPrototypes(stagedCatalog))
		return false;
	const bool_t stagedDebrisPrototypesReady =
		!descriptor.destructionSimulationDocument.empty() &&
		Ensure_DestructionDebrisAuthoringPrototypes();
	const std::string stagedDebrisPrototypeStatus =
		descriptor.destructionSimulationDocument.empty() ?
		"PROJECT_AUTHORED debris is not declared for this Area" : m_Status;

	CMapAssetCatalog previousCatalog = m_Catalog;
	m_Catalog = stagedCatalog;
	std::vector<PLACED_ENTRY> stagedPlacements;
	std::vector<STATIC_BATCH_ENTRY> stagedBatches;
	if (!Stage_PlacementRuntime(records, stagedPlacements, stagedBatches))
	{
		Remove_PlacementRuntime(stagedPlacements, stagedBatches);
		m_Catalog = std::move(previousCatalog);
		m_Status = "Map Area runtime stage rolled back: " + descriptor.areaId;
		return false;
	}
	CDeployPropRuntime stagedDeployRuntime;
	if (!Stage_DeployProps(descriptor, stagedDeployRuntime))
	{
		Remove_PlacementRuntime(stagedPlacements, stagedBatches);
		m_Catalog = std::move(previousCatalog);
		return false;
	}
	if (!stagedWorldSequencePanel->Load_Area(
		worldSequencePath, descriptor.sourcePlacements, descriptor.areaId,
		stagedCatalog, records, stagedDeployRuntime,
		stagedStatus))
	{
		Remove_PlacementRuntime(stagedPlacements, stagedBatches);
		stagedDeployRuntime.Clear();
		m_Catalog = std::move(previousCatalog);
		m_Status = stagedStatus;
		return false;
	}
	std::vector<MAP_PLACEMENT_RECORD> stableLinkedRecords;
	if (!CMapPlacementDocument::Read(descriptor.sourcePlacements,
		stagedCatalog, stableLinkedRecords, stagedStatus) ||
		!AreExactlySamePlacementRecords(records, stableLinkedRecords) ||
		!stagedWorldSequencePanel->Matches_LinkedSourceBaseline(stagedStatus))
	{
		Remove_PlacementRuntime(stagedPlacements, stagedBatches);
		stagedDeployRuntime.Clear();
		m_Catalog = std::move(previousCatalog);
		m_Status = "Linked map/sequence source changed while the Area was loading: " +
			stagedStatus;
		return false;
	}
	if (stagedDestruction.Is_Ready() &&
		!Validate_DestructionExternalReferences(
			stagedDestruction,
			stagedDeployRuntime,
			stagedBlockers,
			stagedWorld,
			stagedEncounterReference,
			stagedStatus))
	{
		Remove_PlacementRuntime(stagedPlacements, stagedBatches);
		stagedDeployRuntime.Clear();
		m_Catalog = std::move(previousCatalog);
		m_Status = stagedStatus;
		return false;
	}
	vector<TRIGGER_BOX_ENTRY> stagedTriggerBoxes;
	if (!Stage_WorldTriggerBoxes(stagedWorld, stagedTriggerBoxes))
	{
		Remove_PlacementRuntime(stagedPlacements, stagedBatches);
		stagedDeployRuntime.Clear();
		m_Catalog = std::move(previousCatalog);
		return false;
	}
	vector<NPC_PREVIEW_ENTRY> stagedNpcPreviews;
	if (!Stage_WorldNpcPreviews(
		stagedWorld, stagedNpcPreviews, &stagedNavigation, &stagedBlockers))
	{
		Remove_PlacementRuntime(stagedPlacements, stagedBatches);
		stagedDeployRuntime.Clear();
		Remove_WorldTriggerBoxes(stagedTriggerBoxes);
		m_Catalog = std::move(previousCatalog);
		return false;
	}
	vector<TRIGGER_BOX_ENTRY> stagedSpawnAnchorBoxes;
	if (!Stage_SpawnAnchorBoxes(stagedSpawnGroups, stagedSpawnAnchorBoxes))
	{
		Remove_PlacementRuntime(stagedPlacements, stagedBatches);
		stagedDeployRuntime.Clear();
		Remove_WorldTriggerBoxes(stagedTriggerBoxes);
		Remove_WorldNpcPreviews(stagedNpcPreviews);
		m_Catalog = std::move(previousCatalog);
		return false;
	}

	/* The staged runtime owns preview seams into the current Deploy runtime.
	   Release them before the Area transaction move-assigns that owner. The
	   previous catalog must be used because m_Catalog currently stages the
	   destination Area. */
	if (nullptr != m_pWorldSequenceToolPanel && previousCatalog.Is_Ready())
	{
		m_pWorldSequenceToolPanel->Stop_AndRestore(
			m_iAuthoringLevelIndex, previousCatalog, m_Placements,
			m_DeployRuntime);
		if (m_pWorldSequenceToolPanel->Is_PreviewActive())
		{
			const std::string restoreFailure =
				m_pWorldSequenceToolPanel->Get_Status();
			Remove_PlacementRuntime(stagedPlacements, stagedBatches);
			stagedDeployRuntime.Clear();
			Remove_WorldTriggerBoxes(stagedTriggerBoxes);
			Remove_WorldNpcPreviews(stagedNpcPreviews);
			Remove_WorldTriggerBoxes(stagedSpawnAnchorBoxes);
			m_Catalog = std::move(previousCatalog);
			m_Status = restoreFailure;
			return false;
		}
	}
	if (nullptr != m_pDestructionSimulationController)
		m_pDestructionSimulationController->Clear();
	m_bDestructionSimulationClearRequested = false;
	Remove_PlacementRuntime(m_Placements, m_StaticBatches);
	Remove_WorldTriggerBoxes(m_WorldTriggerBoxes);
	Remove_WorldTriggerBoxes(m_SpawnAnchorBoxes);
	Remove_WorldNpcPreviews(m_WorldNpcPreviews);
	m_Placements = std::move(stagedPlacements);
	m_StaticBatches = std::move(stagedBatches);
	m_pWorldSequenceToolPanel = std::move(stagedWorldSequencePanel);
	m_DeployRuntime = std::move(stagedDeployRuntime);
	m_WorldGameplayDocument = std::move(stagedWorld);
	m_WorldTriggerBoxes = std::move(stagedTriggerBoxes);
	m_WorldNpcPreviews = std::move(stagedNpcPreviews);
	m_SpawnGroupDocument = std::move(stagedSpawnGroups);
	m_SpawnAnchorBoxes = std::move(stagedSpawnAnchorBoxes);
	m_NavigationDocument = std::move(stagedNavigation);
	m_RuntimeBlockerDocument = std::move(stagedBlockers);
	m_DestructionDocument = std::move(stagedDestruction);
	m_EncounterReference = std::move(stagedEncounterReference);
	m_DestructionSimulationDocument = std::move(stagedSimulation);
	m_pMapLightPresentation = std::move(stagedMapLightPresentation);
	m_bMapLightSubmissionFailureReported = false;
	m_WorldEventsPath = descriptor.worldEventsDocument;
	m_NavigationSourcePath = descriptor.navigationSource;
	m_NavigationPaintPath = descriptor.navigationPaint;
	m_RuntimeBlockerPath = descriptor.navigationBlockers;
	m_NavigationRuntimePath.clear();
	m_NavigationBakeDesc = navigationLoaded ?
		m_NavigationDocument.Get_BakeDesc() : NAVGRID_BAKE_DESC{};
	m_iActiveEditorArea = descriptorIndex;
	m_bDestructionDebrisPrototypesReady = stagedDebrisPrototypesReady;
	m_DestructionDebrisPrototypeStatus = stagedDebrisPrototypeStatus;
	m_iPendingEditorArea = SIZE_MAX;
	m_isEditorAreaSwitchPending = false;
	m_iSelectedPlacementId = 0;
	m_SelectedWorldPlacementId.clear();
	m_bWorldNpcContinuousPlacement = false;
	m_WorldNpcBrushPreset.reset();
	m_bWorldNpcWaypointPickArmed = false;
	m_bWorldNpcBatchCenterPickArmed = false;
	m_bWorldNpcBatchCenterValid = false;
	m_WorldNpcBehaviorDraftPlacementId.clear();
	m_WorldNpcBehaviorDraft.reset();
	m_bWorldNpcBehaviorDraftDirty = false;
	m_WorldNpcBatchArchetypePool.clear();
	m_WorldNpcBatchDraft.clear();
	m_iWorldNpcBatchDraftBaseRevision = 0;
	m_bWorldNpcBatchCopySelectedBehavior = false;
	m_SelectedSpawnAnchorId.clear();
	m_SelectedSpawnGroupId.clear();
	m_SelectedSpawnWaveId.clear();
	m_SelectedAssetId.clear();
	Remove_WorldTriggerBoxes(m_DestructionHighlightBoxes);
	m_SelectedDestructionGroupId.clear();
	m_SelectedDestructionBindingId.clear();
	m_SelectedDestructionStageId.clear();
	m_SelectedDestructionPatternId.clear();
	m_iSelectedDeployPlacementId = 0;
	m_bDeployDirty = false;
	m_bAnimatedPropPlacementArmed = false;
	m_SelectedDeployAssetId.clear();
	m_AnimatedPropFilter[0] = '\0';
	m_iSelectedAnimatedPropPlacementId = 0;
	Sync_AnimatedPropTransformDraft();
	m_DestructionPreviewPreviousStates.clear();
	m_bDestructionPickArmed = false;
	m_bDestructionAddMemberArmed = false;
	m_bDestructionNewSettingArmed = false;
	m_bDestructionTimelinePlaying = false;
	m_fDestructionTimelineMs = 0.f;
	m_EncounterReferenceStatus = descriptor.encounterReference.empty() ?
		"Active Area declares no encounter reference" :
		stagedEncounterStatus;
	Reset_DestructionSimulationUI();
	if (!m_DestructionSimulationDocument.Get_Profiles().empty())
	{
		const DESTRUCTION_SIMULATION_PROFILE& firstProfile =
			m_DestructionSimulationDocument.Get_Profiles().front();
		m_SelectedDestructionGroupId = firstProfile.groupId;
		Select_DestructionSimulationProfile(firstProfile.profileId);
		Refresh_DestructionHighlight();
	}
	if (!m_bDestructionDebrisPrototypesReady &&
		!descriptor.destructionSimulationDocument.empty())
	{
		m_DestructionSimulationStatus = m_DestructionDebrisPrototypeStatus;
	}
	m_DestructionStatus = descriptor.worldEventsDocument.empty() ?
		"World destruction authoring disabled for this Area" :
		"World destruction authoring ready";
	m_iNextPlacementId = 1;
	for (const PLACED_ENTRY& entry : m_Placements)
	{
		if ((entry.record.transformSource == "editor" ||
			entry.record.transformSource == "legacy") &&
			entry.record.placementId <=
				CMapPlacementDocument::MAX_EDITOR_PLACEMENT_ID)
		{
			m_iNextPlacementId = (std::max)(
				m_iNextPlacementId, entry.record.placementId + 1);
		}
	}
	m_bDirty = false;
	m_bWorldGameplayDirty = false;
	m_bSpawnGroupsDirty = false;
	m_WorldGameplayStatus =
		EDITOR_GAMEPLAY_POLICY::REQUIRED == descriptor.gameplayPolicy ?
		"Gameplay authoring ready" : "Gameplay authoring disabled for this Area";
	m_NavigationStatus =
		EDITOR_NAVIGATION_POLICY::NONE == descriptor.navigationPolicy ?
		"Navigation authoring disabled for this Area" :
		(navigationLoaded ? "Navigation authoring ready" :
			"Navigation bootstrap: place Nav Bounds and Bake");
	m_Status = "Active editor Area: " + descriptor.label + " (" +
		descriptor.areaId + ") / " + std::to_string(m_Placements.size()) +
		" placements. Runtime publish is separate.";
	Set_EnvironmentPhase(ENVIRONMENT_PHASE::BASELINE);
	Rebuild_EditorSublevelJumps();

	if (!Focus_ActiveEditorAreaCamera())
		m_Status += " Camera focus unavailable: " + m_CameraStatus;
	return true;
}

void Client::CMapTool::Handle_LevelTransition(
	uint32_t currentLevelIndex,
	bool_t isMapAuthoringLevel)
{
	const uint32_t targetLevelIndex = isMapAuthoringLevel ?
		currentLevelIndex : ETOUI(LEVEL::END);
	if (targetLevelIndex == m_iAuthoringLevelIndex)
		return;
	if (nullptr != m_pWorldSequenceToolPanel && m_Catalog.Is_Ready())
	{
		m_pWorldSequenceToolPanel->Stop_AndRestore(
			m_iAuthoringLevelIndex, m_Catalog, m_Placements,
			m_DeployRuntime);
	}
	if (nullptr != m_pDestructionSimulationController)
		m_pDestructionSimulationController->Clear();
	Reset_DestructionSimulationUI();

	m_ePlacementState = PLACEMENT_STATE::IDLE;
	m_bWorldGameplayPlacementArmed = false;
	m_bWorldNpcContinuousPlacement = false;
	m_WorldNpcBrushPreset.reset();
	m_bWorldTriggerTargetPickArmed = false;
	m_bWorldNpcWaypointPickArmed = false;
	m_bWorldNpcBatchCenterPickArmed = false;
	m_bWorldNpcBatchCenterValid = false;
	m_WorldNpcBehaviorDraftPlacementId.clear();
	m_WorldNpcBehaviorDraft.reset();
	m_bWorldNpcBehaviorDraftDirty = false;
	m_WorldNpcBatchArchetypePool.clear();
	m_WorldNpcBatchDraft.clear();
	m_iWorldNpcBatchDraftBaseRevision = 0;
	m_bWorldNpcBatchCopySelectedBehavior = false;
	m_bSpawnAnchorPlacementArmed = false;
	m_bAnimatedPropPlacementArmed = false;
	m_SelectedWorldPlacementId.clear();
	m_SelectedSpawnAnchorId.clear();
	m_SelectedSpawnGroupId.clear();
	m_SelectedSpawnWaveId.clear();
	m_iSelectedPlacementId = 0;
	m_SelectedAssetId.clear();
	m_SelectedDeployAssetId.clear();
	m_AnimatedPropFilter[0] = '\0';
	m_iSelectedAnimatedPropPlacementId = 0;
	Sync_AnimatedPropTransformDraft();
	m_pAssetTestCamera.reset();
	if (nullptr != m_pAssetPreview)
		m_pAssetPreview->Reset_LevelResources();

	m_Placements.clear();
	m_StaticBatches.clear();
	m_DeployRuntime.Reset_ClearedLevelTracking();
	m_WorldTriggerBoxes.clear();
	m_SpawnAnchorBoxes.clear();
	m_iNextPlacementId = 1;
	m_bDirty = false;
	m_bDeployDirty = false;
	m_bWorldGameplayDirty = false;
	m_bSpawnGroupsDirty = false;
	m_SpawnGroupDocument.Reset();
	m_DestructionSimulationDocument.Clear();
	m_pMapLightPresentation.reset();
	m_bMapLightSubmissionFailureReported = false;
	if (nullptr != m_pWorldSequenceToolPanel)
		m_pWorldSequenceToolPanel->Reset();
	m_Catalog = CMapAssetCatalog{};
	m_EditorAreas.clear();
	m_iActiveEditorArea = SIZE_MAX;
	m_iPendingEditorArea = SIZE_MAX;
	m_isEditorAreaSwitchPending = false;
	m_isEditorExitPending = false;
	m_PrototypeModelPaths.clear();
	m_EditorAreaPreload = {};
	m_bDestructionDebrisPrototypesReady = false;
	/* Prototypes live under the Level index that is being left. */
	m_bWorldObjectPrototypeReady = false;
	m_DestructionDebrisPrototypeStatus =
		"PROJECT_AUTHORED debris models are not admitted";
	m_iAuthoringLevelIndex = targetLevelIndex;

	if (!isMapAuthoringLevel)
	{
		m_Status = "Current level has no map Area to author";
		m_WorldGameplayStatus = "Current level has no gameplay Area";
		m_NavigationStatus = "Current level has no navigation Area";
		m_CameraStatus = "Current level has no authoring camera";
		return;
	}

	Find_AssetTestCamera();
	if (!Load_EditorAreaRegistry() || m_EditorAreas.empty())
		return;
	Switch_EditorArea(0);
}
bool_t Client::CMapTool::Find_AssetTestCamera()
{
	const shared_ptr<CGameObject> gameObject =
		CGameInstance::Get().Get_GameObject(
			m_iAuthoringLevelIndex,
			TEXT("Layer_Camera"),
			0);
	const shared_ptr<CCamera_Free> camera =
		dynamic_pointer_cast<CCamera_Free>(gameObject);
	if (nullptr == camera)
	{
		m_pAssetTestCamera.reset();
		m_CameraStatus = "ASSET_TEST camera is unavailable";
		return false;
	}

	m_pAssetTestCamera = camera;
	m_CameraStatus = "Camera ready";
	return true;
}

bool_t Client::CMapTool::Focus_ActiveEditorAreaCamera()
{
	const EDITOR_AREA_DESCRIPTOR* descriptor = Get_ActiveEditorArea();
	if (nullptr == descriptor)
	{
		m_CameraStatus = "Select an Area before focusing the camera";
		return false;
	}

	float3_t center{};
	f32_t radius = 0.f;
	bool_t hasFrame = false;
	/* Product spawn data is the stable authoring focus. Distant backdrop
	   meshes must not pull the editor camera away from the playable entry.
	   Any Area that declares player spawns uses them; one without a gameplay
	   document falls through to the placement bounds below. */
	hasFrame = TryBuildGameplaySpawnFrame(
		m_WorldGameplayDocument,
		center,
		radius);

	if (!hasFrame && !m_Placements.empty())
	{
		float3_t minimum = m_Placements.front().record.position;
		float3_t maximum = minimum;
		for (const PLACED_ENTRY& entry : m_Placements)
		{
			minimum.x = (std::min)(minimum.x, entry.record.position.x);
			minimum.y = (std::min)(minimum.y, entry.record.position.y);
			minimum.z = (std::min)(minimum.z, entry.record.position.z);
			maximum.x = (std::max)(maximum.x, entry.record.position.x);
			maximum.y = (std::max)(maximum.y, entry.record.position.y);
			maximum.z = (std::max)(maximum.z, entry.record.position.z);
		}
		center = float3_t(
			(minimum.x + maximum.x) * 0.5f,
			(minimum.y + maximum.y) * 0.5f,
			(minimum.z + maximum.z) * 0.5f);
		radius = (std::max)(50.f,
			(std::max)(maximum.x - minimum.x, maximum.z - minimum.z) * 0.35f);
		hasFrame = true;
	}
	if (!hasFrame)
	{
		m_CameraStatus = "Active Area has no valid camera focus target";
		return false;
	}

	shared_ptr<CCamera_Free> camera = m_pAssetTestCamera.lock();
	if (nullptr == camera)
	{
		if (!Find_AssetTestCamera())
			return false;
		camera = m_pAssetTestCamera.lock();
	}
	if (nullptr == camera)
	{
		m_CameraStatus = "ASSET_TEST camera reacquisition failed";
		return false;
	}

	camera->Frame_Area(center, radius);
	m_CameraStatus = "Camera focused on " + descriptor->label;
	return true;
}

/* An extracted Area can span several authored sublevels that sit thousands of
   units apart, so walking between them is impractical. Grouping the committed
   placements by their authored sourceLevel gives one framing target per space
   without hard-coding any single Area's coordinates. */
void Client::CMapTool::Rebuild_EditorSublevelJumps()
{
	m_EditorSublevelJumps.clear();

	struct SUBLEVEL_BOUNDS final
	{
		float3_t minimum{};
		float3_t maximum{};
		size_t count = 0u;
	};
	std::map<std::string, SUBLEVEL_BOUNDS> bounds;
	for (const PLACED_ENTRY& entry : m_Placements)
	{
		if (entry.record.sourceLevel.empty())
			continue;
		const float3_t& position = entry.record.position;
		const auto [iter, inserted] = bounds.try_emplace(
			entry.record.sourceLevel, SUBLEVEL_BOUNDS{ position, position, 0u });
		SUBLEVEL_BOUNDS& value = iter->second;
		if (!inserted)
		{
			value.minimum.x = (std::min)(value.minimum.x, position.x);
			value.minimum.y = (std::min)(value.minimum.y, position.y);
			value.minimum.z = (std::min)(value.minimum.z, position.z);
			value.maximum.x = (std::max)(value.maximum.x, position.x);
			value.maximum.y = (std::max)(value.maximum.y, position.y);
			value.maximum.z = (std::max)(value.maximum.z, position.z);
		}
		++value.count;
	}

	/* A single group is the whole Area, which the Focus Area button already
	   frames, so publishing one shortcut for it would only add a redundant key. */
	if (bounds.size() < 2u)
		return;

	for (const auto& [sourceLevel, value] : bounds)
	{
		EDITOR_SUBLEVEL_JUMP jump;
		jump.label = sourceLevel;
		jump.center = float3_t(
			(value.minimum.x + value.maximum.x) * 0.5f,
			(value.minimum.y + value.maximum.y) * 0.5f,
			(value.minimum.z + value.maximum.z) * 0.5f);
		/* Same framing allowance the Area focus uses, so a jump lands at a
		   comparable distance instead of inside the geometry. */
		jump.radius = (std::max)(50.f,
			(std::max)(value.maximum.x - value.minimum.x,
				value.maximum.z - value.minimum.z) * 0.35f);
		jump.placementCount = value.count;
		m_EditorSublevelJumps.push_back(std::move(jump));
	}
}

bool_t Client::CMapTool::Jump_ToEditorSublevel(const size_t jumpIndex)
{
	if (jumpIndex >= m_EditorSublevelJumps.size())
		return false;

	shared_ptr<CCamera_Free> camera = m_pAssetTestCamera.lock();
	if (nullptr == camera)
	{
		if (!Find_AssetTestCamera())
			return false;
		camera = m_pAssetTestCamera.lock();
	}
	if (nullptr == camera)
	{
		m_CameraStatus = "ASSET_TEST camera reacquisition failed";
		return false;
	}

	const EDITOR_SUBLEVEL_JUMP& jump = m_EditorSublevelJumps[jumpIndex];
	camera->Frame_Area(jump.center, jump.radius);
	m_CameraStatus = "Camera jumped to " + jump.label + " (" +
		std::to_string(jump.placementCount) + " placements)";
	return true;
}

/* Driven from the workspace bar, which only the isolated Development editor
   shell renders. The keys therefore stop existing outside that shell, outside
   an Area that produced jump targets, and while a text field has focus. */
void Client::CMapTool::Update_EditorSublevelJumpShortcuts()
{
	if (m_EditorSublevelJumps.empty() || ImGui::GetIO().WantTextInput)
		return;

	static constexpr ImGuiKey ROW_KEYS[] = {
		ImGuiKey_1, ImGuiKey_2, ImGuiKey_3, ImGuiKey_4, ImGuiKey_5,
		ImGuiKey_6, ImGuiKey_7, ImGuiKey_8, ImGuiKey_9 };
	static constexpr ImGuiKey PAD_KEYS[] = {
		ImGuiKey_Keypad1, ImGuiKey_Keypad2, ImGuiKey_Keypad3,
		ImGuiKey_Keypad4, ImGuiKey_Keypad5, ImGuiKey_Keypad6,
		ImGuiKey_Keypad7, ImGuiKey_Keypad8, ImGuiKey_Keypad9 };
	const size_t bound = (std::min)(
		m_EditorSublevelJumps.size(), std::size(ROW_KEYS));
	for (size_t index = 0u; index < bound; ++index)
	{
		if (ImGui::IsKeyPressed(ROW_KEYS[index], false) ||
			ImGui::IsKeyPressed(PAD_KEYS[index], false))
		{
			(void)Jump_ToEditorSublevel(index);
			return;
		}
	}
}
