#include "Effect_Playback.h"

#include "Effect_DocumentCodec.h"

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace
{
	constexpr f32_t FIXED_STEP_SECONDS = 1.f / 60.f;
	constexpr f64_t FIXED_STEP_SECONDS_EXACT = 1.0 / 60.0;
	constexpr f64_t FIXED_STEP_EPSILON = 1.0e-9;
	constexpr uint32_t MAX_CATCH_UP_STEPS = 8u;

	uint32_t Hash_StableId(const std::string& Value)
	{
		uint32_t Hash = 2166136261u;
		for (const unsigned char Character : Value)
		{
			Hash ^= Character;
			Hash *= 16777619u;
		}
		return 0u == Hash ? 1u : Hash;
	}

	f32_t Clamp01(const f32_t Value)
	{
		return std::clamp(Value, 0.f, 1.f);
	}

	float3_t Lerp3(const float3_t& A, const float3_t& B, const f32_t T)
	{
		return float3_t(
			A.x + (B.x - A.x) * T,
			A.y + (B.y - A.y) * T,
			A.z + (B.z - A.z) * T);
	}

	float4_t Lerp4(const float4_t& A, const float4_t& B, const f32_t T)
	{
		return float4_t(
			A.x + (B.x - A.x) * T,
			A.y + (B.y - A.y) * T,
			A.z + (B.z - A.z) * T,
			A.w + (B.w - A.w) * T);
	}

	float3_t Add3(const float3_t& A, const float3_t& B)
	{
		return float3_t(A.x + B.x, A.y + B.y, A.z + B.z);
	}

	float3_t Scale3(const float3_t& Value, const f32_t Scale)
	{
		return float3_t(Value.x * Scale, Value.y * Scale, Value.z * Scale);
	}

	f32_t DistanceSquared(const float3_t& A, const float3_t& B)
	{
		const f32_t X = A.x - B.x;
		const f32_t Y = A.y - B.y;
		const f32_t Z = A.z - B.z;
		return X * X + Y * Y + Z * Z;
	}

	float3_t Get_Translation(const float4x4_t& Matrix)
	{
		return float3_t(Matrix._41, Matrix._42, Matrix._43);
	}
}

bool_t Client::CEffectPlayback::Stage_Document(
	const EFFECT_DOCUMENT_DESC& Document,
	std::string& strOutError)
{
	if (!CEffectDocumentCodec::Validate(Document, strOutError))
		return false;

	EFFECT_DOCUMENT_DESC StagedDocument = Document;
	std::unordered_map<std::string, ELEMENT_STATE> StagedStates;
	f32_t fStagedDuration = 0.f;
	for (const EFFECT_ELEMENT_DESC& Element : StagedDocument.Elements)
	{
		ELEMENT_STATE State;
		State.iRandomState = Hash_StableId(Element.strElementId) ^
			Element.Detail.Particle.iRandomSeed;
		if (0u == State.iRandomState)
			State.iRandomState = 1u;
		StagedStates.emplace(Element.strElementId, std::move(State));
		f32_t fElementTail = 0.f;
		if (EFFECT_ELEMENT_KIND::PARTICLE == Element.eKind)
			fElementTail = Element.Detail.Particle.vLifeTimeSeconds.y;
		else if (EFFECT_ELEMENT_KIND::TRAIL == Element.eKind)
			fElementTail = Element.Detail.Trail.fPointLifeTimeSeconds;
		fStagedDuration = (std::max)(fStagedDuration,
			Element.Detail.Timing.fStartDelaySeconds +
			Element.Detail.Timing.fLifeTimeSeconds +
			Element.Detail.Timing.fAfterImageSeconds +
			fElementTail);
	}

	m_Document = std::move(StagedDocument);
	m_States = std::move(StagedStates);
	m_fDurationSeconds = fStagedDuration;
	Reset();
	strOutError.clear();
	return true;
}

void Client::CEffectPlayback::Reset()
{
	m_fSampleTimeSeconds = 0.f;
	m_fAccumulatorSeconds = 0.0;
	m_iSimulationStep = 0u;
	m_Frame = {};
	for (const EFFECT_ELEMENT_DESC& Element : m_Document.Elements)
	{
		ELEMENT_STATE& State = m_States[Element.strElementId];
		State = {};
		State.iRandomState = Hash_StableId(Element.strElementId) ^
			Element.Detail.Particle.iRandomSeed;
		if (0u == State.iRandomState)
			State.iRandomState = 1u;
	}
	float4x4_t Identity{};
	XMStoreFloat4x4(&Identity, XMMatrixIdentity());
	Rebuild_Frame(Identity);
}

void Client::CEffectPlayback::Update(
	const f32_t fTimeDelta,
	const float4x4_t& RootWorld)
{
	if (!std::isfinite(fTimeDelta) || fTimeDelta <= 0.f)
	{
		Rebuild_Frame(RootWorld);
		return;
	}

	m_fAccumulatorSeconds += static_cast<f64_t>(fTimeDelta);
	uint32_t iSteps = 0u;
	while (m_fAccumulatorSeconds + FIXED_STEP_EPSILON >=
		FIXED_STEP_SECONDS_EXACT &&
		iSteps < MAX_CATCH_UP_STEPS)
	{
		Step(FIXED_STEP_SECONDS, RootWorld);
		m_fAccumulatorSeconds = (std::max)(0.0,
			m_fAccumulatorSeconds - FIXED_STEP_SECONDS_EXACT);
		++iSteps;
	}
	Rebuild_Frame(RootWorld);
}

void Client::CEffectPlayback::Seek(
	const f32_t fSampleTimeSeconds,
	const float4x4_t& RootWorld)
{
	const f32_t fTarget = std::clamp(
		std::isfinite(fSampleTimeSeconds) ? fSampleTimeSeconds : 0.f,
		0.f,
		m_fDurationSeconds);
	Reset();
	const uint64_t iTargetSteps = static_cast<uint64_t>(std::floor(
		static_cast<f64_t>(fTarget) / FIXED_STEP_SECONDS_EXACT +
		FIXED_STEP_EPSILON));
	for (uint64_t iStep = 0u; iStep < iTargetSteps; ++iStep)
		Step(FIXED_STEP_SECONDS, RootWorld);
	m_fAccumulatorSeconds = (std::max)(0.0,
		static_cast<f64_t>(fTarget) -
		static_cast<f64_t>(iTargetSteps) * FIXED_STEP_SECONDS_EXACT);
	m_fSampleTimeSeconds = fTarget;
	Rebuild_Frame(RootWorld);
}

void Client::CEffectPlayback::Step(
	const f32_t fFixedDelta,
	const float4x4_t& RootWorld)
{
	++m_iSimulationStep;
	m_fSampleTimeSeconds = static_cast<f32_t>(
		static_cast<f64_t>(m_iSimulationStep) * FIXED_STEP_SECONDS_EXACT);

	for (const EFFECT_ELEMENT_DESC& Element : m_Document.Elements)
	{
		ELEMENT_STATE& State = m_States[Element.strElementId];
		const f32_t fLocalTime = m_fSampleTimeSeconds -
			Element.Detail.Timing.fStartDelaySeconds;
		if (fLocalTime < 0.f)
			continue;

		if (EFFECT_ELEMENT_KIND::PARTICLE == Element.eKind &&
			fLocalTime <= Element.Detail.Timing.fLifeTimeSeconds)
		{
			if (!State.bBurstSpawned)
			{
				Spawn_Particles(Element, State,
					Element.Detail.Particle.iBurstCount, RootWorld);
				State.bBurstSpawned = true;
			}
			State.fSpawnAccumulator +=
				Element.Detail.Particle.fSpawnRatePerSecond * fFixedDelta;
			const uint32_t iSpawnCount = static_cast<uint32_t>(
				std::floor(State.fSpawnAccumulator));
			State.fSpawnAccumulator -= static_cast<f32_t>(iSpawnCount);
			Spawn_Particles(Element, State, iSpawnCount, RootWorld);
		}
		Update_Particles(Element, State, fFixedDelta);

		if (EFFECT_ELEMENT_KIND::TRAIL == Element.eKind)
			Sample_Trail(Element, State, fFixedDelta, RootWorld);
		if ((EFFECT_ELEMENT_KIND::MESH == Element.eKind ||
			EFFECT_ELEMENT_KIND::SPRITE == Element.eKind) &&
			Element.Detail.Timing.fAfterImageSeconds > 0.f &&
			Element.Detail.AfterImage.iMaxCopies > 0u)
		{
			Sample_AfterImages(Element, State, fFixedDelta, RootWorld);
		}
	}
}

void Client::CEffectPlayback::Spawn_Particles(
	const EFFECT_ELEMENT_DESC& Element,
	ELEMENT_STATE& State,
	const uint32_t iCount,
	const float4x4_t& RootWorld)
{
	const EFFECT_PARTICLE_DESC& Desc = Element.Detail.Particle;
	const uint32_t iAvailable = Desc.iMaxParticles > State.Particles.size() ?
		Desc.iMaxParticles - static_cast<uint32_t>(State.Particles.size()) : 0u;
	const uint32_t iSpawnCount = (std::min)(iCount, iAvailable);
	for (uint32_t iParticle = 0u; iParticle < iSpawnCount; ++iParticle)
	{
		PARTICLE_STATE Particle;
		Particle.vVelocity = float3_t(
			Random_Range(State, Desc.vInitialVelocityMin.x, Desc.vInitialVelocityMax.x),
			Random_Range(State, Desc.vInitialVelocityMin.y, Desc.vInitialVelocityMax.y),
			Random_Range(State, Desc.vInitialVelocityMin.z, Desc.vInitialVelocityMax.z));
		Particle.fLifeTimeSeconds = Random_Range(
			State, Desc.vLifeTimeSeconds.x, Desc.vLifeTimeSeconds.y);
		Particle.SpawnRootWorld = Evaluate_ElementWorld(
			Element, m_fSampleTimeSeconds, RootWorld);
		State.Particles.push_back(Particle);
	}
}

void Client::CEffectPlayback::Update_Particles(
	const EFFECT_ELEMENT_DESC& Element,
	ELEMENT_STATE& State,
	const f32_t fFixedDelta)
{
	for (PARTICLE_STATE& Particle : State.Particles)
	{
		Particle.vVelocity = Add3(Particle.vVelocity,
			Scale3(Element.Detail.Particle.vAcceleration, fFixedDelta));
		Particle.vPosition = Add3(Particle.vPosition,
			Scale3(Particle.vVelocity, fFixedDelta));
		Particle.fAgeSeconds += fFixedDelta;
	}
	std::erase_if(State.Particles,
		[](const PARTICLE_STATE& Particle)
		{
			return Particle.fAgeSeconds >= Particle.fLifeTimeSeconds;
		});
}

void Client::CEffectPlayback::Sample_Trail(
	const EFFECT_ELEMENT_DESC& Element,
	ELEMENT_STATE& State,
	const f32_t fFixedDelta,
	const float4x4_t& RootWorld)
{
	for (EFFECT_EVALUATED_TRAIL_POINT& Point : State.TrailPoints)
	{
		Point.fNormalizedAge += fFixedDelta /
			Element.Detail.Trail.fPointLifeTimeSeconds;
	}
	std::erase_if(State.TrailPoints,
		[](const EFFECT_EVALUATED_TRAIL_POINT& Point)
		{
			return Point.fNormalizedAge >= 1.f;
		});

	const f32_t fLocalTime = m_fSampleTimeSeconds -
		Element.Detail.Timing.fStartDelaySeconds;
	if (fLocalTime < 0.f ||
		fLocalTime > Element.Detail.Timing.fLifeTimeSeconds)
	{
		return;
	}

	State.fTrailSampleAccumulator += fFixedDelta;
	if (State.fTrailSampleAccumulator <
		Element.Detail.Trail.fSampleIntervalSeconds)
	{
		return;
	}
	State.fTrailSampleAccumulator = 0.f;
	const float4x4_t World = Evaluate_ElementWorld(
		Element, m_fSampleTimeSeconds, RootWorld);
	const float3_t Position = Get_Translation(World);
	if (!State.TrailPoints.empty() &&
		DistanceSquared(Position, State.TrailPoints.back().vWorldPosition) <
		Element.Detail.Trail.fMinimumDistance *
		Element.Detail.Trail.fMinimumDistance)
	{
		return;
	}
	State.TrailPoints.push_back({ Position, 0.f });
	while (State.TrailPoints.size() > Element.Detail.Trail.iMaxPoints)
		State.TrailPoints.erase(State.TrailPoints.begin());
}

void Client::CEffectPlayback::Sample_AfterImages(
	const EFFECT_ELEMENT_DESC& Element,
	ELEMENT_STATE& State,
	const f32_t fFixedDelta,
	const float4x4_t& RootWorld)
{
	for (AFTERIMAGE_STATE& AfterImage : State.AfterImages)
		AfterImage.fAgeSeconds += fFixedDelta;
	std::erase_if(State.AfterImages,
		[&Element](const AFTERIMAGE_STATE& AfterImage)
		{
			return AfterImage.fAgeSeconds >=
				Element.Detail.Timing.fAfterImageSeconds;
		});

	const f32_t fLocalTime = m_fSampleTimeSeconds -
		Element.Detail.Timing.fStartDelaySeconds;
	if (fLocalTime < 0.f ||
		fLocalTime > Element.Detail.Timing.fLifeTimeSeconds)
	{
		return;
	}
	State.fAfterImageAccumulator += fFixedDelta;
	if (State.fAfterImageAccumulator <
		Element.Detail.AfterImage.fSampleIntervalSeconds)
	{
		return;
	}
	State.fAfterImageAccumulator = 0.f;
	State.AfterImages.push_back({
		Evaluate_ElementWorld(Element, m_fSampleTimeSeconds, RootWorld), 0.f });
	while (State.AfterImages.size() > Element.Detail.AfterImage.iMaxCopies)
		State.AfterImages.erase(State.AfterImages.begin());
}

float4x4_t Client::CEffectPlayback::Evaluate_ElementWorld(
	const EFFECT_ELEMENT_DESC& Element,
	const f32_t fSampleTimeSeconds,
	const float4x4_t& RootWorld) const
{
	const EFFECT_DETAIL_DESC& Detail = Element.Detail;
	const f32_t fLocalTime = (std::max)(0.f,
		fSampleTimeSeconds - Detail.Timing.fStartDelaySeconds);
	const f32_t T = Clamp01(fLocalTime / Detail.Timing.fLifeTimeSeconds);
	const EFFECT_LINEAR_LERP_DESC& Lerp = Detail.LinearLerp;

	float3_t Position = Lerp.bPosition ?
		Lerp3(Detail.Transform.vPosition, Lerp.vEndPosition, T) :
		Detail.Transform.vPosition;
	const float3_t StartVelocity = Detail.Transform.vVelocityPerSecond;
	const float3_t EndVelocity = Lerp.bVelocity ?
		Lerp.vEndVelocityPerSecond : StartVelocity;
	Position = Add3(Position, Add3(
		Scale3(StartVelocity, fLocalTime),
		Scale3(Add3(EndVelocity, Scale3(StartVelocity, -1.f)),
			0.5f * fLocalTime * T)));

	const float3_t Rotation = Lerp.bRotation ?
		Lerp3(Detail.Transform.vRotationDegrees,
			Lerp.vEndRotationDegrees, T) :
		Detail.Transform.vRotationDegrees;
	const float3_t Revolution = Lerp.bRevolution ?
		Lerp3(Detail.Transform.vRevolutionDegreesPerSecond,
			Lerp.vEndRevolutionDegreesPerSecond, T) :
		Detail.Transform.vRevolutionDegreesPerSecond;
	const float3_t Scale = Lerp.bScale ?
		Lerp3(Detail.Transform.vScale, Lerp.vEndScale, T) :
		Detail.Transform.vScale;

	const matrix_t Local =
		XMMatrixScaling(Scale.x, Scale.y, Scale.z) *
		XMMatrixRotationRollPitchYaw(
			XMConvertToRadians(Rotation.x + Revolution.x * fLocalTime),
			XMConvertToRadians(Rotation.y + Revolution.y * fLocalTime),
			XMConvertToRadians(Rotation.z + Revolution.z * fLocalTime)) *
		XMMatrixTranslation(Position.x, Position.y, Position.z);
	float4x4_t Result{};
	XMStoreFloat4x4(&Result, Local * XMLoadFloat4x4(&RootWorld));
	return Result;
}

Client::EFFECT_COLOR_DESC Client::CEffectPlayback::Evaluate_Color(
	const EFFECT_ELEMENT_DESC& Element,
	const f32_t fNormalizedLife) const
{
	EFFECT_COLOR_DESC Color = Element.Detail.Color;
	const EFFECT_LINEAR_LERP_DESC& Lerp = Element.Detail.LinearLerp;
	if (Lerp.bColorOffset)
		Color.vColorOffset = Lerp4(Color.vColorOffset, Lerp.vEndColorOffset,
			fNormalizedLife);
	if (Lerp.bColorMultiply)
		Color.vColorMultiply = Lerp4(Color.vColorMultiply,
			Lerp.vEndColorMultiply, fNormalizedLife);
	if (Lerp.bEmissiveIntensity)
	{
		Color.fEmissiveIntensity +=
			(Lerp.fEndEmissiveIntensity - Color.fEmissiveIntensity) *
			fNormalizedLife;
	}
	return Color;
}

void Client::CEffectPlayback::Rebuild_Frame(const float4x4_t& RootWorld)
{
	m_Frame = {};
	m_Frame.fSampleTimeSeconds = m_fSampleTimeSeconds;
	for (const EFFECT_ELEMENT_DESC& Element : m_Document.Elements)
	{
		const f32_t fLocalTime = m_fSampleTimeSeconds -
			Element.Detail.Timing.fStartDelaySeconds;
		const f32_t T = Clamp01(fLocalTime /
			Element.Detail.Timing.fLifeTimeSeconds);
		ELEMENT_STATE& State = m_States[Element.strElementId];

		if (fLocalTime >= 0.f &&
			fLocalTime <= Element.Detail.Timing.fLifeTimeSeconds &&
			EFFECT_ELEMENT_KIND::PARTICLE != Element.eKind &&
			EFFECT_ELEMENT_KIND::TRAIL != Element.eKind)
		{
			m_Frame.Elements.push_back({
				&Element,
				Evaluate_ElementWorld(Element, m_fSampleTimeSeconds, RootWorld),
				Evaluate_Color(Element, T),
				fLocalTime,
				T });
		}

		for (const PARTICLE_STATE& Particle : State.Particles)
		{
			const f32_t ParticleT = Clamp01(
				Particle.fAgeSeconds / Particle.fLifeTimeSeconds);
			const float2_t Size(
				Element.Detail.Particle.vStartSize.x +
				(Element.Detail.Particle.vEndSize.x -
					Element.Detail.Particle.vStartSize.x) * ParticleT,
				Element.Detail.Particle.vStartSize.y +
				(Element.Detail.Particle.vEndSize.y -
					Element.Detail.Particle.vStartSize.y) * ParticleT);
			const float4x4_t CurrentElementWorld = Evaluate_ElementWorld(
				Element, m_fSampleTimeSeconds, RootWorld);
			const float4x4_t& ParticleRoot =
				Element.Detail.Particle.bLocalSpace ?
				CurrentElementWorld : Particle.SpawnRootWorld;
			const matrix_t World = XMMatrixScaling(Size.x, Size.y, 1.f) *
				XMMatrixTranslation(
					Particle.vPosition.x,
					Particle.vPosition.y,
					Particle.vPosition.z) *
				XMLoadFloat4x4(&ParticleRoot);
			EFFECT_EVALUATED_PARTICLE Evaluated;
			Evaluated.pElement = &Element;
			XMStoreFloat4x4(&Evaluated.World, World);
			Evaluated.Color = Evaluate_Color(Element, ParticleT).vColorMultiply;
			Evaluated.Color.w *= 1.f - ParticleT;
			m_Frame.Particles.push_back(Evaluated);
		}

		if (!State.TrailPoints.empty())
			m_Frame.Trails.push_back({ &Element, State.TrailPoints });

		for (const AFTERIMAGE_STATE& AfterImage : State.AfterImages)
		{
			const f32_t Age = Clamp01(AfterImage.fAgeSeconds /
				Element.Detail.Timing.fAfterImageSeconds);
			m_Frame.AfterImages.push_back({
				&Element,
				AfterImage.World,
				std::pow(1.f - Age,
					Element.Detail.AfterImage.fAlphaExponent) });
		}
	}
}

uint32_t Client::CEffectPlayback::Next_Random(ELEMENT_STATE& State) const
{
	uint32_t Value = State.iRandomState;
	Value ^= Value << 13u;
	Value ^= Value >> 17u;
	Value ^= Value << 5u;
	State.iRandomState = 0u == Value ? 1u : Value;
	return State.iRandomState;
}

f32_t Client::CEffectPlayback::Random_Range(
	ELEMENT_STATE& State,
	const f32_t fMin,
	const f32_t fMax) const
{
	const f32_t T = static_cast<f32_t>(Next_Random(State)) /
		static_cast<f32_t>(UINT32_MAX);
	return fMin + (fMax - fMin) * T;
}

bool_t Client::CEffectPlayback::Is_Finished() const
{
	if (m_fSampleTimeSeconds < m_fDurationSeconds)
		return false;
	for (const auto& Pair : m_States)
	{
		if (!Pair.second.Particles.empty() ||
			!Pair.second.TrailPoints.empty() ||
			!Pair.second.AfterImages.empty())
		{
			return false;
		}
	}
	return true;
}
