#pragma once
#include "Client_Defines.h"
#include "MapAssetCatalog.h"
#include "Model.h"
#include <memory>
#include <span>

NS_BEGIN(Engine)
class CShader;
NS_END
NS_BEGIN(Client)
// A preparation-session owner. No scene files or global rendering options change.
class CMapStaticProxyBaker final
{
public:
    static constexpr const wchar_t* ShaderTag = L"Prototype_Component_Shader_VtxMeshMapProxy";
    CMapStaticProxyBaker(ComPtr<ID3D11Device> device, ComPtr<ID3D11DeviceContext> context);
    ~CMapStaticProxyBaker();
    HRESULT Prepare(std::span<const Engine::CModel::STATIC_CLUSTER_SOURCE> sources,
        std::span<const MAP_ASSET_RENDER_PROFILE> profiles,
        const std::shared_ptr<Engine::CModel>& proxy, std::string& error);
    static HRESULT Bind(const std::shared_ptr<Engine::CShader>& shader,
        const std::shared_ptr<Engine::CModel>& proxy);
    static uint64_t Estimate_AtlasBytes(const Engine::CModel::STATIC_PROXY_STATS& stats);
    uint32_t Get_CacheHits() const;
    uint32_t Get_BakeCount() const;
    // cutscene 선택자는 실제 검증 카메라와 같은 보수적 구면 직경식을 소비한다.
    struct REPLACEMENT_LIMITS final
    {
        float maximumProjectedDiameterPixels = 0.f;
        float minimumSurfaceDistanceMetres = 0.f;
    };
    // optional package index만 조회한다. 미지원 18,550 batch의 placement payload 할당을 피한다.
    // 선택적인 error 출력은 index 부재/손상을 미지원 asset과 구분하기 위한 로딩 시 진단이다.
    bool Has_Replacement(const std::string& assetId, std::string* error = nullptr);
    // 모든 원본 mesh/placement를 표현할 자원과 native 재질 출처를 검증한 뒤 한 번에 commit한다.
    // S_FALSE는 미지원 모델/재질, 실패 HRESULT는 손상/변경된 package·의존 파일/자원 실패이며 출력은 보존한다.
    HRESULT Prepare_Replacement(const std::shared_ptr<Engine::CModel>& originalModel,
        const MAP_ASSET_ENTRY& asset,
        std::span<const Engine::CModel::STATIC_PROXY_PLACEMENT_SOURCE> sources,
        std::shared_ptr<Engine::CModel>& outputProxy, std::shared_ptr<Engine::CShader>& outputShader,
        REPLACEMENT_LIMITS& outputLimits, std::string& error);
    // main thread의 최종 선택 preflight와 실제 draw가 같은 SRV/instance/live RNM 입력을 쓴다.
    static HRESULT Bind_Replacement(const std::shared_ptr<Engine::CShader>& shader,
        const std::shared_ptr<Engine::CModel>& proxy, const std::shared_ptr<Engine::CModel>& originalModel);
private:
    struct STATE;
    std::unique_ptr<STATE> m_State;
};
NS_END
