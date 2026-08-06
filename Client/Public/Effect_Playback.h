#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"
#include "Effect_AuthoringDocument.h"

#include <string>
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

struct EFFECT_EVALUATED_PARTICLE final
{
	const EFFECT_ELEMENT_DESC* pElement = nullptr;
	float4x4_t World{};
	float4_t Color = { 1.f, 1.f, 1.f, 1.f };
	f32_t fNormalizedLife = 0.f;
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

struct EFFECT_EVALUATED_FRAME final
{
	f32_t fSampleTimeSeconds = 0.f;
	std::vector<EFFECT_EVALUATED_ELEMENT> Elements;
	std::vector<EFFECT_EVALUATED_PARTICLE> Particles;
	std::vector<EFFECT_EVALUATED_TRAIL> Trails;
	std::vector<EFFECT_EVALUATED_AFTERIMAGE> AfterImages;
};

class CEffectPlayback final
{
private:
	struct PARTICLE_STATE final
	{
		float3_t vPosition{};
		float3_t vVelocity{};
		f32_t fAgeSeconds = 0.f;
		f32_t fLifeTimeSeconds = 1.f;
		float4x4_t SpawnRootWorld{};
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
		std::vector<PARTICLE_STATE> Particles;
		std::vector<EFFECT_EVALUATED_TRAIL_POINT> TrailPoints;
		std::vector<AFTERIMAGE_STATE> AfterImages;
	};

public:
	bool_t Stage_Document(
		const EFFECT_DOCUMENT_DESC& Document,
		std::string& strOutError);
	void Reset();
	void Update(f32_t fTimeDelta, const float4x4_t& RootWorld);
	void Seek(f32_t fSampleTimeSeconds, const float4x4_t& RootWorld);
	const EFFECT_EVALUATED_FRAME& Get_Frame() const { return m_Frame; }
	bool_t Is_Finished() const;
	f32_t Get_DurationSeconds() const { return m_fDurationSeconds; }

private:
	void Step(f32_t fFixedDelta, const float4x4_t& RootWorld);
	void Rebuild_Frame(const float4x4_t& RootWorld);
	void Spawn_Particles(
		const EFFECT_ELEMENT_DESC& Element,
		ELEMENT_STATE& State,
		uint32_t iCount,
		const float4x4_t& RootWorld);
	void Update_Particles(
		const EFFECT_ELEMENT_DESC& Element,
		ELEMENT_STATE& State,
		f32_t fFixedDelta);
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
	float4x4_t Evaluate_ElementWorld(
		const EFFECT_ELEMENT_DESC& Element,
		f32_t fSampleTimeSeconds,
		const float4x4_t& RootWorld) const;
	EFFECT_COLOR_DESC Evaluate_Color(
		const EFFECT_ELEMENT_DESC& Element,
		f32_t fNormalizedLife) const;
	uint32_t Next_Random(ELEMENT_STATE& State) const;
	f32_t Random_Range(ELEMENT_STATE& State, f32_t fMin, f32_t fMax) const;

private:
	EFFECT_DOCUMENT_DESC m_Document;
	std::unordered_map<std::string, ELEMENT_STATE> m_States;
	EFFECT_EVALUATED_FRAME m_Frame;
	f32_t m_fSampleTimeSeconds = 0.f;
	f64_t m_fAccumulatorSeconds = 0.0;
	f32_t m_fDurationSeconds = 0.f;
	uint64_t m_iSimulationStep = 0u;
};

NS_END
