#pragma once

#include "Client_Defines.h"
#include "DataJson.h"

#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

NS_BEGIN(Client)

inline constexpr uint32_t EFFECT_RUNTIME_CATALOG_DERIVED_VERSION = 3u;
inline constexpr uint32_t EFFECT_RUNTIME_AUTHORITY_FORMAT_VERSION = 1u;
inline constexpr uint32_t EFFECT_SOURCE_RUNTIME_PROGRAM_FORMAT_VERSION = 1u;

struct EFFECT_RUNTIME_EXECUTION_CONTRACT final
{
	std::vector<std::string> ArtifactBindingBlockers;
	std::vector<std::string> ExecutionBlockers;
	bool_t bExecutionAdmission = false;
};

struct EFFECT_RUNTIME_HANDLER_RECEIPT final
{
	std::string strHandlerId;
	std::string strHandlerSha256;
	EFFECT_RUNTIME_EXECUTION_CONTRACT ExecutionContract;
};

enum class EFFECT_RUNTIME_RENDERER_FAMILY : uint8_t
{
	MESH,
	SPRITE,
	DECAL,
	RIBBON,
	LIGHT,
	SCREEN_POST,
	END
};

enum class EFFECT_RUNTIME_LITERAL_KIND : uint8_t
{
	BOOLEAN,
	NUMBER,
	STRING,
	END
};

struct EFFECT_RUNTIME_LITERAL final
{
	std::string strLiteralId;
	std::string strPropertyPath;
	EFFECT_RUNTIME_LITERAL_KIND eKind = EFFECT_RUNTIME_LITERAL_KIND::END;
	bool_t bBoolean = false;
	double fNumber = 0.0;
	std::string strString;
};

struct EFFECT_RUNTIME_DISTRIBUTION_KEY final
{
	double fTime = 0.0;
	std::array<double, 4u> Minimum{};
	std::array<double, 4u> Maximum{};
	std::array<double, 4u> ArriveTangentMinimum{};
	std::array<double, 4u> LeaveTangentMinimum{};
	std::array<double, 4u> ArriveTangentMaximum{};
	std::array<double, 4u> LeaveTangentMaximum{};
	std::string strInterpolation;
};

struct EFFECT_RUNTIME_DISTRIBUTION final
{
	std::string strPayloadDistributionId;
	std::string strPropertyPath;
	std::string strSourceClass;
	std::string strSourceObjectPath;
	uint32_t iComponentCount = 0u;
	uint32_t iOperation = 0u;
	uint32_t iRandomLockAxes = 0u;
	uint32_t iLookupTableChunkSize = 0u;
	uint32_t iLookupTableNumElements = 0u;
	double fLookupTableTimeScale = 0.0;
	double fLookupTableStartTime = 0.0;
	std::array<double, 4u> DefaultMinimum{};
	std::array<double, 4u> DefaultMaximum{};
	std::vector<double> LookupTable;
	std::vector<EFFECT_RUNTIME_DISTRIBUTION_KEY> Keys;
	std::string strReferenceId;
	std::string strOccurrenceId;
	std::string strPayloadStatus;
	std::string strFidelity;
};

struct EFFECT_RUNTIME_DISTRIBUTION_ADAPTER final
{
	std::string strDistributionId;
	std::string strPayloadDistributionId;
	std::string strEvaluatorCapabilityId;
	bool_t bReady = false;
	std::vector<std::string> Blockers;
};

struct EFFECT_RUNTIME_OPCODE final
{
	std::string strOpcodeId;
	std::string strEmitterId;
	uint32_t iOrder = 0u;
	std::string strOpcode;
	std::string strHandlerId;
	std::string strExactSourceClass;
	std::string strSourceObjectId;
	std::string strSourceRecordSha256;
	std::string strPayloadStableId;
	std::string strPayloadClassName;
	std::string strPayloadObjectPath;
	std::string strPayloadSha256;
	std::vector<EFFECT_RUNTIME_LITERAL> Literals;
	std::vector<EFFECT_RUNTIME_DISTRIBUTION> Distributions;
	std::vector<EFFECT_RUNTIME_DISTRIBUTION_ADAPTER> DistributionAdapters;
	std::string strSeedCanonicalJson;
	std::vector<std::string> ImplicitDefaultCanonicalJson;
	bool_t bReady = false;
	std::vector<std::string> Blockers;
};

struct EFFECT_RUNTIME_ACTION_PARAMETER final
{
	std::string strName;
	std::string strKind;
	std::vector<double> Values;
	uint32_t iSourceIndex = 0u;
	uint32_t iSourceValueByteOffset = 0u;
};

struct EFFECT_RUNTIME_EMITTER final
{
	std::string strEmitterId;
	std::string strEvidenceId;
	std::string strSourceOccurrenceId;
	std::string strSourceSystemId;
	std::string strSourceEmitterPath;
	EFFECT_RUNTIME_RENDERER_FAMILY eRenderer =
		EFFECT_RUNTIME_RENDERER_FAMILY::END;
	std::string strSelectedLodPath;
	std::string strSelectedLodNodeId;
	std::string strSelectedLodRecordSha256;
	std::vector<EFFECT_RUNTIME_ACTION_PARAMETER> ActionCueParameters;
	std::vector<std::string> OrderedOpcodeIds;
};

struct EFFECT_SOURCE_RUNTIME_PROGRAM final
{
	std::string strEffectAssetId;
	std::string strSourceExecutionReceiptSha256;
	std::string strSourceExecutionReceiptCanonicalSha256;
	std::string strSourceExecutionReceiptObservedRawSha256;
	std::string strProgramSha256;
	std::vector<EFFECT_RUNTIME_EMITTER> Emitters;
	std::vector<EFFECT_RUNTIME_OPCODE> Opcodes;
	std::vector<EFFECT_RUNTIME_HANDLER_RECEIPT> HandlerReceipts;
	EFFECT_RUNTIME_EXECUTION_CONTRACT ExecutionContract;
	bool_t bSourceIdentitySelfConsistent = false;
	bool_t bRuntimeExecutionAdmission = false;
	bool_t bProductAdmission = false;
};

struct EFFECT_RUNTIME_DERIVED_IDENTITY final
{
	std::string strSourceContractHash;
	std::string strSourceSemanticClosureHash;
	std::string strGeometryContractHash;
	std::string strMaterialContractHash;
	std::string strResourceBindingHash;
	std::string strCompilerInputHash;
};

struct EFFECT_RUNTIME_AUTHORITY_IDENTITY final
{
	std::string strEffectAssetId;
	uint32_t iArtifactRevision = 0u;
	std::string strCompilerRevision;
	EFFECT_RUNTIME_DERIVED_IDENTITY Derived;
	std::string strAuthoringCarrierSha256;
	std::string strAssemblySha256;
	std::string strCompiledArtifactSha256;
	std::string strCompiledReceiptSha256;
	std::string strCompiledIrSha256;
	std::string strCompilerReceiptTokenSha256;
};

struct EFFECT_COMPILED_RUNTIME_DOCUMENT final
{
	EFFECT_RUNTIME_AUTHORITY_IDENTITY Identity;
	EFFECT_RUNTIME_EXECUTION_CONTRACT ExecutionContract;
	std::vector<EFFECT_RUNTIME_HANDLER_RECEIPT> HandlerReceipts;
	uint32_t iOpcodeCount = 0u;
	uint32_t iResourceBindingCount = 0u;
	bool_t bArtifactBindingSelfConsistent = false;
	bool_t bExternalIdentityAuthenticated = false;
	bool_t bArtifactExecutionAdmission = false;
	bool_t bTypedProgramMaterialized = false;
	bool_t bRuntimeExecutionAdmission = false;
	bool_t bProductAdmission = false;
	std::vector<std::string> RuntimeBlockers;
};

class CEffectRuntimeAuthorityCodec final
{
public:
	static bool_t Parse_SourceRuntimeProgram(
		const DATA_JSON_VALUE& Value,
		std::shared_ptr<const EFFECT_SOURCE_RUNTIME_PROGRAM>& OutProgram,
		std::string& strOutError);
	static bool_t Parse_DerivedEntry(
		const DATA_JSON_VALUE& Value,
		std::shared_ptr<const EFFECT_COMPILED_RUNTIME_DOCUMENT>& OutDocument,
		std::string& strOutError);
	static std::string Serialize_CanonicalJson(
		const DATA_JSON_VALUE& Value);
	static std::string Serialize_PrettyJson(
		const DATA_JSON_VALUE& Value);
	static std::string Compute_Sha256Hex(std::string_view Value);
};

NS_END
