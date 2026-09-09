#pragma once

#include "Client_Defines.h"

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace Client
{
	/* K-only Server playback surface. Authoring stays in the separate Action
	   Workbench; this tool lists the full published Boss Patterns tree and never imports a
	   Valtan pattern tree, flow, counter, effect, sound, or arena controller. */
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

		struct PRODUCT_FOLDER final { std::string strFolderId, strGateId, strDisplayName; };
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
		bool Play_BundleById(std::string_view bundleId, std::uint32_t expectedSourceRevision, std::string& status);
		// F1 inventory selection resolves the latest published revision at the click.
		bool Play_SavedPatternById(std::string_view patternId, std::string& status);
		bool Play_SavedBundleById(std::string_view bundleId, std::string& status);
		CKoukuSaydonBossTool() = default;

		void Open();
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
		bool Play_Selected(std::string& outStatus);
		bool Play_LoadedPatternById(std::string_view patternId, std::string& status);
		bool Play_LoadedBundleById(std::string_view bundleId, std::string& status);
		void Normalize_Selection();
		[[nodiscard]] const PRODUCT_PATTERN*
			Find_SelectedPattern() const;
		[[nodiscard]] const PRODUCT_BUNDLE* Find_SelectedBundle() const;

	private:
		std::vector<PRODUCT_FOLDER> m_ProductFolders;
		std::vector<PRODUCT_BUNDLE> m_ProductBundles;
		std::vector<PRODUCT_PATTERN> m_ProductPatterns;
		std::vector<std::string> m_PlayAllPatternIds;
		std::string m_strSelectedInventoryId;
		int m_iSelectedInventoryKind = 0;
		int m_iSelectedGate = 0;
		std::string m_strStatus;
		std::uint32_t m_iSourceRevision = 0u;
		bool m_bOpen = true;
		bool m_bLoadAttempted = false;
		bool m_bHasSavedComposition = false;
	};
}
