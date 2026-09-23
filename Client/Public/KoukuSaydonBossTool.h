#pragma once

#include "Client_Defines.h"
#include "KoukuSaydonCompositionDocument.h"
#include "GameplayDataRevision.h"

#include <cstdint>
#include <functional>
#include <optional>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace Client
{
	struct KOUKU_SAYDON_PATTERN_FLOW_ENTRY;
    struct KOUKU_DRAFT_PLAY_REQUEST;
    struct KOUKU_SAYDON_DRAFT_PRODUCT;
	/* K-only Server playback and ordered Pattern Flow authoring. Pattern/Bundle
	   definitions stay in Composition; Flow stores stable references to them. */
	class CKoukuSaydonBossTool final
	{
	public:
		struct PRODUCT_STAGE final
		{
			std::string strStageId;
			std::string strActionId;
			std::string strStageKind;
			std::uint32_t iDurationMs = 0u;
		};

		struct PRODUCT_PATTERN final
		{
			std::string strGateId = "GATE1";
			std::string strFolderId;
			std::string strTargetBossPlacementId;
			std::string strActorProfileId;
			std::string strPatternId;
			std::string strDisplayName;
			std::string strCategory;
			std::string strLoadError;
			std::vector<PRODUCT_STAGE> Stages;
		};

		struct PRODUCT_FOLDER final { std::string strFolderId, strGateId, strDisplayName, strTimelinePatternId; };
		struct PRODUCT_BUNDLE_MEMBER final {
			std::string strMemberId, strPatternId, strTargetBossPlacementId, strActorProfileId;
			std::uint32_t iStartOffsetMs = 0;
		};
		struct PRODUCT_BUNDLE final {
			std::string strBundleId, strFolderId, strGateId, strDisplayName, strLoadError;
			std::uint32_t iDurationMs = 0;
			std::vector<PRODUCT_BUNDLE_MEMBER> Members;
		};
		const std::vector<PRODUCT_FOLDER>& Get_ProductFolders() const { return m_ProductFolders; }
		const std::vector<PRODUCT_BUNDLE>& Get_ProductBundles() const { return m_ProductBundles; }
		bool Play_Draft(KOUKU_DRAFT_PLAY_REQUEST request, std::string& status);
		bool Play_BundleById(std::string_view bundleId, std::uint32_t expectedSourceRevision, std::string& status);
		// F1 inventory selection resolves the latest published revision at the click.
		bool Play_SavedPatternById(std::string_view patternId, std::string& status);
		bool Play_SavedBundleById(std::string_view bundleId, std::string& status);
		CKoukuSaydonBossTool() = default;

		void Open();
		void Update();
		bool Is_PlayPreparationPending() const noexcept { return m_PlayPreparation.has_value(); }
		bool Cancel_PlayPreparation(std::string& status);
		[[nodiscard]] bool Is_Open() const noexcept { return m_bOpen; }
		void Render();
		// Shared F1 tree; selection kinds are 0 none, 1 parent, 2 bundle, 3 pattern.
		bool Render_PatternTree(std::string_view gateId, int& selectionKind, std::string& selectedId) const;
		bool Reload(std::string& outStatus);
		bool Play_PatternById(
			std::string_view patternId,
			std::uint32_t expectedSourceRevision,
			std::string& outStatus);
		/* Start Full Pattern: the executable published Play All order on the Server. */
		bool Play_All(std::string& outStatus);
		bool Play_CompositionAll(std::string_view gateId, std::string& status);
		bool Validate_PatternFlow(std::string_view gateId, std::string& status);
        // Complete Play pins the admission revision; ordinary F1 playback uses the current saved revision.
        bool Play_PatternFlow(std::string_view gateId, std::string& status,
            std::uint32_t expectedSourceRevision = 0u);
		const KOUKU_SAYDON_COMPOSITION_PATTERN_FLOW* Get_SavedFlow(std::string_view gateId) const;
		bool Render_SavedPatternFlow(std::string_view gateId, int& selectionKind, std::string& selectedId) const;
		bool Request_PublishSavedPatterns(std::string& status);
		bool Consume_PublishRequest() { const bool requested = m_bPublishRequested; m_bPublishRequested = false; return requested; }
		void Set_Status(std::string status) { m_strStatus = std::move(status); }
		/* Read-only inventory for the F1 hub's KoukuSaydon Complete Play list.
		The tool stays the single owner of reload, selection and Server play. */
		[[nodiscard]] bool Has_SavedComposition() const noexcept
		{
			return m_bHasSavedComposition;
		}
		[[nodiscard]] const std::vector<PRODUCT_PATTERN>& Get_ProductPatterns()
			const noexcept
		{
			return m_ProductPatterns;
		}
		[[nodiscard]] const std::vector<std::string>& Get_PlayAllPatternIds()
			const noexcept
		{
			return m_PlayAllPatternIds;
		}
		[[nodiscard]] std::uint32_t Get_SourceRevision() const noexcept
		{
			return m_iSourceRevision;
		}
		[[nodiscard]] const std::string& Get_Status() const noexcept
		{
			return m_strStatus;
		}

	private:
		struct PLAY_PREPARATION final
		{
			std::vector<std::string> TargetPlacementIds, PatternIds, BundleIds;
			bool bPreparingResources = false;
            std::shared_ptr<const KOUKU_SAYDON_DRAFT_PRODUCT> Draft;
			LostArk::Shared::GameplayDataRevision GameplayRevision;
			std::uint32_t iSourceRevision = 0u;
			std::uint64_t iWorldGeneration = 0u;
			std::uint64_t iDeadlineMilliseconds = 0u;
			std::uint32_t iGateGeneration = 0u;
			std::uint32_t iPreviousRequestSequence = 0u;
			std::size_t iGateIndex = 0u;
			std::function<bool(std::string&)> Submit;
		};
		bool Prepare_ServerPlay(std::string_view gateId, std::vector<std::string> targets,
            std::vector<std::string> patternIds, std::vector<std::string> bundleIds,
			std::function<bool(std::string&)> submit, std::string& status,
            std::shared_ptr<const KOUKU_SAYDON_DRAFT_PRODUCT> draft = {});
		std::optional<PLAY_PREPARATION> m_PlayPreparation;

		bool Play_Selected(std::string& outStatus);
		bool Play_LoadedPatternById(std::string_view patternId, std::string& status);
		bool Play_LoadedBundleById(std::string_view bundleId, std::string& status);
		bool Play_LoadedFlow(std::string_view gateId,
			const std::vector<KOUKU_SAYDON_PATTERN_FLOW_ENTRY>& entries, std::string& status, std::string_view loopStartEntryId = {});
		void Normalize_Selection();
		[[nodiscard]] const PRODUCT_PATTERN*
			Find_SelectedPattern() const;
		[[nodiscard]] const PRODUCT_BUNDLE* Find_SelectedBundle() const;
		bool Load_PatternFlows(std::string& status);
		bool Save_PatternFlows(std::string& status);
		bool Prepare_PatternFlow(std::string_view gateId, KOUKU_SAYDON_COMPOSITION_PATTERN_FLOW& flow, std::string& status);
		void Render_PatternFlowEditor(std::string_view gateId);
		KOUKU_SAYDON_COMPOSITION_PATTERN_FLOW* Find_DraftFlow(std::string_view gateId);
		std::string Describe_FlowEntry(const KOUKU_SAYDON_COMPOSITION_FLOW_ENTRY& entry, std::string_view gateId, std::string& error) const;

	private:
		std::vector<PRODUCT_FOLDER> m_ProductFolders;
		std::vector<PRODUCT_BUNDLE> m_ProductBundles;
		std::vector<PRODUCT_PATTERN> m_ProductPatterns;
		std::vector<std::string> m_PlayAllPatternIds;
		CKoukuSaydonCompositionDocument m_FlowDocument;
		std::vector<KOUKU_SAYDON_COMPOSITION_PATTERN_FLOW> m_FlowDraft;
		std::string m_strSelectedFlowEntryId;
		bool m_bFlowDirty = false;
		bool m_bPublishRequested = false;
		int m_iFlowAddKind = 0;
		std::string m_strSelectedInventoryId;
		int m_iSelectedInventoryKind = 0;
		int m_iSelectedGate = 0;
		std::string m_strStatus;
		std::string m_strProductLoadError;
		std::string m_strFlowLoadError;
		std::uint32_t m_iSourceRevision = 0u;
		bool m_bOpen = true;
		bool m_bLoadAttempted = false;
		bool m_bReplayGate3OnEncore = true;
		bool m_bHasSavedComposition = false;
	};
}
