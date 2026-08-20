#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"
#include "Effect_AuthoringDocument.h"
#include "Effect_Catalog.h"
#include "Effect_ReconstructedExecution.h"

#include <functional>
#include <string>
#include <string_view>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

NS_BEGIN(Client)

struct EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION;

struct EFFECT_EVALUATED_ELEMENT final
{
	const EFFECT_ELEMENT_DESC* pElement = nullptr;
	float4x4_t World{};
	EFFECT_COLOR_DESC Color{};
	f32_t fLocalTimeSeconds = 0.f;
	f32_t fNormalizedLife = 0.f;
	/* Source-visual DecalParticle owns its evaluated X/Yaw/Z footprint and
	   projection depth in World exactly once.  Authored/static decals leave
	   this false and continue to use Detail.Decal shader constants. */
	bool_t bWorldOwnsDecalProjectionVolume = false;
};

enum class EFFECT_PARTICLE_SPRITE_ALIGNMENT : uint8_t
{
	CAMERA_SQUARE,
	CAMERA_RECTANGLE,
	CAMERA_VELOCITY,
	AXIS_POSITIVE_X,
	AXIS_NEGATIVE_X,
	AXIS_POSITIVE_Y,
	AXIS_NEGATIVE_Y,
	AXIS_POSITIVE_Z,
	AXIS_NEGATIVE_Z,
	ROTATE_X,
	ROTATE_Y,
	ROTATE_Z,
	END
};

struct EFFECT_EVALUATED_PARTICLE final
{
	const EFFECT_ELEMENT_DESC* pElement = nullptr;
	float4x4_t World{};
	float4_t Color = { 1.f, 1.f, 1.f, 1.f };
	float4_t vDynamicParameter{};
	float3_t vWorldVelocity{};
	float2_t vSpritePivot = { 0.5f, 0.5f };
	/* UE3 Cascade can encode horizontal/vertical image flipping with signed
	   StartSize axes.  World decomposition loses those signs, so a source
	   Sprite that explicitly allows image flipping carries them separately to
	   the final current/next atlas-cell UV transform.  Zero is deliberately
	   represented as +1 here; CAMERA_SQUARE may legally derive Y from X. */
	float2_t vSourceImageFlipSign = { 1.f, 1.f };
	EFFECT_PARTICLE_SPRITE_ALIGNMENT eSpriteAlignment =
		EFFECT_PARTICLE_SPRITE_ALIGNMENT::CAMERA_RECTANGLE;
	bool_t bSourceImageFlipping = false;
	f32_t fSpriteRotationDegrees = 0.f;
	f32_t fCameraOffset = 0.f;
	f32_t fSubImageIndex = 0.f;
	f32_t fDistributionRandom = 0.f;
	f32_t fNormalizedLife = 0.f;
};

struct EFFECT_SUBUV_FRAME_DESC final
{
	float4_t Current = { 1.f, 1.f, 0.f, 0.f };
	float4_t Next = { 1.f, 1.f, 0.f, 0.f };
	f32_t fBlend = 0.f;
};

struct EFFECT_EVALUATED_TRAIL_POINT final
{
	float3_t vWorldPosition{};
	f32_t fNormalizedAge = 0.f;
	/* Geometry-derived distance in world units from the beginning of the
	   current connected trail.  It is independent of point eviction and is
	   suitable for a later typed tiling-distance consumer. */
	f32_t fCumulativeDistance = 0.f;
	/* Only lanes selected by the corresponding mask were evaluated from staged
	   source distributions or an explicit authored execution carrier. Unselected
	   color/dynamic lanes are identity/zero placeholders. Source provenance is
	   still owned by the Element recipe, not inferred from this transient mask. */
	float4_t vSourceColor = { 1.f, 1.f, 1.f, 1.f };
	float4_t vDynamicParameter{};
	uint32_t iSourceColorComponentMask = 0u;
	uint32_t iDynamicParameterComponentMask = 0u;
};

struct EFFECT_EVALUATED_TRAIL final
{
	const EFFECT_ELEMENT_DESC* pElement = nullptr;
	std::vector<EFFECT_EVALUATED_TRAIL_POINT> Points;
};

struct EFFECT_EVALUATED_AFTERIMAGE final
{
	const EFFECT_ELEMENT_DESC* pElement = nullptr;
	float4x4_t World{};
	f32_t fAlpha = 1.f;
};

struct EFFECT_EVALUATED_LIGHT final
{
	const EFFECT_ELEMENT_DESC* pElement = nullptr;
	float3_t vWorldPosition{};
	f32_t fRange = 0.f;
	f32_t fIntensity = 0.f;
	float4_t vColor = { 1.f, 1.f, 1.f, 1.f };
	float4_t vAmbient = { 0.f, 0.f, 0.f, 1.f };
	f32_t fFalloffExponent = 0.f;
	f32_t fNormalizedLife = 0.f;
};

struct EFFECT_EVALUATED_SCREEN_POST final
{
	const EFFECT_ELEMENT_DESC* pElement = nullptr;
	EFFECT_SCREEN_POST_PROFILE eProfile = EFFECT_SCREEN_POST_PROFILE::END;
	uint32_t iSourceOrder = 0u;
	uint32_t iRandomSeed = 1u;
	f32_t fSampleTimeSeconds = 0.f;
	f32_t fIntensity = 0.f;
	f32_t fSecondaryIntensity = 0.f;
	f32_t fFrequency = 1.f;
	float4_t vTint = { 1.f, 1.f, 1.f, 1.f };
	f32_t fNormalizedLife = 0.f;
};

struct EFFECT_EVALUATED_GPU_OCCURRENCE final
{
	const EFFECT_ELEMENT_DESC* pElement = nullptr;
	bool_t bActive = false;
	uint32_t iCandidateRowCount = 0u;
};

struct EFFECT_EVALUATED_FRAME final
{
	f32_t fSampleTimeSeconds = 0.f;
	float4x4_t RootWorld{};
	std::vector<EFFECT_EVALUATED_GPU_OCCURRENCE> GpuOccurrences;
	std::vector<EFFECT_EVALUATED_ELEMENT> Elements;
	std::vector<EFFECT_EVALUATED_PARTICLE> Particles;
	std::vector<EFFECT_EVALUATED_TRAIL> Trails;
	std::vector<EFFECT_EVALUATED_AFTERIMAGE> AfterImages;
	std::vector<EFFECT_EVALUATED_LIGHT> Lights;
	std::vector<EFFECT_EVALUATED_SCREEN_POST> ScreenPosts;
};

struct EFFECT_FIXED_STEP_TRANSFORM_SAMPLE final
{
	float4x4_t RootWorld{};
	std::unordered_map<std::string, float4x4_t> SourceAnchorWorlds;
};

using EFFECT_FIXED_STEP_TRANSFORM_PROVIDER = std::function<bool_t(
	f32_t,
	EFFECT_FIXED_STEP_TRANSFORM_SAMPLE&,
	std::string&)>;

struct EFFECT_PARTICLE_RUNTIME_PROBE final
{
	f32_t fSampleTimeSeconds = 0.f;
	uint32_t iActiveParticleCount = 0u;
	bool_t bMeshRenderer = false;
	float4_t vFirstDynamicParameter{};
	float4_t vMinDynamicParameter{};
	float4_t vMaxDynamicParameter{};
	f32_t fFirstAlpha = 0.f;
	f32_t fMinAlpha = 0.f;
	f32_t fMaxAlpha = 0.f;
	f32_t fFirstNormalizedLife = 0.f;
	f32_t fFirstSubImageIndex = 0.f;
	EFFECT_SUBUV_FRAME_DESC FirstSubUV{};
};

class CEffectPlayback final
{
public:
	struct PREPARED_RESOURCES;

private:
	struct PARTICLE_STATE final
	{
		float3_t vPosition{};
		float3_t vVelocity{};
		float3_t vBaseVelocity{};
		/* LocationDirect is an update module, not a spawn-only offset.  Preserve
		   its last exact contribution so each normalized-life sample can replace
		   that contribution without erasing other Location/Velocity modules. */
		float3_t vSourceDirectLocationContribution{};
		float3_t vSourceDirectDirectionContribution{};
		float3_t vInheritedParentVelocity{};
		float3_t vVelocityScale = { 1.f, 1.f, 1.f };
		float3_t vBaseSize = { 1.f, 1.f, 1.f };
		float3_t vSize = { 1.f, 1.f, 1.f };
		float3_t vRotationDegrees{};
		float3_t vRotationRateDegreesPerSecond{};
		float3_t vRotationRateScale = { 1.f, 1.f, 1.f };
		float3_t vSourceMeshRotationDegrees{};
		float3_t vSourceMeshRotationRateDegreesPerSecond{};
		float3_t vSourceMeshRotationRateScale = { 1.f, 1.f, 1.f };
		float4_t vBaseColor = { 1.f, 1.f, 1.f, 1.f };
		float4_t vColor = { 1.f, 1.f, 1.f, 1.f };
		float4_t vDynamicParameter{};
		float3_t vOrbitOffset{};
		float3_t vSourceOrbitRotationDegrees{};
		float3_t vSourceOrbitRotationRateDegreesPerSecond{};
		f32_t fVectorFieldScale = 1.f;
		f32_t fCameraOffset = 0.f;
		f32_t fSubImageIndex = 0.f;
		f32_t fDistributionRandom = 0.f;
		f32_t fSpawnEmitterTimeSeconds = 0.f;
		f32_t fAgeSeconds = 0.f;
		f32_t fLifeTimeSeconds = 1.f;
		uint64_t iSpawnSimulationStep = 0u;
		std::string strSourceAnchorName;
		float3_t vSourceAnchorOffset{};
		bool_t bUpdateSourceAnchor = false;
		float4x4_t SpawnRootWorld{};
	};

	struct MODULE_RANDOM_STATE final
	{
		uint32_t iInitialSeed = 0u;
		uint32_t iCurrentSeed = 0u;
		bool_t bResetOnEmitterLoop = true;
	};

	struct SOURCE_PARTICLE_EVENT final
	{
		std::string strType;
		std::string strName;
		f32_t fEmitterTimeSeconds = 0.f;
		float3_t vPosition{};
		float3_t vVelocity{};
	};

	struct AFTERIMAGE_STATE final
	{
		float4x4_t World{};
		f32_t fAgeSeconds = 0.f;
	};

	struct ELEMENT_STATE final
	{
		uint32_t iRandomState = 1u;
		f32_t fSpawnAccumulator = 0.f;
		f32_t fSourceSpawnPerUnitAccumulator = 0.f;
		f32_t fTrailSampleAccumulator = 0.f;
		f32_t fTrailCumulativeDistance = 0.f;
		f32_t fAfterImageAccumulator = 0.f;
		bool_t bBurstSpawned = false;
		bool_t bSourceSpawnPerUnitOriginInitialized = false;
		float3_t vSourceSpawnPerUnitPreviousOrigin{};
		bool_t bActionRootCaptured = false;
		float4x4_t ActionRootWorld{};
		uint32_t iSourceLoopIndex = 0u;
		size_t iNextSourceBurst = 0u;
		std::unordered_map<std::string, MODULE_RANDOM_STATE> ModuleRandomStates;
		std::unordered_map<std::string, uint32_t> EventTrackingCounts;
		std::unordered_map<std::string, uint32_t> BoneSocketNextIndices;
		std::vector<PARTICLE_STATE> Particles;
		std::vector<EFFECT_EVALUATED_TRAIL_POINT> TrailPoints;
		std::vector<AFTERIMAGE_STATE> AfterImages;
	};

public:
	static bool_t Prepare_DocumentResources(
		const EFFECT_DOCUMENT_DESC& Document,
		std::shared_ptr<const PREPARED_RESOURCES>& OutPrepared,
		std::string& strOutError,
		std::shared_ptr<const EFFECT_DOCUMENT_DESC> pImmutableDocument = nullptr);
	bool_t Stage_Document(
		const EFFECT_DOCUMENT_DESC& Document,
		std::string& strOutError);
	bool_t Stage_PrevalidatedDocument(
		const EFFECT_DOCUMENT_DESC& Document,
		std::shared_ptr<const PREPARED_RESOURCES> pPreparedResources,
		std::string& strOutError);
	bool_t Stage_PrevalidatedVisualProgramDocument(
		std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>
			pProjection,
		std::shared_ptr<const PREPARED_RESOURCES> pPreparedResources,
		std::string& strOutError);
	bool_t Stage_ReconstructedSourceRuntime(
		const EFFECT_DOCUMENT_DESC& Document,
		std::shared_ptr<const PREPARED_RESOURCES> pPreparedResources,
		std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION>
			pPreparation,
		std::string& strOutError);
	bool_t Stage_ReconstructedSourceRuntimeWithVisualProgramAdapter(
		std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>
			pProjection,
		std::shared_ptr<const PREPARED_RESOURCES> pPreparedResources,
		std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION>
			pPreparation,
		std::string& strOutError);
	bool_t Stage_ReconstructedRuntimeProgram(
		std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION>
			pPreparation,
		std::string& strOutError);
	void Reset();
	void Update(f32_t fTimeDelta, const float4x4_t& RootWorld);
	void Seek(f32_t fSampleTimeSeconds, const float4x4_t& RootWorld);
	bool_t Update_WithTransformHistory(
		f32_t fTimeDelta,
		const EFFECT_FIXED_STEP_TRANSFORM_PROVIDER& TransformProvider,
		std::string& strOutError);
	bool_t Seek_WithTransformHistory(
		f32_t fSampleTimeSeconds,
		const EFFECT_FIXED_STEP_TRANSFORM_PROVIDER& TransformProvider,
		std::string& strOutError);
	void Set_SourceAnchorWorlds(
		const std::unordered_map<std::string, float4x4_t>& SourceAnchorWorlds);
	void Set_SourceAnchorWorlds(
		std::unordered_map<std::string, float4x4_t>&& SourceAnchorWorlds);
	const EFFECT_EVALUATED_FRAME& Get_Frame() const { return m_Frame; }
	bool_t Query_ParticleRuntimeProbe(
		std::string_view strElementId,
		EFFECT_PARTICLE_RUNTIME_PROBE& OutProbe) const;
	bool_t Is_Finished() const;
	f32_t Get_DurationSeconds() const { return m_fDurationSeconds; }
	f64_t Get_FixedStepClockSeconds() const;
	bool_t Is_ReconstructedSourceRuntimeActive() const
	{
		return m_bReconstructedSourceRuntimeActive;
	}
	bool_t Is_SourceVisualProgramActive() const
	{
		return m_bSourceVisualProgramActive;
	}
	const std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>&
		Get_SourceVisualProgramProjection() const
	{
		return m_pSourceVisualProgramProjection;
	}
	const std::string& Get_ReconstructedSourceRuntimeStatus() const
	{
		return m_strSourceVisualProgramStatus;
	}
	const std::string& Get_SourceVisualProgramStatus() const
	{
		return m_strSourceVisualProgramStatus;
	}
	static EFFECT_SUBUV_FRAME_DESC Resolve_SourceSubUVFrame(
		uint32_t iColumns,
		uint32_t iRows,
		f32_t fFrameValue,
		bool_t bAllowFlip,
		bool_t bSquareFlip,
		f32_t fDistributionRandom,
		bool_t bLinearBlend);
	static bool_t Resolve_ParticleSpriteSubUV(
		const EFFECT_EVALUATED_PARTICLE& Particle,
		const EFFECT_SUBUV_FRAME_DESC& SourceSubUV,
		EFFECT_SUBUV_FRAME_DESC& OutSubUV);
	static uint64_t Get_VectorFieldDiskLoadCount();
	std::shared_ptr<const EFFECT_RUNTIME_PROGRAM_CATALOG_ENTRY>
		Get_ReconstructedRuntimeEntry() const
	{
		return m_ReconstructedRuntimeBoundary.Get_CatalogEntry();
	}
	std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION>
		Get_ReconstructedRuntimePreparation() const
	{
		return m_ReconstructedRuntimeBoundary.Get_Preparation();
	}
	std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM>
		Get_ReconstructedRuntimeProgram() const
	{
		return m_ReconstructedRuntimeBoundary.Get_Program();
	}
	std::shared_ptr<const EFFECT_RECONSTRUCTED_EXECUTION_PLAN>
		Get_ReconstructedExecutionPlan() const
	{
		return m_pReconstructedExecutionPlan;
	}

private:
	const EFFECT_DOCUMENT_DESC& Get_StagedDocument() const;
	bool_t Stage_PrevalidatedDocumentInternal(
		const EFFECT_DOCUMENT_DESC& Document,
		std::shared_ptr<const PREPARED_RESOURCES> pPreparedResources,
		bool_t bSourceVisualProgramActive,
		bool_t bRequireSourceVisualTargetClosure,
		std::unordered_set<std::string> SourceVisualTargetElementIds,
		std::string& strOutError);
	bool_t Is_SourceVisualProgramElementAdmitted(
		const EFFECT_ELEMENT_DESC& Element) const;
	bool_t Collect_TransformHistorySample(
		f32_t fSampleTimeSeconds,
		const EFFECT_FIXED_STEP_TRANSFORM_PROVIDER& TransformProvider,
		EFFECT_FIXED_STEP_TRANSFORM_SAMPLE& OutSample,
		std::string& strOutError) const;
	bool_t Step(f32_t fFixedDelta, const float4x4_t& RootWorld);
	void Rebuild_Frame(const float4x4_t& RootWorld);
	void Spawn_Particles(
		const EFFECT_ELEMENT_DESC& Element,
		ELEMENT_STATE& State,
		uint32_t iCount,
		const float4x4_t& RootWorld,
		const SOURCE_PARTICLE_EVENT* pSourceEvent = nullptr);
	uint32_t Consume_SourceSpawnPerUnit(
		const EFFECT_ELEMENT_DESC& Element,
		ELEMENT_STATE& State,
		f32_t fEmitterTimeSeconds,
		const float4x4_t& RootWorld);
	void Update_Particles(
		const EFFECT_ELEMENT_DESC& Element,
		ELEMENT_STATE& State,
		f32_t fFixedDelta,
		const float4x4_t& RootWorld);
	void Apply_SourceSpawnModules(
		const EFFECT_ELEMENT_DESC& Element,
		ELEMENT_STATE& State,
		PARTICLE_STATE& Particle,
		f32_t fEmitterTimeSeconds,
		const float4x4_t& ElementWorld);
	void Apply_SourceUpdateModules(
		const EFFECT_ELEMENT_DESC& Element,
		ELEMENT_STATE& State,
		PARTICLE_STATE& Particle,
		f32_t fEmitterTimeSeconds,
		f32_t fNormalizedAge,
		f32_t fFixedDelta,
		const float4x4_t& ElementWorld);
	void Initialize_ModuleRandomStates(
		const EFFECT_ELEMENT_DESC& Element,
		ELEMENT_STATE& State);
	void Reset_LoopingModuleRandomStates(ELEMENT_STATE& State);
	f32_t Next_ModuleRandom(
		ELEMENT_STATE& State,
		const EFFECT_SOURCE_MODULE_DESC& Module);
	f32_t Evaluate_ModuleFloat(
		ELEMENT_STATE& State,
		const EFFECT_SOURCE_MODULE_DESC& Module,
		std::string_view PropertyPath,
		f32_t fTime,
		f32_t fFallback);
	float3_t Evaluate_ModuleVector(
		ELEMENT_STATE& State,
		const EFFECT_SOURCE_MODULE_DESC& Module,
		std::string_view PropertyPath,
		f32_t fTime,
		const float3_t& Fallback);
	void Queue_SpawnEvents(
		const EFFECT_ELEMENT_DESC& Element,
		ELEMENT_STATE& State,
		const PARTICLE_STATE& Particle,
		const float4x4_t& ElementWorld);
	bool_t Dispatch_SourceEvents(
		f32_t fFixedDelta,
		const float4x4_t& RootWorld);
	f32_t Evaluate_SourceFloat(
		const EFFECT_ELEMENT_DESC& Element,
		const char_t* pModuleClass,
		const char_t* pPropertyPath,
		f32_t fTime,
		f32_t fRandomUnit,
		f32_t fFallback) const;
	float3_t Evaluate_SourceVector(
		const EFFECT_ELEMENT_DESC& Element,
		const char_t* pModuleClass,
		const char_t* pPropertyPath,
		f32_t fTime,
		f32_t fRandomUnit,
		const float3_t& Fallback) const;
	void Sample_Trail(
		const EFFECT_ELEMENT_DESC& Element,
		ELEMENT_STATE& State,
		f32_t fFixedDelta,
		const float4x4_t& RootWorld);
	void Sample_AfterImages(
		const EFFECT_ELEMENT_DESC& Element,
		ELEMENT_STATE& State,
		f32_t fFixedDelta,
		const float4x4_t& RootWorld);
	bool_t Can_EvaluateElementWorld(
		const EFFECT_ELEMENT_DESC& Element) const;
	float4x4_t Evaluate_ElementWorld(
		const EFFECT_ELEMENT_DESC& Element,
		f32_t fSampleTimeSeconds,
		const float4x4_t& RootWorld) const;
	float3_t Apply_ParticleEmissionModifier(
		const float3_t& Velocity) const;
	EFFECT_COLOR_DESC Evaluate_Color(
		const EFFECT_ELEMENT_DESC& Element,
		f32_t fNormalizedLife) const;
	uint32_t Next_Random(ELEMENT_STATE& State) const;
	f32_t Random_Range(ELEMENT_STATE& State, f32_t fMin, f32_t fMax) const;
	/* Authored spawn geometry.  Both keep the historical min/max box when the
	   Element still declares POINT and FIXED, so documents written before the
	   spawnShape/initialVelocity blocks existed spawn exactly as they did. */
	float3_t Sample_AuthoredSpawnPosition(
		const EFFECT_PARTICLE_DESC& Desc,
		ELEMENT_STATE& State) const;
	float3_t Sample_AuthoredInitialVelocity(
		const EFFECT_PARTICLE_DESC& Desc,
		ELEMENT_STATE& State,
		const float3_t& vSpawnPosition) const;

private:
	EFFECT_DOCUMENT_DESC m_Document;
	std::shared_ptr<const PREPARED_RESOURCES> m_pPreparedResources;
	CEffectReconstructedRuntimeBoundary m_ReconstructedRuntimeBoundary;
	std::shared_ptr<const EFFECT_RECONSTRUCTED_EXECUTION_PLAN>
		m_pReconstructedExecutionPlan;
	std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>
		m_pSourceVisualProgramProjection;
	std::unordered_map<std::string, ELEMENT_STATE> m_States;
	std::unordered_map<std::string, size_t> m_TransformMasterIndices;
	std::unordered_map<std::string, float4x4_t> m_SourceAnchorWorlds;
	std::vector<SOURCE_PARTICLE_EVENT> m_PendingSourceEvents;
	float3_t m_vPreviousRootPosition{};
	float3_t m_vParentVelocity{};
	EFFECT_EVALUATED_FRAME m_Frame;
	f32_t m_fSampleTimeSeconds = 0.f;
	f64_t m_fAccumulatorSeconds = 0.0;
	f32_t m_fDurationSeconds = 0.f;
	uint64_t m_iSimulationStep = 0u;
	bool_t m_bSourceVisualProgramActive = false;
	bool_t m_bPreviousRootPositionInitialized = false;
	bool_t m_bHasPortableSourceEvents = false;
	bool_t m_bSourceEventQueueOverflow = false;
	bool_t m_bRequireSourceVisualTargetClosure = false;
	bool_t m_bReconstructedSourceRuntimeActive = false;
	std::unordered_set<std::string> m_SourceVisualTargetElementIds;
	std::string m_strSourceVisualProgramStatus;
};

NS_END
