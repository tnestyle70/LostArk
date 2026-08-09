#include "Shadow.h"

#include "Shader.h"

#include <cmath>

namespace
{
	bool_t IsFinite4(const float4_t& Value)
	{
		return std::isfinite(Value.x) && std::isfinite(Value.y) &&
			std::isfinite(Value.z) && std::isfinite(Value.w);
	}

	bool_t IsFiniteShadowPosition(const float4_t& Value)
	{
		constexpr f32_t MAX_ABSOLUTE_POSITION = 1000000.f;
		return IsFinite4(Value) &&
			std::abs(Value.x) <= MAX_ABSOLUTE_POSITION &&
			std::abs(Value.y) <= MAX_ABSOLUTE_POSITION &&
			std::abs(Value.z) <= MAX_ABSOLUTE_POSITION;
	}

	bool_t IsValidEnabledDesc(const SHADOW_LIGHT_DESC& Desc)
	{
		const SHADOW_SETTINGS& Settings = Desc.Settings;
		if (!IsFiniteShadowPosition(Desc.vEye) ||
			!IsFiniteShadowPosition(Desc.vAt) ||
			!std::isfinite(Settings.fOrthographicWidth) ||
			!std::isfinite(Settings.fOrthographicHeight) ||
			!std::isfinite(Settings.fNear) ||
			!std::isfinite(Settings.fFar) ||
			!std::isfinite(Settings.fDepthBias) ||
			!std::isfinite(Settings.fNormalBias) ||
			!std::isfinite(Settings.fStrength))
		{
			return false;
		}

		const float3_t Direction(
			Desc.vAt.x - Desc.vEye.x,
			Desc.vAt.y - Desc.vEye.y,
			Desc.vAt.z - Desc.vEye.z);
		const f32_t DirectionLengthSquared =
			Direction.x * Direction.x +
			Direction.y * Direction.y +
			Direction.z * Direction.z;

		return DirectionLengthSquared > 0.000001f &&
			Settings.fOrthographicWidth >= 0.1f &&
			Settings.fOrthographicWidth <= 10000.f &&
			Settings.fOrthographicHeight >= 0.1f &&
			Settings.fOrthographicHeight <= 10000.f &&
			Settings.fNear > 0.f &&
			Settings.fFar > Settings.fNear &&
			Settings.fFar <= 100000.f &&
			Settings.fDepthBias >= 0.f &&
			Settings.fDepthBias <= 0.05f &&
			Settings.fNormalBias >= 0.f &&
			Settings.fNormalBias <= 10.f &&
			Settings.fStrength >= 0.f &&
			Settings.fStrength <= 1.f;
	}
}

CShadow::CShadow()
{
	XMStoreFloat4x4(
		&m_TransformMatrices[ETOUI(D3DTS::VIEW)],
		XMMatrixIdentity());
	XMStoreFloat4x4(
		&m_TransformMatrices[ETOUI(D3DTS::PROJ)],
		XMMatrixIdentity());
}

CShadow::~CShadow()
{
}

HRESULT CShadow::Apply_Shadow_Light(const SHADOW_LIGHT_DESC& ShadowLightDesc)
{
	if (!ShadowLightDesc.Settings.bEnabled)
	{
		SHADOW_LIGHT_DESC Disabled{};
		Disabled.Settings.bEnabled = false;
		m_ShadowLightDesc = Disabled;
		XMStoreFloat4x4(
			&m_TransformMatrices[ETOUI(D3DTS::VIEW)],
			XMMatrixIdentity());
		XMStoreFloat4x4(
			&m_TransformMatrices[ETOUI(D3DTS::PROJ)],
			XMMatrixIdentity());
		return S_OK;
	}

	if (!IsValidEnabledDesc(ShadowLightDesc))
		return E_INVALIDARG;

	const vector_t Eye = XMLoadFloat4(&ShadowLightDesc.vEye);
	const vector_t At = XMLoadFloat4(&ShadowLightDesc.vAt);
	const vector_t Forward = XMVector3Normalize(At - Eye);
	const f32_t VerticalAlignment = std::abs(
		XMVectorGetX(XMVector3Dot(
			Forward, XMVectorSet(0.f, 1.f, 0.f, 0.f))));
	const vector_t Up = VerticalAlignment > 0.99f ?
		XMVectorSet(0.f, 0.f, 1.f, 0.f) :
		XMVectorSet(0.f, 1.f, 0.f, 0.f);

	float4x4_t StagedView{};
	float4x4_t StagedProjection{};
	XMStoreFloat4x4(
		&StagedView, XMMatrixLookAtLH(Eye, At, Up));
	XMStoreFloat4x4(
		&StagedProjection,
		XMMatrixOrthographicLH(
			ShadowLightDesc.Settings.fOrthographicWidth,
			ShadowLightDesc.Settings.fOrthographicHeight,
			ShadowLightDesc.Settings.fNear,
			ShadowLightDesc.Settings.fFar));

	m_ShadowLightDesc = ShadowLightDesc;
	m_TransformMatrices[ETOUI(D3DTS::VIEW)] = StagedView;
	m_TransformMatrices[ETOUI(D3DTS::PROJ)] = StagedProjection;

	return S_OK;
}

HRESULT CShadow::Bind_ShaderResource(shared_ptr<class CShader> pShader, const char_t* pConstantName, D3DTS eType)
{
	if (nullptr == pShader || nullptr == pConstantName || eType >= D3DTS::END)
		return E_INVALIDARG;
	return pShader->Bind_Matrix(pConstantName, &m_TransformMatrices[ETOUI(eType)]);
	
}

HRESULT CShadow::Bind_LightingShaderResources(shared_ptr<class CShader> pShader)
{
	if (nullptr == pShader)
		return E_INVALIDARG;

	const SHADOW_SETTINGS& Settings = m_ShadowLightDesc.Settings;
	if (FAILED(Bind_ShaderResource(
		pShader, "g_LightViewMatrix", D3DTS::VIEW)) ||
		FAILED(Bind_ShaderResource(
			pShader, "g_LightProjMatrix", D3DTS::PROJ)) ||
		FAILED(pShader->Bind_RawValue(
			"g_fShadowDepthBias", &Settings.fDepthBias,
			sizeof(Settings.fDepthBias))) ||
		FAILED(pShader->Bind_RawValue(
			"g_fShadowNormalBias", &Settings.fNormalBias,
			sizeof(Settings.fNormalBias))) ||
		FAILED(pShader->Bind_RawValue(
			"g_fShadowStrength", &Settings.fStrength,
			sizeof(Settings.fStrength))))
	{
		return E_FAIL;
	}

	return S_OK;
}

unique_ptr<CShadow> CShadow::Create()
{
	return unique_ptr<CShadow>(new CShadow());
}
