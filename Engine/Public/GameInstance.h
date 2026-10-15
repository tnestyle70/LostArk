#pragma once

#include "Prototype_Manager.h"
#include <DirectXColors.h>
#include "Engine_RenderFwd.h"
#include "Sound/Sound_Manager.h"

NS_BEGIN(Engine)

// MainApp이 초기화·갱신·종료를 요청하는 엔진 관리 창구다. Level·원형·객체·렌더링 관리자를 소유한다.
// Object_Manager는 Level별 Layer와 복제 객체를, Prototype_Manager는 복제할 원형을 보관한다.
// 종료 때는 Level·로더가 사용하던 서비스보다 먼저 정리되도록 순서를 묶는다.
class ENGINE_DLL CGameInstance final
{
	DECLARE_SINGLETON(CGameInstance)
	friend class CCamera;
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
	HRESULT Initialize_Engine(const ENGINE_DESC& EngineDesc,
		ComPtr<ID3D11Device>& pOutDevice, ComPtr<ID3D11DeviceContext>& pOutContext);
	// 단위는 Client 물리 pixel이다. 프레임 사이 update·render 전에 호출한다.
	HRESULT Resize_Viewport(uint32_t width, uint32_t height);
	HRESULT Set_FullscreenMode(bool fullscreen, uint32_t width, uint32_t height);
	void Update_Engine(f32_t fTimeDelta);
	void Submit_FinalCameraObjects();
	HRESULT Render_Begin(const float4_t* pClearColor);
	// 세션 설정을 바꾸지 않고 이번 프레임의 정적 occlusion 사용만 요청한다.
	HRESULT Render(bool staticOcclusionAllowed = false);
	HRESULT Render_End();
	HRESULT Clear_Resources(uint32_t iClearLevelID);

public: /* For.Graphic_Device */

public: /* For.Input_Device */
	int8_t	Get_DIKeyState(uint8_t byKeyID);
	bool_t Get_DIKeyPressed(uint8_t byKeyID);
	int8_t Get_DIKeyStateRaw(uint8_t byKeyID) const;
	bool_t Get_DIKeyPressedRaw(uint8_t byKeyID) const;
	int8_t	Get_DIMouseState(DIM eMouse);
	int8_t Get_DIMouseStateRaw(DIM eMouse) const;
	int32_t	Get_DIMouseMove(DIMM eMouseState);
	void SetInputBlocked(bool_t bKeyboardBlocked, bool_t bMouseBlocked);
	void SetMouseButtonBlocked(DIM eMouse, bool_t blocked);
	bool_t IsKeyboardInputBlocked() const;
	bool_t IsMouseInputBlocked() const;

#ifdef _WIN64
public: /* For.Sound_Manager */
	HRESULT Play_Sound(const wstring_t& strSoundFilePath, f32_t fVolume);

	HRESULT Play_Sound(const wstring_t& strSoundFilePath, 
		f32_t fVolume, SOUND_CATEGORY eCategory);
	//Client 준비 목록을 기존 Sound Manager cache에 연결한다.
    HRESULT Prepare_Sound(const wstring_t& strSoundFilePath, bool_t bLoop = false);

	uint64_t Play_SoundCue(const wstring_t& path, f32_t volume, uint32_t ageMs = 0u, bool_t paused = false, f32_t playbackRate = 1.f, uint32_t endMs = 0u);
	bool_t Get_SoundDurationMs(const wstring_t& path, uint32_t& durationMs);
	bool_t Is_SoundCueActive(uint64_t handle) const;
	void Pause_SoundCue(uint64_t handle, bool_t paused);
    void Set_SoundCuePlaybackRate(uint64_t handle, f32_t playbackRate);
	void Seek_SoundCue(uint64_t handle, uint32_t ageMs);
    // 외부 시계에 재생 cue를 맞추되 매 프레임 seek하지 않는다. false면 cue가 끝났거나 사용할 수 없다.
    bool_t Synchronize_SoundCue(uint64_t handle, uint32_t ageMs, uint32_t maxDriftMs = 100u);
	void Stop_SoundCue(uint64_t handle);

	HRESULT Play_LoopingSound(const wstring_t& strSoundFilePath, f32_t fVolume);
	void Stop_LoopingSound();
	HRESULT Play_Music(const wstring_t& strSoundFilePath, f32_t fVolume,
		bool_t bLoop = true);
	void Stop_Music();
	/* 시스템 옵션이 조절하는 mixer bus 음량(0~1)이며 이미 재생 중인 sound에도 적용된다. */
	HRESULT Apply_SoundCategoryVolume(SOUND_CATEGORY eCategory, f32_t fVolume);
	f32_t Get_SoundCategoryVolume(SOUND_CATEGORY eCategory) const;
	/* See CSound_Manager::Set_MuteOnFocusLoss (the sound-in-background option, inverted). */
	void Set_SoundMuteOnFocusLoss(bool_t bMute);
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
	HRESULT Add_Prototypes(
		uint32_t iLevelIndex,
		vector<pair<wstring_t, unique_ptr<class CPrototype>>>&& prototypes);
	shared_ptr<CPrototype> Clone_Prototype(uint32_t iLevelIndex, const wstring_t& strPrototypeTag, void* pArg = nullptr);

public: /* For.Object_Manager */
	shared_ptr<class CGameObject> Get_GameObject(uint32_t iLevelIndex, const wstring_t& strLayerTag, uint32_t iIndex);
	shared_ptr<CComponent> Get_Component(uint32_t iLevelIndex, const wstring_t& strLayerTag, const wstring_t& strComponentTag, uint32_t iIndex);
	shared_ptr<CComponent> Get_Component(uint32_t iLevelIndex, const wstring_t& strLayerTag, const wstring_t& strPartTag, const wstring_t& strComponentTag, uint32_t iIndex);
	HRESULT Add_GameObject_to_Layer(uint32_t iPrototypeLevelIndex, const wstring_t& strPrototypeTag, uint32_t iLayerLevelIndex, const wstring_t& strLayerTag, void* pArg = nullptr, shared_ptr<class CGameObject>* pOutGameObject = nullptr);
	HRESULT Remove_GameObject_from_Layer(uint32_t iLevelIndex, const wstring_t& strLayerTag, const shared_ptr<class CGameObject>& pGameObject);

public: /* Renderer */
	HRESULT Add_RenderObject(RENDERGROUP eRenderGroupID, shared_ptr<class CGameObject> pRenderObject);
    void Register_GpuSceneVisibility(const shared_ptr<class CGpuSceneVisibility>& scene);
    void Unregister_GpuSceneVisibility(const class CGpuSceneVisibility* scene);
	/* 다음 Draw에 world와 같은 deferred 경로로 portrait를 렌더하고 pDestination에 resolve 결과를 쓴다. */
	HRESULT Request_Portrait(ComPtr<ID3D11RenderTargetView> pDestination,
		uint32_t iWidth, uint32_t iHeight,
		const float4x4_t& ViewMatrix, const float4x4_t& ProjMatrix,
		function<HRESULT()> DrawSubject,
		function<HRESULT(RENDERGROUP, ID3D11BlendState*)> DrawForward = {});
	void Request_SceneColorSnapshot();
    void Request_SceneEnvironmentReplacement();
    bool_t Is_SceneEnvironmentReplaced() const;
	HRESULT Refresh_SceneColorSnapshot();
	RENDER_QUALITY_SETTINGS Get_RenderQualitySettings() const;
    MAP_VISIBILITY_SETTINGS Get_MapVisibilitySettings() const;
    HRESULT Apply_MapVisibilitySettings(const MAP_VISIBILITY_SETTINGS& settings);
    RENDER_OPTIMIZATION_SETTINGS Get_RenderOptimizationSettings() const;
    HRESULT Apply_RenderOptimizationSettings(const RENDER_OPTIMIZATION_SETTINGS& settings);
	// draw 중 scalar view만 읽어 quality profile·LUT layer 복사를 피한다.
	uint32_t Get_TextureMinMip() const;
	HRESULT Apply_RenderQualitySettings(const RENDER_QUALITY_SETTINGS& Settings);
	MATERIAL_RENDER_SETTINGS Get_MaterialRenderSettings() const;
	HRESULT Apply_MaterialRenderSettings(const MATERIAL_RENDER_SETTINGS& settings);
	HEIGHT_FOG_SETTINGS Get_HeightFogSettings() const;
	HRESULT Apply_HeightFog(const HEIGHT_FOG_SETTINGS& Settings);
	HRESULT Bind_HeightFog(class CShader* shader) const;
	HRESULT Bind_ActiveSceneColorTransform(class CShader* shader) const;
	HRESULT Stage_RenderEnvironment(const wstring_t& cubePath, const float4_t& color,
		const float4_t& rotationIntensity, RENDER_ENVIRONMENT_STATE& outState, bool_t forceReload = false) const;
	void Commit_RenderEnvironment(const RENDER_ENVIRONMENT_STATE& state);
	RENDER_ENVIRONMENT_STATE Get_RenderEnvironment() const;

	HRESULT Add_DebugComponent(shared_ptr<CComponent> pDebugComponent);

public: /* For.PipeLine */
	void Set_Transform(D3DTS eType, fmatrix_t TransformMatrix);
	const float4_t* Get_CamPosition();
	const float4x4_t* Get_Transform(D3DTS eType);
	const float4x4_t* Get_InverseTransform(D3DTS eType);
	HRESULT Bind_CamPosition(shared_ptr<class CShader> pShader, const char_t* pConstantName);
	HRESULT Bind_Transform(shared_ptr<class CShader> pShader, const char_t* pConstantName, D3DTS eType);
	HRESULT Bind_InverseTransform(shared_ptr<class CShader> pShader, const char_t* pConstantName, D3DTS eType);
	/* 설정된 Transform에서 역행렬·카메라 위치를 다시 구한다. 프레임 중 별도 offscreen 카메라를 쓸 때도 호출한다. */
	void Refresh_CameraState();


public: /* For.Light */
	
	HRESULT Add_Light(const LIGHT_DESC& LightDesc);
	const vector<LIGHT_DESC>& Get_SceneLights() const;
	HRESULT Render_Lights(shared_ptr<class CShader> pShader,
		shared_ptr<class CVIBuffer_Rect> pVIBuffer,
		bool_t bEnableSceneDirectionalShadow,
        LIGHT_RECEIVER ePassReceiver = LIGHT_RECEIVER::ALL,
		bool_t bSourceLightMask = false);


public: /* For.Font_Manager */
	HRESULT Add_Font(const wstring& strFontTag, const tchar_t* pFontFilePath);
	void Draw_Text(const wstring& strFontTag, const tchar_t* pText, const float2_t& vPosition, fvector_t vColor = Colors::White, f32_t fRotation = 0.f, const float2_t& vOrigin = float2_t(0.f, 0.f), f32_t fScale = 1.f);
	float2_t Measure_Text(const wstring& strFontTag, const tchar_t* pText);
	/* See CFont_Manager::Set_ClipOutRect: screen rect Draw_Text refuses to draw over. */
	void Set_TextClipOutRect(f32_t fX, f32_t fY, f32_t fWidth, f32_t fHeight);
	/* 기존 font 제외 영역에 rect 하나를 추가한다. */
	void Add_TextClipOutRect(f32_t fX, f32_t fY, f32_t fWidth, f32_t fHeight);
	void Clear_TextClipOutRect();
	/* See CFont_Manager::Set_ClipInRect: screen rect text must stay wholly inside. */
	void Set_TextClipInRect(f32_t fX, f32_t fY, f32_t fWidth, f32_t fHeight);
	void Clear_TextClipInRect();

public: /* For.Target_Manager */
	HRESULT Add_RenderTarget(const wstring_t& strTargetTag, uint32_t iWidth, uint32_t iHeight, DXGI_FORMAT ePixelFormat, const float4_t& vClearColor);
	HRESULT Add_MRT(const wstring_t& strMRTTag, const wstring_t& strTargetTag);
	HRESULT Begin_MRT(const wstring_t& strMRTTag, ComPtr<ID3D11DepthStencilView> pDSV = nullptr);
	HRESULT End_MRT();
	HRESULT Begin_DepthOnly(ComPtr<ID3D11DepthStencilView> pDSV);
	HRESULT End_DepthOnly();
	HRESULT Bind_RT_SRV(const wstring_t& strTargetTag, shared_ptr<class CShader> pShader, const char_t* pConstantName);
	ComPtr<ID3D11ShaderResourceView> Get_RT_SRV(const wstring_t& strTargetTag) const;
	HRESULT Copy_RT_Resource(const wstring_t& strTargetTag, ComPtr<ID3D11Texture2D> pTexture2D);


#ifdef _DEBUG
public:
	HRESULT Ready_RT_DebugDesc(const wstring_t& strTargetTag, f32_t fX, f32_t fY, f32_t fSizeX, f32_t fSizeY);
	HRESULT Render_MRT(const wstring_t& strMRTTag, shared_ptr<class CShader> pShader, shared_ptr<class CVIBuffer_Rect> pVIBuffer);
#endif


public: /* For.Picking */
	bool_t Picking(float4_t& vOut);
	uint64_t Request_Picking();
	HRESULT Poll_Picking(uint64_t requestId, float4_t& vOut);
	void Cancel_Picking(uint64_t requestId);

public: /* For.Shadow */
	HRESULT Apply_Shadow_Light(const SHADOW_LIGHT_DESC& ShadowLightDesc);
	HRESULT Add_Shadow_Light(const SHADOW_LIGHT_DESC& ShadowLightDesc);
	bool_t Is_ShadowLightEnabled() const;
	const SHADOW_LIGHT_DESC& Get_ShadowLightDesc() const;
	const float4x4_t* Get_ShadowLightTransform(D3DTS eType) const;
	HRESULT Bind_ShadowLight_ShaderResource(shared_ptr<class CShader> pShader, const char_t* pConstantName, D3DTS eType);
	HRESULT Bind_ShadowLight_LightingResources(shared_ptr<class CShader> pShader);

public: /* For.Frustum */	
	void Update_Frustum_InLocalSpace(fmatrix_t WorldMatrix);
	bool_t isIn_Frustum_InWorldSpace(fvector_t vWorldPoint, f32_t fRange);
	bool_t isIn_Frustum_InLocalSpace(fvector_t vLocalPoint, f32_t fRange);

public: /* For.Profiler */
	class CProfiler* Get_Profiler() const { return m_pProfiler.get(); }

public: /* For.Physics */
	class CPhysics_Manager* Get_PhysicsManager() const
	{
		return m_pPhysics_Manager.get();
	}

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
	unique_ptr<class CPhysics_Manager>		m_pPhysics_Manager = { nullptr };

private:
	float2_t			m_vViewportDesc = {};

public:
	void Release_Engine();

};

NS_END
