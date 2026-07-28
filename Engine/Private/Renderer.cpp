#include "Renderer.h"
#include "GameInstance.h"

CRenderer::CRenderer(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: m_pDevice { pDevice }
	, m_pContext { pContext }
{

}

CRenderer::~CRenderer()
{
}

HRESULT CRenderer::Initialize()
{
	float2_t		vViewportSize = CGameInstance::Get().Get_ViewportSize();

	/* For.Target_Diffuse */
	if (FAILED(CGameInstance::Get().Add_RenderTarget(TEXT("Target_Diffuse"), vViewportSize.x, vViewportSize.y,
		DXGI_FORMAT_R8G8B8A8_UNORM, float4_t(1.f, 1.f, 1.f, 0.f))))
		return E_FAIL;

	/* For.Target_Normal */
	if (FAILED(CGameInstance::Get().Add_RenderTarget(TEXT("Target_Normal"), vViewportSize.x, vViewportSize.y,
		DXGI_FORMAT_R16G16B16A16_UNORM, float4_t(1.f, 1.f, 1.f, 1.f))))
		return E_FAIL;

	/* For.Target_Shade */
	if (FAILED(CGameInstance::Get().Add_RenderTarget(TEXT("Target_Shade"), vViewportSize.x, vViewportSize.y,
		DXGI_FORMAT_R8G8B8A8_UNORM, float4_t(0.f, 0.f, 0.f, 0.f))))
		return E_FAIL;

	/* For.Target_Depth */
	if (FAILED(CGameInstance::Get().Add_RenderTarget(TEXT("Target_Depth"), vViewportSize.x, vViewportSize.y,
		DXGI_FORMAT_R32G32B32A32_FLOAT, float4_t(1.f, 1.f, 1.f, 1.f))))
		return E_FAIL;

	/* For.Target_Specular */
	if (FAILED(CGameInstance::Get().Add_RenderTarget(TEXT("Target_Specular"), vViewportSize.x, vViewportSize.y,
		DXGI_FORMAT_R16G16B16A16_UNORM, float4_t(0.f, 0.f, 0.f, 0.f))))
		return E_FAIL;

	/* For.Target_PickPos */
	if (FAILED(CGameInstance::Get().Add_RenderTarget(TEXT("Target_PickPos"), vViewportSize.x, vViewportSize.y,
		DXGI_FORMAT_R32G32B32A32_FLOAT, float4_t(0.f, 0.f, 0.f, 0.f))))
		return E_FAIL;


	/* For.Target_LightDepth */
	if (FAILED(CGameInstance::Get().Add_RenderTarget(TEXT("Target_LightDepth"), g_iMaxWidth, g_iMaxHeight,
		DXGI_FORMAT_R32G32B32A32_FLOAT, float4_t(1.f, 1.f, 1.f, 1.f))))
		return E_FAIL;

	if (FAILED(Ready_Shadow_DSV()))
		return E_FAIL;



	/* MRT_GameObject */
	if (FAILED(CGameInstance::Get().Add_MRT(TEXT("MRT_GameObject"), TEXT("Target_Diffuse"))))
		return E_FAIL;
	if (FAILED(CGameInstance::Get().Add_MRT(TEXT("MRT_GameObject"), TEXT("Target_Normal"))))
		return E_FAIL;
	if (FAILED(CGameInstance::Get().Add_MRT(TEXT("MRT_GameObject"), TEXT("Target_Depth"))))
		return E_FAIL;
	if (FAILED(CGameInstance::Get().Add_MRT(TEXT("MRT_GameObject"), TEXT("Target_PickPos"))))
		return E_FAIL;

	/* MRT_LightAcc */
	if (FAILED(CGameInstance::Get().Add_MRT(TEXT("MRT_LightAcc"), TEXT("Target_Shade"))))
		return E_FAIL;
	if (FAILED(CGameInstance::Get().Add_MRT(TEXT("MRT_LightAcc"), TEXT("Target_Specular"))))
		return E_FAIL;

	/* MRT_ShadowObject */
	if (FAILED(CGameInstance::Get().Add_MRT(TEXT("MRT_ShadowObject"), TEXT("Target_LightDepth"))))
		return E_FAIL;


	m_pVIBuffer = CVIBuffer_Rect::Create(m_pDevice, m_pContext);
	if (nullptr == m_pVIBuffer)
		return E_FAIL;

	m_pShader = CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_Deferred.hlsl"), VTXTEX::Elements, VTXTEX::iNumElements);
	if (nullptr == m_pShader)
		return E_FAIL;
	
	XMStoreFloat4x4(&m_WorldMatrix, XMMatrixScaling(vViewportSize.x, vViewportSize.y, 1.f));
	XMStoreFloat4x4(&m_ViewMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&m_ProjMatrix,
		XMMatrixOrthographicLH(vViewportSize.x, vViewportSize.y, 0.f, 1.f));

#ifdef _DEBUG
	if (FAILED(CGameInstance::Get().Ready_RT_DebugDesc(TEXT("Target_LightDepth"), 150.f, 150.f, 300.f, 300.f)))
		return E_FAIL;
	//if (FAILED(CGameInstance::Get().Ready_RT_DebugDesc(TEXT("Target_Diffuse"), 150.f, 150.f, 300.f, 300.f)))
	//	return E_FAIL;
	//if (FAILED(CGameInstance::Get().Ready_RT_DebugDesc(TEXT("Target_Normal"), 150.f, 450.f, 300.f, 300.f)))
	//	return E_FAIL;
	//if (FAILED(CGameInstance::Get().Ready_RT_DebugDesc(TEXT("Target_Shade"), 450.f, 150.f, 300.f, 300.f)))
	//	return E_FAIL;
	//if (FAILED(CGameInstance::Get().Ready_RT_DebugDesc(TEXT("Target_Specular"), 450.f, 450.f, 300.f, 300.f)))
	//	return E_FAIL;
#endif

	return S_OK;
}

HRESULT CRenderer::Add_RenderObject(RENDERGROUP eRenderGroupID, shared_ptr<CGameObject> pRenderObject)
{
	if (nullptr == pRenderObject ||
		eRenderGroupID >= RENDERGROUP::END)
		return E_FAIL;

	m_RenderObjects[ETOUI(eRenderGroupID)].push_back(pRenderObject);	

	return S_OK;
}

HRESULT CRenderer::Draw()
{
	if (FAILED(Render_Priority()))
		return E_FAIL;
	if (FAILED(Render_Shadow()))
		return E_FAIL;
	if (FAILED(Render_NonBlend()))
		return E_FAIL;
	if (FAILED(Render_Lights()))
		return E_FAIL;
	if (FAILED(Render_Combined()))
		return E_FAIL;
	if (FAILED(Render_NonLight()))
		return E_FAIL;
	if (FAILED(Render_Blend()))
		return E_FAIL;
	if (FAILED(Render_UI()))
		return E_FAIL;

#ifdef _DEBUG
	if (FAILED(Render_Debug()))
		return E_FAIL;
#endif

	return S_OK;
}

#ifdef _DEBUG

HRESULT CRenderer::Add_DebugComponent(shared_ptr<CComponent> pDebugComponent)
{
	m_DebugComponent.push_back(pDebugComponent);
	return S_OK;
}

#endif

HRESULT CRenderer::Render_Priority()
{
	for (auto& pRenderObject : m_RenderObjects[ETOUI(RENDERGROUP::PRIORITY)])
	{
		if (nullptr != pRenderObject)
			pRenderObject->Render();
	}

	m_RenderObjects[ETOUI(RENDERGROUP::PRIORITY)].clear();

	return S_OK;
}

HRESULT CRenderer::Render_Shadow()
{
	if (FAILED(CGameInstance::Get().Begin_MRT(TEXT("MRT_ShadowObject"), m_pShadowDSV)))
		return E_FAIL;

	SetUp_ViewportDesc(g_iMaxWidth, g_iMaxHeight);

	for (auto& pRenderObject : m_RenderObjects[ETOUI(RENDERGROUP::SHADOW)])
	{
		if (nullptr != pRenderObject)
			pRenderObject->Render_Shadow();
	}

	m_RenderObjects[ETOUI(RENDERGROUP::SHADOW)].clear();

	if (FAILED(CGameInstance::Get().End_MRT()))
		return E_FAIL;

	SetUp_ViewportDesc(CGameInstance::Get().Get_ViewportSize().x, CGameInstance::Get().Get_ViewportSize().y);

	return S_OK;
}

HRESULT CRenderer::Render_NonBlend()
{
	/* Diffuse + Normal */
	if (FAILED(CGameInstance::Get().Begin_MRT(TEXT("MRT_GameObject"))))
		return E_FAIL;

	for (auto& pRenderObject : m_RenderObjects[ETOUI(RENDERGROUP::NONBLEND)])
	{
		if (nullptr != pRenderObject)
			pRenderObject->Render();
	}

	m_RenderObjects[ETOUI(RENDERGROUP::NONBLEND)].clear();

	if (FAILED(CGameInstance::Get().End_MRT()))
		return E_FAIL;

	return S_OK;
}

HRESULT CRenderer::Render_Lights()
{
	/* Shade */
	if (FAILED(CGameInstance::Get().Begin_MRT(TEXT("MRT_LightAcc"))))
		return E_FAIL;

	/* 빛 정보를 셰이더로 던진다. */
	if (FAILED(CGameInstance::Get().Bind_RT_SRV(TEXT("Target_Normal"), m_pShader, "g_NormalTexture")))
		return E_FAIL;	
	if (FAILED(CGameInstance::Get().Bind_RT_SRV(TEXT("Target_Depth"), m_pShader, "g_DepthTexture")))
		return E_FAIL;


	if (FAILED(m_pShader->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrixInverse", CGameInstance::Get().Get_InverseTransform(D3DTS::VIEW))))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrixInverse", CGameInstance::Get().Get_InverseTransform(D3DTS::PROJ))))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_RawValue("g_vCamPosition", CGameInstance::Get().Get_CamPosition(), sizeof(float4_t))))
		return E_FAIL;

	if (FAILED(m_pVIBuffer->Bind_Resources()))
		return E_FAIL;

	if (FAILED(CGameInstance::Get().Render_Lights(m_pShader, m_pVIBuffer)))
		return E_FAIL;

	if (FAILED(CGameInstance::Get().End_MRT()))
		return E_FAIL;

	return S_OK;
}

HRESULT CRenderer::Render_Combined()
{
	if (FAILED(CGameInstance::Get().Bind_RT_SRV(TEXT("Target_Diffuse"), m_pShader, "g_DiffuseTexture")))
		return E_FAIL;
	if (FAILED(CGameInstance::Get().Bind_RT_SRV(TEXT("Target_Shade"), m_pShader, "g_ShadeTexture")))
		return E_FAIL;
	if (FAILED(CGameInstance::Get().Bind_RT_SRV(TEXT("Target_Specular"), m_pShader, "g_SpecularTexture")))
		return E_FAIL;
	if (FAILED(CGameInstance::Get().Bind_RT_SRV(TEXT("Target_LightDepth"), m_pShader, "g_LightDepthTexture")))
		return E_FAIL;

	if (FAILED(m_pShader->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;
	if (FAILED(CGameInstance::Get().Bind_ShadowLight_ShaderResource(m_pShader, "g_LightViewMatrix", D3DTS::VIEW)))
		return E_FAIL;
	if (FAILED(CGameInstance::Get().Bind_ShadowLight_ShaderResource(m_pShader, "g_LightProjMatrix", D3DTS::PROJ)))
		return E_FAIL;


	if (FAILED(m_pShader->Begin(ETOUI(DEFERRED::COMBINED))))
		return E_FAIL;

	if (FAILED(m_pVIBuffer->Bind_Resources()))
		return E_FAIL;

	if (FAILED(m_pVIBuffer->Render()))
		return E_FAIL;

	return S_OK;
}

HRESULT CRenderer::Render_NonLight()
{
	for (auto& pRenderObject : m_RenderObjects[ETOUI(RENDERGROUP::NONLIGHT)])
	{
		if (nullptr != pRenderObject)
			pRenderObject->Render();
	}

	m_RenderObjects[ETOUI(RENDERGROUP::NONLIGHT)].clear();

	return S_OK;
}

HRESULT CRenderer::Render_Blend()
{
	for (auto& pRenderObject : m_RenderObjects[ETOUI(RENDERGROUP::BLEND)])
	{
		if (nullptr != pRenderObject)
			pRenderObject->Render();
	}

	m_RenderObjects[ETOUI(RENDERGROUP::BLEND)].clear();

	return S_OK;
}

HRESULT CRenderer::Render_UI()
{
	for (auto& pRenderObject : m_RenderObjects[ETOUI(RENDERGROUP::UI)])
	{
		if (nullptr != pRenderObject)
			pRenderObject->Render();
	}

	m_RenderObjects[ETOUI(RENDERGROUP::UI)].clear();

	return S_OK;
}

HRESULT CRenderer::Ready_Shadow_DSV()
{
	ComPtr<ID3D11Texture2D> pDepthStencilTexture = { nullptr };

	D3D11_TEXTURE2D_DESC	TextureDesc{};

	/* 깊이 버퍼의 픽셀은 백버퍼의 픽셀과 갯수가 동일해야만 깊이 테스트가 가능해진다. */
	/* 픽셀의 수가 다르면 아에 렌더링을 못함. */
	TextureDesc.Width = g_iMaxWidth;
	TextureDesc.Height = g_iMaxHeight;
	TextureDesc.MipLevels = 1;
	TextureDesc.ArraySize = 1;
	TextureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

	TextureDesc.SampleDesc.Quality = 0;
	TextureDesc.SampleDesc.Count = 1;

	/* 동적? 정적?  */
	TextureDesc.Usage = D3D11_USAGE_DEFAULT /* 정적 */;
	/* 추후에 어떤 용도로 바인딩 될 수 있는 View타입의 텍스쳐를 만들기위한 Texture2D입니까? */
	TextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL
		/*| D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE*/;
	TextureDesc.CPUAccessFlags = 0;
	TextureDesc.MiscFlags = 0;

	if (FAILED(m_pDevice->CreateTexture2D(&TextureDesc, nullptr, pDepthStencilTexture.GetAddressOf())))
		return E_FAIL;


	if (FAILED(m_pDevice->CreateDepthStencilView(pDepthStencilTexture.Get(), nullptr, m_pShadowDSV.GetAddressOf())))
		return E_FAIL;


	return S_OK;
}

void CRenderer::SetUp_ViewportDesc(uint32_t iWidth, uint32_t iHeight)
{
	D3D11_VIEWPORT			ViewPortDesc;
	ZeroMemory(&ViewPortDesc, sizeof(D3D11_VIEWPORT));
	ViewPortDesc.TopLeftX = 0;
	ViewPortDesc.TopLeftY = 0;
	ViewPortDesc.Width = static_cast<f32_t>(iWidth);
	ViewPortDesc.Height = static_cast<f32_t>(iHeight);
	ViewPortDesc.MinDepth = 0.f;
	ViewPortDesc.MaxDepth = 1.f;

	m_pContext->RSSetViewports(1, &ViewPortDesc);
}

#ifdef _DEBUG

HRESULT CRenderer::Render_Debug()
{
	for (auto& pDebugComponent : m_DebugComponent)
	{
		if (nullptr != pDebugComponent)
			pDebugComponent->Render();
	}
	m_DebugComponent.clear();

	if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;

	if (FAILED(m_pVIBuffer->Bind_Resources()))
		return E_FAIL;

	//if (FAILED(CGameInstance::Get().Render_MRT(TEXT("MRT_GameObject"), m_pShader, m_pVIBuffer)))
	//	return E_FAIL;

	//if (FAILED(CGameInstance::Get().Render_MRT(TEXT("MRT_LightAcc"), m_pShader, m_pVIBuffer)))
	//	return E_FAIL;

	if (FAILED(CGameInstance::Get().Render_MRT(TEXT("MRT_ShadowObject"), m_pShader, m_pVIBuffer)))
		return E_FAIL;

	return S_OK;
}

#endif

unique_ptr<CRenderer> CRenderer::Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
{
	auto pInstance = unique_ptr<CRenderer>(new CRenderer(pDevice, pContext));

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created : CRenderer");
		return nullptr;
	}

	return pInstance;
}

