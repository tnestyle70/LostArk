#pragma once

#include "GameObject.h"

/* 화면에 그려져야할 객체들을 그리는 순서에 따른 그룹별로 모아둔다. */
/* 모아둔 순서대로 객체들의 드로우콜을 수행해준다.*/

NS_BEGIN(Engine)

struct PRESENTATION_SCREEN_POST_DESC;

class CRenderer final 
{
private:	
	CRenderer(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
public:
	~CRenderer();

public:
	HRESULT Initialize();
	HRESULT Add_RenderObject(RENDERGROUP eRenderGroupID, shared_ptr<CGameObject> pRenderObject);
	HRESULT Draw();
	const RENDER_QUALITY_SETTINGS& Get_RenderQualitySettings() const { return m_RenderQualitySettings; }
	HRESULT Apply_RenderQualitySettings(const RENDER_QUALITY_SETTINGS& Settings);

#ifdef _DEBUG
	HRESULT Add_DebugComponent(shared_ptr<CComponent> pDebugComponent);
#endif

private:
	ComPtr<ID3D11Device>					m_pDevice = { nullptr };
	ComPtr<ID3D11DeviceContext>				m_pContext = { nullptr };
	ComPtr<ID3D11DepthStencilView>			m_pShadowDSV = { nullptr };
	ComPtr<ID3D11DepthStencilView>			m_pBloomDSV = { nullptr };
	list<shared_ptr<CGameObject>>			m_RenderObjects[ETOUI(RENDERGROUP::END)];

	shared_ptr<class CVIBuffer_Rect>		m_pVIBuffer = { nullptr };
	shared_ptr<class CShader>				m_pShader = { nullptr };

	float4x4_t								m_WorldMatrix{}, m_ViewMatrix{}, m_ProjMatrix{};
	uint32_t								m_iBloomWidth = {};
	uint32_t								m_iBloomHeight = {};
	float2_t								m_vBloomTexelSize = {};
	ComPtr<ID3D11Texture2D>				m_pScenePostTextures[2];
	ComPtr<ID3D11RenderTargetView>		m_pScenePostRTVs[2];
	ComPtr<ID3D11ShaderResourceView>	m_pScenePostSRVs[2];
	uint32_t								m_iScenePostWidth = {};
	uint32_t								m_iScenePostHeight = {};
	uint32_t								m_iScenePostFinalTarget = {};
	RENDER_QUALITY_SETTINGS				m_RenderQualitySettings = {};

#ifdef _DEBUG
	list<shared_ptr<CComponent>>			m_DebugComponent;
#endif

private:
	HRESULT Render_Priority();
	HRESULT Render_Shadow();
	HRESULT Render_NonBlend();
	HRESULT Render_Lights();
	HRESULT Render_Combined();
	HRESULT Render_NonLight();
	HRESULT Render_Blend();
	HRESULT Render_ScreenPosts();
	HRESULT Render_ScreenPostPass(
		ComPtr<ID3D11ShaderResourceView> pSourceSRV,
		ComPtr<ID3D11RenderTargetView> pDestinationRTV,
		uint32_t iPassIndex,
		const PRESENTATION_SCREEN_POST_DESC* pPostDesc = nullptr);
	HRESULT Render_Bloom();
	HRESULT Render_BloomPass(const wstring_t& strMRTTag,
		ComPtr<ID3D11ShaderResourceView> pSourceSRV, DEFERRED ePass);
	HRESULT Render_Final();
	HRESULT Render_UI();

private:
	HRESULT Ready_Shadow_DSV();
	HRESULT Ready_Bloom_DSV();
	HRESULT Ready_ScenePostTargets(uint32_t iWidth, uint32_t iHeight);
	void SetUp_ViewportDesc(uint32_t iWidth, uint32_t iHeight);

#ifdef _DEBUG
private:
	HRESULT Render_Debug();
#endif



public:
	static unique_ptr<CRenderer> Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);

};

NS_END
