#include "Effect_Catalog.h"

#include "DataJson.h"
#include "Effect_DocumentCodec.h"
#include "Effect_MaterialProgramRegistry.h"
#include "Effect_OccurrenceTuning.h"
#include "Effect_ScreenOverlayPresentation.h"
#include "Effect_VisualProgramCorpus.h"
#include "ProjectDataRoot.h"
#include "RuntimeAssetRoot.h"
#include <algorithm>
#include <array>
#include <cfloat>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <map>
#include <set>
#include <string_view>
#include <unordered_set>

namespace
{
	struct DIRECT_AUTHORED_RUNTIME_SOURCE final
	{
		std::filesystem::path DocumentPath;
	};

    std::map<std::string,
        std::shared_ptr<const Client::EFFECT_DOCUMENT_DESC>,
        std::less<>> g_Effects;
	std::map<std::string,
		std::shared_ptr<const DIRECT_AUTHORED_RUNTIME_SOURCE>, std::less<>>
		g_DirectAuthoredSources;
	std::map<std::string,
		std::shared_ptr<const Client::EFFECT_ASSEMBLY_DESC>,
		std::less<>> g_Assemblies;
	std::map<std::string,
		std::shared_ptr<const Client::EFFECT_COMPONENT_DESC>,
		std::less<>> g_Components;
	std::map<std::string,
		std::shared_ptr<const Client::EFFECT_COMPILED_RUNTIME_DOCUMENT>,
		std::less<>> g_RuntimeAuthorities;
	std::map<std::string,
		std::shared_ptr<const Client::EFFECT_RUNTIME_PROGRAM_CATALOG_ENTRY>,
		std::less<>> g_RuntimeProgramEntries;
	std::shared_ptr<const Client::EFFECT_VISUAL_PROGRAM_CORPUS>
		g_pVisualProgramCorpus;
	std::map<std::string,
		std::shared_ptr<const Client::EFFECT_VISUAL_PROGRAM>, std::less<>>
		g_VisualPrograms;
	std::map<std::string,
		std::shared_ptr<const
			Client::EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>, std::less<>>
		g_VisualProjections;
	std::map<std::string,
		std::shared_ptr<const Client::EFFECT_SCREEN_OVERLAY_PRODUCT_BINDING>,
		std::less<>> g_ScreenOverlayProductBindings;
	std::shared_ptr<const Client::CEffectMaterialProgramRegistry>
		g_pMaterialProgramRegistry;
    uint64_t g_iRuntimeRevision = 0u;
	uint32_t g_iCatalogOwnerThreadId = 0u;
    std::string g_strStatus = "Effect catalog has not been loaded.";

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

	bool Resolve_DebugDirectAuthoredDocumentPath(
		const std::string& EffectAssetId,
		const std::filesystem::path& AuthoredPath,
		std::filesystem::path& OutPath,
		std::string& strOutError)
	{
		OutPath.clear();
		if (EffectAssetId.empty() || AuthoredPath.empty() ||
			!AuthoredPath.is_absolute() ||
			AuthoredPath.filename() !=
				std::filesystem::path(EffectAssetId + ".effect.json"))
		{
			strOutError =
				"Debug direct authored replacement path or filename is invalid.";
			return false;
		}

		const std::filesystem::path AuthoredRoot = Client::CProjectDataRoot::Resolve(
			std::filesystem::path(L"Effects") / L"Authored");
		std::error_code Error;
		const std::filesystem::path CanonicalRoot =
			std::filesystem::weakly_canonical(AuthoredRoot, Error);
		if (Error || CanonicalRoot.empty() ||
			!std::filesystem::is_directory(CanonicalRoot, Error) || Error)
		{
			strOutError = "Debug direct authored root is unavailable.";
			return false;
		}
		const std::filesystem::path CanonicalPath =
			std::filesystem::weakly_canonical(AuthoredPath, Error);
		if (Error || CanonicalPath.empty() ||
			!std::filesystem::is_regular_file(CanonicalPath, Error) || Error)
		{
			strOutError = "Debug direct authored document is unavailable.";
			return false;
		}
		const std::filesystem::path ExpectedPath =
			std::filesystem::weakly_canonical(
				CanonicalRoot / (EffectAssetId + ".effect.json"), Error);
		if (Error || ExpectedPath.empty() ||
			!std::filesystem::equivalent(
				CanonicalPath, ExpectedPath, Error) || Error)
		{
			strOutError =
				"Debug direct authored document escaped its exact authoring path.";
			return false;
		}

		OutPath = CanonicalPath;
		strOutError.clear();
		return true;
	}

	bool Validate_DebugCatalogOwnerThread(std::string& strOutError)
	{
		const uint32_t CurrentThreadId =
			static_cast<uint32_t>(GetCurrentThreadId());
		if (0u == g_iCatalogOwnerThreadId ||
			CurrentThreadId != g_iCatalogOwnerThreadId)
		{
			strOutError =
				"Debug direct authored replacement must run on the catalog owner/main thread.";
			return false;
		}
		strOutError.clear();
		return true;
	}

	bool Is_NormalizedOccurrenceTuningSourcePath(const std::string_view Value)
	{
		constexpr std::string_view Prefix = "Effects/AuthoredCorrections/";
		constexpr std::string_view Suffix = ".occurrence-tuning.json";
		if (!Value.starts_with(Prefix) || !Value.ends_with(Suffix) ||
			Value.size() > 1024u || Value.find('\\') != std::string_view::npos ||
			Value.find(':') != std::string_view::npos || Value.starts_with('/') ||
			Value.find("//") != std::string_view::npos ||
			std::any_of(Value.begin(), Value.end(), [](const unsigned char Character)
			{
				return Character < 0x20u || Character == 0x7fu;
			}))
		{
			return false;
		}
		size_t Begin = 0u;
		while (Begin < Value.size())
		{
			const size_t End = Value.find('/', Begin);
			const std::string_view Segment = Value.substr(
				Begin, End == std::string_view::npos ? Value.size() - Begin :
					End - Begin);
			if (Segment.empty() || Segment == "." || Segment == "..")
				return false;
			if (End == std::string_view::npos)
				break;
			Begin = End + 1u;
		}
		return true;
	}

	bool Has_ExactKeys(const Client::DATA_JSON_VALUE& Value,
		const std::initializer_list<std::string_view> Keys)
	{
		if (!Value.Is_Object() || Value.Get_Object().size() != Keys.size())
			return false;
		return std::all_of(Keys.begin(), Keys.end(), [&Value](const auto Key)
		{
			return Value.Get_Object().contains(Key);
		});
	}

	bool Has_ExactOrderedKeys(const Client::DATA_JSON_VALUE& Value,
		const std::initializer_list<std::string_view> Keys)
	{
		if (!Has_ExactKeys(Value, Keys) ||
			Value.Get_ObjectInsertionOrder().size() != Keys.size())
		{
			return false;
		}
		size_t Index = 0u;
		for (const std::string_view Key : Keys)
		{
			if (Value.Get_ObjectInsertionOrder()[Index++] != Key)
				return false;
		}
		return true;
	}

	bool Is_StableId(const std::string& Value)
	{
		return !Value.empty() && Value.size() <= 256u &&
			std::all_of(Value.begin(), Value.end(), [](const char Character)
			{
				return (Character >= 'a' && Character <= 'z') ||
					(Character >= 'A' && Character <= 'Z') ||
					(Character >= '0' && Character <= '9') ||
					Character == '_' || Character == '-' || Character == '.';
			});
	}

	bool Read_U32(const Client::DATA_JSON_VALUE& Object,
		const char* pName, uint32_t& iOutValue);


	bool Read_U64Exact(const Client::DATA_JSON_VALUE& Object,
		const char* pName, uint64_t& iOutValue)
	{
		const Client::DATA_JSON_VALUE* Value = Required(
			Object, pName, Client::DATA_JSON_TYPE::NUMBER);
		if (nullptr == Value || Value->Was_FloatingPointToken() ||
			!std::isfinite(Value->Get_Number()) ||
			Value->Get_Number() != std::floor(Value->Get_Number()) ||
			Value->Get_Number() < 0.0 ||
			Value->Get_Number() > 9007199254740991.0)
		{
			return false;
		}
		iOutValue = static_cast<uint64_t>(Value->Get_Number());
		return true;
	}
	bool Read_StringExact(const Client::DATA_JSON_VALUE& Object,
		const char* pName, std::string& strOutValue,
		bool bAllowEmpty, size_t iMaximumLength);
	bool Read_ShaExact(const Client::DATA_JSON_VALUE& Object,
		const char* pName, std::string& strOutValue);

	bool Read_BoundedSourceFile(const std::filesystem::path& Path,
		const std::uintmax_t iMaximumBytes,
		std::string& strOutBytes,
		std::string& strOutError)
	{
		strOutBytes.clear();
		std::error_code FileError;
		const std::uintmax_t iFileBytes =
			std::filesystem::file_size(Path, FileError);
		if (FileError || 0u == iFileBytes || iFileBytes > iMaximumBytes)
		{
			strOutError =
				"source file is missing, empty, or exceeds its size limit";
			return false;
		}
		std::ifstream Input(Path, std::ios::binary);
		if (!Input)
		{
			strOutError = "source file could not be opened";
			return false;
		}
		strOutBytes.assign(std::istreambuf_iterator<char>(Input),
			std::istreambuf_iterator<char>());
		if (strOutBytes.size() != iFileBytes)
		{
			strOutError = "source file read was incomplete";
			strOutBytes.clear();
			return false;
		}
		strOutError.clear();
		return true;
	}

	bool Stage_SourceScreenOverlayBinding(
		const std::string& EffectAssetId,
		const std::string& RelativePath,
		Client::EFFECT_SCREEN_OVERLAY_PRODUCT_BINDING& OutBinding,
		std::string& strOutError)
	{
		using namespace Client;
		const std::string ExpectedPath =
			"Effects/ScreenOverlays/" + EffectAssetId +
			".screen-overlay.json";
		if (RelativePath != ExpectedPath)
		{
			strOutError =
				"screen-overlay source path does not match its Effect ID";
			return false;
		}
		const std::filesystem::path SourcePath =
			CProjectDataRoot::Resolve(std::filesystem::path(RelativePath));
		std::string Text;
		if (SourcePath.empty() || !Read_BoundedSourceFile(
				SourcePath, 4u * 1024u * 1024u, Text, strOutError))
		{
			if (strOutError.empty())
				strOutError = "screen-overlay source path escaped project Data";
			return false;
		}

		CEffectScreenOverlayPresentation::SOURCE_DOCUMENT_MANIFEST Manifest;
		if (!CEffectScreenOverlayPresentation::Parse_SourceDocument(
				Text, Manifest, strOutError) ||
			Manifest.strPresentationId != EffectAssetId + ".screen-overlay")
		{
			if (strOutError.empty())
				strOutError =
					"screen-overlay source presentation identity mismatches its Effect";
			return false;
		}

		EFFECT_SCREEN_OVERLAY_PRODUCT_BINDING Staged;
		Staged.strEffectAssetId = EffectAssetId;
		Staged.strPresentationId = Manifest.strPresentationId;
		Staged.iByteCount = static_cast<uint64_t>(Text.size());
		Staged.strSha256 =
			CEffectRuntimeAuthorityCodec::Compute_Sha256Hex(Text);
		Staged.strUtf8Json = std::move(Text);
		Staged.Resources.reserve(Manifest.TextureAssetIds.size());
		for (const std::string& ResourceId : Manifest.TextureAssetIds)
		{
			const std::filesystem::path ResourcePath =
				CRuntimeAssetRoot::Resolve(std::filesystem::path(ResourceId));
			std::string ResourceBytes;
			if (ResourcePath.empty() || !Read_BoundedSourceFile(
					ResourcePath, 512u * 1024u * 1024u,
					ResourceBytes, strOutError))
			{
				strOutError = "screen-overlay source resource rejected: " +
					ResourceId + "; " + strOutError;
				return false;
			}
			EFFECT_SCREEN_OVERLAY_RESOURCE_IDENTITY Resource;
			Resource.strAssetId = ResourceId;
			Resource.iByteCount =
				static_cast<uint64_t>(ResourceBytes.size());
			Resource.strSha256 =
				CEffectRuntimeAuthorityCodec::Compute_Sha256Hex(ResourceBytes);
			Staged.Resources.push_back(std::move(Resource));
		}

		OutBinding = std::move(Staged);
		strOutError.clear();
		return true;
	}

	bool Stage_SourceDirectAuthoredCatalog(
		std::map<std::string,
			std::shared_ptr<const DIRECT_AUTHORED_RUNTIME_SOURCE>,
			std::less<>>& OutSources,
		std::map<std::string,
			std::shared_ptr<const
				Client::EFFECT_SCREEN_OVERLAY_PRODUCT_BINDING>,
			std::less<>>& OutScreenOverlayBindings,
		std::string& strOutError)
	{
		using namespace Client;
		const std::filesystem::path CatalogPath = CProjectDataRoot::Resolve(
			std::filesystem::path(L"Effects") / L"EffectCatalog.json");
		const std::filesystem::path AuthoredRoot = CProjectDataRoot::Resolve(
			std::filesystem::path(L"Effects") / L"Authored");
		std::string Text;
		if (CatalogPath.empty() || AuthoredRoot.empty() ||
			!Read_BoundedSourceFile(CatalogPath, 16u * 1024u * 1024u,
				Text, strOutError))
		{
			if (strOutError.empty())
				strOutError = "Effect source catalog path escaped project Data";
			return false;
		}

		DATA_JSON_VALUE Root;
		DATA_JSON_PARSE_LIMITS Limits;
		Limits.iMaximumBytes = 16u * 1024u * 1024u;
		Limits.iMaximumDepth = 16u;
		Limits.iMaximumValues = 100'000u;
		std::string ParseError;
		if (!CDataJson::Parse(Text, Root, ParseError, Limits) ||
			!Has_ExactOrderedKeys(Root, { "formatVersion", "effects" }))
		{
			strOutError = "Effect source catalog JSON is invalid: " + ParseError;
			return false;
		}
		const DATA_JSON_VALUE* pVersion = Required(
			Root, "formatVersion", DATA_JSON_TYPE::NUMBER);
		const DATA_JSON_VALUE* pEffects = Required(
			Root, "effects", DATA_JSON_TYPE::ARRAY);
		if (nullptr == pVersion || pVersion->Was_FloatingPointToken() ||
			pVersion->Get_Number() != 1.0 || nullptr == pEffects)
		{
			strOutError = "Effect source catalog version or effects array is invalid";
			return false;
		}

		std::map<std::string,
			std::shared_ptr<const DIRECT_AUTHORED_RUNTIME_SOURCE>,
			std::less<>> StagedSources;
		std::map<std::string,
			std::shared_ptr<const EFFECT_SCREEN_OVERLAY_PRODUCT_BINDING>,
			std::less<>> StagedScreenOverlayBindings;
		for (const DATA_JSON_VALUE& Entry : pEffects->Get_Array())
		{
			if (!Entry.Is_Object())
			{
				strOutError = "Effect source catalog contains a non-object row";
				return false;
			}
			const DATA_JSON_VALUE* pPayloadKind = Entry.Find("payloadKind");
			if (nullptr == pPayloadKind)
				continue;
			if (!pPayloadKind->Is_String())
			{
				strOutError = "Effect source catalog payloadKind is not a string";
				return false;
			}
			if (pPayloadKind->Get_String() != "DIRECT_AUTHORED_DOCUMENT")
				continue;

			const bool_t bOrdinary = Has_ExactOrderedKeys(Entry, {
				"effectAssetId", "payloadKind", "authoringPath" });
			const bool_t bOrdinaryWithOverlay = Has_ExactOrderedKeys(Entry, {
				"effectAssetId", "payloadKind", "authoringPath",
				"screenOverlayPresentationPath" });
			const bool_t bAudition = Has_ExactOrderedKeys(Entry, {
				"effectAssetId", "payloadKind", "authoringPath",
				"runtimeAdmission", "fidelityClass", "sourceEffectAssetId",
				"sourceDocumentRawSha256" });
			const bool_t bAuditionWithOverlay = Has_ExactOrderedKeys(Entry, {
				"effectAssetId", "payloadKind", "authoringPath",
				"runtimeAdmission", "fidelityClass", "sourceEffectAssetId",
				"sourceDocumentRawSha256",
				"screenOverlayPresentationPath" });
			if (!bOrdinary && !bOrdinaryWithOverlay &&
				!bAudition && !bAuditionWithOverlay)
			{
				strOutError =
					"Effect source catalog direct-authored row shape is invalid";
				return false;
			}

			const DATA_JSON_VALUE* pAssetId = Required(
				Entry, "effectAssetId", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* pAuthoringPath = Required(
				Entry, "authoringPath", DATA_JSON_TYPE::STRING);
			if (nullptr == pAssetId || !Is_StableId(pAssetId->Get_String()) ||
				nullptr == pAuthoringPath ||
				pAuthoringPath->Get_String() != "Effects/Authored/" +
					pAssetId->Get_String() + ".effect.json")
			{
				strOutError =
					"Effect source catalog direct-authored identity/path is invalid";
				return false;
			}
			if (bAudition || bAuditionWithOverlay)
			{
				const DATA_JSON_VALUE* pRuntimeAdmission = Required(
					Entry, "runtimeAdmission", DATA_JSON_TYPE::STRING);
				const DATA_JSON_VALUE* pFidelityClass = Required(
					Entry, "fidelityClass", DATA_JSON_TYPE::STRING);
				const DATA_JSON_VALUE* pSourceAssetId = Required(
					Entry, "sourceEffectAssetId", DATA_JSON_TYPE::STRING);
				const DATA_JSON_VALUE* pSourceSha = Required(
					Entry, "sourceDocumentRawSha256", DATA_JSON_TYPE::STRING);
				if (nullptr == pRuntimeAdmission ||
					pRuntimeAdmission->Get_String() !=
						"REGISTRY_BOUND_AUDITION_ONLY" ||
					nullptr == pFidelityClass ||
					pFidelityClass->Get_String() != "PROJECT_TUNED_APPROX" ||
					nullptr == pSourceAssetId ||
					!Is_StableId(pSourceAssetId->Get_String()) ||
					pSourceAssetId->Get_String() == pAssetId->Get_String() ||
					nullptr == pSourceSha ||
					!Is_LowerHexSha256(pSourceSha->Get_String()))
				{
					strOutError =
						"Effect source catalog audition identity is invalid";
					return false;
				}
			}

			const std::filesystem::path SourcePath = CProjectDataRoot::Resolve(
				std::filesystem::path(pAuthoringPath->Get_String()));
			std::error_code FileError;
			const std::filesystem::path ExpectedPath =
				std::filesystem::weakly_canonical(
					AuthoredRoot /
						(pAssetId->Get_String() + ".effect.json"), FileError);
			if (FileError || SourcePath.empty() || ExpectedPath.empty() ||
				!std::filesystem::equivalent(SourcePath, ExpectedPath, FileError) ||
				FileError || !std::filesystem::is_regular_file(SourcePath, FileError) ||
				FileError)
			{
				strOutError =
					"Effect direct-authored source file is missing or unsafe: " +
					pAssetId->Get_String();
				return false;
			}
			auto Source =
				std::make_shared<DIRECT_AUTHORED_RUNTIME_SOURCE>();
			Source->DocumentPath = SourcePath;
			if (!StagedSources.emplace(
					pAssetId->Get_String(), std::move(Source)).second)
			{
				strOutError = "Duplicate direct-authored source Effect ID: " +
					pAssetId->Get_String();
				return false;
			}

			if (bOrdinaryWithOverlay || bAuditionWithOverlay)
			{
				const DATA_JSON_VALUE* pOverlayPath = Required(Entry,
					"screenOverlayPresentationPath", DATA_JSON_TYPE::STRING);
				EFFECT_SCREEN_OVERLAY_PRODUCT_BINDING Binding;
				if (nullptr == pOverlayPath || !Stage_SourceScreenOverlayBinding(
						pAssetId->Get_String(), pOverlayPath->Get_String(),
						Binding, strOutError) ||
					!StagedScreenOverlayBindings.emplace(
						pAssetId->Get_String(),
						std::make_shared<const
							EFFECT_SCREEN_OVERLAY_PRODUCT_BINDING>(
								std::move(Binding))).second)
				{
					if (strOutError.empty())
						strOutError =
							"Duplicate source screen-overlay Effect ID";
					return false;
				}
			}
		}
		if (StagedSources.empty())
		{
			strOutError =
				"Effect source catalog contains no direct-authored documents";
			return false;
		}

		OutSources = std::move(StagedSources);
		OutScreenOverlayBindings =
			std::move(StagedScreenOverlayBindings);
		strOutError.clear();
		return true;
	}


	bool Read_U32(const Client::DATA_JSON_VALUE& Object,
		const char* pName, uint32_t& iOutValue)
	{
		const Client::DATA_JSON_VALUE* pValue = Required(
			Object, pName, Client::DATA_JSON_TYPE::NUMBER);
		if (nullptr == pValue || !std::isfinite(pValue->Get_Number()) ||
			pValue->Get_Number() != std::floor(pValue->Get_Number()) ||
			pValue->Get_Number() < 0.0 ||
			pValue->Get_Number() > static_cast<double>(UINT32_MAX))
		{
			return false;
		}
		iOutValue = static_cast<uint32_t>(pValue->Get_Number());
		return true;
	}

	bool Read_Float(const Client::DATA_JSON_VALUE& Object,
		const char* pName, f32_t& fOutValue)
	{
		const Client::DATA_JSON_VALUE* pValue = Required(
			Object, pName, Client::DATA_JSON_TYPE::NUMBER);
		if (nullptr == pValue || !std::isfinite(pValue->Get_Number()) ||
			pValue->Get_Number() < -FLT_MAX || pValue->Get_Number() > FLT_MAX)
		{
			return false;
		}
		fOutValue = static_cast<f32_t>(pValue->Get_Number());
		return true;
	}

	bool Read_Float3(const Client::DATA_JSON_VALUE& Object,
		const char* pName, float3_t& vOutValue)
	{
		const Client::DATA_JSON_VALUE* pValue = Required(
			Object, pName, Client::DATA_JSON_TYPE::ARRAY);
		if (nullptr == pValue || 3u != pValue->Get_Array().size())
			return false;
		f32_t Values[3]{};
		for (size_t i = 0u; i < 3u; ++i)
		{
			const Client::DATA_JSON_VALUE& Item = pValue->Get_Array()[i];
			if (!Item.Is_Number() || !std::isfinite(Item.Get_Number()) ||
				Item.Get_Number() < -FLT_MAX || Item.Get_Number() > FLT_MAX)
			{
				return false;
			}
			Values[i] = static_cast<f32_t>(Item.Get_Number());
		}
		vOutValue = { Values[0], Values[1], Values[2] };
		return true;
	}

	bool Parse_Transform(const Client::DATA_JSON_VALUE& Value,
		Client::EFFECT_TRANSFORM_DESC& OutTransform)
	{
		return Value.Is_Object() &&
			Read_Float3(Value, "position", OutTransform.vPosition) &&
			Read_Float3(Value, "rotationDegrees",
				OutTransform.vRotationDegrees) &&
			Read_Float3(Value, "scale", OutTransform.vScale) &&
			OutTransform.vScale.x > 0.f && OutTransform.vScale.y > 0.f &&
			OutTransform.vScale.z > 0.f;
	}

	bool Is_Identity(const Client::EFFECT_TRANSFORM_DESC& Transform)
	{
		return Transform.vPosition.x == 0.f && Transform.vPosition.y == 0.f &&
			Transform.vPosition.z == 0.f &&
			Transform.vRotationDegrees.x == 0.f &&
			Transform.vRotationDegrees.y == 0.f &&
			Transform.vRotationDegrees.z == 0.f &&
			Transform.vScale.x == 1.f && Transform.vScale.y == 1.f &&
			Transform.vScale.z == 1.f;
	}

	bool Parse_Component(const Client::DATA_JSON_VALUE& Value,
		Client::EFFECT_COMPONENT_DESC& OutComponent, std::string& strOutError)
	{
		using namespace Client;
		const DATA_JSON_VALUE* pSchema = Required(
			Value, "schema", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pVersion = Required(
			Value, "version", DATA_JSON_TYPE::NUMBER);
		const DATA_JSON_VALUE* pAssetId = Required(
			Value, "componentAssetId", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pDisplayName = Required(
			Value, "displayName", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pType = Required(
			Value, "componentType", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pSource = Required(
			Value, "source", DATA_JSON_TYPE::OBJECT);
		const DATA_JSON_VALUE* pEmitters = Required(
			Value, "emitters", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* pDocument = Required(
			Value, "document", DATA_JSON_TYPE::OBJECT);
		if (nullptr == pSchema || pSchema->Get_String() !=
				"lostark.effect-component" || nullptr == pVersion ||
			pVersion->Get_Number() != 1.0 || nullptr == pAssetId ||
			!Is_StableId(pAssetId->Get_String()) || nullptr == pDisplayName ||
			pDisplayName->Get_String().empty() || nullptr == pType ||
			pType->Get_String().empty() || nullptr == pSource ||
			nullptr == pEmitters || nullptr == pDocument)
		{
			strOutError = "Effect Component header is invalid.";
			return false;
		}
		EFFECT_COMPONENT_DESC Staged;
		Staged.strComponentAssetId = pAssetId->Get_String();
		Staged.strDisplayName = pDisplayName->Get_String();
		Staged.strComponentType = pType->Get_String();
		const DATA_JSON_VALUE* pSourceEffect = Required(
			*pSource, "effectAssetId", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pGroup = Required(
			*pSource, "groupId", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pNodes = Required(
			*pSource, "sourceNodes", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* pSourceSha = Required(
			*pSource, "sourceElementSha256", DATA_JSON_TYPE::STRING);
		if (nullptr == pSourceEffect || !Is_StableId(pSourceEffect->Get_String()) ||
			nullptr == pGroup || !Is_StableId(pGroup->Get_String()) ||
			nullptr == pNodes || nullptr == pSourceSha ||
			!Is_LowerHexSha256(pSourceSha->Get_String()))
		{
			strOutError = "Effect Component source provenance is invalid.";
			return false;
		}
		Staged.strSourceEffectAssetId = pSourceEffect->Get_String();
		Staged.strSourceGroupId = pGroup->Get_String();
		Staged.strSourceElementSha256 = pSourceSha->Get_String();
		for (const DATA_JSON_VALUE& Node : pNodes->Get_Array())
		{
			if (!Node.Is_String())
			{
				strOutError = "Effect Component source node is invalid.";
				return false;
			}
			Staged.SourceNodes.push_back(Node.Get_String());
		}
		if (!CEffectDocumentCodec::Parse_Value(
			*pDocument, Staged.Document, strOutError) ||
			Staged.Document.strEffectAssetId != Staged.strComponentAssetId)
		{
			return false;
		}
		std::map<std::string, const EFFECT_ELEMENT_DESC*, std::less<>> Elements;
		for (const EFFECT_ELEMENT_DESC& Element : Staged.Document.Elements)
			Elements.emplace(Element.strElementId, &Element);
		if (Elements.size() != Staged.Document.Elements.size() ||
			pEmitters->Get_Array().size() != Elements.size())
		{
			strOutError = "Effect Component Emitter/Element identity is invalid.";
			return false;
		}
		for (const DATA_JSON_VALUE& EmitterValue : pEmitters->Get_Array())
		{
			const DATA_JSON_VALUE* pEmitterId = Required(
				EmitterValue, "emitterId", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* pElementId = Required(
				EmitterValue, "elementId", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* pRenderer = Required(
				EmitterValue, "renderer", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* pVisible = Required(
				EmitterValue, "visible", DATA_JSON_TYPE::BOOLEAN);
			EFFECT_COMPONENT_EMITTER_DESC Emitter;
			if (nullptr == pEmitterId || !Is_StableId(pEmitterId->Get_String()) ||
				nullptr == pElementId || !Is_StableId(pElementId->Get_String()) ||
				nullptr == pRenderer || pRenderer->Get_String().empty() ||
				nullptr == pVisible ||
				!Read_U32(EmitterValue, "sourceElementIndex",
					Emitter.iSourceElementIndex) ||
				!Read_U32(EmitterValue, "resourceBindingCount",
					Emitter.iResourceBindingCount) ||
				!Read_U32(EmitterValue, "moduleCount", Emitter.iModuleCount))
			{
				strOutError = "Effect Component Emitter metadata is invalid.";
				return false;
			}
			const auto ElementIterator = Elements.find(pElementId->Get_String());
			if (Elements.end() == ElementIterator ||
				Emitter.iResourceBindingCount !=
					ElementIterator->second->ResourceBindings.size() ||
				Emitter.iModuleCount !=
					ElementIterator->second->SourceRecipe.Modules.size() ||
				pVisible->Get_Boolean() != ElementIterator->second->bVisible)
			{
				strOutError = "Effect Component Emitter payload does not match its Element.";
				return false;
			}
			Emitter.strEmitterId = pEmitterId->Get_String();
			Emitter.strElementId = pElementId->Get_String();
			Emitter.strRendererType = pRenderer->Get_String();
			Emitter.bVisible = pVisible->Get_Boolean();
			Staged.Emitters.push_back(std::move(Emitter));
		}
		OutComponent = std::move(Staged);
		return true;
	}

	bool Parse_Assembly(const Client::DATA_JSON_VALUE& Value,
		Client::EFFECT_ASSEMBLY_DESC& OutAssembly, std::string& strOutError)
	{
		using namespace Client;
		const DATA_JSON_VALUE* pSchema = Required(
			Value, "schema", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pVersion = Required(
			Value, "version", DATA_JSON_TYPE::NUMBER);
		const DATA_JSON_VALUE* pAssetId = Required(
			Value, "effectAssetId", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pDisplayName = Required(
			Value, "displayName", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pSourceSha = Required(
			Value, "sourceDocumentSha256", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pSourceFileSha = Required(
			Value, "sourceDocumentFileSha256", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pParticleSystem = Required(
			Value, "particleSystem", DATA_JSON_TYPE::OBJECT);
		const DATA_JSON_VALUE* pModelCues = Required(
			Value, "modelCues", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* pComponentCues = Required(
			Value, "componentCues", DATA_JSON_TYPE::ARRAY);
		uint32_t iSourceVersion = 0u;
		if (nullptr == pSchema || pSchema->Get_String() !=
				"lostark.effect-assembly" || nullptr == pVersion ||
			pVersion->Get_Number() != 1.0 || nullptr == pAssetId ||
			!Is_StableId(pAssetId->Get_String()) || nullptr == pDisplayName ||
			pDisplayName->Get_String().empty() || nullptr == pSourceSha ||
			!Is_LowerHexSha256(pSourceSha->Get_String()) ||
			nullptr == pSourceFileSha ||
			!Is_LowerHexSha256(pSourceFileSha->Get_String()) ||
			nullptr == pParticleSystem || nullptr == pModelCues ||
			nullptr == pComponentCues ||
			!Read_U32(Value, "sourceAuthoringVersion", iSourceVersion) ||
			iSourceVersion < EFFECT_AUTHORING_MIN_SUPPORTED_VERSION ||
			iSourceVersion > EFFECT_AUTHORING_FORMAT_VERSION)
		{
			strOutError = "Effect Assembly header is invalid.";
			return false;
		}

		DATA_JSON_VALUE::OBJECT HeaderFields;
		HeaderFields.emplace("schema",
			DATA_JSON_VALUE::String("lostark.effect-authoring"));
		HeaderFields.emplace("version",
			DATA_JSON_VALUE::Number(static_cast<double>(iSourceVersion)));
		HeaderFields.emplace("effectAssetId", *pAssetId);
		HeaderFields.emplace("displayName", *pDisplayName);
		HeaderFields.emplace("particleSystem", *pParticleSystem);
		HeaderFields.emplace("modelCues", *pModelCues);
		HeaderFields.emplace("elements", DATA_JSON_VALUE::Array({}));
		EFFECT_DOCUMENT_DESC HeaderDocument;
		if (!CEffectDocumentCodec::Parse_Value(
			DATA_JSON_VALUE::Object(std::move(HeaderFields)),
			HeaderDocument, strOutError))
		{
			return false;
		}

		EFFECT_ASSEMBLY_DESC Staged;
		Staged.strEffectAssetId = pAssetId->Get_String();
		Staged.strDisplayName = pDisplayName->Get_String();
		Staged.iSourceAuthoringVersion = iSourceVersion;
		Staged.strSourceDocumentSha256 = pSourceSha->Get_String();
		Staged.strSourceDocumentFileSha256 = pSourceFileSha->Get_String();
		Staged.ParticleSystem = HeaderDocument.ParticleSystem;
		Staged.ModelCues = std::move(HeaderDocument.ModelCues);
		std::unordered_set<std::string> CueIds;
		std::unordered_set<std::string> ComponentIds;
		for (const DATA_JSON_VALUE& CueValue : pComponentCues->Get_Array())
		{
			const DATA_JSON_VALUE* pCueId = Required(
				CueValue, "cueId", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* pComponentId = Required(
				CueValue, "componentAssetId", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* pVisible = Required(
				CueValue, "visible", DATA_JSON_TYPE::BOOLEAN);
			const DATA_JSON_VALUE* pAnchor = Required(
				CueValue, "anchor", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* pTransform = Required(
				CueValue, "localTransform", DATA_JSON_TYPE::OBJECT);
			EFFECT_COMPONENT_CUE_DESC Cue;
			if (nullptr == pCueId || !Is_StableId(pCueId->Get_String()) ||
				!CueIds.insert(pCueId->Get_String()).second ||
				nullptr == pComponentId ||
				!Is_StableId(pComponentId->Get_String()) ||
				!ComponentIds.insert(pComponentId->Get_String()).second ||
				nullptr == pVisible || !pVisible->Get_Boolean() ||
				nullptr == pAnchor || pAnchor->Get_String() != "root" ||
				nullptr == pTransform ||
				!Read_Float(CueValue, "startDelaySeconds",
					Cue.fStartDelaySeconds) || Cue.fStartDelaySeconds < 0.f ||
				!Parse_Transform(*pTransform, Cue.LocalTransform) ||
				!Is_Identity(Cue.LocalTransform))
			{
				strOutError = "Effect Assembly Component cue is invalid or requires an unsupported transform.";
				return false;
			}
			Cue.strCueId = pCueId->Get_String();
			Cue.strComponentAssetId = pComponentId->Get_String();
			Cue.bVisible = pVisible->Get_Boolean();
			Cue.strAnchorSlotId = pAnchor->Get_String();
			Staged.ComponentCues.push_back(std::move(Cue));
		}
		if (Staged.ComponentCues.empty())
		{
			strOutError = "Effect Assembly has no Component cues.";
			return false;
		}
		OutAssembly = std::move(Staged);
		return true;
	}

	bool Compile_Assembly(const Client::EFFECT_ASSEMBLY_DESC& Assembly,
		const std::map<std::string,
			std::shared_ptr<const Client::EFFECT_COMPONENT_DESC>,
			std::less<>>& Components,
		Client::EFFECT_DOCUMENT_DESC& OutDocument, std::string& strOutError)
	{
		using namespace Client;
		struct INDEXED_ELEMENT final
		{
			uint32_t iSourceIndex = 0u;
			EFFECT_ELEMENT_DESC Element;
		};
		std::vector<INDEXED_ELEMENT> IndexedElements;
		std::set<uint32_t> SourceIndices;
		std::unordered_set<std::string> ElementIds;
		for (const EFFECT_COMPONENT_CUE_DESC& Cue : Assembly.ComponentCues)
		{
			const auto ComponentIterator = Components.find(Cue.strComponentAssetId);
			if (Components.end() == ComponentIterator ||
				ComponentIterator->second->strSourceEffectAssetId !=
					Assembly.strEffectAssetId)
			{
				strOutError = "Effect Assembly references a missing or foreign Component: " +
					Cue.strComponentAssetId;
				return false;
			}
			const EFFECT_COMPONENT_DESC& Component = *ComponentIterator->second;
			std::map<std::string, const EFFECT_ELEMENT_DESC*, std::less<>> Elements;
			for (const EFFECT_ELEMENT_DESC& Element : Component.Document.Elements)
				Elements.emplace(Element.strElementId, &Element);
			for (const EFFECT_COMPONENT_EMITTER_DESC& Emitter : Component.Emitters)
			{
				const auto ElementIterator = Elements.find(Emitter.strElementId);
				if (Elements.end() == ElementIterator ||
					!SourceIndices.insert(Emitter.iSourceElementIndex).second ||
					!ElementIds.insert(Emitter.strElementId).second)
				{
					strOutError = "Effect Assembly has a duplicate or missing Emitter Element.";
					return false;
				}
				INDEXED_ELEMENT Indexed;
				Indexed.iSourceIndex = Emitter.iSourceElementIndex;
				Indexed.Element = *ElementIterator->second;
				Indexed.Element.Detail.Timing.fStartDelaySeconds +=
					Cue.fStartDelaySeconds;
				if (!std::isfinite(
					Indexed.Element.Detail.Timing.fStartDelaySeconds))
				{
					strOutError = "Effect Assembly produced a non-finite timeline.";
					return false;
				}
				IndexedElements.push_back(std::move(Indexed));
			}
		}
		std::sort(IndexedElements.begin(), IndexedElements.end(),
			[](const INDEXED_ELEMENT& Left, const INDEXED_ELEMENT& Right)
			{
				return Left.iSourceIndex < Right.iSourceIndex;
			});
		EFFECT_DOCUMENT_DESC Staged;
		Staged.iLoadedFormatVersion = Assembly.iSourceAuthoringVersion;
		Staged.strEffectAssetId = Assembly.strEffectAssetId;
		Staged.strDisplayName = Assembly.strDisplayName;
		Staged.ParticleSystem = Assembly.ParticleSystem;
		Staged.ModelCues = Assembly.ModelCues;
		Staged.Elements.reserve(IndexedElements.size());
		for (INDEXED_ELEMENT& Indexed : IndexedElements)
			Staged.Elements.push_back(std::move(Indexed.Element));
		if (!CEffectDocumentCodec::Validate_Drawable(Staged, strOutError))
			return false;
		OutDocument = std::move(Staged);
		return true;
	}

	bool Parse_DirectAuthoredRuntimeDocument(
		const std::string& EffectAssetId,
		const DIRECT_AUTHORED_RUNTIME_SOURCE& Source,
		Client::EFFECT_DOCUMENT_DESC& OutDocument,
		std::string& strOutError)
	{
		using namespace Client;
		constexpr std::uintmax_t MaximumDocumentBytes =
			16u * 1024u * 1024u;
		std::error_code FileError;
		const std::uintmax_t FileBytes = std::filesystem::file_size(
			Source.DocumentPath, FileError);
		if (FileError || 0u == FileBytes || FileBytes > MaximumDocumentBytes)
		{
			strOutError =
				"authored source document is missing, empty, or exceeds 16 MiB";
			return false;
		}

		std::ifstream Input(Source.DocumentPath, std::ios::binary);
		if (!Input)
		{
			strOutError = "authored source document could not be opened";
			return false;
		}
		const std::string Text{
			std::istreambuf_iterator<char>(Input),
			std::istreambuf_iterator<char>() };
		if (Text.size() != FileBytes)
		{
			strOutError = "authored source document read was incomplete";
			return false;
		}

		EFFECT_DOCUMENT_DESC Document;
		if (!CEffectDocumentCodec::Parse(Text, Document, strOutError) ||
			Document.bSourceContract ||
			(Document.iLoadedFormatVersion != EFFECT_AUTHORING_FORMAT_VERSION &&
			 Document.iLoadedFormatVersion !=
				EFFECT_AUTHORED_RUNTIME_EXTENSION_FORMAT_VERSION) ||
			Document.strEffectAssetId != EffectAssetId)
		{
			if (strOutError.empty())
			{
				strOutError =
					"authored source document identity or drawable admission mismatched";
			}
			return false;
		}

		OutDocument = std::move(Document);
		strOutError.clear();
		return true;
	}

	std::shared_ptr<const Client::EFFECT_DOCUMENT_DESC>
		Load_DirectAuthoredRuntimeDocument(
			const std::string& EffectAssetId,
			std::string& strOutError)
	{
		using namespace Client;
		const auto Existing = g_Effects.find(EffectAssetId);
		if (Existing != g_Effects.end())
		{
			strOutError.clear();
			return Existing->second;
		}
		const auto SourceIterator = g_DirectAuthoredSources.find(EffectAssetId);
		if (SourceIterator == g_DirectAuthoredSources.end() ||
			nullptr == SourceIterator->second)
		{
			strOutError = "direct authored runtime metadata is absent";
			return nullptr;
		}

		EFFECT_DOCUMENT_DESC Document;
		if (!Parse_DirectAuthoredRuntimeDocument(
				EffectAssetId, *SourceIterator->second, Document, strOutError))
		{
			return nullptr;
		}
		std::shared_ptr<const EFFECT_DOCUMENT_DESC> Committed =
			std::make_shared<const EFFECT_DOCUMENT_DESC>(std::move(Document));
		std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>
			SourceProjection;

		if (Committed->iLoadedFormatVersion ==
			EFFECT_AUTHORED_RUNTIME_EXTENSION_FORMAT_VERSION)
		{
			if (!CEffectVisualProgramCorpusCodec::
					Create_DocumentOwnedRuntimeProjection(
						Committed, SourceProjection, strOutError) ||
				nullptr == SourceProjection || !SourceProjection->Is_Valid() ||
				SourceProjection->Get_EffectAssetId() != EffectAssetId ||
				SourceProjection->Get_ProjectionKind() !=
					EFFECT_VISUAL_PROGRAM_PROJECTION_KIND::ADAPTER_PACKET_V1 ||
				SourceProjection->Get_DocumentShared().get() != Committed.get())
			{
				if (strOutError.empty())
					strOutError =
						"source-owned runtime projection is invalid";
				return nullptr;
			}
		}

		const auto [Iterator, Inserted] =
			g_Effects.emplace(EffectAssetId, std::move(Committed));
		if (!Inserted)
		{
			strOutError = "direct authored runtime cache identity is duplicate";
			return nullptr;
		}
		if (nullptr != SourceProjection && !g_VisualProjections.emplace(
				EffectAssetId, std::move(SourceProjection)).second)
		{
			g_Effects.erase(Iterator);
			strOutError =
				"source-owned runtime projection identity is duplicate";
			return nullptr;
		}
		strOutError.clear();
		return Iterator->second;
	}
}

bool_t Client::CEffectCatalog::Load(std::string& strOutStatus)
{
	struct STATUS_ROLLBACK_GUARD final
	{
		std::string& Target;
		std::string Previous;
		bool_t bCommitted = false;
		~STATUS_ROLLBACK_GUARD()
		{
			if (!bCommitted)
				Target = std::move(Previous);
		}
	} StatusGuard{ g_strStatus, g_strStatus };

	if (UINT64_MAX == g_iRuntimeRevision)
	{
		strOutStatus = "Effect source catalog revision is exhausted.";
		g_strStatus = strOutStatus;
		return false;
	}
	const uint64_t iStagedCatalogRevision = g_iRuntimeRevision + 1u;
	std::map<std::string,
		std::shared_ptr<const DIRECT_AUTHORED_RUNTIME_SOURCE>, std::less<>>
		StagedDirectAuthoredSources;
	std::map<std::string,
		std::shared_ptr<const EFFECT_SCREEN_OVERLAY_PRODUCT_BINDING>,
		std::less<>> StagedScreenOverlayProductBindings;
	std::string Error;
	if (!Stage_SourceDirectAuthoredCatalog(
			StagedDirectAuthoredSources,
			StagedScreenOverlayProductBindings, Error))
	{
		strOutStatus = "Effect source catalog rejected: " + Error;
		g_strStatus = strOutStatus;
		return false;
	}

	std::shared_ptr<const CEffectMaterialProgramRegistry>
		StagedMaterialProgramRegistry =
			CEffectMaterialProgramRegistry::Create_Empty(
				iStagedCatalogRevision, iStagedCatalogRevision, Error);
	if (nullptr == StagedMaterialProgramRegistry)
	{
		strOutStatus =
			"Empty Effect material registry could not be created: " + Error;
		g_strStatus = strOutStatus;
		return false;
	}
	std::shared_ptr<const EFFECT_VISUAL_PROGRAM_CORPUS>
		StagedVisualProgramCorpus =
			std::make_shared<const EFFECT_VISUAL_PROGRAM_CORPUS>();

	/* Direct-authored source files are the Product authority. Generated copies
	   and document-replacing projections are deliberately absent: an element's
	   saved inline authored packet is what the next lookup and next occurrence
	   consume. */
	g_Effects.clear();
	g_DirectAuthoredSources = std::move(StagedDirectAuthoredSources);
	g_Assemblies.clear();
	g_Components.clear();
	g_RuntimeAuthorities.clear();
	g_RuntimeProgramEntries.clear();
	g_pVisualProgramCorpus = std::move(StagedVisualProgramCorpus);
	g_VisualPrograms.clear();
	g_VisualProjections.clear();
	g_ScreenOverlayProductBindings =
		std::move(StagedScreenOverlayProductBindings);
	g_pMaterialProgramRegistry = std::move(StagedMaterialProgramRegistry);
	g_iRuntimeRevision = iStagedCatalogRevision;
	g_iCatalogOwnerThreadId = static_cast<uint32_t>(GetCurrentThreadId());
	g_strStatus = "Loaded " +
		std::to_string(g_DirectAuthoredSources.size()) +
		" direct-authored Effect source documents from "
		"Data/Effects/Authored (pending first use=" +
		std::to_string(g_DirectAuthoredSources.size()) +
		", material bindings=0, visual projections=0).";
	strOutStatus = g_strStatus;
	StatusGuard.bCommitted = true;
	return true;
}

bool_t Client::CEffectCatalog::Stage_DebugDirectAuthoredReplacement(
	const std::string& strEffectAssetId,
	const std::filesystem::path& AuthoredPath,
	std::shared_ptr<const EFFECT_DEBUG_DIRECT_AUTHORED_REPLACEMENT>& OutCandidate,
	std::string& strOutStatus)
{
	OutCandidate.reset();
#if !defined(_DEBUG)
	(void)strEffectAssetId;
	(void)AuthoredPath;
	strOutStatus =
		"Direct authored runtime replacement is available only in a Debug Client.";
	return false;
#else
	if (!Validate_DebugCatalogOwnerThread(strOutStatus) ||
		0u == g_iRuntimeRevision || strEffectAssetId.empty())
	{
		if (strOutStatus.empty())
			strOutStatus = "Effect source catalog is not loaded.";
		return false;
	}

	const auto SourceIterator =
		g_DirectAuthoredSources.find(strEffectAssetId);
	if (SourceIterator == g_DirectAuthoredSources.end() ||
		nullptr == SourceIterator->second ||
		g_Assemblies.contains(strEffectAssetId) ||
		g_RuntimeAuthorities.contains(strEffectAssetId) ||
		g_RuntimeProgramEntries.contains(strEffectAssetId) ||
		Is_ReconstructedRuntimeProgramAssetId(strEffectAssetId))
	{
		strOutStatus =
			"Debug replacement target is not one direct-authored Product Effect.";
		return false;
	}

	std::filesystem::path CanonicalAuthoredPath;
	if (!Resolve_DebugDirectAuthoredDocumentPath(
			strEffectAssetId, AuthoredPath, CanonicalAuthoredPath, strOutStatus))
	{
		return false;
	}

	DIRECT_AUTHORED_RUNTIME_SOURCE CandidateSource;
	CandidateSource.DocumentPath = std::move(CanonicalAuthoredPath);
	EFFECT_DOCUMENT_DESC ParsedDocument;
	std::string Error;
	if (!Parse_DirectAuthoredRuntimeDocument(
			strEffectAssetId, CandidateSource, ParsedDocument, Error))
	{
		strOutStatus = "Debug direct authored document rejected: " + Error;
		return false;
	}

	std::shared_ptr<const EFFECT_DOCUMENT_DESC> StagedDocument =
		std::make_shared<const EFFECT_DOCUMENT_DESC>(std::move(ParsedDocument));
	std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>
		StagedProjection;
	if (StagedDocument->iLoadedFormatVersion ==
		EFFECT_AUTHORED_RUNTIME_EXTENSION_FORMAT_VERSION)
	{
		if (!CEffectVisualProgramCorpusCodec::
				Create_DocumentOwnedRuntimeProjection(
					StagedDocument, StagedProjection, Error) ||
			nullptr == StagedProjection ||
			!StagedProjection->Is_Valid() ||
			StagedProjection->Get_EffectAssetId() != strEffectAssetId ||
			StagedProjection->Get_ProjectionKind() !=
				EFFECT_VISUAL_PROGRAM_PROJECTION_KIND::ADAPTER_PACKET_V1 ||
			StagedProjection->Get_DocumentShared().get() !=
				StagedDocument.get())
		{
			strOutStatus =
				"Debug source-owned runtime projection rejected: " +
				(Error.empty() ? std::string("invalid projection identity") : Error);
			return false;
		}
	}

	const auto PreviousDocument = g_Effects.find(strEffectAssetId);
	const auto PreviousProjection = g_VisualProjections.find(strEffectAssetId);
	const bool_t bHadPreviousDocument = PreviousDocument != g_Effects.end();
	const bool_t bHadPreviousProjection =
		PreviousProjection != g_VisualProjections.end();
	const bool_t bPreviousDocumentRequiresProjection =
		bHadPreviousDocument && nullptr != PreviousDocument->second &&
		PreviousDocument->second->iLoadedFormatVersion ==
			EFFECT_AUTHORED_RUNTIME_EXTENSION_FORMAT_VERSION;
	if (bHadPreviousProjection != bPreviousDocumentRequiresProjection)
	{
		strOutStatus =
			"Loaded direct-authored document/projection state is inconsistent.";
		return false;
	}

	std::shared_ptr<EFFECT_DEBUG_DIRECT_AUTHORED_REPLACEMENT> Staged(
		new EFFECT_DEBUG_DIRECT_AUTHORED_REPLACEMENT());
	Staged->m_strEffectAssetId = strEffectAssetId;
	Staged->m_iRuntimeRevision = g_iRuntimeRevision;
	Staged->m_iStagingThreadId =
		static_cast<uint32_t>(GetCurrentThreadId());
	Staged->m_ExpectedAuthoredSourcePath =
		SourceIterator->second->DocumentPath;
	Staged->m_bHadPreviousDocument = bHadPreviousDocument;
	Staged->m_bHadPreviousVisualProjection = bHadPreviousProjection;
	if (bHadPreviousDocument)
		Staged->m_pPreviousDocument = PreviousDocument->second;
	if (bHadPreviousProjection)
		Staged->m_pPreviousVisualProjection = PreviousProjection->second;
	Staged->m_pDocument = std::move(StagedDocument);
	Staged->m_pVisualProjection = std::move(StagedProjection);
	Staged->m_strPreviousCatalogStatus = g_strStatus;
	OutCandidate = std::move(Staged);
	strOutStatus = "Staged direct-authored runtime replacement for " +
		strEffectAssetId + " at existing catalog revision " +
		std::to_string(g_iRuntimeRevision) + ".";
	return true;
#endif
}

bool_t Client::CEffectCatalog::Commit_DebugDirectAuthoredReplacement(
	const std::shared_ptr<const EFFECT_DEBUG_DIRECT_AUTHORED_REPLACEMENT>&
		pCandidate,
	std::string& strOutStatus)
{
#if !defined(_DEBUG)
	(void)pCandidate;
	strOutStatus =
		"Direct authored runtime replacement is available only in a Debug Client.";
	return false;
#else
	if (!Validate_DebugCatalogOwnerThread(strOutStatus) ||
		nullptr == pCandidate || nullptr == pCandidate->m_pDocument ||
		pCandidate->m_strEffectAssetId.empty() ||
		pCandidate->m_iStagingThreadId !=
			static_cast<uint32_t>(GetCurrentThreadId()) ||
		pCandidate->m_iRuntimeRevision != g_iRuntimeRevision ||
		pCandidate->m_pDocument->strEffectAssetId !=
			pCandidate->m_strEffectAssetId)
	{
		if (strOutStatus.empty())
			strOutStatus =
				"Debug direct authored replacement candidate is stale or invalid.";
		return false;
	}

	const std::string& EffectAssetId = pCandidate->m_strEffectAssetId;
	const auto SourceIterator = g_DirectAuthoredSources.find(EffectAssetId);
	if (SourceIterator == g_DirectAuthoredSources.end() ||
		nullptr == SourceIterator->second ||
		SourceIterator->second->DocumentPath !=
			pCandidate->m_ExpectedAuthoredSourcePath)
	{
		strOutStatus =
			"Debug direct authored replacement source metadata changed after staging.";
		return false;
	}

	const auto CurrentDocument = g_Effects.find(EffectAssetId);
	const auto CurrentProjection = g_VisualProjections.find(EffectAssetId);
	const bool_t bHasCurrentDocument = CurrentDocument != g_Effects.end();
	const bool_t bHasCurrentProjection =
		CurrentProjection != g_VisualProjections.end();
	if (bHasCurrentDocument != pCandidate->m_bHadPreviousDocument ||
		(bHasCurrentDocument && CurrentDocument->second.get() !=
			pCandidate->m_pPreviousDocument.get()) ||
		bHasCurrentProjection !=
			pCandidate->m_bHadPreviousVisualProjection ||
		(bHasCurrentProjection && CurrentProjection->second.get() !=
			pCandidate->m_pPreviousVisualProjection.get()))
	{
		strOutStatus =
			"Debug direct authored runtime pointers changed after staging.";
		return false;
	}

	if (pCandidate->m_bHadPreviousDocument)
		CurrentDocument->second = pCandidate->m_pDocument;
	else if (!g_Effects.emplace(
			EffectAssetId, pCandidate->m_pDocument).second)
	{
		strOutStatus =
			"Debug direct authored document commit identity became duplicate.";
		return false;
	}

	if (nullptr != pCandidate->m_pVisualProjection)
	{
		if (pCandidate->m_bHadPreviousVisualProjection)
			CurrentProjection->second = pCandidate->m_pVisualProjection;
		else if (!g_VisualProjections.emplace(
				EffectAssetId, pCandidate->m_pVisualProjection).second)
		{
			if (pCandidate->m_bHadPreviousDocument)
				g_Effects.find(EffectAssetId)->second =
					pCandidate->m_pPreviousDocument;
			else
				g_Effects.erase(EffectAssetId);
			strOutStatus =
				"Debug direct authored projection commit identity became duplicate.";
			return false;
		}
	}
	else if (pCandidate->m_bHadPreviousVisualProjection)
	{
		g_VisualProjections.erase(CurrentProjection);
	}

	g_strStatus = "Debug hot-reloaded direct-authored Effect " +
		EffectAssetId + " at unchanged catalog revision " +
		std::to_string(g_iRuntimeRevision) +
		"; active occurrences retained their previous document.";
	strOutStatus = g_strStatus;
	return true;
#endif
}

bool_t Client::CEffectCatalog::Restore_DebugDirectAuthoredReplacement(
	const std::shared_ptr<const EFFECT_DEBUG_DIRECT_AUTHORED_REPLACEMENT>&
		pCandidate,
	std::string& strOutStatus)
{
#if !defined(_DEBUG)
	(void)pCandidate;
	strOutStatus =
		"Direct authored runtime replacement is available only in a Debug Client.";
	return false;
#else
	if (!Validate_DebugCatalogOwnerThread(strOutStatus) ||
		nullptr == pCandidate || nullptr == pCandidate->m_pDocument ||
		pCandidate->m_iStagingThreadId !=
			static_cast<uint32_t>(GetCurrentThreadId()) ||
		pCandidate->m_iRuntimeRevision != g_iRuntimeRevision)
	{
		if (strOutStatus.empty())
			strOutStatus =
				"Debug direct authored restore candidate is stale or invalid.";
		return false;
	}

	const std::string& EffectAssetId = pCandidate->m_strEffectAssetId;
	const auto SourceIterator = g_DirectAuthoredSources.find(EffectAssetId);
	const auto CurrentDocument = g_Effects.find(EffectAssetId);
	const auto CurrentProjection = g_VisualProjections.find(EffectAssetId);
	const bool_t bHasCurrentProjection =
		CurrentProjection != g_VisualProjections.end();
	if (SourceIterator == g_DirectAuthoredSources.end() ||
		nullptr == SourceIterator->second ||
		SourceIterator->second->DocumentPath !=
			pCandidate->m_ExpectedAuthoredSourcePath ||
		CurrentDocument == g_Effects.end() ||
		CurrentDocument->second.get() != pCandidate->m_pDocument.get() ||
		bHasCurrentProjection !=
			(nullptr != pCandidate->m_pVisualProjection) ||
		(bHasCurrentProjection && CurrentProjection->second.get() !=
			pCandidate->m_pVisualProjection.get()))
	{
		strOutStatus =
			"Debug direct authored replacement cannot restore changed runtime pointers.";
		return false;
	}

	if (pCandidate->m_bHadPreviousDocument)
		CurrentDocument->second = pCandidate->m_pPreviousDocument;
	else
		g_Effects.erase(CurrentDocument);

	if (pCandidate->m_bHadPreviousVisualProjection)
	{
		if (bHasCurrentProjection)
			CurrentProjection->second =
				pCandidate->m_pPreviousVisualProjection;
		else if (!g_VisualProjections.emplace(
				EffectAssetId,
				pCandidate->m_pPreviousVisualProjection).second)
		{
			if (pCandidate->m_bHadPreviousDocument)
				g_Effects.find(EffectAssetId)->second = pCandidate->m_pDocument;
			else
				g_Effects.emplace(EffectAssetId, pCandidate->m_pDocument);
			strOutStatus =
				"Debug direct authored previous projection restore became duplicate.";
			return false;
		}
	}
	else if (bHasCurrentProjection)
	{
		g_VisualProjections.erase(CurrentProjection);
	}

	g_strStatus = pCandidate->m_strPreviousCatalogStatus;
	strOutStatus = "Restored previous direct-authored runtime pointers for " +
		EffectAssetId + " at catalog revision " +
		std::to_string(g_iRuntimeRevision) + ".";
	return true;
#endif
}

bool_t Client::CEffectCatalog::Stage_DebugDirectAuthoredRegistration(
	const std::string& strEffectAssetId,
	const std::filesystem::path& AuthoredPath,
	std::shared_ptr<const EFFECT_DEBUG_DIRECT_AUTHORED_REGISTRATION>&
		OutCandidate,
	std::string& strOutStatus)
{
	OutCandidate.reset();
#if !defined(_DEBUG)
	(void)strEffectAssetId;
	(void)AuthoredPath;
	strOutStatus =
		"Direct authored runtime registration is available only in a Debug Client.";
	return false;
#else
	if (!Validate_DebugCatalogOwnerThread(strOutStatus) ||
		0u == g_iRuntimeRevision || !Is_StableId(strEffectAssetId))
	{
		if (strOutStatus.empty())
			strOutStatus = "Effect source catalog is not loaded.";
		return false;
	}
	if (g_DirectAuthoredSources.contains(strEffectAssetId) ||
		g_Effects.contains(strEffectAssetId) ||
		g_Assemblies.contains(strEffectAssetId) ||
		g_Components.contains(strEffectAssetId) ||
		g_RuntimeAuthorities.contains(strEffectAssetId) ||
		g_RuntimeProgramEntries.contains(strEffectAssetId) ||
		g_VisualPrograms.contains(strEffectAssetId) ||
		g_VisualProjections.contains(strEffectAssetId) ||
		g_ScreenOverlayProductBindings.contains(strEffectAssetId) ||
		Is_ReconstructedRuntimeProgramAssetId(strEffectAssetId))
	{
		strOutStatus =
			"Debug direct-authored registration target already exists in the loaded catalog.";
		return false;
	}

	std::filesystem::path CanonicalAuthoredPath;
	if (!Resolve_DebugDirectAuthoredDocumentPath(
			strEffectAssetId, AuthoredPath, CanonicalAuthoredPath, strOutStatus))
	{
		return false;
	}

	const std::filesystem::path CatalogPath = CProjectDataRoot::Resolve(
		std::filesystem::path(L"Effects") / L"EffectCatalog.json");
	std::string CatalogRawBefore;
	std::string Error;
	if (CatalogPath.empty() || !Read_BoundedSourceFile(
			CatalogPath, 16u * 1024u * 1024u,
			CatalogRawBefore, Error))
	{
		strOutStatus = "Debug source catalog read failed: " + Error;
		return false;
	}

	std::map<std::string,
		std::shared_ptr<const DIRECT_AUTHORED_RUNTIME_SOURCE>, std::less<>>
		StagedSources;
	std::map<std::string,
		std::shared_ptr<const EFFECT_SCREEN_OVERLAY_PRODUCT_BINDING>,
		std::less<>> StagedScreenOverlayBindings;
	if (!Stage_SourceDirectAuthoredCatalog(
			StagedSources, StagedScreenOverlayBindings, Error))
	{
		strOutStatus =
			"Debug registration source catalog rejected: " + Error;
		return false;
	}
	std::string CatalogRawAfter;
	if (!Read_BoundedSourceFile(CatalogPath, 16u * 1024u * 1024u,
			CatalogRawAfter, Error) || CatalogRawAfter != CatalogRawBefore)
	{
		strOutStatus =
			"EffectCatalog changed while the registration candidate was staged.";
		return false;
	}
	if (StagedSources.size() != g_DirectAuthoredSources.size() + 1u)
	{
		strOutStatus =
			"Debug registration requires EffectCatalog to add exactly one direct-authored row.";
		return false;
	}
	const auto StagedTarget = StagedSources.find(strEffectAssetId);
	if (StagedTarget == StagedSources.end() ||
		nullptr == StagedTarget->second ||
		StagedTarget->second->DocumentPath != CanonicalAuthoredPath)
	{
		strOutStatus =
			"Debug registration target path is not the one staged by EffectCatalog.";
		return false;
	}
	for (const auto& [ExistingId, ExistingSource] :
		g_DirectAuthoredSources)
	{
		const auto CandidateSource = StagedSources.find(ExistingId);
		if (nullptr == ExistingSource ||
			CandidateSource == StagedSources.end() ||
			nullptr == CandidateSource->second ||
			CandidateSource->second->DocumentPath !=
				ExistingSource->DocumentPath)
		{
			strOutStatus =
				"Debug registration refuses a changed existing direct-authored row: " +
				ExistingId;
			return false;
		}
	}
	const auto ScreenOverlayEquals = [](
		const EFFECT_SCREEN_OVERLAY_PRODUCT_BINDING& Left,
		const EFFECT_SCREEN_OVERLAY_PRODUCT_BINDING& Right)
	{
		if (Left.iByteCount != Right.iByteCount ||
			Left.strEffectAssetId != Right.strEffectAssetId ||
			Left.strPresentationId != Right.strPresentationId ||
			Left.strSha256 != Right.strSha256 ||
			Left.strUtf8Json != Right.strUtf8Json ||
			Left.Resources.size() != Right.Resources.size())
		{
			return false;
		}
		for (size_t iResource = 0u;
			iResource < Left.Resources.size(); ++iResource)
		{
			const auto& LeftResource = Left.Resources[iResource];
			const auto& RightResource = Right.Resources[iResource];
			if (LeftResource.iByteCount != RightResource.iByteCount ||
				LeftResource.strAssetId != RightResource.strAssetId ||
				LeftResource.strSha256 != RightResource.strSha256)
			{
				return false;
			}
		}
		return true;
	};
	if (StagedScreenOverlayBindings.size() !=
		g_ScreenOverlayProductBindings.size() ||
		StagedScreenOverlayBindings.contains(strEffectAssetId))
	{
		strOutStatus =
			"Debug registration cannot add or remove screen-overlay ownership.";
		return false;
	}
	for (const auto& [ExistingId, ExistingBinding] :
		g_ScreenOverlayProductBindings)
	{
		const auto CandidateBinding =
			StagedScreenOverlayBindings.find(ExistingId);
		if (nullptr == ExistingBinding ||
			CandidateBinding == StagedScreenOverlayBindings.end() ||
			nullptr == CandidateBinding->second ||
			!ScreenOverlayEquals(
				*ExistingBinding, *CandidateBinding->second))
		{
			strOutStatus =
				"Debug registration refuses changed screen-overlay ownership: " +
				ExistingId;
			return false;
		}
	}

	EFFECT_DOCUMENT_DESC ParsedDocument;
	if (!Parse_DirectAuthoredRuntimeDocument(
			strEffectAssetId, *StagedTarget->second, ParsedDocument, Error))
	{
		strOutStatus =
			"Debug direct-authored registration document rejected: " + Error;
		return false;
	}
	std::shared_ptr<const EFFECT_DOCUMENT_DESC> StagedDocument =
		std::make_shared<const EFFECT_DOCUMENT_DESC>(
			std::move(ParsedDocument));
	std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>
		StagedProjection;
	if (StagedDocument->iLoadedFormatVersion ==
		EFFECT_AUTHORED_RUNTIME_EXTENSION_FORMAT_VERSION)
	{
		if (!CEffectVisualProgramCorpusCodec::
				Create_DocumentOwnedRuntimeProjection(
					StagedDocument, StagedProjection, Error) ||
			nullptr == StagedProjection || !StagedProjection->Is_Valid() ||
			StagedProjection->Get_EffectAssetId() != strEffectAssetId ||
			StagedProjection->Get_ProjectionKind() !=
				EFFECT_VISUAL_PROGRAM_PROJECTION_KIND::ADAPTER_PACKET_V1 ||
			StagedProjection->Get_DocumentShared().get() !=
				StagedDocument.get())
		{
			strOutStatus =
				"Debug registration source-owned projection rejected: " +
				(Error.empty() ? std::string("invalid projection identity") :
					Error);
			return false;
		}
	}

	std::shared_ptr<EFFECT_DEBUG_DIRECT_AUTHORED_REGISTRATION> Staged(
		new EFFECT_DEBUG_DIRECT_AUTHORED_REGISTRATION());
	Staged->m_strEffectAssetId = strEffectAssetId;
	Staged->m_iRuntimeRevision = g_iRuntimeRevision;
	Staged->m_iStagingThreadId =
		static_cast<uint32_t>(GetCurrentThreadId());
	Staged->m_ExpectedAuthoredSourcePath =
		std::move(CanonicalAuthoredPath);
	Staged->m_strExpectedSourceCatalogRawBytes =
		std::move(CatalogRawAfter);
	Staged->m_pDocument = std::move(StagedDocument);
	Staged->m_pVisualProjection = std::move(StagedProjection);
	Staged->m_strPreviousCatalogStatus = g_strStatus;
	OutCandidate = std::move(Staged);
	strOutStatus = "Staged one direct-authored catalog registration for " +
		strEffectAssetId + " at existing catalog revision " +
		std::to_string(g_iRuntimeRevision) + ".";
	return true;
#endif
}

bool_t Client::CEffectCatalog::Commit_DebugDirectAuthoredRegistration(
	const std::shared_ptr<const EFFECT_DEBUG_DIRECT_AUTHORED_REGISTRATION>&
		pCandidate,
	std::string& strOutStatus)
{
#if !defined(_DEBUG)
	(void)pCandidate;
	strOutStatus =
		"Direct authored runtime registration is available only in a Debug Client.";
	return false;
#else
	if (!Validate_DebugCatalogOwnerThread(strOutStatus) ||
		nullptr == pCandidate || nullptr == pCandidate->m_pDocument ||
		pCandidate->m_strEffectAssetId.empty() ||
		pCandidate->m_iStagingThreadId !=
			static_cast<uint32_t>(GetCurrentThreadId()) ||
		pCandidate->m_iRuntimeRevision != g_iRuntimeRevision ||
		pCandidate->m_pDocument->strEffectAssetId !=
			pCandidate->m_strEffectAssetId)
	{
		if (strOutStatus.empty())
			strOutStatus =
				"Debug direct-authored registration candidate is stale or invalid.";
		return false;
	}
	const std::string& EffectAssetId = pCandidate->m_strEffectAssetId;
	if (g_DirectAuthoredSources.contains(EffectAssetId) ||
		g_Effects.contains(EffectAssetId) ||
		g_Assemblies.contains(EffectAssetId) ||
		g_Components.contains(EffectAssetId) ||
		g_RuntimeAuthorities.contains(EffectAssetId) ||
		g_RuntimeProgramEntries.contains(EffectAssetId) ||
		g_VisualPrograms.contains(EffectAssetId) ||
		g_VisualProjections.contains(EffectAssetId) ||
		g_ScreenOverlayProductBindings.contains(EffectAssetId))
	{
		strOutStatus =
			"Debug direct-authored registration identity became duplicate.";
		return false;
	}
	const std::filesystem::path CatalogPath = CProjectDataRoot::Resolve(
		std::filesystem::path(L"Effects") / L"EffectCatalog.json");
	std::string CurrentCatalogRaw;
	std::string Error;
	if (CatalogPath.empty() || !Read_BoundedSourceFile(
			CatalogPath, 16u * 1024u * 1024u,
			CurrentCatalogRaw, Error) ||
		CurrentCatalogRaw != pCandidate->m_strExpectedSourceCatalogRawBytes)
	{
		strOutStatus =
			"EffectCatalog changed after direct-authored registration staging.";
		return false;
	}

	auto Source = std::make_shared<DIRECT_AUTHORED_RUNTIME_SOURCE>();
	Source->DocumentPath = pCandidate->m_ExpectedAuthoredSourcePath;
	if (!g_DirectAuthoredSources.emplace(
			EffectAssetId, std::move(Source)).second)
	{
		strOutStatus =
			"Debug direct-authored source registration became duplicate.";
		return false;
	}
	if (!g_Effects.emplace(EffectAssetId, pCandidate->m_pDocument).second)
	{
		g_DirectAuthoredSources.erase(EffectAssetId);
		strOutStatus =
			"Debug direct-authored document registration became duplicate.";
		return false;
	}
	if (nullptr != pCandidate->m_pVisualProjection &&
		!g_VisualProjections.emplace(
			EffectAssetId, pCandidate->m_pVisualProjection).second)
	{
		g_Effects.erase(EffectAssetId);
		g_DirectAuthoredSources.erase(EffectAssetId);
		strOutStatus =
			"Debug direct-authored projection registration became duplicate.";
		return false;
	}

	g_strStatus = "Debug registered direct-authored Effect " +
		EffectAssetId + " at unchanged catalog revision " +
		std::to_string(g_iRuntimeRevision) +
		"; current-session Product prewarm is still required.";
	strOutStatus = g_strStatus;
	return true;
#endif
}

bool_t Client::CEffectCatalog::Restore_DebugDirectAuthoredRegistration(
	const std::shared_ptr<const EFFECT_DEBUG_DIRECT_AUTHORED_REGISTRATION>&
		pCandidate,
	std::string& strOutStatus)
{
#if !defined(_DEBUG)
	(void)pCandidate;
	strOutStatus =
		"Direct authored runtime registration is available only in a Debug Client.";
	return false;
#else
	if (!Validate_DebugCatalogOwnerThread(strOutStatus) ||
		nullptr == pCandidate || nullptr == pCandidate->m_pDocument ||
		pCandidate->m_iStagingThreadId !=
			static_cast<uint32_t>(GetCurrentThreadId()) ||
		pCandidate->m_iRuntimeRevision != g_iRuntimeRevision)
	{
		if (strOutStatus.empty())
			strOutStatus =
				"Debug direct-authored registration restore candidate is stale or invalid.";
		return false;
	}
	const std::string& EffectAssetId = pCandidate->m_strEffectAssetId;
	const auto Source = g_DirectAuthoredSources.find(EffectAssetId);
	const auto Document = g_Effects.find(EffectAssetId);
	const auto Projection = g_VisualProjections.find(EffectAssetId);
	const bool_t bHasProjection = Projection != g_VisualProjections.end();
	if (Source == g_DirectAuthoredSources.end() ||
		nullptr == Source->second ||
		Source->second->DocumentPath !=
			pCandidate->m_ExpectedAuthoredSourcePath ||
		Document == g_Effects.end() ||
		Document->second.get() != pCandidate->m_pDocument.get() ||
		bHasProjection != (nullptr != pCandidate->m_pVisualProjection) ||
		(bHasProjection && Projection->second.get() !=
			pCandidate->m_pVisualProjection.get()))
	{
		strOutStatus =
			"Debug direct-authored registration cannot remove changed runtime pointers.";
		return false;
	}
	if (bHasProjection)
		g_VisualProjections.erase(Projection);
	g_Effects.erase(Document);
	g_DirectAuthoredSources.erase(Source);
	g_strStatus = pCandidate->m_strPreviousCatalogStatus;
	strOutStatus = "Removed the staged direct-authored registration for " +
		EffectAssetId + " at catalog revision " +
		std::to_string(g_iRuntimeRevision) + ".";
	return true;
#endif
}

std::shared_ptr<const Client::EFFECT_DOCUMENT_DESC>
Client::CEffectCatalog::Find(const std::string& strEffectAssetId)
{
	if (Is_ReconstructedRuntimeProgramAssetId(strEffectAssetId))
		return nullptr;
    const auto Iterator = g_Effects.find(strEffectAssetId);
	if (g_Effects.end() != Iterator)
		return Iterator->second;
	std::string Error;
	const std::shared_ptr<const EFFECT_DOCUMENT_DESC> Loaded =
		Load_DirectAuthoredRuntimeDocument(strEffectAssetId, Error);
	if (nullptr == Loaded && !Error.empty())
	{
		g_strStatus = "Effect direct authored first-use load failed for " +
			strEffectAssetId + ": " + Error;
		OutputDebugStringA((g_strStatus + "\n").c_str());
	}
	return Loaded;
}

std::shared_ptr<const Client::EFFECT_DOCUMENT_DESC>
Client::CEffectCatalog::Find_Loaded(const std::string& strEffectAssetId)
{
	if (Is_ReconstructedRuntimeProgramAssetId(strEffectAssetId))
		return nullptr;
	const auto Iterator = g_Effects.find(strEffectAssetId);
	return g_Effects.end() == Iterator ? nullptr : Iterator->second;
}

std::shared_ptr<const Client::EFFECT_ASSEMBLY_DESC>
Client::CEffectCatalog::Find_Assembly(const std::string& strEffectAssetId)
{
	if (Is_ReconstructedRuntimeProgramAssetId(strEffectAssetId))
		return nullptr;
	const auto Iterator = g_Assemblies.find(strEffectAssetId);
	return g_Assemblies.end() == Iterator ? nullptr : Iterator->second;
}

std::shared_ptr<const Client::EFFECT_COMPONENT_DESC>
Client::CEffectCatalog::Find_Component(const std::string& strComponentAssetId)
{
	const auto Iterator = g_Components.find(strComponentAssetId);
	return g_Components.end() == Iterator ? nullptr : Iterator->second;
}

std::shared_ptr<const Client::EFFECT_COMPILED_RUNTIME_DOCUMENT>
Client::CEffectCatalog::Find_RuntimeAuthority(
	const std::string& strEffectAssetId)
{
	if (Is_ReconstructedRuntimeProgramAssetId(strEffectAssetId))
		return nullptr;
	const auto Iterator = g_RuntimeAuthorities.find(strEffectAssetId);
	return g_RuntimeAuthorities.end() == Iterator ? nullptr : Iterator->second;
}

std::shared_ptr<const Client::EFFECT_RUNTIME_PROGRAM_CATALOG_ENTRY>
Client::CEffectCatalog::Find_RuntimeProgramEntry(
	const std::string& strEffectAssetId)
{
	const auto Iterator = g_RuntimeProgramEntries.find(strEffectAssetId);
	return g_RuntimeProgramEntries.end() == Iterator ?
		nullptr : Iterator->second;
}

std::shared_ptr<const Client::EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM>
Client::CEffectCatalog::Find_ReconstructedRuntimeProgram(
	const std::string& strEffectAssetId)
{
	const std::shared_ptr<const EFFECT_RUNTIME_PROGRAM_CATALOG_ENTRY> Entry =
		Find_RuntimeProgramEntry(strEffectAssetId);
	return nullptr == Entry ? nullptr : Entry->Get_Program();
}

std::shared_ptr<const Client::EFFECT_VISUAL_PROGRAM_CORPUS>
Client::CEffectCatalog::Find_VisualProgramCorpus()
{
	return g_pVisualProgramCorpus;
}

std::shared_ptr<const Client::EFFECT_VISUAL_PROGRAM>
Client::CEffectCatalog::Find_VisualProgram(
	const std::string& strEffectAssetId)
{
	const auto Iterator = g_VisualPrograms.find(strEffectAssetId);
	return g_VisualPrograms.end() == Iterator ? nullptr : Iterator->second;
}

std::shared_ptr<const Client::EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>
Client::CEffectCatalog::Find_VisualProjection(
	const std::string& strEffectAssetId)
{
	if (g_DirectAuthoredSources.contains(strEffectAssetId) &&
		!g_Effects.contains(strEffectAssetId))
	{
		(void)Find(strEffectAssetId);
	}
	const auto Iterator = g_VisualProjections.find(strEffectAssetId);
	return g_VisualProjections.end() == Iterator ? nullptr : Iterator->second;
}

std::shared_ptr<const Client::EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>
Client::CEffectCatalog::Find_VisualProjection_Loaded(
	const std::string& strEffectAssetId)
{
	const auto Iterator = g_VisualProjections.find(strEffectAssetId);
	return g_VisualProjections.end() == Iterator ? nullptr : Iterator->second;
}

std::shared_ptr<const Client::EFFECT_SCREEN_OVERLAY_PRODUCT_BINDING>
Client::CEffectCatalog::Find_ScreenOverlayProductBinding(
	const std::string& strEffectAssetId)
{
	const auto Iterator =
		g_ScreenOverlayProductBindings.find(strEffectAssetId);
	return g_ScreenOverlayProductBindings.end() == Iterator ?
		nullptr : Iterator->second;
}

bool_t Client::CEffectCatalog::Prepare_ReconstructedRuntimeProgram(
	const std::string& strEffectAssetId,
	std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION>&
		OutPreparation,
	std::string& strOutError)
{
	const std::shared_ptr<const EFFECT_RUNTIME_PROGRAM_CATALOG_ENTRY> Entry =
		Find_RuntimeProgramEntry(strEffectAssetId);
	const std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM> Program =
		nullptr == Entry ? nullptr : Entry->Get_Program();
	if (nullptr == Entry || nullptr == Program ||
		Entry->Get_Identity().strEffectAssetId != strEffectAssetId ||
		Entry->Get_Identity().iCatalogRevision == 0u ||
		Program->strRuntimeCatalogAssetId != strEffectAssetId ||
		Program->Admission.bRuntimeExecution || Program->Admission.bProduct)
	{
		strOutError =
			"Reconstructed Effect program is absent or has invalid admission.";
		return false;
	}
	constexpr std::array<std::string_view, 5u> EXPECTED_OWNER_EMITTER_IDS{
		"fx_pc_sdm_07.par_v_sdm_ink_spw_01::action-31470/stage-000/notify-000::FX_PC_SDM_07.par_v_sdm_ink_spw_01.particlespriteemitter_21",
		"fx_pc_sdm_07.par_v_sdm_ink_spw_01::action-31470/stage-000/notify-000::FX_PC_SDM_07.par_v_sdm_ink_spw_01.particlespriteemitter_1",
		"fx_pc_sdm_07.par_v_sdm_ink_spw_01::action-31470/stage-000/notify-000::FX_PC_SDM_07.par_v_sdm_ink_spw_01.particlespriteemitter_6",
		"fx_pc_sdm_07.par_v_sdm_ink_spw_01::action-31470/stage-000/notify-000::FX_PC_SDM_07.par_v_sdm_ink_spw_01.particlespriteemitter_0",
		"fx_pc_sdm_07.par_v_smd_onestroke_weapon_01::action-31470/stage-000/notify-014::FX_PC_SDM_07.par_v_smd_onestroke_weapon_01.particlespriteemitter_6" };
	std::vector<EFFECT_RECONSTRUCTED_ANCHOR_BINDING> AnchorRequests;
	std::set<std::string, std::less<>> RequestIds;
	for (const EFFECT_RUNTIME_PROGRAM_EMITTER& Emitter : Program->Emitters)
	{
		for (const EFFECT_RUNTIME_PROGRAM_ANCHOR_REQUEST& Request :
			Emitter.AnchorRequests)
		{
			const std::string ExpectedRequestId =
				Emitter.Row.strId + "::anchor:action-cue";
			if (Request.strAnchorRequestId.empty() ||
				Request.strRuntimeAnchorSlotId.empty() ||
				Request.strRuntimeBoneName.empty() ||
				Request.strAnchorRequestId != ExpectedRequestId ||
				!RequestIds.insert(Request.strAnchorRequestId).second)
			{
				strOutError =
					"Reconstructed Effect typed AnchorRequest is invalid.";
				return false;
			}
			AnchorRequests.push_back({ Emitter.Row.strId, Request });
		}
	}
	if (AnchorRequests.size() != EXPECTED_OWNER_EMITTER_IDS.size())
	{
		strOutError =
			"Reconstructed Effect typed AnchorRequest count is not frozen at 5.";
		return false;
	}
	for (size_t Index = 0u; Index < EXPECTED_OWNER_EMITTER_IDS.size(); ++Index)
	{
		const EFFECT_RECONSTRUCTED_ANCHOR_BINDING& Binding =
			AnchorRequests[Index];
		if (Binding.strOwnerEmitterId != EXPECTED_OWNER_EMITTER_IDS[Index] ||
			Binding.Request.strAnchorRequestId !=
				Binding.strOwnerEmitterId + "::anchor:action-cue")
		{
			strOutError =
				"Reconstructed Effect typed AnchorRequest owner tuple is invalid.";
			return false;
		}
	}
	std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION> Staged(
		new EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION(
			Entry, std::move(AnchorRequests)));
	OutPreparation = std::move(Staged);
	strOutError.clear();
	return true;
}

bool_t Client::CEffectReconstructedRuntimeBoundary::Prepare_Presentation(
	const std::string& strEffectAssetId,
	std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION>&
		OutPreparation,
	std::string& strOutError)
{
	return CEffectCatalog::Prepare_ReconstructedRuntimeProgram(
		strEffectAssetId, OutPreparation, strOutError);
}

bool_t Client::CEffectReconstructedRuntimeBoundary::Admit_ProductSpawn(
	const std::string& strEffectAssetId,
	std::string& strOutError)
{
	if (CEffectCatalog::Is_ReconstructedRuntimeProgramAssetId(strEffectAssetId))
	{
		strOutError =
			"Reconstructed Effect program is not Product-admitted for spawn.";
		return false;
	}
	strOutError.clear();
	return true;
}

bool_t Client::CEffectReconstructedRuntimeBoundary::Stage(
	std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION> pPreparation,
	const EFFECT_RECONSTRUCTED_RUNTIME_SEAM eSeam,
	std::string& strOutError)
{
	const std::shared_ptr<const EFFECT_RUNTIME_PROGRAM_CATALOG_ENTRY> Entry =
		nullptr == pPreparation ? nullptr : pPreparation->Get_CatalogEntry();
	const std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM> Program =
		nullptr == pPreparation ? nullptr : pPreparation->Get_Program();
	const EFFECT_RUNTIME_PROGRAM_CATALOG_IDENTITY* Identity =
		nullptr == Entry ? nullptr : &Entry->Get_Identity();
	if (eSeam >= EFFECT_RECONSTRUCTED_RUNTIME_SEAM::END ||
		nullptr == Entry || nullptr == Program || nullptr == Identity ||
		0u == Identity->iCatalogRevision ||
		Identity->strEffectAssetId != "effect.artist.skill.31470" ||
		Identity->strProgramId != Program->Identity.strProgramId ||
		Identity->strProgramSha256 != Program->Identity.strProgramSha256 ||
		Identity->strCandidateRawSha256 !=
			Program->Identity.strCandidateRawSha256 ||
		Program->Emitters.size() != 35u ||
		Program->ActionSchedules.size() != 7u ||
		Program->Modules.size() != 399u ||
		Program->Distributions.size() != 629u ||
		pPreparation->Get_AnchorRequests().size() != 5u ||
		Program->Admission.bRuntimeExecution || Program->Admission.bProduct)
	{
		strOutError =
			"Reconstructed Effect preparation is invalid for the runtime seam.";
		return false;
	}
	m_pPreparation = std::move(pPreparation);
	strOutError.clear();
	return true;
}

bool_t Client::CEffectReconstructedRuntimeBoundary::Admit_Execution(
	std::string& strOutError) const
{
	if (!Is_Staged())
	{
		strOutError.clear();
		return true;
	}
	strOutError = "Reconstructed Effect execution is not admitted.";
	return false;
}

bool_t Client::CEffectReconstructedRuntimeBoundary::Admit_Submit(
	std::string& strOutError) const
{
	if (!Is_Staged())
	{
		strOutError.clear();
		return true;
	}
	strOutError = "Reconstructed Effect presentation submit is not admitted.";
	return false;
}

bool_t Client::CEffectReconstructedRuntimeBoundary::Admit_Render(
	std::string& strOutError) const
{
	if (!Is_Staged())
	{
		strOutError.clear();
		return true;
	}
	strOutError = "Reconstructed Effect rendering is not admitted.";
	return false;
}

bool_t Client::CEffectCatalog::Contains(const std::string& strEffectAssetId)
{
	if (Is_ReconstructedRuntimeProgramAssetId(strEffectAssetId))
		return false;
	return g_Effects.contains(strEffectAssetId) ||
		g_DirectAuthoredSources.contains(strEffectAssetId);
}

bool_t Client::CEffectCatalog::Try_ContainsSourceRegistrationFresh(
	const std::string& strEffectAssetId,
	bool_t& bOutContains,
	std::string& strOutStatus)
{
	bOutContains = false;
	if (strEffectAssetId.empty())
	{
		strOutStatus =
			"Fresh Effect source membership requires one stable Effect ID.";
		return false;
	}
	std::map<std::string,
		std::shared_ptr<const DIRECT_AUTHORED_RUNTIME_SOURCE>, std::less<>>
		StagedSources;
	std::map<std::string,
		std::shared_ptr<const EFFECT_SCREEN_OVERLAY_PRODUCT_BINDING>,
		std::less<>> StagedScreenOverlayBindings;
	std::string Error;
	if (!Stage_SourceDirectAuthoredCatalog(
			StagedSources, StagedScreenOverlayBindings, Error))
	{
		strOutStatus =
			"Fresh Effect source catalog preflight failed: " + Error;
		return false;
	}
	bOutContains = StagedSources.contains(strEffectAssetId);
	strOutStatus = bOutContains ?
		("Fresh Effect source catalog contains " + strEffectAssetId + ".") :
		("Fresh Effect source catalog does not contain " + strEffectAssetId + ".");
	return true;
}

bool_t Client::CEffectCatalog::Is_DirectAuthoredDocument(
	const std::string& strEffectAssetId)
{
	return g_DirectAuthoredSources.contains(strEffectAssetId);
}

bool_t Client::CEffectCatalog::Contains_RuntimeAuthority(
	const std::string& strEffectAssetId)
{
	if (Is_ReconstructedRuntimeProgramAssetId(strEffectAssetId))
		return false;
	return g_RuntimeAuthorities.contains(strEffectAssetId);
}

bool_t Client::CEffectCatalog::Contains_ReconstructedRuntimeProgram(
	const std::string& strEffectAssetId)
{
	return g_RuntimeProgramEntries.contains(strEffectAssetId);
}

bool_t Client::CEffectCatalog::Is_ReconstructedRuntimeProgramAssetId(
	const std::string& strEffectAssetId)
{
	return strEffectAssetId == "effect.artist.skill.31470";
}

std::vector<std::string> Client::CEffectCatalog::Get_EffectAssetIds()
{
	std::set<std::string, std::less<>> AssetIds;
	for (const auto& [AssetId, Document] : g_Effects)
	{
		if (!Is_ReconstructedRuntimeProgramAssetId(AssetId))
			AssetIds.insert(AssetId);
	}
	for (const auto& [AssetId, Source] : g_DirectAuthoredSources)
		AssetIds.insert(AssetId);
	std::vector<std::string> Result;
	Result.reserve(AssetIds.size());
	for (const std::string& AssetId : AssetIds)
		Result.push_back(AssetId);
	return Result;
}

std::vector<std::string> Client::CEffectCatalog::Get_ComponentAssetIds()
{
	std::vector<std::string> Result;
	Result.reserve(g_Components.size());
	for (const auto& [AssetId, Component] : g_Components)
		Result.push_back(AssetId);
	return Result;
}

std::vector<std::string> Client::CEffectCatalog::Get_RuntimeAuthorityAssetIds()
{
	std::vector<std::string> Result;
	Result.reserve(g_RuntimeAuthorities.size());
	for (const auto& [AssetId, Document] : g_RuntimeAuthorities)
	{
		if (!Is_ReconstructedRuntimeProgramAssetId(AssetId))
			Result.push_back(AssetId);
	}
	return Result;
}

std::vector<std::string>
Client::CEffectCatalog::Get_ReconstructedRuntimeProgramAssetIds()
{
	std::vector<std::string> Result;
	Result.reserve(g_RuntimeProgramEntries.size());
	for (const auto& [AssetId, Entry] : g_RuntimeProgramEntries)
		Result.push_back(AssetId);
	return Result;
}

std::vector<std::string> Client::CEffectCatalog::Get_VisualProgramAssetIds()
{
	std::vector<std::string> Result;
	Result.reserve(g_VisualPrograms.size());
	for (const auto& [AssetId, Program] : g_VisualPrograms)
		Result.push_back(AssetId);
	return Result;
}

std::shared_ptr<const Client::CEffectMaterialProgramRegistry>
Client::CEffectCatalog::Acquire_MaterialProgramRegistry()
{
	return g_pMaterialProgramRegistry;
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
	g_DirectAuthoredSources.clear();
	g_Assemblies.clear();
	g_Components.clear();
	g_RuntimeAuthorities.clear();
	g_RuntimeProgramEntries.clear();
	g_pVisualProgramCorpus.reset();
	g_VisualPrograms.clear();
	g_VisualProjections.clear();
	g_ScreenOverlayProductBindings.clear();
	g_pMaterialProgramRegistry.reset();
	g_iCatalogOwnerThreadId = 0u;
    g_strStatus = "Effect catalog cleared.";
}
