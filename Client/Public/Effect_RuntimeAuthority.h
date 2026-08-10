#pragma once

#include "Client_Defines.h"
#include "DataJson.h"

#include <array>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

NS_BEGIN(Client)

inline constexpr uint32_t EFFECT_RUNTIME_CATALOG_DERIVED_VERSION = 3u;
inline constexpr uint32_t EFFECT_RUNTIME_AUTHORITY_FORMAT_VERSION = 1u;
inline constexpr uint32_t EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM_FORMAT_VERSION = 1u;

enum class EFFECT_RUNTIME_HANDLER_KIND : uint8_t
{
	SOURCE_MODULE,
	SOURCE_PROPERTY,
	SOURCE_PRIMITIVE,
	RECONSTRUCTED_MODULE,
	DISTRIBUTION,
	MATERIAL_EVALUATOR,
	MATERIAL_POLICY,
};

enum class EFFECT_RUNTIME_MODULE_SELECTION : uint8_t
{
	SOURCE_HANDLER,
	RECONSTRUCTED_HANDLER,
};

enum class EFFECT_RUNTIME_RENDERER_KIND : uint8_t
{
	SPRITE_PARTICLE,
	MESH_PARTICLE,
	DECAL_PARTICLE,
	CASCADE_RIBBON,
	SCREEN_POST,
	LIGHT_PARTICLE,
};

enum class EFFECT_RUNTIME_LITERAL_VARIANT : uint8_t
{
	BOOLEAN,
	F64,
	ENUM_STRING,
};

enum class EFFECT_RUNTIME_DISTRIBUTION_VARIANT : uint8_t
{
	INLINE,
	FLOAT_PARAMETER,
	VECTOR_PARAMETER,
	FLOAT_CURVE,
	EF_MULTIPLY,
};

enum class EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT : uint8_t
{
	BOOLEAN,
	F64,
	F64X4,
	TEXTURE_ID,
	ENUM_STRING,
	SAMPLER_DESCRIPTOR,
};

enum class EFFECT_RUNTIME_MATERIAL_POLICY_DOMAIN : uint8_t
{
	RENDER_STATE,
	STATIC_PERMUTATION,
	SAMPLER_DESCRIPTOR,
};

enum class EFFECT_RUNTIME_TEXTURE_RESOLUTION_STATUS : uint8_t
{
	RESOLVED_EXACT_RUNTIME_ASSET,
	UNRESOLVED_RUNTIME_ASSET,
};

enum class EFFECT_RUNTIME_ACTION_CUE_VALUE_KIND : uint8_t
{
	SCALAR,
	VECTOR,
};

enum class EFFECT_RUNTIME_IMPLICIT_DEFAULT_VARIANT : uint8_t
{
	BOOLEAN,
	RIBBON_DEFAULTS,
	DECAL_DEFAULTS,
	SCREEN_POST_IRRELEVANT,
	POINT_LIGHT_OWNER_REFERENCE,
};

enum class EFFECT_RUNTIME_POINT_LIGHT_VALUE_VARIANT : uint8_t
{
	BOOLEAN,
	F64,
	COLOR_RGBA8,
	GUID128,
};

enum class EFFECT_RUNTIME_EMITTER_DELAY_POLICY : uint8_t
{
	EXPLICIT_REQUIRED_LITERAL,
	RECONSTRUCTED_UE3_ZERO_DISTRIBUTION_DEFAULT,
};

enum class EFFECT_RUNTIME_EMITTER_DURATION_POLICY : uint8_t
{
	EXPLICIT_REQUIRED_LITERAL,
	CURRENT_REVISION_CDO_RECONSTRUCTED_DEFAULT,
};

enum class EFFECT_RUNTIME_DISTRIBUTION_CURVE_INTERPOLATION : uint8_t
{
	CUBIC,
	LINEAR,
};

enum class EFFECT_RUNTIME_DISTRIBUTION_SAMPLE_DOMAIN : uint8_t
{
	DISTRIBUTION_EVALUATOR,
	PARTICLE_PARAMETER_BRANCH,
};

enum class EFFECT_RUNTIME_DISTRIBUTION_SAMPLE_INPUT_VARIANT : uint8_t
{
	TIME_RANDOM_UNITS,
	PARTICLE_PARAMETER_INPUT,
};

enum class EFFECT_RUNTIME_DISTRIBUTION_SAMPLE_BRANCH : uint8_t
{
	PARAMETER_INPUT,
	CONSTANT_FALLBACK,
};

enum class EFFECT_RUNTIME_D3D_DESCRIPTOR_KIND : uint8_t
{
	DEPTH_STENCIL,
	RASTERIZER,
	SAMPLER,
};

struct EFFECT_RUNTIME_PROGRAM_SOURCE_POLICY_FAMILY final
{
	std::string strUpstreamClusterId;
	std::string strUpstreamNativeFamily;
	std::string strPolicyFamilyId;
	std::string strClosureBasis;
	uint32_t iModuleOccurrenceCount = 0u;
	std::vector<std::string> RequiredMutatedOutputs;
	std::vector<std::string> RequiredOracleIds;
	bool_t bSourceExact = false;
	bool_t bExecutionAdmission = false;
};

struct EFFECT_RUNTIME_PROGRAM_MATERIAL_POLICY_FAMILY final
{
	std::string strPolicyFamilyId;
	std::string strClosureBasis;
	std::vector<std::string> RequiredOracleIds;
};

struct EFFECT_RUNTIME_PROGRAM_POLICY_ROUTE final
{
	std::string strApprovalPolicyId;
	uint32_t iApprovalPolicyVersion = 0u;
	std::string strApprovalReceiptSha256;
	std::vector<EFFECT_RUNTIME_PROGRAM_SOURCE_POLICY_FAMILY>
		SourceExecutionFamilies;
	std::vector<EFFECT_RUNTIME_PROGRAM_MATERIAL_POLICY_FAMILY>
		MaterialExecutionFamilies;
	uint32_t iSourcePolicyRowCount = 0u;
	uint32_t iMaterialPolicyRowCount = 0u;
	uint32_t iMaterialArithmeticRowCount = 0u;
	uint32_t iGeometryPolicyRowCount = 0u;
	std::string strSourceExecutionFamilyProjectionSha256;
	std::string strSourcePolicyRowProjectionSha256;
	std::string strMaterialExecutionFamilyProjectionSha256;
	std::string strMaterialPolicyRowProjectionSha256;
	std::string strMaterialArithmeticRowProjectionSha256;
	std::string strGeometryPolicyRowProjectionSha256;
	std::string strSourceCapabilityReceiptSha256;
	std::string strSourceCapabilityCanonicalSha256;
	std::string strMaterialPolicyReceiptSha256;
	std::string strMaterialPolicyCanonicalSha256;
	std::string strBindingSha256;
};

struct EFFECT_RUNTIME_PROGRAM_PARTICLE_POLICY final
{
	double fUniformScaleMultiplier = 1.0;
	double fYawOffsetDegrees = 0.0;
	double fDirectionYawDegrees = 0.0;
	double fInitialSpeedMultiplier = 1.0;
	std::string strPolicySha256;
};

struct EFFECT_RUNTIME_PROGRAM_MATERIAL_EVALUATOR_INPUT final
{
	std::string strSampleId;
	double fTime = 0.0;
	std::array<double, 2u> vUvScale{};
	std::array<double, 4u> vPanRotationAux{};
	std::array<double, 4u> vTexture0{};
	std::array<double, 4u> vTexture1{};
	std::array<double, 4u> vColor{};
	std::array<double, 4u> vParams0{};
	std::array<double, 4u> vParams1{};
};

struct EFFECT_RUNTIME_PROGRAM_MATERIAL_EVALUATOR_CONTRACT final
{
	uint32_t iVersion = 0u;
	std::array<std::string, 10u> OperationOrder{};
	std::vector<EFFECT_RUNTIME_PROGRAM_MATERIAL_EVALUATOR_INPUT> InputSamples;
	double fNumericTolerance = 0.0;
	std::string strFidelity;
	bool_t bSourceExact = false;
	std::string strContractSha256;
};

struct EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM_IDENTITY final
{
	std::string strBuilderAuthorityCommitId;
	std::string strBuilderAuthorityTreeId;
	std::string strCandidateRawSha256;
	std::string strProgramId;
	uint32_t iProgramVersion = 0u;
	std::string strProgramSha256;
};

struct EFFECT_RUNTIME_PROGRAM_ROW_IDENTITY final
{
	std::string strId;
	uint32_t iOrder = 0u;
	std::string strRowSha256;
	bool_t bSourceExact = false;
};

struct EFFECT_RUNTIME_PROGRAM_INPUT_ARTIFACT final
{
	EFFECT_RUNTIME_PROGRAM_ROW_IDENTITY Row;
	std::string strPath;
	std::string strAuthorityCommitId;
	std::string strAuthorityTreeId;
	std::string strBlobId;
	std::string strSchema;
	std::string strVersionField;
	uint32_t iVersionValue = 0u;
	std::string strHashDomain;
	std::string strTrackedTextSha256;
	std::string strCanonicalJsonSha256;
	std::string strSelfHashField;
	std::string strSelfSha256;
};

struct EFFECT_RUNTIME_PROGRAM_HANDLER final
{
	EFFECT_RUNTIME_PROGRAM_ROW_IDENTITY Row;
	EFFECT_RUNTIME_HANDLER_KIND eKind = EFFECT_RUNTIME_HANDLER_KIND::SOURCE_MODULE;
	std::string strImplementationId;
	uint32_t iImplementationVersion = 0u;
	std::string strImplementationSha256;
	std::string strExactSourceClass;
	std::string strVariant;
	std::string strConsumerContract;
	std::string strContractSha256;
};

struct EFFECT_RUNTIME_PROGRAM_ACTION_CUE_VALUE final
{
	std::string strName;
	EFFECT_RUNTIME_ACTION_CUE_VALUE_KIND eKind =
		EFFECT_RUNTIME_ACTION_CUE_VALUE_KIND::SCALAR;
	std::optional<double> fScalarValue;
	std::vector<double> VectorValue;
	int32_t iSourceIndex = 0;
	int32_t iSourceValueByteOffset = 0;
};

struct EFFECT_RUNTIME_PROGRAM_SOCKET_TRANSFORM final
{
	std::array<double, 3u> vPosition{};
	std::array<double, 3u> vRotationDegrees{};
	std::array<double, 3u> vScale{};
};

struct EFFECT_RUNTIME_PROGRAM_CUE_TRANSFORM final
{
	std::array<double, 3u> vSourcePositionUeUnits{};
	std::array<double, 3u> vPosition{};
	std::array<double, 3u> vRotationDegrees{};
	std::array<double, 3u> vScale{};
};

struct EFFECT_RUNTIME_PROGRAM_DETAIL_TRANSFORM final
{
	std::array<double, 3u> vPosition{};
	std::array<double, 3u> vRotationDegrees{};
	std::array<double, 3u> vRevolutionDegreesPerSecond{};
	std::array<double, 3u> vScale{};
	std::array<double, 3u> vVelocityPerSecond{};
	std::string strDecision;
	std::string strConsumptionPolicy;
	std::string strProjectionSha256;
};

struct EFFECT_RUNTIME_PROGRAM_ATTACHMENT final
{
	bool_t bEnabled = false;
	bool_t bFollow = false;
	std::string strSourceAnchorSlotId;
	std::string strRuntimeAnchorSlotId;
	std::string strRuntimeBoneName;
	EFFECT_RUNTIME_PROGRAM_SOCKET_TRANSFORM SocketLocalTransform;
};

struct EFFECT_RUNTIME_PROGRAM_TRANSFORM_INHERITANCE final
{
	bool_t bEnabled = false;
	std::string strMasterEmitterId;
	std::string strDecision;
	std::string strConsumptionPolicy;
	std::string strProjectionSha256;
};

struct EFFECT_RUNTIME_PROGRAM_ANCHOR_REQUEST final
{
	std::string strAnchorRequestId;
	uint32_t iOrder = 0u;
	std::string strSourceKind;
	std::string strSourceModuleId;
	std::string strSourceAnchorSlotId;
	std::string strRuntimeAnchorSlotId;
	std::string strRuntimeBoneName;
	EFFECT_RUNTIME_PROGRAM_SOCKET_TRANSFORM SocketLocalTransform;
	bool_t bFollow = false;
};

struct EFFECT_RUNTIME_PROGRAM_RENDERER_COLOR final
{
	std::array<double, 4u> vOffset{};
	std::array<double, 4u> vMultiply{};
	double fClip = 0.0;
	double fEmissiveIntensity = 0.0;
	double fDistortionIntensity = 0.0;
	bool_t bDistortionOnBaseMaterial = false;
	double fRadialTime = 0.0;
	double fRadialIntensity = 0.0;
};

struct EFFECT_RUNTIME_PROGRAM_RENDERER_UV final
{
	std::array<double, 2u> vStart{};
	std::array<double, 2u> vSpeed{};
	bool_t bWave = false;
	std::array<double, 2u> vWaveAmplitude{};
	double fWaveFrequency = 0.0;
	bool_t bSequence = false;
	bool_t bLoop = false;
	double fSequenceTerm = 0.0;
	uint32_t iTileColumns = 0u;
	uint32_t iTileRows = 0u;
	uint32_t iTileIndex = 0u;
};

struct EFFECT_RUNTIME_PROGRAM_RENDERER_LINEAR_LERP final
{
	bool_t bPosition = false;
	std::array<double, 3u> vEndPosition{};
	bool_t bRotation = false;
	std::array<double, 3u> vEndRotationDegrees{};
	bool_t bRevolution = false;
	std::array<double, 3u> vEndRevolutionDegreesPerSecond{};
	bool_t bScale = false;
	std::array<double, 3u> vEndScale{};
	bool_t bVelocity = false;
	std::array<double, 3u> vEndVelocityPerSecond{};
	bool_t bColorOffset = false;
	std::array<double, 4u> vEndColorOffset{};
	bool_t bColorMultiply = false;
	std::array<double, 4u> vEndColorMultiply{};
	bool_t bEmissiveIntensity = false;
	double fEndEmissiveIntensity = 0.0;
};

struct EFFECT_RUNTIME_PROGRAM_RENDERER_MESH final
{
	bool_t bUseModelMaterial = false;
};

struct EFFECT_RUNTIME_PROGRAM_RENDERER_SPRITE final
{
	bool_t bBillboard = false;
	double fBillboardRollDegrees = 0.0;
};

struct EFFECT_RUNTIME_PROGRAM_RENDERER_DECAL final
{
	std::array<double, 2u> vSize{};
	double fDepth = 0.0;
};

struct EFFECT_RUNTIME_PROGRAM_RENDERER_TRAIL final
{
	uint32_t iMaxPoints = 0u;
	double fPointLifeTimeSeconds = 0.0;
	double fSampleIntervalSeconds = 0.0;
	double fMinimumDistance = 0.0;
	double fStartWidth = 0.0;
	double fEndWidth = 0.0;
	bool_t bFaceCamera = false;
};

struct EFFECT_RUNTIME_PROGRAM_RENDERER_AFTER_IMAGE final
{
	double fSampleIntervalSeconds = 0.0;
	uint32_t iMaxCopies = 0u;
	double fAlphaExponent = 0.0;
};

struct EFFECT_RUNTIME_PROGRAM_RENDERER_SCREEN_POST final
{
	bool_t bEnabled = false;
	std::string strProfileId;
	std::string strStatus;
	double fIntensity = 0.0;
	double fSecondaryIntensity = 0.0;
	double fFrequency = 0.0;
	std::array<double, 4u> vTint{};
	uint32_t iRandomSeed = 0u;
};

struct EFFECT_RUNTIME_PROGRAM_RENDERER_CONFIG final
{
	std::optional<EFFECT_RUNTIME_PROGRAM_RENDERER_COLOR> Color;
	std::optional<EFFECT_RUNTIME_PROGRAM_RENDERER_UV> Uv;
	std::optional<EFFECT_RUNTIME_PROGRAM_RENDERER_LINEAR_LERP> LinearLerp;
	std::optional<EFFECT_RUNTIME_PROGRAM_RENDERER_MESH> Mesh;
	std::optional<EFFECT_RUNTIME_PROGRAM_RENDERER_SPRITE> Sprite;
	std::optional<EFFECT_RUNTIME_PROGRAM_RENDERER_DECAL> Decal;
	std::optional<EFFECT_RUNTIME_PROGRAM_RENDERER_TRAIL> Trail;
	std::optional<EFFECT_RUNTIME_PROGRAM_RENDERER_AFTER_IMAGE> AfterImage;
	std::optional<EFFECT_RUNTIME_PROGRAM_RENDERER_SCREEN_POST> ScreenPost;
	std::string strSourceProjectionSha256;
	std::string strFidelity;
	bool_t bSourceExact = false;
	std::string strConsumptionPolicy;
	std::vector<std::string> Blockers;
};

struct EFFECT_RUNTIME_PROGRAM_BURST final
{
	std::string strSpawnModuleId;
	uint32_t iBurstIndex = 0u;
	double fTimeSeconds = 0.0;
	uint32_t iCountMinimum = 0u;
	uint32_t iCountMaximum = 0u;
	std::string strSourceLiteralProjectionSha256;
};

struct EFFECT_RUNTIME_PROGRAM_TIMING final
{
	std::string strRequiredModuleId;
	std::string strSpawnModuleId;
	std::string strLifetimeModuleId;
	double fEmitterDelaySeconds = 0.0;
	EFFECT_RUNTIME_EMITTER_DELAY_POLICY eEmitterDelayPolicy =
		EFFECT_RUNTIME_EMITTER_DELAY_POLICY::EXPLICIT_REQUIRED_LITERAL;
	double fEmitterDurationSeconds = 0.0;
	EFFECT_RUNTIME_EMITTER_DURATION_POLICY eEmitterDurationPolicy =
		EFFECT_RUNTIME_EMITTER_DURATION_POLICY::EXPLICIT_REQUIRED_LITERAL;
	uint32_t iEmitterLoopCount = 0u;
	std::vector<EFFECT_RUNTIME_PROGRAM_BURST> Bursts;
	std::string strSourceProjectionSha256;
	std::string strFidelity;
	bool_t bSourceExact = false;
	std::vector<std::string> Blockers;
	std::string strTimingSha256;
};

struct EFFECT_RUNTIME_PROGRAM_RANDOM final
{
	std::string strPolicyId;
	std::string strSeedDerivationInputSha256;
	uint32_t iEmitterRandomSeed = 0u;
	bool_t bSourceExact = false;
	std::vector<std::string> Blockers;
	std::string strPolicySha256;
};

struct EFFECT_RUNTIME_PROGRAM_ADAPTER final
{
	std::string strAdapterId;
	bool_t bEnabled = false;
	bool_t bSourceExact = false;
	std::vector<std::string> Blockers;
	std::string strAdapterSha256;
};

struct EFFECT_RUNTIME_PROGRAM_SCREEN_POST_ADAPTER final
{
	EFFECT_RUNTIME_PROGRAM_ADAPTER Common;
	std::string strOccurrenceId;
	std::string strMaterialOccurrenceId;
	std::string strRecipeId;
	std::string strFamilyId;
	std::string strIntensityDistributionId;
	std::string strAlphaDistributionId;
	double fSecondaryIntensity = 0.0;
	std::string strFrequencyPolicy;
	std::array<double, 4u> vTint{};
	std::string strSourceSpace;
	std::string strFidelity;
};

struct EFFECT_RUNTIME_PROGRAM_LIGHT_ADAPTER final
{
	EFFECT_RUNTIME_PROGRAM_ADAPTER Common;
	std::string strModuleId;
	std::vector<std::string> FieldIds;
	std::string strPositionSourcePolicy;
	double fUeUnitScale = 0.0;
};

struct EFFECT_RUNTIME_PROGRAM_DECAL_ADAPTER final
{
	EFFECT_RUNTIME_PROGRAM_ADAPTER Common;
	std::string strModuleId;
	std::string strDefaultId;
	std::string strSizeDistributionId;
	double fNearPlane = 0.0;
	double fFarPlane = 0.0;
	std::array<double, 2u> vDefaultSize{};
	std::array<double, 2u> vBlendRange{};
	bool_t bYawOnlyCdoDefault = false;
	bool_t bYawOnlyCapabilityOutput = false;
	std::string strYawOnlyDecision;
	bool_t bExecutionAdmission = false;
	bool_t bSupports3dDrawMode = false;
	double fDepthWorldUnits = 0.0;
	double fDepthRuntimeUnits = 0.0;
	std::string strFidelity;
};

struct EFFECT_RUNTIME_PROGRAM_RIBBON_ADAPTER final
{
	EFFECT_RUNTIME_PROGRAM_ADAPTER Common;
	std::string strTypeDataModuleId;
	std::string strDefaultId;
	std::string strSpawnModuleId;
	std::string strLifetimeModuleId;
	std::string strSizeModuleId;
	std::vector<std::string> ColorModuleIds;
	std::string strDynamicParameterModuleId;
	std::vector<std::string> SpawnDistributionIds;
	std::vector<std::string> LifetimeDistributionIds;
	std::vector<std::string> SizeDistributionIds;
	std::vector<std::string> ColorDistributionIds;
	std::vector<std::string> DynamicDistributionIds;
	double fTilingDistance = 0.0;
	double fDistanceTessellationStepSize = 0.0;
	uint32_t iTypedMaxParticleInTrailCount = 0u;
	uint32_t iOperationalMaxPoints = 0u;
	std::string strWidthPolicy;
	std::string strGeometryPolicy;
	std::string strOrientationPolicy;
	std::string strFidelity;
};

struct EFFECT_RUNTIME_PROGRAM_EMITTER final
{
	EFFECT_RUNTIME_PROGRAM_ROW_IDENTITY Row;
	std::string strEvidenceId;
	std::string strSourceOccurrenceId;
	std::string strSourceSystemId;
	std::string strSourceEmitterPath;
	std::string strSourceEmitterNodeId;
	std::string strSourceCueId;
	std::string strSourceElementId;
	std::string strSourceNode;
	std::string strSourceActionCueProjectionSha256;
	bool_t bVisible = false;
	EFFECT_RUNTIME_RENDERER_KIND eRenderer =
		EFFECT_RUNTIME_RENDERER_KIND::SPRITE_PARTICLE;
	std::string strRendererSourceSpace;
	std::string strSelectedLodPath;
	std::string strSelectedLodNodeId;
	std::string strSelectedLodRecordSha256;
	std::string strSelectedLodDecision;
	std::string strSelectedLodOracleId;
	bool_t bSourceRecipeEnabled = false;
	bool_t bLocalSpace = false;
	std::string strSizeUnitPolicy;
	uint32_t iOperationalMaxParticles = 0u;
	uint32_t iSourcePeakActiveParticles = 0u;
	std::string strSpawnRateFallbackPolicy;
	std::string strLifetimeFallbackPolicy;
	std::string strOperationalCapPolicy;
	std::string strOperationalCapProjectionSha256;
	std::vector<std::string> OperationalCapBlockers;
	EFFECT_RUNTIME_PROGRAM_RENDERER_CONFIG RendererRuntimeConfig;
	std::optional<EFFECT_RUNTIME_PROGRAM_SCREEN_POST_ADAPTER> ScreenPostAdapter;
	std::optional<EFFECT_RUNTIME_PROGRAM_LIGHT_ADAPTER> LightAdapter;
	std::optional<EFFECT_RUNTIME_PROGRAM_DECAL_ADAPTER> DecalAdapter;
	std::optional<EFFECT_RUNTIME_PROGRAM_RIBBON_ADAPTER> RibbonAdapter;
	EFFECT_RUNTIME_PROGRAM_TIMING Timing;
	EFFECT_RUNTIME_PROGRAM_RANDOM Random;
	EFFECT_RUNTIME_PROGRAM_CUE_TRANSFORM CueLocalTransform;
	EFFECT_RUNTIME_PROGRAM_DETAIL_TRANSFORM DetailTransform;
	EFFECT_RUNTIME_PROGRAM_ATTACHMENT ActionCueAttachment;
	EFFECT_RUNTIME_PROGRAM_TRANSFORM_INHERITANCE TransformInheritance;
	std::vector<std::string> TransformCompositionOrder;
	std::vector<EFFECT_RUNTIME_PROGRAM_ANCHOR_REQUEST> AnchorRequests;
	std::vector<std::string> ModuleIds;
	std::vector<EFFECT_RUNTIME_PROGRAM_ACTION_CUE_VALUE> ActionCueParameterInputs;
	std::string strScheduleId;
	std::optional<std::string> strMaterialOccurrenceId;
	std::vector<std::string> TextureResourceIds;
	std::optional<std::string> strGeometryUseId;
};

struct EFFECT_RUNTIME_PROGRAM_ACTION_SCHEDULE final
{
	EFFECT_RUNTIME_PROGRAM_ROW_IDENTITY Row;
	std::string strSourceCueId;
	std::string strSourceOccurrenceId;
	std::string strSourceSystemId;
	uint32_t iSourceReceiptEventIndex = 0u;
	double fGlobalTimeSeconds = 0.0;
	double fDurationSeconds = 0.0;
	std::string strSourceCueRowSha256;
};

struct EFFECT_RUNTIME_PROGRAM_CAPABILITY_SAMPLE final
{
	std::string strSampleId;
	uint32_t iOrder = 0u;
	std::string strOwnerModuleId;
	double fTime = 0.0;
	int32_t iFixedSeed = 0;
	std::string strFixedSeedSource;
	std::array<double, 4u> RandomUnits{};
	std::string strInputVariant;
	std::string strInputLiteralProjectionSha256;
	std::string strInputDistributionProjectionSha256;
	std::vector<double> InputValues;
	std::string strOutputVariant;
	std::vector<double> OutputValues;
	std::string strTypedInputSha256;
	std::string strOutputSha256;
	double fAbsoluteTolerance = 0.0;
	double fRelativeTolerance = 0.0;
};

struct EFFECT_RUNTIME_PROGRAM_APPROVAL_SOURCE_RECEIPT final
{
	std::string strPolicyRowId;
	std::string strUpstreamIdentitySha256;
	std::string strPolicyBindingSha256;
	std::string strExactSourceClass;
	std::string strModuleOccurrenceId;
	std::string strUpstreamClusterId;
	std::string strRequiredMutatedOutput;
	std::string strUpstreamDecision;
	std::string strPolicyFamilyId;
	std::string strEvidenceFidelity;
	std::string strExecutionFidelity;
	bool_t bSourceExact = false;
	std::vector<std::string> PreservedEvidenceBlockers;
	std::vector<std::string> PolicyFidelityGuards;
	std::vector<std::string> RequiredOracleIds;
	std::vector<std::string> ExecutionBlockers;
	bool_t bExecutionAdmission = false;
	bool_t bProductAdmission = false;
};

struct EFFECT_RUNTIME_PROGRAM_MODULE final
{
	EFFECT_RUNTIME_PROGRAM_ROW_IDENTITY Row;
	std::string strEmitterId;
	std::string strSourceObjectId;
	std::string strSourceRecordSha256;
	std::string strExactSourceClass;
	EFFECT_RUNTIME_MODULE_SELECTION eSelection =
		EFFECT_RUNTIME_MODULE_SELECTION::SOURCE_HANDLER;
	std::string strHandlerRegistryId;
	std::string strHandlerVariant;
	std::string strCapabilityPolicyFamilyId;
	std::string strCapabilityImplementationId;
	uint32_t iCapabilityImplementationVersion = 0u;
	std::string strCapabilityImplementationSha256;
	std::string strCapabilityFamilySemanticSha256;
	std::string strCapabilityInputSchemaSha256;
	std::string strCapabilityOutputSchemaSha256;
	std::string strCapabilityDefaultPolicySha256;
	std::vector<EFFECT_RUNTIME_PROGRAM_CAPABILITY_SAMPLE> CapabilityNumericSamples;
	std::string strCapabilitySourceRowSha256;
	std::string strCapabilityLiteralBindingsSha256;
	std::string strCapabilityDistributionBindingsSha256;
	std::string strCapabilityPropertyConsumptionSha256;
	std::string strCapabilitySeedBindingSha256;
	std::vector<std::string> CapabilityActionCueInputNames;
	std::string strCapabilityActionCueInputsSha256;
	std::string strApprovalPolicyRowId;
	std::string strApprovalUpstreamIdentitySha256;
	std::string strApprovalPolicyBindingSha256;
	std::string strApprovalRequiredMutatedOutput;
	int32_t iApprovalSourceOrder = -1;
	std::vector<std::string> ApprovalRequiredOracleIds;
	std::vector<std::string> ApprovalExecutionBlockers;
	std::optional<EFFECT_RUNTIME_PROGRAM_APPROVAL_SOURCE_RECEIPT>
		ApprovalSourceReceipt;
	std::string strApprovalSourceRowSha256;
	std::vector<std::string> PropertyIds;
	std::vector<std::string> PrimitiveLeafIds;
	std::vector<std::string> LiteralIds;
	std::vector<std::string> DistributionIds;
	std::string strSeedPolicyId;
	std::vector<std::string> ImplicitDefaultIds;
	std::string strSourceDecision;
	std::vector<std::string> PreservedBlockers;
};

struct EFFECT_RUNTIME_PROGRAM_PROPERTY final
{
	EFFECT_RUNTIME_PROGRAM_ROW_IDENTITY Row;
	std::string strModuleId;
	std::string strPropertyPath;
	std::string strHandlerRegistryId;
	std::string strConsumptionDecision;
	std::string strIrrelevanceOracleId;
	std::vector<std::string> PayloadLiteralIds;
	std::vector<std::string> PayloadDistributionIds;
	std::vector<std::string> SemanticDistributionIds;
	std::string strSourceFidelity;
	std::string strCapabilityConsumptionDecision;
	std::string strSemanticRole;
	bool_t bOutputDependencyRequired = false;
	std::string strCapabilitySourceRowSha256;
	std::vector<std::string> PreservedBlockers;
};

struct EFFECT_RUNTIME_PROGRAM_PRIMITIVE_LEAF final
{
	EFFECT_RUNTIME_PROGRAM_ROW_IDENTITY Row;
	std::string strModuleId;
	std::string strPropertyId;
	std::string strPropertyPath;
	std::string strTopLevelPropertyPath;
	std::string strLiteralId;
	EFFECT_RUNTIME_LITERAL_VARIANT eValueVariant =
		EFFECT_RUNTIME_LITERAL_VARIANT::F64;
	std::string strHandlerRegistryId;
	std::string strConsumptionDecision;
	std::vector<std::string> PreservedBlockers;
};

struct EFFECT_RUNTIME_PROGRAM_LITERAL final
{
	EFFECT_RUNTIME_PROGRAM_ROW_IDENTITY Row;
	std::string strModuleId;
	std::string strPropertyId;
	std::string strPropertyPath;
	EFFECT_RUNTIME_LITERAL_VARIANT eVariant =
		EFFECT_RUNTIME_LITERAL_VARIANT::F64;
	std::optional<bool_t> bValue;
	std::optional<double> fValue;
	std::string strEnumValue;
};

struct EFFECT_RUNTIME_PROGRAM_DISTRIBUTION_CURVE_KEY final
{
	double fTime = 0.0;
	std::array<double, 4u> vMinimum{};
	std::array<double, 4u> vMaximum{};
	std::array<double, 4u> vArriveTangentMinimum{};
	std::array<double, 4u> vLeaveTangentMinimum{};
	std::array<double, 4u> vArriveTangentMaximum{};
	std::array<double, 4u> vLeaveTangentMaximum{};
	EFFECT_RUNTIME_DISTRIBUTION_CURVE_INTERPOLATION eInterpolation =
		EFFECT_RUNTIME_DISTRIBUTION_CURVE_INTERPOLATION::CUBIC;
};

struct EFFECT_RUNTIME_PROGRAM_DISTRIBUTION_FIELD_PROVENANCE final
{
	std::string strFieldPath;
	std::string strProvenanceTier;
	std::string strEvidenceStatus;
	std::string strValueSha256;
	std::string strTypedValueBindingSha256;
};

struct EFFECT_RUNTIME_PROGRAM_PARAMETER_INPUT final
{
	std::string strName;
	EFFECT_RUNTIME_ACTION_CUE_VALUE_KIND eKind =
		EFFECT_RUNTIME_ACTION_CUE_VALUE_KIND::SCALAR;
	std::optional<double> fScalarValue;
	std::vector<double> VectorValue;
	int32_t iSourceIndex = 0;
	int32_t iSourceValueByteOffset = 0;
};

struct EFFECT_RUNTIME_PROGRAM_DISTRIBUTION_SAMPLE final
{
	std::string strSampleId;
	EFFECT_RUNTIME_DISTRIBUTION_SAMPLE_DOMAIN eDomain =
		EFFECT_RUNTIME_DISTRIBUTION_SAMPLE_DOMAIN::DISTRIBUTION_EVALUATOR;
	EFFECT_RUNTIME_DISTRIBUTION_SAMPLE_INPUT_VARIANT eInputVariant =
		EFFECT_RUNTIME_DISTRIBUTION_SAMPLE_INPUT_VARIANT::TIME_RANDOM_UNITS;
	double fTime = 0.0;
	std::vector<double> RandomUnits;
	std::optional<std::string> strSourceCueId;
	std::optional<EFFECT_RUNTIME_DISTRIBUTION_SAMPLE_BRANCH> eBranch;
	std::optional<EFFECT_RUNTIME_PROGRAM_PARAMETER_INPUT> ParameterInput;
	std::optional<double> fDiagnosticStandardBaseValue;
	std::optional<bool_t> bBlocked;
	std::string strActionCueBindingsSha256;
	std::string strInputSha256;
	std::string strOutputSha256;
	std::vector<double> OutputValues;
	double fAbsoluteTolerance = 0.0;
	double fRelativeTolerance = 0.0;
};

struct EFFECT_RUNTIME_PROGRAM_DISTRIBUTION final
{
	EFFECT_RUNTIME_PROGRAM_ROW_IDENTITY Row;
	std::string strModuleId;
	std::string strPropertyId;
	EFFECT_RUNTIME_DISTRIBUTION_VARIANT eVariant =
		EFFECT_RUNTIME_DISTRIBUTION_VARIANT::INLINE;
	std::string strEvaluatorRegistryId;
	std::string strPayloadDistributionId;
	std::string strPropertyPath;
	std::string strSourceClass;
	std::string strSourceObjectPath;
	std::string strReferenceId;
	std::string strOccurrenceId;
	std::string strPayloadStatus;
	std::string strFidelity;
	uint32_t iComponentCount = 0u;
	std::optional<uint32_t> iOperation;
	std::optional<uint32_t> iRandomLockAxes;
	std::optional<uint32_t> iLookupTableChunkSize;
	std::optional<uint32_t> iLookupTableNumElements;
	std::optional<double> fLookupTableTimeScale;
	std::optional<double> fLookupTableStartTime;
	std::vector<double> DefaultMinimum;
	std::vector<double> DefaultMaximum;
	std::vector<double> LookupTable;
	std::vector<EFFECT_RUNTIME_PROGRAM_DISTRIBUTION_CURVE_KEY> CurveKeys;
	std::string strParameterName;
	std::vector<std::string> ParamModes;
	std::vector<double> MinimumInput;
	std::vector<double> MaximumInput;
	std::vector<double> MinimumOutput;
	std::vector<double> MaximumOutput;
	std::vector<double> ConstantValues;
	std::optional<bool_t> bIsDirty;
	std::vector<EFFECT_RUNTIME_PROGRAM_ACTION_CUE_VALUE> ActionCueBindings;
	std::vector<EFFECT_RUNTIME_PROGRAM_DISTRIBUTION_FIELD_PROVENANCE>
		FieldProvenance;
	std::vector<EFFECT_RUNTIME_PROGRAM_DISTRIBUTION_SAMPLE> Samples;
	std::string strCapabilityImplementationId;
	uint32_t iCapabilityImplementationVersion = 0u;
	std::string strCapabilityImplementationSha256;
	std::vector<std::string> PreservedBlockers;
};

struct EFFECT_RUNTIME_PROGRAM_SEED_POLICY final
{
	EFFECT_RUNTIME_PROGRAM_ROW_IDENTITY Row;
	std::string strModuleId;
	std::string strEvaluatorId;
	std::vector<int32_t> RandomSeeds;
	std::optional<std::string> strParameterName;
	bool_t bGetSeedFromInstance = false;
	bool_t bInstanceSeedIsIndex = false;
	bool_t bResetSeedOnEmitterLooping = false;
	bool_t bRandomlySelectSeedArray = false;
	bool_t bEmptyArrayUsesOccurrenceRandomStream = false;
	std::string strCurrentCdoEvidenceKey;
	std::string strSource;
	std::string strSourceFidelity;
};

struct EFFECT_RUNTIME_PROGRAM_RIBBON_DEFAULTS final
{
	uint32_t iMaxTessellationBetweenParticles = 0u;
	uint32_t iSheetsPerTrail = 0u;
	uint32_t iMaxTrailCount = 0u;
	uint32_t iMaxParticleInTrailCount = 0u;
	bool_t bDeadTrailsOnDeactivate = false;
	bool_t bDeadTrailsOnSourceLoss = false;
	bool_t bClipSourceSegment = false;
	bool_t bEnablePreviousTangentRecalculation = false;
	bool_t bRenderGeometry = false;
	double fDistanceTessellationStepSize = 0.0;
	double fTangentTessellationScalar = 0.0;
};

struct EFFECT_RUNTIME_PROGRAM_DECAL_DEFAULTS final
{
	std::array<double, 2u> vDefaultSize{};
	double fFarPlane = 0.0;
	std::array<double, 2u> vBlendRange{};
	bool_t bOnlyCalculateRotationYaw = false;
	bool_t bSupports3dDrawMode = false;
};

struct EFFECT_RUNTIME_PROGRAM_IMPLICIT_DEFAULT final
{
	EFFECT_RUNTIME_PROGRAM_ROW_IDENTITY Row;
	std::string strModuleId;
	std::string strFamily;
	std::string strFieldPath;
	EFFECT_RUNTIME_IMPLICIT_DEFAULT_VARIANT eVariant =
		EFFECT_RUNTIME_IMPLICIT_DEFAULT_VARIANT::BOOLEAN;
	std::optional<bool_t> bValue;
	std::optional<EFFECT_RUNTIME_PROGRAM_RIBBON_DEFAULTS> RibbonValues;
	std::optional<EFFECT_RUNTIME_PROGRAM_DECAL_DEFAULTS> DecalValues;
	std::string strDecision;
	std::string strProvenance;
	std::string strReason;
	std::vector<std::string> ValuesOwnedBy;
};

struct EFFECT_RUNTIME_PROGRAM_POINT_LIGHT_FIELD final
{
	EFFECT_RUNTIME_PROGRAM_ROW_IDENTITY Row;
	std::string strModuleId;
	std::string strFieldPath;
	EFFECT_RUNTIME_POINT_LIGHT_VALUE_VARIANT eVariant =
		EFFECT_RUNTIME_POINT_LIGHT_VALUE_VARIANT::BOOLEAN;
	std::optional<bool_t> bValue;
	std::optional<double> fValue;
	std::optional<std::array<uint32_t, 4u>> ColorRgba8Value;
	std::string strGuid128Value;
	std::string strSourceTier;
	std::string strSourceFidelity;
	std::string strDecision;
	std::string strOracleId;
};

struct EFFECT_RUNTIME_PROGRAM_MATERIAL_FAMILY_SAMPLE final
{
	std::string strSampleId;
	uint32_t iOrder = 0u;
	std::string strInputSha256;
	std::array<double, 4u> vExpected{};
};

struct EFFECT_RUNTIME_PROGRAM_MATERIAL_RECIPE_SAMPLE final
{
	std::string strSampleId;
	uint32_t iOrder = 0u;
	double fTime = 0.0;
	std::array<double, 2u> vUvScale{};
	std::array<double, 4u> vPanRotationAux{};
	std::array<double, 4u> vTexture0{};
	std::array<double, 4u> vTexture1{};
	std::array<double, 4u> vColor{};
	std::array<double, 4u> vParams0{};
	std::array<double, 4u> vParams1{};
	std::string strInputSha256;
	std::array<double, 4u> vExpected{};
};

struct EFFECT_RUNTIME_PROGRAM_APPROVAL_ARITHMETIC_RECEIPT final
{
	std::string strPolicyRowId;
	std::string strUpstreamFamilyId;
	std::string strFamilyIdentitySha256;
	std::string strEvaluatorId;
	uint32_t iEvaluatorVersion = 0u;
	std::string strEvaluatorSha256;
	std::string strPolicyFamilyId;
	std::string strEvidenceFidelity;
	bool_t bSourceExact = false;
	bool_t bCpuNumericOracleVerified = false;
	bool_t bHlslNumericOracleVerified = false;
	std::vector<std::string> PreservedEvidenceBlockers;
	std::vector<std::string> RequiredOracleIds;
	std::vector<std::string> ExecutionBlockers;
	bool_t bExecutionAdmission = false;
	bool_t bProductAdmission = false;
};

struct EFFECT_RUNTIME_PROGRAM_MATERIAL_FAMILY final
{
	EFFECT_RUNTIME_PROGRAM_ROW_IDENTITY Row;
	std::string strFamilyIdentitySha256;
	std::string strEvaluatorRegistryId;
	std::string strEvaluatorId;
	uint32_t iEvaluatorVersion = 0u;
	std::string strEvaluatorSha256;
	std::vector<std::string> RendererShapes;
	uint32_t iFeatureMask = 0u;
	std::vector<std::string> Features;
	std::string strGraphProvenance;
	bool_t bCpuNumericOracleVerified = false;
	bool_t bHlslNumericOracleVerified = false;
	std::vector<EFFECT_RUNTIME_PROGRAM_MATERIAL_FAMILY_SAMPLE> NumericSamples;
	std::string strSampleProjectionSha256;
	std::string strApprovalPolicyRowId;
	uint32_t iApprovalArithmeticOrder = 0u;
	std::string strApprovalPolicyFamilyId;
	std::vector<std::string> ApprovalRequiredOracleIds;
	std::vector<std::string> ApprovalExecutionBlockers;
	EFFECT_RUNTIME_PROGRAM_APPROVAL_ARITHMETIC_RECEIPT ApprovalArithmeticReceipt;
	std::string strApprovalArithmeticRowSha256;
	std::vector<std::string> PreservedBlockers;
};

struct EFFECT_RUNTIME_PROGRAM_MATERIAL_RECIPE final
{
	EFFECT_RUNTIME_PROGRAM_ROW_IDENTITY Row;
	std::string strSourceMaterialPath;
	std::string strSourceRecipeCompositionSha256;
	std::string strFamilyId;
	std::string strEvaluatorRegistryId;
	std::vector<std::string> InputIds;
	std::vector<std::string> StaticBindingIds;
	std::vector<std::string> RenderBindingIds;
	std::vector<EFFECT_RUNTIME_PROGRAM_MATERIAL_RECIPE_SAMPLE> NumericBindingSamples;
	std::string strBindingSha256;
	std::vector<std::string> PreservedBlockers;
};

struct EFFECT_RUNTIME_PROGRAM_MATERIAL_VALUE final
{
	EFFECT_RUNTIME_PROGRAM_ROW_IDENTITY Row;
	std::string strRecipeId;
	EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT eVariant =
		EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64;
	std::optional<bool_t> bValue;
	std::optional<double> fValue;
	std::array<double, 4u> vValue{};
	std::string strStringValue;
	std::string strPolicyRowId;
	std::string strFieldKind;
	std::string strBindingRole;
	std::string strBindingOrigin;
	std::string strSourceSection;
	uint32_t iSourceSectionIndex = 0u;
	std::string strParameterName;
	std::string strNormalizedParameterName;
	std::string strSelectionRole;
	std::optional<bool_t> bSourceValue;
	std::optional<bool_t> bSelectedValue;
	std::string strFieldName;
	std::string strSourceStatus;
	std::string strSourceFidelity;
	std::string strSourceRecordSha256;
	std::string strSourceBlocker;
	std::string strTypedValueSha256;
	std::string strSourceFieldValueSha256;
	std::string strSourceLineageSha256;
};

struct EFFECT_RUNTIME_PROGRAM_MATERIAL_OCCURRENCE final
{
	EFFECT_RUNTIME_PROGRAM_ROW_IDENTITY Row;
	std::string strEmitterId;
	std::string strCueId;
	EFFECT_RUNTIME_RENDERER_KIND eRenderer =
		EFFECT_RUNTIME_RENDERER_KIND::SPRITE_PARTICLE;
	std::string strRecipeId;
	std::string strFamilyId;
	std::string strEvaluatorRegistryId;
	std::string strSourceOccurrenceIdentitySha256;
	std::string strBindingSha256;
	std::string strSourceOccurrenceBindingSha256;
	std::vector<std::string> PreservedBlockers;
};

struct EFFECT_RUNTIME_PROGRAM_SAMPLER_DESCRIPTOR final
{
	std::string strType;
	std::string strFilterUe3;
	uint32_t iFilterD3d11 = 0u;
	std::string strAddressUUe3;
	uint32_t iAddressUD3d11 = 0u;
	std::string strAddressVUe3;
	uint32_t iAddressVD3d11 = 0u;
	std::string strAddressWUe3;
	uint32_t iAddressWD3d11 = 0u;
	double fMipLodBias = 0.0;
	uint32_t iMaxAnisotropy = 0u;
	std::string strComparisonFuncName;
	uint32_t iComparisonFuncD3d11 = 0u;
	std::array<double, 4u> vBorderColor{};
	double fMinLod = 0.0;
	double fMaxLod = 0.0;
	bool_t bSrgb = false;
	std::string strSrvColorSpace;
	std::string strLodGroup;
};

struct EFFECT_RUNTIME_PROGRAM_D3D_STENCIL_FACE final
{
	uint32_t iStencilFailOp = 0u;
	uint32_t iStencilDepthFailOp = 0u;
	uint32_t iStencilPassOp = 0u;
	uint32_t iStencilFunc = 0u;
};

struct EFFECT_RUNTIME_PROGRAM_D3D_DEPTH_STENCIL final
{
	bool_t bDepthEnable = false;
	uint32_t iDepthWriteMask = 0u;
	uint32_t iDepthFunc = 0u;
	bool_t bStencilEnable = false;
	uint32_t iStencilReadMask = 0u;
	uint32_t iStencilWriteMask = 0u;
	EFFECT_RUNTIME_PROGRAM_D3D_STENCIL_FACE FrontFace;
	EFFECT_RUNTIME_PROGRAM_D3D_STENCIL_FACE BackFace;
};

struct EFFECT_RUNTIME_PROGRAM_D3D_RASTERIZER final
{
	uint32_t iFillMode = 0u;
	uint32_t iCullMode = 0u;
	bool_t bFrontCounterClockwise = false;
	int32_t iDepthBias = 0;
	double fDepthBiasClamp = 0.0;
	double fSlopeScaledDepthBias = 0.0;
	bool_t bDepthClipEnable = false;
	bool_t bScissorEnable = false;
	bool_t bMultisampleEnable = false;
	bool_t bAntialiasedLineEnable = false;
};

struct EFFECT_RUNTIME_PROGRAM_D3D_SAMPLER final
{
	uint32_t iFilter = 0u;
	uint32_t iAddressU = 0u;
	uint32_t iAddressV = 0u;
	uint32_t iAddressW = 0u;
	double fMipLodBias = 0.0;
	uint32_t iMaxAnisotropy = 0u;
	uint32_t iComparisonFunc = 0u;
	std::array<double, 4u> vBorderColor{};
	double fMinLod = 0.0;
	double fMaxLod = 0.0;
};

struct EFFECT_RUNTIME_PROGRAM_D3D_DESCRIPTOR final
{
	EFFECT_RUNTIME_D3D_DESCRIPTOR_KIND eKind =
		EFFECT_RUNTIME_D3D_DESCRIPTOR_KIND::DEPTH_STENCIL;
	std::optional<EFFECT_RUNTIME_PROGRAM_D3D_DEPTH_STENCIL> DepthStencil;
	std::optional<EFFECT_RUNTIME_PROGRAM_D3D_RASTERIZER> Rasterizer;
	std::optional<EFFECT_RUNTIME_PROGRAM_D3D_SAMPLER> Sampler;
};

struct EFFECT_RUNTIME_PROGRAM_D3D_DESCRIPTOR_ORACLE final
{
	std::string strPolicyRowId;
	EFFECT_RUNTIME_D3D_DESCRIPTOR_KIND eKind =
		EFFECT_RUNTIME_D3D_DESCRIPTOR_KIND::DEPTH_STENCIL;
	EFFECT_RUNTIME_PROGRAM_D3D_DESCRIPTOR Expected;
	EFFECT_RUNTIME_PROGRAM_D3D_DESCRIPTOR Actual;
	double fNumericTolerance = 0.0;
	std::string strDecision;
};

struct EFFECT_RUNTIME_PROGRAM_D3D_SRV final
{
	uint32_t iFormat = 0u;
	uint32_t iViewDimension = 0u;
	uint32_t iMostDetailedMip = 0u;
	uint32_t iMipLevels = 0u;
	std::string strColorSpace;
};

struct EFFECT_RUNTIME_PROGRAM_D3D_SRV_ORACLE final
{
	std::string strPolicyRowId;
	EFFECT_RUNTIME_PROGRAM_D3D_SRV Expected;
	EFFECT_RUNTIME_PROGRAM_D3D_SRV Actual;
	double fNumericTolerance = 0.0;
	std::string strDecision;
};

struct EFFECT_RUNTIME_PROGRAM_APPROVAL_MATERIAL_RECEIPT final
{
	std::string strPolicyRowId;
	std::string strUpstreamIdentitySha256;
	std::string strPolicyBindingSha256;
	std::string strDomain;
	std::string strUpstreamMatrixRowId;
	std::string strMaterialRecipeId;
	std::vector<std::string> MaterialOccurrenceIds;
	std::string strFieldId;
	std::string strFieldKind;
	std::string strBindingOrigin;
	std::string strUpstreamDecision;
	std::string strPolicyFamilyId;
	std::string strEvidenceFidelity;
	std::string strExecutionFidelity;
	bool_t bSourceValueAcquired = false;
	std::vector<std::string> PartialSourceExactFields;
	std::optional<std::string> PreviousSamplerAdmission;
	bool_t bFullDescriptorSourceExact = false;
	bool_t bSourceExact = false;
	std::vector<std::string> PreservedEvidenceBlockers;
	std::vector<std::string> PolicyFidelityGuards;
	std::vector<std::string> RequiredOracleIds;
	std::vector<std::string> ExecutionBlockers;
	bool_t bExecutionAdmission = false;
	bool_t bProductAdmission = false;
};

struct EFFECT_RUNTIME_PROGRAM_MATERIAL_POLICY final
{
	EFFECT_RUNTIME_PROGRAM_ROW_IDENTITY Row;
	EFFECT_RUNTIME_MATERIAL_POLICY_DOMAIN eDomain =
		EFFECT_RUNTIME_MATERIAL_POLICY_DOMAIN::RENDER_STATE;
	std::string strSourceMatrixRowId;
	std::string strRecipeId;
	std::vector<std::string> MaterialOccurrenceIds;
	std::string strFieldId;
	std::string strFieldKind;
	std::string strBindingOrigin;
	std::string strEvidenceOwnerRecipeId;
	std::string strPolicyFidelity;
	bool_t bPolicySelectionAdmission = false;
	std::string strImplementationRegistryId;
	std::string strImplementationId;
	uint32_t iImplementationVersion = 0u;
	std::string strConsumerContract;
	EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT eValueVariant =
		EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64;
	std::optional<bool_t> bValue;
	std::optional<double> fValue;
	std::string strEnumType;
	std::string strEnumValue;
	std::optional<uint32_t> iEnumOrdinal;
	std::optional<EFFECT_RUNTIME_PROGRAM_SAMPLER_DESCRIPTOR> SamplerDescriptor;
	std::string strProviderBasisSha256;
	std::string strNumericOracleSha256;
	std::string strD3dStateOracleId;
	std::optional<EFFECT_RUNTIME_PROGRAM_D3D_DESCRIPTOR_ORACLE>
		D3dDescriptorOracle;
	std::optional<EFFECT_RUNTIME_PROGRAM_D3D_SRV_ORACLE> D3dSrvOracle;
	std::string strApprovalPolicyRowId;
	std::string strApprovalUpstreamIdentitySha256;
	std::string strApprovalPolicyBindingSha256;
	std::string strApprovalPolicyFamilyId;
	uint32_t iApprovalMaterialOrder = 0u;
	std::vector<std::string> ApprovalRequiredOracleIds;
	std::vector<std::string> EvidenceBlockers;
	std::vector<std::string> ApprovalExecutionBlockers;
	EFFECT_RUNTIME_PROGRAM_APPROVAL_MATERIAL_RECEIPT ApprovalMaterialReceipt;
	std::string strApprovalMaterialRowSha256;
	std::string strSourceRowSha256;
};

struct EFFECT_RUNTIME_PROGRAM_TEXTURE_BINDING final
{
	EFFECT_RUNTIME_PROGRAM_ROW_IDENTITY Row;
	std::string strRecipeId;
	std::string strMaterialInputFieldId;
	std::string strLogicalTexturePath;
	std::string strSamplerPolicyRowId;
	std::vector<std::string> MaterialOccurrenceIds;
	std::string strSourceBindingId;
	std::string strSourceBindingRowSha256;
	std::string strSourceTextureResourceId;
	std::string strSourceTextureResourceRowSha256;
	std::string strSourceProvisioningProposalId;
	std::string strSourceProvisioningProposalRowSha256;
	std::string strSourceDeploymentRowId;
	std::string strSourceDeploymentRowSha256;
	std::string strSourceReceiptStatus;
	std::optional<std::string> strRuntimeAssetId;
	EFFECT_RUNTIME_TEXTURE_RESOLUTION_STATUS eResolutionStatus =
		EFFECT_RUNTIME_TEXTURE_RESOLUTION_STATUS::RESOLVED_EXACT_RUNTIME_ASSET;
	std::string strBindingBasis;
	std::vector<std::string> Blockers;
};

struct EFFECT_RUNTIME_PROGRAM_RENDERER_TEXTURE final
{
	EFFECT_RUNTIME_PROGRAM_ROW_IDENTITY Row;
	std::string strEmitterId;
	std::string strSourceNode;
	std::string strMaterialOccurrenceId;
	std::string strSourceMaterialPath;
	std::string strSlotId;
	std::string strAssetId;
	std::string strSourceResourceProjectionSha256;
	std::string strProjectionRole;
	std::vector<std::string> Blockers;
};

struct EFFECT_RUNTIME_PROGRAM_GEOMETRY_CHANNEL_COUNTS final
{
	uint32_t iPosition = 0u;
	uint32_t iNormal = 0u;
	uint32_t iTangentXyz = 0u;
	uint32_t iTangentW = 0u;
	uint32_t iUv0 = 0u;
	uint32_t iColor0 = 0u;
};

struct EFFECT_RUNTIME_PROGRAM_GEOMETRY_CHANNEL_HASHES final
{
	std::string strPosition;
	std::string strNormal;
	std::string strTangentXyz;
	std::string strTangentW;
	std::string strUv0;
	std::optional<std::string> strColor0;
	std::string strIndicesU32;
};

struct EFFECT_RUNTIME_PROGRAM_GEOMETRY_SUBMESH final
{
	std::string strName;
	uint32_t iMaterialIndex = 0u;
	uint32_t iVertexCount = 0u;
	uint32_t iIndexCount = 0u;
	EFFECT_RUNTIME_PROGRAM_GEOMETRY_CHANNEL_COUNTS ChannelCounts;
	EFFECT_RUNTIME_PROGRAM_GEOMETRY_CHANNEL_HASHES ChannelSha256;
	std::array<std::string, 10u> BoundsF32Hex{};
};

struct EFFECT_RUNTIME_PROGRAM_APPROVAL_GEOMETRY_RECEIPT final
{
	std::string strAssetId;
	std::string strSourceObject;
	std::string strCandidateResourceSha256;
	std::string strPayloadSha256;
	std::string strMetadataIdentitySha256;
	double fGeometryPreScale = 0.0;
	std::string strArtifactBindingIntegrity;
	std::string strSourceFidelity;
	bool_t bSourceExact = false;
	std::vector<std::string> ExecutionBlockers;
	bool_t bExecutionAdmission = false;
	bool_t bProductAdmission = false;
};

struct EFFECT_RUNTIME_PROGRAM_GEOMETRY_CARRIER final
{
	EFFECT_RUNTIME_PROGRAM_ROW_IDENTITY Row;
	std::string strBindingId;
	std::string strSourceObject;
	std::string strAssetId;
	std::string strFormatVersion;
	uint32_t iCandidateResourceByteSize = 0u;
	std::string strCandidateResourceSha256;
	std::string strPayloadSha256;
	std::string strProvenanceSha256;
	std::string strProvenanceRole;
	std::string strMetadataIdentitySha256;
	std::string strCacheIdentitySha256;
	double fGeometryPreScale = 0.0;
	std::string strGeometryPreScaleF32Hex;
	uint32_t iChannelMask = 0u;
	uint32_t iEvidenceFlags = 0u;
	std::string strExpectedTupleSha256;
	std::vector<EFFECT_RUNTIME_PROGRAM_GEOMETRY_SUBMESH> Submeshes;
	uint32_t iApprovalGeometryOrder = 0u;
	EFFECT_RUNTIME_PROGRAM_APPROVAL_GEOMETRY_RECEIPT ApprovalGeometryReceipt;
	std::string strApprovalGeometryRowSha256;
	std::string strPreparedCacheIdentitySha256;
	bool_t bPreScaleConsumed = false;
};

struct EFFECT_RUNTIME_PROGRAM_GEOMETRY_USE final
{
	EFFECT_RUNTIME_PROGRAM_ROW_IDENTITY Row;
	std::string strEmitterId;
	std::string strModuleId;
	std::string strSourceEmitterPath;
	int32_t iSourceMeshPackageRef = 0;
	std::string strCarrierId;
	std::string strAssetId;
	std::string strSizeSemantics;
	std::string strPreScaleApplication;
	bool_t bPreScaleConsumed = false;
};

struct EFFECT_RUNTIME_PROGRAM_BLOCKER_OWNERSHIP final
{
	uint32_t iVersion = 0u;
	uint32_t iFieldCount = 0u;
	uint32_t iTokenOccurrenceCount = 0u;
	std::string strProjectionSha256;
};

struct EFFECT_RUNTIME_PROGRAM_ADMISSION final
{
	bool_t bArtifactBinding = false;
	bool_t bPolicyRoute = false;
	bool_t bSourceHandlerSelection = false;
	bool_t bDistributionEvaluatorSelection = false;
	bool_t bMaterialPolicySelection = false;
	bool_t bGeometryBinding = false;
	bool_t bSourceExact = false;
	bool_t bRuntimeExecution = false;
	bool_t bProduct = false;
	std::vector<std::string> Blockers;
};

struct EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM final
{
	EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM_IDENTITY Identity;
	std::string strCharacterClass;
	uint32_t iSkillId = 0u;
	std::string strInputSlot;
	std::string strPolicyTargetId;
	std::string strSourceCandidateId;
	std::string strGeometryBindingAssetId;
	std::string strRuntimeCatalogAssetId;
	std::string strIdentityRouteId;
	std::string strPolicyRouteBindingSha256;
	std::string strParticleSystemPolicySha256;
	std::string strMaterialEvaluatorOracleContractSha256;
	EFFECT_RUNTIME_PROGRAM_POLICY_ROUTE PolicyRoute;
	EFFECT_RUNTIME_PROGRAM_PARTICLE_POLICY ParticleSystemPolicy;
	EFFECT_RUNTIME_PROGRAM_MATERIAL_EVALUATOR_CONTRACT
		MaterialEvaluatorOracleContract;
	EFFECT_RUNTIME_PROGRAM_BLOCKER_OWNERSHIP BlockerOwnership;
	EFFECT_RUNTIME_PROGRAM_ADMISSION Admission;
	std::vector<EFFECT_RUNTIME_PROGRAM_INPUT_ARTIFACT> InputArtifacts;
	std::vector<EFFECT_RUNTIME_PROGRAM_HANDLER> Handlers;
	std::vector<EFFECT_RUNTIME_PROGRAM_EMITTER> Emitters;
	std::vector<EFFECT_RUNTIME_PROGRAM_ACTION_SCHEDULE> ActionSchedules;
	std::vector<EFFECT_RUNTIME_PROGRAM_MODULE> Modules;
	std::vector<EFFECT_RUNTIME_PROGRAM_PROPERTY> Properties;
	std::vector<EFFECT_RUNTIME_PROGRAM_PRIMITIVE_LEAF> PrimitiveLeaves;
	std::vector<EFFECT_RUNTIME_PROGRAM_LITERAL> Literals;
	std::vector<EFFECT_RUNTIME_PROGRAM_DISTRIBUTION> Distributions;
	std::vector<EFFECT_RUNTIME_PROGRAM_SEED_POLICY> SeedPolicies;
	std::vector<EFFECT_RUNTIME_PROGRAM_IMPLICIT_DEFAULT> ImplicitDefaults;
	std::vector<EFFECT_RUNTIME_PROGRAM_POINT_LIGHT_FIELD> PointLightFields;
	std::vector<EFFECT_RUNTIME_PROGRAM_MATERIAL_FAMILY> MaterialFamilies;
	std::vector<EFFECT_RUNTIME_PROGRAM_MATERIAL_RECIPE> MaterialRecipes;
	std::vector<EFFECT_RUNTIME_PROGRAM_MATERIAL_VALUE> MaterialInputs;
	std::vector<EFFECT_RUNTIME_PROGRAM_MATERIAL_VALUE> MaterialStaticBindings;
	std::vector<EFFECT_RUNTIME_PROGRAM_MATERIAL_VALUE> MaterialRenderBindings;
	std::vector<EFFECT_RUNTIME_PROGRAM_MATERIAL_OCCURRENCE> MaterialOccurrences;
	std::vector<EFFECT_RUNTIME_PROGRAM_MATERIAL_POLICY> MaterialPolicies;
	std::vector<EFFECT_RUNTIME_PROGRAM_TEXTURE_BINDING> MaterialTextureBindings;
	std::vector<EFFECT_RUNTIME_PROGRAM_RENDERER_TEXTURE> RendererTextureResources;
	std::vector<EFFECT_RUNTIME_PROGRAM_GEOMETRY_CARRIER> GeometryCarriers;
	std::vector<EFFECT_RUNTIME_PROGRAM_GEOMETRY_USE> GeometryUses;
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
	static EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM_IDENTITY
		Get_FrozenArtist31470FProgramIdentity();
	static bool_t Parse_ReconstructedRuntimeProgram(
		std::string_view Utf8Json,
		const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM_IDENTITY& ExpectedIdentity,
		std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM>& InOutProgram,
		std::string& strOutError);
};

NS_END
