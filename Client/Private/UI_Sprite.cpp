// CUIObject의 화면 좌표·Transform에 이미지와 caption 렌더를 더한다.
#include "UI_Sprite.h"

// 렌더 그룹 등록·viewport·caption 그리기와 컷신 UI 차단 상태를 사용한다.
#include "GameInstance.h"
#include "UIInputRouter.h"

// Create가 장치 참조를 CUIObject로 전달한다.
Client::CUI_Sprite::CUI_Sprite(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: CUIObject { pDevice, pContext }
{
}

// 멤버 smart pointer와 기반 클래스가 각 참조를 자동 해제한다.
Client::CUI_Sprite::~CUI_Sprite()
{
}

// MainApp이 등록할 원형은 여기서 별도 Component를 만들지 않는다.
HRESULT Client::CUI_Sprite::Initialize_Prototype()
{
	return S_OK;
}

// Clone 호출자는 유효한 UI_SPRITE_DESC를 전달해야 한다. 기반 상태를 준비하고 이미지일 때만 Component를 붙인다.
HRESULT Client::CUI_Sprite::Initialize(void* pArg)
{
	auto pDesc = static_cast<UI_SPRITE_DESC*>(pArg);

	m_strTextureTag = pDesc->strTextureTag;
	m_vReferenceResolution = pDesc->referenceResolution;
    m_Caption = pDesc->caption;
    m_CaptionFont = pDesc->captionFont;

	if (FAILED(__super::Initialize(pDesc)))
		return E_FAIL;

	if (m_Caption.empty() && FAILED(Ready_Components()))
		return E_FAIL;

	return S_OK;
}

// Layer 갱신이 보이는 sprite를 UI 또는 SCENE_UI 렌더 그룹에 등록한다.
void Client::CUI_Sprite::Late_Update(f32_t fTimeDelta)
{
	if (!m_bVisible)
		return;
	CGameInstance::Get().Add_RenderObject(m_bScenePresentation ? RENDERGROUP::SCENE_UI : RENDERGROUP::UI,
		static_pointer_cast<CGameObject>(shared_from_this()));
}

// Renderer가 호출한다. SCENE_UI의 실패만 이 sprite에 격리하고 일반 UI 실패는 그대로 돌려준다.
HRESULT Client::CUI_Sprite::Render_Group(const RENDERGROUP group)
{
    const HRESULT result = Render();
    if (FAILED(result) && group == RENDERGROUP::SCENE_UI)
    {
        m_bScenePresentationFailed = true;
        m_bVisible = false;
        OutputDebugStringA("Scene presentation UI sprite failed; isolated until the view is recreated.\n");
        return S_OK;
    }
    return result;
}

// Renderer가 caption 또는 텍스처 quad를 그린다. 자원 바인딩·Draw 실패는 E_FAIL로 전달한다.
HRESULT Client::CUI_Sprite::Render()
{
	// 컷신 시작 전에 렌더 큐에 들어온 UI도 실제 그리기 직전에 숨긴다.
	if (!m_bVisible || (!m_bCinematicOverlay && CUIInputRouter::Get().Is_CinematicSuppressed()))
		return S_OK;
	const auto viewport = CGameInstance::Get().Get_ViewportSize();
	// caption은 Texture·Shader Component 없이 폰트로 그린다.
	if (!m_Caption.empty())
	{
		const auto measured = CGameInstance::Get().Measure_Text(m_CaptionFont, m_Caption.c_str());
		if (measured.y <= 0.f) return E_FAIL;
		const float sx = viewport.x / m_vReferenceResolution.x;
		const float sy = viewport.y / m_vReferenceResolution.y;
		CGameInstance::Get().Draw_Text(m_CaptionFont, m_Caption.c_str(), {m_fX * sx, m_fY * sy},
			Colors::White, 0.f, {0.5f, 0.5f}, m_fSizeY * 0.6f / measured.y * (std::min)(sx, sy));
		return S_OK;
	}
	if (viewport.x != m_vAppliedViewport.x || viewport.y != m_vAppliedViewport.y)
		Apply_Transform();
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Bind_Resources()))
		return E_FAIL;

	// Shader_VtxTex의 pass 2는 UI 알파 합성, pass 3은 가산 합성이다. 두 pass는 깊이 텍스처를 요구하지 않는다.
	if (FAILED(m_pShaderCom->Begin(m_bAdditive ? 3 : 2)))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Render()))
		return E_FAIL;

	return S_OK;
}

// 아래 표현 setter는 view가 정한 값을 보관하고 Render의 shader 바인딩에서 사용한다.
void Client::CUI_Sprite::Set_Tint(const float4_t& vTint)
{
	m_vTint = vTint;
}

void Client::CUI_Sprite::Set_TintMultiplier(const float4_t& vTintMultiplier)
{
	m_vTintMultiplier = vTintMultiplier;
}

void Client::CUI_Sprite::Set_FlipX(bool_t bFlipX)
{
	m_bFlipX = bFlipX;
}

void Client::CUI_Sprite::Set_Additive(bool_t bAdditive)
{
	m_bAdditive = bAdditive;
}

void Client::CUI_Sprite::Set_FillRatio(f32_t fFillRatio)
{
	m_fFillRatio = fFillRatio;
}

void Client::CUI_Sprite::Set_ArcRatio(f32_t fArcRatio)
{
	m_fArcRatio = fArcRatio;
}

void Client::CUI_Sprite::Set_UVWindow(const float2_t& vOffset, const float2_t& vScale)
{
	m_vUVOffset = vOffset;
	m_vUVScale = vScale;
}

void Client::CUI_Sprite::Set_UVRotation(f32_t fRadians, f32_t fAspect)
{
	m_fUVRotation = fRadians;
	m_fUVAspect = fAspect > 0.f ? fAspect : 1.f;
}

void Client::CUI_Sprite::Set_Rotation(f32_t fDegrees)
{
	if (m_fRotationDeg == fDegrees)
		return;
	m_fRotationDeg = fDegrees;
	Apply_Transform();
}

// SCENE_UI에서 실패한 sprite는 true 요청으로도 다시 보이지 않는다.
void Client::CUI_Sprite::Set_Visible(bool_t bVisible)
{
	m_bVisible = bVisible && !m_bScenePresentationFailed;
}

// SRV의 COM 참조를 보유한다. nullptr면 다음 Render에서 원래 Texture Component를 사용한다.
void Client::CUI_Sprite::Set_Texture(ComPtr<ID3D11ShaderResourceView> pOverrideSRV)
{
	m_pOverrideTextureSRV = pOverrideSRV;
}

// view가 문서 기준 중심·크기를 바꾸면 즉시 Transform에 반영한다.
void Client::CUI_Sprite::Set_Rect(f32_t fCenterX, f32_t fCenterY, f32_t fSizeX, f32_t fSizeY)
{
	m_fX = fCenterX;
	m_fY = fCenterY;
	m_fSizeX = fSizeX;
	m_fSizeY = fSizeY;

	Apply_Transform();
}

// Set_Rect·Set_Rotation·viewport 변경이 문서 좌표를 화면 중심 기준 Transform으로 바꾼다.
void Client::CUI_Sprite::Apply_Transform()
{
	const auto viewport = CGameInstance::Get().Get_ViewportSize();
	const float sx = m_vReferenceResolution.x > 0.f ? viewport.x / m_vReferenceResolution.x : 1.f;
	const float sy = m_vReferenceResolution.y > 0.f ? viewport.y / m_vReferenceResolution.y : 1.f;
	m_vAppliedViewport = viewport;
	// 폭이 0이었던 축도 다시 커질 수 있도록 이전 축을 정규화하지 않고 크기부터 재구성한다.
	m_pTransformCom->Set_State(STATE::RIGHT, XMVectorSet(m_fSizeX * sx, 0.f, 0.f, 0.f));
	m_pTransformCom->Set_State(STATE::UP, XMVectorSet(0.f, m_fSizeY * sy, 0.f, 0.f));
	m_pTransformCom->Set_State(STATE::LOOK, XMVectorSet(0.f, 0.f, 1.f, 0.f));
	// 화면 Y는 아래로 증가하므로 시계 방향 degree를 +Z축의 음수 회전으로 바꾼다.
	m_pTransformCom->Rotation(XMVectorSet(0.f, 0.f, 1.f, 0.f), -m_fRotationDeg);
	m_pTransformCom->Set_State(STATE::POSITION,
		XMVectorSet(
			m_fX * sx - viewport.x * 0.5f,
			-m_fY * sy + viewport.y * 0.5f,
			0.f, 1.f));
}

// Initialize가 원형에서 Texture·Shader·사각형 버퍼를 복제한다. 실패 즉시 E_FAIL을 돌려준다.
HRESULT Client::CUI_Sprite::Ready_Components()
{
	// 복제할 이미지의 Texture Component를 tag로 찾는다.
	if (FAILED(__super::Add_Component(ETOUI(LEVEL::STATIC), m_strTextureTag,
		TEXT("Com_Texture"), m_pTextureCom)))
		return E_FAIL;

	// UI용 pass가 있는 공용 Shader Component를 복제한다.
	if (FAILED(__super::Add_Component(ETOUI(LEVEL::STATIC), TEXT("Prototype_Component_Shader_VtxTex"),
		TEXT("Com_Shader"), m_pShaderCom)))
		return E_FAIL;

	// 이미지를 그릴 사각형 정점·인덱스 버퍼를 복제한다.
	if (FAILED(__super::Add_Component(ETOUI(LEVEL::STATIC), TEXT("Prototype_Component_VIBuffer_Rect"),
		TEXT("Com_VIBuffer"), m_pVIBufferCom)))
		return E_FAIL;

	return S_OK;
}

// Render가 행렬·이미지·색·UV를 Shader_VtxTex에 전달한다. 실패하면 Draw로 진행하지 않는다.
HRESULT Client::CUI_Sprite::Bind_ShaderResources()
{
	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	if (FAILED(__super::Bind_ShaderResource(m_pShaderCom, "g_ViewMatrix", D3DTS::VIEW)))
		return E_FAIL;

	const auto viewport = CGameInstance::Get().Get_ViewportSize();
	float4x4_t projection{};
	XMStoreFloat4x4(&projection, XMMatrixOrthographicLH(viewport.x, viewport.y, 0.f, 1.f));
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &projection)))
		return E_FAIL;

	if (nullptr != m_pOverrideTextureSRV)
	{
		if (FAILED(m_pShaderCom->Bind_Texture("g_Texture", m_pOverrideTextureSRV)))
			return E_FAIL;
	}
	else if (FAILED(m_pTextureCom->Bind_ShaderResource(m_pShaderCom, "g_Texture", 0)))
	{
		return E_FAIL;
	}

	// 원래 RGBA와 화면 소유자의 배율을 성분별로 곱해 최종 색을 만든다.
	const float4_t vEffectiveTint = float4_t(
		m_vTint.x * m_vTintMultiplier.x,
		m_vTint.y * m_vTintMultiplier.y,
		m_vTint.z * m_vTintMultiplier.z,
		m_vTint.w * m_vTintMultiplier.w);
	if (FAILED(m_pShaderCom->Bind_RawValue(
		"g_TintColor", &vEffectiveTint, sizeof(vEffectiveTint))))
		return E_FAIL;

	const int32_t iFlipX = m_bFlipX ? 1 : 0;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_FlipX", &iFlipX, sizeof(iFlipX))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_RawValue("g_FillRatio", &m_fFillRatio, sizeof(m_fFillRatio))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_RawValue("g_ArcRatio", &m_fArcRatio, sizeof(m_fArcRatio))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_RawValue("g_UVOffset", &m_vUVOffset, sizeof(m_vUVOffset))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_RawValue("g_UVScale", &m_vUVScale, sizeof(m_vUVScale))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_RawValue("g_UVRotation", &m_fUVRotation, sizeof(m_fUVRotation))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_RawValue("g_UVAspect", &m_fUVAspect, sizeof(m_fUVAspect))))
		return E_FAIL;

	return S_OK;
}

// MainApp의 원형 등록에 unique_ptr를 넘긴다. 현재 초기화 실패도 메시지 뒤 객체를 반환한다.
unique_ptr<Client::CUI_Sprite> Client::CUI_Sprite::Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
{
	auto pInstance = unique_ptr<CUI_Sprite>(new CUI_Sprite(pDevice, pContext));

	if (FAILED(pInstance->Initialize_Prototype()))
		MSG_BOX("Failed to Created : CUI_Sprite");

	return move(pInstance);
}

// 원형을 복사한 shared_ptr에 인스턴스 입력을 적용한다. 현재 초기화 실패도 메시지 뒤 객체를 반환한다.
shared_ptr<CPrototype> Client::CUI_Sprite::Clone(void* pArg)
{
	auto pInstance = shared_ptr<CUI_Sprite>(new CUI_Sprite(*this));

	if (FAILED(pInstance->Initialize(pArg)))
		MSG_BOX("Failed to Cloned : CUI_Sprite");

	return pInstance;
}
