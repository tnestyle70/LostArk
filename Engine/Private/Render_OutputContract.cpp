// 렌더 호출 scope의 출력 계약과 실제 context 바인딩 비교를 구현한다.
#include "Render_OutputContract.h"

// D3D11의 전체 RTV slot을 고정 배열로 읽어 추가 출력이 없는지 확인한다.
#include <array>

namespace
{
	// 현재 scope에서 관측한 context/view identity다. 포인터는 차용하며 구조체 자체가 COM 참조를 소유하지 않는다.
	struct ACTIVE_RENDER_OUTPUT_CONTRACT final
	{
		Engine::RENDER_OUTPUT_CONTRACT eContract =
			Engine::RENDER_OUTPUT_CONTRACT::NONE;
		ID3D11DeviceContext* pContext = nullptr;
		ID3D11RenderTargetView* pSceneColor = nullptr;
		ID3D11RenderTargetView* pDistortion = nullptr;
		ID3D11RenderTargetView* pBloom = nullptr;
		ID3D11DepthStencilView* pDepthStencil = nullptr;
	};

	// 스레드마다 별도 조회 상태를 가진다. 다른 렌더 스레드의 scope가 현재 호출의 계약을 바꾸지 않는다.
	thread_local ACTIVE_RENDER_OUTPUT_CONTRACT g_ActiveOutputContract;
}

NS_BEGIN(Engine)

// scope가 선언한 종류만 반환한다. 외부가 바꾼 GPU 바인딩까지 이 조회로 확인한 것은 아니다.
RENDER_OUTPUT_CONTRACT
CRenderOutputContract::Get_Active()
{
	return g_ActiveOutputContract.eContract;
}

// 현재 scope와 실제 context의 출력 identity를 대조한다. 실패해도 context나 scope를 변경하지 않는다.
bool_t CRenderOutputContract::Matches_ActiveRenderTargets(
	ID3D11DeviceContext* pContext)
{
	if (nullptr == pContext ||
		RENDER_OUTPUT_CONTRACT::NONE == g_ActiveOutputContract.eContract ||
		pContext != g_ActiveOutputContract.pContext ||
		nullptr == g_ActiveOutputContract.pSceneColor ||
		nullptr == g_ActiveOutputContract.pDistortion ||
		nullptr == g_ActiveOutputContract.pBloom ||
		nullptr == g_ActiveOutputContract.pDepthStencil)
	{
		return false;
	}

	std::array<ID3D11RenderTargetView*,
		D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT> RenderTargets{};
	ID3D11DepthStencilView* pDepthStencilRaw = nullptr;
	pContext->OMGetRenderTargets(
		static_cast<UINT>(RenderTargets.size()), RenderTargets.data(),
		&pDepthStencilRaw);
	// OMGetRenderTargets가 반환한 참조를 Attach로 인계받아 모든 반환 경로에서 해제한다. 불필요한 추가 AddRef는 하지 않는다.
	std::array<ComPtr<ID3D11RenderTargetView>,
		D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT> RenderTargetRefs;
	for (size_t iTarget = 0u; iTarget < RenderTargets.size(); ++iTarget)
	{
		RenderTargetRefs[iTarget].Attach(RenderTargets[iTarget]);
	}
	ComPtr<ID3D11DepthStencilView> pDepthStencil;
	pDepthStencil.Attach(pDepthStencilRaw);
	// RT0 scene color·RT1 distortion·RT2 bloom·DSV가 같은 view인지 확인한다. 같은 resource의 다른 view도 여기서는 불일치다.
	if (RenderTargetRefs[0u].Get() !=
			g_ActiveOutputContract.pSceneColor ||
		RenderTargetRefs[1u].Get() !=
			g_ActiveOutputContract.pDistortion ||
		RenderTargetRefs[2u].Get() != g_ActiveOutputContract.pBloom ||
		pDepthStencil.Get() != g_ActiveOutputContract.pDepthStencil)
	{
		return false;
	}
	for (size_t iTarget = 3u; iTarget < RenderTargetRefs.size(); ++iTarget)
	{
		if (nullptr != RenderTargetRefs[iTarget].Get())
		{
			return false;
		}
	}
	return true;
}

// 이전 thread-local 상태를 값으로 보관하고 지금 bound된 출력을 새 identity로 관측한다. GPU 출력 선택은 먼저 Renderer가 수행한다.
CRenderOutputContractScope::CRenderOutputContractScope(
	const RENDER_OUTPUT_CONTRACT eContract,
	ID3D11DeviceContext* pContext)
	: m_ePrevious(g_ActiveOutputContract.eContract)
	, m_pPreviousContext(g_ActiveOutputContract.pContext)
	, m_pPreviousSceneColor(g_ActiveOutputContract.pSceneColor)
	, m_pPreviousDistortion(g_ActiveOutputContract.pDistortion)
	, m_pPreviousBloom(g_ActiveOutputContract.pBloom)
	, m_pPreviousDepthStencil(g_ActiveOutputContract.pDepthStencil)
{
	std::array<ID3D11RenderTargetView*,
		D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT> RenderTargets{};
	ID3D11DepthStencilView* pDepthStencil = nullptr;
	if (nullptr != pContext)
	{
		pContext->OMGetRenderTargets(
			static_cast<UINT>(RenderTargets.size()), RenderTargets.data(),
			&pDepthStencil);
	}
	// 조회용 identity를 기록한 뒤 OMGet의 임시 COM 참조는 아래에서 반환한다. 자원 수명은 Renderer/context가 유지해야 한다.
	g_ActiveOutputContract.eContract = eContract;
	g_ActiveOutputContract.pContext = pContext;
	g_ActiveOutputContract.pSceneColor = RenderTargets[0u];
	g_ActiveOutputContract.pDistortion = RenderTargets[1u];
	g_ActiveOutputContract.pBloom = RenderTargets[2u];
	g_ActiveOutputContract.pDepthStencil = pDepthStencil;
	// 관측용 Get이 늘린 참조만 반환한다. scope 자체는 view 소유 참조를 계속 보관하지 않는다.
	for (ID3D11RenderTargetView* pRenderTarget : RenderTargets)
	{
		if (nullptr != pRenderTarget)
		{
			pRenderTarget->Release();
		}
	}
	if (nullptr != pDepthStencil)
	{
		pDepthStencil->Release();
	}
	// 추가 RTV가 있으면 identity를 무효화해 Matches가 false가 되게 한다. 이미 선언한 enum 값 자체는 그대로 남긴다.
	for (size_t iTarget = 3u; iTarget < RenderTargets.size(); ++iTarget)
	{
		if (nullptr != RenderTargets[iTarget])
		{
			g_ActiveOutputContract.pContext = nullptr;
			g_ActiveOutputContract.pSceneColor = nullptr;
			g_ActiveOutputContract.pDistortion = nullptr;
			g_ActiveOutputContract.pBloom = nullptr;
			g_ActiveOutputContract.pDepthStencil = nullptr;
			break;
		}
	}
}

// 중첩 이전 조회 상태로 복구한다. 그 사이 GPU target이 바뀌었다면 소비자는 Matches에서 따로 확인해야 한다.
CRenderOutputContractScope::~CRenderOutputContractScope()
{
	g_ActiveOutputContract.eContract = m_ePrevious;
	g_ActiveOutputContract.pContext = m_pPreviousContext;
	g_ActiveOutputContract.pSceneColor = m_pPreviousSceneColor;
	g_ActiveOutputContract.pDistortion = m_pPreviousDistortion;
	g_ActiveOutputContract.pBloom = m_pPreviousBloom;
	g_ActiveOutputContract.pDepthStencil = m_pPreviousDepthStencil;
}

NS_END
