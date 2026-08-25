#include "Picking.h"
#include "GameInstance.h"
#include "Profiler.h"

#include <chrono>
#include <cmath>

CPicking::CPicking(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
    : m_pDevice { pDevice }
    , m_pContext { pContext }
{
}

CPicking::~CPicking()
{

}

HRESULT CPicking::Initialize(HWND hWnd)
{
    m_hWnd = hWnd;

    m_vViewportSize = CGameInstance::Get().Get_ViewportSize();

    D3D11_TEXTURE2D_DESC            TextureDesc{};

	TextureDesc.Width = 1u;
	TextureDesc.Height = 1u;
    TextureDesc.MipLevels = 1;
    TextureDesc.ArraySize = 1;
    TextureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    TextureDesc.SampleDesc.Quality = 0;
    TextureDesc.SampleDesc.Count = 1;
    TextureDesc.Usage = D3D11_USAGE_STAGING;
    TextureDesc.BindFlags = 0;
	TextureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    TextureDesc.MiscFlags = 0;

    if (FAILED(m_pDevice->CreateTexture2D(&TextureDesc, nullptr, &m_pTexture2D)))
        return E_FAIL;

	return S_OK;
}

bool_t CPicking::Picking(float4_t& vOut)
{
	if (nullptr == m_pTexture2D)
		return false;

    ::POINT ptMouse = {};

    if (FALSE == GetCursorPos(&ptMouse) ||
        FALSE == ScreenToClient(m_hWnd, &ptMouse))
        return false;

    const LONG iViewportWidth =
        static_cast<LONG>(m_vViewportSize.x);
    const LONG iViewportHeight =
        static_cast<LONG>(m_vViewportSize.y);

    if (iViewportWidth <= 0 || iViewportHeight <= 0)
        return false;

    if (ptMouse.x < 0 ||
        ptMouse.y < 0 ||
        ptMouse.x >= iViewportWidth ||
        ptMouse.y >= iViewportHeight)
        return false;

	const ComPtr<ID3D11ShaderResourceView> pDepthSRV =
		CGameInstance::Get().Get_RT_SRV(TEXT("Target_Depth"));
	if (nullptr == pDepthSRV)
		return false;

	ComPtr<ID3D11Resource> pResource;
	pDepthSRV->GetResource(&pResource);
	ComPtr<ID3D11Texture2D> pSourceTexture;
	if (nullptr == pResource || FAILED(pResource.As(&pSourceTexture)))
		return false;

	Engine::CProfiler* pProfiler = CGameInstance::Get().Get_Profiler();
	Engine::CProfilerScope scope(pProfiler, "Client.PickingReadback");
	const auto readbackBegin = std::chrono::steady_clock::now();

	const D3D11_BOX sourceBox{
		static_cast<UINT>(ptMouse.x),
		static_cast<UINT>(ptMouse.y),
		0u,
		static_cast<UINT>(ptMouse.x + 1),
		static_cast<UINT>(ptMouse.y + 1),
		1u
	};
	m_pContext->CopySubresourceRegion(
		m_pTexture2D.Get(), 0u, 0u, 0u, 0u,
		pSourceTexture.Get(), 0u, &sourceBox);

	D3D11_MAPPED_SUBRESOURCE mapped{};
	const HRESULT mapResult = m_pContext->Map(
		m_pTexture2D.Get(), 0u, D3D11_MAP_READ, 0u, &mapped);
	if (FAILED(mapResult))
	{
		if (nullptr != pProfiler)
		{
			const auto elapsed =
				std::chrono::duration_cast<std::chrono::microseconds>(
					std::chrono::steady_clock::now() - readbackBegin).count();
			pProfiler->Add_Counter(
				Engine::EProfilerCounter::PickingReadbackMicroseconds,
				elapsed > 0 ? static_cast<uint64_t>(elapsed) : 0u);
		}
		return false;
	}

	const float4_t depthSample =
		*static_cast<const float4_t*>(mapped.pData);
	m_pContext->Unmap(m_pTexture2D.Get(), 0u);
	if (nullptr != pProfiler)
	{
		const auto elapsed =
			std::chrono::duration_cast<std::chrono::microseconds>(
				std::chrono::steady_clock::now() - readbackBegin).count();
		pProfiler->Add_Counter(
			Engine::EProfilerCounter::PickingReadbackBytes,
			sizeof(float4_t));
		pProfiler->Add_Counter(
			Engine::EProfilerCounter::PickingReadbackMicroseconds,
			elapsed > 0 ? static_cast<uint64_t>(elapsed) : 0u);
	}
	if (!std::isfinite(depthSample.x) ||
		depthSample.x < 0.f || depthSample.x >= 1.f)
	{
		return false;
	}

	const float4x4_t* pInverseProjection =
		CGameInstance::Get().Get_InverseTransform(D3DTS::PROJ);
	const float4x4_t* pInverseView =
		CGameInstance::Get().Get_InverseTransform(D3DTS::VIEW);
	if (nullptr == pInverseProjection || nullptr == pInverseView)
		return false;

	const f32_t ndcX =
		((static_cast<f32_t>(ptMouse.x) + 0.5f) /
			static_cast<f32_t>(iViewportWidth)) * 2.f - 1.f;
	const f32_t ndcY =
		1.f - ((static_cast<f32_t>(ptMouse.y) + 0.5f) /
			static_cast<f32_t>(iViewportHeight)) * 2.f;
	vector_t viewPosition = XMVector4Transform(
		XMVectorSet(ndcX, ndcY, depthSample.x, 1.f),
		XMLoadFloat4x4(pInverseProjection));
	const f32_t viewW = XMVectorGetW(viewPosition);
	if (!std::isfinite(viewW) || std::abs(viewW) <= 1.e-6f)
		return false;
	viewPosition = XMVectorScale(viewPosition, 1.f / viewW);

	vector_t worldPosition = XMVector4Transform(
		viewPosition, XMLoadFloat4x4(pInverseView));
	const f32_t worldW = XMVectorGetW(worldPosition);
	if (!std::isfinite(worldW) || std::abs(worldW) <= 1.e-6f)
		return false;
	worldPosition = XMVectorScale(worldPosition, 1.f / worldW);
	XMStoreFloat4(&vOut, worldPosition);
	vOut.w = 1.f;
	return std::isfinite(vOut.x) && std::isfinite(vOut.y) &&
		std::isfinite(vOut.z);
}

unique_ptr<CPicking> CPicking::Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext, HWND hWnd)
{
    auto pInstance = unique_ptr<CPicking>(new CPicking(pDevice, pContext));

    if (FAILED(pInstance->Initialize(hWnd)))
    {
        MSG_BOX("Failed to Created : CPicking");
        return nullptr;
    }

    return pInstance;
}
