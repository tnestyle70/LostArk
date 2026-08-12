#pragma once

#include "Client_Defines.h"
#include "Effect_Catalog.h"

#include <array>
#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

NS_BEGIN(Client)

inline constexpr uint32_t EFFECT_RECONSTRUCTED_EXECUTION_PLAN_VERSION = 1u;
inline constexpr uint32_t EFFECT_RECONSTRUCTED_OCCURRENCE_RNG_VERSION = 1u;
inline constexpr uint32_t EFFECT_RECONSTRUCTED_FIXED_STEP_HZ = 60u;
inline constexpr uint32_t EFFECT_RECONSTRUCTED_SELECTED_EVALUATOR_VERSION = 1u;
inline constexpr std::string_view
	EFFECT_RECONSTRUCTED_SELECTED_OCCURRENCE_RNG_CONTRACT =
		"artist-f.selected-occurrence-xorshift32.v1";

enum class EFFECT_RECONSTRUCTED_VISUAL_SCOPE : uint8_t
{
	/* Product-style Complete output is fail-closed.  Artist 31470 currently has
	   no occurrence whose source fidelity and human review are both sealed. */
	ADMITTED_ONLY,
	/* Explicit V3 audition of the shortest defensible main carrier stack.  These
	   rows are conditional review candidates, never restoration claims. */
	V3_MAIN_REVIEW,
	/* V4 keeps the same three-row admission surface while replacing only the
	   Artist-F material-composition revision.  The alias deliberately prevents
	   a new Product or occurrence scope from being inferred. */
	V4_ARTIST_F_MAIN_REVIEW = V3_MAIN_REVIEW,
	/* All occurrence families with enough typed or versioned evidence to merit
	   isolated human review.  They remain excluded from Complete output. */
	CONDITIONAL_REVIEW,
	/* Keeps all source occurrences available to explicit Solo/diagnostic
	   tooling without relabelling them as restored or Product-admitted. */
	ALL_DIAGNOSTIC,
	END
};

enum class EFFECT_RECONSTRUCTED_EXECUTION_MODULE_ROLE : uint8_t
{
	REQUIRED_TIMING,
	SPAWN_TIMING,
	LIFETIME,
	LIFETIME_SEEDED,
	DEFERRED_TYPED,
};

enum class EFFECT_RECONSTRUCTED_CPU_EMITTER_PHASE : uint8_t
{
	WAITING_FOR_SCHEDULE,
	WAITING_FOR_DELAY,
	EMITTING,
	COMPLETE,
};

struct EFFECT_RECONSTRUCTED_EXECUTION_PLAN_IDENTITY final
{
	uint32_t iPlanVersion = 0u;
	uint32_t iOccurrenceRngVersion = 0u;
	uint32_t iFixedStepHz = 0u;
	uint64_t iCatalogRevision = 0u;
	std::string strEffectAssetId;
	std::string strProgramId;
	uint32_t iProgramVersion = 0u;
	std::string strProgramSha256;
	std::string strCandidateRawSha256;
	std::string strSemanticProjectionSha256;
};

struct EFFECT_RECONSTRUCTED_EXECUTION_SCHEDULE final
{
	std::string strScheduleId;
	uint32_t iOrder = 0u;
	std::string strSourceCueId;
	std::string strSourceOccurrenceId;
	std::string strSourceSystemId;
	uint32_t iSourceReceiptEventIndex = 0u;
	double fGlobalTimeSeconds = 0.0;
	double fDurationSeconds = 0.0;
	std::vector<std::string> EmitterIds;
};

struct EFFECT_RECONSTRUCTED_EXECUTION_DISTRIBUTION final
{
	std::string strDistributionId;
	std::string strModuleId;
	std::string strPropertyId;
	uint32_t iOrder = 0u;
	EFFECT_RUNTIME_DISTRIBUTION_VARIANT eVariant =
		EFFECT_RUNTIME_DISTRIBUTION_VARIANT::INLINE;
	std::string strSourceClass;
	std::string strEvaluatorRegistryId;
	std::string strEvaluatorImplementationId;
	uint32_t iEvaluatorImplementationVersion = 0u;
	std::string strEvaluatorImplementationSha256;
	std::string strEvaluatorConsumerContract;
	bool_t bCpuTimingExecutable = false;
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
};

struct EFFECT_RECONSTRUCTED_EXECUTION_SEED_POLICY final
{
	std::string strSeedPolicyId;
	std::string strModuleId;
	uint32_t iOrder = 0u;
	std::string strEvaluatorId;
	std::vector<int32_t> RandomSeeds;
	std::optional<std::string> strParameterName;
	bool_t bGetSeedFromInstance = false;
	bool_t bInstanceSeedIsIndex = false;
	bool_t bResetSeedOnEmitterLooping = false;
	bool_t bRandomlySelectSeedArray = false;
	bool_t bEmptyArrayUsesOccurrenceRandomStream = false;
};

struct EFFECT_RECONSTRUCTED_EXECUTION_MODULE final
{
	std::string strModuleId;
	std::string strEmitterId;
	uint32_t iOrder = 0u;
	std::string strExactSourceClass;
	EFFECT_RUNTIME_MODULE_SELECTION eSelection =
		EFFECT_RUNTIME_MODULE_SELECTION::SOURCE_HANDLER;
	EFFECT_RECONSTRUCTED_EXECUTION_MODULE_ROLE eRole =
		EFFECT_RECONSTRUCTED_EXECUTION_MODULE_ROLE::DEFERRED_TYPED;
	std::string strHandlerRegistryId;
	std::string strHandlerVariant;
	std::string strHandlerImplementationId;
	uint32_t iHandlerImplementationVersion = 0u;
	std::string strHandlerImplementationSha256;
	std::string strHandlerConsumerContract;
	std::vector<std::string> DistributionIds;
	std::optional<std::string> strSeedPolicyId;
};

struct EFFECT_RECONSTRUCTED_EXECUTION_BURST final
{
	std::string strBurstId;
	std::string strSpawnModuleId;
	uint32_t iBurstIndex = 0u;
	double fTimeSeconds = 0.0;
	uint32_t iCountMinimum = 0u;
	uint32_t iCountMaximum = 0u;
};

struct EFFECT_RECONSTRUCTED_EXECUTION_EMITTER final
{
	std::string strEmitterId;
	uint32_t iOrder = 0u;
	std::string strScheduleId;
	std::string strSourceCueId;
	std::string strSourceOccurrenceId;
	std::string strSourceSystemId;
	EFFECT_RUNTIME_RENDERER_KIND eRenderer =
		EFFECT_RUNTIME_RENDERER_KIND::SPRITE_PARTICLE;
	bool_t bVisible = false;
	uint32_t iOperationalMaxParticles = 0u;
	uint32_t iRibbonOperationalMaxPoints = 0u;
	double fEmitterDelaySeconds = 0.0;
	double fEmitterDurationSeconds = 0.0;
	uint32_t iEmitterLoopCount = 0u;
	std::string strRandomPolicyId;
	uint32_t iEmitterRandomSeed = 0u;
	std::string strRequiredModuleId;
	std::string strSpawnModuleId;
	std::string strLifetimeModuleId;
	std::string strSpawnRateDistributionId;
	std::string strSpawnRateScaleDistributionId;
	std::string strLifetimeDistributionId;
	std::optional<std::string> strLifetimeSeedPolicyId;
	std::vector<EFFECT_RECONSTRUCTED_EXECUTION_BURST> Bursts;
	std::vector<std::string> ModuleIds;
};

struct EFFECT_RECONSTRUCTED_EXECUTION_PLAN_SUMMARY final
{
	uint32_t iScheduleCount = 0u;
	uint32_t iEmitterCount = 0u;
	uint32_t iModuleCount = 0u;
	uint32_t iDistributionCount = 0u;
	uint32_t iBurstCount = 0u;
	uint32_t iLifetimeModuleCount = 0u;
	uint32_t iSeededLifetimeModuleCount = 0u;
	uint32_t iOperationalCapSum = 0u;
	uint32_t iOperationalCapMaximum = 0u;
	uint32_t iRibbonOperationalMaxPoints = 0u;
	std::array<uint32_t, 5u> DistributionVariantCounts{};
};

struct EFFECT_RECONSTRUCTED_EXECUTION_SEEDED_LIFETIME_AUTHORITY final
{
	std::string strEmitterId;
	std::string strModuleId;
	std::string strPropertyId;
	std::string strDistributionId;
	std::string strSeedPolicyId;
	std::string strModuleHandlerRegistryId;
	std::string strPropertyHandlerRegistryId;
	std::string strEvaluatorRegistryId;
	std::string strCapabilityPolicyFamilyId;
	std::string strCapabilityImplementationId;
	uint32_t iCapabilityImplementationVersion = 0u;
	std::string strCapabilityImplementationSha256;
	std::string strCapabilityFamilySemanticSha256;
	std::string strCapabilityInputSchemaSha256;
	std::string strCapabilityOutputSchemaSha256;
	std::string strCapabilityDefaultPolicySha256;
	std::string strModuleRowSha256;
	std::string strPropertyRowSha256;
	std::string strDistributionRowSha256;
	std::string strSeedPolicyRowSha256;
	std::string strModuleHandlerRowSha256;
	std::string strPropertyHandlerRowSha256;
	std::string strEvaluatorRowSha256;
};

class CEffectReconstructedExecutionPlanCompiler;

class EFFECT_RECONSTRUCTED_EXECUTION_PLAN final
{
public:
	const EFFECT_RECONSTRUCTED_EXECUTION_PLAN_IDENTITY& Get_Identity() const
	{
		return m_Identity;
	}
	const EFFECT_RECONSTRUCTED_EXECUTION_PLAN_SUMMARY& Get_Summary() const
	{
		return m_Summary;
	}
	const EFFECT_RECONSTRUCTED_EXECUTION_SEEDED_LIFETIME_AUTHORITY&
		Get_SeededLifetimeAuthority() const
	{
		return m_SeededLifetimeAuthority;
	}
	const std::vector<std::string>& Get_ScheduleOrder() const
	{
		return m_ScheduleOrder;
	}
	const std::vector<std::string>& Get_EmitterOrder() const
	{
		return m_EmitterOrder;
	}
	const std::vector<std::string>& Get_ModuleOrder() const
	{
		return m_ModuleOrder;
	}
	const std::vector<std::string>& Get_DistributionOrder() const
	{
		return m_DistributionOrder;
	}
	const std::vector<std::string>& Get_SeedPolicyOrder() const
	{
		return m_SeedPolicyOrder;
	}
	const std::map<std::string, EFFECT_RECONSTRUCTED_EXECUTION_SCHEDULE,
		std::less<>>& Get_Schedules() const
	{
		return m_Schedules;
	}
	const std::map<std::string, EFFECT_RECONSTRUCTED_EXECUTION_EMITTER,
		std::less<>>& Get_Emitters() const
	{
		return m_Emitters;
	}
	const std::map<std::string, EFFECT_RECONSTRUCTED_EXECUTION_MODULE,
		std::less<>>& Get_Modules() const
	{
		return m_Modules;
	}
	const std::map<std::string, EFFECT_RECONSTRUCTED_EXECUTION_DISTRIBUTION,
		std::less<>>& Get_Distributions() const
	{
		return m_Distributions;
	}
	const std::map<std::string, EFFECT_RECONSTRUCTED_EXECUTION_SEED_POLICY,
		std::less<>>& Get_SeedPolicies() const
	{
		return m_SeedPolicies;
	}
	std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION>
		Get_Preparation() const
	{
		return m_pPreparation;
	}
	std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM>
		Get_Program() const
	{
		return m_pProgram;
	}

private:
	friend class CEffectReconstructedExecutionPlanCompiler;
	EFFECT_RECONSTRUCTED_EXECUTION_PLAN() = default;
	EFFECT_RECONSTRUCTED_EXECUTION_PLAN(
		const EFFECT_RECONSTRUCTED_EXECUTION_PLAN&) = delete;
	EFFECT_RECONSTRUCTED_EXECUTION_PLAN& operator=(
		const EFFECT_RECONSTRUCTED_EXECUTION_PLAN&) = delete;

private:
	EFFECT_RECONSTRUCTED_EXECUTION_PLAN_IDENTITY m_Identity;
	EFFECT_RECONSTRUCTED_EXECUTION_PLAN_SUMMARY m_Summary;
	EFFECT_RECONSTRUCTED_EXECUTION_SEEDED_LIFETIME_AUTHORITY
		m_SeededLifetimeAuthority;
	std::vector<std::string> m_ScheduleOrder;
	std::vector<std::string> m_EmitterOrder;
	std::vector<std::string> m_ModuleOrder;
	std::vector<std::string> m_DistributionOrder;
	std::vector<std::string> m_SeedPolicyOrder;
	std::map<std::string, EFFECT_RECONSTRUCTED_EXECUTION_SCHEDULE, std::less<>>
		m_Schedules;
	std::map<std::string, EFFECT_RECONSTRUCTED_EXECUTION_EMITTER, std::less<>>
		m_Emitters;
	std::map<std::string, EFFECT_RECONSTRUCTED_EXECUTION_MODULE, std::less<>>
		m_Modules;
	std::map<std::string, EFFECT_RECONSTRUCTED_EXECUTION_DISTRIBUTION,
		std::less<>> m_Distributions;
	std::map<std::string, EFFECT_RECONSTRUCTED_EXECUTION_SEED_POLICY,
		std::less<>> m_SeedPolicies;
	std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION>
		m_pPreparation;
	std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM> m_pProgram;
};

class CEffectReconstructedExecutionPlanCompiler final
{
public:
	static bool_t Compile_Preparation(
		std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION>
			pPreparation,
		std::shared_ptr<const EFFECT_RECONSTRUCTED_EXECUTION_PLAN>& InOutPlan,
		std::string& strOutError);

#if defined(LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS)
	static bool_t Compile_ProgramForTests(
		const EFFECT_RUNTIME_PROGRAM_CATALOG_IDENTITY& CatalogIdentity,
		std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM> pProgram,
		std::shared_ptr<const EFFECT_RECONSTRUCTED_EXECUTION_PLAN>& InOutPlan,
		std::string& strOutError);
#endif
};

/* Builds the non-Product Artist F runtime document from the exact immutable
   Catalog preparation.  The native-v14 file is used only as the structural
   carrier; schedule, emitter, renderer and material values are rebound from
   the parsed runtime Program before the document can be staged. */
class CEffectReconstructedSourceRuntimeFactory final
{
public:
	static bool_t Build_Document(
		std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION>
			pPreparation,
		EFFECT_DOCUMENT_DESC& OutDocument,
		std::string& strOutError,
		EFFECT_RECONSTRUCTED_VISUAL_SCOPE eVisualScope =
			EFFECT_RECONSTRUCTED_VISUAL_SCOPE::ADMITTED_ONLY);
};

struct EFFECT_RECONSTRUCTED_CPU_EMITTER_STATE final
{
	std::string strEmitterId;
	std::string strScheduleId;
	EFFECT_RECONSTRUCTED_CPU_EMITTER_PHASE ePhase =
		EFFECT_RECONSTRUCTED_CPU_EMITTER_PHASE::WAITING_FOR_SCHEDULE;
	uint32_t iLoopIndex = 0u;
	uint32_t iRandomState = 0u;
	uint32_t iLifetimeRandomState = 0u;
	uint64_t iNextSpawnSerial = 0u;
	uint64_t iSpawnedTotal = 0u;
	uint64_t iDroppedByCap = 0u;
	uint32_t iActiveCount = 0u;
};

struct EFFECT_RECONSTRUCTED_CPU_OCCURRENCE_PACKET final
{
	std::string strOccurrenceId;
	std::string strScheduleId;
	std::string strEmitterId;
	EFFECT_RUNTIME_RENDERER_KIND eRenderer =
		EFFECT_RUNTIME_RENDERER_KIND::SPRITE_PARTICLE;
	uint32_t iLoopIndex = 0u;
	uint64_t iSpawnSerial = 0u;
	uint64_t iSpawnStep = 0u;
	uint32_t iOccurrenceRandomValue = 0u;
	uint32_t iLifetimeRandomValue = 0u;
	double fAgeSeconds = 0.0;
	double fLifetimeSeconds = 0.0;
};

enum class EFFECT_RECONSTRUCTED_SELECTED_PACKET_KIND : uint8_t
{
	MESH,
	SPRITE,
	END
};

enum class EFFECT_RECONSTRUCTED_SPRITE_ALIGNMENT : uint8_t
{
	VELOCITY,
	END
};

enum class EFFECT_RECONSTRUCTED_SPRITE_ORIENTATION : uint8_t
{
	CAMERA_BILLBOARD_WITH_VELOCITY_ALIGNMENT,
	END
};

struct EFFECT_RECONSTRUCTED_SELECTED_ROW_IDENTITY final
{
	std::string strId;
	std::string strRowSha256;
};

struct EFFECT_RECONSTRUCTED_SELECTED_HANDLER_IDENTITY final
{
	EFFECT_RECONSTRUCTED_SELECTED_ROW_IDENTITY Module;
	EFFECT_RECONSTRUCTED_SELECTED_ROW_IDENTITY Handler;
	std::string strExactSourceClass;
	std::string strImplementationId;
	uint32_t iImplementationVersion = 0u;
	std::string strImplementationSha256;
	std::vector<EFFECT_RECONSTRUCTED_SELECTED_ROW_IDENTITY> ImplicitDefaults;
};

struct EFFECT_RECONSTRUCTED_SELECTED_CYLINDER_POLICY final
{
	EFFECT_RECONSTRUCTED_SELECTED_ROW_IDENTITY Module;
	EFFECT_RECONSTRUCTED_SELECTED_ROW_IDENTITY SurfaceOnlyLiteral;
	EFFECT_RECONSTRUCTED_SELECTED_ROW_IDENTITY VelocityLiteral;
	std::string strAbsentHeightAxisDefault;
};

struct EFFECT_RECONSTRUCTED_SELECTED_TEXTURE_LANE final
{
	std::string strShaderVariableName;
	std::optional<EFFECT_RECONSTRUCTED_SELECTED_ROW_IDENTITY>
		RendererTextureResource;
	EFFECT_RECONSTRUCTED_SELECTED_ROW_IDENTITY MaterialInput;
	EFFECT_RECONSTRUCTED_SELECTED_ROW_IDENTITY MaterialTextureBinding;
	EFFECT_RECONSTRUCTED_SELECTED_ROW_IDENTITY MaterialPolicy;
	std::optional<EFFECT_RECONSTRUCTED_SELECTED_ROW_IDENTITY>
		SidecarRendererSlotDecision;
	EFFECT_RECONSTRUCTED_SELECTED_ROW_IDENTITY SidecarTextureBinding;
	EFFECT_RECONSTRUCTED_SELECTED_ROW_IDENTITY SidecarTextureResource;
	std::string strRuntimeAssetId;
	std::string strRawSha256;
};

struct EFFECT_RECONSTRUCTED_SELECTED_STATE_BINDING final
{
	EFFECT_RECONSTRUCTED_SELECTED_ROW_IDENTITY ProgramBinding;
	std::optional<EFFECT_RECONSTRUCTED_SELECTED_ROW_IDENTITY> ProgramPolicy;
	std::optional<EFFECT_RECONSTRUCTED_SELECTED_ROW_IDENTITY> SidecarDecision;
};

struct EFFECT_RECONSTRUCTED_SELECTED_GEOMETRY_BINDING final
{
	EFFECT_RECONSTRUCTED_SELECTED_ROW_IDENTITY GeometryUse;
	EFFECT_RECONSTRUCTED_SELECTED_ROW_IDENTITY GeometryCarrier;
	std::string strRuntimeAssetId;
	uint32_t iCandidateResourceByteSize = 0u;
	std::string strCandidateResourceSha256;
	std::string strPayloadSha256;
	std::string strMetadataIdentitySha256;
	std::string strCacheIdentitySha256;
	std::string strExpectedTupleSha256;
	std::string strApprovalGeometryRowSha256;
	std::string strPreparedCacheIdentitySha256;
	double fGeometryPreScale = 0.0;
	uint32_t iSubmeshCount = 0u;
	uint32_t iVertexCount = 0u;
	uint32_t iIndexCount = 0u;
};

struct EFFECT_RECONSTRUCTED_SELECTED_SPRITE_SINK final
{
	EFFECT_RECONSTRUCTED_SELECTED_ROW_IDENTITY RequiredModule;
	EFFECT_RECONSTRUCTED_SELECTED_ROW_IDENTITY ScreenAlignmentProperty;
	EFFECT_RECONSTRUCTED_SELECTED_ROW_IDENTITY ScreenAlignmentLiteral;
	EFFECT_RECONSTRUCTED_SELECTED_ROW_IDENTITY AllowImageFlippingProperty;
	EFFECT_RECONSTRUCTED_SELECTED_ROW_IDENTITY AllowImageFlippingLiteral;
	EFFECT_RECONSTRUCTED_SELECTED_ROW_IDENTITY OffsetCenterEnabledProperty;
	EFFECT_RECONSTRUCTED_SELECTED_ROW_IDENTITY OffsetCenterEnabledLiteral;
	EFFECT_RECONSTRUCTED_SELECTED_ROW_IDENTITY OffsetCenterXProperty;
	EFFECT_RECONSTRUCTED_SELECTED_ROW_IDENTITY OffsetCenterXLiteral;
	EFFECT_RECONSTRUCTED_SELECTED_ROW_IDENTITY OffsetCenterYProperty;
	EFFECT_RECONSTRUCTED_SELECTED_ROW_IDENTITY OffsetCenterYLiteral;
	EFFECT_RECONSTRUCTED_SPRITE_ALIGNMENT eAlignment =
		EFFECT_RECONSTRUCTED_SPRITE_ALIGNMENT::END;
	EFFECT_RECONSTRUCTED_SPRITE_ORIENTATION eOrientation =
		EFFECT_RECONSTRUCTED_SPRITE_ORIENTATION::END;
	bool_t bAllowImageFlipping = false;
	bool_t bOffsetCenter = false;
	std::array<double, 2u> vPivotCenter{};
	bool_t bBillboard = false;
	double fBillboardRollDegrees = 0.0;
};

struct EFFECT_RECONSTRUCTED_SELECTED_MATERIAL_CONSTANTS final
{
	std::array<double, 2u> vUvScale{};
	std::array<double, 4u> vPanRotationAux{};
	std::array<double, 4u> vColor{};
	std::array<double, 4u> vParams0{};
	std::array<double, 4u> vParams1{};
};

struct EFFECT_RECONSTRUCTED_SELECTED_SHADER_BINDING final
{
	std::string strShaderAssetId;
	std::string strTechniqueName;
	std::string strPassName;
	uint32_t iPassIndex = 0u;
	std::string strEvaluatorEnabledVariable;
	std::string strFeatureMaskVariable;
	std::string strUvScaleVariable;
	std::string strPanRotationAuxVariable;
	std::string strColorVariable;
	std::string strParams0Variable;
	std::string strParams1Variable;
};

struct EFFECT_RECONSTRUCTED_SELECTED_MATERIAL_BINDING final
{
	EFFECT_RECONSTRUCTED_SELECTED_ROW_IDENTITY Occurrence;
	EFFECT_RECONSTRUCTED_SELECTED_ROW_IDENTITY Recipe;
	EFFECT_RECONSTRUCTED_SELECTED_ROW_IDENTITY Family;
	EFFECT_RECONSTRUCTED_SELECTED_ROW_IDENTITY RecipeTextureDecision;
	std::string strEvaluatorId;
	uint32_t iEvaluatorVersion = 0u;
	std::string strEvaluatorSha256;
	uint32_t iFeatureMask = 0u;
	std::array<EFFECT_RECONSTRUCTED_SELECTED_TEXTURE_LANE, 2u> TextureLanes;
	EFFECT_RECONSTRUCTED_SELECTED_STATE_BINDING BlendState;
	EFFECT_RECONSTRUCTED_SELECTED_STATE_BINDING RasterizerState;
	EFFECT_RECONSTRUCTED_SELECTED_STATE_BINDING DepthStencilState;
	EFFECT_RECONSTRUCTED_SELECTED_MATERIAL_CONSTANTS Constants;
	EFFECT_RECONSTRUCTED_SELECTED_SHADER_BINDING Shader;
};

struct EFFECT_RECONSTRUCTED_SELECTED_EMITTER_SELECTION final
{
	EFFECT_RECONSTRUCTED_SELECTED_PACKET_KIND eKind =
		EFFECT_RECONSTRUCTED_SELECTED_PACKET_KIND::END;
	EFFECT_RUNTIME_RENDERER_KIND eRenderer =
		EFFECT_RUNTIME_RENDERER_KIND::SPRITE_PARTICLE;
	EFFECT_RECONSTRUCTED_SELECTED_ROW_IDENTITY Schedule;
	EFFECT_RECONSTRUCTED_SELECTED_ROW_IDENTITY Emitter;
	uint32_t iEmitterOrder = 0u;
	bool_t bLocalSpace = false;
	std::string strSizeUnitPolicy;
	uint32_t iExpectedVisualRandomDrawCount = 0u;
	uint32_t iExpectedFinalRandomState = 0u;
	uint32_t iExpectedOccurrenceRandomValue = 0u;
	uint32_t iExpectedLifetimeRandomValue = 0u;
	double fExpectedLifetimeSeconds = 0.0;
	std::vector<EFFECT_RECONSTRUCTED_SELECTED_HANDLER_IDENTITY> Handlers;
	std::optional<EFFECT_RECONSTRUCTED_SELECTED_CYLINDER_POLICY> Cylinder;
	std::optional<EFFECT_RECONSTRUCTED_SELECTED_GEOMETRY_BINDING> Geometry;
	std::optional<EFFECT_RECONSTRUCTED_SELECTED_SPRITE_SINK> SpriteSink;
	EFFECT_RECONSTRUCTED_SELECTED_MATERIAL_BINDING Material;
};

struct EFFECT_RECONSTRUCTED_SELECTED_EVALUATOR_REQUEST final
{
	uint32_t iEvaluatorVersion = 0u;
	std::string strOccurrenceRngContract;
	uint32_t iOccurrenceRngVersion = 0u;
	uint64_t iRequiredFixedStepIndex = 0u;
	uint64_t iRequiredSpawnSerial = 0u;
	uint32_t iExpectedConsumedHandlerCount = 0u;
	std::vector<EFFECT_RECONSTRUCTED_SELECTED_EMITTER_SELECTION> Emitters;
};

struct EFFECT_RECONSTRUCTED_SELECTED_DIAGNOSTIC_SELECTOR final
{
	std::string strScheduleId;
	std::string strMeshEmitterId;
	std::string strSpriteEmitterId;
	uint64_t iFixedStepIndex = 0u;
	uint64_t iSpawnSerial = 0u;
};

struct EFFECT_RECONSTRUCTED_SELECTED_HANDLER_CONSUMPTION final
{
	EFFECT_RECONSTRUCTED_SELECTED_ROW_IDENTITY Module;
	EFFECT_RECONSTRUCTED_SELECTED_ROW_IDENTITY Handler;
	std::string strImplementationId;
	uint32_t iImplementationVersion = 0u;
	std::string strImplementationSha256;
};

struct EFFECT_RECONSTRUCTED_SELECTED_PARTICLE_VALUES final
{
	std::optional<std::array<double, 3u>> vMeshDimensionlessScaleXzy;
	std::optional<std::array<double, 3u>> vSpriteSignedWorldSizeXzy;
	std::array<double, 3u> vLocalPosition{};
	std::array<double, 3u> vVelocityPerSecond{};
	std::array<double, 3u> vAccelerationPerSecondSquared{};
	std::array<double, 4u> vColor{};
	std::array<double, 4u> vDynamicParameter{};
	double fRotationDegrees = 0.0;
	double fRotationRateDegreesPerSecond = 0.0;
};

struct EFFECT_RECONSTRUCTED_SELECTED_EVALUATOR_PREPARED_DATA;
class CEffectReconstructedSelectedEvaluator;

class EFFECT_RECONSTRUCTED_SELECTED_EVALUATOR_PREPARATION final
{
public:
	std::shared_ptr<const EFFECT_RECONSTRUCTED_EXECUTION_PLAN> Get_Plan() const
	{
		return m_pPlan;
	}
	std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION>
		Get_RuntimePreparation() const
	{
		return m_pRuntimePreparation;
	}
	std::shared_ptr<const EFFECT_RUNTIME_PROGRAM_CATALOG_ENTRY>
		Get_CatalogEntry() const
	{
		return m_pCatalogEntry;
	}
	std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM> Get_Program() const
	{
		return m_pProgram;
	}
	std::shared_ptr<const EFFECT_RECONSTRUCTED_RENDER_RESOURCE_AUTHORITY>
		Get_RenderResourceAuthority() const
	{
		return m_pRenderResourceAuthority;
	}
	const EFFECT_RECONSTRUCTED_SELECTED_EVALUATOR_REQUEST& Get_Request() const
	{
		return m_Request;
	}

private:
	friend class CEffectReconstructedSelectedEvaluator;
	std::shared_ptr<const EFFECT_RECONSTRUCTED_EXECUTION_PLAN> m_pPlan;
	std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION>
		m_pRuntimePreparation;
	std::shared_ptr<const EFFECT_RUNTIME_PROGRAM_CATALOG_ENTRY> m_pCatalogEntry;
	std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM> m_pProgram;
	std::shared_ptr<const EFFECT_RECONSTRUCTED_RENDER_RESOURCE_AUTHORITY>
		m_pRenderResourceAuthority;
	EFFECT_RECONSTRUCTED_SELECTED_EVALUATOR_REQUEST m_Request;
	std::shared_ptr<const EFFECT_RECONSTRUCTED_SELECTED_EVALUATOR_PREPARED_DATA>
		m_pPreparedData;
};

class EFFECT_RECONSTRUCTED_SELECTED_PACKET final
{
public:
	std::shared_ptr<const EFFECT_RECONSTRUCTED_SELECTED_EVALUATOR_PREPARATION>
		Get_Preparation() const
	{
		return m_pPreparation;
	}
	uint32_t Get_SelectionIndex() const { return m_iSelectionIndex; }
	EFFECT_RECONSTRUCTED_SELECTED_PACKET_KIND Get_Kind() const { return m_eKind; }
	const EFFECT_RECONSTRUCTED_CPU_OCCURRENCE_PACKET& Get_Timing() const
	{
		return m_Timing;
	}
	uint32_t Get_FinalRandomState() const { return m_iFinalRandomState; }
	uint32_t Get_RandomDrawCount() const { return m_iRandomDrawCount; }
	const EFFECT_RECONSTRUCTED_SELECTED_PARTICLE_VALUES& Get_Values() const
	{
		return m_Values;
	}
	const std::vector<EFFECT_RECONSTRUCTED_SELECTED_HANDLER_CONSUMPTION>&
		Get_ConsumedHandlers() const
	{
		return m_ConsumedHandlers;
	}
	const std::optional<EFFECT_RECONSTRUCTED_SELECTED_SPRITE_SINK>&
		Get_SpriteSink() const
	{
		return m_SpriteSink;
	}

private:
	friend class CEffectReconstructedSelectedEvaluator;
	std::shared_ptr<const EFFECT_RECONSTRUCTED_SELECTED_EVALUATOR_PREPARATION>
		m_pPreparation;
	uint32_t m_iSelectionIndex = 0u;
	EFFECT_RECONSTRUCTED_SELECTED_PACKET_KIND m_eKind =
		EFFECT_RECONSTRUCTED_SELECTED_PACKET_KIND::END;
	EFFECT_RECONSTRUCTED_CPU_OCCURRENCE_PACKET m_Timing;
	uint32_t m_iFinalRandomState = 0u;
	uint32_t m_iRandomDrawCount = 0u;
	EFFECT_RECONSTRUCTED_SELECTED_PARTICLE_VALUES m_Values;
	std::optional<EFFECT_RECONSTRUCTED_SELECTED_SPRITE_SINK> m_SpriteSink;
	std::vector<EFFECT_RECONSTRUCTED_SELECTED_HANDLER_CONSUMPTION>
		m_ConsumedHandlers;
};

class EFFECT_RECONSTRUCTED_SELECTED_FRAME final
{
public:
	std::shared_ptr<const EFFECT_RECONSTRUCTED_SELECTED_EVALUATOR_PREPARATION>
		Get_Preparation() const
	{
		return m_pPreparation;
	}
	uint64_t Get_FixedStepIndex() const { return m_iFixedStepIndex; }
	double Get_SampleTimeSeconds() const { return m_fSampleTimeSeconds; }
	const std::string& Get_OccurrenceRngContract() const
	{
		return m_strOccurrenceRngContract;
	}
	uint32_t Get_OccurrenceRngVersion() const
	{
		return m_iOccurrenceRngVersion;
	}
	const std::vector<EFFECT_RECONSTRUCTED_SELECTED_PACKET>& Get_Packets() const
	{
		return m_Packets;
	}
	uint32_t Get_ConsumedHandlerCount() const
	{
		return m_iConsumedHandlerCount;
	}
	const std::string& Get_ProjectionSha256() const
	{
		return m_strProjectionSha256;
	}

private:
	friend class CEffectReconstructedSelectedEvaluator;
	std::shared_ptr<const EFFECT_RECONSTRUCTED_SELECTED_EVALUATOR_PREPARATION>
		m_pPreparation;
	uint64_t m_iFixedStepIndex = 0u;
	double m_fSampleTimeSeconds = 0.0;
	std::string m_strOccurrenceRngContract;
	uint32_t m_iOccurrenceRngVersion = 0u;
	uint32_t m_iConsumedHandlerCount = 0u;
	std::vector<EFFECT_RECONSTRUCTED_SELECTED_PACKET> m_Packets;
	std::string m_strProjectionSha256;
};

class CEffectReconstructedSelectedEvaluator final
{
public:
	static bool_t Prepare(
		std::shared_ptr<const EFFECT_RECONSTRUCTED_EXECUTION_PLAN> pPlan,
		const EFFECT_RECONSTRUCTED_SELECTED_EVALUATOR_REQUEST& Request,
		std::shared_ptr<const EFFECT_RECONSTRUCTED_SELECTED_EVALUATOR_PREPARATION>&
			InOutPreparation,
		std::string& strOutError);
	static bool_t Evaluate(
		std::shared_ptr<const EFFECT_RECONSTRUCTED_SELECTED_EVALUATOR_PREPARATION>
			pPreparation,
		uint64_t iFixedStepIndex,
		uint64_t iSpawnSerial,
		std::shared_ptr<const EFFECT_RECONSTRUCTED_SELECTED_FRAME>& InOutFrame,
		std::string& strOutError);
};

class CEffectReconstructedSelectedDiagnosticFactory final
{
public:
	static bool_t Prepare(
		std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION>
			pRuntimePreparation,
		const EFFECT_RECONSTRUCTED_SELECTED_DIAGNOSTIC_SELECTOR& Selector,
		std::shared_ptr<const EFFECT_RECONSTRUCTED_SELECTED_EVALUATOR_PREPARATION>&
			InOutPreparation,
		std::string& strOutError);
};

class CEffectReconstructedCpuInspector;

class EFFECT_RECONSTRUCTED_CPU_INSPECTION_STATE final
{
public:
	std::shared_ptr<const EFFECT_RECONSTRUCTED_EXECUTION_PLAN> Get_Plan() const
	{
		return m_pPlan;
	}
	uint64_t Get_FixedStepIndex() const { return m_iFixedStepIndex; }
	double Get_SampleTimeSeconds() const { return m_fSampleTimeSeconds; }
	const std::vector<EFFECT_RECONSTRUCTED_CPU_EMITTER_STATE>&
		Get_Emitters() const
	{
		return m_Emitters;
	}
	const std::string& Get_ProjectionSha256() const
	{
		return m_strProjectionSha256;
	}

private:
	friend class CEffectReconstructedCpuInspector;
	std::shared_ptr<const EFFECT_RECONSTRUCTED_EXECUTION_PLAN> m_pPlan;
	uint64_t m_iFixedStepIndex = 0u;
	double m_fSampleTimeSeconds = 0.0;
	std::vector<EFFECT_RECONSTRUCTED_CPU_EMITTER_STATE> m_Emitters;
	std::string m_strProjectionSha256;
};

class EFFECT_RECONSTRUCTED_CPU_INSPECTION_FRAME final
{
public:
	std::shared_ptr<const EFFECT_RECONSTRUCTED_EXECUTION_PLAN> Get_Plan() const
	{
		return m_pPlan;
	}
	uint64_t Get_FixedStepIndex() const { return m_iFixedStepIndex; }
	double Get_SampleTimeSeconds() const { return m_fSampleTimeSeconds; }
	const std::vector<EFFECT_RECONSTRUCTED_CPU_OCCURRENCE_PACKET>&
		Get_ActiveOccurrences() const
	{
		return m_ActiveOccurrences;
	}
	const std::string& Get_ProjectionSha256() const
	{
		return m_strProjectionSha256;
	}

private:
	friend class CEffectReconstructedCpuInspector;
	std::shared_ptr<const EFFECT_RECONSTRUCTED_EXECUTION_PLAN> m_pPlan;
	uint64_t m_iFixedStepIndex = 0u;
	double m_fSampleTimeSeconds = 0.0;
	std::vector<EFFECT_RECONSTRUCTED_CPU_OCCURRENCE_PACKET>
		m_ActiveOccurrences;
	std::string m_strProjectionSha256;
};

#if defined(LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS)
class CEffectReconstructedCpuInspector final
{
public:
	static bool_t Simulate(
		std::shared_ptr<const EFFECT_RECONSTRUCTED_EXECUTION_PLAN> pPlan,
		double fSampleTimeSeconds,
		std::shared_ptr<const EFFECT_RECONSTRUCTED_CPU_INSPECTION_STATE>&
			InOutState,
		std::shared_ptr<const EFFECT_RECONSTRUCTED_CPU_INSPECTION_FRAME>&
			InOutFrame,
		std::string& strOutError);
};
#endif

NS_END
