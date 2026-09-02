#pragma once

#include "GameplayDataRevision.h"

#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

namespace Client
{
	struct VALTAN_CANONICAL_READ_DIAGNOSTIC;

	struct VALTAN_PRESENTATION_GENERATION_ARTIFACT_RECEIPT final
	{
		std::string strRelativePath;
		std::string strLane;
		LostArk::Shared::GameplayDataRevision Revision{};
		std::uint64_t iBytes = 0u;

		bool operator==(
			const VALTAN_PRESENTATION_GENERATION_ARTIFACT_RECEIPT&) const = default;
	};

	/* Read-only transaction receipt for the existing typed Valtan Product
	   documents. It is deliberately not a second presentation owner: each
	   artifact remains owned and parsed by its established typed loader. */
	struct VALTAN_PRESENTATION_GENERATION_RECEIPT final
	{
		LostArk::Shared::GameplayDataRevision ServerGameplayRevision{};
		LostArk::Shared::GameplayDataRevision PresentationGenerationId{};
		std::vector<VALTAN_PRESENTATION_GENERATION_ARTIFACT_RECEIPT> Artifacts;

		bool Is_Valid() const;
		bool operator==(
			const VALTAN_PRESENTATION_GENERATION_RECEIPT&) const = default;
	};

	/* Holds canonical Valtan Product writer admission while pinning the current
	   typed source closure to one valid Server gameplay revision. The caller's
	   historical presentation receipt is not an equality gate. Callers stage
	   typed caches while this object lives, then call Validate_StillCurrent before
	   committing. */
	class CValtanPresentationGenerationReadAdmission final
	{
	public:
		CValtanPresentationGenerationReadAdmission();
		~CValtanPresentationGenerationReadAdmission();
		CValtanPresentationGenerationReadAdmission(
			const CValtanPresentationGenerationReadAdmission&) = delete;
		CValtanPresentationGenerationReadAdmission& operator=(
			const CValtanPresentationGenerationReadAdmission&) = delete;

		bool Acquire_PackagedBaseline(
			VALTAN_PRESENTATION_GENERATION_RECEIPT& OutReceipt,
			std::string& strOutStatus);
		bool Acquire_PackagedBaseline(
			VALTAN_PRESENTATION_GENERATION_RECEIPT& OutReceipt,
			VALTAN_CANONICAL_READ_DIAGNOSTIC& OutDiagnostic);
		bool Acquire_Receipt(
			const LostArk::Shared::GameplayDataRevision& ExpectedServerRevision,
			const VALTAN_PRESENTATION_GENERATION_RECEIPT& ExpectedReceipt,
			std::string& strOutStatus);
		/* Network generation activation uses the exact receipt retained at
		   PREPARE. Unlike the ordinary authoring reload above, this rejects a
		   physical closure that changed after that generation was staged. */
		bool Acquire_ExactReceipt(
			const LostArk::Shared::GameplayDataRevision& ExpectedServerRevision,
			const VALTAN_PRESENTATION_GENERATION_RECEIPT& ExpectedReceipt,
			std::string& strOutStatus);

		/* Explicit-root overloads exist only for deterministic native oracles. */
		bool Acquire_PackagedBaselineFromRoot(
			const std::filesystem::path& RepositoryRoot,
			VALTAN_PRESENTATION_GENERATION_RECEIPT& OutReceipt,
			std::string& strOutStatus);
		bool Acquire_PackagedBaselineFromRoot(
			const std::filesystem::path& RepositoryRoot,
			VALTAN_PRESENTATION_GENERATION_RECEIPT& OutReceipt,
			VALTAN_CANONICAL_READ_DIAGNOSTIC& OutDiagnostic);
		bool Acquire_ReceiptFromRoot(
			const std::filesystem::path& RepositoryRoot,
			const LostArk::Shared::GameplayDataRevision& ExpectedServerRevision,
			const VALTAN_PRESENTATION_GENERATION_RECEIPT& ExpectedReceipt,
			std::string& strOutStatus);
		bool Acquire_ExactReceiptFromRoot(
			const std::filesystem::path& RepositoryRoot,
			const LostArk::Shared::GameplayDataRevision& ExpectedServerRevision,
			const VALTAN_PRESENTATION_GENERATION_RECEIPT& ExpectedReceipt,
			std::string& strOutStatus);

		bool Validate_StillCurrent(std::string& strOutStatus) const;
		bool Validate_StillCurrent(
			VALTAN_CANONICAL_READ_DIAGNOSTIC& OutDiagnostic) const;
		bool Try_Get_CurrentReceipt(
			VALTAN_PRESENTATION_GENERATION_RECEIPT& OutReceipt) const;
		bool Is_Acquired() const;

	private:
		struct STATE;
		std::unique_ptr<STATE> m_pState;
	};
}
