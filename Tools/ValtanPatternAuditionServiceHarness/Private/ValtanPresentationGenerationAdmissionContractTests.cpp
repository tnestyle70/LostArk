#include "ProjectDataRoot.h"
#include "ValtanPresentationGenerationAdmission.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <system_error>

namespace
{
	constexpr const char* TEMPORARY_ROOT_PREFIX =
		"LostArk.ValtanPresentationGenerationAdmission.";

	struct SCOPED_TEMPORARY_ROOT final
	{
		std::filesystem::path Path;

		~SCOPED_TEMPORARY_ROOT()
		{
			if (Path.empty() ||
				!Path.filename().string().starts_with(TEMPORARY_ROOT_PREFIX))
			{
				return;
			}
			std::error_code ignored;
			std::filesystem::remove_all(Path, ignored);
		}
	};

	std::filesystem::path Create_TemporaryRoot(std::string& status)
	{
		std::error_code error;
		const std::filesystem::path base =
			std::filesystem::temp_directory_path(error);
		if (error)
		{
			status = "Could not resolve the temporary directory: " +
				error.message();
			return {};
		}
		const auto nonce =
			std::chrono::steady_clock::now().time_since_epoch().count();
		for (unsigned int attempt = 0u; attempt < 16u; ++attempt)
		{
			const std::filesystem::path candidate = base /
				(TEMPORARY_ROOT_PREFIX + std::to_string(nonce) + "." +
					std::to_string(attempt));
			error.clear();
			if (std::filesystem::create_directories(candidate, error))
				return candidate;
			if (error)
			{
				status = "Could not create the temporary presentation root: " +
					error.message();
				return {};
			}
		}
		status = "Could not allocate a unique temporary presentation root.";
		return {};
	}

	bool Copy_FixtureFile(
		const std::filesystem::path& sourceRoot,
		const std::filesystem::path& targetRoot,
		const std::filesystem::path& relativePath,
		std::string& status)
	{
		std::error_code error;
		const std::filesystem::path destination = targetRoot / relativePath;
		std::filesystem::create_directories(destination.parent_path(), error);
		if (error)
		{
			status = "Could not create a presentation fixture directory: " +
				error.message();
			return false;
		}
		if (!std::filesystem::copy_file(
				sourceRoot / relativePath, destination,
				std::filesystem::copy_options::overwrite_existing, error))
		{
			status = "Could not copy presentation fixture file " +
				relativePath.generic_string() + ": " + error.message();
			return false;
		}
		return true;
	}
}

int Run_ValtanPresentationGenerationAdmissionContractTests()
{
	using Client::CValtanPresentationGenerationReadAdmission;
	using Client::VALTAN_PRESENTATION_GENERATION_RECEIPT;

	int failures = 0;
	const auto require = [&failures](const bool condition, const char* message)
	{
		if (condition)
			return;
		++failures;
		std::cerr << "FAIL presentation generation admission: " << message << '\n';
	};

	VALTAN_PRESENTATION_GENERATION_RECEIPT receipt;
	std::string status;
	{
		CValtanPresentationGenerationReadAdmission admission;
		require(admission.Acquire_PackagedBaseline(receipt, status),
			"packaged Gameplay.bootstrap R did not admit its exact M closure");
		require(receipt.Is_Valid(), "successful admission returned no exact receipt");
		require(admission.Validate_StillCurrent(status),
			"exact M closure changed before commit");
	}

	if (receipt.Is_Valid())
	{
		CValtanPresentationGenerationReadAdmission admission;
		require(admission.Acquire_Receipt(
			receipt.ServerGameplayRevision, receipt, status),
			"exact previously admitted receipt did not re-open");
		require(admission.Validate_StillCurrent(status),
			"re-opened exact receipt changed before commit");

		auto wrongRevision = receipt.ServerGameplayRevision;
		wrongRevision.Bytes[0] ^= 0xffu;
		CValtanPresentationGenerationReadAdmission wrongAdmission;
		require(!wrongAdmission.Acquire_Receipt(
			wrongRevision, receipt, status),
			"a different Server R re-labelled the existing receipt");

		auto wrongManifest = receipt;
		wrongManifest.PresentationGenerationId.Bytes[0] ^= 0xffu;
		CValtanPresentationGenerationReadAdmission wrongManifestAdmission;
		require(!wrongManifestAdmission.Acquire_Receipt(
			receipt.ServerGameplayRevision, wrongManifest, status),
			"a different presentation M admitted the current physical closure");

		/* Simulate a canonical commit which reached disk after world-entry M was
		   pinned, while its command receipt/reload was lost. The same recomputed
		   admission used by Complete Play must reject the changed physical bytes
		   without relying on process-local publication state. */
		SCOPED_TEMPORARY_ROOT fixture;
		fixture.Path = Create_TemporaryRoot(status);
		require(!fixture.Path.empty(),
			"could not create a physical-generation mismatch fixture");
		bool fixtureCopied = !fixture.Path.empty();
		const std::filesystem::path repositoryRoot =
			Client::CProjectDataRoot::Get().parent_path();
		const std::filesystem::path bootstrap =
			"Server/Bin/DataFiles/Gameplay/Gameplay.bootstrap";
		const std::filesystem::path manifest =
			std::filesystem::path(
				"Server/Bin/DataFiles/Gameplay/ValtanPresentationGenerations") /
			(LostArk::Shared::Format_GameplayDataRevision(
				receipt.PresentationGenerationId) + ".json");
		if (fixtureCopied)
			fixtureCopied = Copy_FixtureFile(
				repositoryRoot, fixture.Path, bootstrap, status);
		if (fixtureCopied)
			fixtureCopied = Copy_FixtureFile(
				repositoryRoot, fixture.Path, manifest, status);
		for (const auto& artifact : receipt.Artifacts)
		{
			if (!fixtureCopied)
				break;
			fixtureCopied = Copy_FixtureFile(
				repositoryRoot, fixture.Path,
				std::filesystem::path(artifact.strRelativePath), status);
		}
		require(fixtureCopied,
			"could not copy the exact physical presentation closure");

		VALTAN_PRESENTATION_GENERATION_RECEIPT fixtureReceipt;
		bool fixtureAdmitted = false;
		if (fixtureCopied)
		{
			CValtanPresentationGenerationReadAdmission fixtureAdmission;
			fixtureAdmitted = fixtureAdmission.Acquire_PackagedBaselineFromRoot(
				fixture.Path, fixtureReceipt, status);
			require(fixtureAdmitted,
				"the exact copied physical presentation closure was rejected");
			require(!fixtureAdmitted || fixtureReceipt == receipt,
				"the copied physical closure changed its immutable receipt");
		}

		bool artifactChanged = false;
		if (fixtureAdmitted && !fixtureReceipt.Artifacts.empty())
		{
			const std::filesystem::path changedArtifact = fixture.Path /
				std::filesystem::path(
					fixtureReceipt.Artifacts.front().strRelativePath);
			std::ofstream stream(
				changedArtifact, std::ios::binary | std::ios::app);
			if (stream)
			{
				stream.put('\n');
				stream.flush();
				artifactChanged = stream.good();
			}
		}
		require(artifactChanged,
			"could not mutate one copied presentation artifact");
		if (artifactChanged)
		{
			CValtanPresentationGenerationReadAdmission changedAdmission;
			require(!changedAdmission.Acquire_ReceiptFromRoot(
					fixture.Path, fixtureReceipt.ServerGameplayRevision,
					fixtureReceipt, status),
				"changed physical Product bytes re-opened the old world-entry M");
		}
	}

	std::cout << "ValtanPresentationGenerationAdmissionContractTests: " <<
		(0 == failures ? "PASS" : "FAIL") <<
		" (Pattern Sound remains an independent typed receipt)\n";
	return failures;
}
