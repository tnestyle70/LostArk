# Bern 이동 시 카메라·프러스텀 프레임 순서 디테일 코드 계획서

## 문서 목적

이 문서는 [구현 계획서](./2026-08-09_BERN_CAMERA_FRUSTUM_FRAME_ORDER_IMPLEMENTATION_PLAN.md)의 변경을 코드 작성 전에 고정한다. 기존 파일은 변경 후 전체 코드를, 신규 하네스는 추가할 전체 코드를 기록한다. 실제 검증 결과와 완료 상태는 대응 RESULT 문서에만 기록한다.

## G00. authoritative Level update를 render submission 이전으로 이동

### 목표와 근거

현재 `CGameInstance::Update_Engine`은 모든 GameObject의 `Late_Update`가 끝난 뒤 `CLevel_Bern::Update`를 호출한다. Bern의 Server snapshot 반영과 follow target binding이 카메라 `Late_Update`, 프러스텀 갱신, 정적 맵 visible-list 작성 뒤에 실행되므로 이동·최초 bind 프레임에는 서로 다른 시점의 상태가 한 화면에 섞인다.

호출 순서를 `Post_Physics_Update -> Level_Update -> Object Late_Update`로 바꾼다. 따라서 같은 프레임 안에서 다음 쓰기 순서를 만든다.

```text
Server snapshot → local character Transform
→ Bern follow target binding
→ CCamera_Free::Late_Update
→ CCamera::Update_PipeLine
→ Refresh_CameraState / CFrustum::Transform_ToWorldSpace
→ map frustum culling / render-group submission
```

### 대상 파일

- `Engine/Private/GameInstance.cpp`

### H/공개 계약 변화

없다. 새 타입, struct, public 함수, 멤버 변수, include를 추가하지 않는다. `CMainApp::Apply_LevelRequest`와 `Change_Level`의 프레임 끝 전환 계약도 변경하지 않는다.

### 변경 후 전체 코드

```cpp
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

CGameInstance::CGameInstance()
{
}

CGameInstance::~CGameInstance()
{
}

HRESULT CGameInstance::Initialize_Engine(const ENGINE_DESC& EngineDesc, ComPtr<ID3D11Device>& pOutDevice, ComPtr<ID3D11DeviceContext>& pOutContext)
{
	//viewport 사이즈 설정
	m_vViewportDesc = float2_t(EngineDesc.iWinSizeX, EngineDesc.iWinSizeY);

	m_pGraphic_Device = CGraphic_Device::Create(EngineDesc.hWnd, EngineDesc.eWinMode, EngineDesc.iWinSizeX, EngineDesc.iWinSizeY, pOutDevice, pOutContext);
	if (nullptr == m_pGraphic_Device)
		return E_FAIL;

	m_pInput_Device = CInput_Device::Create(EngineDesc.hInstance, EngineDesc.hWnd);
	if (nullptr == m_pInput_Device)
		return E_FAIL;

#ifdef _WIN64
	m_pSound_Manager = CSound_Manager::Create();
	if (nullptr == m_pSound_Manager)
		return E_FAIL;
#endif

	m_pTarget_Manager = CTarget_Manager::Create(pOutDevice, pOutContext);
	if (nullptr == m_pTarget_Manager)
		return E_FAIL;

	m_pRenderer = CRenderer::Create(pOutDevice, pOutContext);
	if (nullptr == m_pRenderer)
		return E_FAIL;

	m_pTimer_Manager = CTimer_Manager::Create();
	if (nullptr == m_pTimer_Manager)
		return E_FAIL;

	m_pPrototype_Manager = CPrototype_Manager::Create(EngineDesc.iNumLevels);
	if (nullptr == m_pPrototype_Manager)
		return E_FAIL;

	m_pObject_Manager = CObject_Manager::Create(EngineDesc.iNumLevels);
	if (nullptr == m_pObject_Manager)
		return E_FAIL;

	m_pLevel_Manager = CLevel_Manager::Create();
	if (nullptr == m_pLevel_Manager)
		return E_FAIL;

	m_pPipeLine = CPipeLine::Create();
	if (nullptr == m_pPipeLine)
		return E_FAIL;

	m_pLight_Manager = CLight_Manager::Create();
	if (nullptr == m_pLight_Manager)
		return E_FAIL;

	m_pFont_Manager = CFont_Manager::Create(pOutDevice, pOutContext);
	if (nullptr == m_pFont_Manager)
		return E_FAIL;

	m_pPicking = CPicking::Create(pOutDevice, pOutContext, EngineDesc.hWnd);
	if (nullptr == m_pPicking)
		return E_FAIL;

	m_pShadow = CShadow::Create();
	if (nullptr == m_pShadow)
		return E_FAIL;

	m_pFrustum = CFrustum::Create();
	if (nullptr == m_pFrustum)
		return E_FAIL;

	m_pPhysics_Manager = CPhysics_Manager::Create(EngineDesc.iNumLevels);
	if (nullptr == m_pPhysics_Manager)
		return E_FAIL;

	m_pProfiler = std::make_unique<CProfiler>();
	if (FAILED(m_pProfiler->Initialize(pOutDevice, pOutContext)))
		return E_FAIL;

	return S_OK;
}

void CGameInstance::Update_Engine(f32_t fTimeDelta)
{
	m_pPicking->Update();

	m_pInput_Device->Update();

#ifdef _WIN64
	m_pSound_Manager->Update();
#endif

	m_pObject_Manager->Priority_Update(fTimeDelta);

	Refresh_CameraState();

	m_pObject_Manager->Update(fTimeDelta);
	m_pPhysics_Manager->Update(fTimeDelta);
	m_pObject_Manager->Post_Physics_Update(fTimeDelta);

	/* Level gameplay can commit authoritative transforms and bind the
	follow camera. Run it before render submission and frustum culling. */
	m_pLevel_Manager->Update(fTimeDelta);
	m_pObject_Manager->Late_Update(fTimeDelta);
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
	if (FAILED(m_pRenderer->Draw()))
		return E_FAIL;

	if (FAILED(m_pLevel_Manager->Render()))
		return E_FAIL;

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
	m_pProfiler.reset();
	m_pFrustum.reset();
	m_pTarget_Manager.reset();
	m_pFont_Manager.reset();
	m_pLight_Manager.reset();
	m_pPipeLine.reset();
	m_pRenderer.reset();
	/* 현재 Loading Level이 보유한 Loader 스레드를 먼저 종료시킨다. */
	m_pLevel_Manager.reset();
	m_pObject_Manager.reset();
	m_pPhysics_Manager.reset();
	m_pPrototype_Manager.reset();
	m_pTimer_Manager.reset();
#ifdef _WIN64
	m_pSound_Manager.reset();
#endif
	m_pInput_Device.reset();
	m_pGraphic_Device->Shutdown();
	m_pGraphic_Device.reset();
}

```

## G01. 프레임 순서 회귀 하네스 추가

### 목표와 근거

호출 순서 한 줄이 되돌아가도 컴파일은 성공하므로 전용 정적 계약 하네스로 다음 연결을 함께 잠근다.

1. Engine 순서가 `Post_Physics_Update -> Level_Update -> Late_Update`인지 확인한다.
2. Bern은 replication을 먼저 적용하고 camera binding을 수행하는지 확인한다.
3. follow camera가 pipeline을 갱신하고 base camera가 frustum을 refresh하는지 확인한다.
4. 정적 batch와 fallback map object가 해당 frustum으로 visible list를 작성하는지 확인한다.
5. ordered layer tag에서 camera가 map layer보다 먼저 `Late_Update`되는지 확인한다.

### 대상 파일

- 신규 `Tools/ProjectAudit/Test-CameraFrustumFrameOrder.ps1`

### H/공개 계약 변화

없다. 소스와 layer contract를 읽기 전용으로 검사하며 runtime data나 build output을 쓰지 않는다.

### 신규 파일 전체 코드

```powershell
[CmdletBinding()]
param()

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot '../..')).Path

function Read-RepositorySource {
    param(
        [Parameter(Mandatory = $true)]
        [string]$RelativePath
    )

    $path = Join-Path $repoRoot $RelativePath
    if (-not (Test-Path -LiteralPath $path)) {
        throw "[CameraFrustumFrameOrder] Missing source file: $RelativePath"
    }

    return Get-Content -LiteralPath $path -Raw
}

function Assert-SourceContains {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Source,
        [Parameter(Mandatory = $true)]
        [string]$Expected,
        [Parameter(Mandatory = $true)]
        [string]$FailureMessage
    )

    if (-not $Source.Contains($Expected)) {
        throw "[CameraFrustumFrameOrder] $FailureMessage"
    }
}

$gameInstance = Read-RepositorySource 'Engine/Private/GameInstance.cpp'
$cameraFree = Read-RepositorySource 'Client/Private/Camera_Free.cpp'
$camera = Read-RepositorySource 'Engine/Private/Camera.cpp'
$levelBern = Read-RepositorySource 'Client/Private/Level_Bern.cpp'
$mapStaticBatch = Read-RepositorySource 'Client/Private/MapStaticBatchObject.cpp'
$mapAsset = Read-RepositorySource 'Client/Private/MapAssetObject.cpp'
$objectManagerHeader = Read-RepositorySource 'Engine/Public/Object_Manager.h'
$objectManager = Read-RepositorySource 'Engine/Private/Object_Manager.cpp'
$mapPlacementRuntime = Read-RepositorySource 'Client/Private/MapPlacementRuntime.cpp'

$updateEngineMatch = [regex]::Match(
    $gameInstance,
    '(?s)void\s+CGameInstance::Update_Engine\s*\([^)]*\)\s*\{(?<body>.*?)\r?\n\}\r?\n\r?\nvoid\s+CGameInstance::Refresh_CameraState')
if (-not $updateEngineMatch.Success) {
    throw '[CameraFrustumFrameOrder] Could not isolate CGameInstance::Update_Engine.'
}

$updateBody = $updateEngineMatch.Groups['body'].Value
$postPhysicsIndex = $updateBody.IndexOf(
    'm_pObject_Manager->Post_Physics_Update(fTimeDelta);',
    [System.StringComparison]::Ordinal)
$levelIndex = $updateBody.IndexOf(
    'm_pLevel_Manager->Update(fTimeDelta);',
    [System.StringComparison]::Ordinal)
$lateIndex = $updateBody.IndexOf(
    'm_pObject_Manager->Late_Update(fTimeDelta);',
    [System.StringComparison]::Ordinal)

if ($postPhysicsIndex -lt 0 -or
    $levelIndex -lt 0 -or
    $lateIndex -lt 0 -or
    $postPhysicsIndex -ge $levelIndex -or
    $levelIndex -ge $lateIndex) {
    throw (
        '[CameraFrustumFrameOrder] Required frame order is ' +
        'Post_Physics_Update -> Level_Update -> Late_Update.')
}

$bernUpdateMatch = [regex]::Match(
    $levelBern,
    '(?s)void\s+CLevel_Bern::Update\s*\([^)]*\)\s*\{(?<body>.*?)\r?\n\}')
if (-not $bernUpdateMatch.Success) {
    throw '[CameraFrustumFrameOrder] Could not isolate CLevel_Bern::Update.'
}

$bernUpdateBody = $bernUpdateMatch.Groups['body'].Value
$replicationIndex = $bernUpdateBody.IndexOf(
    'm_Replication.Update()',
    [System.StringComparison]::Ordinal)
$bindIndex = $bernUpdateBody.IndexOf(
    'Bind_CameraToLocalCharacter()',
    [System.StringComparison]::Ordinal)
if ($replicationIndex -lt 0 -or
    $bindIndex -lt 0 -or
    $replicationIndex -ge $bindIndex) {
    throw (
        '[CameraFrustumFrameOrder] Bern must commit replication before ' +
        'binding the follow camera.')
}

$cameraLateUpdateMatch = [regex]::Match(
    $cameraFree,
    '(?s)void\s+CCamera_Free::Late_Update\s*\([^)]*\)\s*\{(?<body>.*?)\r?\n\}')
if (-not $cameraLateUpdateMatch.Success) {
    throw '[CameraFrustumFrameOrder] Could not isolate CCamera_Free::Late_Update.'
}

$cameraLateUpdateBody = $cameraLateUpdateMatch.Groups['body'].Value
$followIndex = $cameraLateUpdateBody.IndexOf(
    'Update_FollowCamera(fTimeDelta);',
    [System.StringComparison]::Ordinal)
$pipelineIndex = $cameraLateUpdateBody.IndexOf(
    '__super::Update_PipeLine();',
    [System.StringComparison]::Ordinal)
if ($followIndex -lt 0 -or
    $pipelineIndex -lt 0 -or
    $followIndex -ge $pipelineIndex) {
    throw (
        '[CameraFrustumFrameOrder] Follow camera must update before ' +
        'publishing the view pipeline.')
}

Assert-SourceContains $camera `
    'CGameInstance::Get().Refresh_CameraState();' `
    'CCamera::Update_PipeLine must refresh inverse matrices and the frustum.'
Assert-SourceContains $mapStaticBatch `
    'const HRESULT hVisibleResult = Upload_VisibleInstances();' `
    'Map static batches must build their visible instance list in Late_Update.'
Assert-SourceContains $mapStaticBatch `
    'isIn_Frustum_InWorldSpace' `
    'Map static batch visibility must consume the engine frustum.'
Assert-SourceContains $mapAsset `
    'isIn_Frustum_InWorldSpace' `
    'Map fallback objects must consume the engine frustum.'
Assert-SourceContains $objectManagerHeader `
    'map<const wstring_t, shared_ptr<CLayer>>' `
    'Object layers must retain their ordered map contract.'
Assert-SourceContains $objectManager `
    'Pair.second->Late_Update(fTimeDelta);' `
    'Object manager must dispatch layer Late_Update.'
Assert-SourceContains $mapPlacementRuntime `
    'TEXT("Layer_MapStaticBatch")' `
    'Static batch layer tag changed unexpectedly.'
Assert-SourceContains $mapPlacementRuntime `
    'L"Layer_MapAsset_"' `
    'Fallback map asset layer prefix changed unexpectedly.'

if ([string]::CompareOrdinal('Layer_Camera', 'Layer_MapStaticBatch') -ge 0 -or
    [string]::CompareOrdinal('Layer_Camera', 'Layer_MapAsset_') -ge 0) {
    throw (
        '[CameraFrustumFrameOrder] Ordered layer tags no longer place the ' +
        'camera before map culling.')
}

Write-Host (
    '[CameraFrustumFrameOrder] PASS: replication and camera binding run ' +
    'before camera pipeline refresh, map culling, and render submission.')

```

## G02. 구현 후 검증 계획

1. `Test-CameraFrustumFrameOrder.ps1`를 실행해 호출 그래프 계약을 검증한다.
2. `git diff --check`로 변경 줄의 공백 오류를 확인한다.
3. `Invoke-ProjectAudit.ps1`를 실행해 기존 전체 계약과 함께 회귀를 확인한다.
4. Engine부터 Client까지 정본 build/regression을 실행한다. 실행 중인 다른 세션의 Client/Server 프로세스는 종료하지 않으며, 잠금이 있으면 충돌하지 않는 구성부터 검증하고 잠금 해제 뒤 나머지를 실행한다.
5. Bern Server+Client에서 정지, 연속 이동, 최초 follow bind를 확인하고, 이동 중 정상 도로가 대형 배치 단위로 사라지지 않는지 수동 smoke evidence를 남긴다.
