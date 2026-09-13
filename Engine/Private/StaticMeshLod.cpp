#include "../ThirdPartyLib/meshoptimizer/meshoptimizer.h"
#include "Engine_VertexTypes.h"
#include "StaticMeshLod.h"

#include <algorithm>
#include <cmath>
#include <d3dcompiler.h>
#include <filesystem>
#include <limits>
#include <mutex>
#include <unordered_map>

NS_BEGIN(Engine)

struct STATIC_MESH_LOD_PROGRAM final
{
    ComPtr<ID3D11ComputeShader> shader;
};

namespace
{
    HRESULT LoadProgram(ID3D11Device* device,
        std::shared_ptr<STATIC_MESH_LOD_PROGRAM>& result)
    {
        static std::mutex mutex;
        static std::unordered_map<ID3D11Device*, std::weak_ptr<STATIC_MESH_LOD_PROGRAM>> programs;
        std::lock_guard lock(mutex);
        if (auto existing = programs[device].lock())
        {
            result = std::move(existing);
            return S_OK;
        }
        wchar_t module[32768]{};
        const DWORD length = GetModuleFileNameW(nullptr, module, _countof(module));
        if (0u == length || length >= _countof(module)) return E_FAIL;
        const auto path = std::filesystem::path(module).parent_path() / L"Shader_MeshLod.cso";
        std::error_code error;
        const auto bytes = std::filesystem::file_size(path, error);
        if (error || bytes == 0u || bytes > 1024u * 1024u) return HRESULT_FROM_WIN32(ERROR_FILE_INVALID);
        ComPtr<ID3DBlob> code;
        HRESULT hr = D3DReadFileToBlob(path.c_str(), &code);
        if (FAILED(hr)) return hr;
        auto staged = std::make_shared<STATIC_MESH_LOD_PROGRAM>();
        hr = device->CreateComputeShader(code->GetBufferPointer(), code->GetBufferSize(), nullptr, &staged->shader);
        if (FAILED(hr)) return hr;
        programs[device] = staged;
        result = std::move(staged);
        return S_OK;
    }

    struct alignas(16) SELECTION_CONSTANTS final
    {
        uint32_t ranges[3][4]{};
        float4_t errorsAndScale{};
        float4_t viewBounds{};
        float4_t projectionNearError{};
        uint32_t instanceAndRangeCount[4]{};
    };
    static_assert(sizeof(SELECTION_CONSTANTS) == 112u);
}

HRESULT CStaticMeshLod::Create(ID3D11Device* device, std::span<const VTXMESH> vertices,
    std::span<const uint32_t> indices, std::shared_ptr<CStaticMeshLod>& result)
{
    result.reset();
    // Small meshes keep the original direct draw; dispatch/indirect overhead can
    // exceed their vertex work. Generation is also bounded at asset load time.
    constexpr size_t minimumIndices = 24576u;
    constexpr size_t maximumIndices = 3u * 1024u * 1024u;
    if (!device || vertices.empty() || indices.size() % 3u != 0u) return E_INVALIDARG;
    if (indices.size() < minimumIndices || indices.size() > maximumIndices ||
        vertices.size() > 1024u * 1024u || device->GetFeatureLevel() < D3D_FEATURE_LEVEL_11_0)
        return S_FALSE;
    try
    {
        // All source shading channels participate in the error metric. The
        // simplifier keeps the original vertices and locks mesh boundaries.
        constexpr size_t attributeCount = 19u;
        const float weights[attributeCount] = {
            1.f, 1.f, 1.f, .5f, .5f, .5f, .5f, .5f, .5f,
            100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 1.f, 1.f, 1.f, 1.f };
        std::vector<std::array<float, attributeCount>> attributes(vertices.size());
        for (size_t i = 0; i < vertices.size(); ++i)
        {
            const auto& v = vertices[i];
            auto& a = attributes[i];
            a = { v.vNormal.x, v.vNormal.y, v.vNormal.z,
                v.vTangent.x, v.vTangent.y, v.vTangent.z,
                v.vBinormal.x, v.vBinormal.y, v.vBinormal.z,
                v.vTexcoord.x, v.vTexcoord.y, v.vTexcoord1.x, v.vTexcoord1.y,
                v.vTexcoord2.x, v.vTexcoord2.y,
                float(v.color0Rgba8 & 255u) / 255.f,
                float((v.color0Rgba8 >> 8u) & 255u) / 255.f,
                float((v.color0Rgba8 >> 16u) & 255u) / 255.f,
                float((v.color0Rgba8 >> 24u) & 255u) / 255.f };
            if (!std::isfinite(v.vPosition.x) || !std::isfinite(v.vPosition.y) ||
                !std::isfinite(v.vPosition.z) ||
                std::any_of(a.begin(), a.end(), [](float f) { return !std::isfinite(f); }))
                return E_INVALIDARG;
        }
        if (std::any_of(indices.begin(), indices.end(), [&](uint32_t i) { return i >= vertices.size(); }))
            return E_INVALIDARG;
        const float scale = meshopt_simplifyScale(&vertices[0].vPosition.x, vertices.size(), sizeof(VTXMESH));
        if (!std::isfinite(scale) || scale <= 0.f) return S_FALSE;
        auto staged = std::make_shared<CStaticMeshLod>();
        staged->m_Ranges[0] = { static_cast<uint32_t>(indices.size()), 0u, 0.f };
        staged->m_RangeCount = 1u;
        std::vector<uint32_t> combined(indices.begin(), indices.end()), reduced(indices.size());
        for (uint32_t level = 1u; level < 3u; ++level)
        {
            const float fraction = level == 1u ? .55f : .30f;
            const size_t target = (static_cast<size_t>(indices.size() * fraction) / 3u) * 3u;
            float error = 0.f;
            const size_t count = meshopt_simplifyWithAttributes(reduced.data(), indices.data(), indices.size(),
                &vertices[0].vPosition.x, vertices.size(), sizeof(VTXMESH),
                attributes[0].data(), sizeof(attributes[0]), weights, attributeCount, nullptr,
                target, level == 1u ? .001f : .002f, meshopt_SimplifyLockBorder, &error);
            if (count == 0u || count % 3u != 0u || !std::isfinite(error) || error < 0.f ||
                count >= staged->m_Ranges[staged->m_RangeCount - 1u].count * .85)
                continue;
            if (std::any_of(reduced.begin(), reduced.begin() + count,
                [&](uint32_t i) { return i >= vertices.size(); })) return E_UNEXPECTED;
            staged->m_Ranges[staged->m_RangeCount++] = {
                static_cast<uint32_t>(count), static_cast<uint32_t>(combined.size()), error * scale };
            combined.insert(combined.end(), reduced.begin(), reduced.begin() + count);
        }
        if (staged->m_RangeCount == 1u) return S_FALSE;
        HRESULT hr = LoadProgram(device, staged->m_Program);
        if (FAILED(hr)) return hr;
        D3D11_BUFFER_DESC desc{};
        desc.ByteWidth = static_cast<UINT>(combined.size() * sizeof(uint32_t));
        desc.Usage = D3D11_USAGE_IMMUTABLE;
        desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        D3D11_SUBRESOURCE_DATA initial{};
        initial.pSysMem = combined.data();
        hr = device->CreateBuffer(&desc, &initial, &staged->m_IndexBuffer);
        if (FAILED(hr)) return hr;
        desc = {};
        desc.ByteWidth = sizeof(SELECTION_CONSTANTS);
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        hr = device->CreateBuffer(&desc, nullptr, &staged->m_Constants);
        if (FAILED(hr)) return hr;
        desc = {};
        desc.ByteWidth = sizeof(D3D11_DRAW_INDEXED_INSTANCED_INDIRECT_ARGS);
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
        desc.MiscFlags = D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS | D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
        hr = device->CreateBuffer(&desc, nullptr, &staged->m_DrawArguments);
        if (FAILED(hr)) return hr;
        D3D11_UNORDERED_ACCESS_VIEW_DESC uav{};
        uav.Format = DXGI_FORMAT_R32_TYPELESS;
        uav.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
        uav.Buffer.NumElements = desc.ByteWidth / sizeof(uint32_t);
        uav.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_RAW;
        hr = device->CreateUnorderedAccessView(staged->m_DrawArguments.Get(), &uav, &staged->m_ArgumentsUAV);
        if (FAILED(hr)) return hr;
        result = std::move(staged);
        return S_OK;
    }
    catch (const std::bad_alloc&) { return E_OUTOFMEMORY; }
    catch (const std::length_error&) { return E_OUTOFMEMORY; }
}

HRESULT CStaticMeshLod::Prepare(ID3D11DeviceContext* context, const MESH_SCREEN_LOD_DESC& view,
    uint32_t instanceCount)
{
    const float values[] = { view.viewBounds.x, view.viewBounds.y, view.viewBounds.z,
        view.viewBounds.w, view.projectionPixels.x, view.projectionPixels.y,
        view.maximumScale, view.nearPlane, view.maximumPixelError };
    if (!context || !m_Program || !instanceCount || m_RangeCount < 2u ||
        std::any_of(std::begin(values), std::end(values), [](float f) { return !std::isfinite(f); }) ||
        view.viewBounds.w <= 0.f || view.projectionPixels.x <= 0.f || view.projectionPixels.y <= 0.f ||
        view.maximumScale <= 0.f || view.nearPlane <= 0.f ||
        view.maximumPixelError <= 0.f || view.maximumPixelError > 1.f)
        return E_INVALIDARG;
    SELECTION_CONSTANTS constants{};
    for (uint32_t i = 0u; i < m_RangeCount; ++i)
    {
        constants.ranges[i][0] = m_Ranges[i].count;
        constants.ranges[i][1] = m_Ranges[i].first;
        (&constants.errorsAndScale.x)[i] = m_Ranges[i].error;
    }
    constants.errorsAndScale.w = view.maximumScale;
    constants.viewBounds = view.viewBounds;
    constants.projectionNearError = { view.projectionPixels.x, view.projectionPixels.y,
        view.nearPlane, view.maximumPixelError };
    constants.instanceAndRangeCount[0] = instanceCount;
    constants.instanceAndRangeCount[1] = m_RangeCount;
    context->UpdateSubresource(m_Constants.Get(), 0u, nullptr, &constants, 0u, 0u);
    ID3D11Buffer* buffer = m_Constants.Get();
    ID3D11UnorderedAccessView* uav = m_ArgumentsUAV.Get();
    context->CSSetConstantBuffers(0u, 1u, &buffer);
    context->CSSetUnorderedAccessViews(0u, 1u, &uav, nullptr);
    context->CSSetShader(m_Program->shader.Get(), nullptr, 0u);
    context->Dispatch(1u, 1u, 1u);
    // End ownership before the arguments become an indirect-draw input.
    uav = nullptr;
    buffer = nullptr;
    context->CSSetUnorderedAccessViews(0u, 1u, &uav, nullptr);
    context->CSSetConstantBuffers(0u, 1u, &buffer);
    context->CSSetShader(nullptr, nullptr, 0u);
    return S_OK;
}

NS_END
