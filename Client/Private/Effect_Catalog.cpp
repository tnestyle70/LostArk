#include "Effect_Catalog.h"

#include "DataJson.h"
#include "Effect_DocumentCodec.h"
#include "RuntimeAssetRoot.h"

#include <algorithm>
#include <array>
#include <bcrypt.h>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <map>
#include <unordered_set>

#pragma comment(lib, "bcrypt.lib")

namespace
{
    std::map<std::string,
        std::shared_ptr<const Client::EFFECT_DOCUMENT_DESC>,
        std::less<>> g_Effects;
    uint64_t g_iRuntimeRevision = 0u;
    std::string g_strStatus = "Effect catalog has not been loaded.";

    std::filesystem::path Get_ModuleDirectory()
    {
        wchar_t Buffer[32768]{};
        const DWORD Length = GetModuleFileNameW(
            nullptr, Buffer, static_cast<DWORD>(std::size(Buffer)));
        if (0u == Length || Length >= std::size(Buffer))
            return {};
        return std::filesystem::path(Buffer).parent_path();
    }

    std::filesystem::path Find_RuntimeCatalog()
    {
        const std::filesystem::path Module = Get_ModuleDirectory();
        const std::filesystem::path Adjacent =
            Module / L"DataFiles" / L"Effect" / L"EffectCatalog.runtime.json";
        if (std::filesystem::is_regular_file(Adjacent))
            return Adjacent;
        const std::filesystem::path Parent = Module.parent_path() /
            L"DataFiles" / L"Effect" / L"EffectCatalog.runtime.json";
        return std::filesystem::is_regular_file(Parent) ? Parent : Adjacent;
    }

    const Client::DATA_JSON_VALUE* Required(
        const Client::DATA_JSON_VALUE& Object,
        const char* pName,
        const Client::DATA_JSON_TYPE eType)
    {
        const Client::DATA_JSON_VALUE* pValue = Object.Find(pName);
        return nullptr != pValue && pValue->Get_Type() == eType ?
            pValue : nullptr;
    }

    bool Is_LowerHexSha256(const std::string& Value)
    {
        return 64u == Value.size() && std::all_of(
            Value.begin(), Value.end(), [](const char Character)
            {
                return (Character >= '0' && Character <= '9') ||
                    (Character >= 'a' && Character <= 'f');
            });
    }

	bool Compute_Sha256(
		const std::filesystem::path& Path,
		std::string& OutHex)
	{
		BCRYPT_ALG_HANDLE Algorithm = nullptr;
		BCRYPT_HASH_HANDLE Hash = nullptr;
		DWORD ObjectBytes = 0u;
		DWORD HashBytes = 0u;
		DWORD ResultBytes = 0u;
		if (0 > BCryptOpenAlgorithmProvider(
			&Algorithm, BCRYPT_SHA256_ALGORITHM, nullptr, 0u) ||
			0 > BCryptGetProperty(Algorithm, BCRYPT_OBJECT_LENGTH,
				reinterpret_cast<PUCHAR>(&ObjectBytes), sizeof(ObjectBytes),
				&ResultBytes, 0u) ||
			0 > BCryptGetProperty(Algorithm, BCRYPT_HASH_LENGTH,
				reinterpret_cast<PUCHAR>(&HashBytes), sizeof(HashBytes),
				&ResultBytes, 0u) || 32u != HashBytes)
		{
			if (nullptr != Algorithm)
				BCryptCloseAlgorithmProvider(Algorithm, 0u);
			return false;
		}
		std::vector<UCHAR> Object(ObjectBytes);
		std::array<UCHAR, 32> Digest{};
		if (0 > BCryptCreateHash(Algorithm, &Hash, Object.data(),
			ObjectBytes, nullptr, 0u, 0u))
		{
			BCryptCloseAlgorithmProvider(Algorithm, 0u);
			return false;
		}
		std::ifstream Input(Path, std::ios::binary);
		std::array<char, 64u * 1024u> Buffer{};
		bool Success = static_cast<bool>(Input);
		while (Success && Input.read(Buffer.data(), Buffer.size()) ||
			(Success && Input.gcount() > 0))
		{
			if (0 > BCryptHashData(Hash,
				reinterpret_cast<PUCHAR>(Buffer.data()),
				static_cast<ULONG>(Input.gcount()), 0u))
			{
				Success = false;
			}
			if (!Input)
				break;
		}
		if (Input.bad() || !Success ||
			0 > BCryptFinishHash(Hash, Digest.data(),
				static_cast<ULONG>(Digest.size()), 0u))
		{
			Success = false;
		}
		BCryptDestroyHash(Hash);
		BCryptCloseAlgorithmProvider(Algorithm, 0u);
		if (!Success)
			return false;
		constexpr char Hex[] = "0123456789abcdef";
		OutHex.resize(Digest.size() * 2u);
		for (size_t i = 0u; i < Digest.size(); ++i)
		{
			OutHex[i * 2u] = Hex[Digest[i] >> 4u];
			OutHex[i * 2u + 1u] = Hex[Digest[i] & 0x0fu];
		}
		return true;
	}
}

bool_t Client::CEffectCatalog::Load(std::string& strOutStatus)
{
    const std::filesystem::path Path = Find_RuntimeCatalog();
    std::ifstream Input(Path, std::ios::binary);
    if (!Input)
    {
        strOutStatus = "Missing EffectCatalog.runtime.json: " + Path.string();
        g_strStatus = strOutStatus;
        return false;
    }
    const std::string Text{
        std::istreambuf_iterator<char>(Input),
        std::istreambuf_iterator<char>() };
    DATA_JSON_VALUE Root;
    std::string Error;
    if (!CDataJson::Parse(Text, Root, Error) || !Root.Is_Object())
    {
        strOutStatus = "Effect runtime catalog JSON parse failed: " + Error;
        g_strStatus = strOutStatus;
        return false;
    }
    const DATA_JSON_VALUE* pVersion = Required(
        Root, "formatVersion", DATA_JSON_TYPE::NUMBER);
    const DATA_JSON_VALUE* pEffects = Required(
        Root, "effects", DATA_JSON_TYPE::ARRAY);
    if (nullptr == pVersion || 1.0 != pVersion->Get_Number() ||
        nullptr == pEffects)
    {
        strOutStatus = "Effect runtime catalog must be formatVersion 1 with an effects array.";
        g_strStatus = strOutStatus;
        return false;
    }

    std::map<std::string,
        std::shared_ptr<const EFFECT_DOCUMENT_DESC>, std::less<>> Staged;
    std::map<std::string, std::string, std::less<>> VerifiedHashes;
    for (const DATA_JSON_VALUE& Entry : pEffects->Get_Array())
    {
        if (!Entry.Is_Object())
        {
            strOutStatus = "Effect runtime catalog contains a non-object entry.";
            g_strStatus = strOutStatus;
            return false;
        }
        const DATA_JSON_VALUE* pAssetId = Required(
            Entry, "effectAssetId", DATA_JSON_TYPE::STRING);
        const DATA_JSON_VALUE* pAuthoringVersion = Required(
            Entry, "authoringFormatVersion", DATA_JSON_TYPE::NUMBER);
        const DATA_JSON_VALUE* pContentSha = Required(
            Entry, "contentSha256", DATA_JSON_TYPE::STRING);
        const DATA_JSON_VALUE* pDependencies = Required(
            Entry, "dependencies", DATA_JSON_TYPE::ARRAY);
        const DATA_JSON_VALUE* pDocument = Entry.Find("document");
        const DATA_JSON_VALUE* pEmbeddedVersion =
            nullptr == pDocument ? nullptr : pDocument->Find("version");
        const double AuthoringVersion = nullptr == pAuthoringVersion ?
            0.0 : pAuthoringVersion->Get_Number();
        if (nullptr == pAssetId || pAssetId->Get_String().empty() ||
            nullptr == pAuthoringVersion ||
            AuthoringVersion != std::floor(AuthoringVersion) ||
            AuthoringVersion < EFFECT_AUTHORING_MIN_SUPPORTED_VERSION ||
            AuthoringVersion > EFFECT_AUTHORING_FORMAT_VERSION ||
            nullptr == pContentSha || !Is_LowerHexSha256(pContentSha->Get_String()) ||
            nullptr == pDependencies || nullptr == pDocument || !pDocument->Is_Object() ||
            nullptr == pEmbeddedVersion || !pEmbeddedVersion->Is_Number() ||
            pEmbeddedVersion->Get_Number() != AuthoringVersion)
        {
            strOutStatus = "Effect runtime catalog entry has an invalid header.";
            g_strStatus = strOutStatus;
            return false;
        }
        std::map<std::string, std::string, std::less<>> Dependencies;
        for (const DATA_JSON_VALUE& Dependency : pDependencies->Get_Array())
        {
            const DATA_JSON_VALUE* pDependencyId = Required(
                Dependency, "assetId", DATA_JSON_TYPE::STRING);
            const DATA_JSON_VALUE* pDependencySha = Required(
                Dependency, "sha256", DATA_JSON_TYPE::STRING);
            if (nullptr == pDependencyId ||
                !CEffectDocumentCodec::Is_SafeResourceAssetId(
                    pDependencyId->Get_String()) ||
                nullptr == pDependencySha ||
                !Is_LowerHexSha256(pDependencySha->Get_String()))
            {
                strOutStatus = "Effect runtime catalog has an invalid dependency.";
                g_strStatus = strOutStatus;
                return false;
            }
            if (!Dependencies.emplace(
                pDependencyId->Get_String(),
                pDependencySha->Get_String()).second)
            {
                strOutStatus = "Effect runtime catalog has a duplicate dependency.";
                g_strStatus = strOutStatus;
                return false;
            }
        }

        EFFECT_DOCUMENT_DESC Document;
        if (!CEffectDocumentCodec::Parse_Value(
            *pDocument, Document, Error) ||
            !CEffectDocumentCodec::Validate_Drawable(Document, Error) ||
            Document.strEffectAssetId != pAssetId->Get_String())
        {
            strOutStatus = "Effect runtime document rejected for " +
                pAssetId->Get_String() + ": " + Error;
            g_strStatus = strOutStatus;
            return false;
        }
        std::vector<std::string> DocumentDependencies;
        CEffectDocumentCodec::Collect_ResourceAssetIds(
            Document, DocumentDependencies);
        if (DocumentDependencies.size() != Dependencies.size())
        {
            strOutStatus = "Effect runtime dependency set does not match its embedded Document.";
            g_strStatus = strOutStatus;
            return false;
        }
        for (const std::string& DependencyId : DocumentDependencies)
        {
            const auto Expected = Dependencies.find(DependencyId);
            if (Expected == Dependencies.end())
            {
                strOutStatus = "Effect runtime dependency is missing from the manifest: " +
                    DependencyId;
                g_strStatus = strOutStatus;
                return false;
            }
            auto Actual = VerifiedHashes.find(DependencyId);
            if (Actual == VerifiedHashes.end())
            {
                std::string Hash;
                const std::filesystem::path ResourcePath =
                    CRuntimeAssetRoot::Resolve(DependencyId);
                if (ResourcePath.empty() ||
                    !Compute_Sha256(ResourcePath, Hash))
                {
					strOutStatus = "Effect runtime dependency cannot be hashed: " +
						DependencyId;
					g_strStatus = strOutStatus;
					return false;
				}
				Actual = VerifiedHashes.emplace(DependencyId,
					std::move(Hash)).first;
			}
			if (Actual->second != Expected->second)
			{
				strOutStatus = "Effect runtime dependency hash mismatch: " +
					DependencyId;
				g_strStatus = strOutStatus;
				return false;
			}
		}
        auto Committed = std::make_shared<const EFFECT_DOCUMENT_DESC>(
            std::move(Document));
        if (!Staged.emplace(pAssetId->Get_String(),
            std::move(Committed)).second)
        {
            strOutStatus = "Duplicate EffectAssetId in runtime catalog: " +
                pAssetId->Get_String();
            g_strStatus = strOutStatus;
            return false;
        }
    }

    g_Effects = std::move(Staged);
    ++g_iRuntimeRevision;
    g_strStatus = "Loaded " + std::to_string(g_Effects.size()) +
        " admitted Effect Documents.";
    strOutStatus = g_strStatus;
    return true;
}

std::shared_ptr<const Client::EFFECT_DOCUMENT_DESC>
Client::CEffectCatalog::Find(const std::string& strEffectAssetId)
{
    const auto Iterator = g_Effects.find(strEffectAssetId);
    return g_Effects.end() == Iterator ? nullptr : Iterator->second;
}

bool_t Client::CEffectCatalog::Contains(const std::string& strEffectAssetId)
{
    return g_Effects.contains(strEffectAssetId);
}

std::vector<std::string> Client::CEffectCatalog::Get_EffectAssetIds()
{
    std::vector<std::string> Result;
    Result.reserve(g_Effects.size());
    for (const auto& [AssetId, Document] : g_Effects)
        Result.push_back(AssetId);
    return Result;
}

uint64_t Client::CEffectCatalog::Get_RuntimeRevision()
{
    return g_iRuntimeRevision;
}

const std::string& Client::CEffectCatalog::Get_Status()
{
    return g_strStatus;
}

void Client::CEffectCatalog::Clear()
{
    g_Effects.clear();
    g_strStatus = "Effect catalog cleared.";
}
