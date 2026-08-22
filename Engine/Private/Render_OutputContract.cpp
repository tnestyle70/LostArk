#include "Render_OutputContract.h"

#include <array>

namespace
{
	struct ACTIVE_RENDER_OUTPUT_CONTRACT final
	{
		Engine::RENDER_OUTPUT_CONTRACT eContract =
			Engine::RENDER_OUTPUT_CONTRACT::NONE;
		ID3D11DeviceContext* pContext = nullptr;
		ID3D11RenderTargetView* pSceneColor = nullptr;
		ID3D11RenderTargetView* pDistortion = nullptr;
		ID3D11DepthStencilView* pDepthStencil = nullptr;
	};

	thread_local ACTIVE_RENDER_OUTPUT_CONTRACT g_ActiveOutputContract;
}

Engine::RENDER_OUTPUT_CONTRACT
Engine::CRenderOutputContract::Get_Active()
{
	return g_ActiveOutputContract.eContract;
}

bool_t Engine::CRenderOutputContract::Matches_ActiveRenderTargets(
	ID3D11DeviceContext* pContext)
{
	if (nullptr == pContext ||
		RENDER_OUTPUT_CONTRACT::NONE == g_ActiveOutputContract.eContract ||
		pContext != g_ActiveOutputContract.pContext ||
		nullptr == g_ActiveOutputContract.pSceneColor ||
		nullptr == g_ActiveOutputContract.pDistortion ||
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
	std::array<ComPtr<ID3D11RenderTargetView>,
		D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT> RenderTargetRefs;
	for (size_t iTarget = 0u; iTarget < RenderTargets.size(); ++iTarget)
		RenderTargetRefs[iTarget].Attach(RenderTargets[iTarget]);
	ComPtr<ID3D11DepthStencilView> pDepthStencil;
	pDepthStencil.Attach(pDepthStencilRaw);
	if (RenderTargetRefs[0u].Get() !=
			g_ActiveOutputContract.pSceneColor ||
		RenderTargetRefs[1u].Get() !=
			g_ActiveOutputContract.pDistortion ||
		pDepthStencil.Get() != g_ActiveOutputContract.pDepthStencil)
	{
		return false;
	}
	for (size_t iTarget = 2u; iTarget < RenderTargetRefs.size(); ++iTarget)
	{
		if (nullptr != RenderTargetRefs[iTarget].Get())
			return false;
	}
	return true;
}

Engine::CRenderOutputContractScope::CRenderOutputContractScope(
	const RENDER_OUTPUT_CONTRACT eContract,
	ID3D11DeviceContext* pContext)
	: m_ePrevious(g_ActiveOutputContract.eContract)
	, m_pPreviousContext(g_ActiveOutputContract.pContext)
	, m_pPreviousSceneColor(g_ActiveOutputContract.pSceneColor)
	, m_pPreviousDistortion(g_ActiveOutputContract.pDistortion)
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
	g_ActiveOutputContract.eContract = eContract;
	g_ActiveOutputContract.pContext = pContext;
	g_ActiveOutputContract.pSceneColor = RenderTargets[0u];
	g_ActiveOutputContract.pDistortion = RenderTargets[1u];
	g_ActiveOutputContract.pDepthStencil = pDepthStencil;
	for (ID3D11RenderTargetView* pRenderTarget : RenderTargets)
	{
		if (nullptr != pRenderTarget)
			pRenderTarget->Release();
	}
	if (nullptr != pDepthStencil)
		pDepthStencil->Release();
	for (size_t iTarget = 2u; iTarget < RenderTargets.size(); ++iTarget)
	{
		if (nullptr != RenderTargets[iTarget])
		{
			g_ActiveOutputContract.pContext = nullptr;
			g_ActiveOutputContract.pSceneColor = nullptr;
			g_ActiveOutputContract.pDistortion = nullptr;
			g_ActiveOutputContract.pDepthStencil = nullptr;
			break;
		}
	}
}

Engine::CRenderOutputContractScope::~CRenderOutputContractScope()
{
	g_ActiveOutputContract.eContract = m_ePrevious;
	g_ActiveOutputContract.pContext = m_pPreviousContext;
	g_ActiveOutputContract.pSceneColor = m_pPreviousSceneColor;
	g_ActiveOutputContract.pDistortion = m_pPreviousDistortion;
	g_ActiveOutputContract.pDepthStencil = m_pPreviousDepthStencil;
}
