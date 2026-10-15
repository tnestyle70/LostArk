#pragma once
#include "Client_Defines.h"
#include "GpuSceneVisibility.h"
#include <span>
namespace Engine { class CModel; }

NS_BEGIN(Client)
class CMapStaticBatchObject;
struct MAP_STATIC_BATCH_FRAME_STATE;
struct MAP_CHUNK_CLAIM;
struct MAP_CAMERA_CULL_SNAPSHOT;

// 기존 MapPlacementRuntime이 소유하는 제출 조정자다. 모델/재질/placement 정본은
// 기존 batch에 남고, 같은 불변 geometry/재질을 쓰는 떨어진 cell들의 draw만 모은다.
class CMapGpuScene final : public std::enable_shared_from_this<CMapGpuScene>
{
public:
    static std::shared_ptr<CMapGpuScene> Create(
        std::span<const std::shared_ptr<CMapStaticBatchObject>> batches,
        const std::shared_ptr<MAP_STATIC_BATCH_FRAME_STATE>& frameState);
    ~CMapGpuScene();
    bool Can_Use(const CMapStaticBatchObject& batch) const;
    bool Is_Ready(const CMapStaticBatchObject& batch) const;
    void Mark_Submitted(CMapStaticBatchObject& batch);
    // HLOD 후보가 이번 카메라의 cohort/mesh 원본 명령을 없애는지 조회한다.
    // 다른 chunk의 lazy claim을 깨우지 않으며 아직 publish하지 않은 후보만 읽는다.
    bool Can_ReplaceHlodMesh(const CMapStaticBatchObject& batch, uint32_t mesh,
        const MAP_CHUNK_CLAIM& claim, std::span<const uint8_t> candidates,
        const MAP_CAMERA_CULL_SNAPSHOT& camera) const;
    // GPU의 인스턴스별 LOD와 CPU fallback 중 더 작은 index 수를 HLOD가 넘지 않게 한다.
    uint32_t Get_HlodMeshIndexLimit(const CMapStaticBatchObject& batch, uint32_t mesh) const;
    void Invalidate() noexcept;
    // S_FALSE는 아직 어떤 GPU draw도 제출하지 않은 CPU fallback 계약이다.
    HRESULT Try_Render(CMapStaticBatchObject& batch);
private:
    CMapGpuScene() = default;
    struct MEMBER { std::weak_ptr<CMapStaticBatchObject> Batch; uint32_t Bank = 0u, ActiveFirst = 0u; };
    struct DRAW { uint32_t Mesh = 0u, Lod = 0u, Bin = 0u, IndexCount = 0u; };
    struct COHORT
    {
        std::vector<MEMBER> Members;
        std::vector<std::shared_ptr<Engine::CModel>> Banks;
        std::vector<DRAW> Draws;
        // owner thread가 mask 준비 후 확정한다. 원본이 전부 HLOD로 옮겨진 mesh는
        // 빈 indirect 명령도 제출하지 않으며 다른 mesh/cohort의 GPU 제출은 유지한다.
        std::vector<uint8_t> ActiveMeshes;
        uint64_t DrawnFrame = 0u;
        bool Active = false;
        HRESULT FrameResult = S_OK;
    };
    HRESULT Build_Resident();
    HRESULT Prepare_Frame(const Engine::CGpuSceneVisibility::FRAME& frame);
    bool Enabled() const;
    static bool Eligible(const CMapStaticBatchObject& batch);
    std::vector<COHORT> m_Cohorts;
    std::shared_ptr<Engine::CGpuSceneVisibility> m_Gpu;
    std::shared_ptr<MAP_STATIC_BATCH_FRAME_STATE> m_FrameState;
    std::vector<uint32_t> m_Active;
    uint64_t m_Frame = 0u, m_SettingsRevision = 0u, m_OptimizationRevision = 0u;
    bool m_ResidentValid = false;
};
NS_END
