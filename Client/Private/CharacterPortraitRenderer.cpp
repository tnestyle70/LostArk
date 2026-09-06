#include "CharacterPortraitRenderer.h"

#include "Character.h"
#include "GameInstance.h"

#include <array>

namespace
{
	/* Shader_VtxAnimMeshBinary pass order: Default, Shadow, three model-cue passes, ScreenCutin. */
	constexpr uint32_t SKINNED_FORWARD_PASS = 5u;
	/* Shader_VtxMeshBinary: ScreenCutin appended after DeferredEmissiveOverlayPass. */
	constexpr uint32_t SOCKETED_FORWARD_PASS = 19u;
	constexpr f32_t PORTRAIT_NEAR = 0.05f;
	constexpr f32_t PORTRAIT_FAR = 100.f;
}

Client::CCharacterPortraitRenderer::CCharacterPortraitRenderer(
	ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: m_pDevice{ pDevice }
	, m_pContext{ pContext }
{
}

HRESULT Client::CCharacterPortraitRenderer::Ensure_Target(const uint32_t iWidth, const uint32_t iHeight)
{
	if (nullptr != m_pRTV && iWidth == m_iWidth && iHeight == m_iHeight)
		return S_OK;
	m_pSRV.Reset();
	m_pRTV.Reset();
	m_pTexture.Reset();
	m_pDSV.Reset();
	m_pDepth.Reset();
	m_iWidth = 0;
	m_iHeight = 0;

	D3D11_TEXTURE2D_DESC ColorDesc{};
	ColorDesc.Width = iWidth;
	ColorDesc.Height = iHeight;
	ColorDesc.MipLevels = 1;
	ColorDesc.ArraySize = 1;
	ColorDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	ColorDesc.SampleDesc.Count = 1;
	ColorDesc.Usage = D3D11_USAGE_DEFAULT;
	ColorDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	if (FAILED(m_pDevice->CreateTexture2D(&ColorDesc, nullptr, &m_pTexture)) ||
		FAILED(m_pDevice->CreateRenderTargetView(m_pTexture.Get(), nullptr, &m_pRTV)) ||
		FAILED(m_pDevice->CreateShaderResourceView(m_pTexture.Get(), nullptr, &m_pSRV)))
		return E_FAIL;

	D3D11_TEXTURE2D_DESC DepthDesc = ColorDesc;
	DepthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	DepthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	if (FAILED(m_pDevice->CreateTexture2D(&DepthDesc, nullptr, &m_pDepth)) ||
		FAILED(m_pDevice->CreateDepthStencilView(m_pDepth.Get(), nullptr, &m_pDSV)))
		return E_FAIL;

	m_iWidth = iWidth;
	m_iHeight = iHeight;
	return S_OK;
}

HRESULT Client::CCharacterPortraitRenderer::Render(const std::shared_ptr<CCharacter>& pCharacter,
	const uint32_t iWidth, const uint32_t iHeight, const CAMERA& Camera,
	const uint32_t iAvatarOverrideKinds, const uint32_t iAvatarHiddenKinds)
{
	if (nullptr == pCharacter || nullptr == pCharacter->Get_Transform())
		return S_FALSE;
	if (0 == iWidth || 0 == iHeight || FAILED(Ensure_Target(iWidth, iHeight)))
		return E_FAIL;

	ComPtr<ID3D11RenderTargetView> pPreviousRTV;
	ComPtr<ID3D11DepthStencilView> pPreviousDSV;
	m_pContext->OMGetRenderTargets(1, &pPreviousRTV, &pPreviousDSV);
	std::array<D3D11_VIEWPORT, D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE>
		PreviousViewports{};
	uint32_t iPreviousViewportCount = static_cast<uint32_t>(PreviousViewports.size());
	m_pContext->RSGetViewports(&iPreviousViewportCount, PreviousViewports.data());

	/* The UI pass sampled this target last frame; unbind before it becomes a render target. */
	ID3D11ShaderResourceView* pNullSRVs[8] = {};
	m_pContext->PSSetShaderResources(0, 8, pNullSRVs);
	ID3D11RenderTargetView* pRTV = m_pRTV.Get();
	m_pContext->OMSetRenderTargets(1, &pRTV, m_pDSV.Get());
	const f32_t ClearColor[4] = { 0.f, 0.f, 0.f, 0.f };
	m_pContext->ClearRenderTargetView(m_pRTV.Get(), ClearColor);
	m_pContext->ClearDepthStencilView(m_pDSV.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);
	D3D11_VIEWPORT Viewport{};
	Viewport.Width = static_cast<f32_t>(iWidth);
	Viewport.Height = static_cast<f32_t>(iHeight);
	Viewport.MinDepth = 0.f;
	Viewport.MaxDepth = 1.f;
	m_pContext->RSSetViewports(1, &Viewport);

	/* Camera parked in front of the character (its own LOOK axis, turned by the yaw), so the
	portrait faces the viewer no matter where the character stands or looks in the world. */
	const shared_ptr<CTransform> pTransform = pCharacter->Get_Transform();
	const vector_t vPosition = pTransform->Get_State(STATE::POSITION);
	vector_t vLook = XMVector3Normalize(XMVectorSetY(pTransform->Get_State(STATE::LOOK), 0.f));
	if (XMVector3Equal(vLook, XMVectorZero()))
		vLook = XMVectorSet(0.f, 0.f, 1.f, 0.f);
	const vector_t vDirection = XMVector3TransformNormal(
		vLook, XMMatrixRotationY(XMConvertToRadians(Camera.fYawDegrees)));
	const vector_t vUp = XMVectorSet(0.f, 1.f, 0.f, 0.f);
	const vector_t vEye = XMVectorSetW(
		vPosition + vDirection * Camera.fDistance + vUp * Camera.fEyeHeight, 1.f);
	const vector_t vAt = XMVectorSetW(vPosition + vUp * Camera.fLookHeight, 1.f);
	const matrix_t ViewMatrix = XMMatrixLookAtLH(vEye, vAt, vUp);
	const matrix_t ProjectionMatrix = XMMatrixPerspectiveFovLH(
		XMConvertToRadians(Camera.fFovDegrees),
		static_cast<f32_t>(iWidth) / static_cast<f32_t>(iHeight), PORTRAIT_NEAR, PORTRAIT_FAR);

	CGameInstance& GameInstance = CGameInstance::Get();
	const float4x4_t PreviousView = *GameInstance.Get_Transform(D3DTS::VIEW);
	const float4x4_t PreviousProjection = *GameInstance.Get_Transform(D3DTS::PROJ);
	GameInstance.Set_Transform(D3DTS::VIEW, ViewMatrix);
	GameInstance.Set_Transform(D3DTS::PROJ, ProjectionMatrix);
	const HRESULT hResult = pCharacter->Render_PreviewParts(
		SKINNED_FORWARD_PASS, SOCKETED_FORWARD_PASS, iAvatarOverrideKinds, iAvatarHiddenKinds);
	GameInstance.Set_Transform(D3DTS::VIEW, XMLoadFloat4x4(&PreviousView));
	GameInstance.Set_Transform(D3DTS::PROJ, XMLoadFloat4x4(&PreviousProjection));

	ID3D11RenderTargetView* pRestoredRTV = pPreviousRTV.Get();
	m_pContext->OMSetRenderTargets(1, &pRestoredRTV, pPreviousDSV.Get());
	if (0 != iPreviousViewportCount)
		m_pContext->RSSetViewports(iPreviousViewportCount, PreviousViewports.data());
	return SUCCEEDED(hResult) ? S_OK : E_FAIL;
}
