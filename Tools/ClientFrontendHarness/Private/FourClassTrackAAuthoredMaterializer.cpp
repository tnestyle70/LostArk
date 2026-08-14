#include "FourClassTrackAAuthoredMaterializer.h"

#include "DataJson.h"
#include "Effect_AuthoringDocument.h"
#include "Effect_DocumentCodec.h"
#include "Effect_RuntimeAuthority.h"

#include <DirectXMath.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iomanip>
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
#include <Windows.h>

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
		Limits.iMaximumBytes = 16u * 1024u * 1024u;
		Limits.iMaximumDepth = 96u;
		Limits.iMaximumValues = 2'000'000u;
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
		/* The generic seam owns executable Particle timing/modules; the Track A
		   target owns drawable resources and Material.  Some authenticated source
		   emitters intentionally have only mask/noise bindings and cannot form an
		   ordinary drawable Element by themselves.  Join the already authenticated
		   target lanes in this non-committing validation copy instead of inventing
		   a fallback texture or admitting an unresolved source Material. */
		MutableSource->ResourceBindings = TargetBlueprintElement.ResourceBindings;
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
		const EFFECT_ELEMENT_DESC Blueprint = InOutBlueprintElement;
		EFFECT_ELEMENT_DESC Authored = *Imported;
		Authored.strElementId = Blueprint.strElementId;
		Authored.strDisplayName = Blueprint.strDisplayName;
		Authored.strGroupId = Blueprint.strGroupId;
		Authored.strSourceNode.clear();
		Authored.bVisible = Blueprint.bVisible;
		Authored.Renderer = {};
		Authored.ResourceBindings = Blueprint.ResourceBindings;
		Authored.Material = Blueprint.Material;
		Authored.ActionCueAttachment = Blueprint.ActionCueAttachment;
		Authored.TransformInheritance = Blueprint.TransformInheritance;
		Authored.Detail = Blueprint.Detail;
		Authored.Detail.Mesh.vSourceTypeDataRotationDegrees = {};
		Authored.SourcePresentation = {};
		InOutBlueprintElement = std::move(Authored);
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
	MATERIALIZER_BATCH Batch;
	FOUR_CLASS_TRACK_A_MATERIALIZER_RESULT StagedResult;
	std::vector<STAGED_TARGET_FILE> Files;
	if (!Parse_MaterializerBatch(BatchPath, Batch, OutError))
		return false;
	if (bApprovedCandidateWrite && !bApply)
	{
		OutError =
			"--approve-candidate-write is valid only with an explicit --apply.";
		return false;
	}
	if (bApply && !bApprovedCandidateWrite)
	{
		OutError =
			"--apply is refused: the batch targets 101 isolated .unified "
			"candidate files, but this run is dry-run-only. Candidate creation/replacement "
			"requires a separately approved candidate-write gate; catalog and animation "
			"event switching remain a later explicit product-mutation transaction.";
		return false;
	}
	SCOPED_RUNTIME_RESOURCE_ROOT RuntimeResourceRoot;
	if (!RuntimeResourceRoot.Initialize(
			Batch.RepositoryRoot / L"Client" / L"Bin" / L"Resources",
			OutError) ||
		!Build_StagedTargetFiles(Batch, Files, StagedResult, OutError))
	{
		return false;
	}
	if (Files.size() != EXPECTED_TOTAL_CANDIDATE_COUNT)
	{
		OutError = "Materializer did not stage exactly 101 target files.";
		return false;
	}
	for (const STAGED_TARGET_FILE& File : Files)
	{
		if (!Verify_TargetBaseline(File, OutError))
			return false;
	}
	if (bApply)
	{
		if (!Apply_StagedFilesTransaction(Files, std::nullopt, OutError, true))
			return false;
		for (const STAGED_TARGET_FILE& File : Files)
		{
			if (!Verify_StagedDocumentFile(File.Path,
					File.strCanonicalDocument, true, OutError))
			{
				return false;
			}
		}
	}
	StagedResult.iStageCount = Batch.Stages.size();
	StagedResult.iElementPlanCount = Batch.Plans.size();
	StagedResult.bApplied = bApply;
	OutResult = StagedResult;
	return true;
}

bool ClientFrontendHarness::Test_FourClassTrackAAuthoredMaterializerTransaction(
	std::string& OutError)
{
	OutError.clear();
	const std::filesystem::path Root = std::filesystem::temp_directory_path() /
		(L"lostark-four-class-materializer-" +
		 std::to_wstring(GetCurrentProcessId()));
	std::error_code Error;
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
