#include "NetworkManager.h"

#include "DataJson.h"
#include "ProjectDataRoot.h"

#include "Network/PacketReader.h"
#include "Network/PacketWriter.h"

#include <fstream>
#include <filesystem>
#include <iterator>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <bcrypt.h>

#pragma comment(lib, "bcrypt.lib")

#include <algorithm>
#include <array>
#include <cmath>
#include <cwctype>
#include <limits>
#include <utility>

namespace
{
#ifdef _DEBUG
	std::string Resolve_DebugLocalServerHost()
	{
		constexpr char OVERRIDE_PATH[] = "LocalServerEndpoint.user.json";
		std::ifstream input(OVERRIDE_PATH, std::ios::binary);
		if (!input)
			return {};

		const std::string text(
			(std::istreambuf_iterator<char>(input)),
			std::istreambuf_iterator<char>());
		if (text.empty() || text.size() > 1024u)
			return {};

		Client::DATA_JSON_VALUE root;
		std::string error;
		if (!Client::CDataJson::Parse(text, root, error) ||
			!root.Is_Object() || 4u != root.Get_Object().size())
		{
			return {};
		}

		const Client::DATA_JSON_VALUE* schema = root.Find("schema");
		const Client::DATA_JSON_VALUE* formatVersion =
			root.Find("formatVersion");
		const Client::DATA_JSON_VALUE* enabled = root.Find("enabled");
		const Client::DATA_JSON_VALUE* host = root.Find("host");
		if (nullptr == schema || !schema->Is_String() ||
			"lostark.local-server-endpoint" != schema->Get_String() ||
			nullptr == formatVersion || !formatVersion->Is_Number() ||
			formatVersion->Was_FloatingPointToken() ||
			1.0 != formatVersion->Get_Number() ||
			nullptr == enabled || !enabled->Is_Boolean() ||
			!enabled->Get_Boolean() ||
			nullptr == host || !host->Is_String() ||
			"127.0.0.1" != host->Get_String())
		{
			return {};
		}

		return host->Get_String();
	}

	constexpr std::uint64_t MAX_PRESENTATION_ALIAS_ARTIFACT_BYTES =
		64ull * 1024ull * 1024ull;

	bool IsLowerSha256(const std::string& value)
	{
		return value.size() ==
			LostArk::Shared::GAMEPLAY_DATA_REVISION_HEX_BYTES &&
			std::all_of(value.begin(), value.end(), [](const char character)
				{
					return ('0' <= character && character <= '9') ||
						('a' <= character && character <= 'f');
				});
	}

	bool IsDescendantPath(
		const std::filesystem::path& root,
		const std::filesystem::path& candidate)
	{
		std::error_code error;
		const std::filesystem::path canonicalRoot =
			std::filesystem::weakly_canonical(root, error);
		if (error || canonicalRoot.empty())
			return false;
		error.clear();
		const std::filesystem::path canonicalCandidate =
			std::filesystem::weakly_canonical(candidate, error);
		if (error || canonicalCandidate.empty())
			return false;

		auto rootPart = canonicalRoot.begin();
		auto candidatePart = canonicalCandidate.begin();
		for (; rootPart != canonicalRoot.end(); ++rootPart, ++candidatePart)
		{
			if (candidatePart == canonicalCandidate.end())
				return false;
			std::wstring left = rootPart->native();
			std::wstring right = candidatePart->native();
			std::transform(left.begin(), left.end(), left.begin(), ::towlower);
			std::transform(right.begin(), right.end(), right.begin(), ::towlower);
			if (left != right)
				return false;
		}
		return candidatePart != canonicalCandidate.end();
	}

	bool ReadBoundedText(
		const std::filesystem::path& path,
		const std::uint64_t maximumBytes,
		std::string& text,
		std::string& status)
	{
		std::error_code error;
		const std::uint64_t bytes = std::filesystem::file_size(path, error);
		if (error || 0u == bytes || bytes > maximumBytes ||
			bytes > static_cast<std::uint64_t>(
				(std::numeric_limits<std::streamsize>::max)()))
		{
			status = "Presentation alias document size is invalid: " +
				path.string();
			return false;
		}
		std::ifstream input(path, std::ios::binary);
		if (!input)
		{
			status = "Presentation alias document is missing: " + path.string();
			return false;
		}
		std::string staged(static_cast<std::size_t>(bytes), '\0');
		input.read(staged.data(), static_cast<std::streamsize>(staged.size()));
		if (!input || input.gcount() != static_cast<std::streamsize>(staged.size()))
		{
			status = "Presentation alias document read was incomplete: " +
				path.string();
			return false;
		}
		text = std::move(staged);
		return true;
	}

	bool HashFileSha256(
		const std::filesystem::path& path,
		std::string& sha256,
		std::uint64_t& byteCount,
		std::string& status)
	{
		std::error_code error;
		const std::uint64_t fileBytes = std::filesystem::file_size(path, error);
		if (error || fileBytes > MAX_PRESENTATION_ALIAS_ARTIFACT_BYTES)
		{
			status = "Presentation alias artifact size is invalid: " + path.string();
			return false;
		}
		std::ifstream input(path, std::ios::binary);
		if (!input)
		{
			status = "Presentation alias artifact is missing: " + path.string();
			return false;
		}

		BCRYPT_ALG_HANDLE algorithm = nullptr;
		BCRYPT_HASH_HANDLE hash = nullptr;
		DWORD objectBytes = 0u;
		DWORD hashBytes = 0u;
		DWORD written = 0u;
		std::vector<unsigned char> hashObject;
		LostArk::Shared::GameplayDataRevision digest{};
		bool succeeded = false;
		if (0 <= BCryptOpenAlgorithmProvider(
				&algorithm, BCRYPT_SHA256_ALGORITHM, nullptr, 0u) &&
			0 <= BCryptGetProperty(
				algorithm, BCRYPT_OBJECT_LENGTH,
				reinterpret_cast<PUCHAR>(&objectBytes), sizeof(objectBytes),
				&written, 0u) &&
			0 <= BCryptGetProperty(
				algorithm, BCRYPT_HASH_LENGTH,
				reinterpret_cast<PUCHAR>(&hashBytes), sizeof(hashBytes),
				&written, 0u) &&
			hashBytes == digest.Bytes.size())
		{
			hashObject.resize(objectBytes);
			if (0 <= BCryptCreateHash(
					algorithm, &hash, hashObject.data(), objectBytes,
					nullptr, 0u, 0u))
			{
				std::array<char, 64u * 1024u> buffer{};
				std::uint64_t consumed = 0u;
				while (input)
				{
					input.read(buffer.data(),
						static_cast<std::streamsize>(buffer.size()));
					const std::streamsize count = input.gcount();
					if (count > 0 && 0 > BCryptHashData(
						hash, reinterpret_cast<PUCHAR>(buffer.data()),
						static_cast<ULONG>(count), 0u))
					{
						break;
					}
					consumed += static_cast<std::uint64_t>(count);
				}
				if (input.eof() && consumed == fileBytes &&
					0 <= BCryptFinishHash(
						hash, digest.Bytes.data(), hashBytes, 0u) &&
					digest.Is_Valid())
				{
					succeeded = true;
					byteCount = consumed;
					sha256 = LostArk::Shared::Format_GameplayDataRevision(digest);
				}
			}
		}
		if (nullptr != hash)
			BCryptDestroyHash(hash);
		if (nullptr != algorithm)
			BCryptCloseAlgorithmProvider(algorithm, 0u);
		if (!succeeded)
			status = "Could not hash presentation alias artifact: " + path.string();
		return succeeded;
	}

	bool HashBytesSha256(
		const std::string_view bytes,
		std::string& sha256)
	{
		if (bytes.empty() ||
			bytes.size() > static_cast<std::size_t>(
				(std::numeric_limits<ULONG>::max)()))
		{
			return false;
		}

		BCRYPT_ALG_HANDLE algorithm = nullptr;
		BCRYPT_HASH_HANDLE hash = nullptr;
		DWORD objectBytes = 0u;
		DWORD hashBytes = 0u;
		DWORD written = 0u;
		std::vector<unsigned char> hashObject;
		LostArk::Shared::GameplayDataRevision digest{};
		bool succeeded = false;
		if (0 <= BCryptOpenAlgorithmProvider(
				&algorithm, BCRYPT_SHA256_ALGORITHM, nullptr, 0u) &&
			0 <= BCryptGetProperty(
				algorithm, BCRYPT_OBJECT_LENGTH,
				reinterpret_cast<PUCHAR>(&objectBytes), sizeof(objectBytes),
				&written, 0u) &&
			0 <= BCryptGetProperty(
				algorithm, BCRYPT_HASH_LENGTH,
				reinterpret_cast<PUCHAR>(&hashBytes), sizeof(hashBytes),
				&written, 0u) &&
			hashBytes == digest.Bytes.size())
		{
			hashObject.resize(objectBytes);
			if (0 <= BCryptCreateHash(
					algorithm, &hash, hashObject.data(), objectBytes,
					nullptr, 0u, 0u) &&
				0 <= BCryptHashData(
					hash,
					reinterpret_cast<PUCHAR>(
						const_cast<char*>(bytes.data())),
					static_cast<ULONG>(bytes.size()), 0u) &&
				0 <= BCryptFinishHash(
					hash, digest.Bytes.data(), hashBytes, 0u) &&
				digest.Is_Valid())
			{
				sha256 = LostArk::Shared::Format_GameplayDataRevision(digest);
				succeeded = true;
			}
		}
		if (nullptr != hash)
			BCryptDestroyHash(hash);
		if (nullptr != algorithm)
			BCryptCloseAlgorithmProvider(algorithm, 0u);
		return succeeded;
	}

	bool JsonValuesEqual(
		const Client::DATA_JSON_VALUE& left,
		const Client::DATA_JSON_VALUE& right)
	{
		using Client::DATA_JSON_TYPE;
		if (left.Get_Type() != right.Get_Type())
			return false;
		switch (left.Get_Type())
		{
		case DATA_JSON_TYPE::NULL_VALUE:
			return true;
		case DATA_JSON_TYPE::BOOLEAN:
			return left.Get_Boolean() == right.Get_Boolean();
		case DATA_JSON_TYPE::NUMBER:
			return left.Get_Number() == right.Get_Number() &&
				left.Was_FloatingPointToken() ==
					right.Was_FloatingPointToken();
		case DATA_JSON_TYPE::STRING:
			return left.Get_String() == right.Get_String();
		case DATA_JSON_TYPE::ARRAY:
		{
			const auto& leftArray = left.Get_Array();
			const auto& rightArray = right.Get_Array();
			if (leftArray.size() != rightArray.size())
				return false;
			for (std::size_t index = 0u; index < leftArray.size(); ++index)
				if (!JsonValuesEqual(leftArray[index], rightArray[index]))
					return false;
			return true;
		}
		case DATA_JSON_TYPE::OBJECT:
		{
			const auto& leftObject = left.Get_Object();
			const auto& rightObject = right.Get_Object();
			if (leftObject.size() != rightObject.size())
				return false;
			for (const auto& [key, value] : leftObject)
			{
				const auto found = rightObject.find(key);
				if (rightObject.end() == found ||
					!JsonValuesEqual(value, found->second))
				{
					return false;
				}
			}
			return true;
		}
		default:
			return false;
		}
	}

	bool ManifestMatchesRevisionIdentity(
		const Client::DATA_JSON_VALUE& manifest,
		const Client::DATA_JSON_VALUE& identity,
		const std::string& revisionHex)
	{
		if (!manifest.Is_Object() || !identity.Is_Object() ||
			manifest.Get_Object().size() != identity.Get_Object().size())
		{
			return false;
		}
		for (const auto& [key, value] : manifest.Get_Object())
		{
			const auto found = identity.Get_Object().find(key);
			if (identity.Get_Object().end() == found)
				return false;
			if ("revisionId" == key)
			{
				if (!value.Is_String() || value.Get_String() != revisionHex ||
					!found->second.Is_String() ||
					!found->second.Get_String().empty())
				{
					return false;
				}
				continue;
			}
			if (!JsonValuesEqual(value, found->second))
				return false;
		}
		return true;
	}

	bool ReadExactUnsigned(
		const Client::DATA_JSON_VALUE& object,
		const char* field,
		std::uint64_t& output)
	{
		const Client::DATA_JSON_VALUE* value = object.Find(field);
		if (nullptr == value || !value->Is_Number() ||
			value->Was_FloatingPointToken() ||
			!std::isfinite(value->Get_Number()) || value->Get_Number() < 0.0 ||
			std::floor(value->Get_Number()) != value->Get_Number() ||
			value->Get_Number() > static_cast<double>(
				(std::numeric_limits<std::uint64_t>::max)()))
		{
			return false;
		}
		output = static_cast<std::uint64_t>(value->Get_Number());
		return true;
	}

	std::uint32_t PresentationLaneBit(const std::string& lane)
	{
		using LostArk::Shared::GAMEPLAY_PRESENTATION_LANE;
		if ("ANIMATION" == lane)
			return static_cast<std::uint32_t>(
				GAMEPLAY_PRESENTATION_LANE::ANIMATION);
		if ("EFFECT" == lane)
			return static_cast<std::uint32_t>(GAMEPLAY_PRESENTATION_LANE::EFFECT);
		if ("COMBAT_VISUAL" == lane)
			return static_cast<std::uint32_t>(
				GAMEPLAY_PRESENTATION_LANE::COMBAT_VISUAL);
		if ("CAMERA" == lane)
			return static_cast<std::uint32_t>(GAMEPLAY_PRESENTATION_LANE::CAMERA);
		if ("WORLD_EVENT_SET" == lane)
			return static_cast<std::uint32_t>(
				GAMEPLAY_PRESENTATION_LANE::WORLD_EVENT_SET);
		return 0u;
	}

	bool CapturePresentationArtifactBaseline(
		std::vector<CNetworkManager::PRESENTATION_ARTIFACT_BASELINE>& output,
		Client::VALTAN_PRESENTATION_GENERATION_RECEIPT& receipt,
		std::string& status)
	{
		Client::CValtanPresentationGenerationReadAdmission admission;
		Client::VALTAN_PRESENTATION_GENERATION_RECEIPT stagedReceipt;
		if (!admission.Acquire_PackagedBaseline(stagedReceipt, status) ||
			!admission.Validate_StillCurrent(status))
		{
			return false;
		}
		std::vector<CNetworkManager::PRESENTATION_ARTIFACT_BASELINE> staged;
		staged.reserve(stagedReceipt.Artifacts.size());
		for (const auto& artifact : stagedReceipt.Artifacts)
		{
			CNetworkManager::PRESENTATION_ARTIFACT_BASELINE row;
			row.strRelativePath = artifact.strRelativePath;
			row.strLane = artifact.strLane;
			row.strSha256 = LostArk::Shared::Format_GameplayDataRevision(
				artifact.Revision);
			row.iBytes = artifact.iBytes;
			staged.push_back(std::move(row));
		}
		output = std::move(staged);
		receipt = std::move(stagedReceipt);
		status = "Captured the current validated Client presentation sources.";
		return true;
	}

	bool ValidateCurrentCandidatePresentationGeneration(
		const LostArk::Shared::GameplayDataRevision& revision,
		const std::uint32_t requestedLaneMask,
		const LostArk::Shared::GameplayDataRevision& presentationGenerationId,
		const std::vector<CNetworkManager::PRESENTATION_ARTIFACT_BASELINE>&
			baselineArtifacts,
		std::string& status)
	{
		using Client::DATA_JSON_VALUE;
		using namespace LostArk::Shared;
		if (!revision.Is_Valid() || 0u == requestedLaneMask ||
			0u != (requestedLaneMask & ~GAMEPLAY_PRESENTATION_KNOWN_LANE_MASK))
		{
			status = "Saved presentation generation request lane mask is invalid.";
			return false;
		}
		const std::string revisionHex = Format_GameplayDataRevision(revision);
		const std::filesystem::path repositoryRoot =
			Client::CProjectDataRoot::Get().parent_path();
		const std::filesystem::path candidateRoot =
			repositoryRoot / L"Intermediate" / L"ValtanTuningCandidates";
		const std::filesystem::path revisionRoot =
			candidateRoot / L"revisions" /
			std::filesystem::path(revisionHex);
		const std::filesystem::path manifestPath =
			revisionRoot / L"revision-manifest.json";
		const std::filesystem::path identityPath =
			revisionRoot / L"revision-identity.json";
		if (!IsDescendantPath(candidateRoot, revisionRoot) ||
			!IsDescendantPath(revisionRoot, manifestPath) ||
			!IsDescendantPath(revisionRoot, identityPath))
		{
			status = "Candidate revision path escaped its immutable root.";
			return false;
		}

		std::string manifestText;
		std::string identityText;
		if (!ReadBoundedText(
				manifestPath, 2ull * 1024ull * 1024ull, manifestText, status) ||
			!ReadBoundedText(
				identityPath, 2ull * 1024ull * 1024ull, identityText, status))
		{
			return false;
		}
		std::string identitySha256;
		if (!HashBytesSha256(identityText, identitySha256) ||
			identitySha256 != revisionHex)
		{
			status = "Candidate revision identity bytes do not match the announced revision.";
			return false;
		}
		DATA_JSON_VALUE manifest;
		DATA_JSON_VALUE identity;
		std::string parseError;
		if (!Client::CDataJson::Parse(manifestText, manifest, parseError) ||
			!manifest.Is_Object() ||
			!Client::CDataJson::Parse(identityText, identity, parseError) ||
			!identity.Is_Object())
		{
			status = "Candidate revision manifest/identity parse failed: " + parseError;
			return false;
		}
		if (!ManifestMatchesRevisionIdentity(manifest, identity, revisionHex))
		{
			status = "Candidate manifest differs from its hashed parent identity.";
			return false;
		}
		const DATA_JSON_VALUE* schema = manifest.Find("schema");
		const DATA_JSON_VALUE* formatVersion = manifest.Find("formatVersion");
		const DATA_JSON_VALUE* revisionId = manifest.Find("revisionId");
		const DATA_JSON_VALUE* compatibility =
			manifest.Find("clientPresentationCompatibility");
		if (nullptr == schema || !schema->Is_String() ||
			"lostark.valtan-tuning-revision-manifest" != schema->Get_String() ||
			nullptr == formatVersion || !formatVersion->Is_Number() ||
			formatVersion->Was_FloatingPointToken() ||
			1.0 != formatVersion->Get_Number() ||
			nullptr == revisionId || !revisionId->Is_String() ||
			revisionHex != revisionId->Get_String() ||
			nullptr == compatibility || !compatibility->Is_Object())
		{
			status = "Candidate revision manifest identity is invalid.";
			return false;
		}
		const DATA_JSON_VALUE* mode = compatibility->Find("mode");
		const DATA_JSON_VALUE* generation =
			compatibility->Find("presentationGenerationId");
		const DATA_JSON_VALUE* lanes = compatibility->Find("requiredLanes");
		const DATA_JSON_VALUE* artifacts = compatibility->Find("artifacts");
		if (nullptr == mode || !mode->Is_String() ||
			"BYTE_IDENTICAL_TO_ACTIVE" != mode->Get_String() ||
			nullptr == generation || !generation->Is_String() ||
			!IsLowerSha256(generation->Get_String()) ||
			nullptr == lanes || !lanes->Is_Array() ||
			nullptr == artifacts || !artifacts->Is_Array())
		{
			status = "Candidate presentation compatibility contract is invalid.";
			return false;
		}
		const std::filesystem::path generationManifestPath = revisionRoot /
			"Runtime" / "Gameplay" / "ValtanPresentationGenerations" /
			(generation->Get_String() + ".json");
		std::string generationManifestSha;
		std::uint64_t generationManifestBytes = 0u;
		if (!IsDescendantPath(revisionRoot, generationManifestPath) ||
			!HashFileSha256(generationManifestPath, generationManifestSha,
				generationManifestBytes, status) ||
			generationManifestSha != generation->Get_String())
		{
			status =
				"Candidate presentation generation manifest is missing or has the wrong content identity.";
			return false;
		}

		std::uint32_t declaredLaneMask = 0u;
		for (const DATA_JSON_VALUE& laneValue : lanes->Get_Array())
		{
			if (!laneValue.Is_String())
			{
				status = "Candidate presentation lane is not a stable token.";
				return false;
			}
			const std::uint32_t bit = PresentationLaneBit(laneValue.Get_String());
			if (0u == bit || 0u != (declaredLaneMask & bit))
			{
				status = "Candidate presentation lane is unknown or duplicated.";
				return false;
			}
			declaredLaneMask |= bit;
		}
		if (GAMEPLAY_PRESENTATION_KNOWN_LANE_MASK != declaredLaneMask ||
			0u != (requestedLaneMask & ~declaredLaneMask))
		{
			status = "Candidate does not declare every required presentation lane.";
			return false;
		}

		std::unordered_map<std::string, std::string> allowedArtifacts;
		for (const CNetworkManager::PRESENTATION_ARTIFACT_BASELINE& baseline :
			baselineArtifacts)
		{
			if (!allowedArtifacts.emplace(
					baseline.strRelativePath, baseline.strLane).second)
			{
				status = "Current saved presentation artifact set is duplicated.";
				return false;
			}
		}
		std::unordered_map<std::string,
			const CNetworkManager::PRESENTATION_ARTIFACT_BASELINE*>
			baselineByPath;
		for (const CNetworkManager::PRESENTATION_ARTIFACT_BASELINE& baseline :
			baselineArtifacts)
		{
			const auto allowed = allowedArtifacts.find(baseline.strRelativePath);
			if (allowedArtifacts.end() == allowed ||
				allowed->second != baseline.strLane ||
				!IsLowerSha256(baseline.strSha256) ||
				baseline.iBytes > MAX_PRESENTATION_ALIAS_ARTIFACT_BYTES ||
				!baselineByPath.emplace(
					baseline.strRelativePath, &baseline).second)
			{
				status = "Current saved presentation artifact set is invalid.";
				return false;
			}
		}
		if (baselineByPath.size() != allowedArtifacts.size())
		{
			status = "Current saved presentation artifact set is incomplete.";
			return false;
		}
		std::unordered_set<std::string> admittedPaths;
		std::uint32_t artifactLaneMask = 0u;
		std::string firstCurrentMismatch =
			generation->Get_String() ==
				Format_GameplayDataRevision(presentationGenerationId) ?
			std::string{} : std::string{ "presentation generation M" };
		for (const DATA_JSON_VALUE& artifact : artifacts->Get_Array())
		{
			if (!artifact.Is_Object())
			{
				status = "Candidate presentation artifact row is invalid.";
				return false;
			}
			const DATA_JSON_VALUE* pathValue = artifact.Find("path");
			const DATA_JSON_VALUE* laneValue = artifact.Find("lane");
			const DATA_JSON_VALUE* shaValue = artifact.Find("sha256");
			const DATA_JSON_VALUE* sourceShaValue =
				artifact.Find("repositorySourceSha256");
			std::uint64_t declaredBytes = 0u;
			if (nullptr == pathValue || !pathValue->Is_String() ||
				nullptr == laneValue || !laneValue->Is_String() ||
				nullptr == shaValue || !shaValue->Is_String() ||
				nullptr == sourceShaValue || !sourceShaValue->Is_String() ||
				!ReadExactUnsigned(artifact, "bytes", declaredBytes) ||
				declaredBytes > MAX_PRESENTATION_ALIAS_ARTIFACT_BYTES)
			{
				status = "Candidate presentation artifact fields are invalid.";
				return false;
			}
			const std::string relative = pathValue->Get_String();
			const auto allowed = allowedArtifacts.find(relative);
			if (allowedArtifacts.end() == allowed ||
				allowed->second != laneValue->Get_String() ||
				!admittedPaths.insert(relative).second ||
				!IsLowerSha256(shaValue->Get_String()) ||
				!IsLowerSha256(sourceShaValue->Get_String()) ||
				shaValue->Get_String() != sourceShaValue->Get_String())
			{
				status = "Candidate presentation artifact identity is not allowlisted.";
				return false;
			}
			const std::filesystem::path relativePath =
				std::filesystem::path(relative);
			const std::filesystem::path candidatePath = revisionRoot / relativePath;
			if (!IsDescendantPath(revisionRoot, candidatePath) ||
				!IsDescendantPath(repositoryRoot, candidatePath))
			{
				status = "Candidate presentation artifact path escaped its root.";
				return false;
			}
			std::string candidateSha;
			std::uint64_t candidateBytes = 0u;
			const auto baseline = baselineByPath.find(relative);
			if (!HashFileSha256(
					candidatePath, candidateSha, candidateBytes, status))
			{
				return false;
			}
			if (baselineByPath.end() == baseline ||
				candidateBytes != declaredBytes ||
				candidateSha != shaValue->Get_String())
			{
				status =
					"Candidate presentation artifact bytes do not match its "
					"immutable manifest: " + relative + ".";
				return false;
			}
			if (baseline->second->iBytes != declaredBytes ||
				baseline->second->strSha256 != sourceShaValue->Get_String())
			{
				if (firstCurrentMismatch.empty())
					firstCurrentMismatch = relative;
			}
			artifactLaneMask |= PresentationLaneBit(laneValue->Get_String());
		}
		if (admittedPaths.size() != allowedArtifacts.size() ||
			artifactLaneMask != GAMEPLAY_PRESENTATION_KNOWN_LANE_MASK)
		{
			status = "Candidate presentation compatibility artifact set is incomplete.";
			return false;
		}
		if (!firstCurrentMismatch.empty())
		{
			status =
				"The immutable candidate does not match the current saved typed "
				"presentation generation at " + firstCurrentMismatch + ".";
			return false;
		}
		status =
			"The immutable candidate exactly matches the current saved typed "
			"presentation generation.";
		return true;
	}

#else
	bool CapturePresentationArtifactBaseline(
		std::vector<CNetworkManager::PRESENTATION_ARTIFACT_BASELINE>& output,
		Client::VALTAN_PRESENTATION_GENERATION_RECEIPT& receipt,
		std::string& status)
	{
		Client::CValtanPresentationGenerationReadAdmission admission;
		Client::VALTAN_PRESENTATION_GENERATION_RECEIPT stagedReceipt;
		if (!admission.Acquire_PackagedBaseline(stagedReceipt, status) ||
			!admission.Validate_StillCurrent(status))
		{
			return false;
		}
		std::vector<CNetworkManager::PRESENTATION_ARTIFACT_BASELINE> staged;
		staged.reserve(stagedReceipt.Artifacts.size());
		for (const auto& artifact : stagedReceipt.Artifacts)
		{
			CNetworkManager::PRESENTATION_ARTIFACT_BASELINE row;
			row.strRelativePath = artifact.strRelativePath;
			row.strLane = artifact.strLane;
			row.strSha256 = LostArk::Shared::Format_GameplayDataRevision(
				artifact.Revision);
			row.iBytes = artifact.iBytes;
			staged.push_back(std::move(row));
		}
		output = std::move(staged);
		receipt = std::move(stagedReceipt);
		status = "Captured the current validated Client presentation sources.";
		return true;
	}
#endif
}

//Socket worker thread�� client main thread�� �и��ϱ� ���ؼ� �����Ѵ�.
//workter thread -> byte ���Ű� frame ������ ����
//main thread -> frame �ؼ��� replication event ����

CNetworkManager& CNetworkManager::Get()
{
	static CNetworkManager instance;
	return instance;
}

std::string CNetworkManager::Resolve_ServerHost()
{
	/* The temporary team LAN endpoint is the direct-launch fallback. The
	   process-local environment still wins so isolated tests can name loopback. */
	constexpr char DEFAULT_SERVER_HOST[] = "192.168.0.14";
	constexpr char SERVER_HOST_ENVIRONMENT[] = "LOSTARK_SERVER_HOST";
	char configuredHost[64]{};
	const DWORD configuredLength = ::GetEnvironmentVariableA(
		SERVER_HOST_ENVIRONMENT,
		configuredHost,
		static_cast<DWORD>(std::size(configuredHost)));
	if (0u != configuredLength &&
		configuredLength < std::size(configuredHost) &&
		"0.0.0.0" != std::string_view{ configuredHost })
	{
		return configuredHost;
	}
#ifdef _DEBUG
	/* The VS debugger environment is the team endpoint authority. A developer
	   may still opt into a loopback Server when launching outside VS, but that
	   disabled-by-default convenience file never overrides an explicit host. */
	if (const std::string localHost = Resolve_DebugLocalServerHost();
		!localHost.empty())
	{
		return localHost;
	}
#endif
	return DEFAULT_SERVER_HOST;
}

std::string CNetworkManager::Resolve_MapEditorServerHost()
{
	// Test(Map Editor) and product worlds must enter through the same
	// authoritative Server endpoint. Keep this compatibility entry point so
	// existing lobby code cannot reintroduce a private loopback route.
	return Resolve_ServerHost();
}

bool CNetworkManager::Initialize()
{
	if (m_isWinSocketInitialized)
		return true;

	WSADATA winSockData{};
	const int result = ::WSAStartup(MAKEWORD(2, 2), &winSockData);
	if (0 != result)
	{
		m_iLastErrorCode = result;
		return false;
	}

	const bool isVersionSupported =
		2 == LOBYTE(winSockData.wVersion) &&
		2 == HIBYTE(winSockData.wVersion);

	if (!isVersionSupported)
	{
		m_iLastErrorCode = WSAVERNOTSUPPORTED;
		::WSACleanup();
		return false;
	}

	m_isWinSocketInitialized = true;
	m_iLastErrorCode = 0;
	return true;
}

void CNetworkManager::Shutdown()
{
	Close_ServerConnection();

	if (!m_isWinSocketInitialized)
		return;

	::WSACleanup();
	m_isWinSocketInitialized = false;
}
//�� �����Ӹ��� main thread���� ȣ��
void CNetworkManager::Update()
{
	if (m_hasProtocolFailure.load())
	{
		if (INVALID_SOCKET != m_hServerSocket)
			Fail_Protocol(m_iLastErrorCode.load());
		return;
	}

	//Inbound mutex ��� -> Worker�� ���� raw frame queue�� ���� queue�� swap
	//mutex ���� -> frame�� ���� ������� handle_frame�� ����

	std::deque<LostArk::Shared::PACKET_FRAME> receivedFrames;
	// Worker�� �ϼ��� Frame�� Main Thread�� �Ű� Packet �޽�����
	// Replication Event�� �����Ѵ�. Engine ��ü�� ���⼭ ���� �������� �ʴ´�.
	{
		std::scoped_lock lock
		{
			m_InboundMutex
		};
		//swap�� ���ؼ� frame�� �ؼ��ϴ� ���� network workter�� ���
		//�� frame�� ���� �� �ִ�.
		receivedFrames.swap(m_InboundFrames);
		m_SessionDiagnostic.Record_RawQueueDepth(m_InboundFrames.size());
	}
	
	for (const auto& frame : receivedFrames)
	{
		Handle_Frame(frame);
		if (m_hasProtocolFailure.load())
			break;
	}
}

bool CNetworkManager::Connect_To_Server(
	const std::string_view host,
	const std::uint16_t port)
{
	if (Is_Connected())
		return true;

	// A new explicit connect is the only boundary that clears the previous
	// terminal latch. First reclaim any stale worker/socket from that previous
	// generation, then start the new capture generation.
	if (INVALID_SOCKET != m_hServerSocket || m_ReceiveThread.joinable())
		Close_ServerConnection();
	m_SessionDiagnostic.Begin_Attempt(
		host, port, LostArk::Shared::NETWORK_PROTOCOL_VERSION);

	if (!m_isWinSocketInitialized)
	{
		m_iLastErrorCode = WSANOTINITIALISED;
		m_SessionDiagnostic.Record_Terminal(
			LostArk::Shared::SESSION_DIAGNOSTIC_REASON::CLIENT_CONNECT_FAILED,
			WSANOTINITIALISED,
			LostArk::Shared::PACKET_TYPE::INVALID,
			"WinSock was not initialized before connect.");
		return false;
	}

	if (host.empty() || host.size() > 63u || 0u == port)
	{
		m_iLastErrorCode = WSAEINVAL;
		m_SessionDiagnostic.Record_Terminal(
			LostArk::Shared::SESSION_DIAGNOSTIC_REASON::CLIENT_CONNECT_FAILED,
			WSAEINVAL,
			LostArk::Shared::PACKET_TYPE::INVALID,
			"Server host or port was invalid.");
		return false;
	}

	m_hServerSocket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == m_hServerSocket)
	{
		m_iLastErrorCode = ::WSAGetLastError();
		m_SessionDiagnostic.Record_Terminal(
			LostArk::Shared::SESSION_DIAGNOSTIC_REASON::CLIENT_CONNECT_FAILED,
			m_iLastErrorCode.load(),
			LostArk::Shared::PACKET_TYPE::INVALID,
			"TCP socket creation failed.");
		return false;
	}

	sockaddr_in serverAddress{};
	serverAddress.sin_family = AF_INET;
	const std::string hostText{ host };
	if ("localhost" == hostText)
	{
		serverAddress.sin_addr.s_addr = ::htonl(INADDR_LOOPBACK);
	}
	else if (1 != ::InetPtonA(
		AF_INET,
		hostText.c_str(),
		&serverAddress.sin_addr))
	{
		m_iLastErrorCode = WSAEINVAL;
		m_SessionDiagnostic.Record_Terminal(
			LostArk::Shared::SESSION_DIAGNOSTIC_REASON::CLIENT_CONNECT_FAILED,
			WSAEINVAL,
			LostArk::Shared::PACKET_TYPE::INVALID,
			"Server host was not an IPv4 address or localhost.");
		Close_ServerConnection();
		return false;
	}
	serverAddress.sin_port = ::htons(port);

	u_long nonBlocking = 1;
	if (SOCKET_ERROR == ::ioctlsocket(
		m_hServerSocket,
		FIONBIO,
		&nonBlocking))
	{
		m_iLastErrorCode = ::WSAGetLastError();
		m_SessionDiagnostic.Record_Terminal(
			LostArk::Shared::SESSION_DIAGNOSTIC_REASON::CLIENT_CONNECT_FAILED,
			m_iLastErrorCode.load(),
			LostArk::Shared::PACKET_TYPE::INVALID,
			"Could not enable non-blocking connect mode.");
		Close_ServerConnection();
		return false;
	}

	const int connectResult = ::connect(
		m_hServerSocket,
		reinterpret_cast<const sockaddr*>(&serverAddress),
		sizeof(serverAddress));
	if (SOCKET_ERROR == connectResult)
	{
		const int connectError = ::WSAGetLastError();
		if (WSAEWOULDBLOCK != connectError)
		{
			m_iLastErrorCode = connectError;
			m_SessionDiagnostic.Record_Terminal(
				LostArk::Shared::SESSION_DIAGNOSTIC_REASON::CLIENT_CONNECT_FAILED,
				connectError,
				LostArk::Shared::PACKET_TYPE::INVALID,
				"connect() failed before the socket became writable.");
			Close_ServerConnection();
			return false;
		}

		fd_set writableSockets;
		FD_ZERO(&writableSockets);
		FD_SET(m_hServerSocket, &writableSockets);
		fd_set errorSockets;
		FD_ZERO(&errorSockets);
		FD_SET(m_hServerSocket, &errorSockets);
		timeval timeout{};
		timeout.tv_sec = 1;
		timeout.tv_usec = 500000;
		const int selectResult = ::select(
			0,
			nullptr,
			&writableSockets,
			&errorSockets,
			&timeout);
		if (selectResult <= 0)
		{
			m_iLastErrorCode = 0 == selectResult ?
				WSAETIMEDOUT : ::WSAGetLastError();
			m_SessionDiagnostic.Record_Terminal(
				LostArk::Shared::SESSION_DIAGNOSTIC_REASON::CLIENT_CONNECT_FAILED,
				m_iLastErrorCode.load(),
				LostArk::Shared::PACKET_TYPE::INVALID,
				0 == selectResult ?
					"Connect did not complete within 1500 ms." :
					"select() failed while waiting for connect.");
			Close_ServerConnection();
			return false;
		}

		int socketError = 0;
		int socketErrorSize = sizeof(socketError);
		if (SOCKET_ERROR == ::getsockopt(
			m_hServerSocket,
			SOL_SOCKET,
			SO_ERROR,
			reinterpret_cast<char*>(&socketError),
			&socketErrorSize) ||
			0 != socketError)
		{
			m_iLastErrorCode = 0 != socketError ?
				socketError : ::WSAGetLastError();
			m_SessionDiagnostic.Record_Terminal(
				LostArk::Shared::SESSION_DIAGNOSTIC_REASON::CLIENT_CONNECT_FAILED,
				m_iLastErrorCode.load(),
				LostArk::Shared::PACKET_TYPE::INVALID,
				"Connected socket reported SO_ERROR.");
			Close_ServerConnection();
			return false;
		}
	}

	nonBlocking = 0;
	if (SOCKET_ERROR == ::ioctlsocket(
		m_hServerSocket,
		FIONBIO,
		&nonBlocking))
	{
		m_iLastErrorCode = ::WSAGetLastError();
		m_SessionDiagnostic.Record_Terminal(
			LostArk::Shared::SESSION_DIAGNOSTIC_REASON::CLIENT_CONNECT_FAILED,
			m_iLastErrorCode.load(),
			LostArk::Shared::PACKET_TYPE::INVALID,
			"Could not restore blocking socket mode after connect.");
		Close_ServerConnection();
		return false;
	}

	sockaddr_in localAddress{};
	int localAddressLength = sizeof(localAddress);
	char localAddressText[INET_ADDRSTRLEN]{};
	if (0 == ::getsockname(
		m_hServerSocket,
		reinterpret_cast<sockaddr*>(&localAddress),
		&localAddressLength) &&
		nullptr != ::InetNtopA(
			AF_INET,
			&localAddress.sin_addr,
			localAddressText,
			static_cast<DWORD>(std::size(localAddressText))))
	{
		m_SessionDiagnostic.Record_LocalEndpoint(
			std::string{ localAddressText } + ":" +
			std::to_string(::ntohs(localAddress.sin_port)));
	}
	else
	{
		// Correlation is best-effort and must never turn a usable gameplay
		// connection into a failed attempt.
		m_SessionDiagnostic.Record_LocalEndpoint("unavailable");
	}

	m_StreamParser.Reset();
	Reset_WorldInboundState();
	m_iLastErrorCode.store(0);
	m_hasProtocolFailure.store(false);
	m_isReceiveRunning.store(true);
	// Persist success before the worker can observe an immediate FIN/RST, so
	// JSONL event order always reflects connect before its terminal edge.
	m_SessionDiagnostic.Record_Event("connect.succeeded");
	m_ReceiveThread = std::thread(
		&CNetworkManager::Receive_Loop,
		this,
		m_hServerSocket);
	return true;
}
bool CNetworkManager::Send_EnterWorld(
	LostArk::Shared::WORLD_ID worldId,
	LostArk::Shared::CHARACTER_CLASS_ID characterClass,
	std::string_view nickName)
{
	using namespace LostArk::Shared;

	if (!Is_Connected())
	{
		m_SessionDiagnostic.Record_Terminal(
			SESSION_DIAGNOSTIC_REASON::CLIENT_ENTER_SEND_FAILED,
			WSAENOTCONN,
			PACKET_TYPE::C2S_ENTER_WORLD,
			"C2S_ENTER_WORLD was requested without a live socket.");
		return false;
	}

	C2S_ENTER_WORLD message{};
	message.iProtocolVersion = NETWORK_PROTOCOL_VERSION;
	message.eWorldId = worldId;
	message.eCharacterClass = characterClass;
	message.strNickName = std::string{ nickName };

	CPacketWriter payloadWriter;
	if (!Write_Message(payloadWriter, message))
	{
		m_SessionDiagnostic.Record_Terminal(
			SESSION_DIAGNOSTIC_REASON::CLIENT_ENTER_SEND_FAILED,
			WSAEINVAL,
			PACKET_TYPE::C2S_ENTER_WORLD,
			"C2S_ENTER_WORLD serialization failed.");
		return false;
	}

	std::vector<std::uint8_t> frameBytes;
	if (!Build_Packet_Frame(
		PACKET_TYPE::C2S_ENTER_WORLD,
		payloadWriter.Get_Buffer(),
		frameBytes))
	{
		m_SessionDiagnostic.Record_Terminal(
			SESSION_DIAGNOSTIC_REASON::CLIENT_ENTER_SEND_FAILED,
			WSAEINVAL,
			PACKET_TYPE::C2S_ENTER_WORLD,
			"C2S_ENTER_WORLD frame construction failed.");
		return false;
	}

	if (!Send_All(frameBytes, PACKET_TYPE::C2S_ENTER_WORLD))
		return false;

	// The request is now committed to the socket. From this point only its
	// future acceptance may establish a world; older room events are stale.
	Reset_WorldInboundState();
	m_eLocalCharacterClass = characterClass;
	m_SessionDiagnostic.Record_EnterSent(worldId);
	return true;
}

bool CNetworkManager::Send_MoveGoal(std::uint32_t clientSequence, float goalX, float goalZ)
{
	//���� ���� �˻� -> C2S_MOVE �� ����ü ���� -> sequence�� goal XZ ����
	//packetwriter�� payload ����ȭ -> C2S_MOVE frame ���� -> send_all
	//client sequence�� animation�� ������ ��� �ִ� �ǰ�? �ִϸ��̼� 1 2 3 4 ������ ������ ��� �ִ�?
	//�� �ִϸ��̼ǿ� ���� �κ��� ��� ó���ؾ� �ұ�?
	using namespace LostArk::Shared;

	if (!Is_Connected())
		return false;

	C2S_MOVE message{};
	message.iClientSequence = clientSequence;
	message.fGoalX = goalX;
	message.fGoalZ = goalZ;

	CPacketWriter payloadWriter;
	if (!Write_Message(payloadWriter, message))
		return false;

	std::vector<std::uint8_t> frameBytes;
	if (!Build_Packet_Frame(
		PACKET_TYPE::C2S_MOVE,
		payloadWriter.Get_Buffer(),
		frameBytes))
	{
		return false;
	}

	return Send_All(frameBytes);
}

bool CNetworkManager::Send_UseSkill(
	const std::uint32_t clientSequence,
	const LostArk::Shared::SKILL_ID skillId,
	const float aimX,
	const float aimZ)
{
	using namespace LostArk::Shared;
	if (!Is_Connected())
		return false;
	C2S_USE_SKILL message{};
	message.iClientSequence = clientSequence;
	message.iSkillId = skillId;
	message.fAimX = aimX;
	message.fAimZ = aimZ;
	CPacketWriter payloadWriter;
	if (!Write_Message(payloadWriter, message))
		return false;
	std::vector<std::uint8_t> frameBytes;
	return Build_Packet_Frame(
		PACKET_TYPE::C2S_USE_SKILL,
		payloadWriter.Get_Buffer(),
		frameBytes) && Send_All(frameBytes);
}

bool CNetworkManager::Send_UseGroundTargetSkill(
	const std::uint32_t clientSequence,
	const LostArk::Shared::SKILL_ID skillId,
	const float targetX,
	const float targetZ)
{
	using namespace LostArk::Shared;
	if (!Is_Connected())
		return false;
	C2S_USE_SKILL message{};
	message.iClientSequence = clientSequence;
	message.iSkillId = skillId;
	message.eTargetIntent = SKILL_TARGET_INTENT_KIND::GROUND_POINT;
	message.fAimX = targetX;
	message.fAimZ = targetZ;
	CPacketWriter payloadWriter;
	if (!Write_Message(payloadWriter, message))
		return false;
	std::vector<std::uint8_t> frameBytes;
	return Build_Packet_Frame(
		PACKET_TYPE::C2S_USE_SKILL,
		payloadWriter.Get_Buffer(), frameBytes) && Send_All(frameBytes);
}

bool CNetworkManager::Send_ReleaseSkill(
	const std::uint32_t clientSequence,
	const LostArk::Shared::SKILL_ID skillId)
{
	using namespace LostArk::Shared;
	if (!Is_Connected())
		return false;
	C2S_RELEASE_SKILL message{};
	message.iClientSequence = clientSequence;
	message.iSkillId = skillId;
	CPacketWriter payloadWriter;
	if (!Write_Message(payloadWriter, message))
		return false;
	std::vector<std::uint8_t> frameBytes;
	return Build_Packet_Frame(
		PACKET_TYPE::C2S_RELEASE_SKILL,
		payloadWriter.Get_Buffer(),
		frameBytes) && Send_All(frameBytes);
}

bool CNetworkManager::Send_SkillAim(
	const std::uint32_t clientSequence,
	const LostArk::Shared::SKILL_ID skillId,
	const float aimX,
	const float aimZ)
{
	using namespace LostArk::Shared;
	if (!Is_Connected())
		return false;
	C2S_UPDATE_SKILL_AIM message{};
	message.iClientSequence = clientSequence;
	message.iSkillId = skillId;
	message.fAimX = aimX;
	message.fAimZ = aimZ;
	CPacketWriter payloadWriter;
	if (!Write_Message(payloadWriter, message))
		return false;
	std::vector<std::uint8_t> frameBytes;
	return Build_Packet_Frame(
		PACKET_TYPE::C2S_UPDATE_SKILL_AIM,
		payloadWriter.Get_Buffer(),
		frameBytes) && Send_All(frameBytes);
}

bool CNetworkManager::Send_EstherSkill(
	const std::uint32_t clientSequence,
	const std::uint8_t slotIndex,
	const float aimX,
	const float aimZ)
{
	using namespace LostArk::Shared;
	if (!Is_Connected())
		return false;
	C2S_USE_ESTHER_SKILL message{};
	message.iClientSequence = clientSequence;
	message.iSlotIndex = slotIndex;
	message.fAimX = aimX;
	message.fAimZ = aimZ;
	CPacketWriter payloadWriter;
	if (!Write_Message(payloadWriter, message))
		return false;
	std::vector<std::uint8_t> frameBytes;
	return Build_Packet_Frame(
		PACKET_TYPE::C2S_USE_ESTHER_SKILL,
		payloadWriter.Get_Buffer(),
		frameBytes) && Send_All(frameBytes);
}

bool CNetworkManager::Send_RevivePlayer(
	const std::uint32_t clientSequence)
{
	using namespace LostArk::Shared;
	if (!Is_Connected())
		return false;
	C2S_REVIVE_PLAYER message{};
	message.iClientSequence = clientSequence;
	CPacketWriter payloadWriter;
	if (!Write_Message(payloadWriter, message))
		return false;
	std::vector<std::uint8_t> frameBytes;
	return Build_Packet_Frame(
		PACKET_TYPE::C2S_REVIVE_PLAYER,
		payloadWriter.Get_Buffer(),
		frameBytes) && Send_All(frameBytes);
}

#ifdef _DEBUG
bool CNetworkManager::Send_DebugKillSelf(
	const std::uint32_t clientSequence)
{
	using namespace LostArk::Shared;
	if (!Is_Connected())
		return false;
	C2S_DEBUG_KILL_SELF message{};
	message.iClientSequence = clientSequence;
	CPacketWriter payloadWriter;
	if (!Write_Message(payloadWriter, message))
		return false;
	std::vector<std::uint8_t> frameBytes;
	return Build_Packet_Frame(
		PACKET_TYPE::C2S_DEBUG_KILL_SELF,
		payloadWriter.Get_Buffer(),
		frameBytes) && Send_All(frameBytes);
}
#endif

bool CNetworkManager::Send_DebugEnterKakulSaydonArena(
	const std::uint32_t requestSequence)
{
	using namespace LostArk::Shared;
	if (!Is_Connected())
		return false;
	C2S_DEBUG_ENTER_KAKULSAYDON_ARENA message{};
	message.iRequestSequence = requestSequence;
	CPacketWriter payloadWriter;
	if (!Write_Message(payloadWriter, message))
		return false;
	std::vector<std::uint8_t> frameBytes;
	return Build_Packet_Frame(
		PACKET_TYPE::C2S_DEBUG_ENTER_KAKULSAYDON_ARENA,
		payloadWriter.Get_Buffer(), frameBytes) && Send_All(frameBytes);
}

bool CNetworkManager::Send_DebugTeleportToPlacement(
	const std::uint32_t requestSequence,
	const std::string_view placementId)
{
	using namespace LostArk::Shared;
	if (!Is_Connected())
		return false;
	C2S_DEBUG_TELEPORT_TO_PLACEMENT message{};
	message.iRequestSequence = requestSequence;
	message.strPlacementId.assign(placementId.begin(), placementId.end());
	CPacketWriter payloadWriter;
	if (!Write_Message(payloadWriter, message))
		return false;
	std::vector<std::uint8_t> frameBytes;
	return Build_Packet_Frame(
		PACKET_TYPE::C2S_DEBUG_TELEPORT_TO_PLACEMENT,
		payloadWriter.Get_Buffer(), frameBytes) && Send_All(frameBytes);
}

bool CNetworkManager::Send_ChangeCharacterClass(
	const std::uint32_t clientSequence,
	const LostArk::Shared::CHARACTER_CLASS_ID characterClass)
{
	using namespace LostArk::Shared;
	if (!Is_Connected())
		return false;
	C2S_CHANGE_CHARACTER_CLASS message{};
	message.iClientSequence = clientSequence;
	message.eCharacterClass = characterClass;
	CPacketWriter payloadWriter;
	if (!Write_Message(payloadWriter, message))
		return false;
	std::vector<std::uint8_t> frameBytes;
	return Build_Packet_Frame(
		PACKET_TYPE::C2S_CHANGE_CHARACTER_CLASS,
		payloadWriter.Get_Buffer(),
		frameBytes) && Send_All(frameBytes);
}

bool CNetworkManager::Send_SpawnWorldEntity(
	const std::string_view placementId)
{
	using namespace LostArk::Shared;
	if (!Is_Connected())
		return false;

	C2S_SPAWN_WORLD_ENTITY message{};
	message.strPlacementId = std::string{ placementId };
	CPacketWriter payloadWriter;
	if (!Write_Message(payloadWriter, message))
		return false;

	std::vector<std::uint8_t> frameBytes;
	return Build_Packet_Frame(
		PACKET_TYPE::C2S_SPAWN_WORLD_ENTITY,
		payloadWriter.Get_Buffer(),
		frameBytes) && Send_All(frameBytes);
}

bool CNetworkManager::Send_DespawnAllWorldEntities(
	const std::uint32_t requestSequence)
{
	using namespace LostArk::Shared;
	if (!Is_Connected())
		return false;

	C2S_DESPAWN_ALL_WORLD_ENTITIES message{};
	message.iRequestSequence = requestSequence;
	CPacketWriter payloadWriter;
	if (!Write_Message(payloadWriter, message))
		return false;

	std::vector<std::uint8_t> frameBytes;
	return Build_Packet_Frame(
		PACKET_TYPE::C2S_DESPAWN_ALL_WORLD_ENTITIES,
		payloadWriter.Get_Buffer(),
		frameBytes) && Send_All(frameBytes);
}

bool CNetworkManager::Send_ConfirmNpcEntry(
	const std::uint32_t requestSequence,
	const std::string_view npcPlacementId)
{
	using namespace LostArk::Shared;
	if (!Is_Connected())
		return false;

	C2S_CONFIRM_NPC_ENTRY message{};
	message.iRequestSequence = requestSequence;
	message.strNpcPlacementId = std::string{ npcPlacementId };
	CPacketWriter payloadWriter;
	if (!Write_Message(payloadWriter, message))
		return false;

	std::vector<std::uint8_t> frameBytes;
	return Build_Packet_Frame(
		PACKET_TYPE::C2S_CONFIRM_NPC_ENTRY,
		payloadWriter.Get_Buffer(),
		frameBytes) && Send_All(frameBytes);
}

bool CNetworkManager::Send_ReturnToBern(const std::uint32_t requestSequence)
{
	using namespace LostArk::Shared;
	if (!Is_Connected())
		return false;

	C2S_RETURN_TO_BERN message{};
	message.iRequestSequence = requestSequence;
	CPacketWriter payloadWriter;
	if (!Write_Message(payloadWriter, message))
		return false;

	std::vector<std::uint8_t> frameBytes;
	return Build_Packet_Frame(
		PACKET_TYPE::C2S_RETURN_TO_BERN,
		payloadWriter.Get_Buffer(),
		frameBytes) && Send_All(frameBytes);
}

bool CNetworkManager::Send_PartyInvite(
	const std::uint32_t requestSequence,
	const LostArk::Shared::NET_ENTITY_ID targetNetEntityId)
{
	using namespace LostArk::Shared;
	if (!Is_Connected())
		return false;

	C2S_PARTY_INVITE message{};
	message.iRequestSequence = requestSequence;
	message.iTargetNetEntityId = targetNetEntityId;
	CPacketWriter payloadWriter;
	if (!Write_Message(payloadWriter, message))
		return false;

	std::vector<std::uint8_t> frameBytes;
	return Build_Packet_Frame(
		PACKET_TYPE::C2S_PARTY_INVITE,
		payloadWriter.Get_Buffer(),
		frameBytes) && Send_All(frameBytes);
}

bool CNetworkManager::Send_PartyInviteRespond(
	const std::uint32_t requestSequence,
	const LostArk::Shared::NET_ENTITY_ID fromNetEntityId,
	const bool accepted)
{
	using namespace LostArk::Shared;
	if (!Is_Connected())
		return false;

	C2S_PARTY_INVITE_RESPOND message{};
	message.iRequestSequence = requestSequence;
	message.iFromNetEntityId = fromNetEntityId;
	message.bAccepted = accepted;
	CPacketWriter payloadWriter;
	if (!Write_Message(payloadWriter, message))
		return false;

	std::vector<std::uint8_t> frameBytes;
	return Build_Packet_Frame(
		PACKET_TYPE::C2S_PARTY_INVITE_RESPOND,
		payloadWriter.Get_Buffer(),
		frameBytes) && Send_All(frameBytes);
}

bool CNetworkManager::Send_Chat(const std::string& text)
{
	using namespace LostArk::Shared;
	if (!Is_Connected())
		return false;

	C2S_CHAT message{};
	message.strText = text;
	CPacketWriter payloadWriter;
	if (!Write_Message(payloadWriter, message))
		return false;

	std::vector<std::uint8_t> frameBytes;
	return Build_Packet_Frame(
		PACKET_TYPE::C2S_CHAT,
		payloadWriter.Get_Buffer(),
		frameBytes) && Send_All(frameBytes);
}

bool CNetworkManager::Send_DebugGiveItem(
	const std::uint32_t requestSequence,
	const std::string_view itemId,
	const std::uint32_t quantity)
{
	using namespace LostArk::Shared;
	if (!Is_Connected())
		return false;

	C2S_DEBUG_GIVE_ITEM message{};
	message.iRequestSequence = requestSequence;
	message.strItemId = std::string{ itemId };
	message.iQuantity = quantity;
	CPacketWriter payloadWriter;
	if (!Write_Message(payloadWriter, message))
		return false;

	std::vector<std::uint8_t> frameBytes;
	return Build_Packet_Frame(
		PACKET_TYPE::C2S_DEBUG_GIVE_ITEM,
		payloadWriter.Get_Buffer(),
		frameBytes) && Send_All(frameBytes);
}

bool CNetworkManager::Send_UseItem(
	const std::uint32_t requestSequence,
	const std::string_view itemId)
{
	using namespace LostArk::Shared;
	if (!Is_Connected())
		return false;

	C2S_USE_ITEM message{};
	message.iRequestSequence = requestSequence;
	message.strItemId = std::string{ itemId };
	CPacketWriter payloadWriter;
	if (!Write_Message(payloadWriter, message))
		return false;

	std::vector<std::uint8_t> frameBytes;
	return Build_Packet_Frame(
		PACKET_TYPE::C2S_USE_ITEM,
		payloadWriter.Get_Buffer(),
		frameBytes) && Send_All(frameBytes);
}

bool CNetworkManager::Send_ValtanAudition(
	const std::uint32_t requestSequence,
	const LostArk::Shared::VALTAN_AUDITION_OPERATION operation,
	const std::uint32_t targetHealthBar)
{
	using namespace LostArk::Shared;
	if (!Is_Connected())
		return false;

	C2S_VALTAN_AUDITION_REQUEST message{};
	message.iRequestSequence = requestSequence;
	message.eOperation = operation;
	message.iTargetHealthBar = targetHealthBar;
	CPacketWriter payloadWriter;
	if (!Write_Message(payloadWriter, message))
		return false;

	std::vector<std::uint8_t> frameBytes;
	return Build_Packet_Frame(
		PACKET_TYPE::C2S_VALTAN_AUDITION_REQUEST,
		payloadWriter.Get_Buffer(),
		frameBytes) && Send_All(frameBytes);
}

bool CNetworkManager::Send_ValtanPatternAuditionById(
	const std::uint32_t requestSequence,
	const std::string_view bossPlacementId,
	const std::string_view patternId,
	const LostArk::Shared::GameplayDataRevision&
		expectedActiveDefinitionRevision)
{
	using namespace LostArk::Shared;
	if (!Is_Connected())
		return false;

	C2S_VALTAN_AUDITION_REQUEST message{};
	message.iRequestSequence = requestSequence;
	message.eOperation = VALTAN_AUDITION_OPERATION::PLAY_PATTERN_ID;
	message.iTargetHealthBar = 0u;
	message.strBossPlacementId = std::string{ bossPlacementId };
	message.strPatternId = std::string{ patternId };
	message.ExpectedDefinitionRevision = expectedActiveDefinitionRevision;
	CPacketWriter payloadWriter;
	if (!Write_Message(payloadWriter, message))
		return false;

	std::vector<std::uint8_t> frameBytes;
	return Build_Packet_Frame(
		PACKET_TYPE::C2S_VALTAN_AUDITION_REQUEST,
		payloadWriter.Get_Buffer(),
		frameBytes) && Send_All(frameBytes);
}

bool CNetworkManager::Send_ValtanPatternRestart(
	const LostArk::Shared::C2S_VALTAN_AUDITION_REQUEST& message)
{
	using namespace LostArk::Shared;
	if (!Is_Connected() ||
		VALTAN_AUDITION_OPERATION::RESTART_PATTERN_ID != message.eOperation)
	{
		return false;
	}
	CPacketWriter payloadWriter;
	if (!Write_Message(payloadWriter, message))
		return false;
	std::vector<std::uint8_t> frameBytes;
	return Build_Packet_Frame(PACKET_TYPE::C2S_VALTAN_AUDITION_REQUEST,
		payloadWriter.Get_Buffer(), frameBytes) && Send_All(frameBytes);
}

bool CNetworkManager::Send_ValtanNextPatternCommand(
	const LostArk::Shared::C2S_VALTAN_AUDITION_REQUEST& message)
{
	using namespace LostArk::Shared;
	if (!Is_Connected() ||
		(VALTAN_AUDITION_OPERATION::QUEUE_NEXT_PATTERN_ID != message.eOperation &&
		 VALTAN_AUDITION_OPERATION::CLEAR_NEXT_PATTERN_ID != message.eOperation &&
		 VALTAN_AUDITION_OPERATION::QUEUE_NEXT_LIVE_PATTERN_ID != message.eOperation))
		return false;
	CPacketWriter payloadWriter;
	if (!Write_Message(payloadWriter, message))
		return false;
	std::vector<std::uint8_t> frameBytes;
	return Build_Packet_Frame(PACKET_TYPE::C2S_VALTAN_AUDITION_REQUEST,
		payloadWriter.Get_Buffer(), frameBytes) && Send_All(frameBytes);
}

bool CNetworkManager::Send_ValtanPatternFlowStart(
	const LostArk::Shared::C2S_DEBUG_VALTAN_PATTERN_FLOW_START& message)
{
	using namespace LostArk::Shared;
	if (!Is_Connected())
		return false;
	CPacketWriter payloadWriter;
	if (!Write_Message(payloadWriter, message))
		return false;
	std::vector<std::uint8_t> frameBytes;
	return Build_Packet_Frame(
		PACKET_TYPE::C2S_DEBUG_VALTAN_PATTERN_FLOW_START,
		payloadWriter.Get_Buffer(), frameBytes) && Send_All(frameBytes);
}

bool CNetworkManager::Send_ValtanPatternFlowStopAfterCurrent(
	const LostArk::Shared::C2S_DEBUG_VALTAN_PATTERN_FLOW_STOP_AFTER_CURRENT&
		message)
{
	using namespace LostArk::Shared;
	if (!Is_Connected())
		return false;
	CPacketWriter payloadWriter;
	if (!Write_Message(payloadWriter, message))
		return false;
	std::vector<std::uint8_t> frameBytes;
	return Build_Packet_Frame(
		PACKET_TYPE::C2S_DEBUG_VALTAN_PATTERN_FLOW_STOP_AFTER_CURRENT,
		payloadWriter.Get_Buffer(), frameBytes) && Send_All(frameBytes);
}

bool CNetworkManager::Send_DataRevisionPrepareResponse(
	const LostArk::Shared::C2S_DATA_REVISION_PREPARE_RESPONSE& message)
{
	using namespace LostArk::Shared;
	if (!Is_Connected())
		return false;

	CPacketWriter payloadWriter;
	if (!Write_Message(payloadWriter, message))
		return false;
	std::vector<std::uint8_t> frameBytes;
	return Build_Packet_Frame(
		PACKET_TYPE::C2S_DATA_REVISION_PREPARE_RESPONSE,
		payloadWriter.Get_Buffer(),
		frameBytes) && Send_All(frameBytes);
}

bool CNetworkManager::Send_DataRevisionPrepareRequest(
	const LostArk::Shared::C2S_DATA_REVISION_PREPARE_REQUEST& message)
{
	using namespace LostArk::Shared;
	if (!Is_Connected() ||
		m_GameplayRevisionState.hasOutstandingPrepareRequest)
		return false;

	CPacketWriter payloadWriter;
	if (!Write_Message(payloadWriter, message))
		return false;
	std::vector<std::uint8_t> frameBytes;
	if (!Build_Packet_Frame(
			PACKET_TYPE::C2S_DATA_REVISION_PREPARE_REQUEST,
			payloadWriter.Get_Buffer(), frameBytes) ||
		!Send_All(frameBytes))
	{
		return false;
	}
	m_GameplayRevisionState.hasOutstandingPrepareRequest = true;
	m_GameplayRevisionState.iOutstandingPrepareRequestSequence =
		message.iTransactionSequence;
	m_GameplayRevisionState.OutstandingPrepareCandidateRevision =
		message.CandidateRevision;
	return true;
}

bool CNetworkManager::Send_ValtanDecisionTraceQuery(
	const std::uint32_t requestSequence,
	const std::string_view bossPlacementId,
	const std::uint64_t afterTraceSequence)
{
	using namespace LostArk::Shared;
	if (!Is_Connected() || m_ValtanDecisionTraceState.isQueryPending)
		return false;

	C2S_VALTAN_DECISION_TRACE_QUERY message{};
	message.iRequestSequence = requestSequence;
	message.strBossPlacementId = std::string{ bossPlacementId };
	message.iAfterTraceSequence = afterTraceSequence;
	CPacketWriter payloadWriter;
	if (!Write_Message(payloadWriter, message))
		return false;
	std::vector<std::uint8_t> frameBytes;
	if (!Build_Packet_Frame(
			PACKET_TYPE::C2S_VALTAN_DECISION_TRACE_QUERY,
			payloadWriter.Get_Buffer(), frameBytes) ||
		!Send_All(frameBytes))
	{
		return false;
	}
	m_ValtanDecisionTraceState.isQueryPending = true;
	m_ValtanDecisionTraceState.iSubmittedRequestSequence = requestSequence;
	m_ValtanDecisionTraceState.strSubmittedBossPlacementId =
		std::string{ bossPlacementId };
	m_ValtanDecisionTraceState.iSubmittedAfterTraceSequence =
		afterTraceSequence;
	return true;
}

bool CNetworkManager::Try_Consume_EnterAccepted(LostArk::Shared::S2C_ENTER_ACCEPTED& message)
{
	// ���� �ϳ��� �� ���� �Һ��Ͽ� Lobby�� ���� �������� Level�� �ߺ� ��ȯ���� �ʰ� �Ѵ�.
	if (!m_hasPendingEnterAccepted)
		return false;

	message = m_PendingEnterAccepted;

	m_hasPendingEnterAccepted = false;

	return true;
}

bool CNetworkManager::Try_Consume_EnterRejected(
	LostArk::Shared::S2C_ENTER_REJECTED& message)
{
	if (!m_hasPendingEnterRejected)
		return false;

	message = m_PendingEnterRejected;
	m_hasPendingEnterRejected = false;
	return true;
}

bool CNetworkManager::Try_Consume_WorldEntitySpawnResult(
	LostArk::Shared::S2C_WORLD_ENTITY_SPAWN_RESULT& message)
{
	if (m_WorldEntitySpawnResults.empty())
		return false;
	message = std::move(m_WorldEntitySpawnResults.front());
	m_WorldEntitySpawnResults.pop_front();
	return true;
}

bool CNetworkManager::Try_Consume_CharacterClassChangeResult(
	LostArk::Shared::S2C_CHARACTER_CLASS_CHANGE_RESULT& message)
{
	if (m_CharacterClassChangeResults.empty())
		return false;
	message = std::move(m_CharacterClassChangeResults.front());
	m_CharacterClassChangeResults.pop_front();
	return true;
}

bool CNetworkManager::Try_Consume_ValtanAuditionResult(
	LostArk::Shared::S2C_VALTAN_AUDITION_RESULT& message)
{
	if (m_ValtanAuditionResults.empty())
		return false;
	message = m_ValtanAuditionResults.front();
	m_ValtanAuditionResults.pop_front();
	return true;
}

bool CNetworkManager::Try_Consume_ValtanPatternAuditionByIdResult(
	LostArk::Shared::S2C_VALTAN_AUDITION_RESULT& message)
{
	if (m_ValtanPatternAuditionByIdResults.empty())
		return false;
	message = std::move(m_ValtanPatternAuditionByIdResults.front());
	m_ValtanPatternAuditionByIdResults.pop_front();
	return true;
}

bool CNetworkManager::Try_Consume_ValtanAuditionLifecycle(
	LostArk::Shared::S2C_VALTAN_AUDITION_LIFECYCLE& message)
{
	if (m_ValtanAuditionLifecycleEvents.empty())
		return false;
	message = std::move(m_ValtanAuditionLifecycleEvents.front());
	m_ValtanAuditionLifecycleEvents.pop_front();
	return true;
}

bool CNetworkManager::Try_Consume_ValtanPatternFlowResult(
	LostArk::Shared::S2C_DEBUG_VALTAN_PATTERN_FLOW_RESULT& message)
{
	if (m_ValtanPatternFlowResults.empty())
		return false;
	message = std::move(m_ValtanPatternFlowResults.front());
	m_ValtanPatternFlowResults.pop_front();
	return true;
}

bool CNetworkManager::Try_Consume_ValtanPatternFlowLifecycle(
	LostArk::Shared::S2C_DEBUG_VALTAN_PATTERN_FLOW_LIFECYCLE& message)
{
	if (m_ValtanPatternFlowLifecycleEvents.empty())
		return false;
	message = std::move(m_ValtanPatternFlowLifecycleEvents.front());
	m_ValtanPatternFlowLifecycleEvents.pop_front();
	return true;
}

bool CNetworkManager::Try_Get_LatestValtanDecisionTrace(
	LostArk::Shared::GameplayDataRevision& outDefinitionRevision,
	LostArk::Shared::VALTAN_DECISION_TRACE_WIRE& outTrace) const
{
	if (!m_ValtanDecisionTraceState.hasLatestTrace)
		return false;
	outDefinitionRevision =
		m_ValtanDecisionTraceState.LatestDefinitionRevision;
	outTrace = m_ValtanDecisionTraceState.LatestTrace;
	return true;
}

bool CNetworkManager::Try_Consume_ReplicationEvent(Client::CLIENT_REPLICATION_EVENT& event)
{
	if (m_ReplicationEvents.empty())
	{
		return false;
	}

	event = std::move(m_ReplicationEvents.front());
	m_ReplicationEvents.pop_front();
	m_SessionDiagnostic.Record_EventQueueDepth(m_ReplicationEvents.size());
	return true;
}

bool CNetworkManager::Enqueue_ReplicationEvent(
	Client::CLIENT_REPLICATION_EVENT&& event)
{
	/* Loading levels do not own a CClientReplication consumer yet, while the
	   admitted Server room continues to publish WORLD_SNAPSHOT at 30 Hz. Keep
	   only the newest adjacent snapshot: spawn/despawn/destruction events stay
	   as ordering barriers, but a cold level load can no longer fill the queue
	   and disconnect with WSAENOBUFS before activation. */
	if (!m_ReplicationEvents.empty() &&
		Client::Can_CoalesceAdjacentReplicationEvents(
			m_ReplicationEvents.back().eType, event.eType))
	{
		m_ReplicationEvents.back() = std::move(event);
		m_SessionDiagnostic.Record_EventQueueDepth(m_ReplicationEvents.size());
		return true;
	}
	if (m_ReplicationEvents.size() >= MAX_REPLICATION_EVENT_QUEUE)
	{
		Fail_Protocol(
			WSAENOBUFS,
			LostArk::Shared::SESSION_DIAGNOSTIC_REASON::
				CLIENT_EVENT_QUEUE_OVERFLOW,
			LostArk::Shared::PACKET_TYPE::INVALID,
			"Replication event queue reached its 4096-event bound.");
		return false;
	}
	m_ReplicationEvents.push_back(std::move(event));
	m_SessionDiagnostic.Record_EventQueueDepth(m_ReplicationEvents.size());
	return true;
}

void CNetworkManager::Fail_Protocol(
	const int errorCode,
	const LostArk::Shared::SESSION_DIAGNOSTIC_REASON reason,
	const LostArk::Shared::PACKET_TYPE triggeringPacket,
	const std::string_view detail)
{
	m_iLastErrorCode.store(errorCode);
	m_SessionDiagnostic.Record_Terminal(
		reason, errorCode, triggeringPacket, detail);
	m_hasProtocolFailure.store(true);
	m_isReceiveRunning.store(false);
	const SOCKET socketToClose = m_hServerSocket;
	m_hServerSocket = INVALID_SOCKET;
	if (INVALID_SOCKET != socketToClose)
	{
		::shutdown(socketToClose, SD_BOTH);
		::closesocket(socketToClose);
	}
	if (m_ReceiveThread.joinable() &&
		m_ReceiveThread.get_id() != std::this_thread::get_id())
	{
		m_ReceiveThread.join();
	}
	{
		std::scoped_lock lock{ m_InboundMutex };
		m_InboundFrames.clear();
		m_SessionDiagnostic.Record_RawQueueDepth(0u);
	}
	m_StreamParser.Reset();
	Reset_WorldInboundState();
}

void CNetworkManager::Reset_WorldInboundState()
{
	m_iWorldInboundGeneration =
		(std::numeric_limits<std::uint64_t>::max)() == m_iWorldInboundGeneration ?
			1u : m_iWorldInboundGeneration + 1u;
	m_ReplicationEvents.clear();
	m_SessionDiagnostic.Record_EventQueueDepth(0u);
	m_WorldEntitySpawnResults.clear();
	m_CharacterClassChangeResults.clear();
	m_ValtanAuditionResults.clear();
	m_ValtanPatternAuditionByIdResults.clear();
	m_ValtanAuditionLifecycleEvents.clear();
	m_ValtanPatternFlowResults.clear();
	m_ValtanPatternFlowLifecycleEvents.clear();
	m_pStagedPresentationAdmission.reset();
	m_GameplayRevisionState = {};
	m_ValtanDecisionTraceState = {};
	m_hasPendingEnterAccepted = false;
	m_PendingEnterAccepted = {};
	m_hasPendingEnterRejected = false;
	m_PendingEnterRejected = {};
	m_iLocalPlayerId = LostArk::Shared::INVALID_PLAYER_ID;
	m_iLocalNetEntityId = LostArk::Shared::INVALID_NET_ENTITY_ID;
	m_eWorldId = LostArk::Shared::WORLD_ID::END;
	m_eLocalCharacterClass = LostArk::Shared::CHARACTER_CLASS_ID::END;
	m_hasLocalSpawn = false;
	m_LocalSpawn = {};
}

void CNetworkManager::Record_WorldRevisionSet(
	const LostArk::Shared::GameplayDataRevision& activeRevision,
	const std::vector<LostArk::Shared::GameplayDataRevision>&
		requiredPinnedRevisions)
{
	m_GameplayRevisionState.ServerActiveRevision = activeRevision;
	m_GameplayRevisionState.RequiredPinnedRevisions =
		requiredPinnedRevisions;
	Prune_PresentationAliases();
}

void CNetworkManager::Prune_PresentationAliases()
{
	const auto isRetained = [this](
		const LostArk::Shared::GameplayDataRevision& revision)
	{
		if (revision == m_GameplayRevisionState.ServerActiveRevision ||
			(m_GameplayRevisionState.hasStagedPresentationAlias &&
			 revision == m_GameplayRevisionState.StagedPresentationAlias))
		{
			return true;
		}
		return m_GameplayRevisionState.RequiredPinnedRevisions.end() !=
			std::find(
				m_GameplayRevisionState.RequiredPinnedRevisions.begin(),
				m_GameplayRevisionState.RequiredPinnedRevisions.end(),
				revision);
	};
	auto& aliases = m_GameplayRevisionState.AvailablePresentationAliases;
	aliases.erase(
		std::remove_if(
			aliases.begin(), aliases.end(),
			[&isRetained](
				const LostArk::Shared::GameplayDataRevision& revision)
			{
				return !isRetained(revision);
			}),
		aliases.end());
	auto& receipts =
		m_GameplayRevisionState.AvailablePresentationReceipts;
	receipts.erase(
		std::remove_if(
			receipts.begin(), receipts.end(),
			[&isRetained](
				const Client::VALTAN_PRESENTATION_GENERATION_RECEIPT& receipt)
			{
				return !isRetained(receipt.ServerGameplayRevision);
			}),
		receipts.end());
}

bool CNetworkManager::Is_AnnouncedWorldRevision(
	const LostArk::Shared::GameplayDataRevision& revision) const
{
	if (revision == m_GameplayRevisionState.ServerActiveRevision)
		return true;
	for (const LostArk::Shared::GameplayDataRevision& required :
		m_GameplayRevisionState.RequiredPinnedRevisions)
	{
		if (revision == required)
			return true;
	}
	return false;
}

bool CNetworkManager::Is_PresentationRevisionAvailable(
	const LostArk::Shared::GameplayDataRevision& revision) const
{
	if (!revision.Is_Valid())
		return false;
	if (m_GameplayRevisionState.hasBootstrapPresentationRevision &&
		revision == m_GameplayRevisionState.BootstrapPresentationRevision)
	{
		return true;
	}
	return std::find(
		m_GameplayRevisionState.AvailablePresentationAliases.begin(),
		m_GameplayRevisionState.AvailablePresentationAliases.end(),
		revision) !=
		m_GameplayRevisionState.AvailablePresentationAliases.end();
}

bool CNetworkManager::Try_Get_ValtanPresentationGenerationReceipt(
	const LostArk::Shared::GameplayDataRevision& revision,
	Client::VALTAN_PRESENTATION_GENERATION_RECEIPT& outReceipt,
	std::string& status) const
{
	if (!Is_PresentationRevisionAvailable(revision) ||
		!m_GameplayRevisionState.BootstrapPresentationReceipt.Is_Valid())
	{
		status =
			"No admitted Valtan presentation generation receipt exists for the requested Server revision.";
		return false;
	}
	if (m_GameplayRevisionState.hasBootstrapPresentationRevision &&
		revision == m_GameplayRevisionState.BootstrapPresentationRevision)
	{
		outReceipt = m_GameplayRevisionState.BootstrapPresentationReceipt;
		status = "Resolved the Valtan presentation receipt captured at world entry.";
		return true;
	}
	const auto found = std::find_if(
		m_GameplayRevisionState.AvailablePresentationReceipts.begin(),
		m_GameplayRevisionState.AvailablePresentationReceipts.end(),
		[&revision](
			const Client::VALTAN_PRESENTATION_GENERATION_RECEIPT& receipt)
		{
			return receipt.ServerGameplayRevision == revision;
		});
	if (m_GameplayRevisionState.AvailablePresentationReceipts.end() != found)
	{
		outReceipt = *found;
		status = "Resolved the exact saved Valtan presentation generation receipt.";
		return true;
	}
	/* Required pinned revisions announced during world entry share the one
	   validated entry closure. They predate Client-side transaction receipts. */
	auto entryReceipt = m_GameplayRevisionState.BootstrapPresentationReceipt;
	entryReceipt.ServerGameplayRevision = revision;
	outReceipt = std::move(entryReceipt);
	status = "Resolved the validated world-entry Valtan presentation receipt.";
	return true;
}

bool CNetworkManager::Is_CurrentPresentationBaselineIntact(
	std::string& status) const
{
	if (!m_GameplayRevisionState.hasPresentationArtifactBaseline ||
		!m_GameplayRevisionState.BootstrapPresentationReceipt.Is_Valid())
	{
		status = "No validated world-entry presentation source receipt is available.";
		return false;
	}
	Client::CValtanPresentationGenerationReadAdmission admission;
	return admission.Acquire_Receipt(
		m_GameplayRevisionState.BootstrapPresentationReceipt.
			ServerGameplayRevision,
		m_GameplayRevisionState.BootstrapPresentationReceipt,
		status) && admission.Validate_StillCurrent(status);
}

CNetworkManager::PRESENTATION_CANDIDATE_PREFLIGHT_RESULT
CNetworkManager::Preflight_PresentationCandidate(
	const LostArk::Shared::GameplayDataRevision& candidateRevision,
	const std::uint32_t requiredPresentationLaneMask,
	std::string& status) const
{
#if !defined(_DEBUG)
	(void)candidateRevision;
	(void)requiredPresentationLaneMask;
	status = "Release Client does not initiate presentation revision transactions.";
	return PRESENTATION_CANDIDATE_PREFLIGHT_RESULT::REJECTED;
#else
	std::vector<PRESENTATION_ARTIFACT_BASELINE> currentArtifacts;
	Client::VALTAN_PRESENTATION_GENERATION_RECEIPT currentReceipt;
	if (!CapturePresentationArtifactBaseline(
			currentArtifacts, currentReceipt, status))
	{
		return PRESENTATION_CANDIDATE_PREFLIGHT_RESULT::REJECTED;
	}
	if (ValidateCurrentCandidatePresentationGeneration(
			candidateRevision, requiredPresentationLaneMask,
			currentReceipt.PresentationGenerationId,
			currentArtifacts, status))
	{
		return PRESENTATION_CANDIDATE_PREFLIGHT_RESULT::
			CURRENT_GENERATION_READY;
	}
	return PRESENTATION_CANDIDATE_PREFLIGHT_RESULT::REJECTED;
#endif
}

bool CNetworkManager::Stage_ByteIdenticalPresentationAlias(
	const LostArk::Shared::S2C_DATA_REVISION_PREPARE& prepare,
	std::string& status)
{
	using namespace LostArk::Shared;
	if (m_GameplayRevisionState.hasStagedPresentationAlias)
	{
		const bool isExactRetransmit =
			m_GameplayRevisionState.iStagedPresentationTransactionSequence ==
				prepare.iTransactionSequence &&
			m_GameplayRevisionState.StagedPresentationAlias ==
				prepare.CandidateRevision &&
			m_GameplayRevisionState.iStagedPresentationLaneMask ==
				prepare.iRequiredPresentationLaneMask &&
			m_GameplayRevisionState.ServerActiveRevision == prepare.BaseRevision;
		if (isExactRetransmit)
		{
			/* TCP does not require application retransmission, but accepting the
			   byte-identical transaction is idempotent and lets the Server recover
			   from a duplicated dispatch without re-reading candidate artifacts. */
			if (nullptr == m_pStagedPresentationAdmission ||
				!m_pStagedPresentationAdmission->Validate_StillCurrent(status))
			{
				return false;
			}
			status = "Exact revision prepare retransmit is already staged.";
			return true;
		}

		status =
			"Overlapping or stale revision prepare was rejected; the current "
			"staged alias remains intact.";
		return false;
	}
	if (!m_GameplayRevisionState.ServerActiveRevision.Is_Valid() ||
		prepare.BaseRevision != m_GameplayRevisionState.ServerActiveRevision)
	{
		status = "Revision prepare base does not match the announced active revision.";
		return false;
	}
	if (!prepare.CandidateRevision.Is_Valid() ||
		prepare.CandidateRevision == prepare.BaseRevision)
	{
		status = "Revision prepare candidate is invalid or already active.";
		return false;
	}
	/* RequiredPinnedRevisions is refreshed by every world snapshot.  Retain only
	   the active generation, live occurrence pins, and an in-flight stage before
	   applying the hard generation bound; obsolete aliases must not make the
	   seventeenth sequential tuning transaction fail forever. */
	Prune_PresentationAliases();
	if (m_GameplayRevisionState.AvailablePresentationAliases.size() >=
			MAX_PRESENTATION_ALIAS_GENERATIONS &&
		!Is_PresentationRevisionAvailable(prepare.CandidateRevision))
	{
		status = "Presentation alias generation bound is exhausted.";
		return false;
	}
	auto stagedAdmission = std::make_unique<
		Client::CValtanPresentationGenerationReadAdmission>();
	Client::VALTAN_PRESENTATION_GENERATION_RECEIPT currentReceipt;
#if defined(_DEBUG)
	if (nullptr == stagedAdmission ||
		!stagedAdmission->Acquire_PackagedBaseline(currentReceipt, status) ||
		!stagedAdmission->Validate_StillCurrent(status))
	{
		return false;
	}
	std::vector<PRESENTATION_ARTIFACT_BASELINE> currentArtifacts;
	currentArtifacts.reserve(currentReceipt.Artifacts.size());
	for (const auto& artifact : currentReceipt.Artifacts)
	{
		PRESENTATION_ARTIFACT_BASELINE row;
		row.strRelativePath = artifact.strRelativePath;
		row.strLane = artifact.strLane;
		row.strSha256 = Format_GameplayDataRevision(artifact.Revision);
		row.iBytes = artifact.iBytes;
		currentArtifacts.push_back(std::move(row));
	}
	if (!ValidateCurrentCandidatePresentationGeneration(
			prepare.CandidateRevision,
			prepare.iRequiredPresentationLaneMask,
			currentReceipt.PresentationGenerationId,
			currentArtifacts,
			status))
	{
		return false;
	}
#else
	status = "Release Client rejects gameplay presentation revision staging.";
	return false;
#endif
	m_GameplayRevisionState.hasStagedPresentationAlias = true;
	m_GameplayRevisionState.StagedPresentationAlias = prepare.CandidateRevision;
	m_GameplayRevisionState.iStagedPresentationTransactionSequence =
		prepare.iTransactionSequence;
	m_GameplayRevisionState.iStagedPresentationLaneMask =
		prepare.iRequiredPresentationLaneMask;
	currentReceipt.ServerGameplayRevision = prepare.CandidateRevision;
	m_GameplayRevisionState.StagedPresentationReceipt =
		std::move(currentReceipt);
	m_pStagedPresentationAdmission = std::move(stagedAdmission);
	return true;
}

bool CNetworkManager::Commit_StagedPresentationAlias(
	const LostArk::Shared::S2C_DATA_REVISION_RESULT& result,
	std::string& status)
{
	using namespace LostArk::Shared;
	if (DATA_REVISION_RESULT::ABORTED == result.eResult)
	{
		const bool matchesStaged =
			m_GameplayRevisionState.hasStagedPresentationAlias &&
			m_GameplayRevisionState.iStagedPresentationTransactionSequence ==
				result.iTransactionSequence &&
			m_GameplayRevisionState.StagedPresentationAlias ==
				result.CandidateRevision;
		const bool matchesOutstanding =
			m_GameplayRevisionState.hasOutstandingPrepareRequest &&
			m_GameplayRevisionState.iOutstandingPrepareRequestSequence ==
				result.iTransactionSequence &&
			m_GameplayRevisionState.OutstandingPrepareCandidateRevision ==
				result.CandidateRevision;
		const bool matchesRejectedPrepare =
			m_GameplayRevisionState.hasRejectedPrepareAwaitingAbort &&
			m_GameplayRevisionState.iRejectedPrepareTransactionSequence ==
				result.iTransactionSequence &&
			m_GameplayRevisionState.RejectedPrepareBaseRevision ==
				result.ActiveRevision &&
			m_GameplayRevisionState.RejectedPrepareCandidateRevision ==
				result.CandidateRevision;
		if ((!matchesStaged && !matchesOutstanding && !matchesRejectedPrepare) ||
			result.ActiveRevision !=
				m_GameplayRevisionState.ServerActiveRevision)
		{
			status =
				"Stale revision ABORT did not exactly match the local transaction and active generation.";
			return false;
		}
		if (matchesStaged)
			Discard_StagedPresentationAlias();
		if (matchesOutstanding)
		{
			m_GameplayRevisionState.hasOutstandingPrepareRequest = false;
			m_GameplayRevisionState.iOutstandingPrepareRequestSequence = 0u;
			m_GameplayRevisionState.OutstandingPrepareCandidateRevision = {};
		}
		if (matchesRejectedPrepare)
		{
			m_GameplayRevisionState.hasRejectedPrepareAwaitingAbort = false;
			m_GameplayRevisionState.iRejectedPrepareTransactionSequence = 0u;
			m_GameplayRevisionState.RejectedPrepareBaseRevision = {};
			m_GameplayRevisionState.RejectedPrepareCandidateRevision = {};
		}
		status = result.strReason;
		return true;
	}
	const bool matchesOutstanding =
		m_GameplayRevisionState.hasOutstandingPrepareRequest &&
		m_GameplayRevisionState.iOutstandingPrepareRequestSequence ==
			result.iTransactionSequence &&
		m_GameplayRevisionState.OutstandingPrepareCandidateRevision ==
			result.CandidateRevision;
	const bool isAlreadyActiveIdempotentCommit =
		matchesOutstanding &&
		result.ActiveRevision == result.CandidateRevision &&
		result.ActiveRevision ==
			m_GameplayRevisionState.ServerActiveRevision &&
		Is_PresentationRevisionAvailable(result.CandidateRevision);
	if (!isAlreadyActiveIdempotentCommit &&
		(!m_GameplayRevisionState.hasStagedPresentationAlias ||
		m_GameplayRevisionState.iStagedPresentationTransactionSequence !=
			result.iTransactionSequence ||
		m_GameplayRevisionState.StagedPresentationAlias !=
			result.CandidateRevision ||
		result.ActiveRevision != result.CandidateRevision))
	{
		status = "Committed revision has no matching prepared presentation alias.";
		return false;
	}
	if (!isAlreadyActiveIdempotentCommit &&
		(nullptr == m_pStagedPresentationAdmission ||
		 !m_GameplayRevisionState.StagedPresentationReceipt.Is_Valid() ||
		 m_GameplayRevisionState.StagedPresentationReceipt.
			 ServerGameplayRevision != result.CandidateRevision ||
		 !m_pStagedPresentationAdmission->Validate_StillCurrent(status)))
	{
		if (status.empty())
			status = "Prepared presentation generation is no longer current.";
		return false;
	}
	if (!isAlreadyActiveIdempotentCommit &&
		!Is_PresentationRevisionAvailable(result.CandidateRevision))
	{
		if (m_GameplayRevisionState.AvailablePresentationAliases.size() >=
				MAX_PRESENTATION_ALIAS_GENERATIONS ||
			m_GameplayRevisionState.AvailablePresentationReceipts.size() >=
				MAX_PRESENTATION_ALIAS_GENERATIONS)
		{
			status = "Presentation generation bound was exceeded at commit.";
			return false;
		}
		m_GameplayRevisionState.AvailablePresentationAliases.push_back(
			result.CandidateRevision);
		m_GameplayRevisionState.AvailablePresentationReceipts.push_back(
			m_GameplayRevisionState.StagedPresentationReceipt);
	}
	if (matchesOutstanding)
	{
		m_GameplayRevisionState.hasOutstandingPrepareRequest = false;
		m_GameplayRevisionState.iOutstandingPrepareRequestSequence = 0u;
		m_GameplayRevisionState.OutstandingPrepareCandidateRevision = {};
	}
	if (!isAlreadyActiveIdempotentCommit)
		Discard_StagedPresentationAlias();
	status = result.strReason;
	return true;
}

void CNetworkManager::Discard_StagedPresentationAlias() noexcept
{
	m_GameplayRevisionState.hasStagedPresentationAlias = false;
	m_GameplayRevisionState.StagedPresentationAlias = {};
	m_GameplayRevisionState.iStagedPresentationTransactionSequence = 0u;
	m_GameplayRevisionState.iStagedPresentationLaneMask = 0u;
	m_GameplayRevisionState.StagedPresentationReceipt = {};
	m_pStagedPresentationAdmission.reset();
}

void CNetworkManager::Record_PresentationIsolation(
	const LostArk::Shared::GameplayDataRevision& revision,
	const std::string_view context)
{
	m_GameplayRevisionState.isPresentationIsolated = true;
	if (!m_GameplayRevisionState.strIsolationReason.empty())
		return;
	std::string revisionText =
		LostArk::Shared::Format_GameplayDataRevision(revision);
	m_GameplayRevisionState.strIsolationReason =
		std::string(context) + " requires unavailable presentation revision " +
		(revisionText.empty() ? std::string("INVALID") : revisionText) +
		"; the revision-dependent lane was isolated.";
}

void CNetworkManager::Close_ServerConnection()
{
	if (0u != m_SessionDiagnostic.Get_Snapshot().iConnectionGeneration)
		m_SessionDiagnostic.Record_Event("connection.close-requested");
	m_isReceiveRunning.store(false);
	const SOCKET socketToClose = m_hServerSocket;
	m_hServerSocket = INVALID_SOCKET;

	if (INVALID_SOCKET != socketToClose)
	{
		::shutdown(socketToClose, SD_BOTH);
		::closesocket(socketToClose);
	}

	if (m_ReceiveThread.joinable())
		m_ReceiveThread.join();

	{
		std::scoped_lock lock{ m_InboundMutex };
		m_InboundFrames.clear();
		m_SessionDiagnostic.Record_RawQueueDepth(0u);
	}

	m_StreamParser.Reset();
	Reset_WorldInboundState();
	m_hasProtocolFailure.store(false);
}

bool CNetworkManager::Is_Connected() const
{
	return
		INVALID_SOCKET != m_hServerSocket &&
		m_isReceiveRunning.load();
}

int CNetworkManager::Get_LastErrorCode() const
{
	return m_iLastErrorCode.load();
}

void CNetworkManager::Record_SessionEvent(
	const std::string_view eventName,
	const std::string_view detail)
{
	m_SessionDiagnostic.Record_Event(eventName, detail);
}

void CNetworkManager::Record_SessionRecovery(
	const LostArk::Shared::SESSION_DIAGNOSTIC_REASON reason,
	const std::string_view source,
	const std::string_view detail)
{
	m_SessionDiagnostic.Record_Recovery(reason, source, detail);
}

bool CNetworkManager::Record_SessionTerminal(
	const LostArk::Shared::SESSION_DIAGNOSTIC_REASON reason,
	const int wsaError,
	const LostArk::Shared::PACKET_TYPE triggeringPacket,
	const std::string_view detail)
{
	return m_SessionDiagnostic.Record_Terminal(
		reason, wsaError, triggeringPacket, detail);
}

LostArk::Shared::PLAYER_ID CNetworkManager::Get_LocalPlayerId() const
{
	return m_iLocalPlayerId;
}

LostArk::Shared::NET_ENTITY_ID CNetworkManager::Get_LocalEntityId() const
{
	return m_iLocalNetEntityId;
}

LostArk::Shared::CHARACTER_CLASS_ID
CNetworkManager::Get_LocalCharacterClass() const
{
	return m_eLocalCharacterClass;
}

bool CNetworkManager::Try_Get_LocalSpawn(
	LostArk::Shared::S2C_PLAYER_SPAWNED& outSpawn) const
{
	if (!m_hasLocalSpawn)
		return false;

	outSpawn = m_LocalSpawn;
	return true;
}

void CNetworkManager::Receive_Loop(const SOCKET serverSocket)
{
	//recv()�� server�� ���� ����Ʈ�� �޴´�
	//���� ����Ʈ�� PacketStreamParser�� �߰��Ѵ�.
	//parser���� �ϼ��� �������� �����Ѹ�ŭ ������.
	//�ϼ��� �������� inbound queue�� �ִ´�.
	using namespace LostArk::Shared;

	std::array<std::uint8_t, 4096> receiveBuffer{};

	// Main Thread�� Connect/Close�� ���� �ٲٰ� Receive Worker�� �ݺ� �������� �д´�.
	while (m_isReceiveRunning.load())
	{
		//serversocket�� �ִ� data recv�� �б�
		const int receiveByteCount = ::recv(
			serverSocket,
			reinterpret_cast<char*>(
				receiveBuffer.data()),
			static_cast<int>(
				receiveBuffer.size()),
			0);
		//ByteCount�� ���ؼ� ���� �� ���� ���� �Ǵ�

		//��밡 ���������� ������ �����޴�.
		if (0 == receiveByteCount)
		{
			if (m_isReceiveRunning.load())
			{
				m_SessionDiagnostic.Record_Terminal(
					SESSION_DIAGNOSTIC_REASON::CLIENT_PEER_CLOSED,
					0,
					PACKET_TYPE::INVALID,
					"Server completed an orderly TCP close (FIN).");
			}
			break;
		}

		//socket I/O ���� �Ǵ� shutdown���� recv�� �����ƴ�.
		if (SOCKET_ERROR == receiveByteCount)
		{
			const int errorCode = ::WSAGetLastError();

			//����ڰ� ������ ����� ������ ���� ��� ������ ��� X
			if (m_isReceiveRunning.load())
			{
				m_iLastErrorCode.store(errorCode);
				m_SessionDiagnostic.Record_Terminal(
					SESSION_DIAGNOSTIC_REASON::CLIENT_RECEIVE_ERROR,
					errorCode,
					PACKET_TYPE::INVALID,
					"recv() failed while the connection was active.");
			}

			break;
		}

		// recv ����� Header�� Payload ��踦 �������� �ʴ� TCP ����Ʈ �����̴�.
		// Parser�� ���� recv ������ �����Ͽ� �ϼ��� Frame���� �����Ѵ�.
		const std::span<const std::uint8_t> receiveBytes
		{
			receiveBuffer.data(), static_cast<std::size_t>(receiveByteCount)
		};
		//TCP���� ���� ����Ʈ�� Parser�� ���� ���ۿ� ���δ�.
		if (!m_StreamParser.Append(receiveBytes))
		{
			m_iLastErrorCode.store(WSAEMSGSIZE);
			m_SessionDiagnostic.Record_Terminal(
				SESSION_DIAGNOSTIC_REASON::CLIENT_PARSER_OVERFLOW,
				WSAEMSGSIZE,
				PACKET_TYPE::INVALID,
				"TCP parser buffered-byte bound was exceeded.");
			break;
		}
		//�̹� recv�� �ϼ��� �������� ���� �� ������ �� �ִ�. ;; ���� ���� ���鼭 �ľ�
		for (;;)
		{
			// PACKET_FRAME�� Header���� ������ PacketType�� Payload�� ���� �ǹ� ������.
			PACKET_FRAME frame{};

			const PACKET_PARSE_RESULT parseResult = m_StreamParser.Try_Pop(frame);

			if (PACKET_PARSE_RESULT::NEED_MORE_DATA == parseResult)
			{
				//���� ������ �ϳ��� �ϼ����� �ʾұ� ������, ���� ����� ��ٸ���.
				break;
			}
			if (PACKET_PARSE_RESULT::INVALID_FRAME == parseResult)
			{
				//�߸��� ũ�� �Ǵ� ��Ŷ Ÿ���� �߰߉Ѵ�.
				m_iLastErrorCode.store(WSAEPROTONOSUPPORT);
				m_SessionDiagnostic.Record_Terminal(
					SESSION_DIAGNOSTIC_REASON::CLIENT_INVALID_FRAME,
					WSAEPROTONOSUPPORT,
					PACKET_TYPE::INVALID,
					"TCP parser rejected an invalid frame header or packet type.");
				
				m_isReceiveRunning.store(false);
				return;
			}
			//FRAME_READY�� ��쿡�� main thread ���� ť�� �ִ´�.
			{
				std::scoped_lock lock{
				   m_InboundMutex
				};
				/* Level activation can synchronously prepare GPU resources before the
				   main thread resumes Update().  Coalesce adjacent snapshots at the
				   worker boundary as well as the parsed-event boundary so that cold
				   loading cannot exhaust the raw frame queue.  Any lifecycle or
				   destruction frame remains an ordering barrier. */
				if (!m_InboundFrames.empty() &&
					Client::Can_CoalesceAdjacentInboundFrames(
						m_InboundFrames.back().ePacketType,
						frame.ePacketType))
				{
					m_InboundFrames.back() = std::move(frame);
					m_SessionDiagnostic.Record_InboundFrame(
						m_InboundFrames.back().ePacketType,
						m_InboundFrames.size());
					continue;
				}
				if (m_InboundFrames.size() >= MAX_INBOUND_FRAME_QUEUE)
				{
					m_iLastErrorCode.store(WSAENOBUFS);
					m_SessionDiagnostic.Record_Terminal(
						SESSION_DIAGNOSTIC_REASON::CLIENT_RAW_QUEUE_OVERFLOW,
						WSAENOBUFS,
						frame.ePacketType,
						"Inbound raw frame queue reached its 4096-frame bound.");
					m_hasProtocolFailure.store(true);
					m_isReceiveRunning.store(false);
					return;
				}
				m_InboundFrames.push_back(
					std::move(frame));
				m_SessionDiagnostic.Record_InboundFrame(
					m_InboundFrames.back().ePacketType,
					m_InboundFrames.size());
			}
		}
	}
	m_isReceiveRunning.store(false);
}

void CNetworkManager::Handle_Frame(const LostArk::Shared::PACKET_FRAME & frame)
{
	if (m_hasProtocolFailure.load())
		return;

	using namespace LostArk::Shared;

	//frame�� payload ������ �д´�. packet - ������ ��� header�� payload - class,strName �̷��� 2���� ������
	CPacketReader reader{ frame.Payload };

	switch (frame.ePacketType)
	{
	//Server Enter
	case PACKET_TYPE::S2C_ENTER_ACCEPTED:
	{
		S2C_ENTER_ACCEPTED accepted{};

		if (!Read_Message(reader, accepted) ||
			0 != reader.Get_RemainingSize())
		{
			Fail_Protocol(
				WSAEINVAL,
				SESSION_DIAGNOSTIC_REASON::CLIENT_MESSAGE_DECODE_FAILED,
				PACKET_TYPE::S2C_ENTER_ACCEPTED,
				"S2C_ENTER_ACCEPTED payload decode or trailing-byte validation failed.");
			return;
		}

		/* Validate and stage the current typed Client presentation sources before
		   the accepted world becomes observable. Server gameplay revisions remain
		   authoritative CAS identities, but stale world-entry presentation hashes,
		   byte counts, generation IDs, and inventories do not relabel or reject the
		   current typed Client closure. */
		bool stagedHasBootstrapPresentationRevision = false;
		GameplayDataRevision stagedBootstrapPresentationRevision{};
		std::vector<GameplayDataRevision> stagedPresentationAliases;
		std::vector<CNetworkManager::PRESENTATION_ARTIFACT_BASELINE>
			stagedPresentationArtifactBaseline;
		Client::VALTAN_PRESENTATION_GENERATION_RECEIPT
			stagedPresentationReceipt;
		std::string baselineStatus;
		const bool hasPresentationArtifactBaseline =
			CapturePresentationArtifactBaseline(
				stagedPresentationArtifactBaseline,
				stagedPresentationReceipt, baselineStatus);
		if (!hasPresentationArtifactBaseline)
		{
			/* Presentation source skew is not a gameplay protocol violation. Keep
			   the validated Server admission, isolate presentation revision
			   transactions, and leave an actionable reload warning. Packet decode
			   and typed gameplay revision validation below remain fail-closed. */
			stagedPresentationArtifactBaseline.clear();
			stagedPresentationReceipt = {};
			m_SessionDiagnostic.Record_Event(
				"presentation.baseline-unavailable",
				baselineStatus.empty() ?
					"Client presentation sources could not be validated; gameplay entry continues and presentation reload is required." :
					baselineStatus);
		}
		const auto admitEntryRevision = [
			&stagedHasBootstrapPresentationRevision,
			&stagedBootstrapPresentationRevision,
			&stagedPresentationAliases](
			const GameplayDataRevision& revision,
			const bool_t bPrimaryRevision,
			std::string& failure)
		{
			if (!revision.Is_Valid())
			{
				failure = "World entry announced an invalid gameplay revision.";
				return false;
			}
			if ((stagedHasBootstrapPresentationRevision &&
				 revision == stagedBootstrapPresentationRevision) ||
				stagedPresentationAliases.end() != std::find(
					stagedPresentationAliases.begin(),
					stagedPresentationAliases.end(), revision))
			{
				return true;
			}
			if (bPrimaryRevision)
			{
				if (stagedHasBootstrapPresentationRevision &&
					stagedBootstrapPresentationRevision != revision)
				{
					failure = "World entry declared conflicting bootstrap revisions.";
					return false;
				}
				stagedHasBootstrapPresentationRevision = true;
				stagedBootstrapPresentationRevision = revision;
				return true;
			}
			if (stagedPresentationAliases.size() >=
				MAX_PRESENTATION_ALIAS_GENERATIONS)
			{
				failure = "World entry presentation alias bound was exceeded.";
				return false;
			}
			stagedPresentationAliases.push_back(revision);
			return true;
		};
		std::string entryAdmissionFailure;
		if (!admitEntryRevision(
				accepted.ActiveGameplayRevision,
				true, entryAdmissionFailure))
		{
			Fail_Protocol(
				WSAEINVAL,
				SESSION_DIAGNOSTIC_REASON::
					CLIENT_ENTRY_PRESENTATION_REVISION_FAILED,
				PACKET_TYPE::S2C_ENTER_ACCEPTED,
				entryAdmissionFailure);
			return;
		}
		for (const GameplayDataRevision& required :
			accepted.RequiredPinnedGameplayRevisions)
		{
			if (!admitEntryRevision(
					required, false, entryAdmissionFailure))
			{
				Fail_Protocol(
					WSAEINVAL,
					SESSION_DIAGNOSTIC_REASON::
						CLIENT_ENTRY_PRESENTATION_REVISION_FAILED,
					PACKET_TYPE::S2C_ENTER_ACCEPTED,
					entryAdmissionFailure);
				return;
			}
		}
		if (hasPresentationArtifactBaseline)
		{
			stagedPresentationReceipt.ServerGameplayRevision =
				accepted.ActiveGameplayRevision;
		}
		// Acceptance is the generation boundary. It intentionally drops every
		// queued event that may have arrived for the previous room while the
		// loading transition was pending. The requested class belongs to this
		// new generation, so preserve it for the target Level loader.
		const CHARACTER_CLASS_ID requestedCharacterClass =
			m_eLocalCharacterClass;
		Reset_WorldInboundState();
		m_eLocalCharacterClass = requestedCharacterClass;
		m_iLocalPlayerId = accepted.iPlayerId;
		m_iLocalNetEntityId = accepted.iNetEntityId;
		m_eWorldId = accepted.eWorldId;
		m_GameplayRevisionState.ServerActiveRevision =
			accepted.ActiveGameplayRevision;
		m_GameplayRevisionState.RequiredPinnedRevisions =
			accepted.RequiredPinnedGameplayRevisions;
		m_GameplayRevisionState.hasPresentationArtifactBaseline =
			hasPresentationArtifactBaseline;
		m_GameplayRevisionState.PresentationArtifactBaseline =
			std::move(stagedPresentationArtifactBaseline);
		m_GameplayRevisionState.BootstrapPresentationReceipt =
			std::move(stagedPresentationReceipt);
		m_GameplayRevisionState.hasBootstrapPresentationRevision =
			hasPresentationArtifactBaseline &&
			stagedHasBootstrapPresentationRevision;
		m_GameplayRevisionState.BootstrapPresentationRevision =
			hasPresentationArtifactBaseline ?
				stagedBootstrapPresentationRevision : GameplayDataRevision{};
		if (hasPresentationArtifactBaseline)
		{
			m_GameplayRevisionState.AvailablePresentationAliases =
				std::move(stagedPresentationAliases);
		}
		else
		{
			m_GameplayRevisionState.isPresentationIsolated = true;
			m_GameplayRevisionState.strIsolationReason =
				"Gameplay entry was admitted, but Client presentation source validation failed. Reload the presentation sources before using revision-dependent preview or live apply. " +
				(baselineStatus.empty() ?
					std::string{ "No validation detail was reported." } :
					baselineStatus);
		}
		m_hasLocalSpawn = false;
		m_LocalSpawn = {};
		m_hasPendingEnterAccepted = true;
		m_PendingEnterAccepted = accepted;
		m_SessionDiagnostic.Record_EnterAccepted(
			accepted.eWorldId, accepted.iPlayerId, accepted.iNetEntityId);
		break;
	}
	case PACKET_TYPE::S2C_ENTER_REJECTED:
	{
		S2C_ENTER_REJECTED rejected{};
		if (!Read_Message(reader, rejected) ||
			0 != reader.Get_RemainingSize())
		{
			Fail_Protocol(
				WSAEINVAL,
				SESSION_DIAGNOSTIC_REASON::CLIENT_MESSAGE_DECODE_FAILED,
				PACKET_TYPE::S2C_ENTER_REJECTED,
				"S2C_ENTER_REJECTED payload decode or trailing-byte validation failed.");
			return;
		}
		m_hasPendingEnterRejected = true;
		m_PendingEnterRejected = rejected;
		m_SessionDiagnostic.Record_EnterRejected(rejected.eWorldId);
		break;
	}
	//Player Spawn
	case PACKET_TYPE::S2C_PLAYER_SPAWNED:
	{
		S2C_PLAYER_SPAWNED spawned{};

		if (!Read_Message(reader, spawned) ||
			0 != reader.Get_RemainingSize())
		{
			m_iLastErrorCode.store(WSAEINVAL);
			return;
		}
		//Client Replication Event ����
		if (spawned.iPlayerId == m_iLocalPlayerId &&
			spawned.iNetEntityId == m_iLocalNetEntityId &&
			std::isfinite(spawned.fPositionX) &&
			std::isfinite(spawned.fPositionY) &&
			std::isfinite(spawned.fPositionZ) &&
			std::isfinite(spawned.fYawDegrees))
		{
			m_LocalSpawn = spawned;
			m_hasLocalSpawn = true;
		}
		Client::CLIENT_REPLICATION_EVENT event{};
		event.eType = Client::CLIENT_REPLICATION_EVENT_TYPE::PLAYER_SPAWNED;
		event.PlayerSpawned = std::move(spawned);
		Enqueue_ReplicationEvent(std::move(event));
		break;
	}
	case PACKET_TYPE::S2C_WORLD_ENTITY_SPAWNED:
	{
		S2C_WORLD_ENTITY_SPAWNED spawned{};
		if (!Read_Message(reader, spawned) ||
			0 != reader.Get_RemainingSize())
		{
			m_iLastErrorCode.store(WSAEINVAL);
			return;
		}
		if (!Is_AnnouncedWorldRevision(spawned.PinnedDefinitionRevision))
		{
			const std::string revision = Format_GameplayDataRevision(
				spawned.PinnedDefinitionRevision);
			Fail_Protocol(
				WSAEINVAL,
				SESSION_DIAGNOSTIC_REASON::CLIENT_INVALID_SERVER_RESPONSE,
				PACKET_TYPE::S2C_WORLD_ENTITY_SPAWNED,
				"World-entity spawn referenced unannounced pinned revision " +
					(revision.empty() ? std::string{ "INVALID" } : revision) + ".");
			return;
		}
		/* Bind the presentation to the entity's exact occurrence generation,
		   never to whichever room-active revision happens to be current when
		   the network thread dequeues this reliable frame. */
		if (!Is_PresentationRevisionAvailable(
				spawned.PinnedDefinitionRevision))
		{
			Record_PresentationIsolation(
				spawned.PinnedDefinitionRevision,
				"World-entity spawn");
			break;
		}
		Client::CLIENT_REPLICATION_EVENT event{};
		event.eType =
			Client::CLIENT_REPLICATION_EVENT_TYPE::WORLD_ENTITY_SPAWNED;
		event.WorldEntitySpawned = std::move(spawned);
		Enqueue_ReplicationEvent(std::move(event));
		break;
	}
	case PACKET_TYPE::S2C_COMBAT_OBJECT_SPAWNED:
	{
		S2C_COMBAT_OBJECT_SPAWNED spawned{};
		if (!Read_Message(reader, spawned) ||
			0 != reader.Get_RemainingSize())
		{
			m_iLastErrorCode.store(WSAEINVAL);
			return;
		}
		if (!Is_PresentationRevisionAvailable(
				spawned.PinnedDefinitionRevision))
		{
			Record_PresentationIsolation(
				spawned.PinnedDefinitionRevision,
				"Combat-object spawn");
			break;
		}
		Client::CLIENT_REPLICATION_EVENT event{};
		event.eType =
			Client::CLIENT_REPLICATION_EVENT_TYPE::COMBAT_OBJECT_SPAWNED;
		event.CombatObjectSpawned = std::move(spawned);
		Enqueue_ReplicationEvent(std::move(event));
		break;
	}
	case PACKET_TYPE::S2C_COMBAT_OBJECT_PRESENTATION_EVENT:
	{
		S2C_COMBAT_OBJECT_PRESENTATION_EVENT presentation{};
		if (!Read_Message(reader, presentation) ||
			0 != reader.Get_RemainingSize())
		{
			m_iLastErrorCode.store(WSAEINVAL);
			return;
		}
		if (!Is_PresentationRevisionAvailable(
				presentation.PinnedDefinitionRevision))
		{
			Record_PresentationIsolation(
				presentation.PinnedDefinitionRevision,
				"Combat-object presentation event");
			break;
		}
		Client::CLIENT_REPLICATION_EVENT event{};
		event.eType = Client::CLIENT_REPLICATION_EVENT_TYPE::
			COMBAT_OBJECT_PRESENTATION;
		event.CombatObjectPresentation = std::move(presentation);
		Enqueue_ReplicationEvent(std::move(event));
		break;
	}
	case PACKET_TYPE::S2C_WORLD_ENTITY_DESPAWNED:
	{
		S2C_WORLD_ENTITY_DESPAWNED despawned{};
		if (!Read_Message(reader, despawned) ||
			0 != reader.Get_RemainingSize())
		{
			m_iLastErrorCode.store(WSAEINVAL);
			return;
		}
		Client::CLIENT_REPLICATION_EVENT event{};
		event.eType =
			Client::CLIENT_REPLICATION_EVENT_TYPE::WORLD_ENTITY_DESPAWNED;
		event.WorldEntityDespawned = despawned;
		Enqueue_ReplicationEvent(std::move(event));
		break;
	}
	case PACKET_TYPE::S2C_COMBAT_OBJECT_DESPAWNED:
	{
		S2C_COMBAT_OBJECT_DESPAWNED despawned{};
		if (!Read_Message(reader, despawned) ||
			0 != reader.Get_RemainingSize())
		{
			m_iLastErrorCode.store(WSAEINVAL);
			return;
		}
		Client::CLIENT_REPLICATION_EVENT event{};
		event.eType =
			Client::CLIENT_REPLICATION_EVENT_TYPE::COMBAT_OBJECT_DESPAWNED;
		event.CombatObjectDespawned = despawned;
		Enqueue_ReplicationEvent(std::move(event));
		break;
	}
	case PACKET_TYPE::S2C_WORLD_ENTITY_SPAWN_RESULT:
	{
		S2C_WORLD_ENTITY_SPAWN_RESULT result{};
		if (!Read_Message(reader, result) ||
			0 != reader.Get_RemainingSize())
		{
			m_iLastErrorCode.store(WSAEINVAL);
			return;
		}
		m_WorldEntitySpawnResults.push_back(std::move(result));
		break;
	}
	case PACKET_TYPE::S2C_VALTAN_AUDITION_RESULT:
	{
		S2C_VALTAN_AUDITION_RESULT result{};
		if (!Read_Message(reader, result) ||
			0 != reader.Get_RemainingSize())
		{
			m_iLastErrorCode.store(WSAEINVAL);
			return;
		}
		if (VALTAN_AUDITION_OPERATION::PLAY_PATTERN_ID == result.eOperation ||
			VALTAN_AUDITION_OPERATION::RESTART_PATTERN_ID == result.eOperation ||
			VALTAN_AUDITION_OPERATION::QUEUE_NEXT_PATTERN_ID == result.eOperation ||
			VALTAN_AUDITION_OPERATION::CLEAR_NEXT_PATTERN_ID == result.eOperation ||
			VALTAN_AUDITION_OPERATION::QUEUE_NEXT_LIVE_PATTERN_ID == result.eOperation)
		{
			if (m_ValtanPatternAuditionByIdResults.size() >= MAX_REVISION_CONTROL_QUEUE)
			{
				Fail_Protocol(WSAENOBUFS);
				return;
			}
			m_ValtanPatternAuditionByIdResults.push_back(std::move(result));
		}
		else
			m_ValtanAuditionResults.push_back(std::move(result));
		break;
	}
	case PACKET_TYPE::S2C_VALTAN_AUDITION_LIFECYCLE:
	{
		S2C_VALTAN_AUDITION_LIFECYCLE lifecycle{};
		if (!Read_Message(reader, lifecycle) ||
			0u != reader.Get_RemainingSize())
		{
			Fail_Protocol(WSAEINVAL);
			return;
		}
		if (m_ValtanAuditionLifecycleEvents.size() >=
			MAX_REVISION_CONTROL_QUEUE)
		{
			Fail_Protocol(WSAENOBUFS);
			return;
		}
		m_ValtanAuditionLifecycleEvents.push_back(std::move(lifecycle));
		break;
	}
	case PACKET_TYPE::S2C_DEBUG_VALTAN_PATTERN_FLOW_RESULT:
	{
		S2C_DEBUG_VALTAN_PATTERN_FLOW_RESULT result{};
		if (!Read_Message(reader, result) || 0u != reader.Get_RemainingSize())
		{
			Fail_Protocol(WSAEINVAL);
			return;
		}
		if (m_ValtanPatternFlowResults.size() >= MAX_REVISION_CONTROL_QUEUE)
		{
			Fail_Protocol(WSAENOBUFS);
			return;
		}
		m_ValtanPatternFlowResults.push_back(std::move(result));
		break;
	}
	case PACKET_TYPE::S2C_DEBUG_VALTAN_PATTERN_FLOW_LIFECYCLE:
	{
		S2C_DEBUG_VALTAN_PATTERN_FLOW_LIFECYCLE lifecycle{};
		if (!Read_Message(reader, lifecycle) ||
			0u != reader.Get_RemainingSize())
		{
			Fail_Protocol(WSAEINVAL);
			return;
		}
		if (m_ValtanPatternFlowLifecycleEvents.size() >=
			MAX_REVISION_CONTROL_QUEUE)
		{
			Fail_Protocol(WSAENOBUFS);
			return;
		}
		m_ValtanPatternFlowLifecycleEvents.push_back(std::move(lifecycle));
		break;
	}
	case PACKET_TYPE::S2C_VALTAN_DECISION_TRACE_RESPONSE:
	{
		S2C_VALTAN_DECISION_TRACE_RESPONSE response{};
		if (!Read_Message(reader, response) ||
			0u != reader.Get_RemainingSize())
		{
			Fail_Protocol(WSAEINVAL);
			return;
		}
		if (!m_ValtanDecisionTraceState.isQueryPending ||
			response.iRequestSequence !=
				m_ValtanDecisionTraceState.iSubmittedRequestSequence ||
			response.strBossPlacementId !=
				m_ValtanDecisionTraceState.strSubmittedBossPlacementId ||
			(VALTAN_DECISION_TRACE_QUERY_RESULT::TRACE == response.eResult &&
			 response.Trace.iTraceSequence <=
				m_ValtanDecisionTraceState.iSubmittedAfterTraceSequence))
		{
			Fail_Protocol(WSAEINVAL);
			return;
		}
		m_ValtanDecisionTraceState.isQueryPending = false;
		m_ValtanDecisionTraceState.hasLatestResponse = true;
		m_ValtanDecisionTraceState.iLatestResponseRequestSequence =
			response.iRequestSequence;
		m_ValtanDecisionTraceState.eLatestResponse = response.eResult;
		if (VALTAN_DECISION_TRACE_QUERY_RESULT::TRACE == response.eResult)
		{
			m_ValtanDecisionTraceState.hasLatestTrace = true;
			m_ValtanDecisionTraceState.strLatestBossPlacementId =
				std::move(response.strBossPlacementId);
			m_ValtanDecisionTraceState.LatestDefinitionRevision =
				response.DefinitionRevision;
			m_ValtanDecisionTraceState.LatestTrace =
				std::move(response.Trace);
		}
		break;
	}
	case PACKET_TYPE::S2C_DATA_REVISION_PREPARE:
	{
		S2C_DATA_REVISION_PREPARE prepare{};
		if (!Read_Message(reader, prepare) ||
			0u != reader.Get_RemainingSize())
		{
			Fail_Protocol(WSAEINVAL);
			return;
		}
		C2S_DATA_REVISION_PREPARE_RESPONSE response{};
		response.iTransactionSequence = prepare.iTransactionSequence;
		response.CandidateRevision = prepare.CandidateRevision;
		response.iRequiredPresentationLaneMask =
			prepare.iRequiredPresentationLaneMask;
		std::string stageStatus;
		const bool staged = Stage_ByteIdenticalPresentationAlias(
			prepare, stageStatus);
		response.eStatus = staged ?
			DATA_REVISION_PREPARE_STATUS::READY :
			DATA_REVISION_PREPARE_STATUS::NACK;
		response.iPreparedPresentationLaneMask = staged ?
			prepare.iRequiredPresentationLaneMask : 0u;
		response.iFailedPresentationLaneMask = staged ? 0u :
			prepare.iRequiredPresentationLaneMask;
		/* Shared requires an empty reason for READY; diagnostics are carried only
		   on the local observation state. NACK preserves the exact admission
		   failure for the coordinator and Balance Tool. */
		response.strReason = staged ? std::string{} : stageStatus;
		m_GameplayRevisionState.iLatestTransactionSequence =
			prepare.iTransactionSequence;
		m_GameplayRevisionState.hasLatestPrepare = true;
		m_GameplayRevisionState.LatestPrepareBaseRevision =
			prepare.BaseRevision;
		m_GameplayRevisionState.LatestCandidateRevision =
			prepare.CandidateRevision;
		m_GameplayRevisionState.iLatestRequiredPresentationLaneMask =
			prepare.iRequiredPresentationLaneMask;
		m_GameplayRevisionState.eLatestPrepareResponse =
			response.eStatus;
		m_GameplayRevisionState.strLatestTransactionReason = stageStatus;
		if (staged)
		{
			m_GameplayRevisionState.hasRejectedPrepareAwaitingAbort = false;
			m_GameplayRevisionState.iRejectedPrepareTransactionSequence = 0u;
			m_GameplayRevisionState.RejectedPrepareBaseRevision = {};
			m_GameplayRevisionState.RejectedPrepareCandidateRevision = {};
		}
		else
		{
			/* NACK is not terminal: the coordinator broadcasts one matching
			   process-wide ABORT to every participant, including this rejector. */
			m_GameplayRevisionState.hasRejectedPrepareAwaitingAbort = true;
			m_GameplayRevisionState.iRejectedPrepareTransactionSequence =
				prepare.iTransactionSequence;
			m_GameplayRevisionState.RejectedPrepareBaseRevision =
				prepare.BaseRevision;
			m_GameplayRevisionState.RejectedPrepareCandidateRevision =
				prepare.CandidateRevision;
		}
		if (!Send_DataRevisionPrepareResponse(response))
		{
			const int sendError = m_iLastErrorCode.load();
			Fail_Protocol(0 != sendError ? sendError : WSAECONNABORTED);
			return;
		}
		break;
	}
	case PACKET_TYPE::S2C_DATA_REVISION_RESULT:
	{
		S2C_DATA_REVISION_RESULT result{};
		if (!Read_Message(reader, result) ||
			0u != reader.Get_RemainingSize())
		{
			Fail_Protocol(WSAEINVAL);
			return;
		}
		std::string commitStatus;
		const bool presentationCommitted =
			Commit_StagedPresentationAlias(result, commitStatus);
		if (!presentationCommitted)
		{
			/* A conflicting result cannot advance the observed Server generation or
			   consume a legitimate local stage. Fail the connection before copying
			   any result fields into the Client revision state. */
			Fail_Protocol(WSAEINVAL);
			return;
		}
		m_GameplayRevisionState.iLatestTransactionSequence =
			result.iTransactionSequence;
		m_GameplayRevisionState.hasLatestResult = true;
		m_GameplayRevisionState.LatestCandidateRevision =
			result.CandidateRevision;
		m_GameplayRevisionState.eLatestResult = result.eResult;
		m_GameplayRevisionState.strLatestTransactionReason = commitStatus;
		if (DATA_REVISION_RESULT::COMMITTED == result.eResult)
		{
			m_GameplayRevisionState.ServerActiveRevision =
				result.ActiveRevision;
			if (!Is_PresentationRevisionAvailable(result.ActiveRevision))
			{
				Record_PresentationIsolation(
					result.ActiveRevision, "Committed Server revision");
			}
		}
		break;
	}
	case PACKET_TYPE::S2C_CHARACTER_CLASS_CHANGE_RESULT:
	{
		S2C_CHARACTER_CLASS_CHANGE_RESULT result{};
		if (!Read_Message(reader, result) || 0 != reader.Get_RemainingSize())
		{
			m_iLastErrorCode.store(WSAEINVAL);
			return;
		}
		if (CHARACTER_CLASS_CHANGE_RESULT::ACCEPTED == result.eResult)
		{
			m_eLocalCharacterClass = result.eActiveClass;
			if (m_hasLocalSpawn)
				m_LocalSpawn.eCharacterClass = result.eActiveClass;
		}
		m_CharacterClassChangeResults.push_back(std::move(result));
		break;
	}
	case PACKET_TYPE::S2C_INVENTORY_SNAPSHOT:
	{
		S2C_INVENTORY_SNAPSHOT snapshot{};
		if (!Read_Message(reader, snapshot) || 0 != reader.Get_RemainingSize())
		{
			m_iLastErrorCode.store(WSAEINVAL);
			return;
		}
		Client::CLIENT_REPLICATION_EVENT event{};
		event.eType =
			Client::CLIENT_REPLICATION_EVENT_TYPE::INVENTORY_SNAPSHOT;
		event.InventorySnapshot = std::move(snapshot);
		Enqueue_ReplicationEvent(std::move(event));
		break;
	}
	case PACKET_TYPE::S2C_PARTY_INVITE_RECEIVED:
	{
		S2C_PARTY_INVITE_RECEIVED received{};
		if (!Read_Message(reader, received) || 0 != reader.Get_RemainingSize())
		{
			m_iLastErrorCode.store(WSAEINVAL);
			return;
		}
		Client::CLIENT_REPLICATION_EVENT event{};
		event.eType =
			Client::CLIENT_REPLICATION_EVENT_TYPE::PARTY_INVITE_RECEIVED;
		event.PartyInviteReceived = std::move(received);
		Enqueue_ReplicationEvent(std::move(event));
		break;
	}
	case PACKET_TYPE::S2C_PARTY_ROSTER:
	{
		S2C_PARTY_ROSTER roster{};
		if (!Read_Message(reader, roster) || 0 != reader.Get_RemainingSize())
		{
			m_iLastErrorCode.store(WSAEINVAL);
			return;
		}
		Client::CLIENT_REPLICATION_EVENT event{};
		event.eType = Client::CLIENT_REPLICATION_EVENT_TYPE::PARTY_ROSTER;
		event.PartyRoster = std::move(roster);
		Enqueue_ReplicationEvent(std::move(event));
		break;
	}
	case PACKET_TYPE::S2C_PARTY_TRANSFER_RESULT:
	{
		S2C_PARTY_TRANSFER_RESULT result{};
		if (!Read_Message(reader, result) || 0 != reader.Get_RemainingSize())
		{
			Fail_Protocol(WSAEINVAL,
				SESSION_DIAGNOSTIC_REASON::CLIENT_MESSAGE_DECODE_FAILED,
				PACKET_TYPE::S2C_PARTY_TRANSFER_RESULT,
				"S2C_PARTY_TRANSFER_RESULT payload decode or trailing-byte validation failed.");
			return;
		}
		Client::CLIENT_REPLICATION_EVENT event{};
		event.eType = Client::CLIENT_REPLICATION_EVENT_TYPE::PARTY_TRANSFER_RESULT;
		event.PartyTransferResult = result;
		Enqueue_ReplicationEvent(std::move(event));
		break;
	}
	case PACKET_TYPE::S2C_CHAT:
	{
		S2C_CHAT chat{};
		if (!Read_Message(reader, chat) || 0 != reader.Get_RemainingSize())
		{
			m_iLastErrorCode.store(WSAEINVAL);
			return;
		}
		Client::CLIENT_REPLICATION_EVENT event{};
		event.eType = Client::CLIENT_REPLICATION_EVENT_TYPE::CHAT_RECEIVED;
		event.ChatReceived = std::move(chat);
		Enqueue_ReplicationEvent(std::move(event));
		break;
	}
	//snapshot
	case PACKET_TYPE::S2C_WORLD_SNAPSHOT:
	{
		//world�� snapshot�� ���� ����ü ����
		S2C_WORLD_SNAPSHOT snapshot{};

		if (!Read_Message(reader, snapshot) ||
			0 != reader.Get_RemainingSize())
		{
			Fail_Protocol(
				WSAEINVAL,
				SESSION_DIAGNOSTIC_REASON::CLIENT_MESSAGE_DECODE_FAILED,
				PACKET_TYPE::S2C_WORLD_SNAPSHOT,
				"S2C_WORLD_SNAPSHOT payload decode or trailing-byte validation failed.");
			return;
		}
		if (snapshot.eWorldId != m_eWorldId)
		{
			const std::string detail =
				"S2C_WORLD_SNAPSHOT world mismatch: expected " +
				std::to_string(static_cast<std::uint16_t>(m_eWorldId)) +
				", received " +
				std::to_string(static_cast<std::uint16_t>(snapshot.eWorldId)) + ".";
			Fail_Protocol(
				WSAEINVAL,
				SESSION_DIAGNOSTIC_REASON::CLIENT_INVALID_SERVER_RESPONSE,
				PACKET_TYPE::S2C_WORLD_SNAPSHOT,
				detail);
			return;
		}
		if (snapshot.ActiveGameplayRevision !=
			m_GameplayRevisionState.ServerActiveRevision)
		{
			const std::string expected = Format_GameplayDataRevision(
				m_GameplayRevisionState.ServerActiveRevision);
			const std::string received = Format_GameplayDataRevision(
				snapshot.ActiveGameplayRevision);
			const std::string detail =
				"S2C_WORLD_SNAPSHOT active gameplay revision mismatch: expected " +
				(expected.empty() ? std::string{ "INVALID" } : expected) +
				", received " +
				(received.empty() ? std::string{ "INVALID" } : received) + ".";
			Fail_Protocol(
				WSAEINVAL,
				SESSION_DIAGNOSTIC_REASON::CLIENT_INVALID_SERVER_RESPONSE,
				PACKET_TYPE::S2C_WORLD_SNAPSHOT,
				detail);
			return;
		}

		m_SessionDiagnostic.Record_ServerTick(snapshot.iServerTick);
		Record_WorldRevisionSet(
			snapshot.ActiveGameplayRevision,
			snapshot.RequiredPinnedGameplayRevisions);
		std::vector<NET_ENTITY_ID> isolatedEntityIds;
		for (const WORLD_ENTITY_SNAPSHOT& entity : snapshot.Entities)
		{
			if (!Is_AnnouncedWorldRevision(
					entity.PinnedDefinitionRevision))
			{
				const std::string revision = Format_GameplayDataRevision(
					entity.PinnedDefinitionRevision);
				Fail_Protocol(
					WSAEINVAL,
					SESSION_DIAGNOSTIC_REASON::CLIENT_INVALID_SERVER_RESPONSE,
					PACKET_TYPE::S2C_WORLD_SNAPSHOT,
					"World entity " + std::to_string(entity.iNetEntityId) +
					" referenced unannounced pinned revision " +
					(revision.empty() ? std::string{ "INVALID" } : revision) + ".");
				return;
			}
			if (!Is_PresentationRevisionAvailable(
					entity.PinnedDefinitionRevision))
			{
				isolatedEntityIds.push_back(entity.iNetEntityId);
				Record_PresentationIsolation(
					entity.PinnedDefinitionRevision,
					"World-entity occurrence");
			}
		}
		snapshot.Entities.erase(
			std::remove_if(
				snapshot.Entities.begin(),
				snapshot.Entities.end(),
				[this](const WORLD_ENTITY_SNAPSHOT& entity)
				{
					return !Is_PresentationRevisionAvailable(
						entity.PinnedDefinitionRevision);
				}),
			snapshot.Entities.end());
		for (const COMBAT_OBJECT_SNAPSHOT& object : snapshot.CombatObjects)
		{
			if (!Is_AnnouncedWorldRevision(
					object.PinnedDefinitionRevision))
			{
				const std::string revision = Format_GameplayDataRevision(
					object.PinnedDefinitionRevision);
				Fail_Protocol(
					WSAEINVAL,
					SESSION_DIAGNOSTIC_REASON::CLIENT_INVALID_SERVER_RESPONSE,
					PACKET_TYPE::S2C_WORLD_SNAPSHOT,
					"Combat object " +
					std::to_string(object.iCombatObjectId) +
					" from entity " +
					std::to_string(object.iSourceNetEntityId) +
					" referenced unannounced pinned revision " +
					(revision.empty() ? std::string{ "INVALID" } : revision) + ".");
				return;
			}
			if (!Is_PresentationRevisionAvailable(
					object.PinnedDefinitionRevision))
			{
				Record_PresentationIsolation(
					object.PinnedDefinitionRevision,
					"Combat-object occurrence");
			}
		}
		snapshot.CombatObjects.erase(
			std::remove_if(
				snapshot.CombatObjects.begin(),
				snapshot.CombatObjects.end(),
				[this](const COMBAT_OBJECT_SNAPSHOT& object)
				{
					return !Is_PresentationRevisionAvailable(
						object.PinnedDefinitionRevision);
				}),
			snapshot.CombatObjects.end());
		if (!isolatedEntityIds.empty())
		{
			snapshot.BossCombatEvents.erase(
				std::remove_if(
					snapshot.BossCombatEvents.begin(),
					snapshot.BossCombatEvents.end(),
					[&isolatedEntityIds](const BOSS_COMBAT_EVENT& event)
					{
						return isolatedEntityIds.end() != std::find(
							isolatedEntityIds.begin(),
							isolatedEntityIds.end(),
							event.iBossNetEntityId);
					}),
				snapshot.BossCombatEvents.end());
		}

		Client::CLIENT_REPLICATION_EVENT event{};
		event.eType =
			Client::CLIENT_REPLICATION_EVENT_TYPE::WORLD_SNAPSHOT;
		event.WorldSnapshot = std::move(snapshot);
		Enqueue_ReplicationEvent(std::move(event));
		break;
	}
	case PACKET_TYPE::S2C_WORLD_DESTRUCTION_FULL_SYNC:
	{
		S2C_WORLD_DESTRUCTION_FULL_SYNC sync{};
		if (!Read_Message(reader, sync) ||
			0 != reader.Get_RemainingSize() ||
			WORLD_ID::VALTAN_ARENA != m_eWorldId)
		{
			Fail_Protocol(WSAEINVAL);
			return;
		}
		Client::CLIENT_REPLICATION_EVENT event{};
		event.eType = Client::CLIENT_REPLICATION_EVENT_TYPE::
			WORLD_DESTRUCTION_FULL_SYNC;
		event.WorldDestructionFullSync = std::move(sync);
		Enqueue_ReplicationEvent(std::move(event));
		break;
	}
	case PACKET_TYPE::S2C_ENCOUNTER_PROP_SYNC:
	{
		S2C_ENCOUNTER_PROP_SYNC sync{};
		if (!Read_Message(reader, sync) ||
			0 != reader.Get_RemainingSize() ||
			WORLD_ID::VALTAN_ARENA != m_eWorldId)
		{
			Fail_Protocol(WSAEINVAL);
			return;
		}
		Client::CLIENT_REPLICATION_EVENT event{};
		event.eType = Client::CLIENT_REPLICATION_EVENT_TYPE::
			ENCOUNTER_PROP_SYNC;
		event.EncounterPropSync = std::move(sync);
		Enqueue_ReplicationEvent(std::move(event));
		break;
	}
	case PACKET_TYPE::S2C_WORLD_DESTRUCTION_DELTA:
	{
		S2C_WORLD_DESTRUCTION_DELTA delta{};
		if (!Read_Message(reader, delta) ||
			0 != reader.Get_RemainingSize() ||
			WORLD_ID::VALTAN_ARENA != m_eWorldId)
		{
			Fail_Protocol(WSAEINVAL);
			return;
		}
		Client::CLIENT_REPLICATION_EVENT event{};
		event.eType = Client::CLIENT_REPLICATION_EVENT_TYPE::
			WORLD_DESTRUCTION_DELTA;
		event.WorldDestructionDelta = std::move(delta);
		Enqueue_ReplicationEvent(std::move(event));
		break;
	}
	//player despawn
	case PACKET_TYPE::S2C_PLAYER_DESPAWNED:
	{
		S2C_PLAYER_DESPAWNED despawned{};

		if (!Read_Message(reader, despawned) ||
			0 != reader.Get_RemainingSize())
		{
			m_iLastErrorCode.store(WSAEINVAL);
			return;
		}
		if (despawned.iNetEntityId == m_iLocalNetEntityId)
		{
			m_hasLocalSpawn = false;
			m_LocalSpawn = {};
		}
		Client::CLIENT_REPLICATION_EVENT event{};
		event.eType = Client::CLIENT_REPLICATION_EVENT_TYPE::PLAYER_DESPAWNED;
		event.PlayerDespawned = despawned;
		Enqueue_ReplicationEvent(std::move(event));
		break;
	}
	default:
		break;
	}
}

bool CNetworkManager::Send_All(
	std::span<const std::uint8_t> bytes,
	const LostArk::Shared::PACKET_TYPE triggeringPacket)
{
	if (!Is_Connected())
	{
		m_iLastErrorCode.store(WSAENOTCONN);
		m_SessionDiagnostic.Record_Terminal(
			LostArk::Shared::SESSION_DIAGNOSTIC_REASON::
				CLIENT_CONNECTION_LOST,
			WSAENOTCONN,
			triggeringPacket,
			"A packet send was attempted without a live connection.");
		return false;
	}

	std::size_t sentByteCount = 0;

	while (sentByteCount < bytes.size())
	{
		const int result = ::send(
			m_hServerSocket,
			reinterpret_cast<const char*>(
				bytes.data() + sentByteCount),
			static_cast<int>(bytes.size() - sentByteCount),
			0);

		if (SOCKET_ERROR == result)
		{
			const int errorCode = ::WSAGetLastError();
			m_iLastErrorCode.store(errorCode);
			m_SessionDiagnostic.Record_Terminal(
				LostArk::Shared::SESSION_DIAGNOSTIC_REASON::CLIENT_SEND_ERROR,
				errorCode,
				triggeringPacket,
				"send() failed before the full packet frame was written.");
			return false;
		}

		if (0 == result)
		{
			m_iLastErrorCode.store(WSAECONNRESET);
			m_SessionDiagnostic.Record_Terminal(
				LostArk::Shared::SESSION_DIAGNOSTIC_REASON::CLIENT_SEND_ERROR,
				WSAECONNRESET,
				triggeringPacket,
				"send() returned zero before the full packet frame was written.");
			return false;
		}

		sentByteCount += static_cast<std::size_t>(result);
	}

	return true;
}
