#include "imgui.h"
#include "Effect_Tool_Internal.h"
#include "Character.h"
#include "CharacterSpec.h"
#include "CombatHUDViewModel.h"
#include "Effect_Catalog.h"
#include "Effect_DocumentCodec.h"
#include "Effect_DirectAuthoredSourceIndex.h"
#include "EffectResourceCatalog.h"
#include "EffectV2_Catalog.h"
#include "Effect_Object.h"
#include "Effect_PresentationService.h"
#include "GameInstance.h"
#include "Logic_DimensionMaster.h"
#include "MainApp.h"
#include <algorithm>
#include <array>
#include <atomic>
#include <cctype>
#include <chrono>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <map>
#include <sstream>
#include <set>
#include <string_view>
#include <system_error>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include "Model.h"
#include "Transform.h"
#include "MapEffectPresentationRuntime.h"

bool_t Client::CEffect_Tool::Refresh_ValtanAreaStaticEffects()
{
	m_bValtanAreaMapEffectLoadAttempted = true;
	if (m_bValtanAreaMapEffectDirty)
	{
		m_bValtanAreaMapEffectLastRefreshSucceeded = false;
		m_strValtanAreaMapEffectStatus =
			"Reload Source refused to discard the dirty Area draft. Use Discard Area Draft explicitly.";
		return false;
	}
	const std::filesystem::path path =
		CProjectDataRoot::Resolve(VALTAN_AREA_MAP_EFFECT_SOURCE);
	CMapEffectDocument staged;
	std::string rawBaseline;
	std::string status;
	if (path.empty() || !staged.Load_WithRawBaseline(
			path, VALTAN_AREA_ID, rawBaseline, status))
	{
		m_bValtanAreaMapEffectLastRefreshSucceeded = false;
		m_strValtanAreaMapEffectStatus = path.empty() ?
			"Valtan Area Map Effect source path escaped the project Data root." :
			"Valtan Area Map Effect refresh preserved the current draft: " + status;
		return false;
	}
	m_ValtanAreaMapEffectDocument = std::move(staged);
	m_ValtanAreaMapEffectPath = path;
	m_strValtanAreaMapEffectBaseline = std::move(rawBaseline);
	m_bValtanAreaMapEffectDirty = false;
	m_bValtanAreaMapEffectLastRefreshSucceeded = true;
	if (m_StaticAreaPreviewPresentation.has_value() &&
		!m_UnpublishedStaticAreaWorldDraft.has_value())
	{
		const MAP_EFFECT_WORLD_PRESENTATION* pReloadedPlacement =
			m_ValtanAreaMapEffectDocument.Find_WorldEffect(
				m_StaticAreaPreviewPresentation->independentEffectId);
		if (nullptr != pReloadedPlacement &&
			pReloadedPlacement->effectAssetId ==
				m_StaticAreaPreviewPresentation->effectAssetId)
		{
			m_StaticAreaPreviewPresentation = *pReloadedPlacement;
			(void)Update_StaticAreaPreviewRoot();
		}
	}
	m_strValtanAreaMapEffectStatus =
		"Loaded Area/static authoring source: " +
		std::to_string(
			m_ValtanAreaMapEffectDocument.Get_Surfaces().size()) +
		" surface + " + std::to_string(
			m_ValtanAreaMapEffectDocument.Get_WorldEffects().size()) +
		" world presentation rows.";
	return true;
}

bool_t Client::CEffect_Tool::Discard_ValtanAreaStaticEffectDraft()
{
	if (!m_bValtanAreaMapEffectDirty)
	{
		m_strValtanAreaMapEffectStatus = "No dirty Area draft exists.";
		return true;
	}
	CMapEffectDocument restored;
	std::string status;
	if (m_strValtanAreaMapEffectBaseline.empty() ||
		!restored.Parse(m_strValtanAreaMapEffectBaseline,
			VALTAN_AREA_ID, status))
	{
		m_strValtanAreaMapEffectStatus =
			"Discard Area Draft failed; the dirty draft was preserved: " + status;
		return false;
	}
	m_ValtanAreaMapEffectDocument = std::move(restored);
	m_bValtanAreaMapEffectDirty = false;
	if (m_StaticAreaPreviewPresentation.has_value() &&
		!m_UnpublishedStaticAreaWorldDraft.has_value())
	{
		const MAP_EFFECT_WORLD_PRESENTATION* pRestoredPlacement =
			m_ValtanAreaMapEffectDocument.Find_WorldEffect(
				m_StaticAreaPreviewPresentation->independentEffectId);
		if (nullptr != pRestoredPlacement &&
			pRestoredPlacement->effectAssetId ==
				m_StaticAreaPreviewPresentation->effectAssetId)
		{
			m_StaticAreaPreviewPresentation = *pRestoredPlacement;
			(void)Update_StaticAreaPreviewRoot();
		}
	}
	m_strValtanAreaMapEffectStatus =
		"Discarded only the in-memory Area Map Effect draft; active Effect edits and source bytes were preserved.";
	return true;
}

bool_t Client::CEffect_Tool::Try_ApplyValtanAreaStaticEffectDraft()
{
	if (!m_ValtanAreaMapEffectDocument.Is_Ready())
	{
		m_strValtanAreaMapEffectStatus =
			"Load the Valtan Area Map Effect source before Apply.";
		return false;
	}
	CMapEffectDocument roundTrip;
	std::string status;
	const std::string canonical =
		m_ValtanAreaMapEffectDocument.Serialize();
	if (!roundTrip.Parse(canonical, VALTAN_AREA_ID, status))
	{
		m_strValtanAreaMapEffectStatus =
			"Map Effect draft Apply rejected before staging: " + status;
		return false;
	}
	const uint32_t levelIndex = CGameInstance::Get().Get_CurrentLevelID();
	if (!CMapEffectPresentationRuntime::Request_DebugApply(
			levelIndex, roundTrip, status))
	{
		m_strValtanAreaMapEffectStatus =
			"Map Effect live Apply preserved the current runtime: " + status;
		return false;
	}
	m_strValtanAreaMapEffectStatus = status +
		" Source remains dirty until Save.";
	return true;
}

bool_t Client::CEffect_Tool::Try_SavePublishValtanAreaStaticEffects()
{
	if (!m_ValtanAreaMapEffectDocument.Is_Ready() ||
		m_ValtanAreaMapEffectPath.empty())
	{
		m_strValtanAreaMapEffectStatus =
			"Load the Valtan Area Map Effect source before Save.";
		return false;
	}

	const std::string previousBaseline =
		m_strValtanAreaMapEffectBaseline;
	const std::string savedRaw =
		m_ValtanAreaMapEffectDocument.Serialize();
	std::string status;
	if (!m_ValtanAreaMapEffectDocument.Save_AtomicIfUnchanged(
			m_ValtanAreaMapEffectPath, previousBaseline, status))
	{
		m_strValtanAreaMapEffectStatus =
			"Map Effect source save failed; source/runtime were preserved: " +
			status;
		return false;
	}

	const std::filesystem::path projectRoot =
		CProjectDataRoot::Get().parent_path();
	const std::filesystem::path publisher = projectRoot /
		L"Tools" / L"MapPipeline" / L"Publish-MapAuthoring.ps1";
	std::wstring command = L"powershell.exe -NoProfile -ExecutionPolicy Bypass -File \"" +
		publisher.wstring() + L"\" -AreaId \"" +
		std::filesystem::path(VALTAN_AREA_ID).wstring() +
		L"\" -ProjectRoot \"" + projectRoot.wstring() + L"\"";
	std::string publishStatus;
	if (!Run_OwnedToolProcess(command, projectRoot, 60000u,
			"Map Effect save", publishStatus))
	{
		std::string rollbackStatus;
		const bool_t rolledBack =
			CMapEffectDocument::Restore_RawBytesAtomicIfUnchanged(
				m_ValtanAreaMapEffectPath, VALTAN_AREA_ID,
				previousBaseline, savedRaw, rollbackStatus);
		m_bValtanAreaMapEffectDirty = true;
		m_strValtanAreaMapEffectStatus = publishStatus +
			(rolledBack ?
				" Source CAS rollback succeeded; the draft remains in memory." :
				" Source rollback failed: " + rollbackStatus);
		return false;
	}

	m_strValtanAreaMapEffectBaseline = savedRaw;
	m_bValtanAreaMapEffectDirty = false;
	if (ETOUI(LEVEL::VALTAN_ARENA) ==
		CGameInstance::Get().Get_CurrentLevelID())
	{
		std::string reloadStatus;
		if (!CMapEffectPresentationRuntime::Request_PublishedReload(
				ETOUI(LEVEL::VALTAN_ARENA), reloadStatus))
		{
			m_strValtanAreaMapEffectStatus = publishStatus +
				" Saved files committed, but live reload preserved the prior runtime: " +
				reloadStatus;
			return false;
		}
		m_strValtanAreaMapEffectStatus = publishStatus + " " + reloadStatus;
	}
	else
	{
		m_strValtanAreaMapEffectStatus = publishStatus +
			" Saved data will load on the next Valtan Arena activation.";
	}
	return true;
}

bool_t Client::CEffect_Tool::Update_StaticAreaPreviewRoot()
{
	if (!m_StaticAreaPreviewPresentation.has_value())
		return false;
	const MAP_EFFECT_WORLD_PRESENTATION& presentation =
		*m_StaticAreaPreviewPresentation;
	if (m_ActiveDocument.has_value() &&
		m_ActiveDocument->strEffectAssetId != presentation.effectAssetId)
		return false;

	const vector_t quaternion = XMQuaternionNormalize(XMVectorSet(
		presentation.rotationQuaternion.x,
		presentation.rotationQuaternion.y,
		presentation.rotationQuaternion.z,
		presentation.rotationQuaternion.w));
	matrix_t orientation = XMMatrixRotationQuaternion(quaternion);
	if (MAP_EFFECT_ORIENTATION_POLICY::CAMERA_FACING_WORLD ==
		presentation.orientationPolicy)
	{
		const float4_t* camera = CGameInstance::Get().Get_CamPosition();
		if (nullptr != camera)
		{
			const vector_t position = XMVectorSet(presentation.position.x,
				presentation.position.y, presentation.position.z, 1.f);
			vector_t forward = XMVectorSubtract(XMLoadFloat4(camera), position);
			if (XMVectorGetX(XMVector3LengthSq(forward)) > 0.000001f)
			{
				forward = XMVector3Normalize(forward);
				vector_t upReference = XMVectorSet(0.f, 1.f, 0.f, 0.f);
				if (std::abs(XMVectorGetX(XMVector3Dot(
						forward, upReference))) > 0.98f)
				{
					upReference = XMVectorSet(0.f, 0.f, 1.f, 0.f);
				}
				const vector_t right = XMVector3Normalize(
					XMVector3Cross(upReference, forward));
				const vector_t up = XMVector3Cross(forward, right);
				orientation *= matrix_t(right, up, forward,
					XMVectorSet(0.f, 0.f, 0.f, 1.f));
			}
		}
	}
	XMStoreFloat4x4(&m_PreviewWorldRoot,
		XMMatrixScaling(presentation.scale.x, presentation.scale.y,
			presentation.scale.z) * orientation *
		XMMatrixTranslation(presentation.position.x,
			presentation.position.y, presentation.position.z));
	m_vPickedWorldPosition = presentation.position;
	m_ePreviewPivotKind = EFFECT_PREVIEW_PIVOT_KIND::WORLD;
	m_strPreviewAnchorSlotId.clear();
	Copy_Buffer(m_PreviewAnchorBuffer.data(),
		m_PreviewAnchorBuffer.size(), m_strPreviewAnchorSlotId);
	return true;
}

bool_t Client::CEffect_Tool::Try_OpenValtanStaticAreaEffect(
	const MAP_EFFECT_WORLD_PRESENTATION& presentation,
	const bool_t previewAtPlacement,
	const bool_t play)
{
	const bool_t bAlreadyActive = m_ActiveDocument.has_value() &&
		m_ActiveDocument->strEffectAssetId == presentation.effectAssetId &&
		(EFFECT_DOCUMENT_SOURCE::NEW_DOCUMENT == m_eActiveDocumentSource ||
		 EFFECT_DOCUMENT_SOURCE::AUTHORED == m_eActiveDocumentSource);
	const std::filesystem::path authoredPath = CProjectDataRoot::Resolve(
		std::filesystem::path("Effects") / "Authored" /
		(presentation.effectAssetId + ".effect.json"));
	std::error_code pathError;
	if (!bAlreadyActive && (authoredPath.empty() ||
		!std::filesystem::is_regular_file(authoredPath, pathError) || pathError)
		)
	{
		m_strValtanAreaMapEffectStatus =
			"World presentation has no exact direct-authored Effect document: " +
			presentation.effectAssetId;
		return false;
	}
	m_StaticAreaPreviewPresentation = presentation;
	if (!bAlreadyActive && !Try_LoadDocumentPath(authoredPath,
			EFFECT_DOCUMENT_SOURCE::AUTHORED, presentation.effectAssetId,
			EFFECT_DOCUMENT_PREVIEW_INTENT::STATIC_AREA_PLACEMENT))
	{
		if (m_PendingDocumentLoad.has_value() &&
			m_PendingDocumentLoad->Path == authoredPath)
		{
			m_PendingDocumentLoad->bPreviewAtPlacementAfterLoad =
				previewAtPlacement;
			m_PendingDocumentLoad->bPlayCompleteAfterLoad = play;
		}
		return false;
	}
	if (bAlreadyActive)
	{
		m_eActiveDocumentPreviewIntent =
			EFFECT_DOCUMENT_PREVIEW_INTENT::STATIC_AREA_PLACEMENT;
		m_strActiveValtanPatternDraftId.clear();
		m_eActiveValtanPatternDraftPreviewPath =
			VALTAN_PATTERN_PREVIEW_PATH::NORMAL;
	}
	if (!Update_StaticAreaPreviewRoot())
		return false;
	if (!previewAtPlacement)
	{
		m_strValtanAreaMapEffectStatus =
			"Opened the exact Area placement Effect without a boss/model preview owner.";
		return true;
	}
	m_fPreviewTimeSeconds = 0.f;
	m_bPreviewPlaying = false;
	m_bPreviewVisibleRequested = true;
	Release_WorldPreview(true);
	if (!Stage_WorldPreview())
	{
		m_bPreviewVisibleRequested = false;
		m_strValtanAreaMapEffectStatus =
			"World presentation preview stage failed: " + m_strPreviewStatus;
		return false;
	}
	const shared_ptr<CEffectObject> preview = m_pWorldPreviewObject.lock();
	if (nullptr == preview)
	{
		m_bPreviewVisibleRequested = false;
		m_strValtanAreaMapEffectStatus =
			"World presentation preview object was not committed.";
		return false;
	}
	preview->Set_RootWorld(m_PreviewWorldRoot);
	preview->Set_SampleTime(0.f);
	preview->Set_Playing(false);
	preview->Set_Visible(true);
	m_bPreviewPlaying = play;
	m_strValtanAreaMapEffectStatus = play ?
		"Playing direct-authored Effect at the Area placement." :
		"Previewing direct-authored Effect at the Area placement at time zero.";
	return true;
}

bool_t Client::CEffect_Tool::Try_PreviewValtanAreaWorldEffect(
	const MAP_EFFECT_WORLD_PRESENTATION& presentation,
	const bool_t play)
{
	return Try_OpenValtanStaticAreaEffect(presentation, true, play);
}

bool_t Client::CEffect_Tool::Try_CreateStaticAreaWorldEffectDraft()
{
	if (!m_ValtanAreaMapEffectDocument.Is_Ready())
	{
		m_strValtanAreaMapEffectStatus =
			"Load the Area Map Effect source before creating a static world draft.";
		return false;
	}
	if (Has_UnsavedWork() ||
		m_UnpublishedStaticAreaWorldDraft.has_value())
	{
		m_strValtanAreaMapEffectStatus =
			"Save, register, or explicitly discard the current Effect/Area draft first.";
		return false;
	}

	const std::string EffectAssetId = m_NewStaticAreaEffectAssetId.data();
	const std::string IndependentEffectId =
		m_NewStaticAreaIndependentId.data();
	const std::string PlacementId = m_NewStaticAreaPlacementId.data();
	std::string DisplayName = m_NewStaticAreaDisplayName.data();
	if (DisplayName.empty())
		DisplayName = IndependentEffectId;
	if (EffectAssetId.empty() || IndependentEffectId.empty() ||
		PlacementId.empty())
	{
		m_strValtanAreaMapEffectStatus =
			"Effect Asset ID, Independent Effect ID, and Placement ID are required.";
		return false;
	}
	if (CEffectCatalog::Contains(EffectAssetId))
	{
		m_strValtanAreaMapEffectStatus =
			"New static world Effect refuses an ID already loaded by EffectCatalog.";
		return false;
	}
	const std::filesystem::path EffectPath = CProjectDataRoot::Resolve(
		std::filesystem::path("Effects") / "Authored" /
		(EffectAssetId + ".effect.json"));
	std::error_code FileError;
	if (EffectPath.empty() ||
		std::filesystem::exists(EffectPath, FileError) || FileError)
	{
		m_strValtanAreaMapEffectStatus = EffectPath.empty() ?
			"New static world Effect path escaped Data/Effects/Authored." :
			"New static world Effect refuses an existing destination file.";
		return false;
	}

	MAP_EFFECT_WORLD_PRESENTATION Presentation;
	Presentation.independentEffectId = IndependentEffectId;
	Presentation.displayName = DisplayName;
	Presentation.placementId = PlacementId;
	Presentation.effectAssetId = EffectAssetId;
	Presentation.position = m_vPickedWorldPosition;
	Presentation.rotationQuaternion = { 0.f, 0.f, 0.f, 1.f };
	Presentation.scale = { 1.f, 1.f, 1.f };
	Presentation.orientationPolicy = MAP_EFFECT_ORIENTATION_POLICY::WORLD;
	Presentation.activationPolicy = MAP_EFFECT_ACTIVATION_POLICY::LEVEL_ACTIVE;
	Presentation.playbackPolicy = MAP_EFFECT_PLAYBACK_POLICY::LOCAL_LOOP;
	CMapEffectDocument CandidateMap = m_ValtanAreaMapEffectDocument;
	std::string Error;
	if (!CandidateMap.Add_WorldEffectForAuthoring(Presentation, Error))
	{
		m_strValtanAreaMapEffectStatus =
			"New static world placement identity is invalid or collides: " + Error;
		return false;
	}

	Copy_Buffer(m_NewAssetId.data(), m_NewAssetId.size(), EffectAssetId);
	Copy_Buffer(m_NewDisplayName.data(), m_NewDisplayName.size(), DisplayName);
	if (!Try_CreateDocument())
	{
		m_strValtanAreaMapEffectStatus = m_strDocumentStatus;
		return false;
	}
	m_eActiveDocumentPreviewIntent =
		EFFECT_DOCUMENT_PREVIEW_INTENT::STATIC_AREA_PLACEMENT;
	m_StaticAreaPreviewPresentation = Presentation;
	m_UnpublishedStaticAreaWorldDraft = Presentation;
	(void)Update_StaticAreaPreviewRoot();
	m_strValtanAreaMapEffectStatus =
		"UNPUBLISHED DRAFT created in memory. Add drawable Effect elements and tune the placement, then Register in Area.";
	return true;
}

bool_t Client::CEffect_Tool::Try_RegisterStaticAreaWorldEffectDraft()
{
	if (!m_UnpublishedStaticAreaWorldDraft.has_value() ||
		!m_ActiveDocument.has_value() ||
		EFFECT_DOCUMENT_SOURCE::NEW_DOCUMENT != m_eActiveDocumentSource ||
		EFFECT_DOCUMENT_PREVIEW_INTENT::STATIC_AREA_PLACEMENT !=
			m_eActiveDocumentPreviewIntent ||
		m_ActiveDocument->strEffectAssetId !=
			m_UnpublishedStaticAreaWorldDraft->effectAssetId)
	{
		m_strValtanAreaMapEffectStatus =
			"Register requires the active unsaved static world Effect draft.";
		return false;
	}
	if (Has_UnappliedDetailDraft())
	{
		m_strValtanAreaMapEffectStatus =
			"Apply or Revert the open Effect Detail draft before registration.";
		return false;
	}
	if (m_bValtanAreaMapEffectDirty ||
		!m_ValtanAreaMapEffectDocument.Is_Ready() ||
		m_ValtanAreaMapEffectPath.empty() ||
		m_strValtanAreaMapEffectBaseline.empty())
	{
		m_strValtanAreaMapEffectStatus =
			"Register requires a clean, exact-baseline Area Map Effect source.";
		return false;
	}
	std::string Error;
	if (!CEffectDocumentCodec::Validate(*m_ActiveDocument, Error) ||
		!CEffectDocumentCodec::Validate_Drawable(*m_ActiveDocument, Error))
	{
		m_strValtanAreaMapEffectStatus =
			"Register requires one structurally valid drawable Effect: " + Error;
		return false;
	}

	const MAP_EFFECT_WORLD_PRESENTATION Presentation =
		*m_UnpublishedStaticAreaWorldDraft;
	CMapEffectDocument CandidateMap = m_ValtanAreaMapEffectDocument;
	if (!CandidateMap.Add_WorldEffectForAuthoring(Presentation, Error))
	{
		m_strValtanAreaMapEffectStatus =
			"Area registration collision/validation failed: " + Error;
		return false;
	}
	const std::string CandidateMapRaw = CandidateMap.Serialize();
	const std::filesystem::path EffectPath = CProjectDataRoot::Resolve(
		std::filesystem::path("Effects") / "Authored" /
		(Presentation.effectAssetId + ".effect.json"));
	std::error_code FileError;
	if (EffectPath.empty() || CEffectCatalog::Contains(
			Presentation.effectAssetId) ||
		std::filesystem::exists(EffectPath, FileError) || FileError)
	{
		m_strValtanAreaMapEffectStatus =
			"Registration collision: the Effect catalog identity or authored path already exists.";
		return false;
	}

	std::string PreviousCatalogRaw;
	std::string CandidateCatalogRaw;
	std::filesystem::path CatalogPath;
	if (!Build_EffectCatalogWithDirectAuthoredRow(
			Presentation.effectAssetId, PreviousCatalogRaw,
			CandidateCatalogRaw, CatalogPath, Error))
	{
		m_strValtanAreaMapEffectStatus =
			"EffectCatalog registration candidate failed: " + Error;
		return false;
	}
	const std::string EffectRaw =
		CEffectDocumentCodec::Serialize(*m_ActiveDocument);
	bool_t bEffectWritten = false;
	bool_t bCatalogWritten = false;
	bool_t bMapWritten = false;
	bool_t bPublished = false;
	bool_t bRuntimeCatalogCommitted = false;
	std::shared_ptr<const EFFECT_DEBUG_DIRECT_AUTHORED_REGISTRATION>
		RuntimeCandidate;

	const std::filesystem::path ProjectRoot =
		CProjectDataRoot::Get().parent_path();
	const std::filesystem::path Publisher = ProjectRoot /
		L"Tools" / L"MapPipeline" / L"Publish-MapAuthoring.ps1";
	const auto RunPublisher = [&ProjectRoot, &Publisher](
		std::string& OutStatus)
	{
		std::wstring Command =
			L"powershell.exe -NoProfile -ExecutionPolicy Bypass -File \"" +
			Publisher.wstring() + L"\" -AreaId \"" +
			std::filesystem::path(VALTAN_AREA_ID).wstring() +
			L"\" -ProjectRoot \"" + ProjectRoot.wstring() + L"\"";
		return Run_OwnedToolProcess(Command, ProjectRoot, 60000u,
			"Map Effect save", OutStatus);
	};
	const auto Rollback = [&](const std::string& Failure)
	{
		std::vector<std::string> RollbackStatuses;
		bool_t bSourcesRestored = true;
		if (bRuntimeCatalogCommitted)
		{
			std::string Status;
			const bool_t Restored =
				CEffectCatalog::Restore_DebugDirectAuthoredRegistration(
					RuntimeCandidate, Status);
			bSourcesRestored = bSourcesRestored && Restored;
			RollbackStatuses.push_back(
				std::string("runtime catalog: ") + Status);
		}
		if (bMapWritten)
		{
			std::string Status;
			const bool_t Restored =
				CMapEffectDocument::Restore_RawBytesAtomicIfUnchanged(
					m_ValtanAreaMapEffectPath, VALTAN_AREA_ID,
					m_strValtanAreaMapEffectBaseline,
					CandidateMapRaw, Status);
			bSourcesRestored = bSourcesRestored && Restored;
			RollbackStatuses.push_back(std::string("Area source: ") + Status);
		}
		if (bCatalogWritten)
		{
			std::string Status;
			const bool_t Restored =
				Write_TransactionRawBytesAtomicIfUnchanged(
					CatalogPath, PreviousCatalogRaw,
					CandidateCatalogRaw, Status);
			bSourcesRestored = bSourcesRestored && Restored;
			RollbackStatuses.push_back(
				std::string("EffectCatalog source: ") + Status);
		}
		if (bEffectWritten)
		{
			std::string Status;
			const bool_t Restored = Remove_TransactionFileIfExactRaw(
				EffectPath, EffectRaw, Status);
			bSourcesRestored = bSourcesRestored && Restored;
			RollbackStatuses.push_back(
				std::string("Effect source: ") + Status);
		}
		if (bPublished && bSourcesRestored)
		{
			std::string Status;
			const bool_t Republished = RunPublisher(Status);
			bSourcesRestored = bSourcesRestored && Republished;
			RollbackStatuses.push_back(
				std::string("saved runtime restore: ") + Status);
		}
		m_strValtanAreaMapEffectStatus = Failure;
		for (const std::string& Status : RollbackStatuses)
			m_strValtanAreaMapEffectStatus += " | " + Status;
		if (!bSourcesRestored)
			m_strValtanAreaMapEffectStatus +=
				" | CRITICAL: exact transaction rollback was incomplete.";
		return false;
	};

	if (!Create_TransactionRawBytesDurableIfAbsent(
			EffectPath, EffectRaw, Error))
	{
		m_strValtanAreaMapEffectStatus =
			"Registration did not create the Effect source: " + Error;
		return false;
	}
	bEffectWritten = true;
	if (!Write_TransactionRawBytesAtomicIfUnchanged(
			CatalogPath, CandidateCatalogRaw, PreviousCatalogRaw, Error))
	{
		return Rollback("EffectCatalog exact-byte CAS failed: " + Error);
	}
	bCatalogWritten = true;
	if (!CEffectCatalog::Stage_DebugDirectAuthoredRegistration(
			Presentation.effectAssetId, EffectPath,
			RuntimeCandidate, Error))
	{
		return Rollback("Runtime catalog registration staging failed: " + Error);
	}
	if (!CandidateMap.Save_AtomicIfUnchanged(
			m_ValtanAreaMapEffectPath,
			m_strValtanAreaMapEffectBaseline, Error))
	{
		return Rollback("Area source exact-byte CAS failed: " + Error);
	}
	bMapWritten = true;
	std::string PublishStatus;
	if (!RunPublisher(PublishStatus))
	{
		return Rollback("Three-document save failed: " +
			PublishStatus);
	}
	bPublished = true;
	if (!CEffectCatalog::Commit_DebugDirectAuthoredRegistration(
			RuntimeCandidate, Error))
	{
		return Rollback("Runtime catalog registration commit failed: " + Error);
	}
	bRuntimeCatalogCommitted = true;
	if (!CEffectPresentationService::Replace_ProductPreparedTarget(
			m_pDevice, m_pContext, RuntimeCandidate->Get_RuntimeRevision(),
			RuntimeCandidate->Get_EffectAssetId(),
			RuntimeCandidate->Get_DocumentShared(),
			RuntimeCandidate->Get_VisualProjection(), Error))
	{
		return Rollback("Current-session Product prewarm failed: " + Error);
	}

	m_ValtanAreaMapEffectDocument = std::move(CandidateMap);
	m_strValtanAreaMapEffectBaseline = CandidateMapRaw;
	m_bValtanAreaMapEffectDirty = false;
	m_ActiveDocumentPath = EffectPath;
	m_eActiveDocumentSource = EFFECT_DOCUMENT_SOURCE::AUTHORED;
	m_strActiveDocumentBaselineCanonical = EffectRaw;
	m_bDocumentDirty = false;
	m_UnpublishedStaticAreaWorldDraft.reset();
	m_StaticAreaPreviewPresentation = Presentation;
	Refresh_RuntimeEquivalence();
	(void)Refresh_DataFiles();
	(void)Refresh_AllEffects();
	(void)Update_StaticAreaPreviewRoot();

	std::string ReloadStatus;
	if (ETOUI(LEVEL::VALTAN_ARENA) ==
			CGameInstance::Get().Get_CurrentLevelID() &&
		!CMapEffectPresentationRuntime::Request_PublishedReload(
			ETOUI(LEVEL::VALTAN_ARENA), ReloadStatus))
	{
		m_strValtanAreaMapEffectStatus =
			"Registered and prewarmed the three-document static world Effect, but live Area reload preserved the prior runtime: " +
			ReloadStatus;
		return true;
	}
	m_strValtanAreaMapEffectStatus =
		"Registered, saved, and loaded static world Effect '" +
		Presentation.effectAssetId + "'. " + PublishStatus;
	if (!ReloadStatus.empty())
		m_strValtanAreaMapEffectStatus += " " + ReloadStatus;
	return true;
}

void Client::CEffect_Tool::Render_ValtanAreaStaticEffectSection(
	const std::string& strSearch)
{
	if (!m_bValtanAreaMapEffectLoadAttempted)
		Refresh_ValtanAreaStaticEffects();
	if (!m_ValtanAreaMapEffectDocument.Is_Ready())
	{
		if (!ImGui::TreeNodeEx("AREA-STATIC (unavailable)",
				ImGuiTreeNodeFlags_OpenOnArrow))
		{
			return;
		}
		ImGui::TextWrapped("%s", m_strValtanAreaMapEffectStatus.c_str());
		ImGui::BeginDisabled(m_bValtanAreaMapEffectDirty);
		if (ImGui::SmallButton("Reload Area Source"))
			Refresh_ValtanAreaStaticEffects();
		ImGui::EndDisabled();
		ImGui::TreePop();
		return;
	}

	const auto matchesSurface = [&strSearch](const auto& row)
	{
		return strSearch.empty() ||
			Contains_NoCase(row.independentEffectId, strSearch) ||
			Contains_NoCase(row.displayName, strSearch);
	};
	const auto matchesWorld = [&strSearch](const auto& row)
	{
		return strSearch.empty() ||
			Contains_NoCase(row.independentEffectId, strSearch) ||
			Contains_NoCase(row.displayName, strSearch) ||
			Contains_NoCase(row.effectAssetId, strSearch) ||
			Contains_NoCase(row.placementId, strSearch);
	};
	const size_t visibleCount = std::ranges::count_if(
		m_ValtanAreaMapEffectDocument.Get_Surfaces(), matchesSurface) +
		std::ranges::count_if(
			m_ValtanAreaMapEffectDocument.Get_WorldEffects(), matchesWorld);
	if (0u == visibleCount)
		return;

	if (!strSearch.empty())
		ImGui::SetNextItemOpen(true, ImGuiCond_Always);
	const std::string label = "AREA-STATIC (" +
		std::to_string(visibleCount) + ")";
	if (!ImGui::TreeNodeEx(label.c_str(), ImGuiTreeNodeFlags_OpenOnArrow))
		return;

	ImGui::TextDisabled(
		"Area-owned static presentation rows share Independent Effect authoring without creating a fake boss/model owner.");
	ImGui::TextWrapped("Source: %s",
		m_ValtanAreaMapEffectPath.generic_string().c_str());
	ImGui::BeginDisabled(!m_bValtanAreaMapEffectDirty);
	if (ImGui::SmallButton("Preview Draft on Valtan"))
		Try_ApplyValtanAreaStaticEffectDraft();
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::BeginDisabled(!m_bValtanAreaMapEffectDirty);
	if (ImGui::SmallButton("Save"))
		Try_SavePublishValtanAreaStaticEffects();
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::BeginDisabled(m_bValtanAreaMapEffectDirty);
	if (ImGui::SmallButton("Reload Source"))
		Refresh_ValtanAreaStaticEffects();
	ImGui::EndDisabled();
	if (m_bValtanAreaMapEffectDirty)
	{
		ImGui::SameLine();
		if (ImGui::SmallButton("Discard Area Draft"))
			Discard_ValtanAreaStaticEffectDraft();
	}
	ImGui::SameLine();
	ImGui::BeginDisabled(ETOUI(LEVEL::VALTAN_ARENA) !=
		CGameInstance::Get().Get_CurrentLevelID());
	if (ImGui::SmallButton("Load Saved"))
	{
		std::string status;
		if (!CMapEffectPresentationRuntime::Request_PublishedReload(
				ETOUI(LEVEL::VALTAN_ARENA), status))
		{
			m_strValtanAreaMapEffectStatus =
				"Saved reload preserved the live runtime: " + status;
		}
		else
		{
			m_strValtanAreaMapEffectStatus = status;
		}
	}
	ImGui::EndDisabled();
	if (!m_strValtanAreaMapEffectStatus.empty())
		ImGui::TextWrapped("%s", m_strValtanAreaMapEffectStatus.c_str());
	if (m_bValtanAreaMapEffectDirty)
		ImGui::TextColored(ImVec4(1.f, 0.72f, 0.24f, 1.f),
			"UNSAVED AREA DRAFT");

	ImGui::SeparatorText("New Static World Effect");
	ImGui::InputTextWithHint("Effect Asset ID##new-static-area",
		"effect.valtan.environment...",
		m_NewStaticAreaEffectAssetId.data(),
		m_NewStaticAreaEffectAssetId.size());
	ImGui::InputTextWithHint("Independent Effect ID##new-static-area",
		"valtan.independent-effect...",
		m_NewStaticAreaIndependentId.data(),
		m_NewStaticAreaIndependentId.size());
	ImGui::InputTextWithHint("Placement ID##new-static-area",
		"valtan.static...",
		m_NewStaticAreaPlacementId.data(),
		m_NewStaticAreaPlacementId.size());
	ImGui::InputTextWithHint("Display Name##new-static-area",
		"Static world presentation",
		m_NewStaticAreaDisplayName.data(),
		m_NewStaticAreaDisplayName.size());
	ImGui::BeginDisabled(Has_UnsavedWork() ||
		m_UnpublishedStaticAreaWorldDraft.has_value());
	if (ImGui::SmallButton("New Static World Effect"))
		Try_CreateStaticAreaWorldEffectDraft();
	ImGui::EndDisabled();
	if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
	{
		ImGui::SetTooltip(
			"Creates one in-memory v13 Effect plus a LEVEL_ACTIVE/LOCAL_LOOP world placement. No file or runtime catalog changes occur until Register in Area.");
	}

	if (m_UnpublishedStaticAreaWorldDraft.has_value())
	{
		MAP_EFFECT_WORLD_PRESENTATION& Draft =
			*m_UnpublishedStaticAreaWorldDraft;
		ImGui::PushID("unpublished-static-area-world");
		ImGui::TextColored(ImVec4(1.f, 0.72f, 0.24f, 1.f),
			"UNPUBLISHED DRAFT");
		ImGui::TextWrapped("%s | %s | %s", Draft.effectAssetId.c_str(),
			Draft.independentEffectId.c_str(), Draft.placementId.c_str());
		bool_t bDraftTransformChanged = false;
		bDraftTransformChanged |= ImGui::DragFloat3(
			"Position", &Draft.position.x, 0.05f,
			-100000.f, 100000.f, "%.3f");
		bDraftTransformChanged |= ImGui::DragFloat3(
			"Scale", &Draft.scale.x, 0.01f,
			0.001f, 1000.f, "%.3f");
		float Quaternion[4] = {
			Draft.rotationQuaternion.x, Draft.rotationQuaternion.y,
			Draft.rotationQuaternion.z, Draft.rotationQuaternion.w };
		if (ImGui::DragFloat4("Rotation Quaternion", Quaternion,
				0.005f, -1.f, 1.f, "%.4f"))
		{
			const f32_t fLength = std::sqrt(
				Quaternion[0] * Quaternion[0] +
				Quaternion[1] * Quaternion[1] +
				Quaternion[2] * Quaternion[2] +
				Quaternion[3] * Quaternion[3]);
			if (std::isfinite(fLength) && fLength > 0.000001f)
			{
				Draft.rotationQuaternion = {
					Quaternion[0] / fLength, Quaternion[1] / fLength,
					Quaternion[2] / fLength, Quaternion[3] / fLength };
				bDraftTransformChanged = true;
			}
			else
			{
				m_strValtanAreaMapEffectStatus =
					"Unsaved placement quaternion cannot be zero.";
			}
		}
		const char_t* pOrientation =
			MAP_EFFECT_ORIENTATION_POLICY::WORLD == Draft.orientationPolicy ?
				"WORLD" : "CAMERA_FACING_WORLD";
		if (ImGui::BeginCombo("Orientation Policy", pOrientation))
		{
			for (const MAP_EFFECT_ORIENTATION_POLICY Policy : {
				MAP_EFFECT_ORIENTATION_POLICY::WORLD,
				MAP_EFFECT_ORIENTATION_POLICY::CAMERA_FACING_WORLD })
			{
				const char_t* pOption =
					MAP_EFFECT_ORIENTATION_POLICY::WORLD == Policy ?
						"WORLD" : "CAMERA_FACING_WORLD";
				if (ImGui::Selectable(
						pOption, Draft.orientationPolicy == Policy))
				{
					Draft.orientationPolicy = Policy;
					bDraftTransformChanged = true;
				}
			}
			ImGui::EndCombo();
		}
		if (bDraftTransformChanged)
		{
			m_StaticAreaPreviewPresentation = Draft;
			(void)Update_StaticAreaPreviewRoot();
			m_strValtanAreaMapEffectStatus =
				"Unsaved static placement transform changed in memory.";
		}
		ImGui::BeginDisabled(!m_bActiveDocumentDrawable);
		if (ImGui::SmallButton("Preview Draft at Placement"))
			Try_PreviewValtanAreaWorldEffect(Draft, false);
		ImGui::SameLine();
		if (ImGui::SmallButton("Play Draft"))
			Try_PreviewValtanAreaWorldEffect(Draft, true);
		ImGui::EndDisabled();
		ImGui::SameLine();
		ImGui::BeginDisabled(!m_bActiveDocumentDrawable ||
			Has_UnappliedDetailDraft());
		if (ImGui::SmallButton("Register in Area"))
			Try_RegisterStaticAreaWorldEffectDraft();
		ImGui::EndDisabled();
		ImGui::SameLine();
		if (ImGui::SmallButton("Discard Unsaved Draft"))
		{
			Discard_ActiveDocument();
			m_strValtanAreaMapEffectStatus =
				"Discarded only the unsaved in-memory Effect/placement draft.";
		}
		if (!m_bActiveDocumentDrawable)
		{
			ImGui::TextDisabled(
				"Registration is locked until the Current Effect has at least one drawable element: %s",
				m_strActiveDocumentDrawableError.c_str());
		}
		ImGui::PopID();
	}

	for (const MAP_EFFECT_SURFACE_PRESENTATION& view :
		m_ValtanAreaMapEffectDocument.Get_Surfaces())
	{
		if (!matchesSurface(view))
			continue;
		MAP_EFFECT_SURFACE_PRESENTATION* row =
			m_ValtanAreaMapEffectDocument.Edit_Surface(
				view.independentEffectId);
		if (nullptr == row)
			continue;
		ImGui::PushID(row->independentEffectId.c_str());
		const std::string rowLabel = "Surface | " + row->displayName +
			" | " + row->independentEffectId;
		if (ImGui::TreeNodeEx(rowLabel.c_str(),
				ImGuiTreeNodeFlags_OpenOnArrow))
		{
			ImGui::TextDisabled(
				"DEPLOY_SURFACE_OVERLAY | visible exactly while Server destruction state is INTACT");
			ImGui::Text("Owners: %zu | Material: %u",
				row->owners.size(), row->materialIndex);
			for (const MAP_EFFECT_SURFACE_OWNER& owner : row->owners)
			{
				ImGui::BulletText("%s / %llu", owner.groupId.c_str(),
					static_cast<unsigned long long>(owner.placementId));
			}
			bool_t changed = false;
			changed |= ImGui::DragFloat("Emissive Intensity",
				&row->emissiveIntensity, 0.01f, 0.f, 64.f, "%.3f");
			changed |= ImGui::ColorEdit4("Emissive Color",
				&row->emissiveColor.x,
				ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR);
			changed |= ImGui::DragFloat("Mask Power", &row->maskPower,
				0.01f, 0.01f, 32.f, "%.3f");
			if (changed)
			{
				m_bValtanAreaMapEffectDirty = true;
				m_strValtanAreaMapEffectStatus =
					"Surface presentation draft changed; Preview shows it and Save stores it.";
			}
			ImGui::TreePop();
		}
		ImGui::PopID();
	}

	for (const MAP_EFFECT_WORLD_PRESENTATION& view :
		m_ValtanAreaMapEffectDocument.Get_WorldEffects())
	{
		if (!matchesWorld(view))
			continue;
		MAP_EFFECT_WORLD_PRESENTATION* row =
			m_ValtanAreaMapEffectDocument.Edit_WorldEffect(
				view.independentEffectId);
		if (nullptr == row)
			continue;
		ImGui::PushID(row->independentEffectId.c_str());
		const std::string rowLabel = "World | " + row->displayName +
			" | " + row->independentEffectId;
		if (ImGui::TreeNodeEx(rowLabel.c_str(),
				ImGuiTreeNodeFlags_OpenOnArrow))
		{
			ImGui::TextWrapped("Effect: %s", row->effectAssetId.c_str());
			ImGui::TextWrapped("Placement: %s", row->placementId.c_str());
			ImGui::TextDisabled("Activation set: %s | %zu Server stage windows",
				row->activationSetId.c_str(), row->activationWindows.size());
			bool_t changed = false;
			changed |= ImGui::DragFloat3("Position", &row->position.x,
				0.05f, -100000.f, 100000.f, "%.3f");
			changed |= ImGui::DragFloat3("Scale", &row->scale.x,
				0.01f, 0.001f, 1000.f, "%.3f");
			float quaternion[4] = {
				row->rotationQuaternion.x, row->rotationQuaternion.y,
				row->rotationQuaternion.z, row->rotationQuaternion.w };
			if (ImGui::DragFloat4("Rotation Quaternion", quaternion,
					0.005f, -1.f, 1.f, "%.4f"))
			{
				const f32_t length = std::sqrt(
					quaternion[0] * quaternion[0] +
					quaternion[1] * quaternion[1] +
					quaternion[2] * quaternion[2] +
					quaternion[3] * quaternion[3]);
				if (std::isfinite(length) && length > 0.000001f)
				{
					row->rotationQuaternion = {
						quaternion[0] / length, quaternion[1] / length,
						quaternion[2] / length, quaternion[3] / length };
					changed = true;
				}
				else
				{
					m_strValtanAreaMapEffectStatus =
						"World placement quaternion cannot be zero.";
				}
			}
			const char_t* orientationLabel =
				MAP_EFFECT_ORIENTATION_POLICY::CAMERA_FACING_WORLD ==
					row->orientationPolicy ? "CAMERA_FACING_WORLD" : "WORLD";
			if (ImGui::BeginCombo("Orientation Policy", orientationLabel))
			{
				for (const MAP_EFFECT_ORIENTATION_POLICY policy : {
					MAP_EFFECT_ORIENTATION_POLICY::WORLD,
					MAP_EFFECT_ORIENTATION_POLICY::CAMERA_FACING_WORLD })
				{
					const char_t* option =
						MAP_EFFECT_ORIENTATION_POLICY::WORLD == policy ?
							"WORLD" : "CAMERA_FACING_WORLD";
					if (ImGui::Selectable(option,
							row->orientationPolicy == policy))
					{
						row->orientationPolicy = policy;
						changed = true;
					}
				}
				ImGui::EndCombo();
			}
			if (changed)
			{
				m_bValtanAreaMapEffectDirty = true;
				if (m_ActiveDocument.has_value() &&
					m_eActiveDocumentPreviewIntent ==
						EFFECT_DOCUMENT_PREVIEW_INTENT::STATIC_AREA_PLACEMENT &&
					m_ActiveDocument->strEffectAssetId == row->effectAssetId)
				{
					m_StaticAreaPreviewPresentation = *row;
					(void)Update_StaticAreaPreviewRoot();
				}
				m_strValtanAreaMapEffectStatus =
					"World placement draft changed; Preview shows it and Save stores it.";
			}

			const std::filesystem::path authoredPath = CProjectDataRoot::Resolve(
				std::filesystem::path("Effects") / "Authored" /
				(row->effectAssetId + ".effect.json"));
			const bool_t hasAuthored = !authoredPath.empty() &&
				(CEffectCatalog::Is_DirectAuthoredDocument(
					row->effectAssetId) ||
				 m_DirectAuthoredEditableEntries.contains(
					row->effectAssetId));
			const bool_t active = m_ActiveDocument.has_value() &&
				m_eActiveDocumentSource == EFFECT_DOCUMENT_SOURCE::AUTHORED &&
				m_ActiveDocument->strEffectAssetId == row->effectAssetId;
			ImGui::BeginDisabled(!hasAuthored || active);
			if (ImGui::SmallButton("Open Editor") && hasAuthored)
			{
				Try_OpenValtanStaticAreaEffect(*row, false, false);
			}
			ImGui::EndDisabled();
			ImGui::SameLine();
			ImGui::BeginDisabled(!hasAuthored);
			if (ImGui::SmallButton("Preview at Placement"))
				Try_PreviewValtanAreaWorldEffect(*row, false);
			ImGui::SameLine();
			if (ImGui::SmallButton("Play"))
				Try_PreviewValtanAreaWorldEffect(*row, true);
			ImGui::EndDisabled();
			ImGui::SameLine();
			ImGui::Checkbox("Loop", &m_bPreviewLoop);
			ImGui::SameLine();
			if (ImGui::SmallButton("Stop"))
				Hide_WorldPreview();

			const VALTAN_PATTERN_VIEW* ownerPattern =
				row->activationWindows.empty() ? nullptr :
				Find_ValtanPattern(row->activationWindows.front().patternId);
			std::string serverReason;
			const bool_t canPlayServer = nullptr != ownerPattern &&
				Can_PlayValtanServerPattern(*ownerPattern, serverReason);
			if (nullptr == ownerPattern)
				serverReason = "No exact Server activation owner Pattern was resolved.";
			ImGui::BeginDisabled(!canPlayServer);
			if (ImGui::SmallButton("Complete Play Activation Owner") &&
				nullptr != ownerPattern)
			{
				Try_PlayValtanServerPattern(*ownerPattern);
			}
			ImGui::EndDisabled();
			if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
				ImGui::SetTooltip("%s", canPlayServer ?
					"Run the complete Server-owned activation Pattern." :
					serverReason.c_str());
			ImGui::TreePop();
		}
		ImGui::PopID();
	}
	ImGui::TreePop();
}

bool_t Client::CEffect_Tool::Refresh_ValtanEffectResourceSnapshot()
{
	m_bValtanEffectResourceLoadAttempted = true;
	std::string OwnerStatus;
	const std::shared_ptr<const EFFECT_V2_CATALOG_SNAPSHOT> pTypedBefore =
		CEffectV2Catalog::Get().Get_Snapshot();
	if ((nullptr == pTypedBefore || !pTypedBefore->Is_Ready()) &&
		!CEffectV2Catalog::Get().Reload_BossValtan(OwnerStatus))
	{
		const auto pPrevious = CEffectResourceCatalog::Get().Get_Snapshot();
		if (nullptr != pPrevious && pPrevious->Is_Ready())
		{
			m_pValtanEffectResourceSnapshot = pPrevious;
			m_eValtanEffectResourceAdmission =
				VALTAN_VIEW_ADMISSION::STALE_PRESERVED;
			m_strValtanEffectResourceStatus =
				"STALE PRESERVED / READ ONLY: typed Effect owner reload failed; "
				"the last validated V1/V2 namespace remains visible. " +
				OwnerStatus;
		}
		else
		{
			m_eValtanEffectResourceAdmission = VALTAN_VIEW_ADMISSION::REJECTED;
			m_strValtanEffectResourceStatus =
				"Unified Effect Resources rejected before first commit: " +
				OwnerStatus;
		}
		return false;
	}

	std::string FacadeStatus;
	if (!CEffectResourceCatalog::Get().Reload_Valtan(FacadeStatus))
	{
		const auto pPrevious = CEffectResourceCatalog::Get().Get_Snapshot();
		if (nullptr != pPrevious && pPrevious->Is_Ready())
		{
			m_pValtanEffectResourceSnapshot = pPrevious;
			m_eValtanEffectResourceAdmission =
				VALTAN_VIEW_ADMISSION::STALE_PRESERVED;
			m_strValtanEffectResourceStatus =
				"STALE PRESERVED / READ ONLY: the V1/V2 owner join was rejected; "
				"the last collision-free namespace remains visible. " +
				FacadeStatus;
		}
		else
		{
			m_pValtanEffectResourceSnapshot.reset();
			m_eValtanEffectResourceAdmission = VALTAN_VIEW_ADMISSION::REJECTED;
			m_strValtanEffectResourceStatus =
				"Unified Effect Resources rejected before first commit: " +
				FacadeStatus;
		}
		return false;
	}

	m_pValtanEffectResourceSnapshot =
		CEffectResourceCatalog::Get().Get_Snapshot();
	if (nullptr == m_pValtanEffectResourceSnapshot ||
		!m_pValtanEffectResourceSnapshot->Is_Ready())
	{
		m_eValtanEffectResourceAdmission = VALTAN_VIEW_ADMISSION::REJECTED;
		m_strValtanEffectResourceStatus =
			"Unified Effect Resources committed no readable snapshot.";
		return false;
	}
	m_eValtanEffectResourceAdmission = VALTAN_VIEW_ADMISSION::ADMITTED;
	m_strValtanEffectResourceStatus =
		"ADMITTED: one collision-free V1 document / V2 leaf / V2 group "
		"namespace is ready (revision " +
		std::to_string(m_pValtanEffectResourceSnapshot->Get_Revision()) + ").";
	return true;
}

bool_t Client::CEffect_Tool::Open_ValtanEffectResource(
	const EFFECT_RESOURCE_DESCRIPTOR& Resource)
{
	if (!Can_MutateValtanView(m_eValtanEffectResourceAdmission))
	{
		m_strValtanEffectResourceStatus =
			"Resource open is read-only while the owner join is stale. Refresh "
			"after resolving the displayed rejection reason.";
		return false;
	}
	if (!Resource.Key.Is_Valid())
	{
		m_strValtanEffectResourceStatus =
			"Resource open rejected an invalid owner/stable-ID key.";
		return false;
	}

	if (EFFECT_RESOURCE_OWNER_KIND::V1_DOCUMENT ==
		Resource.Key.eOwnerKind)
	{
		std::string Status;
		const std::filesystem::path* pPath =
			Resolve_DirectAuthoredEditablePath(
				Resource.Key.strStableId, Status);
		if (nullptr == pPath)
		{
			m_strValtanEffectResourceStatus = std::move(Status);
			return false;
		}
		const bool_t bOpened = Try_LoadDocumentPath(
			*pPath, EFFECT_DOCUMENT_SOURCE::AUTHORED,
			Resource.Key.strStableId,
			EFFECT_DOCUMENT_PREVIEW_INTENT::STANDALONE_EFFECT);
		m_strValtanEffectResourceStatus = bOpened ?
			"Opened the V1 composition document in its owning Effect editor: " +
				Resource.Key.strStableId :
			"V1 owner preserved the current editor state: " + m_strPreviewStatus;
		return bOpened;
	}

	if (EFFECT_RESOURCE_OWNER_KIND::V2_LEAF == Resource.Key.eOwnerKind ||
		EFFECT_RESOURCE_OWNER_KIND::V2_GROUP == Resource.Key.eOwnerKind)
	{
		m_PendingTypedEffectResourceOpen = Resource.Key;
		m_strValtanEffectResourceStatus =
			"Queued the selected resource for its typed Effect owner: " +
			Resource.Key.strStableId;
		return true;
	}

	m_strValtanEffectResourceStatus =
		"Resource open rejected an unknown owner kind.";
	return false;
}

void Client::CEffect_Tool::Render_ValtanEffectResourceSection(
	const std::string& strSearch)
{
	if (!m_bValtanEffectResourceLoadAttempted)
		(void)Refresh_ValtanEffectResourceSnapshot();
	if (!m_strValtanEffectResourceStatus.empty())
		ImGui::TextWrapped("%s", m_strValtanEffectResourceStatus.c_str());
	if (!Can_DisplayValtanView(m_eValtanEffectResourceAdmission) ||
		nullptr == m_pValtanEffectResourceSnapshot ||
		!m_pValtanEffectResourceSnapshot->Is_Ready())
	{
		ImGui::TextDisabled(
			"No unified Effect Resource snapshot was admitted; exact authored "
			"diagnostics remain listed below.");
		return;
	}

	std::vector<const EFFECT_RESOURCE_DESCRIPTOR*> Groups;
	std::vector<const EFFECT_RESOURCE_DESCRIPTOR*> Leaves;
	for (const EFFECT_RESOURCE_DESCRIPTOR& Resource :
		m_pValtanEffectResourceSnapshot->Get_Resources())
	{
		if (!strSearch.empty() &&
			!Contains_NoCase(Resource.Key.strStableId, strSearch) &&
			!Contains_NoCase(Resource.strCategoryLabel, strSearch))
		{
			continue;
		}
		(Resource.Capabilities.bComposite ? Groups : Leaves).push_back(&Resource);
	}

	const std::string RootLabel = "EFFECT RESOURCES (" +
		std::to_string(Groups.size() + Leaves.size()) + "/" +
		std::to_string(
			m_pValtanEffectResourceSnapshot->Get_Resources().size()) + ")";
	if (!strSearch.empty())
		ImGui::SetNextItemOpen(true, ImGuiCond_Always);
	if (!ImGui::TreeNodeEx(RootLabel.c_str(), ImGuiTreeNodeFlags_OpenOnArrow))
		return;
	ImGui::TextDisabled(
		"One stable-ID namespace; Groups contain V1 composition documents and "
		"typed groups, while Leaves contain typed atomic resources.");
	if (!Can_MutateValtanView(m_eValtanEffectResourceAdmission))
	{
		ImGui::TextColored(ImVec4(1.f, 0.72f, 0.18f, 1.f),
			"STALE PRESERVED / READ ONLY");
	}

	const auto RenderRows = [this](
		const char_t* pLabel,
		const std::vector<const EFFECT_RESOURCE_DESCRIPTOR*>& Rows)
	{
		const std::string Label = std::string(pLabel) + " (" +
			std::to_string(Rows.size()) + ")";
		if (!ImGui::TreeNodeEx(Label.c_str(), ImGuiTreeNodeFlags_OpenOnArrow))
			return;
		for (const EFFECT_RESOURCE_DESCRIPTOR* pResource : Rows)
		{
			if (nullptr == pResource)
				continue;
			ImGui::PushID(pResource->Key.strStableId.c_str());
			ImGui::TextWrapped("%s | %s",
				pResource->strDisplayLabel.c_str(),
				pResource->strCategoryLabel.c_str());
			ImGui::SameLine();
			const bool_t bCanOpen =
				Can_MutateValtanView(m_eValtanEffectResourceAdmission) &&
				pResource->Capabilities.bCanLoad;
			ImGui::BeginDisabled(!bCanOpen);
			const char_t* pAction =
				EFFECT_RESOURCE_OWNER_KIND::V1_DOCUMENT ==
					pResource->Key.eOwnerKind ?
					"Open Editor" : "Open Owner Tool";
			if (ImGui::SmallButton(pAction))
				(void)Open_ValtanEffectResource(*pResource);
			ImGui::EndDisabled();
			if (!bCanOpen &&
				ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
			{
				ImGui::SetTooltip(
					"Read-only last-good snapshot. Resolve the join error and Refresh "
					"before opening a mutable owner editor.");
			}
			ImGui::PopID();
		}
		ImGui::TreePop();
	};
	RenderRows("GROUPS / COMPOSITIONS", Groups);
	RenderRows("LEAVES", Leaves);
	ImGui::TreePop();
}

void Client::CEffect_Tool::Render_ValtanExactAuthoredSourceSection(
	const std::string& strSearch)
{
	std::vector<const EFFECT_DIRECT_AUTHORED_SOURCE_ENTRY*> VisibleSources;
	VisibleSources.reserve(m_ValtanExactAuthoredSources.size());
	for (const EFFECT_DIRECT_AUTHORED_SOURCE_ENTRY& Source :
		m_ValtanExactAuthoredSources)
	{
		const std::string strPath = Source.Path.generic_string();
		const bool_t bMatches = strSearch.empty() ||
			Contains_NoCase(Source.strEffectAssetId, strSearch) ||
			Contains_NoCase(Source.strOwnerArchetypeId, strSearch) ||
			Contains_NoCase(Source.strPatternId, strSearch) ||
			Contains_NoCase(Source.strStageId, strSearch) ||
			Contains_NoCase(Source.strActionId, strSearch) ||
			Contains_NoCase(Source.strCombatObjectArchetypeId, strSearch) ||
			Contains_NoCase(Source.strClientVisualId, strSearch) ||
			Contains_NoCase(strPath, strSearch);
		if (bMatches)
			VisibleSources.push_back(&Source);
	}

	if (!strSearch.empty())
		ImGui::SetNextItemOpen(true, ImGuiCond_Always);
	else
		ImGui::SetNextItemOpen(true, ImGuiCond_FirstUseEver);
	const std::string strLabel =
		"EXISTING AUTHORED EFFECTS (" +
		std::to_string(VisibleSources.size()) + "/" +
		std::to_string(m_ValtanExactAuthoredSources.size()) + ")";
	if (!ImGui::TreeNodeEx(strLabel.c_str(), ImGuiTreeNodeFlags_OpenOnArrow))
		return;

	ImGui::TextDisabled(
		"Exact EffectCatalog/Authored JSON sources. Open Editor remains available when the Pattern Product join is rejected; Server/Product Play does not.");
	if (m_ValtanExactAuthoredSources.empty())
	{
		ImGui::TextDisabled(
			"No exact effect.valtan.* authored source was indexed. Refresh reports source-catalog errors without substituting a runtime row.");
		ImGui::TreePop();
		return;
	}
	if (VisibleSources.empty())
	{
		ImGui::TextDisabled("No existing Valtan authored Effect matches the search.");
		ImGui::TreePop();
		return;
	}

	const f32_t fRowListHeight = std::clamp(
		ImGui::GetTextLineHeightWithSpacing() * 14.f, 220.f, 360.f);
	ImGui::BeginChild(
		"ValtanExactAuthoredSourceList", ImVec2(0.f, fRowListHeight), true);
	for (const EFFECT_DIRECT_AUTHORED_SOURCE_ENTRY* pSource : VisibleSources)
	{
		if (nullptr == pSource)
			continue;
		ImGui::PushID(pSource->strEffectAssetId.c_str());
		ImGui::SeparatorText(pSource->strEffectAssetId.c_str());

		std::string strOwner = "SOURCE ONLY | Product owner unavailable";
		switch (pSource->eOwnerKind)
		{
		case EFFECT_DIRECT_AUTHORED_OWNER_KIND::BOSS_PATTERN:
			strOwner = "BOSS PATTERN | " + pSource->strPatternId + " / " +
				pSource->strStageId + " / " + pSource->strActionId;
			break;
		case EFFECT_DIRECT_AUTHORED_OWNER_KIND::BOSS_COMBAT_OBJECT:
			strOwner = "BOSS COMBAT OBJECT | " +
				pSource->strCombatObjectArchetypeId + " / " +
				pSource->strClientVisualId;
			break;
		case EFFECT_DIRECT_AUTHORED_OWNER_KIND::PLAYER_SKILL:
			strOwner = "PLAYER SKILL OWNER";
			break;
		default:
			break;
		}
		ImGui::TextWrapped("%s", strOwner.c_str());
		ImGui::TextDisabled("%s", pSource->Path.generic_string().c_str());

		const bool_t bActive = m_ActiveDocument.has_value() &&
			m_eActiveDocumentSource == EFFECT_DOCUMENT_SOURCE::AUTHORED &&
			m_ActiveDocument->strEffectAssetId == pSource->strEffectAssetId;
		std::string strEditableStatus;
		const std::filesystem::path* pEditablePath =
			Observe_DirectAuthoredEditablePath(
				pSource->strEffectAssetId, strEditableStatus);
		ImGui::BeginDisabled(bActive || nullptr == pEditablePath);
		if (ImGui::SmallButton("Open Editor") && nullptr != pEditablePath)
		{
			std::string strExactStatus;
			const std::filesystem::path* pExactPath =
				Resolve_DirectAuthoredEditablePath(
					pSource->strEffectAssetId, strExactStatus);
			if (nullptr == pExactPath)
				m_strElementStatus = std::move(strExactStatus);
			else
				Try_LoadDocumentPath(*pExactPath,
					EFFECT_DOCUMENT_SOURCE::AUTHORED,
					pSource->strEffectAssetId,
					EFFECT_DOCUMENT_PREVIEW_INTENT::STANDALONE_EFFECT);
		}
		ImGui::EndDisabled();
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
		{
			ImGui::SetTooltip("%s", bActive ?
				"This exact authored document is already the Current Effect." :
				strEditableStatus.c_str());
		}
		ImGui::PopID();
	}
	ImGui::EndChild();
	ImGui::TreePop();
}

void Client::CEffect_Tool::Render_ValtanPatternTreeSection(
	const std::string& strSearch)
{
	/* Retired local-preview labels are intentionally not rendered:
	   "Manual Audition | P"
	   "Animation-first manual audition | phase %u | source chain %s | automatic rotation disabled"
	   Animator order now comes from ManualAuditions. All Effects and Valtan Boss Tool
	   both submit the one shared typed Server audition service. */
	Render_ValtanEffectResourceSection(strSearch);
	Render_ValtanExactAuthoredSourceSection(strSearch);
	if (m_ValtanPatternProductUnlinkOperation.has_value())
	{
		ImGui::TextWrapped("%s", m_strValtanPatternEffectStatus.c_str());
		ImGui::TextDisabled(
			"Pattern rows, Refresh, Create, Delete, Open, and Play stay paused until the non-killing Product transaction exits.");
		return;
	}
	if (!m_bValtanAreaMapEffectLoadAttempted)
		Refresh_ValtanAreaStaticEffects();
	if (VALTAN_VIEW_ADMISSION::UNLOADED ==
			m_eValtanPatternTreeAdmission &&
		!m_bValtanPatternTreeLoadAttempted)
		Refresh_ValtanPatternTree();
	if (m_bValtanPatternTreeReloadRetryPending &&
		ImGui::GetTime() >= m_dNextValtanPatternTreeReloadRetrySeconds)
	{
		(void)Refresh_ValtanPatternTree();
	}
	if (!m_bValtanPatternTreeLastRefreshSucceeded &&
		!m_strValtanPatternTreeStatus.empty())
		ImGui::TextDisabled("%s", m_strValtanPatternTreeStatus.c_str());
	if (Can_DisplayValtanView(m_eValtanPatternTreeAdmission) &&
		!Can_MutateValtanView(m_eValtanPatternTreeAdmission))
	{
		ImGui::TextColored(
			ImVec4(1.f, 0.72f, 0.18f, 1.f),
			"STALE PRESERVED / READ ONLY");
	}
	if (!m_bValtanPatternAuthoringEffectsLastRefreshSucceeded &&
		!m_strValtanPatternAuthoringEffectsStatus.empty())
	{
		ImGui::TextDisabled("Pattern Effect ownership: %s",
			m_strValtanPatternAuthoringEffectsStatus.c_str());
	}
	if (!Can_DisplayValtanView(m_eValtanPatternTreeAdmission))
	{
		ImGui::TextDisabled(
			"No Valtan pattern inventory was staged; press Refresh for the reason.");
		if (m_ValtanAreaMapEffectDocument.Is_Ready())
		{
			if (!strSearch.empty())
				ImGui::SetNextItemOpen(true, ImGuiCond_Always);
			if (ImGui::TreeNodeEx("INDEPENDENT EFFECT",
					ImGuiTreeNodeFlags_OpenOnArrow))
			{
				Render_ValtanAreaStaticEffectSection(strSearch);
				ImGui::TreePop();
			}
		}
		return;
	}
	if (!m_bValtanPatternTreeLoaded && m_bValtanProductFallbackReady)
	{
		Render_ValtanProductFallbackSection(strSearch);
		if (m_ValtanAreaMapEffectDocument.Is_Ready())
		{
			if (!strSearch.empty())
				ImGui::SetNextItemOpen(true, ImGuiCond_Always);
			if (ImGui::TreeNodeEx("INDEPENDENT EFFECT",
					ImGuiTreeNodeFlags_OpenOnArrow))
			{
				Render_ValtanAreaStaticEffectSection(strSearch);
				ImGui::TreePop();
			}
		}
		return;
	}

	std::vector<const VALTAN_INDEPENDENT_EFFECT_VIEW*> IndependentRows;
	IndependentRows.reserve(m_ValtanPatternTree.IndependentEffects.size());
	std::set<std::string, std::less<>> VisibleIndependentIds;
	bool_t bInventoryExact = true;
	for (const VALTAN_INDEPENDENT_EFFECT_VIEW& Effect :
		m_ValtanPatternTree.IndependentEffects)
	{
		if (Effect.strIndependentEffectId.empty() ||
			Effect.strEffectAssetId.empty() ||
			Effect.strOwnerPatternId.empty() || Effect.strOwnerStageId.empty() ||
			!VisibleIndependentIds.insert(
				Effect.strIndependentEffectId).second)
		{
			bInventoryExact = false;
			continue;
		}
		IndependentRows.push_back(&Effect);
	}

	std::vector<const VALTAN_PATTERN_VIEW*> CorePatterns;
	CorePatterns.reserve(
		m_ValtanToolAuditionInventory.CorePatternIds.size());
	std::set<std::string, std::less<>> VisiblePatternIds;
	for (const std::string& strPatternId :
		m_ValtanToolAuditionInventory.CorePatternIds)
	{
		const VALTAN_PATTERN_VIEW* pPattern =
			Find_ValtanPattern(strPatternId);
		if (nullptr == pPattern ||
			!VisiblePatternIds.insert(pPattern->strPatternId).second)
		{
			bInventoryExact = false;
			continue;
		}
		CorePatterns.push_back(pPattern);
	}

	std::vector<const VALTAN_PATTERN_VIEW*> AnimatorPatterns;
	AnimatorPatterns.reserve(
		m_ValtanToolAuditionInventory.AnimatorPatternIds.size());
	for (const std::string& strPatternId :
		m_ValtanToolAuditionInventory.AnimatorPatternIds)
	{
		const VALTAN_PATTERN_VIEW* pPattern =
			Find_ValtanPattern(strPatternId);
		if (nullptr == pPattern || !pPattern->bManualServerAudition ||
			!VisiblePatternIds.insert(strPatternId).second)
		{
			bInventoryExact = false;
			continue;
		}
		AnimatorPatterns.push_back(pPattern);
	}
	std::vector<const VALTAN_PATTERN_VIEW*> DerivedPatterns;
	DerivedPatterns.reserve(m_ValtanToolAuditionInventory.DerivedPatternIds.size());
	for (const std::string& strPatternId :
		m_ValtanToolAuditionInventory.DerivedPatternIds)
	{
		const VALTAN_PATTERN_VIEW* pPattern = Find_ValtanPattern(strPatternId);
		if (nullptr == pPattern || !pPattern->bManualServerAudition ||
			"DERIVED_SERVER_PATTERN" != pPattern->strAdmissionState ||
			!VisiblePatternIds.insert(strPatternId).second)
		{
			bInventoryExact = false;
			continue;
		}
		DerivedPatterns.push_back(pPattern);
	}
	if (m_ValtanPatternTree.IndependentEffects.size() !=
			IndependentRows.size() ||
		m_ValtanToolAuditionInventory.CorePatternIds.size() !=
			CorePatterns.size() ||
		m_ValtanToolAuditionInventory.AnimatorPatternIds.size() !=
			AnimatorPatterns.size() ||
		m_ValtanToolAuditionInventory.Get_PatternCount() != VisiblePatternIds.size() ||
		m_ValtanToolAuditionInventory.DerivedPatternIds.size() != DerivedPatterns.size())
	{
		bInventoryExact = false;
	}

	if (!bInventoryExact)
	{
		ImGui::TextColored(
			ImVec4(1.f, 0.45f, 0.35f, 1.f),
			"Valtan All Effects inventory is incomplete (%zu/%zu Effects, %zu/%zu Patterns); no legacy or replacement row was substituted.",
			IndependentRows.size(),
			m_ValtanPatternTree.IndependentEffects.size(),
			VisiblePatternIds.size(),
			m_ValtanToolAuditionInventory.Get_PatternCount());
	}

#ifdef _DEBUG
	if (CMainApp* const pApp = CMainApp::Get_Active())
	{
		const std::string& strSharedPatternId =
			pApp->Debug_GetSelectedCompletePlayPatternId();
		if (!strSharedPatternId.empty() &&
			VisiblePatternIds.contains(strSharedPatternId) &&
			m_strSelectedValtanPatternId != strSharedPatternId)
		{
			m_strSelectedValtanPatternId = strSharedPatternId;
			m_SelectedValtanPatternEffect.reset();
		}
	}
#endif

	const VALTAN_PATTERN_VIEW* pSelectedPattern =
		m_strSelectedValtanPatternId.empty() ? nullptr :
			Find_ValtanPattern(m_strSelectedValtanPatternId);
	if (nullptr != pSelectedPattern &&
		!VisiblePatternIds.contains(pSelectedPattern->strPatternId))
	{
		pSelectedPattern = nullptr;
		m_strSelectedValtanPatternId.clear();
	}
	const VALTAN_PATTERN_AUTHORING_EFFECT_BINDING* pSelectedBinding =
		nullptr == pSelectedPattern ? nullptr :
			Find_ValtanPatternAuthoringEffect(
				pSelectedPattern->strPatternId);
	ImGui::SeparatorText("Pattern Authoring");
	ImGui::TextWrapped("Selected Pattern: %s",
		nullptr == pSelectedPattern ?
			"(select one Pattern row)" :
			(pSelectedPattern->strDisplayName.empty() ?
				pSelectedPattern->strPatternId.c_str() :
				pSelectedPattern->strDisplayName.c_str()));
	if (nullptr != pSelectedPattern)
	{
		ImGui::TextWrapped("%s",
			CValtanPatternTree::Build_PatternIdentitySummary(*pSelectedPattern).c_str());
	}
	std::filesystem::path AggregateEffectPath;
	bool_t bExistingAggregate = false;
	if (nullptr != pSelectedPattern)
	{
		VALTAN_PATTERN_AUTHORING_EFFECT_BINDING Aggregate;
		Aggregate.strPatternId = pSelectedPattern->strPatternId;
		Aggregate.strEffectAssetId =
			Build_ValtanPatternAggregateEffectAssetId(*pSelectedPattern);
		Aggregate.strAuthoringPath =
			CValtanPatternAuthoringEffectDocument::Build_AuthoringPath(
				Aggregate.strEffectAssetId);
		AggregateEffectPath =
			CValtanPatternAuthoringEffectDocument::Resolve_AuthoringPath(Aggregate);
		bExistingAggregate =
			CEffectCatalog::Contains(Aggregate.strEffectAssetId) ||
			m_DirectAuthoredEditableEntries.contains(Aggregate.strEffectAssetId);
	}
	const bool_t bCanCreate =
		Can_MutateValtanView(m_eValtanPatternTreeAdmission) &&
		nullptr != pSelectedPattern &&
		nullptr == pSelectedBinding &&
		m_bValtanPatternAuthoringEffectsLoaded &&
		!Has_UnsavedWork() && !bExistingAggregate &&
		!AggregateEffectPath.empty();
	ImGui::BeginDisabled(!bCanCreate);
	if (ImGui::Button("Create Effect") && nullptr != pSelectedPattern)
		Try_CreateValtanPatternEffect(*pSelectedPattern);
	ImGui::EndDisabled();
	if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
	{
		const char_t* pReason =
			"Create one empty v13 Effect and attach DRAFT_ATTACHED ownership.";
		if (!Can_MutateValtanView(m_eValtanPatternTreeAdmission))
			pReason = "Reload the canonical Valtan Pattern view before creating or changing a Pattern Effect.";
		else if (nullptr == pSelectedPattern)
			pReason = "Select one Pattern row first.";
		else if (nullptr != pSelectedBinding)
			pReason = "The selected Pattern already owns its one aggregate Effect.";
		else if (bExistingAggregate)
			pReason = "The aggregate Effect already exists. Open Existing Effect preserves its Elements and any deleted Product links.";
		else if (!m_bValtanPatternAuthoringEffectsLoaded)
			pReason = "Refresh the Pattern Effect ownership document first.";
		else if (Has_UnsavedWork())
			pReason = "Save or discard the active Effect edits first.";
		else if (AggregateEffectPath.empty())
			pReason = "The canonical Effect destination could not be resolved; Refresh before creating.";
		ImGui::SetTooltip("%s", pReason);
	}
	if (bExistingAggregate && nullptr == pSelectedBinding)
	{
		ImGui::SameLine();
		const bool_t bOpenExistingRequested = ImGui::Button("Open Existing Effect");
		if (bOpenExistingRequested)
			Try_OpenExistingValtanPatternEffect(*pSelectedPattern);
		if (ImGui::IsItemHovered())
		{
			ImGui::SetTooltip(
				"Open the preserved canonical file. An unlinked Effect stays authoring-only; deleted Product links are not restored.");
		}
		/* Product Open refreshes the joined tree even when a later check fails.
		   Discard every pointer collected for this render frame before continuing. */
		if (bOpenExistingRequested)
			return;
	}
	std::string DeleteReason;
	const bool_t bCanDelete =
		Can_DeleteSelectedValtanPatternEffect(DeleteReason);
	ImGui::SameLine();
	ImGui::BeginDisabled(!bCanDelete);
	if (ImGui::Button("Delete Effect"))
	{
		m_PendingValtanPatternEffectDeletion =
			m_SelectedValtanPatternEffect;
		ImGui::OpenPopup("Delete Effect##ValtanPatternEffect");
	}
	ImGui::EndDisabled();
	if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
		ImGui::SetTooltip("%s", DeleteReason.c_str());
	if (nullptr != pSelectedBinding)
	{
		const std::filesystem::path Path =
			CValtanPatternAuthoringEffectDocument::Resolve_AuthoringPath(
				*pSelectedBinding);
		const bool_t bActive = m_ActiveDocument.has_value() &&
			m_eActiveDocumentSource == EFFECT_DOCUMENT_SOURCE::AUTHORED &&
			m_ActiveDocument->strEffectAssetId ==
				pSelectedBinding->strEffectAssetId &&
			m_eActiveDocumentPreviewIntent ==
				EFFECT_DOCUMENT_PREVIEW_INTENT::VALTAN_PATTERN_DRAFT &&
			m_strActiveValtanPatternDraftId ==
				pSelectedPattern->strPatternId;
		ImGui::SameLine();
		ImGui::BeginDisabled(Path.empty() || bActive);
		if (ImGui::Button("Open Editor"))
		{
			Try_OpenValtanPatternDraftEffect(
				Path, pSelectedBinding->strEffectAssetId,
				*pSelectedPattern, false);
		}
		ImGui::EndDisabled();
		ImGui::SameLine();
		ImGui::BeginDisabled(Path.empty());
		if (ImGui::Button("Local Effect + Pattern Preview"))
		{
			Try_OpenValtanPatternDraftEffect(
				Path, pSelectedBinding->strEffectAssetId,
				*pSelectedPattern, true);
		}
		ImGui::EndDisabled();
	}
	bool_t bDeleteAttemptedThisFrame = false;
	if (ImGui::BeginPopupModal(
			"Delete Effect##ValtanPatternEffect", nullptr,
			ImGuiWindowFlags_AlwaysAutoResize))
	{
		if (!m_PendingValtanPatternEffectDeletion.has_value())
		{
			ImGui::TextWrapped(
				"The selected Effect changed before confirmation. Cancel and select it again.");
		}
		else
		{
			const VALTAN_PATTERN_EFFECT_SELECTION& Pending =
				*m_PendingValtanPatternEffectDeletion;
			ImGui::TextWrapped("Pattern: %s", Pending.strPatternId.c_str());
			ImGui::TextWrapped("Effect: %s", Pending.strEffectAssetId.c_str());
			if (VALTAN_PATTERN_EFFECT_SELECTION_KIND::PRODUCT_CUE_LINK ==
				Pending.eKind)
			{
				ImGui::TextWrapped(
					"Remove %zu exact Product cue connection(s) from this Pattern only?",
					Pending.CueIds.size());
				ImGui::TextDisabled(
					"The shared Effect asset, authored file, catalog row, and other Pattern links are preserved.");
			}
			else
			{
				ImGui::TextWrapped(
					"Delete this unsaved DRAFT_ATTACHED ownership and its exact authored Effect file?");
				ImGui::TextDisabled(
					"Server Pattern and Product presentation data are not changed.");
			}
		}
		if (ImGui::Button("Cancel"))
		{
			m_PendingValtanPatternEffectDeletion.reset();
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		ImGui::BeginDisabled(
			!m_PendingValtanPatternEffectDeletion.has_value());
		if (ImGui::Button("Confirm Delete"))
		{
			bDeleteAttemptedThisFrame = true;
			(void)Try_DeleteSelectedValtanPatternEffect();
			m_PendingValtanPatternEffectDeletion.reset();
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndDisabled();
		ImGui::EndPopup();
	}
	/* Delete may replace the pattern tree or sidecar vectors. Never reuse the
	   render-frame pointers captured above after a confirmed transaction. */
	if (bDeleteAttemptedThisFrame)
		return;
	if (!m_strValtanPatternEffectStatus.empty())
		ImGui::TextWrapped("%s", m_strValtanPatternEffectStatus.c_str());

	const bool_t bHasVisibleIndependent = std::any_of(
		IndependentRows.begin(), IndependentRows.end(),
		[this, &strSearch](const VALTAN_INDEPENDENT_EFFECT_VIEW* pEffect)
		{
			return nullptr != pEffect &&
				Matches_ValtanIndependentEffectSearch(*pEffect, strSearch);
		});
	const auto MatchesAreaSurface = [&strSearch](const auto& Row)
	{
		return strSearch.empty() ||
			Contains_NoCase(Row.independentEffectId, strSearch) ||
			Contains_NoCase(Row.displayName, strSearch);
	};
	const auto MatchesAreaWorld = [&strSearch](const auto& Row)
	{
		return strSearch.empty() ||
			Contains_NoCase(Row.independentEffectId, strSearch) ||
			Contains_NoCase(Row.displayName, strSearch) ||
			Contains_NoCase(Row.effectAssetId, strSearch) ||
			Contains_NoCase(Row.placementId, strSearch);
	};
	const size_t iVisibleAreaStaticCount =
		m_ValtanAreaMapEffectDocument.Is_Ready() ?
			std::ranges::count_if(
				m_ValtanAreaMapEffectDocument.Get_Surfaces(),
				MatchesAreaSurface) +
			std::ranges::count_if(
				m_ValtanAreaMapEffectDocument.Get_WorldEffects(),
				MatchesAreaWorld) : 0u;
	if (bHasVisibleIndependent || 0u != iVisibleAreaStaticCount)
	{
		if (!strSearch.empty())
			ImGui::SetNextItemOpen(true, ImGuiCond_Always);
		const std::string IndependentLabel =
			"INDEPENDENT EFFECT (PATTERN " +
			std::to_string(IndependentRows.size()) + " + AREA " +
			std::to_string(m_ValtanAreaMapEffectDocument.Is_Ready() ?
				m_ValtanAreaMapEffectDocument.Get_Surfaces().size() +
				m_ValtanAreaMapEffectDocument.Get_WorldEffects().size() : 0u) +
			")";
		if (ImGui::TreeNodeEx(
				IndependentLabel.c_str(),
				ImGuiTreeNodeFlags_OpenOnArrow))
		{
			if (bHasVisibleIndependent)
			{
				if (!strSearch.empty())
					ImGui::SetNextItemOpen(true, ImGuiCond_Always);
				const std::string PatternIndependentLabel =
					"PATTERN-OWNED INDEPENDENT EFFECT (" +
					std::to_string(IndependentRows.size()) + ")";
				if (ImGui::TreeNodeEx(
						PatternIndependentLabel.c_str(),
						ImGuiTreeNodeFlags_OpenOnArrow))
				{
					for (const VALTAN_INDEPENDENT_EFFECT_VIEW* pEffect :
						IndependentRows)
					{
						if (nullptr != pEffect)
							Render_ValtanIndependentEffectNode(
								*pEffect, strSearch);
					}
					ImGui::TreePop();
				}
			}
			Render_ValtanAreaStaticEffectSection(strSearch);
			ImGui::TreePop();
		}
	}

	const bool_t bHasVisibleCore = std::any_of(
		CorePatterns.begin(), CorePatterns.end(),
		[this, &strSearch](const VALTAN_PATTERN_VIEW* pPattern)
		{
			return nullptr != pPattern &&
				Matches_ValtanPatternSearch(*pPattern, strSearch);
		});
	if (bHasVisibleCore)
	{
		if (!strSearch.empty())
			ImGui::SetNextItemOpen(true, ImGuiCond_Always);
		if (ImGui::TreeNodeEx(
				"CORE SERVER PATTERNS", ImGuiTreeNodeFlags_OpenOnArrow,
				"CORE SERVER PATTERNS (%zu)", CorePatterns.size()))
		{
			for (const VALTAN_PATTERN_VIEW* pPattern : CorePatterns)
			{
				if (nullptr != pPattern)
					Render_ValtanPatternNode(
						*pPattern, "Core", strSearch);
			}
			ImGui::TreePop();
		}
	}

	const bool_t bHasVisibleDerived = std::any_of(
		DerivedPatterns.begin(), DerivedPatterns.end(),
		[this, &strSearch](const VALTAN_PATTERN_VIEW* pPattern)
		{ return nullptr != pPattern && Matches_ValtanPatternSearch(*pPattern, strSearch); });
	if (bHasVisibleDerived)
	{
		if (!strSearch.empty())
			ImGui::SetNextItemOpen(true, ImGuiCond_Always);
		if (ImGui::TreeNodeEx(
				"DERIVED SERVER PATTERNS", ImGuiTreeNodeFlags_OpenOnArrow,
				"DERIVED SERVER PATTERNS (%zu)", DerivedPatterns.size()))
		{
			for (const VALTAN_PATTERN_VIEW* pPattern : DerivedPatterns)
				if (nullptr != pPattern)
					Render_ValtanPatternNode(*pPattern, "Derived", strSearch);
			ImGui::TreePop();
		}
	}

	const bool_t bHasVisibleAnimator = std::any_of(
		AnimatorPatterns.begin(), AnimatorPatterns.end(),
		[this, &strSearch](const VALTAN_PATTERN_VIEW* pPattern)
		{
			return nullptr != pPattern &&
				Matches_ValtanPatternSearch(*pPattern, strSearch);
		});
	if (bHasVisibleAnimator)
	{
		if (!strSearch.empty())
			ImGui::SetNextItemOpen(true, ImGuiCond_Always);
		if (ImGui::TreeNodeEx(
				"ANIMATOR PATTERNS", ImGuiTreeNodeFlags_OpenOnArrow,
				"ANIMATOR PATTERNS (%zu)", AnimatorPatterns.size()))
		{
			for (const VALTAN_PATTERN_VIEW* pPattern : AnimatorPatterns)
			{
				if (nullptr != pPattern)
					Render_ValtanPatternNode(
						*pPattern, "Animator", strSearch);
			}
			ImGui::TreePop();
		}
	}
}

void Client::CEffect_Tool::Render_ValtanProductFallbackSection(
	const std::string& strSearch)
{
	ImGui::TextColored(
		ImVec4(1.f, 0.72f, 0.18f, 1.f),
		"READ-ONLY PRODUCT FALLBACK");
	ImGui::TextWrapped(
		"Generated Product pattern identities remain browsable. Exact authored Effect documents are listed above; Product cue ownership, Open/Play, Create/Delete, and Server playback stay blocked until the strict split join succeeds.");
	const auto Matches = [&strSearch](
		const ENCOUNTER_PATTERN_REFERENCE& Pattern)
	{
		if (strSearch.empty() ||
			Contains_NoCase(Pattern.patternId, strSearch) ||
			Contains_NoCase(Pattern.displayName, strSearch) ||
			Contains_NoCase(Pattern.actionId, strSearch))
		{
			return true;
		}
		return std::any_of(
			Pattern.stages.begin(), Pattern.stages.end(),
			[&strSearch](const ENCOUNTER_STAGE_REFERENCE& Stage)
			{
				return Contains_NoCase(Stage.stageId, strSearch) ||
					Contains_NoCase(Stage.actionId, strSearch);
			});
	};
	const auto RenderGroup = [&Matches](
		const char_t* const pLabel,
		const std::vector<const ENCOUNTER_PATTERN_REFERENCE*>& Patterns)
	{
		const size_t iVisible = std::count_if(
			Patterns.begin(), Patterns.end(),
			[&Matches](const ENCOUNTER_PATTERN_REFERENCE* const pPattern)
			{ return nullptr != pPattern && Matches(*pPattern); });
		if (0u == iVisible)
			return;
		const std::string Label = std::string(pLabel) + " (" +
			std::to_string(Patterns.size()) + ")";
		if (!ImGui::TreeNodeEx(Label.c_str(), ImGuiTreeNodeFlags_OpenOnArrow))
			return;
		for (const ENCOUNTER_PATTERN_REFERENCE* const pPattern : Patterns)
		{
			if (nullptr == pPattern || !Matches(*pPattern))
				continue;
			ImGui::PushID(pPattern->patternId.c_str());
			const std::string PatternLabel =
				(pPattern->displayName.empty() ? pPattern->patternId :
					pPattern->displayName) + "###productFallbackPattern";
			if (ImGui::TreeNodeEx(
					PatternLabel.c_str(), ImGuiTreeNodeFlags_OpenOnArrow))
			{
				ImGui::TextDisabled("%s | entry %s | %u ms",
					pPattern->patternId.c_str(), pPattern->actionId.c_str(),
					pPattern->iTotalDurationMs);
				for (const ENCOUNTER_STAGE_REFERENCE& Stage : pPattern->stages)
				{
					ImGui::BulletText("%s | %s | %s | %u ms%s",
						Stage.stageId.c_str(), Stage.stageKind.c_str(),
						Stage.actionId.c_str(), Stage.iDurationMs,
						Stage.bHasCounterHitBranch ? " | COUNTER" : "");
				}
				ImGui::TreePop();
			}
			ImGui::PopID();
		}
		ImGui::TreePop();
	};

	std::vector<const ENCOUNTER_PATTERN_REFERENCE*> Gimmicks;
	std::vector<const ENCOUNTER_PATTERN_REFERENCE*> Rotation;
	for (const ENCOUNTER_PATTERN_REFERENCE& Pattern :
		m_ValtanProductFallbackEncounter.Get_Patterns())
	{
		(0u != Pattern.iTriggerHealthBar ? Gimmicks : Rotation).push_back(
			&Pattern);
	}
	RenderGroup("PRODUCT GIMMICK PATTERNS / READ-ONLY", Gimmicks);
	RenderGroup("PRODUCT ROTATION PATTERNS / READ-ONLY", Rotation);
}
