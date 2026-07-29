#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"
#include "Effect_Types.h"

NS_BEGIN(Client)

/*
 * Phase 2 CPU reference simulator.
 *
 * This class owns no D3D resource, ImGui state, or file handle. A future GPU
 * renderer consumes Get_Particles(), while the editor and asset loader only
 * edit/create EFFECT_EMITTER_DESC.
 */
class CEffect_ParticleSimulator final
{
public:
	bool_t Initialize(const EFFECT_EMITTER_DESC& EmitterDesc,
		uint32_t iRandomSeed = 0x13572468u);
	void Reset();
	void Update(f32_t fTimeDelta);
	void Set_LODDistance(f32_t fDistance) {
		m_fLODDistance = max(0.f, fDistance);
	}

	const vector<EFFECT_PARTICLE>& Get_Particles() const { return m_Particles; }
	const EFFECT_PARTICLE_SIMULATION_STATS& Get_Stats() const { return m_Stats; }
	f32_t Get_ElapsedTime() const { return m_fElapsedTime; }

private:
	const EFFECT_MODULE_DESC* Find_Module(EFFECT_MODULE_TYPE eType) const;
	f32_t Calculate_ActiveTime(f32_t fBeginTime, f32_t fEndTime) const;
	f32_t Sample(const EFFECT_DISTRIBUTION_FLOAT_DESC& Desc);
	float3_t Sample(const EFFECT_DISTRIBUTION_VECTOR_DESC& Desc);
	float4_t Sample(const EFFECT_DISTRIBUTION_COLOR_DESC& Desc);
	f32_t Evaluate(const EFFECT_CURVE_FLOAT_DESC& Curve,
		f32_t fNormalizedTime) const;
	f32_t Next_Random01();
	void Spawn(uint32_t iCount);

private:
	EFFECT_EMITTER_DESC m_EmitterDesc;
	vector<EFFECT_PARTICLE> m_Particles;
	EFFECT_PARTICLE_SIMULATION_STATS m_Stats;
	f32_t m_fElapsedTime = {};
	f32_t m_fSpawnAccumulator = {};
	f32_t m_fLODDistance = {};
	uint32_t m_iInitialRandomSeed = { 0x13572468u };
	uint32_t m_iRandomState = { 0x13572468u };
	bool_t m_isBurstSpawned = { false };
};

EFFECT_PARTICLE_COUNT_VALIDATION_RESULT
	Run_Phase2ParticleCountValidation();

NS_END
