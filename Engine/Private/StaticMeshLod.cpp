#include "../ThirdPartyLib/meshoptimizer/meshoptimizer.h"
#include "Engine_VertexTypes.h"
#include "StaticMeshLod.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>

NS_BEGIN(Engine)

HRESULT CStaticMeshLod::Create(ID3D11Device* device, std::span<const VTXMESH> vertices,
    std::span<const uint32_t> indices, std::shared_ptr<CStaticMeshLod>& result)
{
    result.reset();
    // Keep generation bounded at asset load time. Selection and drawing reuse
    // these immutable ranges without per-draw compute work or GPU readback.
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
        D3D11_BUFFER_DESC desc{};
        desc.ByteWidth = static_cast<UINT>(combined.size() * sizeof(uint32_t));
        desc.Usage = D3D11_USAGE_IMMUTABLE;
        desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        D3D11_SUBRESOURCE_DATA initial{};
        initial.pSysMem = combined.data();
        const HRESULT hr = device->CreateBuffer(&desc, &initial, &staged->m_IndexBuffer);
        if (FAILED(hr)) return hr;
        result = std::move(staged);
        return S_OK;
    }
    catch (const std::bad_alloc&) { return E_OUTOFMEMORY; }
    catch (const std::length_error&) { return E_OUTOFMEMORY; }
}

HRESULT CStaticMeshLod::Select_Range(const MESH_SCREEN_LOD_DESC& view,
    SELECTION& result) const
{
    const float values[] = { view.viewBounds.x, view.viewBounds.y, view.viewBounds.z,
        view.viewBounds.w, view.projectionPixels.x, view.projectionPixels.y,
        view.maximumScale, view.nearPlane, view.maximumPixelError };
    if (!m_IndexBuffer || m_RangeCount < 2u ||
        std::any_of(std::begin(values), std::end(values), [](float f) { return !std::isfinite(f); }) ||
        view.viewBounds.w <= 0.f || view.projectionPixels.x <= 0.f || view.projectionPixels.y <= 0.f ||
        view.maximumScale <= 0.f || view.nearPlane <= 0.f ||
        view.maximumPixelError <= 0.f || view.maximumPixelError > 1.f)
        return E_INVALIDARG;

    if (view.hasTightViewBounds &&
        (!std::isfinite(view.maximumAbsViewXY.x) || !std::isfinite(view.maximumAbsViewXY.y) ||
         !std::isfinite(view.minimumViewDepth) || view.maximumAbsViewXY.x < 0.f || view.maximumAbsViewXY.y < 0.f))
        return E_INVALIDARG;

    uint32_t selected = 0u;
    const double nearestZ = view.hasTightViewBounds ? view.minimumViewDepth :
        double(view.viewBounds.z) - view.viewBounds.w;
    if (nearestZ > view.nearPlane)
    {
        const double largestX = view.hasTightViewBounds ? view.maximumAbsViewXY.x :
            std::abs(double(view.viewBounds.x)) + view.viewBounds.w;
        const double largestY = view.hasTightViewBounds ? view.maximumAbsViewXY.y :
            std::abs(double(view.viewBounds.y)) + view.viewBounds.w;
        for (uint32_t level = 1u; level < m_RangeCount; ++level)
        {
            const double error = double(m_Ranges[level].error) * view.maximumScale;
            const double safeZ = nearestZ - error;
            if (safeZ <= view.nearPlane) continue;
            // Same conservative off-axis bound and 10% margin as the former
            // compute selector. All inputs already live on the render thread.
            const double pixelX = view.projectionPixels.x * error * (1. + largestX / nearestZ) / safeZ;
            const double pixelY = view.projectionPixels.y * error * (1. + largestY / nearestZ) / safeZ;
            if (std::isfinite(pixelX) && std::isfinite(pixelY) &&
                (std::max)(pixelX, pixelY) <= double(view.maximumPixelError) * .9)
                selected = level;
        }
    }
    result = { m_Ranges[selected].count, m_Ranges[selected].first, selected };
    return S_OK;
}

NS_END
