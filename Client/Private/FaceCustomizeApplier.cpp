#include "FaceCustomizeApplier.h"

#include "Model.h"

#include <algorithm>
#include <cmath>

namespace
{
	constexpr f32_t NEUTRAL_WEIGHT = 0.5f;
	constexpr f32_t WEIGHT_EPSILON = 1e-4f;
	constexpr f32_t MATRIX_EPSILON = 1e-5f;

	bool_t Is_Same_Matrix(const float4x4_t& A, const float4x4_t& B)
	{
		const f32_t* pA = &A._11;
		const f32_t* pB = &B._11;
		for (size_t i = 0; i < 16; ++i)
		{
			if (std::fabs(pA[i] - pB[i]) > MATRIX_EPSILON)
				return false;
		}
		return true;
	}

	bool_t Is_Finite(fmatrix_t M)
	{
		float4x4_t Stored;
		XMStoreFloat4x4(&Stored, M);
		const f32_t* pValues = &Stored._11;
		for (size_t i = 0; i < 16; ++i)
		{
			if (!std::isfinite(pValues[i]))
				return false;
		}
		return true;
	}
}

bool_t Client::CFaceCustomizeApplier::Initialize(
	const shared_ptr<Engine::CModel>& pModel,
	const FACE_SLIDER_DOCUMENT& Document)
{
	m_Sliders.clear();
	m_BoneStates.clear();
	m_AccumulatedDeltas.clear();
	m_AccumulatedTouched.clear();
	if (nullptr == pModel)
		return false;

	for (const FACE_SLIDER& Source : Document.Sliders)
	{
		SLIDER slider;
		slider.strId = Source.strId;
		for (const FACE_SLIDER_BONE& Bone : Source.Bones)
		{
			const int32_t iBoneIndex = pModel->Find_BoneIndex(Bone.strBoneName.c_str());
			if (iBoneIndex < 0)
			{
				OutputDebugStringA(("[FaceCustomize] slider " + Source.strId +
					" skips missing bone " + Bone.strBoneName + "\n").c_str());
				continue;
			}
			SLIDER_BONE target;
			target.iBoneState = Find_Or_Add_BoneState(static_cast<uint32_t>(iBoneIndex));
			target.Keys = Bone.Keys;
			slider.Bones.push_back(std::move(target));
		}
		if (slider.Bones.empty())
		{
			OutputDebugStringA(("[FaceCustomize] slider " + Source.strId +
				" dropped: none of its bones exist on the model\n").c_str());
			continue;
		}
		m_Sliders.push_back(std::move(slider));
	}

	m_AccumulatedDeltas.resize(m_BoneStates.size());
	m_AccumulatedTouched.resize(m_BoneStates.size(), false);
	return !m_Sliders.empty();
}

size_t Client::CFaceCustomizeApplier::Find_Or_Add_BoneState(const uint32_t iBoneIndex)
{
	for (size_t i = 0; i < m_BoneStates.size(); ++i)
	{
		if (m_BoneStates[i].iBoneIndex == iBoneIndex)
			return i;
	}
	BONE_STATE state;
	state.iBoneIndex = iBoneIndex;
	m_BoneStates.push_back(state);
	return m_BoneStates.size() - 1;
}

const std::string& Client::CFaceCustomizeApplier::Get_SliderId(const size_t iSlider) const
{
	static const std::string Empty;
	return iSlider < m_Sliders.size() ? m_Sliders[iSlider].strId : Empty;
}

f32_t Client::CFaceCustomizeApplier::Get_Weight(const size_t iSlider) const
{
	return iSlider < m_Sliders.size() ? m_Sliders[iSlider].fWeight : NEUTRAL_WEIGHT;
}

bool_t Client::CFaceCustomizeApplier::Set_Weight(const size_t iSlider, const f32_t fWeight)
{
	if (iSlider >= m_Sliders.size() || !std::isfinite(fWeight))
		return false;
	m_Sliders[iSlider].fWeight = std::clamp(fWeight, 0.f, 1.f);
	return true;
}

void Client::CFaceCustomizeApplier::Reset_Weights()
{
	for (SLIDER& slider : m_Sliders)
		slider.fWeight = NEUTRAL_WEIGHT;
}

matrix_t Client::CFaceCustomizeApplier::Evaluate_Delta(
	const std::array<FACE_SLIDER_KEY, FACE_SLIDER_KEY_COUNT>& Keys, const f32_t fWeight)
{
	/* Weight 0..1 maps onto keys 0..2 with key 1 (neutral) at 0.5. */
	const f32_t fTrack = std::clamp(fWeight, 0.f, 1.f) *
		static_cast<f32_t>(FACE_SLIDER_KEY_COUNT - 1);
	const size_t iFrom = (fTrack < 1.f) ? 0u : 1u;
	const size_t iTo = iFrom + 1u;
	const f32_t fBlend = fTrack - static_cast<f32_t>(iFrom);

	const vector_t vPositionFrom = XMLoadFloat3(&Keys[iFrom].vPosition);
	const vector_t vPositionTo = XMLoadFloat3(&Keys[iTo].vPosition);
	const vector_t vRotationFrom = XMQuaternionNormalize(XMLoadFloat4(&Keys[iFrom].vRotation));
	const vector_t vRotationTo = XMQuaternionNormalize(XMLoadFloat4(&Keys[iTo].vRotation));

	const vector_t vPosition = XMVectorLerp(vPositionFrom, vPositionTo, fBlend);
	const vector_t vRotation = XMQuaternionSlerp(vRotationFrom, vRotationTo, fBlend);
	return XMMatrixRotationQuaternion(vRotation) *
		XMMatrixTranslationFromVector(vPosition);
}

void Client::CFaceCustomizeApplier::Apply(const shared_ptr<Engine::CModel>& pModel)
{
	if (nullptr == pModel || m_Sliders.empty() || m_BoneStates.empty())
		return;

	std::fill(m_AccumulatedTouched.begin(), m_AccumulatedTouched.end(), false);
	for (const SLIDER& slider : m_Sliders)
	{
		if (std::fabs(slider.fWeight - NEUTRAL_WEIGHT) <= WEIGHT_EPSILON)
			continue;
		for (const SLIDER_BONE& target : slider.Bones)
		{
			const matrix_t Delta = Evaluate_Delta(target.Keys, slider.fWeight);
			float4x4_t& Accumulated = m_AccumulatedDeltas[target.iBoneState];
			if (m_AccumulatedTouched[target.iBoneState])
				XMStoreFloat4x4(&Accumulated, Delta * XMLoadFloat4x4(&Accumulated));
			else
				XMStoreFloat4x4(&Accumulated, Delta);
			m_AccumulatedTouched[target.iBoneState] = true;
		}
	}

	bool_t isWritten = false;
	for (size_t i = 0; i < m_BoneStates.size(); ++i)
	{
		BONE_STATE& state = m_BoneStates[i];
		matrix_t Current;
		if (!pModel->Get_BoneLocalMatrix(state.iBoneIndex, Current))
			continue;
		float4x4_t CurrentStored;
		XMStoreFloat4x4(&CurrentStored, Current);

		/* The animation rewrote this bone since our last write (or this is the
		first frame): what it produced is the new base. Otherwise the current value
		is our own previous composite and the stored base still applies. */
		if (!state.hasLastWritten || !Is_Same_Matrix(CurrentStored, state.LastWritten))
			state.Base = CurrentStored;

		matrix_t Result = XMLoadFloat4x4(&state.Base);
		if (m_AccumulatedTouched[i])
			Result = XMLoadFloat4x4(&m_AccumulatedDeltas[i]) * Result;
		if (!Is_Finite(Result))
			continue;
		XMStoreFloat4x4(&state.LastWritten, Result);
		state.hasLastWritten = true;
		if (!Is_Same_Matrix(state.LastWritten, CurrentStored))
		{
			pModel->Set_BoneLocalMatrix(state.iBoneIndex, Result);
			isWritten = true;
		}
	}
	if (isWritten)
		pModel->Refresh_BoneCombinedMatrices();
}
