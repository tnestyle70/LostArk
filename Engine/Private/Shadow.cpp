// 그림자 광원의 설정 검증과 VIEW/PROJ 생성, 조명 shader 전달을 구현한다.
#include "Shadow.h"
// 비활성 초기값과 유효성 검사에 사용하는 그림자 설정 구조체다.
#include "Engine_RenderTypes.h"

// 검증한 행렬과 scalar 설정을 실제 effect 상수에 바인딩한다.
#include "Shader.h"

// 위치/설정의 유한성, 좌표 크기와 수직 방향 정렬을 검사한다.
#include <cmath>

namespace
{
	// 위치의 w를 포함한 네 성분을 검사한다. w가 반드시 1이라는 조건은 여기서 요구하지 않는다.
	bool_t IsFinite4(const float4_t& Value)
	{
		return std::isfinite(Value.x) && std::isfinite(Value.y) &&
			std::isfinite(Value.z) && std::isfinite(Value.w);
	}

	// 월드 위치의 XYZ 절댓값에 상한을 둬 광원 방향/행렬 계산에 과도한 좌표가 들어가지 않게 한다.
	bool_t IsFiniteShadowPosition(const float4_t& Value)
	{
		constexpr f32_t MAX_ABSOLUTE_POSITION = 1000000.f;
		return IsFinite4(Value) &&
			std::abs(Value.x) <= MAX_ABSOLUTE_POSITION &&
			std::abs(Value.y) <= MAX_ABSOLUTE_POSITION &&
			std::abs(Value.z) <= MAX_ABSOLUTE_POSITION;
	}

	// 활성 광원에 필요한 유한성·방향 길이·직교 투영 크기와 설정 범위를 확인한다.
	// PCF 반경은 비활성 요청에도 적용되는 별도 선행 검사이므로 Apply에서 확인한다.
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
			!std::isfinite(Settings.fStrength) ||
			!std::isfinite(Settings.fDynamicBakedStrength))
		{
			return false;
		}

		// Eye에서 At으로 향하는 월드 방향이다. 길이 제곱은 월드 거리의 제곱이며 거의 같은 두 점을 거른다.
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
			Settings.fStrength <= 1.f &&
			Settings.fDynamicBakedStrength >= 0.f &&
			Settings.fDynamicBakedStrength <= 1.f;
	}
}

// 그림자 광원이 비활성인 초기 상태에서도 바인딩할 VIEW/PROJ는 항등 행렬로 준비한다.
CShadow::CShadow()
{
	XMStoreFloat4x4(
		&m_TransformMatrices[ETOUI(D3DTS::VIEW)],
		XMMatrixIdentity());
	XMStoreFloat4x4(
		&m_TransformMatrices[ETOUI(D3DTS::PROJ)],
		XMMatrixIdentity());
}

// 값 타입만 소유하므로 별도 해제 작업은 없다.
CShadow::~CShadow()
{
}

// GameInstance의 적용 요청을 처리한다. 입력 검증과 지역 행렬 준비를 끝낸 뒤에만 활성 상태를 교체한다.
HRESULT CShadow::Apply_Shadow_Light(const SHADOW_LIGHT_DESC& ShadowLightDesc)
{
	// 비활성 요청도 PCF 반경 0/1/2 범위를 먼저 검사한다. 실패하면 현재 상태를 변경하지 않는다.
	if (ShadowLightDesc.Settings.iPCFFilterRadius > 2u)
		return E_INVALIDARG;

	// 비활성화 시 입력의 다른 튜닝 값은 보관하지 않고 기본 설정으로 초기화하는 기존 계약이다.
	// PCF 반경만 전달값으로 유지하며 그림자 view/projection을 항등 행렬로 되돌린다.
	if (!ShadowLightDesc.Settings.bEnabled)
	{
		SHADOW_LIGHT_DESC Disabled{};
		Disabled.Settings.bEnabled = false;
		Disabled.Settings.iPCFFilterRadius = ShadowLightDesc.Settings.iPCFFilterRadius;
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

	// 왼손 좌표계의 광원 view를 만든다. 전방이 Y축과 거의 평행하면 up을 Z축으로 바꿔 퇴화를 피한다.
	const vector_t Eye = XMLoadFloat4(&ShadowLightDesc.vEye);
	const vector_t At = XMLoadFloat4(&ShadowLightDesc.vAt);
	const vector_t Forward = XMVector3Normalize(At - Eye);
	const f32_t VerticalAlignment = std::abs(
		XMVectorGetX(XMVector3Dot(
			Forward, XMVectorSet(0.f, 1.f, 0.f, 0.f))));
	const vector_t Up = VerticalAlignment > 0.99f ?
		XMVectorSet(0.f, 0.f, 1.f, 0.f) :
		XMVectorSet(0.f, 1.f, 0.f, 0.f);

	// 직교 투영의 폭/높이와 near/far는 입력 위치와 같은 월드 거리 단위다. 완성 전에는 기존 행렬을 건드리지 않는다.
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

	// 검증과 행렬 계산이 끝난 값들을 한 적용 경계에서 채택한다. GPU 자원의 재생성은 이 객체가 담당하지 않는다.
	m_ShadowLightDesc = ShadowLightDesc;
	m_TransformMatrices[ETOUI(D3DTS::VIEW)] = StagedView;
	m_TransformMatrices[ETOUI(D3DTS::PROJ)] = StagedProjection;

	return S_OK;
}

// shader와 상수 이름, 행렬 종류만 검사한다. 비활성 여부나 해당 상수가 shader에 있는지는 별도 경계다.
HRESULT CShadow::Bind_ShaderResource(
	shared_ptr<class CShader> pShader,
	const char_t* pConstantName, D3DTS eType)
{
	if (nullptr == pShader || nullptr == pConstantName || eType >= D3DTS::END)
		return E_INVALIDARG;

	return pShader->Bind_Matrix(pConstantName, &m_TransformMatrices[ETOUI(eType)]);
}

// Renderer의 조명 단계가 GameInstance를 거쳐 호출한다. 실패 이후의 바인딩은 생략되고 기존 성공분은 남는다.
HRESULT CShadow::Bind_LightingShaderResources(shared_ptr<class CShader> pShader)
{
	if (nullptr == pShader)
		return E_INVALIDARG;

	// 행렬과 함께 shader가 소비하는 보정/품질 설정을 보낸다. 각 상수의 이름과 바인딩 순서는 유지한다.
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
			"g_iShadowPCFFilterRadius", &Settings.iPCFFilterRadius,
			sizeof(Settings.iPCFFilterRadius))) ||
		FAILED(pShader->Bind_RawValue(
			"g_fShadowStrength", &Settings.fStrength,
			sizeof(Settings.fStrength))) ||
		FAILED(pShader->Bind_RawValue("g_fDynamicBakedShadowStrength",
			&Settings.fDynamicBakedStrength, sizeof(Settings.fDynamicBakedStrength))))
	{
		return E_FAIL;
	}

	return S_OK;
}

// 생성된 광원 설정 객체의 단독 소유권을 GameInstance에 넘긴다.
unique_ptr<CShadow> CShadow::Create()
{
	return unique_ptr<CShadow>(new CShadow());
}
