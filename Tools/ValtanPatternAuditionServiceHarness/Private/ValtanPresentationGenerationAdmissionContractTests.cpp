#include "ProjectDataRoot.h"
#include "ValtanPresentationGenerationAdmission.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <set>
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

	bool Replace_FixtureText(
		const std::filesystem::path& path,
		const std::string& expected,
		const std::string& replacement,
		std::string& status)
	{
		std::ifstream input(path, std::ios::binary);
		if (!input)
		{
			status = "Could not open the presentation fixture for mutation: " +
				path.string();
			return false;
		}
		std::string bytes{
			std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>() };
		const std::size_t offset = bytes.find(expected);
		if (std::string::npos == offset ||
			std::string::npos != bytes.find(expected, offset + expected.size()))
		{
			status = "Presentation fixture mutation source was not unique: " +
				expected;
			return false;
		}
		bytes.replace(offset, expected.size(), replacement);
		std::ofstream output(path, std::ios::binary | std::ios::trunc);
		if (!output)
		{
			status = "Could not open the presentation fixture mutation output: " +
				path.string();
			return false;
		}
		output.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));
		output.flush();
		if (!output.good())
		{
			status = "Could not write the presentation fixture mutation: " +
				path.string();
			return false;
		}
		return true;
	}

	bool Duplicate_FirstBindingRow(
		const std::filesystem::path& path,
		std::string& status)
	{
		std::ifstream input(path, std::ios::binary);
		if (!input)
		{
			status = "Could not open the Effect V2 binding fixture.";
			return false;
		}
		std::string bytes{
			std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>() };
		const std::string firstIdentity =
			"    { \"effectId\": \"boss.valtan.hand_1\"";
		const std::size_t begin = bytes.find(firstIdentity);
		const std::size_t end = std::string::npos == begin ?
			std::string::npos : bytes.find('\n', begin);
		if (std::string::npos == begin || std::string::npos == end)
		{
			status = "Could not find the first Effect V2 binding fixture row.";
			return false;
		}
		const std::string row = bytes.substr(begin, end - begin + 1u);
		bytes.insert(end + 1u, row);
		std::ofstream output(path, std::ios::binary | std::ios::trunc);
		if (!output)
		{
			status = "Could not open the duplicated Effect V2 binding output.";
			return false;
		}
		output.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));
		output.flush();
		return output.good();
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
		std::string changedArtifactPath;
		std::uint64_t declaredArtifactBytes = 0u;
		std::string declaredArtifactSha;
		if (fixtureAdmitted && !fixtureReceipt.Artifacts.empty())
		{
			changedArtifactPath =
				fixtureReceipt.Artifacts.front().strRelativePath;
			declaredArtifactBytes = fixtureReceipt.Artifacts.front().iBytes;
			declaredArtifactSha = LostArk::Shared::Format_GameplayDataRevision(
				fixtureReceipt.Artifacts.front().Revision);
			const std::filesystem::path changedArtifact = fixture.Path /
				std::filesystem::path(changedArtifactPath);
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
			status = "stale successful admission status";
			const bool reopened = changedAdmission.Acquire_ReceiptFromRoot(
					fixture.Path, fixtureReceipt.ServerGameplayRevision,
					fixtureReceipt, status);
			require(!reopened,
				"changed physical Product bytes re-opened the old world-entry M");
			const std::string expectedPrefix =
				"Valtan presentation artifact mismatch: " + changedArtifactPath +
				"; declared bytes=" + std::to_string(declaredArtifactBytes) +
				", actual bytes=" + std::to_string(declaredArtifactBytes + 1u) +
				"; declared SHA-256=" + declaredArtifactSha +
				", actual SHA-256=";
			const bool hasExactMismatch = status.starts_with(expectedPrefix) &&
				status.size() == expectedPrefix.size() + 65u && status.back() == '.' &&
				status.substr(expectedPrefix.size(), 64u) != declaredArtifactSha;
			require(hasExactMismatch,
				"physical Product mismatch status did not replace stale success with exact path/bytes/SHA-256 evidence");
		}

		/* M now pins the BOSS_VALTAN Effect V2 entry artifact plus only its
		   reachable groups/leaves. Exercise semantic drift before the generic
		   artifact hash comparison so every failure names the broken owner. */
		const auto copyExactClosure = [&](const std::filesystem::path& target)
		{
			bool copied = Copy_FixtureFile(
				repositoryRoot, target, bootstrap, status) &&
				Copy_FixtureFile(repositoryRoot, target, manifest, status);
			for (const auto& artifact : receipt.Artifacts)
			{
				if (!copied)
					break;
				copied = Copy_FixtureFile(repositoryRoot, target,
					std::filesystem::path(artifact.strRelativePath), status);
			}
			return copied;
		};

		const std::string v2BindingRelative =
			"Data/Effects/V2/Bindings/BOSS_VALTAN.effectv2bindings.json";
		const std::string v2GroupRelative =
			"Data/Effects/V2/Groups/boss.valtan.impact.effectv2group.json";
		const std::string v2LeafRelative =
			"Data/Effects/V2/Authored/boss.valtan.hit_1.effectv2.json";
		std::set<std::string> v2Closure;
		for (const auto& artifact : receipt.Artifacts)
		{
			if (artifact.strRelativePath == v2BindingRelative ||
				artifact.strRelativePath.starts_with("Data/Effects/V2/Groups/") ||
				artifact.strRelativePath.starts_with("Data/Effects/V2/Authored/"))
			{
				v2Closure.insert(artifact.strRelativePath);
			}
		}
		require(14u == v2Closure.size() &&
			v2Closure.contains(v2BindingRelative) &&
			v2Closure.contains(v2GroupRelative) &&
			v2Closure.contains(v2LeafRelative),
			"immutable M did not pin the exact 14-artifact BOSS_VALTAN Effect V2 closure");

		struct V2_REPLACE_CASE final
		{
			const char* pRelative;
			const char* pExpected;
			const char* pReplacement;
			const char* pStatus;
			const char* pFailure;
		};
		const V2_REPLACE_CASE replaceCases[]{
			{
				"Data/Effects/V2/Bindings/BOSS_VALTAN.effectv2bindings.json",
				"\"effectId\": \"boss.valtan.hand_1\"",
				"\"effectId\": \"../boss.valtan.hand_1\"",
				"BOSS_VALTAN Effect V2 bindings[0].effectId is not a stable Effect V2 ID: ../boss.valtan.hand_1",
				"unsafe Effect V2 binding identity was not rejected with exact diagnostics"
			},
			{
				"Data/Effects/V2/Groups/boss.valtan.impact.effectv2group.json",
				"\"groupId\": \"boss.valtan.impact\"",
				"\"groupId\": \"boss.valtan.other\"",
				"BOSS_VALTAN Effect V2 group identity/children are invalid: boss.valtan.impact",
				"mismatched Effect V2 group identity was not rejected with exact diagnostics"
			},
			{
				"Data/Effects/V2/Groups/boss.valtan.impact.effectv2group.json",
				"\"effectId\": \"boss.valtan.hit_1\"",
				"\"effectId\": \"boss.valtan.impact\"",
				"BOSS_VALTAN Effect V2 group boss.valtan.impact.children[0] refers to a group instead of an authored leaf: boss.valtan.impact",
				"nested Effect V2 group reference was not rejected with exact diagnostics"
			},
			{
				"Data/Effects/V2/Authored/boss.valtan.hit_1.effectv2.json",
				"\"effectId\": \"boss.valtan.hit_1\"",
				"\"effectId\": \"boss.valtan.other\"",
				"BOSS_VALTAN Effect V2 leaf identity is invalid: boss.valtan.hit_1",
				"mismatched Effect V2 leaf identity was not rejected with exact diagnostics"
			},
		};
		for (const V2_REPLACE_CASE& drift : replaceCases)
		{
			SCOPED_TEMPORARY_ROOT driftFixture;
			driftFixture.Path = Create_TemporaryRoot(status);
			bool prepared = !driftFixture.Path.empty() &&
				copyExactClosure(driftFixture.Path) &&
				Replace_FixtureText(driftFixture.Path / drift.pRelative,
					drift.pExpected, drift.pReplacement, status);
			require(prepared, "could not prepare an Effect V2 semantic drift fixture");
			if (!prepared)
				continue;
			CValtanPresentationGenerationReadAdmission driftAdmission;
			status = "stale successful admission status";
			const bool admitted = driftAdmission.Acquire_PackagedBaselineFromRoot(
				driftFixture.Path, fixtureReceipt, status);
			require(!admitted && status == drift.pStatus, drift.pFailure);
		}

		struct V2_MISSING_CASE final
		{
			const char* pRelative;
			const char* pStatus;
			const char* pFailure;
		};
		const V2_MISSING_CASE missingCases[]{
			{
				"Data/Effects/V2/Bindings/BOSS_VALTAN.effectv2bindings.json",
				"BOSS_VALTAN Effect V2 bindings is missing or is not a regular file: Data/Effects/V2/Bindings/BOSS_VALTAN.effectv2bindings.json",
				"missing BOSS_VALTAN Effect V2 binding artifact was not rejected exactly"
			},
			{
				"Data/Effects/V2/Groups/boss.valtan.impact.effectv2group.json",
				"BOSS_VALTAN Effect V2 group boss.valtan.impact is missing or is not a regular file: Data/Effects/V2/Groups/boss.valtan.impact.effectv2group.json",
				"missing referenced Effect V2 group was not rejected exactly"
			},
			{
				"Data/Effects/V2/Authored/boss.valtan.hit_1.effectv2.json",
				"BOSS_VALTAN Effect V2 leaf boss.valtan.hit_1 is missing or is not a regular file: Data/Effects/V2/Authored/boss.valtan.hit_1.effectv2.json",
				"missing referenced Effect V2 leaf was not rejected exactly"
			},
		};
		for (const V2_MISSING_CASE& missing : missingCases)
		{
			SCOPED_TEMPORARY_ROOT missingFixture;
			missingFixture.Path = Create_TemporaryRoot(status);
			bool prepared = !missingFixture.Path.empty() &&
				copyExactClosure(missingFixture.Path);
			std::error_code removeError;
			prepared = prepared && std::filesystem::remove(
				missingFixture.Path / missing.pRelative, removeError) && !removeError;
			require(prepared, "could not prepare a missing Effect V2 artifact fixture");
			if (!prepared)
				continue;
			CValtanPresentationGenerationReadAdmission missingAdmission;
			status = "stale successful admission status";
			const bool admitted = missingAdmission.Acquire_PackagedBaselineFromRoot(
				missingFixture.Path, fixtureReceipt, status);
			require(!admitted && status == missing.pStatus, missing.pFailure);
		}

		SCOPED_TEMPORARY_ROOT duplicateFixture;
		duplicateFixture.Path = Create_TemporaryRoot(status);
		bool duplicatePrepared = !duplicateFixture.Path.empty() &&
			copyExactClosure(duplicateFixture.Path) &&
			Duplicate_FirstBindingRow(
				duplicateFixture.Path / v2BindingRelative, status);
		require(duplicatePrepared,
			"could not prepare a duplicate Effect V2 binding fixture");
		if (duplicatePrepared)
		{
			CValtanPresentationGenerationReadAdmission duplicateAdmission;
			status = "stale successful admission status";
			const bool admitted = duplicateAdmission.Acquire_PackagedBaselineFromRoot(
				duplicateFixture.Path, fixtureReceipt, status);
			require(!admitted && status ==
				"BOSS_VALTAN Effect V2 bindings[1] duplicates an earlier binding identity: boss.valtan.hand_1.",
				"duplicate Effect V2 binding identity was not rejected with exact diagnostics");
		}
	}

	std::cout << "ValtanPresentationGenerationAdmissionContractTests: " <<
		(0 == failures ? "PASS" : "FAIL") <<
		" (Pattern Sound remains an independent typed receipt)\n";
	return failures;
}
