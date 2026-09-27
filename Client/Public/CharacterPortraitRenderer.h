#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <memory>

NS_BEGIN(Client)

class CCharacter;

/* Off-screen character portrait shared by the character info window, the avatar book and the MVP
award page: the replicated character's own parts drawn a second time into an owned render target
with a camera parked in front of the character. This class owns the target and the camera only;
the draw itself is a request the engine runs through the world's own deferred path, so the
portrait carries the same materials and lighting the field does. The window shows the target
through a CUILayoutRuntime slot (Set_SlotTextureSRV). Nothing about the character itself changes;
an avatar kind mask can hide avatar pieces for this draw only. */
class CCharacterPortraitRenderer final
{
public:
	struct CAMERA
	{
		f32_t fDistance = 3.2f;      /* metres in front of the character */
		f32_t fEyeHeight = 1.05f;
		f32_t fLookHeight = 0.95f;
		f32_t fFovDegrees = 35.f;
		f32_t fYawDegrees = 0.f;     /* extra turn around the up axis (0 = facing the camera) */
	};

public:
	CCharacterPortraitRenderer(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
	~CCharacterPortraitRenderer() = default;

public:
	/* Queues the portrait for this frame's Draw, which fills it before the world pass. Call it
	after CGameInstance::Render_Begin and before CGameInstance::Render; the UI sprites sample the
	target later in that same world render. S_FALSE when there is nothing to draw. */
	HRESULT Render(const std::shared_ptr<CCharacter>& pCharacter, uint32_t iWidth, uint32_t iHeight,
		const CAMERA& Camera, uint32_t iAvatarOverrideKinds, uint32_t iAvatarHiddenKinds);
	ComPtr<ID3D11ShaderResourceView> Get_SRV() const { return m_pSRV; }

private:
	HRESULT Ensure_Target(uint32_t iWidth, uint32_t iHeight);

private:
	ComPtr<ID3D11Device>				m_pDevice;
	ComPtr<ID3D11DeviceContext>			m_pContext;
	ComPtr<ID3D11Texture2D>				m_pTexture;
	ComPtr<ID3D11RenderTargetView>		m_pRTV;
	ComPtr<ID3D11ShaderResourceView>	m_pSRV;
	uint32_t m_iWidth = 0;
	uint32_t m_iHeight = 0;
};

NS_END
