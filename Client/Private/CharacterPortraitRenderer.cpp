#include "CharacterPortraitRenderer.h"

#include "Character.h"
#include "GameInstance.h"

namespace
{
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
	m_iWidth = 0;
	m_iHeight = 0;

	/* Colour only: the depth the portrait needs belongs to the engine G-buffer it borrows. */
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

	float4x4_t ViewMatrix{};
	float4x4_t ProjectionMatrix{};
	XMStoreFloat4x4(&ViewMatrix, XMMatrixLookAtLH(vEye, vAt, vUp));
	XMStoreFloat4x4(&ProjectionMatrix, XMMatrixPerspectiveFovLH(
		XMConvertToRadians(Camera.fFovDegrees),
		static_cast<f32_t>(iWidth) / static_cast<f32_t>(iHeight), PORTRAIT_NEAR, PORTRAIT_FAR));

	/* Drawing is the renderer's: it runs this callback against the same G-buffer and the same
	lighting the world uses, which is the only way a portrait stays on the field's look as
	material programs are added. Pass 0 is the world pass, so CPart_Body and CPart_Equipment
	select each mesh's own material exactly as they do in the field. */
	const std::weak_ptr<CCharacter> pSubject = pCharacter;
	return CGameInstance::Get().Request_Portrait(m_pRTV, iWidth, iHeight,
		ViewMatrix, ProjectionMatrix,
		[pSubject, iAvatarOverrideKinds, iAvatarHiddenKinds]() -> HRESULT
		{
			const shared_ptr<CCharacter> pLocked = pSubject.lock();
			if (nullptr == pLocked)
				return S_FALSE;
			return pLocked->Render_PreviewParts(
				0u, 0u, iAvatarOverrideKinds, iAvatarHiddenKinds);
		});
}
