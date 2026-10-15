#include "MapStaticProxyBaker.h"
#include "MapAssetRenderUtils.h"
#include "Shader.h"
#include "DataJson.h"
#include "RuntimeAssetRoot.h"
#include "GameInstance.h"
#include "Engine_RenderTypes.h"
#include <set>
#include <cmath>
#include <stdexcept>
#include "BinaryAsset/ModelAssetData.h"
#include <d3d11_1.h>
#include <bcrypt.h>
#include <array>
#include <filesystem>
#include <fstream>
#include <map>
#include <algorithm>
#include <cstring>
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

namespace
{
    using ReplacementJson = Client::DATA_JSON_VALUE;
    constexpr const char* ReplacementAtlasNames[] = { "g_MapProxyDiffuseAtlas", "g_MapProxyNormalAtlas",
        "g_MapProxySpecularAtlas", "g_MapProxyAverageAtlas", "g_OriginalTangent", "g_OriginalBinormal",
        "g_OriginalNormal", "g_NativeStoneExtra", "g_MapProxyUvJacobian" };
    struct ReplacementFile final { std::filesystem::path path; std::string hash; };
    struct ReplacementSourceDigest final
    {
        uintmax_t size = 0;
        std::filesystem::file_time_type writeTime{};
        Digest digest{};
    };
    std::filesystem::path ReplacementUtf8Path(const std::string& text)
    { return std::filesystem::path(std::u8string(text.begin(), text.end())); }
    struct ReplacementPackage final
    {
        std::vector<VTXMESH> vertices;
        std::vector<float2_t> uv;
        std::vector<uint32_t> indices;
        std::array<ComPtr<ID3D11ShaderResourceView>, 9> atlases;
        Client::CMapStaticProxyBaker::REPLACEMENT_LIMITS limits;
        ReplacementJson manifest;
        uint32_t width = 0, height = 0;
    };
    const ReplacementJson& ReplacementMember(const ReplacementJson& value, const char* key)
    {
        const auto* member = value.Find(key);
        if (!member) throw std::runtime_error(std::string("Missing replacement field: ") + key);
        return *member;
    }
    std::string ReplacementText(const ReplacementJson& value, const char* key)
    {
        const auto& member = ReplacementMember(value, key);
        if (!member.Is_String()) throw std::runtime_error("Replacement string expected");
        return member.Get_String();
    }
    uint32_t ReplacementUInt(const ReplacementJson& value, const char* key, uint32_t maximum)
    {
        const auto& member = ReplacementMember(value, key);
        const double number = member.Get_Number();
        if (!member.Is_Number() || !std::isfinite(number) || number < 0 || number > maximum || std::floor(number) != number)
            throw std::runtime_error("Replacement integer out of range");
        return uint32_t(number);
    }
    float ReplacementPositive(const ReplacementJson& value, const char* key)
    {
        const auto& member = ReplacementMember(value, key);
        if (!member.Is_Number() || !std::isfinite(member.Get_Number()) || member.Get_Number() <= 0 || member.Get_Number() > 1.e6)
            throw std::runtime_error("Replacement positive finite limit expected");
        return float(member.Get_Number());
    }
    const ReplacementJson::ARRAY& ReplacementArray(const ReplacementJson& value, const char* key)
    {
        const auto& member = ReplacementMember(value, key);
        if (!member.Is_Array()) throw std::runtime_error("Replacement array expected");
        return member.Get_Array();
    }
    bool ReplacementHashText(const std::string& text)
    {
        return text.size() == 64 && std::all_of(text.begin(), text.end(), [](char c) { return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'); });
    }
    std::string ReplacementRead(const std::filesystem::path& path, size_t maximum)
    {
        std::error_code ec; const auto size = std::filesystem::file_size(path, ec);
        if (ec || size > maximum) throw std::runtime_error("Replacement file unavailable or too large");
        std::ifstream input(path, std::ios::binary); std::string bytes(size_t(size), '\0');
        if (!input.read(bytes.data(), std::streamsize(bytes.size())) || input.peek() != std::char_traits<char>::eof())
            throw std::runtime_error("Replacement read failed or size changed");
        return bytes;
    }
    // 검사한 파일을 다시 여는 대신, 실제 GPU/파서에 전달할 동일 바이트의 hash를 검사한다.
    std::string ReplacementReadVerified(const ReplacementFile& file, size_t maximum)
    {
        auto bytes = ReplacementRead(file.path, maximum);
        Hash hash; Digest digest{}; hash.Add(bytes.data(), bytes.size());
        if (!hash.Finish(digest) || Hex(digest) != file.hash)
            throw std::runtime_error("Replacement payload hash changed: " + file.path.generic_string());
        return bytes;
    }
    // 원본 SRV는 CMaterial의 로드 stamp로 검증한다. 이 캐시는 같은 준비 세션에서 큰 DDS를
    // 중복 hash하지 않되, 크기/수정 시간이 달라지면 반드시 다시 읽어 원본 저작 변경을 거부한다.
    bool ReplacementSourceHash(const std::filesystem::path& path,
        std::map<std::filesystem::path, ReplacementSourceDigest>& cache, Digest& digest)
    {
        std::error_code ec;
        const auto size = std::filesystem::file_size(path, ec); if (ec) return false;
        const auto time = std::filesystem::last_write_time(path, ec); if (ec) return false;
        const auto found = cache.find(path);
        if (found != cache.end() && found->second.size == size && found->second.writeTime == time)
        { digest = found->second.digest; return true; }
        std::ifstream input(path, std::ios::binary); if (!input) return false;
        Hash hash; std::array<char, 64u * 1024u> bytes{}; uintmax_t consumed = 0;
        while (input)
        {
            input.read(bytes.data(), bytes.size()); const auto count = input.gcount();
            hash.Add(bytes.data(), size_t(count)); consumed += uintmax_t(count);
        }
        if (!input.eof() || consumed != size || !hash.Finish(digest)) return false;
        const auto afterSize = std::filesystem::file_size(path, ec); if (ec || afterSize != size) return false;
        const auto afterTime = std::filesystem::last_write_time(path, ec); if (ec || afterTime != time) return false;
        cache[path] = {size, time, digest}; return true;
    }
    ReplacementJson ReplacementParse(const std::string& bytes)
    {
        ReplacementJson value; std::string error;
        if (!Client::CDataJson::Parse(bytes, value, error) || !value.Is_Object()) throw std::runtime_error("Replacement JSON invalid: " + error);
        return value;
    }
    ReplacementFile ReplacementResource(const ReplacementJson& value)
    {
        ReplacementFile file{ Client::CRuntimeAssetRoot::Resolve(ReplacementUtf8Path(ReplacementText(value, "file"))), ReplacementText(value, "sha256") };
        if (file.path.empty() || !ReplacementHashText(file.hash)) throw std::runtime_error("Replacement resource path/hash invalid");
        return file;
    }
    std::string ReplacementRelative(const std::filesystem::path& path)
    {
        if (path.empty()) return {};
        const auto relative = path.lexically_normal().lexically_relative(Client::CRuntimeAssetRoot::Get().lexically_normal());
        const auto resolved = Client::CRuntimeAssetRoot::Resolve(relative);
        if (resolved.empty() || resolved.lexically_normal() != path.lexically_normal()) throw std::runtime_error("Replacement source outside Resources");
        return relative.generic_string();
    }
    void ReplacementHashString(Hash& hash, const std::string& value)
    {
        const auto length = uint32_t(value.size()); hash.Add(length); hash.Add(value.data(), value.size());
    }
    // 구조체 padding이나 RNM placement 값을 공유 키에 넣지 않는다. 이름/순서/버전은 저장 ABI다.
    bool ReplacementIdentity(const Engine::CModel& model, const Client::MAP_ASSET_ENTRY& asset, Digest& digest)
    {
        Hash hash; hash.Add(uint32_t(2));
        const auto& p = asset.renderProfile;
        hash.Add(p.renderMode); hash.Add(p.cullMode); hash.Add(p.uvScale); hash.Add(p.uvSpeed);
        hash.Add(p.opacity); hash.Add(p.opacityPower); hash.Add(p.emissiveIntensity); hash.Add(p.specularIntensity);
        hash.Add(p.specularPower); hash.Add(p.colorTint); hash.Add(p.triplanarHeightScale); hash.Add(uint32_t(p.castsShadow));
        hash.Add(model.Get_NumMeshes());
        for (uint32_t mesh = 0; mesh < model.Get_NumMeshes(); ++mesh)
        {
            const auto* surface = model.Get_MaterialSurface(mesh); if (!surface) return false;
            const auto& s = *surface;
            ReplacementHashString(hash, model.Get_MaterialName(mesh));
#define REPLACEMENT_FIELD(name) hash.Add(s.name)
#define REPLACEMENT_BOOL(name) hash.Add(uint32_t(s.name))
            REPLACEMENT_FIELD(renderMode); REPLACEMENT_FIELD(cullMode); REPLACEMENT_FIELD(family); REPLACEMENT_BOOL(pbrAlphaMasked);
            REPLACEMENT_FIELD(sourceBgFlags); REPLACEMENT_BOOL(sourceBgUnlit); REPLACEMENT_FIELD(sourceBgBump); REPLACEMENT_FIELD(sourceBgUV);
            REPLACEMENT_FIELD(sourceBgFlicker); REPLACEMENT_FIELD(sourceBgSubspecular); REPLACEMENT_FIELD(sourceBgRimlight);
            REPLACEMENT_FIELD(sourceBgSpecularSaturation); REPLACEMENT_FIELD(sourceBgPanning); REPLACEMENT_BOOL(sourceFoliageWind);
            REPLACEMENT_FIELD(overlayColor); REPLACEMENT_FIELD(overlayTiling); REPLACEMENT_FIELD(overlayNormalIntensity);
            REPLACEMENT_FIELD(overlaySharpness); REPLACEMENT_FIELD(overlayBrightness); REPLACEMENT_FIELD(overlaySaturation);
            REPLACEMENT_FIELD(overlaySpecularIntensity); REPLACEMENT_BOOL(overlaySRGB); REPLACEMENT_BOOL(overlaySeparateSpecular);
            REPLACEMENT_FIELD(sourceOverlayFlags); REPLACEMENT_FIELD(sourceOverlayDirection);
            REPLACEMENT_FIELD(diffuseBrightness); REPLACEMENT_FIELD(normalIntensity); REPLACEMENT_FIELD(specularIntensity);
            REPLACEMENT_FIELD(specularPower); REPLACEMENT_FIELD(reflectionIntensity); REPLACEMENT_FIELD(reflectionContrast);
            REPLACEMENT_FIELD(reflectionTiling); REPLACEMENT_FIELD(diffuseSaturation); REPLACEMENT_FIELD(diffuseColor);
            REPLACEMENT_FIELD(specularColor); REPLACEMENT_FIELD(reflectionColor); REPLACEMENT_BOOL(diffuseSRGB);
            REPLACEMENT_BOOL(specularSRGB); REPLACEMENT_BOOL(reflectionSRGB); REPLACEMENT_BOOL(ormSRGB); REPLACEMENT_BOOL(castsShadow);
            REPLACEMENT_BOOL(hasEmissive); REPLACEMENT_FIELD(emissiveColor); REPLACEMENT_FIELD(emissiveIntensity);
            REPLACEMENT_FIELD(emissiveUVTiling); REPLACEMENT_FIELD(emissiveFlickerMinimum); REPLACEMENT_FIELD(emissiveFlickerSpeed);
            REPLACEMENT_FIELD(emissivePhaseOffset); REPLACEMENT_BOOL(emissiveSRGB); REPLACEMENT_FIELD(uvTiling);
            REPLACEMENT_BOOL(uvFixedNormal); REPLACEMENT_BOOL(useWorldReflection); REPLACEMENT_FIELD(detailNormalIntensity); REPLACEMENT_FIELD(detailNormalTiling);
#undef REPLACEMENT_BOOL
#undef REPLACEMENT_FIELD
        }
        // 실제 override 경로도 키에 포함한다. lightmap/shadow 경로는 instance별 live 입력이다.
        hash.Add(uint32_t(asset.materialOverrides.size()));
        for (const auto& material : asset.materialOverrides)
        {
            ReplacementHashString(hash, material.materialName);
            for (const auto* path : { &material.surfaceSpecularPath, &material.reflectionPath, &material.surfaceDiffusePath,
                &material.surfaceNormalPath, &material.overlayDiffusePath, &material.overlayNormalPath, &material.detailNormalPath,
                &material.surfaceORMPath, &material.surfaceEmissivePath, &material.environmentCubePath, &material.environmentBRDFPath,
                &material.sourceIndirectCubePath, &material.sourceIndirectBRDFPath })
                ReplacementHashString(hash, ReplacementRelative(*path));
        }
        return hash.Finish(digest);
    }
    bool ReplacementSignedUniformAffine(const VTXMESHINSTANCE& instance)
    {
        // 공유 atlas의 local T/B/N은 균일 직교 변환에서 원본 VS의 정규화와 같은 방향을
        // 유지한다. 반사는 기존 cull pass가 처리하지만 비균일 배율/shear는 이 계약 밖이다.
        const auto& world = instance.World;
        for (size_t row = 0u; row < 4u; ++row)
            for (size_t column = 0u; column < 4u; ++column)
                if (!std::isfinite(world.m[row][column])) return false;
        if (world._14 != 0.f || world._24 != 0.f || world._34 != 0.f || world._44 != 1.f) return false;
        const auto matrix = XMLoadFloat4x4(&world);
        const float determinant = XMVectorGetX(XMMatrixDeterminant(matrix));
        if (!std::isfinite(determinant) || std::abs(determinant) <= 1.e-6f) return false;
        float lengths[3];
        for (uint32_t i = 0; i < 3; ++i) lengths[i] = XMVectorGetX(XMVector3Length(matrix.r[i]));
        const float maximum = (std::max)({ lengths[0], lengths[1], lengths[2] });
        if (!std::isfinite(maximum) || maximum <= 0) return false;
        for (uint32_t i = 0; i < 3; ++i)
            if (std::abs(lengths[i] - maximum) > maximum * 1.e-4f ||
                std::abs(XMVectorGetX(XMVector3Dot(matrix.r[i], matrix.r[(i + 1) % 3]))) > maximum * maximum * 1.e-4f) return false;
        return true;
    }
}

struct Client::CMapStaticProxyBaker::STATE final
{
    ComPtr<ID3D11Device> device;
    ComPtr<ID3D11DeviceContext> context;
    std::shared_ptr<Engine::CShader> shader;
    std::map<std::filesystem::path, Digest> fileHashes;
    // 준비 세션만 소유한다. 성공한 CModel은 SRV 참조를 별도로 가져 baker 종료 후에도 유효하다.
    bool replacementIndexRead = false;
    std::string replacementError;
    std::map<std::string, ReplacementFile> replacementIndex;
    std::map<std::filesystem::path, std::shared_ptr<ReplacementPackage>> replacements;
    std::map<std::filesystem::path, ReplacementSourceDigest> replacementSourceDigests;
    std::shared_ptr<Engine::CShader> replacementShader;
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

bool Client::CMapStaticProxyBaker::Has_Replacement(const std::string& assetId, std::string* error)
{
    auto& state = *m_State;
    if (!state.replacementIndexRead)
    {
        state.replacementIndexRead = true;
        try
        {
            const auto path = CRuntimeAssetRoot::Resolve("Map/Derived/Bern/ReplacementIndex.json");
            if (path.empty() || !std::filesystem::exists(path))
                throw std::runtime_error("replacement.index_missing");
            const auto root = ReplacementParse(ReplacementRead(path, 4u * 1024u * 1024u));
            if (ReplacementText(root, "schema") != "lostark.bern-static-replacements" || ReplacementUInt(root, "version", 1) != 1)
                throw std::runtime_error("Replacement index version unsupported");
            std::map<std::string, ReplacementFile> staged;
            for (const auto& row : ReplacementArray(root, "packages"))
            {
                const auto file = ReplacementResource(ReplacementMember(row, "manifest"));
                for (const auto& id : ReplacementArray(row, "assetIds"))
                    if (!id.Is_String() || id.Get_String().empty() || !staged.emplace(id.Get_String(), file).second)
                        throw std::runtime_error("Replacement asset identity duplicated");
            }
            state.replacementIndex.swap(staged);
        }
        catch (const std::exception& exception) { state.replacementError = exception.what(); }
    }
    if (error) *error = state.replacementError;
    return state.replacementIndex.contains(assetId);
}

HRESULT Client::CMapStaticProxyBaker::Bind_Replacement(const std::shared_ptr<Engine::CShader>& shader,
    const std::shared_ptr<Engine::CModel>& proxy, const std::shared_ptr<Engine::CModel>& originalModel)
{
    if (!shader || !proxy || !originalModel) return E_INVALIDARG;
    const auto* surface = originalModel->Get_MaterialSurface(0);
    const auto settings = Engine::CGameInstance::Get().Get_MaterialRenderSettings();
    // 원본 재질을 끈 진단 모드는 이 파생 표현의 계약 밖이다. caller가 원본을 복원한다.
    if (!surface || !settings.bUseSourceMaterials || uint32_t(settings.eDebugView) != 0u) return E_FAIL;
    const uint32_t baked = surface->hasBakedLighting ? 1u : 0u, zero = 0u, program = 8u;
    if (FAILED(proxy->Bind_StaticProxySources(shader)) || FAILED(proxy->Bind_StaticProxyAtlases(shader, ReplacementAtlasNames)) ||
        FAILED(originalModel->Bind_SurfaceLighting(shader, 0)) ||
        FAILED(shader->Bind_RawValue("g_HasBakedLighting", &baked, sizeof(baked))) ||
        FAILED(shader->Bind_RawValue("g_SurfaceProgram", &program, sizeof(program))) ||
        FAILED(shader->Bind_RawValue("g_SourceFoliageWindEnabled", &zero, sizeof(zero))) ||
        FAILED(shader->Bind_RawValue("g_SurfaceDebugView", &zero, sizeof(zero)))) return E_FAIL;
    return S_OK;
}

HRESULT Client::CMapStaticProxyBaker::Prepare_Replacement(const std::shared_ptr<Engine::CModel>& originalModel,
    const MAP_ASSET_ENTRY& asset, std::span<const Engine::CModel::STATIC_PROXY_PLACEMENT_SOURCE> sources,
    std::shared_ptr<Engine::CModel>& outputProxy, std::shared_ptr<Engine::CShader>& outputShader,
    REPLACEMENT_LIMITS& outputLimits, std::string& error)
{
    error.clear();
    // 거절도 선택 실패의 원인이다. caller가 HRESULT와 이 안정된 reason 접두사를 함께 보존한다.
    const auto reject = [&](const std::string& reason, HRESULT result = S_FALSE)
    {
        error = reason;
        return result;
    };
    if (!originalModel || sources.empty()) return reject("replacement.invalid_source", E_INVALIDARG);
    if (!Has_Replacement(asset.id, &error))
        return error.empty() ? reject("replacement.asset_unsupported") : S_FALSE;
    auto& state = *m_State;
    try
    {
        const auto& entry = state.replacementIndex.at(asset.id);
        const auto verify = [&](const ReplacementFile& file)
        {
            Digest digest{};
            if (!ReplacementSourceHash(file.path, state.replacementSourceDigests, digest) || Hex(digest) != file.hash)
                throw std::runtime_error("Replacement dependency hash changed: " + file.path.generic_string());
        };
        const auto loaded = state.replacements.find(entry.path);
        const auto manifest = loaded == state.replacements.end() ? ReplacementParse(ReplacementReadVerified(entry, 4u * 1024u * 1024u)) : loaded->second->manifest;
        if (ReplacementText(manifest, "schema") != "lostark.bern-native-replacement" || ReplacementUInt(manifest, "version", 2u) != 2u)
            throw std::runtime_error("Replacement package version unsupported");
        const auto& source = ReplacementMember(manifest, "source");
        const auto modelFile = ReplacementResource(ReplacementMember(source, "model"));
        if (modelFile.path.lexically_normal() != asset.resolvedModelPath.lexically_normal()) return reject("replacement.model_path_mismatch");
        verify(modelFile);
        if (Hex(originalModel->Get_GeometryPayloadSha256()) != ReplacementText(source, "geometryPayloadSha256") ||
            Hex(originalModel->Get_GeometryMetadataIdentitySha256()) != ReplacementText(source, "geometryMetadataSha256")) return reject("replacement.geometry_identity_mismatch");
        Digest materialIdentity{};
        if (!ReplacementIdentity(*originalModel, asset, materialIdentity) || Hex(materialIdentity) != ReplacementText(source, "staticMaterialSha256")) return reject("replacement.static_material_identity_mismatch");
        std::map<std::filesystem::path, std::string> textures;
        for (const auto& texture : ReplacementArray(source, "textures"))
        {
            const auto file = ReplacementResource(texture); verify(file);
            if (!textures.emplace(file.path.lexically_normal(), file.hash).second) throw std::runtime_error("Duplicate replacement texture");
        }
        // Debug/Release compiled effect는 같은 bake 입력 source로 빌드한 승인 hash 집합 중 하나여야 한다.
        const auto& compiledShaders = ReplacementArray(source, "compiledShaders");
        if (compiledShaders.size() != 1u) throw std::runtime_error("Replacement shader contract missing");
        for (const auto& shader : compiledShaders)
        {
            const auto name = ReplacementUtf8Path(ReplacementText(shader, "name"));
            if (name != L"Shader_VtxMeshMapInstance.cso") throw std::runtime_error("Invalid shader dependency name");
            Digest digest{}; if (!ReplacementSourceHash(ModuleDirectory() / name, state.replacementSourceDigests, digest)) return reject("replacement.compiled_shader_missing");
            const auto hash = Hex(digest); bool accepted = false;
            for (const auto& candidate : ReplacementArray(shader, "sha256"))
                if (candidate.Is_String() && ReplacementHashText(candidate.Get_String()) && candidate.Get_String() == hash) accepted = true;
            if (!accepted) return reject("replacement.compiled_shader_hash_mismatch:" + hash);
        }
        if (asset.renderProfile.renderMode != MAP_ASSET_RENDER_MODE::DEFERRED || asset.renderProfile.uvSpeed.x != 0.f || asset.renderProfile.uvSpeed.y != 0.f)
            return reject("replacement.asset_render_profile");
        const auto* common = originalModel->Get_MaterialSurface(0);
        std::set<std::filesystem::path> livePaths;
        const auto& overrides = asset.materialOverrides;
        if (overrides.empty()) return reject("replacement.material_overrides_missing");
        const auto& first = overrides.front();
        for (const auto& material : overrides)
        {
            if (material.bakedAveragePath != first.bakedAveragePath || material.bakedDirectionalPath != first.bakedDirectionalPath || material.staticShadowPath != first.staticShadowPath)
                return reject("replacement.material_lighting_bank_mismatch");
            livePaths.insert(material.bakedAveragePath.lexically_normal()); livePaths.insert(material.bakedDirectionalPath.lexically_normal()); livePaths.insert(material.staticShadowPath.lexically_normal());
        }
        for (uint32_t mesh = 0; mesh < originalModel->Get_NumMeshes(); ++mesh)
        {
            const auto* surface = originalModel->Get_MaterialSurface(mesh);
            if (!surface || originalModel->Has_MaterialTextureOverrides(mesh) || originalModel->Has_MorphBaseVertices(mesh) || surface->sourceFoliageWind || surface->hasEmissive ||
                surface->sourceBgFlicker || surface->sourceBgPanning.x != 0.f || surface->sourceBgPanning.y != 0.f ||
                surface->sourceBgSubspecular.x != 0.f || surface->sourceBgRimlight.x != 0.f || surface->sourceBgRimlight.y != 0.f || surface->sourceBgRimlight.z != 0.f)
                return reject("replacement.material_dynamic_or_override:mesh=" + std::to_string(mesh));
            if (surface->family == Engine::MODEL_SURFACE_FAMILY::SOURCE_BG_OPAQUE_MASKED)
            { if (!originalModel->Can_BakeStaticProxy(mesh)) return reject("replacement.material_bake_policy:mesh=" + std::to_string(mesh)); }
            else if (surface->family != Engine::MODEL_SURFACE_FAMILY::SOURCE_OVERLAY_OPAQUE || surface->sourceOverlayFlags != 263u)
                return reject("replacement.material_family:mesh=" + std::to_string(mesh));
            if (surface->hasBakedLighting != common->hasBakedLighting || surface->bakedLightingSRGB != common->bakedLightingSRGB ||
                surface->hasStaticShadow != common->hasStaticShadow || surface->staticShadowChannel != common->staticShadowChannel ||
                std::memcmp(&surface->staticShadowTransfer, &common->staticShadowTransfer, sizeof(surface->staticShadowTransfer)) != 0 ||
                surface->renderMode != common->renderMode || surface->cullMode != common->cullMode) return reject("replacement.material_common_contract:mesh=" + std::to_string(mesh));
            // CMaterial 로드 stamp가 있어야 디스크 hash와 현재 SRV의 출처를 연결할 수 있다.
            if (!originalModel->Validate_StaticProxySourceFiles(mesh)) return reject("replacement.source_file_stamp:mesh=" + std::to_string(mesh));
            for (const auto& path : originalModel->Get_StaticProxySourcePaths(mesh))
                if (!livePaths.contains(path.lexically_normal()) && !textures.contains(path.lexically_normal())) return reject("replacement.source_file_unlisted:mesh=" + std::to_string(mesh) + ":" + path.generic_string());
        }
        for (const auto& placement : sources)
            if (placement.model != originalModel.get() || !ReplacementSignedUniformAffine(placement.instance)) return reject("replacement.placement_transform:placement=" + std::to_string(placement.sourceId));
        std::shared_ptr<ReplacementPackage> package = loaded == state.replacements.end() ? nullptr : loaded->second;
        ContextState isolated;
        if (!isolated.Begin(state.device.Get(), state.context.Get())) throw std::runtime_error("Replacement needs isolated immediate context");
        if (!package)
        {
            package = std::make_shared<ReplacementPackage>(); package->manifest = manifest;
            const auto& geometry = ReplacementMember(manifest, "geometry");
            const auto vbFile = ReplacementResource(ReplacementMember(geometry, "vertices"));
            const auto ibFile = ReplacementResource(ReplacementMember(geometry, "indices"));
            const auto vb = ReplacementReadVerified(vbFile, 32u * 1024u * 1024u), ib = ReplacementReadVerified(ibFile, 32u * 1024u * 1024u);
            uint32_t vh[4]{}, ih[3]{};
            if (vb.size() < sizeof(vh) || ib.size() < sizeof(ih)) throw std::runtime_error("Replacement geometry header truncated");
            std::memcpy(vh, vb.data(), sizeof(vh)); std::memcpy(ih, ib.data(), sizeof(ih));
            if (std::memcmp(vb.data(), "BGVB", 4) || vh[1] != 1 || vh[3] != sizeof(VTXMESH) || vh[2] == 0 ||
                vb.size() != sizeof(vh) + uint64_t(vh[2]) * sizeof(VTXMESH) || std::memcmp(ib.data(), "BGIX", 4) || ih[1] != 1 ||
                ih[2] == 0 || ih[2] % 3 || ib.size() != sizeof(ih) + uint64_t(ih[2]) * sizeof(uint32_t)) throw std::runtime_error("Replacement geometry ABI invalid");
            package->vertices.resize(vh[2]); package->uv.resize(vh[2]); package->indices.resize(ih[2]);
            std::memcpy(package->vertices.data(), vb.data() + sizeof(vh), vb.size() - sizeof(vh));
            std::memcpy(package->indices.data(), ib.data() + sizeof(ih), ib.size() - sizeof(ih));
            for (size_t i = 0; i < package->vertices.size(); ++i) package->uv[i] = package->vertices[i].vTexcoord;
            const auto& atlas = ReplacementMember(manifest, "atlas");
            package->width = ReplacementUInt(atlas, "width", 8192); package->height = ReplacementUInt(atlas, "height", 8192);
            if (package->width < 64 || package->width != package->height || (package->width & (package->width - 1u)))
                throw std::runtime_error("Replacement atlas dimensions invalid");
            const auto& planes = ReplacementArray(atlas, "planes"); if (planes.size() != package->atlases.size()) throw std::runtime_error("Replacement atlas count invalid");
            for (size_t i = 0; i < planes.size(); ++i)
            {
                const auto file = ReplacementResource(planes[i]);
                const uint32_t pixelBytes = i == 8 ? 16u : 8u;
                const auto bytes = ReplacementReadVerified(file, 128u * 1024u * 1024u);
                if (bytes.size() != uint64_t(package->width) * package->height * pixelBytes) throw std::runtime_error("Replacement atlas payload size invalid");
                const DXGI_FORMAT format = i == 3 ? DXGI_FORMAT_R32G32_FLOAT : i == 8 ? DXGI_FORMAT_R32G32B32A32_FLOAT : DXGI_FORMAT_R16G16B16A16_FLOAT;
                const uint32_t mips = (i == 3 || i == 8) ? 1u : 4u;
                if (ReplacementUInt(planes[i], "dxgiFormat", 255) != uint32_t(format) || ReplacementUInt(planes[i], "mipCount", 4) != mips)
                    throw std::runtime_error("Replacement atlas format mismatch");
                // 모든 float/half 채널을 검증하여 nonfinite payload를 GPU에 commit하지 않는다.
                for (size_t offset = 0; offset < bytes.size(); offset += format == DXGI_FORMAT_R16G16B16A16_FLOAT ? 2u : 4u)
                {
                    if (format == DXGI_FORMAT_R16G16B16A16_FLOAT) { uint16_t value; std::memcpy(&value, bytes.data() + offset, 2); if ((value & 0x7c00u) == 0x7c00u) throw std::runtime_error("Nonfinite half atlas"); }
                    else { float value; std::memcpy(&value, bytes.data() + offset, 4); if (!std::isfinite(value)) throw std::runtime_error("Nonfinite float atlas"); }
                }
                D3D11_TEXTURE2D_DESC desc{}; desc.Width = package->width; desc.Height = package->height;
                desc.MipLevels = mips; desc.ArraySize = desc.SampleDesc.Count = 1; desc.Format = format;
                desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | (mips > 1 ? D3D11_BIND_RENDER_TARGET : 0);
                desc.MiscFlags = mips > 1 ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0;
                ComPtr<ID3D11Texture2D> texture;
                if (FAILED(state.device->CreateTexture2D(&desc, nullptr, &texture)) ||
                    FAILED(state.device->CreateShaderResourceView(texture.Get(), nullptr, &package->atlases[i]))) throw std::runtime_error("Replacement atlas allocation failed");
                state.context->UpdateSubresource(texture.Get(), 0, nullptr, bytes.data(), package->width * pixelBytes, 0);
                if (mips > 1) state.context->GenerateMips(package->atlases[i].Get());
            }
            const auto& limits = ReplacementMember(manifest, "limits");
            package->limits.maximumProjectedDiameterPixels = ReplacementPositive(limits, "maximumProjectedDiameterPixels");
            package->limits.minimumSurfaceDistanceMetres = ReplacementPositive(limits, "minimumSurfaceDistanceMetres");
        }
        if (!state.replacementShader)
        {
            std::array<D3D11_INPUT_ELEMENT_DESC, VTXMESH::iNumElements + 2> elements{};
            std::copy(std::begin(VTXMESH::Elements), std::end(VTXMESH::Elements), elements.begin());
            elements[VTXMESH::iNumElements] = { "ATLASUV", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 };
            elements.back() = { "SOURCEINDEX", 0, DXGI_FORMAT_R32_UINT, 1, 8, D3D11_INPUT_PER_VERTEX_DATA, 0 };
            state.replacementShader = Engine::CShader::Create(state.device, state.context, L"Shader_VtxMeshMapProxy.hlsl", elements.data(), uint32_t(elements.size()));
            if (!state.replacementShader) throw std::runtime_error("Replacement shader unavailable");
        }
        Engine::CModel::STATIC_PROXY_OPTIONS options; options.atlasResolution = package->width;
        options.maximumSources = uint32_t(sources.size());
        std::shared_ptr<Engine::CModel> staged;
        Engine::CModel::STATIC_PROXY_STATS stats;
        const Engine::CModel::STATIC_PROXY_GEOMETRY geometry{ package->vertices, package->uv, package->indices };
        const HRESULT created = Engine::CModel::Create_StaticProxy(geometry, sources, options, staged, stats);
        if (created != S_OK) return reject("replacement.create_static_proxy", created);
        std::vector<Engine::CModel::STATIC_PROXY_METADATA> metadata(sources.size());
        if (FAILED(staged->Set_StaticProxyMetadata(metadata)) || FAILED(staged->Set_StaticProxyAtlases(package->atlases)) ||
            FAILED(Bind_Replacement(state.replacementShader, staged, originalModel))) throw std::runtime_error("Replacement binding failed");
        for (uint32_t pass = 4u; pass <= 6u; ++pass)
            if (FAILED(state.replacementShader->Begin(pass))) throw std::runtime_error("Replacement shader pass unavailable");
        for (uint32_t mesh = 0; mesh < originalModel->Get_NumMeshes(); ++mesh)
            if (!originalModel->Validate_StaticProxySourceFiles(mesh)) throw std::runtime_error("Source changed during replacement preparation");
        state.replacements.emplace(entry.path, package);
        outputProxy = std::move(staged); outputShader = state.replacementShader; outputLimits = package->limits;
        return S_OK;
    }
    catch (const std::exception& exception) { error = exception.what(); return E_FAIL; }
    catch (...) { error = "Replacement preparation failed"; return E_FAIL; }
}
