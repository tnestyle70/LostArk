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
private:
    struct STATE;
    std::unique_ptr<STATE> m_State;
};
NS_END
