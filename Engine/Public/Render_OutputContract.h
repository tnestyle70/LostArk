#pragma once

// D3D11 context/view 포인터와 엔진 export·정수 타입을 사용한다.
#include "Engine_Defines.h"

NS_BEGIN(Engine)

/* 렌더 객체가 실제 draw를 제출하는 동안 활성 출력 구성을 나타내는 읽기 전용 식별자다.
   Client는 typed 계약을 조회하며 render target 선택과 변경은 Renderer가 담당한다. */
enum class RENDER_OUTPUT_CONTRACT : uint8_t
{
	NONE,
	// RT2는 bloom 기여분이다. RT0 scene color와 RT1 distortion의 기존 역할을 유지한다.
	SCENE_HDR_RT0_SCENE_COLOR_RT1_DISTORTION,
	END
};

// Client 이펙트가 렌더 출력 계약을 조회하는 읽기 전용 경계다. target 선택·설정 권한은 Renderer가 가진다.
class ENGINE_DLL CRenderOutputContract final
{
public:
	// 현재 스레드 scope의 선언을 읽는다. 실제 bound target 일치는 별도 Matches 검사로 확인해야 한다.
	static RENDER_OUTPUT_CONTRACT Get_Active();
	// 같은 context의 RTV0/1/2·DSV identity와 나머지 RTV의 비어 있음을 확인한다. 포맷/해상도/픽셀 내용 검증은 아니다.
	static bool_t Matches_ActiveRenderTargets(
		ID3D11DeviceContext* pContext);

private:
	friend class CRenderer;
	friend class CRenderOutputContractScope;
};

// Renderer만 생성하는 중첩 가능 scope다. 현재 스레드의 계약과 view identity를 보관/복구하며 GPU target 바인딩을 바꾸지는 않는다.
class ENGINE_DLL CRenderOutputContractScope final
{
private:
	friend class CRenderer;
	CRenderOutputContractScope(
		RENDER_OUTPUT_CONTRACT eContract,
		ID3D11DeviceContext* pContext);

public:
	// 이전 scope의 조회 상태만 복구한다. OMSetRenderTargets나 GPU 자원 해제의 역할은 없다.
	~CRenderOutputContractScope();
	CRenderOutputContractScope(const CRenderOutputContractScope&) = delete;
	CRenderOutputContractScope& operator=(
		const CRenderOutputContractScope&) = delete;

private:
	// 이전 scope의 차용 identity다. COM AddRef를 보유하지 않으므로 Renderer/context의 실제 자원 소유 수명이 필요하다.
	RENDER_OUTPUT_CONTRACT m_ePrevious = RENDER_OUTPUT_CONTRACT::NONE;
	ID3D11DeviceContext* m_pPreviousContext = nullptr;
	ID3D11RenderTargetView* m_pPreviousSceneColor = nullptr;
	ID3D11RenderTargetView* m_pPreviousDistortion = nullptr;
	ID3D11RenderTargetView* m_pPreviousBloom = nullptr;
	ID3D11DepthStencilView* m_pPreviousDepthStencil = nullptr;
};

NS_END
