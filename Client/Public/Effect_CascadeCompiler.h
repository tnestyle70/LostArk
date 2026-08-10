#pragma once

#include "Client_Defines.h"
#include "Effect_AuthoringDocument.h"

#include <array>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

NS_BEGIN(Client)

inline constexpr uint32_t EFFECT_CASCADE_INSPECTION_COMPILER_REVISION = 2u;

enum class EFFECT_CASCADE_OPCODE : uint8_t
{
	ACCELERATION,
	CAMERA_OFFSET,
	COLOR,
	COLOR_OVER_LIFE,
	COLOR_SCALE_OVER_LIFE,
	LIFETIME,
	LOCATION,
	LOCATION_DIRECT,
	LOCATION_ON_GROUND,
	LOCATION_PRIMITIVE_CYLINDER,
	LOCATION_PRIMITIVE_CYLINDER_SPIN,
	LOCATION_PRIMITIVE_SPHERE,
	MESH_ROTATION,
	MESH_ROTATION_RATE,
	MESH_ROTATION_RATE_MULTIPLY_LIFE,
	MESH_ROTATION_RATE_OVER_LIFE,
	ORIENTATION_AXIS_LOCK,
	PARAMETER_DYNAMIC,
	REQUIRED,
	ROTATION,
	ROTATION_RATE,
	SIZE,
	SIZE_MULTIPLY_LIFE,
	SPAWN,
	SPAWN_PER_UNIT,
	SUBUV,
	TYPE_DATA_DECAL,
	TYPE_DATA_LIGHT,
	TYPE_DATA_MESH,
	TYPE_DATA_RIBBON,
	VELOCITY,
	VELOCITY_OVER_LIFE,
	END
};

enum class EFFECT_CASCADE_PROPERTY_STORAGE : uint8_t
{
	LITERAL,
	DISTRIBUTION,
	END
};

enum class EFFECT_CASCADE_MODULE_ROLE : uint8_t
{
	REQUIRED,
	MODULE,
	SPAWN,
	TYPE_DATA,
	END
};

enum class EFFECT_CASCADE_PROPERTY_PROVENANCE : uint8_t
{
	SOURCE_TAGGED_PRIMITIVE,
	RAW_DISTRIBUTION_TO_TYPED_SOURCE_RECIPE,
	DETERMINISTIC_REFERENCE_METADATA_JOIN,
	OPAQUE_HEX_METADATA_ONLY,
	OBJECT_REFERENCE_TARGET_OR_CLASS_DEFAULT_UNRESOLVED,
	SEED_ARRAY_SOURCE_DECODED_CONSUMPTION_UNRESOLVED,
	MODULE_DEFAULT_OR_NATIVE_TAIL_UNRESOLVED,
	PINNED_SOURCE_RECORD_PHYSICAL_ABSENT,
	RECORD_DECODED_PACKAGE_IDENTITY_UNPINNED,
	SOURCE_EXACT_PHYSICAL_PACKAGE,
	END
};

enum class EFFECT_CASCADE_LOD_SELECTION_POLICY : uint8_t
{
	FIRST_LOD_ONLY,
	END
};

enum class EFFECT_CASCADE_LOD_FIELD_PROVENANCE : uint8_t
{
	UNRESOLVED_CLASS_DEFAULT,
	END
};

enum class EFFECT_CASCADE_HANDLER_RESULT : uint8_t
{
	STRUCTURE_CONSUMED_EXECUTION_BLOCKED,
	END
};

enum class EFFECT_CASCADE_BLOCKER_REQUIREMENT : uint8_t
{
	BLOCKERS_PROHIBITED,
	MODULE_BLOCKERS_REQUIRED,
	PROPERTY_BLOCKERS_REQUIRED,
	END
};

enum class EFFECT_CASCADE_CLASS_CLASSIFICATION : uint8_t
{
	SUPPORTED_RECEIPT_OPCODE_SCHEMA,
	KNOWN_LEGACY_MIGRATION_GAP,
	UNKNOWN_REJECTED,
	END
};

enum class EFFECT_CASCADE_CLASS_LINEAGE_STATUS : uint8_t
{
	RECEIPT_NORMALIZED_ONLY,
	EXACT_SOURCE_CLASS,
	ALIAS_REQUIRED_EXECUTION_UNAPPROVED,
	EXPLICIT_ALIAS_EXECUTION_UNAPPROVED,
	END
};

enum class EFFECT_CASCADE_PROPERTY_HANDLER_RESULT : uint8_t
{
	SCHEMA_FIELD_CONSUMED_EXECUTION_BLOCKED,
	END
};

struct EFFECT_CASCADE_EMITTER_IDENTITY final
{
	std::string strSourceSystemId;
	std::string strSourceOccurrenceId;
	std::string strElementId;
	std::string strCanonicalId;
	uint64_t iStableReference = 0u;
};

struct EFFECT_CASCADE_MODULE_REFERENCE final
{
	uint32_t iOrder = 0u;
	uint32_t iSourceReferenceIndex = 0u;
	EFFECT_CASCADE_MODULE_ROLE eRole = EFFECT_CASCADE_MODULE_ROLE::END;
	std::string strReceiptRole;
	std::string strSourceObjectId;
	std::string strSourceRecordSha256;
	std::string strModuleStableId;
	std::string strCanonicalId;
	uint64_t iStableReference = 0u;
};

struct EFFECT_CASCADE_PROPERTY_KEY final
{
	std::string strCanonicalPath;
	std::string strCanonicalReferenceId;
	uint64_t iStableSemantic = 0u;
	uint64_t iStableReference = 0u;
};

struct EFFECT_CASCADE_PROPERTY_EVIDENCE final
{
	EFFECT_CASCADE_PROPERTY_KEY Property;
	EFFECT_CASCADE_PROPERTY_STORAGE eStorage =
		EFFECT_CASCADE_PROPERTY_STORAGE::END;
	EFFECT_SOURCE_COVERAGE_STATUS eCoverageStatus =
		EFFECT_SOURCE_COVERAGE_STATUS::END;
	EFFECT_CASCADE_PROPERTY_PROVENANCE eProvenance =
		EFFECT_CASCADE_PROPERTY_PROVENANCE::END;
	EFFECT_CASCADE_BLOCKER_REQUIREMENT eBlockerRequirement =
		EFFECT_CASCADE_BLOCKER_REQUIREMENT::END;
	std::string strReceiptProvenanceToken;
	std::vector<std::string> Blockers;
};

struct EFFECT_CASCADE_DISTRIBUTION_EVIDENCE final
{
	EFFECT_CASCADE_PROPERTY_KEY Property;
	EFFECT_SOURCE_COVERAGE_STATUS eCoverageStatus =
		EFFECT_SOURCE_COVERAGE_STATUS::END;
	EFFECT_CASCADE_PROPERTY_PROVENANCE eProvenance =
		EFFECT_CASCADE_PROPERTY_PROVENANCE::END;
	bool_t bRawPayloadRead = false;
	bool_t bExecutionAllowed = false;
	std::vector<std::string> Blockers;
};

struct EFFECT_CASCADE_PROPERTY_HANDLER_RECEIPT final
{
	EFFECT_CASCADE_PROPERTY_KEY Property;
	EFFECT_CASCADE_PROPERTY_STORAGE eStorage =
		EFFECT_CASCADE_PROPERTY_STORAGE::END;
	EFFECT_CASCADE_PROPERTY_HANDLER_RESULT eResult =
		EFFECT_CASCADE_PROPERTY_HANDLER_RESULT::END;
	std::string strHandlerFieldId;
	bool_t bRequired = false;
};

struct EFFECT_CASCADE_HANDLER_RECEIPT final
{
	EFFECT_CASCADE_HANDLER_RESULT eResult = EFFECT_CASCADE_HANDLER_RESULT::END;
	EFFECT_SOURCE_COVERAGE_STATUS eModuleCoverageStatus =
		EFFECT_SOURCE_COVERAGE_STATUS::END;
	EFFECT_CASCADE_BLOCKER_REQUIREMENT eAggregateBlockerRequirement =
		EFFECT_CASCADE_BLOCKER_REQUIREMENT::END;
	std::string strReceiptNormalizedClass;
	std::string strExactSourceClass;
	std::string strAliasId;
	EFFECT_CASCADE_CLASS_LINEAGE_STATUS eClassLineageStatus =
		EFFECT_CASCADE_CLASS_LINEAGE_STATUS::END;
	std::string strOpcodeSchemaId;
	bool_t bExactClassLineagePreserved = false;
	bool_t bPayloadAccessAllowed = false;
	std::vector<EFFECT_CASCADE_PROPERTY_HANDLER_RECEIPT>
		PropertyConsumption;
	std::vector<std::string> RequiredPropertyReferenceIds;
	std::vector<std::string> ConsumedPropertyReferenceIds;
	std::vector<std::string> Blockers;
};

struct EFFECT_CASCADE_INSPECTION_OPCODE final
{
	EFFECT_CASCADE_OPCODE eOpcode = EFFECT_CASCADE_OPCODE::END;
	EFFECT_CASCADE_MODULE_REFERENCE Reference;
	std::vector<EFFECT_CASCADE_PROPERTY_EVIDENCE> Properties;
	std::vector<uint32_t> DistributionEvidenceIndices;
	EFFECT_CASCADE_HANDLER_RECEIPT HandlerReceipt;
};

struct EFFECT_CASCADE_SELECTED_LOD_EVIDENCE final
{
	uint32_t iArrayIndex = 0u;
	EFFECT_CASCADE_LOD_SELECTION_POLICY ePolicy =
		EFFECT_CASCADE_LOD_SELECTION_POLICY::END;
	std::string strEmitterPath;
	std::string strEmitterNodeId;
	std::string strSelectedLodPath;
	std::string strSelectedLodNodeId;
	std::string strCanonicalLineageId;
	uint64_t iStableReference = 0u;
	EFFECT_CASCADE_LOD_FIELD_PROVENANCE eLevelProvenance =
		EFFECT_CASCADE_LOD_FIELD_PROVENANCE::END;
	EFFECT_CASCADE_LOD_FIELD_PROVENANCE eEnabledProvenance =
		EFFECT_CASCADE_LOD_FIELD_PROVENANCE::END;
	bool_t bIdentityPreserved = false;
	bool_t bExecutionFidelityProven = false;
	std::vector<std::string> Blockers;
};

struct EFFECT_CASCADE_RENDERER_EVIDENCE final
{
	EFFECT_RENDERER_TYPE eType = EFFECT_RENDERER_TYPE::END;
	EFFECT_SOURCE_SPACE eSourceSpace = EFFECT_SOURCE_SPACE::END;
};

struct EFFECT_CASCADE_GEOMETRY_EVIDENCE final
{
	std::string strAssetId;
	std::string strReceiptFileSha256;
	std::string strReceiptSelfSha256;
	f32_t fGeometryPreScale = 1.f;
	std::string strScaleSemantics;
	std::string strSourceStatus;
	bool_t bPayloadIntegrityValid = false;
	bool_t bRuntimeConsumerReady = false;
	std::vector<std::string> EvidenceFlags;
	std::vector<std::string> ChannelConsumptionBlockers;
};

struct EFFECT_CASCADE_INSPECTION_EMITTER final
{
	EFFECT_CASCADE_EMITTER_IDENTITY Identity;
	EFFECT_ELEMENT_KIND eElementKind = EFFECT_ELEMENT_KIND::END;
	EFFECT_CASCADE_SELECTED_LOD_EVIDENCE SelectedLOD;
	EFFECT_CASCADE_RENDERER_EVIDENCE Renderer;
	std::vector<EFFECT_CASCADE_INSPECTION_OPCODE> OrderedOpcodes;
	std::vector<EFFECT_CASCADE_DISTRIBUTION_EVIDENCE> Distributions;
	std::optional<EFFECT_CASCADE_GEOMETRY_EVIDENCE> Geometry;
	bool_t bSourceExecutionAdmission = false;
	std::vector<std::string> Blockers;
};

struct EFFECT_CASCADE_INSPECTION_SYSTEM final
{
	std::string strSourceSystemId;
	uint64_t iStableSemantic = 0u;
	std::vector<EFFECT_CASCADE_INSPECTION_EMITTER> Emitters;
};

struct EFFECT_CASCADE_CLASS_REPORT final
{
	std::string strReceiptClassKey;
	EFFECT_CASCADE_CLASS_CLASSIFICATION eClassification =
		EFFECT_CASCADE_CLASS_CLASSIFICATION::END;
	std::string strOpcodeSchemaId;
	std::string strReasonCode;
};

struct EFFECT_CASCADE_CONSUMPTION_RECEIPT final
{
	uint32_t iSystemCount = 0u;
	uint32_t iEmitterCount = 0u;
	uint32_t iOrderedOpcodeCount = 0u;
	uint32_t iDistributionEvidenceCount = 0u;
	uint32_t iRequiredPropertyCount = 0u;
	uint32_t iConsumedPropertyCount = 0u;
	uint32_t iUnknownClassCount = 0u;
	uint32_t iUnconsumedRequiredPropertyCount = 0u;
	uint32_t iHandlerPropertyReceiptCount = 0u;
	uint32_t iRawPayloadReadCount = 0u;
	uint32_t iExecutableOpcodeCount = 0u;
	uint32_t iBlockerCount = 0u;
	std::array<uint32_t,
		static_cast<size_t>(EFFECT_RENDERER_TYPE::END)> RendererCounts{};
};

struct EFFECT_CASCADE_INSPECTION_IR final
{
	uint32_t iCompilerRevision =
		EFFECT_CASCADE_INSPECTION_COMPILER_REVISION;
	std::string strEffectAssetId;
	std::string strCanonicalDocumentIdentity;
	std::string strInspectionHash;
	std::vector<EFFECT_CASCADE_INSPECTION_SYSTEM> Systems;
	EFFECT_CASCADE_CONSUMPTION_RECEIPT Consumption;
	std::vector<std::string> Blockers;
	bool_t bExecutable = false;
	bool_t bProductAdmission = false;
};

struct EFFECT_CASCADE_INSPECTION_COMPILER_PROBE final
{
	uint64_t iCompileAttemptCount = 0u;
	uint64_t iCompileSuccessCount = 0u;
	uint64_t iCompileFailureCount = 0u;
};

class CEffectCascadeCompiler final
{
public:
	static bool_t Compile_SourceInspection(
		const EFFECT_DOCUMENT_DESC& Document,
		std::string_view ExpectedCanonicalDocumentIdentity,
		std::shared_ptr<const EFFECT_CASCADE_INSPECTION_IR>& OutInspection,
		std::string& strOutError);
	static std::string Build_CanonicalDocumentIdentity(
		const EFFECT_DOCUMENT_DESC& Document);
	static bool_t Matches_InputIdentity(
		const EFFECT_CASCADE_INSPECTION_IR& Inspection,
		std::string_view ExpectedCanonicalDocumentIdentity);
	static EFFECT_CASCADE_CLASS_REPORT Classify_ReceiptClass(
		std::string_view ReceiptClassKey);
	static EFFECT_CASCADE_INSPECTION_COMPILER_PROBE Get_Probe();
};

NS_END
