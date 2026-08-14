#pragma once

#include <cstddef>
#include <filesystem>
#include <string>
#include <vector>

namespace ClientFrontendHarness
{
	struct FOUR_CLASS_TRACK_A_MATERIALIZER_RESULT final
	{
		std::size_t iStageCount = 0u;
		std::size_t iLegacyStarterStageCount = 0u;
		std::size_t iLegacyStarterClipOccurrenceCount = 0u;
		std::size_t iLegacyStarterVisualClipOccurrenceCount = 0u;
		std::size_t iLegacyStarterSilentClipOccurrenceCount = 0u;
		std::size_t iLegacyStarterCandidateDocumentCount = 0u;
		std::size_t iTotalCandidateDocumentCount = 0u;
		std::size_t iElementPlanCount = 0u;
		std::size_t iGenericImportAttemptCount = 0u;
		std::size_t iGenericImportValidationCount = 0u;
		std::size_t iPortableCarrierCommitCount = 0u;
		std::size_t iPreservedElementCount = 0u;
		std::vector<std::string> GenericImportRejections;
		std::vector<std::string> QuarantinedElements;
		std::string strDimensionMaster2050500PreservedSemanticSha256;
		bool bApplied = false;
	};

	bool Run_FourClassTrackAAuthoredMaterializer(
		const std::filesystem::path& BatchPath,
		bool bApply,
		bool bApprovedCandidateWrite,
		FOUR_CLASS_TRACK_A_MATERIALIZER_RESULT& OutResult,
		std::string& OutError);

	bool Test_FourClassTrackAAuthoredMaterializerTransaction(
		std::string& OutError);
}
