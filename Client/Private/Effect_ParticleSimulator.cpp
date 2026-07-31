#include "Effect_ParticleSimulator.h"

#include <cmath>

namespace
{
	using namespace Client;

	constexpr f32_t MIN_PARTICLE_LIFETIME = 0.0001f;

	f32_t Lerp(const f32_t fMin, const f32_t fMax, const f32_t fRatio)
	{
		return fMin + (fMax - fMin) * fRatio;
	}

	uint32_t CalculateSubUVFrame(
		const EFFECT_SUB_UV_MODULE_DESC& Desc,
		const EFFECT_PARTICLE& Particle)
	{
		const uint64_t iRawFrameCount =
			static_cast<uint64_t>(max(1u, Desc.iColumns)) *
			static_cast<uint64_t>(max(1u, Desc.iRows));
		const uint64_t iTotalFrameCount =
			min(iRawFrameCount, 0x100000000ull);
		const uint64_t iLastAvailableFrame = iTotalFrameCount - 1;
		const uint64_t iStartFrame =
			min(static_cast<uint64_t>(Desc.iStartFrame),
				iLastAvailableFrame);
		const uint64_t iEndFrame = max(iStartFrame,
			min(static_cast<uint64_t>(Desc.iEndFrame),
				iLastAvailableFrame));
		const uint64_t iFrameCount = iEndFrame - iStartFrame + 1;
		if (iFrameCount <= 1)
			return static_cast<uint32_t>(iStartFrame);

		double dFrameCursor = {};
		if (true == Desc.useParticleRelativeTime)
		{
			dFrameCursor =
				static_cast<double>(clamp(
					Particle.fRelativeTime, 0.f, 1.f)) *
				static_cast<double>(iFrameCount);
		}
		else
		{
			dFrameCursor =
				static_cast<double>(max(0.f, Particle.fAge)) *
				static_cast<double>(max(0.f,
					Desc.fFramesPerSecond));
		}

		uint64_t iFrameOffset = {};
		if (true == Desc.isLoop)
		{
			iFrameOffset = static_cast<uint64_t>(fmod(
				floor(dFrameCursor),
				static_cast<double>(iFrameCount)));
		}
		else
		{
			iFrameOffset = static_cast<uint64_t>(min(
				floor(dFrameCursor),
				static_cast<double>(iFrameCount - 1)));
		}
		return static_cast<uint32_t>(iStartFrame + iFrameOffset);
	}
}

bool_t Client::CEffect_ParticleSimulator::Initialize(
	const EFFECT_EMITTER_DESC& EmitterDesc,
	const uint32_t iRandomSeed)
{
	m_EmitterDesc = EmitterDesc;
	for (EFFECT_MODULE_DESC& Module : m_EmitterDesc.Modules)
	{
		if (EFFECT_MODULE_TYPE::SPAWN == Module.eType)
		{
			Module.Spawn.iMaxParticles = min(
				Module.Spawn.iMaxParticles,
				EFFECT_MAX_PARTICLES_PER_EMITTER);
			Module.Spawn.iBurstCount = min(
				Module.Spawn.iBurstCount,
				Module.Spawn.iMaxParticles);
		}
		else if (EFFECT_MODULE_TYPE::TRAIL == Module.eType)
		{
			Module.Trail.iMaxPoints = min(
				Module.Trail.iMaxPoints,
				EFFECT_MAX_TRAIL_POINTS);
		}
		else if (EFFECT_MODULE_TYPE::BEAM == Module.eType)
		{
			Module.Beam.iSegments = min(
				Module.Beam.iSegments,
				EFFECT_MAX_BEAM_SEGMENTS);
		}
		else if (EFFECT_MODULE_TYPE::SUB_UV == Module.eType)
		{
			Module.SubUV.iColumns = clamp(
				Module.SubUV.iColumns, 1u,
				EFFECT_MAX_SUBUV_DIMENSION);
			Module.SubUV.iRows = clamp(
				Module.SubUV.iRows, 1u,
				EFFECT_MAX_SUBUV_DIMENSION);
		}
	}

	const EFFECT_MODULE_DESC* pSpawnModule =
		Find_Module(EFFECT_MODULE_TYPE::SPAWN);
	const EFFECT_MODULE_DESC* pLifetimeModule =
		Find_Module(EFFECT_MODULE_TYPE::LIFETIME);

	if (nullptr == pSpawnModule ||
		nullptr == pLifetimeModule ||
		0 == pSpawnModule->Spawn.iMaxParticles)
	{
		return false;
	}

	m_iInitialRandomSeed =
		(0 == iRandomSeed) ? 0x13572468u : iRandomSeed;
	m_Particles.reserve(pSpawnModule->Spawn.iMaxParticles);
	Reset();
	return true;
}

void Client::CEffect_ParticleSimulator::Reset()
{
	m_Particles.clear();
	m_Stats = {};
	m_fElapsedTime = {};
	m_fSpawnAccumulator = {};
	m_iRandomState = m_iInitialRandomSeed;
	m_isBurstSpawned = false;
}

void Client::CEffect_ParticleSimulator::Update(const f32_t fTimeDelta)
{
	if (false == m_EmitterDesc.isEnabled || fTimeDelta <= 0.f)
		return;

	for (EFFECT_PARTICLE& Particle : m_Particles)
	{
		Particle.fAge += fTimeDelta;
		Particle.fRelativeTime =
			min(Particle.fAge / Particle.fLifetime, 1.f);

		const EFFECT_MODULE_DESC* pVelocityOverLife =
			Find_Module(EFFECT_MODULE_TYPE::VELOCITY_OVER_LIFE);
		if (nullptr != pVelocityOverLife)
		{
			const f32_t fScale = Evaluate(
				pVelocityOverLife->VelocityOverLife.Curve,
				Particle.fRelativeTime);
			Particle.vVelocity = {
				Particle.vInitialVelocity.x * fScale,
				Particle.vInitialVelocity.y * fScale,
				Particle.vInitialVelocity.z * fScale
			};
		}

		Particle.vPosition.x += Particle.vVelocity.x * fTimeDelta;
		Particle.vPosition.y += Particle.vVelocity.y * fTimeDelta;
		Particle.vPosition.z += Particle.vVelocity.z * fTimeDelta;

		Particle.vRotation.x +=
			Particle.vRotationRate.x * fTimeDelta;
		Particle.vRotation.y +=
			Particle.vRotationRate.y * fTimeDelta;
		Particle.vRotation.z +=
			Particle.vRotationRate.z * fTimeDelta;

		const EFFECT_MODULE_DESC* pSizeOverLife =
			Find_Module(EFFECT_MODULE_TYPE::SIZE_OVER_LIFE);
		if (nullptr != pSizeOverLife)
		{
			const f32_t fScale = Evaluate(
				pSizeOverLife->SizeOverLife.Curve,
				Particle.fRelativeTime);
			Particle.vSize = {
				Particle.vInitialSize.x * fScale,
				Particle.vInitialSize.y * fScale,
				Particle.vInitialSize.z * fScale
			};
		}

		const EFFECT_MODULE_DESC* pColorOverLife =
			Find_Module(EFFECT_MODULE_TYPE::COLOR_OVER_LIFE);
		if (nullptr != pColorOverLife)
		{
			Particle.vColor = {
				Particle.vInitialColor.x * Evaluate(
					pColorOverLife->ColorOverLife.Red,
					Particle.fRelativeTime),
				Particle.vInitialColor.y * Evaluate(
					pColorOverLife->ColorOverLife.Green,
					Particle.fRelativeTime),
				Particle.vInitialColor.z * Evaluate(
					pColorOverLife->ColorOverLife.Blue,
					Particle.fRelativeTime),
				Particle.vInitialColor.w * Evaluate(
					pColorOverLife->ColorOverLife.Alpha,
					Particle.fRelativeTime)
			};
		}

		const EFFECT_MODULE_DESC* pAlphaOverLife =
			Find_Module(EFFECT_MODULE_TYPE::ALPHA_OVER_LIFE);
		if (nullptr != pAlphaOverLife)
		{
			const f32_t fAlphaScale = Evaluate(
				pAlphaOverLife->AlphaOverLife.Curve,
				Particle.fRelativeTime);
			Particle.vColor.w =
				(nullptr != pColorOverLife)
				? Particle.vColor.w * fAlphaScale
				: Particle.vInitialColor.w * fAlphaScale;
		}

		const EFFECT_MODULE_DESC* pDynamicParameter =
			Find_Module(EFFECT_MODULE_TYPE::DYNAMIC_PARAMETER);
		if (nullptr != pDynamicParameter)
		{
			Particle.vDynamicParameter = {
				Evaluate(pDynamicParameter->DynamicParameter.X,
					Particle.fRelativeTime),
				Evaluate(pDynamicParameter->DynamicParameter.Y,
					Particle.fRelativeTime),
				Evaluate(pDynamicParameter->DynamicParameter.Z,
					Particle.fRelativeTime),
				Evaluate(pDynamicParameter->DynamicParameter.W,
					Particle.fRelativeTime)
			};
		}

		const EFFECT_MODULE_DESC* pSubUV =
			Find_Module(EFFECT_MODULE_TYPE::SUB_UV);
		if (nullptr != pSubUV)
		{
			Particle.iSubUVFrame =
				CalculateSubUVFrame(pSubUV->SubUV, Particle);
		}

		const EFFECT_MODULE_DESC* pCollision =
			Find_Module(EFFECT_MODULE_TYPE::COLLISION);
		if (nullptr != pCollision)
		{
			auto CollideAxis = [this, &Particle, pCollision](
				f32_t& fPosition, f32_t& fVelocity,
				const f32_t fMin, const f32_t fMax)
			{
				if (fPosition < fMin || fPosition > fMax)
				{
					fPosition = clamp(fPosition, fMin, fMax);
					fVelocity = -fVelocity *
						clamp(pCollision->Collision.fRestitution, 0.f, 1.f);
					++m_Stats.iCollisionCount;
				}
			};
			CollideAxis(Particle.vPosition.x, Particle.vVelocity.x,
				pCollision->Collision.vMinBounds.x,
				pCollision->Collision.vMaxBounds.x);
			CollideAxis(Particle.vPosition.y, Particle.vVelocity.y,
				pCollision->Collision.vMinBounds.y,
				pCollision->Collision.vMaxBounds.y);
			CollideAxis(Particle.vPosition.z, Particle.vVelocity.z,
				pCollision->Collision.vMinBounds.z,
				pCollision->Collision.vMaxBounds.z);
		}

		const EFFECT_MODULE_DESC* pEvent =
			Find_Module(EFFECT_MODULE_TYPE::EVENT);
		if (nullptr != pEvent && !Particle.isEventTriggered &&
			Particle.fRelativeTime >=
			clamp(pEvent->Event.fNormalizedTime, 0.f, 1.f))
		{
			Particle.isEventTriggered = true;
			++m_Stats.iEventCount;
		}

		if (Particle.fAge >= Particle.fLifetime)
			Particle.isAlive = false;
	}

	const size_t iPreviousCount = m_Particles.size();
	erase_if(m_Particles,
		[](const EFFECT_PARTICLE& Particle)
		{
			return false == Particle.isAlive;
		});
	m_Stats.iTotalKilled += iPreviousCount - m_Particles.size();

	const f32_t fPreviousTime = m_fElapsedTime;
	m_fElapsedTime += fTimeDelta;

	const EFFECT_MODULE_DESC* pSpawnModule =
		Find_Module(EFFECT_MODULE_TYPE::SPAWN);
	if (nullptr == pSpawnModule)
		return;

	const f32_t fActiveTime =
		Calculate_ActiveTime(fPreviousTime, m_fElapsedTime);

	uint32_t iSpawnCount = {};
	if (false == m_isBurstSpawned &&
		fPreviousTime <= m_EmitterDesc.fDelay &&
		m_fElapsedTime >= m_EmitterDesc.fDelay)
	{
		iSpawnCount += pSpawnModule->Spawn.iBurstCount;
		m_isBurstSpawned = true;
	}

	f32_t fLODSpawnScale = 1.f;
	const EFFECT_MODULE_DESC* pLOD =
		Find_Module(EFFECT_MODULE_TYPE::LOD);
	if (nullptr != pLOD)
	{
		const f32_t fNear = max(0.f, pLOD->LOD.fNearDistance);
		const f32_t fFar = max(fNear + 0.0001f,
			pLOD->LOD.fFarDistance);
		const f32_t fRatio = clamp(
			(m_fLODDistance - fNear) / (fFar - fNear), 0.f, 1.f);
		fLODSpawnScale = 1.f +
			(clamp(pLOD->LOD.fFarSpawnScale, 0.f, 1.f) - 1.f) *
			fRatio;
	}
	const f32_t fSpawnContribution =
		max(0.f, pSpawnModule->Spawn.fRatePerSecond) *
		fLODSpawnScale * fActiveTime;
	m_fSpawnAccumulator = isfinite(fSpawnContribution)
		? min(
			m_fSpawnAccumulator + fSpawnContribution,
			static_cast<f32_t>(EFFECT_MAX_PARTICLES_PER_EMITTER))
		: static_cast<f32_t>(EFFECT_MAX_PARTICLES_PER_EMITTER);
	const uint32_t iRateSpawnCount =
		static_cast<uint32_t>(m_fSpawnAccumulator + 0.000001f);
	m_fSpawnAccumulator -= static_cast<f32_t>(iRateSpawnCount);
	iSpawnCount = static_cast<uint32_t>(min<uint64_t>(
		static_cast<uint64_t>(iSpawnCount) + iRateSpawnCount,
		EFFECT_MAX_PARTICLES_PER_EMITTER));

	Spawn(iSpawnCount);
	m_Stats.iAliveCount =
		static_cast<uint32_t>(m_Particles.size());
}

const EFFECT_MODULE_DESC*
Client::CEffect_ParticleSimulator::Find_Module(
	const EFFECT_MODULE_TYPE eType) const
{
	const auto Iter = find_if(
		m_EmitterDesc.Modules.begin(),
		m_EmitterDesc.Modules.end(),
		[eType](const EFFECT_MODULE_DESC& Module)
		{
			return true == Module.isEnabled && eType == Module.eType;
		});

	return (m_EmitterDesc.Modules.end() == Iter) ? nullptr : &(*Iter);
}

f32_t Client::CEffect_ParticleSimulator::Calculate_ActiveTime(
	const f32_t fBeginTime,
	const f32_t fEndTime) const
{
	if (fEndTime <= m_EmitterDesc.fDelay ||
		m_EmitterDesc.fDuration <= 0.f)
	{
		return 0.f;
	}

	const f32_t fActiveBegin = max(fBeginTime, m_EmitterDesc.fDelay);
	f32_t fActiveEnd = fEndTime;

	if (EFFECT_LOOP_FOREVER != m_EmitterDesc.iLoopCount)
	{
		const f32_t fEmissionEnd =
			m_EmitterDesc.fDelay +
			m_EmitterDesc.fDuration *
			static_cast<f32_t>(m_EmitterDesc.iLoopCount);
		fActiveEnd = min(fActiveEnd, fEmissionEnd);
	}

	return max(0.f, fActiveEnd - fActiveBegin);
}

f32_t Client::CEffect_ParticleSimulator::Sample(
	const EFFECT_DISTRIBUTION_FLOAT_DESC& Desc)
{
	if (EFFECT_DISTRIBUTION_TYPE::UNIFORM_RANGE == Desc.eType)
		return Lerp(Desc.fMin, Desc.fMax, Next_Random01());

	return Desc.fConstant;
}

float3_t Client::CEffect_ParticleSimulator::Sample(
	const EFFECT_DISTRIBUTION_VECTOR_DESC& Desc)
{
	if (EFFECT_DISTRIBUTION_TYPE::UNIFORM_RANGE == Desc.eType)
	{
		return {
			Lerp(Desc.vMin.x, Desc.vMax.x, Next_Random01()),
			Lerp(Desc.vMin.y, Desc.vMax.y, Next_Random01()),
			Lerp(Desc.vMin.z, Desc.vMax.z, Next_Random01())
		};
	}

	return Desc.vConstant;
}

float4_t Client::CEffect_ParticleSimulator::Sample(
	const EFFECT_DISTRIBUTION_COLOR_DESC& Desc)
{
	if (EFFECT_DISTRIBUTION_TYPE::UNIFORM_RANGE == Desc.eType)
	{
		return {
			Lerp(Desc.vMin.x, Desc.vMax.x, Next_Random01()),
			Lerp(Desc.vMin.y, Desc.vMax.y, Next_Random01()),
			Lerp(Desc.vMin.z, Desc.vMax.z, Next_Random01()),
			Lerp(Desc.vMin.w, Desc.vMax.w, Next_Random01())
		};
	}

	return Desc.vConstant;
}

f32_t Client::CEffect_ParticleSimulator::Next_Random01()
{
	m_iRandomState = 1664525u * m_iRandomState + 1013904223u;
	return static_cast<f32_t>(m_iRandomState >> 8) /
		static_cast<f32_t>(0x00ffffffu);
}

float3_t Client::CEffect_ParticleSimulator::Sample_Location(
	const EFFECT_INITIAL_LOCATION_MODULE_DESC& Desc)
{
	// The authored distribution is the offset of the whole volume. For BOX it
	// is also the volume itself, so nothing else is added.
	const float3_t vOffset = Sample(Desc.Location);
	if (EFFECT_LOCATION_SHAPE::BOX == Desc.eShape)
		return vOffset;

	const f32_t fOuter = max(0.f, Desc.fRadius);
	const f32_t fInner = min(max(0.f, Desc.fInnerRadius), fOuter);
	// Cube root keeps a solid sphere uniformly filled instead of clumping the
	// particles toward the centre. Surface only pins every particle to fOuter.
	const f32_t fUnit = Desc.isSurfaceOnly ? 1.f : powf(Next_Random01(),
		(EFFECT_LOCATION_SHAPE::SPHERE == Desc.eShape) ? (1.f / 3.f) : 0.5f);
	const f32_t fRadius = fInner + (fOuter - fInner) * fUnit;

	constexpr f32_t TWO_PI = 6.28318530718f;
	const f32_t fTheta = Next_Random01() * TWO_PI;
	if (EFFECT_LOCATION_SHAPE::CYLINDER == Desc.eShape)
	{
		return {
			vOffset.x + fRadius * cosf(fTheta),
			vOffset.y + (Next_Random01() - 0.5f) * Desc.fHeight,
			vOffset.z + fRadius * sinf(fTheta)
		};
	}

	// Sphere: sample the polar angle through its cosine so the points spread
	// evenly instead of bunching at the poles.
	const f32_t fCosPhi = 1.f - 2.f * Next_Random01();
	const f32_t fSinPhi = sqrtf(max(0.f, 1.f - fCosPhi * fCosPhi));
	return {
		vOffset.x + fRadius * fSinPhi * cosf(fTheta),
		vOffset.y + fRadius * fCosPhi,
		vOffset.z + fRadius * fSinPhi * sinf(fTheta)
	};
}

f32_t Client::CEffect_ParticleSimulator::Evaluate(
	const EFFECT_CURVE_FLOAT_DESC& Curve,
	const f32_t fNormalizedTime) const
{
	if (Curve.Keys.empty())
		return 1.f;
	if (fNormalizedTime <= Curve.Keys.front().fTime)
		return Curve.Keys.front().fValue;
	if (fNormalizedTime >= Curve.Keys.back().fTime)
		return Curve.Keys.back().fValue;

	for (size_t i = 1; i < Curve.Keys.size(); ++i)
	{
		if (fNormalizedTime <= Curve.Keys[i].fTime)
		{
			const EFFECT_CURVE_KEY& Left = Curve.Keys[i - 1];
			const EFFECT_CURVE_KEY& Right = Curve.Keys[i];
			const f32_t fRange = max(
				Right.fTime - Left.fTime, 0.0001f);
			const f32_t fRatio =
				(fNormalizedTime - Left.fTime) / fRange;
			return Left.fValue +
				(Right.fValue - Left.fValue) * fRatio;
		}
	}
	return Curve.Keys.back().fValue;
}

void Client::CEffect_ParticleSimulator::Spawn(const uint32_t iCount)
{
	if (0 == iCount)
		return;

	const EFFECT_MODULE_DESC* pSpawn =
		Find_Module(EFFECT_MODULE_TYPE::SPAWN);
	const EFFECT_MODULE_DESC* pLifetime =
		Find_Module(EFFECT_MODULE_TYPE::LIFETIME);
	if (nullptr == pSpawn || nullptr == pLifetime)
		return;

	const EFFECT_MODULE_DESC* pLocation =
		Find_Module(EFFECT_MODULE_TYPE::INITIAL_LOCATION);
	const EFFECT_MODULE_DESC* pVelocity =
		Find_Module(EFFECT_MODULE_TYPE::INITIAL_VELOCITY);
	const EFFECT_MODULE_DESC* pSize =
		Find_Module(EFFECT_MODULE_TYPE::INITIAL_SIZE);
	const EFFECT_MODULE_DESC* pColor =
		Find_Module(EFFECT_MODULE_TYPE::INITIAL_COLOR);
	const EFFECT_MODULE_DESC* pInitialRotation =
		Find_Module(EFFECT_MODULE_TYPE::INITIAL_ROTATION);
	const EFFECT_MODULE_DESC* pRotationRate =
		Find_Module(EFFECT_MODULE_TYPE::ROTATION_RATE);
	const EFFECT_MODULE_DESC* pSubUV =
		Find_Module(EFFECT_MODULE_TYPE::SUB_UV);
	const EFFECT_MODULE_DESC* pDynamicParameter =
		Find_Module(EFFECT_MODULE_TYPE::DYNAMIC_PARAMETER);

	const uint32_t iAliveCount =
		static_cast<uint32_t>(m_Particles.size());
	const uint32_t iAvailableCount =
		(pSpawn->Spawn.iMaxParticles > iAliveCount)
		? pSpawn->Spawn.iMaxParticles - iAliveCount
		: 0;
	const uint32_t iAcceptedCount = min(iCount, iAvailableCount);
	m_Stats.iDroppedByCapacity += iCount - iAcceptedCount;

	for (uint32_t i = 0; i < iAcceptedCount; ++i)
	{
		EFFECT_PARTICLE Particle;
		Particle.vPosition =
			(nullptr != pLocation)
			? Sample_Location(pLocation->InitialLocation)
			: float3_t{};
		Particle.vVelocity =
			(nullptr != pVelocity) ? Sample(pVelocity->InitialVelocity.Velocity)
			: float3_t{};
		Particle.vInitialVelocity = Particle.vVelocity;
		Particle.vSize =
			(nullptr != pSize) ? Sample(pSize->InitialSize.Size)
			: float3_t{ 1.f, 1.f, 1.f };
		Particle.vColor =
			(nullptr != pColor) ? Sample(pColor->InitialColor.Color)
			: float4_t{ 1.f, 1.f, 1.f, 1.f };
		Particle.vRotation =
			(nullptr != pInitialRotation)
			? Sample(pInitialRotation->InitialRotation.RotationDegrees)
			: float3_t{};
		Particle.vRotationRate =
			(nullptr != pRotationRate)
			? Sample(pRotationRate->RotationRate.RotationRateDegreesPerSecond)
			: float3_t{};
		Particle.vInitialSize = Particle.vSize;
		Particle.vInitialColor = Particle.vColor;
		Particle.fLifetime =
			max(Sample(pLifetime->Lifetime.Lifetime),
				MIN_PARTICLE_LIFETIME);
		Particle.iRandomSeed = m_iRandomState;
		Particle.isAlive = true;
		if (nullptr != pDynamicParameter)
		{
			Particle.vDynamicParameter = {
				Evaluate(pDynamicParameter->DynamicParameter.X, 0.f),
				Evaluate(pDynamicParameter->DynamicParameter.Y, 0.f),
				Evaluate(pDynamicParameter->DynamicParameter.Z, 0.f),
				Evaluate(pDynamicParameter->DynamicParameter.W, 0.f)
			};
		}
		if (nullptr != pSubUV)
		{
			Particle.iSubUVFrame =
				CalculateSubUVFrame(pSubUV->SubUV, Particle);
		}
		m_Particles.push_back(Particle);
	}

	m_Stats.iTotalSpawned += iAcceptedCount;
}

Client::EFFECT_PARTICLE_COUNT_VALIDATION_RESULT
Client::Run_Phase2ParticleCountValidation()
{
	EFFECT_EMITTER_DESC Emitter;
	Emitter.iEmitterId = 1;
	Emitter.strName = "Phase2_CountValidation";
	Emitter.fDuration = 2.f;
	Emitter.iLoopCount = 1;

	EFFECT_MODULE_DESC Spawn;
	Spawn.iModuleId = 1;
	Spawn.strName = "Spawn";
	Spawn.eType = EFFECT_MODULE_TYPE::SPAWN;
	Spawn.Spawn.fRatePerSecond = 4.f;
	Spawn.Spawn.iMaxParticles = 100;
	Emitter.Modules.push_back(Spawn);

	EFFECT_MODULE_DESC Lifetime;
	Lifetime.iModuleId = 2;
	Lifetime.strName = "Lifetime";
	Lifetime.eType = EFFECT_MODULE_TYPE::LIFETIME;
	Lifetime.Lifetime.Lifetime.fConstant = 1.f;
	Emitter.Modules.push_back(Lifetime);

	CEffect_ParticleSimulator Simulator;
	EFFECT_PARTICLE_COUNT_VALIDATION_RESULT Result;
	if (false == Simulator.Initialize(Emitter))
		return Result;

	Simulator.Update(0.5f);
	Result.iAliveAfterHalfSecond = Simulator.Get_Stats().iAliveCount;
	Simulator.Update(0.5f);
	Result.iAliveAfterOneSecond = Simulator.Get_Stats().iAliveCount;
	Simulator.Update(0.5f);
	Result.iAliveAfterOneAndHalfSeconds =
		Simulator.Get_Stats().iAliveCount;
	Result.iTotalSpawned = Simulator.Get_Stats().iTotalSpawned;
	Result.iTotalKilled = Simulator.Get_Stats().iTotalKilled;

	Result.isPassed =
		2 == Result.iAliveAfterHalfSecond &&
		4 == Result.iAliveAfterOneSecond &&
		4 == Result.iAliveAfterOneAndHalfSeconds &&
		6 == Result.iTotalSpawned &&
		2 == Result.iTotalKilled;
	return Result;
}
