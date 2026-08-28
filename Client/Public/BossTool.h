#pragma once

#include "Client_Defines.h"
#include "CameraTool.h"
#include "Effect_AuthoringDocument.h"
#include "EncounterPatternReference.h"
#include "ValtanCinematicCameraDocument.h"
#include "ValtanPatternFlowDocument.h"
#include "ValtanPatternTree.h"

#include <array>
#include <filesystem>
#include <map>
#include <memory>
#include <string>
#include <vector>

NS_BEGIN(Client)

class IPlayerCommandSink;
struct EFFECT_TOOL_VALTAN_PRODUCT_OPEN_REQUEST;

/* A thin observer/controller over the existing Valtan product path.
   It never samples clips, spawns Effects, or mutates gameplay locally. */
class CBossTool final
{
private:
	struct EFFECT_DOCUMENT_CACHE_ENTRY final
	{
		bool_t bLoadAttempted = false;
		bool_t bLoaded = false;
		bool_t bHasLastWriteTime = false;
		bool_t bNextSpawnCatalogEquivalent = false;
		size_t iGeneration = 0u;
		std::filesystem::path Path;
		std::filesystem::file_time_type LastWriteTime{};
		EFFECT_DOCUMENT_DESC Document;
		std::shared_ptr<const EFFECT_DOCUMENT_DESC> pNextSpawnCatalogDocument;
		std::string strStatus;
		std::string strNextSpawnCatalogStatus;
	};

	struct RESOURCE_OWNER_RESULT final
	{
		std::string strPatternId;
		std::string strStageId;
		std::string strEffectAssetId;
		std::string strElementId;
		std::string strSlotId;
		std::string strResourceAssetId;
		bool_t bNextSpawnCatalogVerified = false;
	};

public:
	explicit CBossTool(std::shared_ptr<IPlayerCommandSink> CommandSink);
	void Open();
	void Update(bool_t bToolVisible);
	void Render();
	bool_t Consume_CameraToolOpenRequest(
		CAMERA_TOOL_OPEN_REQUEST& outRequest);
	bool_t Consume_EffectToolOpenRequest(
		EFFECT_TOOL_VALTAN_PRODUCT_OPEN_REQUEST& outRequest);

private:
	bool_t Reload_Graph();
	bool_t Submit_SelectedPattern();
	bool_t Preview_SelectedFlowSlotIsolated();
	bool_t Start_Flow(bool_t bFromSelectedSlot);
	bool_t Request_RevivePlayer(std::string& strOutStatus);
	bool_t Reload_FlowDocument();
	bool_t Save_FlowDocument();
	void Refresh_PresentationFreshness(bool_t bForce = false);
	void Synchronize_LiveSelection();
	void Render_BossVerificationTab();
	void Render_PatternFlowTab();
	void Render_FlowSlotList();
	void Render_FlowSelectedSlot();
	void Render_AddPatternPopup();
	void Render_NextPatternCard();
	void Render_NextPatternPicker();
	void Render_LiveSummary();
	void Render_ActionBar();
	void Render_PatternList();
	void Render_SelectedPattern();
	void Render_ConnectionSummary(
		const VALTAN_PATTERN_VIEW& Pattern,
		const VALTAN_STAGE_VIEW& Stage);
	void Render_AdvancedDiagnostics(
		const VALTAN_PATTERN_VIEW& Pattern,
		const VALTAN_STAGE_VIEW& Stage);
	void Render_EffectDocument(
		const std::string& strEffectAssetId,
		const std::filesystem::path& PreferredPath);
	void Search_ResourceOwners();
	void Refresh_ResourceSearchFreshness();

	const VALTAN_PATTERN_VIEW* Find_Pattern(
		const std::string& strPatternId) const;
	const VALTAN_PATTERN_VIEW* Find_AuditionPattern(
		const std::string& strPatternId) const;
	const VALTAN_STAGE_VIEW* Find_LiveStage(
		const VALTAN_PATTERN_VIEW& Pattern) const;
	const VALTAN_STAGE_VIEW* Find_SelectedStage(
		const VALTAN_PATTERN_VIEW& Pattern) const;
	const VALTAN_INDEPENDENT_EFFECT_VIEW* Find_IndependentEffect(
		const std::string& strIndependentEffectId) const;
	const VALTAN_CINEMATIC_CAMERA_CUE* Find_CameraCue(
		const std::string& strCueId) const;
	const VALTAN_PATTERN_FLOW_SLOT* Find_SelectedFlowSlot() const;
	std::vector<std::string> Build_AdmittedPatternIds() const;
	std::filesystem::path Resolve_EffectDocumentPath(
		const std::string& strEffectAssetId,
		const std::filesystem::path& PreferredPath) const;
	EFFECT_DOCUMENT_CACHE_ENTRY& Load_EffectDocument(
		const std::string& strEffectAssetId,
		const std::filesystem::path& PreferredPath);

private:
	VALTAN_PATTERN_TREE_VIEW m_Graph;
	VALTAN_TOOL_AUDITION_INVENTORY m_AuditionInventory;
	std::vector<std::string> m_NextPatternIds;
	CEncounterPatternReference m_EncounterReference;
	CValtanCinematicCameraDocument m_CameraDocument;
	CValtanPatternFlowDocument m_FlowDocument;
	std::map<std::string, EFFECT_DOCUMENT_CACHE_ENTRY, std::less<>>
		m_EffectDocuments;
	std::map<std::string, size_t, std::less<>>
		m_ResourceSearchDocumentGenerations;
	std::vector<RESOURCE_OWNER_RESULT> m_ResourceOwnerResults;
	std::shared_ptr<IPlayerCommandSink> m_pCommandSink;

	std::array<char_t, 128u> m_PatternSearch{};
	std::array<char_t, 128u> m_FlowPatternSearch{};
	std::array<char_t, 128u> m_NextPatternSearch{};
	std::array<char_t, 256u> m_ResourceSearch{};
	std::string m_strSelectedPatternId;
	std::string m_strSelectedStageId;
	std::string m_strSelectedFlowSlotId;
	std::string m_strRepeatPatternId;
	std::string m_strStatus =
		"Select a pattern, then play it through the Server.";
	std::string m_strFlowStatus =
		"Load a saved Flow, then start it through the Server.";
	std::string m_strNextPatternStatus =
		"Choose a Next Pattern during an isolated audition or its completed hold.";
	std::string m_strCameraStatus;
	std::string m_strDiagnosticStatus;
	std::string m_strActionFeedback;
	std::string m_strPresentationFreshnessStatus;
	std::string m_strPresentationFreshnessRevision;
	size_t m_iResourceSearchLoadFailureCount = 0u;
	size_t m_iResourceSearchUnverifiedCount = 0u;
	double m_dNextResourceSearchFreshnessCheckSeconds = 0.0;
	uint32_t m_iNextReviveSequence = 1u;
	bool_t m_bOpen = false;
	bool_t m_bFocusPending = false;
	bool_t m_bGraphLoadAttempted = false;
	bool_t m_bGraphReady = false;
	bool_t m_bNextPatternInventoryReady = false;
	bool_t m_bFollowLive = true;
	bool_t m_bRepeat = false;
	bool_t m_bReviveFeedbackPending = false;
	bool_t m_bPresentationBaselineIntact = false;
	bool_t m_bResourceSearchStale = false;
	bool_t m_bConfirmDiscardDirtyFlow = false;
	bool_t m_hasCameraToolOpenRequest = false;
	CAMERA_TOOL_OPEN_REQUEST m_CameraToolOpenRequest;
	bool_t m_hasEffectToolOpenRequest = false;
	std::string m_strEffectToolOpenPatternId;
	std::string m_strEffectToolOpenStageId;
	std::string m_strEffectToolOpenCueOccurrenceId;
	std::string m_strEffectToolOpenEffectAssetId;
};

NS_END
