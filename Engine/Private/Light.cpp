// 단일 빛의 검증과 지연 렌더 shader 바인딩을 구현한다.
#include "Light.h"
// 빛 종류·수신 대상과 shader pass index를 연결한다.
#include "Engine_RenderTypes.h"

// 기존 엔진 공개 include 경로로 CShader/CVIBuffer_Rect 인터페이스를 가져온다.
#include "GameInstance.h"

// 광원 입력의 유한성을 검사한다.
#include <cmath>

namespace
{
	// 단일 빛 경로의 공통 입력 검사다. 방향 벡터를 정규화하거나 색상을 보정하지 않으며 부적합 입력은 거부한다.
	bool_t IsValidLightAttenuation(const LIGHT_DESC& LightDesc)
	{
		// 원본 캐릭터의 간접광 항목은 방향광에만 허용한다. xyz 범위와 w=0을 검사한다.
		const auto& indirect = LightDesc.vSourceCharacterAmbient;
		if (!std::isfinite(indirect.x) || !std::isfinite(indirect.y) || !std::isfinite(indirect.z) ||
			indirect.x < 0.f || indirect.x > 64.f || indirect.y < 0.f || indirect.y > 64.f ||
			indirect.z < 0.f || indirect.z > 64.f || indirect.w != 0.f ||
			(LightDesc.eType != LIGHT::DIRECTIONAL &&
			 (indirect.x != 0.f || indirect.y != 0.f || indirect.z != 0.f)))
			return false;
		if (LightDesc.staticShadowChannel > 15u || (LightDesc.eReceiver != LIGHT_RECEIVER::ALL &&
			LightDesc.eReceiver != LIGHT_RECEIVER::SOURCE_CHARACTER &&
			LightDesc.eReceiver != LIGHT_RECEIVER::UNBAKED) ||
			(LIGHT::POINT != LightDesc.eType &&
			LIGHT::SPOT != LightDesc.eType &&
			LIGHT::DIRECTIONAL != LightDesc.eType) ||
			!std::isfinite(LightDesc.fFalloffExponent) ||
			LightDesc.fFalloffExponent <= 0.f)
		{
			return false;
		}
		// 지역광은 위치와 양수 range가 필요하다. 방향광은 이 위치/range 검사를 소비하지 않는다.
		if (LIGHT::DIRECTIONAL != LightDesc.eType &&
			(!std::isfinite(LightDesc.fRange) || LightDesc.fRange <= 0.f ||
			 !std::isfinite(LightDesc.vPosition.x) ||
			 !std::isfinite(LightDesc.vPosition.y) ||
			 !std::isfinite(LightDesc.vPosition.z) ||
			 !std::isfinite(LightDesc.vPosition.w)))
			return false;
		// 방향광/spot은 0이 아닌 유한 방향이 필요하다. spot 경계는 각도 자체가 아니라 내·외각 cosine의 순서로 검사한다.
		if (LIGHT::POINT != LightDesc.eType)
		{
			const float fDirectionLengthSquared =
				LightDesc.vDirection.x * LightDesc.vDirection.x +
				LightDesc.vDirection.y * LightDesc.vDirection.y +
				LightDesc.vDirection.z * LightDesc.vDirection.z;
			if (!std::isfinite(fDirectionLengthSquared) ||
				fDirectionLengthSquared <= 0.000001f ||
				!std::isfinite(LightDesc.vDirection.w))
				return false;
		}
		return LIGHT::SPOT != LightDesc.eType ||
			(std::isfinite(LightDesc.fSpotInnerCos) &&
			 std::isfinite(LightDesc.fSpotOuterCos) &&
			 LightDesc.fSpotOuterCos > 0.f &&
			 LightDesc.fSpotOuterCos <= LightDesc.fSpotInnerCos &&
			 LightDesc.fSpotOuterCos < 1.f && LightDesc.fSpotInnerCos <= 1.f);
	}
}

CLight::CLight()
{
}

CLight::~CLight()
{
}

// 검사를 모두 통과해야 소유 설명을 교체한다.
HRESULT CLight::Initialize(const LIGHT_DESC& LightDesc)
{
	if (!IsValidLightAttenuation(LightDesc))
		return E_INVALIDARG;

	m_LightDesc = LightDesc;

	return S_OK;
}

// 보관한 설명을 단일 제출 경로에 전달한다. 기본 호출은 방향/정적 그림자 추가 플래그를 켜지 않는다.
HRESULT CLight::Render(
	shared_ptr<class CShader> pShader,
	shared_ptr<class CVIBuffer_Rect> pVIBuffer)
{
	return Render_Desc(
		m_LightDesc, std::move(pShader), std::move(pVIBuffer), false);
}

// Light_Manager도 같은 기하/감쇠 검사를 호출한다. 장면 색상 검사는 별도다.
bool_t CLight::Is_ValidDesc(const LIGHT_DESC& LightDesc)
{
	return IsValidLightAttenuation(LightDesc);
}

// 입력 설명을 shader 상수와 pass로 옮긴 뒤 사각형을 그린다. 반환 성공은 API 제출 성공이며 최종 화면 판정은 아니다.
HRESULT CLight::Render_Desc(
	const LIGHT_DESC& LightDesc,
	shared_ptr<class CShader> pShader,
	shared_ptr<class CVIBuffer_Rect> pVIBuffer,
	bool_t bApplyDirectionalShadow, bool_t bApplyStaticShadow, bool_t bSourceLightMask)
{
	if (!IsValidLightAttenuation(LightDesc))
		return E_INVALIDARG;

	const uint32_t receiver = static_cast<uint32_t>(LightDesc.eReceiver);
	if (FAILED(pShader->Bind_RawValue("g_LightReceiver", &receiver, sizeof(receiver))))
		return E_FAIL;

    // 설명에 명시된 정적 그림자 channel을 우선한다. 없을 때만 호출자의 플래그를 기본 channel 1로 바꾼다.
    const uint32_t applyStaticShadow = LightDesc.staticShadowChannel != 0u ?
        LightDesc.staticShadowChannel : (bApplyStaticShadow ? 1u : 0u);
    if (FAILED(pShader->Bind_RawValue("g_ApplyStaticShadow", &applyStaticShadow, sizeof(applyStaticShadow))))
        return E_FAIL;
    uint32_t            iPassIndex = {};

    // 방향광은 방향/그림자 플래그를, point/spot은 위치·감쇠를 바인딩한다. spot은 cone cosine과 방향을 추가한다.
    if (LIGHT::DIRECTIONAL == LightDesc.eType)
    {
		const uint32_t iApplyDirectionalShadow =
			bApplyDirectionalShadow ? 1u : 0u;
        if (FAILED(pShader->Bind_RawValue("g_vLightDir", &LightDesc.vDirection, sizeof LightDesc.vDirection)))
            return E_FAIL;
		if (FAILED(pShader->Bind_RawValue(
			"g_iApplyDirectionalShadow", &iApplyDirectionalShadow,
			sizeof(iApplyDirectionalShadow))))
			return E_FAIL;

        iPassIndex = ETOUI(DEFERRED::DIRECTIONAL);
    }

    else if (LIGHT::POINT == LightDesc.eType || LIGHT::SPOT == LightDesc.eType)
    {
		const uint32_t iApplyDirectionalShadow = 0u;
		if (FAILED(pShader->Bind_RawValue(
			"g_iApplyDirectionalShadow", &iApplyDirectionalShadow,
			sizeof(iApplyDirectionalShadow))))
			return E_FAIL;
        if (FAILED(pShader->Bind_RawValue("g_vLightPos", &LightDesc.vPosition, sizeof LightDesc.vPosition)))
            return E_FAIL;
        if (FAILED(pShader->Bind_RawValue("g_fLightRange", &LightDesc.fRange, sizeof LightDesc.fRange)))
            return E_FAIL;
		if (FAILED(pShader->Bind_RawValue(
			"g_fLightFalloffExponent", &LightDesc.fFalloffExponent,
			sizeof LightDesc.fFalloffExponent)))
			return E_FAIL;

        iPassIndex = ETOUI(DEFERRED::POINT);
		if (LIGHT::SPOT == LightDesc.eType)
		{
			if (FAILED(pShader->Bind_RawValue(
				"g_vLightDir", &LightDesc.vDirection, sizeof LightDesc.vDirection)) ||
				FAILED(pShader->Bind_RawValue("g_fSpotInnerCos",
					&LightDesc.fSpotInnerCos, sizeof LightDesc.fSpotInnerCos)) ||
				FAILED(pShader->Bind_RawValue("g_fSpotOuterCos",
					&LightDesc.fSpotOuterCos, sizeof LightDesc.fSpotOuterCos)))
				return E_FAIL;
			iPassIndex = ETOUI(DEFERRED::SPOT);
		}
    }


    // 공통 색상과 원본 캐릭터 간접광을 전달한다. 앞선 바인딩이 성공했어도 후속 실패 시 그 상태를 복구하지 않는다.
    if (FAILED(pShader->Bind_RawValue("g_vLightDiffuse", &LightDesc.vDiffuse, sizeof LightDesc.vDiffuse)))
        return E_FAIL;
    if (FAILED(pShader->Bind_RawValue("g_vSourceCharacterAmbient",
        &LightDesc.vSourceCharacterAmbient, sizeof LightDesc.vSourceCharacterAmbient)))
        return E_FAIL;
    if (FAILED(pShader->Bind_RawValue("g_vLightAmbient", &LightDesc.vAmbient, sizeof LightDesc.vAmbient)))
        return E_FAIL;
    if (FAILED(pShader->Bind_RawValue("g_vLightSpecular", &LightDesc.vSpecular, sizeof LightDesc.vSpecular)))
        return E_FAIL;


    // 원본 캐릭터 수신 경로가 요청했으면 같은 빛 종류의 전용 mask pass로 선택을 바꾼다.
    if (bSourceLightMask)
    {
        iPassIndex = LIGHT::DIRECTIONAL == LightDesc.eType ? ETOUI(DEFERRED::SOURCE_DIRECTIONAL) :
            LIGHT::POINT == LightDesc.eType ? ETOUI(DEFERRED::SOURCE_POINT) : ETOUI(DEFERRED::SOURCE_SPOT);
    }

    // 상수 준비가 끝난 pass를 적용한 뒤 draw한다. vertex/index buffer 결합은 이 함수가 호출하지 않는다.
    if (FAILED(pShader->Begin(iPassIndex)))
        return E_FAIL;

    if (FAILED(pVIBuffer->Render()))
        return E_FAIL;

    return S_OK;
}

// 검증 후 임시 객체를 초기화한다. 실패 후보의 수명은 지역 unique_ptr가 정리한다.
unique_ptr<CLight> CLight::Create(const LIGHT_DESC& LightDesc)
{
	if (!IsValidLightAttenuation(LightDesc))
		return nullptr;

    auto pInstance = unique_ptr<CLight>(new CLight());

    if (FAILED(pInstance->Initialize(LightDesc)))
    {
        MSG_BOX("Failed to Created : CLight");
        return nullptr;
    }

    return pInstance;
}
