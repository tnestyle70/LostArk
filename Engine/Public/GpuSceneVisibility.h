#pragma once

#include <d3d11.h>
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif
#include <DirectXMath.h>
#include <wrl/client.h>
#include <cstdint>
#include <functional>
#include <memory>
#include <span>
#include "Engine_Macro.h"

namespace Engine
{
// Client는 원본 모델/재질과 bin 소유권을 유지한다. 이 계층은 불변 instance 자료의
// 가시성 판정과 기존 IA가 읽는 instance VB/indirect args만 만든다.
class ENGINE_DLL CGpuSceneVisibility final
{
public:
    static constexpr uint32_t InvalidBin = 0xffffffffu;
    static constexpr uint32_t LodCount = 5u;
    struct SOURCE_INSTANCE final
    {
        DirectX::XMFLOAT4 WorldSphere{};
        uint32_t LodBins[LodCount]{ InvalidBin, InvalidBin, InvalidBin, InvalidBin, InvalidBin };
        float WorldErrors[LodCount]{};
        float DistanceLimit = 0.f, MaximumPixels = 0.f;
        uint32_t ActiveSource = 0u;
        // bit 0: 변형/alpha 경계를 증명하지 못한 occludee는 HiZ만 건너뛴다.
        uint32_t Flags = 0u;
        uint32_t Reserved[2]{};
    };
    struct DRAW_BIN final
    {
        uint32_t IndexCount = 0u, FirstIndex = 0u;
        int32_t BaseVertex = 0;
        uint32_t Capacity = 0u;
    };
    struct FRAME final
    {
        DirectX::XMFLOAT4X4 View{}, Projection{};
        DirectX::XMFLOAT3 Camera{};
        uint32_t Width = 0u, Height = 0u;
        uint64_t FrameId = 0u;
        float PixelError = 0.9f, DistanceScale = 1.f;
        bool DistanceEnabled = true, MeshLodEnabled = true, HiZEnabled = true, FrustumEnabled = true;
    };
    struct DRAW_OUTPUT final
    {
        ID3D11Buffer* InstanceBuffer = nullptr;
        ID3D11Buffer* Arguments = nullptr;
        uint32_t Stride = 0u, InstanceByteOffset = 0u, ArgumentByteOffset = 0u;
    };
    struct STATISTICS final
    {
        uint64_t FrameId = 0u;
        // ActiveSources는 mesh별 active lane 수, ActiveOwners는 중복을 뺀 배치 instance 수다.
        // HiZ 준비 문턱은 mesh 개수 증가로 바뀌지 않도록 ActiveOwners를 소비한다.
        uint32_t SourceInstances = 0u, Bins = 0u, ActiveSources = 0u, ActiveOwners = 0u;
        uint32_t Dispatches = 0u, UploadBytes = 0u;
        bool UsedHiZ = false, Ready = false;
        double PrepareCpuMs = 0., DispatchCpuMs = 0.;
    };
    CGpuSceneVisibility(ID3D11Device* device, ID3D11DeviceContext* context);
    ~CGpuSceneVisibility();
    CGpuSceneVisibility(const CGpuSceneVisibility&) = delete;
    CGpuSceneVisibility& operator=(const CGpuSceneVisibility&) = delete;
    // CSO는 Engine DLL 옆의 배포 결과를 읽는다. 테스트만 별도 디렉터리를 넘긴다.
    HRESULT Initialize(const wchar_t* shaderDirectory = nullptr);
    // 전체 검증과 GPU 할당이 성공한 후에만 교체하므로 실패는 이전 resident scene을 보존한다.
    HRESULT Stage(std::span<const SOURCE_INSTANCE> instances, std::span<const uint8_t> payload,
        std::span<const DRAW_BIN> bins, uint32_t activeSourceCount, uint32_t stride);
    void Clear() noexcept;
    // final-camera callback은 owner/coarse/HLOD claim을 합친 active bit만 갱신한다.
    // mesh별 lane을 공유하는 호출자는 실제 활성 instance의 중복 없는 수를 전달한다.
    // 생략하면 활성 lane 수를 사용하며, 잘못된 owner 수는 GPU buffer와 통계를 변경하지 않는다.
    HRESULT Set_ActiveSources(std::span<const uint32_t> active, uint32_t activeOwnerCount = UINT32_MAX);
    void Set_FramePrepareCallback(std::function<HRESULT(const FRAME&)> callback);
    HRESULT Prepare_Frame(const FRAME& frame);
    HRESULT Dispatch(const FRAME& frame, ID3D11ShaderResourceView* currentHiZ = nullptr, uint32_t mipCount = 0u);
    void Invalidate_Frame() noexcept;
    bool Get_Draw(uint32_t bin, uint64_t frameId, DRAW_OUTPUT& output) const noexcept;
    const STATISTICS& Get_Statistics() const noexcept;
private:
    struct IMPL;
    std::unique_ptr<IMPL> m_Impl;
};

// 현재 프레임의 신뢰 가능한 불투명 정적 geometry만 입력한다. 이전 프레임 깊이나
// bounding box를 가림자로 쓰지 않아 카메라 이동/동적 오브젝트의 잘못된 가림을 막는다.
class ENGINE_DLL CGpuVisibilityHiZ final
{
public:
    CGpuVisibilityHiZ(ID3D11Device* device, ID3D11DeviceContext* context);
    ~CGpuVisibilityHiZ();
    HRESULT Initialize(const wchar_t* shaderDirectory = nullptr);
    HRESULT Begin_Depth(uint32_t width, uint32_t height);
    HRESULT End_Depth(bool validContributors);
    ID3D11ShaderResourceView* Get_SRV() const noexcept;
    uint32_t Get_MipCount() const noexcept;
private:
    struct IMPL;
    std::unique_ptr<IMPL> m_Impl;
};
}
