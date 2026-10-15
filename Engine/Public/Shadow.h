#pragma once

// 행렬/벡터·HRESULT·D3DTS와 소유 포인터 등 엔진 공통 타입을 사용한다.
#include "Engine_Defines.h"
// 그림자 광원의 위치와 투영/필터 설정을 SHADOW_LIGHT_DESC 값으로 보관한다.
#include "Engine_RenderTypes.h"

NS_BEGIN(Engine)

// CGameInstance가 하나를 소유하고 Renderer가 읽는 그림자 광원의 설정과 행렬이다.
// 깊이 texture/DSV/SRV는 Renderer가 따로 소유한다. 설정 변경과 shader 바인딩은 외부에서 직렬화한다.
class CShadow final 
{
public:

private:
	// Create에서만 생성한다. VIEW/PROJ는 항등 행렬로, 나머지 저장 슬롯은 값 초기화 상태로 시작한다.
	CShadow();
public:
	// 별도 GPU 자원을 소유하지 않아 해제할 texture나 shader가 없다.
	~CShadow();

public:	
	// GameInstance가 전달한 설정을 검증한 뒤 적용한다. 잘못된 입력은 E_INVALIDARG로 기존 설정/행렬을 보존한다.
	// 비활성화 요청은 PCF 반경만 유지하고 나머지 설정을 기본값으로, VIEW/PROJ를 항등 행렬로 초기화한다.
	HRESULT Apply_Shadow_Light(const SHADOW_LIGHT_DESC& ShadowLightDesc);
	// 기존 호출 이름을 유지하는 위임 함수다. 광원을 추가 적재하지 않고 같은 단일 설정을 교체한다.
	HRESULT Add_Shadow_Light(const SHADOW_LIGHT_DESC& ShadowLightDesc)
	{
		return Apply_Shadow_Light(ShadowLightDesc);
	}
	// Renderer가 그림자 pass의 활성 여부를 판단할 때 사용하는 현재 설정 값이다.
	bool_t Is_Enabled() const
	{
		return m_ShadowLightDesc.Settings.bEnabled;
	}
	// 소유 설정의 읽기 전용 참조다. 객체 수명 안에서만 유효하며 다음 Apply로 내용이 바뀔 수 있다.
	const SHADOW_LIGHT_DESC& Get_Desc() const
	{
		return m_ShadowLightDesc;
	}
	// 활성 광원의 VIEW/PROJ만 차용 포인터로 제공한다. 비활성 또는 다른 종류는 nullptr다.
	// Renderer가 shadow pass와 정적 그림자 cache 비교에 사용하므로 읽는 동안 설정을 변경하지 않는다.
	const float4x4_t* Get_Transform(D3DTS eType) const
	{
		if (!Is_Enabled() || (eType != D3DTS::VIEW && eType != D3DTS::PROJ))
			return nullptr;
		return &m_TransformMatrices[ETOUI(eType)];
	}
	// GameInstance가 shader의 지정 상수에 저장 행렬 하나를 바인딩한다. shader 소유권은 저장하지 않는다.
	// Get_Transform과 달리 활성 여부를 검사하지 않으며 END 미만의 D3DTS 슬롯을 허용한다.
	// 정상 D3DTS 값을 전달하는 호출 경계다. 현재 END 이상만 거르므로 음수로 강제 변환한 enum 값까지 검증하지는 않는다.
	HRESULT Bind_ShaderResource(
		shared_ptr<class CShader> pShader,
		const char_t* pConstantName, D3DTS eType);
	// 조명 shader에 VIEW/PROJ와 bias·필터 반경·강도를 순서대로 전달한다.
	// 중간 실패 시 E_FAIL을 반환하지만 앞서 성공한 shader 바인딩은 되돌리지 않는다.
	HRESULT Bind_LightingShaderResources(shared_ptr<class CShader> pShader);

private:
	// 채택한 설정과 이를 바탕으로 만든 행렬을 값으로 소유한다. VIEW/PROJ 외 슬롯은 따로 계산하지 않는다.
	SHADOW_LIGHT_DESC		m_ShadowLightDesc = {};
	float4x4_t				m_TransformMatrices[ETOUI(D3DTS::END)] = {};

public:
	// GameInstance로 단독 소유권을 반환한다. new의 할당 예외를 여기서 변환하지 않는다.
	static unique_ptr<CShadow> Create();
};

NS_END
