#include "Effect_Catalog.h"

#include "DataJson.h"
#include "Effect_DocumentCodec.h"
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
    std::map<std::string,
        std::shared_ptr<const Client::EFFECT_DOCUMENT_DESC>,
        std::less<>> g_Effects;
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
			"a85b8b41afb2f2a51bceafa55d06bf0937b1a245";
		constexpr std::string_view BUILDER_TREE_ID =
			"384ed35ca808ab9a71a4edb703ca4d9121b48c18";
		constexpr std::string_view CANDIDATE_BLOB_ID =
			"345ab15bbb76648a650eaa854f18c4cd63cb1556";
		constexpr std::string_view RESOURCE_BINDING_HASH =
			"df15009e41b6c1fe9161af873b96dfc428771944786c14f9435f7c0ffa4d869c";
		constexpr std::string_view INPUT_ARTIFACTS_ORDERED_SHA256 =
			"938dbd9573ca3a5784675ba9d412b9dc3c12a7431a06c70e37d8c9bf2e614eaa";
		constexpr std::string_view RECONSTRUCTED_LINK_SHA256 =
			"74175fe1e41b22ae593a9d1ff92027606bc0b31d62d17927ef6ac5673dd4a7a2";
		constexpr std::string_view RECEIPT_SELF_SHA256 =
			"5c91709f2f0ec855c54c94e6dad5bcd7ed048c6133ca9a9af7d4873f20da1bd3";
		constexpr std::string_view PUBLISH_RECEIPT_SHA256 =
			"92c883f78d88018a50d8dec09eb6fb155974bec4b3756a796b3499fc2f839d94";
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
		if (!bHistoricalOuter10 && !bRenderResourceOuter13)
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
			iCandidateByteCount != 15'072'141u ||
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
			"5c207e04952971adb553249540e336ba3ad065719e438a9892c6850d2c989c4e",
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
		if (bRenderResourceOuter13 &&
			!Parse_ReconstructedRenderResourceExtension(
				Value, Staged.Identity, Staged.pRenderResourceAuthority,
				strOutError))
		{
			return false;
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
		if (!Value.Is_Array() || Value.Get_Array().size() != 48u)
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
			if (!Read_StringExact(Row, "resourceAuthorityId",
					Resource.strResourceAuthorityId) ||
				!Is_StableId(Resource.strResourceAuthorityId) ||
				!Read_U32(Row, "order", Resource.iOrder) ||
				Resource.iOrder != Index ||
				!Read_StringExact(Row, "runtimeAssetId",
					Resource.strRuntimeAssetId) ||
				!CEffectDocumentCodec::Is_SafeResourceAssetId(
					Resource.strRuntimeAssetId) ||
				!Read_StringArrayExact(Row, "candidateBindingIds",
					CandidateBindingIds) || CandidateBindingIds.empty() ||
				!Read_StringArrayExact(Row, "candidateBindingRowSha256",
					CandidateBindingRowHashes, true) ||
				!Read_U32(Row, "candidateBindingCount", iCandidateBindingCount) ||
				iCandidateBindingCount != CandidateBindingIds.size() ||
				iCandidateBindingCount != CandidateBindingRowHashes.size() ||
				!Read_U64Exact(Row, "byteCount", Resource.iByteCount) ||
				Resource.iByteCount <= 128u ||
				!Read_ShaExact(Row, "rawSha256", Resource.strRawSha256) ||
				nullptr == DdsHeader ||
				!Validate_DdsHeader(*DdsHeader, Resource.iByteCount) ||
				!Read_StringExact(Row, "actualCompressedFormatClassification",
					Classification) ||
				!Read_StringExact(Row, "colorSpacePolicy", ColorSpace) ||
				nullptr == Srv || !Parse_DdsSrvIdentity(*Srv, Resource.ExpectedSrv) ||
				ColorSpace != Resource.ExpectedSrv.strColorSpace ||
				!Read_StringExact(Row, "resourceIdentityBasis", IdentityBasis) ||
				IdentityBasis != "CANONICAL_MAIN_RESOURCES_BYTE_EXACT_DDS" ||
				!Read_BooleanExact(Row, "absolutePathRecorded", false) ||
				!Read_BooleanExact(Row, "actionTimeIoAllowed", false) ||
				!Validate_FailClosedAuthorityRow(Row) ||
				!Read_ShaExact(Row, "rowSha256", Resource.strRowSha256) ||
				!Staged.emplace(Resource.strResourceAuthorityId,
					std::move(Resource)).second)
			{
				strOutError = "Render-resource textureResources row is invalid.";
				return false;
			}
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
		if (!Value.Is_Array() || Value.Get_Array().size() != 72u)
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
			"path", "publisherIntegrationCommitId", "publisherOriginalCommitId",
			"publisherTreeId", "trackedBlobId", "currentCheckoutByteCount",
			"currentCheckoutRawSha256", "currentCheckoutCarriageReturnCount",
			"schema", "formatVersion", "componentCount", "effectCount",
			"artist31470EffectIndex", "outerKeyCount", "outerKeyOrder",
			"outerCanonicalSha256", "linkKeyCount", "linkKeyOrder",
			"linkCanonicalSha256", "receiptKeyCount", "receiptKeyOrder",
			"receiptSelfSha256", "outerPublishReceiptSha256",
			"toolDependencyCount", "toolDependencies", "publicValidator",
			"payloadKind", "effectAssetId", "artifactRevision",
			"compilerRevision", "sourceExact", "runtimeExecutionAdmission",
			"productAdmission" }))
		{
			return false;
		}
		uint64_t iRawBytes = 0u;
		uint32_t iProgramVersion = 0u;
		uint32_t iInputArtifactCount = 0u;
		std::string RawSha;
		std::string ProgramId;
		std::string ProgramSha;
		if (!Read_U64Exact(*Program, "rawByteCount", iRawBytes) ||
			iRawBytes != 15'072'141u ||
			!Read_ShaExact(*Program, "rawSha256", RawSha) || RawSha !=
				"72e417747dee14dd0a3be5ffd64f69f904bd696ef1acc049037fc81f38779849" ||
			!Read_StringExact(*Program, "programId", ProgramId) || ProgramId !=
				"effect.artist.skill.31470.reconstructed-approved-v1" ||
			!Read_U32(*Program, "programVersion", iProgramVersion) ||
			iProgramVersion != 1u ||
			!Read_ShaExact(*Program, "programSha256", ProgramSha) || ProgramSha !=
				"618d5684c94fffa2c21ec0ee911e564fd0f6a1d35fc92843d8efcaeeadd55b4b" ||
			!Read_U32(*Program, "inputArtifactCount", iInputArtifactCount) ||
			iInputArtifactCount != 13u ||
			!Validate_FailClosedAuthorityRow(*Program))
		{
			return false;
		}
		uint32_t iFormatVersion = 0u;
		uint32_t iOuterKeyCount = 0u;
		uint32_t iLinkKeyCount = 0u;
		uint32_t iReceiptKeyCount = 0u;
		uint32_t iToolCount = 0u;
		std::string Schema;
		std::string OuterSha;
		std::string LinkSha;
		std::string ReceiptSelfSha;
		std::string OuterReceiptSha;
		std::string PayloadKind;
		std::string EffectId;
		std::string CompilerRevision;
		return Read_StringExact(*Publisher, "schema", Schema) &&
			Schema == "lostark.effect-runtime-catalog" &&
			Read_U32(*Publisher, "formatVersion", iFormatVersion) &&
			iFormatVersion == 3u &&
			Read_U32(*Publisher, "outerKeyCount", iOuterKeyCount) &&
			iOuterKeyCount == 10u &&
			Read_ShaExact(*Publisher, "outerCanonicalSha256", OuterSha) &&
			OuterSha ==
				"e9694f000a50a426386afd6ff8f65b4a2a5fcafe9883860efff9103e1fff82d2" &&
			Read_U32(*Publisher, "linkKeyCount", iLinkKeyCount) &&
			iLinkKeyCount == 16u &&
			Read_ShaExact(*Publisher, "linkCanonicalSha256", LinkSha) &&
			LinkSha ==
				"74175fe1e41b22ae593a9d1ff92027606bc0b31d62d17927ef6ac5673dd4a7a2" &&
			Read_U32(*Publisher, "receiptKeyCount", iReceiptKeyCount) &&
			iReceiptKeyCount == 25u &&
			Read_ShaExact(*Publisher, "receiptSelfSha256", ReceiptSelfSha) &&
			ReceiptSelfSha ==
				"5c91709f2f0ec855c54c94e6dad5bcd7ed048c6133ca9a9af7d4873f20da1bd3" &&
			Read_ShaExact(*Publisher, "outerPublishReceiptSha256",
				OuterReceiptSha) && OuterReceiptSha ==
				"92c883f78d88018a50d8dec09eb6fb155974bec4b3756a796b3499fc2f839d94" &&
			Read_U32(*Publisher, "toolDependencyCount", iToolCount) &&
			iToolCount == 3u &&
			Read_StringExact(*Publisher, "payloadKind", PayloadKind) &&
			PayloadKind == "IMMUTABLE_RECONSTRUCTED_RUNTIME_PROGRAM" &&
			Read_StringExact(*Publisher, "effectAssetId", EffectId) &&
			EffectId == "effect.artist.skill.31470" &&
			Read_StringExact(*Publisher, "compilerRevision", CompilerRevision) &&
			CompilerRevision ==
				"artist31470.reconstructed-runtime-program-link-v1" &&
			Read_BooleanExact(*Publisher, "sourceExact", false) &&
			Read_BooleanExact(*Publisher, "runtimeExecutionAdmission", false) &&
			Read_BooleanExact(*Publisher, "productAdmission", false);
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
			"bc5cd1accbbe3c628993a47093dc829eec6f050ab8467fca82f6b7bcf2dfe0ff";
		constexpr std::string_view SIDECAR_RECEIPT_SHA256 =
			"bd05c7dca6bdef205b27c208644be19bb94bdbef2e05712bfc49b9b946d8f28a";
		constexpr std::string_view SIDECAR_DECISION_SHA256 =
			"4efa9ea724df336a5f3af719e24211b7206fe21dfd97becc630f88c5dbd9b412";
		if (Text.size() != 746'788u ||
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
			iTextureResourceCount != 48u ||
			!Read_U32(*Summary, "textureBindingCount", iTextureBindingCount) ||
			iTextureBindingCount != 72u ||
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
			"618d5684c94fffa2c21ec0ee911e564fd0f6a1d35fc92843d8efcaeeadd55b4b";
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
			"618d5684c94fffa2c21ec0ee911e564fd0f6a1d35fc92843d8efcaeeadd55b4b";
		constexpr std::string_view BASE_ENTRY_SHA256 =
			"e9694f000a50a426386afd6ff8f65b4a2a5fcafe9883860efff9103e1fff82d2";
		constexpr std::string_view BASE_LINK_SHA256 =
			"74175fe1e41b22ae593a9d1ff92027606bc0b31d62d17927ef6ac5673dd4a7a2";
		constexpr std::string_view BASE_RECEIPT_SHA256 =
			"92c883f78d88018a50d8dec09eb6fb155974bec4b3756a796b3499fc2f839d94";
		constexpr std::string_view SIDECAR_SCHEMA =
			"lostark.artist-31470-reconstructed-render-resource-authority-receipt";
		constexpr std::string_view SIDECAR_AUTHORITY_ID =
			"ARTIST_31470_RECONSTRUCTED_RENDER_RESOURCE_AUTHORITY_V1";
		constexpr std::string_view SIDECAR_DECISION_SHA256 =
			"4efa9ea724df336a5f3af719e24211b7206fe21dfd97becc630f88c5dbd9b412";
		constexpr std::string_view SIDECAR_RECEIPT_SHA256 =
			"bd05c7dca6bdef205b27c208644be19bb94bdbef2e05712bfc49b9b946d8f28a";
		constexpr std::string_view SIDECAR_RAW_SHA256 =
			"bc5cd1accbbe3c628993a47093dc829eec6f050ab8467fca82f6b7bcf2dfe0ff";
		constexpr std::string_view AUTHORITY_LINK_SHA256 =
			"8a856dd473d49ee255f613c2e25395668c7209e434f7e3a869525a10f4a34c4e";
		constexpr std::string_view RECEIPT_SELF_SHA256 =
			"3a5ec8cd44173dde89addfb078303cf8d208be5e45bb28f557f0ca0028811687";
		constexpr std::string_view PUBLISH_RECEIPT_SHA256 =
			"dc5682f98b359fe114fbeab6dfd04591769fb5a2607f2872fdef189f392d2455";

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
			iSidecarByteCount != 746'788u ||
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
			"74473d8be1e5930a0809740f1d8240216d4a5478acb9a8ff75001ce0335ceaef",
			"148d13df44da8c2fbf3378648d92ee83651a1f97cd5b6827a4b411cce78cfb95",
			"2858a8c8f34754435b7daafe61679c0d7b965744af67f34222c31b0dd4ab801d" };
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
}

struct Client::CEffectCatalog::RUNTIME_SNAPSHOT final
{
	std::map<std::string,
		std::shared_ptr<const EFFECT_DOCUMENT_DESC>, std::less<>> Effects;
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
    const std::string Text{
        std::istreambuf_iterator<char>(Input),
        std::istreambuf_iterator<char>() };
    DATA_JSON_VALUE Root;
    std::string Error;
	DATA_JSON_PARSE_LIMITS RuntimeCatalogLimits;
	RuntimeCatalogLimits.iMaximumBytes = 64u * 1024u * 1024u;
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
	const bool_t bIntegralCatalogVersion = nullptr != pVersion &&
		!pVersion->Was_FloatingPointToken();
    const bool_t bLegacyCatalog = bIntegralCatalogVersion &&
		2.0 == pVersion->Get_Number();
	const bool_t bDerivedCatalog = bIntegralCatalogVersion &&
		3.0 == pVersion->Get_Number() && nullptr != pSchema &&
		pSchema->Get_String() == "lostark.effect-runtime-catalog" &&
		Root.Get_Object().size() == 4u &&
		Root.Get_Object().contains("schema") &&
		Root.Get_Object().contains("formatVersion") &&
		Root.Get_Object().contains("components") &&
		Root.Get_Object().contains("effects");
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
					std::move(Parsed.pRenderResourceAuthority)));
			StagedRuntimeProgramEntries.emplace(
				pAssetId->Get_String(), std::move(ProgramEntry));
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

    g_Effects = std::move(Staged);
	g_Assemblies = std::move(StagedAssemblies);
	g_Components = std::move(StagedComponents);
	g_RuntimeAuthorities = std::move(StagedRuntimeAuthorities);
	g_RuntimeProgramEntries = std::move(StagedRuntimeProgramEntries);
	g_iRuntimeRevision = iStagedCatalogRevision;
    g_strStatus = "Loaded " + std::to_string(g_Effects.size()) +
        " Effect Assemblies, " + std::to_string(g_Components.size()) +
		" Components, and " + std::to_string(g_RuntimeAuthorities.size()) +
		" immutable compiled authorities (" +
		std::to_string(g_RuntimeProgramEntries.size()) +
		" reconstructed typed programs).";
    strOutStatus = g_strStatus;
	StatusGuard.bCommitted = true;
    return true;
}

std::shared_ptr<const Client::CEffectCatalog::RUNTIME_SNAPSHOT>
Client::CEffectCatalog::Capture_Runtime()
{
	auto Snapshot = std::make_shared<RUNTIME_SNAPSHOT>();
	Snapshot->Effects = g_Effects;
	Snapshot->Assemblies = g_Assemblies;
	Snapshot->Components = g_Components;
	Snapshot->RuntimeAuthorities = g_RuntimeAuthorities;
	Snapshot->RuntimeProgramEntries = g_RuntimeProgramEntries;
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
	g_Assemblies = pSnapshot->Assemblies;
	g_Components = pSnapshot->Components;
	g_RuntimeAuthorities = pSnapshot->RuntimeAuthorities;
	g_RuntimeProgramEntries = pSnapshot->RuntimeProgramEntries;
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
    return g_Effects.contains(strEffectAssetId);
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
    std::vector<std::string> Result;
    Result.reserve(g_Effects.size());
    for (const auto& [AssetId, Document] : g_Effects)
	{
		if (!Is_ReconstructedRuntimeProgramAssetId(AssetId))
			Result.push_back(AssetId);
	}
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
	g_Assemblies.clear();
	g_Components.clear();
	g_RuntimeAuthorities.clear();
	g_RuntimeProgramEntries.clear();
    g_strStatus = "Effect catalog cleared.";
}
