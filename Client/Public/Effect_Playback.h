#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"
#include "Effect_AuthoringDocument.h"
#include "Effect_Catalog.h"

#include <string>
#include <string_view>
#include <memory>
#include <unordered_map>
#include <vector>

NS_BEGIN(Client)

struct EFFECT_EVALUATED_ELEMENT final
{
	const EFFECT_ELEMENT_DESC* pElement = nullptr;
	float4x4_t World{};
	EFFECT_COLOR_DESC Color{};
	f32_t fLocalTimeSeconds = 0.f;
	f32_t fNormalizedLife = 0.f;
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
	EFFECT_PARTICLE_SPRITE_ALIGNMENT eSpriteAlignment =
		EFFECT_PARTICLE_SPRITE_ALIGNMENT::CAMERA_RECTANGLE;
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

struct EFFECT_EVALUATED_FRAME final
{
	f32_t fSampleTimeSeconds = 0.f;
	float4x4_t RootWorld{};
	std::vector<EFFECT_EVALUATED_ELEMENT> Elements;
	std::vector<EFFECT_EVALUATED_PARTICLE> Particles;
	std::vector<EFFECT_EVALUATED_TRAIL> Trails;
	std::vector<EFFECT_EVALUATED_AFTERIMAGE> AfterImages;
	std::vector<EFFECT_EVALUATED_LIGHT> Lights;
	std::vector<EFFECT_EVALUATED_SCREEN_POST> ScreenPosts;
};

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
		float3_t vVelocityScale = { 1.f, 1.f, 1.f };
		float3_t vBaseSize = { 1.f, 1.f, 1.f };
		float3_t vSize = { 1.f, 1.f, 1.f };
		float3_t vRotationDegrees{};
		float3_t vRotationRateDegreesPerSecond{};
		float3_t vRotationRateScale = { 1.f, 1.f, 1.f };
		float4_t vBaseColor = { 1.f, 1.f, 1.f, 1.f };
		float4_t vColor = { 1.f, 1.f, 1.f, 1.f };
		float4_t vDynamicParameter{};
		float3_t vOrbitOffset{};
		float3_t vOrbitRotationDegrees{};
		float3_t vOrbitRotationRateDegreesPerSecond{};
		f32_t fCameraOffset = 0.f;
		f32_t fSubImageIndex = 0.f;
		f32_t fDistributionRandom = 0.f;
		f32_t fSpawnEmitterTimeSeconds = 0.f;
		f32_t fAgeSeconds = 0.f;
		f32_t fLifeTimeSeconds = 1.f;
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
		f32_t fTrailSampleAccumulator = 0.f;
		f32_t fAfterImageAccumulator = 0.f;
		bool_t bBurstSpawned = false;
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
		std::string& strOutError);
	bool_t Stage_Document(
		const EFFECT_DOCUMENT_DESC& Document,
		std::string& strOutError);
	bool_t Stage_PrevalidatedDocument(
		const EFFECT_DOCUMENT_DESC& Document,
		std::shared_ptr<const PREPARED_RESOURCES> pPreparedResources,
		std::string& strOutError);
	bool_t Stage_ReconstructedRuntimeProgram(
		std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION>
			pPreparation,
		std::string& strOutError);
	void Reset();
	void Update(f32_t fTimeDelta, const float4x4_t& RootWorld);
	void Seek(f32_t fSampleTimeSeconds, const float4x4_t& RootWorld);
	void Set_SourceAnchorWorlds(
		const std::unordered_map<std::string, float4x4_t>& SourceAnchorWorlds);
	const EFFECT_EVALUATED_FRAME& Get_Frame() const { return m_Frame; }
	bool_t Query_ParticleRuntimeProbe(
		std::string_view strElementId,
		EFFECT_PARTICLE_RUNTIME_PROBE& OutProbe) const;
	bool_t Is_Finished() const;
	f32_t Get_DurationSeconds() const { return m_fDurationSeconds; }
	static EFFECT_SUBUV_FRAME_DESC Resolve_SourceSubUVFrame(
		uint32_t iColumns,
		uint32_t iRows,
		f32_t fFrameValue,
		bool_t bAllowFlip,
		bool_t bSquareFlip,
		f32_t fDistributionRandom,
		bool_t bLinearBlend);
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

private:
	void Step(f32_t fFixedDelta, const float4x4_t& RootWorld);
	void Rebuild_Frame(const float4x4_t& RootWorld);
	void Spawn_Particles(
		const EFFECT_ELEMENT_DESC& Element,
		ELEMENT_STATE& State,
		uint32_t iCount,
		const float4x4_t& RootWorld,
		const SOURCE_PARTICLE_EVENT* pSourceEvent = nullptr);
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
	void Dispatch_SourceEvents(
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

private:
	EFFECT_DOCUMENT_DESC m_Document;
	std::shared_ptr<const PREPARED_RESOURCES> m_pPreparedResources;
	CEffectReconstructedRuntimeBoundary m_ReconstructedRuntimeBoundary;
	std::unordered_map<std::string, ELEMENT_STATE> m_States;
	std::unordered_map<std::string, size_t> m_TransformMasterIndices;
	std::unordered_map<std::string, float4x4_t> m_SourceAnchorWorlds;
	std::vector<SOURCE_PARTICLE_EVENT> m_PendingSourceEvents;
	EFFECT_EVALUATED_FRAME m_Frame;
	f32_t m_fSampleTimeSeconds = 0.f;
	f64_t m_fAccumulatorSeconds = 0.0;
	f32_t m_fDurationSeconds = 0.f;
	uint64_t m_iSimulationStep = 0u;
};

NS_END
