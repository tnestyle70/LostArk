#include "Effect_Catalog.h"

#include "DataJson.h"
#include "Effect_DocumentCodec.h"
#include "Effect_OccurrenceTuning.h"
#include "Effect_VisualProgramCorpus.h"
#include "Network/PacketMessages.h"
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
		std::string strContentSha256;
		std::map<std::string, std::string, std::less<>> Dependencies;
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
		std::shared_ptr<const Client::EFFECT_PRODUCT_CUE_ADMISSION>,
		std::less<>> g_ProductCueAdmissions;
	std::map<std::string,
		std::vector<std::shared_ptr<const Client::EFFECT_PRODUCT_CUE_ADMISSION>>,
		std::less<>> g_ProductCueAdmissionsByEffect;
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

	bool Is_SealedDirectAuthoredDocumentPath(
		const std::string_view Value,
		const std::string_view EffectAssetId,
		const std::string_view ContentSha256)
	{
		if (EffectAssetId.empty() || !Is_LowerHexSha256(
				std::string(ContentSha256)))
		{
			return false;
		}
		return Value == "Authored/" + std::string(EffectAssetId) + "." +
			std::string(ContentSha256) + ".effect.json";
	}

	bool Resolve_SealedDirectAuthoredDocumentPath(
		const std::filesystem::path& RuntimeCatalogPath,
		const std::string& RelativePath,
		std::filesystem::path& OutPath)
	{
		OutPath.clear();
		if (RelativePath.empty() || RelativePath.size() > 1024u ||
			RelativePath.find('\\') != std::string::npos ||
			RelativePath.find(':') != std::string::npos ||
			RelativePath.find("//") != std::string::npos ||
			std::any_of(RelativePath.begin(), RelativePath.end(),
				[](const unsigned char Character)
				{
					return Character < 0x20u || Character == 0x7fu;
				}))
		{
			return false;
		}

		std::error_code Error;
		const std::filesystem::path Root = std::filesystem::weakly_canonical(
			RuntimeCatalogPath.parent_path(), Error);
		if (Error || Root.empty())
			return false;
		const std::filesystem::path Candidate =
			std::filesystem::weakly_canonical(Root / RelativePath, Error);
		if (Error || Candidate.empty())
			return false;
		const auto Mismatch = std::mismatch(
			Root.begin(), Root.end(), Candidate.begin(), Candidate.end());
		if (Mismatch.first != Root.end())
			return false;
		OutPath = Candidate;
		return true;
	}

	std::filesystem::path Find_RuntimeVisualProgramSidecar(
		const std::filesystem::path& RuntimeCatalogPath)
	{
		return RuntimeCatalogPath.parent_path() /
			L"EffectVisualPrograms.runtime.json";
	}

	std::filesystem::path Find_ProductCueAdmissionSidecar(
		const std::filesystem::path& RuntimeCatalogPath)
	{
		return RuntimeCatalogPath.parent_path() /
			L"EffectProductCueAdmissions.runtime.json";
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

	bool Build_ObjectProjection(
		const Client::DATA_JSON_VALUE& Value,
		const std::initializer_list<std::string_view> Keys,
		Client::DATA_JSON_VALUE& OutProjection)
	{
		using namespace Client;
		if (!Value.Is_Object())
			return false;
		DATA_JSON_VALUE::OBJECT Fields;
		std::vector<std::string> Order;
		Order.reserve(Keys.size());
		for (const std::string_view Key : Keys)
		{
			const DATA_JSON_VALUE* Field = Value.Find(Key);
			if (nullptr == Field ||
				!Fields.emplace(std::string(Key), *Field).second)
			{
				return false;
			}
			Order.emplace_back(Key);
		}
		OutProjection = DATA_JSON_VALUE::Object(
			std::move(Fields), std::move(Order));
		return true;
	}

	struct PARSED_RECONSTRUCTED_RUNTIME_ENTRY final
	{
		Client::EFFECT_RUNTIME_PROGRAM_CATALOG_IDENTITY Identity;
		std::shared_ptr<const Client::EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM>
			pProgram;
		std::shared_ptr<const
			Client::EFFECT_RECONSTRUCTED_RENDER_RESOURCE_AUTHORITY>
			pRenderResourceAuthority;
		std::shared_ptr<const Client::EFFECT_OCCURRENCE_TUNING_DOCUMENT>
			pOccurrenceTuning;
	};

	bool Parse_ReconstructedRenderResourceExtension(
		const Client::DATA_JSON_VALUE& Value,
		Client::EFFECT_RUNTIME_PROGRAM_CATALOG_IDENTITY& InOutIdentity,
		std::shared_ptr<const
			Client::EFFECT_RECONSTRUCTED_RENDER_RESOURCE_AUTHORITY>&
			OutAuthority,
		std::string& strOutError);

	bool Parse_ReconstructedRuntimeProgramEntry(
		const Client::DATA_JSON_VALUE& Value,
		const uint64_t iCatalogRevision,
		PARSED_RECONSTRUCTED_RUNTIME_ENTRY& OutEntry,
		std::string& strOutError)
	{
		using namespace Client;
		constexpr std::string_view EFFECT_ASSET_ID =
			"effect.artist.skill.31470";
		constexpr std::string_view PAYLOAD_KIND =
			"IMMUTABLE_RECONSTRUCTED_RUNTIME_PROGRAM";
		constexpr std::string_view COMPILER_REVISION =
			"artist31470.reconstructed-runtime-program-link-v1";
		constexpr std::string_view BUILDER_COMMIT_ID =
			"ddef21a5314eb8c3db891d36f702cfeda3149f20";
		constexpr std::string_view BUILDER_TREE_ID =
			"36a36b889dae7be092e0d2f6f3c3aee2c28bc462";
		constexpr std::string_view CANDIDATE_BLOB_ID =
			"4b090268b95ea590587276c8048c3b235ee33571";
		constexpr std::string_view RESOURCE_BINDING_HASH =
			"df15009e41b6c1fe9161af873b96dfc428771944786c14f9435f7c0ffa4d869c";
		constexpr std::string_view INPUT_ARTIFACTS_ORDERED_SHA256 =
			"bcf87806b3635019442f6787c2ca6aed15d7012f2dd4c04d33b448f80814415f";
		constexpr std::string_view RECONSTRUCTED_LINK_SHA256 =
			"282f450d95ae283acf91047fe6b293eb93fddaea9f4bde4cff9671e7aa27c523";
		constexpr std::string_view RECEIPT_SELF_SHA256 =
			"0a870ae15bd33f674b16e9c58a3504b69b9cb1c491d35c9216cb2a647a20724d";
		constexpr std::string_view PUBLISH_RECEIPT_SHA256 =
			"413c34440ee82b9511d1de4bbd31af282dce2e65204fd5cda0d75d8e4e58650b";
		const bool bHistoricalOuter10 = Has_ExactOrderedKeys(Value, {
			"payloadKind", "effectAssetId", "artifactRevision",
			"compilerRevision", "sourceExact", "runtimeExecutionAdmission",
			"productAdmission", "publishReceiptSha256", "publishReceipt",
			"reconstructedRuntimeProgram" });
		const bool bRenderResourceOuter13 = Has_ExactOrderedKeys(Value, {
			"payloadKind", "effectAssetId", "artifactRevision",
			"compilerRevision", "sourceExact", "runtimeExecutionAdmission",
			"productAdmission", "publishReceiptSha256", "publishReceipt",
			"reconstructedRuntimeProgram",
			"renderResourcePublishReceiptSha256",
			"renderResourcePublishReceipt",
			"reconstructedRenderResourceAuthority" });
		const bool bOccurrenceTuningOuter16 = Has_ExactOrderedKeys(Value, {
			"payloadKind", "effectAssetId", "artifactRevision",
			"compilerRevision", "sourceExact", "runtimeExecutionAdmission",
			"productAdmission", "publishReceiptSha256", "publishReceipt",
			"reconstructedRuntimeProgram",
			"renderResourcePublishReceiptSha256",
			"renderResourcePublishReceipt",
			"reconstructedRenderResourceAuthority",
			"occurrenceTuningSourcePath", "occurrenceTuningSha256",
			"occurrenceTuning" });
		if (!bHistoricalOuter10 && !bRenderResourceOuter13 &&
			!bOccurrenceTuningOuter16)
		{
			strOutError =
				"Reconstructed runtime entry fields or order are invalid.";
			return false;
		}
		const DATA_JSON_VALUE* PayloadKind = Required(
			Value, "payloadKind", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* OuterEffectId = Required(
			Value, "effectAssetId", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* OuterCompilerRevision = Required(
			Value, "compilerRevision", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* SourceExact = Required(
			Value, "sourceExact", DATA_JSON_TYPE::BOOLEAN);
		const DATA_JSON_VALUE* RuntimeAdmission = Required(
			Value, "runtimeExecutionAdmission", DATA_JSON_TYPE::BOOLEAN);
		const DATA_JSON_VALUE* ProductAdmission = Required(
			Value, "productAdmission", DATA_JSON_TYPE::BOOLEAN);
		const DATA_JSON_VALUE* PublishReceiptSha = Required(
			Value, "publishReceiptSha256", DATA_JSON_TYPE::STRING);
		uint32_t iArtifactRevision = 0u;
		if (nullptr == PayloadKind || PayloadKind->Get_String() != PAYLOAD_KIND ||
			nullptr == OuterEffectId ||
			OuterEffectId->Get_String() != EFFECT_ASSET_ID ||
			nullptr == OuterCompilerRevision ||
			OuterCompilerRevision->Get_String() != COMPILER_REVISION ||
			nullptr == SourceExact || SourceExact->Get_Boolean() ||
			nullptr == RuntimeAdmission || RuntimeAdmission->Get_Boolean() ||
			nullptr == ProductAdmission || ProductAdmission->Get_Boolean() ||
			nullptr == PublishReceiptSha ||
			PublishReceiptSha->Get_String() != PUBLISH_RECEIPT_SHA256 ||
			!Read_U32(Value, "artifactRevision", iArtifactRevision) ||
			Value.Find("artifactRevision")->Was_FloatingPointToken() ||
			iArtifactRevision != 1u)
		{
			strOutError =
				"Reconstructed runtime outer identity or admission is invalid.";
			return false;
		}

		const DATA_JSON_VALUE* Link = Required(
			Value, "reconstructedRuntimeProgram", DATA_JSON_TYPE::OBJECT);
		if (nullptr == Link || !Has_ExactOrderedKeys(*Link, {
			"schema", "formatVersion", "encoding", "effectAssetId",
			"candidateBuilderCommitId", "candidateBuilderTreeId",
			"candidateBlobId", "resourceBindingHash", "inputArtifactCount",
			"inputArtifactsOrderedSha256", "programId", "programVersion",
			"programSha256", "candidateRawSha256", "candidateByteCount",
			"candidateUtf8Json" }))
		{
			strOutError =
				"Reconstructed runtime program link fields or order are invalid.";
			return false;
		}
		const DATA_JSON_VALUE* Schema = Required(
			*Link, "schema", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* Version = Required(
			*Link, "formatVersion", DATA_JSON_TYPE::NUMBER);
		const DATA_JSON_VALUE* Encoding = Required(
			*Link, "encoding", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* EffectId = Required(
			*Link, "effectAssetId", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* BuilderCommitId = Required(
			*Link, "candidateBuilderCommitId", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* BuilderTreeId = Required(
			*Link, "candidateBuilderTreeId", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* CandidateBlobId = Required(
			*Link, "candidateBlobId", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* ResourceBindingHash = Required(
			*Link, "resourceBindingHash", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* InputArtifactsOrderedSha = Required(
			*Link, "inputArtifactsOrderedSha256", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* ProgramId = Required(
			*Link, "programId", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* ProgramSha = Required(
			*Link, "programSha256", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* CandidateRawSha = Required(
			*Link, "candidateRawSha256", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* CandidateUtf8 = Required(
			*Link, "candidateUtf8Json", DATA_JSON_TYPE::STRING);
		uint32_t iInputArtifactCount = 0u;
		uint32_t iProgramVersion = 0u;
		uint32_t iCandidateByteCount = 0u;
		const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM_IDENTITY Frozen =
			CEffectRuntimeAuthorityCodec::Get_FrozenArtist31470FProgramIdentity();
		if (nullptr == Schema || Schema->Get_String() !=
				"lostark.effect-reconstructed-runtime-program-link" ||
			nullptr == Version || Version->Was_FloatingPointToken() ||
			Version->Get_Number() != 1.0 || nullptr == Encoding ||
			Encoding->Get_String() != "UTF8_JSON_EXACT" ||
			nullptr == EffectId || EffectId->Get_String() != EFFECT_ASSET_ID ||
			nullptr == BuilderCommitId ||
			BuilderCommitId->Get_String() != BUILDER_COMMIT_ID ||
			nullptr == BuilderTreeId ||
			BuilderTreeId->Get_String() != BUILDER_TREE_ID ||
			nullptr == CandidateBlobId ||
			CandidateBlobId->Get_String() != CANDIDATE_BLOB_ID ||
			nullptr == ResourceBindingHash ||
			ResourceBindingHash->Get_String() != RESOURCE_BINDING_HASH ||
			nullptr == InputArtifactsOrderedSha ||
			InputArtifactsOrderedSha->Get_String() !=
				INPUT_ARTIFACTS_ORDERED_SHA256 ||
			nullptr == ProgramId || ProgramId->Get_String() != Frozen.strProgramId ||
			nullptr == ProgramSha ||
			ProgramSha->Get_String() != Frozen.strProgramSha256 ||
			nullptr == CandidateRawSha ||
			CandidateRawSha->Get_String() != Frozen.strCandidateRawSha256 ||
			nullptr == CandidateUtf8 ||
			!Read_U32(*Link, "inputArtifactCount", iInputArtifactCount) ||
			Link->Find("inputArtifactCount")->Was_FloatingPointToken() ||
			iInputArtifactCount != 13u ||
			!Read_U32(*Link, "programVersion", iProgramVersion) ||
			Link->Find("programVersion")->Was_FloatingPointToken() ||
			iProgramVersion != Frozen.iProgramVersion ||
			!Read_U32(*Link, "candidateByteCount", iCandidateByteCount) ||
			Link->Find("candidateByteCount")->Was_FloatingPointToken() ||
			iCandidateByteCount != 15'121'873u ||
			CandidateUtf8->Get_String().size() != iCandidateByteCount ||
			Frozen.strBuilderAuthorityCommitId != BuilderCommitId->Get_String() ||
			Frozen.strBuilderAuthorityTreeId != BuilderTreeId->Get_String())
		{
			strOutError =
				"Reconstructed runtime program link identity is invalid.";
			return false;
		}

		const DATA_JSON_VALUE* Receipt = Required(
			Value, "publishReceipt", DATA_JSON_TYPE::OBJECT);
		if (nullptr == Receipt || !Has_ExactOrderedKeys(*Receipt, {
			"schema", "formatVersion", "receiptRole", "payloadKind",
			"effectAssetId", "artifactRevision", "compilerRevision",
			"sourceExact", "runtimeExecutionAdmission", "productAdmission",
			"candidateBuilderCommitId", "candidateBuilderTreeId",
			"candidateBlobId", "resourceBindingHash", "inputArtifactCount",
			"inputArtifactsOrderedSha256", "programId", "programVersion",
			"programSha256", "candidateRawSha256", "candidateByteCount",
			"reconstructedRuntimeProgramSha256", "toolDependencies",
			"receiptSha256Domain", "receiptSha256" }))
		{
			strOutError =
				"Reconstructed runtime publish receipt fields or order are invalid.";
			return false;
		}
		const auto ReceiptStringEquals = [Receipt](
			const char* pName, const std::string_view Expected)
		{
			const DATA_JSON_VALUE* Field = Required(
				*Receipt, pName, DATA_JSON_TYPE::STRING);
			return nullptr != Field && Field->Get_String() == Expected;
		};
		const DATA_JSON_VALUE* ReceiptSourceExact = Required(
			*Receipt, "sourceExact", DATA_JSON_TYPE::BOOLEAN);
		const DATA_JSON_VALUE* ReceiptRuntimeAdmission = Required(
			*Receipt, "runtimeExecutionAdmission", DATA_JSON_TYPE::BOOLEAN);
		const DATA_JSON_VALUE* ReceiptProductAdmission = Required(
			*Receipt, "productAdmission", DATA_JSON_TYPE::BOOLEAN);
		const DATA_JSON_VALUE* ReconstructedLinkSha = Required(
			*Receipt, "reconstructedRuntimeProgramSha256", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* ReceiptSha = Required(
			*Receipt, "receiptSha256", DATA_JSON_TYPE::STRING);
		uint32_t iReceiptFormatVersion = 0u;
		uint32_t iReceiptArtifactRevision = 0u;
		uint32_t iReceiptInputArtifactCount = 0u;
		uint32_t iReceiptProgramVersion = 0u;
		uint32_t iReceiptCandidateByteCount = 0u;
		if (!ReceiptStringEquals("schema",
				"lostark.effect-reconstructed-runtime-program-publish-receipt") ||
			!Read_U32(*Receipt, "formatVersion", iReceiptFormatVersion) ||
			Receipt->Find("formatVersion")->Was_FloatingPointToken() ||
			iReceiptFormatVersion != 1u ||
			!ReceiptStringEquals("receiptRole",
				"PUBLICATION_PROVENANCE_ONLY_NOT_EXECUTION_AUTHORITY") ||
			!ReceiptStringEquals("payloadKind", PAYLOAD_KIND) ||
			!ReceiptStringEquals("effectAssetId", EFFECT_ASSET_ID) ||
			!Read_U32(*Receipt, "artifactRevision", iReceiptArtifactRevision) ||
			Receipt->Find("artifactRevision")->Was_FloatingPointToken() ||
			iReceiptArtifactRevision != iArtifactRevision ||
			!ReceiptStringEquals("compilerRevision", COMPILER_REVISION) ||
			nullptr == ReceiptSourceExact || ReceiptSourceExact->Get_Boolean() ||
			nullptr == ReceiptRuntimeAdmission ||
			ReceiptRuntimeAdmission->Get_Boolean() ||
			nullptr == ReceiptProductAdmission ||
			ReceiptProductAdmission->Get_Boolean() ||
			!ReceiptStringEquals("candidateBuilderCommitId", BUILDER_COMMIT_ID) ||
			!ReceiptStringEquals("candidateBuilderTreeId", BUILDER_TREE_ID) ||
			!ReceiptStringEquals("candidateBlobId", CANDIDATE_BLOB_ID) ||
			!ReceiptStringEquals("resourceBindingHash", RESOURCE_BINDING_HASH) ||
			!Read_U32(*Receipt, "inputArtifactCount",
				iReceiptInputArtifactCount) ||
			Receipt->Find("inputArtifactCount")->Was_FloatingPointToken() ||
			iReceiptInputArtifactCount != iInputArtifactCount ||
			!ReceiptStringEquals("inputArtifactsOrderedSha256",
				INPUT_ARTIFACTS_ORDERED_SHA256) ||
			!ReceiptStringEquals("programId", Frozen.strProgramId) ||
			!Read_U32(*Receipt, "programVersion", iReceiptProgramVersion) ||
			Receipt->Find("programVersion")->Was_FloatingPointToken() ||
			iReceiptProgramVersion != iProgramVersion ||
			!ReceiptStringEquals("programSha256", Frozen.strProgramSha256) ||
			!ReceiptStringEquals("candidateRawSha256",
				Frozen.strCandidateRawSha256) ||
			!Read_U32(*Receipt, "candidateByteCount",
				iReceiptCandidateByteCount) ||
			Receipt->Find("candidateByteCount")->Was_FloatingPointToken() ||
			iReceiptCandidateByteCount != iCandidateByteCount ||
			nullptr == ReconstructedLinkSha ||
			ReconstructedLinkSha->Get_String() != RECONSTRUCTED_LINK_SHA256 ||
			!ReceiptStringEquals("receiptSha256Domain",
				"CANONICAL_JSON_EXCLUDING_RECEIPT_SHA256") ||
			nullptr == ReceiptSha ||
			ReceiptSha->Get_String() != RECEIPT_SELF_SHA256)
		{
			strOutError =
				"Reconstructed runtime publish receipt identity is invalid.";
			return false;
		}

		const DATA_JSON_VALUE* ToolDependencies = Required(
			*Receipt, "toolDependencies", DATA_JSON_TYPE::ARRAY);
		constexpr std::array<std::string_view, 3u> TOOL_ROLES{
			"RECONSTRUCTED_RUNTIME_PROGRAM_CANDIDATE_BUILDER",
			"RECONSTRUCTED_RUNTIME_PROGRAM_CATALOG_VALIDATOR",
			"EFFECT_PUBLISHER" };
		constexpr std::array<std::string_view, 3u> TOOL_PATHS{
			"Tools/EffectPipeline/build_artist_31470_reconstructed_runtime_program.py",
			"Tools/EffectPipeline/build_effect_derived_artifact.py",
			"Tools/EffectPipeline/Publish-Effects.ps1" };
		constexpr std::array<std::string_view, 3u> TOOL_SHA256{
			"5421a989573aed54e24e98f3d5dc55874475332e3446bf966541800ab6db4c65",
			"5407c3d0983c3aaf4bf085904ef8d7b5f3e9119ae448703ff7e8f612a1c144fb",
			"ee4a12cf5cbd63bc9af6b0af18ca37da7631a4b0b6ed1465c95bf99fb9be8825" };
		if (nullptr == ToolDependencies || ToolDependencies->Get_Array().size() !=
			TOOL_ROLES.size())
		{
			strOutError =
				"Reconstructed runtime publish receipt tool set is invalid.";
			return false;
		}
		for (size_t Index = 0u; Index < TOOL_ROLES.size(); ++Index)
		{
			const DATA_JSON_VALUE& Tool = ToolDependencies->Get_Array()[Index];
			const DATA_JSON_VALUE* ToolSha = Required(
				Tool, "sha256", DATA_JSON_TYPE::STRING);
			if (!Has_ExactOrderedKeys(Tool,
					{ "role", "path", "hashDomain", "sha256" }) ||
				nullptr == Required(Tool, "role", DATA_JSON_TYPE::STRING) ||
				Tool.Find("role")->Get_String() != TOOL_ROLES[Index] ||
				nullptr == Required(Tool, "path", DATA_JSON_TYPE::STRING) ||
				Tool.Find("path")->Get_String() != TOOL_PATHS[Index] ||
				nullptr == Required(Tool, "hashDomain", DATA_JSON_TYPE::STRING) ||
				Tool.Find("hashDomain")->Get_String() !=
					"TRACKED_SOURCE_EOL_CANONICAL_TEXT" ||
				nullptr == ToolSha || ToolSha->Get_String() != TOOL_SHA256[Index])
			{
				strOutError =
					"Reconstructed runtime publish receipt tool identity is invalid.";
				return false;
			}
		}

		const std::string ComputedLinkSha =
			CEffectRuntimeAuthorityCodec::Compute_Sha256Hex(
				CEffectRuntimeAuthorityCodec::Serialize_CanonicalJson(*Link));
		DATA_JSON_VALUE UnsignedReceipt;
		if (ComputedLinkSha != ReconstructedLinkSha->Get_String() ||
			!Build_ObjectProjection(*Receipt, {
				"schema", "formatVersion", "receiptRole", "payloadKind",
				"effectAssetId", "artifactRevision", "compilerRevision",
				"sourceExact", "runtimeExecutionAdmission", "productAdmission",
				"candidateBuilderCommitId", "candidateBuilderTreeId",
				"candidateBlobId", "resourceBindingHash", "inputArtifactCount",
				"inputArtifactsOrderedSha256", "programId", "programVersion",
				"programSha256", "candidateRawSha256", "candidateByteCount",
				"reconstructedRuntimeProgramSha256", "toolDependencies",
				"receiptSha256Domain" }, UnsignedReceipt) ||
			CEffectRuntimeAuthorityCodec::Compute_Sha256Hex(
				CEffectRuntimeAuthorityCodec::Serialize_CanonicalJson(UnsignedReceipt)) !=
				ReceiptSha->Get_String() ||
			CEffectRuntimeAuthorityCodec::Compute_Sha256Hex(
				CEffectRuntimeAuthorityCodec::Serialize_CanonicalJson(*Receipt)) !=
				PublishReceiptSha->Get_String())
		{
			strOutError =
				"Reconstructed runtime publication digest binding is invalid.";
			return false;
		}

		std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM> Program;
		if (!CEffectRuntimeAuthorityCodec::Parse_ReconstructedRuntimeProgram(
			CandidateUtf8->Get_String(), Frozen, Program, strOutError) ||
			nullptr == Program)
		{
			return false;
		}
		std::array<size_t, 6u> RendererCounts{};
		for (const EFFECT_RUNTIME_PROGRAM_EMITTER& Emitter : Program->Emitters)
		{
			const size_t Index = static_cast<size_t>(Emitter.eRenderer);
			if (Index >= RendererCounts.size())
			{
				strOutError = "Reconstructed runtime renderer kind is invalid.";
				return false;
			}
			++RendererCounts[Index];
		}
		if (Program->strRuntimeCatalogAssetId != EffectId->Get_String() ||
			Program->Identity.strProgramId != ProgramId->Get_String() ||
			Program->Identity.iProgramVersion != iProgramVersion ||
			Program->Identity.strProgramSha256 != ProgramSha->Get_String() ||
			Program->Identity.strCandidateRawSha256 !=
				CandidateRawSha->Get_String() ||
			Program->Emitters.size() != 35u ||
			Program->ActionSchedules.size() != 7u ||
			Program->Modules.size() != 399u ||
			Program->Distributions.size() != 629u ||
			RendererCounts != std::array<size_t, 6u>{ 16u, 13u, 3u, 1u, 1u, 1u } ||
			Program->Admission.bRuntimeExecution || Program->Admission.bProduct)
		{
			strOutError =
				"Reconstructed runtime program identity or fixed denominator mismatch.";
			return false;
		}

		PARSED_RECONSTRUCTED_RUNTIME_ENTRY Staged;
		Staged.Identity.iCatalogRevision = iCatalogRevision;
		Staged.Identity.iArtifactRevision = iArtifactRevision;
		Staged.Identity.iProgramVersion = iProgramVersion;
		Staged.Identity.iInputArtifactCount = iInputArtifactCount;
		Staged.Identity.iCandidateByteCount = iCandidateByteCount;
		Staged.Identity.strEffectAssetId = EffectId->Get_String();
		Staged.Identity.strCompilerRevision = COMPILER_REVISION;
		Staged.Identity.strCandidateBuilderCommitId =
			BuilderCommitId->Get_String();
		Staged.Identity.strCandidateBuilderTreeId = BuilderTreeId->Get_String();
		Staged.Identity.strCandidateBlobId = CandidateBlobId->Get_String();
		Staged.Identity.strProgramId = ProgramId->Get_String();
		Staged.Identity.strProgramSha256 = ProgramSha->Get_String();
		Staged.Identity.strCandidateRawSha256 = CandidateRawSha->Get_String();
		Staged.Identity.strResourceBindingHash =
			ResourceBindingHash->Get_String();
		Staged.Identity.strInputArtifactsOrderedSha256 =
			InputArtifactsOrderedSha->Get_String();
		Staged.Identity.strReconstructedRuntimeProgramSha256 =
			ReconstructedLinkSha->Get_String();
		Staged.Identity.strPublishReceiptSha256 =
			PublishReceiptSha->Get_String();
		if ((bRenderResourceOuter13 || bOccurrenceTuningOuter16) &&
			!Parse_ReconstructedRenderResourceExtension(
				Value, Staged.Identity, Staged.pRenderResourceAuthority,
				strOutError))
		{
			return false;
		}
		if (bOccurrenceTuningOuter16)
		{
			const DATA_JSON_VALUE* SourcePath = Required(
				Value, "occurrenceTuningSourcePath", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* TuningSha = Required(
				Value, "occurrenceTuningSha256", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* Tuning = Required(
				Value, "occurrenceTuning", DATA_JSON_TYPE::OBJECT);
			if (nullptr == SourcePath ||
				!Is_NormalizedOccurrenceTuningSourcePath(SourcePath->Get_String()) ||
				nullptr == TuningSha || nullptr == Tuning ||
				!CEffectOccurrenceTuningCodec::Parse_RuntimePayload(
					*Tuning, TuningSha->Get_String(), *Program,
					Staged.pOccurrenceTuning, strOutError))
			{
				if (strOutError.empty())
					strOutError = "Occurrence tuning catalog binding is invalid.";
				return false;
			}
			Staged.Identity.strOccurrenceTuningSourcePath =
				SourcePath->Get_String();
			Staged.Identity.strOccurrenceTuningSha256 = TuningSha->Get_String();
			Staged.Identity.iOccurrenceTuningEntryCount = static_cast<uint32_t>(
				Staged.pOccurrenceTuning->Entries.size());
		}
		Staged.pProgram = std::move(Program);
		OutEntry = std::move(Staged);
		strOutError.clear();
		return true;
	}

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

	bool Read_I32Exact(const Client::DATA_JSON_VALUE& Object,
		const char* pName, int32_t& iOutValue)
	{
		const Client::DATA_JSON_VALUE* Value = Required(
			Object, pName, Client::DATA_JSON_TYPE::NUMBER);
		if (nullptr == Value || Value->Was_FloatingPointToken() ||
			!std::isfinite(Value->Get_Number()) ||
			Value->Get_Number() != std::floor(Value->Get_Number()) ||
			Value->Get_Number() < static_cast<double>(INT32_MIN) ||
			Value->Get_Number() > static_cast<double>(INT32_MAX))
		{
			return false;
		}
		iOutValue = static_cast<int32_t>(Value->Get_Number());
		return true;
	}

	bool Read_F32Exact(const Client::DATA_JSON_VALUE& Object,
		const char* pName, float& fOutValue)
	{
		const Client::DATA_JSON_VALUE* Value = Required(
			Object, pName, Client::DATA_JSON_TYPE::NUMBER);
		if (nullptr == Value || !Value->Was_FloatingPointToken() ||
			!std::isfinite(Value->Get_Number()) ||
			Value->Get_Number() < -static_cast<double>(FLT_MAX) ||
			Value->Get_Number() > static_cast<double>(FLT_MAX))
		{
			return false;
		}
		fOutValue = static_cast<float>(Value->Get_Number());
		return true;
	}

	bool Read_BooleanExact(const Client::DATA_JSON_VALUE& Object,
		const char* pName, const bool bExpected)
	{
		const Client::DATA_JSON_VALUE* Value = Required(
			Object, pName, Client::DATA_JSON_TYPE::BOOLEAN);
		return nullptr != Value && Value->Get_Boolean() == bExpected;
	}

	bool Read_StringExact(const Client::DATA_JSON_VALUE& Object,
		const char* pName, std::string& strOutValue,
		const bool bAllowEmpty = false, const size_t iMaximumLength = 4096u)
	{
		const Client::DATA_JSON_VALUE* Value = Required(
			Object, pName, Client::DATA_JSON_TYPE::STRING);
		if (nullptr == Value || Value->Get_String().size() > iMaximumLength ||
			(!bAllowEmpty && Value->Get_String().empty()))
		{
			return false;
		}
		strOutValue = Value->Get_String();
		return true;
	}

	bool Read_ShaExact(const Client::DATA_JSON_VALUE& Object,
		const char* pName, std::string& strOutValue)
	{
		return Read_StringExact(Object, pName, strOutValue) &&
			Is_LowerHexSha256(strOutValue);
	}

	bool Read_StringArrayExact(const Client::DATA_JSON_VALUE& Object,
		const char* pName, std::vector<std::string>& OutValues,
		const bool bShaValues = false)
	{
		const Client::DATA_JSON_VALUE* Value = Required(
			Object, pName, Client::DATA_JSON_TYPE::ARRAY);
		if (nullptr == Value || Value->Get_Array().size() > 128u)
			return false;
		std::vector<std::string> Staged;
		Staged.reserve(Value->Get_Array().size());
		for (const Client::DATA_JSON_VALUE& Item : Value->Get_Array())
		{
			if (!Item.Is_String() || Item.Get_String().empty() ||
				Item.Get_String().size() > 4096u ||
				(bShaValues && !Is_LowerHexSha256(Item.Get_String())))
			{
				return false;
			}
			Staged.push_back(Item.Get_String());
		}
		OutValues = std::move(Staged);
		return true;
	}

	bool Validate_FailClosedAuthorityRow(
		const Client::DATA_JSON_VALUE& Value,
		const bool bRequiresValidation = false)
	{
		return Read_BooleanExact(Value, "sourceExact", false) &&
			Read_BooleanExact(Value, "runtimeExecutionAdmission", false) &&
			Read_BooleanExact(Value, "product", false) &&
			(!bRequiresValidation ||
				(Read_BooleanExact(Value, "requiresAutomatedWARPProbe", true) &&
				 Read_BooleanExact(Value, "requiresManualEyeValidation", true)));
	}

	bool Parse_DdsSrvIdentity(const Client::DATA_JSON_VALUE& Value,
		Client::EFFECT_RECONSTRUCTED_DDS_SRV_IDENTITY& OutIdentity)
	{
		using namespace Client;
		if (!Has_ExactOrderedKeys(Value, {
			"Format", "FormatName", "ViewDimension", "ViewDimensionName",
			"MostDetailedMip", "MipLevels", "srvColorSpace" }))
		{
			return false;
		}
		uint32_t iFormat = 0u;
		uint32_t iViewDimension = 0u;
		EFFECT_RECONSTRUCTED_DDS_SRV_IDENTITY Staged;
		if (!Read_U32(Value, "Format", iFormat) ||
			!Read_U32(Value, "ViewDimension", iViewDimension) ||
			!Read_U32(Value, "MostDetailedMip", Staged.iMostDetailedMip) ||
			!Read_U32(Value, "MipLevels", Staged.iMipLevels) ||
			Staged.iMipLevels == 0u ||
			iFormat > static_cast<uint32_t>(DXGI_FORMAT_V408) ||
			iViewDimension != static_cast<uint32_t>(
				D3D11_SRV_DIMENSION_TEXTURE2D) ||
			!Read_StringExact(Value, "FormatName", Staged.strFormatName) ||
			!Read_StringExact(Value, "ViewDimensionName",
				Staged.strViewDimensionName) ||
			!Read_StringExact(Value, "srvColorSpace", Staged.strColorSpace) ||
			(Staged.strColorSpace != "LINEAR" &&
			 Staged.strColorSpace != "SRGB"))
		{
			return false;
		}
		Staged.eFormat = static_cast<DXGI_FORMAT>(iFormat);
		Staged.eViewDimension = static_cast<D3D11_SRV_DIMENSION>(iViewDimension);
		OutIdentity = std::move(Staged);
		return true;
	}

	bool Parse_SamplerDescriptor(const Client::DATA_JSON_VALUE& Value,
		D3D11_SAMPLER_DESC& OutDescriptor, std::string& strOutColorSpace)
	{
		using namespace Client;
		if (!Has_ExactOrderedKeys(Value, {
			"type", "filterUe3", "filterD3d11", "addressUUe3",
			"addressUD3d11", "addressVUe3", "addressVD3d11",
			"addressWUe3", "addressWD3d11", "mipLODBias",
			"maxAnisotropy", "comparisonFuncName", "comparisonFuncD3d11",
			"borderColor", "minLOD", "maxLOD", "sRgb", "srvColorSpace",
			"lodGroup" }))
		{
			return false;
		}
		std::string Type;
		std::string FilterUe3;
		std::string AddressUe3;
		std::string AddressVe3;
		std::string AddressWe3;
		std::string ComparisonName;
		std::string LodGroup;
		uint32_t iFilter = 0u;
		uint32_t iAddressU = 0u;
		uint32_t iAddressV = 0u;
		uint32_t iAddressW = 0u;
		uint32_t iMaxAnisotropy = 0u;
		uint32_t iComparison = 0u;
		D3D11_SAMPLER_DESC Staged{};
		const DATA_JSON_VALUE* BorderColor = Required(
			Value, "borderColor", DATA_JSON_TYPE::ARRAY);
		if (!Read_StringExact(Value, "type", Type) ||
			Type != "D3D11_SAMPLER_DESC_AND_SRV_COLOR_SPACE" ||
			!Read_StringExact(Value, "filterUe3", FilterUe3) ||
			!Read_U32(Value, "filterD3d11", iFilter) ||
			!Read_StringExact(Value, "addressUUe3", AddressUe3) ||
			!Read_U32(Value, "addressUD3d11", iAddressU) ||
			!Read_StringExact(Value, "addressVUe3", AddressVe3) ||
			!Read_U32(Value, "addressVD3d11", iAddressV) ||
			!Read_StringExact(Value, "addressWUe3", AddressWe3) ||
			!Read_U32(Value, "addressWD3d11", iAddressW) ||
			!Read_F32Exact(Value, "mipLODBias", Staged.MipLODBias) ||
			!Read_U32(Value, "maxAnisotropy", iMaxAnisotropy) ||
			!Read_StringExact(Value, "comparisonFuncName", ComparisonName) ||
			!Read_U32(Value, "comparisonFuncD3d11", iComparison) ||
			nullptr == BorderColor || BorderColor->Get_Array().size() != 4u ||
			!Read_F32Exact(Value, "minLOD", Staged.MinLOD) ||
			!Read_F32Exact(Value, "maxLOD", Staged.MaxLOD) ||
			!Read_StringExact(Value, "srvColorSpace", strOutColorSpace) ||
			!Read_StringExact(Value, "lodGroup", LodGroup) ||
			!Read_BooleanExact(Value, "sRgb", strOutColorSpace == "SRGB") ||
			iFilter != static_cast<uint32_t>(
				D3D11_FILTER_MIN_MAG_MIP_LINEAR) ||
			iAddressU < static_cast<uint32_t>(D3D11_TEXTURE_ADDRESS_WRAP) ||
			iAddressU > static_cast<uint32_t>(D3D11_TEXTURE_ADDRESS_MIRROR_ONCE) ||
			iAddressV < static_cast<uint32_t>(D3D11_TEXTURE_ADDRESS_WRAP) ||
			iAddressV > static_cast<uint32_t>(D3D11_TEXTURE_ADDRESS_MIRROR_ONCE) ||
			iAddressW < static_cast<uint32_t>(D3D11_TEXTURE_ADDRESS_WRAP) ||
			iAddressW > static_cast<uint32_t>(D3D11_TEXTURE_ADDRESS_MIRROR_ONCE) ||
			iMaxAnisotropy != 0u ||
			iComparison != static_cast<uint32_t>(D3D11_COMPARISON_NEVER) ||
			ComparisonName != "D3D11_COMPARISON_NEVER" ||
			Staged.MinLOD != 0.f || Staged.MaxLOD != FLT_MAX ||
			(strOutColorSpace != "LINEAR" && strOutColorSpace != "SRGB"))
		{
			return false;
		}
		for (size_t Index = 0u; Index < 4u; ++Index)
		{
			const DATA_JSON_VALUE& Item = BorderColor->Get_Array()[Index];
			if (!Item.Is_Number() || !Item.Was_FloatingPointToken() ||
				!std::isfinite(Item.Get_Number()) ||
				Item.Get_Number() < -static_cast<double>(FLT_MAX) ||
				Item.Get_Number() > static_cast<double>(FLT_MAX))
			{
				return false;
			}
			Staged.BorderColor[Index] = static_cast<float>(Item.Get_Number());
		}
		Staged.Filter = static_cast<D3D11_FILTER>(iFilter);
		Staged.AddressU = static_cast<D3D11_TEXTURE_ADDRESS_MODE>(iAddressU);
		Staged.AddressV = static_cast<D3D11_TEXTURE_ADDRESS_MODE>(iAddressV);
		Staged.AddressW = static_cast<D3D11_TEXTURE_ADDRESS_MODE>(iAddressW);
		Staged.MaxAnisotropy = iMaxAnisotropy;
		Staged.ComparisonFunc = static_cast<D3D11_COMPARISON_FUNC>(iComparison);
		OutDescriptor = Staged;
		return true;
	}

	bool Same_DdsSrvIdentity(
		const Client::EFFECT_RECONSTRUCTED_DDS_SRV_IDENTITY& Left,
		const Client::EFFECT_RECONSTRUCTED_DDS_SRV_IDENTITY& Right)
	{
		return Left.eFormat == Right.eFormat &&
			Left.eViewDimension == Right.eViewDimension &&
			Left.iMostDetailedMip == Right.iMostDetailedMip &&
			Left.iMipLevels == Right.iMipLevels &&
			Left.strFormatName == Right.strFormatName &&
			Left.strViewDimensionName == Right.strViewDimensionName &&
			Left.strColorSpace == Right.strColorSpace;
	}

	bool Validate_DdsHeader(const Client::DATA_JSON_VALUE& Value,
		const uint64_t iExpectedByteCount)
	{
		using namespace Client;
		if (!Has_ExactOrderedKeys(Value, {
			"magic", "headerSize", "flags", "height", "width",
			"pitchOrLinearSize", "depth", "rawMipMapCount",
			"effectiveMipLevelCount", "reserved1", "pixelFormat", "caps",
			"caps2", "caps3", "caps4", "reserved2", "dataOffset",
			"payloadByteCount", "expectedCompressedPayloadByteCount",
			"payloadByteCountExact", "compression" }))
		{
			return false;
		}
		std::string Magic;
		uint32_t iHeaderSize = 0u;
		uint32_t iWidth = 0u;
		uint32_t iHeight = 0u;
		uint32_t iMipCount = 0u;
		uint64_t iDataOffset = 0u;
		uint64_t iPayloadBytes = 0u;
		uint64_t iExpectedPayloadBytes = 0u;
		const DATA_JSON_VALUE* Reserved = Required(
			Value, "reserved1", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* PixelFormat = Required(
			Value, "pixelFormat", DATA_JSON_TYPE::OBJECT);
		const DATA_JSON_VALUE* Compression = Required(
			Value, "compression", DATA_JSON_TYPE::OBJECT);
		if (!Read_StringExact(Value, "magic", Magic) || Magic != "DDS " ||
			!Read_U32(Value, "headerSize", iHeaderSize) || iHeaderSize != 124u ||
			!Read_U32(Value, "width", iWidth) || iWidth == 0u ||
			!Read_U32(Value, "height", iHeight) || iHeight == 0u ||
			!Read_U32(Value, "effectiveMipLevelCount", iMipCount) ||
			iMipCount == 0u || !Read_U64Exact(Value, "dataOffset", iDataOffset) ||
			iDataOffset != 128u ||
			!Read_U64Exact(Value, "payloadByteCount", iPayloadBytes) ||
			!Read_U64Exact(Value, "expectedCompressedPayloadByteCount",
				iExpectedPayloadBytes) ||
			iPayloadBytes != iExpectedPayloadBytes ||
			iExpectedByteCount != iDataOffset + iPayloadBytes ||
			!Read_BooleanExact(Value, "payloadByteCountExact", true) ||
			nullptr == Reserved || Reserved->Get_Array().size() != 11u ||
			nullptr == PixelFormat || !Has_ExactOrderedKeys(*PixelFormat, {
				"size", "flags", "fourCC", "rgbBitCount", "rBitMask",
				"gBitMask", "bBitMask", "aBitMask" }) ||
			nullptr == Compression || !Has_ExactOrderedKeys(*Compression, {
				"family", "bytesPerFourByFourBlock", "linearDxgiFormat",
				"linearDxgiFormatName", "srgbDxgiFormat",
				"srgbDxgiFormatName" }))
		{
			return false;
		}
		uint32_t iPixelFormatSize = 0u;
		std::string FourCc;
		std::string CompressionFamily;
		return Read_U32(*PixelFormat, "size", iPixelFormatSize) &&
			iPixelFormatSize == 32u &&
			Read_StringExact(*PixelFormat, "fourCC", FourCc) &&
			Read_StringExact(*Compression, "family", CompressionFamily) &&
			!CompressionFamily.empty();
	}

	bool Parse_RenderTextureResources(const Client::DATA_JSON_VALUE& Value,
		std::map<std::string,
			Client::EFFECT_RECONSTRUCTED_RENDER_TEXTURE_RESOURCE,
			std::less<>>& OutResources, std::string& strOutError)
	{
		using namespace Client;
		if (!Value.Is_Array() || Value.Get_Array().size() != 52u)
		{
			strOutError = "Render-resource textureResources denominator is invalid.";
			return false;
		}
		std::map<std::string, EFFECT_RECONSTRUCTED_RENDER_TEXTURE_RESOURCE,
			std::less<>> Staged;
		for (size_t Index = 0u; Index < Value.Get_Array().size(); ++Index)
		{
			const DATA_JSON_VALUE& Row = Value.Get_Array()[Index];
			if (!Has_ExactOrderedKeys(Row, {
				"resourceAuthorityId", "order", "runtimeAssetId",
				"candidateBindingIds", "candidateBindingRowSha256",
				"candidateBindingCount", "byteCount", "rawSha256",
				"ddsHeader", "actualCompressedFormatClassification",
				"colorSpacePolicy", "actualExpectedSrvDescriptor",
				"resourceIdentityBasis", "absolutePathRecorded",
				"actionTimeIoAllowed", "sourceExact",
				"runtimeExecutionAdmission", "product", "rowSha256" }))
			{
				strOutError = "Render-resource textureResources row schema is invalid.";
				return false;
			}
			EFFECT_RECONSTRUCTED_RENDER_TEXTURE_RESOURCE Resource;
			uint32_t iCandidateBindingCount = 0u;
			std::vector<std::string> CandidateBindingIds;
			std::vector<std::string> CandidateBindingRowHashes;
			std::string Classification;
			std::string ColorSpace;
			std::string IdentityBasis;
			const DATA_JSON_VALUE* DdsHeader = Required(
				Row, "ddsHeader", DATA_JSON_TYPE::OBJECT);
			const DATA_JSON_VALUE* Srv = Required(
				Row, "actualExpectedSrvDescriptor", DATA_JSON_TYPE::OBJECT);
			const auto FailRow = [&](const std::string_view Field)
			{
				strOutError = "Render-resource textureResources row " +
					std::to_string(Index) + " has an invalid " +
					std::string(Field) + ".";
				return false;
			};
			if (!Read_StringExact(Row, "resourceAuthorityId",
				Resource.strResourceAuthorityId) ||
				!Is_StableId(Resource.strResourceAuthorityId))
				return FailRow("resourceAuthorityId");
			if (!Read_U32(Row, "order", Resource.iOrder) ||
				Resource.iOrder != Index)
				return FailRow("order");
			if (!Read_StringExact(Row, "runtimeAssetId",
				Resource.strRuntimeAssetId) ||
				!CEffectDocumentCodec::Is_SafeResourceAssetId(
					Resource.strRuntimeAssetId))
				return FailRow("runtimeAssetId");
			if (!Read_StringArrayExact(Row, "candidateBindingIds",
				CandidateBindingIds) || CandidateBindingIds.empty())
				return FailRow("candidateBindingIds");
			if (!Read_StringArrayExact(Row, "candidateBindingRowSha256",
				CandidateBindingRowHashes, true))
				return FailRow("candidateBindingRowSha256");
			if (!Read_U32(Row, "candidateBindingCount", iCandidateBindingCount) ||
				iCandidateBindingCount != CandidateBindingIds.size() ||
				iCandidateBindingCount != CandidateBindingRowHashes.size())
				return FailRow("candidateBindingCount");
			if (!Read_U64Exact(Row, "byteCount", Resource.iByteCount) ||
				Resource.iByteCount <= 128u)
				return FailRow("byteCount");
			if (!Read_ShaExact(Row, "rawSha256", Resource.strRawSha256))
				return FailRow("rawSha256");
			if (nullptr == DdsHeader ||
				!Validate_DdsHeader(*DdsHeader, Resource.iByteCount))
				return FailRow("ddsHeader");
			if (!Read_StringExact(Row, "actualCompressedFormatClassification",
				Classification))
				return FailRow("actualCompressedFormatClassification");
			if (!Read_StringExact(Row, "colorSpacePolicy", ColorSpace))
				return FailRow("colorSpacePolicy");
			if (nullptr == Srv || !Parse_DdsSrvIdentity(*Srv, Resource.ExpectedSrv) ||
				ColorSpace != Resource.ExpectedSrv.strColorSpace)
				return FailRow("actualExpectedSrvDescriptor");
			if (!Read_StringExact(Row, "resourceIdentityBasis", IdentityBasis) ||
				IdentityBasis != "CANONICAL_MAIN_RESOURCES_BYTE_EXACT_DDS")
				return FailRow("resourceIdentityBasis");
			if (!Read_BooleanExact(Row, "absolutePathRecorded", false) ||
				!Read_BooleanExact(Row, "actionTimeIoAllowed", false) ||
				!Validate_FailClosedAuthorityRow(Row))
				return FailRow("admission flags");
			if (!Read_ShaExact(Row, "rowSha256", Resource.strRowSha256))
				return FailRow("rowSha256");
			if (!Staged.emplace(Resource.strResourceAuthorityId,
				std::move(Resource)).second)
				return FailRow("duplicate resourceAuthorityId");
		}
		OutResources = std::move(Staged);
		return true;
	}

	bool Validate_PriorPolicyFixture(const Client::DATA_JSON_VALUE& Value,
		const std::string_view strExpectedPolicyId)
	{
		using namespace Client;
		if (!Has_ExactOrderedKeys(Value, {
			"fixtureKind", "policyRowId", "expectedSrv", "actualSrv",
			"numericTolerance", "decision" }))
		{
			return false;
		}
		std::string FixtureKind;
		std::string PolicyId;
		std::string Decision;
		float fTolerance = 0.f;
		const DATA_JSON_VALUE* Expected = Required(
			Value, "expectedSrv", DATA_JSON_TYPE::OBJECT);
		const DATA_JSON_VALUE* Actual = Required(
			Value, "actualSrv", DATA_JSON_TYPE::OBJECT);
		return Read_StringExact(Value, "fixtureKind", FixtureKind) &&
			FixtureKind ==
				"MATERIAL_POLICY_1X1_RGBA8_SRV_ORACLE_NOT_ACTUAL_DDS_DESCRIPTOR" &&
			Read_StringExact(Value, "policyRowId", PolicyId) &&
			PolicyId == strExpectedPolicyId && nullptr != Expected &&
			Has_ExactOrderedKeys(*Expected, { "Format", "ViewDimension",
				"MostDetailedMip", "MipLevels", "srvColorSpace" }) &&
			nullptr != Actual && Has_ExactOrderedKeys(*Actual, {
				"Format", "ViewDimension", "MostDetailedMip", "MipLevels",
				"srvColorSpace" }) &&
			Read_F32Exact(Value, "numericTolerance", fTolerance) &&
			fTolerance == 0.f && Read_StringExact(Value, "decision", Decision) &&
			Decision == "PASS";
	}

	bool Parse_RenderTextureBindings(const Client::DATA_JSON_VALUE& Value,
		const std::map<std::string,
			Client::EFFECT_RECONSTRUCTED_RENDER_TEXTURE_RESOURCE,
			std::less<>>& Resources,
		std::map<std::string,
			Client::EFFECT_RECONSTRUCTED_RENDER_TEXTURE_BINDING,
			std::less<>>& OutBindings, std::string& strOutError)
	{
		using namespace Client;
		if (!Value.Is_Array() || Value.Get_Array().size() != 77u)
		{
			strOutError = "Render-resource textureBindings denominator is invalid.";
			return false;
		}
		std::map<std::string, EFFECT_RECONSTRUCTED_RENDER_TEXTURE_BINDING,
			std::less<>> Staged;
		for (size_t Index = 0u; Index < Value.Get_Array().size(); ++Index)
		{
			const DATA_JSON_VALUE& Row = Value.Get_Array()[Index];
			if (!Has_ExactOrderedKeys(Row, {
				"bindingAuthorityId", "order", "candidateBindingId",
				"candidateBindingRowSha256", "recipeId", "materialInputFieldId",
				"samplerPolicyRowId", "samplerPolicyRowSha256",
				"materialOccurrenceIds", "sourceBindingId",
				"sourceBindingRowSha256", "sourceTextureResourceId",
				"sourceTextureResourceRowSha256", "sourceReceiptStatus",
				"runtimeAssetId", "resourceAuthorityId",
				"resourceAuthorityRowSha256", "samplerDescriptor",
				"colorSpacePolicy", "priorPolicySrvFixture",
				"actualDdsSrvDescriptor", "actualDdsByteCount",
				"actualDdsRawSha256", "bindingIdentityBasis",
				"actionTimeIoAllowed", "sourceExact",
				"runtimeExecutionAdmission", "product", "rowSha256" }))
			{
				strOutError = "Render-resource textureBindings row schema is invalid.";
				return false;
			}
			EFFECT_RECONSTRUCTED_RENDER_TEXTURE_BINDING Binding;
			std::string ColorSpace;
			std::string SourceBindingId;
			std::string SourceBindingHash;
			std::string SourceTextureId;
			std::string SourceTextureHash;
			std::string SourceReceiptStatus;
			std::string IdentityBasis;
			const DATA_JSON_VALUE* Sampler = Required(
				Row, "samplerDescriptor", DATA_JSON_TYPE::OBJECT);
			const DATA_JSON_VALUE* PriorFixture = Required(
				Row, "priorPolicySrvFixture", DATA_JSON_TYPE::OBJECT);
			const DATA_JSON_VALUE* ActualSrv = Required(
				Row, "actualDdsSrvDescriptor", DATA_JSON_TYPE::OBJECT);
			if (!Read_StringExact(Row, "bindingAuthorityId",
					Binding.strBindingAuthorityId) ||
				!Is_StableId(Binding.strBindingAuthorityId) ||
				!Read_U32(Row, "order", Binding.iOrder) || Binding.iOrder != Index ||
				!Read_StringExact(Row, "candidateBindingId",
					Binding.strCandidateBindingId) ||
				!Read_ShaExact(Row, "candidateBindingRowSha256",
					Binding.strCandidateBindingRowSha256) ||
				!Read_StringExact(Row, "recipeId", Binding.strRecipeId) ||
				!Is_StableId(Binding.strRecipeId) ||
				!Read_StringExact(Row, "materialInputFieldId",
					Binding.strMaterialInputFieldId) ||
				!Is_StableId(Binding.strMaterialInputFieldId) ||
				!Read_StringExact(Row, "samplerPolicyRowId",
					Binding.strSamplerPolicyRowId) ||
				!Is_StableId(Binding.strSamplerPolicyRowId) ||
				!Read_ShaExact(Row, "samplerPolicyRowSha256",
					Binding.strSamplerPolicyRowSha256) ||
				!Read_StringArrayExact(Row, "materialOccurrenceIds",
					Binding.MaterialOccurrenceIds) ||
				Binding.MaterialOccurrenceIds.empty() ||
				!Read_StringExact(Row, "sourceBindingId", SourceBindingId) ||
				!Read_ShaExact(Row, "sourceBindingRowSha256", SourceBindingHash) ||
				!Read_StringExact(Row, "sourceTextureResourceId", SourceTextureId) ||
				!Read_ShaExact(Row, "sourceTextureResourceRowSha256",
					SourceTextureHash) ||
				!Read_StringExact(Row, "sourceReceiptStatus", SourceReceiptStatus) ||
				(SourceReceiptStatus != "RESOLVED_EXACT_RUNTIME_COOK_RECEIPT" &&
				 SourceReceiptStatus !=
					"RESOLVED_RECONSTRUCTED_EXACT_DDS_DEPLOYMENT_RECEIPT") ||
				!Read_StringExact(Row, "runtimeAssetId",
					Binding.strRuntimeAssetId) ||
				!CEffectDocumentCodec::Is_SafeResourceAssetId(
					Binding.strRuntimeAssetId) ||
				!Read_StringExact(Row, "resourceAuthorityId",
					Binding.strResourceAuthorityId) ||
				!Read_ShaExact(Row, "resourceAuthorityRowSha256",
					Binding.strResourceAuthorityRowSha256) ||
				nullptr == Sampler || !Parse_SamplerDescriptor(*Sampler,
					Binding.SamplerDescriptor, Binding.strSamplerSrvColorSpace) ||
				!Read_StringExact(Row, "colorSpacePolicy", ColorSpace) ||
				ColorSpace != Binding.strSamplerSrvColorSpace ||
				nullptr == PriorFixture || !Validate_PriorPolicyFixture(
					*PriorFixture, Binding.strSamplerPolicyRowId) ||
				nullptr == ActualSrv ||
				!Parse_DdsSrvIdentity(*ActualSrv, Binding.ActualDdsSrv) ||
				Binding.ActualDdsSrv.strColorSpace != ColorSpace ||
				!Read_U64Exact(Row, "actualDdsByteCount",
					Binding.iActualDdsByteCount) ||
				!Read_ShaExact(Row, "actualDdsRawSha256",
					Binding.strActualDdsRawSha256) ||
				!Read_StringExact(Row, "bindingIdentityBasis", IdentityBasis) ||
				IdentityBasis !=
					"FROZEN_MATERIAL_BINDING_ROW_AND_CANONICAL_MAIN_RESOURCES_BYTES" ||
				!Read_BooleanExact(Row, "actionTimeIoAllowed", false) ||
				!Validate_FailClosedAuthorityRow(Row) ||
				!Read_ShaExact(Row, "rowSha256", Binding.strRowSha256))
			{
				strOutError = "Render-resource textureBindings row is invalid.";
				return false;
			}
			const auto ResourceIt = Resources.find(Binding.strResourceAuthorityId);
			if (Resources.end() == ResourceIt ||
				ResourceIt->second.strRowSha256 !=
					Binding.strResourceAuthorityRowSha256 ||
				ResourceIt->second.strRuntimeAssetId != Binding.strRuntimeAssetId ||
				ResourceIt->second.strRawSha256 != Binding.strActualDdsRawSha256 ||
				ResourceIt->second.iByteCount != Binding.iActualDdsByteCount ||
				!Same_DdsSrvIdentity(
					ResourceIt->second.ExpectedSrv, Binding.ActualDdsSrv) ||
				!Staged.emplace(Binding.strBindingAuthorityId,
					std::move(Binding)).second)
			{
				strOutError =
					"Render-resource texture binding/resource join is invalid.";
				return false;
			}
		}
		OutBindings = std::move(Staged);
		return true;
	}

	bool Parse_RenderNeutralProviders(const Client::DATA_JSON_VALUE& Value,
		std::map<std::string,
			Client::EFFECT_RECONSTRUCTED_RENDER_NEUTRAL_PROVIDER,
			std::less<>>& OutProviders, std::string& strOutError)
	{
		using namespace Client;
		if (!Value.Is_Array() || Value.Get_Array().size() != 4u)
		{
			strOutError = "Render-resource neutralProviders denominator is invalid.";
			return false;
		}
		std::map<std::string, EFFECT_RECONSTRUCTED_RENDER_NEUTRAL_PROVIDER,
			std::less<>> Staged;
		for (size_t Index = 0u; Index < Value.Get_Array().size(); ++Index)
		{
			const DATA_JSON_VALUE& Row = Value.Get_Array()[Index];
			if (!Has_ExactOrderedKeys(Row, {
				"neutralProviderId", "order", "rgbaF32", "evaluatorSemantic",
				"secondaryMultiplyFactor", "signedDistortionOffset", "rationale",
				"sourceExact", "requiresAutomatedWARPProbe",
				"requiresManualEyeValidation", "runtimeExecutionAdmission",
				"product", "rowSha256" }))
			{
				strOutError = "Render-resource neutralProviders row schema is invalid.";
				return false;
			}
			EFFECT_RECONSTRUCTED_RENDER_NEUTRAL_PROVIDER Provider;
			std::string Rationale;
			const DATA_JSON_VALUE* Rgba = Required(
				Row, "rgbaF32", DATA_JSON_TYPE::ARRAY);
			if (!Read_StringExact(Row, "neutralProviderId",
					Provider.strNeutralProviderId) ||
				!Is_StableId(Provider.strNeutralProviderId) ||
				!Read_U32(Row, "order", Provider.iOrder) || Provider.iOrder != Index ||
				nullptr == Rgba || Rgba->Get_Array().size() != 4u ||
				!Read_StringExact(Row, "evaluatorSemantic",
					Provider.strEvaluatorSemantic) ||
				!Read_F32Exact(Row, "secondaryMultiplyFactor",
					Provider.fSecondaryMultiplyFactor) ||
				!Read_F32Exact(Row, "signedDistortionOffset",
					Provider.fSignedDistortionOffset) ||
				!Read_StringExact(Row, "rationale", Rationale, false, 8192u) ||
				!Validate_FailClosedAuthorityRow(Row, true) ||
				!Read_ShaExact(Row, "rowSha256", Provider.strRowSha256))
			{
				strOutError = "Render-resource neutralProviders row is invalid.";
				return false;
			}
			for (size_t Component = 0u; Component < 4u; ++Component)
			{
				const DATA_JSON_VALUE& Item = Rgba->Get_Array()[Component];
				if (!Item.Is_Number() || !Item.Was_FloatingPointToken() ||
					!std::isfinite(Item.Get_Number()) ||
					Item.Get_Number() < -static_cast<double>(FLT_MAX) ||
					Item.Get_Number() > static_cast<double>(FLT_MAX))
				{
					strOutError =
						"Render-resource neutralProviders RGBA is invalid.";
					return false;
				}
				Provider.RgbaF32[Component] =
					static_cast<float>(Item.Get_Number());
			}
			if (!Staged.emplace(Provider.strNeutralProviderId,
				std::move(Provider)).second)
			{
				strOutError = "Render-resource neutralProvider ID is duplicate.";
				return false;
			}
		}
		OutProviders = std::move(Staged);
		return true;
	}

	bool Parse_RenderTextureProvider(const Client::DATA_JSON_VALUE& Value,
		const std::map<std::string,
			Client::EFFECT_RECONSTRUCTED_RENDER_TEXTURE_BINDING,
			std::less<>>& Bindings,
		const std::map<std::string,
			Client::EFFECT_RECONSTRUCTED_RENDER_NEUTRAL_PROVIDER,
			std::less<>>& NeutralProviders,
		Client::EFFECT_RECONSTRUCTED_RENDER_TEXTURE_PROVIDER& OutProvider)
	{
		using namespace Client;
		if (!Has_ExactOrderedKeys(Value, {
			"providerKind", "neutralProviderId", "materialInputFieldId",
			"materialInputRowSha256", "textureBindingId",
			"textureBindingRowSha256", "samplerPolicyRowId",
			"samplerPolicyRowSha256", "runtimeAssetId", "selectionBasis" }))
		{
			return false;
		}
		EFFECT_RECONSTRUCTED_RENDER_TEXTURE_PROVIDER Staged;
		if (!Read_StringExact(Value, "providerKind", Staged.strProviderKind) ||
			!Read_StringExact(Value, "neutralProviderId",
				Staged.strNeutralProviderId, true) ||
			!Read_StringExact(Value, "materialInputFieldId",
				Staged.strMaterialInputFieldId, true) ||
			!Read_StringExact(Value, "materialInputRowSha256",
				Staged.strMaterialInputRowSha256, true) ||
			!Read_StringExact(Value, "textureBindingId",
				Staged.strTextureBindingId, true) ||
			!Read_StringExact(Value, "textureBindingRowSha256",
				Staged.strTextureBindingRowSha256, true) ||
			!Read_StringExact(Value, "samplerPolicyRowId",
				Staged.strSamplerPolicyRowId, true) ||
			!Read_StringExact(Value, "samplerPolicyRowSha256",
				Staged.strSamplerPolicyRowSha256, true) ||
			!Read_StringExact(Value, "runtimeAssetId",
				Staged.strRuntimeAssetId, true) ||
			!Read_StringExact(Value, "selectionBasis", Staged.strSelectionBasis))
		{
			return false;
		}
		if (Staged.strProviderKind == "NEUTRAL_CONSTANT")
		{
			if (!NeutralProviders.contains(Staged.strNeutralProviderId) ||
				!Staged.strMaterialInputFieldId.empty() ||
				!Staged.strMaterialInputRowSha256.empty() ||
				!Staged.strTextureBindingId.empty() ||
				!Staged.strTextureBindingRowSha256.empty() ||
				!Staged.strSamplerPolicyRowId.empty() ||
				!Staged.strSamplerPolicyRowSha256.empty() ||
				!Staged.strRuntimeAssetId.empty())
			{
				return false;
			}
		}
		else if (Staged.strProviderKind == "MATERIAL_TEXTURE_BINDING")
		{
			if (!Staged.strNeutralProviderId.empty() ||
				!Is_StableId(Staged.strMaterialInputFieldId) ||
				!Is_LowerHexSha256(Staged.strMaterialInputRowSha256) ||
				Staged.strTextureBindingId.empty() ||
				!Is_LowerHexSha256(Staged.strTextureBindingRowSha256) ||
				!Is_StableId(Staged.strSamplerPolicyRowId) ||
				!Is_LowerHexSha256(Staged.strSamplerPolicyRowSha256) ||
				!CEffectDocumentCodec::Is_SafeResourceAssetId(
					Staged.strRuntimeAssetId))
			{
				return false;
			}
			const auto Match = std::find_if(Bindings.begin(), Bindings.end(),
				[&Staged](const auto& Item)
				{
					const EFFECT_RECONSTRUCTED_RENDER_TEXTURE_BINDING& Binding =
						Item.second;
					return Binding.strCandidateBindingId ==
							Staged.strTextureBindingId &&
						Binding.strCandidateBindingRowSha256 ==
							Staged.strTextureBindingRowSha256 &&
						Binding.strMaterialInputFieldId ==
							Staged.strMaterialInputFieldId &&
						Binding.strSamplerPolicyRowId ==
							Staged.strSamplerPolicyRowId &&
						Binding.strSamplerPolicyRowSha256 ==
							Staged.strSamplerPolicyRowSha256 &&
						Binding.strRuntimeAssetId == Staged.strRuntimeAssetId;
				});
			if (Bindings.end() == Match)
				return false;
		}
		else
		{
			return false;
		}
		OutProvider = std::move(Staged);
		return true;
	}

	bool Parse_RenderRecipeTextureBindings(const Client::DATA_JSON_VALUE& Value,
		const std::map<std::string,
			Client::EFFECT_RECONSTRUCTED_RENDER_TEXTURE_BINDING,
			std::less<>>& Bindings,
		const std::map<std::string,
			Client::EFFECT_RECONSTRUCTED_RENDER_NEUTRAL_PROVIDER,
			std::less<>>& NeutralProviders,
		std::map<std::string,
			Client::EFFECT_RECONSTRUCTED_RENDER_RECIPE_TEXTURE_BINDING,
			std::less<>>& OutRecipes, std::string& strOutError)
	{
		using namespace Client;
		if (!Value.Is_Array() || Value.Get_Array().size() != 27u)
		{
			strOutError =
				"Render-resource recipeTextureBindings denominator is invalid.";
			return false;
		}
		std::map<std::string,
			EFFECT_RECONSTRUCTED_RENDER_RECIPE_TEXTURE_BINDING,
			std::less<>> Staged;
		for (size_t Index = 0u; Index < Value.Get_Array().size(); ++Index)
		{
			const DATA_JSON_VALUE& Row = Value.Get_Array()[Index];
			if (!Has_ExactOrderedKeys(Row, {
				"recipeTextureDecisionId", "order", "recipeId", "recipeRowSha256",
				"familyId", "familyRowSha256", "featureMask",
				"secondTextureOperationEnabled", "distortionOperationEnabled",
				"distortionStrengthF32", "candidateTextureBindingIds",
				"candidateTextureBindingRowSha256", "texture0Provider",
				"texture1Provider", "neutralFallbackDecision", "decisionBasis",
				"sourceExact", "requiresAutomatedWARPProbe",
				"requiresManualEyeValidation", "runtimeExecutionAdmission",
				"product", "rowSha256" }))
			{
				strOutError =
					"Render-resource recipeTextureBindings row schema is invalid.";
				return false;
			}
			EFFECT_RECONSTRUCTED_RENDER_RECIPE_TEXTURE_BINDING Recipe;
			std::vector<std::string> CandidateBindingIds;
			std::vector<std::string> CandidateBindingHashes;
			std::string DecisionBasis;
			const DATA_JSON_VALUE* Texture0 = Required(
				Row, "texture0Provider", DATA_JSON_TYPE::OBJECT);
			const DATA_JSON_VALUE* Texture1 = Required(
				Row, "texture1Provider", DATA_JSON_TYPE::OBJECT);
			const DATA_JSON_VALUE* Fallback = Required(
				Row, "neutralFallbackDecision", DATA_JSON_TYPE::OBJECT);
			const DATA_JSON_VALUE* SecondTextureEnabled = Required(
				Row, "secondTextureOperationEnabled", DATA_JSON_TYPE::BOOLEAN);
			const DATA_JSON_VALUE* DistortionEnabled = Required(
				Row, "distortionOperationEnabled", DATA_JSON_TYPE::BOOLEAN);
			if (!Read_StringExact(Row, "recipeTextureDecisionId",
					Recipe.strRecipeTextureDecisionId) ||
				!Is_StableId(Recipe.strRecipeTextureDecisionId) ||
				!Read_U32(Row, "order", Recipe.iOrder) || Recipe.iOrder != Index ||
				!Read_StringExact(Row, "recipeId", Recipe.strRecipeId) ||
				!Is_StableId(Recipe.strRecipeId) ||
				!Read_ShaExact(Row, "recipeRowSha256", Recipe.strRecipeRowSha256) ||
				!Read_StringExact(Row, "familyId", Recipe.strFamilyId) ||
				!Is_StableId(Recipe.strFamilyId) ||
				!Read_ShaExact(Row, "familyRowSha256", Recipe.strFamilyRowSha256) ||
				!Read_U32(Row, "featureMask", Recipe.iFeatureMask) ||
				nullptr == SecondTextureEnabled || nullptr == DistortionEnabled ||
				!Read_F32Exact(Row, "distortionStrengthF32",
					Recipe.fDistortionStrength) ||
				!Read_StringArrayExact(Row, "candidateTextureBindingIds",
					CandidateBindingIds) ||
				!Read_StringArrayExact(Row, "candidateTextureBindingRowSha256",
					CandidateBindingHashes, true) ||
				CandidateBindingIds.size() != CandidateBindingHashes.size() ||
				nullptr == Texture0 || !Parse_RenderTextureProvider(
					*Texture0, Bindings, NeutralProviders, Recipe.Texture0Provider) ||
				nullptr == Texture1 || !Parse_RenderTextureProvider(
					*Texture1, Bindings, NeutralProviders, Recipe.Texture1Provider) ||
				nullptr == Fallback || !Has_ExactOrderedKeys(*Fallback, {
					"texture0NeutralProviderId", "texture1NeutralProviderId",
					"neutralProviderApplicationPolicy",
					"materialBindingFailurePolicy", "rationale" }) ||
				!Read_StringExact(Row, "decisionBasis", DecisionBasis) ||
				!Validate_FailClosedAuthorityRow(Row, true) ||
				!Read_ShaExact(Row, "rowSha256", Recipe.strRowSha256))
			{
				strOutError = "Render-resource recipeTextureBindings row is invalid.";
				return false;
			}
			Recipe.bSecondTextureOperationEnabled =
				SecondTextureEnabled->Get_Boolean();
			Recipe.bDistortionOperationEnabled =
				DistortionEnabled->Get_Boolean();
			std::string NeutralPolicy;
			std::string FailurePolicy;
			if (!Read_StringExact(*Fallback, "neutralProviderApplicationPolicy",
					NeutralPolicy) || NeutralPolicy !=
					"ONLY_WHEN_THIS_APPROVAL_EXPLICITLY_SELECTS_NEUTRAL" ||
				!Read_StringExact(*Fallback, "materialBindingFailurePolicy",
					FailurePolicy) ||
				FailurePolicy != "FAIL_CLOSED_TRANSACTION_ROLLBACK" ||
				!Staged.emplace(Recipe.strRecipeTextureDecisionId,
					std::move(Recipe)).second)
			{
				strOutError =
					"Render-resource recipe texture fallback or ID is invalid.";
				return false;
			}
		}
		OutRecipes = std::move(Staged);
		return true;
	}

	bool Parse_RendererSlotBindings(const Client::DATA_JSON_VALUE& Value,
		const std::map<std::string,
			Client::EFFECT_RECONSTRUCTED_RENDER_TEXTURE_BINDING,
			std::less<>>& Bindings,
		std::map<std::string,
			Client::EFFECT_RECONSTRUCTED_RENDERER_SLOT_BINDING,
			std::less<>>& OutBindings, std::string& strOutError)
	{
		using namespace Client;
		if (!Value.Is_Array() || Value.Get_Array().size() != 57u)
		{
			strOutError =
				"Render-resource rendererSlotBindings denominator is invalid.";
			return false;
		}
		std::map<std::string, EFFECT_RECONSTRUCTED_RENDERER_SLOT_BINDING,
			std::less<>> Staged;
		for (size_t Index = 0u; Index < Value.Get_Array().size(); ++Index)
		{
			const DATA_JSON_VALUE& Row = Value.Get_Array()[Index];
			if (!Has_ExactOrderedKeys(Row, {
				"rendererBindingDecisionId", "order", "textureResourceId",
				"rendererResourceRowSha256", "materialOccurrenceId",
				"materialOccurrenceRowSha256", "recipeId", "slotId",
				"runtimeAssetId", "candidateCount", "candidates",
				"selectedMaterialInputFieldId", "selectedMaterialInputRowSha256",
				"selectedNormalizedParameterName", "selectedTextureBindingId",
				"selectedTextureBindingRowSha256", "selectedSamplerPolicyRowId",
				"selectedSamplerPolicyRowSha256", "decisionBasis", "rationale",
				"sourceExact", "requiresAutomatedWARPProbe",
				"requiresManualEyeValidation", "runtimeExecutionAdmission",
				"product", "rowSha256" }))
			{
				strOutError =
					"Render-resource rendererSlotBindings row schema is invalid.";
				return false;
			}
			EFFECT_RECONSTRUCTED_RENDERER_SLOT_BINDING Binding;
			std::string Rationale;
			const DATA_JSON_VALUE* Candidates = Required(
				Row, "candidates", DATA_JSON_TYPE::ARRAY);
			if (!Read_StringExact(Row, "rendererBindingDecisionId",
					Binding.strRendererBindingDecisionId) ||
				!Is_StableId(Binding.strRendererBindingDecisionId) ||
				!Read_U32(Row, "order", Binding.iOrder) || Binding.iOrder != Index ||
				!Read_StringExact(Row, "textureResourceId",
					Binding.strTextureResourceId, false, 4096u) ||
				!Read_ShaExact(Row, "rendererResourceRowSha256",
					Binding.strRendererResourceRowSha256) ||
				!Read_StringExact(Row, "materialOccurrenceId",
					Binding.strMaterialOccurrenceId) ||
				!Is_StableId(Binding.strMaterialOccurrenceId) ||
				!Read_ShaExact(Row, "materialOccurrenceRowSha256",
					Binding.strMaterialOccurrenceRowSha256) ||
				!Read_StringExact(Row, "recipeId", Binding.strRecipeId) ||
				!Is_StableId(Binding.strRecipeId) ||
				!Read_StringExact(Row, "slotId", Binding.strSlotId) ||
				!Read_StringExact(Row, "runtimeAssetId",
					Binding.strRuntimeAssetId) ||
				!CEffectDocumentCodec::Is_SafeResourceAssetId(
					Binding.strRuntimeAssetId) ||
				!Read_U32(Row, "candidateCount", Binding.iCandidateCount) ||
				Binding.iCandidateCount < 1u || Binding.iCandidateCount > 2u ||
				nullptr == Candidates ||
				Candidates->Get_Array().size() != Binding.iCandidateCount ||
				!Read_StringExact(Row, "selectedMaterialInputFieldId",
					Binding.strSelectedMaterialInputFieldId) ||
				!Read_ShaExact(Row, "selectedMaterialInputRowSha256",
					Binding.strSelectedMaterialInputRowSha256) ||
				!Read_StringExact(Row, "selectedNormalizedParameterName",
					Binding.strSelectedNormalizedParameterName) ||
				!Read_StringExact(Row, "selectedTextureBindingId",
					Binding.strSelectedTextureBindingId) ||
				!Read_ShaExact(Row, "selectedTextureBindingRowSha256",
					Binding.strSelectedTextureBindingRowSha256) ||
				!Read_StringExact(Row, "selectedSamplerPolicyRowId",
					Binding.strSelectedSamplerPolicyRowId) ||
				!Read_ShaExact(Row, "selectedSamplerPolicyRowSha256",
					Binding.strSelectedSamplerPolicyRowSha256) ||
				!Read_StringExact(Row, "decisionBasis", Binding.strDecisionBasis) ||
				!Read_StringExact(Row, "rationale", Rationale, false, 8192u) ||
				!Validate_FailClosedAuthorityRow(Row, true) ||
				!Read_ShaExact(Row, "rowSha256", Binding.strRowSha256))
			{
				strOutError = "Render-resource rendererSlotBindings row is invalid.";
				return false;
			}
			bool bSelectedCandidatePresent = false;
			for (const DATA_JSON_VALUE& Candidate : Candidates->Get_Array())
			{
				if (!Has_ExactOrderedKeys(Candidate, {
					"materialInputFieldId", "materialInputRowSha256",
					"textureBindingId", "textureBindingRowSha256",
					"normalizedParameterName" }))
				{
					strOutError = "Render-resource renderer candidate schema is invalid.";
					return false;
				}
				std::string MaterialInputId;
				std::string MaterialInputHash;
				std::string TextureBindingId;
				std::string TextureBindingHash;
				std::string ParameterName;
				if (!Read_StringExact(Candidate, "materialInputFieldId",
						MaterialInputId) ||
					!Read_ShaExact(Candidate, "materialInputRowSha256",
						MaterialInputHash) ||
					!Read_StringExact(Candidate, "textureBindingId",
						TextureBindingId) ||
					!Read_ShaExact(Candidate, "textureBindingRowSha256",
						TextureBindingHash) ||
					!Read_StringExact(Candidate, "normalizedParameterName",
						ParameterName))
				{
					strOutError = "Render-resource renderer candidate is invalid.";
					return false;
				}
				bSelectedCandidatePresent = bSelectedCandidatePresent ||
					(MaterialInputId == Binding.strSelectedMaterialInputFieldId &&
					 MaterialInputHash == Binding.strSelectedMaterialInputRowSha256 &&
					 TextureBindingId == Binding.strSelectedTextureBindingId &&
					 TextureBindingHash ==
						Binding.strSelectedTextureBindingRowSha256 &&
					 ParameterName == Binding.strSelectedNormalizedParameterName);
			}
			const auto SelectedBinding = std::find_if(
				Bindings.begin(), Bindings.end(), [&Binding](const auto& Item)
				{
					const EFFECT_RECONSTRUCTED_RENDER_TEXTURE_BINDING& Candidate =
						Item.second;
					return Candidate.strCandidateBindingId ==
							Binding.strSelectedTextureBindingId &&
						Candidate.strCandidateBindingRowSha256 ==
							Binding.strSelectedTextureBindingRowSha256 &&
						Candidate.strMaterialInputFieldId ==
							Binding.strSelectedMaterialInputFieldId &&
						Candidate.strSamplerPolicyRowId ==
							Binding.strSelectedSamplerPolicyRowId &&
						Candidate.strSamplerPolicyRowSha256 ==
							Binding.strSelectedSamplerPolicyRowSha256 &&
						Candidate.strRuntimeAssetId == Binding.strRuntimeAssetId;
				});
			if (!bSelectedCandidatePresent || Bindings.end() == SelectedBinding ||
				!Staged.emplace(Binding.strRendererBindingDecisionId,
					std::move(Binding)).second)
			{
				strOutError =
					"Render-resource renderer selected binding join is invalid.";
				return false;
			}
		}
		OutBindings = std::move(Staged);
		return true;
	}

	bool Parse_BlendDescriptor(const Client::DATA_JSON_VALUE& Value,
		D3D11_BLEND_DESC& OutDescriptor)
	{
		using namespace Client;
		if (!Has_ExactOrderedKeys(Value, {
			"AlphaToCoverageEnable", "IndependentBlendEnable", "RenderTarget" }))
		{
			return false;
		}
		const DATA_JSON_VALUE* AlphaToCoverage = Required(
			Value, "AlphaToCoverageEnable", DATA_JSON_TYPE::BOOLEAN);
		const DATA_JSON_VALUE* IndependentBlend = Required(
			Value, "IndependentBlendEnable", DATA_JSON_TYPE::BOOLEAN);
		const DATA_JSON_VALUE* RenderTargets = Required(
			Value, "RenderTarget", DATA_JSON_TYPE::ARRAY);
		if (nullptr == AlphaToCoverage || nullptr == IndependentBlend ||
			nullptr == RenderTargets || RenderTargets->Get_Array().size() != 8u)
		{
			return false;
		}
		D3D11_BLEND_DESC Staged{};
		Staged.AlphaToCoverageEnable = AlphaToCoverage->Get_Boolean();
		Staged.IndependentBlendEnable = IndependentBlend->Get_Boolean();
		for (size_t Index = 0u; Index < 8u; ++Index)
		{
			const DATA_JSON_VALUE& Row = RenderTargets->Get_Array()[Index];
			if (!Has_ExactOrderedKeys(Row, {
				"BlendEnable", "SrcBlend", "DestBlend", "BlendOp",
				"SrcBlendAlpha", "DestBlendAlpha", "BlendOpAlpha",
				"RenderTargetWriteMask" }))
			{
				return false;
			}
			const DATA_JSON_VALUE* BlendEnabled = Required(
				Row, "BlendEnable", DATA_JSON_TYPE::BOOLEAN);
			uint32_t iSrcBlend = 0u;
			uint32_t iDestBlend = 0u;
			uint32_t iBlendOp = 0u;
			uint32_t iSrcBlendAlpha = 0u;
			uint32_t iDestBlendAlpha = 0u;
			uint32_t iBlendOpAlpha = 0u;
			uint32_t iWriteMask = 0u;
			if (nullptr == BlendEnabled ||
				!Read_U32(Row, "SrcBlend", iSrcBlend) ||
				!Read_U32(Row, "DestBlend", iDestBlend) ||
				!Read_U32(Row, "BlendOp", iBlendOp) ||
				!Read_U32(Row, "SrcBlendAlpha", iSrcBlendAlpha) ||
				!Read_U32(Row, "DestBlendAlpha", iDestBlendAlpha) ||
				!Read_U32(Row, "BlendOpAlpha", iBlendOpAlpha) ||
				!Read_U32(Row, "RenderTargetWriteMask", iWriteMask) ||
				iSrcBlend < D3D11_BLEND_ZERO || iSrcBlend > D3D11_BLEND_INV_SRC1_ALPHA ||
				iDestBlend < D3D11_BLEND_ZERO ||
				iDestBlend > D3D11_BLEND_INV_SRC1_ALPHA ||
				iBlendOp < D3D11_BLEND_OP_ADD || iBlendOp > D3D11_BLEND_OP_MAX ||
				iSrcBlendAlpha < D3D11_BLEND_ZERO ||
				iSrcBlendAlpha > D3D11_BLEND_INV_SRC1_ALPHA ||
				iDestBlendAlpha < D3D11_BLEND_ZERO ||
				iDestBlendAlpha > D3D11_BLEND_INV_SRC1_ALPHA ||
				iBlendOpAlpha < D3D11_BLEND_OP_ADD ||
				iBlendOpAlpha > D3D11_BLEND_OP_MAX || iWriteMask > 0x0fu)
			{
				return false;
			}
			D3D11_RENDER_TARGET_BLEND_DESC& Target = Staged.RenderTarget[Index];
			Target.BlendEnable = BlendEnabled->Get_Boolean();
			Target.SrcBlend = static_cast<D3D11_BLEND>(iSrcBlend);
			Target.DestBlend = static_cast<D3D11_BLEND>(iDestBlend);
			Target.BlendOp = static_cast<D3D11_BLEND_OP>(iBlendOp);
			Target.SrcBlendAlpha = static_cast<D3D11_BLEND>(iSrcBlendAlpha);
			Target.DestBlendAlpha = static_cast<D3D11_BLEND>(iDestBlendAlpha);
			Target.BlendOpAlpha = static_cast<D3D11_BLEND_OP>(iBlendOpAlpha);
			Target.RenderTargetWriteMask = static_cast<UINT8>(iWriteMask);
		}
		OutDescriptor = Staged;
		return true;
	}

	bool Parse_RasterizerDescriptor(const Client::DATA_JSON_VALUE& Value,
		D3D11_RASTERIZER_DESC& OutDescriptor)
	{
		using namespace Client;
		if (!Has_ExactOrderedKeys(Value, {
			"FillMode", "CullMode", "FrontCounterClockwise", "DepthBias",
			"DepthBiasClamp", "SlopeScaledDepthBias", "DepthClipEnable",
			"ScissorEnable", "MultisampleEnable", "AntialiasedLineEnable" }))
		{
			return false;
		}
		uint32_t iFillMode = 0u;
		uint32_t iCullMode = 0u;
		int32_t iDepthBias = 0;
		float fDepthBiasClamp = 0.f;
		float fSlopeScaledDepthBias = 0.f;
		const DATA_JSON_VALUE* FrontCounterClockwise = Required(
			Value, "FrontCounterClockwise", DATA_JSON_TYPE::BOOLEAN);
		const DATA_JSON_VALUE* DepthClip = Required(
			Value, "DepthClipEnable", DATA_JSON_TYPE::BOOLEAN);
		const DATA_JSON_VALUE* Scissor = Required(
			Value, "ScissorEnable", DATA_JSON_TYPE::BOOLEAN);
		const DATA_JSON_VALUE* Multisample = Required(
			Value, "MultisampleEnable", DATA_JSON_TYPE::BOOLEAN);
		const DATA_JSON_VALUE* AntialiasedLine = Required(
			Value, "AntialiasedLineEnable", DATA_JSON_TYPE::BOOLEAN);
		if (!Read_U32(Value, "FillMode", iFillMode) ||
			!Read_U32(Value, "CullMode", iCullMode) ||
			!Read_I32Exact(Value, "DepthBias", iDepthBias) ||
			!Read_F32Exact(Value, "DepthBiasClamp", fDepthBiasClamp) ||
			!Read_F32Exact(Value, "SlopeScaledDepthBias",
				fSlopeScaledDepthBias) ||
			nullptr == FrontCounterClockwise || nullptr == DepthClip ||
			nullptr == Scissor || nullptr == Multisample ||
			nullptr == AntialiasedLine ||
			(iFillMode != D3D11_FILL_WIREFRAME && iFillMode != D3D11_FILL_SOLID) ||
			iCullMode < D3D11_CULL_NONE || iCullMode > D3D11_CULL_BACK)
		{
			return false;
		}
		D3D11_RASTERIZER_DESC Staged{};
		Staged.FillMode = static_cast<D3D11_FILL_MODE>(iFillMode);
		Staged.CullMode = static_cast<D3D11_CULL_MODE>(iCullMode);
		Staged.FrontCounterClockwise = FrontCounterClockwise->Get_Boolean();
		Staged.DepthBias = iDepthBias;
		Staged.DepthBiasClamp = fDepthBiasClamp;
		Staged.SlopeScaledDepthBias = fSlopeScaledDepthBias;
		Staged.DepthClipEnable = DepthClip->Get_Boolean();
		Staged.ScissorEnable = Scissor->Get_Boolean();
		Staged.MultisampleEnable = Multisample->Get_Boolean();
		Staged.AntialiasedLineEnable = AntialiasedLine->Get_Boolean();
		OutDescriptor = Staged;
		return true;
	}

	bool Parse_DepthStencilFaceDescriptor(const Client::DATA_JSON_VALUE& Value,
		D3D11_DEPTH_STENCILOP_DESC& OutDescriptor)
	{
		if (!Has_ExactOrderedKeys(Value, {
			"StencilFailOp", "StencilDepthFailOp", "StencilPassOp",
			"StencilFunc" }))
		{
			return false;
		}
		uint32_t iFail = 0u;
		uint32_t iDepthFail = 0u;
		uint32_t iPass = 0u;
		uint32_t iFunc = 0u;
		if (!Read_U32(Value, "StencilFailOp", iFail) ||
			!Read_U32(Value, "StencilDepthFailOp", iDepthFail) ||
			!Read_U32(Value, "StencilPassOp", iPass) ||
			!Read_U32(Value, "StencilFunc", iFunc) ||
			iFail < D3D11_STENCIL_OP_KEEP || iFail > D3D11_STENCIL_OP_DECR ||
			iDepthFail < D3D11_STENCIL_OP_KEEP ||
			iDepthFail > D3D11_STENCIL_OP_DECR ||
			iPass < D3D11_STENCIL_OP_KEEP || iPass > D3D11_STENCIL_OP_DECR ||
			iFunc < D3D11_COMPARISON_NEVER || iFunc > D3D11_COMPARISON_ALWAYS)
		{
			return false;
		}
		OutDescriptor.StencilFailOp = static_cast<D3D11_STENCIL_OP>(iFail);
		OutDescriptor.StencilDepthFailOp =
			static_cast<D3D11_STENCIL_OP>(iDepthFail);
		OutDescriptor.StencilPassOp = static_cast<D3D11_STENCIL_OP>(iPass);
		OutDescriptor.StencilFunc = static_cast<D3D11_COMPARISON_FUNC>(iFunc);
		return true;
	}

	bool Parse_DepthStencilDescriptor(const Client::DATA_JSON_VALUE& Value,
		D3D11_DEPTH_STENCIL_DESC& OutDescriptor)
	{
		using namespace Client;
		if (!Has_ExactOrderedKeys(Value, {
			"DepthEnable", "DepthWriteMask", "DepthFunc", "StencilEnable",
			"StencilReadMask", "StencilWriteMask", "FrontFace", "BackFace" }))
		{
			return false;
		}
		const DATA_JSON_VALUE* DepthEnable = Required(
			Value, "DepthEnable", DATA_JSON_TYPE::BOOLEAN);
		const DATA_JSON_VALUE* StencilEnable = Required(
			Value, "StencilEnable", DATA_JSON_TYPE::BOOLEAN);
		const DATA_JSON_VALUE* Front = Required(
			Value, "FrontFace", DATA_JSON_TYPE::OBJECT);
		const DATA_JSON_VALUE* Back = Required(
			Value, "BackFace", DATA_JSON_TYPE::OBJECT);
		uint32_t iWriteMask = 0u;
		uint32_t iDepthFunc = 0u;
		uint32_t iReadMask = 0u;
		uint32_t iStencilWriteMask = 0u;
		D3D11_DEPTH_STENCIL_DESC Staged{};
		if (nullptr == DepthEnable || nullptr == StencilEnable ||
			nullptr == Front || nullptr == Back ||
			!Read_U32(Value, "DepthWriteMask", iWriteMask) ||
			iWriteMask > D3D11_DEPTH_WRITE_MASK_ALL ||
			!Read_U32(Value, "DepthFunc", iDepthFunc) ||
			iDepthFunc < D3D11_COMPARISON_NEVER ||
			iDepthFunc > D3D11_COMPARISON_ALWAYS ||
			!Read_U32(Value, "StencilReadMask", iReadMask) || iReadMask > 0xffu ||
			!Read_U32(Value, "StencilWriteMask", iStencilWriteMask) ||
			iStencilWriteMask > 0xffu ||
			!Parse_DepthStencilFaceDescriptor(*Front, Staged.FrontFace) ||
			!Parse_DepthStencilFaceDescriptor(*Back, Staged.BackFace))
		{
			return false;
		}
		Staged.DepthEnable = DepthEnable->Get_Boolean();
		Staged.DepthWriteMask = static_cast<D3D11_DEPTH_WRITE_MASK>(iWriteMask);
		Staged.DepthFunc = static_cast<D3D11_COMPARISON_FUNC>(iDepthFunc);
		Staged.StencilEnable = StencilEnable->Get_Boolean();
		Staged.StencilReadMask = static_cast<UINT8>(iReadMask);
		Staged.StencilWriteMask = static_cast<UINT8>(iStencilWriteMask);
		OutDescriptor = Staged;
		return true;
	}

	bool Parse_RenderStateDescriptors(const Client::DATA_JSON_VALUE& Value,
		std::map<std::string,
			Client::EFFECT_RECONSTRUCTED_RENDER_STATE_DESCRIPTOR,
			std::less<>>& OutDescriptors, std::string& strOutError)
	{
		using namespace Client;
		if (!Value.Is_Array() || Value.Get_Array().size() != 46u)
		{
			strOutError =
				"Render-resource renderStateDescriptors denominator is invalid.";
			return false;
		}
		std::map<std::string, EFFECT_RECONSTRUCTED_RENDER_STATE_DESCRIPTOR,
			std::less<>> Staged;
		for (size_t Index = 0u; Index < Value.Get_Array().size(); ++Index)
		{
			const DATA_JSON_VALUE& Row = Value.Get_Array()[Index];
			if (!Has_ExactOrderedKeys(Row, {
				"renderStateDecisionId", "order", "renderBindingId",
				"renderBindingRowSha256", "recipeId", "recipeRowSha256",
				"fieldName", "approvedFieldValue", "descriptorKind",
				"standardMappingId", "implementationReferencePath",
				"implementationReferenceTrackedTextSha256",
				"implementationStateName", "expectedDescriptor", "decisionBasis",
				"sourceExact", "requiresAutomatedWARPProbe",
				"requiresManualEyeValidation", "runtimeExecutionAdmission",
				"product", "rowSha256" }))
			{
				strOutError =
					"Render-resource renderStateDescriptors row schema is invalid.";
				return false;
			}
			EFFECT_RECONSTRUCTED_RENDER_STATE_DESCRIPTOR Descriptor;
			std::string DescriptorKind;
			std::string StandardMappingId;
			std::string ReferencePath;
			std::string ReferenceHash;
			std::string DecisionBasis;
			const DATA_JSON_VALUE* ApprovedValue = Row.Find("approvedFieldValue");
			const DATA_JSON_VALUE* Expected = Required(
				Row, "expectedDescriptor", DATA_JSON_TYPE::OBJECT);
			if (!Read_StringExact(Row, "renderStateDecisionId",
					Descriptor.strRenderStateDecisionId) ||
				!Is_StableId(Descriptor.strRenderStateDecisionId) ||
				!Read_U32(Row, "order", Descriptor.iOrder) ||
				Descriptor.iOrder != Index ||
				!Read_StringExact(Row, "renderBindingId",
					Descriptor.strRenderBindingId) ||
				!Read_ShaExact(Row, "renderBindingRowSha256",
					Descriptor.strRenderBindingRowSha256) ||
				!Read_StringExact(Row, "recipeId", Descriptor.strRecipeId) ||
				!Is_StableId(Descriptor.strRecipeId) ||
				!Read_ShaExact(Row, "recipeRowSha256",
					Descriptor.strRecipeRowSha256) ||
				!Read_StringExact(Row, "fieldName", Descriptor.strFieldName) ||
				(nullptr == ApprovedValue ||
				 (!ApprovedValue->Is_String() && !ApprovedValue->Is_Boolean())) ||
				!Read_StringExact(Row, "descriptorKind", DescriptorKind) ||
				!Read_StringExact(Row, "standardMappingId", StandardMappingId) ||
				!Read_StringExact(Row, "implementationReferencePath",
					ReferencePath) ||
				!Read_ShaExact(Row, "implementationReferenceTrackedTextSha256",
					ReferenceHash) ||
				!Read_StringExact(Row, "implementationStateName",
					Descriptor.strImplementationStateName) ||
				nullptr == Expected ||
				!Read_StringExact(Row, "decisionBasis", DecisionBasis) ||
				!Validate_FailClosedAuthorityRow(Row, true) ||
				!Read_ShaExact(Row, "rowSha256", Descriptor.strRowSha256))
			{
				strOutError = "Render-resource render state row is invalid.";
				return false;
			}
			if (DescriptorKind == "D3D11_BLEND_DESC")
			{
				Descriptor.eKind = EFFECT_RECONSTRUCTED_RENDER_STATE_KIND::BLEND;
				if (!Parse_BlendDescriptor(*Expected, Descriptor.BlendDescriptor) ||
					(Descriptor.strImplementationStateName != "BS_EffectAlpha" &&
					 Descriptor.strImplementationStateName != "BS_EffectAdditive" &&
					 Descriptor.strImplementationStateName != "BS_EffectOpaque"))
				{
					strOutError = "Render-resource blend descriptor is invalid.";
					return false;
				}
			}
			else if (DescriptorKind == "D3D11_RASTERIZER_DESC")
			{
				Descriptor.eKind =
					EFFECT_RECONSTRUCTED_RENDER_STATE_KIND::RASTERIZER;
				if (!Parse_RasterizerDescriptor(
						*Expected, Descriptor.RasterizerDescriptor) ||
					Descriptor.strImplementationStateName != "RS_Cull_None")
				{
					strOutError = "Render-resource rasterizer descriptor is invalid.";
					return false;
				}
			}
			else if (DescriptorKind == "D3D11_DEPTH_STENCIL_DESC")
			{
				Descriptor.eKind =
					EFFECT_RECONSTRUCTED_RENDER_STATE_KIND::DEPTH_STENCIL;
				if (!Parse_DepthStencilDescriptor(
						*Expected, Descriptor.DepthStencilDescriptor) ||
					Descriptor.strImplementationStateName != "DSS_ZNone")
				{
					strOutError = "Render-resource depth descriptor is invalid.";
					return false;
				}
			}
			else
			{
				strOutError = "Render-resource descriptor kind is unsupported.";
				return false;
			}
			if (!Staged.emplace(Descriptor.strRenderStateDecisionId,
				std::move(Descriptor)).second)
			{
				strOutError = "Render-resource state descriptor ID is duplicate.";
				return false;
			}
		}
		OutDescriptors = std::move(Staged);
		return true;
	}

	bool Validate_RenderResourceSidecarAuthorityContract(
		const Client::DATA_JSON_VALUE& Value)
	{
		if (!Has_ExactOrderedKeys(Value, {
			"authorityKind", "resourceIdentityDomain", "materialDecisionDomain",
			"priorPolicySrvFixtureIsActualDdsDescriptor",
			"runtimeNameOrRoleHeuristicsAllowed",
			"absoluteResourcePathsAllowedInReceipt", "actionTimeIoAllowed",
			"transactionPolicy", "partialCommitAllowed", "sourceExact",
			"requiresAutomatedWARPProbe", "requiresManualEyeValidation",
			"runtimeExecutionAdmission", "product" }))
		{
			return false;
		}
		std::string AuthorityKind;
		std::string ResourceDomain;
		std::string MaterialDomain;
		std::string TransactionPolicy;
		return Read_StringExact(Value, "authorityKind", AuthorityKind) &&
			AuthorityKind == "IMMUTABLE_RECONSTRUCTED_RENDER_RESOURCE_SIDECAR" &&
			Read_StringExact(Value, "resourceIdentityDomain", ResourceDomain) &&
			ResourceDomain == "CANONICAL_MAIN_RESOURCES_RELATIVE_DDS_BYTES" &&
			Read_StringExact(Value, "materialDecisionDomain", MaterialDomain) &&
			MaterialDomain == "INDEPENDENT_RECONSTRUCTED_POLICY_APPROVAL" &&
			Read_BooleanExact(Value,
				"priorPolicySrvFixtureIsActualDdsDescriptor", false) &&
			Read_BooleanExact(Value, "runtimeNameOrRoleHeuristicsAllowed", false) &&
			Read_BooleanExact(Value, "absoluteResourcePathsAllowedInReceipt", false) &&
			Read_BooleanExact(Value, "actionTimeIoAllowed", false) &&
			Read_StringExact(Value, "transactionPolicy", TransactionPolicy) &&
			TransactionPolicy == "PARSE_VALIDATE_STAGE_COMMIT_OR_ROLLBACK" &&
			Read_BooleanExact(Value, "partialCommitAllowed", false) &&
			Validate_FailClosedAuthorityRow(Value, true);
	}

	bool Validate_RenderResourceSidecarSourceEvidence(
		const Client::DATA_JSON_VALUE& Value)
	{
		using namespace Client;
		if (!Has_ExactOrderedKeys(Value, {
			"programAndParserTuple", "materialTextureBindingAuthority",
			"materialRenderResourceApproval", "publisherRuntimeCatalogAuthority",
			"generatorAndValidator", "canonicalResourceRootContract",
			"sourceExact" }) ||
			!Read_BooleanExact(Value, "sourceExact", false))
		{
			return false;
		}
		const DATA_JSON_VALUE* Program = Required(
			Value, "programAndParserTuple", DATA_JSON_TYPE::OBJECT);
		const DATA_JSON_VALUE* Publisher = Required(
			Value, "publisherRuntimeCatalogAuthority", DATA_JSON_TYPE::OBJECT);
		if (nullptr == Program || !Has_ExactOrderedKeys(*Program, {
			"path", "candidateBuilderCommitId", "candidateBuilderTreeId",
			"candidateBlobIdAtIntegration", "rawByteCount", "rawSha256",
			"programId", "programVersion", "programSha256", "inputArtifactCount",
			"inputArtifactsOrderedSha256", "parserIntegrationCommitId",
			"parserIntegrationTreeId", "parserFiles", "sourceExact",
			"runtimeExecutionAdmission", "product" }) ||
			nullptr == Publisher || !Has_ExactOrderedKeys(*Publisher, {
			"authorityScope", "runtimeCatalogBytesRead",
			"completedRuntimeEntryRead", "renderResourceSidecarRead",
			"selfReferenceExcluded", "projectionKeyCount",
			"projectionKeyOrder", "projectionCanonicalSha256",
			"baseProjection", "publicValidator" }))
		{
			return false;
		}
		uint64_t iRawBytes = 0u;
		uint32_t iProgramVersion = 0u;
		uint32_t iInputArtifactCount = 0u;
		std::string RawSha;
		std::string ProgramId;
		std::string ProgramSha;
		std::string Text;
		if (!Read_U64Exact(*Program, "rawByteCount", iRawBytes) ||
			iRawBytes != 15'121'873u ||
			!Read_StringExact(*Program, "path", Text) || Text !=
				"Data/Effects/Imported/Artist/Candidates/skill.31470.reconstructed-runtime-program.candidate.json" ||
			!Read_StringExact(*Program, "candidateBuilderCommitId", Text) ||
			Text != "ddef21a5314eb8c3db891d36f702cfeda3149f20" ||
			!Read_StringExact(*Program, "candidateBuilderTreeId", Text) ||
			Text != "36a36b889dae7be092e0d2f6f3c3aee2c28bc462" ||
			!Read_StringExact(*Program, "candidateBlobIdAtIntegration", Text) ||
			Text != "4b090268b95ea590587276c8048c3b235ee33571" ||
			!Read_ShaExact(*Program, "rawSha256", RawSha) || RawSha !=
				"430ed1aa42a34e23d1f216a69c6f51e81a8cbcdbb03318930894e0dfe16cd6c6" ||
			!Read_StringExact(*Program, "programId", ProgramId) || ProgramId !=
				"effect.artist.skill.31470.reconstructed-approved-v1" ||
			!Read_U32(*Program, "programVersion", iProgramVersion) ||
			iProgramVersion != 1u ||
			!Read_ShaExact(*Program, "programSha256", ProgramSha) || ProgramSha !=
				"0666164bce946fd3b7e72dd92422b21a13e58d3388a3e3264ab30b8065e9c802" ||
			!Read_U32(*Program, "inputArtifactCount", iInputArtifactCount) ||
			iInputArtifactCount != 13u ||
			!Read_StringExact(*Program, "inputArtifactsOrderedSha256", Text) ||
			Text != "bcf87806b3635019442f6787c2ca6aed15d7012f2dd4c04d33b448f80814415f" ||
			!Read_StringExact(*Program, "parserIntegrationCommitId", Text) ||
			Text != "a57f5d27bb1ac29f890e6cb59121c886991f28d5" ||
			!Read_StringExact(*Program, "parserIntegrationTreeId", Text) ||
			Text != "bea6e94e0535038bdaabfb53f3f3442ef2fe296c" ||
			!Validate_FailClosedAuthorityRow(*Program))
		{
			return false;
		}
		const DATA_JSON_VALUE* ParserFiles = Required(
			*Program, "parserFiles", DATA_JSON_TYPE::ARRAY);
		constexpr std::array<std::string_view, 2u> PARSER_PATHS{
			"Client/Public/Effect_RuntimeAuthority.h",
			"Client/Private/Effect_RuntimeAuthority.cpp" };
		constexpr std::array<std::string_view, 2u> PARSER_BLOBS{
			"f86fcaa1f7554e5b1c274f3eaec3a6a0f8f22d9a",
			"cf33b11374379e52c0eab9c57d34063b4fe78b93" };
		constexpr std::array<std::string_view, 2u> PARSER_TEXT_SHA256{
			"691ca79cd71cafe4067d36441950c0efebcbf5756ce283fcb9151926d5bed999",
			"aa0740303e1ed9ed97bba6ebdc083c26e4ff028c72f11b1a1537f9be4ab01369" };
		if (nullptr == ParserFiles ||
			ParserFiles->Get_Array().size() != PARSER_PATHS.size())
		{
			return false;
		}
		for (size_t Index = 0u; Index < PARSER_PATHS.size(); ++Index)
		{
			const DATA_JSON_VALUE& File = ParserFiles->Get_Array()[Index];
			std::string Path;
			std::string Blob;
			std::string TextSha;
			if (!Has_ExactOrderedKeys(File, {
					"path", "parserIntegrationBlobId", "currentTrackedTextSha256" }) ||
				!Read_StringExact(File, "path", Path) ||
				Path != PARSER_PATHS[Index] ||
				!Read_StringExact(File, "parserIntegrationBlobId", Blob) ||
				Blob != PARSER_BLOBS[Index] ||
				!Read_ShaExact(File, "currentTrackedTextSha256", TextSha) ||
				TextSha != PARSER_TEXT_SHA256[Index])
			{
				return false;
			}
		}

		const DATA_JSON_VALUE* ProjectionOrder = Required(
			*Publisher, "projectionKeyOrder", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* BaseProjection = Required(
			*Publisher, "baseProjection", DATA_JSON_TYPE::OBJECT);
		const DATA_JSON_VALUE* PublicValidator = Required(
			*Publisher, "publicValidator", DATA_JSON_TYPE::OBJECT);
		constexpr std::array<std::string_view, 17u> PROJECTION_KEYS{
			"schema", "formatVersion", "projectionRole", "payloadKind",
			"effectAssetId", "artifactRevision", "compilerRevision", "programId",
			"programVersion", "programSha256", "candidateRawSha256",
			"candidateByteCount", "inputArtifactCount",
			"inputArtifactsOrderedSha256", "sourceExact",
			"runtimeExecutionAdmission", "productAdmission" };
		if (nullptr == ProjectionOrder || nullptr == BaseProjection ||
			nullptr == PublicValidator ||
			ProjectionOrder->Get_Array().size() != PROJECTION_KEYS.size() ||
			!Has_ExactOrderedKeys(*BaseProjection, {
				"schema", "formatVersion", "projectionRole", "payloadKind",
				"effectAssetId", "artifactRevision", "compilerRevision",
				"programId", "programVersion", "programSha256",
				"candidateRawSha256", "candidateByteCount", "inputArtifactCount",
				"inputArtifactsOrderedSha256", "sourceExact",
				"runtimeExecutionAdmission", "productAdmission" }) ||
			!Has_ExactOrderedKeys(*PublicValidator,
				{ "path", "builderFunction", "validatorFunction" }))
		{
			return false;
		}
		for (size_t Index = 0u; Index < PROJECTION_KEYS.size(); ++Index)
		{
			const DATA_JSON_VALUE& Key = ProjectionOrder->Get_Array()[Index];
			if (!Key.Is_String() || Key.Get_String() != PROJECTION_KEYS[Index])
				return false;
		}

		uint32_t iProjectionKeyCount = 0u;
		uint32_t iBaseFormatVersion = 0u;
		uint32_t iBaseArtifactRevision = 0u;
		uint32_t iBaseProgramVersion = 0u;
		uint32_t iBaseInputCount = 0u;
		uint64_t iBaseCandidateBytes = 0u;
		std::string Sha;
		return Read_StringExact(*Publisher, "authorityScope", Text) &&
			Text == "BASE_RUNTIME_ENTRY_PROJECTION_BEFORE_RENDER_RESOURCE_SIDECAR" &&
			Read_BooleanExact(*Publisher, "runtimeCatalogBytesRead", false) &&
			Read_BooleanExact(*Publisher, "completedRuntimeEntryRead", false) &&
			Read_BooleanExact(*Publisher, "renderResourceSidecarRead", false) &&
			Read_BooleanExact(*Publisher, "selfReferenceExcluded", true) &&
			Read_U32(*Publisher, "projectionKeyCount", iProjectionKeyCount) &&
			iProjectionKeyCount == PROJECTION_KEYS.size() &&
			Read_ShaExact(*Publisher, "projectionCanonicalSha256", Sha) &&
			Sha == "86ce2f989ab41dab54cc2147b1fde170aa8c1fae01fa74ed043d177d6d23d453" &&
			Read_StringExact(*BaseProjection, "schema", Text) &&
			Text == "lostark.effect-reconstructed-base-authority-projection" &&
			Read_U32(*BaseProjection, "formatVersion", iBaseFormatVersion) &&
			iBaseFormatVersion == 1u &&
			Read_StringExact(*BaseProjection, "projectionRole", Text) &&
			Text == "PROGRAM_TO_BASE_ENTRY_BEFORE_RENDER_RESOURCE_SIDECAR" &&
			Read_StringExact(*BaseProjection, "payloadKind", Text) &&
			Text == "IMMUTABLE_RECONSTRUCTED_RUNTIME_PROGRAM" &&
			Read_StringExact(*BaseProjection, "effectAssetId", Text) &&
			Text == "effect.artist.skill.31470" &&
			Read_U32(*BaseProjection, "artifactRevision", iBaseArtifactRevision) &&
			iBaseArtifactRevision == 1u &&
			Read_StringExact(*BaseProjection, "compilerRevision", Text) &&
			Text == "artist31470.reconstructed-runtime-program-link-v1" &&
			Read_StringExact(*BaseProjection, "programId", Text) &&
			Text == "effect.artist.skill.31470.reconstructed-approved-v1" &&
			Read_U32(*BaseProjection, "programVersion", iBaseProgramVersion) &&
			iBaseProgramVersion == 1u &&
			Read_ShaExact(*BaseProjection, "programSha256", Sha) &&
			Sha == "0666164bce946fd3b7e72dd92422b21a13e58d3388a3e3264ab30b8065e9c802" &&
			Read_ShaExact(*BaseProjection, "candidateRawSha256", Sha) &&
			Sha == "430ed1aa42a34e23d1f216a69c6f51e81a8cbcdbb03318930894e0dfe16cd6c6" &&
			Read_U64Exact(*BaseProjection, "candidateByteCount",
				iBaseCandidateBytes) && iBaseCandidateBytes == 15'121'873u &&
			Read_U32(*BaseProjection, "inputArtifactCount", iBaseInputCount) &&
			iBaseInputCount == 13u &&
			Read_ShaExact(*BaseProjection, "inputArtifactsOrderedSha256", Sha) &&
			Sha == "bcf87806b3635019442f6787c2ca6aed15d7012f2dd4c04d33b448f80814415f" &&
			Read_BooleanExact(*BaseProjection, "sourceExact", false) &&
			Read_BooleanExact(*BaseProjection, "runtimeExecutionAdmission", false) &&
			Read_BooleanExact(*BaseProjection, "productAdmission", false) &&
			Read_StringExact(*PublicValidator, "path", Text) &&
			Text == "Tools/EffectPipeline/build_effect_derived_artifact.py" &&
			Read_StringExact(*PublicValidator, "builderFunction", Text) &&
			Text == "make_reconstructed_base_authority_projection" &&
			Read_StringExact(*PublicValidator, "validatorFunction", Text) &&
			Text == "validate_reconstructed_base_authority_projection";
	}

	bool Parse_ReconstructedRenderResourceSidecar(
		const std::string& Text,
		Client::EFFECT_RECONSTRUCTED_RENDER_RESOURCE_AUTHORITY& OutAuthority,
		std::string& strOutError)
	{
		using namespace Client;
		constexpr std::string_view SIDECAR_SCHEMA =
			"lostark.artist-31470-reconstructed-render-resource-authority-receipt";
		constexpr std::string_view AUTHORITY_ID =
			"ARTIST_31470_RECONSTRUCTED_RENDER_RESOURCE_AUTHORITY_V1";
		constexpr std::string_view SIDECAR_RAW_SHA256 =
			"d69e262b7841f831f6e5d479fd588ff9fc52e4b72c896544d3c3ec178516139e";
		constexpr std::string_view SIDECAR_RECEIPT_SHA256 =
			"5af0e5e7b7882644ad6c3371bf888b191a6591684c4ac7a42f29b8c1e2a5a98e";
		constexpr std::string_view SIDECAR_DECISION_SHA256 =
			"e3b19c2c8102746d8e9ba5b5494ec0b194baeb7bd9df61979c6fade2e5dc70eb";
		if (Text.size() != 774'127u ||
			CEffectRuntimeAuthorityCodec::Compute_Sha256Hex(Text) !=
				SIDECAR_RAW_SHA256)
		{
			strOutError = "Render-resource sidecar raw identity is invalid.";
			return false;
		}
		DATA_JSON_VALUE Root;
		DATA_JSON_PARSE_LIMITS Limits;
		Limits.iMaximumBytes = 1024u * 1024u;
		Limits.iMaximumDepth = 64u;
		Limits.iMaximumValues = 250'000u;
		std::string ParseError;
		if (!CDataJson::Parse(Text, Root, ParseError, Limits) ||
			!Has_ExactOrderedKeys(Root, {
				"schema", "formatVersion", "authorityId", "characterClass",
				"skillId", "inputSlot", "authorityContract", "sourceEvidence",
				"textureResources", "textureBindings", "neutralProviders",
				"recipeTextureBindings", "rendererSlotBindings",
				"renderStateDescriptors", "blockerProjection", "admission",
				"summary", "decisionProjectionSha256", "receiptSha256" }))
		{
			strOutError = "Render-resource sidecar JSON/root schema is invalid: " +
				ParseError;
			return false;
		}
		std::string Schema;
		std::string AuthorityId;
		std::string CharacterClass;
		std::string InputSlot;
		std::string DecisionSha;
		std::string ReceiptSha;
		uint32_t iFormatVersion = 0u;
		uint32_t iSkillId = 0u;
		const DATA_JSON_VALUE* AuthorityContract = Required(
			Root, "authorityContract", DATA_JSON_TYPE::OBJECT);
		const DATA_JSON_VALUE* SourceEvidence = Required(
			Root, "sourceEvidence", DATA_JSON_TYPE::OBJECT);
		const DATA_JSON_VALUE* Blocker = Required(
			Root, "blockerProjection", DATA_JSON_TYPE::OBJECT);
		const DATA_JSON_VALUE* Admission = Required(
			Root, "admission", DATA_JSON_TYPE::OBJECT);
		const DATA_JSON_VALUE* Summary = Required(
			Root, "summary", DATA_JSON_TYPE::OBJECT);
		if (!Read_StringExact(Root, "schema", Schema) || Schema != SIDECAR_SCHEMA ||
			!Read_U32(Root, "formatVersion", iFormatVersion) ||
			iFormatVersion != 1u ||
			!Read_StringExact(Root, "authorityId", AuthorityId) ||
			AuthorityId != AUTHORITY_ID ||
			!Read_StringExact(Root, "characterClass", CharacterClass) ||
			CharacterClass != "ARTIST" ||
			!Read_U32(Root, "skillId", iSkillId) || iSkillId != 31470u ||
			!Read_StringExact(Root, "inputSlot", InputSlot) || InputSlot != "F" ||
			nullptr == AuthorityContract ||
			!Validate_RenderResourceSidecarAuthorityContract(*AuthorityContract) ||
			nullptr == SourceEvidence ||
			!Validate_RenderResourceSidecarSourceEvidence(*SourceEvidence) ||
			nullptr == Blocker || !Has_ExactOrderedKeys(*Blocker, {
				"blockers", "canonicalResourceBytesVerifiedAtOfflineBuildAndValidation",
				"actionTimeIoAllowed", "bindingFailureBehavior",
				"partialCommitAllowed", "sourceExact",
				"requiresAutomatedWARPProbe", "requiresManualEyeValidation",
				"runtimeExecutionAdmission", "product" }) ||
			!Read_BooleanExact(*Blocker,
				"canonicalResourceBytesVerifiedAtOfflineBuildAndValidation", true) ||
			!Read_BooleanExact(*Blocker, "actionTimeIoAllowed", false) ||
			!Read_BooleanExact(*Blocker, "partialCommitAllowed", false) ||
			!Validate_FailClosedAuthorityRow(*Blocker, true) ||
			nullptr == Admission || !Has_ExactOrderedKeys(*Admission, {
				"sourceExact", "requiresAutomatedWARPProbe",
				"requiresManualEyeValidation", "runtimeExecutionAdmission",
				"product" }) ||
			!Validate_FailClosedAuthorityRow(*Admission, true) ||
			nullptr == Summary || !Has_ExactOrderedKeys(*Summary, {
				"textureResourceCount", "textureBindingCount", "resourceFormatCounts",
				"bindingSrvDxgiFormatCounts", "bindingColorSpaceCounts",
				"neutralProviderCount", "recipeTextureBindingCount",
				"rendererSlotBindingCount", "ambiguousRendererDecisionCount",
				"renderStateDescriptorCount", "blendDescriptorCount",
				"twoSidedRasterDescriptorCount", "disableDepthDescriptorCount",
				"actionTimeIoAllowed", "sourceExact", "runtimeExecutionAdmission",
				"product" }) ||
			!Read_BooleanExact(*Summary, "actionTimeIoAllowed", false) ||
			!Validate_FailClosedAuthorityRow(*Summary) ||
			!Read_ShaExact(Root, "decisionProjectionSha256", DecisionSha) ||
			DecisionSha != SIDECAR_DECISION_SHA256 ||
			!Read_ShaExact(Root, "receiptSha256", ReceiptSha) ||
			ReceiptSha != SIDECAR_RECEIPT_SHA256)
		{
			strOutError = "Render-resource sidecar identity/admission is invalid.";
			return false;
		}

		DATA_JSON_VALUE UnsignedReceipt;
		if (!Build_ObjectProjection(Root, {
			"schema", "formatVersion", "authorityId", "characterClass",
			"skillId", "inputSlot", "authorityContract", "sourceEvidence",
			"textureResources", "textureBindings", "neutralProviders",
			"recipeTextureBindings", "rendererSlotBindings",
			"renderStateDescriptors", "blockerProjection", "admission",
			"summary", "decisionProjectionSha256" }, UnsignedReceipt) ||
			CEffectRuntimeAuthorityCodec::Compute_Sha256Hex(
				CEffectRuntimeAuthorityCodec::Serialize_CanonicalJson(UnsignedReceipt)) !=
				SIDECAR_RECEIPT_SHA256)
		{
			strOutError = "Render-resource sidecar self digest is invalid.";
			return false;
		}
		const DATA_JSON_VALUE* Publisher = SourceEvidence->Find(
			"publisherRuntimeCatalogAuthority");
		if (nullptr == Publisher)
		{
			strOutError = "Render-resource sidecar publisher projection is absent.";
			return false;
		}
		DATA_JSON_VALUE::OBJECT DecisionFields;
		DecisionFields.emplace("authorityId", *Root.Find("authorityId"));
		DecisionFields.emplace("authorityContract", *AuthorityContract);
		DecisionFields.emplace("publisherRuntimeCatalogAuthority", *Publisher);
		DecisionFields.emplace("textureResources", *Root.Find("textureResources"));
		DecisionFields.emplace("textureBindings", *Root.Find("textureBindings"));
		DecisionFields.emplace("neutralProviders", *Root.Find("neutralProviders"));
		DecisionFields.emplace("recipeTextureBindings",
			*Root.Find("recipeTextureBindings"));
		DecisionFields.emplace("rendererSlotBindings",
			*Root.Find("rendererSlotBindings"));
		DecisionFields.emplace("renderStateDescriptors",
			*Root.Find("renderStateDescriptors"));
		DecisionFields.emplace("blockerProjection", *Blocker);
		DecisionFields.emplace("admission", *Admission);
		const DATA_JSON_VALUE DecisionProjection = DATA_JSON_VALUE::Object(
			std::move(DecisionFields));
		if (CEffectRuntimeAuthorityCodec::Compute_Sha256Hex(
				CEffectRuntimeAuthorityCodec::Serialize_CanonicalJson(
					DecisionProjection)) != SIDECAR_DECISION_SHA256)
		{
			strOutError = "Render-resource sidecar decision digest is invalid.";
			return false;
		}

		uint32_t iTextureResourceCount = 0u;
		uint32_t iTextureBindingCount = 0u;
		uint32_t iNeutralProviderCount = 0u;
		uint32_t iRecipeCount = 0u;
		uint32_t iRendererCount = 0u;
		uint32_t iAmbiguousCount = 0u;
		uint32_t iStateCount = 0u;
		uint32_t iBlendCount = 0u;
		uint32_t iRasterCount = 0u;
		uint32_t iDepthCount = 0u;
		if (!Read_U32(*Summary, "textureResourceCount", iTextureResourceCount) ||
			iTextureResourceCount != 52u ||
			!Read_U32(*Summary, "textureBindingCount", iTextureBindingCount) ||
			iTextureBindingCount != 77u ||
			!Read_U32(*Summary, "neutralProviderCount", iNeutralProviderCount) ||
			iNeutralProviderCount != 4u ||
			!Read_U32(*Summary, "recipeTextureBindingCount", iRecipeCount) ||
			iRecipeCount != 27u ||
			!Read_U32(*Summary, "rendererSlotBindingCount", iRendererCount) ||
			iRendererCount != 57u ||
			!Read_U32(*Summary, "ambiguousRendererDecisionCount",
				iAmbiguousCount) || iAmbiguousCount != 3u ||
			!Read_U32(*Summary, "renderStateDescriptorCount", iStateCount) ||
			iStateCount != 46u ||
			!Read_U32(*Summary, "blendDescriptorCount", iBlendCount) ||
			iBlendCount != 27u ||
			!Read_U32(*Summary, "twoSidedRasterDescriptorCount", iRasterCount) ||
			iRasterCount != 18u ||
			!Read_U32(*Summary, "disableDepthDescriptorCount", iDepthCount) ||
			iDepthCount != 1u)
		{
			strOutError = "Render-resource sidecar summary is invalid.";
			return false;
		}

		EFFECT_RECONSTRUCTED_RENDER_RESOURCE_AUTHORITY Staged;
		Staged.Identity.iFormatVersion = iFormatVersion;
		Staged.Identity.iProgramVersion = 1u;
		Staged.Identity.iSidecarByteCount = Text.size();
		Staged.Identity.strSchema = Schema;
		Staged.Identity.strAuthorityId = AuthorityId;
		Staged.Identity.strProgramId =
			"effect.artist.skill.31470.reconstructed-approved-v1";
		Staged.Identity.strProgramSha256 =
			"0666164bce946fd3b7e72dd92422b21a13e58d3388a3e3264ab30b8065e9c802";
		Staged.Identity.strSidecarDecisionProjectionSha256 = DecisionSha;
		Staged.Identity.strSidecarReceiptSha256 = ReceiptSha;
		Staged.Identity.strSidecarRawSha256 = std::string(SIDECAR_RAW_SHA256);
		const DATA_JSON_VALUE* TextureResources = Root.Find("textureResources");
		const DATA_JSON_VALUE* TextureBindings = Root.Find("textureBindings");
		const DATA_JSON_VALUE* NeutralProviders = Root.Find("neutralProviders");
		const DATA_JSON_VALUE* RecipeBindings = Root.Find("recipeTextureBindings");
		const DATA_JSON_VALUE* RendererBindings = Root.Find("rendererSlotBindings");
		const DATA_JSON_VALUE* StateDescriptors = Root.Find("renderStateDescriptors");
		if (nullptr == TextureResources || nullptr == TextureBindings ||
			nullptr == NeutralProviders || nullptr == RecipeBindings ||
			nullptr == RendererBindings || nullptr == StateDescriptors ||
			!Parse_RenderTextureResources(*TextureResources,
				Staged.TextureResourcesById, strOutError) ||
			!Parse_RenderTextureBindings(*TextureBindings,
				Staged.TextureResourcesById, Staged.TextureBindingsById,
				strOutError) ||
			!Parse_RenderNeutralProviders(*NeutralProviders,
				Staged.NeutralProvidersById, strOutError) ||
			!Parse_RenderRecipeTextureBindings(*RecipeBindings,
				Staged.TextureBindingsById, Staged.NeutralProvidersById,
				Staged.RecipeTextureBindingsById, strOutError) ||
			!Parse_RendererSlotBindings(*RendererBindings,
				Staged.TextureBindingsById, Staged.RendererSlotBindingsById,
				strOutError) ||
			!Parse_RenderStateDescriptors(*StateDescriptors,
				Staged.RenderStateDescriptorsById, strOutError))
		{
			return false;
		}
		OutAuthority = std::move(Staged);
		return true;
	}

	bool Parse_ReconstructedRenderResourceExtension(
		const Client::DATA_JSON_VALUE& Value,
		Client::EFFECT_RUNTIME_PROGRAM_CATALOG_IDENTITY& InOutIdentity,
		std::shared_ptr<const
			Client::EFFECT_RECONSTRUCTED_RENDER_RESOURCE_AUTHORITY>&
			OutAuthority,
		std::string& strOutError)
	{
		using namespace Client;
		constexpr std::string_view EFFECT_ASSET_ID =
			"effect.artist.skill.31470";
		constexpr std::string_view PAYLOAD_KIND =
			"IMMUTABLE_RECONSTRUCTED_RUNTIME_PROGRAM";
		constexpr std::string_view COMPILER_REVISION =
			"artist31470.reconstructed-runtime-program-link-v1";
		constexpr std::string_view PROGRAM_ID =
			"effect.artist.skill.31470.reconstructed-approved-v1";
		constexpr std::string_view PROGRAM_SHA256 =
			"0666164bce946fd3b7e72dd92422b21a13e58d3388a3e3264ab30b8065e9c802";
		constexpr std::string_view BASE_ENTRY_SHA256 =
			"6cd44dbd323d50e46e854b865d70ebf922355ed3db4ff5b430ae241c83aa01b3";
		constexpr std::string_view BASE_LINK_SHA256 =
			"282f450d95ae283acf91047fe6b293eb93fddaea9f4bde4cff9671e7aa27c523";
		constexpr std::string_view BASE_RECEIPT_SHA256 =
			"413c34440ee82b9511d1de4bbd31af282dce2e65204fd5cda0d75d8e4e58650b";
		constexpr std::string_view SIDECAR_SCHEMA =
			"lostark.artist-31470-reconstructed-render-resource-authority-receipt";
		constexpr std::string_view SIDECAR_AUTHORITY_ID =
			"ARTIST_31470_RECONSTRUCTED_RENDER_RESOURCE_AUTHORITY_V1";
		constexpr std::string_view SIDECAR_DECISION_SHA256 =
			"e3b19c2c8102746d8e9ba5b5494ec0b194baeb7bd9df61979c6fade2e5dc70eb";
		constexpr std::string_view SIDECAR_RECEIPT_SHA256 =
			"5af0e5e7b7882644ad6c3371bf888b191a6591684c4ac7a42f29b8c1e2a5a98e";
		constexpr std::string_view SIDECAR_RAW_SHA256 =
			"d69e262b7841f831f6e5d479fd588ff9fc52e4b72c896544d3c3ec178516139e";
		constexpr std::string_view AUTHORITY_LINK_SHA256 =
			"2eb98fb864d5ba3c3e13c6eb2cbbec903e26341c7345e0e24a1eccd06878b56b";
		constexpr std::string_view RECEIPT_SELF_SHA256 =
			"0681a54f2ca708d2bfdbc1391d5756262cefe04bf9586ef14205ae72bd8ebf05";
		constexpr std::string_view PUBLISH_RECEIPT_SHA256 =
			"e0691cd5bd857d78c1a87007c1799b91817cf5f8ced41dcdcbfbf9b55ac3db91";

		const DATA_JSON_VALUE* Link = Required(
			Value, "reconstructedRenderResourceAuthority", DATA_JSON_TYPE::OBJECT);
		if (nullptr == Link || !Has_ExactOrderedKeys(*Link, {
			"schema", "formatVersion", "encoding", "effectAssetId", "programId",
			"programVersion", "programSha256", "sidecarSchema",
			"sidecarFormatVersion", "sidecarAuthorityId",
			"sidecarDecisionProjectionSha256", "sidecarReceiptSha256",
			"sidecarRawSha256", "sidecarByteCount", "sourceExact",
			"runtimeExecutionAdmission", "executeAdmission", "submitAdmission",
			"renderAdmission", "productAdmission", "sidecarUtf8Json" }))
		{
			strOutError =
				"Reconstructed render-resource link fields or order are invalid.";
			return false;
		}
		const auto LinkStringEquals = [Link](
			const char* pName, const std::string_view Expected)
		{
			const DATA_JSON_VALUE* Field = Required(
				*Link, pName, DATA_JSON_TYPE::STRING);
			return nullptr != Field && Field->Get_String() == Expected;
		};
		uint32_t iLinkFormatVersion = 0u;
		uint32_t iLinkProgramVersion = 0u;
		uint32_t iSidecarFormatVersion = 0u;
		uint64_t iSidecarByteCount = 0u;
		const DATA_JSON_VALUE* SidecarText = Required(
			*Link, "sidecarUtf8Json", DATA_JSON_TYPE::STRING);
		if (!LinkStringEquals("schema",
				"lostark.effect-reconstructed-render-resource-authority-link") ||
			!Read_U32(*Link, "formatVersion", iLinkFormatVersion) ||
			Link->Find("formatVersion")->Was_FloatingPointToken() ||
			iLinkFormatVersion != 1u ||
			!LinkStringEquals("encoding", "UTF8_JSON_EXACT") ||
			!LinkStringEquals("effectAssetId", EFFECT_ASSET_ID) ||
			!LinkStringEquals("programId", PROGRAM_ID) ||
			!Read_U32(*Link, "programVersion", iLinkProgramVersion) ||
			Link->Find("programVersion")->Was_FloatingPointToken() ||
			iLinkProgramVersion != 1u ||
			!LinkStringEquals("programSha256", PROGRAM_SHA256) ||
			!LinkStringEquals("sidecarSchema", SIDECAR_SCHEMA) ||
			!Read_U32(*Link, "sidecarFormatVersion", iSidecarFormatVersion) ||
			Link->Find("sidecarFormatVersion")->Was_FloatingPointToken() ||
			iSidecarFormatVersion != 1u ||
			!LinkStringEquals("sidecarAuthorityId", SIDECAR_AUTHORITY_ID) ||
			!LinkStringEquals("sidecarDecisionProjectionSha256",
				SIDECAR_DECISION_SHA256) ||
			!LinkStringEquals("sidecarReceiptSha256", SIDECAR_RECEIPT_SHA256) ||
			!LinkStringEquals("sidecarRawSha256", SIDECAR_RAW_SHA256) ||
			!Read_U64Exact(*Link, "sidecarByteCount", iSidecarByteCount) ||
			iSidecarByteCount != 774'127u ||
			!Read_BooleanExact(*Link, "sourceExact", false) ||
			!Read_BooleanExact(*Link, "runtimeExecutionAdmission", false) ||
			!Read_BooleanExact(*Link, "executeAdmission", false) ||
			!Read_BooleanExact(*Link, "submitAdmission", false) ||
			!Read_BooleanExact(*Link, "renderAdmission", false) ||
			!Read_BooleanExact(*Link, "productAdmission", false) ||
			nullptr == SidecarText ||
			SidecarText->Get_String().size() != iSidecarByteCount ||
			CEffectRuntimeAuthorityCodec::Compute_Sha256Hex(
				SidecarText->Get_String()) != SIDECAR_RAW_SHA256)
		{
			strOutError =
				"Reconstructed render-resource link identity/admission is invalid.";
			return false;
		}
		const std::string ComputedLinkSha =
			CEffectRuntimeAuthorityCodec::Compute_Sha256Hex(
				CEffectRuntimeAuthorityCodec::Serialize_CanonicalJson(*Link));
		if (ComputedLinkSha != AUTHORITY_LINK_SHA256)
		{
			strOutError =
				"Reconstructed render-resource link canonical digest is invalid.";
			return false;
		}

		const DATA_JSON_VALUE* Receipt = Required(
			Value, "renderResourcePublishReceipt", DATA_JSON_TYPE::OBJECT);
		const DATA_JSON_VALUE* OuterReceiptSha = Required(
			Value, "renderResourcePublishReceiptSha256", DATA_JSON_TYPE::STRING);
		if (nullptr == Receipt || !Has_ExactOrderedKeys(*Receipt, {
			"schema", "formatVersion", "receiptRole", "payloadKind",
			"effectAssetId", "artifactRevision", "compilerRevision",
			"sourceExact", "runtimeExecutionAdmission", "executeAdmission",
			"submitAdmission", "renderAdmission", "productAdmission", "programId",
			"programVersion", "programSha256", "baseRuntimeEntryProjectionSha256",
			"reconstructedRuntimeProgramSha256", "basePublishReceiptSha256",
			"renderResourceAuthorityLinkSha256", "sidecarRawSha256",
			"sidecarReceiptSha256", "sidecarDecisionProjectionSha256",
			"toolDependencies", "receiptSha256Domain", "receiptSha256" }) ||
			nullptr == OuterReceiptSha ||
			OuterReceiptSha->Get_String() != PUBLISH_RECEIPT_SHA256)
		{
			strOutError =
				"Reconstructed render-resource receipt fields or order are invalid.";
			return false;
		}
		const auto ReceiptStringEquals = [Receipt](
			const char* pName, const std::string_view Expected)
		{
			const DATA_JSON_VALUE* Field = Required(
				*Receipt, pName, DATA_JSON_TYPE::STRING);
			return nullptr != Field && Field->Get_String() == Expected;
		};
		uint32_t iReceiptFormatVersion = 0u;
		uint32_t iReceiptArtifactRevision = 0u;
		uint32_t iReceiptProgramVersion = 0u;
		if (!ReceiptStringEquals("schema",
				"lostark.effect-reconstructed-render-resource-publication-receipt") ||
			!Read_U32(*Receipt, "formatVersion", iReceiptFormatVersion) ||
			Receipt->Find("formatVersion")->Was_FloatingPointToken() ||
			iReceiptFormatVersion != 1u ||
			!ReceiptStringEquals("receiptRole",
				"PUBLICATION_PROVENANCE_ONLY_NOT_EXECUTION_SUBMIT_RENDER_AUTHORITY") ||
			!ReceiptStringEquals("payloadKind", PAYLOAD_KIND) ||
			!ReceiptStringEquals("effectAssetId", EFFECT_ASSET_ID) ||
			!Read_U32(*Receipt, "artifactRevision", iReceiptArtifactRevision) ||
			Receipt->Find("artifactRevision")->Was_FloatingPointToken() ||
			iReceiptArtifactRevision != 1u ||
			!ReceiptStringEquals("compilerRevision", COMPILER_REVISION) ||
			!Read_BooleanExact(*Receipt, "sourceExact", false) ||
			!Read_BooleanExact(*Receipt, "runtimeExecutionAdmission", false) ||
			!Read_BooleanExact(*Receipt, "executeAdmission", false) ||
			!Read_BooleanExact(*Receipt, "submitAdmission", false) ||
			!Read_BooleanExact(*Receipt, "renderAdmission", false) ||
			!Read_BooleanExact(*Receipt, "productAdmission", false) ||
			!ReceiptStringEquals("programId", PROGRAM_ID) ||
			!Read_U32(*Receipt, "programVersion", iReceiptProgramVersion) ||
			Receipt->Find("programVersion")->Was_FloatingPointToken() ||
			iReceiptProgramVersion != 1u ||
			!ReceiptStringEquals("programSha256", PROGRAM_SHA256) ||
			!ReceiptStringEquals("baseRuntimeEntryProjectionSha256",
				BASE_ENTRY_SHA256) ||
			!ReceiptStringEquals("reconstructedRuntimeProgramSha256",
				BASE_LINK_SHA256) ||
			!ReceiptStringEquals("basePublishReceiptSha256", BASE_RECEIPT_SHA256) ||
			!ReceiptStringEquals("renderResourceAuthorityLinkSha256",
				AUTHORITY_LINK_SHA256) ||
			!ReceiptStringEquals("sidecarRawSha256", SIDECAR_RAW_SHA256) ||
			!ReceiptStringEquals("sidecarReceiptSha256", SIDECAR_RECEIPT_SHA256) ||
			!ReceiptStringEquals("sidecarDecisionProjectionSha256",
				SIDECAR_DECISION_SHA256) ||
			!ReceiptStringEquals("receiptSha256Domain",
				"CANONICAL_JSON_EXCLUDING_RECEIPT_SHA256") ||
			!ReceiptStringEquals("receiptSha256", RECEIPT_SELF_SHA256))
		{
			strOutError =
				"Reconstructed render-resource receipt identity/admission is invalid.";
			return false;
		}

		const DATA_JSON_VALUE* Tools = Required(
			*Receipt, "toolDependencies", DATA_JSON_TYPE::ARRAY);
		constexpr std::array<std::string_view, 3u> TOOL_ROLES{
			"RECONSTRUCTED_RENDER_RESOURCE_INDEPENDENT_PINS",
			"RECONSTRUCTED_RENDER_RESOURCE_CATALOG_VALIDATOR",
			"EFFECT_PUBLISHER" };
		constexpr std::array<std::string_view, 3u> TOOL_PATHS{
			"Tools/LevelPlacementExtractor/artist_31470_reconstructed_render_resource_authority.py",
			"Tools/EffectPipeline/build_effect_derived_artifact.py",
			"Tools/EffectPipeline/Publish-Effects.ps1" };
		constexpr std::array<std::string_view, 3u> TOOL_SHA256{
			"be8262731b3d69120107acd6e347279a06f741af2b0eea7edb7fd40db45390c8",
			"a67cfcc02c00a83d4f18aa49635b6aedb7982d7803cb0feb6c9437b1b244232f",
			"5390ee17b06b5d718dd48848e33bc9a69e6df5a58c3250726169159ce2eb56e2" };
		if (nullptr == Tools || Tools->Get_Array().size() != TOOL_ROLES.size())
		{
			strOutError =
				"Reconstructed render-resource receipt tool set is invalid.";
			return false;
		}
		for (size_t Index = 0u; Index < TOOL_ROLES.size(); ++Index)
		{
			const DATA_JSON_VALUE& Tool = Tools->Get_Array()[Index];
			if (!Has_ExactOrderedKeys(Tool,
					{ "role", "path", "hashDomain", "sha256" }) ||
				nullptr == Required(Tool, "role", DATA_JSON_TYPE::STRING) ||
				Tool.Find("role")->Get_String() != TOOL_ROLES[Index] ||
				nullptr == Required(Tool, "path", DATA_JSON_TYPE::STRING) ||
				Tool.Find("path")->Get_String() != TOOL_PATHS[Index] ||
				nullptr == Required(Tool, "hashDomain", DATA_JSON_TYPE::STRING) ||
				Tool.Find("hashDomain")->Get_String() !=
					"TRACKED_SOURCE_EOL_CANONICAL_TEXT" ||
				nullptr == Required(Tool, "sha256", DATA_JSON_TYPE::STRING) ||
				Tool.Find("sha256")->Get_String() != TOOL_SHA256[Index])
			{
				strOutError =
					"Reconstructed render-resource receipt tool identity is invalid.";
				return false;
			}
		}

		DATA_JSON_VALUE BaseProjection;
		DATA_JSON_VALUE UnsignedReceipt;
		if (!Build_ObjectProjection(Value, {
			"payloadKind", "effectAssetId", "artifactRevision",
			"compilerRevision", "sourceExact", "runtimeExecutionAdmission",
			"productAdmission", "publishReceiptSha256", "publishReceipt",
			"reconstructedRuntimeProgram" }, BaseProjection) ||
			CEffectRuntimeAuthorityCodec::Compute_Sha256Hex(
				CEffectRuntimeAuthorityCodec::Serialize_CanonicalJson(BaseProjection)) !=
				BASE_ENTRY_SHA256 ||
			!Build_ObjectProjection(*Receipt, {
				"schema", "formatVersion", "receiptRole", "payloadKind",
				"effectAssetId", "artifactRevision", "compilerRevision",
				"sourceExact", "runtimeExecutionAdmission", "executeAdmission",
				"submitAdmission", "renderAdmission", "productAdmission", "programId",
				"programVersion", "programSha256",
				"baseRuntimeEntryProjectionSha256",
				"reconstructedRuntimeProgramSha256", "basePublishReceiptSha256",
				"renderResourceAuthorityLinkSha256", "sidecarRawSha256",
				"sidecarReceiptSha256", "sidecarDecisionProjectionSha256",
				"toolDependencies", "receiptSha256Domain" }, UnsignedReceipt) ||
			CEffectRuntimeAuthorityCodec::Compute_Sha256Hex(
				CEffectRuntimeAuthorityCodec::Serialize_CanonicalJson(UnsignedReceipt)) !=
				RECEIPT_SELF_SHA256 ||
			CEffectRuntimeAuthorityCodec::Compute_Sha256Hex(
				CEffectRuntimeAuthorityCodec::Serialize_CanonicalJson(*Receipt)) !=
				PUBLISH_RECEIPT_SHA256)
		{
			strOutError =
				"Reconstructed render-resource publication digest binding is invalid.";
			return false;
		}

		EFFECT_RECONSTRUCTED_RENDER_RESOURCE_AUTHORITY StagedAuthority;
		if (!Parse_ReconstructedRenderResourceSidecar(
			SidecarText->Get_String(), StagedAuthority, strOutError))
		{
			return false;
		}
		if (InOutIdentity.strEffectAssetId != EFFECT_ASSET_ID ||
			InOutIdentity.strProgramId != PROGRAM_ID ||
			InOutIdentity.iProgramVersion != iLinkProgramVersion ||
			InOutIdentity.strProgramSha256 != PROGRAM_SHA256 ||
			StagedAuthority.Identity.strProgramId != InOutIdentity.strProgramId ||
			StagedAuthority.Identity.iProgramVersion !=
				InOutIdentity.iProgramVersion ||
			StagedAuthority.Identity.strProgramSha256 !=
				InOutIdentity.strProgramSha256)
		{
			strOutError =
				"Reconstructed render-resource program join identity is invalid.";
			return false;
		}
		StagedAuthority.Identity.strAuthorityLinkSha256 = ComputedLinkSha;
		StagedAuthority.Identity.strPublishReceiptSha256 =
			OuterReceiptSha->Get_String();
		EFFECT_RUNTIME_PROGRAM_CATALOG_IDENTITY StagedIdentity = InOutIdentity;
		StagedIdentity.iRenderResourceSidecarFormatVersion =
			iSidecarFormatVersion;
		StagedIdentity.iRenderResourceSidecarByteCount = iSidecarByteCount;
		StagedIdentity.strRenderResourceSidecarSchema = std::string(SIDECAR_SCHEMA);
		StagedIdentity.strRenderResourceAuthorityId =
			std::string(SIDECAR_AUTHORITY_ID);
		StagedIdentity.strRenderResourceSidecarDecisionProjectionSha256 =
			std::string(SIDECAR_DECISION_SHA256);
		StagedIdentity.strRenderResourceSidecarReceiptSha256 =
			std::string(SIDECAR_RECEIPT_SHA256);
		StagedIdentity.strRenderResourceSidecarRawSha256 =
			std::string(SIDECAR_RAW_SHA256);
		StagedIdentity.strRenderResourceAuthorityLinkSha256 = ComputedLinkSha;
		StagedIdentity.strRenderResourcePublishReceiptSha256 =
			OuterReceiptSha->Get_String();
		std::shared_ptr<const EFFECT_RECONSTRUCTED_RENDER_RESOURCE_AUTHORITY>
			StagedPointer =
				std::make_shared<const
					EFFECT_RECONSTRUCTED_RENDER_RESOURCE_AUTHORITY>(
						std::move(StagedAuthority));
		InOutIdentity = std::move(StagedIdentity);
		OutAuthority = std::move(StagedPointer);
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

	bool Is_ProductBoundedString(const std::string& Value)
	{
		return !Value.empty() && Value.size() <= 512u;
	}

	bool Read_ProductU32(const Client::DATA_JSON_VALUE& Object,
		const char* pName, uint32_t& iOutValue)
	{
		const Client::DATA_JSON_VALUE* pValue = Required(
			Object, pName, Client::DATA_JSON_TYPE::NUMBER);
		if (nullptr == pValue || pValue->Was_FloatingPointToken() ||
			!std::isfinite(pValue->Get_Number()) ||
			pValue->Get_Number() != std::floor(pValue->Get_Number()) ||
			pValue->Get_Number() < 0.0 ||
			pValue->Get_Number() > static_cast<double>(UINT32_MAX))
		{
			return false;
		}
		iOutValue = static_cast<uint32_t>(pValue->Get_Number());
		return true;
	}

	bool Is_ProductStableId(const std::string& Value)
	{
		return Is_ProductBoundedString(Value) &&
			std::all_of(Value.begin(), Value.end(), [](const char Character)
			{
				return (Character >= 'a' && Character <= 'z') ||
					(Character >= 'A' && Character <= 'Z') ||
					(Character >= '0' && Character <= '9') ||
					Character == '_' || Character == '-' || Character == '.';
			});
	}

	std::string Build_ProductCueAdmissionKey(
		const std::string& strAnimationAssetId,
		const std::string& strClipName,
		const uint32_t iStartMs,
		const std::string& strEffectAssetId)
	{
		return strAnimationAssetId + "\n" + strClipName + "\n" +
			std::to_string(iStartMs) + "\n" + strEffectAssetId;
	}

	const char_t* ProductCueCharacterClassLabel(
		const LostArk::Shared::CHARACTER_CLASS_ID eCharacterClass)
	{
		using LostArk::Shared::CHARACTER_CLASS_ID;
		switch (eCharacterClass)
		{
		case CHARACTER_CLASS_ID::LANCE_MASTER: return "LANCE_MASTER";
		case CHARACTER_CLASS_ID::GUNSLINGER: return "GUNSLINGER";
		case CHARACTER_CLASS_ID::SLAYER: return "SLAYER";
		case CHARACTER_CLASS_ID::ARTIST: return "ARTIST";
		case CHARACTER_CLASS_ID::DIMENSIONMASTER: return "DIMENSIONMASTER";
		case CHARACTER_CLASS_ID::WARLORD: return "WARLORD";
		case CHARACTER_CLASS_ID::END:
		default: return nullptr;
		}
	}

	void Append_ProductCueAdmissionIdentityPart(
		std::string& Target, const std::string_view Value)
	{
		Target += std::to_string(Value.size());
		Target.push_back(':');
		Target.append(Value.data(), Value.size());
	}

	std::string Build_ProductCueAdmissionIdentity(
		const Client::EFFECT_PRODUCT_CUE_ADMISSION& Admission)
	{
		std::string Projection;
		Projection.reserve(1024u);
		const auto Append = [&Projection](const std::string_view Value)
		{
			Append_ProductCueAdmissionIdentityPart(Projection, Value);
		};
		Append(Admission.strCueId);
		Append(std::to_string(static_cast<uint32_t>(Admission.eAdmissionClass)));
		Append(Admission.strApprovalCeiling);
		Append(Admission.strObservedExactness);
		Append(Admission.strCharacterClass);
		Append(Admission.strInputSlot);
		Append(std::to_string(Admission.iSkillId));
		Append(std::to_string(Admission.iStageIndex));
		Append(Admission.strAnimationAssetId);
		Append(Admission.strClipName);
		Append(std::to_string(Admission.iStartMs));
		Append(Admission.strEffectAssetId);
		Append(Admission.strRollbackEffectAssetId);
		Append(Admission.strEffectContentSha256);
		Append(std::to_string(Admission.iElementCount));
		Append(std::to_string(Admission.iFullElementCount));
		Append(std::to_string(Admission.iApproximateElementCount));
		Append(std::to_string(Admission.iHardSuppressedElementCount));
		Append(Admission.Provenance.strDecision);
		Append(Admission.Provenance.strApprovedAtKst);
		Append(Admission.Provenance.strSourceThreadId);
		Append(Admission.Provenance.strScope);
		return Client::CEffectRuntimeAuthorityCodec::Compute_Sha256Hex(Projection);
	}

	bool Count_ProductAdmissionElements(
		const Client::EFFECT_DOCUMENT_DESC& Document,
		uint32_t& iOutFullCount,
		uint32_t& iOutApproximateCount,
		uint32_t& iOutHardSuppressedCount,
		std::string& strOutError)
	{
		using namespace Client;
		if (Document.Elements.size() > UINT32_MAX)
		{
			strOutError = "Product-managed Effect element count exceeds uint32.";
			return false;
		}
		iOutFullCount = 0u;
		iOutApproximateCount = 0u;
		iOutHardSuppressedCount = 0u;
		for (const EFFECT_ELEMENT_DESC& Element : Document.Elements)
		{
			const EFFECT_MATERIAL_EXECUTION_DESC& Execution =
				Element.Material.Execution;
			if (Execution.bAuthoringApproximate)
			{
				if (!Execution.bFailClosed)
				{
					strOutError =
						"Product-managed approximate carrier is not fail-closed tagged: " +
						Element.strElementId;
					return false;
				}
				++iOutApproximateCount;
			}
			else if (Execution.bFailClosed)
			{
				if (Execution.bEnabled || Element.bVisible)
				{
					strOutError =
						"Product-managed hard carrier is not suppressed: " +
						Element.strElementId;
					return false;
				}
				++iOutHardSuppressedCount;
			}
			else
			{
				++iOutFullCount;
			}
		}
		strOutError.clear();
		return true;
	}

	bool Load_ProductCueAdmissionSidecar(
		const std::filesystem::path& SidecarPath,
		const std::string& strRuntimeCatalogUtf8,
		const bool_t bSidecarRequired,
		const std::string& strExpectedPolicySha256,
		const std::map<std::string,
			std::shared_ptr<const Client::EFFECT_DOCUMENT_DESC>,
			std::less<>>& Effects,
		const std::map<std::string,
			std::shared_ptr<const Client::EFFECT_RUNTIME_PROGRAM_CATALOG_ENTRY>,
			std::less<>>& RuntimeProgramEntries,
		const std::map<std::string, std::string, std::less<>>&
			DirectAuthoredContentShaByEffect,
		std::map<std::string,
			std::shared_ptr<const Client::EFFECT_PRODUCT_CUE_ADMISSION>,
			std::less<>>& OutAdmissions,
		std::map<std::string,
			std::vector<std::shared_ptr<const
				Client::EFFECT_PRODUCT_CUE_ADMISSION>>, std::less<>>&
			OutAdmissionsByEffect,
		std::string& strOutError)
	{
		using namespace Client;
		OutAdmissions.clear();
		OutAdmissionsByEffect.clear();
		if (!bSidecarRequired)
		{
			/* Direct authored Product uses the document's three-way execution
			   state directly: Full and authoring-approximate carriers execute,
			   while hard fail-closed carriers remain suppressed.  An older
			   admission sidecar may remain beside the catalog as inert rollback
			   evidence, but it is no longer a runtime authority.  The publisher
			   validates all three states; startup keeps direct documents sealed
			   and loads only the cues that are actually prepared. */
			strOutError.clear();
			return true;
		}
		if (!std::filesystem::is_regular_file(SidecarPath))
		{
			strOutError =
				"Typed Effect runtime catalog requires EffectProductCueAdmissions.runtime.json.";
			return false;
		}
		if (!bSidecarRequired ||
			!Is_LowerHexSha256(strExpectedPolicySha256))
		{
			strOutError =
				"Product cue admission sidecar is present without a catalog policy marker.";
			return false;
		}

		std::ifstream Input(SidecarPath, std::ios::binary);
		if (!Input)
		{
			strOutError = "Product cue admission sidecar could not be opened.";
			return false;
		}
		const std::string Text{
			std::istreambuf_iterator<char>(Input),
			std::istreambuf_iterator<char>() };
		DATA_JSON_VALUE Root;
		DATA_JSON_PARSE_LIMITS Limits;
		Limits.iMaximumBytes = 4u * 1024u * 1024u;
		Limits.iMaximumDepth = 16u;
		Limits.iMaximumValues = 100'000u;
		if (!CDataJson::Parse(Text, Root, strOutError, Limits) ||
			!Has_ExactOrderedKeys(Root, {
				"schema", "formatVersion", "runtimeCatalogSha256",
				"sourcePolicySha256", "sourcePolicyUtf8Json", "decisionSetId",
				"admissionMode", "approvals" }))
		{
			if (strOutError.empty())
				strOutError = "Product cue admission root fields or order are invalid.";
			return false;
		}
		const DATA_JSON_VALUE* pSchema = Required(
			Root, "schema", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pRuntimeCatalogSha = Required(
			Root, "runtimeCatalogSha256", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pSourcePolicySha = Required(
			Root, "sourcePolicySha256", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pSourcePolicyText = Required(
			Root, "sourcePolicyUtf8Json", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pDecisionSetId = Required(
			Root, "decisionSetId", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pAdmissionMode = Required(
			Root, "admissionMode", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pApprovals = Required(
			Root, "approvals", DATA_JSON_TYPE::ARRAY);
		uint32_t iFormatVersion = 0u;
		if (nullptr == pSchema ||
			pSchema->Get_String() != "lostark.effect-product-cue-admissions" ||
			!Read_ProductU32(Root, "formatVersion", iFormatVersion) ||
			1u != iFormatVersion || nullptr == pRuntimeCatalogSha ||
			!Is_LowerHexSha256(pRuntimeCatalogSha->Get_String()) ||
			pRuntimeCatalogSha->Get_String() !=
				CEffectRuntimeAuthorityCodec::Compute_Sha256Hex(
					strRuntimeCatalogUtf8) ||
			nullptr == pSourcePolicySha ||
			!Is_LowerHexSha256(pSourcePolicySha->Get_String()) ||
			pSourcePolicySha->Get_String() != strExpectedPolicySha256 ||
			nullptr == pSourcePolicyText ||
			CEffectRuntimeAuthorityCodec::Compute_Sha256Hex(
				pSourcePolicyText->Get_String()) != strExpectedPolicySha256 ||
			nullptr == pDecisionSetId ||
			!Is_ProductStableId(pDecisionSetId->Get_String()) ||
			nullptr == pAdmissionMode ||
			pAdmissionMode->Get_String() != "EXPLICIT_CUE_OPT_IN_ONLY" ||
			nullptr == pApprovals || pApprovals->Get_Array().empty() ||
			pApprovals->Get_Array().size() > 4096u)
		{
			strOutError = "Product cue admission header or catalog hash is invalid.";
			return false;
		}

		DATA_JSON_VALUE PolicyRoot;
		DATA_JSON_PARSE_LIMITS PolicyLimits;
		PolicyLimits.iMaximumBytes = 1024u * 1024u;
		PolicyLimits.iMaximumDepth = 16u;
		PolicyLimits.iMaximumValues = 50'000u;
		std::string PolicyError;
		if (!CDataJson::Parse(pSourcePolicyText->Get_String(), PolicyRoot,
				PolicyError, PolicyLimits) ||
			!Has_ExactOrderedKeys(PolicyRoot, { "schema", "formatVersion",
				"decisionSetId", "defaultAdmission", "approvals" }))
		{
			strOutError = "Product cue embedded source policy is invalid: " +
				PolicyError;
			return false;
		}
		const DATA_JSON_VALUE* pPolicySchema = Required(
			PolicyRoot, "schema", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pPolicyDecisionSetId = Required(
			PolicyRoot, "decisionSetId", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pPolicyDefaultAdmission = Required(
			PolicyRoot, "defaultAdmission", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pPolicyApprovals = Required(
			PolicyRoot, "approvals", DATA_JSON_TYPE::ARRAY);
		uint32_t iPolicyFormatVersion = 0u;
		if (nullptr == pPolicySchema ||
			pPolicySchema->Get_String() !=
				"lostark.effect-product-cue-approval-policy" ||
			!Read_ProductU32(PolicyRoot, "formatVersion",
				iPolicyFormatVersion) || 1u != iPolicyFormatVersion ||
			nullptr == pPolicyDecisionSetId ||
			pPolicyDecisionSetId->Get_String() != pDecisionSetId->Get_String() ||
			nullptr == pPolicyDefaultAdmission ||
			pPolicyDefaultAdmission->Get_String() != "DENY" ||
			nullptr == pPolicyApprovals ||
			pPolicyApprovals->Get_Array().size() !=
				pApprovals->Get_Array().size())
		{
			strOutError =
				"Product cue embedded source policy header does not match its receipt.";
			return false;
		}
		std::map<std::string, const DATA_JSON_VALUE*, std::less<>>
			PolicyApprovalByCueId;
		for (const DATA_JSON_VALUE& PolicyValue :
			pPolicyApprovals->Get_Array())
		{
			if (!Has_ExactOrderedKeys(PolicyValue, {
					"cueId", "approvalCeiling", "animationAssetId",
					"characterClass", "inputSlot", "skillId", "stageIndex",
					"clipName", "startMs", "effectAssetId",
					"rollbackEffectAssetId", "effectContentSha256",
					"provenance" }))
			{
				strOutError =
					"Product cue source policy approval fields or order are invalid.";
				return false;
			}
			const DATA_JSON_VALUE* pPolicyCueId = Required(
				PolicyValue, "cueId", DATA_JSON_TYPE::STRING);
			if (nullptr == pPolicyCueId ||
				!Is_ProductStableId(pPolicyCueId->Get_String()) ||
				!PolicyApprovalByCueId.emplace(
					pPolicyCueId->Get_String(), &PolicyValue).second)
			{
				strOutError =
					"Product cue source policy approval identity is invalid or duplicate.";
				return false;
			}
		}

		std::string strPreviousCueId;
		std::map<std::string, std::string, std::less<>> EffectOwners;
		std::set<std::string, std::less<>> CueOwners;
		std::set<std::string, std::less<>> CandidateEffectIds;
		std::set<std::string, std::less<>> RollbackEffectIds;
		for (const DATA_JSON_VALUE& Value : pApprovals->Get_Array())
		{
			if (!Has_ExactOrderedKeys(Value, {
				"cueId", "admission", "approvalCeiling", "observedExactness",
				"characterClass", "inputSlot", "skillId", "stageIndex",
				"animationAssetId", "clipName", "startMs", "effectAssetId",
				"rollbackEffectAssetId", "effectContentSha256",
				"approximateElementCount", "hardSuppressedElementCount",
				"provenance" }))
			{
				strOutError =
					"Product cue admission fields or order are invalid.";
				return false;
			}
			const DATA_JSON_VALUE* pCueId = Required(
				Value, "cueId", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* pAdmission = Required(
				Value, "admission", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* pApprovalCeiling = Required(
				Value, "approvalCeiling", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* pObservedExactness = Required(
				Value, "observedExactness", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* pCharacterClass = Required(
				Value, "characterClass", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* pInputSlot = Required(
				Value, "inputSlot", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* pAnimationAssetId = Required(
				Value, "animationAssetId", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* pClipName = Required(
				Value, "clipName", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* pEffectAssetId = Required(
				Value, "effectAssetId", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* pRollbackEffectAssetId = Required(
				Value, "rollbackEffectAssetId", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* pContentSha = Required(
				Value, "effectContentSha256", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* pProvenance = Required(
				Value, "provenance", DATA_JSON_TYPE::OBJECT);
			uint32_t iSkillId = 0u;
			uint32_t iStageIndex = 0u;
			uint32_t iStartMs = 0u;
			uint32_t iApproximateCount = 0u;
			uint32_t iHardCount = 0u;
			if (nullptr == pCueId || !Is_ProductStableId(pCueId->Get_String()) ||
				(!strPreviousCueId.empty() &&
				 pCueId->Get_String() <= strPreviousCueId) ||
				nullptr == pAdmission ||
				(pAdmission->Get_String() != "PRODUCT_APPROVED_FULL" &&
				 pAdmission->Get_String() !=
					"PRODUCT_APPROVED_APPROXIMATE") ||
				nullptr == pApprovalCeiling ||
				pApprovalCeiling->Get_String() !=
					"PRODUCT_APPROVED_APPROXIMATE" ||
				nullptr == pObservedExactness ||
				(pObservedExactness->Get_String() != "FULL" &&
				 pObservedExactness->Get_String() !=
					"AUTHORING_APPROXIMATE") ||
				nullptr == pCharacterClass ||
				!Is_ProductStableId(pCharacterClass->Get_String()) ||
				nullptr == pInputSlot ||
				!Is_ProductStableId(pInputSlot->Get_String()) ||
				!Read_ProductU32(Value, "skillId", iSkillId) ||
				!Read_ProductU32(Value, "stageIndex", iStageIndex) ||
				iStageIndex > 255u || nullptr == pAnimationAssetId ||
				!Is_ProductStableId(pAnimationAssetId->Get_String()) ||
				nullptr == pClipName ||
				!Is_ProductBoundedString(pClipName->Get_String()) ||
				!Read_ProductU32(Value, "startMs", iStartMs) ||
				nullptr == pEffectAssetId ||
				!Is_ProductStableId(pEffectAssetId->Get_String()) ||
				nullptr == pRollbackEffectAssetId ||
				!Is_ProductStableId(pRollbackEffectAssetId->Get_String()) ||
				pEffectAssetId->Get_String() ==
					pRollbackEffectAssetId->Get_String() ||
				nullptr == pContentSha ||
				!Is_LowerHexSha256(pContentSha->Get_String()) ||
				!Read_ProductU32(Value, "approximateElementCount",
					iApproximateCount) ||
				!Read_ProductU32(Value, "hardSuppressedElementCount",
					iHardCount) ||
				nullptr == pProvenance ||
				!Has_ExactOrderedKeys(*pProvenance, {
					"decision", "approvedAtKst", "sourceThreadId", "scope" }))
			{
				strOutError =
					"Product cue admission identity or counts are invalid.";
				return false;
			}

			const DATA_JSON_VALUE* pDecision = Required(
				*pProvenance, "decision", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* pApprovedAtKst = Required(
				*pProvenance, "approvedAtKst", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* pSourceThreadId = Required(
				*pProvenance, "sourceThreadId", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* pScope = Required(
				*pProvenance, "scope", DATA_JSON_TYPE::STRING);
			if (nullptr == pDecision ||
				pDecision->Get_String() != "EXPLICIT_USER_OPT_IN" ||
				nullptr == pApprovedAtKst ||
				!Is_ProductBoundedString(pApprovedAtKst->Get_String()) ||
				nullptr == pSourceThreadId ||
				!Is_ProductBoundedString(pSourceThreadId->Get_String()) ||
				nullptr == pScope ||
				!Is_ProductBoundedString(pScope->Get_String()))
			{
				strOutError = "Product cue admission provenance is invalid.";
				return false;
			}

			const auto PolicyIterator =
				PolicyApprovalByCueId.find(pCueId->Get_String());
			if (PolicyApprovalByCueId.end() == PolicyIterator)
			{
				strOutError =
					"Product cue receipt approval is absent from its source policy: " +
					pCueId->Get_String();
				return false;
			}
			const DATA_JSON_VALUE& PolicyValue = *PolicyIterator->second;
			const auto PolicyStringEquals = [&PolicyValue](
				const char* pName, const std::string& Expected)
			{
				const DATA_JSON_VALUE* pValue = Required(
					PolicyValue, pName, DATA_JSON_TYPE::STRING);
				return nullptr != pValue && pValue->Get_String() == Expected;
			};
			uint32_t iPolicySkillId = 0u;
			uint32_t iPolicyStageIndex = 0u;
			uint32_t iPolicyStartMs = 0u;
			const DATA_JSON_VALUE* pPolicyProvenance = Required(
				PolicyValue, "provenance", DATA_JSON_TYPE::OBJECT);
			if (!PolicyStringEquals("approvalCeiling",
					pApprovalCeiling->Get_String()) ||
				!PolicyStringEquals("animationAssetId",
					pAnimationAssetId->Get_String()) ||
				!PolicyStringEquals("characterClass",
					pCharacterClass->Get_String()) ||
				!PolicyStringEquals("inputSlot", pInputSlot->Get_String()) ||
				!Read_ProductU32(PolicyValue, "skillId", iPolicySkillId) ||
				iPolicySkillId != iSkillId ||
				!Read_ProductU32(PolicyValue, "stageIndex",
					iPolicyStageIndex) || iPolicyStageIndex != iStageIndex ||
				!PolicyStringEquals("clipName", pClipName->Get_String()) ||
				!Read_ProductU32(PolicyValue, "startMs", iPolicyStartMs) ||
				iPolicyStartMs != iStartMs ||
				!PolicyStringEquals("effectAssetId",
					pEffectAssetId->Get_String()) ||
				!PolicyStringEquals("rollbackEffectAssetId",
					pRollbackEffectAssetId->Get_String()) ||
				!PolicyStringEquals("effectContentSha256",
					pContentSha->Get_String()) ||
				nullptr == pPolicyProvenance ||
				!Has_ExactOrderedKeys(*pPolicyProvenance, {
					"decision", "approvedAtKst", "sourceThreadId", "scope" }))
			{
				strOutError =
					"Product cue receipt approval diverges from its source policy: " +
					pCueId->Get_String();
				return false;
			}
			for (const char* pField :
				{ "decision", "approvedAtKst", "sourceThreadId", "scope" })
			{
				const DATA_JSON_VALUE* pPolicyField = Required(
					*pPolicyProvenance, pField, DATA_JSON_TYPE::STRING);
				const DATA_JSON_VALUE* pReceiptField = Required(
					*pProvenance, pField, DATA_JSON_TYPE::STRING);
				if (nullptr == pPolicyField || nullptr == pReceiptField ||
					pPolicyField->Get_String() != pReceiptField->Get_String())
				{
					strOutError =
						"Product cue receipt provenance diverges from its source policy: " +
						pCueId->Get_String();
					return false;
				}
			}

			const auto ContentShaIterator =
				DirectAuthoredContentShaByEffect.find(
					pEffectAssetId->Get_String());
			const auto EffectIterator = Effects.find(
				pEffectAssetId->Get_String());
			if (ContentShaIterator ==
					DirectAuthoredContentShaByEffect.end() ||
				EffectIterator == Effects.end() ||
				(!Effects.contains(pRollbackEffectAssetId->Get_String()) &&
				 !RuntimeProgramEntries.contains(
					pRollbackEffectAssetId->Get_String())) ||
				ContentShaIterator->second != pContentSha->Get_String())
			{
				strOutError =
					"Product cue admission target, rollback, or authored hash is missing: " +
					pCueId->Get_String();
				return false;
			}
			uint32_t iFullCount = 0u;
			uint32_t iComputedApproximateCount = 0u;
			uint32_t iComputedHardCount = 0u;
			if (!Count_ProductAdmissionElements(*EffectIterator->second,
					iFullCount, iComputedApproximateCount, iComputedHardCount,
					strOutError) ||
				iComputedApproximateCount != iApproximateCount ||
				iComputedHardCount != iHardCount)
			{
				if (strOutError.empty())
					strOutError =
						"Product cue admission element counts drifted: " +
						pCueId->Get_String();
				return false;
			}
			const bool_t bApproximate = 0u != iApproximateCount;
			if ((bApproximate &&
				 (pAdmission->Get_String() !=
					"PRODUCT_APPROVED_APPROXIMATE" ||
				  pObservedExactness->Get_String() !=
					"AUTHORING_APPROXIMATE")) ||
				(!bApproximate &&
				 (pAdmission->Get_String() != "PRODUCT_APPROVED_FULL" ||
				  pObservedExactness->Get_String() != "FULL")))
			{
				strOutError =
					"Product cue admission exactness does not match its runtime document: " +
					pCueId->Get_String();
				return false;
			}

			auto Admission = std::make_shared<EFFECT_PRODUCT_CUE_ADMISSION>();
			Admission->strCueId = pCueId->Get_String();
			Admission->eAdmissionClass = bApproximate ?
				EFFECT_PRODUCT_CUE_ADMISSION_CLASS::
					PRODUCT_APPROVED_APPROXIMATE :
				EFFECT_PRODUCT_CUE_ADMISSION_CLASS::PRODUCT_APPROVED_FULL;
			Admission->strApprovalCeiling = pApprovalCeiling->Get_String();
			Admission->strObservedExactness =
				pObservedExactness->Get_String();
			Admission->strCharacterClass = pCharacterClass->Get_String();
			Admission->strInputSlot = pInputSlot->Get_String();
			Admission->iSkillId = iSkillId;
			Admission->iStageIndex = iStageIndex;
			Admission->strAnimationAssetId =
				pAnimationAssetId->Get_String();
			Admission->strClipName = pClipName->Get_String();
			Admission->iStartMs = iStartMs;
			Admission->strEffectAssetId = pEffectAssetId->Get_String();
			Admission->strRollbackEffectAssetId =
				pRollbackEffectAssetId->Get_String();
			CandidateEffectIds.emplace(Admission->strEffectAssetId);
			RollbackEffectIds.emplace(Admission->strRollbackEffectAssetId);
			Admission->strEffectContentSha256 = pContentSha->Get_String();
			Admission->iElementCount =
				static_cast<uint32_t>(EffectIterator->second->Elements.size());
			Admission->iFullElementCount = iFullCount;
			Admission->iApproximateElementCount = iApproximateCount;
			Admission->iHardSuppressedElementCount = iHardCount;
			Admission->Provenance.strDecision = pDecision->Get_String();
			Admission->Provenance.strApprovedAtKst =
				pApprovedAtKst->Get_String();
			Admission->Provenance.strSourceThreadId =
				pSourceThreadId->Get_String();
			Admission->Provenance.strScope = pScope->Get_String();

			const std::string Key = Build_ProductCueAdmissionKey(
				Admission->strAnimationAssetId, Admission->strClipName,
				Admission->iStartMs, Admission->strEffectAssetId);
			const std::string Owner = Admission->strAnimationAssetId + "\n" +
				Admission->strClipName + "\n" +
				std::to_string(Admission->iStartMs);
			if (!CueOwners.emplace(Owner).second)
			{
				strOutError =
					"Physical Product cue has multiple admissions: " +
					Admission->strCueId;
				return false;
			}
			const auto [OwnerIterator, bOwnerInserted] = EffectOwners.emplace(
				Admission->strEffectAssetId, Owner);
			if ((!bOwnerInserted && OwnerIterator->second != Owner) ||
				OutAdmissions.contains(Key))
			{
				strOutError =
					"Product cue admission tuple or managed Effect owner is duplicate: " +
					Admission->strCueId;
				return false;
			}
			std::shared_ptr<const EFFECT_PRODUCT_CUE_ADMISSION> Committed =
				std::move(Admission);
			const std::string CommittedEffectId = Committed->strEffectAssetId;
			OutAdmissions.emplace(Key, Committed);
			OutAdmissionsByEffect[CommittedEffectId].push_back(
				std::move(Committed));
			strPreviousCueId = pCueId->Get_String();
		}

		for (const std::string& CandidateEffectId : CandidateEffectIds)
		{
			if (RollbackEffectIds.contains(CandidateEffectId))
			{
				strOutError =
					"Product cue candidate and rollback sets overlap: " +
					CandidateEffectId;
				return false;
			}
		}

		for (const auto& [EffectId, ContentSha] :
			DirectAuthoredContentShaByEffect)
		{
			(void)ContentSha;
			uint32_t iFullCount = 0u;
			uint32_t iApproximateCount = 0u;
			uint32_t iHardCount = 0u;
			if (!Count_ProductAdmissionElements(*Effects.at(EffectId),
					iFullCount, iApproximateCount, iHardCount, strOutError))
			{
				return false;
			}
			if (0u != iApproximateCount &&
				!OutAdmissionsByEffect.contains(EffectId))
			{
				strOutError =
					"Approximate direct-authored Effect lacks explicit cue admission: " +
					EffectId;
				return false;
			}
		}
		strOutError.clear();
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
				"sealed authored document is missing, empty, or exceeds 16 MiB";
			return false;
		}

		std::ifstream Input(Source.DocumentPath, std::ios::binary);
		if (!Input)
		{
			strOutError = "sealed authored document could not be opened";
			return false;
		}
		const std::string Text{
			std::istreambuf_iterator<char>(Input),
			std::istreambuf_iterator<char>() };
		if (Text.size() != FileBytes ||
			CEffectRuntimeAuthorityCodec::Compute_Sha256Hex(Text) !=
				Source.strContentSha256)
		{
			strOutError = "sealed authored document content hash drifted";
			return false;
		}

		EFFECT_DOCUMENT_DESC Document;
		if (!CEffectDocumentCodec::Parse(Text, Document, strOutError) ||
			!CEffectDocumentCodec::Validate_Drawable(Document, strOutError) ||
			Document.bSourceContract ||
			Document.iLoadedFormatVersion != EFFECT_AUTHORING_FORMAT_VERSION ||
			Document.strEffectAssetId != EffectAssetId)
		{
			if (strOutError.empty())
			{
				strOutError =
					"sealed authored document identity or drawable admission mismatched";
			}
			return false;
		}

		std::vector<std::string> DocumentDependencies;
		CEffectDocumentCodec::Collect_ResourceAssetIds(
			Document, DocumentDependencies);
		if (DocumentDependencies.size() != Source.Dependencies.size() ||
			!std::equal(DocumentDependencies.begin(), DocumentDependencies.end(),
				Source.Dependencies.begin(),
				[](const std::string& AssetId, const auto& Dependency)
				{
					return AssetId == Dependency.first;
				}))
		{
			strOutError =
				"sealed authored document dependency set mismatched its catalog row";
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

		const auto ProgramIterator = g_VisualPrograms.find(EffectAssetId);
		if (ProgramIterator != g_VisualPrograms.end())
		{
			std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>
				Projection;
			if (nullptr == g_pVisualProgramCorpus ||
				!CEffectVisualProgramCorpusCodec::Create_DocumentProjection(
					*g_pVisualProgramCorpus, *Committed, Projection, strOutError) ||
				nullptr == Projection || !Projection->Is_Valid() ||
				Projection->Get_EffectAssetId() != EffectAssetId ||
				Projection->Get_ProjectionKind() !=
					ProgramIterator->second->eProjectionKind)
			{
				if (strOutError.empty())
					strOutError = "direct authored visual projection is invalid";
				return nullptr;
			}
			Committed = Projection->Get_DocumentShared();
			g_VisualProjections.emplace(EffectAssetId, std::move(Projection));
		}

		const auto [Iterator, Inserted] =
			g_Effects.emplace(EffectAssetId, std::move(Committed));
		if (!Inserted)
		{
			strOutError = "direct authored runtime cache identity is duplicate";
			return nullptr;
		}
		strOutError.clear();
		return Iterator->second;
	}
}

struct Client::CEffectCatalog::RUNTIME_SNAPSHOT final
{
	std::map<std::string,
		std::shared_ptr<const EFFECT_DOCUMENT_DESC>, std::less<>> Effects;
	std::map<std::string,
		std::shared_ptr<const DIRECT_AUTHORED_RUNTIME_SOURCE>, std::less<>>
		DirectAuthoredSources;
	std::map<std::string,
		std::shared_ptr<const EFFECT_ASSEMBLY_DESC>, std::less<>> Assemblies;
	std::map<std::string,
		std::shared_ptr<const EFFECT_COMPONENT_DESC>, std::less<>> Components;
	std::map<std::string,
		std::shared_ptr<const EFFECT_COMPILED_RUNTIME_DOCUMENT>, std::less<>>
		RuntimeAuthorities;
	std::map<std::string,
		std::shared_ptr<const EFFECT_RUNTIME_PROGRAM_CATALOG_ENTRY>, std::less<>>
		RuntimeProgramEntries;
	std::shared_ptr<const EFFECT_VISUAL_PROGRAM_CORPUS> pVisualProgramCorpus;
	std::map<std::string,
		std::shared_ptr<const EFFECT_VISUAL_PROGRAM>, std::less<>> VisualPrograms;
	std::map<std::string,
		std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>,
		std::less<>> VisualProjections;
	std::map<std::string,
		std::shared_ptr<const EFFECT_PRODUCT_CUE_ADMISSION>, std::less<>>
		ProductCueAdmissions;
	std::map<std::string,
		std::vector<std::shared_ptr<const EFFECT_PRODUCT_CUE_ADMISSION>>,
		std::less<>> ProductCueAdmissionsByEffect;
	uint64_t iRuntimeRevision = 0u;
	std::string strStatus;
};

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
    const std::filesystem::path Path = Find_RuntimeCatalog();
    std::ifstream Input(Path, std::ios::binary);
    if (!Input)
    {
        strOutStatus = "Missing EffectCatalog.runtime.json: " + Path.string();
        g_strStatus = strOutStatus;
        return false;
    }
	const std::filesystem::path VisualProgramPath =
		Find_RuntimeVisualProgramSidecar(Path);
	std::shared_ptr<const EFFECT_VISUAL_PROGRAM_CORPUS>
		StagedVisualProgramCorpus;
	std::string VisualProgramError;
    const std::string Text{
        std::istreambuf_iterator<char>(Input),
        std::istreambuf_iterator<char>() };
    DATA_JSON_VALUE Root;
    std::string Error;
	DATA_JSON_PARSE_LIMITS RuntimeCatalogLimits;
	RuntimeCatalogLimits.iMaximumBytes = 256u * 1024u * 1024u;
	RuntimeCatalogLimits.iMaximumDepth = 64u;
	RuntimeCatalogLimits.iMaximumValues = 3'000'000u;
    if (!CDataJson::Parse(Text, Root, Error, RuntimeCatalogLimits) ||
		!Root.Is_Object())
    {
        strOutStatus = "Effect runtime catalog JSON parse failed: " + Error;
        g_strStatus = strOutStatus;
        return false;
    }
    const DATA_JSON_VALUE* pSchema = Required(
		Root, "schema", DATA_JSON_TYPE::STRING);
	const DATA_JSON_VALUE* pVersion = Required(
        Root, "formatVersion", DATA_JSON_TYPE::NUMBER);
	const DATA_JSON_VALUE* pComponents = Required(
		Root, "components", DATA_JSON_TYPE::ARRAY);
    const DATA_JSON_VALUE* pEffects = Required(
        Root, "effects", DATA_JSON_TYPE::ARRAY);
	const DATA_JSON_VALUE* pProductCueAdmissionsRequired =
		Root.Find("productCueAdmissionsRequired");
	const DATA_JSON_VALUE* pProductCuePolicySha =
		Root.Find("productCuePolicySha256");
	const DATA_JSON_VALUE* pVisualProgramSidecarRequired =
		Root.Find("visualProgramSidecarRequired");
	const bool_t bVisualProgramSidecarRequired =
		nullptr == pVisualProgramSidecarRequired ? true :
		pVisualProgramSidecarRequired->Is_Boolean() &&
		pVisualProgramSidecarRequired->Get_Boolean();
	const bool_t bVisualProgramSidecarMarkerValid =
		nullptr == pVisualProgramSidecarRequired ||
		pVisualProgramSidecarRequired->Is_Boolean();
	const bool_t bIntegralCatalogVersion = nullptr != pVersion &&
		!pVersion->Was_FloatingPointToken();
    const bool_t bLegacyCatalog = bIntegralCatalogVersion &&
		2.0 == pVersion->Get_Number();
	const bool_t bProductCueAdmissionsRequired =
		nullptr != pProductCueAdmissionsRequired &&
		pProductCueAdmissionsRequired->Is_Boolean() &&
		pProductCueAdmissionsRequired->Get_Boolean() &&
		nullptr != pProductCuePolicySha &&
		pProductCuePolicySha->Is_String() &&
		Is_LowerHexSha256(pProductCuePolicySha->Get_String()) &&
		(Has_ExactOrderedKeys(Root, { "schema", "formatVersion",
			"productCueAdmissionsRequired", "productCuePolicySha256",
			"components", "effects" }) ||
		 Has_ExactOrderedKeys(Root, { "schema", "formatVersion",
			"visualProgramSidecarRequired", "productCueAdmissionsRequired",
			"productCuePolicySha256", "components", "effects" }));
	const bool_t bPlainDerivedCatalog =
		Has_ExactOrderedKeys(Root,
			{ "schema", "formatVersion", "components", "effects" }) ||
		Has_ExactOrderedKeys(Root, { "schema", "formatVersion",
			"visualProgramSidecarRequired", "components", "effects" });
	const bool_t bDerivedCatalog = bIntegralCatalogVersion &&
		3.0 == pVersion->Get_Number() && nullptr != pSchema &&
		pSchema->Get_String() == "lostark.effect-runtime-catalog" &&
		bVisualProgramSidecarMarkerValid &&
		(bPlainDerivedCatalog || bProductCueAdmissionsRequired);
    if ((!bLegacyCatalog && !bDerivedCatalog) ||
		nullptr == pComponents || nullptr == pEffects)
    {
        strOutStatus = "Effect runtime catalog must be legacy formatVersion 2 or typed formatVersion 3.";
        g_strStatus = strOutStatus;
        return false;
    }

    std::map<std::string,
        std::shared_ptr<const EFFECT_DOCUMENT_DESC>, std::less<>> Staged;
	std::map<std::string,
		std::shared_ptr<const DIRECT_AUTHORED_RUNTIME_SOURCE>, std::less<>>
		StagedDirectAuthoredSources;
	std::map<std::string,
		std::shared_ptr<const EFFECT_COMPONENT_DESC>, std::less<>>
		StagedComponents;
	std::map<std::string,
		std::shared_ptr<const EFFECT_ASSEMBLY_DESC>, std::less<>>
		StagedAssemblies;
	std::map<std::string,
		std::shared_ptr<const EFFECT_COMPILED_RUNTIME_DOCUMENT>, std::less<>>
		StagedRuntimeAuthorities;
	std::map<std::string,
		std::shared_ptr<const EFFECT_RUNTIME_PROGRAM_CATALOG_ENTRY>, std::less<>>
		StagedRuntimeProgramEntries;
	std::map<std::string,
		std::shared_ptr<const EFFECT_VISUAL_PROGRAM>, std::less<>>
		StagedVisualPrograms;
	std::map<std::string,
		std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>,
		std::less<>> StagedVisualProjections;
	std::map<std::string, std::string, std::less<>>
		StagedDirectAuthoredContentShaByEffect;
	std::map<std::string,
		std::shared_ptr<const EFFECT_PRODUCT_CUE_ADMISSION>, std::less<>>
		StagedProductCueAdmissions;
	std::map<std::string,
		std::vector<std::shared_ptr<const EFFECT_PRODUCT_CUE_ADMISSION>>,
		std::less<>> StagedProductCueAdmissionsByEffect;
	if (UINT64_MAX == g_iRuntimeRevision)
	{
		strOutStatus = "Effect runtime catalog revision is exhausted.";
		g_strStatus = strOutStatus;
		return false;
	}
	const uint64_t iStagedCatalogRevision = g_iRuntimeRevision + 1u;
	for (const DATA_JSON_VALUE& ComponentValue : pComponents->Get_Array())
	{
		EFFECT_COMPONENT_DESC Component;
		if (!Parse_Component(ComponentValue, Component, Error))
		{
			strOutStatus = "Effect runtime Component rejected: " + Error;
			g_strStatus = strOutStatus;
			return false;
		}
		const std::string ComponentId = Component.strComponentAssetId;
		if (!StagedComponents.emplace(ComponentId,
			std::make_shared<const EFFECT_COMPONENT_DESC>(
				std::move(Component))).second)
		{
			strOutStatus = "Duplicate Effect Component in runtime catalog: " +
				ComponentId;
			g_strStatus = strOutStatus;
			return false;
		}
	}
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
		const DATA_JSON_VALUE* pPayloadKind = Required(
			Entry, "payloadKind", DATA_JSON_TYPE::STRING);
		const bool_t bReconstructedPayload = bDerivedCatalog &&
			nullptr != pPayloadKind && pPayloadKind->Get_String() ==
				"IMMUTABLE_RECONSTRUCTED_RUNTIME_PROGRAM";
		if (nullptr != pAssetId &&
			Is_ReconstructedRuntimeProgramAssetId(pAssetId->Get_String()) &&
			!bReconstructedPayload)
		{
			strOutStatus =
				"Artist 31470 reconstructed authority cannot use a legacy or generic payload.";
			g_strStatus = strOutStatus;
			return false;
		}
		if (bReconstructedPayload)
		{
			PARSED_RECONSTRUCTED_RUNTIME_ENTRY Parsed;
			if (!Parse_ReconstructedRuntimeProgramEntry(
					Entry, iStagedCatalogRevision, Parsed, Error) ||
				nullptr == pAssetId || nullptr == Parsed.pProgram ||
				Parsed.Identity.strEffectAssetId != pAssetId->Get_String() ||
				Staged.contains(pAssetId->Get_String()) ||
				StagedDirectAuthoredSources.contains(pAssetId->Get_String()) ||
				StagedRuntimeAuthorities.contains(pAssetId->Get_String()) ||
				StagedRuntimeProgramEntries.contains(pAssetId->Get_String()))
			{
				if (Error.empty())
					Error = "duplicate or mismatched reconstructed runtime identity";
				strOutStatus =
					"Effect reconstructed runtime program rejected: " + Error;
				g_strStatus = strOutStatus;
				return false;
			}
			std::shared_ptr<const EFFECT_RUNTIME_PROGRAM_CATALOG_ENTRY>
				ProgramEntry(new EFFECT_RUNTIME_PROGRAM_CATALOG_ENTRY(
					std::move(Parsed.Identity), std::move(Parsed.pProgram),
					std::move(Parsed.pRenderResourceAuthority),
					std::move(Parsed.pOccurrenceTuning)));
			StagedRuntimeProgramEntries.emplace(
				pAssetId->Get_String(), std::move(ProgramEntry));
			continue;
		}
		const bool_t bDirectAuthoredPayload = bDerivedCatalog &&
			nullptr != pPayloadKind && pPayloadKind->Get_String() ==
				"DIRECT_AUTHORED_DOCUMENT_V13";
		if (bDirectAuthoredPayload)
		{
			if (!Has_ExactOrderedKeys(Entry, {
				"payloadKind", "effectAssetId", "authoringFormatVersion",
				"authoredDocumentPath", "contentSha256", "dependencies" }))
			{
				strOutStatus =
					"Effect direct authored runtime entry fields or order are invalid.";
				g_strStatus = strOutStatus;
				return false;
			}
			const DATA_JSON_VALUE* pAuthoringVersion = Required(
				Entry, "authoringFormatVersion", DATA_JSON_TYPE::NUMBER);
			const DATA_JSON_VALUE* pAuthoredDocumentPath = Required(
				Entry, "authoredDocumentPath", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* pContentSha = Required(
				Entry, "contentSha256", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* pDependencies = Required(
				Entry, "dependencies", DATA_JSON_TYPE::ARRAY);
			if (nullptr == pAssetId || pAssetId->Get_String().empty() ||
				nullptr == pAuthoringVersion ||
				pAuthoringVersion->Was_FloatingPointToken() ||
				pAuthoringVersion->Get_Number() !=
					static_cast<double>(EFFECT_AUTHORING_FORMAT_VERSION) ||
				nullptr == pAuthoredDocumentPath ||
				nullptr == pContentSha ||
				!Is_LowerHexSha256(pContentSha->Get_String()) ||
				!Is_SealedDirectAuthoredDocumentPath(
					pAuthoredDocumentPath->Get_String(), pAssetId->Get_String(),
					pContentSha->Get_String()) ||
				nullptr == pDependencies ||
				Staged.contains(pAssetId->Get_String()) ||
				StagedDirectAuthoredSources.contains(pAssetId->Get_String()) ||
				StagedRuntimeAuthorities.contains(pAssetId->Get_String()) ||
				StagedRuntimeProgramEntries.contains(pAssetId->Get_String()))
			{
				strOutStatus =
					"Effect direct authored runtime entry has an invalid identity or content hash.";
				g_strStatus = strOutStatus;
				return false;
			}

			std::map<std::string, std::string, std::less<>> Dependencies;
			std::string PreviousDependencyId;
			for (const DATA_JSON_VALUE& Dependency :
				pDependencies->Get_Array())
			{
				if (!Has_ExactOrderedKeys(Dependency, { "assetId", "sha256" }))
				{
					strOutStatus =
						"Effect direct authored dependency fields or order are invalid.";
					g_strStatus = strOutStatus;
					return false;
				}
				const DATA_JSON_VALUE* pDependencyId = Required(
					Dependency, "assetId", DATA_JSON_TYPE::STRING);
				const DATA_JSON_VALUE* pDependencySha = Required(
					Dependency, "sha256", DATA_JSON_TYPE::STRING);
				if (nullptr == pDependencyId ||
					(!CEffectDocumentCodec::Is_SafeResourceAssetId(
						pDependencyId->Get_String()) &&
					 !CEffectDocumentCodec::Is_SafeModelCueAssetId(
						pDependencyId->Get_String())) ||
					nullptr == pDependencySha ||
					!Is_LowerHexSha256(pDependencySha->Get_String()) ||
					(!PreviousDependencyId.empty() &&
					 pDependencyId->Get_String() <= PreviousDependencyId) ||
					!Dependencies.emplace(
						pDependencyId->Get_String(),
						pDependencySha->Get_String()).second)
				{
					strOutStatus =
						"Effect direct authored dependency is unsafe, duplicate, or unsorted.";
					g_strStatus = strOutStatus;
					return false;
				}
				PreviousDependencyId = pDependencyId->Get_String();
			}

			std::filesystem::path SealedPath;
			std::error_code SealedFileError;
			if (!Resolve_SealedDirectAuthoredDocumentPath(
					Path, pAuthoredDocumentPath->Get_String(), SealedPath) ||
				!std::filesystem::is_regular_file(SealedPath, SealedFileError) ||
				SealedFileError ||
				0u == std::filesystem::file_size(SealedPath, SealedFileError) ||
				SealedFileError)
			{
				strOutStatus =
					"Effect direct authored sealed document is missing or unsafe for " +
					pAssetId->Get_String() + ".";
				g_strStatus = strOutStatus;
				return false;
			}
			auto Source = std::make_shared<DIRECT_AUTHORED_RUNTIME_SOURCE>();
			Source->DocumentPath = std::move(SealedPath);
			Source->strContentSha256 = pContentSha->Get_String();
			Source->Dependencies = std::move(Dependencies);
			if (!StagedDirectAuthoredSources.emplace(
					pAssetId->Get_String(), std::move(Source)).second ||
				!StagedDirectAuthoredContentShaByEffect.emplace(
					pAssetId->Get_String(), pContentSha->Get_String()).second)
			{
				strOutStatus =
					"Duplicate direct authored EffectAssetId in runtime catalog: " +
					pAssetId->Get_String();
				g_strStatus = strOutStatus;
				return false;
			}
			continue;
		}
		if (bDerivedCatalog && nullptr != pPayloadKind &&
			pPayloadKind->Get_String() == "IMMUTABLE_COMPILED_IR")
		{
			std::shared_ptr<const EFFECT_COMPILED_RUNTIME_DOCUMENT> Document;
			const bool_t bParsed =
				CEffectRuntimeAuthorityCodec::Parse_DerivedEntry(
					Entry, Document, Error);
			if (!bParsed || nullptr == Document || nullptr == pAssetId ||
				Document->Identity.strEffectAssetId != pAssetId->Get_String() ||
				Staged.contains(pAssetId->Get_String()) ||
				StagedDirectAuthoredSources.contains(pAssetId->Get_String()) ||
				StagedRuntimeAuthorities.contains(pAssetId->Get_String()) ||
				StagedRuntimeProgramEntries.contains(pAssetId->Get_String()))
			{
				if (Error.empty())
					Error = "duplicate or mismatched compiled runtime identity";
				strOutStatus = "Effect compiled runtime authority rejected: " + Error;
				g_strStatus = strOutStatus;
				return false;
			}
			StagedRuntimeAuthorities.emplace(
				pAssetId->Get_String(), std::move(Document));
			continue;
		}
		if (bDerivedCatalog && (nullptr == pPayloadKind ||
			pPayloadKind->Get_String() != "LEGACY_ASSEMBLY_V1"))
		{
			strOutStatus = "Effect runtime catalog has an unsupported payload kind.";
			g_strStatus = strOutStatus;
			return false;
		}
        const DATA_JSON_VALUE* pAuthoringVersion = Required(
            Entry, "authoringFormatVersion", DATA_JSON_TYPE::NUMBER);
        const DATA_JSON_VALUE* pContentSha = Required(
            Entry, "contentSha256", DATA_JSON_TYPE::STRING);
        const DATA_JSON_VALUE* pDependencies = Required(
            Entry, "dependencies", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* pAssembly = Required(
			Entry, "assembly", DATA_JSON_TYPE::OBJECT);
        const double AuthoringVersion = nullptr == pAuthoringVersion ?
            0.0 : pAuthoringVersion->Get_Number();
        if (nullptr == pAssetId || pAssetId->Get_String().empty() ||
            nullptr == pAuthoringVersion ||
            AuthoringVersion != std::floor(AuthoringVersion) ||
            AuthoringVersion < EFFECT_AUTHORING_MIN_SUPPORTED_VERSION ||
			AuthoringVersion > EFFECT_AUTHORING_FORMAT_VERSION ||
			nullptr == pContentSha || !Is_LowerHexSha256(pContentSha->Get_String()) ||
			nullptr == pDependencies || nullptr == pAssembly ||
			StagedDirectAuthoredSources.contains(pAssetId->Get_String()) ||
			StagedRuntimeAuthorities.contains(pAssetId->Get_String()) ||
			StagedRuntimeProgramEntries.contains(pAssetId->Get_String()))
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
                (!CEffectDocumentCodec::Is_SafeResourceAssetId(
                    pDependencyId->Get_String()) &&
                 !CEffectDocumentCodec::Is_SafeModelCueAssetId(
                    pDependencyId->Get_String())) ||
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

		EFFECT_ASSEMBLY_DESC Assembly;
		if (!Parse_Assembly(*pAssembly, Assembly, Error) ||
			Assembly.strEffectAssetId != pAssetId->Get_String() ||
			Assembly.iSourceAuthoringVersion !=
				static_cast<uint32_t>(AuthoringVersion) ||
			Assembly.strSourceDocumentFileSha256 != pContentSha->Get_String())
        {
            strOutStatus = "Effect runtime Assembly rejected for " +
                pAssetId->Get_String() + ": " + Error;
            g_strStatus = strOutStatus;
            return false;
        }
		EFFECT_DOCUMENT_DESC Document;
		if (!Compile_Assembly(Assembly, StagedComponents, Document, Error))
		{
			strOutStatus = "Effect runtime Assembly compile failed for " +
				pAssetId->Get_String() + ": " + Error;
			g_strStatus = strOutStatus;
			return false;
		}
        std::vector<std::string> DocumentDependencies;
        CEffectDocumentCodec::Collect_ResourceAssetIds(
            Document, DocumentDependencies);
        if (DocumentDependencies.size() != Dependencies.size())
        {
            strOutStatus = "Effect runtime dependency set does not match its compiled Assembly.";
            g_strStatus = strOutStatus;
            return false;
        }
        for (const std::string& DependencyId : DocumentDependencies)
        {
			if (!Dependencies.contains(DependencyId))
			{
				strOutStatus = "Effect runtime dependency is missing from the manifest: " +
					DependencyId;
				g_strStatus = strOutStatus;
				return false;
			}
		}
		auto CommittedAssembly =
			std::make_shared<const EFFECT_ASSEMBLY_DESC>(std::move(Assembly));
		if (!StagedAssemblies.emplace(pAssetId->Get_String(),
			std::move(CommittedAssembly)).second)
		{
			strOutStatus = "Duplicate Effect Assembly in runtime catalog: " +
				pAssetId->Get_String();
			g_strStatus = strOutStatus;
			return false;
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

	if (bProductCueAdmissionsRequired)
	{
		for (const auto& [EffectId, Source] : StagedDirectAuthoredSources)
		{
			EFFECT_DOCUMENT_DESC Document;
			if (nullptr == Source || !Parse_DirectAuthoredRuntimeDocument(
					EffectId, *Source, Document, Error) ||
				!Staged.emplace(EffectId,
					std::make_shared<const EFFECT_DOCUMENT_DESC>(
						std::move(Document))).second)
			{
				if (Error.empty())
					Error = "duplicate or missing sealed authored document";
				strOutStatus =
					"Effect Product admission preload failed for " + EffectId +
					": " + Error;
				g_strStatus = strOutStatus;
				return false;
			}
		}
	}

	if (!Load_ProductCueAdmissionSidecar(
			Find_ProductCueAdmissionSidecar(Path), Text,
			bProductCueAdmissionsRequired,
			bProductCueAdmissionsRequired ?
				pProductCuePolicySha->Get_String() : std::string{}, Staged,
			StagedRuntimeProgramEntries,
			StagedDirectAuthoredContentShaByEffect,
			StagedProductCueAdmissions,
			StagedProductCueAdmissionsByEffect, Error))
	{
		strOutStatus = "Effect Product cue admission sidecar rejected: " + Error;
		g_strStatus = strOutStatus;
		return false;
	}
	if (bVisualProgramSidecarRequired)
	{
		if (!CEffectVisualProgramCorpusCodec::Load(
				VisualProgramPath, StagedVisualProgramCorpus,
				VisualProgramError) || nullptr == StagedVisualProgramCorpus)
		{
			strOutStatus = "Effect visual-program sidecar rejected: " +
				(VisualProgramError.empty() ? VisualProgramPath.string() :
					VisualProgramError);
			g_strStatus = strOutStatus;
			return false;
		}
	}
	else
	{
		StagedVisualProgramCorpus =
			std::make_shared<const EFFECT_VISUAL_PROGRAM_CORPUS>();
	}

	std::unordered_set<std::string> ReferencedComponentIds;
	for (const auto& [EffectId, Assembly] : StagedAssemblies)
	{
		for (const EFFECT_COMPONENT_CUE_DESC& Cue : Assembly->ComponentCues)
			ReferencedComponentIds.insert(Cue.strComponentAssetId);
	}
	if (ReferencedComponentIds.size() != StagedComponents.size())
	{
		strOutStatus = "Effect runtime catalog contains an unreferenced Component.";
		g_strStatus = strOutStatus;
		return false;
	}

	for (const EFFECT_VISUAL_PROGRAM& Program :
		StagedVisualProgramCorpus->Programs)
	{
		const auto DocumentIterator = Staged.find(Program.strEffectAssetId);
		const auto DirectSourceIterator = StagedDirectAuthoredSources.find(
			Program.strEffectAssetId);
		const auto ReconstructedIterator = StagedRuntimeProgramEntries.find(
			Program.strEffectAssetId);
		const bool_t bMatchesDocument = DocumentIterator != Staged.end();
		const bool_t bMatchesLazyDirect =
			DirectSourceIterator != StagedDirectAuthoredSources.end() &&
			!bMatchesDocument;
		const bool_t bMatchesReconstructed =
			ReconstructedIterator != StagedRuntimeProgramEntries.end();
		if (!bMatchesDocument && !bMatchesLazyDirect && !bMatchesReconstructed)
			continue;
		if ((bMatchesDocument || bMatchesLazyDirect) && bMatchesReconstructed)
		{
			strOutStatus =
				"Effect visual-program target is ambiguous in the staged catalog: " +
				Program.strEffectAssetId;
			g_strStatus = strOutStatus;
			return false;
		}

		std::shared_ptr<const EFFECT_VISUAL_PROGRAM> ProgramAlias(
			StagedVisualProgramCorpus, &Program);
		if (!StagedVisualPrograms.emplace(
				Program.strEffectAssetId, std::move(ProgramAlias)).second)
		{
			strOutStatus = "Duplicate matched Effect visual program: " +
				Program.strEffectAssetId;
			g_strStatus = strOutStatus;
			return false;
		}

		if (bMatchesReconstructed)
		{
			if (Program.eProjectionKind !=
				EFFECT_VISUAL_PROGRAM_PROJECTION_KIND::ADAPTER_PACKET_V1)
			{
				strOutStatus =
					"Reconstructed Effect requires an adapter-packet visual program: " +
					Program.strEffectAssetId;
				g_strStatus = strOutStatus;
				return false;
			}
			continue;
		}
		if (bMatchesLazyDirect)
			continue;

		std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>
			Projection;
		if (!CEffectVisualProgramCorpusCodec::Create_DocumentProjection(
				*StagedVisualProgramCorpus, *DocumentIterator->second,
				Projection, VisualProgramError) || nullptr == Projection ||
			!Projection->Is_Valid() ||
			Projection->Get_EffectAssetId() != Program.strEffectAssetId ||
			Projection->Get_ProjectionKind() != Program.eProjectionKind)
		{
			strOutStatus = "Effect visual-program projection rejected for " +
				Program.strEffectAssetId + ": " +
				(VisualProgramError.empty() ?
					std::string("invalid projection identity") :
					VisualProgramError);
			g_strStatus = strOutStatus;
			return false;
		}
		DocumentIterator->second = Projection->Get_DocumentShared();
		if (!StagedVisualProjections.emplace(
				Program.strEffectAssetId, std::move(Projection)).second)
		{
			strOutStatus = "Duplicate matched Effect visual projection: " +
				Program.strEffectAssetId;
			g_strStatus = strOutStatus;
			return false;
		}
	}

    g_Effects = std::move(Staged);
	g_DirectAuthoredSources = std::move(StagedDirectAuthoredSources);
	g_Assemblies = std::move(StagedAssemblies);
	g_Components = std::move(StagedComponents);
	g_RuntimeAuthorities = std::move(StagedRuntimeAuthorities);
	g_RuntimeProgramEntries = std::move(StagedRuntimeProgramEntries);
	g_pVisualProgramCorpus = std::move(StagedVisualProgramCorpus);
	g_VisualPrograms = std::move(StagedVisualPrograms);
	g_VisualProjections = std::move(StagedVisualProjections);
	g_ProductCueAdmissions = std::move(StagedProductCueAdmissions);
	g_ProductCueAdmissionsByEffect =
		std::move(StagedProductCueAdmissionsByEffect);
	g_iRuntimeRevision = iStagedCatalogRevision;
	size_t iProductApprovedFullCount = 0u;
	size_t iProductApprovedApproximateCount = 0u;
	uint64_t iHardSuppressedElementCount = 0u;
	for (const auto& [Key, Admission] : g_ProductCueAdmissions)
	{
		(void)Key;
		if (Admission->eAdmissionClass ==
			EFFECT_PRODUCT_CUE_ADMISSION_CLASS::PRODUCT_APPROVED_FULL)
		{
			++iProductApprovedFullCount;
		}
		else if (Admission->eAdmissionClass ==
			EFFECT_PRODUCT_CUE_ADMISSION_CLASS::
				PRODUCT_APPROVED_APPROXIMATE)
		{
			++iProductApprovedApproximateCount;
		}
		iHardSuppressedElementCount +=
			Admission->iHardSuppressedElementCount;
	}
	const size_t iLazyDirectDocumentCount = std::count_if(
		g_DirectAuthoredSources.begin(), g_DirectAuthoredSources.end(),
		[](const auto& Entry)
		{
			return !g_Effects.contains(Entry.first);
		});
	g_strStatus = "Loaded " +
		std::to_string(g_Effects.size() + iLazyDirectDocumentCount) +
        " Effect Assemblies, " + std::to_string(g_Components.size()) +
		" Components, and " + std::to_string(g_RuntimeAuthorities.size()) +
		" immutable compiled authorities (" +
		std::to_string(g_RuntimeProgramEntries.size()) +
		" reconstructed typed programs, " +
		std::to_string(g_VisualPrograms.size()) +
		" matched visual programs, and " +
		std::to_string(g_VisualProjections.size()) +
		" committed visual projections; sealed direct documents pending first "
		"use=" + std::to_string(iLazyDirectDocumentCount) +
		"; Product cue admissions: " +
		"PRODUCT_APPROVED_FULL=" +
		std::to_string(iProductApprovedFullCount) + ", " +
		"PRODUCT_APPROVED_APPROXIMATE=" +
		std::to_string(iProductApprovedApproximateCount) + ", " +
		"hard-suppressed elements=" +
		std::to_string(iHardSuppressedElementCount) + ").";
    strOutStatus = g_strStatus;
	StatusGuard.bCommitted = true;
    return true;
}

std::shared_ptr<const Client::CEffectCatalog::RUNTIME_SNAPSHOT>
Client::CEffectCatalog::Capture_Runtime()
{
	auto Snapshot = std::make_shared<RUNTIME_SNAPSHOT>();
	Snapshot->Effects = g_Effects;
	Snapshot->DirectAuthoredSources = g_DirectAuthoredSources;
	Snapshot->Assemblies = g_Assemblies;
	Snapshot->Components = g_Components;
	Snapshot->RuntimeAuthorities = g_RuntimeAuthorities;
	Snapshot->RuntimeProgramEntries = g_RuntimeProgramEntries;
	Snapshot->pVisualProgramCorpus = g_pVisualProgramCorpus;
	Snapshot->VisualPrograms = g_VisualPrograms;
	Snapshot->VisualProjections = g_VisualProjections;
	Snapshot->ProductCueAdmissions = g_ProductCueAdmissions;
	Snapshot->ProductCueAdmissionsByEffect =
		g_ProductCueAdmissionsByEffect;
	Snapshot->iRuntimeRevision = g_iRuntimeRevision;
	Snapshot->strStatus = g_strStatus;
	return Snapshot;
}

bool_t Client::CEffectCatalog::Restore_Runtime(
	std::shared_ptr<const RUNTIME_SNAPSHOT> pSnapshot,
	std::string& strOutStatus)
{
	if (nullptr == pSnapshot)
	{
		strOutStatus = "Effect runtime rollback snapshot is missing.";
		return false;
	}
	g_Effects = pSnapshot->Effects;
	g_DirectAuthoredSources = pSnapshot->DirectAuthoredSources;
	g_Assemblies = pSnapshot->Assemblies;
	g_Components = pSnapshot->Components;
	g_RuntimeAuthorities = pSnapshot->RuntimeAuthorities;
	g_RuntimeProgramEntries = pSnapshot->RuntimeProgramEntries;
	g_pVisualProgramCorpus = pSnapshot->pVisualProgramCorpus;
	g_VisualPrograms = pSnapshot->VisualPrograms;
	g_VisualProjections = pSnapshot->VisualProjections;
	g_ProductCueAdmissions = pSnapshot->ProductCueAdmissions;
	g_ProductCueAdmissionsByEffect =
		pSnapshot->ProductCueAdmissionsByEffect;
	g_iRuntimeRevision = pSnapshot->iRuntimeRevision;
	g_strStatus = pSnapshot->strStatus;
	strOutStatus = "Restored Effect runtime catalog revision " +
		std::to_string(g_iRuntimeRevision) + ".";
	return true;
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

bool_t Client::CEffectCatalog::Is_ProductManagedEffect(
	const std::string& strEffectAssetId)
{
	return g_ProductCueAdmissionsByEffect.contains(strEffectAssetId);
}

bool_t Client::CEffectCatalog::Admit_ProductCue(
	const std::string& strAnimationAssetId,
	const std::string& strClipName,
	const uint32_t iStartMs,
	const std::string& strEffectAssetId,
	std::shared_ptr<const EFFECT_PRODUCT_CUE_ADMISSION_TOKEN>& OutToken,
	std::string& strOutError)
{
	OutToken.reset();
	if (!Is_ProductManagedEffect(strEffectAssetId))
	{
		strOutError.clear();
		return true;
	}
	const std::string Key = Build_ProductCueAdmissionKey(strAnimationAssetId,
		strClipName, iStartMs, strEffectAssetId);
	const auto Iterator = g_ProductCueAdmissions.find(Key);
	if (g_ProductCueAdmissions.end() == Iterator)
	{
		strOutError =
			"Product-managed Effect cue is outside its explicit admission: " +
			strAnimationAssetId + "/" + strClipName + "/" +
			std::to_string(iStartMs) + "/" + strEffectAssetId;
		return false;
	}
	OutToken = std::shared_ptr<const EFFECT_PRODUCT_CUE_ADMISSION_TOKEN>(
		new EFFECT_PRODUCT_CUE_ADMISSION_TOKEN(Key,
			Build_ProductCueAdmissionIdentity(*Iterator->second),
			*Iterator->second));
	strOutError.clear();
	return true;
}

bool_t Client::CEffectCatalog::Admit_ProductSpawn(
	const std::string& strEffectAssetId,
	std::shared_ptr<const EFFECT_PRODUCT_CUE_ADMISSION_TOKEN> pToken,
	std::string& strOutError)
{
	if (!Is_ProductManagedEffect(strEffectAssetId))
	{
		if (nullptr != pToken)
		{
			strOutError =
				"Product Effect spawn token targets an unmanaged or stale Effect.";
			return false;
		}
		strOutError.clear();
		return true;
	}
	if (nullptr == pToken || pToken->m_strEffectAssetId != strEffectAssetId)
	{
		strOutError =
			"Product-managed Effect spawn requires a current admission token: " +
			strEffectAssetId;
		return false;
	}
	const auto Iterator =
		g_ProductCueAdmissions.find(pToken->m_strAdmissionKey);
	if (g_ProductCueAdmissions.end() == Iterator ||
		Build_ProductCueAdmissionIdentity(*Iterator->second) !=
			pToken->m_strAdmissionIdentity ||
		Iterator->second->strCueId != pToken->m_strApprovalId ||
		Iterator->second->strEffectAssetId != strEffectAssetId)
	{
		strOutError =
			"Product-managed Effect spawn token no longer matches its admission: " +
			strEffectAssetId;
		return false;
	}
	strOutError.clear();
	return true;
}

bool_t Client::CEffectCatalog::Validate_ProductCueBinding(
	const std::string& strEffectAssetId,
	std::shared_ptr<const EFFECT_PRODUCT_CUE_ADMISSION_TOKEN> pToken,
	const LostArk::Shared::CHARACTER_CLASS_ID eCharacterClass,
	const std::string& strInputSlot,
	const uint32_t iSkillId,
	const uint32_t iStageIndex,
	std::string& strOutError)
{
	if (nullptr == pToken)
	{
		if (Is_ProductManagedEffect(strEffectAssetId))
		{
			strOutError =
				"Product-managed Effect binding requires an admission token: " +
				strEffectAssetId;
			return false;
		}
		strOutError.clear();
		return true;
	}
	if (!Admit_ProductSpawn(strEffectAssetId, pToken, strOutError))
		return false;

	const auto Iterator =
		g_ProductCueAdmissions.find(pToken->m_strAdmissionKey);
	const char_t* pCharacterClass =
		ProductCueCharacterClassLabel(eCharacterClass);
	if (g_ProductCueAdmissions.end() == Iterator ||
		nullptr == pCharacterClass ||
		Iterator->second->strCharacterClass != pCharacterClass ||
		Iterator->second->strInputSlot != strInputSlot ||
		Iterator->second->iSkillId != iSkillId ||
		Iterator->second->iStageIndex != iStageIndex)
	{
		strOutError =
			"Product cue admission no longer matches its PlayerSkills + skillbindings owner: " +
			strEffectAssetId;
		return false;
	}
	strOutError.clear();
	return true;
}

std::shared_ptr<const Client::EFFECT_PRODUCT_CUE_ADMISSION>
Client::CEffectCatalog::Find_ProductCueAdmission(
	const std::string& strAnimationAssetId,
	const std::string& strClipName,
	const uint32_t iStartMs,
	const std::string& strEffectAssetId)
{
	const auto Iterator = g_ProductCueAdmissions.find(
		Build_ProductCueAdmissionKey(strAnimationAssetId, strClipName,
			iStartMs, strEffectAssetId));
	return g_ProductCueAdmissions.end() == Iterator ? nullptr :
		Iterator->second;
}

std::vector<std::shared_ptr<const Client::EFFECT_PRODUCT_CUE_ADMISSION>>
Client::CEffectCatalog::Get_ProductCueAdmissions(
	const std::string& strEffectAssetId)
{
	const auto Iterator =
		g_ProductCueAdmissionsByEffect.find(strEffectAssetId);
	return g_ProductCueAdmissionsByEffect.end() == Iterator ?
		std::vector<std::shared_ptr<const EFFECT_PRODUCT_CUE_ADMISSION>>{} :
		Iterator->second;
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
	g_ProductCueAdmissions.clear();
	g_ProductCueAdmissionsByEffect.clear();
    g_strStatus = "Effect catalog cleared.";
}
