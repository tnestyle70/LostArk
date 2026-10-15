// 오프스크린 텍스처와 view의 생성·교체·바인딩을 구현한다.
#include "RenderTarget.h"

// Debug 미리보기 viewport 조회와 Shader/VIBuffer_Rect의 기존 엔진 인터페이스를 사용한다.
#include "GameInstance.h"

// 장치/context를 강한 참조로 보관해 이 target의 생성·렌더 호출 동안 수명을 유지한다.
CRenderTarget::CRenderTarget(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
    : m_pDevice { pDevice }
    , m_pContext { pContext }
{

}

// 멤버 ComPtr가 소유 자원을 해제한다. 아래 파일 저장 예시는 실행하지 않는 기존 코드다.
CRenderTarget::~CRenderTarget()
{
	//SaveDDSTextureToFile(m_pContext.Get(), m_pTexture2D.Get(), TEXT("../Bin/Diffuse.dds"));
}

// 현재 view의 소유 참조를 복사해 반환한다. 호출자가 가지고 있는 이전 resize의 view를 자동 갱신하지 않는다.
ComPtr<ID3D11ShaderResourceView> CRenderTarget::Get_SRV() const
{
	return m_pSRV;
}

// Create가 신규 객체에 호출한다. 단일 mip/단일 sample 텍스처와 RTV/SRV를 멤버에 순서대로 만든다.
HRESULT CRenderTarget::Initialize(
	uint32_t iWidth, uint32_t iHeight,
	DXGI_FORMAT ePixelFormat, const float4_t& vClearColor)
{
	D3D11_TEXTURE2D_DESC	TextureDesc{};
	
	TextureDesc.Width = iWidth;
	TextureDesc.Height = iHeight;
	TextureDesc.MipLevels = 1;
	TextureDesc.ArraySize = 1;
	TextureDesc.Format = ePixelFormat;

	TextureDesc.SampleDesc.Quality = 0;
	TextureDesc.SampleDesc.Count = 1;

	TextureDesc.Usage = D3D11_USAGE_DEFAULT;
	TextureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	TextureDesc.CPUAccessFlags = 0;
	TextureDesc.MiscFlags = 0;

	// 중간 실패 시 이 객체 안에는 앞서 생성한 자원이 남을 수 있다. Create/Stage_Resize의 임시 소유가 이를 정리한다.
	if (FAILED(m_pDevice->CreateTexture2D(&TextureDesc, nullptr, &m_pTexture2D)))
		return E_FAIL;

	if (FAILED(m_pDevice->CreateRenderTargetView(m_pTexture2D.Get(), nullptr, &m_pRTV)))
		return E_FAIL;
	if (FAILED(m_pDevice->CreateShaderResourceView(m_pTexture2D.Get(), nullptr, &m_pSRV)))
		return E_FAIL;

	m_vClearColor = vClearColor;

    return S_OK;
}

// Target_Manager의 묶음 준비가 호출한다. 기존 GPU 자원을 건드리지 않고 새 객체를 완성한 뒤 output을 교체한다.
HRESULT CRenderTarget::Stage_Resize(uint32_t width, uint32_t height,
    shared_ptr<CRenderTarget>& output) const
{
    if (!m_pTexture2D || width == 0u || height == 0u ||
        width > D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION || height > D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION)
        return E_INVALIDARG;

    D3D11_TEXTURE2D_DESC original{};
    m_pTexture2D->GetDesc(&original);
    auto staged = shared_ptr<CRenderTarget>(new CRenderTarget(m_pDevice, m_pContext));
    const HRESULT result = staged->Initialize(width, height, original.Format, m_vClearColor);
    if (FAILED(result))
        return result;

    output = std::move(staged);
    return S_OK;
}

// 묶음 준비 성공 후 한 번 호출한다. 자원을 교환해 외부 MRT 목록의 CRenderTarget 포인터를 유지한다.
void CRenderTarget::Commit_Resize(CRenderTarget& staged,
    f32_t viewportDeltaX, f32_t viewportDeltaY) noexcept
{
    // MRT 목록이 같은 객체를 계속 참조하도록 준비된 GPU 자원만 교환한다.
    m_pTexture2D.Swap(staged.m_pTexture2D);
    m_pRTV.Swap(staged.m_pRTV);
    m_pSRV.Swap(staged.m_pSRV);
#ifdef _DEBUG
    // viewport가 커진 절반만큼 중심 좌표를 보정해 기존 Debug 사각형의 좌상단 기준 위치를 유지한다.
    m_WorldMatrix._41 -= viewportDeltaX * .5f;
    m_WorldMatrix._42 += viewportDeltaY * .5f;
#else
    (void)viewportDeltaX;
    (void)viewportDeltaY;
#endif
}

// 후속 pass가 target 내용을 읽을 때 Shader의 리소스 변수로 전달한다.
HRESULT CRenderTarget::Bind_SRV(shared_ptr<class CShader> pShader, const char_t* pConstantName)
{
	return pShader->Bind_Texture(pConstantName, m_pSRV);
}

// 저장된 RGBA clear 색으로 view를 지운다. 유효한 초기화 자원을 가진 렌더 스레드 호출이 전제다.
void CRenderTarget::Clear()
{
	m_pContext->ClearRenderTargetView(m_pRTV.Get(), reinterpret_cast<const f32_t*>(&m_vClearColor));

}

// CopyResource의 목적지는 pTexture2D, 원본은 이 객체다. API가 HRESULT를 반환하지 않으므로 S_OK는 GPU 완료/내용 검증을 뜻하지 않는다.
HRESULT CRenderTarget::Copy_Resource(ComPtr<ID3D11Texture2D> pTexture2D)
{
	m_pContext->CopyResource(pTexture2D.Get(), m_pTexture2D.Get());

	return S_OK;
}

#ifdef _DEBUG

// 화면 중심 픽셀을 원점 중심/위쪽 양수 좌표로 변환해 Debug 표시용 world 행렬에 저장한다.
HRESULT CRenderTarget::Ready_DebugDesc(f32_t fX, f32_t fY, f32_t fSizeX, f32_t fSizeY)
{
	float2_t		vViewportSize = CGameInstance::Get().Get_ViewportSize();

	XMStoreFloat4x4(&m_WorldMatrix, XMMatrixIdentity());
	m_WorldMatrix._11 = fSizeX;
	m_WorldMatrix._22 = fSizeY;
	m_WorldMatrix._41 = fX - vViewportSize.x * 0.5f;
	m_WorldMatrix._42 = -fY + vViewportSize.y * 0.5f;

	return S_OK;
}

// Debug 미리보기의 world/texture/pass를 적용한 뒤 사각형 draw를 제출한다. 앞선 바인딩은 후속 실패에서 되돌리지 않는다.
HRESULT CRenderTarget::Render(shared_ptr<class CShader> pShader, shared_ptr<class CVIBuffer_Rect> pVIBuffer)
{
	if (FAILED(pShader->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)))
		return E_FAIL;

	if (FAILED(pShader->Bind_Texture("g_Texture", m_pSRV)))
		return E_FAIL;

	if (FAILED(pShader->Begin(ETOUI(DEFERRED::DEBUG))))
		return E_FAIL;

	if (FAILED(pVIBuffer->Render()))
		return E_FAIL;

	return S_OK;
}

#endif

// Target_Manager 등록용 객체를 만든다. 초기화 실패는 임시 소유를 해제하고 nullptr로 보고한다.
shared_ptr<CRenderTarget> CRenderTarget::Create(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext,
	uint32_t iWidth, uint32_t iHeight,
	DXGI_FORMAT ePixelFormat, const float4_t& vClearColor)
{
    auto pInstance = shared_ptr<CRenderTarget>(new CRenderTarget(pDevice, pContext));

    if (FAILED(pInstance->Initialize(iWidth, iHeight, ePixelFormat, vClearColor)))
    {
        MSG_BOX("Failed to Created : CRenderTarget");
        return nullptr;
    }

    return pInstance;
}
