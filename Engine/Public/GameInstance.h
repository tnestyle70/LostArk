#pragma once

/* 라이브러리의 기능을 외부 사용자에게 보여주는 역활. */
/* 라이브러리 기준 유일무이한 싱글턴객체. */
#include "Prototype_Manager.h"

NS_BEGIN(Engine)

class ENGINE_DLL CGameInstance final
{
	DECLARE_SINGLETON(CGameInstance)
private:
	CGameInstance();
public:
	~CGameInstance();

public:
	float2_t Get_ViewportSize() {
		return m_vViewportDesc;
	}

	f32_t Random(f32_t fMin = 0.f, f32_t fMax = 1.f) {
		return fMin + (static_cast<f32_t>(rand()) / RAND_MAX) * (fMax - fMin);
	} 


public:
	/* 엔진의 기능을 이용할 수 있도록, 필요한 초기화 과정을 수행한다. */
	HRESULT Initialize_Engine(const ENGINE_DESC& EngineDesc, ComPtr<ID3D11Device>& pOutDevice, ComPtr<ID3D11DeviceContext>& pOutContext);
	void Update_Engine(f32_t fTimeDelta);
	HRESULT Render_Begin(const float4_t* pClearColor);
	HRESULT Render();
	HRESULT Render_End();
	HRESULT Clear_Resources(uint32_t iClearLevelID);

public: /* For.Graphic_Device */


public: /* For.Input_Device */
	int8_t	Get_DIKeyState(uint8_t byKeyID);
	bool_t Get_DIKeyPressed(uint8_t byKeyID);
	int8_t	Get_DIMouseState(DIM eMouse);
	int32_t	Get_DIMouseMove(DIMM eMouseState);
	void SetInputBlocked(bool_t bKeyboardBlocked, bool_t bMouseBlocked);
	void SetMouseButtonBlocked(DIM eMouse, bool_t blocked);
	bool_t IsKeyboardInputBlocked() const;
	bool_t IsMouseInputBlocked() const;

#ifdef _WIN64
public: /* For.Sound_Manager */
	HRESULT Play_Sound(const wstring_t& strSoundFilePath, f32_t fVolume);
#endif


public: /* For.Timer_Manager */
	f32_t Get_TimeDelta(const wstring_t& strTimerTag);
	HRESULT Add_Timer(const wstring_t& strTimerTag);
	void Update_TimeDelta(const wstring_t& strTimerTag);

public: /* For.Level_Manager */
	HRESULT Change_Level(uint32_t iCurrentLevelID, unique_ptr<class CLevel> pNewLevel);
	uint32_t Get_CurrentLevelID() const;

public: /* For.Prototype_Manager */
	HRESULT Add_Prototype(uint32_t iLevelIndex, const wstring_t& strPrototypeTag, unique_ptr<class CPrototype> pPrototype);
	shared_ptr<CPrototype> Clone_Prototype(uint32_t iLevelIndex, const wstring_t& strPrototypeTag, void* pArg = nullptr);

public: /* For.Object_Manager */
	shared_ptr<class CGameObject> Get_GameObject(uint32_t iLevelIndex, const wstring_t& strLayerTag, uint32_t iIndex);
	shared_ptr<CComponent> Get_Component(uint32_t iLevelIndex, const wstring_t& strLayerTag, const wstring_t& strComponentTag, uint32_t iIndex);
	shared_ptr<CComponent> Get_Component(uint32_t iLevelIndex, const wstring_t& strLayerTag, const wstring_t& strPartTag, const wstring_t& strComponentTag, uint32_t iIndex);
	HRESULT Add_GameObject_to_Layer(uint32_t iPrototypeLevelIndex, const wstring_t& strPrototypeTag, uint32_t iLayerLevelIndex, const wstring_t& strLayerTag, void* pArg = nullptr, shared_ptr<class CGameObject>* pOutGameObject = nullptr);
	HRESULT Remove_GameObject_from_Layer(uint32_t iLevelIndex, const wstring_t& strLayerTag, const shared_ptr<class CGameObject>& pGameObject);

public: /* Renderer */
	HRESULT Add_RenderObject(RENDERGROUP eRenderGroupID, shared_ptr<class CGameObject> pRenderObject);
#ifdef _DEBUG
	HRESULT Add_DebugComponent(shared_ptr<CComponent> pDebugComponent);
#endif

public: /* For.PipeLine */
	void Set_Transform(D3DTS eType, fmatrix_t TransformMatrix);
	const float4_t* Get_CamPosition();
	const float4x4_t* Get_Transform(D3DTS eType);
	const float4x4_t* Get_InverseTransform(D3DTS eType);
	HRESULT Bind_CamPosition(shared_ptr<class CShader> pShader, const char_t* pConstantName);
	HRESULT Bind_Transform(shared_ptr<class CShader> pShader, const char_t* pConstantName, D3DTS eType);
	HRESULT Bind_InverseTransform(shared_ptr<class CShader> pShader, const char_t* pConstantName, D3DTS eType);


public: /* For.Light */
	
	HRESULT Add_Light(const LIGHT_DESC& LightDesc);
	HRESULT Render_Lights(shared_ptr<class CShader> pShader, shared_ptr<class CVIBuffer_Rect> pVIBuffer);


public: /* For.Font_Manager */
	HRESULT Add_Font(const wstring& strFontTag, const tchar_t* pFontFilePath);
	void Draw_Text(const wstring& strFontTag, const tchar_t* pText, const float2_t& vPosition, fvector_t vColor = Colors::White, f32_t fRotation = 0.f, const float2_t& vOrigin = float2_t(0.f, 0.f), f32_t fScale = 1.f);

public: /* For.Target_Manager */
	HRESULT Add_RenderTarget(const wstring_t& strTargetTag, uint32_t iWidth, uint32_t iHeight, DXGI_FORMAT ePixelFormat, const float4_t& vClearColor);
	HRESULT Add_MRT(const wstring_t& strMRTTag, const wstring_t& strTargetTag);
	HRESULT Begin_MRT(const wstring_t& strMRTTag, ComPtr<ID3D11DepthStencilView> pDSV = nullptr);
	HRESULT End_MRT();
	HRESULT Bind_RT_SRV(const wstring_t& strTargetTag, shared_ptr<class CShader> pShader, const char_t* pConstantName);
	HRESULT Copy_RT_Resource(const wstring_t& strTargetTag, ComPtr<ID3D11Texture2D> pTexture2D);


#ifdef _DEBUG
public:
	HRESULT Ready_RT_DebugDesc(const wstring_t& strTargetTag, f32_t fX, f32_t fY, f32_t fSizeX, f32_t fSizeY);
	HRESULT Render_MRT(const wstring_t& strMRTTag, shared_ptr<class CShader> pShader, shared_ptr<class CVIBuffer_Rect> pVIBuffer);
#endif


public: /* For.Picking */
	bool_t Picking(float4_t& vOut);

public: /* For.Shadow */
	HRESULT Add_Shadow_Light(const SHADOW_LIGHT_DESC& ShadowLightDesc);
	HRESULT Bind_ShadowLight_ShaderResource(shared_ptr<class CShader> pShader, const char_t* pConstantName, D3DTS eType);

public: /* For.Frustum */	
	void Update_Frustum_InLocalSpace(fmatrix_t WorldMatrix);
	bool_t isIn_Frustum_InWorldSpace(fvector_t vWorldPoint, f32_t fRange);
	bool_t isIn_Frustum_InLocalSpace(fvector_t vLocalPoint, f32_t fRange);

public: /* For.Profiler */
	class CProfiler* Get_Profiler() const { return m_pProfiler.get(); }

private:
	unique_ptr<class CTimer_Manager>		m_pTimer_Manager = { nullptr };
	unique_ptr<class CGraphic_Device>		m_pGraphic_Device = { nullptr };
	unique_ptr<class CInput_Device>			m_pInput_Device = { nullptr };
#ifdef _WIN64
	unique_ptr<class CSound_Manager>		m_pSound_Manager = { nullptr };
#endif
	unique_ptr<class CLevel_Manager>		m_pLevel_Manager = { nullptr };
	unique_ptr<class CPrototype_Manager>	m_pPrototype_Manager = { nullptr };
	unique_ptr<class CObject_Manager>		m_pObject_Manager = { nullptr };
	unique_ptr<class CRenderer>				m_pRenderer = { nullptr };
	unique_ptr<class CPipeLine>				m_pPipeLine = { nullptr };
	unique_ptr<class CLight_Manager>		m_pLight_Manager = { nullptr };
	unique_ptr<class CFont_Manager>			m_pFont_Manager = { nullptr };
	unique_ptr<class CTarget_Manager>		m_pTarget_Manager = { nullptr };
	unique_ptr<class CPicking>				m_pPicking = { nullptr };
	unique_ptr<class CShadow>				m_pShadow = { nullptr };
	unique_ptr<class CFrustum>				m_pFrustum = { nullptr };
	unique_ptr<class CProfiler>				m_pProfiler = { nullptr };

private:
	float2_t			m_vViewportDesc = {};



public:
	void Release_Engine();

};

NS_END
