#pragma once

// 장면 빛 목록의 vector와 렌더 호출 동안 사용할 소유 참조를 사용한다.
#include "Engine_Defines.h"
// 장면이 전달하는 LIGHT_DESC와 수신 대상 구분이다.
#include "Engine_RenderTypes.h"

NS_BEGIN(Engine)

// GameInstance가 소유하는 현재 장면 빛 목록이다. 제출은 렌더 스레드가 직렬화하며 내부 잠금은 없다.
class CLight_Manager final 
{
private:
	CLight_Manager();
public:
	~CLight_Manager();

public:
	// 이름과 달리 기존 목록에 append하지 않고 빛 하나짜리 목록으로 교체한다.
	HRESULT Add_Light(const LIGHT_DESC& LightDesc);
	// 최대 16개 설명을 전부 검사한 뒤 swap한다. 실패하면 기존 장면 빛을 그대로 유지한다.
	HRESULT Replace_SceneLights(vector<LIGHT_DESC> SceneLights);
	// 장면 빛 다음 frame transient 빛을 순서대로 배치 제출한다. 카메라 제외는 이번 deferred draw에만 적용한다.
	// 도중 실패는 이미 제출한 draw나 shader 상태를 되돌리지 않는다.
	HRESULT Render_Lights(shared_ptr<class CShader> pShader,
		shared_ptr<class CVIBuffer_Rect> pVIBuffer,
		bool_t bEnableSceneDirectionalShadow,
        LIGHT_RECEIVER ePassReceiver = LIGHT_RECEIVER::ALL,
		bool_t bSourceLightMask = false);
	// 현재 확정된 장면 빛의 렌더 스레드 차용 view다. 다음 목록 교체 이후 원소 주소를 보관하지 않는다.
	const vector<LIGHT_DESC>& Get_SceneLights() const
	{
		return m_SceneLights;
	}
	// 검증 도구가 목록 교체 성공/실패 뒤 확인하는 장면 빛 수다. transient와 카메라 제외 결과는 포함하지 않는다.
	uint32_t Get_SceneLightCount() const
	{
		return static_cast<uint32_t>(m_SceneLights.size());
	}

private:
	// 장면 설명을 값으로 소유한다. 일시적 빛은 Presentation_Manager 목록에서 호출 중에만 읽는다.
	vector<LIGHT_DESC>						m_SceneLights;

public:
	// 빈 관리자를 만든다. 장면 기본 조명을 자동으로 추가하지 않는다.
	static unique_ptr<CLight_Manager> Create();
};

NS_END
