#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <WinSock2.h>
#include <Windows.h>

#include "FourClassTrackAAuthoredMaterializer.h"

#include "BinaryAsset/ModelDecoderRegistry.h"
#include "Client_Defines.h"
#include "DataJson.h"
#include "Effect_AuthoringDocument.h"
#include "Effect_DocumentCodec.h"
#include "Effect_MaterialTemplate.h"
#include "Effect_Object.h"
#include "Effect_RuntimeAuthority.h"
#include "GameInstance.h"

#include <DirectXMath.h>
#include <d3d11.h>
#include <wrl/client.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <initializer_list>
#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <sstream>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace
{
	using Client::CDataJson;
	using Client::CEffectDocumentCodec;
	using Client::CEffectRuntimeAuthorityCodec;
	using Client::DATA_JSON_PARSE_LIMITS;
	using Client::DATA_JSON_TYPE;
	using Client::DATA_JSON_VALUE;
	using Client::EFFECT_DOCUMENT_DESC;
	using Client::EFFECT_ELEMENT_DESC;
	using Client::EFFECT_GENERIC_AUTHORED_ELEMENT_IMPORT_REQUEST;
	using Client::EFFECT_GENERIC_AUTHORED_ELEMENT_REIMPORT_REQUEST;

	constexpr std::size_t EXPECTED_STAGE_COUNT = 13u;
	constexpr std::size_t EXPECTED_TRACK_A_STAGE_COUNT = 12u;
	constexpr std::size_t EXPECTED_ELEMENT_PLAN_COUNT = 79u;
	constexpr std::size_t EXPECTED_GENERIC_PLAN_COUNT = 71u;
	constexpr std::size_t EXPECTED_ADAPTER_PLAN_COUNT = 4u;
	constexpr std::size_t EXPECTED_SUPPLEMENTAL_PLAN_COUNT = 4u;
	constexpr std::size_t EXPECTED_ADMITTED_MATERIAL_COUNT = 16u;
	constexpr std::size_t EXPECTED_FAIL_CLOSED_MATERIAL_COUNT = 63u;
	constexpr std::size_t EXPECTED_LEGACY_STARTER_SKILL_COUNT = 48u;
	constexpr std::size_t EXPECTED_LEGACY_STARTER_STAGE_COUNT = 61u;
	constexpr std::size_t EXPECTED_LEGACY_STARTER_EFFECT_STAGE_COUNT = 60u;
	constexpr std::size_t EXPECTED_LEGACY_STARTER_SILENT_STAGE_COUNT = 1u;
	constexpr std::size_t EXPECTED_LEGACY_STARTER_OCCURRENCE_COUNT = 100u;
	constexpr std::size_t EXPECTED_LEGACY_STARTER_VISUAL_COUNT = 89u;
	constexpr std::size_t EXPECTED_LEGACY_STARTER_SILENT_COUNT = 11u;
	constexpr std::size_t EXPECTED_LEGACY_STARTER_CANDIDATE_COUNT = 88u;
	constexpr std::size_t EXPECTED_LEGACY_ACTIVE_REFERENCE_COUNT = 85u;
	constexpr std::size_t EXPECTED_LEGACY_ORPHAN_REFERENCE_COUNT = 3u;
	constexpr std::size_t EXPECTED_FULL_SKILL_COUNT = 51u;
	constexpr std::size_t EXPECTED_FULL_STAGE_COUNT = 74u;
	constexpr std::size_t EXPECTED_FULL_CLIP_OCCURRENCE_COUNT = 113u;
	constexpr std::size_t EXPECTED_FULL_VISUAL_CLIP_COUNT = 102u;
	constexpr std::size_t EXPECTED_FULL_SILENT_CLIP_COUNT = 11u;
	constexpr std::size_t EXPECTED_TOTAL_CANDIDATE_COUNT = 101u;
	constexpr std::size_t EXPECTED_RECIPE_RUNTIME_FAMILY_COUNT = 8u;
	constexpr std::size_t EXPECTED_VALTAN_420633_PORTABLE_PARTICLE_COUNT = 3u;
	constexpr std::size_t EXPECTED_VALTAN_420633_FAIL_CLOSED_COUNT = 6u;
	constexpr std::size_t EXPECTED_CHARACTER_GHOST_CUE_COUNT = 72u;
	constexpr std::size_t EXPECTED_CHARACTER_GHOST_TARGET_COUNT = 29u;
	constexpr std::size_t ORDINARY_HARNESS_JSON_MAXIMUM_BYTES =
		16u * 1024u * 1024u;
	constexpr std::size_t IMPORTED_SOURCE_JSON_MAXIMUM_BYTES =
		32u * 1024u * 1024u;
	static_assert(ORDINARY_HARNESS_JSON_MAXIMUM_BYTES == 16'777'216u);
	static_assert(IMPORTED_SOURCE_JSON_MAXIMUM_BYTES == 33'554'432u);
	static_assert(IMPORTED_SOURCE_JSON_MAXIMUM_BYTES >
		ORDINARY_HARNESS_JSON_MAXIMUM_BYTES);
	constexpr std::string_view DIMENSION_MASTER_T_UNIFIED_EFFECT_ID =
		"effect.dimensionmaster.skill.2050500.unified";

	enum class CARRIER_DISPOSITION : uint8_t
	{
		GENERIC_PARTICLE_IMPORT_CANDIDATE,
		FAMILY_ADAPTER_REQUIRED,
		SUPPLEMENTAL_ADAPTER_PRESERVE
	};

	enum class MATERIAL_DISPOSITION : uint8_t
	{
		TYPED_EXECUTION,
		ADMITTED_SOURCE_PROFILE,
		FAIL_CLOSED
	};

	struct FILE_IDENTITY final
	{
		std::string strPath;
		std::string strRawSha256;
		std::string strCanonicalJsonSha256;
	};

	struct CACHED_JSON_ARTIFACT final
	{
		std::string strBytes;
		DATA_JSON_VALUE Root;
	};

	struct MATERIALIZER_STAGE final
	{
		std::string strStageKey;
		std::string strMode;
		std::string strCharacterClass;
		std::string strAnimationAssetId;
		uint32_t iSkillId = 0u;
		uint32_t iStageIndex = 0u;
		std::string strStageId;
		std::string strClip;
		FILE_IDENTITY ImportedDocument;
		std::string strImportedEffectAssetId;
		FILE_IDENTITY SelectionArtifact;
		std::string strSelectionRecordId;
		std::string strSelectionRecordSha256;
		std::string strTargetPath;
		std::string strTargetEffectAssetId;
		std::string strCandidateBaselinePolicy;
		std::optional<std::string> strCandidateRawSha256;
		std::optional<std::string> strCandidateCanonicalJsonSha256;
		std::string strLegacyBaselinePath;
		std::string strLegacyEffectAssetId;
		std::string strLegacyRawSha256;
		std::string strLegacyCanonicalJsonSha256;
		std::string strBlueprintKind;
		std::string strBlueprintCanonicalJsonSha256;
		std::optional<std::string> strBlueprintTypedCodecSha256;
		std::vector<std::string> ElementPlanIds;
	};

	struct MATERIALIZER_ELEMENT_PLAN final
	{
		std::string strPlanId;
		std::string strStageKey;
		bool bImportedSource = false;
		std::string strSourceDocumentPath;
		std::string strSourceDocumentRawSha256;
		std::string strSourceEffectAssetId;
		std::string strSourceElementId;
		std::string strSourceElementCanonicalSha256;
		std::string strSourceRecipeCanonicalSha256;
		std::string strSourceDetailCanonicalSha256;
		std::string strSourceAttachmentCanonicalSha256;
		std::string strSupplementalSourceRecordId;
		std::string strSupplementalSourceRecordSha256;
		std::string strSourceEventId;
		std::string strTargetDocumentPath;
		std::string strTargetEffectAssetId;
		std::string strTargetElementId;
		std::string strTargetGroupId;
		std::string strTargetDisplayName;
		std::string strBlueprintElementCanonicalSha256;
		std::optional<std::string> strBaselineElementCanonicalSha256;
		std::string strTargetDetailCanonicalSha256;
		std::string strSelectionRecordId;
		std::string strSelectionRecordSha256;
		std::string strFamily;
		CARRIER_DISPOSITION eCarrier =
			CARRIER_DISPOSITION::GENERIC_PARTICLE_IMPORT_CANDIDATE;
		MATERIAL_DISPOSITION eMaterial = MATERIAL_DISPOSITION::FAIL_CLOSED;
	};

	struct LEGACY_STARTER_CLIP final
	{
		std::string strClip;
		uint32_t iStageClipIndex = 0u;
		bool bVisualBearing = false;
		std::optional<std::string> strLegacyEffectAssetId;
		std::optional<std::string> strCandidateEffectAssetId;
	};

	struct LEGACY_STARTER_STAGE final
	{
		std::string strStageKey;
		std::string strCharacterClass;
		std::string strAnimationAssetId;
		uint32_t iSkillId = 0u;
		uint32_t iStageIndex = 0u;
		std::string strStageId;
		bool bEffectBearing = false;
		FILE_IDENTITY SourceManifest;
		std::string strRolloutStageCanonicalSha256;
		std::vector<LEGACY_STARTER_CLIP> Clips;
		std::vector<std::string> CandidateEffectAssetIds;
	};

	struct LEGACY_STARTER_CANDIDATE final
	{
		std::string strCandidateKey;
		std::string strCharacterClass;
		std::string strAnimationAssetId;
		uint32_t iSkillId = 0u;
		uint32_t iStageIndex = 0u;
		uint32_t iStageClipIndex = 0u;
		std::string strClip;
		FILE_IDENTITY LegacyBaseline;
		std::string strLegacyEffectAssetId;
		uint32_t iLegacyAuthoringVersion = 0u;
		std::string strRolloutDocumentFileSha256;
		std::string strRolloutHashDisposition;
		FILE_IDENTITY StarterSource;
		std::string strStarterSourceKind;
		std::string strStarterSourceEffectAssetId;
		uint32_t iStarterSourceAuthoringVersion = 0u;
		std::string strTargetPath;
		std::string strTargetEffectAssetId;
		std::string strCandidateBaselinePolicy;
		std::optional<std::string> strCandidateRawSha256;
		std::optional<std::string> strCandidateCanonicalJsonSha256;
		std::optional<uint32_t> iCandidateAuthoringVersion;
		std::string strCatalogPath;
		std::string strCatalogEntryCanonicalSha256;
		std::string strAnimeventPath;
		std::string strAnimeventRawSha256;
		uint32_t iActiveReferenceCount = 0u;
		bool bOrphanedCatalogReference = false;
		std::string strRolloutRecordCanonicalSha256;
	};

	struct MATERIALIZER_BATCH final
	{
		std::filesystem::path RepositoryRoot;
		std::map<std::string, std::shared_ptr<const CACHED_JSON_ARTIFACT>,
			std::less<>> Artifacts;
		std::vector<MATERIALIZER_STAGE> Stages;
		std::vector<MATERIALIZER_ELEMENT_PLAN> Plans;
		std::vector<LEGACY_STARTER_STAGE> LegacyStarterStages;
		std::vector<LEGACY_STARTER_CANDIDATE> LegacyStarterCandidates;
	};

	struct STAGED_TARGET_FILE final
	{
		std::filesystem::path Path;
		bool bMustNotExist = false;
		std::string strExpectedRawSha256;
		std::string strExpectedCanonicalJsonSha256;
		std::string strCanonicalDocument;
	};

	enum class RESTORATION_RECEIPT_ROW_KIND : uint8_t
	{
		PARTICLE,
		DECAL,
		ANIMATION_TRAIL
	};

	struct RESTORATION_RECEIPT_ROW final
	{
		RESTORATION_RECEIPT_ROW_KIND eKind =
			RESTORATION_RECEIPT_ROW_KIND::PARTICLE;
		std::string strTargetElementId;
		std::string strSourceEffectAssetId;
		std::string strSourceElementId;
		std::string strSourceEventId;
		std::string strSourceRecipeCanonicalSha256;
		std::string strNormalizedRecipeCanonicalSha256;
		std::string strSourceDetailCanonicalSha256;
		std::string strTargetDetailCanonicalSha256;
		std::string strSourceBindingsCanonicalSha256;
		std::string strTargetBindingsCanonicalSha256;
		std::string strRendererShape;
		std::string strDeferredReason;
		std::string strBaseStatus;
		std::vector<std::string> FailClosedReasons;
		bool bPortable = false;
	};

	struct RESTORATION_TARGET_RECEIPT final
	{
		std::string strEffectAssetId;
		std::string strTargetPath;
		std::string strOutputCanonicalSha256;
		std::string strCharacterClass;
		uint32_t iSkillId = 0u;
		uint32_t iStageIndex = 0u;
		uint32_t iStageClipIndex = 0u;
		std::string strClip;
		uint32_t iOutputElementCount = 0u;
		uint32_t iSourceParticleCount = 0u;
		uint32_t iSourceDecalCount = 0u;
		uint32_t iSourceDecalReadyCount = 0u;
		uint32_t iSourceDecalIncompleteCount = 0u;
		uint32_t iSupplementalPreservedCount = 0u;
		uint32_t iPlaceholderTrailExcludedCount = 0u;
		uint32_t iPortableCount = 0u;
		uint32_t iRecipeDeferredCount = 0u;
		uint32_t iDrawableAdmittedCount = 0u;
		uint32_t iPortableFailClosedCount = 0u;
		std::vector<RESTORATION_RECEIPT_ROW> Rows;
	};

	struct RESTORATION_RECEIPT final
	{
		uint32_t iTargetCount = 0u;
		uint32_t iStrictMappedParticleCount = 0u;
		uint32_t iSourceDecalCount = 0u;
		uint32_t iSourceDecalReadyCount = 0u;
		uint32_t iSourceDecalIncompleteCount = 0u;
		uint32_t iOutputElementCount = 0u;
		uint32_t iPortableCount = 0u;
		uint32_t iRecipeDeferredCount = 0u;
		uint32_t iDrawableAdmittedCount = 0u;
		uint32_t iPortableFailClosedCount = 0u;
		uint32_t iSupplementalPreservedCount = 0u;
		uint32_t iPlaceholderTrailExcludedCount = 0u;
		uint32_t iSourceAnimationTrailNotifyCount = 0u;
		uint32_t iSourceAnimationTrailElementCount = 0u;
		uint32_t iCharacterGhostCueCount = 0u;
		uint32_t iCharacterGhostTargetCount = 0u;
		bool bHasAnimationTrailDenominators = false;
		std::map<std::string, RESTORATION_TARGET_RECEIPT, std::less<>> Targets;
	};

	enum class RESTORATION_RUNTIME_DISPOSITION : uint8_t
	{
		FULL,
		AUTHORING_APPROXIMATE,
		HARD_FAIL_CLOSED
	};

	bool Is_RestorationPreviewExecutionTarget(
		const EFFECT_ELEMENT_DESC& Element)
	{
		return Element.bVisible && Client::Is_EffectAuthoringExecutionTarget(
			Element.Material.Execution);
	}

	bool Classify_RestorationRuntimeDisposition(
		const RESTORATION_RECEIPT_ROW& Row,
		const EFFECT_ELEMENT_DESC& Element,
		const std::string_view DocumentId,
		RESTORATION_RUNTIME_DISPOSITION& OutDisposition,
		std::string& OutError)
	{
		const auto& Execution = Element.Material.Execution;
		const bool bReceiptFull = Row.bPortable &&
			Row.FailClosedReasons.empty();
		const bool bExecutionTarget =
			Client::Is_EffectAuthoringExecutionTarget(Execution);
		const bool bRuntimeFull = Element.bVisible && bExecutionTarget &&
			!Execution.bFailClosed && !Execution.bAuthoringApproximate;
		const bool bRuntimeApproximate = Element.bVisible && bExecutionTarget &&
			Execution.bFailClosed && Execution.bAuthoringApproximate &&
			!Execution.bEnabled;
		const bool bRuntimeHard = !Element.bVisible &&
			Execution.bFailClosed && !Execution.bAuthoringApproximate &&
			!bExecutionTarget;

		if (bReceiptFull)
		{
			if (!bRuntimeFull)
			{
				OutError = "Receipt-full Element is not a full authoring/runtime target: " +
					std::string(DocumentId) + "/" + Element.strElementId;
				return false;
			}
			OutDisposition = RESTORATION_RUNTIME_DISPOSITION::FULL;
			return true;
		}

		if (bRuntimeApproximate)
		{
			const bool bParticleCarrier =
				Row.eKind == RESTORATION_RECEIPT_ROW_KIND::PARTICLE &&
				Element.eKind == Client::EFFECT_ELEMENT_KIND::PARTICLE &&
				Element.SourceRecipe.bEnabled &&
				(Element.SourceRecipe.strRendererShape == "mesh" ||
				 Element.SourceRecipe.strRendererShape == "sprite");
			if (!Row.bPortable || Row.FailClosedReasons.empty() || !bParticleCarrier)
			{
				OutError =
					"Authoring-approximate Element has no portable receipt blocker/carrier: " +
					std::string(DocumentId) + "/" + Element.strElementId;
				return false;
			}
			OutDisposition =
				RESTORATION_RUNTIME_DISPOSITION::AUTHORING_APPROXIMATE;
			return true;
		}

		if (!bRuntimeHard)
		{
			OutError = "Non-full Element is neither approximate nor hard fail-closed: " +
				std::string(DocumentId) + "/" + Element.strElementId;
			return false;
		}
		OutDisposition = RESTORATION_RUNTIME_DISPOSITION::HARD_FAIL_CLOSED;
		return true;
	}

	class SCOPED_RUNTIME_RESOURCE_ROOT final
	{
	public:
		bool Initialize(const std::filesystem::path& ResourceRoot,
			std::string& OutError)
		{
			SetLastError(ERROR_SUCCESS);
			const DWORD Required = GetEnvironmentVariableW(
				L"LOSTARK_RESOURCE_ROOT", nullptr, 0u);
			m_bWasDefined = 0u != Required ||
				GetLastError() != ERROR_ENVVAR_NOT_FOUND;
			if (Required > 0u)
			{
				std::vector<wchar_t> Buffer(Required);
				const DWORD Length = GetEnvironmentVariableW(
					L"LOSTARK_RESOURCE_ROOT", Buffer.data(), Required);
				if (0u == Length || Length >= Required)
				{
					OutError = "Unable to preserve LOSTARK_RESOURCE_ROOT.";
					return false;
				}
				m_Original.assign(Buffer.data(), Length);
			}
			std::error_code Error;
			if (!std::filesystem::is_directory(ResourceRoot, Error) || Error ||
				FALSE == SetEnvironmentVariableW(
					L"LOSTARK_RESOURCE_ROOT", ResourceRoot.c_str()))
			{
				OutError = "Repository Client/Bin/Resources root is unavailable.";
				return false;
			}
			m_bInitialized = true;
			return true;
		}

		~SCOPED_RUNTIME_RESOURCE_ROOT()
		{
			if (!m_bInitialized)
				return;
			SetEnvironmentVariableW(L"LOSTARK_RESOURCE_ROOT",
				m_bWasDefined ? m_Original.c_str() : nullptr);
		}

		SCOPED_RUNTIME_RESOURCE_ROOT() = default;
		SCOPED_RUNTIME_RESOURCE_ROOT(const SCOPED_RUNTIME_RESOURCE_ROOT&) = delete;
		SCOPED_RUNTIME_RESOURCE_ROOT& operator=(
			const SCOPED_RUNTIME_RESOURCE_ROOT&) = delete;

	private:
		std::wstring m_Original;
		bool m_bWasDefined = false;
		bool m_bInitialized = false;
	};

	class SCOPED_CURRENT_DIRECTORY final
	{
	public:
		bool Initialize(const std::filesystem::path& Directory,
			std::string& OutError)
		{
			std::error_code Error;
			m_Original = std::filesystem::current_path(Error);
			if (Error || m_Original.empty() ||
				!std::filesystem::is_directory(Directory, Error) || Error)
			{
				OutError = "Ordinary Effect stage working directory is unavailable.";
				return false;
			}
			std::filesystem::current_path(Directory, Error);
			if (Error)
			{
				OutError = "Unable to enter Client/Default for ordinary Effect staging.";
				return false;
			}
			m_bInitialized = true;
			return true;
		}

		~SCOPED_CURRENT_DIRECTORY()
		{
			if (!m_bInitialized)
				return;
			std::error_code Ignore;
			std::filesystem::current_path(m_Original, Ignore);
		}

		SCOPED_CURRENT_DIRECTORY() = default;
		SCOPED_CURRENT_DIRECTORY(const SCOPED_CURRENT_DIRECTORY&) = delete;
		SCOPED_CURRENT_DIRECTORY& operator=(
			const SCOPED_CURRENT_DIRECTORY&) = delete;

	private:
		std::filesystem::path m_Original;
		bool m_bInitialized = false;
	};

	bool Has_ExactKeys(const DATA_JSON_VALUE& Value,
		const std::initializer_list<std::string_view> Keys)
	{
		if (!Value.Is_Object() || Value.Get_Object().size() != Keys.size())
			return false;
		return std::all_of(Keys.begin(), Keys.end(),
			[&Value](const std::string_view Key)
			{
				return Value.Get_Object().contains(Key);
			});
	}

	const DATA_JSON_VALUE* Required(const DATA_JSON_VALUE& Object,
		const std::string_view Name, const DATA_JSON_TYPE Type)
	{
		const DATA_JSON_VALUE* Value = Object.Find(Name);
		return nullptr != Value && Value->Get_Type() == Type ? Value : nullptr;
	}

	bool Is_Sha256(const std::string_view Value)
	{
		return Value.size() == 64u && std::all_of(Value.begin(), Value.end(),
			[](const char Character)
			{
				return (Character >= '0' && Character <= '9') ||
					(Character >= 'a' && Character <= 'f');
			});
	}

	bool Decode_Base64Strict(const std::string_view Encoded,
		std::string& OutDecoded)
	{
		if (Encoded.empty() || 0u != Encoded.size() % 4u ||
			Encoded.size() > 4u * 1024u * 1024u)
		{
			return false;
		}
		const auto Decode = [](const char Character) -> int32_t
		{
			if (Character >= 'A' && Character <= 'Z')
				return Character - 'A';
			if (Character >= 'a' && Character <= 'z')
				return 26 + Character - 'a';
			if (Character >= '0' && Character <= '9')
				return 52 + Character - '0';
			if (Character == '+') return 62;
			if (Character == '/') return 63;
			return -1;
		};
		std::string Decoded;
		Decoded.reserve(Encoded.size() / 4u * 3u);
		for (std::size_t Offset = 0u; Offset < Encoded.size(); Offset += 4u)
		{
			const bool bLast = Offset + 4u == Encoded.size();
			const int32_t A = Decode(Encoded[Offset]);
			const int32_t B = Decode(Encoded[Offset + 1u]);
			const bool bPadC = Encoded[Offset + 2u] == '=';
			const bool bPadD = Encoded[Offset + 3u] == '=';
			const int32_t C = bPadC ? 0 : Decode(Encoded[Offset + 2u]);
			const int32_t D = bPadD ? 0 : Decode(Encoded[Offset + 3u]);
			if (A < 0 || B < 0 || C < 0 || D < 0 ||
				(bPadC && !bPadD) || ((bPadC || bPadD) && !bLast) ||
				(bPadC && 0 != (B & 0x0f)) ||
				(!bPadC && bPadD && 0 != (C & 0x03)))
			{
				return false;
			}
			Decoded.push_back(static_cast<char>((A << 2) | (B >> 4)));
			if (!bPadC)
				Decoded.push_back(static_cast<char>((B << 4) | (C >> 2)));
			if (!bPadD)
				Decoded.push_back(static_cast<char>((C << 6) | D));
		}
		OutDecoded = std::move(Decoded);
		return true;
	}

	bool Is_StableId(const std::string_view Value)
	{
		return !Value.empty() && Value.size() <= 128u &&
			std::all_of(Value.begin(), Value.end(), [](const char Character)
			{
				return (Character >= 'a' && Character <= 'z') ||
					(Character >= 'A' && Character <= 'Z') ||
					(Character >= '0' && Character <= '9') ||
					Character == '_' || Character == '-' || Character == '.';
			});
	}

	bool Read_String(const DATA_JSON_VALUE& Object,
		const std::string_view Name, std::string& Out,
		const bool bAllowEmpty = false, const std::size_t Maximum = 4096u)
	{
		const DATA_JSON_VALUE* Value = Required(Object, Name,
			DATA_JSON_TYPE::STRING);
		if (nullptr == Value || Value->Get_String().size() > Maximum ||
			(!bAllowEmpty && Value->Get_String().empty()))
		{
			return false;
		}
		Out = Value->Get_String();
		return true;
	}

	bool Read_U32(const DATA_JSON_VALUE& Object,
		const std::string_view Name, uint32_t& Out)
	{
		const DATA_JSON_VALUE* Value = Required(Object, Name,
			DATA_JSON_TYPE::NUMBER);
		if (nullptr == Value || Value->Was_FloatingPointToken() ||
			!std::isfinite(Value->Get_Number()) || Value->Get_Number() < 0.0 ||
			Value->Get_Number() >
				static_cast<double>((std::numeric_limits<uint32_t>::max)()) ||
			std::floor(Value->Get_Number()) != Value->Get_Number())
		{
			return false;
		}
		Out = static_cast<uint32_t>(Value->Get_Number());
		return true;
	}

	bool Read_False(const DATA_JSON_VALUE& Object,
		const std::string_view Name)
	{
		const DATA_JSON_VALUE* Value = Required(Object, Name,
			DATA_JSON_TYPE::BOOLEAN);
		return nullptr != Value && !Value->Get_Boolean();
	}

	bool Read_True(const DATA_JSON_VALUE& Object,
		const std::string_view Name)
	{
		const DATA_JSON_VALUE* Value = Required(Object, Name,
			DATA_JSON_TYPE::BOOLEAN);
		return nullptr != Value && Value->Get_Boolean();
	}

	bool Read_StringArray(const DATA_JSON_VALUE& Value,
		std::vector<std::string>& Out, const std::size_t Maximum = 4096u)
	{
		if (!Value.Is_Array() || Value.Get_Array().size() > Maximum)
			return false;
		std::vector<std::string> Staged;
		std::unordered_set<std::string> Unique;
		Staged.reserve(Value.Get_Array().size());
		for (const DATA_JSON_VALUE& Item : Value.Get_Array())
		{
			if (!Item.Is_String() || !Is_StableId(Item.Get_String()) ||
				!Unique.insert(Item.Get_String()).second)
			{
				return false;
			}
			Staged.push_back(Item.Get_String());
		}
		Out = std::move(Staged);
		return true;
	}

	bool Read_File(const std::filesystem::path& Path, std::string& Out,
		std::string& OutError)
	{
		std::ifstream Input(Path, std::ios::binary);
		if (!Input)
		{
			OutError = "Unable to open file: " + Path.string();
			return false;
		}
		std::string Bytes{ std::istreambuf_iterator<char>(Input),
			std::istreambuf_iterator<char>() };
		if (Input.bad() || Bytes.empty())
		{
			OutError = "Unable to read non-empty file exactly: " + Path.string();
			return false;
		}
		Out = std::move(Bytes);
		return true;
	}

	bool Parse_Json(const std::string_view Bytes, DATA_JSON_VALUE& Out,
		std::string& OutError)
	{
		DATA_JSON_PARSE_LIMITS Limits;
		Limits.iMaximumBytes = ORDINARY_HARNESS_JSON_MAXIMUM_BYTES;
		Limits.iMaximumDepth = 96u;
		Limits.iMaximumValues = 2'000'000u;
		return CDataJson::Parse(Bytes, Out, OutError, Limits);
	}

	bool Parse_LargeImportedSourceJson(const std::string_view Bytes,
		DATA_JSON_VALUE& Out, std::string& OutError)
	{
		DATA_JSON_PARSE_LIMITS Limits;
		Limits.iMaximumBytes = IMPORTED_SOURCE_JSON_MAXIMUM_BYTES;
		Limits.iMaximumDepth = 96u;
		Limits.iMaximumValues = 4'000'000u;
		return CDataJson::Parse(Bytes, Out, OutError, Limits);
	}

	std::string Python_CanonicalNumberToken(const DATA_JSON_VALUE& Value)
	{
		std::string Token =
			CEffectRuntimeAuthorityCodec::Serialize_CanonicalJson(Value);
		if (!Value.Was_FloatingPointToken())
			return Token;

		const bool Negative = !Token.empty() && Token.front() == '-';
		const std::string_view Unsigned = Negative ?
			std::string_view(Token).substr(1u) : std::string_view(Token);
		const std::size_t ExponentOffset = Unsigned.find_first_of("eE");
		std::string Digits;
		int Exponent = 0;
		if (ExponentOffset != std::string_view::npos)
		{
			for (const char Character : Unsigned.substr(0u, ExponentOffset))
			{
				if (Character != '.')
					Digits.push_back(Character);
			}
			const std::string_view ExponentToken =
				Unsigned.substr(ExponentOffset + 1u);
			if (ExponentToken.empty())
				return Token;
			std::size_t Index = 0u;
			bool NegativeExponent = false;
			if (ExponentToken[Index] == '+' || ExponentToken[Index] == '-')
			{
				NegativeExponent = ExponentToken[Index] == '-';
				++Index;
			}
			if (Index == ExponentToken.size())
				return Token;
			for (; Index < ExponentToken.size(); ++Index)
			{
				const char Character = ExponentToken[Index];
				if (Character < '0' || Character > '9')
					return Token;
				Exponent = Exponent * 10 + (Character - '0');
			}
			if (NegativeExponent)
				Exponent = -Exponent;
		}
		else
		{
			const std::size_t DecimalOffset = Unsigned.find('.');
			const std::size_t DecimalPosition = DecimalOffset ==
				std::string_view::npos ? Unsigned.size() : DecimalOffset;
			std::string RawDigits;
			for (const char Character : Unsigned)
			{
				if (Character != '.')
					RawDigits.push_back(Character);
			}
			const std::size_t FirstNonZero = RawDigits.find_first_not_of('0');
			if (FirstNonZero == std::string::npos)
				return Negative ? "-0.0" : "0.0";
			Exponent = static_cast<int>(DecimalPosition) -
				static_cast<int>(FirstNonZero) - 1;
			Digits = RawDigits.substr(FirstNonZero);
		}
		while (Digits.size() > 1u && Digits.back() == '0')
			Digits.pop_back();
		if (Digits.empty())
			return Negative ? "-0.0" : "0.0";

		std::string Result;
		if (Negative)
			Result.push_back('-');
		if (Exponent < -4 || Exponent >= 16)
		{
			Result.push_back(Digits.front());
			if (Digits.size() > 1u)
			{
				Result.push_back('.');
				Result.append(Digits.substr(1u));
			}
			Result.push_back('e');
			Result.push_back(Exponent < 0 ? '-' : '+');
			const int AbsoluteExponent = Exponent < 0 ? -Exponent : Exponent;
			if (AbsoluteExponent < 10)
				Result.push_back('0');
			Result += std::to_string(AbsoluteExponent);
			return Result;
		}

		const int DecimalPosition = Exponent + 1;
		if (DecimalPosition <= 0)
		{
			Result += "0.";
			Result.append(static_cast<std::size_t>(-DecimalPosition), '0');
			Result += Digits;
		}
		else if (static_cast<std::size_t>(DecimalPosition) >= Digits.size())
		{
			Result += Digits;
			Result.append(static_cast<std::size_t>(DecimalPosition) -
				Digits.size(), '0');
			Result += ".0";
		}
		else
		{
			Result.append(Digits.substr(0u,
				static_cast<std::size_t>(DecimalPosition)));
			Result.push_back('.');
			Result.append(Digits.substr(
				static_cast<std::size_t>(DecimalPosition)));
		}
		return Result;
	}

	void Serialize_PythonCanonicalJson(const DATA_JSON_VALUE& Value,
		std::string& Out)
	{
		switch (Value.Get_Type())
		{
		case DATA_JSON_TYPE::NUMBER:
			Out += Python_CanonicalNumberToken(Value);
			return;
		case DATA_JSON_TYPE::ARRAY:
			Out.push_back('[');
			for (std::size_t Index = 0u; Index < Value.Get_Array().size(); ++Index)
			{
				if (Index != 0u)
					Out.push_back(',');
				Serialize_PythonCanonicalJson(Value.Get_Array()[Index], Out);
			}
			Out.push_back(']');
			return;
		case DATA_JSON_TYPE::OBJECT:
			Out.push_back('{');
			{
				std::size_t Index = 0u;
				for (const auto& [Key, Child] : Value.Get_Object())
				{
					if (Index++ != 0u)
						Out.push_back(',');
					Out += CEffectRuntimeAuthorityCodec::Serialize_CanonicalJson(
						DATA_JSON_VALUE::String(Key));
					Out.push_back(':');
					Serialize_PythonCanonicalJson(Child, Out);
				}
			}
			Out.push_back('}');
			return;
		default:
			Out += CEffectRuntimeAuthorityCodec::Serialize_CanonicalJson(Value);
			return;
		}
	}

	std::string Canonical_JsonSha256(const DATA_JSON_VALUE& Value)
	{
		std::string Canonical;
		Serialize_PythonCanonicalJson(Value, Canonical);
		return CEffectRuntimeAuthorityCodec::Compute_Sha256Hex(Canonical);
	}

	bool Verify_SelfHash(const DATA_JSON_VALUE& Root,
		const std::string_view Field, std::string& OutError)
	{
		const DATA_JSON_VALUE* Expected = Required(Root, Field,
			DATA_JSON_TYPE::STRING);
		if (nullptr == Expected || !Is_Sha256(Expected->Get_String()) ||
			!Root.Is_Object())
		{
			OutError = "Batch artifact SHA-256 field is invalid.";
			return false;
		}
		DATA_JSON_VALUE::OBJECT Object = Root.Get_Object();
		Object.erase(std::string(Field));
		std::vector<std::string> Order;
		for (const std::string& Key : Root.Get_ObjectInsertionOrder())
		{
			if (Key != Field)
				Order.push_back(Key);
		}
		if (Object.size() != Order.size() ||
			Canonical_JsonSha256(DATA_JSON_VALUE::Object(
				std::move(Object), std::move(Order))) != Expected->Get_String())
		{
			OutError = "Batch artifact canonical self SHA-256 changed.";
			return false;
		}
		return true;
	}

	bool Verify_ObjectSelfHash(const DATA_JSON_VALUE& Value,
		const std::string_view Field)
	{
		const DATA_JSON_VALUE* Expected = Required(Value, Field,
			DATA_JSON_TYPE::STRING);
		if (nullptr == Expected || !Is_Sha256(Expected->Get_String()) ||
			!Value.Is_Object())
		{
			return false;
		}
		DATA_JSON_VALUE::OBJECT Object = Value.Get_Object();
		Object.erase(std::string(Field));
		std::vector<std::string> Order;
		for (const std::string& Key : Value.Get_ObjectInsertionOrder())
		{
			if (Key != Field)
				Order.push_back(Key);
		}
		return Object.size() == Order.size() &&
			Canonical_JsonSha256(DATA_JSON_VALUE::Object(
				std::move(Object), std::move(Order))) == Expected->Get_String();
	}

	DATA_JSON_VALUE Replace_ObjectField(const DATA_JSON_VALUE& Value,
		const std::string_view Field, DATA_JSON_VALUE Replacement)
	{
		DATA_JSON_VALUE::OBJECT Object = Value.Get_Object();
		Object[std::string(Field)] = std::move(Replacement);
		return DATA_JSON_VALUE::Object(std::move(Object),
			Value.Get_ObjectInsertionOrder());
	}

	bool Parse_SanitizedAuthoringDocument(const DATA_JSON_VALUE& RawDocument,
		const std::unordered_set<std::string>& KeepSourceProfiles,
		EFFECT_DOCUMENT_DESC& OutDocument, std::string& OutError)
	{
		const DATA_JSON_VALUE* Elements = Required(RawDocument, "elements",
			DATA_JSON_TYPE::ARRAY);
		if (!RawDocument.Is_Object() || nullptr == Elements)
		{
			OutError = "Authoring document has no Element array.";
			return false;
		}
		DATA_JSON_VALUE::ARRAY SanitizedElements;
		SanitizedElements.reserve(Elements->Get_Array().size());
		for (const DATA_JSON_VALUE& Element : Elements->Get_Array())
		{
			const DATA_JSON_VALUE* Id = Required(Element, "id",
				DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* Material = Required(Element, "material",
				DATA_JSON_TYPE::OBJECT);
			if (nullptr == Id || nullptr == Material)
			{
				OutError = "Authoring Element identity/material is invalid.";
				return false;
			}
			if (KeepSourceProfiles.contains(Id->Get_String()))
			{
				SanitizedElements.push_back(Element);
				continue;
			}
			DATA_JSON_VALUE::OBJECT DisabledFields;
			DisabledFields.emplace("enabled", DATA_JSON_VALUE::Boolean(false));
			DATA_JSON_VALUE DisabledProfile = DATA_JSON_VALUE::Object(
				std::move(DisabledFields), { "enabled" });
			DATA_JSON_VALUE SanitizedMaterial = Replace_ObjectField(
				*Material, "sourceProfile", std::move(DisabledProfile));
			SanitizedMaterial = Replace_ObjectField(SanitizedMaterial,
				"templateId", DATA_JSON_VALUE::String("effect.standard"));
			SanitizedElements.push_back(Replace_ObjectField(Element, "material",
				SanitizedMaterial));
		}
		const DATA_JSON_VALUE Sanitized = Replace_ObjectField(RawDocument,
			"elements", DATA_JSON_VALUE::Array(std::move(SanitizedElements)));
		const std::string Json =
			CEffectRuntimeAuthorityCodec::Serialize_CanonicalJson(Sanitized);
		return CEffectDocumentCodec::Parse(Json, OutDocument, OutError);
	}

	bool Is_SafeRelativePath(const std::string_view Text)
	{
		if (Text.empty() || Text.size() > 512u ||
			Text.find('\\') != std::string_view::npos)
		{
			return false;
		}
		const std::filesystem::path Path(Text);
		if (Path.is_absolute() || Path.has_root_name() ||
			Path.lexically_normal().generic_string() != Text)
		{
			return false;
		}
		return std::none_of(Path.begin(), Path.end(),
			[](const std::filesystem::path& Part)
			{
				return Part == L".." || Part == L".";
			});
	}

	bool Resolve_RepositoryRoot(const std::filesystem::path& BatchPath,
		std::filesystem::path& OutRoot, std::string& OutError)
	{
		std::error_code Error;
		const std::filesystem::path Absolute =
			std::filesystem::absolute(BatchPath, Error).lexically_normal();
		if (Error)
		{
			OutError = "Unable to resolve batch path.";
			return false;
		}
		std::filesystem::path Candidate = Absolute.parent_path();
		for (uint32_t Depth = 0u; Depth < 12u && !Candidate.empty(); ++Depth)
		{
			if (std::filesystem::is_regular_file(Candidate / L"AGENTS.md", Error) &&
				!Error && std::filesystem::is_directory(Candidate / L"Client", Error) &&
				!Error && std::filesystem::is_directory(Candidate / L"Data", Error) &&
				!Error && std::filesystem::is_directory(Candidate / L"Tools", Error) &&
				!Error)
			{
				OutRoot = Candidate;
				return true;
			}
			Error.clear();
			const std::filesystem::path Parent = Candidate.parent_path();
			if (Parent == Candidate)
				break;
			Candidate = Parent;
		}
		OutError = "Unable to locate the repository root above the batch.";
		return false;
	}

	bool Resolve_ArtifactPath(const std::filesystem::path& RepositoryRoot,
		const std::string_view Relative, std::filesystem::path& OutPath,
		std::string& OutError)
	{
		if (!Is_SafeRelativePath(Relative))
		{
			OutError = "Batch contains an unsafe repository-relative path: " +
				std::string(Relative);
			return false;
		}
		std::error_code Error;
		const std::filesystem::path Root =
			std::filesystem::weakly_canonical(RepositoryRoot, Error);
		const std::filesystem::path Candidate =
			std::filesystem::weakly_canonical(
				RepositoryRoot / std::filesystem::path(Relative), Error);
		if (Error)
		{
			OutError = "Unable to canonicalize batch artifact: " +
				std::string(Relative);
			return false;
		}
		const std::filesystem::path Difference = Candidate.lexically_relative(Root);
		if (Difference.empty() || Difference.is_absolute() ||
			std::any_of(Difference.begin(), Difference.end(),
				[](const std::filesystem::path& Part) { return Part == L".."; }))
		{
			OutError = "Batch artifact escapes repository root: " +
				std::string(Relative);
			return false;
		}
		OutPath = Candidate;
		return true;
	}

	bool Parse_FileIdentity(const DATA_JSON_VALUE& Value, FILE_IDENTITY& Out,
		std::string& OutError)
	{
		if (!Has_ExactKeys(Value, { "path", "rawSha256",
			"canonicalJsonSha256" }) ||
			!Read_String(Value, "path", Out.strPath) ||
			!Read_String(Value, "rawSha256", Out.strRawSha256) ||
			!Read_String(Value, "canonicalJsonSha256",
				Out.strCanonicalJsonSha256) ||
			!Is_SafeRelativePath(Out.strPath) ||
			!Is_Sha256(Out.strRawSha256) ||
			!Is_Sha256(Out.strCanonicalJsonSha256))
		{
			OutError = "Batch file identity is invalid.";
			return false;
		}
		return true;
	}

	bool Parse_Blockers(const DATA_JSON_VALUE& Value)
	{
		std::vector<std::string> Values;
		return Read_StringArray(Value, Values, 64u) && !Values.empty();
	}

	bool Load_AndVerifyJsonArtifact(const std::filesystem::path& RepositoryRoot,
		const FILE_IDENTITY& Identity,
		std::shared_ptr<const CACHED_JSON_ARTIFACT>& Out,
		std::string& OutError)
	{
		std::filesystem::path Path;
		std::string Bytes;
		DATA_JSON_VALUE Root;
		if (!Resolve_ArtifactPath(RepositoryRoot, Identity.strPath, Path,
				OutError) || !Read_File(Path, Bytes, OutError))
		{
			return false;
		}
		if (CEffectRuntimeAuthorityCodec::Compute_Sha256Hex(Bytes) !=
			Identity.strRawSha256)
		{
			OutError = "Raw SHA-256 drift: " + Identity.strPath;
			return false;
		}
		if (!Parse_Json(Bytes, Root, OutError))
		{
			OutError = "JSON parse failed for " + Identity.strPath + ": " +
				OutError;
			return false;
		}
		const std::string ActualCanonicalSha256 = Canonical_JsonSha256(Root);
		if (ActualCanonicalSha256 != Identity.strCanonicalJsonSha256)
		{
			OutError = "Canonical JSON SHA-256 drift: " + Identity.strPath +
				" (expected " + Identity.strCanonicalJsonSha256 +
				", actual " + ActualCanonicalSha256 + ")";
			return false;
		}
		auto Artifact = std::make_shared<CACHED_JSON_ARTIFACT>();
		Artifact->strBytes = std::move(Bytes);
		Artifact->Root = std::move(Root);
		Out = std::move(Artifact);
		return true;
	}

	bool Match_ArtifactIdentity(const MATERIALIZER_BATCH& Batch,
		const FILE_IDENTITY& Identity)
	{
		const auto Found = Batch.Artifacts.find(Identity.strPath);
		return Found != Batch.Artifacts.end() && nullptr != Found->second &&
			CEffectRuntimeAuthorityCodec::Compute_Sha256Hex(
				Found->second->strBytes) == Identity.strRawSha256 &&
			Canonical_JsonSha256(Found->second->Root) ==
				Identity.strCanonicalJsonSha256;
	}

	bool Parse_InputArtifacts(const DATA_JSON_VALUE& Value,
		MATERIALIZER_BATCH& OutBatch, std::string& OutError)
	{
		if (!Value.Is_Array() || Value.Get_Array().empty() ||
			Value.Get_Array().size() > 512u)
		{
			OutError = "Batch inputArtifacts cardinality is invalid.";
			return false;
		}
		for (const DATA_JSON_VALUE& Row : Value.Get_Array())
		{
			if (!Has_ExactKeys(Row, { "path", "rawSha256",
				"canonicalJsonSha256", "roles" }))
			{
				OutError = "Batch input artifact has unknown or missing fields.";
				return false;
			}
			FILE_IDENTITY Identity;
			if (!Read_String(Row, "path", Identity.strPath) ||
				!Read_String(Row, "rawSha256", Identity.strRawSha256) ||
				!Read_String(Row, "canonicalJsonSha256",
					Identity.strCanonicalJsonSha256) ||
				!Is_SafeRelativePath(Identity.strPath) ||
				!Is_Sha256(Identity.strRawSha256) ||
				!Is_Sha256(Identity.strCanonicalJsonSha256))
			{
				OutError = "Batch input artifact identity is invalid.";
				return false;
			}
			const DATA_JSON_VALUE* Roles = Required(Row, "roles",
				DATA_JSON_TYPE::ARRAY);
			std::vector<std::string> ParsedRoles;
			if (nullptr == Roles || !Read_StringArray(*Roles, ParsedRoles, 8u) ||
				ParsedRoles.empty())
			{
				OutError = "Batch input artifact roles are invalid.";
				return false;
			}
			std::shared_ptr<const CACHED_JSON_ARTIFACT> Artifact;
			if (!Load_AndVerifyJsonArtifact(OutBatch.RepositoryRoot, Identity,
					Artifact, OutError) ||
				!OutBatch.Artifacts.emplace(Identity.strPath,
					std::move(Artifact)).second)
			{
				if (OutError.empty())
					OutError = "Batch input artifact path is duplicated.";
				return false;
			}
		}
		return true;
	}

	bool Validate_BatchEnvelope(const DATA_JSON_VALUE& Root,
		MATERIALIZER_BATCH& OutBatch, std::string& OutError)
	{
		if (!Has_ExactKeys(Root, { "schema", "formatVersion", "batchId",
			"contractRole", "scope", "schemaIdentity", "builderIdentity",
			"inputArtifacts", "stages", "elementPlans", "legacyStarterStages",
			"legacyStarterCandidates", "denominators", "fullScopeDenominators",
			"materialDispositionCounts", "carrierDispositionCounts",
			"admission", "transactionPolicy", "artifactSha256" }))
		{
			OutError = "Batch root has unknown or missing fields.";
			return false;
		}
		std::string Schema;
		std::string BatchId;
		std::string Role;
		uint32_t Version = 0u;
		if (!Read_String(Root, "schema", Schema) ||
			Schema != "lostark.effect-authored-import-batch" ||
			!Read_U32(Root, "formatVersion", Version) || Version != 1u ||
			!Read_String(Root, "batchId", BatchId) ||
			BatchId != "effect.authored-import.four-class.track-a.v1" ||
			!Read_String(Root, "contractRole", Role) ||
			Role != "OFFLINE_AUTHORING_STAGE_INPUT_NOT_PRODUCT_MAPPING" ||
			!Verify_SelfHash(Root, "artifactSha256", OutError))
		{
			if (OutError.empty())
				OutError = "Batch root identity is invalid.";
			return false;
		}

		const DATA_JSON_VALUE* Scope = Required(Root, "scope",
			DATA_JSON_TYPE::OBJECT);
		if (nullptr == Scope || !Has_ExactKeys(*Scope,
			{ "characterClasses", "trackAProgramEffectAssetIds",
			  "extensionCanaryEffectAssetId",
			  "trackAUnifiedCandidateEffectAssetIds",
			  "legacyStarterUnifiedCandidateEffectAssetIds",
			  "unifiedCandidateEffectAssetIds",
			  "artistFDirectSliceEffectAssetId",
			  "artistFExcluded",
			  "productCatalogMutation", "animationEventMutation" }))
		{
			OutError = "Batch scope is invalid.";
			return false;
		}
		const DATA_JSON_VALUE* Classes = Required(*Scope, "characterClasses",
			DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* TrackAIds = Required(*Scope,
			"trackAProgramEffectAssetIds", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* CandidateIds = Required(*Scope,
			"unifiedCandidateEffectAssetIds", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* TrackACandidateIds = Required(*Scope,
			"trackAUnifiedCandidateEffectAssetIds", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* LegacyCandidateIds = Required(*Scope,
			"legacyStarterUnifiedCandidateEffectAssetIds", DATA_JSON_TYPE::ARRAY);
		std::vector<std::string> ParsedClasses;
		std::vector<std::string> ParsedTrackAIds;
		std::vector<std::string> ParsedTrackACandidateIds;
		std::vector<std::string> ParsedLegacyCandidateIds;
		std::vector<std::string> ParsedCandidateIds;
		std::string CanaryId;
		std::string ArtistFId;
		if (nullptr == Classes || nullptr == TrackAIds || nullptr == CandidateIds ||
			nullptr == TrackACandidateIds || nullptr == LegacyCandidateIds ||
			!Read_StringArray(*Classes, ParsedClasses, 4u) ||
			!Read_StringArray(*TrackAIds, ParsedTrackAIds, 12u) ||
			!Read_StringArray(*TrackACandidateIds, ParsedTrackACandidateIds, 13u) ||
			!Read_StringArray(*LegacyCandidateIds, ParsedLegacyCandidateIds, 88u) ||
			!Read_StringArray(*CandidateIds, ParsedCandidateIds, 101u) ||
			ParsedClasses != std::vector<std::string>{ "ARTIST",
				"DIMENSIONMASTER", "LANCE_MASTER", "WARLORD" } ||
			ParsedTrackAIds.size() != EXPECTED_TRACK_A_STAGE_COUNT ||
			ParsedTrackACandidateIds.size() != EXPECTED_STAGE_COUNT ||
			ParsedLegacyCandidateIds.size() !=
				EXPECTED_LEGACY_STARTER_CANDIDATE_COUNT ||
			ParsedCandidateIds.size() != EXPECTED_TOTAL_CANDIDATE_COUNT ||
			std::set<std::string>(ParsedCandidateIds.begin(),
				ParsedCandidateIds.end()).size() != EXPECTED_TOTAL_CANDIDATE_COUNT ||
			!std::equal(ParsedTrackACandidateIds.begin(),
				ParsedTrackACandidateIds.end(), ParsedCandidateIds.begin()) ||
			!std::equal(ParsedLegacyCandidateIds.begin(),
				ParsedLegacyCandidateIds.end(),
				ParsedCandidateIds.begin() + EXPECTED_STAGE_COUNT) ||
			!std::ranges::all_of(ParsedCandidateIds,
				[](const std::string& Id) { return Id.ends_with(".unified"); }) ||
			!Read_String(*Scope, "extensionCanaryEffectAssetId", CanaryId) ||
			CanaryId != "effect.warlord.skill.17000.ba1" ||
			!Read_String(*Scope, "artistFDirectSliceEffectAssetId", ArtistFId) ||
			ArtistFId != "effect.artist.skill.31470.unified" ||
			std::ranges::find(ParsedCandidateIds, ArtistFId) !=
				ParsedCandidateIds.end() ||
			!Read_True(*Scope, "artistFExcluded") ||
			!Read_False(*Scope, "productCatalogMutation") ||
			!Read_False(*Scope, "animationEventMutation") ||
			std::ranges::find(ParsedTrackAIds,
				"effect.artist.skill.31470") != ParsedTrackAIds.end())
		{
			OutError = "Batch four-class scope changed.";
			return false;
		}

		FILE_IDENTITY SchemaIdentity;
		const DATA_JSON_VALUE* SchemaRow = Required(Root, "schemaIdentity",
			DATA_JSON_TYPE::OBJECT);
		if (nullptr == SchemaRow ||
			!Parse_FileIdentity(*SchemaRow, SchemaIdentity, OutError))
		{
			return false;
		}
		const DATA_JSON_VALUE* Builder = Required(Root, "builderIdentity",
			DATA_JSON_TYPE::OBJECT);
		std::string BuilderPath;
		std::string BuilderRawSha;
		if (nullptr == Builder || !Has_ExactKeys(*Builder,
			{ "path", "rawSha256" }) ||
			!Read_String(*Builder, "path", BuilderPath) ||
			!Read_String(*Builder, "rawSha256", BuilderRawSha) ||
			!Is_SafeRelativePath(BuilderPath) || !Is_Sha256(BuilderRawSha))
		{
			OutError = "Batch builder identity is invalid.";
			return false;
		}
		std::filesystem::path SchemaPath;
		std::filesystem::path BuilderFile;
		std::string SchemaBytes;
		std::string BuilderBytes;
		DATA_JSON_VALUE ParsedSchema;
		if (!Resolve_ArtifactPath(OutBatch.RepositoryRoot, SchemaIdentity.strPath,
				SchemaPath, OutError) || !Read_File(SchemaPath, SchemaBytes, OutError) ||
			CEffectRuntimeAuthorityCodec::Compute_Sha256Hex(SchemaBytes) !=
				SchemaIdentity.strRawSha256 ||
			!Parse_Json(SchemaBytes, ParsedSchema, OutError) ||
			Canonical_JsonSha256(ParsedSchema) !=
				SchemaIdentity.strCanonicalJsonSha256 ||
			!Resolve_ArtifactPath(OutBatch.RepositoryRoot, BuilderPath,
				BuilderFile, OutError) ||
			!Read_File(BuilderFile, BuilderBytes, OutError) ||
			CEffectRuntimeAuthorityCodec::Compute_Sha256Hex(BuilderBytes) !=
				BuilderRawSha)
		{
			if (OutError.empty())
				OutError = "Batch schema or builder identity drifted.";
			return false;
		}

		const DATA_JSON_VALUE* Inputs = Required(Root, "inputArtifacts",
			DATA_JSON_TYPE::ARRAY);
		return nullptr != Inputs && Parse_InputArtifacts(*Inputs, OutBatch,
			OutError);
	}

	bool Parse_ReferencedFileIdentity(const DATA_JSON_VALUE& Parent,
		const std::string_view Name, const MATERIALIZER_BATCH& Batch,
		FILE_IDENTITY& Out, std::string& OutError)
	{
		const DATA_JSON_VALUE* Value = Required(Parent, Name,
			DATA_JSON_TYPE::OBJECT);
		if (nullptr == Value || !Parse_FileIdentity(*Value, Out, OutError) ||
			!Match_ArtifactIdentity(Batch, Out))
		{
			if (OutError.empty())
				OutError = "Stage references an unpinned or changed artifact.";
			return false;
		}
		return true;
	}

	bool Validate_CountMap(const DATA_JSON_VALUE& Value,
		const std::initializer_list<std::string_view> Allowed,
		std::size_t& OutTotal)
	{
		if (!Value.Is_Object() || Value.Get_Object().empty() ||
			Value.Get_Object().size() > Allowed.size())
		{
			return false;
		}
		OutTotal = 0u;
		for (const auto& [Key, Count] : Value.Get_Object())
		{
			if (std::ranges::find(Allowed, Key) == Allowed.end() ||
				!Count.Is_Number() || Count.Was_FloatingPointToken() ||
				Count.Get_Number() <= 0.0 ||
				std::floor(Count.Get_Number()) != Count.Get_Number())
			{
				return false;
			}
			OutTotal += static_cast<std::size_t>(Count.Get_Number());
		}
		return true;
	}

	bool Parse_Stage(const DATA_JSON_VALUE& Value,
		const MATERIALIZER_BATCH& Batch, MATERIALIZER_STAGE& Out,
		std::string& OutError)
	{
		if (!Has_ExactKeys(Value, { "stageKey", "mode", "characterClass",
			"animationAssetId", "skillId", "inputSlot", "skillKind",
			"stageIndex", "stageId", "clip", "sourceArtifacts", "selection",
			"target", "elementPlanIds", "elementPlanCount",
			"materialDispositionCounts", "carrierDispositionCounts",
			"productMutation", "visualApproval" }))
		{
			OutError = "Batch stage has unknown or missing fields.";
			return false;
		}
		std::string InputSlot;
		std::string SkillKind;
		if (!Read_String(Value, "stageKey", Out.strStageKey) ||
			!Is_StableId(Out.strStageKey) ||
			!Read_String(Value, "mode", Out.strMode) ||
			(Out.strMode != "TRACK_A_VISUAL_PROGRAM" &&
			 Out.strMode != "WARLORD_FAIL_CLOSED_CANARY") ||
			!Read_String(Value, "characterClass", Out.strCharacterClass) ||
			!Read_String(Value, "animationAssetId", Out.strAnimationAssetId) ||
			!Read_U32(Value, "skillId", Out.iSkillId) ||
			!Read_String(Value, "inputSlot", InputSlot) || InputSlot != "LMB" ||
			!Read_String(Value, "skillKind", SkillKind) || SkillKind != "COMBO" ||
			!Read_U32(Value, "stageIndex", Out.iStageIndex) ||
			!Read_String(Value, "stageId", Out.strStageId) ||
			!Read_String(Value, "clip", Out.strClip) ||
			!Read_False(Value, "productMutation") ||
			!Read_False(Value, "visualApproval"))
		{
			OutError = "Batch stage identity is invalid.";
			return false;
		}

		const bool bCanary = Out.strMode == "WARLORD_FAIL_CLOSED_CANARY";
		if ((bCanary && (Out.strCharacterClass != "WARLORD" ||
			Out.iSkillId != 17000u || Out.iStageIndex != 0u)) ||
			(!bCanary && Out.strCharacterClass != "ARTIST" &&
			 Out.strCharacterClass != "DIMENSIONMASTER" &&
			 Out.strCharacterClass != "LANCE_MASTER"))
		{
			OutError = "Batch stage mode/class boundary is invalid.";
			return false;
		}

		const DATA_JSON_VALUE* Sources = Required(Value, "sourceArtifacts",
			DATA_JSON_TYPE::OBJECT);
		if (nullptr == Sources || !Has_ExactKeys(*Sources,
			{ "manifest", "sourceReceipt", "importedDocument",
			  "conversionReceipt" }))
		{
			OutError = "Batch stage sourceArtifacts is invalid.";
			return false;
		}
		FILE_IDENTITY Manifest;
		FILE_IDENTITY SourceReceipt;
		FILE_IDENTITY ConversionReceipt;
		if (!Parse_ReferencedFileIdentity(*Sources, "manifest", Batch,
				Manifest, OutError) ||
			!Parse_ReferencedFileIdentity(*Sources, "sourceReceipt", Batch,
				SourceReceipt, OutError) ||
			!Parse_ReferencedFileIdentity(*Sources, "conversionReceipt", Batch,
				ConversionReceipt, OutError))
		{
			return false;
		}
		const DATA_JSON_VALUE* Imported = Required(*Sources, "importedDocument",
			DATA_JSON_TYPE::OBJECT);
		if (nullptr == Imported || !Has_ExactKeys(*Imported,
			{ "path", "rawSha256", "canonicalJsonSha256", "effectAssetId" }) ||
			!Read_String(*Imported, "path", Out.ImportedDocument.strPath) ||
			!Read_String(*Imported, "rawSha256",
				Out.ImportedDocument.strRawSha256) ||
			!Read_String(*Imported, "canonicalJsonSha256",
				Out.ImportedDocument.strCanonicalJsonSha256) ||
			!Read_String(*Imported, "effectAssetId",
				Out.strImportedEffectAssetId) ||
			!Is_SafeRelativePath(Out.ImportedDocument.strPath) ||
			!Is_Sha256(Out.ImportedDocument.strRawSha256) ||
			!Is_Sha256(Out.ImportedDocument.strCanonicalJsonSha256) ||
			!Match_ArtifactIdentity(Batch, Out.ImportedDocument))
		{
			OutError = "Batch imported source document identity is invalid.";
			return false;
		}

		const DATA_JSON_VALUE* Selection = Required(Value, "selection",
			DATA_JSON_TYPE::OBJECT);
		if (nullptr == Selection || !Has_ExactKeys(*Selection,
			{ "kind", "artifact", "recordId", "recordSha256",
			  "sourceSelectionReceipt" }))
		{
			OutError = "Batch stage selection is invalid.";
			return false;
		}
		std::string SelectionKind;
		if (!Read_String(*Selection, "kind", SelectionKind) ||
			SelectionKind != Out.strMode ||
			!Parse_ReferencedFileIdentity(*Selection, "artifact", Batch,
				Out.SelectionArtifact, OutError) ||
			!Read_String(*Selection, "recordId", Out.strSelectionRecordId) ||
			!Read_String(*Selection, "recordSha256",
				Out.strSelectionRecordSha256) ||
			!Is_Sha256(Out.strSelectionRecordSha256))
		{
			if (OutError.empty())
				OutError = "Batch selection identity is invalid.";
			return false;
		}
		const DATA_JSON_VALUE* SelectionReceipt = Selection->Find(
			"sourceSelectionReceipt");
		if (nullptr == SelectionReceipt ||
			(!bCanary && !SelectionReceipt->Is_Null()))
		{
			OutError = "Batch Track A selection receipt boundary is invalid.";
			return false;
		}
		if (bCanary)
		{
			FILE_IDENTITY CanaryReceipt;
			if (!SelectionReceipt->Is_Object() ||
				!Parse_FileIdentity(*SelectionReceipt, CanaryReceipt, OutError) ||
				!Match_ArtifactIdentity(Batch, CanaryReceipt))
			{
				OutError = "Batch Warlord canary receipt is invalid.";
				return false;
			}
		}

		const DATA_JSON_VALUE* Target = Required(Value, "target",
			DATA_JSON_TYPE::OBJECT);
		if (nullptr == Target || !Has_ExactKeys(*Target,
			{ "path", "effectAssetId", "requiredOutputVersion",
			  "candidateBaseline", "legacyRollbackBaseline", "blueprint" }) ||
			!Read_String(*Target, "path", Out.strTargetPath) ||
			!Is_SafeRelativePath(Out.strTargetPath) ||
			!Read_String(*Target, "effectAssetId", Out.strTargetEffectAssetId))
		{
			OutError = "Batch target identity is invalid.";
			return false;
		}
		uint32_t OutputVersion = 0u;
		if (!Read_U32(*Target, "requiredOutputVersion", OutputVersion) ||
			OutputVersion != Client::EFFECT_AUTHORING_FORMAT_VERSION ||
			Out.strTargetEffectAssetId.find(".ba") == std::string::npos ||
			!Out.strTargetEffectAssetId.ends_with(".unified") ||
			Out.strTargetEffectAssetId == "effect.artist.skill.31470.unified" ||
			Out.strTargetPath != "Data/Effects/Authored/" +
				Out.strTargetEffectAssetId + ".effect.json")
		{
			OutError = "Batch target output version or scope is invalid.";
			return false;
		}
		const DATA_JSON_VALUE* CandidateBaseline = Required(*Target,
			"candidateBaseline", DATA_JSON_TYPE::OBJECT);
		if (nullptr == CandidateBaseline || !Has_ExactKeys(*CandidateBaseline,
			{ "policy", "expectedRawSha256", "expectedCanonicalJsonSha256",
			  "authoringVersion" }) ||
			!Read_String(*CandidateBaseline, "policy",
				Out.strCandidateBaselinePolicy))
		{
			OutError = "Batch unified candidate baseline contract is invalid.";
			return false;
		}
		const DATA_JSON_VALUE* CandidateRaw = CandidateBaseline->Find(
			"expectedRawSha256");
		const DATA_JSON_VALUE* CandidateCanonical = CandidateBaseline->Find(
			"expectedCanonicalJsonSha256");
		const DATA_JSON_VALUE* CandidateVersion = CandidateBaseline->Find(
			"authoringVersion");
		if (nullptr == CandidateRaw || nullptr == CandidateCanonical ||
			nullptr == CandidateVersion)
		{
			OutError = "Batch unified candidate baseline fields are missing.";
			return false;
		}
		if (Out.strCandidateBaselinePolicy == "MUST_NOT_EXIST")
		{
			if (!CandidateRaw->Is_Null() || !CandidateCanonical->Is_Null() ||
				!CandidateVersion->Is_Null() ||
				Batch.Artifacts.contains(Out.strTargetPath))
			{
				OutError = "MUST_NOT_EXIST candidate carries a baseline artifact.";
				return false;
			}
		}
		else if (Out.strCandidateBaselinePolicy == "EXPECTED_EXACT_OR_REFUSE")
		{
			uint32_t ExistingVersion = 0u;
			if (!CandidateRaw->Is_String() || !CandidateCanonical->Is_String() ||
				!Is_Sha256(CandidateRaw->Get_String()) ||
				!Is_Sha256(CandidateCanonical->Get_String()) ||
				!Read_U32(*CandidateBaseline, "authoringVersion",
					ExistingVersion) ||
				ExistingVersion != Client::EFFECT_AUTHORING_FORMAT_VERSION)
			{
				OutError = "Existing unified candidate baseline identity is invalid.";
				return false;
			}
			Out.strCandidateRawSha256 = CandidateRaw->Get_String();
			Out.strCandidateCanonicalJsonSha256 =
				CandidateCanonical->Get_String();
			const auto CandidateArtifact = Batch.Artifacts.find(Out.strTargetPath);
			if (CandidateArtifact == Batch.Artifacts.end() ||
				CEffectRuntimeAuthorityCodec::Compute_Sha256Hex(
					CandidateArtifact->second->strBytes) !=
					*Out.strCandidateRawSha256 ||
				Canonical_JsonSha256(CandidateArtifact->second->Root) !=
					*Out.strCandidateCanonicalJsonSha256)
			{
				OutError = "Existing unified candidate is not pinned by inputArtifacts.";
				return false;
			}
		}
		else
		{
			OutError = "Batch unified candidate baseline policy is unknown.";
			return false;
		}

		const DATA_JSON_VALUE* Baseline = Required(*Target,
			"legacyRollbackBaseline",
			DATA_JSON_TYPE::OBJECT);
		uint32_t BaselineVersion = 0u;
		std::string BaselinePolicy;
		if (nullptr == Baseline || !Has_ExactKeys(*Baseline,
			{ "path", "effectAssetId", "rawSha256", "canonicalJsonSha256",
			  "authoringVersion", "policy" }) ||
			!Read_String(*Baseline, "path", Out.strLegacyBaselinePath) ||
			!Is_SafeRelativePath(Out.strLegacyBaselinePath) ||
			!Read_String(*Baseline, "effectAssetId",
				Out.strLegacyEffectAssetId) ||
			!Read_String(*Baseline, "rawSha256", Out.strLegacyRawSha256) ||
			!Read_String(*Baseline, "canonicalJsonSha256",
				Out.strLegacyCanonicalJsonSha256) ||
			!Is_Sha256(Out.strLegacyRawSha256) ||
			!Is_Sha256(Out.strLegacyCanonicalJsonSha256) ||
			!Read_U32(*Baseline, "authoringVersion", BaselineVersion) ||
			BaselineVersion != 12u ||
			!Read_String(*Baseline, "policy", BaselinePolicy) ||
			BaselinePolicy != "IMMUTABLE_LEGACY_ROLLBACK_EXACT" ||
			Out.strTargetEffectAssetId !=
				Out.strLegacyEffectAssetId + ".unified" ||
			Out.strLegacyBaselinePath != "Data/Effects/Authored/" +
				Out.strLegacyEffectAssetId + ".effect.json")
		{
			OutError = "Batch legacy rollback baseline contract is invalid.";
			return false;
		}
		const auto BaselineArtifact = Batch.Artifacts.find(
			Out.strLegacyBaselinePath);
		if (BaselineArtifact == Batch.Artifacts.end() ||
			CEffectRuntimeAuthorityCodec::Compute_Sha256Hex(
				BaselineArtifact->second->strBytes) != Out.strLegacyRawSha256 ||
			Canonical_JsonSha256(BaselineArtifact->second->Root) !=
				Out.strLegacyCanonicalJsonSha256)
		{
			OutError = "Batch legacy rollback baseline is not pinned by inputArtifacts.";
			return false;
		}

		const DATA_JSON_VALUE* Blueprint = Required(*Target, "blueprint",
			DATA_JSON_TYPE::OBJECT);
		uint32_t BlueprintVersion = 0u;
		if (nullptr == Blueprint || !Has_ExactKeys(*Blueprint,
			{ "kind", "authoringVersion", "canonicalJsonSha256",
			  "typedCodecSha256" }) ||
			!Read_String(*Blueprint, "kind", Out.strBlueprintKind) ||
			!Read_U32(*Blueprint, "authoringVersion", BlueprintVersion) ||
			BlueprintVersion != 12u ||
			!Read_String(*Blueprint, "canonicalJsonSha256",
				Out.strBlueprintCanonicalJsonSha256) ||
			!Is_Sha256(Out.strBlueprintCanonicalJsonSha256))
		{
			OutError = "Batch target blueprint is invalid.";
			return false;
		}
		const DATA_JSON_VALUE* TypedSha = Blueprint->Find("typedCodecSha256");
		if (nullptr == TypedSha)
		{
			OutError = "Batch target blueprint typed hash field is missing.";
			return false;
		}
		if (bCanary)
		{
			if (Out.strBlueprintKind != "CURRENT_AUTHORED_BASELINE" ||
				!TypedSha->Is_Null() ||
				Out.strBlueprintCanonicalJsonSha256 !=
					Out.strLegacyCanonicalJsonSha256)
			{
				OutError = "Batch Warlord blueprint must remain the current baseline.";
				return false;
			}
		}
		else
		{
			std::string Typed;
			if (Out.strBlueprintKind != "VISUAL_PROGRAM_PROJECTED_DOCUMENT" ||
				!TypedSha->Is_String() || !Is_Sha256(TypedSha->Get_String()))
			{
				OutError = "Batch Track A blueprint typed identity is invalid.";
				return false;
			}
			Out.strBlueprintTypedCodecSha256 = TypedSha->Get_String();
		}

		const DATA_JSON_VALUE* PlanIds = Required(Value, "elementPlanIds",
			DATA_JSON_TYPE::ARRAY);
		uint32_t PlanCount = 0u;
		std::size_t MaterialTotal = 0u;
		std::size_t CarrierTotal = 0u;
		const DATA_JSON_VALUE* MaterialCounts = Required(Value,
			"materialDispositionCounts", DATA_JSON_TYPE::OBJECT);
		const DATA_JSON_VALUE* CarrierCounts = Required(Value,
			"carrierDispositionCounts", DATA_JSON_TYPE::OBJECT);
		if (nullptr == PlanIds ||
			!Read_StringArray(*PlanIds, Out.ElementPlanIds, 64u) ||
			!Read_U32(Value, "elementPlanCount", PlanCount) ||
			PlanCount != Out.ElementPlanIds.size() || 0u == PlanCount ||
			nullptr == MaterialCounts || nullptr == CarrierCounts ||
			!Validate_CountMap(*MaterialCounts,
				{ "TYPED_EXECUTION", "ADMITTED_SOURCE_PROFILE", "FAIL_CLOSED" },
				MaterialTotal) ||
			!Validate_CountMap(*CarrierCounts,
				{ "GENERIC_PARTICLE_IMPORT_CANDIDATE",
				  "FAMILY_ADAPTER_REQUIRED", "SUPPLEMENTAL_ADAPTER_PRESERVE" },
				CarrierTotal) || MaterialTotal != PlanCount ||
			CarrierTotal != PlanCount)
		{
			OutError = "Batch stage plan/count contract is invalid.";
			return false;
		}
		return true;
	}

	bool Parse_Stages(const DATA_JSON_VALUE& Value,
		MATERIALIZER_BATCH& OutBatch, std::string& OutError)
	{
		if (!Value.Is_Array() || Value.Get_Array().size() != EXPECTED_STAGE_COUNT)
		{
			OutError = "Batch must contain exactly thirteen stages.";
			return false;
		}
		std::unordered_set<std::string> StageKeys;
		std::unordered_set<std::string> TargetPaths;
		std::size_t TrackACount = 0u;
		std::size_t CanaryCount = 0u;
		for (const DATA_JSON_VALUE& Row : Value.Get_Array())
		{
			MATERIALIZER_STAGE Stage;
			if (!Parse_Stage(Row, OutBatch, Stage, OutError) ||
				!StageKeys.insert(Stage.strStageKey).second ||
				!TargetPaths.insert(Stage.strTargetPath).second)
			{
				if (OutError.empty())
					OutError = "Batch stage or target is duplicated.";
				return false;
			}
			TrackACount += Stage.strMode == "TRACK_A_VISUAL_PROGRAM" ? 1u : 0u;
			CanaryCount += Stage.strMode == "WARLORD_FAIL_CLOSED_CANARY" ? 1u : 0u;
			OutBatch.Stages.push_back(std::move(Stage));
		}
		if (TrackACount != EXPECTED_TRACK_A_STAGE_COUNT || CanaryCount != 1u)
		{
			OutError = "Batch Track A/canary stage denominator changed.";
			return false;
		}
		return true;
	}

	bool Parse_ElementPlan(const DATA_JSON_VALUE& Value,
		MATERIALIZER_ELEMENT_PLAN& Out, std::string& OutError)
	{
		if (!Has_ExactKeys(Value, { "planId", "stageKey", "source", "target",
			"selectionIdentity", "family", "carrierDisposition",
			"materialDisposition", "productMutation", "visualApproval" }) ||
			!Read_String(Value, "planId", Out.strPlanId) ||
			!Is_StableId(Out.strPlanId) ||
			!Read_String(Value, "stageKey", Out.strStageKey) ||
			!Is_StableId(Out.strStageKey) ||
			!Read_String(Value, "family", Out.strFamily) ||
			!Is_StableId(Out.strFamily) ||
			!Read_False(Value, "productMutation") ||
			!Read_False(Value, "visualApproval"))
		{
			OutError = "Batch element plan identity is invalid.";
			return false;
		}

		const DATA_JSON_VALUE* Source = Required(Value, "source",
			DATA_JSON_TYPE::OBJECT);
		std::string SourceKind;
		if (nullptr == Source || !Read_String(*Source, "kind", SourceKind))
		{
			OutError = "Batch element source is invalid.";
			return false;
		}
		if (SourceKind == "IMPORTED_ELEMENT")
		{
			Out.bImportedSource = true;
			if (!Has_ExactKeys(*Source, { "kind", "documentPath",
				"documentRawSha256", "effectAssetId", "elementId",
				"elementCanonicalSha256", "sourceRecipeCanonicalSha256",
				"sourceDetailCanonicalSha256",
				"sourceAttachmentCanonicalSha256", "sourceEventId" }) ||
				!Read_String(*Source, "documentPath", Out.strSourceDocumentPath) ||
				!Is_SafeRelativePath(Out.strSourceDocumentPath) ||
				!Read_String(*Source, "documentRawSha256",
					Out.strSourceDocumentRawSha256) ||
				!Read_String(*Source, "effectAssetId",
					Out.strSourceEffectAssetId) ||
				!Read_String(*Source, "elementId", Out.strSourceElementId) ||
				!Read_String(*Source, "elementCanonicalSha256",
					Out.strSourceElementCanonicalSha256) ||
				!Read_String(*Source, "sourceRecipeCanonicalSha256",
					Out.strSourceRecipeCanonicalSha256) ||
				!Read_String(*Source, "sourceDetailCanonicalSha256",
					Out.strSourceDetailCanonicalSha256) ||
				!Read_String(*Source, "sourceAttachmentCanonicalSha256",
					Out.strSourceAttachmentCanonicalSha256) ||
				!Is_Sha256(Out.strSourceDocumentRawSha256) ||
				!Is_Sha256(Out.strSourceElementCanonicalSha256) ||
				!Is_Sha256(Out.strSourceRecipeCanonicalSha256) ||
				!Is_Sha256(Out.strSourceDetailCanonicalSha256) ||
				!Is_Sha256(Out.strSourceAttachmentCanonicalSha256))
			{
				OutError = "Batch imported element identity is invalid.";
				return false;
			}
			const DATA_JSON_VALUE* EventId = Source->Find("sourceEventId");
			if (nullptr == EventId ||
				(!EventId->Is_Null() && !EventId->Is_String()))
			{
				OutError = "Batch imported element sourceEventId is invalid.";
				return false;
			}
			if (EventId->Is_String())
				Out.strSourceEventId = EventId->Get_String();
		}
		else if (SourceKind == "VISUAL_PROGRAM_SUPPLEMENTAL")
		{
			if (!Has_ExactKeys(*Source, { "kind", "recordId",
				"recordCanonicalSha256", "sourceEventId" }) ||
				!Read_String(*Source, "recordId",
					Out.strSupplementalSourceRecordId) ||
				!Read_String(*Source, "recordCanonicalSha256",
					Out.strSupplementalSourceRecordSha256) ||
				!Read_String(*Source, "sourceEventId", Out.strSourceEventId) ||
				!Is_Sha256(Out.strSupplementalSourceRecordSha256))
			{
				OutError = "Batch supplemental element source is invalid.";
				return false;
			}
		}
		else
		{
			OutError = "Batch element source kind is unsupported.";
			return false;
		}

		const DATA_JSON_VALUE* Target = Required(Value, "target",
			DATA_JSON_TYPE::OBJECT);
		if (nullptr == Target || !Has_ExactKeys(*Target,
			{ "documentPath", "effectAssetId", "elementId", "groupId",
			  "displayName", "blueprintElementCanonicalSha256",
			  "baselineElementCanonicalSha256", "targetDetailCanonicalSha256" }) ||
			!Read_String(*Target, "documentPath", Out.strTargetDocumentPath) ||
			!Is_SafeRelativePath(Out.strTargetDocumentPath) ||
			!Read_String(*Target, "effectAssetId", Out.strTargetEffectAssetId) ||
			!Read_String(*Target, "elementId", Out.strTargetElementId) ||
			!Read_String(*Target, "groupId", Out.strTargetGroupId) ||
			!Read_String(*Target, "displayName", Out.strTargetDisplayName) ||
			!Read_String(*Target, "blueprintElementCanonicalSha256",
				Out.strBlueprintElementCanonicalSha256) ||
			!Read_String(*Target, "targetDetailCanonicalSha256",
				Out.strTargetDetailCanonicalSha256) ||
			!Is_StableId(Out.strTargetElementId) ||
			!Is_StableId(Out.strTargetGroupId) ||
			!Is_Sha256(Out.strBlueprintElementCanonicalSha256) ||
			!Is_Sha256(Out.strTargetDetailCanonicalSha256))
		{
			OutError = "Batch target element identity is invalid.";
			return false;
		}
		const DATA_JSON_VALUE* BaselineElement = Target->Find(
			"baselineElementCanonicalSha256");
		if (nullptr == BaselineElement ||
			(!BaselineElement->Is_Null() && !BaselineElement->Is_String()) ||
			(BaselineElement->Is_String() &&
			 !Is_Sha256(BaselineElement->Get_String())))
		{
			OutError = "Batch baseline Element SHA-256 is invalid.";
			return false;
		}
		if (BaselineElement->Is_String())
			Out.strBaselineElementCanonicalSha256 = BaselineElement->Get_String();

		const DATA_JSON_VALUE* Selection = Required(Value, "selectionIdentity",
			DATA_JSON_TYPE::OBJECT);
		if (nullptr == Selection || !Has_ExactKeys(*Selection,
			{ "recordId", "recordSha256" }) ||
			!Read_String(*Selection, "recordId", Out.strSelectionRecordId) ||
			!Read_String(*Selection, "recordSha256",
				Out.strSelectionRecordSha256) ||
			!Is_Sha256(Out.strSelectionRecordSha256))
		{
			OutError = "Batch element selection identity is invalid.";
			return false;
		}

		const DATA_JSON_VALUE* Carrier = Required(Value, "carrierDisposition",
			DATA_JSON_TYPE::OBJECT);
		std::string CarrierKind;
		if (nullptr == Carrier || !Has_ExactKeys(*Carrier,
			{ "kind", "blockers" }) ||
			!Read_String(*Carrier, "kind", CarrierKind) ||
			nullptr == Carrier->Find("blockers") ||
			!Parse_Blockers(*Carrier->Find("blockers")))
		{
			OutError = "Batch carrier disposition is invalid.";
			return false;
		}
		if (CarrierKind == "GENERIC_PARTICLE_IMPORT_CANDIDATE")
			Out.eCarrier = CARRIER_DISPOSITION::GENERIC_PARTICLE_IMPORT_CANDIDATE;
		else if (CarrierKind == "FAMILY_ADAPTER_REQUIRED")
			Out.eCarrier = CARRIER_DISPOSITION::FAMILY_ADAPTER_REQUIRED;
		else if (CarrierKind == "SUPPLEMENTAL_ADAPTER_PRESERVE")
			Out.eCarrier = CARRIER_DISPOSITION::SUPPLEMENTAL_ADAPTER_PRESERVE;
		else
		{
			OutError = "Batch carrier disposition kind is unsupported.";
			return false;
		}

		const DATA_JSON_VALUE* Material = Required(Value, "materialDisposition",
			DATA_JSON_TYPE::OBJECT);
		std::string MaterialKind;
		if (nullptr == Material || !Read_String(*Material, "kind", MaterialKind))
		{
			OutError = "Batch material disposition is invalid.";
			return false;
		}
		if (MaterialKind == "TYPED_EXECUTION")
		{
			std::string Sha;
			if (!Has_ExactKeys(*Material,
				{ "kind", "executionSnapshotSha256" }) ||
				!Read_String(*Material, "executionSnapshotSha256", Sha) ||
				!Is_Sha256(Sha))
			{
				OutError = "Batch typed execution disposition is invalid.";
				return false;
			}
			Out.eMaterial = MATERIAL_DISPOSITION::TYPED_EXECUTION;
		}
		else if (MaterialKind == "ADMITTED_SOURCE_PROFILE")
		{
			std::string Sha;
			std::string Profile;
			std::string RuntimeProfile;
			if (!Has_ExactKeys(*Material,
				{ "kind", "sourceProfileSha256", "profileId",
				  "runtimeShaderProfileId" }) ||
				!Read_String(*Material, "sourceProfileSha256", Sha) ||
				!Read_String(*Material, "profileId", Profile) ||
				!Read_String(*Material, "runtimeShaderProfileId", RuntimeProfile) ||
				!Is_Sha256(Sha) || !Is_StableId(Profile) ||
				!Is_StableId(RuntimeProfile))
			{
				OutError = "Batch admitted source-profile disposition is invalid.";
				return false;
			}
			Out.eMaterial = MATERIAL_DISPOSITION::ADMITTED_SOURCE_PROFILE;
		}
		else if (MaterialKind == "FAIL_CLOSED")
		{
			if (!Has_ExactKeys(*Material, { "kind", "blockers" }) ||
				nullptr == Material->Find("blockers") ||
				!Parse_Blockers(*Material->Find("blockers")))
			{
				OutError = "Batch fail-closed material disposition is invalid.";
				return false;
			}
			Out.eMaterial = MATERIAL_DISPOSITION::FAIL_CLOSED;
		}
		else
		{
			OutError = "Batch material disposition kind is unsupported.";
			return false;
		}

		if ((Out.eCarrier ==
			 CARRIER_DISPOSITION::SUPPLEMENTAL_ADAPTER_PRESERVE) !=
			!Out.bImportedSource ||
			Out.eCarrier == CARRIER_DISPOSITION::FAMILY_ADAPTER_REQUIRED &&
			Out.strFamily != "CASCADE_RIBBON" ||
			Out.eCarrier ==
				CARRIER_DISPOSITION::SUPPLEMENTAL_ADAPTER_PRESERVE &&
			Out.strFamily != "ANIMATION_TRAIL")
		{
			OutError = "Batch source/family/carrier one-of contract is invalid.";
			return false;
		}
		return true;
	}

	bool Parse_AndJoinPlans(const DATA_JSON_VALUE& Value,
		MATERIALIZER_BATCH& OutBatch, std::string& OutError)
	{
		if (!Value.Is_Array() ||
			Value.Get_Array().size() != EXPECTED_ELEMENT_PLAN_COUNT)
		{
			OutError = "Batch must contain exactly seventy-nine element plans.";
			return false;
		}
		std::unordered_map<std::string, const MATERIALIZER_STAGE*> Stages;
		std::unordered_map<std::string, std::vector<std::string>> ActualPlanIds;
		std::unordered_map<std::string, std::unordered_set<std::string>>
			TargetIds;
		for (const MATERIALIZER_STAGE& Stage : OutBatch.Stages)
			Stages.emplace(Stage.strStageKey, &Stage);
		std::unordered_set<std::string> PlanIds;
		std::size_t GenericCount = 0u;
		std::size_t AdapterCount = 0u;
		std::size_t SupplementalCount = 0u;
		std::size_t TypedCount = 0u;
		std::size_t AdmittedCount = 0u;
		std::size_t FailClosedCount = 0u;
		for (const DATA_JSON_VALUE& Row : Value.Get_Array())
		{
			MATERIALIZER_ELEMENT_PLAN Plan;
			if (!Parse_ElementPlan(Row, Plan, OutError) ||
				!PlanIds.insert(Plan.strPlanId).second)
			{
				if (OutError.empty())
					OutError = "Batch element plan ID is duplicated.";
				return false;
			}
			const auto StageIterator = Stages.find(Plan.strStageKey);
			if (StageIterator == Stages.end() || nullptr == StageIterator->second)
			{
				OutError = "Batch element plan references an unknown stage.";
				return false;
			}
			const MATERIALIZER_STAGE& Stage = *StageIterator->second;
			if (Plan.strTargetDocumentPath != Stage.strTargetPath ||
				Plan.strTargetEffectAssetId != Stage.strTargetEffectAssetId ||
				(Plan.bImportedSource &&
				 (Plan.strSourceDocumentPath != Stage.ImportedDocument.strPath ||
				  Plan.strSourceDocumentRawSha256 !=
					Stage.ImportedDocument.strRawSha256 ||
				  Plan.strSourceEffectAssetId != Stage.strImportedEffectAssetId)) ||
				!TargetIds[Plan.strStageKey].insert(
					Plan.strTargetElementId).second)
			{
				OutError = "Batch element plan stage/source/target join is invalid.";
				return false;
			}
			const auto SourceArtifact = OutBatch.Artifacts.find(
				Stage.ImportedDocument.strPath);
			if (Plan.bImportedSource &&
				(SourceArtifact == OutBatch.Artifacts.end() ||
				 CEffectRuntimeAuthorityCodec::Compute_Sha256Hex(
					SourceArtifact->second->strBytes) !=
					Plan.strSourceDocumentRawSha256))
			{
				OutError = "Batch plan source document is not pinned.";
				return false;
			}
			ActualPlanIds[Plan.strStageKey].push_back(Plan.strPlanId);
			GenericCount += Plan.eCarrier ==
				CARRIER_DISPOSITION::GENERIC_PARTICLE_IMPORT_CANDIDATE ? 1u : 0u;
			AdapterCount += Plan.eCarrier ==
				CARRIER_DISPOSITION::FAMILY_ADAPTER_REQUIRED ? 1u : 0u;
			SupplementalCount += Plan.eCarrier ==
				CARRIER_DISPOSITION::SUPPLEMENTAL_ADAPTER_PRESERVE ? 1u : 0u;
			TypedCount += Plan.eMaterial ==
				MATERIAL_DISPOSITION::TYPED_EXECUTION ? 1u : 0u;
			AdmittedCount += Plan.eMaterial ==
				MATERIAL_DISPOSITION::ADMITTED_SOURCE_PROFILE ? 1u : 0u;
			FailClosedCount += Plan.eMaterial ==
				MATERIAL_DISPOSITION::FAIL_CLOSED ? 1u : 0u;
			OutBatch.Plans.push_back(std::move(Plan));
		}
		for (const MATERIALIZER_STAGE& Stage : OutBatch.Stages)
		{
			if (ActualPlanIds[Stage.strStageKey] != Stage.ElementPlanIds)
			{
				OutError = "Batch stage elementPlanIds order does not match plans.";
				return false;
			}
		}
		if (GenericCount != EXPECTED_GENERIC_PLAN_COUNT ||
			AdapterCount != EXPECTED_ADAPTER_PLAN_COUNT ||
			SupplementalCount != EXPECTED_SUPPLEMENTAL_PLAN_COUNT ||
			TypedCount != 0u || AdmittedCount != EXPECTED_ADMITTED_MATERIAL_COUNT ||
			FailClosedCount != EXPECTED_FAIL_CLOSED_MATERIAL_COUNT)
		{
			OutError = "Batch carrier/material denominator changed.";
			return false;
		}
		return true;
	}

	bool Read_ExpectedCount(const DATA_JSON_VALUE& Object,
		const std::string_view Name, const uint32_t Expected)
	{
		uint32_t Actual = 0u;
		return Read_U32(Object, Name, Actual) && Actual == Expected;
	}

	const CACHED_JSON_ARTIFACT* Find_UniqueArtifactBySchema(
		const MATERIALIZER_BATCH& Batch, const std::string_view Schema)
	{
		const CACHED_JSON_ARTIFACT* Found = nullptr;
		for (const auto& [Path, Artifact] : Batch.Artifacts)
		{
			(void)Path;
			const DATA_JSON_VALUE* Value = nullptr == Artifact ? nullptr :
				Required(Artifact->Root, "schema", DATA_JSON_TYPE::STRING);
			if (nullptr == Value || Value->Get_String() != Schema)
				continue;
			if (nullptr != Found)
				return nullptr;
			Found = Artifact.get();
		}
		return Found;
	}

	const DATA_JSON_VALUE* Find_UniqueObjectByCanonicalSha(
		const DATA_JSON_VALUE& Array, const std::string_view ExpectedSha)
	{
		if (!Array.Is_Array())
			return nullptr;
		const DATA_JSON_VALUE* Found = nullptr;
		for (const DATA_JSON_VALUE& Row : Array.Get_Array())
		{
			if (!Row.Is_Object() || Canonical_JsonSha256(Row) != ExpectedSha)
				continue;
			if (nullptr != Found)
				return nullptr;
			Found = &Row;
		}
		return Found;
	}

	bool Read_NullableString(const DATA_JSON_VALUE& Object,
		const std::string_view Name, std::optional<std::string>& Out)
	{
		const DATA_JSON_VALUE* Value = Object.Find(Name);
		if (nullptr == Value)
			return false;
		if (Value->Is_Null())
		{
			Out.reset();
			return true;
		}
		if (!Value->Is_String() || Value->Get_String().empty())
			return false;
		Out = Value->Get_String();
		return true;
	}

	bool Build_UnifiedCandidateId(const std::string_view LegacyId,
		std::string& Out)
	{
		constexpr std::string_view Token = ".authored-baseline";
		if (!Is_StableId(LegacyId) || LegacyId.ends_with(".unified"))
			return false;
		const std::size_t Position = LegacyId.find(Token);
		if (Position != std::string_view::npos &&
			LegacyId.find(Token, Position + Token.size()) != std::string_view::npos)
		{
			return false;
		}
		Out.assign(LegacyId);
		if (Position != std::string_view::npos)
			Out.erase(Position, Token.size());
		Out += ".unified";
		return Is_StableId(Out);
	}

	bool Verify_AuthoringArtifactIdentity(const MATERIALIZER_BATCH& Batch,
		const FILE_IDENTITY& Identity, const std::string_view EffectAssetId,
		const uint32_t Version, std::string& OutError)
	{
		const auto Found = Batch.Artifacts.find(Identity.strPath);
		uint32_t ActualVersion = 0u;
		std::string ActualId;
		if (Found == Batch.Artifacts.end() || nullptr == Found->second ||
			!Match_ArtifactIdentity(Batch, Identity) ||
			!Read_U32(Found->second->Root, "version", ActualVersion) ||
			ActualVersion != Version ||
			!Read_String(Found->second->Root, "effectAssetId", ActualId) ||
			ActualId != EffectAssetId)
		{
			OutError = "Legacy starter authored source identity changed: " +
				Identity.strPath;
			return false;
		}
		return true;
	}

	std::string Normalize_CrlfToLf(const std::string_view Bytes)
	{
		std::string Normalized;
		Normalized.reserve(Bytes.size());
		for (std::size_t Index = 0u; Index < Bytes.size(); ++Index)
		{
			if (Bytes[Index] == '\r' && Index + 1u < Bytes.size() &&
				Bytes[Index + 1u] == '\n')
			{
				continue;
			}
			Normalized.push_back(Bytes[Index]);
		}
		return Normalized;
	}

	std::size_t Count_AnimeventEffectReferences(const std::string_view Bytes,
		const std::string_view EffectAssetId)
	{
		const std::string Needle = "payload=\"" +
			std::string(EffectAssetId) + "\"";
		std::size_t Count = 0u;
		std::size_t Position = 0u;
		while ((Position = Bytes.find(Needle, Position)) != std::string_view::npos)
		{
			++Count;
			Position += Needle.size();
		}
		return Count;
	}

	bool Parse_LegacyStarterStages(const DATA_JSON_VALUE& Value,
		MATERIALIZER_BATCH& OutBatch, std::string& OutError)
	{
		if (!Value.Is_Array() ||
			Value.Get_Array().size() != EXPECTED_LEGACY_STARTER_STAGE_COUNT)
		{
			OutError = "Batch must contain exactly sixty-one legacy starter stages.";
			return false;
		}
		const CACHED_JSON_ARTIFACT* Rollout = Find_UniqueArtifactBySchema(
			OutBatch, "lostark.four-class-authored-product-rollout");
		const DATA_JSON_VALUE* RolloutStages = nullptr == Rollout ? nullptr :
			Required(Rollout->Root, "stages", DATA_JSON_TYPE::ARRAY);
		if (nullptr == RolloutStages)
		{
			OutError = "Pinned four-class product rollout is unavailable.";
			return false;
		}

		std::unordered_set<std::string> StageKeys;
		std::set<std::pair<std::string, uint32_t>> SkillIdentities;
		std::unordered_map<std::string, std::size_t> CandidateReferences;
		std::size_t EffectBearingStages = 0u;
		std::size_t SilentStages = 0u;
		std::size_t VisualOccurrences = 0u;
		std::size_t SilentOccurrences = 0u;
		for (const DATA_JSON_VALUE& Row : Value.Get_Array())
		{
			if (!Has_ExactKeys(Row, { "stageKey", "mode", "characterClass",
				"animationAssetId", "skillId", "inputSlot", "skillKind",
				"stageIndex", "stageId", "status", "sourceManifest",
				"rolloutStageCanonicalSha256", "clips",
				"candidateEffectAssetIds", "blockers", "productMutation",
				"visualApproval" }))
			{
				OutError = "Legacy starter stage has unknown or missing fields.";
				return false;
			}
			LEGACY_STARTER_STAGE Stage;
			std::string Mode;
			std::string InputSlot;
			std::string SkillKind;
			std::string Status;
			if (!Read_String(Row, "stageKey", Stage.strStageKey) ||
				!StageKeys.insert(Stage.strStageKey).second ||
				!Read_String(Row, "mode", Mode) ||
				!Read_String(Row, "characterClass", Stage.strCharacterClass) ||
				!Read_String(Row, "animationAssetId", Stage.strAnimationAssetId) ||
				!Read_U32(Row, "skillId", Stage.iSkillId) ||
				!Read_String(Row, "inputSlot", InputSlot) ||
				!Read_String(Row, "skillKind", SkillKind) ||
				!Read_U32(Row, "stageIndex", Stage.iStageIndex) ||
				!Read_String(Row, "stageId", Stage.strStageId) ||
				!Read_String(Row, "status", Status) ||
				!((Mode == "LEGACY_STARTER_STAGE" && Status == "effectBearing") ||
					(Mode == "INTENTIONALLY_SILENT" &&
						Status == "sourceIntentionallySilent")) ||
				!Read_String(Row, "rolloutStageCanonicalSha256",
					Stage.strRolloutStageCanonicalSha256) ||
				!Is_Sha256(Stage.strRolloutStageCanonicalSha256) ||
				!Read_False(Row, "productMutation") ||
				!Read_False(Row, "visualApproval"))
			{
				OutError = "Legacy starter stage identity is invalid.";
				return false;
			}
			Stage.bEffectBearing = Status == "effectBearing";
			EffectBearingStages += Stage.bEffectBearing ? 1u : 0u;
			SilentStages += Stage.bEffectBearing ? 0u : 1u;
			SkillIdentities.emplace(Stage.strCharacterClass, Stage.iSkillId);
			const DATA_JSON_VALUE* Manifest = Required(Row, "sourceManifest",
				DATA_JSON_TYPE::OBJECT);
			if (nullptr == Manifest ||
				!Parse_FileIdentity(*Manifest, Stage.SourceManifest, OutError) ||
				!Match_ArtifactIdentity(OutBatch, Stage.SourceManifest))
			{
				if (OutError.empty())
					OutError = "Legacy starter source manifest is not pinned.";
				return false;
			}
			const DATA_JSON_VALUE* Clips = Required(Row, "clips",
				DATA_JSON_TYPE::ARRAY);
			const DATA_JSON_VALUE* CandidateIds = Required(Row,
				"candidateEffectAssetIds", DATA_JSON_TYPE::ARRAY);
			const DATA_JSON_VALUE* Blockers = Required(Row, "blockers",
				DATA_JSON_TYPE::ARRAY);
			std::vector<std::string> ParsedCandidateIds;
			std::vector<std::string> ParsedBlockers;
			if (nullptr == Clips || Clips->Get_Array().empty() ||
				nullptr == CandidateIds ||
				!Read_StringArray(*CandidateIds, ParsedCandidateIds, 16u) ||
				nullptr == Blockers ||
				!Read_StringArray(*Blockers, ParsedBlockers, 8u) ||
				ParsedBlockers.empty())
			{
				OutError = "Legacy starter stage clip/disposition contract is invalid.";
				return false;
			}
			std::vector<std::string> ClipCandidateIds;
			for (std::size_t Index = 0u; Index < Clips->Get_Array().size(); ++Index)
			{
				const DATA_JSON_VALUE& ClipRow = Clips->Get_Array()[Index];
				if (!Has_ExactKeys(ClipRow, { "clip", "stageClipIndex", "status",
					"legacyEffectAssetId", "candidateEffectAssetId" }))
				{
					OutError = "Legacy starter clip has unknown or missing fields.";
					return false;
				}
				LEGACY_STARTER_CLIP Clip;
				std::string ClipStatus;
				if (!Read_String(ClipRow, "clip", Clip.strClip) ||
					!Read_U32(ClipRow, "stageClipIndex", Clip.iStageClipIndex) ||
					Clip.iStageClipIndex != Index ||
					!Read_String(ClipRow, "status", ClipStatus) ||
					(ClipStatus != "visualBearing" &&
					 ClipStatus != "noSelectedCarrier" &&
					 ClipStatus != "sourceIntentionallySilent") ||
					(Stage.bEffectBearing ==
						(ClipStatus == "sourceIntentionallySilent")) ||
					!Read_NullableString(ClipRow, "legacyEffectAssetId",
						Clip.strLegacyEffectAssetId) ||
					!Read_NullableString(ClipRow, "candidateEffectAssetId",
						Clip.strCandidateEffectAssetId))
				{
					OutError = "Legacy starter clip identity is invalid.";
					return false;
				}
				Clip.bVisualBearing = ClipStatus == "visualBearing";
				if (Clip.bVisualBearing)
				{
					std::string NormalizedId;
					if (!Clip.strLegacyEffectAssetId.has_value() ||
						!Clip.strCandidateEffectAssetId.has_value() ||
						!Build_UnifiedCandidateId(*Clip.strLegacyEffectAssetId,
							NormalizedId) ||
						NormalizedId != *Clip.strCandidateEffectAssetId)
					{
						OutError = "Legacy starter clip candidate normalization changed.";
						return false;
					}
					ClipCandidateIds.push_back(*Clip.strCandidateEffectAssetId);
					++CandidateReferences[*Clip.strCandidateEffectAssetId];
					++VisualOccurrences;
				}
				else
				{
					if (Clip.strLegacyEffectAssetId.has_value() ||
						Clip.strCandidateEffectAssetId.has_value())
					{
						OutError = "Silent legacy starter clip unexpectedly has a target.";
						return false;
					}
					++SilentOccurrences;
				}
				Stage.Clips.push_back(std::move(Clip));
			}
			std::vector<std::string> UniqueClipCandidateIds;
			for (const std::string& Id : ClipCandidateIds)
			{
				if (std::ranges::find(UniqueClipCandidateIds, Id) ==
					UniqueClipCandidateIds.end())
				{
					UniqueClipCandidateIds.push_back(Id);
				}
			}
			if (ParsedCandidateIds != UniqueClipCandidateIds ||
				Stage.bEffectBearing != !ParsedCandidateIds.empty())
			{
				OutError = "Legacy starter stage candidate projection changed.";
				return false;
			}
			Stage.CandidateEffectAssetIds = std::move(ParsedCandidateIds);

			const DATA_JSON_VALUE* RolloutStage =
				Find_UniqueObjectByCanonicalSha(*RolloutStages,
					Stage.strRolloutStageCanonicalSha256);
			uint32_t RolloutSkillId = 0u;
			uint32_t RolloutStageIndex = 0u;
			std::string RolloutClass;
			std::string RolloutAnimation;
			std::string RolloutStageId;
			if (nullptr == RolloutStage ||
				!Read_String(*RolloutStage, "characterClass", RolloutClass) ||
				RolloutClass != Stage.strCharacterClass ||
				!Read_String(*RolloutStage, "animationAssetId", RolloutAnimation) ||
				RolloutAnimation != Stage.strAnimationAssetId ||
				!Read_U32(*RolloutStage, "productSkillId", RolloutSkillId) ||
				RolloutSkillId != Stage.iSkillId ||
				!Read_U32(*RolloutStage, "stageIndex", RolloutStageIndex) ||
				RolloutStageIndex != Stage.iStageIndex ||
				!Read_String(*RolloutStage, "stageId", RolloutStageId) ||
				RolloutStageId != Stage.strStageId)
			{
				OutError = "Legacy starter stage no longer joins its rollout record.";
				return false;
			}
			OutBatch.LegacyStarterStages.push_back(std::move(Stage));
		}
		if (SkillIdentities.size() != EXPECTED_LEGACY_STARTER_SKILL_COUNT ||
			EffectBearingStages != EXPECTED_LEGACY_STARTER_EFFECT_STAGE_COUNT ||
			SilentStages != EXPECTED_LEGACY_STARTER_SILENT_STAGE_COUNT ||
			VisualOccurrences != EXPECTED_LEGACY_STARTER_VISUAL_COUNT ||
			SilentOccurrences != EXPECTED_LEGACY_STARTER_SILENT_COUNT ||
			CandidateReferences.size() != EXPECTED_LEGACY_STARTER_CANDIDATE_COUNT ||
			CandidateReferences["effect.artist.skill.31210.ba1.unified"] != 2u ||
			std::ranges::count_if(CandidateReferences,
				[](const auto& Pair) { return Pair.second == 2u; }) != 1u ||
			std::ranges::count_if(CandidateReferences,
				[](const auto& Pair) { return Pair.second == 1u; }) != 87u)
		{
			OutError = "Legacy starter stage/occurrence denominator changed.";
			return false;
		}
		return true;
	}

	bool Parse_LegacyStarterCandidates(const DATA_JSON_VALUE& Value,
		MATERIALIZER_BATCH& OutBatch, std::string& OutError)
	{
		if (!Value.Is_Array() || Value.Get_Array().size() !=
			EXPECTED_LEGACY_STARTER_CANDIDATE_COUNT)
		{
			OutError = "Batch must contain exactly eighty-eight legacy starter candidates.";
			return false;
		}
		const CACHED_JSON_ARTIFACT* Rollout = Find_UniqueArtifactBySchema(
			OutBatch, "lostark.four-class-authored-product-rollout");
		const DATA_JSON_VALUE* ProductTargets = nullptr == Rollout ? nullptr :
			Required(Rollout->Root, "productTargets", DATA_JSON_TYPE::ARRAY);
		if (nullptr == ProductTargets)
		{
			OutError = "Pinned rollout product targets are unavailable.";
			return false;
		}
		std::unordered_map<std::string, std::size_t> StageReferences;
		for (const LEGACY_STARTER_STAGE& Stage : OutBatch.LegacyStarterStages)
		{
			for (const LEGACY_STARTER_CLIP& Clip : Stage.Clips)
			{
				if (Clip.strCandidateEffectAssetId.has_value())
					++StageReferences[*Clip.strCandidateEffectAssetId];
			}
		}

		std::unordered_set<std::string> CandidateIds;
		std::unordered_set<std::string> CandidatePaths;
		std::unordered_set<std::string> LegacyIds;
		std::size_t ActiveReferenceCount = 0u;
		std::size_t OrphanCount = 0u;
		const std::unordered_set<std::string> ExpectedOrphans = {
			"effect.warlord.skill.17820.clip3.unified",
			"effect.warlord.skill.17820.clip4.unified",
			"effect.warlord.skill.17820.clip8.unified"
		};
		for (const DATA_JSON_VALUE& Row : Value.Get_Array())
		{
			if (!Has_ExactKeys(Row, { "candidateKey", "characterClass",
				"animationAssetId", "skillId", "stageIndex", "stageClipIndex",
				"clip", "legacyRollbackBaseline", "starterSource", "target",
				"productReference", "rolloutRecordCanonicalSha256",
				"disposition", "trackAAdmission", "productMutation",
				"visualApproval" }))
			{
				OutError = "Legacy starter candidate has unknown or missing fields.";
				return false;
			}
			LEGACY_STARTER_CANDIDATE Candidate;
			if (!Read_String(Row, "candidateKey", Candidate.strCandidateKey) ||
				!CandidateIds.insert(Candidate.strCandidateKey).second ||
				!Read_String(Row, "characterClass", Candidate.strCharacterClass) ||
				!Read_String(Row, "animationAssetId", Candidate.strAnimationAssetId) ||
				!Read_U32(Row, "skillId", Candidate.iSkillId) ||
				!Read_U32(Row, "stageIndex", Candidate.iStageIndex) ||
				!Read_U32(Row, "stageClipIndex", Candidate.iStageClipIndex) ||
				!Read_String(Row, "clip", Candidate.strClip) ||
				!Read_String(Row, "rolloutRecordCanonicalSha256",
					Candidate.strRolloutRecordCanonicalSha256) ||
				!Is_Sha256(Candidate.strRolloutRecordCanonicalSha256) ||
				!Read_False(Row, "trackAAdmission") ||
				!Read_False(Row, "productMutation") ||
				!Read_False(Row, "visualApproval"))
			{
				OutError = "Legacy starter candidate identity is invalid.";
				return false;
			}

			const DATA_JSON_VALUE* Baseline = Required(Row,
				"legacyRollbackBaseline", DATA_JSON_TYPE::OBJECT);
			std::string BaselinePolicy;
			if (nullptr == Baseline || !Has_ExactKeys(*Baseline,
				{ "path", "rawSha256", "canonicalJsonSha256", "effectAssetId",
				  "authoringVersion", "policy", "rolloutDocumentFileSha256",
				  "rolloutHashDisposition" }) ||
				!Read_String(*Baseline, "path", Candidate.LegacyBaseline.strPath) ||
				!Read_String(*Baseline, "rawSha256",
					Candidate.LegacyBaseline.strRawSha256) ||
				!Read_String(*Baseline, "canonicalJsonSha256",
					Candidate.LegacyBaseline.strCanonicalJsonSha256) ||
				!Read_String(*Baseline, "effectAssetId",
					Candidate.strLegacyEffectAssetId) ||
				!Read_U32(*Baseline, "authoringVersion",
					Candidate.iLegacyAuthoringVersion) ||
				Candidate.iLegacyAuthoringVersion != 12u ||
				!Read_String(*Baseline, "policy", BaselinePolicy) ||
				BaselinePolicy != "IMMUTABLE_LEGACY_ROLLBACK_EXACT" ||
				!Read_String(*Baseline, "rolloutDocumentFileSha256",
					Candidate.strRolloutDocumentFileSha256) ||
				!Read_String(*Baseline, "rolloutHashDisposition",
					Candidate.strRolloutHashDisposition) ||
				!Is_SafeRelativePath(Candidate.LegacyBaseline.strPath) ||
				!Is_Sha256(Candidate.LegacyBaseline.strRawSha256) ||
				!Is_Sha256(Candidate.LegacyBaseline.strCanonicalJsonSha256) ||
				!Is_Sha256(Candidate.strRolloutDocumentFileSha256) ||
				!LegacyIds.insert(Candidate.strLegacyEffectAssetId).second ||
				Candidate.LegacyBaseline.strPath != "Data/Effects/Authored/" +
					Candidate.strLegacyEffectAssetId + ".effect.json" ||
				!Verify_AuthoringArtifactIdentity(OutBatch,
					Candidate.LegacyBaseline, Candidate.strLegacyEffectAssetId,
					Candidate.iLegacyAuthoringVersion, OutError))
			{
				if (OutError.empty())
					OutError = "Legacy rollback baseline contract is invalid.";
				return false;
			}
			const auto LegacyArtifact = OutBatch.Artifacts.find(
				Candidate.LegacyBaseline.strPath);
			const std::string RolloutComparableSha =
				Candidate.strRolloutHashDisposition == "EXACT_RAW" ?
				Candidate.LegacyBaseline.strRawSha256 :
				Candidate.strRolloutHashDisposition == "EOL_NORMALIZED_MATCH" ?
				CEffectRuntimeAuthorityCodec::Compute_Sha256Hex(
					Normalize_CrlfToLf(LegacyArtifact->second->strBytes)) :
				std::string{};
			if (RolloutComparableSha != Candidate.strRolloutDocumentFileSha256)
			{
				OutError = "Legacy rollback rollout hash disposition is invalid.";
				return false;
			}

			const DATA_JSON_VALUE* Source = Required(Row, "starterSource",
				DATA_JSON_TYPE::OBJECT);
			if (nullptr == Source || !Has_ExactKeys(*Source,
				{ "kind", "path", "rawSha256", "canonicalJsonSha256",
				  "effectAssetId", "authoringVersion" }) ||
				!Read_String(*Source, "kind", Candidate.strStarterSourceKind) ||
				!Read_String(*Source, "path", Candidate.StarterSource.strPath) ||
				!Read_String(*Source, "rawSha256",
					Candidate.StarterSource.strRawSha256) ||
				!Read_String(*Source, "canonicalJsonSha256",
					Candidate.StarterSource.strCanonicalJsonSha256) ||
				!Read_String(*Source, "effectAssetId",
					Candidate.strStarterSourceEffectAssetId) ||
				!Read_U32(*Source, "authoringVersion",
					Candidate.iStarterSourceAuthoringVersion) ||
				(Candidate.iStarterSourceAuthoringVersion != 12u &&
				 Candidate.iStarterSourceAuthoringVersion !=
					Client::EFFECT_AUTHORING_FORMAT_VERSION) ||
				!Is_SafeRelativePath(Candidate.StarterSource.strPath) ||
				!Is_Sha256(Candidate.StarterSource.strRawSha256) ||
				!Is_Sha256(Candidate.StarterSource.strCanonicalJsonSha256) ||
				!Verify_AuthoringArtifactIdentity(OutBatch,
					Candidate.StarterSource,
					Candidate.strStarterSourceEffectAssetId,
					Candidate.iStarterSourceAuthoringVersion, OutError))
			{
				if (OutError.empty())
					OutError = "Legacy starter selected source is invalid.";
				return false;
			}

			const DATA_JSON_VALUE* Target = Required(Row, "target",
				DATA_JSON_TYPE::OBJECT);
			uint32_t RequiredVersion = 0u;
			if (nullptr == Target || !Has_ExactKeys(*Target,
				{ "path", "effectAssetId", "requiredOutputVersion",
				  "candidateBaseline" }) ||
				!Read_String(*Target, "path", Candidate.strTargetPath) ||
				!Read_String(*Target, "effectAssetId",
					Candidate.strTargetEffectAssetId) ||
				!Read_U32(*Target, "requiredOutputVersion", RequiredVersion) ||
				RequiredVersion != Client::EFFECT_AUTHORING_FORMAT_VERSION ||
				Candidate.strCandidateKey != Candidate.strTargetEffectAssetId ||
				Candidate.strTargetPath != "Data/Effects/Authored/" +
					Candidate.strTargetEffectAssetId + ".effect.json" ||
				!CandidatePaths.insert(Candidate.strTargetPath).second)
			{
				OutError = "Legacy starter target identity is invalid.";
				return false;
			}
			std::string NormalizedId;
			if (!Build_UnifiedCandidateId(Candidate.strLegacyEffectAssetId,
					NormalizedId) || NormalizedId != Candidate.strTargetEffectAssetId)
			{
				OutError = "Legacy starter target normalization/collision changed.";
				return false;
			}

			const DATA_JSON_VALUE* CandidateBaseline = Required(*Target,
				"candidateBaseline", DATA_JSON_TYPE::OBJECT);
			if (nullptr == CandidateBaseline || !Has_ExactKeys(*CandidateBaseline,
				{ "policy", "expectedRawSha256", "expectedCanonicalJsonSha256",
				  "authoringVersion" }) ||
				!Read_String(*CandidateBaseline, "policy",
					Candidate.strCandidateBaselinePolicy))
			{
				OutError = "Legacy starter candidate baseline is invalid.";
				return false;
			}
			const DATA_JSON_VALUE* ExpectedRaw = CandidateBaseline->Find(
				"expectedRawSha256");
			const DATA_JSON_VALUE* ExpectedCanonical = CandidateBaseline->Find(
				"expectedCanonicalJsonSha256");
			const DATA_JSON_VALUE* ExpectedVersion = CandidateBaseline->Find(
				"authoringVersion");
			if (nullptr == ExpectedRaw || nullptr == ExpectedCanonical ||
				nullptr == ExpectedVersion)
			{
				OutError = "Legacy starter candidate baseline fields are missing.";
				return false;
			}
			if (Candidate.strCandidateBaselinePolicy == "MUST_NOT_EXIST")
			{
				if (!ExpectedRaw->Is_Null() || !ExpectedCanonical->Is_Null() ||
					!ExpectedVersion->Is_Null() ||
					OutBatch.Artifacts.contains(Candidate.strTargetPath))
				{
					OutError = "MUST_NOT_EXIST legacy candidate has a baseline.";
					return false;
				}
			}
			else if (Candidate.strCandidateBaselinePolicy ==
				"EXPECTED_EXACT_OR_REFUSE")
			{
				uint32_t Version = 0u;
				if (!ExpectedRaw->Is_String() || !ExpectedCanonical->Is_String() ||
					!Is_Sha256(ExpectedRaw->Get_String()) ||
					!Is_Sha256(ExpectedCanonical->Get_String()) ||
					!Read_U32(*CandidateBaseline, "authoringVersion", Version) ||
					(Version != 12u && Version !=
						Client::EFFECT_AUTHORING_FORMAT_VERSION))
				{
					OutError = "Existing legacy candidate baseline is invalid.";
					return false;
				}
				Candidate.strCandidateRawSha256 = ExpectedRaw->Get_String();
				Candidate.strCandidateCanonicalJsonSha256 =
					ExpectedCanonical->Get_String();
				Candidate.iCandidateAuthoringVersion = Version;
				FILE_IDENTITY Existing{ Candidate.strTargetPath,
					*Candidate.strCandidateRawSha256,
					*Candidate.strCandidateCanonicalJsonSha256 };
				if (!Verify_AuthoringArtifactIdentity(OutBatch, Existing,
						Candidate.strTargetEffectAssetId, Version, OutError))
				{
					return false;
				}
			}
			else
			{
				OutError = "Legacy starter candidate baseline policy is unknown.";
				return false;
			}

			if (Candidate.strStarterSourceKind == "IMMUTABLE_LEGACY_ROLLBACK")
			{
				if (Candidate.StarterSource.strPath !=
						Candidate.LegacyBaseline.strPath ||
					Candidate.strStarterSourceEffectAssetId !=
						Candidate.strLegacyEffectAssetId ||
					Candidate.iStarterSourceAuthoringVersion != 12u)
				{
					OutError = "Immutable legacy starter source projection changed.";
					return false;
				}
			}
			else if (Candidate.strStarterSourceKind ==
				"EXISTING_UNIFIED_CANDIDATE")
			{
				if (Candidate.strCandidateBaselinePolicy !=
						"EXPECTED_EXACT_OR_REFUSE" ||
					Candidate.StarterSource.strPath != Candidate.strTargetPath ||
					Candidate.strStarterSourceEffectAssetId !=
						Candidate.strTargetEffectAssetId ||
					!Candidate.iCandidateAuthoringVersion.has_value() ||
					Candidate.iStarterSourceAuthoringVersion !=
						*Candidate.iCandidateAuthoringVersion)
				{
					OutError = "Existing unified candidate source projection changed.";
					return false;
				}
			}
			else
			{
				OutError = "Legacy starter selected source kind is unsupported.";
				return false;
			}

			const DATA_JSON_VALUE* ProductReference = Required(Row,
				"productReference", DATA_JSON_TYPE::OBJECT);
			if (nullptr == ProductReference || !Has_ExactKeys(*ProductReference,
				{ "catalogPath", "catalogEntryCanonicalSha256", "animeventPath",
				  "animeventRawSha256", "activeReferenceCount",
				  "orphanedCatalogReference" }) ||
				!Read_String(*ProductReference, "catalogPath",
					Candidate.strCatalogPath) ||
				Candidate.strCatalogPath != "Data/Effects/EffectCatalog.json" ||
				!Read_String(*ProductReference, "catalogEntryCanonicalSha256",
					Candidate.strCatalogEntryCanonicalSha256) ||
				!Is_Sha256(Candidate.strCatalogEntryCanonicalSha256) ||
				!Read_String(*ProductReference, "animeventPath",
					Candidate.strAnimeventPath) ||
				Candidate.strAnimeventPath != "Data/Animation/Authored/" +
					Candidate.strAnimationAssetId + "/" +
					Candidate.strAnimationAssetId + ".animevents" ||
				!Read_String(*ProductReference, "animeventRawSha256",
					Candidate.strAnimeventRawSha256) ||
				!Is_Sha256(Candidate.strAnimeventRawSha256) ||
				!Read_U32(*ProductReference, "activeReferenceCount",
					Candidate.iActiveReferenceCount))
			{
				OutError = "Legacy starter product reference identity is invalid.";
				return false;
			}
			const DATA_JSON_VALUE* Orphaned = Required(*ProductReference,
				"orphanedCatalogReference", DATA_JSON_TYPE::BOOLEAN);
			if (nullptr == Orphaned)
			{
				OutError = "Legacy starter orphan product reference is invalid.";
				return false;
			}
			Candidate.bOrphanedCatalogReference = Orphaned->Get_Boolean();
			if (Candidate.bOrphanedCatalogReference !=
					(0u == Candidate.iActiveReferenceCount) ||
				ExpectedOrphans.contains(Candidate.strCandidateKey) !=
					Candidate.bOrphanedCatalogReference)
			{
				OutError = "Legacy starter orphan set changed.";
				return false;
			}
			ActiveReferenceCount += Candidate.iActiveReferenceCount;
			OrphanCount += Candidate.bOrphanedCatalogReference ? 1u : 0u;

			const auto CatalogArtifact = OutBatch.Artifacts.find(
				Candidate.strCatalogPath);
			const DATA_JSON_VALUE* CatalogEffects =
				CatalogArtifact == OutBatch.Artifacts.end() ? nullptr :
				Required(CatalogArtifact->second->Root, "effects",
					DATA_JSON_TYPE::ARRAY);
			const DATA_JSON_VALUE* CatalogEntry = nullptr;
			if (nullptr != CatalogEffects)
			{
				for (const DATA_JSON_VALUE& Entry : CatalogEffects->Get_Array())
				{
					const DATA_JSON_VALUE* Id = Required(Entry, "effectAssetId",
						DATA_JSON_TYPE::STRING);
					if (nullptr == Id || Id->Get_String() !=
						Candidate.strLegacyEffectAssetId)
					{
						continue;
					}
					if (nullptr != CatalogEntry)
					{
						OutError = "Legacy EffectCatalog ID is duplicated.";
						return false;
					}
					CatalogEntry = &Entry;
				}
			}
			std::string CatalogPath;
			const std::string ExpectedCatalogPath =
				Candidate.LegacyBaseline.strPath.starts_with("Data/") ?
				Candidate.LegacyBaseline.strPath.substr(5u) : std::string{};
			if (nullptr == CatalogEntry ||
				Canonical_JsonSha256(*CatalogEntry) !=
					Candidate.strCatalogEntryCanonicalSha256 ||
				!Read_String(*CatalogEntry, "authoringPath", CatalogPath) ||
				ExpectedCatalogPath.empty() || CatalogPath != ExpectedCatalogPath)
			{
				OutError = "Legacy starter catalog entry changed.";
				return false;
			}

			std::filesystem::path AnimeventPath;
			std::string AnimeventBytes;
			if (!Resolve_ArtifactPath(OutBatch.RepositoryRoot,
					Candidate.strAnimeventPath, AnimeventPath, OutError) ||
				!Read_File(AnimeventPath, AnimeventBytes, OutError) ||
				CEffectRuntimeAuthorityCodec::Compute_Sha256Hex(AnimeventBytes) !=
					Candidate.strAnimeventRawSha256 ||
				Count_AnimeventEffectReferences(AnimeventBytes,
					Candidate.strLegacyEffectAssetId) !=
					Candidate.iActiveReferenceCount ||
				0u != Count_AnimeventEffectReferences(AnimeventBytes,
					Candidate.strTargetEffectAssetId))
			{
				if (OutError.empty())
					OutError = "Legacy starter animevent evidence changed.";
				return false;
			}

			const DATA_JSON_VALUE* RolloutRecord =
				Find_UniqueObjectByCanonicalSha(*ProductTargets,
					Candidate.strRolloutRecordCanonicalSha256);
			std::string RolloutId;
			std::string RolloutPath;
			std::string RolloutClass;
			std::string RolloutAnimation;
			std::string RolloutClip;
			uint32_t RolloutSkillId = 0u;
			uint32_t RolloutStageIndex = 0u;
			uint32_t RolloutClipIndex = 0u;
			if (nullptr == RolloutRecord ||
				!Read_String(*RolloutRecord, "effectAssetId", RolloutId) ||
				RolloutId != Candidate.strLegacyEffectAssetId ||
				!Read_String(*RolloutRecord, "authoringPath", RolloutPath) ||
				RolloutPath != ExpectedCatalogPath ||
				!Read_String(*RolloutRecord, "characterClass", RolloutClass) ||
				RolloutClass != Candidate.strCharacterClass ||
				!Read_String(*RolloutRecord, "animationAssetId", RolloutAnimation) ||
				RolloutAnimation != Candidate.strAnimationAssetId ||
				!Read_U32(*RolloutRecord, "productSkillId", RolloutSkillId) ||
				RolloutSkillId != Candidate.iSkillId ||
				!Read_U32(*RolloutRecord, "stageIndex", RolloutStageIndex) ||
				RolloutStageIndex != Candidate.iStageIndex ||
				!Read_U32(*RolloutRecord, "stageClipIndex", RolloutClipIndex) ||
				RolloutClipIndex != Candidate.iStageClipIndex ||
				!Read_String(*RolloutRecord, "clip", RolloutClip) ||
				RolloutClip != Candidate.strClip ||
				!StageReferences.contains(Candidate.strCandidateKey))
			{
				OutError = "Legacy starter candidate no longer joins rollout/stage evidence.";
				return false;
			}

			const DATA_JSON_VALUE* Disposition = Required(Row, "disposition",
				DATA_JSON_TYPE::OBJECT);
			std::string DispositionKind;
			std::vector<std::string> DispositionBlockers;
			const DATA_JSON_VALUE* DispositionBlockerRows = nullptr == Disposition ?
				nullptr : Required(*Disposition, "blockers", DATA_JSON_TYPE::ARRAY);
			if (nullptr == Disposition || !Has_ExactKeys(*Disposition,
				{ "kind", "blockers" }) ||
				!Read_String(*Disposition, "kind", DispositionKind) ||
				DispositionKind != "LEGACY_TUNING_STARTER" ||
				nullptr == DispositionBlockerRows ||
				!Read_StringArray(*DispositionBlockerRows,
					DispositionBlockers, 8u) || DispositionBlockers.empty())
			{
				OutError = "Legacy starter candidate disposition is invalid.";
				return false;
			}
			OutBatch.LegacyStarterCandidates.push_back(std::move(Candidate));
		}
		if (CandidateIds.size() != EXPECTED_LEGACY_STARTER_CANDIDATE_COUNT ||
			CandidatePaths.size() != EXPECTED_LEGACY_STARTER_CANDIDATE_COUNT ||
			LegacyIds.size() != EXPECTED_LEGACY_STARTER_CANDIDATE_COUNT ||
			StageReferences.size() != EXPECTED_LEGACY_STARTER_CANDIDATE_COUNT ||
			ActiveReferenceCount != EXPECTED_LEGACY_ACTIVE_REFERENCE_COUNT ||
			OrphanCount != EXPECTED_LEGACY_ORPHAN_REFERENCE_COUNT)
		{
			OutError = "Legacy starter candidate/product denominator changed.";
			return false;
		}
		return true;
	}

	bool Validate_FullBatchPartition(const MATERIALIZER_BATCH& Batch,
		std::string& OutError)
	{
		std::unordered_set<std::string> StageKeys;
		std::set<std::pair<std::string, uint32_t>> SkillIdentities;
		std::unordered_set<std::string> CandidateIds;
		std::unordered_set<std::string> CandidatePaths;
		for (const MATERIALIZER_STAGE& Stage : Batch.Stages)
		{
			if (!StageKeys.insert(Stage.strStageKey).second ||
				!CandidateIds.insert(Stage.strTargetEffectAssetId).second ||
				!CandidatePaths.insert(Stage.strTargetPath).second)
			{
				OutError = "Track A/full rollout partition contains a duplicate.";
				return false;
			}
			SkillIdentities.emplace(Stage.strCharacterClass, Stage.iSkillId);
		}
		for (const LEGACY_STARTER_STAGE& Stage : Batch.LegacyStarterStages)
		{
			if (!StageKeys.insert(Stage.strStageKey).second)
			{
				OutError = "Legacy/full rollout stage partition overlaps Track A.";
				return false;
			}
			SkillIdentities.emplace(Stage.strCharacterClass, Stage.iSkillId);
		}
		for (const LEGACY_STARTER_CANDIDATE& Candidate :
			Batch.LegacyStarterCandidates)
		{
			if (!CandidateIds.insert(Candidate.strTargetEffectAssetId).second ||
				!CandidatePaths.insert(Candidate.strTargetPath).second)
			{
				OutError = "Legacy/full rollout candidate partition overlaps Track A.";
				return false;
			}
		}
		if (SkillIdentities.size() != EXPECTED_FULL_SKILL_COUNT ||
			StageKeys.size() != EXPECTED_FULL_STAGE_COUNT ||
			Batch.Stages.size() + EXPECTED_LEGACY_STARTER_OCCURRENCE_COUNT !=
				EXPECTED_FULL_CLIP_OCCURRENCE_COUNT ||
			Batch.Stages.size() + EXPECTED_LEGACY_STARTER_VISUAL_COUNT !=
				EXPECTED_FULL_VISUAL_CLIP_COUNT ||
			EXPECTED_LEGACY_STARTER_SILENT_COUNT !=
				EXPECTED_FULL_SILENT_CLIP_COUNT ||
			CandidateIds.size() != EXPECTED_TOTAL_CANDIDATE_COUNT ||
			CandidatePaths.size() != EXPECTED_TOTAL_CANDIDATE_COUNT ||
			CandidateIds.contains("effect.artist.skill.31470.unified"))
		{
			OutError = "Full four-class rollout partition denominator changed.";
			return false;
		}
		return true;
	}

	bool Validate_FinalBatchContracts(const DATA_JSON_VALUE& Root,
		std::string& OutError)
	{
		const DATA_JSON_VALUE* Denominators = Required(Root, "denominators",
			DATA_JSON_TYPE::OBJECT);
		if (nullptr == Denominators || !Has_ExactKeys(*Denominators,
			{ "stageCount", "trackAProgramStageCount",
			  "failClosedCanaryStageCount", "trackASelectedSourceRowCount",
			  "trackAExcludedSourceRowCount", "supplementalElementCount",
			  "canaryElementCount", "elementPlanCount",
			  "genericParticleCandidateCount", "familyAdapterRequiredCount",
			  "supplementalAdapterPreserveCount", "typedExecutionMaterialCount",
			  "admittedSourceProfileMaterialCount", "failClosedMaterialCount",
			  "productMutationCount" }) ||
			!Read_ExpectedCount(*Denominators, "stageCount", 13u) ||
			!Read_ExpectedCount(*Denominators, "trackAProgramStageCount", 12u) ||
			!Read_ExpectedCount(*Denominators, "failClosedCanaryStageCount", 1u) ||
			!Read_ExpectedCount(*Denominators, "trackASelectedSourceRowCount", 70u) ||
			!Read_ExpectedCount(*Denominators, "trackAExcludedSourceRowCount", 63u) ||
			!Read_ExpectedCount(*Denominators, "supplementalElementCount", 4u) ||
			!Read_ExpectedCount(*Denominators, "canaryElementCount", 5u) ||
			!Read_ExpectedCount(*Denominators, "elementPlanCount", 79u) ||
			!Read_ExpectedCount(*Denominators, "genericParticleCandidateCount", 71u) ||
			!Read_ExpectedCount(*Denominators, "familyAdapterRequiredCount", 4u) ||
			!Read_ExpectedCount(*Denominators, "supplementalAdapterPreserveCount", 4u) ||
			!Read_ExpectedCount(*Denominators, "typedExecutionMaterialCount", 0u) ||
			!Read_ExpectedCount(*Denominators, "admittedSourceProfileMaterialCount", 16u) ||
			!Read_ExpectedCount(*Denominators, "failClosedMaterialCount", 63u) ||
			!Read_ExpectedCount(*Denominators, "productMutationCount", 0u))
		{
			OutError = "Batch denominators changed.";
			return false;
		}

		const DATA_JSON_VALUE* FullDenominators = Required(Root,
			"fullScopeDenominators", DATA_JSON_TYPE::OBJECT);
		if (nullptr == FullDenominators || !Has_ExactKeys(*FullDenominators,
			{ "fullSkillCount", "fullStageCount", "fullClipOccurrenceCount",
			  "fullVisualClipOccurrenceCount",
			  "fullSilentOrNoCarrierClipOccurrenceCount",
			  "fullUniqueCandidateDocumentCount", "trackASeamStageCount",
			  "trackASeamCandidateDocumentCount",
			  "legacyStarterSkillIdentityCount", "legacyStarterStageCount",
			  "legacyStarterEffectBearingStageCount",
			  "legacyStarterIntentionallySilentStageCount",
			  "legacyStarterClipOccurrenceCount",
			  "legacyStarterVisualClipOccurrenceCount",
			  "legacyStarterSilentOrNoCarrierClipOccurrenceCount",
			  "legacyStarterCandidateDocumentCount",
			  "legacyStarterActiveProductReferenceCount",
			  "legacyStarterOrphanedCatalogReferenceCount",
			  "productMutationCount" }) ||
			!Read_ExpectedCount(*FullDenominators, "fullSkillCount",
				EXPECTED_FULL_SKILL_COUNT) ||
			!Read_ExpectedCount(*FullDenominators, "fullStageCount",
				EXPECTED_FULL_STAGE_COUNT) ||
			!Read_ExpectedCount(*FullDenominators, "fullClipOccurrenceCount",
				EXPECTED_FULL_CLIP_OCCURRENCE_COUNT) ||
			!Read_ExpectedCount(*FullDenominators,
				"fullVisualClipOccurrenceCount", EXPECTED_FULL_VISUAL_CLIP_COUNT) ||
			!Read_ExpectedCount(*FullDenominators,
				"fullSilentOrNoCarrierClipOccurrenceCount",
				EXPECTED_FULL_SILENT_CLIP_COUNT) ||
			!Read_ExpectedCount(*FullDenominators,
				"fullUniqueCandidateDocumentCount", EXPECTED_TOTAL_CANDIDATE_COUNT) ||
			!Read_ExpectedCount(*FullDenominators, "trackASeamStageCount",
				EXPECTED_STAGE_COUNT) ||
			!Read_ExpectedCount(*FullDenominators,
				"trackASeamCandidateDocumentCount", EXPECTED_STAGE_COUNT) ||
			!Read_ExpectedCount(*FullDenominators,
				"legacyStarterSkillIdentityCount", EXPECTED_LEGACY_STARTER_SKILL_COUNT) ||
			!Read_ExpectedCount(*FullDenominators, "legacyStarterStageCount",
				EXPECTED_LEGACY_STARTER_STAGE_COUNT) ||
			!Read_ExpectedCount(*FullDenominators,
				"legacyStarterEffectBearingStageCount",
				EXPECTED_LEGACY_STARTER_EFFECT_STAGE_COUNT) ||
			!Read_ExpectedCount(*FullDenominators,
				"legacyStarterIntentionallySilentStageCount",
				EXPECTED_LEGACY_STARTER_SILENT_STAGE_COUNT) ||
			!Read_ExpectedCount(*FullDenominators,
				"legacyStarterClipOccurrenceCount",
				EXPECTED_LEGACY_STARTER_OCCURRENCE_COUNT) ||
			!Read_ExpectedCount(*FullDenominators,
				"legacyStarterVisualClipOccurrenceCount",
				EXPECTED_LEGACY_STARTER_VISUAL_COUNT) ||
			!Read_ExpectedCount(*FullDenominators,
				"legacyStarterSilentOrNoCarrierClipOccurrenceCount",
				EXPECTED_LEGACY_STARTER_SILENT_COUNT) ||
			!Read_ExpectedCount(*FullDenominators,
				"legacyStarterCandidateDocumentCount",
				EXPECTED_LEGACY_STARTER_CANDIDATE_COUNT) ||
			!Read_ExpectedCount(*FullDenominators,
				"legacyStarterActiveProductReferenceCount",
				EXPECTED_LEGACY_ACTIVE_REFERENCE_COUNT) ||
			!Read_ExpectedCount(*FullDenominators,
				"legacyStarterOrphanedCatalogReferenceCount",
				EXPECTED_LEGACY_ORPHAN_REFERENCE_COUNT) ||
			!Read_ExpectedCount(*FullDenominators, "productMutationCount", 0u))
		{
			OutError = "Full four-class batch denominators changed.";
			return false;
		}

		const DATA_JSON_VALUE* MaterialCounts = Required(Root,
			"materialDispositionCounts", DATA_JSON_TYPE::OBJECT);
		const DATA_JSON_VALUE* CarrierCounts = Required(Root,
			"carrierDispositionCounts", DATA_JSON_TYPE::OBJECT);
		if (nullptr == MaterialCounts || !Has_ExactKeys(*MaterialCounts,
			{ "ADMITTED_SOURCE_PROFILE", "FAIL_CLOSED", "TYPED_EXECUTION" }) ||
			!Read_ExpectedCount(*MaterialCounts, "ADMITTED_SOURCE_PROFILE", 16u) ||
			!Read_ExpectedCount(*MaterialCounts, "FAIL_CLOSED", 63u) ||
			!Read_ExpectedCount(*MaterialCounts, "TYPED_EXECUTION", 0u) ||
			nullptr == CarrierCounts || !Has_ExactKeys(*CarrierCounts,
			{ "FAMILY_ADAPTER_REQUIRED", "GENERIC_PARTICLE_IMPORT_CANDIDATE",
			  "SUPPLEMENTAL_ADAPTER_PRESERVE" }) ||
			!Read_ExpectedCount(*CarrierCounts, "FAMILY_ADAPTER_REQUIRED", 4u) ||
			!Read_ExpectedCount(*CarrierCounts,
				"GENERIC_PARTICLE_IMPORT_CANDIDATE", 71u) ||
			!Read_ExpectedCount(*CarrierCounts,
				"SUPPLEMENTAL_ADAPTER_PRESERVE", 4u))
		{
			OutError = "Batch top-level disposition counts changed.";
			return false;
		}

		const DATA_JSON_VALUE* Admission = Required(Root, "admission",
			DATA_JSON_TYPE::OBJECT);
		if (nullptr == Admission || !Has_ExactKeys(*Admission,
			{ "offlineBatchPlanning", "allElementsMaterialAdmitted",
			  "allElementsGenericCarrierAdmitted", "productMappingMutation",
			  "visualApproval", "blockers" }) ||
			!Read_True(*Admission, "offlineBatchPlanning") ||
			!Read_False(*Admission, "allElementsMaterialAdmitted") ||
			!Read_False(*Admission, "allElementsGenericCarrierAdmitted") ||
			!Read_False(*Admission, "productMappingMutation") ||
			!Read_False(*Admission, "visualApproval") ||
			nullptr == Admission->Find("blockers") ||
			!Parse_Blockers(*Admission->Find("blockers")))
		{
			OutError = "Batch admission boundary is invalid.";
			return false;
		}

		const DATA_JSON_VALUE* Transaction = Required(Root, "transactionPolicy",
			DATA_JSON_TYPE::OBJECT);
		std::string CommitMode;
		std::string FailureAction;
		std::string BaselinePolicy;
		if (nullptr == Transaction || !Has_ExactKeys(*Transaction,
			{ "loadOrder", "commitMode", "failureAction",
			  "candidateBaselinePolicy", "authoredDocumentMutation",
			  "catalogMutation", "animationEventMutation" }) ||
			!Read_String(*Transaction, "commitMode", CommitMode) ||
			CommitMode !=
				"PARSE_VALIDATE_STAGE_THEN_CREATE_OR_EXACT_REPLACE_CANDIDATES_ONLY" ||
			!Read_String(*Transaction, "failureAction", FailureAction) ||
			FailureAction !=
				"PRESERVE_PREVIOUS_BATCH_LEGACY_BASELINES_AND_CANDIDATES" ||
			!Read_String(*Transaction, "candidateBaselinePolicy", BaselinePolicy) ||
			BaselinePolicy != "MUST_NOT_EXIST_OR_EXPECTED_EXACT_OR_REFUSE" ||
			!Read_False(*Transaction, "authoredDocumentMutation") ||
			!Read_False(*Transaction, "catalogMutation") ||
			!Read_False(*Transaction, "animationEventMutation"))
		{
			OutError = "Batch transaction policy is invalid.";
			return false;
		}
		const DATA_JSON_VALUE* LoadOrder = Required(*Transaction, "loadOrder",
			DATA_JSON_TYPE::ARRAY);
		const std::array<std::string_view, 11u> ExpectedLoadOrder = {
			"schema", "visual-program-runtime", "source-admission",
			"source-manifests", "source-receipts-and-documents",
			"selection-receipts", "legacy-rollback-baselines",
			"four-class-product-rollout",
			"effect-catalog-and-animevent-mapping-evidence",
			"legacy-starter-sources",
			"unified-candidate-baselines-if-present"
		};
		if (nullptr == LoadOrder ||
			LoadOrder->Get_Array().size() != ExpectedLoadOrder.size())
		{
			OutError = "Batch transaction load order is invalid.";
			return false;
		}
		for (std::size_t Index = 0u; Index < ExpectedLoadOrder.size(); ++Index)
		{
			if (!LoadOrder->Get_Array()[Index].Is_String() ||
				LoadOrder->Get_Array()[Index].Get_String() != ExpectedLoadOrder[Index])
			{
				OutError = "Batch transaction load order changed.";
				return false;
			}
		}
		return true;
	}

	bool Parse_MaterializerBatch(const std::filesystem::path& BatchPath,
		MATERIALIZER_BATCH& OutBatch, std::string& OutError)
	{
		OutBatch = {};
		if (!Resolve_RepositoryRoot(BatchPath, OutBatch.RepositoryRoot, OutError))
			return false;
		std::string BatchBytes;
		DATA_JSON_VALUE Root;
		if (!Read_File(BatchPath, BatchBytes, OutError) ||
			!Parse_Json(BatchBytes, Root, OutError) ||
			!Validate_BatchEnvelope(Root, OutBatch, OutError))
		{
			return false;
		}
		const DATA_JSON_VALUE* Stages = Required(Root, "stages",
			DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* Plans = Required(Root, "elementPlans",
			DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* LegacyStages = Required(Root,
			"legacyStarterStages", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* LegacyCandidates = Required(Root,
			"legacyStarterCandidates", DATA_JSON_TYPE::ARRAY);
		return nullptr != Stages && nullptr != Plans &&
			nullptr != LegacyStages && nullptr != LegacyCandidates &&
			Parse_Stages(*Stages, OutBatch, OutError) &&
			Parse_AndJoinPlans(*Plans, OutBatch, OutError) &&
			Parse_LegacyStarterStages(*LegacyStages, OutBatch, OutError) &&
			Parse_LegacyStarterCandidates(*LegacyCandidates, OutBatch, OutError) &&
			Validate_FullBatchPartition(OutBatch, OutError) &&
			Validate_FinalBatchContracts(Root, OutError);
	}

	const DATA_JSON_VALUE* Find_UniqueObjectByString(
		const DATA_JSON_VALUE& Array, const std::string_view Field,
		const std::string_view Expected)
	{
		if (!Array.Is_Array())
			return nullptr;
		const DATA_JSON_VALUE* Found = nullptr;
		for (const DATA_JSON_VALUE& Row : Array.Get_Array())
		{
			const DATA_JSON_VALUE* Value = Row.Find(Field);
			if (!Row.Is_Object() || nullptr == Value || !Value->Is_String() ||
				Value->Get_String() != Expected)
			{
				continue;
			}
			if (nullptr != Found)
				return nullptr;
			Found = &Row;
		}
		return Found;
	}

	const DATA_JSON_VALUE* Find_VisualRecordByOccurrence(
		const DATA_JSON_VALUE& Program, const std::string_view SectionName,
		const std::string_view OccurrenceId)
	{
		const DATA_JSON_VALUE* Section = Required(Program, SectionName,
			DATA_JSON_TYPE::ARRAY);
		if (nullptr == Section)
			return nullptr;
		const DATA_JSON_VALUE* Found = nullptr;
		for (const DATA_JSON_VALUE& Row : Section->Get_Array())
		{
			const DATA_JSON_VALUE* Selector = Required(Row, "selector",
				DATA_JSON_TYPE::OBJECT);
			const DATA_JSON_VALUE* Value = nullptr == Selector ? nullptr :
				Required(*Selector, "occurrenceId", DATA_JSON_TYPE::STRING);
			if (nullptr == Value || Value->Get_String() != OccurrenceId)
				continue;
			if (nullptr != Found)
				return nullptr;
			Found = &Row;
		}
		return Found;
	}

	const EFFECT_ELEMENT_DESC* Find_ExactElement(
		const EFFECT_DOCUMENT_DESC& Document, const std::string_view ElementId)
	{
		const EFFECT_ELEMENT_DESC* Found = nullptr;
		for (const EFFECT_ELEMENT_DESC& Element : Document.Elements)
		{
			if (Element.strElementId != ElementId)
				continue;
			if (nullptr != Found)
				return nullptr;
			Found = &Element;
		}
		return Found;
	}

	EFFECT_ELEMENT_DESC* Find_ExactElement(EFFECT_DOCUMENT_DESC& Document,
		const std::string_view ElementId)
	{
		EFFECT_ELEMENT_DESC* Found = nullptr;
		for (EFFECT_ELEMENT_DESC& Element : Document.Elements)
		{
			if (Element.strElementId != ElementId)
				continue;
			if (nullptr != Found)
				return nullptr;
			Found = &Element;
		}
		return Found;
	}

	bool Verify_TargetElementEvidence(const MATERIALIZER_ELEMENT_PLAN& Plan,
		const DATA_JSON_VALUE& BlueprintDocument,
		const DATA_JSON_VALUE& BaselineDocument,
		const EFFECT_DOCUMENT_DESC& TypedBlueprint, std::string& OutError)
	{
		const DATA_JSON_VALUE* BlueprintElements = Required(BlueprintDocument,
			"elements", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* BaselineElements = Required(BaselineDocument,
			"elements", DATA_JSON_TYPE::ARRAY);
		if (nullptr == BlueprintElements || nullptr == BaselineElements)
		{
			OutError = "Target blueprint/baseline Element array is missing.";
			return false;
		}
		const DATA_JSON_VALUE* Blueprint = Find_UniqueObjectByString(
			*BlueprintElements, "id", Plan.strTargetElementId);
		const DATA_JSON_VALUE* Baseline = Find_UniqueObjectByString(
			*BaselineElements, "id", Plan.strTargetElementId);
		const EFFECT_ELEMENT_DESC* Typed = Find_ExactElement(TypedBlueprint,
			Plan.strTargetElementId);
		const DATA_JSON_VALUE* Detail = nullptr == Blueprint ? nullptr :
			Required(*Blueprint, "detail", DATA_JSON_TYPE::OBJECT);
		std::string GroupId;
		std::string DisplayName;
		if (nullptr == Blueprint || nullptr == Typed || nullptr == Detail ||
			!Read_String(*Blueprint, "groupId", GroupId) ||
			!Read_String(*Blueprint, "displayName", DisplayName) ||
			GroupId != Plan.strTargetGroupId ||
			DisplayName != Plan.strTargetDisplayName ||
			Typed->strGroupId != Plan.strTargetGroupId ||
			Typed->strDisplayName != Plan.strTargetDisplayName ||
			Canonical_JsonSha256(*Blueprint) !=
				Plan.strBlueprintElementCanonicalSha256 ||
			Canonical_JsonSha256(*Detail) !=
				Plan.strTargetDetailCanonicalSha256)
		{
			OutError = "Target blueprint Element evidence drifted: " +
				Plan.strTargetElementId;
			return false;
		}
		if (Plan.strBaselineElementCanonicalSha256.has_value())
		{
			if (nullptr == Baseline || Canonical_JsonSha256(*Baseline) !=
				*Plan.strBaselineElementCanonicalSha256)
			{
				OutError = "Target baseline Element evidence drifted: " +
					Plan.strTargetElementId;
				return false;
			}
		}
		else if (nullptr != Baseline)
		{
			OutError = "Target baseline unexpectedly gained supplemental Element: " +
				Plan.strTargetElementId;
			return false;
		}
		return true;
	}

	bool Verify_ImportedElementEvidence(const MATERIALIZER_ELEMENT_PLAN& Plan,
		const DATA_JSON_VALUE& SourceDocument,
		const EFFECT_DOCUMENT_DESC& TypedSource, std::string& OutError)
	{
		const DATA_JSON_VALUE* Elements = Required(SourceDocument, "elements",
			DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* Source = nullptr == Elements ? nullptr :
			Find_UniqueObjectByString(*Elements, "id", Plan.strSourceElementId);
		const EFFECT_ELEMENT_DESC* Typed = Find_ExactElement(TypedSource,
			Plan.strSourceElementId);
		const DATA_JSON_VALUE* Recipe = nullptr == Source ? nullptr :
			Required(*Source, "sourceRecipe", DATA_JSON_TYPE::OBJECT);
		const DATA_JSON_VALUE* Detail = nullptr == Source ? nullptr :
			Required(*Source, "detail", DATA_JSON_TYPE::OBJECT);
		const DATA_JSON_VALUE* Attachment = nullptr == Source ? nullptr :
			Required(*Source, "actionCueAttachment", DATA_JSON_TYPE::OBJECT);
		const DATA_JSON_VALUE* Kind = nullptr == Source ? nullptr :
			Required(*Source, "kind", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* RecipeEnabled = nullptr == Recipe ? nullptr :
			Required(*Recipe, "enabled", DATA_JSON_TYPE::BOOLEAN);
		if (nullptr == Source || nullptr == Typed || nullptr == Recipe ||
			nullptr == Detail || nullptr == Attachment || nullptr == Kind ||
			Kind->Get_String() != "particle" || nullptr == RecipeEnabled ||
			!RecipeEnabled->Get_Boolean() || !Typed->SourceRecipe.bEnabled ||
			Canonical_JsonSha256(*Source) !=
				Plan.strSourceElementCanonicalSha256 ||
			Canonical_JsonSha256(*Recipe) !=
				Plan.strSourceRecipeCanonicalSha256 ||
			Canonical_JsonSha256(*Detail) !=
				Plan.strSourceDetailCanonicalSha256 ||
			Canonical_JsonSha256(*Attachment) !=
				Plan.strSourceAttachmentCanonicalSha256)
		{
			OutError = "Imported source Element evidence drifted: " +
				Plan.strSourceElementId;
			return false;
		}
		return true;
	}

	bool Verify_SelectionEvidence(const MATERIALIZER_STAGE& Stage,
		const MATERIALIZER_ELEMENT_PLAN& Plan,
		const DATA_JSON_VALUE* VisualProgram, std::string& OutError)
	{
		if (Stage.strMode == "WARLORD_FAIL_CLOSED_CANARY")
		{
			if (Plan.strSelectionRecordId != Plan.strSourceElementId ||
				Plan.strSelectionRecordSha256 !=
					Plan.strSourceElementCanonicalSha256)
			{
				OutError = "Warlord canary plan selection is not its exact source Element.";
				return false;
			}
			return true;
		}
		if (nullptr == VisualProgram)
		{
			OutError = "Track A plan is missing its visual program.";
			return false;
		}
		const bool bSupplemental = Plan.eCarrier ==
			CARRIER_DISPOSITION::SUPPLEMENTAL_ADAPTER_PRESERVE;
		const DATA_JSON_VALUE* Record = Find_VisualRecordByOccurrence(
			*VisualProgram, bSupplemental ? "supplementalElements" : "visualRows",
			Plan.strSelectionRecordId);
		const DATA_JSON_VALUE* RowSha = nullptr == Record ? nullptr :
			Required(*Record, "rowSha256", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* Family = nullptr == Record ? nullptr :
			Required(*Record, "family", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* TargetIdentity = nullptr == Record ? nullptr :
			Required(*Record, "targetIdentity", DATA_JSON_TYPE::OBJECT);
		const DATA_JSON_VALUE* TargetId = nullptr == TargetIdentity ? nullptr :
			Required(*TargetIdentity, "targetElementId", DATA_JSON_TYPE::STRING);
		if (nullptr == Record || nullptr == RowSha || nullptr == Family ||
			nullptr == TargetId || RowSha->Get_String() !=
				Plan.strSelectionRecordSha256 ||
			Family->Get_String() != Plan.strFamily ||
			TargetId->Get_String() != Plan.strTargetElementId)
		{
			OutError = "Track A plan selection row drifted: " +
				Plan.strSelectionRecordId;
			return false;
		}
		if (bSupplemental)
		{
			const DATA_JSON_VALUE* SourceIdentity = Required(*Record,
				"sourceIdentity", DATA_JSON_TYPE::OBJECT);
			const DATA_JSON_VALUE* SourceRecordId = nullptr == SourceIdentity ?
				nullptr : Required(*SourceIdentity, "sourceRecordId",
					DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* SourceRecordSha = nullptr == SourceIdentity ?
				nullptr : Required(*SourceIdentity, "sourceRecordSha256",
					DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* Schedule = Required(*Record, "schedule",
				DATA_JSON_TYPE::OBJECT);
			const DATA_JSON_VALUE* EventId = nullptr == Schedule ? nullptr :
				Required(*Schedule, "sourceEventId", DATA_JSON_TYPE::STRING);
			if (nullptr == SourceRecordId || nullptr == SourceRecordSha ||
				nullptr == EventId || SourceRecordId->Get_String() !=
					Plan.strSupplementalSourceRecordId ||
				SourceRecordSha->Get_String() !=
					Plan.strSupplementalSourceRecordSha256 ||
				EventId->Get_String() != Plan.strSourceEventId)
			{
				OutError = "Track A supplemental source identity drifted.";
				return false;
			}
		}
		return true;
	}

	bool Verify_WarlordCanarySelection(const MATERIALIZER_STAGE& Stage,
		const DATA_JSON_VALUE& VisualRoot, std::string& OutError)
	{
		const DATA_JSON_VALUE* Canaries = Required(VisualRoot,
			"extensionCanaries", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* Canary = nullptr == Canaries ? nullptr :
			Find_UniqueObjectByString(*Canaries, "canaryId",
				Stage.strSelectionRecordId);
		const DATA_JSON_VALUE* Sha = nullptr == Canary ? nullptr :
			Required(*Canary, "canarySha256", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* Domain = nullptr == Canary ? nullptr :
			Required(*Canary, "domain", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* Disposition = nullptr == Canary ? nullptr :
			Required(*Canary, "disposition", DATA_JSON_TYPE::STRING);
		if (nullptr == Sha || nullptr == Domain || nullptr == Disposition ||
			Sha->Get_String() != Stage.strSelectionRecordSha256 ||
			Domain->Get_String() != "WARLORD" ||
			Disposition->Get_String() != "FAIL_CLOSED")
		{
			OutError = "Warlord visual-program extension canary drifted.";
			return false;
		}
		return true;
	}

	bool Build_FollowParentLocalMatrix(
		const Client::EFFECT_TRANSFORM_DESC& Transform,
		float4x4_t& OutMatrix)
	{
		if (!std::isfinite(Transform.vPosition.x) ||
			!std::isfinite(Transform.vPosition.y) ||
			!std::isfinite(Transform.vPosition.z) ||
			!std::isfinite(Transform.vRotationDegrees.x) ||
			!std::isfinite(Transform.vRotationDegrees.y) ||
			!std::isfinite(Transform.vRotationDegrees.z) ||
			!std::isfinite(Transform.vScale.x) ||
			!std::isfinite(Transform.vScale.y) ||
			!std::isfinite(Transform.vScale.z) || Transform.vScale.x <= 0.f ||
			Transform.vScale.y <= 0.f || Transform.vScale.z <= 0.f)
		{
			return false;
		}
		const DirectX::XMMATRIX Matrix = DirectX::XMMatrixScaling(
			Transform.vScale.x, Transform.vScale.y, Transform.vScale.z) *
			DirectX::XMMatrixRotationRollPitchYaw(
				DirectX::XMConvertToRadians(Transform.vRotationDegrees.x),
				DirectX::XMConvertToRadians(Transform.vRotationDegrees.y),
				DirectX::XMConvertToRadians(Transform.vRotationDegrees.z)) *
			DirectX::XMMatrixTranslation(Transform.vPosition.x,
				Transform.vPosition.y, Transform.vPosition.z);
		DirectX::XMStoreFloat4x4(&OutMatrix, Matrix);
		return true;
	}

	bool Build_GenericImportValidationStage(
		const EFFECT_DOCUMENT_DESC& SourceDocument,
		const MATERIALIZER_ELEMENT_PLAN& Plan,
		const EFFECT_ELEMENT_DESC& TargetBlueprintElement,
		EFFECT_DOCUMENT_DESC& OutImportedDocument, std::string& OutError)
	{
		EFFECT_DOCUMENT_DESC SeamSource = SourceDocument;
		EFFECT_ELEMENT_DESC* MutableSource = Find_ExactElement(SeamSource,
			Plan.strSourceElementId);
		if (nullptr == MutableSource || !MutableSource->SourceRecipe.bEnabled)
		{
			OutError = "Generic import source Element is missing or not executable.";
			return false;
		}
		/* Every source DDS/WModel lane is compiler-owned and wins by stable slot.
		   Some authenticated emitters intentionally have only mask/noise bindings
		   and cannot form an ordinary drawable Element by themselves, so retain a
		   reviewed target lane only when the source has no value for that slot.  A
		   later source revision therefore promotes itself automatically instead of
		   being hidden by the starter projection. */
		std::vector<EFFECT_RESOURCE_BINDING_DESC> MergedBindings =
			TargetBlueprintElement.ResourceBindings;
		for (const EFFECT_RESOURCE_BINDING_DESC& SourceBinding :
			MutableSource->ResourceBindings)
		{
			auto Existing = std::find_if(MergedBindings.begin(), MergedBindings.end(),
				[&SourceBinding](const EFFECT_RESOURCE_BINDING_DESC& Binding)
				{
					return Binding.strSlotId == SourceBinding.strSlotId;
				});
			if (Existing == MergedBindings.end())
				MergedBindings.push_back(SourceBinding);
			else
				*Existing = SourceBinding;
		}
		MutableSource->ResourceBindings = std::move(MergedBindings);
		MutableSource->Material = TargetBlueprintElement.Material;
		const EFFECT_ELEMENT_DESC* Source = MutableSource;

		EFFECT_DOCUMENT_DESC Seed;
		if (!CEffectDocumentCodec::Build_GenericAuthoredElementStartingCopy(
				SeamSource, Plan.strSourceElementId,
				Plan.strTargetEffectAssetId, Seed, OutError) ||
			Seed.Elements.size() != 1u)
		{
			return false;
		}
		Seed.iFormatVersion = Client::EFFECT_AUTHORING_FORMAT_VERSION;
		Seed.iLoadedFormatVersion = Client::EFFECT_AUTHORING_FORMAT_VERSION;
		Seed.bSourceContract = false;
		Seed.strDisplayName = "Four-class import transaction seed";
		Seed.Elements.front().strElementId = "materializer.stage.seed";
		Seed.Elements.front().strGroupId = "manual.materializer";
		Seed.Elements.front().strDisplayName = "materializer-stage-seed";
		Seed.Elements.front().TransformInheritance = {};
		const std::string SeedCanonical = CEffectDocumentCodec::Serialize(Seed);
		EFFECT_DOCUMENT_DESC CanonicalSeed;
		if (!CEffectDocumentCodec::Parse(SeedCanonical, CanonicalSeed, OutError) ||
			!CEffectDocumentCodec::Validate_Drawable(CanonicalSeed, OutError) ||
			CEffectDocumentCodec::Serialize(CanonicalSeed) != SeedCanonical)
		{
			if (OutError.empty())
				OutError = "Generic import seed did not survive canonical validation.";
			return false;
		}

		EFFECT_GENERIC_AUTHORED_ELEMENT_IMPORT_REQUEST Request;
		Request.strSourceElementId = Plan.strSourceElementId;
		Request.strTargetElementId = Plan.strTargetElementId;
		Request.strTargetGroupId = Plan.strTargetGroupId;
		Request.strTargetDisplayName = Plan.strTargetDisplayName;
		Request.StartingState.fScheduleStartDelaySeconds =
			Source->Detail.Timing.fStartDelaySeconds;
		Request.StartingState.fScheduleLifeTimeSeconds =
			Source->Detail.Timing.fLifeTimeSeconds;
		Request.StartingState.fEmitterDelaySeconds =
			Source->SourceRecipe.fEmitterDelaySeconds;
		Request.StartingState.fEmitterDurationSeconds =
			Source->SourceRecipe.fEmitterDurationSeconds;
		Request.StartingState.iEmitterLoopCount =
			Source->SourceRecipe.iEmitterLoopCount;
		Request.StartingState.bAttachmentEnabled =
			Source->ActionCueAttachment.bEnabled;
		Request.StartingState.bFollowAttachment =
			Source->ActionCueAttachment.bFollow;
		Request.StartingState.fSnapshotRootSourceBasisYawDegrees =
			Source->ActionCueAttachment.fSnapshotRootSourceBasisYawDegrees;
		Request.StartingState.vSourceTypeDataRotationDegrees =
			Source->Detail.Mesh.vSourceTypeDataRotationDegrees;
		Request.StartingState.bTransformInheritanceEnabled =
			Source->TransformInheritance.bEnabled;
		if (Request.StartingState.bFollowAttachment)
		{
			/* Imported v12 stores the stable socket suffix but not an animation
			   sample.  These rows are all fail-closed and never commit their
			   validation carrier; use the only persisted parent-local suffix to
			   exercise the generic seam without inventing a product mapping. */
			Request.StartingState.bHasFollowParentLocalTransform = true;
			if (!Build_FollowParentLocalMatrix(
					Source->ActionCueAttachment.SocketLocalTransform,
					Request.StartingState.FollowParentLocalTransform))
			{
				OutError = "Generic import follow attachment has no finite persisted suffix.";
				return false;
			}
		}

		return CEffectDocumentCodec::Build_GenericAuthoredElementImportStage(
			SeamSource, CanonicalSeed, Request, OutImportedDocument, OutError);
	}

	void Isolate_UnadmittedPreservedElement(
		EFFECT_ELEMENT_DESC& InOutElement, const bool bKeepAdapterCarrier)
	{
		InOutElement.strSourceNode.clear();
		InOutElement.Renderer = {};
		InOutElement.Material.SourceMaterial = {};
		InOutElement.Material.Execution = {};
		InOutElement.SourcePresentation = {};
		InOutElement.Detail.Mesh.vSourceTypeDataRotationDegrees = {};
		if (!bKeepAdapterCarrier)
			InOutElement.SourceRecipe = {};
	}

	bool Validate_IsolatedOrdinaryElement(
		const EFFECT_DOCUMENT_DESC& Parent,
		const EFFECT_ELEMENT_DESC& Element, std::string& OutError)
	{
		EFFECT_DOCUMENT_DESC Probe = Parent;
		Probe.Elements = { Element };
		/* Probe only the Element's self-contained drawable lanes.  A retained
		   inheritance edge may validly resolve to another accepted Element in the
		   complete document and is validated again at the final document gate. */
		Probe.Elements.front().TransformInheritance = {};
		const std::string Canonical = CEffectDocumentCodec::Serialize(Probe);
		EFFECT_DOCUMENT_DESC Reloaded;
		return CEffectDocumentCodec::Parse(Canonical, Reloaded, OutError) &&
			CEffectDocumentCodec::Validate_Drawable(Reloaded, OutError) &&
			CEffectDocumentCodec::Serialize(Reloaded) == Canonical;
	}

	bool Verify_GenericReimportRollbackProbes(
		const EFFECT_DOCUMENT_DESC& ImportedDocument,
		const EFFECT_DOCUMENT_DESC& ExistingDocument,
		const EFFECT_GENERIC_AUTHORED_ELEMENT_REIMPORT_REQUEST& Request,
		std::string& OutError)
	{
		const std::string ExistingCanonical =
			CEffectDocumentCodec::Serialize(ExistingDocument);
		const auto RejectsWithoutCommit = [&](const EFFECT_DOCUMENT_DESC& Compiler,
			const EFFECT_GENERIC_AUTHORED_ELEMENT_REIMPORT_REQUEST& ProbeRequest,
			const std::string_view Label)
		{
			EFFECT_DOCUMENT_DESC Sentinel = ExistingDocument;
			std::string ExpectedError;
			if (CEffectDocumentCodec::Build_GenericAuthoredElementReimportStage(
					Compiler, ExistingDocument, ProbeRequest, Sentinel,
					ExpectedError) || ExpectedError.empty() ||
				CEffectDocumentCodec::Serialize(Sentinel) != ExistingCanonical)
			{
				OutError = "Field-aware reimport rollback probe failed: " +
					std::string(Label) + ".";
				return false;
			}
			return true;
		};

		EFFECT_GENERIC_AUTHORED_ELEMENT_REIMPORT_REQUEST MissingRequest = Request;
		MissingRequest.strElementId += ".missing";
		if (!RejectsWithoutCommit(
				ImportedDocument, MissingRequest, "missing stable join"))
		{
			return false;
		}

		const EFFECT_ELEMENT_DESC* Imported = Find_ExactElement(
			ImportedDocument, Request.strElementId);
		if (nullptr == Imported || Imported->eKind !=
				Client::EFFECT_ELEMENT_KIND::PARTICLE)
		{
			OutError = "Field-aware reimport rollback probe lost its Particle carrier.";
			return false;
		}
		const std::string RequiredSlot =
			Imported->SourceRecipe.strRendererShape == "mesh" ?
				std::string(Client::EFFECT_MESH_SHAPE_SLOT_ID) : "base";

		EFFECT_DOCUMENT_DESC MissingBinding = ImportedDocument;
		EFFECT_ELEMENT_DESC* MissingElement = Find_ExactElement(
			MissingBinding, Request.strElementId);
		if (nullptr == MissingElement)
		{
			OutError = "Field-aware reimport rollback probe lost its target copy.";
			return false;
		}
		std::erase_if(MissingElement->ResourceBindings,
			[&RequiredSlot](const Client::EFFECT_RESOURCE_BINDING_DESC& Binding)
			{
				return Binding.strSlotId == RequiredSlot;
			});
		if (!RejectsWithoutCommit(
				MissingBinding, Request, "missing compiler resource binding"))
		{
			return false;
		}

		EFFECT_DOCUMENT_DESC UnsafeBinding = ImportedDocument;
		EFFECT_ELEMENT_DESC* UnsafeElement = Find_ExactElement(
			UnsafeBinding, Request.strElementId);
		if (nullptr == UnsafeElement)
		{
			OutError = "Field-aware reimport rollback probe has no required resource.";
			return false;
		}
		auto Unsafe = std::find_if(UnsafeElement->ResourceBindings.begin(),
			UnsafeElement->ResourceBindings.end(),
			[&RequiredSlot](const Client::EFFECT_RESOURCE_BINDING_DESC& Binding)
			{
				return Binding.strSlotId == RequiredSlot;
			});
		if (Unsafe == UnsafeElement->ResourceBindings.end())
		{
			OutError = "Field-aware reimport rollback probe has no required resource.";
			return false;
		}
		Unsafe->strAssetId = "../unsafe.dds";
		if (!RejectsWithoutCommit(
				UnsafeBinding, Request, "unsafe compiler resource binding"))
		{
			return false;
		}

		EFFECT_DOCUMENT_DESC AmbiguousBinding = ImportedDocument;
		EFFECT_ELEMENT_DESC* AmbiguousElement = Find_ExactElement(
			AmbiguousBinding, Request.strElementId);
		if (nullptr == AmbiguousElement)
		{
			OutError = "Field-aware reimport rollback probe lost its binding copy.";
			return false;
		}
		const auto ExistingBinding = std::find_if(
			AmbiguousElement->ResourceBindings.begin(),
			AmbiguousElement->ResourceBindings.end(),
			[&RequiredSlot](const Client::EFFECT_RESOURCE_BINDING_DESC& Binding)
			{
				return Binding.strSlotId == RequiredSlot;
			});
		if (ExistingBinding == AmbiguousElement->ResourceBindings.end())
		{
			OutError = "Field-aware reimport rollback probe lost its binding.";
			return false;
		}
		AmbiguousElement->ResourceBindings.push_back(*ExistingBinding);
		return RejectsWithoutCommit(
			AmbiguousBinding, Request, "ambiguous compiler resource binding");
	}

	bool Commit_PortableCarrierToBlueprint(
		const EFFECT_DOCUMENT_DESC& ImportedDocument,
		const MATERIALIZER_ELEMENT_PLAN& Plan,
		EFFECT_ELEMENT_DESC& InOutBlueprintElement, std::string& OutError)
	{
		const EFFECT_ELEMENT_DESC* Imported = Find_ExactElement(ImportedDocument,
			Plan.strTargetElementId);
		if (nullptr == Imported || !Imported->SourceRecipe.bEnabled ||
			Imported->eKind != Client::EFFECT_ELEMENT_KIND::PARTICLE)
		{
			OutError = "Generic import did not produce one portable Particle carrier.";
			return false;
		}
		EFFECT_DOCUMENT_DESC ExistingDocument = ImportedDocument;
		ExistingDocument.Elements.assign(1u, InOutBlueprintElement);
		EFFECT_GENERIC_AUTHORED_ELEMENT_REIMPORT_REQUEST Request;
		Request.strElementId = Plan.strTargetElementId;
		if (!Verify_GenericReimportRollbackProbes(
				ImportedDocument, ExistingDocument, Request, OutError))
		{
			return false;
		}
		EFFECT_DOCUMENT_DESC ReimportedDocument;
		if (!CEffectDocumentCodec::Build_GenericAuthoredElementReimportStage(
				ImportedDocument, ExistingDocument, Request,
				ReimportedDocument, OutError))
		{
			return false;
		}
		const EFFECT_ELEMENT_DESC* Reimported = Find_ExactElement(
			ReimportedDocument, Plan.strTargetElementId);
		if (nullptr == Reimported)
		{
			OutError = "Field-aware reimport lost its committed target Element.";
			return false;
		}
		InOutBlueprintElement = *Reimported;
		return true;
	}

	bool Materialize_Stage(const MATERIALIZER_BATCH& Batch,
		const MATERIALIZER_STAGE& Stage,
		const DATA_JSON_VALUE& VisualRoot,
		const std::unordered_map<std::string,
			const MATERIALIZER_ELEMENT_PLAN*>& PlansById,
		STAGED_TARGET_FILE& OutFile,
		ClientFrontendHarness::FOUR_CLASS_TRACK_A_MATERIALIZER_RESULT& OutResult,
		std::string& OutError)
	{
		const auto SourceArtifact = Batch.Artifacts.find(
			Stage.ImportedDocument.strPath);
		const auto BaselineArtifact = Batch.Artifacts.find(
			Stage.strLegacyBaselinePath);
		if (SourceArtifact == Batch.Artifacts.end() ||
			BaselineArtifact == Batch.Artifacts.end())
		{
			OutError = "Stage source or target baseline artifact is unavailable.";
			return false;
		}
		std::unordered_set<std::string> KeepBlueprintSourceProfiles;
		for (const std::string& PlanId : Stage.ElementPlanIds)
		{
			const auto Plan = PlansById.find(PlanId);
			if (Plan == PlansById.end() || nullptr == Plan->second)
			{
				OutError = "Stage materialization lost an Element plan.";
				return false;
			}
			if (Plan->second->eMaterial != MATERIAL_DISPOSITION::FAIL_CLOSED)
				KeepBlueprintSourceProfiles.insert(
					Plan->second->strTargetElementId);
		}
		EFFECT_DOCUMENT_DESC SourceDocument;
		EFFECT_DOCUMENT_DESC BaselineDocument;
		if (!Parse_SanitizedAuthoringDocument(SourceArtifact->second->Root, {},
				SourceDocument, OutError) ||
			SourceDocument.strEffectAssetId != Stage.strImportedEffectAssetId ||
			!Parse_SanitizedAuthoringDocument(BaselineArtifact->second->Root, {},
				BaselineDocument, OutError) ||
			BaselineDocument.strEffectAssetId != Stage.strLegacyEffectAssetId ||
			BaselineDocument.iLoadedFormatVersion != 12u)
		{
			if (OutError.empty())
				OutError = "Stage typed source/baseline document identity is invalid.";
			return false;
		}

		const DATA_JSON_VALUE* RawProgram = nullptr;
		const DATA_JSON_VALUE* RawBlueprint = nullptr;
		EFFECT_DOCUMENT_DESC BlueprintDocument;
		if (Stage.strMode == "TRACK_A_VISUAL_PROGRAM")
		{
			const DATA_JSON_VALUE* Programs = Required(VisualRoot, "programs",
				DATA_JSON_TYPE::ARRAY);
			RawProgram = nullptr == Programs ? nullptr : Find_UniqueObjectByString(
				*Programs, "effectAssetId", Stage.strSelectionRecordId);
			RawBlueprint = nullptr == RawProgram ? nullptr : Required(*RawProgram,
				"projectedDocument", DATA_JSON_TYPE::OBJECT);
			const DATA_JSON_VALUE* ProgramSha = nullptr == RawProgram ? nullptr :
				Required(*RawProgram, "programSha256", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* ProjectedSha = nullptr == RawProgram ? nullptr :
				Required(*RawProgram, "projectedDocumentSha256",
					DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* ProjectedTypedSha = nullptr == RawProgram ?
				nullptr : Required(*RawProgram,
					"projectedDocumentTypedCodecSha256", DATA_JSON_TYPE::STRING);
			if (nullptr == RawProgram || nullptr == RawBlueprint ||
				nullptr == ProgramSha || nullptr == ProjectedSha ||
				nullptr == ProjectedTypedSha ||
				!Verify_ObjectSelfHash(*RawProgram, "programSha256") ||
				ProgramSha->Get_String() != Stage.strSelectionRecordSha256 ||
				ProjectedSha->Get_String() !=
					Stage.strBlueprintCanonicalJsonSha256 ||
				!Stage.strBlueprintTypedCodecSha256.has_value() ||
				ProjectedTypedSha->Get_String() !=
					*Stage.strBlueprintTypedCodecSha256 ||
				Canonical_JsonSha256(*RawBlueprint) !=
					Stage.strBlueprintCanonicalJsonSha256)
			{
				OutError = "Track A visual-program blueprint identity drifted.";
				return false;
			}
			if (!Parse_SanitizedAuthoringDocument(*RawBlueprint,
					KeepBlueprintSourceProfiles, BlueprintDocument, OutError))
			{
				return false;
			}
		}
		else
		{
			if (!Verify_WarlordCanarySelection(Stage, VisualRoot, OutError))
				return false;
			RawBlueprint = &BaselineArtifact->second->Root;
			BlueprintDocument = BaselineDocument;
		}

		if (nullptr == RawBlueprint ||
			BlueprintDocument.strEffectAssetId != Stage.strLegacyEffectAssetId ||
			BlueprintDocument.Elements.size() != Stage.ElementPlanIds.size())
		{
			OutError = "Stage blueprint cardinality or Effect identity changed.";
			return false;
		}

		BlueprintDocument.strEffectAssetId = Stage.strTargetEffectAssetId;
		BlueprintDocument.iFormatVersion = Client::EFFECT_AUTHORING_FORMAT_VERSION;
		BlueprintDocument.iLoadedFormatVersion =
			Client::EFFECT_AUTHORING_FORMAT_VERSION;
		BlueprintDocument.bSourceContract = false;
		std::vector<std::string> IsolatedTargetElementIds;
		for (const std::string& PlanId : Stage.ElementPlanIds)
		{
			const auto PlanIterator = PlansById.find(PlanId);
			if (PlanIterator == PlansById.end() || nullptr == PlanIterator->second)
			{
				OutError = "Stage materialization lost an Element plan.";
				return false;
			}
			const MATERIALIZER_ELEMENT_PLAN& Plan = *PlanIterator->second;
			if (!Verify_TargetElementEvidence(Plan, *RawBlueprint,
					BaselineArtifact->second->Root, BlueprintDocument, OutError) ||
				!Verify_SelectionEvidence(Stage, Plan, RawProgram, OutError))
			{
				return false;
			}
			EFFECT_ELEMENT_DESC* TargetElement = Find_ExactElement(
				BlueprintDocument, Plan.strTargetElementId);
			if (nullptr == TargetElement)
			{
				OutError = "Stage typed blueprint lost target Element.";
				return false;
			}

			if (Plan.bImportedSource &&
				!Verify_ImportedElementEvidence(Plan,
					SourceArtifact->second->Root, SourceDocument, OutError))
			{
				return false;
			}
			if (Plan.eCarrier ==
				CARRIER_DISPOSITION::GENERIC_PARTICLE_IMPORT_CANDIDATE)
			{
				EFFECT_DOCUMENT_DESC ImportedStage;
				++OutResult.iGenericImportAttemptCount;
				if (!Build_GenericImportValidationStage(SourceDocument, Plan,
						*TargetElement,
						ImportedStage, OutError))
				{
					const std::string Rejection = Plan.strPlanId + ": " + OutError;
					if (Plan.eMaterial != MATERIAL_DISPOSITION::FAIL_CLOSED)
					{
						OutError = "Admitted generic import rejected " + Rejection;
						return false;
					}
					/* CXX_GENERIC_CODEC_ELEMENT_VALIDATION_REQUIRED is an
					   unresolved admission request, not a promise that every source
					   recipe is portable.  A codec rejection is recorded and the
					   fail-closed target is explicitly isolated. */
					OutResult.GenericImportRejections.push_back(Rejection);
					Isolate_UnadmittedPreservedElement(*TargetElement, false);
					IsolatedTargetElementIds.push_back(
						TargetElement->strElementId);
					OutError.clear();
					continue;
				}
				++OutResult.iGenericImportValidationCount;
				if (Plan.eMaterial != MATERIAL_DISPOSITION::FAIL_CLOSED)
				{
					if (!Commit_PortableCarrierToBlueprint(ImportedStage, Plan,
							*TargetElement, OutError))
					{
						return false;
					}
					++OutResult.iPortableCarrierCommitCount;
				}
				else
				{
					/* Validation-only source carriers never replace a fail-closed
					   target.  The prior authored Element remains the rollback-safe
					   visual reference. */
					Isolate_UnadmittedPreservedElement(*TargetElement, false);
					IsolatedTargetElementIds.push_back(
						TargetElement->strElementId);
				}
			}
			else
			{
				/* Ribbon and animation-trail rows need their family adapters;
				   retain the already selected authored blueprint exactly. */
				Isolate_UnadmittedPreservedElement(*TargetElement, true);
				IsolatedTargetElementIds.push_back(TargetElement->strElementId);
			}
		}

		for (const std::string& ElementId : IsolatedTargetElementIds)
		{
			const auto Element = std::find_if(BlueprintDocument.Elements.begin(),
				BlueprintDocument.Elements.end(),
				[&ElementId](const EFFECT_ELEMENT_DESC& Candidate)
				{
					return Candidate.strElementId == ElementId;
				});
			if (Element == BlueprintDocument.Elements.end())
			{
				OutError = "Isolated target Element disappeared before admission.";
				return false;
			}
			std::string ElementError;
			if (Validate_IsolatedOrdinaryElement(
					BlueprintDocument, *Element, ElementError))
			{
				++OutResult.iPreservedElementCount;
				continue;
			}
			OutResult.QuarantinedElements.push_back(
				Stage.strTargetEffectAssetId + "/" + ElementId + ": " +
				ElementError);
			BlueprintDocument.Elements.erase(Element);
		}

		std::string ValidationError;
		if (!CEffectDocumentCodec::Validate_Drawable(BlueprintDocument,
				ValidationError))
		{
			OutError = "Materialized v13 document is not drawable: " +
				Stage.strTargetEffectAssetId + ": " + ValidationError;
			return false;
		}
		const std::string Canonical =
			CEffectDocumentCodec::Serialize(BlueprintDocument);
		EFFECT_DOCUMENT_DESC Reloaded;
		if (!CEffectDocumentCodec::Parse(Canonical, Reloaded, ValidationError) ||
			Reloaded.iLoadedFormatVersion !=
				Client::EFFECT_AUTHORING_FORMAT_VERSION ||
			Reloaded.strEffectAssetId != Stage.strTargetEffectAssetId ||
			Reloaded.Elements.size() != BlueprintDocument.Elements.size() ||
			!CEffectDocumentCodec::Validate_Drawable(Reloaded, ValidationError) ||
			CEffectDocumentCodec::Serialize(Reloaded) != Canonical)
		{
			OutError = "Materialized document failed canonical v13 round-trip: " +
				Stage.strTargetEffectAssetId + ": " + ValidationError;
			return false;
		}

		if (!Resolve_ArtifactPath(Batch.RepositoryRoot, Stage.strTargetPath,
				OutFile.Path, OutError))
		{
			return false;
		}
		OutFile.bMustNotExist =
			Stage.strCandidateBaselinePolicy == "MUST_NOT_EXIST";
		OutFile.strExpectedRawSha256 =
			Stage.strCandidateRawSha256.value_or(std::string{});
		OutFile.strExpectedCanonicalJsonSha256 =
			Stage.strCandidateCanonicalJsonSha256.value_or(std::string{});
		OutFile.strCanonicalDocument = Canonical;
		return true;
	}

	bool Compute_NonIdentityAuthoringSemanticSha256(
		const std::string_view CanonicalDocument, std::string& OutSha256,
		std::string& OutError)
	{
		DATA_JSON_VALUE Root;
		if (!Parse_Json(CanonicalDocument, Root, OutError) || !Root.Is_Object())
		{
			if (OutError.empty())
				OutError = "Authored semantic fingerprint input is not an object.";
			return false;
		}
		DATA_JSON_VALUE::OBJECT Object = Root.Get_Object();
		Object.erase("version");
		Object.erase("effectAssetId");
		std::vector<std::string> Order;
		for (const std::string& Key : Root.Get_ObjectInsertionOrder())
		{
			if (Key != "version" && Key != "effectAssetId")
				Order.push_back(Key);
		}
		if (Object.size() != Order.size())
		{
			OutError = "Authored semantic fingerprint identity projection failed.";
			return false;
		}
		OutSha256 = Canonical_JsonSha256(DATA_JSON_VALUE::Object(
			std::move(Object), std::move(Order)));
		return Is_Sha256(OutSha256);
	}

	bool Materialize_LegacyStarterCandidate(const MATERIALIZER_BATCH& Batch,
		const LEGACY_STARTER_CANDIDATE& Candidate, STAGED_TARGET_FILE& OutFile,
		ClientFrontendHarness::FOUR_CLASS_TRACK_A_MATERIALIZER_RESULT& OutResult,
		std::string& OutError)
	{
		const auto SourceArtifact = Batch.Artifacts.find(
			Candidate.StarterSource.strPath);
		if (SourceArtifact == Batch.Artifacts.end() ||
			nullptr == SourceArtifact->second)
		{
			OutError = "Legacy starter selected source is unavailable: " +
				Candidate.StarterSource.strPath;
			return false;
		}
		EFFECT_DOCUMENT_DESC SourceDocument;
		if (!CEffectDocumentCodec::Parse(SourceArtifact->second->strBytes,
				SourceDocument, OutError) ||
			SourceDocument.strEffectAssetId !=
				Candidate.strStarterSourceEffectAssetId ||
			SourceDocument.iLoadedFormatVersion !=
				Candidate.iStarterSourceAuthoringVersion ||
			SourceDocument.bSourceContract)
		{
			if (OutError.empty())
				OutError = "Legacy starter selected source codec identity changed.";
			return false;
		}

		EFFECT_DOCUMENT_DESC CandidateDocument = SourceDocument;
		CandidateDocument.iFormatVersion = Client::EFFECT_AUTHORING_FORMAT_VERSION;
		CandidateDocument.iLoadedFormatVersion =
			Client::EFFECT_AUTHORING_FORMAT_VERSION;
		CandidateDocument.bSourceContract = false;
		CandidateDocument.strEffectAssetId = Candidate.strTargetEffectAssetId;
		std::string ValidationError;
		if (!CEffectDocumentCodec::Validate_Drawable(CandidateDocument,
				ValidationError))
		{
			OutError = "Legacy starter candidate is not drawable: " +
				Candidate.strTargetEffectAssetId + ": " + ValidationError;
			return false;
		}
		const std::string Canonical =
			CEffectDocumentCodec::Serialize(CandidateDocument);
		EFFECT_DOCUMENT_DESC Reloaded;
		if (!CEffectDocumentCodec::Parse(Canonical, Reloaded, ValidationError) ||
			Reloaded.iLoadedFormatVersion !=
				Client::EFFECT_AUTHORING_FORMAT_VERSION ||
			Reloaded.strEffectAssetId != Candidate.strTargetEffectAssetId ||
			!CEffectDocumentCodec::Validate_Drawable(Reloaded, ValidationError) ||
			CEffectDocumentCodec::Serialize(Reloaded) != Canonical)
		{
			OutError = "Legacy starter candidate failed canonical v13 round-trip: " +
				Candidate.strTargetEffectAssetId + ": " + ValidationError;
			return false;
		}

		std::string SemanticSha256;
		if (!Compute_NonIdentityAuthoringSemanticSha256(Canonical,
				SemanticSha256, OutError))
		{
			return false;
		}
		if (Candidate.strTargetEffectAssetId ==
			"effect.dimensionmaster.skill.2050500.unified")
		{
			OutResult.strDimensionMaster2050500PreservedSemanticSha256 =
				SemanticSha256;
		}

		if (!Resolve_ArtifactPath(Batch.RepositoryRoot, Candidate.strTargetPath,
				OutFile.Path, OutError))
		{
			return false;
		}
		OutFile.bMustNotExist =
			Candidate.strCandidateBaselinePolicy == "MUST_NOT_EXIST";
		OutFile.strExpectedRawSha256 =
			Candidate.strCandidateRawSha256.value_or(std::string{});
		OutFile.strExpectedCanonicalJsonSha256 =
			Candidate.strCandidateCanonicalJsonSha256.value_or(std::string{});
		OutFile.strCanonicalDocument = Canonical;
		return true;
	}

	bool Verify_TargetBaseline(const STAGED_TARGET_FILE& File,
		std::string& OutError)
	{
		if (File.bMustNotExist)
		{
			std::error_code Error;
			const bool Exists = std::filesystem::exists(File.Path, Error);
			if (Error || Exists)
			{
				OutError = Exists ?
					"Unified candidate MUST_NOT_EXIST baseline already exists: " +
						File.Path.string() :
					"Unable to inspect unified candidate path: " +
						File.Path.string();
				return false;
			}
			return true;
		}
		std::string Bytes;
		DATA_JSON_VALUE Root;
		if (!Read_File(File.Path, Bytes, OutError))
			return false;
		if (CEffectRuntimeAuthorityCodec::Compute_Sha256Hex(Bytes) !=
			File.strExpectedRawSha256)
		{
			OutError = "Target baseline raw SHA-256 drifted: " +
				File.Path.string();
			return false;
		}
		if (!Parse_Json(Bytes, Root, OutError) ||
			Canonical_JsonSha256(Root) !=
				File.strExpectedCanonicalJsonSha256)
		{
			OutError = "Target baseline canonical JSON SHA-256 drifted: " +
				File.Path.string();
			return false;
		}
		return true;
	}

	bool Write_NewFile(const std::filesystem::path& Path,
		const std::string_view Bytes, std::string& OutError)
	{
		HANDLE File = CreateFileW(Path.c_str(), GENERIC_WRITE, 0u, nullptr,
			CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (INVALID_HANDLE_VALUE == File)
		{
			OutError = "Unable to create transaction temp file: " + Path.string();
			return false;
		}
		std::size_t Offset = 0u;
		bool Success = true;
		while (Offset < Bytes.size())
		{
			const DWORD Chunk = static_cast<DWORD>((std::min)(
				Bytes.size() - Offset,
				static_cast<std::size_t>((std::numeric_limits<DWORD>::max)())));
			DWORD Written = 0u;
			if (FALSE == WriteFile(File, Bytes.data() + Offset, Chunk,
					&Written, nullptr) || Written != Chunk)
			{
				Success = false;
				break;
			}
			Offset += Written;
		}
		if (Success)
			Success = FALSE != FlushFileBuffers(File);
		if (FALSE == CloseHandle(File))
			Success = false;
		if (!Success)
		{
			std::error_code Ignore;
			std::filesystem::remove(Path, Ignore);
			OutError = "Unable to write transaction temp file exactly: " +
				Path.string();
			return false;
		}
		return true;
	}

	bool Verify_StagedDocumentFile(const std::filesystem::path& Path,
		const std::string_view Expected, const bool bValidateEffectDocument,
		std::string& OutError)
	{
		std::string Bytes;
		if (!Read_File(Path, Bytes, OutError) || Bytes != Expected)
		{
			if (OutError.empty())
				OutError = "Transaction temp file bytes changed.";
			return false;
		}
		if (!bValidateEffectDocument)
		{
			DATA_JSON_VALUE Root;
			return Parse_Json(Bytes, Root, OutError);
		}
		EFFECT_DOCUMENT_DESC Document;
		if (!CEffectDocumentCodec::Load(Path, Document, OutError) ||
			Document.iLoadedFormatVersion !=
				Client::EFFECT_AUTHORING_FORMAT_VERSION ||
			!CEffectDocumentCodec::Validate_Drawable(Document, OutError) ||
			CEffectDocumentCodec::Serialize(Document) != Bytes)
		{
			if (OutError.empty())
				OutError = "Transaction temp file failed canonical v13 verification.";
			return false;
		}
		return true;
	}

	bool Remove_IfExists(const std::filesystem::path& Path)
	{
		std::error_code Error;
		const bool Exists = std::filesystem::exists(Path, Error);
		if (Error)
			return false;
		return !Exists || std::filesystem::remove(Path, Error) && !Error;
	}

	bool Apply_StagedFilesTransaction(
		const std::vector<STAGED_TARGET_FILE>& Files,
		const std::optional<std::size_t> InjectFailureBeforeIndex,
		std::string& OutError, const bool bValidateEffectDocuments = true,
		const std::optional<std::size_t> InjectVerificationFailureAtIndex =
			std::nullopt)
	{
		if (Files.empty())
		{
			OutError = "Authored import transaction has no target files.";
			return false;
		}
		std::vector<std::filesystem::path> Temps(Files.size());
		std::vector<std::filesystem::path> Backups(Files.size());
		std::vector<bool> BackedUp(Files.size(), false);
		std::vector<bool> Promoted(Files.size(), false);
		const std::wstring Suffix = L".four-class-track-a." +
			std::to_wstring(GetCurrentProcessId());
		for (std::size_t Index = 0u; Index < Files.size(); ++Index)
		{
			Temps[Index] = Files[Index].Path;
			Temps[Index] += Suffix + L"." + std::to_wstring(Index) + L".tmp";
			Backups[Index] = Files[Index].Path;
			Backups[Index] += Suffix + L"." + std::to_wstring(Index) + L".bak";
			std::error_code Error;
			if (std::filesystem::exists(Temps[Index], Error) || Error ||
				std::filesystem::exists(Backups[Index], Error) || Error)
			{
				OutError = "Transaction temp/backup path already exists.";
				return false;
			}
			if (!Verify_TargetBaseline(Files[Index], OutError))
				return false;
		}
		const auto RollbackPromotedFiles = [&]()
		{
			bool RollbackSucceeded = true;
			for (std::size_t Reverse = Files.size(); Reverse > 0u; --Reverse)
			{
				const std::size_t Index = Reverse - 1u;
				if (Promoted[Index])
				{
					std::string Current;
					std::string IgnoreError;
					if (!Read_File(Files[Index].Path, Current, IgnoreError) ||
						Current != Files[Index].strCanonicalDocument ||
						!Remove_IfExists(Files[Index].Path))
					{
						RollbackSucceeded = false;
						continue;
					}
				}
				if (BackedUp[Index])
				{
					std::error_code Error;
					std::filesystem::rename(Backups[Index], Files[Index].Path,
						Error);
					if (Error)
						RollbackSucceeded = false;
				}
			}
			for (const std::filesystem::path& Temp : Temps)
				Remove_IfExists(Temp);
			return RollbackSucceeded;
		};

		for (std::size_t Index = 0u; Index < Files.size(); ++Index)
		{
			if (!Write_NewFile(Temps[Index], Files[Index].strCanonicalDocument,
					OutError) ||
				!Verify_StagedDocumentFile(Temps[Index],
					Files[Index].strCanonicalDocument,
					bValidateEffectDocuments, OutError))
			{
				for (const std::filesystem::path& Temp : Temps)
					Remove_IfExists(Temp);
				return false;
			}
		}

		for (const STAGED_TARGET_FILE& File : Files)
		{
			if (!Verify_TargetBaseline(File, OutError))
			{
				for (const std::filesystem::path& Temp : Temps)
					Remove_IfExists(Temp);
				return false;
			}
		}

		bool CommitSucceeded = true;
		std::size_t FailedIndex = Files.size();
		for (std::size_t Index = 0u; Index < Files.size(); ++Index)
		{
			if (InjectFailureBeforeIndex.has_value() &&
				Index == *InjectFailureBeforeIndex)
			{
				OutError = "Injected transaction failure.";
				CommitSucceeded = false;
				FailedIndex = Index;
				break;
			}
			if (!Verify_TargetBaseline(Files[Index], OutError))
			{
				CommitSucceeded = false;
				FailedIndex = Index;
				break;
			}
			std::error_code Error;
			if (!Files[Index].bMustNotExist)
			{
				std::filesystem::rename(Files[Index].Path, Backups[Index], Error);
				if (Error)
				{
					OutError = "Unable to stage target backup: " +
						Files[Index].Path.string();
					CommitSucceeded = false;
					FailedIndex = Index;
					break;
				}
				BackedUp[Index] = true;
			}
			Error.clear();
			std::filesystem::rename(Temps[Index], Files[Index].Path, Error);
			if (Error)
			{
				OutError = "Unable to promote target temp file: " +
					Files[Index].Path.string();
				CommitSucceeded = false;
				FailedIndex = Index;
				break;
			}
			Promoted[Index] = true;
		}

		if (!CommitSucceeded)
		{
			if (!RollbackPromotedFiles())
			{
				OutError += " Rollback could not restore every promoted target.";
			}
			(void)FailedIndex;
			return false;
		}

		for (std::size_t Index = 0u; Index < Files.size(); ++Index)
		{
			std::string Current;
			if ((InjectVerificationFailureAtIndex.has_value() &&
					Index == *InjectVerificationFailureAtIndex) ||
				!Read_File(Files[Index].Path, Current, OutError) ||
				Current != Files[Index].strCanonicalDocument)
			{
				if (InjectVerificationFailureAtIndex.has_value() &&
					Index == *InjectVerificationFailureAtIndex)
				{
					OutError = "Injected post-promotion verification failure.";
				}
				else if (OutError.empty())
				{
					OutError =
						"Committed target bytes changed before transaction close.";
				}
				if (!RollbackPromotedFiles())
					OutError += " Rollback could not restore every promoted target.";
				return false;
			}
		}
		for (const std::filesystem::path& Backup : Backups)
		{
			if (!Remove_IfExists(Backup))
			{
				OutError = "Unable to remove a committed transaction backup.";
				return false;
			}
		}
		return true;
	}

	bool Build_StagedTargetFiles(const MATERIALIZER_BATCH& Batch,
		std::vector<STAGED_TARGET_FILE>& OutFiles,
		ClientFrontendHarness::FOUR_CLASS_TRACK_A_MATERIALIZER_RESULT& OutResult,
		std::string& OutError)
	{
		if (Batch.Stages.size() != EXPECTED_STAGE_COUNT ||
			Batch.Plans.size() != EXPECTED_ELEMENT_PLAN_COUNT)
		{
			OutError = "Materializer batch denominator changed before staging.";
			return false;
		}
		const std::string VisualPath = Batch.Stages.front().SelectionArtifact.strPath;
		if (!std::all_of(Batch.Stages.begin(), Batch.Stages.end(),
			[&VisualPath](const MATERIALIZER_STAGE& Stage)
			{
				return Stage.SelectionArtifact.strPath == VisualPath;
			}))
		{
			OutError = "Batch stages do not share one pinned visual-program corpus.";
			return false;
		}
		const auto VisualArtifact = Batch.Artifacts.find(VisualPath);
		if (VisualArtifact == Batch.Artifacts.end())
		{
			OutError = "Pinned visual-program corpus is unavailable.";
			return false;
		}
		if (!Verify_ObjectSelfHash(VisualArtifact->second->Root,
				"artifactSha256"))
		{
			OutError = "Pinned visual-program corpus self hash changed.";
			return false;
		}
		std::unordered_map<std::string, const MATERIALIZER_ELEMENT_PLAN*>
			PlansById;
		for (const MATERIALIZER_ELEMENT_PLAN& Plan : Batch.Plans)
			PlansById.emplace(Plan.strPlanId, &Plan);

		std::vector<STAGED_TARGET_FILE> Files;
		Files.reserve(EXPECTED_TOTAL_CANDIDATE_COUNT);
		for (const MATERIALIZER_STAGE& Stage : Batch.Stages)
		{
			STAGED_TARGET_FILE File;
			if (!Materialize_Stage(Batch, Stage,
					VisualArtifact->second->Root, PlansById, File, OutResult,
					OutError))
			{
				return false;
			}
			Files.push_back(std::move(File));
		}
		for (const LEGACY_STARTER_CANDIDATE& Candidate :
			Batch.LegacyStarterCandidates)
		{
			STAGED_TARGET_FILE File;
			if (!Materialize_LegacyStarterCandidate(Batch, Candidate, File,
					OutResult, OutError))
			{
				return false;
			}
			Files.push_back(std::move(File));
		}
		if (OutResult.iGenericImportAttemptCount !=
				EXPECTED_GENERIC_PLAN_COUNT ||
			OutResult.iGenericImportValidationCount +
				OutResult.GenericImportRejections.size() !=
				EXPECTED_GENERIC_PLAN_COUNT ||
			OutResult.iPortableCarrierCommitCount !=
				EXPECTED_ADMITTED_MATERIAL_COUNT ||
			OutResult.iPreservedElementCount +
				OutResult.QuarantinedElements.size() !=
				EXPECTED_FAIL_CLOSED_MATERIAL_COUNT)
		{
			OutError = "Materializer staged carrier/preserve denominator changed.";
			return false;
		}
		std::unordered_set<std::wstring> TargetPaths;
		for (const STAGED_TARGET_FILE& File : Files)
		{
			if (!TargetPaths.insert(File.Path.lexically_normal().wstring()).second)
			{
				OutError = "Materializer staged a duplicate candidate path.";
				return false;
			}
		}
		std::size_t LegacyClipCount = 0u;
		std::size_t LegacyVisualCount = 0u;
		for (const LEGACY_STARTER_STAGE& Stage : Batch.LegacyStarterStages)
		{
			LegacyClipCount += Stage.Clips.size();
			LegacyVisualCount += static_cast<std::size_t>(std::count_if(
				Stage.Clips.begin(), Stage.Clips.end(),
				[](const LEGACY_STARTER_CLIP& Clip)
				{
					return Clip.bVisualBearing;
				}));
		}
		if (Files.size() != EXPECTED_TOTAL_CANDIDATE_COUNT ||
			TargetPaths.size() != EXPECTED_TOTAL_CANDIDATE_COUNT ||
			Batch.LegacyStarterStages.size() !=
				EXPECTED_LEGACY_STARTER_STAGE_COUNT ||
			LegacyClipCount != EXPECTED_LEGACY_STARTER_OCCURRENCE_COUNT ||
			LegacyVisualCount != EXPECTED_LEGACY_STARTER_VISUAL_COUNT ||
			LegacyClipCount - LegacyVisualCount !=
				EXPECTED_LEGACY_STARTER_SILENT_COUNT ||
			Batch.LegacyStarterCandidates.size() !=
				EXPECTED_LEGACY_STARTER_CANDIDATE_COUNT ||
			OutResult.strDimensionMaster2050500PreservedSemanticSha256.empty())
		{
			OutError = "Materializer full candidate denominator changed before commit.";
			return false;
		}
		OutResult.iLegacyStarterStageCount = Batch.LegacyStarterStages.size();
		OutResult.iLegacyStarterClipOccurrenceCount = LegacyClipCount;
		OutResult.iLegacyStarterVisualClipOccurrenceCount = LegacyVisualCount;
		OutResult.iLegacyStarterSilentClipOccurrenceCount =
			LegacyClipCount - LegacyVisualCount;
		OutResult.iLegacyStarterCandidateDocumentCount =
			Batch.LegacyStarterCandidates.size();
		OutResult.iTotalCandidateDocumentCount = Files.size();
		OutFiles = std::move(Files);
		return true;
	}

	bool Read_ReceiptStringArray(const DATA_JSON_VALUE& Object,
		const std::string_view Name, std::vector<std::string>& Out)
	{
		const DATA_JSON_VALUE* Values = Required(Object, Name,
			DATA_JSON_TYPE::ARRAY);
		if (nullptr == Values || Values->Get_Array().size() > 4096u)
			return false;
		std::vector<std::string> Staged;
		Staged.reserve(Values->Get_Array().size());
		for (const DATA_JSON_VALUE& Value : Values->Get_Array())
		{
			if (!Value.Is_String() || Value.Get_String().empty() ||
				Value.Get_String().size() > 4096u)
			{
				return false;
			}
			Staged.push_back(Value.Get_String());
		}
		Out = std::move(Staged);
		return true;
	}

	bool Parse_RestorationParticleRow(const DATA_JSON_VALUE& Value,
		RESTORATION_RECEIPT_ROW& Out)
	{
		const DATA_JSON_VALUE* Portable = Required(Value, "portable",
			DATA_JSON_TYPE::BOOLEAN);
		const DATA_JSON_VALUE* SourceBindings = Required(
			Value, "sourceBindings", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* TargetBindings = Required(
			Value, "targetBindings", DATA_JSON_TYPE::ARRAY);
		RESTORATION_RECEIPT_ROW Row;
		Row.eKind = RESTORATION_RECEIPT_ROW_KIND::PARTICLE;
		if (!Value.Is_Object() || nullptr == Portable ||
			nullptr == SourceBindings || nullptr == TargetBindings ||
			!Read_String(Value, "targetElementId", Row.strTargetElementId) ||
			!Read_String(Value, "sourceEffectAssetId",
				Row.strSourceEffectAssetId) ||
			!Read_String(Value, "sourceElementId", Row.strSourceElementId) ||
			!Read_String(Value, "sourceEventId", Row.strSourceEventId) ||
			!Read_String(Value, "sourceRecipeCanonicalSha256",
				Row.strSourceRecipeCanonicalSha256) ||
			!Read_String(Value, "normalizedRecipeCanonicalSha256",
				Row.strNormalizedRecipeCanonicalSha256) ||
			!Read_String(Value, "sourceDetailCanonicalSha256",
				Row.strSourceDetailCanonicalSha256) ||
			!Read_String(Value, "targetDetailCanonicalSha256",
				Row.strTargetDetailCanonicalSha256) ||
			!Is_Sha256(Row.strSourceRecipeCanonicalSha256) ||
			!Is_Sha256(Row.strNormalizedRecipeCanonicalSha256) ||
			!Is_Sha256(Row.strSourceDetailCanonicalSha256) ||
			!Is_Sha256(Row.strTargetDetailCanonicalSha256) ||
			!Read_String(Value, "rendererShape", Row.strRendererShape) ||
			!Read_String(Value, "deferredReason", Row.strDeferredReason, true) ||
			!Read_ReceiptStringArray(Value, "failClosedReasons",
				Row.FailClosedReasons) ||
			(Row.strRendererShape != "mesh" &&
			 Row.strRendererShape != "sprite"))
		{
			return false;
		}
		Row.strSourceBindingsCanonicalSha256 =
			Canonical_JsonSha256(*SourceBindings);
		Row.strTargetBindingsCanonicalSha256 =
			Canonical_JsonSha256(*TargetBindings);
		Row.bPortable = Portable->Get_Boolean();
		if (Row.bPortable != Row.strDeferredReason.empty())
			return false;
		const std::string RecipeFailure =
			"UNSUPPORTED_ORDINARY_RECIPE:" + Row.strDeferredReason;
		const bool bHasRecipeFailure = std::find(
			Row.FailClosedReasons.begin(), Row.FailClosedReasons.end(),
			RecipeFailure) != Row.FailClosedReasons.end();
		if ((!Row.bPortable &&
			 (Row.FailClosedReasons.empty() || !bHasRecipeFailure)) ||
			(Row.bPortable && bHasRecipeFailure))
		{
			return false;
		}
		Out = std::move(Row);
		return true;
	}

	bool Parse_RestorationDecalRow(const DATA_JSON_VALUE& Value,
		RESTORATION_RECEIPT_ROW& Out)
	{
		const DATA_JSON_VALUE* SourceBindings = Required(
			Value, "sourceBindings", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* TargetBindings = Required(
			Value, "targetBindings", DATA_JSON_TYPE::ARRAY);
		RESTORATION_RECEIPT_ROW Row;
		Row.eKind = RESTORATION_RECEIPT_ROW_KIND::DECAL;
		Row.strRendererShape = "decal";
		if (!Value.Is_Object() || nullptr == SourceBindings ||
			nullptr == TargetBindings ||
			!Read_String(Value, "targetElementId", Row.strTargetElementId) ||
			!Read_String(Value, "sourceEffectAssetId",
				Row.strSourceEffectAssetId) ||
			!Read_String(Value, "sourceElementId", Row.strSourceElementId) ||
			!Read_String(Value, "sourceEventId", Row.strSourceEventId) ||
			!Read_String(Value, "sourceRecipeCanonicalSha256",
				Row.strSourceRecipeCanonicalSha256) ||
			!Read_String(Value, "normalizedRecipeCanonicalSha256",
				Row.strNormalizedRecipeCanonicalSha256) ||
			!Is_Sha256(Row.strSourceRecipeCanonicalSha256) ||
			!Is_Sha256(Row.strNormalizedRecipeCanonicalSha256) ||
			!Read_String(Value, "baseStatus", Row.strBaseStatus) ||
			(Row.strBaseStatus != "SOURCE_OR_ARTIST_BOUND" &&
			 Row.strBaseStatus != "AUTHORING_INCOMPLETE"))
		{
			return false;
		}
		Row.strSourceBindingsCanonicalSha256 =
			Canonical_JsonSha256(*SourceBindings);
		Row.strTargetBindingsCanonicalSha256 =
			Canonical_JsonSha256(*TargetBindings);
		Row.bPortable = Row.strBaseStatus == "SOURCE_OR_ARTIST_BOUND";
		if (!Row.bPortable)
			Row.FailClosedReasons.emplace_back("DECAL_AUTHORING_INCOMPLETE");
		Out = std::move(Row);
		return true;
	}

	bool Parse_RestorationAnimationTrailRow(const DATA_JSON_VALUE& Value,
		RESTORATION_RECEIPT_ROW& Out)
	{
		RESTORATION_RECEIPT_ROW Row;
		Row.eKind = RESTORATION_RECEIPT_ROW_KIND::ANIMATION_TRAIL;
		Row.strRendererShape = "animationTrail";
		if (!Value.Is_Object() ||
			!Read_String(Value, "targetElementId", Row.strTargetElementId) ||
			!Read_String(Value, "sourceElementId", Row.strSourceElementId) ||
			!Read_String(Value, "sourceEventId", Row.strSourceEventId) ||
			!Read_String(Value, "deferredReason", Row.strDeferredReason, true) ||
			!Read_ReceiptStringArray(Value, "failClosedReasons",
				Row.FailClosedReasons))
		{
			return false;
		}
		const DATA_JSON_VALUE* Admitted = Required(Value, "admitted",
			DATA_JSON_TYPE::BOOLEAN);
		const DATA_JSON_VALUE* Portable = Required(Value, "portable",
			DATA_JSON_TYPE::BOOLEAN);
		if ((nullptr == Admitted && nullptr == Portable) ||
			(nullptr != Admitted && nullptr != Portable &&
			 Admitted->Get_Boolean() != Portable->Get_Boolean()))
			return false;
		Row.bPortable = nullptr != Admitted ?
			Admitted->Get_Boolean() : Portable->Get_Boolean();
		if (Row.bPortable != Row.strDeferredReason.empty() ||
			(Row.bPortable && !Row.FailClosedReasons.empty()) ||
			(!Row.bPortable && Row.FailClosedReasons.empty()))
		{
			return false;
		}
		Out = std::move(Row);
		return true;
	}

	bool Load_RestorationReceipt(const std::filesystem::path& RepositoryRoot,
		RESTORATION_RECEIPT& OutReceipt, std::string& OutError)
	{
		const std::filesystem::path ReceiptPath = RepositoryRoot / L"Data" /
			L"Effects" / L"AuthoredCorrections" / L"Generated" /
			L"FourClassCombat.track-a-restoration-receipt.json";
		std::string ReceiptBytes;
		DATA_JSON_VALUE Receipt;
		if (!Read_File(ReceiptPath, ReceiptBytes, OutError) ||
			!Parse_Json(ReceiptBytes, Receipt, OutError))
		{
			return false;
		}
		const DATA_JSON_VALUE* Schema = Required(
			Receipt, "schema", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* FormatVersion = Required(
			Receipt, "formatVersion", DATA_JSON_TYPE::NUMBER);
		const DATA_JSON_VALUE* Targets = Required(
			Receipt, "targets", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* CharacterGhostCues = Required(
			Receipt, "characterGhostCues", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* Counts = Required(
			Receipt, "counts", DATA_JSON_TYPE::OBJECT);
		if (nullptr == Schema || Schema->Get_String() !=
				"lostark.four-class-track-a-restoration-receipt" ||
			nullptr == FormatVersion ||
			FormatVersion->Get_Number() != 2.0 ||
			nullptr == Targets || nullptr == CharacterGhostCues ||
			nullptr == Counts ||
			!Verify_SelfHash(Receipt, "artifactSha256", OutError))
		{
			if (OutError.empty())
				OutError =
					"Track A restoration receipt schema/counts are invalid.";
			return false;
		}

		RESTORATION_RECEIPT Parsed;
		if (!Read_U32(*Counts, "targetCount", Parsed.iTargetCount) ||
			!Read_U32(*Counts, "strictMappedParticleCount",
				Parsed.iStrictMappedParticleCount) ||
			!Read_U32(*Counts, "sourceDecalCount", Parsed.iSourceDecalCount) ||
			!Read_U32(*Counts, "sourceDecalBaseReadyCount",
				Parsed.iSourceDecalReadyCount) ||
			!Read_U32(*Counts, "sourceDecalIncompleteCount",
				Parsed.iSourceDecalIncompleteCount) ||
			!Read_U32(*Counts, "supplementalPreservedCount",
				Parsed.iSupplementalPreservedCount) ||
			!Read_U32(*Counts, "placeholderTrailExcludedCount",
				Parsed.iPlaceholderTrailExcludedCount) ||
			!Read_U32(*Counts, "outputElementCount",
				Parsed.iOutputElementCount) ||
			!Read_U32(*Counts, "portableCount", Parsed.iPortableCount) ||
			!Read_U32(*Counts, "sourcePreservedDeferredCount",
				Parsed.iRecipeDeferredCount) ||
			!Read_U32(*Counts, "drawableAdmittedCount",
				Parsed.iDrawableAdmittedCount) ||
			!Read_U32(*Counts, "portableFailClosedCount",
				Parsed.iPortableFailClosedCount) ||
			!Read_U32(*Counts, "characterGhostCueCount",
				Parsed.iCharacterGhostCueCount) ||
			!Read_U32(*Counts, "characterGhostTargetCount",
				Parsed.iCharacterGhostTargetCount) ||
			Parsed.iCharacterGhostCueCount !=
				EXPECTED_CHARACTER_GHOST_CUE_COUNT ||
			Parsed.iCharacterGhostTargetCount !=
				EXPECTED_CHARACTER_GHOST_TARGET_COUNT ||
			Parsed.iCharacterGhostCueCount !=
				CharacterGhostCues->Get_Array().size() ||
			Parsed.iTargetCount != Targets->Get_Array().size() ||
			Parsed.iTargetCount != EXPECTED_TOTAL_CANDIDATE_COUNT ||
			Parsed.iPortableCount + Parsed.iRecipeDeferredCount !=
				Parsed.iStrictMappedParticleCount ||
			Parsed.iDrawableAdmittedCount +
				Parsed.iPortableFailClosedCount != Parsed.iPortableCount ||
			Parsed.iSourceDecalReadyCount +
				Parsed.iSourceDecalIncompleteCount != Parsed.iSourceDecalCount ||
			Parsed.iStrictMappedParticleCount + Parsed.iSourceDecalCount +
				Parsed.iSupplementalPreservedCount != Parsed.iOutputElementCount)
		{
			OutError = "Track A restoration receipt global denominator is invalid.";
			return false;
		}
		const DATA_JSON_VALUE* TrailNotifyCount =
			Counts->Find("sourceAnimationTrailNotifyCount");
		const DATA_JSON_VALUE* TrailElementCount =
			Counts->Find("sourceAnimationTrailElementCount");
		if ((nullptr == TrailNotifyCount) != (nullptr == TrailElementCount) ||
			(nullptr != TrailNotifyCount &&
			 (!Read_U32(*Counts, "sourceAnimationTrailNotifyCount",
				 Parsed.iSourceAnimationTrailNotifyCount) ||
			  !Read_U32(*Counts, "sourceAnimationTrailElementCount",
				 Parsed.iSourceAnimationTrailElementCount))))
		{
			OutError = "Track A AnimationTrail global denominator is invalid.";
			return false;
		}
		Parsed.bHasAnimationTrailDenominators = nullptr != TrailNotifyCount;

		std::set<std::string, std::less<>> EffectIds;
		std::set<std::string, std::less<>> RelativePaths;
		std::set<std::string, std::less<>> GlobalTrailLineages;
		std::set<std::string, std::less<>> GlobalTrailNotifyIdentities;
		uint64_t OutputElementCount = 0u;
		uint64_t ParticleCount = 0u;
		uint64_t DecalCount = 0u;
		uint64_t DecalReadyCount = 0u;
		uint64_t DecalIncompleteCount = 0u;
		uint64_t SupplementalCount = 0u;
		uint64_t PlaceholderTrailCount = 0u;
		uint64_t PortableCount = 0u;
		uint64_t RecipeDeferredCount = 0u;
		uint64_t AdmittedCount = 0u;
		uint64_t PortableFailClosedCount = 0u;
		uint64_t AnimationTrailRowCount = 0u;
		for (const DATA_JSON_VALUE& Target : Targets->Get_Array())
		{
			RESTORATION_TARGET_RECEIPT TargetReceipt;
			const DATA_JSON_VALUE* ParticleRows = Required(
				Target, "particleRows", DATA_JSON_TYPE::ARRAY);
			const DATA_JSON_VALUE* DecalRows = Required(
				Target, "sourceDecalRows", DATA_JSON_TYPE::ARRAY);
			if (!Target.Is_Object() ||
				!Read_String(Target, "targetEffectAssetId",
					TargetReceipt.strEffectAssetId) ||
				!Read_String(Target, "targetPath", TargetReceipt.strTargetPath) ||
				!Read_String(Target, "characterClass",
					TargetReceipt.strCharacterClass) ||
				!Read_U32(Target, "skillId", TargetReceipt.iSkillId) ||
				!Read_U32(Target, "stageIndex", TargetReceipt.iStageIndex) ||
				!Read_U32(Target, "stageClipIndex",
					TargetReceipt.iStageClipIndex) ||
				!Read_String(Target, "clip", TargetReceipt.strClip) ||
				!Read_String(Target, "outputCanonicalSha256",
					TargetReceipt.strOutputCanonicalSha256) ||
				!Is_Sha256(TargetReceipt.strOutputCanonicalSha256) ||
				!EffectIds.insert(TargetReceipt.strEffectAssetId).second ||
				!RelativePaths.insert(TargetReceipt.strTargetPath).second ||
				!std::string_view(TargetReceipt.strEffectAssetId).ends_with(
					".unified") ||
				!std::string_view(TargetReceipt.strTargetPath).ends_with(
					".unified.effect.json") || nullptr == ParticleRows ||
				nullptr == DecalRows ||
				!Read_U32(Target, "outputElementCount",
					TargetReceipt.iOutputElementCount) ||
				!Read_U32(Target, "sourceParticleCount",
					TargetReceipt.iSourceParticleCount) ||
				!Read_U32(Target, "sourceDecalCount",
					TargetReceipt.iSourceDecalCount) ||
				!Read_U32(Target, "sourceDecalBaseReadyCount",
					TargetReceipt.iSourceDecalReadyCount) ||
				!Read_U32(Target, "sourceDecalIncompleteCount",
					TargetReceipt.iSourceDecalIncompleteCount) ||
				!Read_U32(Target, "supplementalPreservedCount",
					TargetReceipt.iSupplementalPreservedCount) ||
				!Read_U32(Target, "placeholderTrailExcludedCount",
					TargetReceipt.iPlaceholderTrailExcludedCount) ||
				!Read_U32(Target, "portableCount", TargetReceipt.iPortableCount) ||
				!Read_U32(Target, "sourcePreservedDeferredCount",
					TargetReceipt.iRecipeDeferredCount) ||
				!Read_U32(Target, "drawableAdmittedCount",
					TargetReceipt.iDrawableAdmittedCount) ||
				!Read_U32(Target, "portableFailClosedCount",
					TargetReceipt.iPortableFailClosedCount))
			{
				OutError = "Track A restoration target receipt is invalid.";
				return false;
			}
			std::set<std::string, std::less<>> TargetElementIds;
			std::set<std::string, std::less<>> SourceLineages;
			uint32_t DerivedPortableCount = 0u;
			uint32_t DerivedRecipeDeferredCount = 0u;
			uint32_t DerivedDrawableAdmittedCount = 0u;
			uint32_t DerivedPortableFailClosedCount = 0u;
			uint32_t DerivedDecalReadyCount = 0u;
			uint32_t DerivedDecalIncompleteCount = 0u;
			for (const DATA_JSON_VALUE& RowValue : ParticleRows->Get_Array())
			{
				RESTORATION_RECEIPT_ROW Row;
				if (!Parse_RestorationParticleRow(RowValue, Row) ||
					!TargetElementIds.insert(Row.strTargetElementId).second ||
					!SourceLineages.insert("particle|" + Row.strSourceElementId +
						"|" + Row.strSourceEventId).second)
				{
					OutError = "Track A Particle receipt lineage is invalid: " +
						TargetReceipt.strEffectAssetId;
					return false;
				}
				if (!Row.bPortable)
					++DerivedRecipeDeferredCount;
				else
				{
					++DerivedPortableCount;
					if (Row.FailClosedReasons.empty())
						++DerivedDrawableAdmittedCount;
					else
						++DerivedPortableFailClosedCount;
				}
				TargetReceipt.Rows.push_back(std::move(Row));
			}
			for (const DATA_JSON_VALUE& RowValue : DecalRows->Get_Array())
			{
				RESTORATION_RECEIPT_ROW Row;
				if (!Parse_RestorationDecalRow(RowValue, Row) ||
					!TargetElementIds.insert(Row.strTargetElementId).second ||
					!SourceLineages.insert("decal|" + Row.strSourceElementId +
						"|" + Row.strSourceEventId).second)
				{
					OutError = "Track A Decal receipt lineage is invalid: " +
						TargetReceipt.strEffectAssetId;
					return false;
				}
				if (Row.bPortable)
					++DerivedDecalReadyCount;
				else
					++DerivedDecalIncompleteCount;
				TargetReceipt.Rows.push_back(std::move(Row));
			}
			const DATA_JSON_VALUE* TrailRows = Target.Find("animationTrailRows");
			if (nullptr == TrailRows)
				TrailRows = Target.Find("sourceTrailRows");
			if (nullptr == TrailRows)
				TrailRows = Target.Find("trailRows");
			if (nullptr != TrailRows)
			{
				if (!TrailRows->Is_Array())
				{
					OutError = "Track A AnimationTrail receipt rows are invalid.";
					return false;
				}
				for (const DATA_JSON_VALUE& RowValue : TrailRows->Get_Array())
				{
					RESTORATION_RECEIPT_ROW Row;
					if (!Parse_RestorationAnimationTrailRow(RowValue, Row) ||
						!TargetElementIds.insert(Row.strTargetElementId).second ||
						!SourceLineages.insert("animationTrail|" +
							Row.strSourceElementId + "|" +
							Row.strSourceEventId).second ||
						!GlobalTrailLineages.insert(Row.strSourceElementId + "|" +
							Row.strSourceEventId).second)
					{
						OutError = "Track A AnimationTrail receipt lineage is invalid: " +
							TargetReceipt.strEffectAssetId;
						return false;
					}
					++AnimationTrailRowCount;
					GlobalTrailNotifyIdentities.insert(
						TargetReceipt.strCharacterClass + "|" +
						std::to_string(TargetReceipt.iSkillId) + "|" +
						Row.strSourceEventId);
					TargetReceipt.Rows.push_back(std::move(Row));
				}
			}
			if (TargetReceipt.iSourceParticleCount !=
					ParticleRows->Get_Array().size() ||
				TargetReceipt.iSourceDecalCount != DecalRows->Get_Array().size() ||
				DerivedPortableCount != TargetReceipt.iPortableCount ||
				DerivedRecipeDeferredCount !=
					TargetReceipt.iRecipeDeferredCount ||
				DerivedDrawableAdmittedCount !=
					TargetReceipt.iDrawableAdmittedCount ||
				DerivedPortableFailClosedCount !=
					TargetReceipt.iPortableFailClosedCount ||
				DerivedDecalReadyCount !=
					TargetReceipt.iSourceDecalReadyCount ||
				DerivedDecalIncompleteCount !=
					TargetReceipt.iSourceDecalIncompleteCount ||
				TargetReceipt.iSourceDecalReadyCount +
					TargetReceipt.iSourceDecalIncompleteCount !=
					TargetReceipt.iSourceDecalCount ||
				TargetReceipt.iPortableCount + TargetReceipt.iRecipeDeferredCount !=
					TargetReceipt.iSourceParticleCount ||
				TargetReceipt.iDrawableAdmittedCount +
					TargetReceipt.iPortableFailClosedCount !=
					TargetReceipt.iPortableCount ||
				TargetReceipt.iSourceParticleCount +
					TargetReceipt.iSourceDecalCount +
					TargetReceipt.iSupplementalPreservedCount !=
					TargetReceipt.iOutputElementCount ||
				(nullptr == TrailRows ? 0u : TrailRows->Get_Array().size()) +
					TargetReceipt.iPlaceholderTrailExcludedCount !=
					TargetReceipt.iSupplementalPreservedCount ||
				TargetReceipt.iPlaceholderTrailExcludedCount >
					TargetReceipt.iSupplementalPreservedCount)
			{
				OutError = "Track A target receipt denominator is invalid: " +
					TargetReceipt.strEffectAssetId;
				return false;
			}
			OutputElementCount += TargetReceipt.iOutputElementCount;
			ParticleCount += TargetReceipt.iSourceParticleCount;
			DecalCount += TargetReceipt.iSourceDecalCount;
			DecalReadyCount += TargetReceipt.iSourceDecalReadyCount;
			DecalIncompleteCount += TargetReceipt.iSourceDecalIncompleteCount;
			SupplementalCount += TargetReceipt.iSupplementalPreservedCount;
			PlaceholderTrailCount +=
				TargetReceipt.iPlaceholderTrailExcludedCount;
			PortableCount += TargetReceipt.iPortableCount;
			RecipeDeferredCount += TargetReceipt.iRecipeDeferredCount;
			AdmittedCount += TargetReceipt.iDrawableAdmittedCount;
			PortableFailClosedCount +=
				TargetReceipt.iPortableFailClosedCount;
			Parsed.Targets.emplace(TargetReceipt.strEffectAssetId,
				std::move(TargetReceipt));
		}

		std::set<std::string, std::less<>> CharacterGhostCueIds;
		std::set<std::string, std::less<>> CharacterGhostOccurrenceIds;
		std::set<std::string, std::less<>> CharacterGhostTargetIds;
		std::map<std::string, uint32_t, std::less<>> CharacterGhostClassCounts;
		for (const DATA_JSON_VALUE& Cue : CharacterGhostCues->Get_Array())
		{
			if (!Has_ExactKeys(Cue, { "cueId", "characterClass", "skillId",
					"stageIndex", "stageClipIndex", "targetEffectAssetId", "clip",
					"sourceEventId", "sourceReceiptEventIndex", "rawNotifyId",
					"sourceStageIndex", "localTimeSeconds", "globalTimeSeconds",
					"durationSeconds", "serializedPayload", "serializedLabels",
					"assetReferences", "materialReferences", "sourceArtifact",
					"semanticFamily", "admitted", "admission" }) ||
				Cue.Find("targetElementId") != nullptr ||
				Cue.Find("kind") != nullptr)
			{
				OutError =
					"CharacterGhost receipt row leaked an Element projection field.";
				return false;
			}
			std::string CueId;
			std::string CharacterClass;
			std::string TargetEffectId;
			std::string Clip;
			std::string SourceEventId;
			std::string RawNotifyId;
			std::string SemanticFamily;
			uint32_t SkillId = 0u;
			uint32_t StageIndex = 0u;
			uint32_t StageClipIndex = 0u;
			uint32_t SourceReceiptEventIndex = 0u;
			uint32_t SourceStageIndex = 0u;
			const DATA_JSON_VALUE* LocalTime = Required(
				Cue, "localTimeSeconds", DATA_JSON_TYPE::NUMBER);
			const DATA_JSON_VALUE* GlobalTime = Required(
				Cue, "globalTimeSeconds", DATA_JSON_TYPE::NUMBER);
			const DATA_JSON_VALUE* Duration = Required(
				Cue, "durationSeconds", DATA_JSON_TYPE::NUMBER);
			const DATA_JSON_VALUE* SerializedLabels = Required(
				Cue, "serializedLabels", DATA_JSON_TYPE::ARRAY);
			const DATA_JSON_VALUE* AssetReferences = Required(
				Cue, "assetReferences", DATA_JSON_TYPE::ARRAY);
			const DATA_JSON_VALUE* MaterialReferences = Required(
				Cue, "materialReferences", DATA_JSON_TYPE::ARRAY);
			const DATA_JSON_VALUE* SourceArtifact = Required(
				Cue, "sourceArtifact", DATA_JSON_TYPE::OBJECT);
			const DATA_JSON_VALUE* Admitted = Required(
				Cue, "admitted", DATA_JSON_TYPE::BOOLEAN);
			const DATA_JSON_VALUE* Admission = Required(
				Cue, "admission", DATA_JSON_TYPE::OBJECT);
			if (!Read_String(Cue, "cueId", CueId) ||
				!Read_String(Cue, "characterClass", CharacterClass) ||
				!Read_U32(Cue, "skillId", SkillId) ||
				!Read_U32(Cue, "stageIndex", StageIndex) ||
				!Read_U32(Cue, "stageClipIndex", StageClipIndex) ||
				!Read_String(Cue, "targetEffectAssetId", TargetEffectId) ||
				!Read_String(Cue, "clip", Clip) ||
				!Read_String(Cue, "sourceEventId", SourceEventId) ||
				!Read_U32(Cue, "sourceReceiptEventIndex",
					SourceReceiptEventIndex) ||
				!Read_String(Cue, "rawNotifyId", RawNotifyId) ||
				!Read_U32(Cue, "sourceStageIndex", SourceStageIndex) ||
				!Read_String(Cue, "semanticFamily", SemanticFamily) ||
				nullptr == LocalTime || nullptr == GlobalTime || nullptr == Duration ||
				nullptr == SerializedLabels || nullptr == AssetReferences ||
				nullptr == MaterialReferences || nullptr == SourceArtifact ||
				nullptr == Admitted || nullptr == Admission ||
				!Is_StableId(CueId) ||
				!std::string_view(CueId).starts_with("character-ghost.") ||
				!Is_StableId(SourceEventId) ||
				!Is_StableId(TargetEffectId) ||
				SemanticFamily != "CHARACTER_AFTERIMAGE" ||
				Admitted->Get_Boolean() ||
				!std::isfinite(LocalTime->Get_Number()) ||
				!std::isfinite(GlobalTime->Get_Number()) ||
				!std::isfinite(Duration->Get_Number()) ||
				LocalTime->Get_Number() < 0.0 || GlobalTime->Get_Number() < 0.0 ||
				Duration->Get_Number() < 0.0 ||
				SerializedLabels->Get_Array().size() > 1024u ||
				AssetReferences->Get_Array().size() > 1024u ||
				MaterialReferences->Get_Array().size() > 1024u ||
				SourceArtifact->Get_Object().empty() ||
				!CharacterGhostCueIds.insert(CueId).second ||
				!CharacterGhostOccurrenceIds.insert(CharacterClass + "|" +
					std::to_string(SkillId) + "|" + SourceEventId).second)
			{
				OutError = "CharacterGhost receipt identity/timing is invalid.";
				return false;
			}
			UNREFERENCED_PARAMETER(SourceReceiptEventIndex);
			UNREFERENCED_PARAMETER(SourceStageIndex);
			UNREFERENCED_PARAMETER(RawNotifyId);
			const auto TargetIterator = Parsed.Targets.find(TargetEffectId);
			if (TargetIterator == Parsed.Targets.end() ||
				TargetIterator->second.strCharacterClass != CharacterClass ||
				TargetIterator->second.iSkillId != SkillId ||
				TargetIterator->second.iStageIndex != StageIndex ||
				TargetIterator->second.iStageClipIndex != StageClipIndex ||
				TargetIterator->second.strClip != Clip)
			{
				OutError = "CharacterGhost cue/target join is invalid: " + CueId;
				return false;
			}
			CharacterGhostTargetIds.insert(TargetEffectId);
			++CharacterGhostClassCounts[CharacterClass];

			if (!Has_ExactKeys(*Admission,
					{ "admitted", "failClosed", "blocker" }))
			{
				OutError = "CharacterGhost admission object is invalid.";
				return false;
			}
			const DATA_JSON_VALUE* AdmissionAdmitted = Required(
				*Admission, "admitted", DATA_JSON_TYPE::BOOLEAN);
			const DATA_JSON_VALUE* FailClosed = Required(
				*Admission, "failClosed", DATA_JSON_TYPE::BOOLEAN);
			std::string Blocker;
			if (nullptr == AdmissionAdmitted || nullptr == FailClosed ||
				AdmissionAdmitted->Get_Boolean() ||
				!FailClosed->Get_Boolean() ||
				!Read_String(*Admission, "blocker", Blocker) ||
				Blocker !=
					"POSE_RUNTIME_BODY_EQUIPMENT_SNAPSHOT_AND_GHOST_"
					"MATERIAL_EXECUTION_UNAVAILABLE")
			{
				OutError = "CharacterGhost pose-runtime fail-closed contract changed.";
				return false;
			}

			const DATA_JSON_VALUE* Payload = Required(
				Cue, "serializedPayload", DATA_JSON_TYPE::OBJECT);
			std::string PayloadBase64;
			std::string PayloadSha256;
			std::string PayloadEncoding;
			uint32_t PayloadByteOffset = 0u;
			uint32_t PayloadByteSize = 0u;
			std::string DecodedPayload;
			if (nullptr == Payload ||
				!Has_ExactKeys(*Payload, { "encoding", "byteOffset", "byteSize",
					"sha256", "data" }) ||
				!Read_String(*Payload, "encoding", PayloadEncoding) ||
				PayloadEncoding != "base64" ||
				!Read_U32(*Payload, "byteOffset", PayloadByteOffset) ||
				!Read_String(*Payload, "data", PayloadBase64, false,
					4u * 1024u * 1024u) ||
				!Read_U32(*Payload, "byteSize", PayloadByteSize) ||
				!Read_String(*Payload, "sha256", PayloadSha256) ||
				!Is_Sha256(PayloadSha256) ||
				!Decode_Base64Strict(PayloadBase64, DecodedPayload) ||
				DecodedPayload.size() != PayloadByteSize ||
				CEffectRuntimeAuthorityCodec::Compute_Sha256Hex(DecodedPayload) !=
					PayloadSha256)
			{
				OutError = "CharacterGhost serialized payload identity changed: " +
					CueId;
				return false;
			}
			UNREFERENCED_PARAMETER(PayloadByteOffset);
		}
		const std::map<std::string, uint32_t, std::less<>>
			ExpectedCharacterGhostClassCounts = {
				{ "ARTIST", 1u }, { "DIMENSIONMASTER", 41u },
				{ "LANCE_MASTER", 24u }, { "WARLORD", 6u } };
		if (Parsed.Targets.size() != Parsed.iTargetCount ||
			OutputElementCount != Parsed.iOutputElementCount ||
			ParticleCount != Parsed.iStrictMappedParticleCount ||
			DecalCount != Parsed.iSourceDecalCount ||
			DecalReadyCount != Parsed.iSourceDecalReadyCount ||
			DecalIncompleteCount != Parsed.iSourceDecalIncompleteCount ||
			SupplementalCount != Parsed.iSupplementalPreservedCount ||
			PlaceholderTrailCount != Parsed.iPlaceholderTrailExcludedCount ||
			PortableCount != Parsed.iPortableCount ||
			RecipeDeferredCount != Parsed.iRecipeDeferredCount ||
			AdmittedCount != Parsed.iDrawableAdmittedCount ||
			PortableFailClosedCount != Parsed.iPortableFailClosedCount ||
			CharacterGhostCueIds.size() != Parsed.iCharacterGhostCueCount ||
			CharacterGhostOccurrenceIds.size() !=
				Parsed.iCharacterGhostCueCount ||
			CharacterGhostTargetIds.size() !=
				Parsed.iCharacterGhostTargetCount ||
			CharacterGhostClassCounts != ExpectedCharacterGhostClassCounts ||
			(AnimationTrailRowCount > 0u &&
			 !Parsed.bHasAnimationTrailDenominators) ||
			(Parsed.bHasAnimationTrailDenominators &&
			 (AnimationTrailRowCount !=
				Parsed.iSourceAnimationTrailElementCount ||
			  GlobalTrailNotifyIdentities.size() !=
				Parsed.iSourceAnimationTrailNotifyCount)))
		{
			OutError = "Track A target/global receipt denominators diverge.";
			return false;
		}
		OutReceipt = std::move(Parsed);
		return true;
	}

	bool Resolve_ImportedDocumentPath(
		const std::filesystem::path& RepositoryRoot,
		std::string_view EffectAssetId,
		std::filesystem::path& OutPath,
		std::string& OutError);
	std::string Extract_SourceElementId(std::string_view SourceNode);

	bool Validate_RestorationSourceLineage(
		const std::filesystem::path& RepositoryRoot,
		const RESTORATION_RECEIPT& Receipt,
		std::string& OutError)
	{
		struct SOURCE_OCCURRENCE_EXPECTATION final
		{
			std::string_view strTargetEffectId;
			const RESTORATION_RECEIPT_ROW* pRow = nullptr;
		};
		std::map<std::string, std::vector<SOURCE_OCCURRENCE_EXPECTATION>,
			std::less<>> Expectations;
		for (const auto& [TargetEffectId, Target] : Receipt.Targets)
		{
			for (const RESTORATION_RECEIPT_ROW& Row : Target.Rows)
			{
				if (Row.eKind == RESTORATION_RECEIPT_ROW_KIND::ANIMATION_TRAIL)
					continue;
				Expectations[Row.strSourceEffectAssetId].push_back(
					{ TargetEffectId, &Row });
			}
		}

		for (const auto& [SourceEffectId, Occurrences] : Expectations)
		{
			std::filesystem::path Path;
			std::string Bytes;
			DATA_JSON_VALUE SourceDocument;
			if (!Resolve_ImportedDocumentPath(RepositoryRoot,
					SourceEffectId, Path, OutError) ||
				!Read_File(Path, Bytes, OutError) ||
				!Parse_LargeImportedSourceJson(Bytes, SourceDocument, OutError))
			{
				OutError = "Track A source document load failed for " +
					SourceEffectId + ": " + OutError;
				return false;
			}
			const DATA_JSON_VALUE* EffectId = Required(
				SourceDocument, "effectAssetId", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* Elements = Required(
				SourceDocument, "elements", DATA_JSON_TYPE::ARRAY);
			if (nullptr == EffectId || EffectId->Get_String() != SourceEffectId ||
				nullptr == Elements)
			{
				OutError = "Track A source document identity changed: " +
					SourceEffectId;
				return false;
			}
			std::map<std::string_view, const DATA_JSON_VALUE*, std::less<>>
				ElementsById;
			for (const DATA_JSON_VALUE& Element : Elements->Get_Array())
			{
				const DATA_JSON_VALUE* Id = Required(
					Element, "id", DATA_JSON_TYPE::STRING);
				if (nullptr == Id ||
					!ElementsById.emplace(Id->Get_String(), &Element).second)
				{
					OutError = "Track A source Element identity is invalid: " +
						SourceEffectId;
					return false;
				}
			}
			for (const SOURCE_OCCURRENCE_EXPECTATION& Occurrence : Occurrences)
			{
				if (nullptr == Occurrence.pRow)
					return false;
				const RESTORATION_RECEIPT_ROW& Row = *Occurrence.pRow;
				const auto Found = ElementsById.find(Row.strSourceElementId);
				const DATA_JSON_VALUE* Element = Found == ElementsById.end() ?
					nullptr : Found->second;
				const DATA_JSON_VALUE* Kind = nullptr == Element ? nullptr :
					Required(*Element, "kind", DATA_JSON_TYPE::STRING);
				const DATA_JSON_VALUE* Recipe = nullptr == Element ? nullptr :
					Required(*Element, "sourceRecipe", DATA_JSON_TYPE::OBJECT);
				const DATA_JSON_VALUE* Detail = nullptr == Element ? nullptr :
					Required(*Element, "detail", DATA_JSON_TYPE::OBJECT);
				const DATA_JSON_VALUE* Bindings = nullptr == Element ? nullptr :
					Required(*Element, "resources", DATA_JSON_TYPE::ARRAY);
				const std::string_view ExpectedKind =
					Row.eKind == RESTORATION_RECEIPT_ROW_KIND::PARTICLE ?
						"particle" : "decal";
				if (nullptr == Element || nullptr == Kind || nullptr == Recipe ||
					nullptr == Detail || nullptr == Bindings ||
					Kind->Get_String() != ExpectedKind ||
					Canonical_JsonSha256(*Recipe) !=
						Row.strSourceRecipeCanonicalSha256 ||
					Canonical_JsonSha256(*Bindings) !=
						Row.strSourceBindingsCanonicalSha256 ||
					(!Row.strSourceDetailCanonicalSha256.empty() &&
					 Canonical_JsonSha256(*Detail) !=
						Row.strSourceDetailCanonicalSha256))
				{
					OutError = "Track A source lineage/hash changed: " +
						std::string(Occurrence.strTargetEffectId) + "/" +
						Row.strTargetElementId + " <- " +
						Row.strSourceEffectAssetId + "/" + Row.strSourceElementId;
					return false;
				}
			}
		}
		return true;
	}

	bool Validate_RestorationTargetProjection(
		const RESTORATION_TARGET_RECEIPT& Target,
		const DATA_JSON_VALUE& RawDocument,
		std::string& OutError)
	{
		const DATA_JSON_VALUE* Elements = Required(
			RawDocument, "elements", DATA_JSON_TYPE::ARRAY);
		if (nullptr == Elements)
		{
			OutError = "Track A target document has no Element array: " +
				Target.strEffectAssetId;
			return false;
		}
		for (const RESTORATION_RECEIPT_ROW& Row : Target.Rows)
		{
			const DATA_JSON_VALUE* Element = Find_UniqueObjectByString(
				*Elements, "id", Row.strTargetElementId);
			const DATA_JSON_VALUE* SourceNode = nullptr == Element ? nullptr :
				Required(*Element, "sourceNode", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* SourcePresentation = nullptr == Element ? nullptr :
				Required(*Element, "sourcePresentation", DATA_JSON_TYPE::OBJECT);
			const DATA_JSON_VALUE* SourceEventId =
				nullptr == SourcePresentation ? nullptr : Required(
					*SourcePresentation, "sourceEventId", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* SourcePresentationEnabled =
				nullptr == SourcePresentation ? nullptr : Required(
					*SourcePresentation, "enabled", DATA_JSON_TYPE::BOOLEAN);
			const bool bAnimationTrail =
				Row.eKind == RESTORATION_RECEIPT_ROW_KIND::ANIMATION_TRAIL;
			const bool bContradictoryEvent = nullptr != SourceEventId &&
				!SourceEventId->Get_String().empty() &&
				SourceEventId->Get_String() != Row.strSourceEventId;
			const bool bInvalidTrailEvent = bAnimationTrail &&
				(nullptr == SourcePresentationEnabled ||
				 !SourcePresentationEnabled->Get_Boolean() ||
				 nullptr == SourceEventId || SourceEventId->Get_String().empty() ||
				 SourceEventId->Get_String() != Row.strSourceEventId);
			if (nullptr == Element || nullptr == SourceNode ||
				nullptr == SourcePresentationEnabled || nullptr == SourceEventId ||
				Extract_SourceElementId(SourceNode->Get_String()) !=
					Row.strSourceElementId ||
				bContradictoryEvent || bInvalidTrailEvent)
			{
				OutError = "Track A target receipt lineage changed: " +
					Target.strEffectAssetId + "/" + Row.strTargetElementId;
				return false;
			}
			/* Disabled ordinary sourcePresentation rows intentionally lose their
			   provenance-only fields in the typed codec.  Validate the event join
			   against the hash-pinned raw projection above, before that lowering. */
			if (bAnimationTrail)
				continue;
			const DATA_JSON_VALUE* Recipe = nullptr == Element ? nullptr :
				Required(*Element, "sourceRecipe", DATA_JSON_TYPE::OBJECT);
			const DATA_JSON_VALUE* Detail = nullptr == Element ? nullptr :
				Required(*Element, "detail", DATA_JSON_TYPE::OBJECT);
			const DATA_JSON_VALUE* Bindings = nullptr == Element ? nullptr :
				Required(*Element, "resources", DATA_JSON_TYPE::ARRAY);
			if (nullptr == Element || nullptr == Recipe || nullptr == Detail ||
				nullptr == Bindings ||
				Canonical_JsonSha256(*Recipe) !=
					Row.strNormalizedRecipeCanonicalSha256 ||
				Canonical_JsonSha256(*Bindings) !=
					Row.strTargetBindingsCanonicalSha256 ||
				(!Row.strTargetDetailCanonicalSha256.empty() &&
				 Canonical_JsonSha256(*Detail) !=
					Row.strTargetDetailCanonicalSha256))
			{
				OutError = "Track A target receipt projection changed: " +
					Target.strEffectAssetId + "/" + Row.strTargetElementId;
				return false;
			}
		}
		return true;
	}

	bool Load_CurrentRestorationCandidates(
		const std::filesystem::path& RepositoryRoot,
		std::vector<STAGED_TARGET_FILE>& OutFiles,
		RESTORATION_RECEIPT& OutReceipt,
		std::string& OutError)
	{
		RESTORATION_RECEIPT Receipt;
		if (!Load_RestorationReceipt(RepositoryRoot, Receipt, OutError) ||
			!Validate_RestorationSourceLineage(
				RepositoryRoot, Receipt, OutError))
			return false;
		std::vector<STAGED_TARGET_FILE> Files;
		Files.reserve(Receipt.Targets.size());
		for (const auto& [EffectId, Target] : Receipt.Targets)
		{
			std::filesystem::path Path;
			std::string Bytes;
			DATA_JSON_VALUE RawDocument;
			EFFECT_DOCUMENT_DESC Document;
			if (!Resolve_ArtifactPath(
					RepositoryRoot, Target.strTargetPath, Path, OutError) ||
				!Read_File(Path, Bytes, OutError) ||
				Bytes.find("TrailGhostEffect") != std::string::npos ||
				Bytes.find("CHARACTER_AFTERIMAGE") != std::string::npos ||
				!Parse_Json(Bytes, RawDocument, OutError) ||
				Canonical_JsonSha256(RawDocument) !=
					Target.strOutputCanonicalSha256 ||
				!Validate_RestorationTargetProjection(
					Target, RawDocument, OutError) ||
				!CEffectDocumentCodec::Parse(Bytes, Document, OutError) ||
				Document.strEffectAssetId != EffectId ||
				Document.Elements.size() != Target.iOutputElementCount)
			{
				const std::string Detail = OutError.empty() ?
					"canonical hash, effect identity, or Element count changed" :
					OutError;
				OutError = "Track A restoration document/receipt mismatch: " +
					EffectId + " (" + Target.strTargetPath + "): " + Detail;
				return false;
			}
			STAGED_TARGET_FILE File;
			File.Path = std::move(Path);
			File.strCanonicalDocument = CEffectDocumentCodec::Serialize(Document);
			Files.push_back(std::move(File));
		}
		OutFiles = std::move(Files);
		OutReceipt = std::move(Receipt);
		return true;
	}

	struct FOCUSED_TRACK_A_DRAW_CASE final
	{
		std::string_view strEffectAssetId;
		std::string_view strElementId;
		std::string_view strSourceElementId;
		std::string_view strRendererShape;
		std::string_view strShapeAssetId;
		std::string_view strBaseAssetId;
	};

	constexpr std::array<FOCUSED_TRACK_A_DRAW_CASE, 9u>
		FOCUSED_TRACK_A_DRAW_CASES = {{
			{ "effect.artist.skill.31000.ba1.unified",
				{}, "fx_pc_sdm_00.par_o_sdm_att_08_01.particlespriteemitter_5",
				"sprite", {},
				"Effect/Artist/Textures/fx_i_rainbowring_01.dds" },
			{ "effect.artist.skill.31200.unified",
				{}, "fx_pc_sdm_04.par_w_sdm_inkpaddle_02.particlespriteemitter_99",
				"mesh",
				"Effect/Artist/Meshes/fm_m_trail_002.wmodel",
				"Effect/Artist/Textures/fx_d_environ_001.dds" },
			{ "effect.dimensionmaster.skill.2050010.ba1.unified",
				{}, "fx_pc_swp_00.par_j_swp_normalatk_0_1.particlespriteemitter_11",
				"mesh",
				"Effect/DimensionMaster/Meshes/fm_d_plane_002.wmodel",
				"Effect/DimensionMaster/Textures/FX_TEX_06/fx_j_caustic_tile_02.dds" },
			{ "effect.dimensionmaster.skill.2050010.ba1.unified",
				{}, "fx_pc_swp_00.par_j_swp_normalatk_0_1.particlespriteemitter_2",
				"sprite", {},
				"Effect/DimensionMaster/Textures/FX_TEX_02/fx_d_noise_014.dds" },
			{ "effect.lancemaster.skill.34010.ba1.unified",
				{}, "fx_pc_flm_01.par_m_flm_pyungmtrail_01.particlespriteemitter_4",
				"mesh",
				"Effect/LanceMaster/Meshes/fm_m_ring_001.wmodel",
				"Effect/LanceMaster/Textures/fx_h_atypical_01_1.dds" },
			{ "effect.lancemaster.skill.34010.ba4.unified",
				{}, "fx_pc_flm_01.par_m_flm_pyungimpact_01.particlespriteemitter_5",
				"sprite", {},
				"Effect/LanceMaster/Textures/fx_m_trail_004_cl.dds" },
			{ "effect.warlord.skill.17000.ba1.unified",
				{}, "fx_pc_wgl_06.par_o_wgl_normalatk_01_01.particlespriteemitter_26",
				"mesh",
				"Effect/Warlord/Meshes/FX_SM_00/fm_d_plane_002.wmodel",
				"Effect/Warlord/Textures/FX_TEX_05/fx_k_auraline_16.dds" },
			{ "effect.warlord.skill.17000.ba1.unified",
				{}, "fx_pc_wgl_06.par_o_wgl_normalatk_01_01.particlespriteemitter_81",
				"sprite", {},
				"Effect/Warlord/Textures/FX_TEX_02/fx_d_noise_014.dds" },
			/* This source-backed carrier was omitted by the 11-row starter
			   selection.  Resolve it by compiler identity so the gate also proves
			   the full Warlord 17090 14-Mesh/2-Sprite denominator. */
			{ "effect.warlord.skill.17090.unified", {},
				"fx_pc_wgl_00.par_d_chain_attraction_31.particlespriteemitter_3",
				"mesh",
				"Effect/Warlord/Meshes/FX_SM_01/fm_d_berchain_07.wmodel",
				{} }
		}};

	struct FOCUSED_DM_ROLE_CONTRACT final
	{
		std::string_view strSourceElementId;
		std::string_view strModelAssetId;
		std::string_view strSourceProfileId;
		float3_t vExpectedAbsoluteWorldScale{};
		bool_t bExpectedNegativeDeterminant = false;
		uint32_t iExpectedPass = UINT32_MAX;
		uint64_t iExpectedVertexCount = 0u;
		uint64_t iExpectedIndexCount = 0u;
		f32_t fExpectedRawRadius = 0.f;
	};

	constexpr std::array<std::string_view, 6u>
		DIMENSION_MASTER_A_GOLDEN_SOURCE_ELEMENTS = {{
			"fx_pc_swp_00.par_j_swp_willowrend_swinghit_00_1.particlespriteemitter_2",
			"fx_pc_swp_00.par_j_swp_willowrend_swinghit_00_1.particlespriteemitter_14",
			"fx_pc_swp_00.par_j_swp_willowrend_swinghit_00_1.particlespriteemitter_15",
			"fx_pc_swp_00.par_j_swp_willowrend_swinghit_00_1.particlespriteemitter_20",
			"fx_pc_swp_00.par_j_swp_willowrend_swinghit_00_1.particlespriteemitter_3",
			"fx_pc_swp_00.par_j_swp_willowrend_swinghit_00_1.particlespriteemitter_9"
		}};
	constexpr std::array<std::string_view, 4u>
		DIMENSION_MASTER_A_GOLDEN_SOURCE_EVENTS = {{
			"source-event-009", "source-event-030",
			"source-event-045", "source-event-060"
		}};

	const std::array<FOCUSED_DM_ROLE_CONTRACT, 5u> FOCUSED_DM_ROLES = {{
		{ "fx_pc_swp_00.par_j_swp_willowrend_swinghit_00_1.particlespriteemitter_2",
			"Effect/DimensionMaster/Meshes/fm_h_swing_05.wmodel",
			"effect.ue3.grouped-translucent.v1", {}, false, UINT32_MAX,
			294u, 1476u, 0.f },
		{ "fx_pc_swp_00.par_j_swp_willowrend_swinghit_00_1.particlespriteemitter_14",
			"Effect/DimensionMaster/Meshes/fm_m_trail_002.wmodel",
			"effect.ue3.linearflow-02.v1", { 5.5f, 5.5f, 5.5f },
			true, 5u, 63u, 240u, 44.1052f },
		{ "fx_pc_swp_00.par_j_swp_willowrend_swinghit_00_1.particlespriteemitter_15",
			"Effect/DimensionMaster/Meshes/fm_h_swing_02.wmodel",
			"effect.ue3.linearflow-02.v1", { 3.41f, 3.41f, 5.5f },
			true, 5u, 166u, 246u, 70.f },
		{ "fx_pc_swp_00.par_j_swp_willowrend_swinghit_00_1.particlespriteemitter_20",
			"Effect/DimensionMaster/Meshes/fm_m_trail_01.wmodel",
			"effect.ue3.linearflow-02.v1", { 7.7f, 7.7f, 4.4f },
			false, 3u, 78u, 288u, 44.4345f },
		{ "fx_pc_swp_00.par_j_swp_willowrend_swinghit_00_1.particlespriteemitter_3",
			"Effect/DimensionMaster/Meshes/fm_h_swing_02.wmodel",
			"effect.ue3.blackline-aura.v1", { 1.1f, 3.135f, 3.355f },
			false, 3u, 166u, 246u, 70.f }
	}};

	struct DECODED_MODEL_METRICS final
	{
		uint64_t iVertexCount = 0u;
		uint64_t iIndexCount = 0u;
		uint32_t iMeshCount = 0u;
		float3_t vMinimum{};
		float3_t vMaximum{};
		f32_t fRadius = 0.f;
	};

	bool Nearly_Equal(const f32_t Left, const f32_t Right,
		const f32_t AbsoluteTolerance = 0.0001f,
		const f32_t RelativeTolerance = 0.0001f)
	{
		return std::abs(Left - Right) <= AbsoluteTolerance +
			RelativeTolerance * (std::max)(std::abs(Left), std::abs(Right));
	}

	bool Same_Float3(const float3_t& Left, const float3_t& Right,
		const f32_t Tolerance = 0.00001f)
	{
		return Nearly_Equal(Left.x, Right.x, Tolerance, Tolerance) &&
			Nearly_Equal(Left.y, Right.y, Tolerance, Tolerance) &&
			Nearly_Equal(Left.z, Right.z, Tolerance, Tolerance);
	}

	bool Same_Transform(const Client::EFFECT_TRANSFORM_DESC& Left,
		const Client::EFFECT_TRANSFORM_DESC& Right)
	{
		return Same_Float3(Left.vPosition, Right.vPosition) &&
			Same_Float3(Left.vRotationDegrees, Right.vRotationDegrees) &&
			Same_Float3(Left.vRevolutionDegreesPerSecond,
				Right.vRevolutionDegreesPerSecond) &&
			Same_Float3(Left.vScale, Right.vScale) &&
			Same_Float3(Left.vVelocityPerSecond, Right.vVelocityPerSecond);
	}

	const Client::EFFECT_RESOURCE_BINDING_DESC* Find_ResourceBinding(
		const EFFECT_ELEMENT_DESC& Element, const std::string_view SlotId)
	{
		const auto Iterator = std::find_if(Element.ResourceBindings.begin(),
			Element.ResourceBindings.end(), [SlotId](const auto& Binding)
			{
				return Binding.strSlotId == SlotId;
			});
		return Iterator == Element.ResourceBindings.end() ? nullptr : &*Iterator;
	}

	std::string Extract_SourceElementId(const std::string_view SourceNode)
	{
		for (const std::string_view Marker : { "|element:", "|source:" })
		{
			const size_t Position = SourceNode.rfind(Marker);
			if (Position != std::string_view::npos)
				return std::string(SourceNode.substr(Position + Marker.size()));
		}
		return {};
	}

	std::string_view SourceElementBaseId(const std::string_view SourceElementId)
	{
		const size_t Event = SourceElementId.find(".event_source-event-");
		return Event == std::string_view::npos ? SourceElementId :
			SourceElementId.substr(0u, Event);
	}

	const RESTORATION_TARGET_RECEIPT* Find_RestorationTarget(
		const RESTORATION_RECEIPT& Receipt, const std::string_view EffectAssetId)
	{
		const auto Target = Receipt.Targets.find(EffectAssetId);
		return Target == Receipt.Targets.end() ? nullptr : &Target->second;
	}

	const RESTORATION_RECEIPT_ROW* Find_RestorationRow(
		const RESTORATION_TARGET_RECEIPT& Target,
		const std::string_view TargetElementId)
	{
		const auto Row = std::find_if(Target.Rows.begin(), Target.Rows.end(),
			[TargetElementId](const RESTORATION_RECEIPT_ROW& Candidate)
			{
				return Candidate.strTargetElementId == TargetElementId;
			});
		return Row == Target.Rows.end() ? nullptr : &*Row;
	}

	bool Parse_AuthoredDocument(const std::filesystem::path& Path,
		EFFECT_DOCUMENT_DESC& OutDocument, std::string& OutError)
	{
		std::string Bytes;
		return Read_File(Path, Bytes, OutError) &&
			CEffectDocumentCodec::Parse(Bytes, OutDocument, OutError);
	}

	bool Resolve_ImportedDocumentPath(
		const std::filesystem::path& RepositoryRoot,
		const std::string_view EffectAssetId,
		std::filesystem::path& OutPath,
		std::string& OutError)
	{
		const size_t SkillMarker = EffectAssetId.find(".skill.");
		if (SkillMarker == std::string_view::npos)
		{
			OutError = "Focused draw source identity has no skill marker.";
			return false;
		}
		const size_t SkillBegin = SkillMarker + std::string_view(".skill.").size();
		const size_t SkillEnd = EffectAssetId.find('.', SkillBegin);
		if (SkillEnd == std::string_view::npos || SkillEnd == SkillBegin)
		{
			OutError = "Focused draw source identity has no stable skill id.";
			return false;
		}
		const std::string LowerPrefix(EffectAssetId.substr(0u, SkillMarker));
		std::string Directory;
		std::string FilePrefix = LowerPrefix;
		if (LowerPrefix == "effect.artist") Directory = "Artist";
		else if (LowerPrefix == "effect.dimensionmaster")
			Directory = "DimensionMaster";
		else if (LowerPrefix == "effect.lancemaster")
			Directory = "LanceMaster";
		else if (LowerPrefix == "effect.lance_master")
		{
			Directory = "LanceMaster";
			FilePrefix = "effect.lancemaster";
		}
		else if (LowerPrefix == "effect.warlord") Directory = "Warlord";
		else
		{
			OutError = "Focused draw source identity has an unknown class.";
			return false;
		}
		const std::string ExpectedName = FilePrefix + ".skill." +
			std::string(EffectAssetId.substr(SkillBegin, SkillEnd - SkillBegin)) +
			".imported.effect.json";
		const std::filesystem::path SearchRoot = RepositoryRoot / L"Data" /
			L"Effects" / L"Imported" / Directory;
		std::error_code Error;
		std::filesystem::path Found;
		for (std::filesystem::recursive_directory_iterator Iterator(SearchRoot,
				 std::filesystem::directory_options::skip_permission_denied, Error), End;
			 !Error && Iterator != End; Iterator.increment(Error))
		{
			if (!Iterator->is_regular_file(Error) || Error)
			{
				Error.clear();
				continue;
			}
			if (Iterator->path().filename().string() != ExpectedName)
				continue;
			if (!Found.empty())
			{
				OutError = "Focused draw source document identity is ambiguous: " +
					ExpectedName;
				return false;
			}
			Found = Iterator->path();
		}
		if (Error || Found.empty())
		{
			OutError = "Focused draw source document is missing: " + ExpectedName;
			return false;
		}
		OutPath = std::move(Found);
		return true;
	}

	std::string SourceRecipe_Canonical(
		const Client::EFFECT_CASCADE_RECIPE_DESC& Recipe)
	{
		EFFECT_DOCUMENT_DESC Document;
		Document.strEffectAssetId = "effect.focused.recipe.signature";
		Document.strDisplayName = "Focused recipe signature";
		EFFECT_ELEMENT_DESC Element;
		Element.strElementId = "focused.recipe";
		Element.strDisplayName = "Focused recipe";
		Element.eKind = Client::EFFECT_ELEMENT_KIND::PARTICLE;
		Element.bVisible = false;
		Element.SourceRecipe = Recipe;
		Document.Elements.push_back(std::move(Element));
		return CEffectDocumentCodec::Serialize(Document);
	}

	template <size_t Size>
	bool Has_ExactNamedTextureMask(
		const Client::EFFECT_SOURCE_MATERIAL_DESC& Source,
		const std::array<std::string_view, Size>& RequiredNames,
		const std::filesystem::path& RuntimeResourceRoot)
	{
		uint32_t Mask = 0u;
		for (const Client::EFFECT_NAMED_TEXTURE_DESC& Texture : Source.Textures)
		{
			const auto Name = std::find(
				RequiredNames.begin(), RequiredNames.end(), Texture.strName);
			if (Name == RequiredNames.end() || Texture.strAssetId.empty())
				continue;
			const uint32_t Bit = 1u << static_cast<uint32_t>(
				std::distance(RequiredNames.begin(), Name));
			if (0u != (Mask & Bit))
				return false;
			std::error_code Error;
			if (!std::filesystem::is_regular_file(
					RuntimeResourceRoot / Texture.strAssetId, Error) || Error)
			{
				return false;
			}
			Mask |= Bit;
		}
		return Mask == (1u << Size) - 1u;
	}

	bool Validate_SourceTextureContract(
		const EFFECT_ELEMENT_DESC& Element,
		const std::filesystem::path& RuntimeResourceRoot)
	{
		const auto& Source = Element.Material.SourceMaterial;
		if (!Source.bEnabled)
			return true;
		if (Source.strRuntimeShaderProfileId == "effect.ue3.linearflow-02.v1")
		{
			constexpr std::array<std::string_view, 7u> Names = {{
				"diff_tex", "diff_noise_tex", "a_mask_tex", "a_noise_01_tex",
				"b_mask_tex", "b_noise_01_tex", "dissolve_tex" }};
			return Has_ExactNamedTextureMask(Source, Names, RuntimeResourceRoot);
		}
		if (Source.strRuntimeShaderProfileId == "effect.ue3.blackline-aura.v1")
		{
			constexpr std::array<std::string_view, 5u> Names = {{
				"diffuse_tex", "flow_tex", "mask_a_tex", "mask_b_tex",
				"dissolve_tex" }};
			return Has_ExactNamedTextureMask(Source, Names, RuntimeResourceRoot);
		}
		return true;
	}

	bool Decode_ModelMetrics(const std::filesystem::path& RuntimeResourceRoot,
		const std::string_view AssetId, DECODED_MODEL_METRICS& Out,
		std::string& OutError)
	{
		Engine::MODEL_ASSET_LOAD_DESC Desc{};
		Desc.assetRoot = RuntimeResourceRoot;
		Desc.meshPath = RuntimeResourceRoot / std::filesystem::path(AssetId);
		Engine::MODEL_ASSET_DATA Asset{};
		if (!Engine::CModelDecoderRegistry::Get().Decode(Desc, Asset) ||
			Asset.meshes.empty())
		{
			OutError = "Focused draw WModel decode failed: " + std::string(AssetId);
			return false;
		}
		const f32_t Maximum = (std::numeric_limits<f32_t>::max)();
		float3_t Minimum{ Maximum, Maximum, Maximum };
		float3_t MaximumPoint{ -Maximum, -Maximum, -Maximum };
		const auto Extend = [&Minimum, &MaximumPoint](const float3_t& Position)
		{
			Minimum.x = (std::min)(Minimum.x, Position.x);
			Minimum.y = (std::min)(Minimum.y, Position.y);
			Minimum.z = (std::min)(Minimum.z, Position.z);
			MaximumPoint.x = (std::max)(MaximumPoint.x, Position.x);
			MaximumPoint.y = (std::max)(MaximumPoint.y, Position.y);
			MaximumPoint.z = (std::max)(MaximumPoint.z, Position.z);
		};
		DECODED_MODEL_METRICS Metrics;
		Metrics.iMeshCount = static_cast<uint32_t>(Asset.meshes.size());
		for (const Engine::MODEL_MESH_DATA& Mesh : Asset.meshes)
		{
			Metrics.iVertexCount += Mesh.vertices.size() +
				Mesh.skinnedVertices.size();
			Metrics.iIndexCount += Mesh.indices.size();
			for (const Engine::VTXMESH& Vertex : Mesh.vertices)
				Extend(Vertex.vPosition);
			for (const Engine::VTXANIMMESH& Vertex : Mesh.skinnedVertices)
				Extend(Vertex.vPosition);
		}
		if (0u == Metrics.iVertexCount || 0u == Metrics.iIndexCount ||
			0u != Metrics.iIndexCount % 3u)
		{
			OutError = "Focused draw WModel geometry is empty or non-triangular: " +
				std::string(AssetId);
			return false;
		}
		const float3_t Center{
			(Minimum.x + MaximumPoint.x) * 0.5f,
			(Minimum.y + MaximumPoint.y) * 0.5f,
			(Minimum.z + MaximumPoint.z) * 0.5f };
		const auto ExtendRadius = [&Metrics, &Center](const float3_t& Position)
		{
			const f32_t X = Position.x - Center.x;
			const f32_t Y = Position.y - Center.y;
			const f32_t Z = Position.z - Center.z;
			Metrics.fRadius = (std::max)(Metrics.fRadius,
				std::sqrt(X * X + Y * Y + Z * Z));
		};
		for (const Engine::MODEL_MESH_DATA& Mesh : Asset.meshes)
		{
			for (const Engine::VTXMESH& Vertex : Mesh.vertices)
				ExtendRadius(Vertex.vPosition);
			for (const Engine::VTXANIMMESH& Vertex : Mesh.skinnedVertices)
				ExtendRadius(Vertex.vPosition);
		}
		Metrics.vMinimum = Minimum;
		Metrics.vMaximum = MaximumPoint;
		if (!std::isfinite(Metrics.fRadius) || Metrics.fRadius <= 0.f)
		{
			OutError = "Focused draw WModel bounds are invalid: " +
				std::string(AssetId);
			return false;
		}
		Out = Metrics;
		return true;
	}

	struct FOCUSED_CAMERA_DIAGNOSTIC final
	{
		float3_t vEye{};
		float3_t vTarget{};
		float3_t vEvaluatedCenter{};
		float3_t vViewSpaceCenter{};
		float3_t vNdcCenter{};
		f32_t fVerticalFovDegrees = 60.f;
		f32_t fAspect = 16.f / 9.f;
		f32_t fNearPlane = 0.05f;
		f32_t fFarPlane = 250.f;
		f32_t fProjectedWidthPixels = 0.f;
		f32_t fProjectedHeightPixels = 0.f;
		f32_t fProofShapeExtent = 0.f;
		f32_t fProofTargetPixels = 0.f;
		size_t iProofOrientation = 0u;
		bool_t bCenterInsideFrustum = false;
		bool_t bShapeFitProof = false;
	};

	class FOCUSED_HEADLESS_RENDER_SCOPE final
	{
	public:
		~FOCUSED_HEADLESS_RENDER_SCOPE()
		{
			if (m_bEngineInitialized)
				Engine::CGameInstance::Get().Release_Engine();
			if (nullptr != m_hWnd)
				DestroyWindow(m_hWnd);
			if (0u != m_ClassAtom)
				UnregisterClassW(CLASS_NAME, m_hInstance);
		}

		bool Initialize(std::string& OutError)
		{
			m_hInstance = GetModuleHandleW(nullptr);
			if (nullptr == m_hInstance)
			{
				OutError = "Focused draw gate cannot resolve its module.";
				return false;
			}
			WNDCLASSEXW WindowClass{};
			WindowClass.cbSize = sizeof(WindowClass);
			WindowClass.lpfnWndProc = DefWindowProcW;
			WindowClass.hInstance = m_hInstance;
			WindowClass.lpszClassName = CLASS_NAME;
			m_ClassAtom = RegisterClassExW(&WindowClass);
			if (0u == m_ClassAtom && ERROR_CLASS_ALREADY_EXISTS != GetLastError())
			{
				OutError = "Focused draw hidden-window registration failed.";
				return false;
			}
			m_hWnd = CreateWindowExW(0u, CLASS_NAME,
				L"LostArk Track A Focused Draw Gate", WS_OVERLAPPED,
				CW_USEDEFAULT, CW_USEDEFAULT, WIDTH, HEIGHT,
				nullptr, nullptr, m_hInstance, nullptr);
			if (nullptr == m_hWnd)
			{
				OutError = "Focused draw hidden-window creation failed.";
				return false;
			}

			Engine::ENGINE_DESC Desc{};
			Desc.hInstance = m_hInstance;
			Desc.hWnd = m_hWnd;
			Desc.eWinMode = Engine::WINMODE::WIN;
			Desc.eDriverType = D3D_DRIVER_TYPE_WARP;
			Desc.bNonInteractiveErrors = true;
			Desc.iNumLevels = ETOUI(Client::LEVEL::END);
			Desc.iWinSizeX = WIDTH;
			Desc.iWinSizeY = HEIGHT;
			if (FAILED(Engine::CGameInstance::Get().Initialize_Engine(
					Desc, m_pDevice, m_pContext)) ||
				nullptr == m_pDevice || nullptr == m_pContext)
			{
				OutError = "Focused draw production Engine initialization failed.";
				return false;
			}
			m_bEngineInitialized = true;

			Microsoft::WRL::ComPtr<IDXGIDevice> DxgiDevice;
			Microsoft::WRL::ComPtr<IDXGIAdapter> Adapter;
			DXGI_ADAPTER_DESC AdapterDesc{};
			if (FAILED(m_pDevice.As(&DxgiDevice)) ||
				FAILED(DxgiDevice->GetAdapter(&Adapter)) ||
				FAILED(Adapter->GetDesc(&AdapterDesc)) ||
				AdapterDesc.VendorId != 0x1414u || AdapterDesc.DeviceId != 0x008cu)
			{
				OutError = "Focused draw gate did not initialize Microsoft WARP.";
				return false;
			}
			return Set_Camera({ 0.f, 1.f, 0.f }, 0u, OutError);
		}

		bool Set_Camera(const float3_t& Target, const size_t ViewIndex,
			std::string& OutError) const
		{
			if (!m_bEngineInitialized || ViewIndex >= EYE_OFFSETS.size())
			{
				OutError = "Focused draw camera request is invalid.";
				return false;
			}
			const float3_t Eye{
				Target.x + EYE_OFFSETS[ViewIndex].x,
				Target.y + EYE_OFFSETS[ViewIndex].y,
				Target.z + EYE_OFFSETS[ViewIndex].z };
			return Set_CameraFromEye(Target, Eye, OutError);
		}

		bool Set_ShapeFitCamera(const float3_t& Target,
			const f32_t ShapeExtent, const f32_t CameraOffset,
			const size_t OrientationIndex,
			std::string& OutError) const
		{
			float3_t Eye;
			if (!Resolve_ShapeFitEye(
					Target, ShapeExtent, CameraOffset, OrientationIndex,
					Eye, OutError))
			{
				return false;
			}
			return Set_CameraFromEye(Target, Eye, OutError);
		}

		bool Describe_Camera(const float3_t& Target, const size_t ViewIndex,
			const float3_t& EvaluatedPosition, const f32_t CameraOffset,
			const float3_t& EvaluatedSpriteScale,
			FOCUSED_CAMERA_DIAGNOSTIC& Out) const
		{
			if (!m_bEngineInitialized || ViewIndex >= EYE_OFFSETS.size())
				return false;
			const float3_t Eye{
				Target.x + EYE_OFFSETS[ViewIndex].x,
				Target.y + EYE_OFFSETS[ViewIndex].y,
				Target.z + EYE_OFFSETS[ViewIndex].z };
			return Describe_CameraFromEye(Target, Eye, EvaluatedPosition,
				CameraOffset, EvaluatedSpriteScale, false, 0.f, 0u, Out);
		}

		bool Describe_ShapeFitCamera(const float3_t& Target,
			const f32_t ShapeExtent, const float3_t& EvaluatedPosition,
			const f32_t CameraOffset, const size_t OrientationIndex,
			const float3_t& EvaluatedScale,
			FOCUSED_CAMERA_DIAGNOSTIC& Out) const
		{
			std::string IgnoredError;
			float3_t Eye;
			if (!Resolve_ShapeFitEye(
					Target, ShapeExtent, CameraOffset, OrientationIndex,
					Eye, IgnoredError))
			{
				return false;
			}
			return Describe_CameraFromEye(Target, Eye, EvaluatedPosition,
				CameraOffset, EvaluatedScale, true, ShapeExtent,
				OrientationIndex, Out);
		}

		bool Begin_Frame(std::string& OutError) const
		{
			const float4_t Clear{ 0.f, 0.f, 0.f, 0.f };
			if (!m_bEngineInitialized ||
				FAILED(Engine::CGameInstance::Get().Render_Begin(&Clear)))
			{
				OutError = "Focused draw frame begin failed.";
				return false;
			}
			return true;
		}

		Microsoft::WRL::ComPtr<ID3D11Device> Get_Device() const
		{
			return m_pDevice;
		}
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> Get_Context() const
		{
			return m_pContext;
		}

	private:
		bool Set_CameraFromEye(const float3_t& Target, const float3_t& Eye,
			std::string& OutError) const
		{
			if (!m_bEngineInitialized)
			{
				OutError = "Focused draw camera is not initialized.";
				return false;
			}
			Engine::CGameInstance::Get().Set_Transform(Engine::D3DTS::VIEW,
				DirectX::XMMatrixLookAtLH(
					DirectX::XMVectorSet(Eye.x, Eye.y, Eye.z, 1.f),
					DirectX::XMVectorSet(Target.x, Target.y, Target.z, 1.f),
					DirectX::XMVectorSet(0.f, 1.f, 0.f, 0.f)));
			Engine::CGameInstance::Get().Set_Transform(Engine::D3DTS::PROJ,
				DirectX::XMMatrixPerspectiveFovLH(
					DirectX::XMConvertToRadians(VERTICAL_FOV_DEGREES), ASPECT,
					NEAR_PLANE, FAR_PLANE));
			Engine::CGameInstance::Get().Update_Engine(0.f);
			OutError.clear();
			return true;
		}

		bool Resolve_ShapeFitEye(const float3_t& Target,
			const f32_t ShapeExtent, const f32_t CameraOffset,
			const size_t OrientationIndex, float3_t& OutEye,
			std::string& OutError) const
		{
			if (!m_bEngineInitialized || !std::isfinite(ShapeExtent) ||
				ShapeExtent <= 0.f || !std::isfinite(CameraOffset) ||
				OrientationIndex >= EYE_OFFSETS.size())
			{
				OutError = "Focused draw shape-fit camera request is invalid.";
				return false;
			}
			const f32_t DesiredCenterDepth = ShapeExtent *
				static_cast<f32_t>(HEIGHT) /
				(2.f * std::tan(DirectX::XMConvertToRadians(
					VERTICAL_FOV_DEGREES) * 0.5f) * FIT_TARGET_PIXELS);
			const f32_t EyeToTarget = std::clamp(
				DesiredCenterDepth - CameraOffset, NEAR_PLANE * 4.f,
				FAR_PLANE * 0.75f);
			const DirectX::XMVECTOR Direction = DirectX::XMVector3Normalize(
				DirectX::XMVectorSet(EYE_OFFSETS[OrientationIndex].x,
					EYE_OFFSETS[OrientationIndex].y,
					EYE_OFFSETS[OrientationIndex].z, 0.f));
			float3_t Offset;
			DirectX::XMStoreFloat3(&Offset,
				DirectX::XMVectorScale(Direction, EyeToTarget));
			OutEye = { Target.x + Offset.x, Target.y + Offset.y,
				Target.z + Offset.z };
			OutError.clear();
			return true;
		}

		bool Describe_CameraFromEye(const float3_t& Target,
			const float3_t& Eye, const float3_t& EvaluatedPosition,
			const f32_t CameraOffset, const float3_t& EvaluatedScale,
			const bool_t bShapeFitProof, const f32_t ProofShapeExtent,
			const size_t ProofOrientation,
			FOCUSED_CAMERA_DIAGNOSTIC& Out) const
		{
			const DirectX::XMVECTOR EyeVector = DirectX::XMVectorSet(
				Eye.x, Eye.y, Eye.z, 1.f);
			const DirectX::XMVECTOR TargetVector = DirectX::XMVectorSet(
				Target.x, Target.y, Target.z, 1.f);
			const DirectX::XMVECTOR Forward = DirectX::XMVector3Normalize(
				DirectX::XMVectorSubtract(TargetVector, EyeVector));
			const DirectX::XMMATRIX View = DirectX::XMMatrixLookAtLH(
				EyeVector, TargetVector,
				DirectX::XMVectorSet(0.f, 1.f, 0.f, 0.f));
			const DirectX::XMMATRIX Projection =
				DirectX::XMMatrixPerspectiveFovLH(
					DirectX::XMConvertToRadians(VERTICAL_FOV_DEGREES), ASPECT,
					NEAR_PLANE, FAR_PLANE);
			const DirectX::XMVECTOR SourceCenter = DirectX::XMVectorSet(
				EvaluatedPosition.x, EvaluatedPosition.y, EvaluatedPosition.z, 1.f);
			const DirectX::XMVECTOR Center = DirectX::XMVectorAdd(SourceCenter,
				DirectX::XMVectorScale(Forward, CameraOffset));
			const DirectX::XMVECTOR ViewCenter =
				DirectX::XMVector3TransformCoord(Center, View);
			const DirectX::XMVECTOR NdcCenter =
				DirectX::XMVector3TransformCoord(Center, View * Projection);
			FOCUSED_CAMERA_DIAGNOSTIC Diagnostic;
			Diagnostic.vEye = Eye;
			Diagnostic.vTarget = Target;
			DirectX::XMStoreFloat3(&Diagnostic.vEvaluatedCenter, Center);
			DirectX::XMStoreFloat3(&Diagnostic.vViewSpaceCenter, ViewCenter);
			DirectX::XMStoreFloat3(&Diagnostic.vNdcCenter, NdcCenter);
			Diagnostic.fVerticalFovDegrees = VERTICAL_FOV_DEGREES;
			Diagnostic.fAspect = ASPECT;
			Diagnostic.fNearPlane = NEAR_PLANE;
			Diagnostic.fFarPlane = FAR_PLANE;
			Diagnostic.fProofShapeExtent = ProofShapeExtent;
			Diagnostic.fProofTargetPixels = bShapeFitProof ? FIT_TARGET_PIXELS : 0.f;
			Diagnostic.iProofOrientation = ProofOrientation;
			Diagnostic.bShapeFitProof = bShapeFitProof;
			Diagnostic.bCenterInsideFrustum =
				std::abs(Diagnostic.vNdcCenter.x) <= 1.f &&
				std::abs(Diagnostic.vNdcCenter.y) <= 1.f &&
				Diagnostic.vNdcCenter.z >= 0.f && Diagnostic.vNdcCenter.z <= 1.f;
			const f32_t Depth = Diagnostic.vViewSpaceCenter.z;
			if (std::isfinite(Depth) && Depth > 0.f)
			{
				const f32_t PixelsPerWorldUnit = static_cast<f32_t>(HEIGHT) /
					(2.f * Depth * std::tan(
						DirectX::XMConvertToRadians(VERTICAL_FOV_DEGREES) * 0.5f));
				Diagnostic.fProjectedWidthPixels =
					std::abs(EvaluatedScale.x) * PixelsPerWorldUnit;
				Diagnostic.fProjectedHeightPixels =
					std::abs(EvaluatedScale.y) * PixelsPerWorldUnit;
			}
			Out = Diagnostic;
			return true;
		}

		static constexpr const wchar_t* CLASS_NAME =
			L"LostArkFourClassTrackAFocusedDraw";
		static constexpr uint32_t WIDTH = 640u;
		static constexpr uint32_t HEIGHT = 360u;
		static constexpr f32_t VERTICAL_FOV_DEGREES = 60.f;
		static constexpr f32_t ASPECT =
			static_cast<f32_t>(WIDTH) / static_cast<f32_t>(HEIGHT);
		static constexpr f32_t NEAR_PLANE = 0.05f;
		static constexpr f32_t FAR_PLANE = 250.f;
		static constexpr f32_t FIT_TARGET_PIXELS = 96.f;
		static constexpr std::array<float3_t, 4u> EYE_OFFSETS = {{
			{ 0.f, 6.f, -18.f }, { 18.f, 6.f, 0.f },
			{ -12.f, 12.f, -12.f }, { 0.f, 20.f, -0.1f } }};
		HINSTANCE m_hInstance = nullptr;
		HWND m_hWnd = nullptr;
		ATOM m_ClassAtom = 0u;
		bool_t m_bEngineInitialized = false;
		Microsoft::WRL::ComPtr<ID3D11Device> m_pDevice;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_pContext;
	};

	struct FOCUSED_PARTICLE_SAMPLE final
	{
		f32_t fSampleTime = 0.f;
		float4x4_t World{};
		float4_t Color = { 1.f, 1.f, 1.f, 1.f };
		float4_t vDynamicParameter{};
		float3_t vWorldVelocity{};
		float2_t vSpritePivot = { 0.5f, 0.5f };
		float2_t vSourceImageFlipSign = { 1.f, 1.f };
		float3_t vResolvedSpriteScale{};
		Client::EFFECT_PARTICLE_SPRITE_ALIGNMENT eSpriteAlignment =
			Client::EFFECT_PARTICLE_SPRITE_ALIGNMENT::CAMERA_RECTANGLE;
		bool_t bSourceImageFlipping = false;
		bool_t bResolvedSpriteScale = false;
		f32_t fSpriteRotationDegrees = 0.f;
		f32_t fCameraOffset = 0.f;
		f32_t fNormalizedLife = 0.f;
		f32_t fEffectiveMaterialOpacity = 0.f;
		Client::EFFECT_PARTICLE_RUNTIME_PROBE Probe{};
	};

	struct FOCUSED_DRAW_EVIDENCE final
	{
		D3D11_QUERY_DATA_PIPELINE_STATISTICS Pipeline{};
		Microsoft::WRL::ComPtr<ID3D11PixelShader> PixelShader;
		uint64_t iColorNonZeroPixelCount = 0u;
		uint64_t iColorByteSum = 0u;
		uint8_t iMaximumColorByte = 0u;
		uint64_t iAlphaNonZeroPixelCount = 0u;
		uint64_t iAlphaOnePixelCount = 0u;
		size_t iViewIndex = 0u;
		std::string strDiagnostics;
	};

	bool Wait_ForPipelineStatistics(
		const Microsoft::WRL::ComPtr<ID3D11DeviceContext>& Context,
		const Microsoft::WRL::ComPtr<ID3D11Query>& Query,
		D3D11_QUERY_DATA_PIPELINE_STATISTICS& Out,
		std::string& OutError)
	{
		const ULONGLONG Deadline = GetTickCount64() + 5000u;
		for (;;)
		{
			const HRESULT Result = Context->GetData(
				Query.Get(), &Out, sizeof(Out), 0u);
			if (S_OK == Result)
				return true;
			if (FAILED(Result) || GetTickCount64() >= Deadline)
			{
				OutError = "Focused draw pipeline-statistics query failed or timed out.";
				return false;
			}
			SwitchToThread();
		}
	}

	bool Read_BackbufferAlpha(
		const Microsoft::WRL::ComPtr<ID3D11Device>& Device,
		const Microsoft::WRL::ComPtr<ID3D11DeviceContext>& Context,
		uint64_t& OutNonZero, uint64_t& OutOne, std::string& OutError,
		uint64_t* OutColorNonZero = nullptr,
		uint64_t* OutColorByteSum = nullptr,
		uint8_t* OutMaximumColorByte = nullptr)
	{
		OutNonZero = 0u;
		OutOne = 0u;
		if (nullptr != OutColorNonZero) *OutColorNonZero = 0u;
		if (nullptr != OutColorByteSum) *OutColorByteSum = 0u;
		if (nullptr != OutMaximumColorByte) *OutMaximumColorByte = 0u;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> TargetView;
		Context->OMGetRenderTargets(1u, &TargetView, nullptr);
		if (nullptr == TargetView)
		{
			OutError = "Focused draw has no bound RT0 for alpha readback.";
			return false;
		}
		Microsoft::WRL::ComPtr<ID3D11Resource> Resource;
		TargetView->GetResource(&Resource);
		Microsoft::WRL::ComPtr<ID3D11Texture2D> Source;
		if (nullptr == Resource || FAILED(Resource.As(&Source)))
		{
			OutError = "Focused draw RT0 is not a texture.";
			return false;
		}
		D3D11_TEXTURE2D_DESC Desc{};
		Source->GetDesc(&Desc);
		if (Desc.Format != DXGI_FORMAT_R8G8B8A8_UNORM &&
			Desc.Format != DXGI_FORMAT_R8G8B8A8_UNORM_SRGB &&
			Desc.Format != DXGI_FORMAT_B8G8R8A8_UNORM &&
			Desc.Format != DXGI_FORMAT_B8G8R8A8_UNORM_SRGB)
		{
			OutError = "Focused draw RT0 format does not expose an 8-bit alpha lane.";
			return false;
		}

		Microsoft::WRL::ComPtr<ID3D11Texture2D> Resolved = Source;
		if (Desc.SampleDesc.Count > 1u)
		{
			D3D11_TEXTURE2D_DESC ResolvedDesc = Desc;
			ResolvedDesc.SampleDesc.Count = 1u;
			ResolvedDesc.SampleDesc.Quality = 0u;
			ResolvedDesc.BindFlags = 0u;
			ResolvedDesc.MiscFlags = 0u;
			if (FAILED(Device->CreateTexture2D(
					&ResolvedDesc, nullptr, &Resolved)))
			{
				OutError = "Focused draw RT0 resolve texture creation failed.";
				return false;
			}
			Context->ResolveSubresource(Resolved.Get(), 0u, Source.Get(), 0u,
				Desc.Format);
			Desc = ResolvedDesc;
		}
		D3D11_TEXTURE2D_DESC StagingDesc = Desc;
		StagingDesc.Usage = D3D11_USAGE_STAGING;
		StagingDesc.BindFlags = 0u;
		StagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		StagingDesc.MiscFlags = 0u;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> Staging;
		if (FAILED(Device->CreateTexture2D(&StagingDesc, nullptr, &Staging)))
		{
			OutError = "Focused draw RT0 staging texture creation failed.";
			return false;
		}
		Context->CopyResource(Staging.Get(), Resolved.Get());
		D3D11_MAPPED_SUBRESOURCE Mapped{};
		if (FAILED(Context->Map(Staging.Get(), 0u, D3D11_MAP_READ, 0u, &Mapped)))
		{
			OutError = "Focused draw RT0 map failed.";
			return false;
		}
		for (uint32_t Y = 0u; Y < Desc.Height; ++Y)
		{
			const auto* Row = static_cast<const uint8_t*>(Mapped.pData) +
				static_cast<size_t>(Y) * Mapped.RowPitch;
			for (uint32_t X = 0u; X < Desc.Width; ++X)
			{
				const auto* Pixel = Row + static_cast<size_t>(X) * 4u;
				const uint8_t MaximumColor =
					(std::max)({ Pixel[0u], Pixel[1u], Pixel[2u] });
				if (nullptr != OutColorNonZero && 0u != MaximumColor)
					++*OutColorNonZero;
				if (nullptr != OutColorByteSum)
					*OutColorByteSum += static_cast<uint64_t>(Pixel[0u]) +
						Pixel[1u] + Pixel[2u];
				if (nullptr != OutMaximumColorByte)
					*OutMaximumColorByte = (std::max)(
						*OutMaximumColorByte, MaximumColor);
				const uint8_t Alpha = Pixel[3u];
				OutNonZero += 0u != Alpha ? 1u : 0u;
				OutOne += 255u == Alpha ? 1u : 0u;
			}
		}
		Context->Unmap(Staging.Get(), 0u);
		return true;
	}

	uint32_t Expected_EffectPass(const EFFECT_ELEMENT_DESC& Element,
		f32_t WorldDeterminant);

	void Capture_FocusedParticleSample(
		const Client::EFFECT_EVALUATED_PARTICLE& Particle,
		const f32_t SampleTime,
		const Client::EFFECT_PARTICLE_RUNTIME_PROBE& Probe,
		FOCUSED_PARTICLE_SAMPLE& Out)
	{
		FOCUSED_PARTICLE_SAMPLE Sample;
		Sample.fSampleTime = SampleTime;
		Sample.World = Particle.World;
		Sample.Color = Particle.Color;
		Sample.vDynamicParameter = Particle.vDynamicParameter;
		Sample.vWorldVelocity = Particle.vWorldVelocity;
		Sample.vSpritePivot = Particle.vSpritePivot;
		Sample.vSourceImageFlipSign = Particle.vSourceImageFlipSign;
		Sample.eSpriteAlignment = Particle.eSpriteAlignment;
		Sample.bSourceImageFlipping = Particle.bSourceImageFlipping;
		Sample.fSpriteRotationDegrees = Particle.fSpriteRotationDegrees;
		Sample.fCameraOffset = Particle.fCameraOffset;
		Sample.fNormalizedLife = Particle.fNormalizedLife;
		Sample.fEffectiveMaterialOpacity = Particle.Color.w;
		if (nullptr != Particle.pElement)
		{
			for (size_t Channel = 0u; Channel <
				Particle.pElement->Material.SourceMaterial.
					DynamicParameterSemantics.size(); ++Channel)
			{
				const std::string& Semantic = Particle.pElement->Material.
					SourceMaterial.DynamicParameterSemantics[Channel];
				const f32_t Value = (&Particle.vDynamicParameter.x)[Channel];
				if (Semantic == "opacity")
				{
					Sample.fEffectiveMaterialOpacity *=
						std::clamp(Value, 0.f, 1.f);
				}
				else if (Semantic == "dissolve")
				{
					Sample.fEffectiveMaterialOpacity *=
						std::clamp(1.f - Value, 0.f, 1.f);
				}
			}
		}
		Sample.Probe = Probe;
		const float3_t Magnitude{
			std::sqrt(Particle.World._11 * Particle.World._11 +
				Particle.World._12 * Particle.World._12 +
				Particle.World._13 * Particle.World._13),
			std::sqrt(Particle.World._21 * Particle.World._21 +
				Particle.World._22 * Particle.World._22 +
				Particle.World._23 * Particle.World._23),
			std::sqrt(Particle.World._31 * Particle.World._31 +
				Particle.World._32 * Particle.World._32 +
				Particle.World._33 * Particle.World._33) };
		Client::EFFECT_PARTICLE_SPRITE_SCALE_DESC Resolved;
		Sample.bResolvedSpriteScale =
			Client::CEffectDocumentRenderer::Resolve_ParticleSpriteScale(
				Particle, Magnitude, Resolved);
		if (Sample.bResolvedSpriteScale)
			Sample.vResolvedSpriteScale = Resolved.vScale;
		Out = Sample;
	}

	void Append_FocusedMaterialAndSampleDiagnostic(
		std::ostringstream& Stream, const EFFECT_ELEMENT_DESC* Element,
		const FOCUSED_PARTICLE_SAMPLE* Sample)
	{
		Stream << std::fixed << std::setprecision(6);
		if (nullptr != Element)
		{
			const auto& Material = Element->Material;
			const auto& Execution = Material.Execution;
			Stream << "element=" << Element->strElementId <<
				" shape=" << Element->SourceRecipe.strRendererShape <<
				" template=" << Material.strTemplateId <<
				" sourceMaterial=" << Material.strSourceMaterialPath <<
				" sourceProfile=" <<
				Material.SourceMaterial.strRuntimeShaderProfileId <<
				" renderProfile=" << static_cast<uint32_t>(Material.eRenderProfile) <<
				" expectedPass=" << Expected_EffectPass(*Element,
					(nullptr == Sample ? 1.f :
					 Sample->World._11 * (Sample->World._22 * Sample->World._33 -
						 Sample->World._23 * Sample->World._32) -
					 Sample->World._12 * (Sample->World._21 * Sample->World._33 -
						 Sample->World._23 * Sample->World._31) +
					 Sample->World._13 * (Sample->World._21 * Sample->World._32 -
						 Sample->World._22 * Sample->World._31))) <<
				" executionEnabled=" << Execution.bEnabled <<
				" failClosed=" << Execution.bFailClosed <<
				" backend=" << static_cast<uint32_t>(Execution.eBackend) <<
				" opcode=" << Execution.iOpcode <<
				" executionPass=" << Execution.iPassIndex <<
				" textureLaneCount=" << Execution.iTextureLaneCount <<
				" textureMask=0x" << std::hex << Execution.iTextureMask <<
					std::dec << " resources=[";
			for (size_t Index = 0u; Index < Element->ResourceBindings.size(); ++Index)
			{
				if (0u != Index) Stream << ',';
				const auto& Binding = Element->ResourceBindings[Index];
				Stream << Binding.strSlotId << '=' << Binding.strAssetId;
			}
			Stream << "] lanes=[";
			for (size_t Index = 0u; Index < Execution.TextureLanes.size(); ++Index)
			{
				if (0u != Index) Stream << ',';
				const auto& Lane = Execution.TextureLanes[Index];
				Stream << Lane.strLaneId << "@t" << Lane.iTextureRegister <<
					"/s" << Lane.iSamplerRegister << '=' << Lane.strAssetId;
			}
			Stream << "] sourceNamedTextures=" <<
				Material.SourceMaterial.Textures.size();
		}
		if (nullptr != Sample)
		{
			const auto Scale = [&Sample]()
			{
				return float3_t{
					std::sqrt(Sample->World._11 * Sample->World._11 +
						Sample->World._12 * Sample->World._12 +
						Sample->World._13 * Sample->World._13),
					std::sqrt(Sample->World._21 * Sample->World._21 +
						Sample->World._22 * Sample->World._22 +
						Sample->World._23 * Sample->World._23),
					std::sqrt(Sample->World._31 * Sample->World._31 +
						Sample->World._32 * Sample->World._32 +
						Sample->World._33 * Sample->World._33) };
			}();
			Stream << " sampleTime=" << Sample->fSampleTime <<
				" worldPos=(" << Sample->World._41 << ',' << Sample->World._42 <<
				',' << Sample->World._43 << ") worldScale=(" << Scale.x << ',' <<
				Scale.y << ',' << Scale.z << ") evaluatedColor=(" <<
				Sample->Color.x << ',' << Sample->Color.y << ',' << Sample->Color.z <<
				',' << Sample->Color.w << ") dynamic=(" <<
				Sample->vDynamicParameter.x << ',' << Sample->vDynamicParameter.y <<
				',' << Sample->vDynamicParameter.z << ',' <<
				Sample->vDynamicParameter.w << ") active=" <<
				Sample->Probe.iActiveParticleCount << " alpha(first/min/max)=(" <<
				Sample->Probe.fFirstAlpha << ',' << Sample->Probe.fMinAlpha << ',' <<
				Sample->Probe.fMaxAlpha << ") dynamicMin=(" <<
				Sample->Probe.vMinDynamicParameter.x << ',' <<
				Sample->Probe.vMinDynamicParameter.y << ',' <<
				Sample->Probe.vMinDynamicParameter.z << ',' <<
				Sample->Probe.vMinDynamicParameter.w << ") dynamicMax=(" <<
				Sample->Probe.vMaxDynamicParameter.x << ',' <<
				Sample->Probe.vMaxDynamicParameter.y << ',' <<
				Sample->Probe.vMaxDynamicParameter.z << ',' <<
				Sample->Probe.vMaxDynamicParameter.w << ") normalizedLife=" <<
				Sample->fNormalizedLife << " effectiveMaterialOpacity=" <<
				Sample->fEffectiveMaterialOpacity << " cameraOffset=" <<
				Sample->fCameraOffset << " alignment=" <<
				static_cast<uint32_t>(Sample->eSpriteAlignment) <<
				" resolvedSpriteScale=" << Sample->bResolvedSpriteScale << "/(" <<
				Sample->vResolvedSpriteScale.x << ',' <<
				Sample->vResolvedSpriteScale.y << ',' <<
				Sample->vResolvedSpriteScale.z << ')';
		}
	}

	bool Render_WithEvidence(
		const std::shared_ptr<Client::CEffectObject>& Object,
		const FOCUSED_HEADLESS_RENDER_SCOPE& Scope,
		const float3_t& Target,
		const std::optional<size_t> RequiredView,
		const bool_t bReadbackAlpha,
		FOCUSED_DRAW_EVIDENCE& Out,
		std::string& OutError,
		const EFFECT_ELEMENT_DESC* DiagnosticElement = nullptr,
		const FOCUSED_PARTICLE_SAMPLE* DiagnosticSample = nullptr,
		const DECODED_MODEL_METRICS* DiagnosticModelMetrics = nullptr,
		const bool_t bRenderNonBlendModelCue = false)
	{
		Out = {};
		if (nullptr == Object)
		{
			OutError = "Focused draw object is null.";
			return false;
		}
		const auto Device = Scope.Get_Device();
		const auto Context = Scope.Get_Context();
		constexpr size_t WORLD_VIEW_COUNT = 4u;
		constexpr size_t SHAPE_FIT_VIEW_COUNT = 4u;
		f32_t ProofShapeExtent = 0.f;
		float3_t DiagnosticScale{ 1.f, 1.f, 1.f };
		float3_t ProofTarget = Target;
		if (nullptr != DiagnosticElement && nullptr != DiagnosticSample)
		{
			if (DiagnosticElement->SourceRecipe.strRendererShape == "sprite" &&
				DiagnosticSample->bResolvedSpriteScale)
			{
				DiagnosticScale = DiagnosticSample->vResolvedSpriteScale;
				ProofShapeExtent = (std::max)(
					std::abs(DiagnosticScale.x), std::abs(DiagnosticScale.y));
			}
			else if (DiagnosticElement->SourceRecipe.strRendererShape == "mesh" &&
				nullptr != DiagnosticModelMetrics)
			{
				const float3_t WorldScale{
					std::sqrt(DiagnosticSample->World._11 *
						DiagnosticSample->World._11 +
						DiagnosticSample->World._12 *
						DiagnosticSample->World._12 +
						DiagnosticSample->World._13 *
						DiagnosticSample->World._13),
					std::sqrt(DiagnosticSample->World._21 *
						DiagnosticSample->World._21 +
						DiagnosticSample->World._22 *
						DiagnosticSample->World._22 +
						DiagnosticSample->World._23 *
						DiagnosticSample->World._23),
					std::sqrt(DiagnosticSample->World._31 *
						DiagnosticSample->World._31 +
						DiagnosticSample->World._32 *
						DiagnosticSample->World._32 +
						DiagnosticSample->World._33 *
						DiagnosticSample->World._33) };
				const float3_t LocalCenter{
					0.5f * (DiagnosticModelMetrics->vMinimum.x +
						DiagnosticModelMetrics->vMaximum.x) *
						DiagnosticElement->Detail.Mesh.fModelPreScale,
					0.5f * (DiagnosticModelMetrics->vMinimum.y +
						DiagnosticModelMetrics->vMaximum.y) *
						DiagnosticElement->Detail.Mesh.fModelPreScale,
					0.5f * (DiagnosticModelMetrics->vMinimum.z +
						DiagnosticModelMetrics->vMaximum.z) *
						DiagnosticElement->Detail.Mesh.fModelPreScale };
				DirectX::XMStoreFloat3(&ProofTarget,
					DirectX::XMVector3TransformCoord(
						DirectX::XMLoadFloat3(&LocalCenter),
						DirectX::XMLoadFloat4x4(&DiagnosticSample->World)));
				const float3_t LocalHalfExtent{
					0.5f * (DiagnosticModelMetrics->vMaximum.x -
						DiagnosticModelMetrics->vMinimum.x),
					0.5f * (DiagnosticModelMetrics->vMaximum.y -
						DiagnosticModelMetrics->vMinimum.y),
					0.5f * (DiagnosticModelMetrics->vMaximum.z -
						DiagnosticModelMetrics->vMinimum.z) };
				const f32_t LocalBoundsRadius = std::sqrt(
					LocalHalfExtent.x * LocalHalfExtent.x +
					LocalHalfExtent.y * LocalHalfExtent.y +
					LocalHalfExtent.z * LocalHalfExtent.z);
				ProofShapeExtent = 2.f * LocalBoundsRadius *
					DiagnosticElement->Detail.Mesh.fModelPreScale *
					(std::max)({ WorldScale.x, WorldScale.y, WorldScale.z });
				DiagnosticScale = { ProofShapeExtent, ProofShapeExtent,
					ProofShapeExtent };
			}
		}
		const bool_t bHasShapeFitProof = std::isfinite(ProofShapeExtent) &&
			ProofShapeExtent > 0.0001f;
		const size_t FirstView = RequiredView.value_or(0u);
		const size_t EndView = RequiredView.has_value() ? FirstView + 1u :
			WORLD_VIEW_COUNT +
				(bHasShapeFitProof ? SHAPE_FIT_VIEW_COUNT : 0u);
		std::ostringstream Diagnostics;
		Append_FocusedMaterialAndSampleDiagnostic(
			Diagnostics, DiagnosticElement, DiagnosticSample);
		for (size_t View = FirstView; View < EndView; ++View)
		{
			const bool_t bShapeFitView = View >= WORLD_VIEW_COUNT &&
				View < WORLD_VIEW_COUNT + SHAPE_FIT_VIEW_COUNT;
			const size_t ProofOrientation = bShapeFitView ?
				View - WORLD_VIEW_COUNT : 0u;
			const bool_t bCameraSet = bShapeFitView ?
				Scope.Set_ShapeFitCamera(ProofTarget, ProofShapeExtent,
					nullptr == DiagnosticSample ? 0.f :
					DiagnosticSample->fCameraOffset, ProofOrientation, OutError) :
				Scope.Set_Camera(Target, View, OutError);
			if (!bCameraSet ||
				!Scope.Begin_Frame(OutError))
			{
				return false;
			}
			D3D11_QUERY_DESC QueryDesc{};
			QueryDesc.Query = D3D11_QUERY_PIPELINE_STATISTICS;
			Microsoft::WRL::ComPtr<ID3D11Query> Query;
			if (FAILED(Device->CreateQuery(&QueryDesc, &Query)))
			{
				OutError = "Focused draw pipeline query creation failed.";
				return false;
			}
			Context->Begin(Query.Get());
			const HRESULT RenderResult = bRenderNonBlendModelCue ?
				Object->Render_NonBlendModelCues() : Object->Render();
			Context->End(Query.Get());
			ID3D11PixelShader* RawPixelShader = nullptr;
			Context->PSGetShader(&RawPixelShader, nullptr, nullptr);
			Microsoft::WRL::ComPtr<ID3D11PixelShader> PixelShader;
			PixelShader.Attach(RawPixelShader);
			std::array<ID3D11ShaderResourceView*, 16u> RawSrvs{};
			Context->PSGetShaderResources(0u,
				static_cast<UINT>(RawSrvs.size()), RawSrvs.data());
			uint32_t BoundSrvMask = 0u;
			for (size_t Slot = 0u; Slot < RawSrvs.size(); ++Slot)
			{
				if (nullptr == RawSrvs[Slot]) continue;
				BoundSrvMask |= 1u << static_cast<uint32_t>(Slot);
				RawSrvs[Slot]->Release();
			}
			Context->Flush();
			D3D11_QUERY_DATA_PIPELINE_STATISTICS Pipeline{};
			if (!Wait_ForPipelineStatistics(
					Context, Query, Pipeline, OutError))
			{
				return false;
			}
			const HRESULT DeviceReason = Device->GetDeviceRemovedReason();
			const bool_t bFailureIsolated = Object->Is_RenderFailureIsolated();
			const auto& Submission = Object->Get_LastRenderSubmissionStats();
			const bool_t bCommitted = SUCCEEDED(RenderResult) &&
				S_OK == DeviceReason && !bFailureIsolated &&
				(bRenderNonBlendModelCue ||
					(Submission.bCompleted && Submission.bCommitted));
			uint64_t NonZero = 0u;
			uint64_t One = 0u;
			uint64_t ColorNonZero = 0u;
			uint64_t ColorByteSum = 0u;
			uint8_t MaximumColorByte = 0u;
			std::string ReadbackError;
			const bool_t bReadbackSucceeded = !bReadbackAlpha ||
				Read_BackbufferAlpha(
					Device, Context, NonZero, One, ReadbackError,
					&ColorNonZero, &ColorByteSum, &MaximumColorByte);
			const Client::EFFECT_GPU_RENDER_OCCURRENCE_STATS* Occurrence = nullptr;
			if (nullptr != DiagnosticElement)
			{
				const auto Row = std::find_if(Submission.Occurrences.begin(),
					Submission.Occurrences.end(),
					[DiagnosticElement](const auto& Candidate)
					{
						return Candidate.strElementId ==
							DiagnosticElement->strElementId;
					});
				if (Row != Submission.Occurrences.end()) Occurrence = &*Row;
			}
			else if (Submission.Occurrences.size() == 1u)
				Occurrence = &Submission.Occurrences.front();
			FOCUSED_CAMERA_DIAGNOSTIC Camera;
			const float3_t EvaluatedPosition = nullptr == DiagnosticSample ? Target :
				float3_t{ DiagnosticSample->World._41,
					DiagnosticSample->World._42, DiagnosticSample->World._43 };
			const f32_t CameraOffset = nullptr == DiagnosticSample ? 0.f :
				DiagnosticSample->fCameraOffset;
			const bool_t bCameraDescribed = bShapeFitView ?
				Scope.Describe_ShapeFitCamera(ProofTarget, ProofShapeExtent,
					ProofTarget, CameraOffset, ProofOrientation,
					DiagnosticScale, Camera) :
				Scope.Describe_Camera(Target, View, EvaluatedPosition,
					CameraOffset, DiagnosticScale, Camera);
			Diagnostics << " | view=" << View << " renderHr=0x" << std::hex <<
				static_cast<uint32_t>(RenderResult) << " deviceHr=0x" <<
				static_cast<uint32_t>(DeviceReason) << std::dec <<
				" isolated=" << bFailureIsolated <<
				" submission(completed/committed)=" << Submission.bCompleted << '/' <<
				Submission.bCommitted << " pipeline(ia/vs/ps)=(" <<
				Pipeline.IAPrimitives << ',' << Pipeline.VSInvocations << ',' <<
				Pipeline.PSInvocations << ") pixelShader=" <<
				(nullptr != PixelShader) << " boundPsSrvMask=0x" << std::hex <<
				BoundSrvMask << std::dec << " rt0Readback=" << bReadbackSucceeded <<
				" color(nonzero/sum/max)=(" << ColorNonZero << ',' <<
				ColorByteSum << ',' << static_cast<uint32_t>(MaximumColorByte) <<
				") alpha(nonzero/one)=(" << NonZero << ',' << One << ')';
			if (!ReadbackError.empty())
				Diagnostics << " readbackError=" << ReadbackError;
			if (nullptr == Occurrence)
			{
				Diagnostics << " occurrence=missing";
			}
			else
			{
				Diagnostics << " occurrence(configured/evaluated/active/candidate)=(" <<
					Occurrence->iConfigured << ',' << Occurrence->iEvaluated << ',' <<
					Occurrence->iActive << ',' << Occurrence->iCandidateRowCount <<
					") disposition(attempted/submitted/suppressed/failed)=(" <<
					Occurrence->iAttempted << ',' << Occurrence->iSubmitted << ',' <<
					Occurrence->iSuppressed << ',' << Occurrence->iFailed <<
					") execution(material/srv/sampler/pass/vbBind/vbDraw/upload/draw/selection)=(" <<
					Occurrence->iMaterialBindCount << ',' <<
					Occurrence->iTextureSrvBindCount << ',' <<
					Occurrence->iSamplerBindCount << ',' <<
					Occurrence->iShaderPassApplyCount << ',' <<
					Occurrence->iVIBufferBindCount << ',' <<
					Occurrence->iVIBufferDrawCount << ',' <<
					Occurrence->iGeometryUploadCount << ',' <<
					Occurrence->iIssuedDrawCallCount << ',' <<
					Occurrence->iDrawSelectionCount << ") selectedPass=" <<
					Occurrence->iSelectedPassIndex << " carrier=" <<
					static_cast<uint32_t>(Occurrence->eCarrier) << " diverged=" <<
					Occurrence->bDrawSelectionDiverged << " submittedBounds=" <<
					Occurrence->bHasSubmittedPosition << "/min(" <<
					Occurrence->vSubmittedPositionMin.x << ',' <<
					Occurrence->vSubmittedPositionMin.y << ',' <<
					Occurrence->vSubmittedPositionMin.z << ")/max(" <<
					Occurrence->vSubmittedPositionMax.x << ',' <<
					Occurrence->vSubmittedPositionMax.y << ',' <<
					Occurrence->vSubmittedPositionMax.z << ')';
			}
			if (bCameraDescribed)
			{
				Diagnostics << " camera proofCamera=" << Camera.bShapeFitProof <<
					" proofExtent/targetPixels=(" << Camera.fProofShapeExtent << ',' <<
					Camera.fProofTargetPixels << ") proofOrientation=" <<
					Camera.iProofOrientation << " eye(" << Camera.vEye.x << ',' <<
					Camera.vEye.y << ',' << Camera.vEye.z << ") target(" <<
					Camera.vTarget.x << ',' << Camera.vTarget.y << ',' <<
					Camera.vTarget.z << ") evaluatedCenter(" <<
					Camera.vEvaluatedCenter.x << ',' << Camera.vEvaluatedCenter.y << ',' <<
					Camera.vEvaluatedCenter.z << ") viewCenter(" <<
					Camera.vViewSpaceCenter.x << ',' << Camera.vViewSpaceCenter.y << ',' <<
					Camera.vViewSpaceCenter.z << ") ndc(" << Camera.vNdcCenter.x << ',' <<
					Camera.vNdcCenter.y << ',' << Camera.vNdcCenter.z << ") frustum=" <<
					Camera.bCenterInsideFrustum << " projection(fov/aspect/near/far)=(" <<
					Camera.fVerticalFovDegrees << ',' << Camera.fAspect << ',' <<
					Camera.fNearPlane << ',' << Camera.fFarPlane <<
					") projectedPixels(width/height)=(" <<
					Camera.fProjectedWidthPixels << ',' <<
					Camera.fProjectedHeightPixels << ')';
			}
			/* Preserve the latest attempted view even when its pixels are all zero.
			   Synthetic material diagnostics use this to distinguish a committed
			   zero-output shader from a stage/submission failure. */
			Out.Pipeline = Pipeline;
			Out.PixelShader = PixelShader;
			Out.iColorNonZeroPixelCount = ColorNonZero;
			Out.iColorByteSum = ColorByteSum;
			Out.iMaximumColorByte = MaximumColorByte;
			Out.iAlphaNonZeroPixelCount = NonZero;
			Out.iAlphaOnePixelCount = One;
			Out.iViewIndex = View;
			Out.strDiagnostics = Diagnostics.str();
			const uint32_t DiagnosticPass =
				nullptr != DiagnosticElement && nullptr != DiagnosticSample ?
				Expected_EffectPass(*DiagnosticElement,
					DiagnosticSample->World._11 *
						(DiagnosticSample->World._22 * DiagnosticSample->World._33 -
						 DiagnosticSample->World._23 * DiagnosticSample->World._32) -
					DiagnosticSample->World._12 *
						(DiagnosticSample->World._21 * DiagnosticSample->World._33 -
						 DiagnosticSample->World._23 * DiagnosticSample->World._31) +
					DiagnosticSample->World._13 *
						(DiagnosticSample->World._21 * DiagnosticSample->World._32 -
						 DiagnosticSample->World._22 * DiagnosticSample->World._31)) :
				UINT32_MAX;
			const bool_t bAdditiveRgbProof = 2u == DiagnosticPass ||
				4u == DiagnosticPass;
			const bool_t bVisibleReadback = !bReadbackAlpha ||
				(bReadbackSucceeded && (bAdditiveRgbProof ?
					ColorNonZero > 0u : ColorNonZero > 0u || NonZero > 0u));
			if (bCommitted && Pipeline.VSInvocations > 0u &&
				Pipeline.PSInvocations > 0u && bVisibleReadback)
			{
				return true;
			}
		}
		Out.strDiagnostics = Diagnostics.str();
		OutError = "Focused draw produced no committed VS/PS-visible pixels. " +
			Out.strDiagnostics;
		return false;
	}

	bool Collect_ActiveParticleLifetimeSamples(
		const std::shared_ptr<Client::CEffectObject>& Object,
		const EFFECT_ELEMENT_DESC& Element,
		std::vector<FOCUSED_PARTICLE_SAMPLE>& Out,
		std::string& OutError,
		FOCUSED_PARTICLE_SAMPLE* OutFirstActive = nullptr)
	{
		Out.clear();
		if (nullptr == Object)
		{
			OutError = "Focused lifetime sample object is null.";
			return false;
		}
		const f32_t Begin = Element.Detail.Timing.fStartDelaySeconds +
			Element.SourceRecipe.fEmitterDelaySeconds;
		const f32_t End = (std::max)(Begin + 10.f,
			Element.Detail.Timing.fStartDelaySeconds +
			Element.Detail.Timing.fLifeTimeSeconds + 2.f);
		bool_t bObservedActive = false;
		FOCUSED_PARTICLE_SAMPLE FirstActive;
		for (f32_t Time = (std::max)(0.f, Begin);
			Time <= End; Time += 1.f / 60.f)
		{
			Object->Set_SampleTime(Time);
			Client::EFFECT_PARTICLE_RUNTIME_PROBE Probe;
			if (!Object->Query_ParticleRuntimeProbe(Element.strElementId, Probe) ||
				0u == Probe.iActiveParticleCount)
			{
				continue;
			}
			const auto& Frame = Object->Get_ReconstructedTestFrame();
			const auto FirstParticle = std::find_if(Frame.Particles.begin(),
				Frame.Particles.end(), [&Element](const auto& Candidate)
				{
					return nullptr != Candidate.pElement &&
						Candidate.pElement->strElementId == Element.strElementId;
				});
			if (FirstParticle == Frame.Particles.end())
				continue;
			if (!bObservedActive)
			{
				Capture_FocusedParticleSample(
					*FirstParticle, Time, Probe, FirstActive);
				bObservedActive = true;
				if (nullptr != OutFirstActive) *OutFirstActive = FirstActive;
			}
			bool_t bCapturedCandidate = false;
			FOCUSED_PARTICLE_SAMPLE BestCandidate;
			for (auto Particle = FirstParticle;
				Particle != Frame.Particles.end(); ++Particle)
			{
				if (nullptr == Particle->pElement ||
					Particle->pElement->strElementId != Element.strElementId)
				{
					continue;
				}
				FOCUSED_PARTICLE_SAMPLE Candidate;
				Capture_FocusedParticleSample(*Particle, Time, Probe, Candidate);
				if (!bCapturedCandidate ||
					(std::isfinite(Candidate.fEffectiveMaterialOpacity) &&
					 (!std::isfinite(BestCandidate.fEffectiveMaterialOpacity) ||
					Candidate.fEffectiveMaterialOpacity >
						BestCandidate.fEffectiveMaterialOpacity)))
				{
					BestCandidate = Candidate;
					bCapturedCandidate = true;
				}
			}
			if (bCapturedCandidate)
				Out.push_back(BestCandidate);
		}
		if (!Out.empty())
		{
			/* Try the strongest effective-opacity samples first, but retain every
			   bounded active sample.  Runtime material evaluation remains the final
			   authority because typed profiles may derive coverage from texture lanes
			   that this generic diagnostic cannot predict. */
			std::stable_sort(Out.begin(), Out.end(),
				[](const FOCUSED_PARTICLE_SAMPLE& Left,
					const FOCUSED_PARTICLE_SAMPLE& Right)
				{
					const f32_t LeftOpacity =
						std::isfinite(Left.fEffectiveMaterialOpacity) ?
						Left.fEffectiveMaterialOpacity :
						-(std::numeric_limits<f32_t>::infinity)();
					const f32_t RightOpacity =
						std::isfinite(Right.fEffectiveMaterialOpacity) ?
						Right.fEffectiveMaterialOpacity :
						-(std::numeric_limits<f32_t>::infinity)();
					if (LeftOpacity != RightOpacity)
						return LeftOpacity > RightOpacity;
					return Left.fSampleTime < Right.fSampleTime;
				});
			return true;
		}
		std::ostringstream Failure;
		Failure << "Focused draw Element never produced an active Particle across "
			"its bounded visible lifetime: " <<
			Element.strElementId << " observedActive=" << bObservedActive;
		if (bObservedActive)
		{
			Failure << " firstActiveTime=" << FirstActive.fSampleTime <<
				" firstEvaluatedAlpha=" << FirstActive.Color.w <<
				" firstProbeMaxAlpha=" << FirstActive.Probe.fMaxAlpha <<
				" firstEffectiveMaterialOpacity=" <<
				FirstActive.fEffectiveMaterialOpacity;
		}
		OutError = Failure.str();
		return false;
	}

	bool Find_VisiblePixelParticleAcrossLifetime(
		const std::shared_ptr<Client::CEffectObject>& Object,
		const EFFECT_ELEMENT_DESC& Element,
		const FOCUSED_HEADLESS_RENDER_SCOPE& Scope,
		FOCUSED_PARTICLE_SAMPLE& OutSample,
		FOCUSED_DRAW_EVIDENCE& OutEvidence,
		std::string& OutError,
		FOCUSED_PARTICLE_SAMPLE* OutFirstActive = nullptr,
		const DECODED_MODEL_METRICS* ModelMetrics = nullptr)
	{
		std::vector<FOCUSED_PARTICLE_SAMPLE> Candidates;
		if (!Collect_ActiveParticleLifetimeSamples(Object, Element, Candidates,
				OutError, OutFirstActive))
		{
			return false;
		}

		std::string PeakDiagnostics;
		std::string LastDiagnostics;
		for (size_t CandidateIndex = 0u;
			CandidateIndex < Candidates.size(); ++CandidateIndex)
		{
			const FOCUSED_PARTICLE_SAMPLE& Candidate = Candidates[CandidateIndex];
			/* The bounded collection ends after this frame.  Restore its exact
			   playback clock before each real renderer/readback witness. */
			Object->Set_SampleTime(Candidate.fSampleTime);
			const float3_t Target{ Candidate.World._41, Candidate.World._42,
				Candidate.World._43 };
			FOCUSED_DRAW_EVIDENCE Evidence;
			std::string AttemptError;
			if (Render_WithEvidence(Object, Scope, Target, std::nullopt, true,
					Evidence, AttemptError, &Element, &Candidate, ModelMetrics))
			{
				std::ostringstream Scan;
				Scan << Evidence.strDiagnostics << " lifetimeScan(candidate=" <<
					(CandidateIndex + 1u) << '/' << Candidates.size() <<
					" sampleTime=" << Candidate.fSampleTime << ')';
				Evidence.strDiagnostics = Scan.str();
				OutSample = Candidate;
				OutEvidence = std::move(Evidence);
				OutError.clear();
				return true;
			}
			if (0u == CandidateIndex)
				PeakDiagnostics = Evidence.strDiagnostics;
			LastDiagnostics = Evidence.strDiagnostics;
			constexpr std::string_view ZERO_PIXEL_PREFIX =
				"Focused draw produced no committed VS/PS-visible pixels.";
			if (!AttemptError.starts_with(ZERO_PIXEL_PREFIX))
			{
				OutError = "Focused lifetime render scan failed before completing: " +
					AttemptError;
				return false;
			}
		}

		std::ostringstream Failure;
		Failure << "Focused draw produced no RGB/alpha pixel across " <<
			Candidates.size() << " bounded active lifetime samples for " <<
			Element.strElementId;
		if (!Candidates.empty())
		{
			Failure << ". peakEffectiveOpacity=" <<
				Candidates.front().fEffectiveMaterialOpacity <<
				" peakSampleTime=" << Candidates.front().fSampleTime;
		}
		if (!PeakDiagnostics.empty())
			Failure << " peakDiagnostic={" << PeakDiagnostics << '}';
		if (!LastDiagnostics.empty() && LastDiagnostics != PeakDiagnostics)
			Failure << " lastDiagnostic={" << LastDiagnostics << '}';
		OutError = Failure.str();
		return false;
	}

	struct FOCUSED_GPU_OCCURRENCE_SAMPLE final
	{
		f32_t fSampleTime = 0.f;
		float4x4_t World{};
		uint32_t iCandidateRowCount = 0u;
	};

	bool Find_FirstActiveGpuOccurrence(
		const std::shared_ptr<Client::CEffectObject>& Object,
		const EFFECT_ELEMENT_DESC& Element,
		FOCUSED_GPU_OCCURRENCE_SAMPLE& Out,
		std::string& OutError)
	{
		if (nullptr == Object)
		{
			OutError = "Focused GPU occurrence object is null.";
			return false;
		}
		const f32_t Begin = (std::max)(0.f,
			Element.Detail.Timing.fStartDelaySeconds +
			Element.SourceRecipe.fEmitterDelaySeconds);
		const f32_t End = (std::max)(Begin + 2.f,
			Element.Detail.Timing.fStartDelaySeconds +
			Element.Detail.Timing.fLifeTimeSeconds + 1.f);
		for (f32_t Time = Begin; Time <= (std::min)(Begin + 10.f, End);
			Time += 1.f / 60.f)
		{
			Object->Set_SampleTime(Time);
			const auto& Frame = Object->Get_ReconstructedTestFrame();
			const auto Occurrence = std::find_if(Frame.GpuOccurrences.begin(),
				Frame.GpuOccurrences.end(), [&Element](const auto& Candidate)
				{
					return nullptr != Candidate.pElement &&
						Candidate.pElement->strElementId == Element.strElementId;
				});
			const auto Evaluated = std::find_if(Frame.Elements.begin(),
				Frame.Elements.end(), [&Element](const auto& Candidate)
				{
					return nullptr != Candidate.pElement &&
						Candidate.pElement->strElementId == Element.strElementId;
				});
			if (Occurrence == Frame.GpuOccurrences.end() ||
				!Occurrence->bActive || 0u == Occurrence->iCandidateRowCount ||
				Evaluated == Frame.Elements.end())
			{
				continue;
			}
			Out.fSampleTime = Time;
			Out.World = Evaluated->World;
			Out.iCandidateRowCount = Occurrence->iCandidateRowCount;
			return true;
		}
		OutError = "Focused GPU occurrence never became active: " +
			Element.strElementId;
		return false;
	}

	bool Probe_ActiveRestorationOccurrences(
		const std::shared_ptr<Client::CEffectObject>& Object,
		const std::vector<const EFFECT_ELEMENT_DESC*>& Particles,
		const std::vector<const EFFECT_ELEMENT_DESC*>& Decals,
		std::size_t& OutParticleCount,
		std::size_t& OutDecalCount,
		std::vector<std::string>& OutFailures)
	{
		OutParticleCount = 0u;
		OutDecalCount = 0u;
		if (nullptr == Object)
		{
			OutFailures.emplace_back(
				"Restoration active-time probe received a null object.");
			return false;
		}
		std::map<std::string, const EFFECT_ELEMENT_DESC*, std::less<>> Unresolved;
		std::vector<f32_t> CandidateTimes;
		f32_t ScanBegin = (std::numeric_limits<f32_t>::max)();
		f32_t ScanEnd = 0.f;
		const auto AddElement = [&](const EFFECT_ELEMENT_DESC* Element,
			const Client::EFFECT_ELEMENT_KIND ExpectedKind)
		{
			if (nullptr == Element || Element->eKind != ExpectedKind ||
				!Unresolved.emplace(Element->strElementId, Element).second)
			{
				OutFailures.emplace_back(
					"Restoration active-time probe has an invalid/duplicate Element.");
				return false;
			}
			const f32_t Begin = (std::max)(0.f,
				Element->Detail.Timing.fStartDelaySeconds +
				Element->SourceRecipe.fEmitterDelaySeconds);
			CandidateTimes.insert(CandidateTimes.end(), {
				Begin, Begin + 1.f / 60.f, Begin + 0.05f,
				Begin + 0.25f, Begin + 0.5f, Begin + 1.f });
			for (const Client::EFFECT_PARTICLE_BURST_DESC& Burst :
				Element->SourceRecipe.Bursts)
			{
				if (Burst.iCountMaximum > 0u)
				{
					CandidateTimes.push_back(Begin + Burst.fTimeSeconds);
					CandidateTimes.push_back(
						Begin + Burst.fTimeSeconds + 1.f / 60.f);
				}
			}
			const f32_t End = (std::max)(Begin + 2.f,
				Element->Detail.Timing.fStartDelaySeconds +
				Element->Detail.Timing.fLifeTimeSeconds + 1.f);
			ScanBegin = (std::min)(ScanBegin, Begin);
			ScanEnd = (std::max)(ScanEnd, (std::min)(Begin + 10.f, End));
			return true;
		};
		for (const EFFECT_ELEMENT_DESC* Element : Particles)
		{
			if (!AddElement(Element, Client::EFFECT_ELEMENT_KIND::PARTICLE))
				return false;
		}
		for (const EFFECT_ELEMENT_DESC* Element : Decals)
		{
			if (!AddElement(Element, Client::EFFECT_ELEMENT_KIND::DECAL))
				return false;
		}
		if (Unresolved.empty())
			return true;
		std::sort(CandidateTimes.begin(), CandidateTimes.end());
		CandidateTimes.erase(std::unique(CandidateTimes.begin(),
			CandidateTimes.end(), [](const f32_t Left, const f32_t Right)
			{
				return std::abs(Left - Right) < 0.000001f;
			}), CandidateTimes.end());

		const auto ObserveAt = [&](const f32_t Time)
		{
			Object->Set_SampleTime(Time);
			const auto& Frame = Object->Get_ReconstructedTestFrame();
			for (auto Iterator = Unresolved.begin(); Iterator != Unresolved.end();)
			{
				const EFFECT_ELEMENT_DESC& Element = *Iterator->second;
				bool bActive = false;
				if (Element.eKind == Client::EFFECT_ELEMENT_KIND::PARTICLE)
				{
					Client::EFFECT_PARTICLE_RUNTIME_PROBE Probe;
					const auto Occurrence = std::find_if(
						Frame.GpuOccurrences.begin(), Frame.GpuOccurrences.end(),
						[&Element](const auto& Candidate)
						{
							return nullptr != Candidate.pElement &&
								Candidate.pElement->strElementId ==
									Element.strElementId;
						});
					bActive = Object->Query_ParticleRuntimeProbe(
							Element.strElementId, Probe) &&
						Probe.iActiveParticleCount > 0u &&
						Probe.bMeshRenderer ==
							(Element.SourceRecipe.strRendererShape == "mesh") &&
						Occurrence != Frame.GpuOccurrences.end() &&
						Occurrence->bActive &&
						Occurrence->iCandidateRowCount > 0u;
				}
				else
				{
					const auto Occurrence = std::find_if(
						Frame.GpuOccurrences.begin(), Frame.GpuOccurrences.end(),
						[&Element](const auto& Candidate)
						{
							return nullptr != Candidate.pElement &&
								Candidate.pElement->strElementId ==
									Element.strElementId;
						});
					bActive = Occurrence != Frame.GpuOccurrences.end() &&
						Occurrence->bActive &&
						Occurrence->iCandidateRowCount > 0u;
				}
				if (!bActive)
				{
					++Iterator;
					continue;
				}
				if (Element.eKind == Client::EFFECT_ELEMENT_KIND::PARTICLE)
					++OutParticleCount;
				else
					++OutDecalCount;
				Iterator = Unresolved.erase(Iterator);
			}
		};
		for (const f32_t Time : CandidateTimes)
		{
			ObserveAt(Time);
			if (Unresolved.empty())
				return true;
		}
		for (f32_t Time = ScanBegin; Time <= ScanEnd; Time += 1.f / 30.f)
		{
			ObserveAt(Time);
			if (Unresolved.empty())
				return true;
		}
		for (const auto& [ElementId, Element] : Unresolved)
		{
			OutFailures.emplace_back(
				"Restoration occurrence never became runtime-active: " + ElementId);
		}
		return false;
	}

	float3_t Matrix_AbsoluteScale(const float4x4_t& World)
	{
		return {
			std::sqrt(World._11 * World._11 + World._12 * World._12 +
				World._13 * World._13),
			std::sqrt(World._21 * World._21 + World._22 * World._22 +
				World._23 * World._23),
			std::sqrt(World._31 * World._31 + World._32 * World._32 +
				World._33 * World._33) };
	}

	f32_t Matrix_Determinant3x3(const float4x4_t& World)
	{
		return World._11 * (World._22 * World._33 - World._23 * World._32) -
			World._12 * (World._21 * World._33 - World._23 * World._31) +
			World._13 * (World._21 * World._32 - World._22 * World._31);
	}

	bool Is_FiniteCanonicalWorld(const float4x4_t& World)
	{
		const f32_t* Values = &World._11;
		for (size_t Index = 0u; Index < 16u; ++Index)
		{
			if (!std::isfinite(Values[Index]))
				return false;
		}
		const float3_t Scale = Matrix_AbsoluteScale(World);
		if (Scale.x <= 0.000001f || Scale.y <= 0.000001f ||
			Scale.z <= 0.000001f)
		{
			return false;
		}
		const float3_t R0{ World._11 / Scale.x, World._12 / Scale.x,
			World._13 / Scale.x };
		const float3_t R1{ World._21 / Scale.y, World._22 / Scale.y,
			World._23 / Scale.y };
		const float3_t R2{ World._31 / Scale.z, World._32 / Scale.z,
			World._33 / Scale.z };
		const auto Dot = [](const float3_t& A, const float3_t& B)
		{
			return A.x * B.x + A.y * B.y + A.z * B.z;
		};
		return std::abs(Dot(R0, R1)) < 0.002f &&
			std::abs(Dot(R0, R2)) < 0.002f &&
			std::abs(Dot(R1, R2)) < 0.002f &&
			std::abs(Matrix_Determinant3x3(World)) > 0.000001f;
	}

	bool Is_FiniteCanonicalParticleWorld(
		const EFFECT_ELEMENT_DESC& Element,
		const FOCUSED_PARTICLE_SAMPLE& Sample)
	{
		if (Element.SourceRecipe.strRendererShape == "mesh")
			return Is_FiniteCanonicalWorld(Sample.World);
		if (Element.SourceRecipe.strRendererShape != "sprite")
			return false;

		const f32_t* Values = &Sample.World._11;
		for (size_t Index = 0u; Index < 16u; ++Index)
		{
			if (!std::isfinite(Values[Index]))
				return false;
		}
		if (!std::isfinite(Sample.World._41) ||
			!std::isfinite(Sample.World._42) ||
			!std::isfinite(Sample.World._43))
		{
			return false;
		}

		constexpr f32_t BASIS_EPSILON = 0.000001f;
		if (Element.Detail.Particle.bBillboard)
		{
			const float3_t Scale = Sample.vResolvedSpriteScale;
			return Sample.bResolvedSpriteScale &&
				std::isfinite(Scale.x) && std::isfinite(Scale.y) &&
				std::isfinite(Scale.z) &&
				std::abs(Scale.x) > BASIS_EPSILON &&
				std::abs(Scale.y) > BASIS_EPSILON &&
				std::abs(Scale.x * Scale.y) > BASIS_EPSILON;
		}

		/* A Sprite is a render plane, not a solid 3D carrier.  Its signed or
		   zero-thickness third axis is source evidence and must not be rewritten
		   merely to satisfy a volume determinant. */
		const float3_t Right{
			Sample.World._11, Sample.World._12, Sample.World._13 };
		const float3_t Up{
			Sample.World._21, Sample.World._22, Sample.World._23 };
		const float3_t Cross{
			Right.y * Up.z - Right.z * Up.y,
			Right.z * Up.x - Right.x * Up.z,
			Right.x * Up.y - Right.y * Up.x };
		const f32_t RightLengthSquared =
			Right.x * Right.x + Right.y * Right.y + Right.z * Right.z;
		const f32_t UpLengthSquared =
			Up.x * Up.x + Up.y * Up.y + Up.z * Up.z;
		const f32_t AreaSquared =
			Cross.x * Cross.x + Cross.y * Cross.y + Cross.z * Cross.z;
		return RightLengthSquared > BASIS_EPSILON * BASIS_EPSILON &&
			UpLengthSquared > BASIS_EPSILON * BASIS_EPSILON &&
			AreaSquared > BASIS_EPSILON * BASIS_EPSILON;
	}

	std::string Describe_ParticleWorldContract(
		const EFFECT_ELEMENT_DESC& Element,
		const FOCUSED_PARTICLE_SAMPLE& Sample)
	{
		const float3_t Scale = Matrix_AbsoluteScale(Sample.World);
		const float3_t Right{
			Sample.World._11, Sample.World._12, Sample.World._13 };
		const float3_t Up{
			Sample.World._21, Sample.World._22, Sample.World._23 };
		const float3_t Cross{
			Right.y * Up.z - Right.z * Up.y,
			Right.z * Up.x - Right.x * Up.z,
			Right.x * Up.y - Right.y * Up.x };
		const f32_t PlaneArea = std::sqrt(
			Cross.x * Cross.x + Cross.y * Cross.y + Cross.z * Cross.z);
		std::ostringstream Diagnostic;
		Diagnostic << std::fixed << std::setprecision(6)
			<< "shape=" << Element.SourceRecipe.strRendererShape
			<< " billboard=" << Element.Detail.Particle.bBillboard
			<< " sampleTime=" << Sample.fSampleTime
			<< " worldScale=(" << Scale.x << ',' << Scale.y << ',' << Scale.z
			<< ") determinant=" << Matrix_Determinant3x3(Sample.World)
			<< " rawPlaneArea=" << PlaneArea
			<< " resolvedSpriteScale=" << Sample.bResolvedSpriteScale << "/("
			<< Sample.vResolvedSpriteScale.x << ','
			<< Sample.vResolvedSpriteScale.y << ','
			<< Sample.vResolvedSpriteScale.z << ") matrix=[";
		const f32_t* Values = &Sample.World._11;
		for (size_t Index = 0u; Index < 16u; ++Index)
		{
			if (0u != Index) Diagnostic << ',';
			Diagnostic << Values[Index];
		}
		Diagnostic << ']';
		return Diagnostic.str();
	}

	uint32_t Expected_EffectPass(const EFFECT_ELEMENT_DESC& Element,
		const f32_t WorldDeterminant)
	{
		using Client::EFFECT_RENDER_PROFILE;
		uint32_t Pass = UINT32_MAX;
		switch (Element.Material.eRenderProfile)
		{
		case EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ: Pass = 1u; break;
		case EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ: Pass = 3u; break;
		case EFFECT_RENDER_PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ: Pass = 2u; break;
		case EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ: Pass = 4u; break;
		default: return UINT32_MAX;
		}
		if (WorldDeterminant < 0.f &&
			Element.SourceRecipe.strRendererShape == "mesh")
		{
			if (3u == Pass) return 5u;
			if (4u == Pass) return 6u;
		}
		return Pass;
	}

	bool Same_Matrix(const float4x4_t& Left, const float4x4_t& Right,
		const f32_t Tolerance = 0.0001f)
	{
		const f32_t* A = &Left._11;
		const f32_t* B = &Right._11;
		for (size_t Index = 0u; Index < 16u; ++Index)
		{
			if (!Nearly_Equal(A[Index], B[Index], Tolerance, Tolerance))
				return false;
		}
		return true;
	}

	void Set_DistributionConstant(Client::EFFECT_DISTRIBUTION_DESC& Distribution,
		const float4_t& Value, const uint32_t ComponentCount)
	{
		Distribution.iComponentCount = ComponentCount;
		Distribution.iOperation = 1u;
		Distribution.iRandomLockAxes = 0u;
		Distribution.iLookupTableChunkSize = ComponentCount;
		Distribution.iLookupTableNumElements = 1u;
		Distribution.fLookupTableTimeScale = 0.f;
		Distribution.fLookupTableStartTime = 0.f;
		Distribution.vDefaultMinimum = Value;
		Distribution.vDefaultMaximum = Value;
		Distribution.LookupTable.assign(2u + ComponentCount, 0.f);
		const f32_t* Components = &Value.x;
		for (uint32_t Component = 0u; Component < ComponentCount; ++Component)
			Distribution.LookupTable[2u + Component] = Components[Component];
		Distribution.Keys.clear();
	}

	Client::EFFECT_SOURCE_MODULE_DESC* Find_SourceModule(
		EFFECT_ELEMENT_DESC& Element, const std::string_view ClassName)
	{
		const auto Iterator = std::find_if(Element.SourceRecipe.Modules.begin(),
			Element.SourceRecipe.Modules.end(), [ClassName](const auto& Module)
			{
				return Module.strClassName == ClassName;
			});
		return Iterator == Element.SourceRecipe.Modules.end() ?
			nullptr : &*Iterator;
	}

	Client::EFFECT_DISTRIBUTION_DESC* Find_Distribution(
		Client::EFFECT_SOURCE_MODULE_DESC& Module,
		const std::string_view PropertyPath)
	{
		const auto Iterator = std::find_if(Module.Distributions.begin(),
			Module.Distributions.end(), [PropertyPath](const auto& Distribution)
			{
				return Distribution.strPropertyPath == PropertyPath;
			});
		return Iterator == Module.Distributions.end() ? nullptr : &*Iterator;
	}

	bool Build_SpawnRateFixture(const EFFECT_ELEMENT_DESC& Source,
		EFFECT_DOCUMENT_DESC& Out, std::string& OutError)
	{
		EFFECT_ELEMENT_DESC Element = Source;
		Element.strElementId = "focused.spawnrate.mesh";
		Element.strDisplayName = "Focused SpawnRate Mesh";
		Element.strGroupId = "focused.spawnrate";
		Element.strSourceNode.clear();
		Element.bVisible = true;
		Element.ActionCueAttachment = {};
		Element.TransformInheritance = {};
		Element.Detail.Timing.fStartDelaySeconds = 0.f;
		Element.Detail.Timing.fLifeTimeSeconds = 5.f;
		Element.Detail.Particle.iMaxParticles = 128u;
		Element.Detail.Particle.fSpawnRatePerSecond = 0.f;
		Element.Detail.Particle.iBurstCount = 0u;
		Element.Detail.Particle.iRandomSeed = 0x13572468u;
		Element.Detail.Mesh.fModelPreScale = 0.01f;
		Element.SourceRecipe.fEmitterDelaySeconds = 0.f;
		Element.SourceRecipe.fEmitterDurationSeconds = 2.f;
		Element.SourceRecipe.iEmitterLoopCount = 1u;
		Element.SourceRecipe.Bursts.clear();
		const auto HasExactDistributions = [](const auto& Module,
			const std::initializer_list<std::string_view> Expected)
			{
				if (Module.Distributions.size() != Expected.size())
					return false;
				return std::all_of(Expected.begin(), Expected.end(),
					[&Module](const std::string_view PropertyPath)
					{
						return 1u == static_cast<size_t>(std::count_if(
							Module.Distributions.begin(),
							Module.Distributions.end(),
							[PropertyPath](const auto& Distribution)
							{
								return Distribution.strPropertyPath == PropertyPath;
							}));
					});
			};
		const auto IsExactFixtureModule = [&HasExactDistributions](
			const auto& Module)
			{
				if (Module.strClassName == "particlemodulerequired")
					return HasExactDistributions(Module, { "spawnrate" });
				if (Module.strClassName == "particlemodulelifetime")
					return HasExactDistributions(Module, { "lifetime" });
				if (Module.strClassName == "particlemodulesize")
					return HasExactDistributions(Module, { "startsize" });
				if (Module.strClassName == "particlemodulespawn")
					return HasExactDistributions(Module, { "rate", "ratescale" });
				if (Module.strClassName == "particlemoduletypedatamesh")
					return Module.Distributions.empty();
				return false;
			};
		std::set<std::string, std::less<>> SelectedClasses;
		std::vector<Client::EFFECT_SOURCE_MODULE_DESC> SelectedModules;
		SelectedModules.reserve(5u);
		for (const auto& Module : Element.SourceRecipe.Modules)
		{
			if (IsExactFixtureModule(Module) &&
				SelectedClasses.insert(Module.strClassName).second)
			{
				SelectedModules.push_back(Module);
			}
		}
		if (SelectedClasses.size() != 5u)
		{
			OutError =
				"Focused SpawnRate fixture has no exact portable Mesh carrier.";
			return false;
		}
		/* A source emitter may preserve multiple historical modules of one class.
		   This bounded synthetic fixture owns one exact Required/Lifetime/Size/
		   Spawn/TypeDataMesh occurrence, so duplicate source cardinality cannot
		   accidentally turn a runtime-consumption check into an admission check. */
		Element.SourceRecipe.Modules = std::move(SelectedModules);
		auto* Lifetime = Find_SourceModule(Element, "particlemodulelifetime");
		auto* Size = Find_SourceModule(Element, "particlemodulesize");
		auto* Spawn = Find_SourceModule(Element, "particlemodulespawn");
		auto* LifeDistribution = nullptr == Lifetime ? nullptr :
			Find_Distribution(*Lifetime, "lifetime");
		auto* SizeDistribution = nullptr == Size ? nullptr :
			Find_Distribution(*Size, "startsize");
		auto* RateDistribution = nullptr == Spawn ? nullptr :
			Find_Distribution(*Spawn, "rate");
		auto* RateScaleDistribution = nullptr == Spawn ? nullptr :
			Find_Distribution(*Spawn, "ratescale");
		if (nullptr == Lifetime || nullptr == Size || nullptr == Spawn ||
			nullptr == LifeDistribution || nullptr == SizeDistribution ||
			nullptr == RateDistribution || nullptr == RateScaleDistribution)
		{
			OutError = "Focused SpawnRate fixture lost a required source module.";
			return false;
		}
		Set_DistributionConstant(*LifeDistribution, { 5.f, 5.f, 5.f, 5.f }, 1u);
		Set_DistributionConstant(*SizeDistribution, { 1.f, 1.f, 1.f, 1.f }, 3u);
		Set_DistributionConstant(*RateDistribution, { 10.f, 10.f, 10.f, 10.f }, 1u);
		Set_DistributionConstant(*RateScaleDistribution, { 1.f, 1.f, 1.f, 1.f }, 1u);

		EFFECT_DOCUMENT_DESC Document;
		Document.strEffectAssetId = "effect.focused.spawnrate.fixture";
		Document.strDisplayName = "Focused SpawnRate fixture";
		Document.Elements.push_back(std::move(Element));
		if (!CEffectDocumentCodec::Validate_Drawable(Document, OutError))
			return false;
		Out = std::move(Document);
		return true;
	}

	bool Stage_AndSampleFixture(
		const std::unique_ptr<Client::CEffectObject>& Prototype,
		const EFFECT_DOCUMENT_DESC& Document, const f32_t SampleTime,
		std::shared_ptr<Client::CEffectObject>& OutObject,
		FOCUSED_PARTICLE_SAMPLE& OutSample, std::string& OutError)
	{
		OutObject = std::dynamic_pointer_cast<Client::CEffectObject>(
			Prototype->Clone(nullptr));
		if (nullptr == OutObject ||
			!OutObject->Stage_Document(Document, OutError))
		{
			if (OutError.empty())
				OutError = "Focused playback fixture stage failed.";
			return false;
		}
		OutObject->Set_SampleTime(SampleTime);
		const EFFECT_ELEMENT_DESC& Element = Document.Elements.front();
		Client::EFFECT_PARTICLE_RUNTIME_PROBE Probe;
		if (!OutObject->Query_ParticleRuntimeProbe(Element.strElementId, Probe) ||
			0u == Probe.iActiveParticleCount)
		{
			OutError = "Focused playback fixture produced no active Particle.";
			return false;
		}
		const auto& Frame = OutObject->Get_ReconstructedTestFrame();
		const auto Particle = std::find_if(Frame.Particles.begin(),
			Frame.Particles.end(), [&Element](const auto& Candidate)
			{
				return nullptr != Candidate.pElement &&
					Candidate.pElement->strElementId == Element.strElementId;
			});
		if (Particle == Frame.Particles.end())
		{
			OutError = "Focused playback fixture lost its evaluated Particle.";
			return false;
		}
		Capture_FocusedParticleSample(*Particle, SampleTime, Probe, OutSample);
		return true;
	}

	bool Stage_OrdinarySingleVisibleElementPreview(
		const std::unique_ptr<Client::CEffectObject>& Prototype,
		const EFFECT_DOCUMENT_DESC& Document, const std::string_view ElementId,
		std::shared_ptr<Client::CEffectObject>& OutObject, std::string& OutError)
	{
		OutObject.reset();
		EFFECT_DOCUMENT_DESC Isolated = Document;
		EFFECT_ELEMENT_DESC* Target = Find_ExactElement(Isolated, ElementId);
		if (nullptr == Target ||
			!Is_RestorationPreviewExecutionTarget(*Target))
		{
			OutError =
				"Ordinary single-Element preview target is not an execution target: " +
				std::string(ElementId);
			return false;
		}
		for (EFFECT_ELEMENT_DESC& Element : Isolated.Elements)
			Element.bVisible = Element.strElementId == ElementId;
		/* Model cues are an independent family and would contaminate a Particle or
		   Decal pipeline witness.  The immutable source document was already hash-
		   validated; this non-persisted copy changes only preview visibility. */
		Isolated.ModelCues.clear();
		std::shared_ptr<Client::CEffectObject> Object =
			std::dynamic_pointer_cast<Client::CEffectObject>(Prototype->Clone(nullptr));
		if (nullptr == Object || !Object->Stage_Document(Isolated, OutError))
		{
			if (OutError.empty())
			{
				OutError = "Ordinary single-Element preview stage failed: " +
					std::string(ElementId);
			}
			return false;
		}
		OutObject = std::move(Object);
		return true;
	}

	bool Verify_AuthoringApproximateCodecMutationFixture(
		const std::unique_ptr<Client::CEffectObject>& Prototype,
		const EFFECT_DOCUMENT_DESC& SourceDocument,
		const std::string_view FullElementId,
		const std::string_view HardElementId,
		std::string& OutError)
	{
		EFFECT_DOCUMENT_DESC Approximate = SourceDocument;
		Approximate.ModelCues.clear();
		for (EFFECT_ELEMENT_DESC& Element : Approximate.Elements)
			Element.bVisible = Element.strElementId == FullElementId;
		EFFECT_ELEMENT_DESC* ApproximateElement = Find_ExactElement(
			Approximate, FullElementId);
		const EFFECT_ELEMENT_DESC* SourceHardElement = Find_ExactElement(
			Approximate, HardElementId);
		if (nullptr == ApproximateElement || nullptr == SourceHardElement ||
			ApproximateElement->eKind != Client::EFFECT_ELEMENT_KIND::PARTICLE ||
			!ApproximateElement->SourceRecipe.bEnabled ||
			!SourceHardElement->Material.Execution.bFailClosed ||
			SourceHardElement->Material.Execution.bAuthoringApproximate)
		{
			OutError =
				"Authoring-approximate codec fixture lacks its full/hard carriers.";
			return false;
		}
		ApproximateElement->Material.Execution = {};
		ApproximateElement->Material.Execution.bFailClosed = true;
		ApproximateElement->Material.Execution.bAuthoringApproximate = true;

		std::string ValidationError;
		if (!CEffectDocumentCodec::Validate(Approximate, ValidationError) ||
			!CEffectDocumentCodec::Validate_Drawable(Approximate, ValidationError))
		{
			OutError =
				"Valid authoring-approximate codec fixture was rejected: " +
				ValidationError;
			return false;
		}
		const std::string Canonical = CEffectDocumentCodec::Serialize(Approximate);
		EFFECT_DOCUMENT_DESC RoundTrip;
		if (!CEffectDocumentCodec::Parse(Canonical, RoundTrip, ValidationError) ||
			CEffectDocumentCodec::Serialize(RoundTrip) != Canonical)
		{
			OutError =
				"Valid authoring-approximate codec fixture did not round-trip: " +
				ValidationError;
			return false;
		}
		ApproximateElement = Find_ExactElement(RoundTrip, FullElementId);
		if (nullptr == ApproximateElement ||
			!ApproximateElement->Material.Execution.bFailClosed ||
			!ApproximateElement->Material.Execution.bAuthoringApproximate ||
			!Is_RestorationPreviewExecutionTarget(*ApproximateElement))
		{
			OutError =
				"Authoring-approximate codec flags changed during round-trip.";
			return false;
		}

		std::shared_ptr<Client::CEffectObject> Object =
			std::dynamic_pointer_cast<Client::CEffectObject>(Prototype->Clone(nullptr));
		if (nullptr == Object || !Object->Stage_Document(RoundTrip, OutError))
		{
			if (OutError.empty())
				OutError = "Valid authoring-approximate Stage fixture failed.";
			return false;
		}
		std::size_t ParticleProbeCount = 0u;
		std::size_t DecalProbeCount = 0u;
		std::vector<std::string> ProbeFailures;
		if (!Probe_ActiveRestorationOccurrences(Object, { ApproximateElement }, {},
				ParticleProbeCount, DecalProbeCount, ProbeFailures) ||
			ParticleProbeCount != 1u || 0u != DecalProbeCount)
		{
			OutError =
				"Valid authoring-approximate Stage/runtime/draw-submission probe failed.";
			for (const std::string& Failure : ProbeFailures)
				OutError += " " + Failure;
			return false;
		}
		Client::EFFECT_PARTICLE_RUNTIME_PROBE CommittedProbe;
		if (!Object->Query_ParticleRuntimeProbe(
				ApproximateElement->strElementId, CommittedProbe) ||
			0u == CommittedProbe.iActiveParticleCount)
		{
			OutError =
				"Valid authoring-approximate fixture lost its committed Particle.";
			return false;
		}
		const std::string CommittedStatus = Object->Get_Status();
		const auto SameFloat4 = [](const float4_t& Left, const float4_t& Right)
		{
			return Left.x == Right.x && Left.y == Right.y &&
				Left.z == Right.z && Left.w == Right.w;
		};
		const auto SameProbe = [&SameFloat4](
			const Client::EFFECT_PARTICLE_RUNTIME_PROBE& Left,
			const Client::EFFECT_PARTICLE_RUNTIME_PROBE& Right)
		{
			return Left.fSampleTimeSeconds == Right.fSampleTimeSeconds &&
				Left.iActiveParticleCount == Right.iActiveParticleCount &&
				Left.bMeshRenderer == Right.bMeshRenderer &&
				SameFloat4(Left.vFirstDynamicParameter,
					Right.vFirstDynamicParameter) &&
				SameFloat4(Left.vMinDynamicParameter,
					Right.vMinDynamicParameter) &&
				SameFloat4(Left.vMaxDynamicParameter,
					Right.vMaxDynamicParameter) &&
				Left.fFirstAlpha == Right.fFirstAlpha &&
				Left.fMinAlpha == Right.fMinAlpha &&
				Left.fMaxAlpha == Right.fMaxAlpha &&
				Left.fFirstNormalizedLife == Right.fFirstNormalizedLife &&
				Left.fFirstSubImageIndex == Right.fFirstSubImageIndex &&
				SameFloat4(Left.FirstSubUV.Current, Right.FirstSubUV.Current) &&
				SameFloat4(Left.FirstSubUV.Next, Right.FirstSubUV.Next) &&
				Left.FirstSubUV.fBlend == Right.FirstSubUV.fBlend;
		};
		const auto RejectsAndRollsBack = [&](EFFECT_DOCUMENT_DESC Invalid,
			const std::string_view Label,
			const std::string_view ExpectedRejection)
		{
			std::string CodecRejection;
			std::string StageRejection;
			const bool bCodecRejected =
				!CEffectDocumentCodec::Validate(Invalid, CodecRejection);
			const bool bStageRejected =
				!Object->Stage_Document(Invalid, StageRejection);
			Client::EFFECT_PARTICLE_RUNTIME_PROBE PreservedProbe;
			const bool bProbePreserved = Object->Query_ParticleRuntimeProbe(
					ApproximateElement->strElementId, PreservedProbe) &&
				SameProbe(CommittedProbe, PreservedProbe);
			const bool bExpectedCodecRejection = ExpectedRejection.empty() ||
				CodecRejection.find(ExpectedRejection) != std::string::npos;
			const bool bExpectedStageRejection = ExpectedRejection.empty() ||
				StageRejection.find(ExpectedRejection) != std::string::npos;
			if (!bCodecRejected || CodecRejection.empty() || !bStageRejected ||
				StageRejection.empty() || !bProbePreserved ||
				Object->Get_Status() != CommittedStatus ||
				!bExpectedCodecRejection || !bExpectedStageRejection)
			{
				std::ostringstream Failure;
				Failure << "Authoring-approximate " << Label <<
					" mutation did not reject and roll back"
					<< ": codecRejected=" << bCodecRejected
					<< " stageRejected=" << bStageRejected
					<< " probePreserved=" << bProbePreserved
					<< " statusPreserved=" <<
						(Object->Get_Status() == CommittedStatus)
					<< " codecError=" << CodecRejection
					<< " stageError=" << StageRejection;
				OutError = Failure.str();
				return false;
			}
			return true;
		};

		EFFECT_DOCUMENT_DESC MissingResource = RoundTrip;
		EFFECT_ELEMENT_DESC* MissingResourceElement = Find_ExactElement(
			MissingResource, FullElementId);
		if (nullptr == MissingResourceElement)
		{
			OutError = "Approximate missing-resource fixture lost its Element.";
			return false;
		}
		std::erase_if(MissingResourceElement->ResourceBindings,
			[](const Client::EFFECT_RESOURCE_BINDING_DESC& Binding)
			{
				return Binding.strSlotId != Client::EFFECT_MESH_SHAPE_SLOT_ID;
			});
		if (!RejectsAndRollsBack(std::move(MissingResource),
				"missing-resource", "Authoring-approximate carrier lacks"))
		{
			return false;
		}

		EFFECT_DOCUMENT_DESC UnsupportedModule = RoundTrip;
		EFFECT_ELEMENT_DESC* UnsupportedElement = Find_ExactElement(
			UnsupportedModule, FullElementId);
		if (nullptr == UnsupportedElement)
		{
			OutError = "Approximate unsupported-module fixture lost its Element.";
			return false;
		}
		Client::EFFECT_SOURCE_MODULE_DESC Unsupported;
		Unsupported.strStableId = "fixture.approximate.unsupported.module";
		Unsupported.strClassName = "particlemodulecollision";
		Unsupported.strObjectPath = "fixture/approximate-unsupported-module";
		UnsupportedElement->SourceRecipe.Modules.push_back(std::move(Unsupported));
		if (!RejectsAndRollsBack(std::move(UnsupportedModule),
				"unsupported-module", "sourceRecipe is not admitted"))
		{
			return false;
		}

		EFFECT_DOCUMENT_DESC UnmatchedEvent = RoundTrip;
		EFFECT_ELEMENT_DESC* EventElement = Find_ExactElement(
			UnmatchedEvent, FullElementId);
		if (nullptr == EventElement)
		{
			OutError = "Approximate unmatched-event fixture lost its Element.";
			return false;
		}
		Client::EFFECT_SOURCE_MODULE_DESC Generator;
		Generator.strStableId = "fixture.approximate.unmatched.generator";
		Generator.strClassName = "particlemoduleeventgenerator";
		Generator.strObjectPath = "fixture/approximate-unmatched-generator";
		for (const auto& [Property, Value] : {
				std::pair<std::string_view, std::string_view>{
					"events[0].type", "epet_spawn" },
				std::pair<std::string_view, std::string_view>{
					"events[0].customname", "fixture_approximate_unmatched" } })
		{
			Client::EFFECT_SOURCE_LITERAL_DESC Literal;
			Literal.strPropertyPath = Property;
			Literal.eKind = Client::EFFECT_SOURCE_LITERAL_KIND::STRING;
			Literal.strString = Value;
			Generator.Literals.push_back(std::move(Literal));
		}
		EventElement->SourceRecipe.Modules.push_back(std::move(Generator));
		if (!RejectsAndRollsBack(std::move(UnmatchedEvent),
				"unmatched-event-route", "same-document"))
		{
			return false;
		}

		EFFECT_DOCUMENT_DESC HardActivation = RoundTrip;
		EFFECT_ELEMENT_DESC* ActivatedHard = Find_ExactElement(
			HardActivation, HardElementId);
		if (nullptr == ActivatedHard)
		{
			OutError = "Approximate hard-activation fixture lost its Element.";
			return false;
		}
		ActivatedHard->bVisible = true;
		return RejectsAndRollsBack(std::move(HardActivation),
			"hard-activation",
			"Hard fail-closed authored Element cannot be made visible");
	}

	bool Verify_RateScaleAndSizeScaleConsumption(
		const std::unique_ptr<Client::CEffectObject>& Prototype,
		const EFFECT_ELEMENT_DESC& Carrier, std::string& OutError)
	{
		EFFECT_DOCUMENT_DESC RateOne;
		if (!Build_SpawnRateFixture(Carrier, RateOne, OutError))
			return false;
		EFFECT_DOCUMENT_DESC RateTwo = RateOne;
		EFFECT_DOCUMENT_DESC RateMissing = RateOne;
		auto* RateTwoSpawn = Find_SourceModule(
			RateTwo.Elements.front(), "particlemodulespawn");
		auto* RateTwoDistribution = nullptr == RateTwoSpawn ? nullptr :
			Find_Distribution(*RateTwoSpawn, "ratescale");
		auto* MissingSpawn = Find_SourceModule(
			RateMissing.Elements.front(), "particlemodulespawn");
		if (nullptr == RateTwoDistribution || nullptr == MissingSpawn)
		{
			OutError = "Focused RateScale mutation lost its source distribution.";
			return false;
		}
		Set_DistributionConstant(*RateTwoDistribution,
			{ 2.f, 2.f, 2.f, 2.f }, 1u);
		std::erase_if(MissingSpawn->Distributions, [](const auto& Distribution)
			{
				return Distribution.strPropertyPath == "ratescale";
			});
		std::shared_ptr<Client::CEffectObject> RateOneObject;
		std::shared_ptr<Client::CEffectObject> RateTwoObject;
		FOCUSED_PARTICLE_SAMPLE One;
		FOCUSED_PARTICLE_SAMPLE Two;
		if (!Stage_AndSampleFixture(
				Prototype, RateOne, 1.f, RateOneObject, One, OutError) ||
			!Stage_AndSampleFixture(
				Prototype, RateTwo, 1.f, RateTwoObject, Two, OutError))
		{
			return false;
		}
		std::string MissingRejection;
		if (CEffectDocumentCodec::Validate_Drawable(
				RateMissing, MissingRejection) || MissingRejection.empty())
		{
			OutError =
				"Focused incomplete RateScale carrier was not rejected by admission.";
			return false;
		}
		if (Two.Probe.iActiveParticleCount <= One.Probe.iActiveParticleCount)
		{
			std::ostringstream Failure;
			Failure << "Focused RateScale consumption failed: one=" <<
				One.Probe.iActiveParticleCount << " two=" <<
				Two.Probe.iActiveParticleCount;
			OutError = Failure.str();
			return false;
		}

		EFFECT_DOCUMENT_DESC SizeIdentity = RateOne;
		EFFECT_ELEMENT_DESC& IdentityElement = SizeIdentity.Elements.front();
		IdentityElement.strElementId = "focused.sizescalebytime.mesh";
		IdentityElement.strDisplayName = "Focused SizeScaleByTime Mesh";
		IdentityElement.SourceRecipe.Bursts = { { 0.f, 1u, 1u } };
		auto* IdentitySpawn = Find_SourceModule(
			IdentityElement, "particlemodulespawn");
		auto* IdentityRate = nullptr == IdentitySpawn ? nullptr :
			Find_Distribution(*IdentitySpawn, "rate");
		auto* IdentityRateScale = nullptr == IdentitySpawn ? nullptr :
			Find_Distribution(*IdentitySpawn, "ratescale");
		if (nullptr == IdentityRate || nullptr == IdentityRateScale)
		{
			OutError = "Focused SizeScaleByTime fixture lost Spawn distributions.";
			return false;
		}
		Set_DistributionConstant(*IdentityRate, { 0.f, 0.f, 0.f, 0.f }, 1u);
		Set_DistributionConstant(*IdentityRateScale,
			{ 1.f, 1.f, 1.f, 1.f }, 1u);
		Client::EFFECT_SOURCE_MODULE_DESC SizeScaleModule;
		SizeScaleModule.strStableId = "focused.sizescalebytime";
		SizeScaleModule.strClassName = "particlemodulesizescalebytime";
		SizeScaleModule.strObjectPath = "fixture/focused.sizescalebytime";
		Client::EFFECT_DISTRIBUTION_DESC SizeScaleDistribution;
		SizeScaleDistribution.strPropertyPath = "sizescalebytime";
		Set_DistributionConstant(SizeScaleDistribution,
			{ 1.f, 1.f, 1.f, 1.f }, 3u);
		SizeScaleModule.Distributions.push_back(std::move(SizeScaleDistribution));
		IdentityElement.SourceRecipe.Modules.push_back(
			std::move(SizeScaleModule));
		SizeIdentity.strEffectAssetId = "effect.focused.sizescalebytime.fixture";
		SizeIdentity.strDisplayName = "Focused SizeScaleByTime fixture";

		EFFECT_DOCUMENT_DESC SizeScaled = SizeIdentity;
		auto* ScaledModule = Find_SourceModule(
			SizeScaled.Elements.front(), "particlemodulesizescalebytime");
		auto* ScaledDistribution = nullptr == ScaledModule ? nullptr :
			Find_Distribution(*ScaledModule, "sizescalebytime");
		if (nullptr == ScaledDistribution)
		{
			OutError = "Focused SizeScaleByTime mutation lost its distribution.";
			return false;
		}
		Set_DistributionConstant(*ScaledDistribution,
			{ 2.f, 3.f, 4.f, 1.f }, 3u);
		EFFECT_DOCUMENT_DESC DetailFallback = SizeIdentity;
		DetailFallback.Elements.front().Detail.Particle.vStartSize = { 77.f, 88.f };
		DetailFallback.Elements.front().Detail.Particle.vEndSize = { 99.f, 111.f };

		std::shared_ptr<Client::CEffectObject> SizeIdentityObject;
		std::shared_ptr<Client::CEffectObject> SizeScaledObject;
		std::shared_ptr<Client::CEffectObject> DetailFallbackObject;
		FOCUSED_PARTICLE_SAMPLE IdentitySample;
		FOCUSED_PARTICLE_SAMPLE ScaledSample;
		FOCUSED_PARTICLE_SAMPLE FallbackSample;
		if (!Stage_AndSampleFixture(Prototype, SizeIdentity, 0.5f,
				SizeIdentityObject, IdentitySample, OutError) ||
			!Stage_AndSampleFixture(Prototype, SizeScaled, 0.5f,
				SizeScaledObject, ScaledSample, OutError) ||
			!Stage_AndSampleFixture(Prototype, DetailFallback, 0.5f,
				DetailFallbackObject, FallbackSample, OutError))
		{
			return false;
		}
		const float3_t IdentityScale = Matrix_AbsoluteScale(IdentitySample.World);
		const float3_t ScaledScale = Matrix_AbsoluteScale(ScaledSample.World);
		const bool_t bSourceScaleConsumed =
			Nearly_Equal(ScaledScale.x / IdentityScale.x, 2.f, 0.002f, 0.002f) &&
			Nearly_Equal(ScaledScale.y / IdentityScale.y, 4.f, 0.002f, 0.002f) &&
			Nearly_Equal(ScaledScale.z / IdentityScale.z, 3.f, 0.002f, 0.002f);
		if (!bSourceScaleConsumed ||
			!Same_Matrix(FallbackSample.World, IdentitySample.World))
		{
			OutError =
				"Focused SizeScaleByTime recipe/Detail ownership consumption failed.";
			return false;
		}
		return true;
	}

	bool Validate_FocusedOccurrenceSubmission(
		const Client::CEffectObject& Object,
		const EFFECT_ELEMENT_DESC& Element,
		const FOCUSED_PARTICLE_SAMPLE& Sample,
		const FOCUSED_DRAW_EVIDENCE& Evidence,
		const DECODED_MODEL_METRICS* ModelMetrics,
		std::string& OutError);

	bool Is_FocusedDrawOnlyRequested()
	{
		std::array<wchar_t, 8u> Value{};
		const DWORD Length = GetEnvironmentVariableW(
			L"LOSTARK_TRACK_A_FOCUSED_DRAW_ONLY", Value.data(),
			static_cast<DWORD>(Value.size()));
		return 1u == Length && L'1' == Value[0];
	}

	bool Is_FocusedRepresentativesOnlyRequested()
	{
		std::array<wchar_t, 8u> Value{};
		const DWORD Length = GetEnvironmentVariableW(
			L"LOSTARK_TRACK_A_FOCUSED_REPRESENTATIVES_ONLY", Value.data(),
			static_cast<DWORD>(Value.size()));
		return 1u == Length && L'1' == Value[0];
	}

	void Set_DiagnosticDistributionConstant(
		EFFECT_DISTRIBUTION_DESC& Distribution, const float4_t& Value)
	{
		Distribution.vDefaultMinimum = Value;
		Distribution.vDefaultMaximum = Value;
		for (EFFECT_DISTRIBUTION_KEY_DESC& Key : Distribution.Keys)
		{
			Key.vMinimum = Value;
			Key.vMaximum = Value;
			Key.vArriveTangentMinimum = {};
			Key.vLeaveTangentMinimum = {};
			Key.vArriveTangentMaximum = {};
			Key.vLeaveTangentMaximum = {};
		}
		if (Distribution.LookupTable.size() <= 2u)
			return;
		const uint32_t ComponentCount = (std::max)(
			1u, Distribution.iComponentCount);
		const f32_t* Components = &Value.x;
		for (size_t Index = 2u; Index < Distribution.LookupTable.size(); ++Index)
		{
			Distribution.LookupTable[Index] =
				Components[(Index - 2u) % ComponentCount];
		}
	}

	bool Is_SourceColorModule(const std::string_view ClassName)
	{
		return ClassName == "particlemodulecolor" ||
			ClassName == "particlemodulecoloroverlife" ||
			ClassName == "particlemodulecolorscaleoverlife";
	}

	void Neutralize_DiagnosticParticleColor(EFFECT_ELEMENT_DESC& Element)
	{
		Element.Detail.Color = {};
		Element.Detail.LinearLerp.bColorOffset = false;
		Element.Detail.LinearLerp.bColorMultiply = false;
		for (EFFECT_SOURCE_MODULE_DESC& Module : Element.SourceRecipe.Modules)
		{
			if (!Is_SourceColorModule(Module.strClassName))
				continue;
			for (EFFECT_DISTRIBUTION_DESC& Distribution : Module.Distributions)
			{
				Set_DiagnosticDistributionConstant(
					Distribution, { 1.f, 1.f, 1.f, 1.f });
			}
		}
	}

	void Zero_DiagnosticParticleColor(EFFECT_ELEMENT_DESC& Element)
	{
		Element.Detail.Color = {};
		Element.Detail.Color.vColorMultiply = {};
		Element.Detail.LinearLerp.bColorOffset = false;
		Element.Detail.LinearLerp.bColorMultiply = false;
		for (EFFECT_SOURCE_MODULE_DESC& Module : Element.SourceRecipe.Modules)
		{
			if (!Is_SourceColorModule(Module.strClassName))
				continue;
			for (EFFECT_DISTRIBUTION_DESC& Distribution : Module.Distributions)
				Set_DiagnosticDistributionConstant(Distribution, {});
		}
	}

	f32_t DiagnosticNeutralDynamicValue(const std::string_view Semantic)
	{
		return Semantic == "opacity" || Semantic == "emissive" ? 1.f : 0.f;
	}

	void Neutralize_DiagnosticDynamicParameters(EFFECT_ELEMENT_DESC& Element)
	{
		for (EFFECT_SOURCE_MODULE_DESC& Module : Element.SourceRecipe.Modules)
		{
			if (Module.strClassName != "particlemoduleparameterdynamic")
				continue;
			for (EFFECT_DISTRIBUTION_DESC& Distribution : Module.Distributions)
			{
				for (size_t Channel = 0u; Channel < 4u; ++Channel)
				{
					const std::string Prefix = "dynamicparams[" +
						std::to_string(Channel) + "].paramvalue";
					if (Distribution.strPropertyPath != Prefix)
						continue;
					const f32_t Neutral = DiagnosticNeutralDynamicValue(
						Element.Material.SourceMaterial.
							DynamicParameterSemantics[Channel]);
					Set_DiagnosticDistributionConstant(Distribution,
						{ Neutral, Neutral, Neutral, Neutral });
					break;
				}
			}
		}
	}

	uint32_t DiagnosticSourceProfileIndex(
		const EFFECT_SOURCE_MATERIAL_DESC& Source)
	{
		if (!Source.bEnabled || Source.strRuntimeShaderProfileId ==
			"effect.ue3.reconstructed-standard.v1")
		{
			return 0u;
		}
		if (Source.strRuntimeShaderProfileId ==
			"effect.ue3.grouped-translucent.v1")
		{
			return 6u;
		}
		return UINT32_MAX;
	}

	bool Capture_DiagnosticParticleAtTime(
		const std::shared_ptr<Client::CEffectObject>& Object,
		const EFFECT_ELEMENT_DESC& Element, const f32_t SampleTime,
		FOCUSED_PARTICLE_SAMPLE& Out, std::string& OutError)
	{
		Object->Set_SampleTime(SampleTime);
		Client::EFFECT_PARTICLE_RUNTIME_PROBE Probe;
		if (!Object->Query_ParticleRuntimeProbe(Element.strElementId, Probe) ||
			0u == Probe.iActiveParticleCount)
		{
			OutError = "Synthetic material diagnostic has no active Particle.";
			return false;
		}
		const auto& Frame = Object->Get_ReconstructedTestFrame();
		const auto Particle = std::find_if(Frame.Particles.begin(),
			Frame.Particles.end(), [&Element](const auto& Candidate)
			{
				return nullptr != Candidate.pElement &&
					Candidate.pElement->strElementId == Element.strElementId;
			});
		if (Particle == Frame.Particles.end())
		{
			OutError = "Synthetic material diagnostic lost its Particle row.";
			return false;
		}
		Capture_FocusedParticleSample(*Particle, SampleTime, Probe, Out);
		return true;
	}

	bool Run_FocusedMaterialDiagnosticLadder(
		const std::unique_ptr<Client::CEffectObject>& Prototype,
		const FOCUSED_HEADLESS_RENDER_SCOPE& Scope,
		const EFFECT_DOCUMENT_DESC& SourceDocument,
		const EFFECT_ELEMENT_DESC& SourceElement,
		const f32_t SampleTime, std::string& OutError)
	{
		struct LADDER_CASE final
		{
			const char_t* pName = nullptr;
			bool_t bStandardMaterial = false;
			bool_t bNeutralColor = false;
			bool_t bNeutralDynamic = false;
		};
		constexpr std::array<LADDER_CASE, 4u> CASES = {{
			{ "A-standard-neutral-color-dynamic", true, true, true },
			{ "B-grouped-neutral-color-dynamic", false, true, true },
			{ "C-grouped-real-color-neutral-dynamic", false, false, true },
			{ "D-grouped-real-row", false, false, false }
		}};

		for (const LADDER_CASE& Case : CASES)
		{
			EFFECT_DOCUMENT_DESC Document = SourceDocument;
			EFFECT_ELEMENT_DESC* Element = Find_ExactElement(
				Document, SourceElement.strElementId);
			if (nullptr == Element)
			{
				OutError = "Synthetic material ladder lost the exact Element.";
				return false;
			}
			if (Case.bStandardMaterial)
			{
				Element->Material.strTemplateId = "effect.standard";
				Element->Material.SourceMaterial.bEnabled = false;
				Element->Material.Execution = {};
			}
			if (Case.bNeutralColor)
				Neutralize_DiagnosticParticleColor(*Element);
			if (Case.bNeutralDynamic)
				Neutralize_DiagnosticDynamicParameters(*Element);

			std::string AdmissionError;
			if (!CEffectDocumentCodec::Validate(Document, AdmissionError))
			{
				OutError = std::string("Synthetic material ladder codec rejected ") +
					Case.pName + ": " + AdmissionError;
				return false;
			}
			std::shared_ptr<Client::CEffectObject> Object;
			if (!Stage_OrdinarySingleVisibleElementPreview(Prototype, Document,
					Element->strElementId, Object, OutError))
			{
				OutError = std::string("Synthetic material ladder stage rejected ") +
					Case.pName + ": " + OutError;
				return false;
			}
			FOCUSED_PARTICLE_SAMPLE Sample;
			if (!Capture_DiagnosticParticleAtTime(
					Object, *Element, SampleTime, Sample, OutError))
			{
				OutError = std::string("Synthetic material ladder sample failed ") +
					Case.pName + ": " + OutError;
				return false;
			}
			const float3_t Target{ Sample.World._41, Sample.World._42,
				Sample.World._43 };
			FOCUSED_DRAW_EVIDENCE Evidence;
			std::string RenderError;
			bool_t bVisible = false;
			for (size_t ProofView = 4u; ProofView < 8u; ++ProofView)
			{
				RenderError.clear();
				if (Render_WithEvidence(Object, Scope, Target, ProofView, true,
						Evidence, RenderError, Element, &Sample))
				{
					bVisible = true;
					break;
				}
				if (!RenderError.starts_with(
						"Focused draw produced no committed VS/PS-visible pixels."))
				{
					break;
				}
			}
			const EFFECT_GROUPED_TRANSLUCENT_CONSTANTS Grouped =
				Build_EffectGroupedTranslucentConstants(
					Element->Material.SourceMaterial);
			const auto* Base = Find_ResourceBinding(*Element, "base");
			const auto HasSlot = [Element](const std::string_view Slot)
			{
				return nullptr != Find_ResourceBinding(*Element, Slot);
			};
			const auto& Execution = Element->Material.Execution;
			std::cout << "[FOCUSED MATERIAL LADDER] case=" << Case.pName <<
				" visible=" << bVisible << " proofView=" << Evidence.iViewIndex <<
				" sampleTime=" << SampleTime <<
				" sampleColor=(" << Sample.Color.x << ',' << Sample.Color.y << ',' <<
				Sample.Color.z << ',' << Sample.Color.w << ") dynamic=(" <<
				Sample.vDynamicParameter.x << ',' << Sample.vDynamicParameter.y << ',' <<
				Sample.vDynamicParameter.z << ',' << Sample.vDynamicParameter.w <<
				") base=" << (nullptr == Base ? "<none>" : Base->strAssetId) <<
				" gHas(base/noise/mask/emissive/dissolve)=(" <<
				HasSlot("base") << '/' << HasSlot("noise") << '/' <<
				HasSlot("mask") << '/' << HasSlot("emissive") << '/' <<
				HasSlot("dissolve") << ") sourceProfileIndex=" <<
				DiagnosticSourceProfileIndex(Element->Material.SourceMaterial) <<
				" groupedUV=(" << Grouped.vUVScalePan.x << ',' <<
				Grouped.vUVScalePan.y << ',' << Grouped.vUVScalePan.z << ',' <<
				Grouped.vUVScalePan.w << ") groupedAlphaEmissive=(" <<
				Grouped.vAlphaEmissive.x << ',' << Grouped.vAlphaEmissive.y << ',' <<
				Grouped.vAlphaEmissive.z << ',' << Grouped.vAlphaEmissive.w <<
				") groupedNoiseDissolve=(" << Grouped.vNoiseDissolve.x << ',' <<
				Grouped.vNoiseDissolve.y << ',' << Grouped.vNoiseDissolve.z << ',' <<
				Grouped.vNoiseDissolve.w << ") groupedTint=(" <<
				Grouped.vTint.x << ',' << Grouped.vTint.y << ',' << Grouped.vTint.z <<
				',' << Grouped.vTint.w << ") groupedFlags=0x" << std::hex <<
				Grouped.iFlags << std::dec << " execution(enabled/failClosed/backend/"
				"opcode/pass/mask)=(" << Execution.bEnabled << '/' <<
				Execution.bFailClosed << '/' << static_cast<uint32_t>(
					Execution.eBackend) << '/' << Execution.iOpcode << '/' <<
				Execution.iPassIndex << "/0x" << std::hex << Execution.iTextureMask <<
				std::dec << ") pipeline(ia/vs/ps)=(" <<
				Evidence.Pipeline.IAPrimitives << ',' <<
				Evidence.Pipeline.VSInvocations << ',' <<
				Evidence.Pipeline.PSInvocations << ") rtColor(nonzero/sum/max)=(" <<
				Evidence.iColorNonZeroPixelCount << ',' << Evidence.iColorByteSum << ',' <<
				static_cast<uint32_t>(Evidence.iMaximumColorByte) <<
				") rtAlpha(nonzero/one)=(" << Evidence.iAlphaNonZeroPixelCount << ',' <<
				Evidence.iAlphaOnePixelCount << ") error=" <<
				(RenderError.empty() ? "<none>" :
					(RenderError.starts_with(
						"Focused draw produced no committed VS/PS-visible pixels.") ?
						"zero-pixel" : RenderError)) << '\n';
		}

		/* A fitted proof camera must not manufacture approval for a shader whose
		   actual carrier color is identically zero.  This negative uses the same
		   Base/geometry and codec/stage path as ladder A. */
		EFFECT_DOCUMENT_DESC ZeroDocument = SourceDocument;
		EFFECT_ELEMENT_DESC* ZeroElement = Find_ExactElement(
			ZeroDocument, SourceElement.strElementId);
		if (nullptr == ZeroElement)
		{
			OutError = "Synthetic zero-output negative lost the exact Element.";
			return false;
		}
		ZeroElement->Material.strTemplateId = "effect.standard";
		ZeroElement->Material.SourceMaterial.bEnabled = false;
		ZeroElement->Material.Execution = {};
		Zero_DiagnosticParticleColor(*ZeroElement);
		Neutralize_DiagnosticDynamicParameters(*ZeroElement);
		std::string ZeroAdmissionError;
		std::shared_ptr<Client::CEffectObject> ZeroObject;
		if (!CEffectDocumentCodec::Validate(ZeroDocument, ZeroAdmissionError) ||
			!Stage_OrdinarySingleVisibleElementPreview(Prototype, ZeroDocument,
				ZeroElement->strElementId, ZeroObject, ZeroAdmissionError))
		{
			OutError = "Synthetic zero-output negative did not stage: " +
				ZeroAdmissionError;
			return false;
		}
		FOCUSED_PARTICLE_SAMPLE ZeroSample;
		if (!Capture_DiagnosticParticleAtTime(
				ZeroObject, *ZeroElement, SampleTime, ZeroSample, OutError))
		{
			return false;
		}
		const float3_t ZeroTarget{ ZeroSample.World._41, ZeroSample.World._42,
			ZeroSample.World._43 };
		FOCUSED_DRAW_EVIDENCE ZeroEvidence;
		for (size_t ProofView = 4u; ProofView < 8u; ++ProofView)
		{
			std::string ZeroRenderError;
			if (Render_WithEvidence(ZeroObject, Scope, ZeroTarget, ProofView, true,
					ZeroEvidence, ZeroRenderError, ZeroElement, &ZeroSample))
			{
				OutError =
					"Shape-fit proof camera approved an identically-zero additive carrier.";
				return false;
			}
			if (!ZeroRenderError.starts_with(
					"Focused draw produced no committed VS/PS-visible pixels."))
			{
				OutError = "Synthetic zero-output negative render failed: " +
					ZeroRenderError;
				return false;
			}
		}
		std::cout << "[FOCUSED ZERO OUTPUT NEGATIVE] rejected=1 proofViews=4 "
			"rtColor(nonzero/sum/max)=(" <<
			ZeroEvidence.iColorNonZeroPixelCount << ',' <<
			ZeroEvidence.iColorByteSum << ',' <<
			static_cast<uint32_t>(ZeroEvidence.iMaximumColorByte) << ")\n";
		return true;
	}

	bool Verify_EarlyOrdinaryPreviewIsolationPreflight(
		const std::filesystem::path& RepositoryRoot, std::string& OutError)
	{
		RESTORATION_RECEIPT Receipt;
		if (!Load_RestorationReceipt(RepositoryRoot, Receipt, OutError))
			return false;
		const FOCUSED_TRACK_A_DRAW_CASE& FocusedCase =
			FOCUSED_TRACK_A_DRAW_CASES.front();
		const RESTORATION_TARGET_RECEIPT* SelectedTarget =
			Find_RestorationTarget(Receipt, FocusedCase.strEffectAssetId);
		const RESTORATION_RECEIPT_ROW* SelectedRow = nullptr;
		if (nullptr != SelectedTarget)
		{
			for (const RESTORATION_RECEIPT_ROW& Row : SelectedTarget->Rows)
			{
				if (Row.eKind != RESTORATION_RECEIPT_ROW_KIND::PARTICLE ||
					Row.strSourceElementId != FocusedCase.strSourceElementId)
				{
					continue;
				}
				if (nullptr != SelectedRow)
				{
					OutError =
						"Early ordinary preview source occurrence is ambiguous: " +
						std::string(FocusedCase.strSourceElementId);
					return false;
				}
				SelectedRow = &Row;
			}
		}
		if (nullptr == SelectedTarget || nullptr == SelectedRow ||
			SelectedRow->strRendererShape != FocusedCase.strRendererShape)
		{
			OutError =
				"Early ordinary preview exact Artist 31000 Sprite is not receipt-admitted.";
			return false;
		}

		std::filesystem::path TargetPath;
		std::string Bytes;
		DATA_JSON_VALUE RawDocument;
		EFFECT_DOCUMENT_DESC Document;
		if (!Resolve_ArtifactPath(RepositoryRoot, SelectedTarget->strTargetPath,
				TargetPath, OutError) ||
			!Read_File(TargetPath, Bytes, OutError) ||
			!Parse_Json(Bytes, RawDocument, OutError) ||
			Canonical_JsonSha256(RawDocument) !=
				SelectedTarget->strOutputCanonicalSha256 ||
			!Validate_RestorationTargetProjection(
				*SelectedTarget, RawDocument, OutError) ||
			!CEffectDocumentCodec::Parse(Bytes, Document, OutError))
		{
			const std::string Detail = OutError.empty() ?
				"target hash/projection/codec changed" : OutError;
			OutError = "Early ordinary preview target load failed: " +
				SelectedTarget->strEffectAssetId + "/" +
				SelectedRow->strTargetElementId + ": " + Detail;
			return false;
		}
		const EFFECT_ELEMENT_DESC* Element = Find_ExactElement(
			Document, SelectedRow->strTargetElementId);
		RESTORATION_RUNTIME_DISPOSITION SelectedDisposition{};
		if (nullptr == Element ||
			!Classify_RestorationRuntimeDisposition(*SelectedRow, *Element,
				Document.strEffectAssetId, SelectedDisposition, OutError) ||
			SelectedDisposition != RESTORATION_RUNTIME_DISPOSITION::FULL ||
			Extract_SourceElementId(Element->strSourceNode) !=
				SelectedRow->strSourceElementId)
		{
			OutError = "Early ordinary preview receipt/runtime join changed: " +
				SelectedTarget->strEffectAssetId + "/" +
				SelectedRow->strTargetElementId;
			return false;
		}
		const RESTORATION_RECEIPT_ROW* BlockedRow = nullptr;
		const EFFECT_ELEMENT_DESC* BlockedElement = nullptr;
		for (const RESTORATION_RECEIPT_ROW& Row : SelectedTarget->Rows)
		{
			if (Row.eKind != RESTORATION_RECEIPT_ROW_KIND::PARTICLE ||
				Row.FailClosedReasons.empty())
			{
				continue;
			}
			const EFFECT_ELEMENT_DESC* Candidate = Find_ExactElement(
				Document, Row.strTargetElementId);
			RESTORATION_RUNTIME_DISPOSITION Disposition{};
			if (nullptr == Candidate ||
				!Classify_RestorationRuntimeDisposition(Row, *Candidate,
					Document.strEffectAssetId, Disposition, OutError))
			{
				return false;
			}
			if (Disposition ==
				RESTORATION_RUNTIME_DISPOSITION::HARD_FAIL_CLOSED)
			{
				BlockedRow = &Row;
				BlockedElement = Candidate;
				break;
			}
		}
		if (nullptr == BlockedRow || nullptr == BlockedElement ||
			BlockedElement->bVisible ||
			!BlockedElement->Material.Execution.bFailClosed ||
			BlockedElement->Material.Execution.bAuthoringApproximate)
		{
			OutError =
				"Early ordinary preview lacks an exact durable blocked occurrence.";
			return false;
		}
		EFFECT_DOCUMENT_DESC Reactivation = Document;
		EFFECT_ELEMENT_DESC* Reactivated = Find_ExactElement(
			Reactivation, BlockedRow->strTargetElementId);
		if (nullptr == Reactivated)
		{
			OutError = "Early blocked reactivation mutation lost its Element.";
			return false;
		}
		Reactivated->bVisible = true;
		std::string ReactivationRejection;
		if (CEffectDocumentCodec::Validate(
				Reactivation, ReactivationRejection) ||
			ReactivationRejection.find(
				"Hard fail-closed authored Element cannot be made visible") ==
					std::string::npos)
		{
			OutError =
				"Early blocked occurrence reactivation bypassed codec admission.";
			return false;
		}
		if (Is_FocusedDrawOnlyRequested())
		{
			std::cout << "[FOCUSED BLOCKED REACTIVATION] rejected=1 element=" <<
				BlockedRow->strTargetElementId << " reason=" <<
				ReactivationRejection << '\n';
		}

		SCOPED_CURRENT_DIRECTORY CurrentDirectory;
		FOCUSED_HEADLESS_RENDER_SCOPE Scope;
		if (!CurrentDirectory.Initialize(
				RepositoryRoot / L"Client" / L"Default", OutError) ||
			!Scope.Initialize(OutError))
		{
			return false;
		}
		std::unique_ptr<Client::CEffectObject> Prototype =
			Client::CEffectObject::Create(Scope.Get_Device(), Scope.Get_Context());
		std::shared_ptr<Client::CEffectObject> Object;
		if (nullptr == Prototype ||
			!Verify_AuthoringApproximateCodecMutationFixture(Prototype, Document,
				Element->strElementId, BlockedElement->strElementId, OutError) ||
			!Stage_OrdinarySingleVisibleElementPreview(Prototype, Document,
				Element->strElementId, Object, OutError))
		{
			if (OutError.empty())
				OutError = "Early ordinary preview object stage failed.";
			return false;
		}
		std::string IsolationRejection;
		if (Object->Set_TestPreviewElementIsolation(
				{ Element->strElementId }, IsolationRejection) ||
			IsolationRejection.find("admitted source visual program") ==
				std::string::npos)
		{
			OutError =
				"Ordinary Stage unexpectedly acquired visual-program isolation authority: " +
				SelectedTarget->strEffectAssetId + "/" + Element->strElementId;
			return false;
		}
		if (Is_FocusedDrawOnlyRequested())
		{
			std::vector<FOCUSED_PARTICLE_SAMPLE> DiagnosticCandidates;
			FOCUSED_PARTICLE_SAMPLE DiagnosticFirstActive;
			if (!Collect_ActiveParticleLifetimeSamples(Object, *Element,
					DiagnosticCandidates, OutError, &DiagnosticFirstActive) ||
				DiagnosticCandidates.empty() ||
				!Run_FocusedMaterialDiagnosticLadder(Prototype, Scope, Document,
					*Element, DiagnosticCandidates.front().fSampleTime, OutError))
			{
				OutError = SelectedTarget->strEffectAssetId + "/" +
					Element->strElementId + ": " + OutError;
				return false;
			}
		}
		FOCUSED_PARTICLE_SAMPLE Sample;
		FOCUSED_PARTICLE_SAMPLE FirstActiveSample;
		FOCUSED_DRAW_EVIDENCE Evidence;
		if (!Find_VisiblePixelParticleAcrossLifetime(Object, *Element, Scope,
				Sample, Evidence, OutError, &FirstActiveSample))
		{
			OutError = SelectedTarget->strEffectAssetId + "/" +
				Element->strElementId + ": " + OutError;
			return false;
		}
		const bool_t bInitialFadeInZero =
			FirstActiveSample.Color.w <= 1.e-4f &&
			FirstActiveSample.Probe.fMaxAlpha <= 1.e-4f;
		if (bInitialFadeInZero &&
			Sample.fSampleTime <= FirstActiveSample.fSampleTime)
		{
			OutError =
				"Early ordinary preview zero-alpha fade-in was not advanced to a "
				"later visible witness.";
			return false;
		}
		if (!Validate_FocusedOccurrenceSubmission(
				*Object, *Element, Sample, Evidence, nullptr, OutError))
		{
			OutError = SelectedTarget->strEffectAssetId + "/" +
				Element->strElementId + ": " + OutError + " " +
				Evidence.strDiagnostics;
			return false;
		}
		if (Is_FocusedDrawOnlyRequested())
		{
			std::cout << "[FOCUSED DRAW DIAGNOSTIC] " <<
				SelectedTarget->strEffectAssetId << '/' << Element->strElementId <<
				" initialActive(time/alpha/probeMax)=(" <<
				FirstActiveSample.fSampleTime << '/' << FirstActiveSample.Color.w <<
				'/' << FirstActiveSample.Probe.fMaxAlpha << ") " <<
				Evidence.strDiagnostics << '\n';
		}
		return true;
	}

	bool Validate_FocusedOccurrenceSubmission(
		const Client::CEffectObject& Object,
		const EFFECT_ELEMENT_DESC& Element,
		const FOCUSED_PARTICLE_SAMPLE& Sample,
		const FOCUSED_DRAW_EVIDENCE& Evidence,
		const DECODED_MODEL_METRICS* ModelMetrics,
		std::string& OutError)
	{
		const auto& Submission = Object.Get_LastRenderSubmissionStats();
		const auto Row = std::find_if(Submission.Occurrences.begin(),
			Submission.Occurrences.end(), [&Element](const auto& Candidate)
			{
				return Candidate.strElementId == Element.strElementId;
			});
		if (Row == Submission.Occurrences.end())
		{
			OutError = "Focused draw occurrence stats are missing: " +
				Element.strElementId;
			return false;
		}
		const bool_t bMesh = Element.SourceRecipe.strRendererShape == "mesh";
		const Client::EFFECT_GPU_RENDER_FAMILY ExpectedFamily = bMesh ?
			Client::EFFECT_GPU_RENDER_FAMILY::MESH :
			Client::EFFECT_GPU_RENDER_FAMILY::SPRITE;
		const Client::EFFECT_GPU_RENDER_CARRIER ExpectedCarrier = bMesh ?
			Client::EFFECT_GPU_RENDER_CARRIER::MESH_CMODEL :
			Client::EFFECT_GPU_RENDER_CARRIER::SPRITE_INSTANCE;
		const uint32_t ExpectedPass = Expected_EffectPass(
			Element, Matrix_Determinant3x3(Sample.World));
		const uint64_t ExpectedMaterialBinds = bMesh ?
			Row->iCandidateRowCount : 1u;
		const uint64_t ExpectedDraws = bMesh && nullptr != ModelMetrics ?
			Row->iCandidateRowCount * ModelMetrics->iMeshCount : 1u;
		const bool_t bStatsExact = Submission.bCompleted && Submission.bCommitted &&
			Row->eFamily == ExpectedFamily && Row->iConfigured == 1u &&
			Row->iEvaluated == 1u && Row->iActive == 1u &&
			Row->iCandidateRowCount == Sample.Probe.iActiveParticleCount &&
			Row->iCandidateRowCount > 0u && Row->iAttempted == 1u &&
			Row->iSubmitted == 1u && Row->iSuppressed == 0u &&
			Row->iFailed == 0u &&
			Row->iMaterialBindCount == ExpectedMaterialBinds &&
			Row->iTextureSrvBindCount == 12u * Row->iMaterialBindCount &&
			Row->iShaderPassApplyCount == ExpectedDraws &&
			Row->iVIBufferBindCount == ExpectedDraws &&
			Row->iVIBufferDrawCount == ExpectedDraws &&
			Row->iIssuedDrawCallCount == ExpectedDraws &&
			Row->iDrawSelectionCount == ExpectedDraws &&
			Row->iSamplerBindCount >= Row->iShaderPassApplyCount &&
			Row->eCarrier == ExpectedCarrier &&
			Row->iSelectedPassIndex == ExpectedPass &&
			!Row->bDrawSelectionDiverged && Row->bHasSubmittedPosition &&
			Evidence.Pipeline.IAPrimitives > 0u &&
			Evidence.Pipeline.VSInvocations > 0u &&
			Evidence.Pipeline.PSInvocations > 0u;
		if (!bStatsExact)
		{
			std::ostringstream Failure;
			Failure << "Focused draw submission mismatch: " <<
				Element.strElementId << " active=" << Row->iActive <<
				" candidates=" << Row->iCandidateRowCount <<
				" attempted=" << Row->iAttempted <<
				" submitted=" << Row->iSubmitted <<
				" binds=" << Row->iMaterialBindCount <<
				" srvs=" << Row->iTextureSrvBindCount <<
				" draws=" << Row->iIssuedDrawCallCount <<
				" pass=" << Row->iSelectedPassIndex << "/" << ExpectedPass <<
				" ia=" << Evidence.Pipeline.IAPrimitives <<
				" vs=" << Evidence.Pipeline.VSInvocations <<
				" ps=" << Evidence.Pipeline.PSInvocations;
			OutError = Failure.str();
			return false;
		}
		if (bMesh)
		{
			if (nullptr == ModelMetrics ||
				Evidence.Pipeline.IAPrimitives !=
					ModelMetrics->iIndexCount / 3u *
					Row->iCandidateRowCount)
			{
				OutError = "Focused Mesh draw did not consume every WModel triangle: " +
					Element.strElementId;
				return false;
			}
			const float3_t WorldScale = Matrix_AbsoluteScale(Sample.World);
			const f32_t EffectiveRadius = ModelMetrics->fRadius *
				Element.Detail.Mesh.fModelPreScale *
				(std::max)({ WorldScale.x, WorldScale.y, WorldScale.z });
			if (!std::isfinite(EffectiveRadius) || EffectiveRadius <= 0.0001f ||
				EffectiveRadius > 250.f)
			{
				OutError = "Focused Mesh evaluated bounds are degenerate or explosive: " +
					Element.strElementId;
				return false;
			}
		}
		return true;
	}

	bool Validate_FocusedDecalSubmission(
		const Client::CEffectObject& Object,
		const EFFECT_ELEMENT_DESC& Element,
		const FOCUSED_GPU_OCCURRENCE_SAMPLE& Sample,
		const FOCUSED_DRAW_EVIDENCE& Evidence,
		std::string& OutError)
	{
		const auto& Submission = Object.Get_LastRenderSubmissionStats();
		const auto Row = std::find_if(Submission.Occurrences.begin(),
			Submission.Occurrences.end(), [&Element](const auto& Candidate)
			{
				return Candidate.strElementId == Element.strElementId;
			});
		const uint32_t ExpectedPass = Expected_EffectPass(
			Element, Matrix_Determinant3x3(Sample.World));
		const bool_t bExact = Row != Submission.Occurrences.end() &&
			Submission.bCompleted && Submission.bCommitted &&
			Row->eFamily == Client::EFFECT_GPU_RENDER_FAMILY::DECAL &&
			Row->eCarrier == Client::EFFECT_GPU_RENDER_CARRIER::DECAL_RECT &&
			Row->iConfigured == 1u && Row->iEvaluated == 1u &&
			Row->iActive == 1u &&
			Row->iCandidateRowCount == Sample.iCandidateRowCount &&
			Row->iCandidateRowCount > 0u && Row->iAttempted == 1u &&
			Row->iSubmitted == 1u && Row->iSuppressed == 0u &&
			Row->iFailed == 0u && Row->iMaterialBindCount == 1u &&
			Row->iTextureSrvBindCount == 12u &&
			Row->iShaderPassApplyCount == 1u &&
			Row->iVIBufferBindCount == 1u && Row->iVIBufferDrawCount == 1u &&
			Row->iIssuedDrawCallCount == 1u &&
			Row->iDrawSelectionCount == 1u &&
			Row->iSelectedPassIndex == ExpectedPass &&
			!Row->bDrawSelectionDiverged && Row->bHasSubmittedPosition &&
			Evidence.Pipeline.IAPrimitives > 0u &&
			Evidence.Pipeline.VSInvocations > 0u &&
			Evidence.Pipeline.PSInvocations > 0u;
		if (!bExact)
		{
			std::ostringstream Failure;
			Failure << "Focused Decal draw submission mismatch: " <<
				Element.strElementId;
			if (Row != Submission.Occurrences.end())
			{
				Failure << " active=" << Row->iActive <<
					" candidates=" << Row->iCandidateRowCount <<
					" attempted=" << Row->iAttempted <<
					" submitted=" << Row->iSubmitted <<
					" binds=" << Row->iMaterialBindCount <<
					" srvs=" << Row->iTextureSrvBindCount <<
					" draws=" << Row->iIssuedDrawCallCount <<
					" pass=" << Row->iSelectedPassIndex << "/" << ExpectedPass;
			}
			Failure << " ia=" << Evidence.Pipeline.IAPrimitives <<
				" vs=" << Evidence.Pipeline.VSInvocations <<
				" ps=" << Evidence.Pipeline.PSInvocations;
			OutError = Failure.str();
			return false;
		}
		return true;
	}

	bool Verify_DimensionMasterTModelCueDraw(
		const EFFECT_DOCUMENT_DESC& SourceDocument,
		const std::filesystem::path& RepositoryRoot,
		const std::filesystem::path& RuntimeResourceRoot,
		const std::unique_ptr<Client::CEffectObject>& Prototype,
		const FOCUSED_HEADLESS_RENDER_SCOPE& Scope,
		std::string& OutError)
	{
		const std::string Canonical =
			CEffectDocumentCodec::Serialize(SourceDocument);
		EFFECT_DOCUMENT_DESC RoundTrip;
		if (!CEffectDocumentCodec::Parse(Canonical, RoundTrip, OutError) ||
			CEffectDocumentCodec::Serialize(RoundTrip) != Canonical ||
			RoundTrip.ModelCues.size() != 1u ||
			RoundTrip.ModelCues.front().eAlphaMode !=
				Client::EFFECT_MODEL_CUE_ALPHA_MODE::MASKED_SURFACE ||
			Canonical.find("\"alphaMode\"") == std::string::npos)
		{
			if (OutError.empty())
				OutError = "DimensionMaster T ModelCue MASKED roundtrip changed.";
			return false;
		}
		const std::filesystem::path ShaderPath = RepositoryRoot / L"Client" /
			L"Bin" / L"ShaderFiles" / L"Shader_VtxAnimMeshBinary.hlsl";
		std::string ShaderSource;
		if (!Read_File(ShaderPath, ShaderSource, OutError))
			return false;
		const size_t iOpaquePass =
			ShaderSource.find("pass EffectModelCueOpaque");
		if (ShaderSource.find(
				"Evaluate_Material(input, false, 0.f)") == std::string::npos ||
			iOpaquePass == std::string::npos ||
			ShaderSource.find("SetRasterizerState(RS_Default)",
				iOpaquePass) == std::string::npos ||
			ShaderSource.find("PS_MAIN_OPAQUE", iOpaquePass) ==
				std::string::npos ||
			ShaderSource.find("if (!alphaClip)") == std::string::npos ||
			ShaderSource.find("diffuse.a = 1.f;",
				ShaderSource.find("if (!alphaClip)")) == std::string::npos ||
			ShaderSource.find("pass EffectModelCueTranslucent") ==
				std::string::npos)
		{
			if (OutError.empty())
				OutError =
					"DimensionMaster T OPAQUE character-surface shader contract is missing.";
			return false;
		}
		const std::filesystem::path RendererPath = RepositoryRoot / L"Client" /
			L"Private" / L"Effect_DocumentRenderer.cpp";
		std::string RendererSource;
		if (!Read_File(RendererPath, RendererSource, OutError) ||
			RendererSource.find("EFFECT_MODEL_CUE_ALPHA_MODE::OPAQUE_SURFACE ?") ==
				std::string::npos ||
			RendererSource.find("2u : 0u") == std::string::npos)
		{
			if (OutError.empty())
				OutError =
					"DimensionMaster T typed OPAQUE pass selection is missing.";
			return false;
		}

		const auto& Cue = RoundTrip.ModelCues.front();
		DECODED_MODEL_METRICS ModelMetrics;
		if (!Decode_ModelMetrics(RuntimeResourceRoot, Cue.strModelAssetId,
				ModelMetrics, OutError) || ModelMetrics.iMeshCount != 4u ||
			ModelMetrics.iVertexCount != 13806u)
		{
			if (OutError.empty())
				OutError = "DimensionMaster T ModelCue geometry denominator changed.";
			return false;
		}

		EFFECT_DOCUMENT_DESC OpaqueDocument = RoundTrip;
		OpaqueDocument.Elements.clear();
		OpaqueDocument.ModelCues.front().eAlphaMode =
			Client::EFFECT_MODEL_CUE_ALPHA_MODE::OPAQUE_SURFACE;
		std::shared_ptr<Client::CEffectObject> OpaqueObject =
			std::dynamic_pointer_cast<Client::CEffectObject>(Prototype->Clone(nullptr));
		if (nullptr == OpaqueObject ||
			!OpaqueObject->Stage_Document(OpaqueDocument, OutError))
		{
			if (OutError.empty())
				OutError = "DimensionMaster T OPAQUE ModelCue stage failed.";
			return false;
		}
		OpaqueObject->Set_SampleTime(0.25f);
		FOCUSED_DRAW_EVIDENCE OpaqueEvidence;
		const float3_t Target = Cue.LocalTransform.vPosition;
		if (!Render_WithEvidence(OpaqueObject, Scope, Target, std::nullopt,
				true, OpaqueEvidence, OutError))
		{
			OutError = "DimensionMaster T OPAQUE ModelCue draw failed: " +
				OutError;
			return false;
		}
		const uint64_t ExpectedPrimitives = ModelMetrics.iIndexCount / 3u;
		if (OpaqueEvidence.Pipeline.IAPrimitives != ExpectedPrimitives ||
			nullptr == OpaqueEvidence.PixelShader ||
			0u == OpaqueEvidence.iAlphaNonZeroPixelCount ||
			OpaqueEvidence.iAlphaOnePixelCount !=
				OpaqueEvidence.iAlphaNonZeroPixelCount)
		{
			OutError =
				"DimensionMaster T OPAQUE pass did not draw all four sections.";
			return false;
		}

		EFFECT_DOCUMENT_DESC MaskedDocument = OpaqueDocument;
		MaskedDocument.ModelCues.front().eAlphaMode =
			Client::EFFECT_MODEL_CUE_ALPHA_MODE::MASKED_SURFACE;
		const std::string MaskedCanonical =
			CEffectDocumentCodec::Serialize(MaskedDocument);
		EFFECT_DOCUMENT_DESC MaskedRoundTrip;
		if (!CEffectDocumentCodec::Parse(
				MaskedCanonical, MaskedRoundTrip, OutError) ||
			MaskedRoundTrip.ModelCues.front().eAlphaMode !=
				Client::EFFECT_MODEL_CUE_ALPHA_MODE::MASKED_SURFACE)
		{
			OutError = "DimensionMaster T MASKED contrast roundtrip failed: " +
				OutError;
			return false;
		}
		std::shared_ptr<Client::CEffectObject> MaskedObject =
			std::dynamic_pointer_cast<Client::CEffectObject>(Prototype->Clone(nullptr));
		if (nullptr == MaskedObject ||
			!MaskedObject->Stage_Document(MaskedRoundTrip, OutError))
		{
			if (OutError.empty())
				OutError = "DimensionMaster T MASKED contrast stage failed.";
			return false;
		}
		MaskedObject->Set_SampleTime(0.25f);
		FOCUSED_DRAW_EVIDENCE MaskedEvidence;
		if (!Render_WithEvidence(MaskedObject, Scope, Target,
				OpaqueEvidence.iViewIndex, false, MaskedEvidence, OutError,
				nullptr, nullptr, nullptr, true))
		{
			OutError = "DimensionMaster T MASKED contrast draw failed: " + OutError;
			return false;
		}
		if (!Read_BackbufferAlpha(Scope.Get_Device(), Scope.Get_Context(),
				MaskedEvidence.iAlphaNonZeroPixelCount,
				MaskedEvidence.iAlphaOnePixelCount, OutError))
		{
			return false;
		}
		if (MaskedEvidence.Pipeline.IAPrimitives != ExpectedPrimitives ||
			nullptr == MaskedEvidence.PixelShader ||
			MaskedEvidence.PixelShader.Get() == OpaqueEvidence.PixelShader.Get() ||
			MaskedEvidence.iAlphaNonZeroPixelCount >=
				OpaqueEvidence.iAlphaNonZeroPixelCount ||
			MaskedEvidence.iAlphaOnePixelCount >
				MaskedEvidence.iAlphaNonZeroPixelCount)
		{
			OutError =
				"DimensionMaster T OPAQUE/MASKED alpha coverage contrast changed.";
			return false;
		}
		return true;
	}

	bool Verify_FocusedTrackAActualDrawGate(
		const std::vector<STAGED_TARGET_FILE>& Files,
		const RESTORATION_RECEIPT& Receipt,
		const std::filesystem::path& RepositoryRoot,
		std::string& OutError)
	{
		if (Files.size() != Receipt.iTargetCount ||
			Receipt.Targets.size() != Receipt.iTargetCount)
		{
			OutError = "Focused draw gate document/receipt denominator changed.";
			return false;
		}
		const std::filesystem::path RuntimeResourceRoot = RepositoryRoot /
			L"Client" / L"Bin" / L"Resources";
		SCOPED_CURRENT_DIRECTORY CurrentDirectory;
		if (!CurrentDirectory.Initialize(
				RepositoryRoot / L"Client" / L"Default", OutError))
		{
			return false;
		}

		std::map<std::string, EFFECT_DOCUMENT_DESC, std::less<>> Documents;
		for (const STAGED_TARGET_FILE& File : Files)
		{
			EFFECT_DOCUMENT_DESC Document;
			if (!CEffectDocumentCodec::Parse(
					File.strCanonicalDocument, Document, OutError) ||
				!Documents.emplace(Document.strEffectAssetId,
					std::move(Document)).second)
			{
				if (OutError.empty())
					OutError = "Focused draw candidate identity is duplicate.";
				return false;
			}
		}
		/* The full-source path is validation-only.  Source/target recipe, Detail,
		   resource, and event lineage were already joined above from the receipt's
		   hash-pinned raw JSON projections.  Do not re-run the old C++ materializer
		   against an entire imported document: one unrelated non-admitted source
		   row must not prevent an exact admitted occurrence from being audited. */

		const RESTORATION_TARGET_RECEIPT* DmReceipt = Find_RestorationTarget(
			Receipt, "effect.dimensionmaster.skill.2050210.unified");
		if (nullptr == DmReceipt)
		{
			OutError = "DimensionMaster 2050210 receipt target is missing.";
			return false;
		}

		std::map<std::string, DECODED_MODEL_METRICS, std::less<>> ModelMetrics;
		const auto GetModelMetrics = [&](const std::string_view AssetId)
			-> const DECODED_MODEL_METRICS*
		{
			const auto Cached = ModelMetrics.find(AssetId);
			if (Cached != ModelMetrics.end())
				return &Cached->second;
			DECODED_MODEL_METRICS Metrics;
			if (!Decode_ModelMetrics(
					RuntimeResourceRoot, AssetId, Metrics, OutError))
			{
				return nullptr;
			}
			return &ModelMetrics.emplace(
				std::string(AssetId), Metrics).first->second;
		};

		std::vector<std::pair<const FOCUSED_TRACK_A_DRAW_CASE*,
			const EFFECT_ELEMENT_DESC*>> ResolvedCases;
		ResolvedCases.reserve(FOCUSED_TRACK_A_DRAW_CASES.size());
		std::size_t iFocusedBlocked = 0u;
		for (const FOCUSED_TRACK_A_DRAW_CASE& Case : FOCUSED_TRACK_A_DRAW_CASES)
		{
			const auto Document = Documents.find(Case.strEffectAssetId);
			const RESTORATION_TARGET_RECEIPT* TargetReceipt =
				Find_RestorationTarget(Receipt, Case.strEffectAssetId);
			if (Document == Documents.end() || nullptr == TargetReceipt)
			{
				OutError = "Focused draw candidate document/receipt is missing: " +
					std::string(Case.strEffectAssetId);
				return false;
			}
			const EFFECT_ELEMENT_DESC* Element = Case.strElementId.empty() ? nullptr :
				Find_ExactElement(Document->second, Case.strElementId);
			if (Case.strElementId.empty())
			{
				for (const EFFECT_ELEMENT_DESC& Candidate : Document->second.Elements)
				{
					if (Extract_SourceElementId(Candidate.strSourceNode) !=
						Case.strSourceElementId)
					{
						continue;
					}
					if (nullptr != Element)
					{
						OutError = "Focused draw compiler source identity is ambiguous: " +
							std::string(Case.strSourceElementId);
						return false;
					}
					Element = &Candidate;
				}
			}
			const RESTORATION_RECEIPT_ROW* ReceiptRow = nullptr == Element ?
				nullptr : Find_RestorationRow(*TargetReceipt,
					Element->strElementId);
			if (nullptr == Element)
			{
				OutError = "Focused draw source occurrence is missing: " +
					std::string(Case.strEffectAssetId) + "/" +
					std::string(Case.strSourceElementId);
				return false;
			}
			if (nullptr == ReceiptRow)
			{
				OutError = "Focused draw receipt row is missing: " +
					Element->strElementId;
				return false;
			}
			if (ReceiptRow->eKind != RESTORATION_RECEIPT_ROW_KIND::PARTICLE ||
				ReceiptRow->strSourceElementId != Case.strSourceElementId)
			{
				OutError = "Focused draw receipt lineage changed: " +
					Element->strElementId;
				return false;
			}
			if (Element->SourceRecipe.strRendererShape != Case.strRendererShape)
			{
				OutError = "Focused draw renderer shape changed: " +
					Element->strElementId;
				return false;
			}
			RESTORATION_RUNTIME_DISPOSITION Disposition{};
			if (!Classify_RestorationRuntimeDisposition(*ReceiptRow, *Element,
					Document->second.strEffectAssetId, Disposition, OutError))
			{
				return false;
			}
			if (Disposition ==
				RESTORATION_RUNTIME_DISPOSITION::HARD_FAIL_CLOSED)
			{
				if (Element->bVisible ||
					!Element->Material.Execution.bFailClosed ||
					Element->Material.Execution.bAuthoringApproximate)
				{
					OutError =
						"Focused blocked occurrence is not durable hidden fail-closed: " +
						Element->strElementId;
					return false;
				}
				EFFECT_DOCUMENT_DESC Mutation = Document->second;
				EFFECT_ELEMENT_DESC* Activated = Find_ExactElement(
					Mutation, Element->strElementId);
				std::string Rejection;
				if (nullptr == Activated)
				{
					OutError = "Focused blocked occurrence mutation join is missing: " +
						Element->strElementId;
					return false;
				}
				Activated->bVisible = true;
				if (CEffectDocumentCodec::Validate(Mutation, Rejection) ||
					Rejection.find(
						"Hard fail-closed authored Element cannot be made visible") ==
							std::string::npos)
				{
					OutError = "Focused blocked occurrence reactivation was admitted: " +
						Element->strElementId;
					return false;
				}
				++iFocusedBlocked;
				continue;
			}
			if (!Is_RestorationPreviewExecutionTarget(*Element))
			{
				OutError = "Focused full/approximate occurrence is not previewable: " +
					Element->strElementId;
				return false;
			}
			if (!Validate_SourceTextureContract(*Element, RuntimeResourceRoot))
			{
				OutError = "Focused draw source texture contract changed: " +
					Element->strElementId;
				return false;
			}
			const auto* Model = Find_ResourceBinding(*Element, "meshModel");
			const auto* Base = Find_ResourceBinding(*Element, "base");
			if ((!Case.strShapeAssetId.empty() &&
				(nullptr == Model || Model->strAssetId != Case.strShapeAssetId ||
				 !Nearly_Equal(Element->Detail.Mesh.fModelPreScale,
					0.01f, 0.000001f, 0.000001f) ||
				 nullptr == GetModelMetrics(Model->strAssetId))) ||
				(!Case.strBaseAssetId.empty() &&
				 (nullptr == Base || Base->strAssetId != Case.strBaseAssetId)))
			{
				OutError = "Focused draw exact WModel/Base identity changed: " +
					Element->strElementId;
				return false;
			}
			for (const std::string_view AssetId :
				{ Case.strShapeAssetId, Case.strBaseAssetId })
			{
				if (AssetId.empty()) continue;
				std::error_code Error;
				if (!std::filesystem::is_regular_file(
						RuntimeResourceRoot / AssetId, Error) || Error)
				{
					OutError = "Focused draw physical resource is missing: " +
						std::string(AssetId);
					return false;
				}
			}
			ResolvedCases.emplace_back(&Case, Element);
		}
		if (ResolvedCases.size() + iFocusedBlocked !=
				FOCUSED_TRACK_A_DRAW_CASES.size())
		{
			OutError = "Focused receipt-driven draw/blocked partition changed.";
			return false;
		}

		using RESTORATION_DRAW_REPRESENTATIVE = std::pair<
			const EFFECT_DOCUMENT_DESC*, const EFFECT_ELEMENT_DESC*>;
		std::array<std::optional<RESTORATION_DRAW_REPRESENTATIVE>, 2u>
			FullShapeRepresentatives;
		std::array<std::optional<RESTORATION_DRAW_REPRESENTATIVE>, 2u>
			ApproximateShapeRepresentatives;
		std::array<std::size_t, 2u> FullShapeCounts{};
		std::array<std::size_t, 2u> ApproximateShapeCounts{};
		std::size_t iCorpusFullParticleCount = 0u;
		std::size_t iCorpusApproximateParticleCount = 0u;
		std::size_t iCorpusPortableHardParticleCount = 0u;
		std::size_t iCorpusDeferredHardParticleCount = 0u;
		for (const auto& [EffectId, CorpusDocument] : Documents)
		{
			const RESTORATION_TARGET_RECEIPT* TargetReceipt =
				Find_RestorationTarget(Receipt, EffectId);
			std::string ValidationError;
			if (nullptr == TargetReceipt ||
				!CEffectDocumentCodec::Validate(CorpusDocument, ValidationError))
			{
				OutError = "Focused corpus codec validation failed: " + EffectId +
					": " + ValidationError;
				return false;
			}
			for (const RESTORATION_RECEIPT_ROW& Row : TargetReceipt->Rows)
			{
				if (Row.eKind != RESTORATION_RECEIPT_ROW_KIND::PARTICLE)
					continue;
				const EFFECT_ELEMENT_DESC* Element = Find_ExactElement(
					CorpusDocument, Row.strTargetElementId);
				RESTORATION_RUNTIME_DISPOSITION Disposition{};
				if (nullptr == Element ||
					!Classify_RestorationRuntimeDisposition(Row, *Element,
						EffectId, Disposition, OutError))
				{
					return false;
				}
				if (Disposition == RESTORATION_RUNTIME_DISPOSITION::HARD_FAIL_CLOSED)
				{
					if (Row.bPortable)
						++iCorpusPortableHardParticleCount;
					else
						++iCorpusDeferredHardParticleCount;
					continue;
				}
				const size_t ShapeIndex =
					Element->SourceRecipe.strRendererShape == "mesh" ? 0u : 1u;
				if (Disposition == RESTORATION_RUNTIME_DISPOSITION::FULL)
				{
					++iCorpusFullParticleCount;
					++FullShapeCounts[ShapeIndex];
					if (!FullShapeRepresentatives[ShapeIndex].has_value())
					{
						FullShapeRepresentatives[ShapeIndex] =
							RESTORATION_DRAW_REPRESENTATIVE{
								&CorpusDocument, Element };
					}
				}
				else
				{
					++iCorpusApproximateParticleCount;
					++ApproximateShapeCounts[ShapeIndex];
					if (!ApproximateShapeRepresentatives[ShapeIndex].has_value())
					{
						ApproximateShapeRepresentatives[ShapeIndex] =
							RESTORATION_DRAW_REPRESENTATIVE{
								&CorpusDocument, Element };
					}
				}
			}
		}
		if (iCorpusFullParticleCount != Receipt.iDrawableAdmittedCount ||
			iCorpusApproximateParticleCount +
				iCorpusPortableHardParticleCount !=
				Receipt.iPortableFailClosedCount ||
			iCorpusDeferredHardParticleCount != Receipt.iRecipeDeferredCount ||
			iCorpusFullParticleCount + iCorpusApproximateParticleCount +
				iCorpusPortableHardParticleCount +
				iCorpusDeferredHardParticleCount !=
				Receipt.iStrictMappedParticleCount)
		{
			std::ostringstream Failure;
			Failure << "Focused three-way receipt/data denominator changed"
				<< ": full=" << iCorpusFullParticleCount << "/" <<
					Receipt.iDrawableAdmittedCount
				<< " approximate=" << iCorpusApproximateParticleCount
				<< " portable-hard=" << iCorpusPortableHardParticleCount
				<< " portable-fail-closed=" <<
					Receipt.iPortableFailClosedCount
				<< " deferred-hard=" << iCorpusDeferredHardParticleCount << "/" <<
					Receipt.iRecipeDeferredCount;
			OutError = Failure.str();
			return false;
		}

		/* Run the bounded codec/Stage/runtime semantics fixture before any corpus
		   draw work.  Carrier choice follows the current receipt-admitted focused
		   partition rather than a fixed vector position. */
		FOCUSED_HEADLESS_RENDER_SCOPE Scope;
		if (!Scope.Initialize(OutError))
			return false;
		std::unique_ptr<Client::CEffectObject> Prototype =
			Client::CEffectObject::Create(Scope.Get_Device(), Scope.Get_Context());
		if (nullptr == Prototype)
		{
			OutError = "Focused draw CEffectObject prototype creation failed.";
			return false;
		}
		const EFFECT_ELEMENT_DESC* SemanticsCarrier = nullptr;
		std::string CarrierError;
		for (const auto& [FocusedCase, Element] : ResolvedCases)
		{
			if (nullptr == FocusedCase || nullptr == Element ||
				Element->SourceRecipe.strRendererShape != "mesh")
			{
				continue;
			}
			EFFECT_DOCUMENT_DESC Preflight;
			std::string CandidateError;
			if (Build_SpawnRateFixture(*Element, Preflight, CandidateError))
			{
				SemanticsCarrier = Element;
				break;
			}
			CarrierError = std::move(CandidateError);
		}
		if (nullptr == SemanticsCarrier ||
			!Verify_RateScaleAndSizeScaleConsumption(
				Prototype, *SemanticsCarrier, OutError))
		{
			if (nullptr == SemanticsCarrier)
			{
				OutError = "Focused receipt-admitted Mesh semantics carrier is missing";
				if (!CarrierError.empty())
					OutError += ": " + CarrierError;
			}
			return false;
		}
		const auto Warlord17090 = Documents.find(
			"effect.warlord.skill.17090.unified");
		const auto IsWarlordChainModel = [](const auto& Element,
			const std::string_view strModelAssetId)
		{
			if (Element.Material.strSourceMaterialPath !=
				"fx_m_mi_d_00.fx_mi.fx_d_me_chain_01_101_ma")
			{
				return false;
			}
			return std::ranges::count_if(Element.ResourceBindings,
				[strModelAssetId](const auto& Binding)
				{
					return Binding.strSlotId == "meshModel" &&
						Binding.strAssetId == strModelAssetId;
				}) == 1;
		};
		if (Warlord17090 == Documents.end() ||
			Warlord17090->second.Elements.size() != 16u ||
			std::count_if(Warlord17090->second.Elements.begin(),
				Warlord17090->second.Elements.end(), [](const auto& Element)
				{
					return Element.SourceRecipe.strRendererShape == "mesh";
				}) != 14 ||
			std::count_if(Warlord17090->second.Elements.begin(),
				Warlord17090->second.Elements.end(), [](const auto& Element)
				{
					return Element.SourceRecipe.strRendererShape == "sprite";
				}) != 2 ||
			std::count_if(Warlord17090->second.Elements.begin(),
				Warlord17090->second.Elements.end(),
				[&IsWarlordChainModel](const auto& Element)
				{
					return IsWarlordChainModel(Element,
						"Effect/Warlord/Meshes/FX_SM_01/fm_d_berchain_06.wmodel");
				}) != 8 ||
			std::count_if(Warlord17090->second.Elements.begin(),
				Warlord17090->second.Elements.end(),
				[&IsWarlordChainModel](const auto& Element)
				{
					return IsWarlordChainModel(Element,
						"Effect/Warlord/Meshes/FX_SM_01/fm_d_berchain_07.wmodel");
				}) != 4)
		{
			OutError = "Warlord 17090 full source 14-Mesh/2-Sprite and Chain06x8/Chain07x4 denominator changed.";
			return false;
		}

		const auto DimensionMaster = Documents.find(
			"effect.dimensionmaster.skill.2050210.unified");
		if (DimensionMaster == Documents.end() ||
			DimensionMaster->second.Elements.size() != DmReceipt->iOutputElementCount)
		{
			OutError =
				"DimensionMaster 2050210 document/receipt denominator changed.";
			return false;
		}
		std::set<std::string, std::less<>> DmSourceIds;
		std::size_t iDmRecipePortable = 0u;
		std::size_t iDmRecipeDeferred = 0u;
		std::size_t iDmDrawableAdmitted = 0u;
		std::size_t iDmAuthoringApproximate = 0u;
		std::size_t iDmPortableHard = 0u;
		std::size_t iDmPortableFailClosed = 0u;
		std::size_t iDmDeferredRejected = 0u;
		std::set<std::string, std::less<>> DmGoldenLineage;
		std::vector<const EFFECT_ELEMENT_DESC*> DmSilhouetteRows;
		std::size_t iDmGoldenBlocked = 0u;
		std::array<std::size_t, FOCUSED_DM_ROLES.size()>
			DmExpectedAdmittedRoleCounts{};
		const EFFECT_ELEMENT_DESC* DmGroupedNoBaseSprite = nullptr;
		for (const EFFECT_ELEMENT_DESC& Element : DimensionMaster->second.Elements)
		{
			const std::string SourceId = Extract_SourceElementId(Element.strSourceNode);
			const RESTORATION_RECEIPT_ROW* ReceiptRow =
				Find_RestorationRow(*DmReceipt, Element.strElementId);
			if (SourceId.empty() || !DmSourceIds.insert(SourceId).second ||
				nullptr == ReceiptRow ||
				ReceiptRow->eKind != RESTORATION_RECEIPT_ROW_KIND::PARTICLE ||
				ReceiptRow->strSourceElementId != SourceId)
			{
				OutError =
					"DimensionMaster 2050210 receipt lineage is missing or duplicate.";
				return false;
			}
			RESTORATION_RUNTIME_DISPOSITION Disposition{};
			if (!Classify_RestorationRuntimeDisposition(*ReceiptRow, Element,
					DimensionMaster->second.strEffectAssetId, Disposition, OutError))
			{
				return false;
			}
			const bool_t bPreviewExecutionTarget =
				Disposition != RESTORATION_RUNTIME_DISPOSITION::HARD_FAIL_CLOSED;
			if (ReceiptRow->bPortable)
			{
				++iDmRecipePortable;
				if (Disposition == RESTORATION_RUNTIME_DISPOSITION::FULL)
					++iDmDrawableAdmitted;
				else
				{
					++iDmPortableFailClosed;
					if (Disposition ==
						RESTORATION_RUNTIME_DISPOSITION::AUTHORING_APPROXIMATE)
					{
						++iDmAuthoringApproximate;
					}
					else
					{
						++iDmPortableHard;
					}
				}
				if (Element.SourceRecipe.strRendererShape == "mesh" &&
					(!Nearly_Equal(Element.Detail.Mesh.fModelPreScale,
						0.01f, 0.000001f, 0.000001f) ||
					 nullptr == Find_ResourceBinding(Element, "meshModel")))
				{
					OutError = "DimensionMaster portable Mesh lost canonical 0.01 WModel carrier.";
					return false;
				}
			}
			else
			{
				++iDmRecipeDeferred;
			}
			if (!bPreviewExecutionTarget)
			{
				if (Element.bVisible || !Element.Material.Execution.bFailClosed ||
					Element.Material.Execution.bAuthoringApproximate)
				{
					OutError =
						"DimensionMaster non-admitted source row is not hidden fail-closed.";
					return false;
				}
			}
			if (!ReceiptRow->bPortable)
			{
				EFFECT_DOCUMENT_DESC Mutation = DimensionMaster->second;
				EFFECT_ELEMENT_DESC* Activated = Find_ExactElement(
					Mutation, Element.strElementId);
				if (nullptr != Activated)
				{
					Activated->bVisible = true;
					std::string Rejection;
					if (!CEffectDocumentCodec::Validate(Mutation, Rejection) &&
						Rejection.find(
							"Hard fail-closed authored Element cannot be made visible") !=
								std::string::npos)
					{
						++iDmDeferredRejected;
					}
				}
			}
			if (bPreviewExecutionTarget &&
				Element.SourceRecipe.strRendererShape == "sprite" &&
				nullptr == Find_ResourceBinding(Element, "base") &&
				Element.Material.SourceMaterial.bEnabled &&
				Element.Material.SourceMaterial.strRuntimeShaderProfileId ==
					"effect.ue3.grouped-translucent.v1")
			{
				bool_t bHasPhysicalNamedTexture = false;
				for (const auto& Texture :
					Element.Material.SourceMaterial.Textures)
				{
					std::error_code Error;
					if (!Texture.strAssetId.empty() &&
						std::filesystem::is_regular_file(
							RuntimeResourceRoot / Texture.strAssetId, Error) && !Error)
					{
						bHasPhysicalNamedTexture = true;
						break;
					}
				}
				if (!bHasPhysicalNamedTexture)
				{
					OutError =
						"DimensionMaster grouped no-Base Sprite has no physical named texture.";
					return false;
				}
				if (nullptr == DmGroupedNoBaseSprite)
					DmGroupedNoBaseSprite = &Element;
			}
			const std::string_view BaseSourceId = SourceElementBaseId(SourceId);
			const bool bGoldenSource = std::find(
				DIMENSION_MASTER_A_GOLDEN_SOURCE_ELEMENTS.begin(),
				DIMENSION_MASTER_A_GOLDEN_SOURCE_ELEMENTS.end(), BaseSourceId) !=
				DIMENSION_MASTER_A_GOLDEN_SOURCE_ELEMENTS.end();
			const bool bGoldenEvent = std::find(
				DIMENSION_MASTER_A_GOLDEN_SOURCE_EVENTS.begin(),
				DIMENSION_MASTER_A_GOLDEN_SOURCE_EVENTS.end(),
				ReceiptRow->strSourceEventId) !=
				DIMENSION_MASTER_A_GOLDEN_SOURCE_EVENTS.end();
			if (bGoldenSource && bGoldenEvent)
			{
				const std::string Lineage = std::string(BaseSourceId) + "|" +
					ReceiptRow->strSourceEventId;
				if (!DmGoldenLineage.insert(Lineage).second)
				{
					OutError =
						"DimensionMaster A golden lineage is duplicate: " +
						Lineage;
					return false;
				}
				if (Disposition ==
					RESTORATION_RUNTIME_DISPOSITION::HARD_FAIL_CLOSED)
				{
					++iDmGoldenBlocked;
					continue;
				}
				DmSilhouetteRows.push_back(&Element);
				for (size_t RoleIndex = 0u;
					RoleIndex < FOCUSED_DM_ROLES.size(); ++RoleIndex)
				{
					if (BaseSourceId ==
						FOCUSED_DM_ROLES[RoleIndex].strSourceElementId)
					{
						++DmExpectedAdmittedRoleCounts[RoleIndex];
					}
				}
			}
		}
		const std::size_t iDmGoldenExpected =
			DIMENSION_MASTER_A_GOLDEN_SOURCE_ELEMENTS.size() *
			DIMENSION_MASTER_A_GOLDEN_SOURCE_EVENTS.size();
		if (iDmRecipePortable != DmReceipt->iPortableCount ||
			iDmRecipeDeferred != DmReceipt->iRecipeDeferredCount ||
			iDmDrawableAdmitted != DmReceipt->iDrawableAdmittedCount ||
			iDmPortableFailClosed != DmReceipt->iPortableFailClosedCount ||
			iDmAuthoringApproximate + iDmPortableHard !=
				DmReceipt->iPortableFailClosedCount ||
			iDmDeferredRejected != DmReceipt->iRecipeDeferredCount ||
			nullptr == DmGroupedNoBaseSprite ||
			DmGoldenLineage.size() != iDmGoldenExpected ||
			DmSilhouetteRows.size() + iDmGoldenBlocked != iDmGoldenExpected)
		{
			std::ostringstream Failure;
			Failure << "DimensionMaster 2050210 receipt-pinned disposition changed: portable=" <<
				iDmRecipePortable << "/" << DmReceipt->iPortableCount <<
				" deferred=" << iDmRecipeDeferred << "/" <<
				DmReceipt->iRecipeDeferredCount << " admitted=" <<
				iDmDrawableAdmitted << "/" << DmReceipt->iDrawableAdmittedCount <<
				" portableFailClosed=" << iDmPortableFailClosed << "/" <<
				DmReceipt->iPortableFailClosedCount <<
				" approximate=" << iDmAuthoringApproximate <<
				" portableHard=" << iDmPortableHard <<
				" rejected=" << iDmDeferredRejected <<
				" golden-admitted=" << DmSilhouetteRows.size() <<
				" golden-blocked=" << iDmGoldenBlocked;
			OutError = Failure.str();
			return false;
		}

		const auto EffectClass = [](const std::string_view EffectId)
			-> std::string_view
		{
			if (EffectId.starts_with("effect.artist.")) return "artist";
			if (EffectId.starts_with("effect.dimensionmaster."))
				return "dimensionmaster";
			if (EffectId.starts_with("effect.lancemaster."))
				return "lancemaster";
			if (EffectId.starts_with("effect.warlord.")) return "warlord";
			return {};
		};
		std::map<std::string, std::pair<const EFFECT_DOCUMENT_DESC*,
			const EFFECT_ELEMENT_DESC*>, std::less<>> DecalRepresentatives;
		for (const auto& [EffectId, Document] : Documents)
		{
			const std::string_view Class = EffectClass(EffectId);
			if (Class.empty() || DecalRepresentatives.contains(Class))
				continue;
			const RESTORATION_TARGET_RECEIPT* TargetReceipt =
				Find_RestorationTarget(Receipt, EffectId);
			if (nullptr == TargetReceipt)
				continue;
			for (const RESTORATION_RECEIPT_ROW& Row : TargetReceipt->Rows)
			{
				if (Row.eKind != RESTORATION_RECEIPT_ROW_KIND::DECAL ||
					!Row.bPortable || !Row.FailClosedReasons.empty())
				{
					continue;
				}
				const EFFECT_ELEMENT_DESC* Element = Find_ExactElement(
					Document, Row.strTargetElementId);
				RESTORATION_RUNTIME_DISPOSITION Disposition{};
				if (nullptr == Element ||
					!Classify_RestorationRuntimeDisposition(Row, *Element,
						EffectId, Disposition, OutError) ||
					Disposition != RESTORATION_RUNTIME_DISPOSITION::FULL ||
					Element->eKind != Client::EFFECT_ELEMENT_KIND::DECAL)
				{
					OutError = "Focused Decal receipt/runtime join changed: " +
						EffectId + "/" + Row.strTargetElementId;
					return false;
				}
				DecalRepresentatives.emplace(std::string(Class),
					std::pair{ &Document, Element });
				break;
			}
		}
		if (DecalRepresentatives.size() != 4u)
		{
			OutError =
				"Focused receipt-driven Decal representative is missing for a class.";
			return false;
		}

		const auto GetStagedObject = [&](const EFFECT_DOCUMENT_DESC& Document,
			const EFFECT_ELEMENT_DESC& Element)
			-> std::shared_ptr<Client::CEffectObject>
		{
			std::shared_ptr<Client::CEffectObject> Object;
			if (!Stage_OrdinarySingleVisibleElementPreview(
					Prototype, Document, Element.strElementId, Object, OutError))
			{
				return nullptr;
			}
			return Object;
		};
		const auto DrawElement = [&](const EFFECT_DOCUMENT_DESC& Document,
			const EFFECT_ELEMENT_DESC& Element,
			FOCUSED_PARTICLE_SAMPLE& Sample,
			FOCUSED_DRAW_EVIDENCE& Evidence) -> bool_t
		{
			std::shared_ptr<Client::CEffectObject> Object =
				GetStagedObject(Document, Element);
			const auto* ModelBinding = Find_ResourceBinding(Element, "meshModel");
			const DECODED_MODEL_METRICS* Metrics = nullptr == ModelBinding ? nullptr :
				GetModelMetrics(ModelBinding->strAssetId);
			if (nullptr == Object ||
				(nullptr != ModelBinding && nullptr == Metrics) ||
				!Find_VisiblePixelParticleAcrossLifetime(Object, Element, Scope,
					Sample, Evidence, OutError, nullptr, Metrics))
			{
				const std::string Detail = OutError.empty() ?
					"stage/sample failed" : OutError;
				OutError = Document.strEffectAssetId + "/" +
					Element.strElementId + ": " + Detail;
				return false;
			}
			return (nullptr == ModelBinding || nullptr != Metrics) &&
				Validate_FocusedOccurrenceSubmission(
					*Object, Element, Sample, Evidence, Metrics, OutError);
		};
		const auto DrawDecal = [&](const EFFECT_DOCUMENT_DESC& Document,
			const EFFECT_ELEMENT_DESC& Element) -> bool_t
		{
			std::shared_ptr<Client::CEffectObject> Object =
				GetStagedObject(Document, Element);
			FOCUSED_GPU_OCCURRENCE_SAMPLE Sample;
			if (nullptr == Object ||
				!Find_FirstActiveGpuOccurrence(Object, Element, Sample, OutError) ||
				!Is_FiniteCanonicalWorld(Sample.World))
			{
				const std::string Detail = OutError.empty() ?
					"stage/sample failed" : OutError;
				OutError = Document.strEffectAssetId + "/" +
					Element.strElementId + ": " + Detail;
				return false;
			}
			const float3_t Target{ Sample.World._41, Sample.World._42,
				Sample.World._43 };
			FOCUSED_DRAW_EVIDENCE Evidence;
			if (!Render_WithEvidence(Object, Scope, Target, std::nullopt,
					false, Evidence, OutError) ||
				!Validate_FocusedDecalSubmission(
					*Object, Element, Sample, Evidence, OutError))
			{
				OutError = Document.strEffectAssetId + "/" + Element.strElementId +
					": " + OutError;
				return false;
			}
			return true;
		};

		/* Draw the stable representatives after all data-only joins have passed.
		   Keeping the call outside the condition avoids hiding a draw failure in a
		   matrix predicate. */
		const bool_t bRepresentativesOnly =
			Is_FocusedRepresentativesOnlyRequested();
		for (const auto& [Case, Element] : ResolvedCases)
		{
			const auto Document = Documents.find(Case->strEffectAssetId);
			FOCUSED_PARTICLE_SAMPLE Sample;
			FOCUSED_DRAW_EVIDENCE Evidence;
			const std::string ElementIdentity =
				std::string(Case->strEffectAssetId) + "/" + Element->strElementId;
			const std::string Identity = ElementIdentity + " source=" +
				std::string(Case->strSourceElementId);
			if (bRepresentativesOnly)
				std::cout << "[FOCUSED REPRESENTATIVE] begin " << Identity <<
					'\n' << std::flush;
			if (Document == Documents.end())
			{
				OutError = "Focused representative document disappeared: " + Identity;
				return false;
			}
			if (!DrawElement(Document->second, *Element, Sample, Evidence))
			{
				if (OutError.empty())
					OutError = "Focused representative draw failed: " + Identity;
				else if (!OutError.starts_with(ElementIdentity))
					OutError = Identity + ": " + OutError;
				return false;
			}
			if (!Is_FiniteCanonicalParticleWorld(*Element, Sample))
			{
				OutError = "Focused representative renderer-world contract failed: " +
					Identity + " " + Describe_ParticleWorldContract(*Element, Sample);
				return false;
			}
			if (bRepresentativesOnly)
			{
				std::cout << "[FOCUSED REPRESENTATIVE] pass " << Identity <<
					" shape=" << Element->SourceRecipe.strRendererShape <<
					" sampleTime=" << Sample.fSampleTime << " view=" <<
					Evidence.iViewIndex << " rgb=" <<
					Evidence.iColorNonZeroPixelCount << " alpha=" <<
					Evidence.iAlphaNonZeroPixelCount << '\n' << std::flush;
			}
		}
		const auto DrawDispositionShapeRepresentatives = [&](const auto& Representatives,
			const std::array<std::size_t, 2u>& ShapeCounts,
			const std::string_view Label)
		{
			for (size_t ShapeIndex = 0u; ShapeIndex < ShapeCounts.size(); ++ShapeIndex)
			{
				if (0u == ShapeCounts[ShapeIndex])
				{
					if (Representatives[ShapeIndex].has_value())
					{
						OutError = std::string(Label) +
							" draw representative exists for an empty shape partition.";
						return false;
					}
					continue;
				}
				if (!Representatives[ShapeIndex].has_value() ||
					nullptr == Representatives[ShapeIndex]->first ||
					nullptr == Representatives[ShapeIndex]->second)
				{
					OutError = std::string(Label) +
						" receipt-driven mesh/sprite draw representative is missing.";
					return false;
				}
				const EFFECT_DOCUMENT_DESC& RepresentativeDocument =
					*Representatives[ShapeIndex]->first;
				const EFFECT_ELEMENT_DESC& RepresentativeElement =
					*Representatives[ShapeIndex]->second;
				FOCUSED_PARTICLE_SAMPLE Sample;
				FOCUSED_DRAW_EVIDENCE Evidence;
				if (!Is_RestorationPreviewExecutionTarget(RepresentativeElement) ||
					!Validate_SourceTextureContract(
						RepresentativeElement, RuntimeResourceRoot) ||
					!DrawElement(RepresentativeDocument, RepresentativeElement,
						Sample, Evidence) ||
					!Is_FiniteCanonicalParticleWorld(RepresentativeElement, Sample))
				{
					if (OutError.empty())
					{
						OutError = std::string(Label) +
							" receipt-driven mesh/sprite draw witness failed: " +
							RepresentativeDocument.strEffectAssetId + "/" +
							RepresentativeElement.strElementId;
					}
					return false;
				}
			}
			return true;
		};
		if (!DrawDispositionShapeRepresentatives(FullShapeRepresentatives,
				FullShapeCounts, "full") ||
			!DrawDispositionShapeRepresentatives(ApproximateShapeRepresentatives,
				ApproximateShapeCounts, "authoring-approximate"))
		{
			return false;
		}
		if (bRepresentativesOnly)
			return true;
		for (const auto& [Class, Representative] : DecalRepresentatives)
		{
			if (nullptr == Representative.first || nullptr == Representative.second ||
				!DrawDecal(*Representative.first, *Representative.second))
			{
				if (OutError.empty())
					OutError = "Focused Decal draw witness failed for " + Class + ".";
				return false;
			}
		}
		/* A grouped-translucent source profile can own the drawable texture
		   contract through named parameters even when the generic Base slot is
		   absent.  This is the exact DM admission expansion that a Base-only
		   gate used to hide. */
		{
			FOCUSED_PARTICLE_SAMPLE Sample;
			FOCUSED_DRAW_EVIDENCE Evidence;
			if (nullptr == DmGroupedNoBaseSprite ||
				nullptr != Find_ResourceBinding(*DmGroupedNoBaseSprite, "base") ||
				!DrawElement(DimensionMaster->second, *DmGroupedNoBaseSprite,
					Sample, Evidence) ||
				!Is_FiniteCanonicalParticleWorld(*DmGroupedNoBaseSprite, Sample))
			{
				if (OutError.empty())
					OutError =
						"DimensionMaster grouped no-Base Sprite did not submit a real draw.";
				return false;
			}
		}

		std::array<std::size_t, FOCUSED_DM_ROLES.size()> DmRoleCounts{};
		std::size_t iDmDrawnMesh = 0u;
		std::size_t iDmDrawnSprite = 0u;
		for (const EFFECT_ELEMENT_DESC* Element : DmSilhouetteRows)
		{
			if (nullptr == Element ||
				!Is_RestorationPreviewExecutionTarget(*Element) ||
				!Validate_SourceTextureContract(*Element, RuntimeResourceRoot))
			{
				OutError = "DimensionMaster silhouette row is not draw-admitted.";
				return false;
			}
			FOCUSED_PARTICLE_SAMPLE Sample;
			FOCUSED_DRAW_EVIDENCE Evidence;
			if (!DrawElement(DimensionMaster->second, *Element, Sample, Evidence) ||
				!Is_FiniteCanonicalParticleWorld(*Element, Sample))
			{
				return false;
			}
			if (Element->SourceRecipe.strRendererShape == "sprite")
			{
				++iDmDrawnSprite;
				continue;
			}
			++iDmDrawnMesh;
			const auto* ModelBinding = Find_ResourceBinding(*Element, "meshModel");
			if (nullptr == ModelBinding ||
				!Nearly_Equal(Element->Detail.Mesh.fModelPreScale,
					0.01f, 0.000001f, 0.000001f))
			{
				OutError = "DimensionMaster silhouette Mesh lost WModel pre-scale.";
				return false;
			}
			for (size_t RoleIndex = 0u;
				RoleIndex < FOCUSED_DM_ROLES.size(); ++RoleIndex)
			{
				const FOCUSED_DM_ROLE_CONTRACT& Role =
					FOCUSED_DM_ROLES[RoleIndex];
				if (SourceElementBaseId(
						Extract_SourceElementId(Element->strSourceNode)) !=
					Role.strSourceElementId)
					continue;
				++DmRoleCounts[RoleIndex];
				const DECODED_MODEL_METRICS* Metrics =
					GetModelMetrics(ModelBinding->strAssetId);
				const f32_t Determinant = Matrix_Determinant3x3(Sample.World);
				const float3_t Scale = Matrix_AbsoluteScale(Sample.World);
				const bool_t bOptionalScale =
					0.f == Role.vExpectedAbsoluteWorldScale.x ||
					Same_Float3(Scale,
						Role.vExpectedAbsoluteWorldScale, 0.003f);
				const bool_t bOptionalPass = UINT32_MAX == Role.iExpectedPass ||
					Expected_EffectPass(*Element, Determinant) == Role.iExpectedPass;
				const bool_t bOptionalRadius = 0.f == Role.fExpectedRawRadius ||
					(nullptr != Metrics && Nearly_Equal(Metrics->fRadius,
						Role.fExpectedRawRadius, 0.01f, 0.001f));
				if (ModelBinding->strAssetId != Role.strModelAssetId ||
					Element->Material.SourceMaterial.strRuntimeShaderProfileId !=
						Role.strSourceProfileId || nullptr == Metrics ||
					Metrics->iVertexCount != Role.iExpectedVertexCount ||
					Metrics->iIndexCount != Role.iExpectedIndexCount ||
					!bOptionalRadius || !bOptionalScale || !bOptionalPass ||
					(UINT32_MAX != Role.iExpectedPass &&
					 ((Determinant < 0.f) !=
						Role.bExpectedNegativeDeterminant)))
				{
					OutError = "DimensionMaster silhouette role scale/pass/geometry changed: " +
						Element->strElementId;
					return false;
				}
			}
		}
		if (iDmDrawnMesh + iDmDrawnSprite != DmSilhouetteRows.size() ||
			DmRoleCounts != DmExpectedAdmittedRoleCounts)
		{
			OutError =
				"DimensionMaster admitted golden draw/role partition changed.";
			return false;
		}

		const auto DimensionMasterT = Documents.find(
			DIMENSION_MASTER_T_UNIFIED_EFFECT_ID);
		if (DimensionMasterT == Documents.end() ||
			!Verify_DimensionMasterTModelCueDraw(DimensionMasterT->second,
				RepositoryRoot, RuntimeResourceRoot, Prototype, Scope, OutError))
		{
			return false;
		}
		return true;
	}

	bool Verify_OrdinaryRuntimeStageGate(
		const std::vector<STAGED_TARGET_FILE>& Files,
		const RESTORATION_RECEIPT& Receipt,
		const std::filesystem::path& RepositoryRoot,
		std::string& OutError)
	{
		if (Files.size() != Receipt.iTargetCount ||
			Receipt.Targets.size() != Receipt.iTargetCount)
		{
			OutError =
				"Ordinary Effect stage gate document/receipt denominator changed.";
			return false;
		}

		SCOPED_CURRENT_DIRECTORY CurrentDirectory;
		if (!CurrentDirectory.Initialize(
				RepositoryRoot / L"Client" / L"Default", OutError))
		{
			return false;
		}

		Microsoft::WRL::ComPtr<ID3D11Device> Device;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> Context;
		D3D_FEATURE_LEVEL FeatureLevel{};
		if (FAILED(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_WARP, nullptr,
				0u, nullptr, 0u, D3D11_SDK_VERSION, &Device, &FeatureLevel,
				&Context)) || nullptr == Device || nullptr == Context)
		{
			OutError =
				"Ordinary Effect stage gate could not create its WARP device.";
			return false;
		}

		std::unique_ptr<Client::CEffectObject> Prototype =
			Client::CEffectObject::Create(Device, Context);
		if (nullptr == Prototype)
		{
			OutError =
				"Ordinary Effect stage gate could not initialize CEffectObject.";
			return false;
		}
		const auto CloneObject = [&Prototype]()
		{
			return std::dynamic_pointer_cast<Client::CEffectObject>(
				Prototype->Clone(nullptr));
		};
		const auto IsVisiblePortableParticle =
			[](const EFFECT_ELEMENT_DESC& Element)
			{
				return Is_RestorationPreviewExecutionTarget(Element) &&
					Element.eKind == Client::EFFECT_ELEMENT_KIND::PARTICLE &&
					Element.SourceRecipe.bEnabled &&
					(Element.SourceRecipe.strRendererShape == "mesh" ||
					 Element.SourceRecipe.strRendererShape == "sprite");
			};
		const auto EffectClassFamily = [](const std::string_view EffectId)
			-> std::string_view
		{
			if (EffectId.starts_with("effect.artist.")) return "artist";
			if (EffectId.starts_with("effect.dimensionmaster."))
				return "dimensionmaster";
			if (EffectId.starts_with("effect.lancemaster."))
				return "lancemaster";
			if (EffectId.starts_with("effect.warlord.")) return "warlord";
			return {};
		};
		const auto SourceBool = [](const Client::EFFECT_SOURCE_MODULE_DESC& Module,
			const std::string_view PropertyPath, const bool bFallback)
		{
			const auto Literal = std::find_if(Module.Literals.begin(),
				Module.Literals.end(), [PropertyPath](const auto& Candidate)
				{
					return Candidate.strPropertyPath == PropertyPath &&
						Candidate.eKind ==
							Client::EFFECT_SOURCE_LITERAL_KIND::BOOLEAN;
				});
			return Literal == Module.Literals.end() ?
				bFallback : static_cast<bool>(Literal->bBoolean);
		};
		const auto DynamicComponent = [](const std::string_view PropertyPath)
			-> std::optional<uint32_t>
		{
			constexpr std::string_view Prefix = "dynamicparams[";
			constexpr std::string_view Suffix = "].paramvalue";
			if (!PropertyPath.starts_with(Prefix) ||
				!PropertyPath.ends_with(Suffix) ||
				PropertyPath.size() != Prefix.size() + 1u + Suffix.size())
			{
				return std::nullopt;
			}
			const char Digit = PropertyPath[Prefix.size()];
			return Digit >= '0' && Digit <= '3' ?
				std::optional<uint32_t>(static_cast<uint32_t>(Digit - '0')) :
				std::nullopt;
		};
		const auto GetComponent = [](const float4_t& Value,
			const uint32_t Component)
		{
			switch (Component)
			{
			case 0u: return Value.x;
			case 1u: return Value.y;
			case 2u: return Value.z;
			default: return Value.w;
			}
		};
		const auto SetComponent = [](float4_t& Value,
			const uint32_t Component, const float ValueToSet)
		{
			switch (Component)
			{
			case 0u: Value.x = ValueToSet; break;
			case 1u: Value.y = ValueToSet; break;
			case 2u: Value.z = ValueToSet; break;
			default: Value.w = ValueToSet; break;
			}
		};
		const auto SetDistributionConstant = [](Client::EFFECT_DISTRIBUTION_DESC&
			Distribution, const float Value)
		{
			Distribution.vDefaultMinimum = { Value, Value, Value, Value };
			Distribution.vDefaultMaximum = { Value, Value, Value, Value };
			std::fill(Distribution.LookupTable.begin(),
				Distribution.LookupTable.end(), Value);
			for (Client::EFFECT_DISTRIBUTION_KEY_DESC& Key : Distribution.Keys)
			{
				Key.vMinimum = { Value, Value, Value, Value };
				Key.vMaximum = { Value, Value, Value, Value };
				Key.vArriveTangentMinimum = {};
				Key.vLeaveTangentMinimum = {};
				Key.vArriveTangentMaximum = {};
				Key.vLeaveTangentMaximum = {};
			}
		};

		std::size_t iStageableDocumentCount = 0u;
		std::size_t iStagedPreviewableCount = 0u;
		std::size_t iRestorationElementCount = 0u;
		std::size_t iReceiptJoinedElementCount = 0u;
		std::size_t iAdmittedParticleCount = 0u;
		std::size_t iAuthoringApproximateParticleCount = 0u;
		std::size_t iPortableHardParticleCount = 0u;
		std::size_t iPortableRuntimeProbeCount = 0u;
		std::size_t iRecipeDeferredParticleCount = 0u;
		std::size_t iMaterialDrawFailClosedParticleCount = 0u;
		std::size_t iRejectedRecipeDeferredActivationCount = 0u;
		std::size_t iRejectedMaterialFailClosedActivationCount = 0u;
		std::size_t iAdmittedSourceDecalCount = 0u;
		std::size_t iAdmittedDecalRuntimeProbeCount = 0u;
		std::size_t iIncompleteSourceDecalCount = 0u;
		std::size_t iRejectedIncompleteDecalActivationCount = 0u;
		std::size_t iReceiptAnimationTrailCount = 0u;
		std::size_t iAdmittedAnimationTrailCount = 0u;
		std::size_t iRejectedReceiptTrailActivationCount = 0u;
		std::size_t iPlaceholderTrailCount = 0u;
		std::size_t iRejectedPlaceholderTrailActivationCount = 0u;
		std::vector<std::string> CandidateStageFailures;
		std::vector<std::string> RuntimeProbeFailures;
		std::vector<std::string> ActivationAdmissionFailures;
		std::vector<std::string> RecipeFamilyFailures;
		std::set<std::string, std::less<>> RecipeFamilyAttempts;
		std::set<std::string, std::less<>> ObservedRecipeFamilies;
		std::set<std::string, std::less<>> ExpectedRecipeFamilies;
		for (const std::string_view Class :
			{ "artist", "dimensionmaster", "lancemaster", "warlord" })
		{
			ExpectedRecipeFamilies.emplace(std::string(Class) + "/mesh");
			ExpectedRecipeFamilies.emplace(std::string(Class) + "/sprite");
		}
		bool bDimensionMasterTObserved = false;
		bool bDimensionMasterTStaged = false;
		std::string DimensionMasterTStageError;
		std::optional<EFFECT_DOCUMENT_DESC> RollbackBaseline;
		std::optional<EFFECT_DOCUMENT_DESC> DeferredActivationMutation;
		std::optional<EFFECT_DOCUMENT_DESC> PlaceholderTrailActivationMutation;
		std::string RollbackElementId;
		std::string RollbackRequiredBindingSlot;
		const auto TryVerifyRecipeRuntimeFamily = [&] (
			const EFFECT_DOCUMENT_DESC& Document,
			const std::shared_ptr<Client::CEffectObject>& CandidateObject,
			const EFFECT_ELEMENT_DESC& Element)
		{
			const std::string_view Class =
				EffectClassFamily(Document.strEffectAssetId);
			if (Class.empty())
				return;
			const std::string Family = std::string(Class) + "/" +
				Element.SourceRecipe.strRendererShape;
			if (!ExpectedRecipeFamilies.contains(Family) ||
				RecipeFamilyAttempts.contains(Family) ||
				ObservedRecipeFamilies.contains(Family))
			{
				return;
			}

			const auto Burst = std::min_element(
				Element.SourceRecipe.Bursts.begin(),
				Element.SourceRecipe.Bursts.end(), [](const auto& Left,
					const auto& Right)
				{
					const bool bLeftActive = Left.iCountMaximum > 0u;
					const bool bRightActive = Right.iCountMaximum > 0u;
					return bLeftActive != bRightActive ? bLeftActive :
						Left.fTimeSeconds < Right.fTimeSeconds;
				});
			if (Burst == Element.SourceRecipe.Bursts.end() ||
				0u == Burst->iCountMaximum)
			{
				return;
			}
			const auto Module = std::find_if(
				Element.SourceRecipe.Modules.begin(),
				Element.SourceRecipe.Modules.end(), [](const auto& Candidate)
				{
					return Candidate.strClassName ==
						"particlemoduleparameterdynamic";
				});
			if (Module == Element.SourceRecipe.Modules.end())
				return;

			const Client::EFFECT_DISTRIBUTION_DESC* pDistribution = nullptr;
			uint32_t iComponent = 0u;
			for (const Client::EFFECT_DISTRIBUTION_DESC& Distribution :
				Module->Distributions)
			{
				const std::optional<uint32_t> Component =
					DynamicComponent(Distribution.strPropertyPath);
				if (!Component.has_value() || 1u != Distribution.iOperation ||
					1u != Distribution.iComponentCount)
				{
					continue;
				}
				const std::string ScalePath = "dynamicparams[" +
					std::to_string(*Component) +
					"].bscalevelocitybyparamvalue";
				if (!SourceBool(*Module, ScalePath, false))
				{
					pDistribution = &Distribution;
					iComponent = *Component;
					break;
				}
			}
			if (nullptr == pDistribution)
				return;

			const float fSampleTime =
				Element.Detail.Timing.fStartDelaySeconds +
				Element.SourceRecipe.fEmitterDelaySeconds +
				Burst->fTimeSeconds + 1.f / 60.f;
			CandidateObject->Set_SampleTime(fSampleTime);
			Client::EFFECT_PARTICLE_RUNTIME_PROBE OriginalProbe;
			if (!CandidateObject->Query_ParticleRuntimeProbe(
					Element.strElementId, OriginalProbe) ||
				0u == OriginalProbe.iActiveParticleCount)
			{
				return;
			}
			RecipeFamilyAttempts.insert(Family);

			const float fOriginalValue = GetComponent(
				OriginalProbe.vFirstDynamicParameter, iComponent);
			const float fDetailSentinel = fOriginalValue + 73.25f;
			const float fRecipeSentinel = fOriginalValue + 149.5f;
			EFFECT_DOCUMENT_DESC DetailFallbackMutation = Document;
			EFFECT_ELEMENT_DESC* DetailElement = Find_ExactElement(
				DetailFallbackMutation, Element.strElementId);
			if (nullptr == DetailElement)
			{
				RecipeFamilyFailures.push_back(
					Family + ": Detail fallback mutation lost its Element.");
				return;
			}
			DetailElement->Detail.Particle.iDynamicParameterComponentMask = 0xFu;
			SetComponent(DetailElement->Detail.Particle.vDynamicParameterStart,
				iComponent, fDetailSentinel);
			SetComponent(DetailElement->Detail.Particle.vDynamicParameterEnd,
				iComponent, fDetailSentinel + 1.f);

			std::string MutationError;
			const std::shared_ptr<Client::CEffectObject> DetailObject = CloneObject();
			Client::EFFECT_PARTICLE_RUNTIME_PROBE DetailProbe;
			const bool bDetailExact = nullptr != DetailObject &&
				DetailObject->Stage_Document(
					DetailFallbackMutation, MutationError);
			if (bDetailExact)
			{
				DetailObject->Set_SampleTime(fSampleTime);
			}
			const bool bDetailProbeExact = bDetailExact &&
				DetailObject->Query_ParticleRuntimeProbe(
					Element.strElementId, DetailProbe) &&
				DetailProbe.iActiveParticleCount ==
					OriginalProbe.iActiveParticleCount &&
				std::abs(GetComponent(DetailProbe.vFirstDynamicParameter,
					iComponent) - fOriginalValue) < 0.0001f;

			EFFECT_DOCUMENT_DESC RecipeMutation = Document;
			EFFECT_ELEMENT_DESC* RecipeElement = Find_ExactElement(
				RecipeMutation, Element.strElementId);
			if (nullptr == RecipeElement)
			{
				RecipeFamilyFailures.push_back(
					Family + ": SourceRecipe mutation lost its Element.");
				return;
			}
			auto RecipeModuleIterator = std::find_if(
				RecipeElement->SourceRecipe.Modules.begin(),
				RecipeElement->SourceRecipe.Modules.end(),
				[&Module](const auto& Candidate)
				{
					return Candidate.strStableId == Module->strStableId;
				});
			if (RecipeModuleIterator ==
				RecipeElement->SourceRecipe.Modules.end())
			{
				RecipeFamilyFailures.push_back(
					Family + ": SourceRecipe mutation lost its module.");
				return;
			}
			Client::EFFECT_SOURCE_MODULE_DESC* RecipeModule =
				&*RecipeModuleIterator;
			auto RecipeDistribution = std::find_if(
				RecipeModule->Distributions.begin(),
				RecipeModule->Distributions.end(),
				[&pDistribution](const auto& Candidate)
				{
					return Candidate.strPropertyPath ==
						pDistribution->strPropertyPath;
				});
			if (RecipeDistribution == RecipeModule->Distributions.end())
			{
				RecipeFamilyFailures.push_back(
					Family + ": SourceRecipe mutation lost its distribution.");
				return;
			}
			SetDistributionConstant(*RecipeDistribution, fRecipeSentinel);
			MutationError.clear();
			const std::shared_ptr<Client::CEffectObject> RecipeObject = CloneObject();
			Client::EFFECT_PARTICLE_RUNTIME_PROBE RecipeProbe;
			const bool bRecipeStaged = nullptr != RecipeObject &&
				RecipeObject->Stage_Document(RecipeMutation, MutationError);
			if (bRecipeStaged)
				RecipeObject->Set_SampleTime(fSampleTime);
			const bool bRecipeProbeExact = bRecipeStaged &&
				RecipeObject->Query_ParticleRuntimeProbe(
					Element.strElementId, RecipeProbe) &&
				RecipeProbe.iActiveParticleCount ==
					OriginalProbe.iActiveParticleCount &&
				std::abs(GetComponent(RecipeProbe.vFirstDynamicParameter,
					iComponent) - fRecipeSentinel) < 0.0001f;
			if (!bDetailProbeExact || !bRecipeProbeExact ||
				std::abs(fOriginalValue - fRecipeSentinel) < 0.0001f)
			{
				std::ostringstream Failure;
				Failure << Family << ": SourceRecipe/Detail ownership mismatch"
					<< " detail-stage=" << bDetailExact
					<< " detail-probe=" << bDetailProbeExact
					<< " recipe-stage=" << bRecipeStaged
					<< " recipe-probe=" << bRecipeProbeExact;
				if (!MutationError.empty())
					Failure << " error=" << MutationError;
				RecipeFamilyFailures.push_back(Failure.str());
				return;
			}
			ObservedRecipeFamilies.insert(Family);
		};

		for (const STAGED_TARGET_FILE& File : Files)
		{
			EFFECT_DOCUMENT_DESC Document;
			std::string StageError;
			if (!CEffectDocumentCodec::Parse(
					File.strCanonicalDocument, Document, StageError) ||
				!CEffectDocumentCodec::Validate(Document, StageError) ||
				CEffectDocumentCodec::Serialize(Document) !=
					File.strCanonicalDocument)
			{
				OutError = "Ordinary Effect stage parse/canonical gate failed: " +
					File.Path.generic_string() + ": " + StageError;
				return false;
			}
			const RESTORATION_TARGET_RECEIPT* TargetReceipt =
				Find_RestorationTarget(Receipt, Document.strEffectAssetId);
			if (nullptr == TargetReceipt ||
				TargetReceipt->iOutputElementCount != Document.Elements.size())
			{
				OutError = "Ordinary Effect document/receipt join is missing: " +
					Document.strEffectAssetId;
				return false;
			}
			iRestorationElementCount += Document.Elements.size();
			std::set<std::string, std::less<>> JoinedElementIds;
			std::vector<const EFFECT_ELEMENT_DESC*> ExecutionTargetParticles;
			std::vector<const EFFECT_ELEMENT_DESC*> AdmittedDecals;
			for (const RESTORATION_RECEIPT_ROW& Row : TargetReceipt->Rows)
			{
				const EFFECT_ELEMENT_DESC* Element = Find_ExactElement(
					Document, Row.strTargetElementId);
				if (nullptr == Element ||
					!JoinedElementIds.insert(Row.strTargetElementId).second ||
					Extract_SourceElementId(Element->strSourceNode) !=
						Row.strSourceElementId)
				{
					OutError = "Ordinary Effect receipt lineage join failed: " +
						Document.strEffectAssetId + "/" + Row.strTargetElementId;
					return false;
				}
				const bool bExpectedParticle =
					Row.eKind == RESTORATION_RECEIPT_ROW_KIND::PARTICLE;
				const bool bExpectedDecal =
					Row.eKind == RESTORATION_RECEIPT_ROW_KIND::DECAL;
				const bool bExpectedTrail =
					Row.eKind == RESTORATION_RECEIPT_ROW_KIND::ANIMATION_TRAIL;
				RESTORATION_RUNTIME_DISPOSITION Disposition{};
				if (!Classify_RestorationRuntimeDisposition(Row, *Element,
						Document.strEffectAssetId, Disposition, OutError) ||
					(bExpectedParticle &&
					 (Element->eKind != Client::EFFECT_ELEMENT_KIND::PARTICLE ||
					  Element->SourceRecipe.strRendererShape != Row.strRendererShape)) ||
					(bExpectedDecal &&
					 (Element->eKind != Client::EFFECT_ELEMENT_KIND::DECAL ||
					  Element->SourceRecipe.strRendererShape != "decal")) ||
					(bExpectedTrail &&
					 Element->eKind != Client::EFFECT_ELEMENT_KIND::TRAIL))
				{
					OutError = "Ordinary Effect receipt/runtime disposition changed: " +
						Document.strEffectAssetId + "/" + Element->strElementId;
					return false;
				}
				++iReceiptJoinedElementCount;
				if (bExpectedParticle)
				{
					if (!Row.bPortable)
						++iRecipeDeferredParticleCount;
					else if (!Row.FailClosedReasons.empty())
					{
						++iMaterialDrawFailClosedParticleCount;
						if (Disposition ==
							RESTORATION_RUNTIME_DISPOSITION::AUTHORING_APPROXIMATE)
						{
							++iAuthoringApproximateParticleCount;
							ExecutionTargetParticles.push_back(Element);
						}
						else
						{
							++iPortableHardParticleCount;
						}
					}
					else
					{
						++iAdmittedParticleCount;
						ExecutionTargetParticles.push_back(Element);
					}
				}
				else if (bExpectedDecal)
				{
					if (Disposition == RESTORATION_RUNTIME_DISPOSITION::FULL)
					{
						++iAdmittedSourceDecalCount;
						AdmittedDecals.push_back(Element);
					}
					else
						++iIncompleteSourceDecalCount;
				}
				else
				{
					++iReceiptAnimationTrailCount;
					if (Disposition == RESTORATION_RUNTIME_DISPOSITION::FULL)
						++iAdmittedAnimationTrailCount;
				}

				if (Disposition !=
					RESTORATION_RUNTIME_DISPOSITION::HARD_FAIL_CLOSED)
					continue;
				EFFECT_DOCUMENT_DESC Mutation = Document;
				EFFECT_ELEMENT_DESC* Activated = Find_ExactElement(
					Mutation, Element->strElementId);
				if (nullptr == Activated)
				{
					ActivationAdmissionFailures.push_back(
						Document.strEffectAssetId + "/" + Element->strElementId +
						": fail-closed mutation lost its Element.");
					continue;
				}
				Activated->bVisible = true;
				std::string AdmissionError;
				if (CEffectDocumentCodec::Validate(Mutation, AdmissionError) ||
					AdmissionError.find(
						"Hard fail-closed authored Element cannot be made visible") ==
							std::string::npos)
				{
					ActivationAdmissionFailures.push_back(
						Document.strEffectAssetId + "/" + Element->strElementId +
						": receipt fail-closed activation was admitted.");
					continue;
				}
				if (bExpectedParticle && !Row.bPortable)
					++iRejectedRecipeDeferredActivationCount;
				else if (bExpectedParticle)
					++iRejectedMaterialFailClosedActivationCount;
				else if (bExpectedDecal)
					++iRejectedIncompleteDecalActivationCount;
				else
					++iRejectedReceiptTrailActivationCount;
				if (!DeferredActivationMutation.has_value())
					DeferredActivationMutation = std::move(Mutation);
			}

			std::size_t TargetPlaceholderTrailCount = 0u;
			for (const EFFECT_ELEMENT_DESC& Element : Document.Elements)
			{
				if (JoinedElementIds.contains(Element.strElementId))
					continue;
				const bool bReceiptPlaceholderTrail =
					Element.eKind == Client::EFFECT_ELEMENT_KIND::TRAIL &&
					!Element.bVisible &&
					Element.Material.Execution.bFailClosed &&
					!Element.SourceRecipe.bEnabled &&
					Element.SourcePresentation.bEnabled &&
					Element.SourcePresentation.eStatus ==
						Client::EFFECT_SOURCE_PRESENTATION_STATUS::UNRESOLVED;
				if (!bReceiptPlaceholderTrail ||
					!JoinedElementIds.insert(Element.strElementId).second)
				{
					OutError = "Ordinary Effect has an unreceipted Element: " +
						Document.strEffectAssetId + "/" + Element.strElementId;
					return false;
				}
				++TargetPlaceholderTrailCount;
				++iPlaceholderTrailCount;
				++iReceiptJoinedElementCount;
				EFFECT_DOCUMENT_DESC Mutation = Document;
				EFFECT_ELEMENT_DESC* Activated = Find_ExactElement(
					Mutation, Element.strElementId);
				if (nullptr == Activated)
				{
					ActivationAdmissionFailures.push_back(
						Document.strEffectAssetId + "/" + Element.strElementId +
						": Trail placeholder mutation lost its Element.");
					continue;
				}
				Activated->bVisible = true;
				std::string AdmissionError;
				if (CEffectDocumentCodec::Validate(Mutation, AdmissionError) ||
					AdmissionError.find(
						"Hard fail-closed authored Element cannot be made visible") ==
							std::string::npos)
				{
					ActivationAdmissionFailures.push_back(
						Document.strEffectAssetId + "/" + Element.strElementId +
						": Trail placeholder activation was admitted.");
				}
				else
				{
					++iRejectedPlaceholderTrailActivationCount;
					if (!PlaceholderTrailActivationMutation.has_value())
						PlaceholderTrailActivationMutation = std::move(Mutation);
				}
			}
			if (JoinedElementIds.size() != Document.Elements.size() ||
				TargetPlaceholderTrailCount !=
					TargetReceipt->iPlaceholderTrailExcludedCount)
			{
				OutError = "Ordinary Effect supplemental receipt denominator changed: " +
					Document.strEffectAssetId;
				return false;
			}

			const std::shared_ptr<Client::CEffectObject> CandidateObject =
				CloneObject();
			if (nullptr == CandidateObject)
			{
				OutError =
					"Ordinary Effect stage gate could not clone a candidate object.";
				return false;
			}
			StageError.clear();
			const bool bStaged =
				CandidateObject->Stage_Document(Document, StageError);
			if (Document.strEffectAssetId == DIMENSION_MASTER_T_UNIFIED_EFFECT_ID)
			{
				bDimensionMasterTObserved = true;
				bDimensionMasterTStaged = bStaged;
				DimensionMasterTStageError = StageError;
			}
			const bool bHasVisibleDrawable = std::any_of(
				Document.Elements.begin(), Document.Elements.end(),
				[](const EFFECT_ELEMENT_DESC& Element)
				{
					return Is_RestorationPreviewExecutionTarget(Element);
				}) || std::any_of(Document.ModelCues.begin(),
					Document.ModelCues.end(), [](const auto& Cue)
					{
						return Cue.bVisible;
					});
			if (!bHasVisibleDrawable)
			{
				if (bStaged ||
					StageError.find("no visible Element or Model / Summon") ==
						std::string::npos)
				{
					OutError =
						"Ordinary Effect receipt-derived non-stageable classification changed: " +
						Document.strEffectAssetId + ": " + StageError;
					return false;
				}
				continue;
			}

			++iStageableDocumentCount;
			if (!bStaged)
			{
				CandidateStageFailures.push_back(
					Document.strEffectAssetId + ": " + StageError);
				continue;
			}
			++iStagedPreviewableCount;
			std::size_t DocumentParticleProbeCount = 0u;
			std::size_t DocumentDecalProbeCount = 0u;
			std::vector<std::string> DocumentProbeFailures;
			Probe_ActiveRestorationOccurrences(CandidateObject,
				ExecutionTargetParticles, AdmittedDecals,
				DocumentParticleProbeCount, DocumentDecalProbeCount,
				DocumentProbeFailures);
			iPortableRuntimeProbeCount += DocumentParticleProbeCount;
			iAdmittedDecalRuntimeProbeCount += DocumentDecalProbeCount;
			for (const std::string& Failure : DocumentProbeFailures)
			{
				RuntimeProbeFailures.push_back(
					Document.strEffectAssetId + ": " + Failure);
			}
			for (const EFFECT_ELEMENT_DESC* Element : ExecutionTargetParticles)
			{
				if (nullptr != Element)
					TryVerifyRecipeRuntimeFamily(
						Document, CandidateObject, *Element);
			}

			if (!RollbackBaseline.has_value() &&
				!ExecutionTargetParticles.empty())
			{
				const EFFECT_ELEMENT_DESC* PortableVisible =
					ExecutionTargetParticles.front();
				const std::string RequiredSlot =
					PortableVisible->SourceRecipe.strRendererShape == "mesh" ?
						std::string(Client::EFFECT_MESH_SHAPE_SLOT_ID) :
						std::string(Client::EFFECT_STANDARD_MATERIAL_INPUTS.front().
							strSlotId);
				const bool bReferencedAsMaster = std::any_of(
					Document.Elements.begin(), Document.Elements.end(),
					[&PortableVisible](const EFFECT_ELEMENT_DESC& Element)
					{
						return Element.TransformInheritance.bEnabled &&
							Element.TransformInheritance.strMasterElementId ==
								PortableVisible->strElementId;
					});
				const bool bHasRequiredBinding = std::any_of(
					PortableVisible->ResourceBindings.begin(),
					PortableVisible->ResourceBindings.end(),
					[&RequiredSlot](const auto& Binding)
					{
						return Binding.strSlotId == RequiredSlot;
					});
				if (!bReferencedAsMaster && bHasRequiredBinding)
				{
					RollbackBaseline = Document;
					RollbackElementId = PortableVisible->strElementId;
					RollbackRequiredBindingSlot = RequiredSlot;
				}
			}
		}

		bool bValtan420633Staged = false;
		std::size_t iValtan420633ProbeCount = 0u;
		std::size_t iValtan420633FailClosedCount = 0u;
		std::size_t iValtan420633RejectedActivationCount = 0u;
		std::optional<EFFECT_DOCUMENT_DESC> Valtan420633ActivationMutation;
		std::string Valtan420633Failure;
		const std::filesystem::path Valtan420633Path = RepositoryRoot / L"Data" /
			L"Effects" / L"Authored" /
			L"effect.valtan.pattern.420633.active.effect.json";
		std::string ValtanBytes;
		EFFECT_DOCUMENT_DESC Valtan420633;
		std::string ValtanError;
		if (!Read_File(Valtan420633Path, ValtanBytes, ValtanError) ||
			!CEffectDocumentCodec::Parse(
				ValtanBytes, Valtan420633, ValtanError))
		{
			OutError = "Valtan 420633 ordinary-stage fixture could not load: " +
				ValtanError;
			return false;
		}
		const std::size_t iValtan420633PortableCount =
			static_cast<std::size_t>(std::count_if(
				Valtan420633.Elements.begin(), Valtan420633.Elements.end(),
				IsVisiblePortableParticle));
		for (const EFFECT_ELEMENT_DESC& Element : Valtan420633.Elements)
		{
			if (Element.bVisible ||
				!Element.Material.Execution.bFailClosed)
			{
				continue;
			}
			++iValtan420633FailClosedCount;
			EFFECT_DOCUMENT_DESC Mutation = Valtan420633;
			EFFECT_ELEMENT_DESC* Activated = Find_ExactElement(
				Mutation, Element.strElementId);
			if (nullptr == Activated)
			{
				Valtan420633Failure +=
					" hidden activation mutation lost " + Element.strElementId + ";";
				continue;
			}
			Activated->bVisible = true;
			Activated->Material.Execution.bFailClosed = false;
			if (!Valtan420633ActivationMutation.has_value())
				Valtan420633ActivationMutation = Mutation;
			std::string AdmissionError;
			if (!CEffectDocumentCodec::Validate(Mutation, AdmissionError))
			{
				++iValtan420633RejectedActivationCount;
			}
			else
			{
				Valtan420633Failure +=
					" hidden activation admitted " + Element.strElementId + ";";
			}
		}
		const std::shared_ptr<Client::CEffectObject> ValtanObject = CloneObject();
		bValtan420633Staged = nullptr != ValtanObject &&
			ValtanObject->Stage_Document(Valtan420633, ValtanError);
		if (bValtan420633Staged)
		{
			for (const EFFECT_ELEMENT_DESC& Element : Valtan420633.Elements)
			{
				if (!IsVisiblePortableParticle(Element))
					continue;
				Client::EFFECT_PARTICLE_RUNTIME_PROBE Probe;
				if (ValtanObject->Query_ParticleRuntimeProbe(
						Element.strElementId, Probe) &&
					Probe.bMeshRenderer ==
						(Element.SourceRecipe.strRendererShape == "mesh"))
				{
					++iValtan420633ProbeCount;
				}
			}
		}
		if (iValtan420633PortableCount !=
				EXPECTED_VALTAN_420633_PORTABLE_PARTICLE_COUNT ||
			iValtan420633FailClosedCount !=
				EXPECTED_VALTAN_420633_FAIL_CLOSED_COUNT ||
			iValtan420633RejectedActivationCount !=
				EXPECTED_VALTAN_420633_FAIL_CLOSED_COUNT ||
			!bValtan420633Staged ||
			iValtan420633ProbeCount !=
				EXPECTED_VALTAN_420633_PORTABLE_PARTICLE_COUNT)
		{
			std::ostringstream Failure;
			Failure << "Valtan 420633 ordinary Stage/probe gate failed: staged=" <<
				bValtan420633Staged << " portable=" <<
				iValtan420633PortableCount << " probes=" <<
				iValtan420633ProbeCount << " hidden-fail-closed=" <<
				iValtan420633FailClosedCount << " activation-rejections=" <<
				iValtan420633RejectedActivationCount;
			if (!ValtanError.empty()) Failure << " error=" << ValtanError;
			if (!Valtan420633Failure.empty())
				Failure << " detail=" << Valtan420633Failure;
			Valtan420633Failure = Failure.str();
		}

		if (iRestorationElementCount != Receipt.iOutputElementCount ||
			iReceiptJoinedElementCount != Receipt.iOutputElementCount ||
			iAdmittedParticleCount != Receipt.iDrawableAdmittedCount ||
			iRecipeDeferredParticleCount != Receipt.iRecipeDeferredCount ||
			iMaterialDrawFailClosedParticleCount !=
				Receipt.iPortableFailClosedCount ||
			iAuthoringApproximateParticleCount + iPortableHardParticleCount !=
				Receipt.iPortableFailClosedCount ||
			iRejectedRecipeDeferredActivationCount !=
				Receipt.iRecipeDeferredCount ||
			iRejectedMaterialFailClosedActivationCount !=
				iPortableHardParticleCount ||
			iAdmittedSourceDecalCount != Receipt.iSourceDecalReadyCount ||
			iIncompleteSourceDecalCount !=
				Receipt.iSourceDecalIncompleteCount ||
			iRejectedIncompleteDecalActivationCount !=
				Receipt.iSourceDecalIncompleteCount ||
			iRejectedReceiptTrailActivationCount !=
				iReceiptAnimationTrailCount - iAdmittedAnimationTrailCount ||
			iPlaceholderTrailCount != Receipt.iPlaceholderTrailExcludedCount ||
			iRejectedPlaceholderTrailActivationCount !=
				Receipt.iPlaceholderTrailExcludedCount ||
			!bDimensionMasterTObserved ||
			!RollbackBaseline.has_value() || RollbackElementId.empty() ||
			RollbackRequiredBindingSlot.empty())
		{
			std::ostringstream Failure;
			Failure << "Ordinary Effect receipt/runtime denominator changed"
				<< ": docs=" << Files.size()
				<< " stageable=" << iStageableDocumentCount
				<< " elements=" << iRestorationElementCount
				<< " receipt-joined=" << iReceiptJoinedElementCount
				<< " admitted-particle=" << iAdmittedParticleCount
				<< " authoring-approximate-particle=" <<
					iAuthoringApproximateParticleCount
				<< " portable-hard-particle=" << iPortableHardParticleCount
				<< " recipe-deferred=" << iRecipeDeferredParticleCount
				<< " material-fail-closed=" <<
					iMaterialDrawFailClosedParticleCount
				<< " admitted-decal=" << iAdmittedSourceDecalCount
				<< " incomplete-decal=" << iIncompleteSourceDecalCount
				<< " receipt-trail=" << iReceiptAnimationTrailCount
				<< " admitted-trail=" << iAdmittedAnimationTrailCount
				<< " placeholder-trail=" << iPlaceholderTrailCount;
			OutError = Failure.str();
			return false;
		}

		std::shared_ptr<Client::CEffectObject> Object = CloneObject();
		if (nullptr == Object)
		{
			OutError =
				"Ordinary Effect stage gate could not clone its rollback object.";
			return false;
		}
		std::string StageError;
		if (!Object->Stage_Document(*RollbackBaseline, StageError))
		{
			OutError = "Ordinary Effect rollback baseline could not restage: " +
				StageError;
			return false;
		}
		Client::EFFECT_PARTICLE_RUNTIME_PROBE CommittedProbe;
		if (!Object->Query_ParticleRuntimeProbe(
				RollbackElementId, CommittedProbe))
		{
			OutError =
				"Ordinary Effect rollback baseline lost its committed Particle.";
			return false;
		}
		const std::string CommittedStatus = Object->Get_Status();
		const auto SameFloat4 = [](const float4_t& Left, const float4_t& Right)
		{
			return Left.x == Right.x && Left.y == Right.y &&
				Left.z == Right.z && Left.w == Right.w;
		};
		const auto SameProbe = [&SameFloat4](
			const Client::EFFECT_PARTICLE_RUNTIME_PROBE& Left,
			const Client::EFFECT_PARTICLE_RUNTIME_PROBE& Right)
		{
			return Left.fSampleTimeSeconds == Right.fSampleTimeSeconds &&
				Left.iActiveParticleCount == Right.iActiveParticleCount &&
				Left.bMeshRenderer == Right.bMeshRenderer &&
				SameFloat4(Left.vFirstDynamicParameter,
					Right.vFirstDynamicParameter) &&
				SameFloat4(Left.vMinDynamicParameter,
					Right.vMinDynamicParameter) &&
				SameFloat4(Left.vMaxDynamicParameter,
					Right.vMaxDynamicParameter) &&
				Left.fFirstAlpha == Right.fFirstAlpha &&
				Left.fMinAlpha == Right.fMinAlpha &&
				Left.fMaxAlpha == Right.fMaxAlpha &&
				Left.fFirstNormalizedLife == Right.fFirstNormalizedLife &&
				Left.fFirstSubImageIndex == Right.fFirstSubImageIndex &&
				SameFloat4(Left.FirstSubUV.Current, Right.FirstSubUV.Current) &&
				SameFloat4(Left.FirstSubUV.Next, Right.FirstSubUV.Next) &&
				Left.FirstSubUV.fBlend == Right.FirstSubUV.fBlend;
		};
		const auto RejectsAndPreserves = [&](EFFECT_DOCUMENT_DESC Invalid,
			const std::string_view InvalidElementId,
			const std::string_view Label,
			const std::string_view ExpectedRejection = {})
		{
			std::string Rejection;
			Client::EFFECT_PARTICLE_RUNTIME_PROBE PreservedProbe;
			Client::EFFECT_PARTICLE_RUNTIME_PROBE InvalidProbe;
			const bool bRejected = !Object->Stage_Document(Invalid, Rejection);
			const bool bPreservedElement = Object->Query_ParticleRuntimeProbe(
				RollbackElementId, PreservedProbe);
			const bool bInvalidElementCommitted = !InvalidElementId.empty() &&
				Object->Query_ParticleRuntimeProbe(InvalidElementId, InvalidProbe);
			const bool bProbePreserved = bPreservedElement &&
				SameProbe(CommittedProbe, PreservedProbe);
			const bool bStatusPreserved =
				Object->Get_Status() == CommittedStatus;
			const bool bExpectedRejection = ExpectedRejection.empty() ||
				Rejection.find(ExpectedRejection) != std::string::npos;
			if (!bRejected || Rejection.empty() || !bProbePreserved ||
				bInvalidElementCommitted || !bStatusPreserved ||
				!bExpectedRejection)
			{
				std::ostringstream Failure;
				Failure << "Ordinary Effect " << Label <<
					" rejection did not preserve the committed CEffectObject"
					<< ": rejected=" << bRejected
					<< " rejection-message=" << !Rejection.empty()
					<< " old-probe=" << bPreservedElement
					<< " old-probe-exact=" << bProbePreserved
					<< " invalid-probe=" << bInvalidElementCommitted
					<< " status-preserved=" << bStatusPreserved
					<< " expected-rejection=" << bExpectedRejection
					<< " dimensionmaster-t-stage=" << bDimensionMasterTStaged;
				if (!bDimensionMasterTStaged &&
					!DimensionMasterTStageError.empty())
				{
					Failure << " dimensionmaster-t-error=" <<
						DimensionMasterTStageError;
				}
				if (!Rejection.empty())
					Failure << " rejection=" << Rejection;
				OutError = Failure.str();
				return false;
			}
			return true;
		};

		EFFECT_DOCUMENT_DESC MissingResource = *RollbackBaseline;
		EFFECT_ELEMENT_DESC* MissingElement = Find_ExactElement(
			MissingResource, RollbackElementId);
		if (nullptr == MissingElement)
		{
			OutError = "Ordinary Effect missing-resource fixture lost its Element.";
			return false;
		}
		auto MissingBinding = std::find_if(
			MissingElement->ResourceBindings.begin(),
			MissingElement->ResourceBindings.end(),
			[&RollbackRequiredBindingSlot](const auto& Binding)
			{
				return Binding.strSlotId == RollbackRequiredBindingSlot;
			});
		if (MissingBinding == MissingElement->ResourceBindings.end())
		{
			OutError =
				"Ordinary Effect missing-resource fixture lost its required binding.";
			return false;
		}
		const bool bModelBinding =
			std::filesystem::path(MissingBinding->strAssetId).extension() ==
				".wmodel";
		MissingBinding->strAssetId = bModelBinding ?
			"Effect/__ordinary_stage_missing__.wmodel" :
			"Effect/__ordinary_stage_missing__.dds";
		const std::string MissingElementId =
			"ordinary.stage.missing.resource.probe";
		MissingElement->strElementId = MissingElementId;
		std::error_code ResourceError;
		if (std::filesystem::exists(
				RepositoryRoot / L"Client" / L"Bin" / L"Resources" /
				std::filesystem::path(MissingBinding->strAssetId), ResourceError) ||
			ResourceError || !RejectsAndPreserves(std::move(MissingResource),
				MissingElementId, "missing-resource"))
		{
			if (OutError.empty())
			{
				OutError =
					"Ordinary Effect missing-resource probe path unexpectedly exists.";
			}
			return false;
		}

		EFFECT_DOCUMENT_DESC UnsupportedModule = *RollbackBaseline;
		EFFECT_ELEMENT_DESC* UnsupportedElement = Find_ExactElement(
			UnsupportedModule, RollbackElementId);
		if (nullptr == UnsupportedElement)
		{
			OutError = "Ordinary Effect unsupported-module fixture lost its Element.";
			return false;
		}
		const std::string UnsupportedElementId =
			"ordinary.stage.unsupported.module.probe";
		UnsupportedElement->strElementId = UnsupportedElementId;
		Client::EFFECT_SOURCE_MODULE_DESC Unsupported;
		Unsupported.strStableId = "ordinary.stage.unsupported.module";
		Unsupported.strClassName = "particlemodulecollision";
		Unsupported.strObjectPath = "fixture/ordinary-stage-unsupported";
		UnsupportedElement->SourceRecipe.Modules.push_back(
			std::move(Unsupported));
		if (!RejectsAndPreserves(std::move(UnsupportedModule),
				UnsupportedElementId, "unsupported-module"))
		{
			return false;
		}
		if (DeferredActivationMutation.has_value() &&
			!RejectsAndPreserves(*DeferredActivationMutation, {},
				"deferred-activation"))
		{
			return false;
		}
		if (PlaceholderTrailActivationMutation.has_value() &&
			!RejectsAndPreserves(*PlaceholderTrailActivationMutation, {},
				"placeholder-Trail-activation"))
		{
			return false;
		}
		if (Valtan420633ActivationMutation.has_value() &&
			!RejectsAndPreserves(*Valtan420633ActivationMutation, {},
				"Valtan-420633-hidden-activation"))
		{
			return false;
		}

		const std::filesystem::path FixtureResourceRoot = RepositoryRoot /
			L"Client" / L"Bin" / L"Resources";
		std::string EventFixtureTextureId;
		std::error_code EventFixtureResourceError;
		for (std::filesystem::recursive_directory_iterator Iterator(
				 FixtureResourceRoot / L"Effect",
				 std::filesystem::directory_options::skip_permission_denied,
				 EventFixtureResourceError), End;
			 !EventFixtureResourceError && Iterator != End &&
				 EventFixtureTextureId.empty();
			 Iterator.increment(EventFixtureResourceError))
		{
			if (!Iterator->is_regular_file(EventFixtureResourceError) ||
				EventFixtureResourceError ||
				Iterator->path().extension() != L".dds")
			{
				EventFixtureResourceError.clear();
				continue;
			}
			const std::filesystem::path Relative = std::filesystem::relative(
				Iterator->path(), FixtureResourceRoot, EventFixtureResourceError);
			if (!EventFixtureResourceError)
				EventFixtureTextureId = Relative.generic_string();
		}
		if (EventFixtureResourceError || EventFixtureTextureId.empty())
		{
			OutError =
				"Ordinary source-event fixtures require one physical Effect DDS.";
			return false;
		}
		const auto MakeConstantModule = [&](const std::string_view StableId,
			const std::string_view ClassName,
			const std::initializer_list<std::pair<std::string_view, float>>
				Properties)
		{
			Client::EFFECT_SOURCE_MODULE_DESC Module;
			Module.strStableId = StableId;
			Module.strClassName = ClassName;
			Module.strObjectPath = "fixture/" + std::string(StableId);
			for (const auto& [Property, Value] : Properties)
			{
				Client::EFFECT_DISTRIBUTION_DESC Distribution;
				Distribution.strPropertyPath = Property;
				if (ClassName != "particlemodulerequired")
					SetDistributionConstant(Distribution, Value);
				Module.Distributions.push_back(std::move(Distribution));
			}
			return Module;
		};
		EFFECT_ELEMENT_DESC EventTemplate;
		EventTemplate.strElementId = "ordinary.source_event.template";
		EventTemplate.strDisplayName = "Source event template";
		EventTemplate.strGroupId = "ordinary.source-event.fixture";
		EventTemplate.eKind = Client::EFFECT_ELEMENT_KIND::PARTICLE;
		EventTemplate.ResourceBindings.push_back(
			{ "base", EventFixtureTextureId });
		EventTemplate.Detail.Particle.fSpawnRatePerSecond = 0.f;
		EventTemplate.Detail.Particle.vLifeTimeSeconds = { 1.f, 1.f };
		EventTemplate.SourceRecipe.bEnabled = true;
		EventTemplate.SourceRecipe.strRendererShape = "sprite";
		EventTemplate.SourceRecipe.fEmitterDurationSeconds = 1.f;
		EventTemplate.SourceRecipe.iEmitterLoopCount = 1u;
		EventTemplate.SourceRecipe.Modules.push_back(MakeConstantModule(
			"fixture.required", "particlemodulerequired",
			{ { "spawnrate", 0.f } }));
		EventTemplate.SourceRecipe.Modules.push_back(MakeConstantModule(
			"fixture.lifetime", "particlemodulelifetime",
			{ { "lifetime", 1.f } }));
		EventTemplate.SourceRecipe.Modules.push_back(MakeConstantModule(
			"fixture.size", "particlemodulesize",
			{ { "startsize", 1.f } }));
		EventTemplate.SourceRecipe.Modules.push_back(MakeConstantModule(
			"fixture.spawn", "particlemodulespawn",
			{ { "rate", 0.f }, { "ratescale", 1.f } }));
		const auto MakeEventDocument = [&](const std::string_view EffectId,
			const std::string_view DisplayName)
		{
			EFFECT_DOCUMENT_DESC Fixture;
			Fixture.strEffectAssetId = EffectId;
			Fixture.strDisplayName = DisplayName;
			return Fixture;
		};
		const auto MakeEventElement = [&](const std::string_view ElementId)
		{
			EFFECT_ELEMENT_DESC Element = EventTemplate;
			Element.strElementId = ElementId;
			Element.strDisplayName = ElementId;
			Element.strGroupId = "ordinary.source-event.fixture";
			Element.strSourceNode.clear();
			Element.bVisible = true;
			Element.Material.Execution.bFailClosed = false;
			Element.ActionCueAttachment = {};
			Element.TransformInheritance = {};
			Element.SourcePresentation = {};
			Element.SourceRecipe.fEmitterDelaySeconds = 0.f;
			Element.SourceRecipe.fEmitterDurationSeconds = 1.f;
			Element.SourceRecipe.iEmitterLoopCount = 1u;
			Element.SourceRecipe.Bursts.clear();
			for (Client::EFFECT_SOURCE_MODULE_DESC& Module :
				Element.SourceRecipe.Modules)
			{
				if (Module.strClassName != "particlemodulespawn")
					continue;
				for (Client::EFFECT_DISTRIBUTION_DESC& Distribution :
					Module.Distributions)
				{
					if (Distribution.strPropertyPath == "rate" ||
						Distribution.strPropertyPath == "ratescale")
					{
						SetDistributionConstant(Distribution, 0.f);
					}
				}
			}
			Element.Detail.Particle.fSpawnRatePerSecond = 0.f;
			return Element;
		};
		const auto StringLiteral = [](const std::string_view Property,
			const std::string_view Value)
		{
			Client::EFFECT_SOURCE_LITERAL_DESC Literal;
			Literal.strPropertyPath = Property;
			Literal.eKind = Client::EFFECT_SOURCE_LITERAL_KIND::STRING;
			Literal.strString = Value;
			return Literal;
		};
		const auto NumberLiteral = [](const std::string_view Property,
			const double Value)
		{
			Client::EFFECT_SOURCE_LITERAL_DESC Literal;
			Literal.strPropertyPath = Property;
			Literal.eKind = Client::EFFECT_SOURCE_LITERAL_KIND::NUMBER;
			Literal.fNumber = Value;
			return Literal;
		};
		const auto MakeGeneratorModule = [&](const std::string_view StableId,
			const std::string_view Route)
		{
			Client::EFFECT_SOURCE_MODULE_DESC Module;
			Module.strStableId = StableId;
			Module.strClassName = "particlemoduleeventgenerator";
			Module.strObjectPath = "fixture/" + std::string(StableId);
			Module.Literals.push_back(StringLiteral(
				"events[0].type", "epet_spawn"));
			Module.Literals.push_back(StringLiteral(
				"events[0].customname", Route));
			return Module;
		};
		const auto MakeReceiverModule = [&](const std::string_view StableId,
			const std::string_view Route, const float SpawnCount)
		{
			Client::EFFECT_SOURCE_MODULE_DESC Module;
			Module.strStableId = StableId;
			Module.strClassName = "particlemoduleeventreceiverspawn";
			Module.strObjectPath = "fixture/" + std::string(StableId);
			Module.Literals.push_back(StringLiteral(
				"eventgeneratortype", "epet_spawn"));
			Module.Literals.push_back(StringLiteral("eventname", Route));
			for (const std::pair<std::string_view, float> Property : {
				std::pair<std::string_view, float>{ "inheritvelocityscale", 1.f },
				std::pair<std::string_view, float>{ "spawncount", SpawnCount } })
			{
				Client::EFFECT_DISTRIBUTION_DESC Distribution;
				Distribution.strPropertyPath = Property.first;
				SetDistributionConstant(Distribution, Property.second);
				Module.Distributions.push_back(std::move(Distribution));
			}
			return Module;
		};

		EFFECT_DOCUMENT_DESC UnmatchedRoute = MakeEventDocument(
			"effect.ordinary.source_event.unmatched.fixture",
			"Source event unmatched fixture");
		EFFECT_ELEMENT_DESC UnmatchedGenerator = MakeEventElement(
			"ordinary.source_event.unmatched.generator");
		UnmatchedGenerator.SourceRecipe.Modules.push_back(
			MakeGeneratorModule("fixture.event.unmatched.generator",
				"fixture_unmatched_route"));
		UnmatchedRoute.Elements.push_back(std::move(UnmatchedGenerator));
		if (!RejectsAndPreserves(std::move(UnmatchedRoute), {},
				"unmatched-source-event-route", "same-document"))
		{
			return false;
		}

		EFFECT_DOCUMENT_DESC RouteCycle = MakeEventDocument(
			"effect.ordinary.source_event.cycle.fixture",
			"Source event cycle fixture");
		EFFECT_ELEMENT_DESC CycleA = MakeEventElement(
			"ordinary.source_event.cycle.a");
		CycleA.SourceRecipe.Modules.push_back(MakeGeneratorModule(
			"fixture.event.cycle.a.generator", "fixture_cycle_a_to_b"));
		CycleA.SourceRecipe.Modules.push_back(MakeReceiverModule(
			"fixture.event.cycle.a.receiver", "fixture_cycle_b_to_a", 1.f));
		EFFECT_ELEMENT_DESC CycleB = MakeEventElement(
			"ordinary.source_event.cycle.b");
		CycleB.SourceRecipe.Modules.push_back(MakeReceiverModule(
			"fixture.event.cycle.b.receiver", "fixture_cycle_a_to_b", 1.f));
		CycleB.SourceRecipe.Modules.push_back(MakeGeneratorModule(
			"fixture.event.cycle.b.generator", "fixture_cycle_b_to_a"));
		RouteCycle.Elements.push_back(std::move(CycleA));
		RouteCycle.Elements.push_back(std::move(CycleB));
		if (!RejectsAndPreserves(std::move(RouteCycle), {},
				"cyclic-source-event-route", "cycle"))
		{
			return false;
		}

		EFFECT_DOCUMENT_DESC Overflow = MakeEventDocument(
			"effect.ordinary.source_event.overflow.fixture",
			"Source event overflow fixture");
		EFFECT_ELEMENT_DESC OverflowGenerator = MakeEventElement(
			"ordinary.source_event.overflow.generator");
		OverflowGenerator.Detail.Particle.iMaxParticles = 4096u;
		OverflowGenerator.SourceRecipe.Bursts = {
			{ 0.f, 1u, 1u }, { 2.f / 60.f, 4095u, 4095u } };
		Client::EFFECT_SOURCE_MODULE_DESC OverflowGeneratorModule =
			MakeGeneratorModule("fixture.event.overflow.generator",
				"fixture_overflow_route");
		for (uint32_t iEvent = 1u; iEvent <= 2u; ++iEvent)
		{
			const std::string Prefix = "events[" + std::to_string(iEvent) + "].";
			OverflowGeneratorModule.Literals.push_back(StringLiteral(
				Prefix + "type", "epet_spawn"));
			OverflowGeneratorModule.Literals.push_back(StringLiteral(
				Prefix + "customname", "fixture_overflow_route"));
			OverflowGeneratorModule.Literals.push_back(NumberLiteral(
				Prefix + "frequency", 4096.0));
		}
		OverflowGenerator.SourceRecipe.Modules.push_back(
			std::move(OverflowGeneratorModule));
		EFFECT_ELEMENT_DESC OverflowReceiver = MakeEventElement(
			"ordinary.source_event.overflow.receiver");
		OverflowReceiver.Detail.Particle.iMaxParticles = 4096u;
		OverflowReceiver.SourceRecipe.Modules.push_back(MakeReceiverModule(
			"fixture.event.overflow.receiver", "fixture_overflow_route", 1.f));
		Overflow.Elements.push_back(std::move(OverflowGenerator));
		Overflow.Elements.push_back(std::move(OverflowReceiver));

		std::shared_ptr<const Client::CEffectPlayback::PREPARED_RESOURCES>
			OverflowResources;
		std::string OverflowError;
		Client::CEffectPlayback OverflowPlayback;
		if (!Client::CEffectPlayback::Prepare_DocumentResources(
				Overflow, OverflowResources, OverflowError) ||
			!OverflowPlayback.Stage_PrevalidatedDocument(
				Overflow, std::move(OverflowResources), OverflowError))
		{
			OutError = "Source-event overflow fixture could not stage: " +
				OverflowError;
			return false;
		}
		const auto IdentityHistory = [](const float,
			Client::EFFECT_FIXED_STEP_TRANSFORM_SAMPLE& OutSample,
			std::string& OutHistoryError)
		{
			DirectX::XMStoreFloat4x4(
				&OutSample.RootWorld, DirectX::XMMatrixIdentity());
			OutSample.SourceAnchorWorlds.clear();
			OutHistoryError.clear();
			return true;
		};
		constexpr float EVENT_FIXED_STEP = 1.f / 60.f;
		if (!OverflowPlayback.Update_WithTransformHistory(
				EVENT_FIXED_STEP, IdentityHistory, OverflowError))
		{
			OutError = "Source-event overflow fixture first step failed: " +
				OverflowError;
			return false;
		}
		Client::EFFECT_PARTICLE_RUNTIME_PROBE CommittedGeneratorProbe;
		Client::EFFECT_PARTICLE_RUNTIME_PROBE CommittedReceiverProbe;
		if (!OverflowPlayback.Query_ParticleRuntimeProbe(
				"ordinary.source_event.overflow.generator",
				CommittedGeneratorProbe) ||
			!OverflowPlayback.Query_ParticleRuntimeProbe(
				"ordinary.source_event.overflow.receiver",
				CommittedReceiverProbe) ||
			CommittedGeneratorProbe.iActiveParticleCount != 1u ||
			CommittedReceiverProbe.iActiveParticleCount != 1u)
		{
			OutError =
				"Source-event overflow fixture did not establish its committed step.";
			return false;
		}
		const double CommittedEventClock =
			OverflowPlayback.Get_FixedStepClockSeconds();
		const auto OverflowAttemptPreservesState = [&]()
		{
			std::string AttemptError;
			Client::EFFECT_PARTICLE_RUNTIME_PROBE GeneratorProbe;
			Client::EFFECT_PARTICLE_RUNTIME_PROBE ReceiverProbe;
			const bool bRejected = !OverflowPlayback.Update_WithTransformHistory(
				EVENT_FIXED_STEP, IdentityHistory, AttemptError);
			const bool bClockPreserved =
				OverflowPlayback.Get_FixedStepClockSeconds() == CommittedEventClock;
			const bool bGeneratorPreserved =
				OverflowPlayback.Query_ParticleRuntimeProbe(
					"ordinary.source_event.overflow.generator", GeneratorProbe) &&
				SameProbe(CommittedGeneratorProbe, GeneratorProbe);
			const bool bReceiverPreserved =
				OverflowPlayback.Query_ParticleRuntimeProbe(
					"ordinary.source_event.overflow.receiver", ReceiverProbe) &&
				SameProbe(CommittedReceiverProbe, ReceiverProbe);
			const bool bOverflowChannel = AttemptError.find(
				"source event queue overflow rolled back the fixed step") !=
				std::string::npos;
			if (!bRejected || !bClockPreserved || !bGeneratorPreserved ||
				!bReceiverPreserved || !bOverflowChannel)
			{
				std::ostringstream Failure;
				Failure << "Source-event overflow rollback changed committed playback"
					<< ": rejected=" << bRejected
					<< " clock=" << bClockPreserved
					<< " generator=" << bGeneratorPreserved
					<< " receiver=" << bReceiverPreserved
					<< " channel=" << bOverflowChannel;
				if (!AttemptError.empty()) Failure << " error=" << AttemptError;
				OutError = Failure.str();
				return false;
			}
			return true;
		};
		/* The second identical rejection makes EventTrackingCounts rollback
		   observable: without restoring the two frequency-4096 counters, the
		   retried 4095-particle burst emits no extra events and would commit. */
		if (!OverflowAttemptPreservesState() ||
			!OverflowAttemptPreservesState())
		{
			return false;
		}

		const bool bRuntimeContractExact =
			iStagedPreviewableCount == iStageableDocumentCount &&
			bDimensionMasterTStaged &&
			iPortableRuntimeProbeCount == iAdmittedParticleCount +
				iAuthoringApproximateParticleCount &&
			iAdmittedDecalRuntimeProbeCount == iAdmittedSourceDecalCount &&
			RuntimeProbeFailures.empty() &&
			iRejectedRecipeDeferredActivationCount ==
				Receipt.iRecipeDeferredCount &&
			iRejectedMaterialFailClosedActivationCount ==
				iPortableHardParticleCount &&
			iRejectedIncompleteDecalActivationCount ==
				Receipt.iSourceDecalIncompleteCount &&
			iRejectedReceiptTrailActivationCount ==
				iReceiptAnimationTrailCount - iAdmittedAnimationTrailCount &&
			iRejectedPlaceholderTrailActivationCount ==
				Receipt.iPlaceholderTrailExcludedCount &&
			ActivationAdmissionFailures.empty() &&
			RecipeFamilyAttempts.size() ==
				EXPECTED_RECIPE_RUNTIME_FAMILY_COUNT &&
			ObservedRecipeFamilies == ExpectedRecipeFamilies &&
			RecipeFamilyFailures.empty() &&
			bValtan420633Staged &&
			iValtan420633ProbeCount ==
				EXPECTED_VALTAN_420633_PORTABLE_PARTICLE_COUNT &&
			iValtan420633FailClosedCount ==
				EXPECTED_VALTAN_420633_FAIL_CLOSED_COUNT &&
			iValtan420633RejectedActivationCount ==
				EXPECTED_VALTAN_420633_FAIL_CLOSED_COUNT &&
			Valtan420633ActivationMutation.has_value() &&
			(0u == Receipt.iPlaceholderTrailExcludedCount ||
			 PlaceholderTrailActivationMutation.has_value()) &&
			Valtan420633Failure.empty() &&
			CandidateStageFailures.empty();
		if (!bRuntimeContractExact)
		{
			std::ostringstream Failure;
			Failure << "Ordinary Effect runtime-consumption gate failed"
				<< ": staged-docs=" << iStagedPreviewableCount << "/" <<
					iStageableDocumentCount
				<< " dm-t-stage=" << bDimensionMasterTStaged
				<< " particle-active-probes=" << iPortableRuntimeProbeCount << "/" <<
					(iAdmittedParticleCount + iAuthoringApproximateParticleCount)
				<< " particle-full=" << iAdmittedParticleCount
				<< " particle-authoring-approximate=" <<
					iAuthoringApproximateParticleCount
				<< " particle-portable-hard=" << iPortableHardParticleCount
				<< " decal-active-probes=" << iAdmittedDecalRuntimeProbeCount <<
					"/" << iAdmittedSourceDecalCount
				<< " recipe-deferred-rejections=" <<
					iRejectedRecipeDeferredActivationCount << "/" <<
					Receipt.iRecipeDeferredCount
				<< " material-fail-closed-rejections=" <<
					iRejectedMaterialFailClosedActivationCount << "/" <<
					iPortableHardParticleCount
				<< " decal-incomplete-rejections=" <<
					iRejectedIncompleteDecalActivationCount << "/" <<
					Receipt.iSourceDecalIncompleteCount
				<< " trail-rejections=" <<
					iRejectedPlaceholderTrailActivationCount << "/" <<
					Receipt.iPlaceholderTrailExcludedCount
				<< " recipe-families=" << ObservedRecipeFamilies.size() << "/" <<
					EXPECTED_RECIPE_RUNTIME_FAMILY_COUNT
				<< " valtan-420633-stage=" << bValtan420633Staged
				<< " valtan-420633-probes=" << iValtan420633ProbeCount
				<< " valtan-420633-hidden=" <<
					iValtan420633FailClosedCount
				<< " valtan-420633-hidden-rejections=" <<
					iValtan420633RejectedActivationCount;
			for (const std::string& CandidateFailure : CandidateStageFailures)
				Failure << "; stage: " << CandidateFailure;
			for (const std::string& ProbeFailure : RuntimeProbeFailures)
				Failure << "; probe: " << ProbeFailure;
			for (const std::string& AdmissionFailure :
				ActivationAdmissionFailures)
			{
				Failure << "; activation: " << AdmissionFailure;
			}
			for (const std::string& RecipeFailure : RecipeFamilyFailures)
				Failure << "; recipe: " << RecipeFailure;
			if (!Valtan420633Failure.empty())
				Failure << "; valtan: " << Valtan420633Failure;
			for (const std::string& ExpectedFamily : ExpectedRecipeFamilies)
			{
				if (!ObservedRecipeFamilies.contains(ExpectedFamily))
					Failure << "; missing-family: " << ExpectedFamily;
			}
			if (!bDimensionMasterTStaged &&
				!DimensionMasterTStageError.empty())
			{
				Failure << "; dm-t-error: " << DimensionMasterTStageError;
			}
			OutError = Failure.str();
			return false;
		}
		return true;
	}

	bool Test_GenericReimportFieldOwnership(
		const std::filesystem::path& RuntimeResourceRoot,
		std::string& OutError)
	{
		std::vector<std::string> TextureIds;
		std::vector<std::string> ModelIds;
		std::error_code Error;
		const std::filesystem::path EffectRoot = RuntimeResourceRoot / L"Effect";
		for (std::filesystem::recursive_directory_iterator Iterator(
				 EffectRoot,
				 std::filesystem::directory_options::skip_permission_denied,
				 Error), End;
			 !Error && Iterator != End &&
				(TextureIds.size() < 2u || ModelIds.size() < 2u);
			 Iterator.increment(Error))
		{
			if (!Iterator->is_regular_file(Error) || Error)
			{
				Error.clear();
				continue;
			}
			const std::filesystem::path Relative = std::filesystem::relative(
				Iterator->path(), RuntimeResourceRoot, Error);
			if (Error)
				break;
			const std::string AssetId = Relative.generic_string();
			Client::EFFECT_RESOURCE_FILE_KIND Kind =
				Client::EFFECT_RESOURCE_FILE_KIND::END;
			if (!CEffectDocumentCodec::Is_SafeResourceAssetId(AssetId, &Kind))
				continue;
			if (Kind == Client::EFFECT_RESOURCE_FILE_KIND::TEXTURE &&
				TextureIds.size() < 2u)
			{
				TextureIds.push_back(AssetId);
			}
			else if (Kind == Client::EFFECT_RESOURCE_FILE_KIND::MODEL &&
				ModelIds.size() < 2u)
			{
				ModelIds.push_back(AssetId);
			}
		}
		if (Error || TextureIds.size() != 2u || ModelIds.size() != 2u)
		{
			OutError =
				"Field-aware reimport fixture requires two physical Effect DDS and WModel files.";
			return false;
		}

		const auto FindAsset = [](const EFFECT_ELEMENT_DESC& Element,
			const std::string_view Slot) -> std::string_view
		{
			const auto Binding = std::find_if(Element.ResourceBindings.begin(),
				Element.ResourceBindings.end(),
				[Slot](const Client::EFFECT_RESOURCE_BINDING_DESC& Candidate)
				{
					return Candidate.strSlotId == Slot;
				});
			return Binding == Element.ResourceBindings.end() ?
				std::string_view{} : Binding->strAssetId;
		};
		const auto AddPortableModule = [](EFFECT_ELEMENT_DESC& Element,
			const std::string_view StableId, const std::string_view ClassName,
			const std::initializer_list<std::string_view> Properties)
		{
			Client::EFFECT_SOURCE_MODULE_DESC Module;
			Module.strStableId = StableId;
			Module.strClassName = ClassName;
			Module.strObjectPath = "fixture/" + std::string(StableId);
			for (const std::string_view Property : Properties)
			{
				Client::EFFECT_DISTRIBUTION_DESC Distribution;
				Distribution.strPropertyPath = Property;
				Module.Distributions.push_back(std::move(Distribution));
			}
			Element.SourceRecipe.Modules.push_back(std::move(Module));
		};

		EFFECT_DOCUMENT_DESC ParticleCompiler;
		ParticleCompiler.strEffectAssetId =
			"effect.track_a.reimport.particle.fixture";
		ParticleCompiler.strDisplayName = "Track A Particle compiler fixture";
		EFFECT_ELEMENT_DESC ParticleSupport;
		ParticleSupport.strElementId = "particle.fixture.support";
		ParticleSupport.strDisplayName = "Visible support";
		ParticleSupport.strGroupId = "manual.fixture";
		ParticleSupport.eKind = Client::EFFECT_ELEMENT_KIND::DECAL;
		ParticleSupport.ResourceBindings.push_back({ "base", TextureIds[0u] });
		ParticleCompiler.Elements.push_back(ParticleSupport);

		EFFECT_ELEMENT_DESC ParticleTarget;
		ParticleTarget.strElementId = "particle.fixture.target";
		ParticleTarget.strDisplayName = "Compiler Particle target";
		ParticleTarget.strGroupId = "compiler.fixture";
		ParticleTarget.eKind = Client::EFFECT_ELEMENT_KIND::PARTICLE;
		ParticleTarget.ResourceBindings = {
			{ "base", TextureIds[0u] }, { "noise", TextureIds[0u] },
			{ std::string(Client::EFFECT_MESH_SHAPE_SLOT_ID), ModelIds[0u] } };
		ParticleTarget.Material.eRenderProfile =
			Client::EFFECT_RENDER_PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ;
		ParticleTarget.Detail.Transform.vPosition = { 1.f, 2.f, 3.f };
		ParticleTarget.Detail.Particle.iMaxParticles = 111u;
		ParticleTarget.Detail.Color.vColorMultiply = { 1.f, 0.5f, 0.25f, 0.75f };
		ParticleTarget.Detail.Timing.fLifeTimeSeconds = 1.25f;
		ParticleTarget.Detail.UV.vSpeed = { 0.25f, 0.5f };
		ParticleTarget.Detail.Mesh.bUseModelMaterial = false;
		ParticleTarget.SourceRecipe.bEnabled = true;
		ParticleTarget.SourceRecipe.strRendererShape = "mesh";
		ParticleTarget.SourceRecipe.fEmitterDurationSeconds = 0.75f;
		ParticleTarget.SourceRecipe.iEmitterLoopCount = 2u;
		AddPortableModule(ParticleTarget, "fixture.required",
			"particlemodulerequired", { "spawnrate" });
		AddPortableModule(ParticleTarget, "fixture.lifetime",
			"particlemodulelifetime", { "lifetime" });
		AddPortableModule(ParticleTarget, "fixture.size",
			"particlemodulesize", { "startsize" });
		AddPortableModule(ParticleTarget, "fixture.spawn",
			"particlemodulespawn", { "rate", "ratescale" });
		AddPortableModule(ParticleTarget, "fixture.mesh",
			"particlemoduletypedatamesh", {});
		ParticleCompiler.Elements.push_back(ParticleTarget);

		EFFECT_DOCUMENT_DESC ParticleExisting = ParticleCompiler;
		EFFECT_ELEMENT_DESC* ExistingParticleSupport = Find_ExactElement(
			ParticleExisting, ParticleSupport.strElementId);
		EFFECT_ELEMENT_DESC* ExistingParticle = Find_ExactElement(
			ParticleExisting, ParticleTarget.strElementId);
		if (nullptr == ExistingParticleSupport || nullptr == ExistingParticle)
		{
			OutError =
				"Field-aware Particle reimport fixture lost its support or target.";
			return false;
		}
		Client::EFFECT_ACTION_CUE_ATTACHMENT_DESC ArtistAttachment;
		ArtistAttachment.bEnabled = true;
		ArtistAttachment.bFollow = false;
		ArtistAttachment.strSourceAnchorSlotId = "source.root.fixture";
		ArtistAttachment.strRuntimeAnchorSlotId = "runtime.root.fixture";
		ArtistAttachment.fSnapshotRootSourceBasisYawDegrees = 37.5f;
		ArtistAttachment.SocketLocalTransform.vPosition =
			{ 0.125f, -0.25f, 0.5f };
		ArtistAttachment.SocketLocalTransform.vRotationDegrees =
			{ 11.f, 22.f, 33.f };
		ArtistAttachment.SocketLocalTransform.vScale = { 1.25f, 0.75f, 1.5f };
		ExistingParticleSupport->strGroupId = "manual.user";
		ExistingParticleSupport->ActionCueAttachment = ArtistAttachment;
		ExistingParticle->strDisplayName = "User-authored Particle target";
		ExistingParticle->strGroupId = "manual.user";
		ExistingParticle->bVisible = false;
		ExistingParticle->ActionCueAttachment = ArtistAttachment;
		ExistingParticle->TransformInheritance.bEnabled = true;
		ExistingParticle->TransformInheritance.strMasterElementId =
			ParticleSupport.strElementId;
		ExistingParticle->ResourceBindings = {
			{ "base", TextureIds[1u] }, { "noise", TextureIds[1u] },
			{ std::string(Client::EFFECT_MESH_SHAPE_SLOT_ID), ModelIds[1u] } };
		ExistingParticle->Material.eRenderProfile =
			Client::EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ;
		ExistingParticle->Detail.Transform.vPosition = { 9.f, 8.f, 7.f };
		ExistingParticle->Detail.Particle.iMaxParticles = 222u;
		ExistingParticle->Detail.Color.vColorMultiply =
			{ 0.125f, 0.25f, 0.5f, 0.625f };
		ExistingParticle->Detail.Timing.fLifeTimeSeconds = 3.5f;
		ExistingParticle->Detail.UV.vSpeed = { 4.f, 5.f };
		ExistingParticle->SourceRecipe.fEmitterDurationSeconds = 9.f;

		EFFECT_GENERIC_AUTHORED_ELEMENT_REIMPORT_REQUEST ParticleRequest;
		ParticleRequest.strElementId = ParticleTarget.strElementId;
		EFFECT_DOCUMENT_DESC ParticleReimported;
		if (!CEffectDocumentCodec::Build_GenericAuthoredElementReimportStage(
				ParticleCompiler, ParticleExisting, ParticleRequest,
				ParticleReimported, OutError))
		{
			return false;
		}
		const EFFECT_ELEMENT_DESC* ParticleResult = Find_ExactElement(
			ParticleReimported, ParticleRequest.strElementId);
		if (nullptr == ParticleResult ||
			ParticleResult->strDisplayName != ExistingParticle->strDisplayName ||
			ParticleResult->strGroupId != ExistingParticle->strGroupId ||
			ParticleResult->bVisible ||
			ParticleResult->Detail.Transform.vPosition.x != 9.f ||
			ParticleResult->Detail.Transform.vPosition.y != 8.f ||
			ParticleResult->Detail.Transform.vPosition.z != 7.f ||
			ParticleResult->Detail.Particle.iMaxParticles != 222u ||
			ParticleResult->Detail.Color.vColorMultiply.x != 0.125f ||
			ParticleResult->Detail.Color.vColorMultiply.y != 0.25f ||
			ParticleResult->Detail.Color.vColorMultiply.z != 0.5f ||
			ParticleResult->Detail.Color.vColorMultiply.w != 0.625f ||
			ParticleResult->Detail.Timing.fLifeTimeSeconds != 3.5f ||
			ParticleResult->Detail.UV.vSpeed.x != 4.f ||
			ParticleResult->Detail.UV.vSpeed.y != 5.f ||
			!ParticleResult->ActionCueAttachment.bEnabled ||
			ParticleResult->ActionCueAttachment.bFollow ||
			ParticleResult->ActionCueAttachment.strSourceAnchorSlotId !=
				"source.root.fixture" ||
			ParticleResult->ActionCueAttachment.strRuntimeAnchorSlotId !=
				"runtime.root.fixture" ||
			ParticleResult->ActionCueAttachment.
				fSnapshotRootSourceBasisYawDegrees != 37.5f ||
			ParticleResult->ActionCueAttachment.SocketLocalTransform.vPosition.x !=
				0.125f ||
			ParticleResult->ActionCueAttachment.SocketLocalTransform.vPosition.y !=
				-0.25f ||
			ParticleResult->ActionCueAttachment.SocketLocalTransform.vPosition.z !=
				0.5f ||
			ParticleResult->ActionCueAttachment.SocketLocalTransform.
				vRotationDegrees.x != 11.f ||
			ParticleResult->ActionCueAttachment.SocketLocalTransform.
				vRotationDegrees.y != 22.f ||
			ParticleResult->ActionCueAttachment.SocketLocalTransform.
				vRotationDegrees.z != 33.f ||
			ParticleResult->ActionCueAttachment.SocketLocalTransform.vScale.x !=
				1.25f ||
			ParticleResult->ActionCueAttachment.SocketLocalTransform.vScale.y !=
				0.75f ||
			ParticleResult->ActionCueAttachment.SocketLocalTransform.vScale.z !=
				1.5f ||
			!ParticleResult->TransformInheritance.bEnabled ||
			ParticleResult->TransformInheritance.strMasterElementId !=
				ParticleSupport.strElementId ||
			FindAsset(*ParticleResult, "base") != TextureIds[0u] ||
			FindAsset(*ParticleResult, "noise") != TextureIds[0u] ||
			FindAsset(*ParticleResult, Client::EFFECT_MESH_SHAPE_SLOT_ID) !=
				ModelIds[0u] ||
			ParticleResult->Material.eRenderProfile !=
				ParticleTarget.Material.eRenderProfile ||
			ParticleResult->SourceRecipe.fEmitterDurationSeconds != 0.75f ||
			ParticleResult->SourceRecipe.iEmitterLoopCount != 2u)
		{
			OutError = "Field-aware Particle reimport ownership changed.";
			return false;
		}
		if (!Verify_GenericReimportRollbackProbes(ParticleCompiler,
				ParticleExisting, ParticleRequest, OutError))
		{
			return false;
		}

		EFFECT_DOCUMENT_DESC Compiler;
		Compiler.strEffectAssetId = "effect.track_a.reimport.decal.fixture";
		Compiler.strDisplayName = "Track A Decal compiler fixture";
		EFFECT_ELEMENT_DESC Support;
		Support.strElementId = "decal.fixture.support";
		Support.strDisplayName = "Visible support";
		Support.strGroupId = "manual.fixture";
		Support.eKind = Client::EFFECT_ELEMENT_KIND::DECAL;
		Support.ResourceBindings.push_back({ "base", TextureIds[0u] });
		Compiler.Elements.push_back(Support);

		EFFECT_ELEMENT_DESC CompilerTarget = Support;
		CompilerTarget.strElementId = "decal.fixture.target";
		CompilerTarget.strDisplayName = "Compiler target";
		CompilerTarget.bVisible = true;
		CompilerTarget.ResourceBindings = {
			{ "base", TextureIds[0u] }, { "noise", TextureIds[0u] } };
		CompilerTarget.Material.eRenderProfile =
			Client::EFFECT_RENDER_PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ;
		CompilerTarget.Detail.Transform.vPosition = { 1.f, 2.f, 3.f };
		CompilerTarget.Detail.Decal.vSize = { 8.f, 7.f };
		CompilerTarget.Detail.Color.vColorMultiply = { 1.f, 0.5f, 0.25f, 0.75f };
		CompilerTarget.SourceRecipe.bEnabled = true;
		CompilerTarget.SourceRecipe.strRendererShape = "decal";
		CompilerTarget.SourceRecipe.fEmitterDurationSeconds = 0.75f;
		CompilerTarget.SourceRecipe.iEmitterLoopCount = 2u;
		Compiler.Elements.push_back(CompilerTarget);

		EFFECT_DOCUMENT_DESC Existing = Compiler;
		EFFECT_ELEMENT_DESC* ExistingTarget = Find_ExactElement(
			Existing, CompilerTarget.strElementId);
		if (nullptr == ExistingTarget)
		{
			OutError = "Field-aware Decal reimport fixture lost its target.";
			return false;
		}
		ExistingTarget->strDisplayName = "User-authored target";
		ExistingTarget->strGroupId = "manual.user";
		ExistingTarget->bVisible = false;
		ExistingTarget->ResourceBindings = {
			{ "base", TextureIds[1u] }, { "noise", TextureIds[1u] } };
		ExistingTarget->Material.eRenderProfile =
			Client::EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ;
		ExistingTarget->Detail.Transform.vPosition = { 9.f, 8.f, 7.f };
		ExistingTarget->Detail.Decal.vSize = { 3.f, 4.f };
		ExistingTarget->Detail.Color.vColorMultiply =
			{ 0.125f, 0.25f, 0.5f, 0.625f };

		EFFECT_GENERIC_AUTHORED_ELEMENT_REIMPORT_REQUEST Request;
		Request.strElementId = CompilerTarget.strElementId;
		EFFECT_DOCUMENT_DESC Reimported;
		if (!CEffectDocumentCodec::Build_GenericAuthoredElementReimportStage(
				Compiler, Existing, Request, Reimported, OutError))
		{
			return false;
		}
		const EFFECT_ELEMENT_DESC* Result = Find_ExactElement(
			Reimported, Request.strElementId);
		if (nullptr == Result)
		{
			OutError = "Field-aware Decal reimport fixture lost its result.";
			return false;
		}
		if (Result->strDisplayName != ExistingTarget->strDisplayName ||
			Result->strGroupId != ExistingTarget->strGroupId || Result->bVisible ||
			Result->Detail.Transform.vPosition.x != 9.f ||
			Result->Detail.Transform.vPosition.y != 8.f ||
			Result->Detail.Transform.vPosition.z != 7.f ||
			Result->Detail.Decal.vSize.x != 3.f ||
			Result->Detail.Decal.vSize.y != 4.f ||
			Result->Detail.Color.vColorMultiply.x != 0.125f ||
			Result->Detail.Color.vColorMultiply.y != 0.25f ||
			Result->Detail.Color.vColorMultiply.z != 0.5f ||
			Result->Detail.Color.vColorMultiply.w != 0.625f ||
			FindAsset(*Result, "base") != TextureIds[1u] ||
			FindAsset(*Result, "noise") != TextureIds[0u] ||
			Result->Material.eRenderProfile !=
				ExistingTarget->Material.eRenderProfile ||
			Result->SourceRecipe.fEmitterDurationSeconds != 0.75f ||
			Result->SourceRecipe.iEmitterLoopCount != 2u)
		{
			OutError = "Field-aware Decal reimport ownership changed.";
			return false;
		}

		const std::string SentinelCanonical =
			CEffectDocumentCodec::Serialize(Reimported);
		const auto RejectsWithoutCommit = [&](const EFFECT_DOCUMENT_DESC& BadExisting,
			const std::string_view Label)
		{
			EFFECT_DOCUMENT_DESC Sentinel = Reimported;
			std::string ExpectedError;
			if (CEffectDocumentCodec::Build_GenericAuthoredElementReimportStage(
					Compiler, BadExisting, Request, Sentinel, ExpectedError) ||
				ExpectedError.empty() ||
				CEffectDocumentCodec::Serialize(Sentinel) != SentinelCanonical)
			{
				OutError = "Field-aware Decal reimport rollback probe failed: " +
					std::string(Label) + ".";
				return false;
			}
			return true;
		};
		EFFECT_DOCUMENT_DESC MissingBaseCompiler = Compiler;
		EFFECT_DOCUMENT_DESC MissingBase = Existing;
		EFFECT_ELEMENT_DESC* MissingCompilerTarget = Find_ExactElement(
			MissingBaseCompiler, Request.strElementId);
		EFFECT_ELEMENT_DESC* MissingTarget = Find_ExactElement(
			MissingBase, Request.strElementId);
		if (nullptr == MissingCompilerTarget || nullptr == MissingTarget)
		{
			OutError = "Field-aware Decal missing-Base fixture lost its target.";
			return false;
		}
		const auto EraseBase = [](EFFECT_ELEMENT_DESC& Element)
		{
			std::erase_if(Element.ResourceBindings,
				[](const Client::EFFECT_RESOURCE_BINDING_DESC& Binding)
				{
					return Binding.strSlotId == "base";
				});
		};
		EraseBase(*MissingCompilerTarget);
		MissingCompilerTarget->bVisible = false;
		MissingCompilerTarget->Material.Execution.bFailClosed = true;
		EraseBase(*MissingTarget);
		MissingTarget->bVisible = true;
		MissingTarget->Material.Execution.bFailClosed = true;
		EFFECT_DOCUMENT_DESC MissingSentinel = Reimported;
		std::string MissingError;
		if (CEffectDocumentCodec::Build_GenericAuthoredElementReimportStage(
				MissingBaseCompiler, MissingBase, Request,
				MissingSentinel, MissingError) || MissingError.empty() ||
			CEffectDocumentCodec::Serialize(MissingSentinel) != SentinelCanonical)
		{
			OutError =
				"Field-aware Decal visible compiler+existing missing-Base rollback probe failed.";
			return false;
		}

		EFFECT_DOCUMENT_DESC HiddenMissingBase = MissingBase;
		EFFECT_ELEMENT_DESC* HiddenMissingTarget = Find_ExactElement(
			HiddenMissingBase, Request.strElementId);
		if (nullptr == HiddenMissingTarget)
		{
			OutError =
				"Field-aware Decal hidden missing-Base fixture lost its target.";
			return false;
		}
		HiddenMissingTarget->bVisible = false;
		EFFECT_DOCUMENT_DESC HiddenDraft;
		if (!CEffectDocumentCodec::Build_GenericAuthoredElementReimportStage(
				MissingBaseCompiler, HiddenMissingBase, Request,
				HiddenDraft, OutError))
		{
			return false;
		}
		const EFFECT_ELEMENT_DESC* HiddenDraftTarget = Find_ExactElement(
			HiddenDraft, Request.strElementId);
		if (nullptr == HiddenDraftTarget || HiddenDraftTarget->bVisible ||
			!HiddenDraftTarget->Material.Execution.bFailClosed ||
			!FindAsset(*HiddenDraftTarget, "base").empty())
		{
			OutError =
				"Field-aware Decal hidden fail-closed missing-Base draft changed.";
			return false;
		}
		EFFECT_DOCUMENT_DESC UnsafeBase = Existing;
		EFFECT_ELEMENT_DESC* UnsafeTarget = Find_ExactElement(
			UnsafeBase, Request.strElementId);
		if (nullptr == UnsafeTarget || UnsafeTarget->ResourceBindings.empty())
		{
			OutError = "Field-aware Decal unsafe-Base fixture lost its target.";
			return false;
		}
		UnsafeTarget->ResourceBindings.front().strAssetId = "../unsafe.dds";
		if (!RejectsWithoutCommit(UnsafeBase, "unsafe authored Base DDS"))
			return false;
		EFFECT_DOCUMENT_DESC AmbiguousBase = Existing;
		EFFECT_ELEMENT_DESC* AmbiguousTarget = Find_ExactElement(
			AmbiguousBase, Request.strElementId);
		if (nullptr == AmbiguousTarget || AmbiguousTarget->ResourceBindings.empty())
		{
			OutError = "Field-aware Decal ambiguous-Base fixture lost its target.";
			return false;
		}
		AmbiguousTarget->ResourceBindings.push_back(
			AmbiguousTarget->ResourceBindings.front());
		return RejectsWithoutCommit(
			AmbiguousBase, "ambiguous authored Base DDS");
	}

	bool Build_StagedFixture(const std::filesystem::path& Path,
		const std::string& Baseline, const std::string& Candidate,
		STAGED_TARGET_FILE& Out, std::string& OutError)
	{
		DATA_JSON_VALUE Root;
		if (!Parse_Json(Baseline, Root, OutError))
			return false;
		Out.Path = Path;
		Out.strExpectedRawSha256 =
			CEffectRuntimeAuthorityCodec::Compute_Sha256Hex(Baseline);
		Out.strExpectedCanonicalJsonSha256 = Canonical_JsonSha256(Root);
		Out.strCanonicalDocument = Candidate;
		return true;
	}
}

bool ClientFrontendHarness::Run_FourClassTrackAAuthoredMaterializer(
	const std::filesystem::path& BatchPath, const bool bApply,
	const bool bApprovedCandidateWrite,
	FOUR_CLASS_TRACK_A_MATERIALIZER_RESULT& OutResult,
	std::string& OutError)
{
	OutResult = {};
	OutError.clear();
	if (bApply || bApprovedCandidateWrite)
	{
		OutError =
			"The four-class C++ harness is validation-only. Candidate generation "
			"and writes are owned by materialize_four_class_track_a_candidates.py; "
			"validate the receipt-backed documents after that transaction.";
		return false;
	}

	std::filesystem::path RepositoryRoot;
	std::vector<STAGED_TARGET_FILE> Files;
	RESTORATION_RECEIPT Receipt;
	if (!Resolve_RepositoryRoot(BatchPath, RepositoryRoot, OutError))
		return false;
	const std::filesystem::path PhysicalResourceRoot =
		RepositoryRoot / L"Client" / L"Bin" / L"Resources";
	SCOPED_RUNTIME_RESOURCE_ROOT RuntimeResourceRoot;
	if (!RuntimeResourceRoot.Initialize(PhysicalResourceRoot, OutError) ||
		!Test_GenericReimportFieldOwnership(PhysicalResourceRoot, OutError) ||
		!Verify_EarlyOrdinaryPreviewIsolationPreflight(
			RepositoryRoot, OutError))
	{
		return false;
	}
	if (Is_FocusedDrawOnlyRequested())
	{
		OutResult.bApplied = false;
		return true;
	}
	if (!Load_CurrentRestorationCandidates(
			RepositoryRoot, Files, Receipt, OutError) ||
		!Verify_FocusedTrackAActualDrawGate(
			Files, Receipt, RepositoryRoot, OutError))
	{
		return false;
	}
	if (Is_FocusedRepresentativesOnlyRequested())
	{
		OutResult.iTotalCandidateDocumentCount = Receipt.iTargetCount;
		OutResult.bApplied = false;
		return true;
	}
	if (!Verify_OrdinaryRuntimeStageGate(
			Files, Receipt, RepositoryRoot, OutError))
	{
		return false;
	}
	if (Files.size() != Receipt.iTargetCount ||
		Files.size() != EXPECTED_TOTAL_CANDIDATE_COUNT)
	{
		OutError =
			"Receipt-backed validation did not load exactly 101 target files.";
		return false;
	}
	OutResult.iTotalCandidateDocumentCount = Receipt.iTargetCount;
	/* The legacy result shape is retained for CLI compatibility, but every
	   producer/commit counter deliberately remains zero. */
	OutResult.bApplied = false;
	return true;
}

bool ClientFrontendHarness::Test_FourClassTrackAAuthoredMaterializerTransaction(
	std::string& OutError)
{
	OutError.clear();
	std::error_code Error;
	const std::filesystem::path WorkingDirectory =
		std::filesystem::current_path(Error);
	std::filesystem::path RepositoryRoot;
	if (Error || !Resolve_RepositoryRoot(
			WorkingDirectory / L"materializer.fast.fixture",
			RepositoryRoot, OutError))
	{
		if (OutError.empty())
			OutError = "Unable to resolve the focused reimport fixture root.";
		return false;
	}
	const std::filesystem::path PhysicalResourceRoot =
		RepositoryRoot / L"Client" / L"Bin" / L"Resources";
	SCOPED_RUNTIME_RESOURCE_ROOT RuntimeResourceRoot;
	std::vector<STAGED_TARGET_FILE> OrdinaryStageFiles;
	RESTORATION_RECEIPT Receipt;
	if (!RuntimeResourceRoot.Initialize(PhysicalResourceRoot, OutError) ||
		!Test_GenericReimportFieldOwnership(PhysicalResourceRoot, OutError) ||
		!Verify_EarlyOrdinaryPreviewIsolationPreflight(
			RepositoryRoot, OutError))
	{
		return false;
	}
	if (Is_FocusedDrawOnlyRequested())
		return true;
	if (!Load_CurrentRestorationCandidates(
			RepositoryRoot, OrdinaryStageFiles, Receipt, OutError) ||
		!Verify_FocusedTrackAActualDrawGate(
			OrdinaryStageFiles, Receipt, RepositoryRoot, OutError))
	{
		return false;
	}
	if (Is_FocusedRepresentativesOnlyRequested())
		return true;
	if (!Verify_OrdinaryRuntimeStageGate(
			OrdinaryStageFiles, Receipt, RepositoryRoot, OutError))
	{
		return false;
	}

	const std::filesystem::path Root = std::filesystem::temp_directory_path() /
		(L"lostark-four-class-materializer-" +
		 std::to_wstring(GetCurrentProcessId()));
	if (std::filesystem::exists(Root, Error) || Error ||
		!std::filesystem::create_directories(Root, Error) || Error)
	{
		OutError = "Unable to create isolated materializer transaction fixture.";
		return false;
	}
	const auto Cleanup = [&Root]()
	{
		std::error_code Ignore;
		std::filesystem::remove_all(Root, Ignore);
	};

	const std::string BaselineA = "{\"fixture\":\"baseline-a\"}";
	const std::string BaselineB = "{\"fixture\":\"baseline-b\"}";
	const std::string CandidateA = "{\"fixture\":\"candidate-a\"}";
	const std::string CandidateB = "{\"fixture\":\"candidate-b\"}";
	const std::string ExternalB = "{\"fixture\":\"external-b\"}";
	const std::filesystem::path PathA = Root / L"a.effect.json";
	const std::filesystem::path PathB = Root / L"b.effect.json";
	if (!Write_NewFile(PathA, BaselineA, OutError) ||
		!Write_NewFile(PathB, BaselineB, OutError))
	{
		Cleanup();
		return false;
	}
	STAGED_TARGET_FILE FileA;
	STAGED_TARGET_FILE FileB;
	if (!Build_StagedFixture(PathA, BaselineA, CandidateA, FileA, OutError) ||
		!Build_StagedFixture(PathB, BaselineB, CandidateB, FileB, OutError))
	{
		Cleanup();
		return false;
	}

	if (!Remove_IfExists(PathB) || !Write_NewFile(PathB, ExternalB, OutError))
	{
		Cleanup();
		return false;
	}
	std::string ExpectedFailure;
	const bool DriftRejected = !Apply_StagedFilesTransaction(
		{ FileA, FileB }, std::nullopt, ExpectedFailure, false);
	std::string ActualA;
	std::string ActualB;
	std::string ReadError;
	if (!DriftRejected || !Read_File(PathA, ActualA, ReadError) ||
		!Read_File(PathB, ActualB, ReadError) || ActualA != BaselineA ||
		ActualB != ExternalB)
	{
		OutError = "Baseline drift test did not preserve both current targets.";
		Cleanup();
		return false;
	}

	if (!Remove_IfExists(PathB) || !Write_NewFile(PathB, BaselineB, OutError))
	{
		Cleanup();
		return false;
	}
	ExpectedFailure.clear();
	const bool MidCommitRejected = !Apply_StagedFilesTransaction(
		{ FileA, FileB }, 1u, ExpectedFailure, false);
	ActualA.clear();
	ActualB.clear();
	if (!MidCommitRejected || !Read_File(PathA, ActualA, ReadError) ||
		!Read_File(PathB, ActualB, ReadError) || ActualA != BaselineA ||
		ActualB != BaselineB)
	{
		OutError = "Mid-commit failure did not roll back every promoted target.";
		Cleanup();
		return false;
	}

	if (!Apply_StagedFilesTransaction({ FileA, FileB }, std::nullopt,
			OutError, false) || !Read_File(PathA, ActualA, ReadError) ||
		!Read_File(PathB, ActualB, ReadError) || ActualA != CandidateA ||
		ActualB != CandidateB)
	{
		if (OutError.empty())
			OutError = "Successful transaction did not promote both candidates.";
		Cleanup();
		return false;
	}

	const std::filesystem::path PathC = Root / L"c.unified.effect.json";
	const std::filesystem::path PathD = Root / L"d.unified.effect.json";
	STAGED_TARGET_FILE FileC;
	STAGED_TARGET_FILE FileD;
	FileC.Path = PathC;
	FileC.bMustNotExist = true;
	FileC.strCanonicalDocument = CandidateA;
	FileD.Path = PathD;
	FileD.bMustNotExist = true;
	FileD.strCanonicalDocument = CandidateB;
	if (!Write_NewFile(PathD, ExternalB, OutError))
	{
		Cleanup();
		return false;
	}
	ExpectedFailure.clear();
	const bool CreateRaceRejected = !Apply_StagedFilesTransaction(
		{ FileC, FileD }, std::nullopt, ExpectedFailure, false);
	ActualB.clear();
	if (!CreateRaceRejected || std::filesystem::exists(PathC, Error) || Error ||
		!Read_File(PathD, ActualB, ReadError) || ActualB != ExternalB ||
		!Remove_IfExists(PathD))
	{
		OutError = "MUST_NOT_EXIST race did not preserve every candidate path.";
		Cleanup();
		return false;
	}

	ExpectedFailure.clear();
	const bool CreateRollbackRejected = !Apply_StagedFilesTransaction(
		{ FileC, FileD }, 1u, ExpectedFailure, false);
	Error.clear();
	const bool PathCExists = std::filesystem::exists(PathC, Error);
	if (!CreateRollbackRejected || Error || PathCExists ||
		std::filesystem::exists(PathD, Error) || Error)
	{
		OutError = "MUST_NOT_EXIST mid-commit failure left a partial candidate.";
		Cleanup();
		return false;
	}

	if (!Apply_StagedFilesTransaction({ FileC, FileD }, std::nullopt,
			OutError, false) || !Read_File(PathC, ActualA, ReadError) ||
		!Read_File(PathD, ActualB, ReadError) || ActualA != CandidateA ||
		ActualB != CandidateB)
	{
		if (OutError.empty())
			OutError = "MUST_NOT_EXIST transaction did not create both candidates.";
		Cleanup();
		return false;
	}

	std::vector<STAGED_TARGET_FILE> BulkFiles;
	BulkFiles.reserve(EXPECTED_TOTAL_CANDIDATE_COUNT);
	for (std::size_t Index = 0u; Index < EXPECTED_TOTAL_CANDIDATE_COUNT;
		++Index)
	{
		STAGED_TARGET_FILE File;
		std::wostringstream Name;
		Name << L"bulk-" << std::setw(3) << std::setfill(L'0') << Index <<
			L".unified.effect.json";
		File.Path = Root / Name.str();
		File.bMustNotExist = true;
		File.strCanonicalDocument = "{\"candidateIndex\":" +
			std::to_string(Index) + "}";
		BulkFiles.push_back(std::move(File));
	}
	const auto BulkTargetsAbsent = [&BulkFiles]()
	{
		std::error_code ExistsError;
		for (const STAGED_TARGET_FILE& File : BulkFiles)
		{
			if (std::filesystem::exists(File.Path, ExistsError) || ExistsError)
				return false;
		}
		return true;
	};
	ExpectedFailure.clear();
	if (Apply_StagedFilesTransaction(BulkFiles, 13u, ExpectedFailure, false) ||
		!BulkTargetsAbsent())
	{
		OutError = "101-file transaction left candidates after first legacy failure.";
		Cleanup();
		return false;
	}
	ExpectedFailure.clear();
	if (Apply_StagedFilesTransaction(BulkFiles, 100u, ExpectedFailure, false) ||
		!BulkTargetsAbsent())
	{
		OutError = "101-file transaction left candidates after final-file failure.";
		Cleanup();
		return false;
	}
	ExpectedFailure.clear();
	if (Apply_StagedFilesTransaction(BulkFiles, std::nullopt,
			ExpectedFailure, false, 42u) || !BulkTargetsAbsent())
	{
		OutError = "Post-promotion verification failure did not roll back 101 files.";
		Cleanup();
		return false;
	}
	if (!Apply_StagedFilesTransaction(BulkFiles, std::nullopt, OutError, false))
	{
		Cleanup();
		return false;
	}
	for (const STAGED_TARGET_FILE& File : BulkFiles)
	{
		std::string Actual;
		if (!Read_File(File.Path, Actual, ReadError) ||
			Actual != File.strCanonicalDocument)
		{
			OutError = "Successful 101-file transaction changed candidate bytes.";
			Cleanup();
			return false;
		}
	}
	Cleanup();
	return true;
}
