#pragma once

#include "Client_Defines.h"

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace Client
{
	/* K-only Server playback surface. Authoring stays in the separate Action
	   Workbench; this tool lists only saved PRODUCT patterns and never imports a
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
			std::string strPatternId;
			std::string strDisplayName;
			std::string strCategory;
			std::string strLoadError;
			std::vector<PRODUCT_STAGE> Stages;
		};

		CKoukuSaydonBossTool() = default;

		void Open();
		[[nodiscard]] bool Is_Open() const noexcept { return m_bOpen; }
		void Render();
		bool Reload(std::string& outStatus);
		bool Play_PatternById(
			std::string_view patternId,
			std::uint32_t expectedSourceRevision,
			std::string& outStatus);
		/* Start Full Pattern: the saved PRODUCT Play All order on the Server. */
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
		void Normalize_Selection();
		[[nodiscard]] const PRODUCT_PATTERN*
			Find_SelectedPattern() const;

	private:
		std::vector<PRODUCT_PATTERN> m_ProductPatterns;
		std::vector<std::string> m_PlayAllPatternIds;
		std::string m_strSelectedPatternId;
		std::string m_strStatus;
		std::uint32_t m_iSourceRevision = 0u;
		bool m_bOpen = true;
		bool m_bLoadAttempted = false;
		bool m_bHasSavedComposition = false;
	};
}
