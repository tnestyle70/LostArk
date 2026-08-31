#include "Effect_DirectAuthoredSourceIndex.h"

#include "DataJson.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <cwchar>
#include <fstream>
#include <iomanip>
#include <iterator>
#include <limits>
#include <sstream>
#include <string_view>
#include <system_error>
#include <tuple>
#include <unordered_map>
#include <unordered_set>

#pragma comment(lib, "bcrypt.lib")

namespace
{
	bool_t Read_TextFile(const std::filesystem::path& Path,
		std::string& strOutText, std::string& strOutStatus)
	{
		strOutText.clear();
		std::ifstream Input(Path, std::ios::binary);
		if (Path.empty() || !Input)
		{
			strOutStatus = "Could not open EffectCatalog.json: " +
				Path.string();
			return false;
		}
		std::ostringstream Buffer;
		Buffer << Input.rdbuf();
		if (!Input.good() && !Input.eof())
		{
			strOutStatus = "Could not read EffectCatalog.json: " +
				Path.string();
			return false;
		}
		strOutText = Buffer.str();
		return true;
	}

	bool_t Read_BoundedBinaryFile(const std::filesystem::path& Path,
		const std::uintmax_t iMaximumBytes,
		std::string& strOutBytes, std::string& strOutStatus)
	{
		strOutBytes.clear();
		std::error_code Error;
		const std::uintmax_t iFileSize =
			std::filesystem::file_size(Path, Error);
		if (Path.empty() || Error || 0u == iFileSize ||
			iFileSize > iMaximumBytes)
		{
			strOutStatus =
				"Registry-bound audition source is missing, empty, or too large: " +
				Path.string();
			return false;
		}
		std::ifstream Input(Path, std::ios::binary);
		if (!Input)
		{
			strOutStatus =
				"Could not open registry-bound audition source: " + Path.string();
			return false;
		}
		strOutBytes.assign(std::istreambuf_iterator<char>(Input),
			std::istreambuf_iterator<char>());
		if (strOutBytes.size() != iFileSize)
		{
			strOutBytes.clear();
			strOutStatus =
				"Could not read complete registry-bound audition source: " +
				Path.string();
			return false;
		}
		strOutStatus.clear();
		return true;
	}

	bool_t Try_ParsePlayerOwner(const std::string_view strAssetId,
		LostArk::Shared::CHARACTER_CLASS_ID& eOutClass,
		LostArk::Shared::SKILL_ID& iOutSkillId)
	{
		using LostArk::Shared::CHARACTER_CLASS_ID;
		struct OWNER_PREFIX final
		{
			std::string_view Prefix;
			CHARACTER_CLASS_ID eClass = CHARACTER_CLASS_ID::END;
		};
		static constexpr std::array<OWNER_PREFIX, 4u> Prefixes = {
			OWNER_PREFIX{ "effect.artist.skill.", CHARACTER_CLASS_ID::ARTIST },
			OWNER_PREFIX{ "effect.dimensionmaster.skill.",
				CHARACTER_CLASS_ID::DIMENSIONMASTER },
			OWNER_PREFIX{ "effect.lancemaster.skill.",
				CHARACTER_CLASS_ID::LANCE_MASTER },
			OWNER_PREFIX{ "effect.warlord.skill.",
				CHARACTER_CLASS_ID::WARLORD }
		};
		if (!strAssetId.ends_with(".unified"))
			return false;
		for (const OWNER_PREFIX& Owner : Prefixes)
		{
			if (!strAssetId.starts_with(Owner.Prefix))
				continue;
			const std::string_view Tail = strAssetId.substr(Owner.Prefix.size());
			const size_t iSkillEnd = Tail.find('.');
			const std::string_view SkillToken = Tail.substr(0u, iSkillEnd);
			if (SkillToken.empty())
				return false;
			uint32_t iSkillId = 0u;
			for (const char Character : SkillToken)
			{
				if (Character < '0' || Character > '9')
					return false;
				const uint32_t iDigit =
					static_cast<uint32_t>(Character - '0');
				if (iSkillId >
					((std::numeric_limits<uint32_t>::max)() - iDigit) / 10u)
				{
					return false;
				}
				iSkillId = iSkillId * 10u + iDigit;
			}
			if (0u == iSkillId)
				return false;
			eOutClass = Owner.eClass;
			iOutSkillId =
				static_cast<LostArk::Shared::SKILL_ID>(iSkillId);
			return true;
		}
		return false;
	}

	bool_t Try_DeriveAssetId(const std::filesystem::path& Path,
		std::string& strOutAssetId)
	{
		constexpr std::string_view EffectSuffix = ".effect.json";
		const std::string Name = Path.filename().string();
		if (!Name.ends_with(EffectSuffix))
			return false;
		std::string AssetId = Name.substr(
			0u, Name.size() - EffectSuffix.size());
		if (AssetId.empty() || AssetId.size() > 128u ||
			!std::all_of(AssetId.begin(), AssetId.end(),
				[](const char Character)
				{
					const unsigned char Value =
						static_cast<unsigned char>(Character);
					return 0 != std::isalnum(Value) || Character == '_' ||
						Character == '-' || Character == '.';
				}))
		{
			return false;
		}
		strOutAssetId = std::move(AssetId);
		return true;
	}

	bool_t PathComponentEquals(const std::filesystem::path& Left,
		const std::filesystem::path& Right)
	{
		const std::wstring LeftText = Left.native();
		const std::wstring RightText = Right.native();
#if defined(_WIN32)
		return 0 == _wcsicmp(LeftText.c_str(), RightText.c_str());
#else
		return LeftText == RightText;
#endif
	}

	bool_t Is_StrictDescendant(const std::filesystem::path& Root,
		const std::filesystem::path& Candidate)
	{
		auto RootIterator = Root.begin();
		auto CandidateIterator = Candidate.begin();
		for (; RootIterator != Root.end();
			++RootIterator, ++CandidateIterator)
		{
			if (CandidateIterator == Candidate.end() ||
				!PathComponentEquals(*RootIterator, *CandidateIterator))
			{
				return false;
			}
		}
		return CandidateIterator != Candidate.end();
	}

	bool_t Is_LowerHexSha256(const std::string_view Value)
	{
		return Value.size() == 64u && std::all_of(
			Value.begin(), Value.end(), [](const char Character)
			{
				return (Character >= '0' && Character <= '9') ||
					(Character >= 'a' && Character <= 'f');
			});
	}

	std::string Compute_Sha256Hex(const std::string_view Value)
	{
		BCRYPT_ALG_HANDLE Algorithm = nullptr;
		BCRYPT_HASH_HANDLE Hash = nullptr;
		DWORD iObjectSize = 0u;
		DWORD iResultSize = 0u;
		std::vector<uint8_t> Object;
		std::array<uint8_t, 32u> Digest{};
		bool_t bSuccess = false;
		if (Value.size() <= (std::numeric_limits<ULONG>::max)() &&
			BCryptOpenAlgorithmProvider(
				&Algorithm, BCRYPT_SHA256_ALGORITHM, nullptr, 0u) >= 0 &&
			BCryptGetProperty(Algorithm, BCRYPT_OBJECT_LENGTH,
				reinterpret_cast<PUCHAR>(&iObjectSize), sizeof(iObjectSize),
				&iResultSize, 0u) >= 0)
		{
			Object.resize(iObjectSize);
			bSuccess = BCryptCreateHash(Algorithm, &Hash, Object.data(),
				iObjectSize, nullptr, 0u, 0u) >= 0 &&
				BCryptHashData(Hash, reinterpret_cast<PUCHAR>(
					const_cast<char*>(Value.data())),
					static_cast<ULONG>(Value.size()), 0u) >= 0 &&
				BCryptFinishHash(Hash, Digest.data(),
					static_cast<ULONG>(Digest.size()), 0u) >= 0;
		}
		if (nullptr != Hash)
			BCryptDestroyHash(Hash);
		if (nullptr != Algorithm)
			BCryptCloseAlgorithmProvider(Algorithm, 0u);
		if (!bSuccess)
			return {};
		std::ostringstream Output;
		Output << std::hex << std::setfill('0');
		for (const uint8_t Byte : Digest)
			Output << std::setw(2) << static_cast<uint32_t>(Byte);
		return Output.str();
	}

	bool_t Try_ResolveCatalogAuthoredPath(
		const std::filesystem::path& AuthoredRoot,
		const std::string_view strAuthoringPath,
		const std::string_view strExpectedAssetId,
		const std::filesystem::path& ExpectedPath,
		std::string& strOutStatus)
	{
		if (strAuthoringPath.empty() ||
			strAuthoringPath.find('\\') != std::string_view::npos ||
			strAuthoringPath.find(':') != std::string_view::npos ||
			strAuthoringPath.starts_with('/') ||
			strAuthoringPath.ends_with('/') ||
			strAuthoringPath.find("//") != std::string_view::npos)
		{
			strOutStatus =
				"Registry-bound audition catalog authoringPath is unsafe.";
			return false;
		}
		const std::filesystem::path RelativePath{
			std::string(strAuthoringPath) };
		std::vector<std::string> Components;
		for (const std::filesystem::path& Component : RelativePath)
		{
			const std::string Token = Component.generic_string();
			if (Token.empty() || Token == "." || Token == "..")
			{
				Components.clear();
				break;
			}
			Components.push_back(Token);
		}
		std::string strDerivedAssetId;
		if (RelativePath.empty() || RelativePath.is_absolute() ||
			RelativePath.has_root_path() ||
			RelativePath.generic_string() != std::string(strAuthoringPath) ||
			Components.size() < 3u || Components[0] != "Effects" ||
			Components[1] != "Authored" ||
			!Try_DeriveAssetId(RelativePath, strDerivedAssetId) ||
			strDerivedAssetId != strExpectedAssetId)
		{
			strOutStatus =
				"Registry-bound audition catalog authoringPath/Effect ID changed.";
			return false;
		}

		std::error_code Error;
		const std::filesystem::path CanonicalAuthoredRoot =
			std::filesystem::weakly_canonical(AuthoredRoot, Error);
		if (Error || CanonicalAuthoredRoot.empty())
		{
			strOutStatus =
				"Registry-bound audition Authored root is unavailable.";
			return false;
		}
		std::filesystem::path ResolvedPath = CanonicalAuthoredRoot;
		for (size_t iComponent = 2u; iComponent < Components.size(); ++iComponent)
			ResolvedPath /= Components[iComponent];
		Error.clear();
		ResolvedPath = std::filesystem::weakly_canonical(ResolvedPath, Error);
		if (Error || ResolvedPath.empty() ||
			!Is_StrictDescendant(CanonicalAuthoredRoot, ResolvedPath) ||
			!std::filesystem::is_regular_file(ResolvedPath, Error) || Error)
		{
			strOutStatus =
				"Registry-bound audition catalog authoringPath no longer resolves to an admitted file.";
			return false;
		}
		Error.clear();
		const std::filesystem::path CanonicalExpectedPath =
			std::filesystem::weakly_canonical(ExpectedPath, Error);
		if (Error || CanonicalExpectedPath.empty())
		{
			strOutStatus =
				"Registry-bound audition expected document path is unavailable.";
			return false;
		}
		Error.clear();
		if (!std::filesystem::equivalent(
				ResolvedPath, CanonicalExpectedPath, Error) || Error)
		{
			strOutStatus =
				"Registry-bound audition catalog authoringPath changed after open.";
			return false;
		}
		return true;
	}
}

bool Client::CEffectDirectAuthoredSourceIndex::
	Validate_RegistryBoundAuditionCatalogProvenanceFresh(
		const std::filesystem::path& ProductCatalogPath,
		const std::filesystem::path& AuditionCatalogPath,
		const std::filesystem::path& AuthoredRoot,
		const EFFECT_DIRECT_AUTHORED_SOURCE_ENTRY& Expected,
		std::string& strOutStatus)
{
	strOutStatus.clear();
	if (ProductCatalogPath.empty() || AuditionCatalogPath.empty() ||
		AuthoredRoot.empty() ||
		!Expected.bRegistryBoundAuditionOnly ||
		Expected.strEffectAssetId.empty() || Expected.Path.empty() ||
		Expected.strSourceEffectAssetId.empty() ||
		Expected.SourceDocumentPath.empty() ||
		!Is_LowerHexSha256(Expected.strSourceDocumentRawSha256) ||
		Expected.strEffectAssetId == Expected.strSourceEffectAssetId)
	{
		strOutStatus =
			"Registry-bound audition open-time provenance is incomplete.";
		return false;
	}
	const auto ReadCatalog = [&strOutStatus](
		const std::filesystem::path& Path, const std::string_view strLabel,
		DATA_JSON_VALUE& OutRoot,
		const DATA_JSON_VALUE*& pOutEffects)
	{
		std::error_code FileError;
		const std::uintmax_t iCatalogBytes =
			std::filesystem::file_size(Path, FileError);
		if (FileError || 0u == iCatalogBytes ||
			iCatalogBytes > 16u * 1024u * 1024u)
		{
			strOutStatus = std::string(strLabel) +
				" is missing, empty, or too large for live audition validation.";
			return false;
		}
		std::string Text;
		std::string Error;
		if (!Read_TextFile(Path, Text, Error))
		{
			strOutStatus = Error;
			return false;
		}
		DATA_JSON_PARSE_LIMITS Limits;
		Limits.iMaximumBytes = 16u * 1024u * 1024u;
		Limits.iMaximumDepth = 16u;
		Limits.iMaximumValues = 100'000u;
		if (!CDataJson::Parse(Text, OutRoot, Error, Limits) ||
			!OutRoot.Is_Object() || OutRoot.Get_Object().size() != 2u ||
			OutRoot.Get_ObjectInsertionOrder() !=
				std::vector<std::string>{ "formatVersion", "effects" })
		{
			strOutStatus = std::string(strLabel) +
				" changed to an invalid root during audition: " + Error;
			return false;
		}
		const DATA_JSON_VALUE* pVersion = OutRoot.Find("formatVersion");
		pOutEffects = OutRoot.Find("effects");
		if (nullptr == pVersion || !pVersion->Is_Number() ||
			pVersion->Was_FloatingPointToken() ||
			pVersion->Get_Number() != 1.0 || nullptr == pOutEffects ||
			!pOutEffects->Is_Array())
		{
			strOutStatus = std::string(strLabel) +
				" version/effects changed during audition.";
			return false;
		}
		return true;
	};
	DATA_JSON_VALUE ProductRoot;
	DATA_JSON_VALUE AuditionRoot;
	const DATA_JSON_VALUE* pProductEffects = nullptr;
	const DATA_JSON_VALUE* pAuditionEffects = nullptr;
	if (!ReadCatalog(ProductCatalogPath, "EffectCatalog.json", ProductRoot,
			pProductEffects) ||
		!ReadCatalog(AuditionCatalogPath, "EffectAuditionCatalog.json",
			AuditionRoot, pAuditionEffects))
	{
		return false;
	}

	const DATA_JSON_VALUE* pAuditionRow = nullptr;
	const DATA_JSON_VALUE* pSourceRow = nullptr;
	size_t iAuditionRows = 0u;
	size_t iSourceRows = 0u;
	for (const DATA_JSON_VALUE& Row : pAuditionEffects->Get_Array())
	{
		if (!Row.Is_Object())
			continue;
		const DATA_JSON_VALUE* pAssetId = Row.Find("effectAssetId");
		if (nullptr == pAssetId || !pAssetId->Is_String())
			continue;
		if (pAssetId->Get_String() == Expected.strEffectAssetId)
		{
			pAuditionRow = &Row;
			++iAuditionRows;
		}
	}
	for (const DATA_JSON_VALUE& Row : pProductEffects->Get_Array())
	{
		if (!Row.Is_Object())
			continue;
		const DATA_JSON_VALUE* pAssetId = Row.Find("effectAssetId");
		if (nullptr == pAssetId || !pAssetId->Is_String())
			continue;
		if (pAssetId->Get_String() == Expected.strSourceEffectAssetId)
		{
			pSourceRow = &Row;
			++iSourceRows;
		}
		if (pAssetId->Get_String() == Expected.strEffectAssetId)
			++iAuditionRows;
	}
	for (const DATA_JSON_VALUE& Row : pAuditionEffects->Get_Array())
	{
		if (!Row.Is_Object())
			continue;
		const DATA_JSON_VALUE* pAssetId = Row.Find("effectAssetId");
		if (nullptr != pAssetId && pAssetId->Is_String() &&
			pAssetId->Get_String() == Expected.strSourceEffectAssetId)
		{
			++iSourceRows;
		}
	}
	if (1u != iAuditionRows || nullptr == pAuditionRow ||
		1u != iSourceRows || nullptr == pSourceRow)
	{
		strOutStatus =
			"Registry-bound audition row or its ordinary Product source row disappeared, crossed registries, or became ambiguous after open.";
		return false;
	}

	const std::vector<std::string>& AuditionKeys =
		pAuditionRow->Get_ObjectInsertionOrder();
	const bool_t bAuditionShape =
		AuditionKeys == std::vector<std::string>{
			"effectAssetId", "payloadKind", "authoringPath",
			"runtimeAdmission", "fidelityClass", "sourceEffectAssetId",
			"sourceDocumentRawSha256" } ||
		AuditionKeys == std::vector<std::string>{
			"effectAssetId", "payloadKind", "authoringPath",
			"screenOverlayPresentationPath", "runtimeAdmission",
			"fidelityClass", "sourceEffectAssetId",
			"sourceDocumentRawSha256" };
	const DATA_JSON_VALUE* pPayloadKind = pAuditionRow->Find("payloadKind");
	const DATA_JSON_VALUE* pAuthoringPath = pAuditionRow->Find("authoringPath");
	const DATA_JSON_VALUE* pRuntimeAdmission =
		pAuditionRow->Find("runtimeAdmission");
	const DATA_JSON_VALUE* pFidelityClass =
		pAuditionRow->Find("fidelityClass");
	const DATA_JSON_VALUE* pSourceEffectAssetId =
		pAuditionRow->Find("sourceEffectAssetId");
	const DATA_JSON_VALUE* pSourceDocumentRawSha256 =
		pAuditionRow->Find("sourceDocumentRawSha256");
	if (!bAuditionShape || nullptr == pPayloadKind ||
		!pPayloadKind->Is_String() ||
		pPayloadKind->Get_String() != "DIRECT_AUTHORED_DOCUMENT" ||
		nullptr == pAuthoringPath || !pAuthoringPath->Is_String() ||
		nullptr == pRuntimeAdmission || !pRuntimeAdmission->Is_String() ||
		pRuntimeAdmission->Get_String() != "REGISTRY_BOUND_AUDITION_ONLY" ||
		nullptr == pFidelityClass || !pFidelityClass->Is_String() ||
		pFidelityClass->Get_String() != "PROJECT_TUNED_APPROX" ||
		nullptr == pSourceEffectAssetId || !pSourceEffectAssetId->Is_String() ||
		pSourceEffectAssetId->Get_String() != Expected.strSourceEffectAssetId ||
		nullptr == pSourceDocumentRawSha256 ||
		!pSourceDocumentRawSha256->Is_String() ||
		pSourceDocumentRawSha256->Get_String() !=
			Expected.strSourceDocumentRawSha256 ||
		!Try_ResolveCatalogAuthoredPath(AuthoredRoot,
			pAuthoringPath->Get_String(), Expected.strEffectAssetId,
			Expected.Path, strOutStatus))
	{
		if (strOutStatus.empty())
		{
			strOutStatus =
				"Registry-bound audition row was reclassified or retargeted after open.";
		}
		return false;
	}

	const std::vector<std::string>& SourceKeys =
		pSourceRow->Get_ObjectInsertionOrder();
	const bool_t bSourceShape =
		SourceKeys == std::vector<std::string>{
			"effectAssetId", "payloadKind", "authoringPath" } ||
		SourceKeys == std::vector<std::string>{
			"effectAssetId", "payloadKind", "authoringPath",
			"screenOverlayPresentationPath" };
	const DATA_JSON_VALUE* pSourcePayloadKind =
		pSourceRow->Find("payloadKind");
	const DATA_JSON_VALUE* pSourceAuthoringPath =
		pSourceRow->Find("authoringPath");
	if (!bSourceShape || nullptr == pSourcePayloadKind ||
		!pSourcePayloadKind->Is_String() ||
		pSourcePayloadKind->Get_String() != "DIRECT_AUTHORED_DOCUMENT" ||
		nullptr == pSourceAuthoringPath || !pSourceAuthoringPath->Is_String() ||
		!Try_ResolveCatalogAuthoredPath(AuthoredRoot,
			pSourceAuthoringPath->Get_String(),
			Expected.strSourceEffectAssetId,
			Expected.SourceDocumentPath, strOutStatus))
	{
		if (strOutStatus.empty())
		{
			strOutStatus =
				"Registry-bound audition ordinary source row was reclassified or retargeted after open.";
		}
		return false;
	}
	strOutStatus.clear();
	return true;
}

bool Client::CEffectDirectAuthoredSourceIndex::Build(
	const std::filesystem::path& CatalogPath,
	const std::filesystem::path& AuditionCatalogPath,
	const std::filesystem::path& AuthoredRoot,
	const std::vector<EFFECT_DIRECT_AUTHORED_SCANNED_FILE>& ScannedFiles,
	const EFFECT_DIRECT_AUTHORED_OWNER_SET& ValidOwners,
	const EFFECT_DIRECT_AUTHORED_BOSS_OWNER_MAP& ValidBossOwners,
	const EFFECT_DIRECT_AUTHORED_BOSS_COMBAT_OBJECT_OWNER_MAP&
		ValidBossCombatObjectOwners,
	EFFECT_DIRECT_AUTHORED_SOURCE_INDEX& InOutIndex,
	std::string& strOutStatus)
{
	std::string Text;
	std::string AuditionText;
	std::string Error;
	if (CatalogPath.empty() || AuditionCatalogPath.empty() ||
		AuthoredRoot.empty() || !Read_TextFile(CatalogPath, Text, Error) ||
		!Read_TextFile(AuditionCatalogPath, AuditionText, Error))
	{
		strOutStatus = CatalogPath.empty() || AuditionCatalogPath.empty() ||
			AuthoredRoot.empty() ?
			"Direct authored source index path is empty." : Error;
		return false;
	}
	std::error_code CanonicalError;
	const std::filesystem::path CanonicalAuthoredRoot =
		std::filesystem::weakly_canonical(AuthoredRoot, CanonicalError);
	if (CanonicalError || CanonicalAuthoredRoot.empty() ||
		!std::filesystem::is_directory(CanonicalAuthoredRoot, CanonicalError) ||
		CanonicalError)
	{
		strOutStatus =
			"Direct authored source index could not canonicalize the Authored root.";
		return false;
	}
	DATA_JSON_VALUE Root;
	DATA_JSON_VALUE AuditionRoot;
	const auto ParseCatalogRoot = [&Error](const std::string& Raw,
		const std::string_view strLabel, DATA_JSON_VALUE& OutRoot,
		const DATA_JSON_VALUE*& pOutEffects, std::string& strStatus)
	{
		if (!CDataJson::Parse(Raw, OutRoot, Error) || !OutRoot.Is_Object())
		{
			strStatus = "Could not parse " + std::string(strLabel) + ": " + Error;
			return false;
		}
		const DATA_JSON_VALUE* pVersion = OutRoot.Find("formatVersion");
		pOutEffects = OutRoot.Find("effects");
		if (OutRoot.Get_Object().size() != 2u ||
			OutRoot.Get_ObjectInsertionOrder() !=
				std::vector<std::string>{ "formatVersion", "effects" } ||
			nullptr == pVersion ||
			pVersion->Get_Type() != DATA_JSON_TYPE::NUMBER ||
			pVersion->Was_FloatingPointToken() ||
			pVersion->Get_Number() != 1.0 || nullptr == pOutEffects ||
			pOutEffects->Get_Type() != DATA_JSON_TYPE::ARRAY)
		{
			strStatus = std::string(strLabel) +
				" rejected the exact source-index root contract.";
			return false;
		}
		return true;
	};
	const DATA_JSON_VALUE* pEffects = nullptr;
	const DATA_JSON_VALUE* pAuditionEffects = nullptr;
	if (!ParseCatalogRoot(Text, "EffectCatalog.json", Root,
			pEffects, strOutStatus) ||
		!ParseCatalogRoot(AuditionText, "EffectAuditionCatalog.json",
			AuditionRoot, pAuditionEffects, strOutStatus))
	{
		return false;
	}
	struct CATALOG_ROW_REF final
	{
		const DATA_JSON_VALUE* pRow = nullptr;
		bool_t bAuditionRegistry = false;
	};
	std::vector<CATALOG_ROW_REF> CatalogRows;
	CatalogRows.reserve(pEffects->Get_Array().size() +
		pAuditionEffects->Get_Array().size());
	for (const DATA_JSON_VALUE& Row : pEffects->Get_Array())
		CatalogRows.push_back({ &Row, false });
	for (const DATA_JSON_VALUE& Row : pAuditionEffects->Get_Array())
		CatalogRows.push_back({ &Row, true });

	std::unordered_map<std::string, const EFFECT_DIRECT_AUTHORED_SCANNED_FILE*>
		ScannedByAssetId;
	ScannedByAssetId.reserve(ScannedFiles.size());
	for (const EFFECT_DIRECT_AUTHORED_SCANNED_FILE& File : ScannedFiles)
		ScannedByAssetId.try_emplace(File.strEffectAssetId, &File);

	EFFECT_DIRECT_AUTHORED_SOURCE_INDEX Staged;
	Staged.Entries.reserve(CatalogRows.size());
	std::unordered_set<std::string> DirectAssetIds;
	DirectAssetIds.reserve(CatalogRows.size());
	std::unordered_set<std::string> DuplicateAssetIds;
	DuplicateAssetIds.reserve(CatalogRows.size());
	std::unordered_set<std::string> NonAuditionDirectAssetIds;
	NonAuditionDirectAssetIds.reserve(pEffects->Get_Array().size());
	for (const CATALOG_ROW_REF& RowRef : CatalogRows)
	{
		if (RowRef.bAuditionRegistry)
			continue;
		const DATA_JSON_VALUE& CatalogEntry = *RowRef.pRow;
		if (!CatalogEntry.Is_Object())
			continue;
		const DATA_JSON_VALUE* pPayloadKind = CatalogEntry.Find("payloadKind");
		const DATA_JSON_VALUE* pAssetId = CatalogEntry.Find("effectAssetId");
		if (nullptr != pPayloadKind &&
			pPayloadKind->Get_Type() == DATA_JSON_TYPE::STRING &&
			pPayloadKind->Get_String() == "DIRECT_AUTHORED_DOCUMENT" &&
			nullptr != pAssetId &&
			pAssetId->Get_Type() == DATA_JSON_TYPE::STRING &&
			!pAssetId->Get_String().empty() &&
			nullptr == CatalogEntry.Find("runtimeAdmission"))
		{
			NonAuditionDirectAssetIds.emplace(pAssetId->Get_String());
		}
	}
	const auto RecordUnavailable = [&Staged](std::string Status)
	{
		++Staged.iUnavailableCount;
		if (Staged.strFirstUnavailable.empty())
			Staged.strFirstUnavailable = std::move(Status);
	};
	const auto RecordOwnerJoinUnavailable = [&Staged](std::string Status)
	{
		++Staged.iOwnerJoinUnavailableCount;
		if (Staged.strFirstOwnerJoinUnavailable.empty())
			Staged.strFirstOwnerJoinUnavailable = std::move(Status);
	};

	for (const CATALOG_ROW_REF& RowRef : CatalogRows)
	{
		const DATA_JSON_VALUE& CatalogEntry = *RowRef.pRow;
		if (!CatalogEntry.Is_Object())
		{
			RecordUnavailable(RowRef.bAuditionRegistry ?
				"EffectAuditionCatalog.json contains a non-object row." :
				"EffectCatalog.json contains a non-object row.");
			continue;
		}
		const DATA_JSON_VALUE* pPayloadKind = CatalogEntry.Find("payloadKind");
		if (nullptr == pPayloadKind)
			continue;
		if (pPayloadKind->Get_Type() != DATA_JSON_TYPE::STRING)
		{
			RecordUnavailable(
				"EffectCatalog.json contains a non-string payloadKind.");
			continue;
		}
		if (pPayloadKind->Get_String() != "DIRECT_AUTHORED_DOCUMENT")
			continue;
		++Staged.iCatalogDirectCount;
		const DATA_JSON_VALUE* pAssetId = CatalogEntry.Find("effectAssetId");
		const DATA_JSON_VALUE* pAuthoringPath = CatalogEntry.Find("authoringPath");
		const DATA_JSON_VALUE* pScreenOverlayPresentationPath =
			CatalogEntry.Find("screenOverlayPresentationPath");
		const DATA_JSON_VALUE* pRuntimeAdmission =
			CatalogEntry.Find("runtimeAdmission");
		const DATA_JSON_VALUE* pFidelityClass =
			CatalogEntry.Find("fidelityClass");
		const DATA_JSON_VALUE* pSourceEffectAssetId =
			CatalogEntry.Find("sourceEffectAssetId");
		const DATA_JSON_VALUE* pSourceDocumentRawSha256 =
			CatalogEntry.Find("sourceDocumentRawSha256");
		const bool_t bOptionalScreenOverlayPathValid =
			nullptr == pScreenOverlayPresentationPath ||
			(pScreenOverlayPresentationPath->Get_Type() == DATA_JSON_TYPE::STRING &&
			 !pScreenOverlayPresentationPath->Get_String().empty() &&
			 pScreenOverlayPresentationPath->Get_String().starts_with(
				 "Effects/ScreenOverlays/") &&
			 pScreenOverlayPresentationPath->Get_String().ends_with(
				 ".screen-overlay.json"));
		const bool_t bOrdinaryShape =
			(CatalogEntry.Get_Object().size() == 3u &&
			 nullptr == pScreenOverlayPresentationPath) ||
			(CatalogEntry.Get_Object().size() == 4u &&
			 nullptr != pScreenOverlayPresentationPath);
		const bool_t bAuditionShape =
			((CatalogEntry.Get_Object().size() == 7u &&
			  nullptr == pScreenOverlayPresentationPath) ||
			 (CatalogEntry.Get_Object().size() == 8u &&
			  nullptr != pScreenOverlayPresentationPath)) &&
			nullptr != pRuntimeAdmission && nullptr != pFidelityClass &&
			nullptr != pSourceEffectAssetId &&
			nullptr != pSourceDocumentRawSha256;
		const bool_t bAuditionIdentityValid = !bAuditionShape ||
			(pRuntimeAdmission->Get_Type() == DATA_JSON_TYPE::STRING &&
			 pRuntimeAdmission->Get_String() == "REGISTRY_BOUND_AUDITION_ONLY" &&
			 pFidelityClass->Get_Type() == DATA_JSON_TYPE::STRING &&
			 pFidelityClass->Get_String() == "PROJECT_TUNED_APPROX" &&
			 pSourceEffectAssetId->Get_Type() == DATA_JSON_TYPE::STRING &&
			 !pSourceEffectAssetId->Get_String().empty() &&
			 pSourceDocumentRawSha256->Get_Type() == DATA_JSON_TYPE::STRING &&
			 Is_LowerHexSha256(pSourceDocumentRawSha256->Get_String()));
		const bool_t bRegistryShapeValid = RowRef.bAuditionRegistry ?
			bAuditionShape : bOrdinaryShape;
		if (!bRegistryShapeValid || nullptr == pAssetId ||
			pAssetId->Get_Type() != DATA_JSON_TYPE::STRING ||
			pAssetId->Get_String().empty() || nullptr == pAuthoringPath ||
			pAuthoringPath->Get_Type() != DATA_JSON_TYPE::STRING ||
			pAuthoringPath->Get_String().empty() ||
			!bOptionalScreenOverlayPathValid || !bAuditionIdentityValid)
		{
			RecordUnavailable(
				RowRef.bAuditionRegistry ?
				"EffectAuditionCatalog.json contains a malformed audition row." :
				"EffectCatalog.json contains a malformed direct-authored row.");
			continue;
		}
		const std::string strAssetId = pAssetId->Get_String();
		if (!DirectAssetIds.insert(strAssetId).second)
		{
			if (DuplicateAssetIds.emplace(strAssetId).second)
			{
				RecordUnavailable(
					"EffectCatalog.json invalidated the first direct-authored row for duplicate Effect ID: " +
					strAssetId);
			}
			RecordUnavailable(
				"EffectCatalog.json contains duplicate Effect ID: " + strAssetId);
			continue;
		}

		EFFECT_DIRECT_AUTHORED_SOURCE_ENTRY Entry;
		Entry.strEffectAssetId = strAssetId;
		if (bAuditionShape)
		{
			Entry.bRegistryBoundAuditionOnly = true;
			Entry.strSourceEffectAssetId =
				pSourceEffectAssetId->Get_String();
			Entry.strSourceDocumentRawSha256 =
				pSourceDocumentRawSha256->Get_String();
		}
		if (Try_ParsePlayerOwner(
				strAssetId, Entry.eCharacterClass, Entry.iSkillId))
		{
			if (ValidOwners.contains(
					std::make_pair(Entry.eCharacterClass, Entry.iSkillId)))
			{
				Entry.eOwnerKind =
					EFFECT_DIRECT_AUTHORED_OWNER_KIND::PLAYER_SKILL;
			}
			else
			{
				RecordOwnerJoinUnavailable(
					"direct authored Effect ID has no PlayerSkills owner: " +
					strAssetId);
			}
		}
		else
		{
			const auto BossOwner = ValidBossOwners.find(strAssetId);
			const auto CombatObjectOwner =
				ValidBossCombatObjectOwners.find(strAssetId);
			const bool_t hasBossPatternOwner =
				ValidBossOwners.end() != BossOwner;
			const bool_t hasBossCombatObjectOwner =
				ValidBossCombatObjectOwners.end() != CombatObjectOwner;
			if (hasBossPatternOwner && hasBossCombatObjectOwner)
			{
				RecordOwnerJoinUnavailable(
					"direct authored Effect ID has ambiguous boss-pattern and boss-combat-object owners: " +
					strAssetId);
			}
			else if (hasBossPatternOwner &&
				!BossOwner->second.strOwnerArchetypeId.empty() &&
				!BossOwner->second.strPatternId.empty() &&
				!BossOwner->second.strStageId.empty() &&
				!BossOwner->second.strActionId.empty())
			{
				Entry.eOwnerKind = EFFECT_DIRECT_AUTHORED_OWNER_KIND::BOSS_PATTERN;
				Entry.strOwnerArchetypeId =
					BossOwner->second.strOwnerArchetypeId;
				Entry.strPatternId = BossOwner->second.strPatternId;
				Entry.strStageId = BossOwner->second.strStageId;
				Entry.strActionId = BossOwner->second.strActionId;
			}
			else if (hasBossCombatObjectOwner &&
				!CombatObjectOwner->second.strOwnerArchetypeId.empty() &&
				!CombatObjectOwner->second.strCombatObjectArchetypeId.empty() &&
				!CombatObjectOwner->second.strClientVisualId.empty())
			{
				Entry.eOwnerKind =
					EFFECT_DIRECT_AUTHORED_OWNER_KIND::BOSS_COMBAT_OBJECT;
				Entry.strOwnerArchetypeId =
					CombatObjectOwner->second.strOwnerArchetypeId;
				Entry.strCombatObjectArchetypeId =
					CombatObjectOwner->second.strCombatObjectArchetypeId;
				Entry.strClientVisualId =
					CombatObjectOwner->second.strClientVisualId;
			}
			else
			{
				RecordOwnerJoinUnavailable(
					"direct authored Effect ID has no stable player-skill, boss-pattern, or boss-combat-object owner: " +
					strAssetId);
			}
		}
		if (bAuditionShape &&
			EFFECT_DIRECT_AUTHORED_OWNER_KIND::END != Entry.eOwnerKind)
		{
			const std::string& strSourceEffectAssetId =
				pSourceEffectAssetId->Get_String();
			bool_t bSameOwner = strSourceEffectAssetId != strAssetId &&
				NonAuditionDirectAssetIds.contains(strSourceEffectAssetId);
			if (bSameOwner && Entry.eOwnerKind ==
					EFFECT_DIRECT_AUTHORED_OWNER_KIND::PLAYER_SKILL)
			{
				LostArk::Shared::CHARACTER_CLASS_ID eSourceClass =
					LostArk::Shared::CHARACTER_CLASS_ID::END;
				LostArk::Shared::SKILL_ID iSourceSkillId =
					LostArk::Shared::INVALID_SKILL_ID;
				bSameOwner = Try_ParsePlayerOwner(strSourceEffectAssetId,
					eSourceClass, iSourceSkillId) &&
					eSourceClass == Entry.eCharacterClass &&
					iSourceSkillId == Entry.iSkillId;
			}
			else if (bSameOwner && Entry.eOwnerKind ==
					EFFECT_DIRECT_AUTHORED_OWNER_KIND::BOSS_PATTERN)
			{
				const auto SourceOwner = ValidBossOwners.find(
					strSourceEffectAssetId);
				bSameOwner = SourceOwner != ValidBossOwners.end() &&
					SourceOwner->second.strOwnerArchetypeId ==
						Entry.strOwnerArchetypeId &&
					SourceOwner->second.strPatternId == Entry.strPatternId &&
					SourceOwner->second.strStageId == Entry.strStageId &&
					SourceOwner->second.strActionId == Entry.strActionId;
			}
			else if (bSameOwner && Entry.eOwnerKind ==
					EFFECT_DIRECT_AUTHORED_OWNER_KIND::BOSS_COMBAT_OBJECT)
			{
				const auto SourceOwner = ValidBossCombatObjectOwners.find(
					strSourceEffectAssetId);
				bSameOwner = SourceOwner !=
						ValidBossCombatObjectOwners.end() &&
					SourceOwner->second.strOwnerArchetypeId ==
						Entry.strOwnerArchetypeId &&
					SourceOwner->second.strCombatObjectArchetypeId ==
						Entry.strCombatObjectArchetypeId &&
					SourceOwner->second.strClientVisualId ==
						Entry.strClientVisualId;
			}
			if (!bSameOwner)
			{
				RecordOwnerJoinUnavailable(
					"registry-bound audition source owner mismatch: " +
					strAssetId);
				Entry.eOwnerKind = EFFECT_DIRECT_AUTHORED_OWNER_KIND::END;
				Entry.strOwnerArchetypeId.clear();
				Entry.strPatternId.clear();
				Entry.strStageId.clear();
				Entry.strActionId.clear();
				Entry.strCombatObjectArchetypeId.clear();
				Entry.strClientVisualId.clear();
			}
		}

		const std::string strAuthoringPath = pAuthoringPath->Get_String();
		if (strAuthoringPath.find('\\') != std::string::npos ||
			strAuthoringPath.find(':') != std::string::npos ||
			strAuthoringPath.starts_with('/') ||
			strAuthoringPath.ends_with('/') ||
			strAuthoringPath.find("//") != std::string::npos)
		{
			RecordUnavailable("unsafe direct authoringPath: " + strAuthoringPath);
			continue;
		}
		const std::filesystem::path RelativePath(strAuthoringPath);
		if (RelativePath.empty() || RelativePath.is_absolute() ||
			RelativePath.has_root_path() ||
			RelativePath.generic_string() != strAuthoringPath)
		{
			RecordUnavailable(
				"noncanonical direct authoringPath: " + strAuthoringPath);
			continue;
		}
		std::vector<std::string> Components;
		for (const std::filesystem::path& Component : RelativePath)
		{
			const std::string Token = Component.generic_string();
			if (Token.empty() || Token == "." || Token == "..")
			{
				Components.clear();
				break;
			}
			Components.push_back(Token);
		}
		std::string strDerivedAssetId;
		if (Components.size() < 3u || Components[0] != "Effects" ||
			Components[1] != "Authored" ||
			!Try_DeriveAssetId(RelativePath, strDerivedAssetId) ||
			strDerivedAssetId != strAssetId)
		{
			RecordUnavailable(
				"direct authoringPath/Effect ID mismatch: " + strAssetId);
			continue;
		}
		std::filesystem::path ExpectedPath = CanonicalAuthoredRoot;
		for (size_t iComponent = 2u; iComponent < Components.size();
			++iComponent)
		{
			ExpectedPath /= Components[iComponent];
		}
		std::error_code FileError;
		FileError.clear();
		ExpectedPath =
			std::filesystem::weakly_canonical(ExpectedPath, FileError);
		if (FileError || ExpectedPath.empty() ||
			!Is_StrictDescendant(CanonicalAuthoredRoot, ExpectedPath) ||
			!std::filesystem::is_regular_file(ExpectedPath, FileError) ||
			FileError)
		{
			RecordUnavailable(
				"direct authoringPath escaped Data/Effects/Authored: " + strAssetId);
			continue;
		}
		const auto Scanned = ScannedByAssetId.find(strAssetId);
		if (Scanned == ScannedByAssetId.end())
		{
			RecordUnavailable(
				"direct authored file is absent from the Authored scan: " +
				strAssetId);
			continue;
		}
		FileError.clear();
		if (!std::filesystem::equivalent(
				ExpectedPath, Scanned->second->Path, FileError) || FileError)
		{
			RecordUnavailable(
				"direct authoringPath disagrees with the Authored scan: " +
				strAssetId);
			continue;
		}
		Entry.LastWriteTime =
			std::filesystem::last_write_time(ExpectedPath, FileError);
		if (FileError)
		{
			RecordUnavailable(
				"direct authored timestamp is unavailable: " + strAssetId);
			continue;
		}
		Entry.iFileSize = static_cast<uint64_t>(
			std::filesystem::file_size(ExpectedPath, FileError));
		if (FileError)
		{
			RecordUnavailable(
				"direct authored file size is unavailable: " + strAssetId);
			continue;
		}
		Entry.Path = ExpectedPath;
		Staged.Entries.push_back(std::move(Entry));
	}

	std::erase_if(Staged.Entries,
		[&DuplicateAssetIds](const EFFECT_DIRECT_AUTHORED_SOURCE_ENTRY& Entry)
		{
			return DuplicateAssetIds.contains(Entry.strEffectAssetId);
		});
	std::unordered_map<std::string, std::filesystem::path>
		OrdinarySourcePaths;
	OrdinarySourcePaths.reserve(Staged.Entries.size());
	for (const EFFECT_DIRECT_AUTHORED_SOURCE_ENTRY& Entry : Staged.Entries)
	{
		if (!Entry.bRegistryBoundAuditionOnly)
			OrdinarySourcePaths.emplace(Entry.strEffectAssetId, Entry.Path);
	}
	for (EFFECT_DIRECT_AUTHORED_SOURCE_ENTRY& Entry : Staged.Entries)
	{
		if (!Entry.bRegistryBoundAuditionOnly)
			continue;
		const auto Source = OrdinarySourcePaths.find(
			Entry.strSourceEffectAssetId);
		std::string SourceBytes;
		std::string FreshnessStatus;
		if (Source == OrdinarySourcePaths.end())
		{
			Entry.bAuditionSourceFreshnessValid = false;
			RecordUnavailable(
				"registry-bound audition source is not an admitted ordinary row: " +
				Entry.strEffectAssetId);
			continue;
		}
		Entry.SourceDocumentPath = Source->second;
		if (!Read_BoundedBinaryFile(Entry.SourceDocumentPath,
				64u * 1024u * 1024u, SourceBytes, FreshnessStatus) ||
			Compute_Sha256Hex(SourceBytes) !=
				Entry.strSourceDocumentRawSha256)
		{
			Entry.bAuditionSourceFreshnessValid = false;
			RecordUnavailable(
				"registry-bound audition source hash is stale: " +
				Entry.strEffectAssetId);
		}
	}
	std::sort(Staged.Entries.begin(), Staged.Entries.end(),
		[](const EFFECT_DIRECT_AUTHORED_SOURCE_ENTRY& Left,
			const EFFECT_DIRECT_AUTHORED_SOURCE_ENTRY& Right)
		{
			return std::tie(Left.eOwnerKind, Left.eCharacterClass,
				Left.iSkillId, Left.strOwnerArchetypeId, Left.strPatternId,
				Left.strStageId, Left.strActionId,
				Left.strCombatObjectArchetypeId, Left.strClientVisualId,
				Left.strEffectAssetId) <
				std::tie(Right.eOwnerKind, Right.eCharacterClass,
					Right.iSkillId, Right.strOwnerArchetypeId,
					Right.strPatternId, Right.strStageId,
					Right.strActionId,
					Right.strCombatObjectArchetypeId,
					Right.strClientVisualId, Right.strEffectAssetId);
		});
	strOutStatus = "Direct authored source index admitted " +
		std::to_string(Staged.Entries.size()) + " / " +
		std::to_string(Staged.iCatalogDirectCount) +
		" DIRECT_AUTHORED_DOCUMENT source paths.";
	if (0u != Staged.iUnavailableCount)
	{
		strOutStatus += " Isolated or locked " +
			std::to_string(Staged.iUnavailableCount) +
			" unavailable rows; first: " + Staged.strFirstUnavailable;
	}
	if (0u != Staged.iOwnerJoinUnavailableCount)
	{
		strOutStatus += " Product owner join isolated for " +
			std::to_string(Staged.iOwnerJoinUnavailableCount) +
			" editor-ready rows; first: " +
			Staged.strFirstOwnerJoinUnavailable;
	}
	InOutIndex = std::move(Staged);
	return true;
}
