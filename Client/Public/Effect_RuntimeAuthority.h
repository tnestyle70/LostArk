#pragma once

#include "Client_Defines.h"
#include "DataJson.h"

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

NS_BEGIN(Client)

inline constexpr uint32_t EFFECT_RUNTIME_CATALOG_DERIVED_VERSION = 3u;
inline constexpr uint32_t EFFECT_RUNTIME_AUTHORITY_FORMAT_VERSION = 1u;

struct EFFECT_RUNTIME_DERIVED_IDENTITY final
{
	std::string strSourceContractHash;
	std::string strSourceSemanticClosureHash;
	std::string strGeometryContractHash;
	std::string strMaterialContractHash;
	std::string strResourceBindingHash;
	std::string strCompilerInputHash;
};

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
