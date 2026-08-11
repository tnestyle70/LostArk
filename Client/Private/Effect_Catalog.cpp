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
	};

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
		if (!Has_ExactOrderedKeys(Value, {
			"payloadKind", "effectAssetId", "artifactRevision",
			"compilerRevision", "sourceExact", "runtimeExecutionAdmission",
			"productAdmission", "publishReceiptSha256", "publishReceipt",
			"reconstructedRuntimeProgram" }))
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
		Staged.pProgram = std::move(Program);
		OutEntry = std::move(Staged);
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
					std::move(Parsed.Identity), std::move(Parsed.pProgram)));
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
