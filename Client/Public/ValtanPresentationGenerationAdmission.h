#pragma once

#include "GameplayDataRevision.h"

#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

namespace Client
{
	struct VALTAN_PRESENTATION_GENERATION_ARTIFACT_RECEIPT final
	{
		std::string strRelativePath;
		std::string strLane;
		LostArk::Shared::GameplayDataRevision Revision{};
		std::uint64_t iBytes = 0u;

		bool operator==(
			const VALTAN_PRESENTATION_GENERATION_ARTIFACT_RECEIPT&) const = default;
	};

	/* Read-only receipt for the existing typed Valtan Product documents.  It is
	   deliberately not a second presentation owner: Gameplay.bootstrap commits
	   the immutable manifest ID, while each artifact remains owned and parsed by
	   its established typed loader. */
	struct VALTAN_PRESENTATION_GENERATION_RECEIPT final
	{
		LostArk::Shared::GameplayDataRevision ServerGameplayRevision{};
		LostArk::Shared::GameplayDataRevision PresentationGenerationId{};
		std::vector<VALTAN_PRESENTATION_GENERATION_ARTIFACT_RECEIPT> Artifacts;

		bool Is_Valid() const;
		bool operator==(
			const VALTAN_PRESENTATION_GENERATION_RECEIPT&) const = default;
	};

	/* Holds the canonical Valtan Product writer admission while validating the
	   exact R -> M -> artifact closure.  Callers stage their typed caches while
	   this object lives, then call Validate_StillCurrent before committing. */
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
		bool Acquire_Receipt(
			const LostArk::Shared::GameplayDataRevision& ExpectedServerRevision,
			const VALTAN_PRESENTATION_GENERATION_RECEIPT& ExpectedReceipt,
			std::string& strOutStatus);

		/* Explicit-root overloads exist only for deterministic native oracles. */
		bool Acquire_PackagedBaselineFromRoot(
			const std::filesystem::path& RepositoryRoot,
			VALTAN_PRESENTATION_GENERATION_RECEIPT& OutReceipt,
			std::string& strOutStatus);
		bool Acquire_ReceiptFromRoot(
			const std::filesystem::path& RepositoryRoot,
			const LostArk::Shared::GameplayDataRevision& ExpectedServerRevision,
			const VALTAN_PRESENTATION_GENERATION_RECEIPT& ExpectedReceipt,
			std::string& strOutStatus);

		bool Validate_StillCurrent(std::string& strOutStatus) const;
		bool Is_Acquired() const;

	private:
		struct STATE;
		std::unique_ptr<STATE> m_pState;
	};
}
