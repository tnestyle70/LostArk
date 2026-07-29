# LostArk MapTool 선택 에셋 ImGui 프리뷰 계획

작성일: 2026-07-29  
상태: IMPLEMENTATION PLAN  
기준 저장소: C:/Users/user/Desktop/LostArk  
선행 규칙: .md/GB/07-29/gotchas.md  
목표 UI: MapTool 상단 Palette/Hierarchy/Inspector 아래의 기존 Last .wmodel decode 영역

## 1. C1~C8 관점

| 관점 | 이번 작업 적용 | 중요도 |
|---|---|---:|
| C1 기준계 | CModel의 로컬 bounds를 중심 원점으로 옮기고 프리뷰 전용 View/Projection으로 자동 프레이밍한다. 월드 배치 Transform은 사용하지 않는다. | ★★★ |
| C2 이동>계산 | 모델 decode와 GPU buffer 생성은 기존 Loader의 CModel Prototype에서 끝낸다. 프리뷰는 선택 시 Clone하고 카메라 행렬만 계산한다. | ★★★ |
| C3 공유는 비싸다 | 프리뷰는 선택 모델 하나만 보유한다. 별도 메시/머티리얼 복사나 128개 썸네일 선생성을 하지 않는다. | ★★ |
| C4 수명은 선언된다 | RenderTarget은 Debug Tool 수명, CModel/CShader Clone은 LEVEL::ASSET_TEST 진입 수명으로 분리하고 레벨 이탈 시 즉시 해제한다. | ★★★ |
| C5 이산화와 오차 | 프리뷰 크기는 64픽셀 단위, 256~2048 범위로 양자화해 ImGui 창 리사이즈 때 매 프레임 GPU 자원을 재생성하지 않는다. | ★★ |
| C6 가지치기 | 프리뷰가 보이지 않거나 선택 모델이 없거나 카메라가 변하지 않았으면 offscreen draw를 생략한다. | ★★ |
| C7 권위와 정합성 | Catalog의 asset ID, label, prototypeTag, model path가 정본이다. 프리뷰 전용 경로나 두 번째 모델 로더를 만들지 않는다. | ★★★ |
| C8 검증이 병목 | 선택/프리뷰/회전/줌/배치 분리, 레벨 왕복, RTV 복구, multi-viewport, textureless fallback을 실행 검증한다. | ★★★ |

이번 작업의 핵심 축은 C1, C2, C4, C7, C8이다.

## 2. 문제 해결 ①~⑤

① 문제·제약: 현재 Palette 선택은 즉시 Placement Armed가 되며, 실제 월드에 배치하기 전에는 형상과 텍스처를 확인할 수 없다. 신규 프리뷰도 반드시 CModel → CMaterial 통합 경로를 사용해야 한다.  
② 단순 해법의 문제: 선택 모델을 임시 Layer에 배치하면 Scene/Hierarchy/저장 데이터가 오염되고, 기존 카메라와 조명에 종속된다. 128개 썸네일을 미리 렌더하면 로딩 시간과 VRAM도 불필요하게 증가한다.  
③ 해결 방식: Palette 선택 시 기존 CModel Prototype 하나만 Clone하고, MapTool 하단의 전용 color RTV/SRV + depth DSV에 직접 렌더한다. ImGui::Image가 SRV를 표시한다.  
④ 비교: 월드 오브젝트는 CMapAssetObject가 Layer와 Renderer를 사용하지만, 프리뷰는 Scene 인스턴스가 아닌 Debug Tool 소유의 일시적 CModel Clone이다. CCookedModel과 CBinaryAssetObject는 사용하지 않는다.  
⑤ 대가: 전용 preview shader, color/depth target, 상태 저장·복구 코드가 추가된다. 최대 2048²에서 약 32 MiB를 사용하며, 향후 다중 프리뷰가 필요할 때만 캐시 구조를 재검토한다.

## 3. 자료구조·알고리즘 핵심

### 3.1 확정 UI

별도 OS 창이나 별도 ImGui viewport를 만들지 않는다. 현재 MapTool 내부 레이아웃을 다음처럼 사용한다.

~~~text
LostArk Map Tool
├─ Level / Catalog / Status
├─ Toolbar
├─ Palette | Hierarchy | Inspector
└─ Selected Asset Preview
   ├─ 왼쪽  : 큰 offscreen model image
   └─ 오른쪽
      ├─ label / asset ID / .wmodel path
      ├─ meshes / bounds dimensions
      ├─ Reset View / Arm Placement
      └─ Last .wmodel decode
~~~

기존 Last .wmodel decode는 제거하지 않고 프리뷰 오른쪽 진단 영역으로 이동한다.

### 3.2 선택과 배치 상태 분리

~~~text
Palette 한 번 클릭
  -> m_SelectedAssetId 갱신
  -> 기존 CModel Prototype Clone
  -> Preview 갱신
  -> PLACEMENT_STATE::IDLE 유지

Arm Placement 버튼 또는 Palette 더블클릭
  -> PLACEMENT_STATE::ARMED
  -> 월드 클릭 시 기존 Try_PlaceSelected()
  -> 배치 1개 생성 후 IDLE
~~~

선택만으로 Scene, Layer, placement ID, .mapplacements는 변하지 않는다.

### 3.3 소유권

~~~text
CMainApp
  └─ unique_ptr<CMapTool>
      └─ unique_ptr<CMapAssetPreview>
          ├─ ComPtr<ID3D11Device / Context>
          ├─ color Texture2D / RTV / SRV
          ├─ depth Texture2D / DSV
          ├─ shared_ptr<CShader>       LEVEL::ASSET_TEST Prototype Clone
          └─ shared_ptr<CModel>        선택된 Catalog Prototype Clone 1개
~~~

- color/depth target은 CMapTool의 Debug Tool 수명이다.
- CModel과 CShader Clone은 ASSET_TEST 레벨 Prototype을 참조하므로 레벨 이탈 전에 Reset_LevelResources()로 해제한다.
- 프리뷰 CModel은 Layer에 등록하지 않고 Renderer queue에도 넣지 않는다.
- CCookedModel, CBinaryAssetObject, 별도 decode 경로는 사용하지 않는다.

### 3.4 자동 프레이밍

정의:

~~~text
center = (boundsMin + boundsMax) / 2
halfExtent = (boundsMax - boundsMin) / 2
radius = length(halfExtent)
halfFov = min(verticalFov / 2, horizontalFov / 2)
distance = radius / sin(halfFov) * zoom
world = Translate(-center) * Rotate(pitch, yaw)
~~~

이 방식은 벽처럼 가로로 긴 모델과 기둥처럼 세로로 긴 모델을 동일 UI 영역 안에 자동으로 넣는다. Catalog defaultScale이나 실제 placement Transform은 프리뷰 식별에 필요하지 않으므로 적용하지 않는다.

### 3.5 렌더 순서와 D3D11 상태

현재 CMainApp::Render()는 Scene 렌더가 끝난 뒤 CMapTool::Render(), 마지막에 ImGui EndFrame을 호출한다. 따라서 프리뷰는 다음 순서를 사용한다.

~~~text
Scene Render 완료
  -> MapTool Render
     -> 기존 BackBuffer RTV / DSV / Viewport 보관
     -> 이전 프레임 ImGui SRV slot 0 해제
     -> Preview RTV + DSV + Viewport bind
     -> clear
     -> CModel mesh를 preview shader로 draw
     -> BackBuffer RTV / DSV / Viewport 복구
     -> ImGui::Image(preview SRV)
  -> ImGui EndFrame
  -> Present
~~~

Scene 렌더 이후라서 IA/VS/PS 상태는 ImGui DX11 backend가 다시 설정한다. 하지만 OM target과 viewport는 직접 보관·복구한다.

### 3.6 프리뷰 target 재생성

- ImGui image 영역 크기를 256~2048로 clamp한다.
- 각 축을 64픽셀 단위로 올림한다.
- 기존 target과 같은 크기면 재사용한다.
- 새 Texture/RTV/SRV/DSV를 지역 ComPtr에 모두 만든 뒤 성공 시에만 멤버와 교체한다.
- 생성 실패 시 이전 target과 이전 프리뷰는 유지하고 status만 실패로 바꾼다.

### 3.7 복잡도

- A = Catalog 에셋 수, M = 선택 모델 mesh 수, V = 선택 모델 vertex/index draw 비용.
- Palette 화면: 기존 O(A).
- 선택: Catalog 행은 이미 순회 중이므로 O(1), CModel 정적 Clone은 mesh/material GPU 자원을 공유한다.
- 프리뷰 draw: dirty frame에만 O(M + V), 선택 모델 하나만 렌더.
- target 메모리: RGBA8 + D24S8이므로 W × H × 8 bytes. 2048² 최대 약 32 MiB.
- Scene placement 수 P와 프리뷰 비용은 무관하다.

## 4. 추가·수정·삭제 파일 목록

| 구분 | 절대 경로 | 역할 |
|---|---|---|
| 추가 | C:/Users/user/Desktop/LostArk/Client/Public/MapAssetPreview.h | 선택 모델 Clone, offscreen target, orbit/zoom 상태 선언 |
| 추가 | C:/Users/user/Desktop/LostArk/Client/Private/MapAssetPreview.cpp | CModel 기반 프리뷰 렌더와 D3D11 상태 복구 |
| 추가 | C:/Users/user/Desktop/LostArk/Client/Bin/ShaderFiles/Shader_VtxMeshPreview.hlsl | 단일 color target용 diffuse/normal 간이 조명 |
| 수정 | C:/Users/user/Desktop/LostArk/Client/Public/MapTool.h | 프리뷰 소유권, 초기화, 선택/배치 분리 API |
| 수정 | C:/Users/user/Desktop/LostArk/Client/Private/MapTool.cpp | 하단 프리뷰 UI와 입력, Palette 동작 변경 |
| 수정 | C:/Users/user/Desktop/LostArk/Client/Private/MainApp.cpp | CMapTool에 Device/Context 초기화 |
| 수정 | C:/Users/user/Desktop/LostArk/Client/Private/Loader.cpp | ASSET_TEST preview shader Prototype 등록 |
| 수정 | C:/Users/user/Desktop/LostArk/Client/Default/Client.vcxproj | 신규 C++/HLSL 등록 |
| 수정 | C:/Users/user/Desktop/LostArk/Client/Default/Client.vcxproj.filters | 물리 폴더 기준 필터 등록 |
| 삭제 | 없음 | CCookedModel 경로나 기존 MapAssetObject를 제거하지 않는다 |

## 5. 파일별 전체 구현 코드

### 5-1. C:/Users/user/Desktop/LostArk/Client/Public/MapAssetPreview.h

변경 종류: 추가  
적용 위치: 새 파일 전체

~~~cpp
#pragma once

#include "Client_Defines.h"

#include <string>

NS_BEGIN(Engine)
class CModel;
class CShader;
NS_END

NS_BEGIN(Client)

struct MAP_ASSET_ENTRY;

class CMapAssetPreview final
{
public:
    static constexpr const wchar_t* SHADER_PROTOTYPE_TAG =
        L"Prototype_Component_Shader_MapAssetPreview";

public:
    HRESULT Initialize(ComPtr<ID3D11Device> pDevice,
        ComPtr<ID3D11DeviceContext> pContext);
    void Reset_LevelResources();

    HRESULT Select_Asset(const MAP_ASSET_ENTRY& asset);
    HRESULT Render(uint32_t width, uint32_t height);

    void Orbit(f32_t deltaX, f32_t deltaY);
    void Zoom(f32_t wheelDelta);
    void Reset_Camera();

    bool_t Has_Asset() const { return nullptr != m_pModel; }
    ID3D11ShaderResourceView* Get_TextureView() const
    {
        return m_pColorSRV.Get();
    }

    const std::string& Get_AssetId() const { return m_AssetId; }
    const std::string& Get_Label() const { return m_Label; }
    const std::string& Get_ModelPath() const { return m_ModelPath; }
    const std::string& Get_Status() const { return m_Status; }
    uint32_t Get_MeshCount() const;
    float3_t Get_Dimensions() const;

private:
    HRESULT Ensure_RenderTarget(uint32_t width, uint32_t height);
    HRESULT Render_Model();

private:
    ComPtr<ID3D11Device> m_pDevice = { nullptr };
    ComPtr<ID3D11DeviceContext> m_pContext = { nullptr };

    ComPtr<ID3D11Texture2D> m_pColorTexture = { nullptr };
    ComPtr<ID3D11RenderTargetView> m_pColorRTV = { nullptr };
    ComPtr<ID3D11ShaderResourceView> m_pColorSRV = { nullptr };
    ComPtr<ID3D11Texture2D> m_pDepthTexture = { nullptr };
    ComPtr<ID3D11DepthStencilView> m_pDepthDSV = { nullptr };
    uint32_t m_iTargetWidth = {};
    uint32_t m_iTargetHeight = {};

    shared_ptr<CShader> m_pShader = { nullptr };
    shared_ptr<CModel> m_pModel = { nullptr };

    std::string m_AssetId;
    std::string m_Label;
    std::string m_ModelPath;
    std::string m_Status = "Select an asset from the Palette.";

    float3_t m_vBoundsCenter = {};
    float3_t m_vHalfExtent = {};
    f32_t m_fBoundsRadius = 1.f;
    f32_t m_fYaw = {};
    f32_t m_fPitch = {};
    f32_t m_fZoom = 1.f;
    bool_t m_bDirty = true;
};

NS_END
~~~

### 5-2. C:/Users/user/Desktop/LostArk/Client/Private/MapAssetPreview.cpp

변경 종류: 추가  
적용 위치: 새 파일 전체

~~~cpp
#include "MapAssetPreview.h"

#include "GameInstance.h"
#include "MapAssetCatalog.h"
#include "Model.h"
#include "Shader.h"

#include <algorithm>
#include <array>
#include <cmath>

namespace
{
    constexpr uint32_t MIN_TARGET_SIZE = 256;
    constexpr uint32_t MAX_TARGET_SIZE = 2048;
    constexpr uint32_t TARGET_GRANULARITY = 64;

    uint32_t QuantizeTargetSize(uint32_t value)
    {
        value = (std::max)(MIN_TARGET_SIZE,
            (std::min)(MAX_TARGET_SIZE, value));
        value = (value + TARGET_GRANULARITY - 1) /
            TARGET_GRANULARITY * TARGET_GRANULARITY;
        return (std::min)(MAX_TARGET_SIZE, value);
    }

    bool_t IsFinite(const float3_t& value)
    {
        return std::isfinite(value.x) &&
            std::isfinite(value.y) &&
            std::isfinite(value.z);
    }
}

HRESULT CMapAssetPreview::Initialize(
    ComPtr<ID3D11Device> pDevice,
    ComPtr<ID3D11DeviceContext> pContext)
{
    if (nullptr == pDevice || nullptr == pContext)
        return E_FAIL;

    m_pDevice = pDevice;
    m_pContext = pContext;
    Reset_Camera();
    return S_OK;
}

void CMapAssetPreview::Reset_LevelResources()
{
    m_pModel.reset();
    m_pShader.reset();
    m_AssetId.clear();
    m_Label.clear();
    m_ModelPath.clear();
    m_vBoundsCenter = {};
    m_vHalfExtent = {};
    m_fBoundsRadius = 1.f;
    m_Status = "Select an asset from the Palette.";
    Reset_Camera();
}

HRESULT CMapAssetPreview::Select_Asset(
    const MAP_ASSET_ENTRY& asset)
{
    if (asset.id.empty() || asset.prototypeTag.empty())
        return E_FAIL;

    shared_ptr<CShader> stagedShader = m_pShader;
    if (nullptr == stagedShader)
    {
        stagedShader = dynamic_pointer_cast<CShader>(
            CGameInstance::Get().Clone_Prototype(
                ETOUI(LEVEL::ASSET_TEST),
                SHADER_PROTOTYPE_TAG));
        if (nullptr == stagedShader)
        {
            m_Status = "Preview shader clone failed.";
            m_pModel.reset();
            return E_FAIL;
        }
    }

    shared_ptr<CModel> stagedModel =
        dynamic_pointer_cast<CModel>(
            CGameInstance::Get().Clone_Prototype(
                ETOUI(LEVEL::ASSET_TEST),
                asset.prototypeTag));
    if (nullptr == stagedModel ||
        !stagedModel->Has_LocalBounds())
    {
        m_Status =
            "Selected CModel clone has no valid local bounds.";
        m_pModel.reset();
        return E_FAIL;
    }

    const float3_t minimum =
        stagedModel->Get_LocalBoundsMin();
    const float3_t maximum =
        stagedModel->Get_LocalBoundsMax();
    const float3_t center(
        (minimum.x + maximum.x) * 0.5f,
        (minimum.y + maximum.y) * 0.5f,
        (minimum.z + maximum.z) * 0.5f);
    const float3_t halfExtent(
        (maximum.x - minimum.x) * 0.5f,
        (maximum.y - minimum.y) * 0.5f,
        (maximum.z - minimum.z) * 0.5f);
    const f32_t radius = std::sqrt(
        halfExtent.x * halfExtent.x +
        halfExtent.y * halfExtent.y +
        halfExtent.z * halfExtent.z);

    if (!IsFinite(center) || !IsFinite(halfExtent) ||
        !std::isfinite(radius) || radius <= 0.0001f)
    {
        m_Status = "Selected CModel bounds are invalid.";
        m_pModel.reset();
        return E_FAIL;
    }

    m_pShader = std::move(stagedShader);
    m_pModel = std::move(stagedModel);
    m_AssetId = asset.id;
    m_Label = asset.label;
    m_ModelPath =
        asset.modelRelativePath.generic_string();
    m_vBoundsCenter = center;
    m_vHalfExtent = halfExtent;
    m_fBoundsRadius = radius;
    m_Status = "Preview ready.";
    Reset_Camera();
    return S_OK;
}

HRESULT CMapAssetPreview::Ensure_RenderTarget(
    uint32_t width, uint32_t height)
{
    width = QuantizeTargetSize(width);
    height = QuantizeTargetSize(height);
    if (width == m_iTargetWidth &&
        height == m_iTargetHeight &&
        nullptr != m_pColorSRV &&
        nullptr != m_pDepthDSV)
        return S_OK;

    D3D11_TEXTURE2D_DESC colorDesc{};
    colorDesc.Width = width;
    colorDesc.Height = height;
    colorDesc.MipLevels = 1;
    colorDesc.ArraySize = 1;
    colorDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    colorDesc.SampleDesc.Count = 1;
    colorDesc.Usage = D3D11_USAGE_DEFAULT;
    colorDesc.BindFlags =
        D3D11_BIND_RENDER_TARGET |
        D3D11_BIND_SHADER_RESOURCE;

    ComPtr<ID3D11Texture2D> stagedColorTexture;
    ComPtr<ID3D11RenderTargetView> stagedColorRTV;
    ComPtr<ID3D11ShaderResourceView> stagedColorSRV;
    if (FAILED(m_pDevice->CreateTexture2D(
            &colorDesc, nullptr, &stagedColorTexture)) ||
        FAILED(m_pDevice->CreateRenderTargetView(
            stagedColorTexture.Get(),
            nullptr,
            &stagedColorRTV)) ||
        FAILED(m_pDevice->CreateShaderResourceView(
            stagedColorTexture.Get(),
            nullptr,
            &stagedColorSRV)))
        return E_FAIL;

    D3D11_TEXTURE2D_DESC depthDesc{};
    depthDesc.Width = width;
    depthDesc.Height = height;
    depthDesc.MipLevels = 1;
    depthDesc.ArraySize = 1;
    depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthDesc.SampleDesc.Count = 1;
    depthDesc.Usage = D3D11_USAGE_DEFAULT;
    depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    ComPtr<ID3D11Texture2D> stagedDepthTexture;
    ComPtr<ID3D11DepthStencilView> stagedDepthDSV;
    if (FAILED(m_pDevice->CreateTexture2D(
            &depthDesc, nullptr, &stagedDepthTexture)) ||
        FAILED(m_pDevice->CreateDepthStencilView(
            stagedDepthTexture.Get(),
            nullptr,
            &stagedDepthDSV)))
        return E_FAIL;

    m_pColorTexture = std::move(stagedColorTexture);
    m_pColorRTV = std::move(stagedColorRTV);
    m_pColorSRV = std::move(stagedColorSRV);
    m_pDepthTexture = std::move(stagedDepthTexture);
    m_pDepthDSV = std::move(stagedDepthDSV);
    m_iTargetWidth = width;
    m_iTargetHeight = height;
    m_bDirty = true;
    return S_OK;
}

HRESULT CMapAssetPreview::Render(
    uint32_t width, uint32_t height)
{
    if (nullptr == m_pModel || nullptr == m_pShader)
        return S_FALSE;

    if (FAILED(Ensure_RenderTarget(width, height)))
    {
        m_Status =
            "Preview render target creation failed.";
        return E_FAIL;
    }

    if (!m_bDirty)
        return S_OK;

    ComPtr<ID3D11RenderTargetView> previousRTV;
    ComPtr<ID3D11DepthStencilView> previousDSV;
    m_pContext->OMGetRenderTargets(
        1, &previousRTV, &previousDSV);

    std::array<D3D11_VIEWPORT,
        D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE>
        previousViewports{};
    uint32_t previousViewportCount =
        static_cast<uint32_t>(previousViewports.size());
    m_pContext->RSGetViewports(
        &previousViewportCount,
        previousViewports.data());

    ID3D11ShaderResourceView* nullSRV = nullptr;
    m_pContext->PSSetShaderResources(
        0, 1, &nullSRV);

    ID3D11RenderTargetView* previewRTV =
        m_pColorRTV.Get();
    m_pContext->OMSetRenderTargets(
        1, &previewRTV, m_pDepthDSV.Get());

    const float clearColor[4] = {
        0.025f, 0.035f, 0.045f, 1.f
    };
    m_pContext->ClearRenderTargetView(
        m_pColorRTV.Get(), clearColor);
    m_pContext->ClearDepthStencilView(
        m_pDepthDSV.Get(),
        D3D11_CLEAR_DEPTH |
        D3D11_CLEAR_STENCIL,
        1.f,
        0);

    D3D11_VIEWPORT previewViewport{};
    previewViewport.Width =
        static_cast<f32_t>(m_iTargetWidth);
    previewViewport.Height =
        static_cast<f32_t>(m_iTargetHeight);
    previewViewport.MinDepth = 0.f;
    previewViewport.MaxDepth = 1.f;
    m_pContext->RSSetViewports(
        1, &previewViewport);

    const HRESULT result = Render_Model();

    ID3D11RenderTargetView* restoredRTV =
        previousRTV.Get();
    m_pContext->OMSetRenderTargets(
        1, &restoredRTV, previousDSV.Get());
    if (0 != previousViewportCount)
    {
        m_pContext->RSSetViewports(
            previousViewportCount,
            previousViewports.data());
    }

    if (FAILED(result))
    {
        m_Status = "Preview model rendering failed.";
        return E_FAIL;
    }

    m_bDirty = false;
    m_Status = "Preview ready.";
    return S_OK;
}

HRESULT CMapAssetPreview::Render_Model()
{
    const f32_t aspect =
        static_cast<f32_t>(m_iTargetWidth) /
        static_cast<f32_t>(m_iTargetHeight);
    const f32_t verticalFov =
        XMConvertToRadians(45.f);
    const f32_t horizontalFov =
        2.f * std::atan(
            std::tan(verticalFov * 0.5f) * aspect);
    const f32_t limitingHalfFov =
        (std::min)(
            verticalFov,
            horizontalFov) * 0.5f;
    const f32_t distance =
        m_fBoundsRadius /
        (std::max)(
            0.1f,
            std::sin(limitingHalfFov)) *
        m_fZoom;

    const matrix_t worldMatrix =
        XMMatrixTranslation(
            -m_vBoundsCenter.x,
            -m_vBoundsCenter.y,
            -m_vBoundsCenter.z) *
        XMMatrixRotationRollPitchYaw(
            m_fPitch,
            m_fYaw,
            0.f);
    const vector_t eye =
        XMVectorSet(0.f, 0.f, -distance, 1.f);
    const matrix_t viewMatrix =
        XMMatrixLookAtLH(
            eye,
            XMVectorZero(),
            XMVectorSet(0.f, 1.f, 0.f, 0.f));
    const f32_t nearPlane =
        (std::max)(
            0.001f,
            distance - m_fBoundsRadius * 1.25f);
    const f32_t farPlane =
        distance + m_fBoundsRadius * 3.f;
    const matrix_t projectionMatrix =
        XMMatrixPerspectiveFovLH(
            verticalFov,
            aspect,
            nearPlane,
            farPlane);

    float4x4_t world{};
    float4x4_t view{};
    float4x4_t projection{};
    XMStoreFloat4x4(&world, worldMatrix);
    XMStoreFloat4x4(&view, viewMatrix);
    XMStoreFloat4x4(
        &projection, projectionMatrix);

    float3_t cameraPosition{};
    XMStoreFloat3(&cameraPosition, eye);
    const float3_t lightDirection(
        -0.45f, -0.75f, 0.35f);

    if (FAILED(m_pShader->Bind_Matrix(
            "g_WorldMatrix", &world)) ||
        FAILED(m_pShader->Bind_Matrix(
            "g_ViewMatrix", &view)) ||
        FAILED(m_pShader->Bind_Matrix(
            "g_ProjMatrix", &projection)) ||
        FAILED(m_pShader->Bind_RawValue(
            "g_CameraPosition",
            &cameraPosition,
            sizeof(cameraPosition))) ||
        FAILED(m_pShader->Bind_RawValue(
            "g_LightDirection",
            &lightDirection,
            sizeof(lightDirection))))
        return E_FAIL;

    for (uint32_t meshIndex = 0;
        meshIndex < m_pModel->Get_NumMeshes();
        ++meshIndex)
    {
        const uint32_t hasNormalTexture =
            m_pModel->Has_MaterialTexture(
                meshIndex,
                aiTextureType_NORMALS) ?
                1u : 0u;

        if (FAILED(m_pModel->Bind_Material(
                m_pShader,
                "g_DiffuseTexture",
                meshIndex,
                aiTextureType_DIFFUSE)) ||
            FAILED(m_pShader->Bind_RawValue(
                "g_HasNormalTexture",
                &hasNormalTexture,
                sizeof(hasNormalTexture))) ||
            (0 != hasNormalTexture &&
                FAILED(m_pModel->Bind_Material(
                    m_pShader,
                    "g_NormalTexture",
                    meshIndex,
                    aiTextureType_NORMALS))) ||
            FAILED(m_pShader->Begin(0)) ||
            FAILED(m_pModel->Render(meshIndex)))
            return E_FAIL;
    }

    return S_OK;
}

void CMapAssetPreview::Orbit(
    f32_t deltaX, f32_t deltaY)
{
    m_fYaw += deltaX * 0.01f;
    m_fPitch += deltaY * 0.01f;
    m_fPitch = (std::max)(
        XMConvertToRadians(-80.f),
        (std::min)(
            XMConvertToRadians(80.f),
            m_fPitch));
    m_bDirty = true;
}

void CMapAssetPreview::Zoom(f32_t wheelDelta)
{
    m_fZoom *= std::pow(0.9f, wheelDelta);
    m_fZoom = (std::max)(
        0.55f,
        (std::min)(4.f, m_fZoom));
    m_bDirty = true;
}

void CMapAssetPreview::Reset_Camera()
{
    m_fYaw = XMConvertToRadians(-35.f);
    m_fPitch = XMConvertToRadians(20.f);
    m_fZoom = 1.f;
    m_bDirty = true;
}

uint32_t CMapAssetPreview::Get_MeshCount() const
{
    return nullptr == m_pModel ?
        0u : m_pModel->Get_NumMeshes();
}

float3_t CMapAssetPreview::Get_Dimensions() const
{
    return float3_t(
        m_vHalfExtent.x * 2.f,
        m_vHalfExtent.y * 2.f,
        m_vHalfExtent.z * 2.f);
}
~~~

### 5-3. C:/Users/user/Desktop/LostArk/Client/Bin/ShaderFiles/Shader_VtxMeshPreview.hlsl

변경 종류: 추가  
적용 위치: 새 파일 전체

~~~hlsl
#include "Engine_Shader_Defines.hlsli"

float4x4 g_WorldMatrix;
float4x4 g_ViewMatrix;
float4x4 g_ProjMatrix;

float3 g_CameraPosition;
float3 g_LightDirection;

Texture2D g_DiffuseTexture;
Texture2D g_NormalTexture;
uint g_HasNormalTexture = 0;

RasterizerState RS_Preview
{
    FillMode = Solid;
    CullMode = None;
    FrontCounterClockwise = false;
};

struct VS_IN
{
    float3 vPosition : POSITION;
    float3 vNormal : NORMAL;
    float3 vTangent : TANGENT;
    float3 vBinormal : BINORMAL;
    float2 vTexcoord : TEXCOORD0;
};

struct VS_OUT
{
    float4 vPosition : SV_POSITION;
    float3 vNormal : NORMAL;
    float3 vTangent : TANGENT;
    float3 vBinormal : BINORMAL;
    float2 vTexcoord : TEXCOORD0;
    float3 vWorldPosition : TEXCOORD1;
};

VS_OUT VS_MAIN(VS_IN input)
{
    VS_OUT output;
    const matrix worldView =
        mul(g_WorldMatrix, g_ViewMatrix);
    const matrix worldViewProjection =
        mul(worldView, g_ProjMatrix);

    output.vPosition = mul(
        float4(input.vPosition, 1.f),
        worldViewProjection);
    output.vNormal = normalize(mul(
        float4(input.vNormal, 0.f),
        g_WorldMatrix).xyz);
    output.vTangent = normalize(mul(
        float4(input.vTangent, 0.f),
        g_WorldMatrix).xyz);
    output.vBinormal = normalize(mul(
        float4(input.vBinormal, 0.f),
        g_WorldMatrix).xyz);
    output.vTexcoord = input.vTexcoord;
    output.vWorldPosition = mul(
        float4(input.vPosition, 1.f),
        g_WorldMatrix).xyz;
    return output;
}

float4 PS_MAIN(VS_OUT input) : SV_TARGET0
{
    const float4 albedo =
        g_DiffuseTexture.Sample(
            LinearSampler,
            input.vTexcoord);
    if (albedo.a < 0.3f)
        discard;

    float3 normal = normalize(input.vNormal);
    if (0 != g_HasNormalTexture)
    {
        const float3 tangentNormal =
            g_NormalTexture.Sample(
                LinearSampler,
                input.vTexcoord).xyz *
            2.f - 1.f;
        const float3x3 tangentToWorld =
            float3x3(
                normalize(input.vTangent),
                normalize(input.vBinormal) * -1.f,
                normal);
        normal = normalize(
            mul(tangentNormal, tangentToWorld));
    }

    const float3 light =
        normalize(-g_LightDirection);
    const float diffuseLight =
        saturate(dot(normal, light));
    const float hemisphere =
        0.38f + saturate(normal.y) * 0.18f;

    const float3 viewDirection =
        normalize(
            g_CameraPosition -
            input.vWorldPosition);
    const float rim = pow(
        1.f - saturate(
            dot(normal, viewDirection)),
        3.f) * 0.12f;

    const float3 color =
        albedo.rgb *
        (hemisphere + diffuseLight * 0.72f) +
        rim;
    return float4(color, albedo.a);
}

technique11 DefaultTechnique
{
    pass PreviewPass
    {
        SetRasterizerState(RS_Preview);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(
            BS_Default,
            float4(0.f, 0.f, 0.f, 0.f),
            0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN();
    }
}
~~~

### 5-4. C:/Users/user/Desktop/LostArk/Client/Public/MapTool.h

변경 종류: 전체 교체  
적용 위치: 파일 전체

~~~cpp
#pragma once

#include "Client_Defines.h"
#include "MapAssetCatalog.h"

#include <memory>
#include <string>
#include <vector>

NS_BEGIN(Client)

class CMapAssetObject;
class CMapAssetPreview;

class CMapTool final
{
private:
    enum class PLACEMENT_STATE
    {
        IDLE,
        ARMED,
    };

    struct PLACED_ENTRY
    {
        uint64_t placementId = {};
        std::string assetId;
        shared_ptr<CMapAssetObject> object;
    };

public:
    HRESULT Initialize(
        ComPtr<ID3D11Device> pDevice,
        ComPtr<ID3D11DeviceContext> pContext);

    void Toggle();
    void Update(f32_t fTimeDelta);
    void Render();

    bool IsOpen() const;

private:
    void Handle_LevelTransition(
        bool_t isAssetTest);
    bool_t Try_PickPlacementPosition(
        float3_t& outPosition) const;
    bool_t Try_PlaceSelected();
    bool_t Create_Placement(
        uint64_t placementId,
        const std::string& assetId,
        const float3_t& position,
        const float3_t& rotationDegrees,
        const float3_t& scale,
        bool_t visible,
        PLACED_ENTRY& outEntry);
    bool_t Remove_Placement(
        uint64_t placementId);
    void Remove_AllPlacements();
    bool_t Save_Placements();
    bool_t Load_Placements();

    void Select_Asset(
        const MAP_ASSET_ENTRY& asset);
    void Arm_SelectedAsset();

    void Render_Toolbar();
    void Render_Palette(f32_t childHeight);
    void Render_Hierarchy(f32_t childHeight);
    void Render_Inspector();
    void Render_AssetPreview();
    void Render_DecoderReport() const;

    PLACED_ENTRY* Find_Placement(
        uint64_t placementId);
    const MAP_ASSET_ENTRY*
        Get_SelectedAsset() const;

private:
    bool_t m_bOpen = false;
    bool_t m_bWasInAssetTest = false;
    bool_t m_bPreviousMouseDown = false;
    bool_t m_bDirty = false;
    PLACEMENT_STATE m_ePlacementState =
        PLACEMENT_STATE::IDLE;

    CMapAssetCatalog m_Catalog;
    std::unique_ptr<CMapAssetPreview>
        m_pAssetPreview;
    std::string m_SelectedAssetId;
    std::string m_Status =
        "Enter AssetTest with F2";
    char m_Filter[128]{};

    vector<PLACED_ENTRY> m_Placements;
    uint64_t m_iSelectedPlacementId = {};
    uint64_t m_iNextPlacementId = 1;
};

NS_END
~~~

### 5-5. C:/Users/user/Desktop/LostArk/Client/Private/MapTool.cpp

변경 종류: include 및 함수 교체/추가  
적용 위치: include 블록, Initialize, Handle_LevelTransition, Render, Render_Toolbar, Render_Palette, Render_Hierarchy, 신규 Preview 함수

include 블록은 다음으로 교체한다.

~~~cpp
#include "imgui.h"

#include "MapTool.h"

#include "BinaryAsset/ModelDecoderRegistry.h"
#include "GameInstance.h"
#include "MapAssetObject.h"
#include "MapAssetPreview.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <unordered_set>
~~~

Initialize 함수를 Toggle 앞에 추가한다.

~~~cpp
HRESULT Client::CMapTool::Initialize(
    ComPtr<ID3D11Device> pDevice,
    ComPtr<ID3D11DeviceContext> pContext)
{
    auto preview =
        std::make_unique<CMapAssetPreview>();
    if (FAILED(preview->Initialize(
        pDevice, pContext)))
        return E_FAIL;

    m_pAssetPreview = std::move(preview);
    return S_OK;
}
~~~

Render 함수 전체를 교체한다.

~~~cpp
void Client::CMapTool::Render()
{
    if (!m_bOpen)
        return;

    ImGui::SetNextWindowSize(
        ImVec2(1180.f, 900.f),
        ImGuiCond_FirstUseEver);
    if (!ImGui::Begin(
        "LostArk Map Tool", &m_bOpen))
    {
        ImGui::End();
        return;
    }

    const bool_t isAssetTest =
        ETOUI(LEVEL::ASSET_TEST) ==
        CGameInstance::Get()
            .Get_CurrentLevelID();
    ImGui::Text(
        "Level: %s",
        isAssetTest ?
            "ASSET_TEST" :
            "Open ASSET_TEST with F2");
    ImGui::SameLine();
    ImGui::Text(
        "| Catalog: %s",
        m_Catalog.Is_Ready() ?
            "READY" : "NOT READY");
    ImGui::TextWrapped(
        "%s", m_Status.c_str());
    ImGui::Separator();

    ImGui::BeginDisabled(
        !isAssetTest ||
        !m_Catalog.Is_Ready());
    Render_Toolbar();

    const f32_t availableHeight =
        ImGui::GetContentRegionAvail().y;
    const f32_t topPanelHeight =
        (std::max)(
            280.f,
            (std::min)(
                480.f,
                availableHeight * 0.48f));

    if (ImGui::BeginTable(
        "MapEditorColumns",
        3,
        ImGuiTableFlags_Resizable |
        ImGuiTableFlags_BordersInnerV))
    {
        ImGui::TableSetupColumn(
            "Asset Palette",
            ImGuiTableColumnFlags_WidthStretch,
            0.38f);
        ImGui::TableSetupColumn(
            "Hierarchy",
            ImGuiTableColumnFlags_WidthStretch,
            0.27f);
        ImGui::TableSetupColumn(
            "Inspector",
            ImGuiTableColumnFlags_WidthStretch,
            0.35f);
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        Render_Palette(topPanelHeight);
        ImGui::TableSetColumnIndex(1);
        Render_Hierarchy(topPanelHeight);
        ImGui::TableSetColumnIndex(2);
        Render_Inspector();
        ImGui::EndTable();
    }

    Render_AssetPreview();
    ImGui::EndDisabled();
    ImGui::End();
}
~~~

Handle_LevelTransition 함수 전체를 교체한다.

~~~cpp
void Client::CMapTool::Handle_LevelTransition(
    bool_t isAssetTest)
{
    if (isAssetTest == m_bWasInAssetTest)
        return;

    m_bWasInAssetTest = isAssetTest;
    m_ePlacementState =
        PLACEMENT_STATE::IDLE;
    m_iSelectedPlacementId = 0;
    m_SelectedAssetId.clear();

    if (nullptr != m_pAssetPreview)
        m_pAssetPreview
            ->Reset_LevelResources();

    if (!isAssetTest)
    {
        m_Placements.clear();
        m_iNextPlacementId = 1;
        m_bDirty = false;
        m_Status = "Enter AssetTest with F2";
        return;
    }

    if (!m_Catalog.Load_Default())
    {
        m_Status = m_Catalog.Get_Status();
        return;
    }

    m_Status = m_Catalog.Get_Status();
    Load_Placements();
}
~~~

Render_Toolbar 함수 전체를 교체한다.

~~~cpp
void Client::CMapTool::Render_Toolbar()
{
    if (ImGui::Button("Save"))
        Save_Placements();
    ImGui::SameLine();
    if (ImGui::Button("Reload"))
        Load_Placements();
    ImGui::SameLine();
    if (ImGui::Button("Clear"))
        ImGui::OpenPopup(
            "Clear all placements?");
    ImGui::SameLine();

    ImGui::BeginDisabled(
        nullptr == Get_SelectedAsset() ||
        PLACEMENT_STATE::ARMED ==
        m_ePlacementState);
    if (ImGui::Button("Arm placement"))
        Arm_SelectedAsset();
    ImGui::EndDisabled();

    ImGui::SameLine();
    ImGui::Text(
        "Objects: %zu%s",
        m_Placements.size(),
        m_bDirty ? "  *unsaved" : "");

    if (ImGui::BeginPopupModal(
        "Clear all placements?",
        nullptr,
        ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::TextUnformatted(
            "Remove every placed map object "
            "from this level?");
        if (ImGui::Button("Clear all"))
        {
            Remove_AllPlacements();
            m_Status =
                "Cleared all placements "
                "(not saved yet)";
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel"))
            ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }

    if (PLACEMENT_STATE::ARMED ==
        m_ePlacementState)
    {
        ImGui::TextColored(
            ImVec4(1.f, 0.85f, 0.2f, 1.f),
            "PLACEMENT ARMED: click the world "
            "(Esc cancels)");
    }
    ImGui::Separator();
}
~~~

Render_Palette 함수 전체를 교체한다.

~~~cpp
void Client::CMapTool::Render_Palette(
    f32_t childHeight)
{
    ImGui::TextUnformatted("Palette");
    ImGui::SetNextItemWidth(-FLT_MIN);
    ImGui::InputTextWithHint(
        "##AssetFilter",
        "filter walls, ruins, cliffs, "
        "foliage...",
        m_Filter,
        sizeof(m_Filter));

    const f32_t listHeight = (std::max)(
        120.f,
        childHeight -
        ImGui::GetTextLineHeightWithSpacing() *
        2.f);
    ImGui::BeginChild(
        "AssetPaletteList",
        ImVec2(0.f, listHeight),
        true);

    for (const MAP_ASSET_ENTRY& asset :
        m_Catalog.Get_Entries())
    {
        if (!MatchesFilter(
            asset.label + " " + asset.id,
            m_Filter))
            continue;

        const bool_t selected =
            asset.id == m_SelectedAssetId;
        if (ImGui::Selectable(
            asset.label.c_str(), selected))
            Select_Asset(asset);

        if (ImGui::IsItemHovered() &&
            ImGui::IsMouseDoubleClicked(
                ImGuiMouseButton_Left))
        {
            if (!selected)
                Select_Asset(asset);
            Arm_SelectedAsset();
        }

        if (ImGui::IsItemHovered())
        {
            ImGui::BeginTooltip();
            ImGui::TextUnformatted(
                asset.id.c_str());
            ImGui::TextWrapped(
                "%s",
                asset.modelRelativePath
                    .string().c_str());
            ImGui::TextDisabled(
                "Single click: preview | "
                "Double click: arm placement");
            ImGui::EndTooltip();
        }
    }

    ImGui::EndChild();
}
~~~

Render_Hierarchy 함수 전체를 교체한다.

~~~cpp
void Client::CMapTool::Render_Hierarchy(
    f32_t childHeight)
{
    ImGui::TextUnformatted("Hierarchy");
    const f32_t listHeight = (std::max)(
        120.f,
        childHeight -
        ImGui::GetTextLineHeightWithSpacing());
    ImGui::BeginChild(
        "PlacementHierarchy",
        ImVec2(0.f, listHeight),
        true);

    for (const PLACED_ENTRY& entry :
        m_Placements)
    {
        const MAP_ASSET_ENTRY* pAsset =
            m_Catalog.Find(entry.assetId);
        const std::string label =
            nullptr == pAsset ?
            entry.assetId : pAsset->label;

        ImGui::PushID(
            reinterpret_cast<void*>(
                static_cast<uintptr_t>(
                    entry.placementId)));
        const bool_t selected =
            entry.placementId ==
            m_iSelectedPlacementId;
        if (ImGui::Selectable(
            label.c_str(), selected))
            m_iSelectedPlacementId =
                entry.placementId;
        ImGui::SameLine();
        ImGui::TextDisabled(
            "#%llu",
            static_cast<unsigned long long>(
                entry.placementId));
        ImGui::PopID();
    }

    ImGui::EndChild();
}
~~~

다음 세 함수를 Render_DecoderReport 앞에 추가한다.

~~~cpp
void Client::CMapTool::Select_Asset(
    const MAP_ASSET_ENTRY& asset)
{
    m_SelectedAssetId = asset.id;
    m_ePlacementState =
        PLACEMENT_STATE::IDLE;

    if (nullptr == m_pAssetPreview ||
        FAILED(m_pAssetPreview
            ->Select_Asset(asset)))
    {
        m_Status =
            nullptr == m_pAssetPreview ?
            "Preview service is not initialized." :
            m_pAssetPreview
                ->Get_Status();
        return;
    }

    m_Status =
        "Previewing " + asset.label +
        "; press Arm placement or "
        "double-click the Palette row "
        "to place it.";
}

void Client::CMapTool::Arm_SelectedAsset()
{
    const MAP_ASSET_ENTRY* pAsset =
        Get_SelectedAsset();
    if (nullptr == pAsset)
    {
        m_Status =
            "Select an asset before "
            "arming placement.";
        return;
    }

    m_ePlacementState =
        PLACEMENT_STATE::ARMED;
    m_Status =
        "Placement armed for " +
        pAsset->label +
        "; click the rendered surface "
        "or Y=0 plane.";
}

void Client::CMapTool::Render_AssetPreview()
{
    ImGui::SeparatorText(
        "Selected Asset Preview");

    const MAP_ASSET_ENTRY* pAsset =
        Get_SelectedAsset();
    if (nullptr == pAsset ||
        nullptr == m_pAssetPreview ||
        !m_pAssetPreview->Has_Asset())
    {
        ImGui::BeginChild(
            "AssetPreviewEmpty",
            ImVec2(0.f, 280.f),
            true);
        ImGui::TextDisabled(
            "Select an asset from the Palette. "
            "No world placement is required.");
        if (nullptr != m_pAssetPreview)
        {
            ImGui::TextWrapped(
                "%s",
                m_pAssetPreview
                    ->Get_Status().c_str());
        }
        Render_DecoderReport();
        ImGui::EndChild();
        return;
    }

    const f32_t panelHeight =
        (std::max)(
            320.f,
            ImGui::GetContentRegionAvail().y);
    ImGui::BeginChild(
        "AssetPreviewPanel",
        ImVec2(0.f, panelHeight),
        true);

    if (ImGui::BeginTable(
        "AssetPreviewColumns",
        2,
        ImGuiTableFlags_Resizable |
        ImGuiTableFlags_BordersInnerV))
    {
        ImGui::TableSetupColumn(
            "Preview Image",
            ImGuiTableColumnFlags_WidthStretch,
            0.68f);
        ImGui::TableSetupColumn(
            "Preview Information",
            ImGuiTableColumnFlags_WidthStretch,
            0.32f);
        ImGui::TableNextRow();

        ImGui::TableSetColumnIndex(0);
        const ImVec2 available =
            ImGui::GetContentRegionAvail();
        const ImVec2 imageSize(
            (std::max)(256.f, available.x),
            (std::max)(
                256.f,
                panelHeight - 12.f));

        m_pAssetPreview->Render(
            static_cast<uint32_t>(
                imageSize.x),
            static_cast<uint32_t>(
                imageSize.y));

        ID3D11ShaderResourceView* pTexture =
            m_pAssetPreview->Get_TextureView();
        if (nullptr != pTexture)
        {
            const ImTextureID textureId =
                static_cast<ImTextureID>(
                    reinterpret_cast<uintptr_t>(
                        pTexture));
            ImGui::Image(
                ImTextureRef(textureId),
                imageSize,
                ImVec2(0.f, 0.f),
                ImVec2(1.f, 1.f));

            if (ImGui::IsItemHovered())
            {
                const ImGuiIO& io =
                    ImGui::GetIO();
                if (ImGui::IsMouseDragging(
                    ImGuiMouseButton_Left))
                {
                    m_pAssetPreview->Orbit(
                        io.MouseDelta.x,
                        io.MouseDelta.y);
                }
                if (0.f != io.MouseWheel)
                {
                    m_pAssetPreview->Zoom(
                        io.MouseWheel);
                }
            }
        }
        else
        {
            ImGui::TextWrapped(
                "Preview texture is "
                "unavailable: %s",
                m_pAssetPreview
                    ->Get_Status().c_str());
        }

        ImGui::TableSetColumnIndex(1);
        ImGui::TextWrapped(
            "%s",
            m_pAssetPreview
                ->Get_Label().c_str());
        ImGui::Separator();
        ImGui::TextWrapped(
            "Asset ID: %s",
            m_pAssetPreview
                ->Get_AssetId().c_str());
        ImGui::TextWrapped(
            "Model: %s",
            m_pAssetPreview
                ->Get_ModelPath().c_str());

        const float3_t dimensions =
            m_pAssetPreview
                ->Get_Dimensions();
        ImGui::Text(
            "Meshes: %u",
            m_pAssetPreview
                ->Get_MeshCount());
        ImGui::Text(
            "Bounds: %.3f x %.3f x %.3f",
            dimensions.x,
            dimensions.y,
            dimensions.z);
        ImGui::TextWrapped(
            "Preview: %s",
            m_pAssetPreview
                ->Get_Status().c_str());

        if (ImGui::Button("Reset view"))
            m_pAssetPreview->Reset_Camera();
        ImGui::SameLine();
        if (ImGui::Button("Arm placement"))
            Arm_SelectedAsset();

        ImGui::TextDisabled(
            "LMB drag: orbit");
        ImGui::TextDisabled(
            "Mouse wheel: zoom");

        Render_DecoderReport();
        ImGui::EndTable();
    }

    ImGui::EndChild();
}
~~~

기존 Render_DecoderReport() 호출은 Render()의 끝에서 삭제한다. 함수 구현 자체는 그대로 유지하며 Render_AssetPreview() 오른쪽 열에서 호출한다.

### 5-6. C:/Users/user/Desktop/LostArk/Client/Private/MainApp.cpp

변경 종류: 함수 교체  
적용 위치: CMainApp::ReadyDebugTools()

~~~cpp
#ifdef _DEBUG
HRESULT CMainApp::ReadyDebugTools()
{
    m_pImGuiLayer =
        std::make_unique<
            Engine::CImGuiLayer>();
    if (!m_pImGuiLayer->Initialize(
        g_hWnd,
        m_pDevice.Get(),
        m_pContext.Get()))
    {
        OutputDebugStringA(
            "[ImGui] Failed to initialize "
            "Win32/DX11 runtime.\n");
        return E_FAIL;
    }

    auto mapTool =
        std::make_unique<CMapTool>();
    if (FAILED(mapTool->Initialize(
        m_pDevice,
        m_pContext)))
    {
        OutputDebugStringA(
            "[MapTool] Failed to initialize "
            "asset preview runtime.\n");
        return E_FAIL;
    }

    m_pMapTool = std::move(mapTool);
    m_pEffectTool =
        std::make_unique<CEffect_Tool>();
    return S_OK;
}
#endif
~~~

CMainApp::Render() 순서는 변경하지 않는다. Scene 렌더 후 MapTool, 그 뒤 ImGui EndFrame이라는 현재 순서가 프리뷰 target 안전 계약이다.

### 5-7. C:/Users/user/Desktop/LostArk/Client/Private/Loader.cpp

변경 종류: include 추가 및 함수 교체  
적용 위치: include 블록, CLoader::Ready_For_Level_AssetTest()

MapAsset include 블록에 다음 한 줄을 추가한다.

~~~cpp
#include "MapAssetPreview.h"
~~~

Ready_For_Level_AssetTest 함수 전체를 다음으로 교체한다.

~~~cpp
HRESULT CLoader::Ready_For_Level_AssetTest()
{
    lstrcpy(
        m_szLoadingText,
        TEXT("바이너리 에셋 테스트 자원을 "
            "로딩중입니다."));

    if (FAILED(
        CGameInstance::Get().Add_Prototype(
            ETOUI(LEVEL::ASSET_TEST),
            TEXT(
                "Prototype_Component_Shader_"
                "VtxAnimMeshBinary"),
            CShader::Create(
                m_pDevice,
                m_pContext,
                TEXT(
                    "../Bin/ShaderFiles/"
                    "Shader_VtxAnimMeshBinary.hlsl"),
                VTXANIMMESH::Elements,
                VTXANIMMESH::iNumElements))))
        return E_FAIL;

    if (FAILED(
        CGameInstance::Get().Add_Prototype(
            ETOUI(LEVEL::ASSET_TEST),
            TEXT(
                "Prototype_Component_Shader_"
                "VtxMeshBinary"),
            CShader::Create(
                m_pDevice,
                m_pContext,
                TEXT(
                    "../Bin/ShaderFiles/"
                    "Shader_VtxMeshBinary.hlsl"),
                VTXMESH::Elements,
                VTXMESH::iNumElements))))
        return E_FAIL;

    if (FAILED(
        CGameInstance::Get().Add_Prototype(
            ETOUI(LEVEL::ASSET_TEST),
            CMapAssetPreview::
                SHADER_PROTOTYPE_TAG,
            CShader::Create(
                m_pDevice,
                m_pContext,
                TEXT(
                    "../Bin/ShaderFiles/"
                    "Shader_VtxMeshPreview.hlsl"),
                VTXMESH::Elements,
                VTXMESH::iNumElements))))
        return E_FAIL;

    const matrix_t
        lostArkAssetPreTransform =
            XMMatrixScaling(
                0.0001f,
                0.0001f,
                0.0001f);
    const matrix_t mapAssetTransform =
        XMMatrixScaling(
            0.01f,
            0.01f,
            0.01f);

    CMapAssetCatalog mapCatalog;
    if (!mapCatalog.Load_Default())
        return E_FAIL;

    for (const MAP_ASSET_ENTRY& entry :
        mapCatalog.Get_Entries())
    {
        const string modelPath =
            entry.resolvedModelPath.string();
        if (FAILED(
            CGameInstance::Get().Add_Prototype(
                ETOUI(LEVEL::ASSET_TEST),
                entry.prototypeTag,
                CModel::Create(
                    m_pDevice,
                    m_pContext,
                    MODEL::NONANIM,
                    modelPath.c_str(),
                    mapAssetTransform))))
            return E_FAIL;
    }

    if (FAILED(
        CGameInstance::Get().Add_Prototype(
            ETOUI(LEVEL::ASSET_TEST),
            TEXT(
                "Prototype_Component_Model_"
                "Valtan"),
            CModel::Create(
                m_pDevice,
                m_pContext,
                MODEL::ANIM,
                "../Bin/Resources/LostArk/"
                "Character/MN_RPBF_01/"
                "MN_RPBF_01.wmodel",
                lostArkAssetPreTransform))))
        return E_FAIL;

    if (FAILED(
        CGameInstance::Get().Add_Prototype(
            ETOUI(LEVEL::ASSET_TEST),
            TEXT(
                "Prototype_GameObject_"
                "Camera_Free"),
            CCamera_Free::Create(
                m_pDevice,
                m_pContext))))
        return E_FAIL;

    if (FAILED(
        CGameInstance::Get().Add_Prototype(
            ETOUI(LEVEL::ASSET_TEST),
            TEXT(
                "Prototype_GameObject_"
                "Body_Valtan"),
            CBody_Valtan::Create(
                m_pDevice,
                m_pContext))))
        return E_FAIL;

    if (FAILED(
        CGameInstance::Get().Add_Prototype(
            ETOUI(LEVEL::ASSET_TEST),
            TEXT(
                "Prototype_GameObject_Valtan"),
            CValtan::Create(
                m_pDevice,
                m_pContext))))
        return E_FAIL;

    if (FAILED(
        CGameInstance::Get().Add_Prototype(
            ETOUI(LEVEL::ASSET_TEST),
            TEXT(
                "Prototype_GameObject_"
                "MapAsset"),
            CMapAssetObject::Create(
                m_pDevice,
                m_pContext))))
        return E_FAIL;

    lstrcpy(
        m_szLoadingText,
        TEXT("바이너리 에셋 테스트 로딩이 "
            "완료되었습니다."));
    m_isFinished = true;
    return S_OK;
}
~~~

### 5-8. C:/Users/user/Desktop/LostArk/Client/Default/Client.vcxproj

변경 종류: XML 항목 추가  
적용 위치: 기존 ClInclude, ClCompile, FxCompile ItemGroup

~~~xml
<ClInclude Include="..\Public\MapAssetPreview.h" />
<ClCompile Include="..\Private\MapAssetPreview.cpp" />
<FxCompile Include="..\Bin\ShaderFiles\Shader_VtxMeshPreview.hlsl" />
~~~

### 5-9. C:/Users/user/Desktop/LostArk/Client/Default/Client.vcxproj.filters

변경 종류: XML 항목 추가  
적용 위치: 기존 대응 ItemGroup

~~~xml
<ClInclude Include="..\Public\MapAssetPreview.h">
  <Filter>03. Tools\00. Map</Filter>
</ClInclude>

<ClCompile Include="..\Private\MapAssetPreview.cpp">
  <Filter>03. Tools\00. Map</Filter>
</ClCompile>

<FxCompile Include="..\Bin\ShaderFiles\Shader_VtxMeshPreview.hlsl">
  <Filter>97.ShaderFiles</Filter>
</FxCompile>
~~~

기존 03. Tools\00. Map과 97.ShaderFiles 필터를 재사용하므로 새 Filter GUID는 추가하지 않는다.

## 6. 프로젝트 등록과 불변식

### 6.1 등록 후 확인

~~~text
Client
├─ 03. Tools
│  └─ 00. Map
│     ├─ MapTool.h/.cpp
│     ├─ MapAssetCatalog.h/.cpp
│     ├─ RuntimeAssetRoot.h/.cpp
│     └─ MapAssetPreview.h/.cpp
└─ 97.ShaderFiles
   └─ Shader_VtxMeshPreview.hlsl
~~~

### 6.2 반드시 유지할 불변식

1. Palette 선택만으로 Add_GameObject_to_Layer를 호출하지 않는다.
2. 프리뷰 객체는 placement ID를 만들지 않고 .mapplacements dirty flag를 바꾸지 않는다.
3. 선택 모델은 Catalog prototypeTag로 CModel Clone한다.
4. CCookedModel과 CBinaryAssetObject를 include하거나 호출하지 않는다.
5. 레벨 이탈 시 CModel/CShader Clone을 target보다 먼저 해제한다.
6. 프리뷰 렌더 후 기존 RTV, DSV, viewport를 항상 복구한다.
7. RenderTarget 생성은 stage 후 commit한다.
8. diffuse가 없는 모델도 CMaterial의 회색 안전망으로 형상이 보여야 한다.
9. 프리뷰 shader는 단일 SV_TARGET0만 출력하며 Scene G-buffer를 사용하지 않는다.
10. Release 경로에는 MapTool/Preview 런타임 의존성을 추가하지 않는다.

## 7. 적용 순서와 검증

### 7.1 적용 순서

1. MapAssetPreview.h/.cpp와 Shader_VtxMeshPreview.hlsl 추가
2. vcxproj와 filters 등록
3. Loader에 preview shader Prototype 등록
4. MainApp에서 CMapTool Initialize 연결
5. MapTool header와 cpp 교체
6. Debug 빌드
7. ASSET_TEST 실행 검증
8. 성공 후 같은 주제의 RESULT 문서 작성

### 7.2 빌드

~~~powershell
$MSBuild = 'C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe'

& $MSBuild .\Engine\Default\Engine.vcxproj /m /t:Build /p:Configuration=Debug /p:Platform=x64 /v:minimal

.\UpdateLib.bat Debug

& $MSBuild .\Client\Default\Client.vcxproj /m /t:Build /p:Configuration=Debug /p:Platform=x64 /v:minimal
~~~

### 7.3 실행 절차

~~~text
1. Client Debug 실행
2. Logo에서 F2 -> LEVEL::ASSET_TEST
3. F1 -> LostArk Map Tool
4. Palette의 WALL 한 행을 한 번 클릭
5. 월드를 클릭하지 않아도 하단 Preview에 모델이 보이는지 확인
6. Palette의 PILLAR, GATE, FOLIAGE, FLOOR를 차례로 선택
7. LMB drag로 회전, wheel로 확대/축소, Reset view 확인
8. Arm placement 버튼 전에는 월드 클릭해도 생성되지 않는지 확인
9. Arm placement 또는 Palette 더블클릭 후 월드 클릭하면 기존 배치가 생성되는지 확인
10. Save/Reload 후 기존 placement가 유지되는지 확인
~~~

### 7.4 성공 기준

- 하단 영역에서 실제 diffuse가 적용된 선택 에셋을 배치 없이 확인할 수 있다.
- wall처럼 긴 모델과 pillar처럼 높은 모델이 자동으로 화면 안에 들어온다.
- Preview 우측에 label, asset ID, model path, mesh count, bounds, decode report가 보인다.
- 선택만으로 Objects 수와 unsaved 상태가 바뀌지 않는다.
- Preview 조작 중 게임 카메라가 움직이지 않는다.
- 프리뷰 뒤에도 Scene, Effect Tool, ImGui multi-viewport가 정상 렌더된다.
- ASSET_TEST → LOGO/다른 레벨 → ASSET_TEST 왕복 뒤 stale CModel pointer 없이 다시 선택된다.
- textureless/fallback 에셋도 회색 형상으로 보이고 렌더 실패로 사라지지 않는다.

### 7.5 실패·회귀 검증

- 존재하지 않는 prototypeTag를 임시 테스트하면 이전 Scene과 placement는 유지되고 Preview status만 실패해야 한다.
- 128개를 빠르게 연속 선택해도 프리뷰 CModel은 마지막 하나만 남아야 한다.
- ImGui 창을 계속 리사이즈해도 target은 64픽셀 단위에서만 재생성되어야 한다.
- 2048² clamp를 넘어 창을 키워도 VRAM 사용량이 계속 증가하지 않아야 한다.
- F1로 MapTool을 닫았을 때 preview draw가 발생하지 않아야 한다.
- Debug Layer에서 RTV/SRV 동시 바인딩 경고가 없어야 한다.
- Client 종료 시 D3D11 live object 보고에 Preview Texture/RTV/SRV/DSV 누수가 없어야 한다.

## 8. 최종 결정

프리뷰는 사용자 제안대로 현재 하단의 Model Decode 영역에 통합한다. 별도 창을 만들지 않는다.

~~~text
Palette selection
  -> CModel Prototype Clone
  -> dedicated preview RTV/DSV
  -> Shader_VtxMeshPreview
  -> ImGui::Image
  -> right-side metadata + decoder report
~~~

이 기능은 에셋을 월드에 임시 배치하는 기능이 아니다. Scene과 무관한 Debug Tool 프리뷰이며, 신규 모델 런타임은 끝까지 CModel → CMaterial을 사용한다.
