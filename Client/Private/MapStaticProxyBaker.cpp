#include "MapStaticProxyBaker.h"
#include "MapAssetRenderUtils.h"
#include "Shader.h"
#include "BinaryAsset/ModelAssetData.h"
#pragma push_macro("new")
#undef new
#include <d3d11_1.h>
#include <bcrypt.h>
#include <array>
#include <filesystem>
#include <fstream>
#include <map>
#include <algorithm>
#include <cstring>
#pragma pop_macro("new")
#pragma comment(lib, "bcrypt.lib")

namespace
{
    using Digest = std::array<uint8_t, 32>;
    constexpr std::array<DXGI_FORMAT, 6> Formats = { DXGI_FORMAT_R8G8B8A8_UNORM,
        DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_R16G16B16A16_FLOAT,
        DXGI_FORMAT_R11G11B10_FLOAT, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R11G11B10_FLOAT };
    constexpr std::array<uint32_t, 6> PixelBytes = { 4, 8, 8, 4, 4, 4 };
    constexpr std::array<const char*, 6> Names = { "g_MapProxyDiffuseAtlas", "g_MapProxyNormalAtlas",
        "g_MapProxySpecularAtlas", "g_MapProxyIndirectAtlas", "g_MapProxyAverageAtlas", "g_MapProxyDirectionalAtlas" };
    constexpr uint32_t MipCount = 4;
    using Pixels = std::array<std::vector<uint8_t>, 6>;

    class Hash final
    {
        BCRYPT_ALG_HANDLE algorithm = nullptr;
        BCRYPT_HASH_HANDLE hash = nullptr;
        std::vector<uint8_t> object;
        bool valid = false;
    public:
        Hash()
        {
            DWORD bytes = 0, written = 0;
            if (BCryptOpenAlgorithmProvider(&algorithm, BCRYPT_SHA256_ALGORITHM, nullptr, 0) < 0 ||
                BCryptGetProperty(algorithm, BCRYPT_OBJECT_LENGTH, reinterpret_cast<PUCHAR>(&bytes),
                    sizeof(bytes), &written, 0) < 0) return;
            object.resize(bytes);
            valid = BCryptCreateHash(algorithm, &hash, object.data(), bytes, nullptr, 0, 0) >= 0;
        }
        ~Hash() { if (hash) BCryptDestroyHash(hash); if (algorithm) BCryptCloseAlgorithmProvider(algorithm, 0); }
        void Add(const void* p, size_t bytes)
        {
            if (!valid || bytes > 0xffffffffull) { valid = false; return; }
            if (bytes) valid = BCryptHashData(hash, static_cast<PUCHAR>(const_cast<void*>(p)), static_cast<ULONG>(bytes), 0) >= 0;
        }
        template<class T> void Add(const T& value) { Add(&value, sizeof(value)); }
        bool Finish(Digest& result) { return valid && BCryptFinishHash(hash, result.data(), ULONG(result.size()), 0) >= 0; }
    };

    std::string Hex(const Digest& digest)
    {
        std::string result; result.reserve(64);
        for (uint8_t byte : digest) { result += "0123456789abcdef"[byte >> 4]; result += "0123456789abcdef"[byte & 15]; }
        return result;
    }

    std::filesystem::path ModuleDirectory()
    {
        wchar_t path[32768]{};
        const DWORD length = GetModuleFileNameW(nullptr, path, DWORD(std::size(path)));
        return length && length < std::size(path) ? std::filesystem::path(path).parent_path() : std::filesystem::path{};
    }

    std::filesystem::path CacheDirectory()
    {
        wchar_t path[32768]{};
        const DWORD length = GetEnvironmentVariableW(L"LOCALAPPDATA", path, DWORD(std::size(path)));
        return length && length < std::size(path) ? std::filesystem::path(path) / L"LostArk/MapProxy/Atlas-v1" : std::filesystem::path{};
    }

    // Swap the complete D3D11 state, including caller render targets and resources.
    // No partial state snapshot can silently leave the loading renderer altered.
    class ContextState final
    {
        ComPtr<ID3D11DeviceContext1> context;
        ComPtr<ID3DDeviceContextState> saved;
    public:
        bool Begin(ID3D11Device* device, ID3D11DeviceContext* original)
        {
            ComPtr<ID3D11Device1> device1;
            ComPtr<ID3DDeviceContextState> isolated;
            const D3D_FEATURE_LEVEL level = device->GetFeatureLevel();
            D3D_FEATURE_LEVEL selected{};
            if (original->GetType() != D3D11_DEVICE_CONTEXT_IMMEDIATE ||
                FAILED(device->QueryInterface(IID_PPV_ARGS(&device1))) ||
                FAILED(original->QueryInterface(IID_PPV_ARGS(&context))) ||
                FAILED(device1->CreateDeviceContextState(0, &level, 1, D3D11_SDK_VERSION,
                    __uuidof(ID3D11Device), &selected, &isolated))) return false;
            context->SwapDeviceContextState(isolated.Get(), &saved);
            return saved != nullptr;
        }
        ~ContextState() { if (saved) { context->ClearState(); context->SwapDeviceContextState(saved.Get(), nullptr); } }
    };

    bool ReadCache(const std::filesystem::path& path, uint32_t width, uint32_t height, Pixels& pixels)
    {
        if (path.empty()) return false;
        std::ifstream input(path, std::ios::binary);
        const std::array<uint32_t, 4> expected{0x5850424d, 1, width, height};
        std::array<uint32_t, 4> header{}; Digest stored{}, actual{};
        input.read(reinterpret_cast<char*>(header.data()), sizeof(header));
        input.read(reinterpret_cast<char*>(stored.data()), stored.size());
        if (!input || header != expected) return false;
        Hash hash;
        for (size_t i = 0; i < pixels.size(); ++i)
        {
            pixels[i].resize(size_t(width) * height * PixelBytes[i]);
            input.read(reinterpret_cast<char*>(pixels[i].data()), pixels[i].size());
            if (!input) return false;
            hash.Add(pixels[i].data(), pixels[i].size());
        }
        return input.peek() == std::char_traits<char>::eof() && hash.Finish(actual) && actual == stored;
    }

    void WriteCache(const std::filesystem::path& path, uint32_t width, uint32_t height, const Pixels& pixels)
    {
        if (path.empty()) return;
        std::error_code ec;
        std::filesystem::create_directories(path.parent_path(), ec);
        if (ec) return;
        // Derived cache never replaces an authoring or Resources file.
        const auto temporary = std::filesystem::path(path.wstring() + L"." + std::to_wstring(GetCurrentProcessId()) +
            L"." + std::to_wstring(GetTickCount64()) + L".tmp");
        Digest digest{}; Hash hash;
        for (const auto& plane : pixels) hash.Add(plane.data(), plane.size());
        if (!hash.Finish(digest)) return;
        const std::array<uint32_t, 4> header{0x5850424d, 1, width, height};
        std::ofstream output(temporary, std::ios::binary | std::ios::trunc);
        output.write(reinterpret_cast<const char*>(header.data()), sizeof(header));
        output.write(reinterpret_cast<const char*>(digest.data()), digest.size());
        for (const auto& plane : pixels) output.write(reinterpret_cast<const char*>(plane.data()), plane.size());
        output.flush(); const bool written = bool(output); output.close();
        if (!written || !MoveFileExW(temporary.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
            std::filesystem::remove(temporary, ec);
    }

    // Copy all six channels from the same nearest reached valid texel. Half-float
    // diffuseScale is >= 1 on covered pixels and zero in the cleared background.
    void Dilate(Pixels& pixels, uint32_t width, uint32_t height)
    {
        const uint32_t count = width * height;
        std::vector<uint8_t> distance(count, 255);
        std::vector<uint32_t> queue; queue.reserve(count);
        for (uint32_t i = 0; i < count; ++i)
        {
            uint16_t scale = 0; std::memcpy(&scale, pixels[2].data() + size_t(i) * 8 + 6, 2);
            if (scale >= 0x3c00 && scale < 0x7c00) distance[i] = 0;
        }
        for (uint32_t i = 0; i < count; ++i)
        {
            if (distance[i]) continue;
            const uint32_t x = i % width, y = i / width;
            if ((x && distance[i - 1]) || (x + 1 < width && distance[i + 1]) ||
                (y && distance[i - width]) || (y + 1 < height && distance[i + width])) queue.push_back(i);
        }
        for (size_t head = 0; head < queue.size(); ++head)
        {
            const uint32_t from = queue[head];
            if (distance[from] >= 8) continue;
            const int x = int(from % width), y = int(from / width);
            for (int dy = -1; dy <= 1; ++dy) for (int dx = -1; dx <= 1; ++dx)
            {
                if ((!dx && !dy) || x + dx < 0 || y + dy < 0 || x + dx >= int(width) || y + dy >= int(height)) continue;
                const uint32_t to = uint32_t(y + dy) * width + uint32_t(x + dx);
                if (distance[to] != 255) continue;
                distance[to] = uint8_t(distance[from] + 1);
                for (size_t p = 0; p < pixels.size(); ++p)
                    std::memcpy(pixels[p].data() + size_t(to) * PixelBytes[p], pixels[p].data() + size_t(from) * PixelBytes[p], PixelBytes[p]);
                queue.push_back(to);
            }
        }
    }
}

struct Client::CMapStaticProxyBaker::STATE final
{
    ComPtr<ID3D11Device> device;
    ComPtr<ID3D11DeviceContext> context;
    std::shared_ptr<Engine::CShader> shader;
    std::map<std::filesystem::path, Digest> fileHashes;
    uint32_t hits = 0, bakes = 0;
    bool FileDigest(const std::filesystem::path& path, Digest& digest)
    {
        const auto found = fileHashes.find(path);
        if (found != fileHashes.end()) { digest = found->second; return true; }
        std::ifstream input(path, std::ios::binary);
        if (!input) return false;
        Hash hash; std::array<char, 65536> buffer{};
        while (input) { input.read(buffer.data(), buffer.size()); hash.Add(buffer.data(), size_t(input.gcount())); }
        if (!input.eof() || !hash.Finish(digest)) return false;
        fileHashes.emplace(path, digest); return true;
    }
};

Client::CMapStaticProxyBaker::CMapStaticProxyBaker(ComPtr<ID3D11Device> device, ComPtr<ID3D11DeviceContext> context)
    : m_State(std::make_unique<STATE>()) { m_State->device = device; m_State->context = context; }
Client::CMapStaticProxyBaker::~CMapStaticProxyBaker() = default;
uint32_t Client::CMapStaticProxyBaker::Get_CacheHits() const { return m_State->hits; }
uint32_t Client::CMapStaticProxyBaker::Get_BakeCount() const { return m_State->bakes; }

uint64_t Client::CMapStaticProxyBaker::Estimate_AtlasBytes(const Engine::CModel::STATIC_PROXY_STATS& stats)
{
    uint64_t total = 0;
    for (uint32_t mip = 0; mip < MipCount; ++mip)
        total += uint64_t((std::max)(1u, stats.atlasWidth >> mip)) * (std::max)(1u, stats.atlasHeight >> mip) * 32u;
    return total;
}

HRESULT Client::CMapStaticProxyBaker::Bind(const std::shared_ptr<Engine::CShader>& shader,
    const std::shared_ptr<Engine::CModel>& proxy)
{
    if (!shader || !proxy) return E_INVALIDARG;
    const HRESULT hr = proxy->Bind_StaticProxySources(shader);
    return FAILED(hr) ? hr : proxy->Bind_StaticProxyAtlases(shader, Names);
}

HRESULT Client::CMapStaticProxyBaker::Prepare(std::span<const Engine::CModel::STATIC_CLUSTER_SOURCE> sources,
    std::span<const MAP_ASSET_RENDER_PROFILE> profiles, const std::shared_ptr<Engine::CModel>& proxy, std::string& error)
{
    error.clear();
    const auto fail = [&](const char* reason) { error = reason; return E_FAIL; };
    const auto* stats = proxy ? proxy->Get_StaticProxyStats() : nullptr;
    if (!stats || sources.empty() || stats->sourceCount != sources.size() ||
        (profiles.size() != 1 && profiles.size() != sources.size()) ||
        stats->atlasWidth < 8 || stats->atlasHeight < 8 || stats->atlasWidth > 2048 || stats->atlasHeight > 2048)
        return fail("Invalid proxy atlas input");
    try
    {
        auto& state = *m_State;
        std::vector<Engine::CModel::STATIC_PROXY_METADATA> metadata;
        metadata.reserve(sources.size());
        Hash keyHash; Digest key{}, fileDigest{};
        const uint32_t version = 1;
        keyHash.Add(version); keyHash.Add(proxy->Get_StaticProxyGeometryIdentity());
        bool cacheable = state.FileDigest(ModuleDirectory() / L"Shader_VtxMeshMapProxy.cso", fileDigest);
        keyHash.Add(fileDigest);
        for (size_t i = 0; i < sources.size(); ++i)
        {
            const auto& source = sources[i];
            const auto* surface = source.model ? source.model->Get_MaterialSurface(source.meshIndex) : nullptr;
            if (!surface || !source.model->Can_BakeStaticProxy(source.meshIndex) ||
                !source.model->Validate_StaticProxySourceFiles(source.meshIndex)) return fail("Unsupported proxy material");
            metadata.push_back({ surface->sourceBgFlags & 1023u, (surface->hasBakedLighting && source.instance.vLightmapAverageScale.w != 0.f ? 1u : 0u) |
                (surface->hasStaticShadow ? 2u : 0u) | ((surface->staticShadowChannel & 15u) << 2u) });
            // This POD's padding may cause a harmless cache miss, never a stale hit.
            keyHash.Add(*surface); keyHash.Add(source.instance);
            const auto& profile = profiles[profiles.size() == 1 ? 0 : i];
            keyHash.Add(profile.renderMode); keyHash.Add(profile.cullMode); keyHash.Add(profile.opacity);
            for (const auto& path : source.model->Get_StaticProxySourcePaths(source.meshIndex))
            {
                if (path.empty()) continue;
                if (!state.FileDigest(path, fileDigest)) cacheable = false;
                keyHash.Add(fileDigest);
                const auto name = path.generic_u8string(); keyHash.Add(name.data(), name.size());
            }
        }
        cacheable = keyHash.Finish(key) && cacheable;
        const auto directory = CacheDirectory();
        const auto cachePath = cacheable && !directory.empty() ? directory / (Hex(key) + ".bin") : std::filesystem::path{};
        const uint32_t width = stats->atlasWidth, height = stats->atlasHeight;
        Pixels pixels;
        const bool cached = ReadCache(cachePath, width, height, pixels);
        ContextState isolated;
        if (!isolated.Begin(state.device.Get(), state.context.Get())) return fail("Proxy requires isolated D3D11.1 immediate context");
        if (!cached)
        {
            if (!state.shader)
            {
                std::array<D3D11_INPUT_ELEMENT_DESC, VTXMESH::iNumElements + 2> elements{};
                std::copy(std::begin(VTXMESH::Elements), std::end(VTXMESH::Elements), elements.begin());
                elements[VTXMESH::iNumElements] = { "ATLASUV", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 };
                elements.back() = { "SOURCEINDEX", 0, DXGI_FORMAT_R32_UINT, 1, 8, D3D11_INPUT_PER_VERTEX_DATA, 0 };
                state.shader = Engine::CShader::Create(state.device, state.context, L"Shader_VtxMeshMapProxy.hlsl", elements.data(), uint32_t(elements.size()));
                if (!state.shader) return fail("Proxy bake shader unavailable");
            }
            std::array<ComPtr<ID3D11Texture2D>, 6> targets;
            std::array<ComPtr<ID3D11RenderTargetView>, 6> views;
            std::array<ID3D11RenderTargetView*, 6> raw{};
            for (size_t p = 0; p < targets.size(); ++p)
            {
                D3D11_TEXTURE2D_DESC desc{};
                desc.Width = width; desc.Height = height; desc.MipLevels = desc.ArraySize = 1;
                desc.Format = Formats[p]; desc.SampleDesc.Count = 1;
                desc.BindFlags = D3D11_BIND_RENDER_TARGET;
                if (FAILED(state.device->CreateTexture2D(&desc, nullptr, &targets[p])) ||
                    FAILED(state.device->CreateRenderTargetView(targets[p].Get(), nullptr, &views[p]))) return fail("Proxy bake target allocation failed");
                const float clear[4]{}; state.context->ClearRenderTargetView(views[p].Get(), clear); raw[p] = views[p].Get();
            }
            const D3D11_VIEWPORT viewport{0, 0, float(width), float(height), 0, 1};
            state.context->RSSetViewports(1, &viewport);
            state.context->OMSetRenderTargets(uint32_t(raw.size()), raw.data(), nullptr);
            float4x4_t identity; XMStoreFloat4x4(&identity, XMMatrixIdentity());
            const float4_t camera{};
            if (FAILED(state.shader->Bind_Matrix("g_ViewMatrix", &identity)) || FAILED(state.shader->Bind_Matrix("g_ProjMatrix", &identity)) ||
                FAILED(state.shader->Bind_RawValue("g_vCamPosition", &camera, sizeof(camera))) ||
                FAILED(proxy->Set_StaticProxyMetadata(metadata)) || FAILED(proxy->Bind_StaticProxySources(state.shader))) return fail("Proxy bake constants failed");
            for (uint32_t i = 0; i < sources.size(); ++i)
            {
                const auto& source = sources[i];
                const auto borrowed = std::shared_ptr<Engine::CModel>(const_cast<Engine::CModel*>(source.model), [](Engine::CModel*) {});
                if (FAILED(CMapAssetRenderUtils::Bind_Material(borrowed, state.shader, source.meshIndex,
                    profiles[profiles.size() == 1 ? 0 : i], 0.f, nullptr, {}, nullptr, nullptr, MAP_MATERIAL_BINDING_MODE::INSTANCED)) ||
                    FAILED(state.shader->Begin(0)) || FAILED(proxy->Render_StaticProxySource(i))) return fail("Proxy source bake failed");
            }
            state.context->OMSetRenderTargets(0, nullptr, nullptr);
            for (size_t p = 0; p < targets.size(); ++p)
            {
                D3D11_TEXTURE2D_DESC desc{}; targets[p]->GetDesc(&desc);
                desc.BindFlags = 0; desc.Usage = D3D11_USAGE_STAGING; desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
                ComPtr<ID3D11Texture2D> readback;
                if (FAILED(state.device->CreateTexture2D(&desc, nullptr, &readback))) return fail("Proxy readback allocation failed");
                state.context->CopyResource(readback.Get(), targets[p].Get());
                D3D11_MAPPED_SUBRESOURCE mapped{};
                if (FAILED(state.context->Map(readback.Get(), 0, D3D11_MAP_READ, 0, &mapped))) return fail("Proxy readback failed");
                pixels[p].resize(size_t(width) * height * PixelBytes[p]);
                for (uint32_t y = 0; y < height; ++y)
                    std::memcpy(pixels[p].data() + size_t(y) * width * PixelBytes[p],
                        static_cast<const uint8_t*>(mapped.pData) + size_t(y) * mapped.RowPitch, size_t(width) * PixelBytes[p]);
                state.context->Unmap(readback.Get(), 0);
            }
            Dilate(pixels, width, height);
            for (const auto& source : sources)
                if (!source.model->Validate_StaticProxySourceFiles(source.meshIndex)) return fail("Source textures changed during proxy preparation");
            WriteCache(cachePath, width, height, pixels);
        }
        std::array<ComPtr<ID3D11ShaderResourceView>, 6> atlases;
        for (size_t p = 0; p < atlases.size(); ++p)
        {
            UINT support = 0;
            if (FAILED(state.device->CheckFormatSupport(Formats[p], &support)) || !(support & D3D11_FORMAT_SUPPORT_MIP_AUTOGEN))
                return fail("Proxy atlas format lacks mip generation");
            D3D11_TEXTURE2D_DESC desc{};
            desc.Width = width; desc.Height = height; desc.MipLevels = MipCount; desc.ArraySize = 1;
            desc.Format = Formats[p]; desc.SampleDesc.Count = 1;
            desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
            desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
            ComPtr<ID3D11Texture2D> texture;
            if (FAILED(state.device->CreateTexture2D(&desc, nullptr, &texture)) ||
                FAILED(state.device->CreateShaderResourceView(texture.Get(), nullptr, &atlases[p]))) return fail("Proxy atlas allocation failed");
            state.context->UpdateSubresource(texture.Get(), 0, nullptr, pixels[p].data(), width * PixelBytes[p], 0);
            state.context->GenerateMips(atlases[p].Get());
        }
        if (FAILED(proxy->Set_StaticProxyMetadata(metadata)) || FAILED(proxy->Set_StaticProxyAtlases(atlases)))
            return fail("Proxy atlas commit failed");
        if (cached) ++state.hits; else ++state.bakes;
        return S_OK;
    }
    catch (...) { return fail("Proxy atlas preparation failed"); }
}
