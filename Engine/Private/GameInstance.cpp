#include "GameInstance.h"

#include "Shadow.h"
#include "Picking.h"
#include "Frustum.h"
#include "Renderer.h"
#include "PipeLine.h"
#include "Font_Manager.h"
#include "Input_Device.h"
#ifdef _WIN64
#include "Sound/Sound_Manager.h"
#endif
#include "Light_Manager.h"
#include "Timer_Manager.h"
#include "Level_Manager.h"
#include "Target_Manager.h"
#include "Object_Manager.h"
#include "Graphic_Device.h"
#include "Prototype_Manager.h"
#include "Profiler.h"
#include "Physics_Manager.h"

#include <atomic>

namespace
{
	std::atomic_bool g_isNonInteractiveErrorMode = false;
}

void Engine::Set_NonInteractiveErrorMode(const bool isEnabled)
{
	g_isNonInteractiveErrorMode.store(isEnabled, std::memory_order_release);
}

bool Engine::Is_NonInteractiveErrorMode()
{
	return g_isNonInteractiveErrorMode.load(std::memory_order_acquire);
}

int Engine::Show_EngineMessage(const wchar_t* pMessage)
{
	const wchar_t* pResolvedMessage = nullptr != pMessage ? pMessage : L"Engine error";
	if (Is_NonInteractiveErrorMode())
	{
		OutputDebugStringW(pResolvedMessage);
		OutputDebugStringW(L"\n");
		return IDOK;
	}

	return MessageBoxW(nullptr, pResolvedMessage, L"System Message", MB_OK);
}

CGameInstance::CGameInstance()
{
}

CGameInstance::~CGameInstance()
{
}

HRESULT CGameInstance::Initialize_Engine(const ENGINE_DESC& EngineDesc, ComPtr<ID3D11Device>& pOutDevice, ComPtr<ID3D11DeviceContext>& pOutContext)
{
	Set_NonInteractiveErrorMode(EngineDesc.bNonInteractiveErrors);
	pOutDevice.Reset();
	pOutContext.Reset();
	Release_Engine();
	const auto FailInitialization = [this, &pOutDevice, &pOutContext](
		const HRESULT hResult) -> HRESULT
	{
		Release_Engine();
		pOutDevice.Reset();
		pOutContext.Reset();
		return FAILED(hResult) ? hResult : E_FAIL;
	};

	//viewport ������ ����
	m_vViewportDesc = float2_t(EngineDesc.iWinSizeX, EngineDesc.iWinSizeY);

	m_pGraphic_Device = CGraphic_Device::Create(EngineDesc.hWnd, EngineDesc.eWinMode,
		EngineDesc.eDriverType, EngineDesc.iWinSizeX, EngineDesc.iWinSizeY,
		pOutDevice, pOutContext);
	if (nullptr == m_pGraphic_Device)
		return FailInitialization(E_FAIL);

	m_pInput_Device = CInput_Device::Create(EngineDesc.hInstance, EngineDesc.hWnd);
	if (nullptr == m_pInput_Device)
		return FailInitialization(E_FAIL);

#ifdef _WIN64
	m_pSound_Manager = CSound_Manager::Create();
	if (nullptr == m_pSound_Manager)
		return FailInitialization(E_FAIL);
#endif

	m_pTarget_Manager = CTarget_Manager::Create(pOutDevice, pOutContext);
	if (nullptr == m_pTarget_Manager)
		return FailInitialization(E_FAIL);

	m_pRenderer = CRenderer::Create(pOutDevice, pOutContext);
	if (nullptr == m_pRenderer)
		return FailInitialization(E_FAIL);

	m_pTimer_Manager = CTimer_Manager::Create();
	if (nullptr == m_pTimer_Manager)
		return FailInitialization(E_FAIL);

	m_pPrototype_Manager = CPrototype_Manager::Create(EngineDesc.iNumLevels);
	if (nullptr == m_pPrototype_Manager)
		return FailInitialization(E_FAIL);

	m_pObject_Manager = CObject_Manager::Create(EngineDesc.iNumLevels);
	if (nullptr == m_pObject_Manager)
		return FailInitialization(E_FAIL);

	m_pLevel_Manager = CLevel_Manager::Create();
	if (nullptr == m_pLevel_Manager)
		return FailInitialization(E_FAIL);

	m_pPipeLine = CPipeLine::Create();
	if (nullptr == m_pPipeLine)
		return FailInitialization(E_FAIL);

	m_pLight_Manager = CLight_Manager::Create();
	if (nullptr == m_pLight_Manager)
		return FailInitialization(E_FAIL);

	m_pFont_Manager = CFont_Manager::Create(pOutDevice, pOutContext);
	if (nullptr == m_pFont_Manager)
		return FailInitialization(E_FAIL);

	m_pPicking = CPicking::Create(pOutDevice, pOutContext, EngineDesc.hWnd);
	if (nullptr == m_pPicking)
		return FailInitialization(E_FAIL);

	m_pShadow = CShadow::Create();
	if (nullptr == m_pShadow)
		return FailInitialization(E_FAIL);

	m_pFrustum = CFrustum::Create();
	if (nullptr == m_pFrustum)
		return FailInitialization(E_FAIL);

	m_pPhysics_Manager = CPhysics_Manager::Create(EngineDesc.iNumLevels);
	if (nullptr == m_pPhysics_Manager)
		return FailInitialization(E_FAIL);

	m_pProfiler = std::make_unique<CProfiler>();
	const HRESULT hProfilerResult =
		m_pProfiler->Initialize(pOutDevice, pOutContext);
	if (FAILED(hProfilerResult))
		return FailInitialization(hProfilerResult);

	return S_OK;
}

void CGameInstance::Update_Engine(f32_t fTimeDelta)
{
	/* The deferred fog drifts on a renderer owned clock, advanced once per
	   frame so no screen pass has to be handed a time value. */
	m_pRenderer->Advance_PresentationClock(fTimeDelta);

	m_pPicking->Update();

	m_pInput_Device->Update();

#ifdef _WIN64
	m_pSound_Manager->Update();
#endif

	CProfiler* const pProfiler = m_pProfiler.get();
	{
		CProfilerScope scope(pProfiler, "Engine.PriorityUpdate");
		m_pObject_Manager->Priority_Update(fTimeDelta);
	}

	Refresh_CameraState();

	{
		CProfilerScope scope(pProfiler, "Engine.ObjectUpdate");
		m_pObject_Manager->Update(fTimeDelta);
	}
	{
		CProfilerScope scope(pProfiler, "Engine.Physics");
		m_pPhysics_Manager->Update(fTimeDelta);
	}
	{
		CProfilerScope scope(pProfiler, "Engine.PostPhysicsUpdate");
		m_pObject_Manager->Post_Physics_Update(fTimeDelta);
	}

	/* Level gameplay can commit authoritative transforms and bind the
	follow camera. Run it before render submission and frustum culling. */
	{
		CProfilerScope scope(pProfiler, "Engine.LevelUpdate");
		m_pLevel_Manager->Update(fTimeDelta);
	}
	{
		CProfilerScope scope(pProfiler, "Engine.LateUpdate");
		m_pObject_Manager->Late_Update(fTimeDelta);
	}
}

void CGameInstance::Refresh_CameraState()
{
	m_pPipeLine->Update();
	m_pFrustum->Update_InWorldSpace();
}

HRESULT CGameInstance::Render_Begin(const float4_t* pClearColor)
{
	if (FAILED(m_pGraphic_Device->Clear_BackBuffer_View(pClearColor)))
		return E_FAIL;

	if (FAILED(m_pGraphic_Device->Clear_DepthStencil_View()))
		return E_FAIL;

	return S_OK;
}

HRESULT CGameInstance::Render()
{
	const HRESULT hDrawResult = m_pRenderer->Draw();
	if (FAILED(hDrawResult))
		return hDrawResult;

	const HRESULT hLevelResult = m_pLevel_Manager->Render();
	if (FAILED(hLevelResult))
		return hLevelResult;

	return S_OK;
}

HRESULT CGameInstance::Render_End()
{
	return m_pGraphic_Device->Present();
}

HRESULT CGameInstance::Clear_Resources(uint32_t iClearLevelID)
{
	if (FAILED(m_pObject_Manager->Clear(iClearLevelID)))
		return E_FAIL;

	if (FAILED(m_pPrototype_Manager->Clear(iClearLevelID)))
		return E_FAIL;

	if (FAILED(m_pPhysics_Manager->Clear(iClearLevelID)))
		return E_FAIL;

	return S_OK;
	
}

int8_t CGameInstance::Get_DIKeyState(uint8_t byKeyID)
{
	return m_pInput_Device->Get_DIKeyState(byKeyID);
}

bool_t CGameInstance::Get_DIKeyPressed(uint8_t byKeyID)
{
	return m_pInput_Device->Get_DIKeyPressed(byKeyID);
}

int8_t CGameInstance::Get_DIKeyStateRaw(uint8_t byKeyID) const
{
	return nullptr != m_pInput_Device ?
		m_pInput_Device->Get_DIKeyStateRaw(byKeyID) : 0;
}

bool_t CGameInstance::Get_DIKeyPressedRaw(uint8_t byKeyID) const
{
	return nullptr != m_pInput_Device &&
		m_pInput_Device->Get_DIKeyPressedRaw(byKeyID);
}

int8_t CGameInstance::Get_DIMouseState(DIM eMouse)
{
	return m_pInput_Device->Get_DIMouseState(eMouse);
}

int8_t CGameInstance::Get_DIMouseStateRaw(DIM eMouse) const
{
	return nullptr != m_pInput_Device ?
		m_pInput_Device->Get_DIMouseStateRaw(eMouse) : 0;
}

int32_t CGameInstance::Get_DIMouseMove(DIMM eMouseState)
{
	return m_pInput_Device->Get_DIMouseMove(eMouseState);
}

void CGameInstance::SetInputBlocked(bool_t bKeyboardBlocked, bool_t bMouseBlocked)
{
	if (nullptr != m_pInput_Device)
		m_pInput_Device->SetInputBlocked(bKeyboardBlocked, bMouseBlocked);
}

void CGameInstance::SetMouseButtonBlocked(DIM eMouse, bool_t blocked)
{
	if (nullptr != m_pInput_Device)
		m_pInput_Device->SetMouseButtonBlocked(eMouse, blocked);
}

bool_t CGameInstance::IsKeyboardInputBlocked() const
{
	return nullptr != m_pInput_Device && m_pInput_Device->IsKeyboardInputBlocked();
}

bool_t CGameInstance::IsMouseInputBlocked() const
{
	return nullptr != m_pInput_Device && m_pInput_Device->IsMouseInputBlocked();
}

#ifdef _WIN64
HRESULT CGameInstance::Play_Sound(const wstring_t& strSoundFilePath, f32_t fVolume)
{
	return m_pSound_Manager->Play_Sound(strSoundFilePath, fVolume);
}

HRESULT CGameInstance::Play_LoopingSound(const wstring_t& strSoundFilePath, f32_t fVolume)
{
	return nullptr != m_pSound_Manager ?
		m_pSound_Manager->Play_LoopingSound(strSoundFilePath, fVolume) : E_FAIL;
}

void CGameInstance::Stop_LoopingSound()
{
	if (nullptr != m_pSound_Manager)
		m_pSound_Manager->Stop_LoopingSound();
}

HRESULT CGameInstance::Play_Music(const wstring_t& strSoundFilePath,
	f32_t fVolume, const bool_t bLoop)
{
	return m_pSound_Manager->Play_Music(strSoundFilePath, fVolume, bLoop);
}

void CGameInstance::Stop_Music()
{
	m_pSound_Manager->Stop_Music();
}
#endif

f32_t CGameInstance::Get_TimeDelta(const wstring_t& strTimerTag)
{
	return m_pTimer_Manager->Get_TimeDelta(strTimerTag);
}

HRESULT CGameInstance::Add_Timer(const wstring_t& strTimerTag)
{
	return m_pTimer_Manager->Add_Timer(strTimerTag);
}

void CGameInstance::Update_TimeDelta(const wstring_t& strTimerTag)
{
	m_pTimer_Manager->Update_TimeDelta(strTimerTag);
}

HRESULT CGameInstance::Change_Level(uint32_t iCurrentLevelID, unique_ptr<class CLevel> pNewLevel)
{
	return m_pLevel_Manager->Change_Level(iCurrentLevelID, move(pNewLevel));
}

uint32_t CGameInstance::Get_CurrentLevelID() const
{
	return nullptr == m_pLevel_Manager ? 0u : m_pLevel_Manager->Get_CurrentLevelID();
}

HRESULT CGameInstance::Add_Prototype(uint32_t iLevelIndex, const wstring_t& strPrototypeTag, unique_ptr<class CPrototype> pPrototype)
{
	return m_pPrototype_Manager->Add_Prototype(iLevelIndex, strPrototypeTag, move(pPrototype));
}

HRESULT CGameInstance::Add_Prototypes(
	const uint32_t iLevelIndex,
	vector<pair<wstring_t, unique_ptr<CPrototype>>>&& prototypes)
{
	return m_pPrototype_Manager->Add_Prototypes(
		iLevelIndex,
		std::move(prototypes));
}

shared_ptr<CPrototype> CGameInstance::Clone_Prototype(uint32_t iLevelIndex, const wstring_t& strPrototypeTag, void* pArg)
{
	return m_pPrototype_Manager->Clone_Prototype(iLevelIndex, strPrototypeTag, pArg);
}

shared_ptr<CGameObject> CGameInstance::Get_GameObject(
	uint32_t iLevelIndex,
	const wstring_t& strLayerTag,
	uint32_t iIndex)
{
	return m_pObject_Manager->Get_GameObject(
		iLevelIndex,
		strLayerTag,
		iIndex);
}

shared_ptr<CComponent> CGameInstance::Get_Component(uint32_t iLevelIndex, const wstring_t& strLayerTag, const wstring_t& strComponentTag, uint32_t iIndex)
{
	return m_pObject_Manager->Get_Component(iLevelIndex, strLayerTag, strComponentTag, iIndex);
}

shared_ptr<CComponent> CGameInstance::Get_Component(uint32_t iLevelIndex, const wstring_t& strLayerTag, const wstring_t& strPartTag, const wstring_t& strComponentTag, uint32_t iIndex)
{
	return m_pObject_Manager->Get_Component(iLevelIndex, strLayerTag, strPartTag, strComponentTag, iIndex);
}

HRESULT CGameInstance::Add_GameObject_to_Layer(uint32_t iPrototypeLevelIndex, const wstring_t& strPrototypeTag, uint32_t iLayerLevelIndex, const wstring_t& strLayerTag, void* pArg, shared_ptr<CGameObject>* pOutGameObject)
{
	return m_pObject_Manager->Add_GameObject_to_Layer(iPrototypeLevelIndex, strPrototypeTag, iLayerLevelIndex, strLayerTag, pArg, pOutGameObject);
}

HRESULT CGameInstance::Remove_GameObject_from_Layer(uint32_t iLevelIndex, const wstring_t& strLayerTag, const shared_ptr<CGameObject>& pGameObject)
{
	return m_pObject_Manager->Remove_GameObject_from_Layer(iLevelIndex, strLayerTag, pGameObject);
}

HRESULT CGameInstance::Add_RenderObject(RENDERGROUP eRenderGroupID, shared_ptr<CGameObject> pRenderObject)
{
	return m_pRenderer->Add_RenderObject(eRenderGroupID, pRenderObject);
}

RENDER_QUALITY_SETTINGS CGameInstance::Get_RenderQualitySettings() const
{
	return m_pRenderer->Get_RenderQualitySettings();
}

HRESULT CGameInstance::Apply_RenderQualitySettings(
	const RENDER_QUALITY_SETTINGS& Settings)
{
	return m_pRenderer->Apply_RenderQualitySettings(Settings);
}

HEIGHT_FOG_SETTINGS CGameInstance::Get_HeightFogSettings() const
{
	return m_pRenderer->Get_HeightFogSettings();
}

HRESULT CGameInstance::Apply_HeightFog(
	const HEIGHT_FOG_SETTINGS& Settings)
{
	return m_pRenderer->Apply_HeightFog(Settings);
}

#ifdef _DEBUG
HRESULT CGameInstance::Add_DebugComponent(shared_ptr<CComponent> pDebugComponent)
{
	return m_pRenderer->Add_DebugComponent(pDebugComponent);
}
#endif

void CGameInstance::Set_Transform(D3DTS eType, fmatrix_t TransformMatrix)
{
	m_pPipeLine->Set_Transform(eType, TransformMatrix);
}

const float4_t* CGameInstance::Get_CamPosition()
{
	return m_pPipeLine->Get_CamPosition();
}

const float4x4_t* CGameInstance::Get_Transform(D3DTS eType)
{
	return 	m_pPipeLine->Get_Transform(eType);
}

const float4x4_t* CGameInstance::Get_InverseTransform(D3DTS eType)
{
	return 	m_pPipeLine->Get_InverseTransform(eType);
}

HRESULT CGameInstance::Bind_CamPosition(shared_ptr<class CShader> pShader, const char_t* pConstantName)
{
	return 	m_pPipeLine->Bind_CamPos_ShaderResource(pShader, pConstantName);
}

HRESULT CGameInstance::Bind_Transform(shared_ptr<class CShader> pShader, const char_t* pConstantName, D3DTS eType)
{
	return 	m_pPipeLine->Bind_ShaderResource(pShader, pConstantName, eType);
}

HRESULT CGameInstance::Bind_InverseTransform(shared_ptr<class CShader> pShader, const char_t* pConstantName, D3DTS eType)
{
	return 	m_pPipeLine->Bind_Inverse_ShaderResource(pShader, pConstantName, eType);
}

HRESULT CGameInstance::Add_Light(const LIGHT_DESC& LightDesc)
{
	return m_pLight_Manager->Add_Light(LightDesc);
}

HRESULT CGameInstance::Render_Lights(
	shared_ptr<class CShader> pShader,
	shared_ptr<class CVIBuffer_Rect> pVIBuffer,
	bool_t bEnableSceneDirectionalShadow)
{
	return m_pLight_Manager->Render_Lights(
		pShader, pVIBuffer, bEnableSceneDirectionalShadow);
}

HRESULT CGameInstance::Add_Font(const wstring& strFontTag, const tchar_t* pFontFilePath)
{
	return m_pFont_Manager->Add_Font(strFontTag, pFontFilePath);	
}

void CGameInstance::Draw_Text(const wstring& strFontTag, const tchar_t* pText, const float2_t& vPosition, fvector_t vColor, f32_t fRotation, const float2_t& vOrigin, f32_t fScale)
{
	return m_pFont_Manager->Draw(strFontTag, pText, vPosition, vColor, fRotation, vOrigin, fScale);
}

float2_t CGameInstance::Measure_Text(const wstring& strFontTag, const tchar_t* pText)
{
	return m_pFont_Manager->Measure(strFontTag, pText);
}

void CGameInstance::Set_TextClipOutRect(f32_t fX, f32_t fY, f32_t fWidth, f32_t fHeight)
{
	m_pFont_Manager->Set_ClipOutRect(fX, fY, fWidth, fHeight);
}

void CGameInstance::Clear_TextClipOutRect()
{
	m_pFont_Manager->Clear_ClipOutRect();
}

HRESULT CGameInstance::Add_RenderTarget(const wstring_t& strTargetTag, uint32_t iWidth, uint32_t iHeight, DXGI_FORMAT ePixelFormat, const float4_t& vClearColor)
{
	return m_pTarget_Manager->Add_RenderTarget(strTargetTag, iWidth, iHeight, ePixelFormat, vClearColor);	
}

HRESULT CGameInstance::Add_MRT(const wstring_t& strMRTTag, const wstring_t& strTargetTag)
{
	return m_pTarget_Manager->Add_MRT(strMRTTag, strTargetTag);
}

HRESULT CGameInstance::Begin_MRT(const wstring_t& strMRTTag, ComPtr<ID3D11DepthStencilView> pDSV)
{
	return m_pTarget_Manager->Begin_MRT(strMRTTag, pDSV);
}

HRESULT CGameInstance::End_MRT()
{
	return m_pTarget_Manager->End_MRT();
}

HRESULT CGameInstance::Begin_DepthOnly(
	ComPtr<ID3D11DepthStencilView> pDSV)
{
	return m_pTarget_Manager->Begin_DepthOnly(pDSV);
}

HRESULT CGameInstance::End_DepthOnly()
{
	return m_pTarget_Manager->End_DepthOnly();
}

HRESULT CGameInstance::Bind_RT_SRV(const wstring_t& strTargetTag, shared_ptr<class CShader> pShader, const char_t* pConstantName)
{
	return m_pTarget_Manager->Bind_SRV(strTargetTag, pShader, pConstantName);
}

ComPtr<ID3D11ShaderResourceView> CGameInstance::Get_RT_SRV(
	const wstring_t& strTargetTag) const
{
	return m_pTarget_Manager->Get_SRV(strTargetTag);
}

HRESULT CGameInstance::Copy_RT_Resource(const wstring_t& strTargetTag, ComPtr<ID3D11Texture2D> pTexture2D)
{
	return m_pTarget_Manager->Copy_Resource(strTargetTag, pTexture2D);
}

#ifdef _DEBUG

HRESULT CGameInstance::Ready_RT_DebugDesc(const wstring_t& strTargetTag, f32_t fX, f32_t fY, f32_t fSizeX, f32_t fSizeY)
{
	return m_pTarget_Manager->Ready_DebugDesc(strTargetTag, fX, fY, fSizeX, fSizeY);	
}

HRESULT CGameInstance::Render_MRT(const wstring_t& strMRTTag, shared_ptr<class CShader> pShader, shared_ptr<class CVIBuffer_Rect> pVIBuffer)
{
	return m_pTarget_Manager->Render_MRT(strMRTTag, pShader, pVIBuffer);
}

#endif

bool_t CGameInstance::Picking(float4_t& vOut)
{
	return m_pPicking->Picking(vOut);	
}

HRESULT CGameInstance::Apply_Shadow_Light(
	const SHADOW_LIGHT_DESC& ShadowLightDesc)
{
	return m_pShadow->Apply_Shadow_Light(ShadowLightDesc);
}

HRESULT CGameInstance::Add_Shadow_Light(
	const SHADOW_LIGHT_DESC& ShadowLightDesc)
{
	return Apply_Shadow_Light(ShadowLightDesc);
}

bool_t CGameInstance::Is_ShadowLightEnabled() const
{
	return m_pShadow->Is_Enabled();
}

const SHADOW_LIGHT_DESC& CGameInstance::Get_ShadowLightDesc() const
{
	return m_pShadow->Get_Desc();
}

HRESULT CGameInstance::Bind_ShadowLight_ShaderResource(shared_ptr<class CShader> pShader, const char_t* pConstantName, D3DTS eType)
{
	return m_pShadow->Bind_ShaderResource(pShader, pConstantName, eType);
}

HRESULT CGameInstance::Bind_ShadowLight_LightingResources(
	shared_ptr<class CShader> pShader)
{
	return m_pShadow->Bind_LightingShaderResources(pShader);
}

void CGameInstance::Update_Frustum_InLocalSpace(fmatrix_t WorldMatrix)
{
	m_pFrustum->Update_InLocalSpace(WorldMatrix);
}

bool_t CGameInstance::isIn_Frustum_InLocalSpace(fvector_t vLocalPoint, f32_t fRange)
{
	return m_pFrustum->isIn_Frustum_InLocalSpace(vLocalPoint, fRange);
}

bool_t CGameInstance::isIn_Frustum_InWorldSpace(fvector_t vWorldPoint, f32_t fRange)
{
	return m_pFrustum->isIn_Frustum_InWorldSpace(vWorldPoint, fRange);
}

void CGameInstance::Release_Engine()
{
	/* A level can restore a presentation camera from its destructor. Keep the
	   pipeline, frustum and renderer alive until that teardown has completed;
	   otherwise CCamera::End_PresentationOverride writes its final view/proj
	   matrices through an already-destroyed CPipeLine. This also joins a
	   Loading level's worker before any service it may still reference. */
	m_pLevel_Manager.reset();
	m_pProfiler.reset();
	m_pFrustum.reset();
	m_pShadow.reset();
	m_pPicking.reset();
	m_pTarget_Manager.reset();
	m_pFont_Manager.reset();
	m_pLight_Manager.reset();
	m_pPipeLine.reset();
	m_pRenderer.reset();
	m_pObject_Manager.reset();
	m_pPhysics_Manager.reset();
	m_pPrototype_Manager.reset();
	m_pTimer_Manager.reset();
#ifdef _WIN64
	m_pSound_Manager.reset();
#endif
	m_pInput_Device.reset();
	if (nullptr != m_pGraphic_Device)
		m_pGraphic_Device->Shutdown();
	m_pGraphic_Device.reset();
}
