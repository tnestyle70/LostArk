#include "ProjectDataRoot.h"
#include "DataJson.h"
#include "EffectV2_Document.h"
#include "ValtanPatternTree.h"
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
		std::string& outBindingId,
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
		std::vector<Client::EFFECT_V2_BINDING> bindings;
		std::string parseError;
		if (!Client::CEffectV2Document::Parse_Bindings(
				bytes, "BOSS_VALTAN", bindings, parseError) || bindings.empty())
		{
			status = "Could not parse the strict v2 binding fixture: " +
				parseError;
			return false;
		}
		outBindingId = bindings.front().strBindingId;
		bindings.push_back(bindings.front());
		bytes = Client::CEffectV2Document::Serialize_Bindings(
			"BOSS_VALTAN", bindings);
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

	bool Load_JsonFixture(
		const std::filesystem::path& path,
		Client::DATA_JSON_VALUE& outRoot,
		std::string& status)
	{
		std::ifstream input(path, std::ios::binary);
		if (!input)
		{
			status = "Could not open the Effect V2 closure fixture: " +
				path.string();
			return false;
		}
		const std::string bytes{
			std::istreambuf_iterator<char>(input),
			std::istreambuf_iterator<char>() };
		if (!Client::CDataJson::Parse(bytes, outRoot, status))
		{
			status = "Could not parse the Effect V2 closure fixture " +
				path.string() + ": " + status;
			return false;
		}
		return true;
	}

	bool Read_V2Resource(
		const Client::DATA_JSON_VALUE& owner,
		const std::string& context,
		std::string& outKind,
		std::string& outId,
		std::string& status)
	{
		const Client::DATA_JSON_VALUE* const resource = owner.Find("resource");
		if (nullptr == resource || !resource->Is_Object() ||
			2u != resource->Get_Object().size())
		{
			status = context +
				" has no exact resource { kind, id } object.";
			return false;
		}
		const Client::DATA_JSON_VALUE* const kind = resource->Find("kind");
		const Client::DATA_JSON_VALUE* const id = resource->Find("id");
		if (nullptr == kind || !kind->Is_String() ||
			("LEAF" != kind->Get_String() && "GROUP" != kind->Get_String()) ||
			nullptr == id || !id->Is_String() || id->Get_String().empty())
		{
			status = context + " has an invalid v2 resource identity.";
			return false;
		}
		outKind = kind->Get_String();
		outId = id->Get_String();
		return true;
	}

	bool Collect_ReachableV2Closure(
		const std::filesystem::path& repositoryRoot,
		std::set<std::string>& outClosure,
		std::string& status)
	{
		constexpr const char* bindingRelative =
			"Data/Effects/V2/Bindings/BOSS_VALTAN.effectv2bindings.json";
		Client::DATA_JSON_VALUE bindingRoot;
		if (!Load_JsonFixture(
				repositoryRoot / bindingRelative, bindingRoot, status))
		{
			return false;
		}
		const Client::DATA_JSON_VALUE* const bindings =
			bindingRoot.Find("bindings");
		if (nullptr == bindings || !bindings->Is_Array())
		{
			status = "Effect V2 binding closure fixture has no bindings array.";
			return false;
		}

		outClosure.clear();
		outClosure.insert(bindingRelative);
		std::set<std::string> groupIds;
		std::set<std::string> leafIds;
		for (const Client::DATA_JSON_VALUE& binding : bindings->Get_Array())
		{
			if (!binding.Is_Object() || 6u != binding.Get_Object().size() ||
				nullptr == binding.Find("bindingId") ||
				nullptr == binding.Find("scope") ||
				nullptr == binding.Find("clock") ||
				nullptr == binding.Find("anchor") ||
				nullptr == binding.Find("stopPolicy"))
			{
				status = "Effect V2 binding closure fixture contains a non-v2 row.";
				return false;
			}
			std::string kind;
			std::string id;
			if (!Read_V2Resource(binding,
					"Effect V2 binding closure fixture row", kind, id, status))
				return false;
			(kind == "GROUP" ? groupIds : leafIds).insert(id);
		}

		/* Combat-object visuals are a second typed BOSS_VALTAN owner lane.  They
		   are not duplicated into animation bindings, but their groups are part
		   of the same entry-required presentation closure. */
		Client::DATA_JSON_VALUE bossCatalog;
		if (!Load_JsonFixture(repositoryRoot / "Data/Actors/BossCatalog.json",
				bossCatalog, status))
		{
			return false;
		}
		const Client::DATA_JSON_VALUE* const bosses = bossCatalog.Find("bosses");
		if (nullptr == bosses || !bosses->Is_Array())
		{
			status = "Effect V2 owner closure fixture has no bosses array.";
			return false;
		}
		std::size_t valtanOwnerCount = 0u;
		for (const Client::DATA_JSON_VALUE& boss : bosses->Get_Array())
		{
			const Client::DATA_JSON_VALUE* const archetypeId =
				boss.Is_Object() ? boss.Find("archetypeId") : nullptr;
			if (nullptr == archetypeId || !archetypeId->Is_String() ||
				"BOSS_VALTAN" != archetypeId->Get_String())
			{
				continue;
			}
			++valtanOwnerCount;
			const Client::DATA_JSON_VALUE* const visuals =
				boss.Find("combatObjectVisuals");
			if (nullptr == visuals || !visuals->Is_Array())
			{
				status =
					"Effect V2 owner closure fixture has invalid combatObjectVisuals.";
				return false;
			}
			for (const Client::DATA_JSON_VALUE& visual : visuals->Get_Array())
			{
				const Client::DATA_JSON_VALUE* const owner =
					visual.Is_Object() ? visual.Find("effectV2Group") : nullptr;
				if (nullptr == owner)
					continue;
				if (!owner->Is_Object())
				{
					status =
						"Effect V2 owner closure fixture has an invalid group object.";
					return false;
				}
				const Client::DATA_JSON_VALUE* const groupId = owner->Find("groupId");
				if (nullptr == groupId ||
					!groupId->Is_String() || groupId->Get_String().empty())
				{
					status =
						"Effect V2 owner closure fixture has an invalid group identity.";
					return false;
				}
				groupIds.insert(groupId->Get_String());
			}
		}
		if (1u != valtanOwnerCount)
		{
			status = "Effect V2 owner closure fixture has no unique BOSS_VALTAN.";
			return false;
		}

		for (const std::string& groupId : groupIds)
		{
			const std::string groupRelative =
				"Data/Effects/V2/Groups/" + groupId + ".effectv2group.json";
			outClosure.insert(groupRelative);
			Client::DATA_JSON_VALUE groupRoot;
			if (!Load_JsonFixture(
					repositoryRoot / groupRelative, groupRoot, status))
			{
				return false;
			}
			const Client::DATA_JSON_VALUE* const children =
				groupRoot.Find("children");
			if (nullptr == children || !children->Is_Array())
			{
				status = "Effect V2 group closure fixture has no children array: " +
					groupId;
				return false;
			}
			for (const Client::DATA_JSON_VALUE& child : children->Get_Array())
			{
				if (!child.Is_Object() || 6u != child.Get_Object().size() ||
					nullptr == child.Find("childId") ||
					nullptr == child.Find("startMs") ||
					nullptr == child.Find("durationMs") ||
					nullptr == child.Find("stop") ||
					nullptr == child.Find("localTransform"))
				{
					status = "Effect V2 group closure fixture has a non-v2 child: " +
						groupId;
					return false;
				}
				std::string kind;
				std::string id;
				if (!Read_V2Resource(child,
						"Effect V2 group closure fixture child " + groupId,
						kind, id, status))
				{
					return false;
				}
				if ("GROUP" == kind)
				{
					status = "Effect V2 group closure fixture contains a nested group: " +
						groupId + "/" + id;
					return false;
				}
				leafIds.insert(id);
			}
		}

		for (const std::string& leafId : leafIds)
		{
			outClosure.insert(
				"Data/Effects/V2/Authored/" + leafId + ".effectv2.json");
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
		Client::VALTAN_CANONICAL_READ_DIAGNOSTIC diagnostic;
		const bool acquired =
			admission.Acquire_PackagedBaseline(receipt, diagnostic);
		status = diagnostic.strStatus;
		if (!acquired)
			std::cerr << "  admission detail: " << status << '\n';
		require(acquired,
			"current typed presentation sources were not admitted");
		require(Client::VALTAN_CANONICAL_READ_FAILURE_KIND::NONE ==
				diagnostic.eFailure,
			"successful presentation admission returned a failure kind");
		require(receipt.Is_Valid(), "successful admission returned no source receipt");
		require(admission.Validate_StillCurrent(diagnostic),
			"typed presentation sources changed before commit");
		status = diagnostic.strStatus;
	}

	if (receipt.Is_Valid())
	{
		{
			CValtanPresentationGenerationReadAdmission admission;
			require(admission.Acquire_Receipt(
				receipt.ServerGameplayRevision, receipt, status),
				"current typed sources did not re-open for the Server revision");
			require(admission.Validate_StillCurrent(status),
				"re-opened typed sources changed before commit");
		}
		{
			CValtanPresentationGenerationReadAdmission exactAdmission;
			require(exactAdmission.Acquire_ExactReceipt(
				receipt.ServerGameplayRevision, receipt, status),
				"the exact prepared presentation receipt was rejected");
			require(exactAdmission.Validate_StillCurrent(status),
				"the exact prepared presentation receipt changed before commit");
		}

		LostArk::Shared::GameplayDataRevision invalidRevision{};
		CValtanPresentationGenerationReadAdmission invalidRevisionAdmission;
		require(!invalidRevisionAdmission.Acquire_Receipt(
			invalidRevision, receipt, status),
			"an invalid Server gameplay revision was admitted");

		auto currentServerRevision = receipt.ServerGameplayRevision;
		currentServerRevision.Bytes[0] ^= 0xffu;
		{
			CValtanPresentationGenerationReadAdmission currentServerAdmission;
			require(currentServerAdmission.Acquire_Receipt(
				currentServerRevision, receipt, status),
				"a valid Server gameplay revision did not pin the current typed closure");
			VALTAN_PRESENTATION_GENERATION_RECEIPT currentServerReceipt;
			require(currentServerAdmission.Try_Get_CurrentReceipt(currentServerReceipt) &&
				currentServerReceipt.ServerGameplayRevision == currentServerRevision,
				"the current typed closure did not retain the caller's Server revision");
		}

		auto staleWorldEntryReceipt = receipt;
		staleWorldEntryReceipt.PresentationGenerationId.Bytes[0] ^= 0xffu;
		staleWorldEntryReceipt.Artifacts.pop_back();
		{
			CValtanPresentationGenerationReadAdmission staleReceiptAdmission;
			require(staleReceiptAdmission.Acquire_Receipt(
				receipt.ServerGameplayRevision, staleWorldEntryReceipt, status),
				"stale world-entry generation/inventory blocked the current typed closure");
			VALTAN_PRESENTATION_GENERATION_RECEIPT refreshedReceipt;
			require(staleReceiptAdmission.Try_Get_CurrentReceipt(refreshedReceipt) &&
				refreshedReceipt == receipt,
				"admission retained stale world-entry presentation identity");
		}
		{
			CValtanPresentationGenerationReadAdmission staleExactAdmission;
			require(!staleExactAdmission.Acquire_ExactReceipt(
				receipt.ServerGameplayRevision, staleWorldEntryReceipt, status),
				"a stale receipt was admitted as an exact prepared generation");
		}

		/* Simulate ordinary local authoring after world entry. Re-opening must stage
		   the current typed sources rather than comparing them with the historical
		   receipt, while a write during the held transaction must still fail its
		   final currentness check. */
		SCOPED_TEMPORARY_ROOT fixture;
		fixture.Path = Create_TemporaryRoot(status);
		require(!fixture.Path.empty(),
			"could not create a physical-generation mismatch fixture");
		bool fixtureCopied = !fixture.Path.empty();
		const std::filesystem::path repositoryRoot =
			Client::CProjectDataRoot::Get().parent_path();
		const std::filesystem::path bootstrap =
			"Server/Bin/DataFiles/Gameplay/Gameplay.bootstrap";
		if (fixtureCopied)
			fixtureCopied = Copy_FixtureFile(
				repositoryRoot, fixture.Path, bootstrap, status);
		for (const auto& artifact : receipt.Artifacts)
		{
			if (!fixtureCopied)
				break;
			fixtureCopied = Copy_FixtureFile(
				repositoryRoot, fixture.Path,
				std::filesystem::path(artifact.strRelativePath), status);
		}
		require(fixtureCopied,
			"could not copy the current typed presentation closure");

		VALTAN_PRESENTATION_GENERATION_RECEIPT fixtureReceipt;
		bool fixtureAdmitted = false;
		if (fixtureCopied)
		{
			CValtanPresentationGenerationReadAdmission fixtureAdmission;
			fixtureAdmitted = fixtureAdmission.Acquire_PackagedBaselineFromRoot(
				fixture.Path, fixtureReceipt, status);
			require(fixtureAdmitted,
				"the copied typed presentation closure was rejected");
			require(!fixtureAdmitted || fixtureReceipt == receipt,
				"the copied typed closure changed its current receipt");
		}

		bool artifactChanged = false;
		std::string changedArtifactPath;
		if (fixtureAdmitted && !fixtureReceipt.Artifacts.empty())
		{
			changedArtifactPath =
				fixtureReceipt.Artifacts.front().strRelativePath;
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
			require(reopened,
				"changed local presentation bytes were compared with the world-entry receipt");
			VALTAN_PRESENTATION_GENERATION_RECEIPT changedReceipt;
			require(reopened && changedAdmission.Try_Get_CurrentReceipt(changedReceipt) &&
				changedReceipt != fixtureReceipt,
				"re-open did not pin the newly validated physical closure");
			require(reopened && changedAdmission.Validate_StillCurrent(status),
				"unchanged staged presentation sources failed currentness validation");

			CValtanPresentationGenerationReadAdmission changedExactAdmission;
			require(!changedExactAdmission.Acquire_ExactReceiptFromRoot(
					fixture.Path, fixtureReceipt.ServerGameplayRevision,
					fixtureReceipt, status),
				"changed physical bytes were admitted as an exact prepared generation");

			const std::filesystem::path changedArtifact = fixture.Path /
				std::filesystem::path(changedArtifactPath);
			std::ofstream stream(
				changedArtifact, std::ios::binary | std::ios::app);
			bool changedDuringAdmission = false;
			if (stream)
			{
				stream.put('\n');
				stream.flush();
				changedDuringAdmission = stream.good();
			}
			require(changedDuringAdmission,
				"could not mutate a presentation source during held admission");
			require(!changedDuringAdmission ||
				!changedAdmission.Validate_StillCurrent(status),
				"concurrent presentation write passed the transactional currentness check");
		}

		/* The typed closure includes the BOSS_VALTAN Effect V2 entry artifact plus
		   only its reachable groups/leaves. Semantic drift must still fail at the
		   owning parser even though stale world-entry receipts are not gates. */
		const auto copyExactClosure = [&](const std::filesystem::path& target)
		{
			bool copied = Copy_FixtureFile(
				repositoryRoot, target, bootstrap, status);
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
		std::set<std::string> expectedV2Closure;
		const bool collectedV2Closure = Collect_ReachableV2Closure(
			repositoryRoot, expectedV2Closure, status);
		require(collectedV2Closure,
			"could not derive the source-reachable BOSS_VALTAN Effect V2 closure");
		require(collectedV2Closure && v2Closure == expectedV2Closure &&
			v2Closure.contains(v2BindingRelative) &&
			v2Closure.contains(v2GroupRelative) &&
			v2Closure.contains(v2LeafRelative),
			"typed admission did not resolve the exact source-reachable BOSS_VALTAN Effect V2 closure");

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
				"\"id\": \"boss.valtan.hand_1\"",
				"\"id\": \"../boss.valtan.hand_1\"",
				"BOSS_VALTAN Effect V2 bindings failed strict v2 admission: id must be a stable ID.",
				"unsafe Effect V2 binding identity was not rejected with exact diagnostics"
			},
			{
				"Data/Effects/V2/Groups/boss.valtan.impact.effectv2group.json",
				"\"groupId\": \"boss.valtan.impact\"",
				"\"groupId\": \"boss.valtan.other\"",
				"BOSS_VALTAN Effect V2 group failed strict v2 admission: boss.valtan.impact: ",
				"mismatched Effect V2 group identity was not rejected with exact diagnostics"
			},
			{
				"Data/Effects/V2/Groups/boss.valtan.impact.effectv2group.json",
				"\"id\": \"boss.valtan.hit_1\"",
				"\"id\": \"boss.valtan.impact\"",
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
		std::string duplicatedBindingId;
		bool duplicatePrepared = !duplicateFixture.Path.empty() &&
			copyExactClosure(duplicateFixture.Path) &&
			Duplicate_FirstBindingRow(
				duplicateFixture.Path / v2BindingRelative,
				duplicatedBindingId, status);
		require(duplicatePrepared,
			"could not prepare a duplicate Effect V2 binding fixture");
		if (duplicatePrepared)
		{
			CValtanPresentationGenerationReadAdmission duplicateAdmission;
			status = "stale successful admission status";
			const bool admitted = duplicateAdmission.Acquire_PackagedBaselineFromRoot(
				duplicateFixture.Path, fixtureReceipt, status);
			const std::string expectedDuplicateStatus =
				"BOSS_VALTAN Effect V2 bindings failed strict v2 admission: "
				"duplicate Effect V2 bindingId: " + duplicatedBindingId;
			require(!admitted && status == expectedDuplicateStatus,
				"duplicate Effect V2 binding identity was not rejected with exact diagnostics");
		}
	}

	std::cout << "ValtanPresentationGenerationAdmissionContractTests: " <<
		(0 == failures ? "PASS" : "FAIL") <<
		" (Pattern Sound remains an independent typed receipt)\n";
	return failures;
}
