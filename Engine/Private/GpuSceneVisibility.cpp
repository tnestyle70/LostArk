#include "GpuSceneVisibility.h"
#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <limits>
#include <vector>

using Microsoft::WRL::ComPtr;
using namespace DirectX;
namespace Engine
{
namespace
{
    using Clock = std::chrono::steady_clock;
    double Milliseconds(Clock::time_point begin) { return std::chrono::duration<double, std::milli>(Clock::now()-begin).count(); }
    HRESULT LoadCompute(ID3D11Device* device, const wchar_t* directory, const wchar_t* name, ComPtr<ID3D11ComputeShader>& output)
    {
        try
        {
            std::filesystem::path path;
            if (directory) path = directory;
            else
            {
                HMODULE module = nullptr;
                if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                    reinterpret_cast<LPCWSTR>(&LoadCompute), &module)) return HRESULT_FROM_WIN32(GetLastError());
                wchar_t full[32768]{};
                const auto length = GetModuleFileNameW(module, full, static_cast<DWORD>(std::size(full)));
                if (!length || length >= std::size(full)) return E_FAIL;
                path = std::filesystem::path(full).parent_path();
            }
            path /= name;
            std::ifstream input(path, std::ios::binary | std::ios::ate);
            if (!input) return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
            const auto size = input.tellg();
            if (size <= 0 || size > 4 * 1024 * 1024) return E_INVALIDARG;
            std::vector<char> bytes(static_cast<size_t>(size));
            input.seekg(0);
            if (!input.read(bytes.data(), size)) return E_FAIL;
            return device->CreateComputeShader(bytes.data(), bytes.size(), nullptr, &output);
        }
        catch (...) { return E_OUTOFMEMORY; }
    }
    HRESULT Buffer(ID3D11Device* device, uint32_t bytes, uint32_t binds, uint32_t misc, uint32_t stride,
        const void* data, ComPtr<ID3D11Buffer>& output)
    {
        D3D11_BUFFER_DESC desc{};
        desc.ByteWidth = bytes; desc.BindFlags = binds; desc.MiscFlags = misc; desc.StructureByteStride = stride;
        desc.Usage = D3D11_USAGE_DEFAULT;
        D3D11_SUBRESOURCE_DATA initial{}; initial.pSysMem = data;
        return device->CreateBuffer(&desc, data ? &initial : nullptr, &output);
    }
    HRESULT BufferSRV(ID3D11Device* device, ID3D11Buffer* buffer, uint32_t count, bool raw, ComPtr<ID3D11ShaderResourceView>& output)
    {
        D3D11_SHADER_RESOURCE_VIEW_DESC desc{};
        desc.Format = raw ? DXGI_FORMAT_R32_TYPELESS : DXGI_FORMAT_UNKNOWN;
        desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
        desc.BufferEx.NumElements = count;
        desc.BufferEx.Flags = raw ? D3D11_BUFFEREX_SRV_FLAG_RAW : 0;
        return device->CreateShaderResourceView(buffer, &desc, &output);
    }
    HRESULT BufferUAV(ID3D11Device* device, ID3D11Buffer* buffer, uint32_t bytes, ComPtr<ID3D11UnorderedAccessView>& output)
    {
        D3D11_UNORDERED_ACCESS_VIEW_DESC desc{};
        desc.Format = DXGI_FORMAT_R32_TYPELESS; desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
        desc.Buffer.NumElements = bytes / 4; desc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_RAW;
        return device->CreateUnorderedAccessView(buffer, &desc, &output);
    }
    struct FRAME_CONSTANTS final
    {
        XMFLOAT4X4 View{}, Projection{};
        XMFLOAT4 Planes[6]{}, CameraNear{}, ViewportPolicy{};
        uint32_t Counts[4]{}, Flags[4]{};
    };
    static_assert(sizeof(FRAME_CONSTANTS) == 288);
    static_assert(sizeof(CGpuSceneVisibility::SOURCE_INSTANCE) == 80);
    bool FrameConstants(const CGpuSceneVisibility::FRAME& frame, FRAME_CONSTANTS& output)
    {
        if (!frame.FrameId || !frame.Width || !frame.Height || frame.Width > 16384 || frame.Height > 16384 ||
            !std::isfinite(frame.PixelError) || frame.PixelError <= 0 || frame.PixelError > 1 ||
            !std::isfinite(frame.DistanceScale) || frame.DistanceScale <= 0) return false;
        const float* view = &frame.View._11;
        const float* projection = &frame.Projection._11;
        for (uint32_t i = 0; i < 16; ++i)
            if (!std::isfinite(view[i]) || !std::isfinite(projection[i])) return false;
        if (!std::isfinite(frame.Camera.x) || !std::isfinite(frame.Camera.y) || !std::isfinite(frame.Camera.z)) return false;
        // 표준 LH perspective만 허용한다. 다른 projection은 기존 CPU 경로가 책임진다.
        const auto& p = frame.Projection;
        if (p._11 <= 0 || p._22 <= 0 || p._33 <= 1 || p._43 >= 0 ||
            std::abs(p._34-1) > 1e-5f || std::abs(p._44) > 1e-5f ||
            std::abs(p._12)+std::abs(p._13)+std::abs(p._14)+std::abs(p._21)+std::abs(p._23)+std::abs(p._24)+
            std::abs(p._31)+std::abs(p._32)+std::abs(p._41)+std::abs(p._42) > 1e-5f) return false;
        const auto& v = frame.View;
        if (std::abs(v._14)+std::abs(v._24)+std::abs(v._34)+std::abs(v._44-1) > 1e-4f) return false;
        const XMVECTOR axes[3]{ XMVectorSet(v._11,v._12,v._13,0), XMVectorSet(v._21,v._22,v._23,0), XMVectorSet(v._31,v._32,v._33,0) };
        for (uint32_t a = 0; a < 3; ++a)
            for (uint32_t b = a; b < 3; ++b)
                if (std::abs(XMVectorGetX(XMVector3Dot(axes[a],axes[b]))-(a == b ? 1.f : 0.f)) > 1e-3f) return false;
        XMFLOAT4X4 vp{};
        XMStoreFloat4x4(&vp, XMMatrixMultiply(XMLoadFloat4x4(&v), XMLoadFloat4x4(&p)));
        const XMVECTOR columns[4]{ XMVectorSet(vp._11,vp._21,vp._31,vp._41), XMVectorSet(vp._12,vp._22,vp._32,vp._42),
            XMVectorSet(vp._13,vp._23,vp._33,vp._43), XMVectorSet(vp._14,vp._24,vp._34,vp._44) };
        const XMVECTOR planes[6]{columns[3]+columns[0],columns[3]-columns[0],columns[3]+columns[1],columns[3]-columns[1],columns[2],columns[3]-columns[2]};
        for (uint32_t i = 0; i < 6; ++i)
        {
            const float length = XMVectorGetX(XMVector3Length(planes[i]));
            if (!std::isfinite(length) || length < 1e-8f) return false;
            XMStoreFloat4(&output.Planes[i], planes[i]/length);
        }
        output.View = v; output.Projection = p;
        output.CameraNear = XMFLOAT4(frame.Camera.x, frame.Camera.y, frame.Camera.z, -p._43/p._33);
        output.ViewportPolicy = XMFLOAT4(static_cast<float>(frame.Width),static_cast<float>(frame.Height),frame.PixelError,frame.DistanceScale);
        output.Flags[0] = frame.DistanceEnabled; output.Flags[1] = frame.MeshLodEnabled;
        output.Flags[3] = frame.FrustumEnabled;
        return true;
    }
    void ClearCompute(ID3D11DeviceContext* context)
    {
        ID3D11ShaderResourceView* srvs[5]{}; ID3D11UnorderedAccessView* uavs[2]{}; ID3D11Buffer* cb = nullptr;
        context->CSSetShaderResources(0,5,srvs); context->CSSetUnorderedAccessViews(0,2,uavs,nullptr);
        context->CSSetConstantBuffers(0,1,&cb); context->CSSetShader(nullptr,nullptr,0);
    }
}
struct CGpuSceneVisibility::IMPL final
{
    ComPtr<ID3D11Device> Device; ComPtr<ID3D11DeviceContext> Context;
    ComPtr<ID3D11ComputeShader> Shader;
    ComPtr<ID3D11Buffer> Source, Payload, Bins, Active, Output, Args, ArgsTemplate, Constants;
    ComPtr<ID3D11ShaderResourceView> SourceSRV, PayloadSRV, BinsSRV, ActiveSRV;
    ComPtr<ID3D11UnorderedAccessView> OutputUAV, ArgsUAV;
    std::vector<uint32_t> Offsets;
    uint32_t Count = 0, Stride = 0, ActiveCount = 0;
    uint64_t PreparedFrame = 0, ReadyFrame = 0;
    FRAME_CONSTANTS FrameConstants{};
    std::function<HRESULT(const FRAME&)> Prepare;
    STATISTICS Stats{};
};
CGpuSceneVisibility::CGpuSceneVisibility(ID3D11Device* device, ID3D11DeviceContext* context) : m_Impl(std::make_unique<IMPL>())
{ m_Impl->Device = device; m_Impl->Context = context; }
CGpuSceneVisibility::~CGpuSceneVisibility() = default;
HRESULT CGpuSceneVisibility::Initialize(const wchar_t* directory)
{
    auto& s = *m_Impl;
    if (!s.Device || !s.Context || s.Device->GetFeatureLevel() < D3D_FEATURE_LEVEL_11_0 ||
        s.Context->GetType() != D3D11_DEVICE_CONTEXT_IMMEDIATE) return E_NOINTERFACE;
    HRESULT result = LoadCompute(s.Device.Get(),directory,L"Shader_GpuSceneVisibility.cso",s.Shader);
    if (FAILED(result)) return result;
    return Buffer(s.Device.Get(),sizeof(FRAME_CONSTANTS),D3D11_BIND_CONSTANT_BUFFER,0,0,nullptr,s.Constants);
}
HRESULT CGpuSceneVisibility::Stage(std::span<const SOURCE_INSTANCE> instances, std::span<const uint8_t> payload,
    std::span<const DRAW_BIN> bins, uint32_t activeSourceCount, uint32_t stride)
{
    auto& s = *m_Impl;
    if (!s.Shader || !s.Constants || instances.empty() || bins.empty() || !activeSourceCount || activeSourceCount > 65535u*64u || !stride || stride % 4 ||
        stride > 2048 || instances.size() > 65535u*64u || bins.size() > 1000000u ||
        payload.size() != instances.size()*stride || payload.size() > UINT32_MAX) return E_INVALIDARG;
    try
    {
        IMPL staged;
        staged.Count = static_cast<uint32_t>(instances.size()); staged.Stride = stride; staged.ActiveCount = activeSourceCount;
        std::vector<std::array<uint32_t,4>> binData(bins.size());
        std::vector<std::array<uint32_t,5>> args(bins.size());
        std::vector<uint32_t> counts(bins.size()), active(activeSourceCount,1u);
        for (const auto& source : instances)
        {
            const auto& sphere = source.WorldSphere;
            if (!std::isfinite(sphere.x)||!std::isfinite(sphere.y)||!std::isfinite(sphere.z)||!std::isfinite(sphere.w)||sphere.w<0 ||
                !std::isfinite(source.DistanceLimit)||source.DistanceLimit<0||!std::isfinite(source.MaximumPixels)||source.MaximumPixels<0||
                source.ActiveSource >= activeSourceCount || source.LodBins[0] >= bins.size()) return E_INVALIDARG;
            for (uint32_t lod = 0; lod < LodCount; ++lod)
            {
                const uint32_t bin = source.LodBins[lod];
                if (!std::isfinite(source.WorldErrors[lod]) || source.WorldErrors[lod] < 0) return E_INVALIDARG;
                if (bin == InvalidBin) continue;
                if (bin >= bins.size()) return E_INVALIDARG;
                // 같은 source가 중복 후보를 가져도 한 bin에 최대 한 번만 들어간다.
                bool duplicate = false;
                for (uint32_t prior = 0; prior < lod; ++prior) duplicate |= source.LodBins[prior] == bin;
                if (!duplicate) ++counts[bin];
            }
        }
        uint64_t capacity = 0;
        staged.Offsets.reserve(bins.size());
        for (size_t i = 0; i < bins.size(); ++i)
        {
            if (!bins[i].IndexCount || !bins[i].Capacity || counts[i] > bins[i].Capacity) return E_INVALIDARG;
            staged.Offsets.push_back(static_cast<uint32_t>(capacity*stride));
            binData[i] = {static_cast<uint32_t>(capacity),bins[i].Capacity,0u,0u};
            args[i] = {bins[i].IndexCount,0u,bins[i].FirstIndex,static_cast<uint32_t>(bins[i].BaseVertex),0u};
            capacity += bins[i].Capacity;
            if (capacity*stride > 512ull*1024*1024) return E_INVALIDARG;
        }
        HRESULT hr = S_OK;
#define GPU_BUFFER(call) do { hr = (call); if (FAILED(hr)) return hr; } while (false)
        GPU_BUFFER(Buffer(s.Device.Get(),static_cast<uint32_t>(instances.size_bytes()),D3D11_BIND_SHADER_RESOURCE,D3D11_RESOURCE_MISC_BUFFER_STRUCTURED,sizeof(SOURCE_INSTANCE),instances.data(),staged.Source));
        GPU_BUFFER(BufferSRV(s.Device.Get(),staged.Source.Get(),staged.Count,false,staged.SourceSRV));
        GPU_BUFFER(Buffer(s.Device.Get(),static_cast<uint32_t>(payload.size()),D3D11_BIND_SHADER_RESOURCE,D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS,0,payload.data(),staged.Payload));
        GPU_BUFFER(BufferSRV(s.Device.Get(),staged.Payload.Get(),static_cast<uint32_t>(payload.size()/4),true,staged.PayloadSRV));
        GPU_BUFFER(Buffer(s.Device.Get(),static_cast<uint32_t>(binData.size()*16),D3D11_BIND_SHADER_RESOURCE,D3D11_RESOURCE_MISC_BUFFER_STRUCTURED,16,binData.data(),staged.Bins));
        GPU_BUFFER(BufferSRV(s.Device.Get(),staged.Bins.Get(),static_cast<uint32_t>(bins.size()),false,staged.BinsSRV));
        GPU_BUFFER(Buffer(s.Device.Get(),activeSourceCount*4,D3D11_BIND_SHADER_RESOURCE,D3D11_RESOURCE_MISC_BUFFER_STRUCTURED,4,active.data(),staged.Active));
        GPU_BUFFER(BufferSRV(s.Device.Get(),staged.Active.Get(),activeSourceCount,false,staged.ActiveSRV));
        const uint32_t outputBytes = static_cast<uint32_t>(capacity*stride), argumentBytes = static_cast<uint32_t>(bins.size()*20);
        GPU_BUFFER(Buffer(s.Device.Get(),outputBytes,D3D11_BIND_VERTEX_BUFFER|D3D11_BIND_UNORDERED_ACCESS,D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS,0,nullptr,staged.Output));
        GPU_BUFFER(BufferUAV(s.Device.Get(),staged.Output.Get(),outputBytes,staged.OutputUAV));
        GPU_BUFFER(Buffer(s.Device.Get(),argumentBytes,D3D11_BIND_UNORDERED_ACCESS,D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS|D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS,0,args.data(),staged.Args));
        GPU_BUFFER(BufferUAV(s.Device.Get(),staged.Args.Get(),argumentBytes,staged.ArgsUAV));
        GPU_BUFFER(Buffer(s.Device.Get(),argumentBytes,0,0,0,args.data(),staged.ArgsTemplate));
#undef GPU_BUFFER
        staged.Device = s.Device; staged.Context = s.Context; staged.Shader = s.Shader; staged.Constants = s.Constants;
        staged.Prepare = std::move(s.Prepare);
        s = std::move(staged);
        return S_OK;
    }
    catch (...) { return E_OUTOFMEMORY; }
}
void CGpuSceneVisibility::Clear() noexcept
{
    auto& s = *m_Impl;
    s.Source.Reset(); s.Payload.Reset(); s.Bins.Reset(); s.Active.Reset(); s.Output.Reset(); s.Args.Reset(); s.ArgsTemplate.Reset();
    s.SourceSRV.Reset(); s.PayloadSRV.Reset(); s.BinsSRV.Reset(); s.ActiveSRV.Reset(); s.OutputUAV.Reset(); s.ArgsUAV.Reset();
    s.Offsets.clear(); s.Count = s.ActiveCount = s.Stride = 0; s.Stats = {}; s.PreparedFrame = s.ReadyFrame = 0;
}
HRESULT CGpuSceneVisibility::Set_ActiveSources(std::span<const uint32_t> active, uint32_t activeOwnerCount)
{
    auto& s = *m_Impl;
    if (!s.Active || active.size() != s.ActiveCount) return E_INVALIDARG;
    const uint32_t activeSources = static_cast<uint32_t>(std::count_if(active.begin(),active.end(),[](uint32_t value) { return value != 0; }));
    // 하나의 instance가 여러 mesh lane을 켜도 HiZ 비용 판단에서는 한 번만 센다.
    // 검증을 upload보다 먼저 끝내 잘못된 집계가 기존 active buffer나 통계를 바꾸지 못하게 한다.
    const uint32_t activeOwners = activeOwnerCount == UINT32_MAX ? activeSources : activeOwnerCount;
    if (activeOwners > activeSources) return E_INVALIDARG;
    s.Context->UpdateSubresource(s.Active.Get(),0,nullptr,active.data(),0,0);
    s.Stats.UploadBytes += static_cast<uint32_t>(active.size_bytes());
    s.Stats.ActiveSources = activeSources;
    s.Stats.ActiveOwners = activeOwners;
    return S_OK;
}
void CGpuSceneVisibility::Set_FramePrepareCallback(std::function<HRESULT(const FRAME&)> callback) { m_Impl->Prepare = std::move(callback); }
HRESULT CGpuSceneVisibility::Prepare_Frame(const FRAME& frame)
{
    auto& s = *m_Impl;
    const auto begin = Clock::now();
    Invalidate_Frame(); s.Stats = {}; s.Stats.FrameId = frame.FrameId;
    FRAME_CONSTANTS prepared{};
    if (!s.Shader || !FrameConstants(frame,prepared)) return S_FALSE;
    HRESULT hr = S_OK;
    try { if (s.Prepare) hr = s.Prepare(frame); }
    catch (...) { hr = E_FAIL; }
    s.Stats.PrepareCpuMs = Milliseconds(begin);
    if (hr != S_OK || !s.Count || !s.Output) return hr == S_OK ? S_FALSE : hr;
    s.FrameConstants = prepared;
    s.PreparedFrame = frame.FrameId; s.Stats.FrameId = frame.FrameId;
    s.Stats.SourceInstances = s.Count; s.Stats.Bins = static_cast<uint32_t>(s.Offsets.size());
    return S_OK;
}
HRESULT CGpuSceneVisibility::Dispatch(const FRAME& frame, ID3D11ShaderResourceView* hiZ, uint32_t mipCount)
{
    auto& s = *m_Impl;
    const auto begin = Clock::now();
    s.ReadyFrame = 0; s.Stats.Ready = false;
    FRAME_CONSTANTS constants{};
    if (s.PreparedFrame != frame.FrameId || !frame.FrameId || !s.Count || !FrameConstants(frame,constants) ||
        std::memcmp(&constants,&s.FrameConstants,sizeof(constants)) != 0) return S_FALSE;
    const bool useHiZ = frame.HiZEnabled && hiZ && mipCount > 0;
    constants.Counts[0] = s.Count; constants.Counts[1] = s.Stride;
    constants.Counts[2] = static_cast<uint32_t>(s.Offsets.size()); constants.Counts[3] = useHiZ ? mipCount : 0;
    constants.Flags[2] = useHiZ;
    // 이전 draw의 IA 참조를 끊고 UAV를 바인딩한다. Dispatch 뒤에는 반드시 반대로 정리한다.
    ID3D11Buffer* nullVB = nullptr; const UINT zero = 0;
    s.Context->IASetVertexBuffers(1,1,&nullVB,&zero,&zero);
    s.Context->CopyResource(s.Args.Get(),s.ArgsTemplate.Get());
    s.Context->UpdateSubresource(s.Constants.Get(),0,nullptr,&constants,0,0);
    ID3D11ShaderResourceView* srvs[]{s.SourceSRV.Get(),s.PayloadSRV.Get(),s.ActiveSRV.Get(),s.BinsSRV.Get(),useHiZ ? hiZ : nullptr};
    ID3D11UnorderedAccessView* uavs[]{s.OutputUAV.Get(),s.ArgsUAV.Get()}; ID3D11Buffer* cb = s.Constants.Get();
    s.Context->CSSetShader(s.Shader.Get(),nullptr,0); s.Context->CSSetConstantBuffers(0,1,&cb);
    s.Context->CSSetShaderResources(0,5,srvs); s.Context->CSSetUnorderedAccessViews(0,2,uavs,nullptr);
    s.Context->Dispatch((s.Count+63)/64,1,1);
    ClearCompute(s.Context.Get());
    const HRESULT removed = s.Device->GetDeviceRemovedReason();
    if (FAILED(removed)) return removed;
    s.ReadyFrame = frame.FrameId; s.Stats.Ready = true; s.Stats.Dispatches = 1;
    s.Stats.UsedHiZ = useHiZ; s.Stats.UploadBytes += sizeof(constants); s.Stats.DispatchCpuMs = Milliseconds(begin);
    return S_OK;
}
void CGpuSceneVisibility::Invalidate_Frame() noexcept { m_Impl->PreparedFrame = m_Impl->ReadyFrame = 0; m_Impl->Stats.Ready = false; }
bool CGpuSceneVisibility::Get_Draw(uint32_t bin, uint64_t frameId, DRAW_OUTPUT& output) const noexcept
{
    output = {}; const auto& s = *m_Impl;
    if (!frameId || frameId != s.ReadyFrame || bin >= s.Offsets.size()) return false;
    output.InstanceBuffer = s.Output.Get(); output.Arguments = s.Args.Get(); output.Stride = s.Stride;
    output.InstanceByteOffset = s.Offsets[bin]; output.ArgumentByteOffset = bin*20; return true;
}
const CGpuSceneVisibility::STATISTICS& CGpuSceneVisibility::Get_Statistics() const noexcept { return m_Impl->Stats; }

struct CGpuVisibilityHiZ::IMPL final
{
    ComPtr<ID3D11Device> Device; ComPtr<ID3D11DeviceContext> Context;
    ComPtr<ID3D11ComputeShader> Copy, Reduce;
    ComPtr<ID3D11Texture2D> Depth, Pyramid;
    ComPtr<ID3D11DepthStencilView> DSV, SavedDSV;
    ComPtr<ID3D11ShaderResourceView> DepthSRV, PyramidSRV;
    std::vector<ComPtr<ID3D11ShaderResourceView>> MipSRVs;
    std::vector<ComPtr<ID3D11UnorderedAccessView>> MipUAVs;
    std::array<ComPtr<ID3D11RenderTargetView>,8> SavedRTVs;
    D3D11_VIEWPORT SavedViewports[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE]{};
    uint32_t SavedViewportCount = 0, Width = 0, Height = 0, PaddedWidth = 0, PaddedHeight = 0;
    bool InDepth = false, Ready = false;
};
CGpuVisibilityHiZ::CGpuVisibilityHiZ(ID3D11Device* device, ID3D11DeviceContext* context) : m_Impl(std::make_unique<IMPL>())
{ m_Impl->Device = device; m_Impl->Context = context; }
CGpuVisibilityHiZ::~CGpuVisibilityHiZ() = default;
HRESULT CGpuVisibilityHiZ::Initialize(const wchar_t* directory)
{
    auto& s = *m_Impl;
    if (!s.Device || !s.Context || s.Device->GetFeatureLevel() < D3D_FEATURE_LEVEL_11_0) return E_NOINTERFACE;
    HRESULT hr = LoadCompute(s.Device.Get(),directory,L"Shader_GpuHiZCopy.cso",s.Copy);
    if (FAILED(hr)) return hr;
    return LoadCompute(s.Device.Get(),directory,L"Shader_GpuHiZReduce.cso",s.Reduce);
}
HRESULT CGpuVisibilityHiZ::Begin_Depth(uint32_t width, uint32_t height)
{
    auto& s = *m_Impl; s.Ready = false;
    if (!s.Copy || !s.Reduce || s.InDepth || !width || !height || width > 16384 || height > 16384) return E_INVALIDARG;
    if (s.Width != width || s.Height != height)
    {
        IMPL staged;
        staged.Width = width; staged.Height = height; staged.PaddedWidth = staged.PaddedHeight = 1;
        while (staged.PaddedWidth < width) staged.PaddedWidth *= 2;
        while (staged.PaddedHeight < height) staged.PaddedHeight *= 2;
        D3D11_TEXTURE2D_DESC desc{};
        desc.Width = width; desc.Height = height; desc.MipLevels = desc.ArraySize = 1; desc.SampleDesc.Count = 1;
        desc.Format = DXGI_FORMAT_R32_TYPELESS; desc.BindFlags = D3D11_BIND_DEPTH_STENCIL|D3D11_BIND_SHADER_RESOURCE;
        HRESULT hr = s.Device->CreateTexture2D(&desc,nullptr,&staged.Depth);
        if (FAILED(hr)) return hr;
        D3D11_DEPTH_STENCIL_VIEW_DESC dsv{}; dsv.Format = DXGI_FORMAT_D32_FLOAT; dsv.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        hr = s.Device->CreateDepthStencilView(staged.Depth.Get(),&dsv,&staged.DSV); if (FAILED(hr)) return hr;
        D3D11_SHADER_RESOURCE_VIEW_DESC srv{}; srv.Format = DXGI_FORMAT_R32_FLOAT; srv.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D; srv.Texture2D.MipLevels = 1;
        hr = s.Device->CreateShaderResourceView(staged.Depth.Get(),&srv,&staged.DepthSRV); if (FAILED(hr)) return hr;
        desc.Width = staged.PaddedWidth; desc.Height = staged.PaddedHeight;
        desc.MipLevels = 1;
        for (uint32_t size = std::max(desc.Width,desc.Height); size > 1; size /= 2) ++desc.MipLevels;
        desc.Format = DXGI_FORMAT_R32_FLOAT; desc.BindFlags = D3D11_BIND_SHADER_RESOURCE|D3D11_BIND_UNORDERED_ACCESS;
        hr = s.Device->CreateTexture2D(&desc,nullptr,&staged.Pyramid); if (FAILED(hr)) return hr;
        srv.Texture2D.MipLevels = desc.MipLevels;
        hr = s.Device->CreateShaderResourceView(staged.Pyramid.Get(),&srv,&staged.PyramidSRV); if (FAILED(hr)) return hr;
        staged.MipSRVs.resize(desc.MipLevels); staged.MipUAVs.resize(desc.MipLevels);
        for (uint32_t mip = 0; mip < desc.MipLevels; ++mip)
        {
            srv.Texture2D.MostDetailedMip = mip; srv.Texture2D.MipLevels = 1;
            hr = s.Device->CreateShaderResourceView(staged.Pyramid.Get(),&srv,&staged.MipSRVs[mip]); if (FAILED(hr)) return hr;
            D3D11_UNORDERED_ACCESS_VIEW_DESC uav{}; uav.Format = DXGI_FORMAT_R32_FLOAT; uav.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D; uav.Texture2D.MipSlice = mip;
            hr = s.Device->CreateUnorderedAccessView(staged.Pyramid.Get(),&uav,&staged.MipUAVs[mip]); if (FAILED(hr)) return hr;
        }
        staged.Device = s.Device; staged.Context = s.Context; staged.Copy = s.Copy; staged.Reduce = s.Reduce;
        s = std::move(staged);
    }
    ID3D11RenderTargetView* rtvs[8]{};
    s.Context->OMGetRenderTargets(8,rtvs,&s.SavedDSV);
    for (uint32_t i = 0; i < 8; ++i) s.SavedRTVs[i].Attach(rtvs[i]);
    s.SavedViewportCount = static_cast<uint32_t>(std::size(s.SavedViewports));
    s.Context->RSGetViewports(&s.SavedViewportCount,s.SavedViewports);
    s.Context->ClearDepthStencilView(s.DSV.Get(),D3D11_CLEAR_DEPTH,1.f,0);
    s.Context->OMSetRenderTargets(0,nullptr,s.DSV.Get());
    const D3D11_VIEWPORT viewport{0,0,static_cast<float>(width),static_cast<float>(height),0,1};
    s.Context->RSSetViewports(1,&viewport); s.InDepth = true;
    return S_OK;
}
HRESULT CGpuVisibilityHiZ::End_Depth(bool validContributors)
{
    auto& s = *m_Impl;
    if (!s.InDepth) return E_UNEXPECTED;
    // DSV를 해제한 뒤에만 같은 깊이를 SRV로 읽는다. 기존 MRT와 viewport는 항상 복원한다.
    ID3D11RenderTargetView* rtvs[8]{};
    for (uint32_t i = 0; i < 8; ++i) rtvs[i] = s.SavedRTVs[i].Get();
    s.Context->OMSetRenderTargets(8,rtvs,s.SavedDSV.Get());
    s.Context->RSSetViewports(s.SavedViewportCount,s.SavedViewports);
    for (auto& rtv : s.SavedRTVs) rtv.Reset(); s.SavedDSV.Reset(); s.InDepth = false; s.Ready = false;
    if (!validContributors) return S_FALSE;
    for (uint32_t mip = 0; mip < s.MipUAVs.size(); ++mip)
    {
        ID3D11ShaderResourceView* srv = mip == 0 ? s.DepthSRV.Get() : s.MipSRVs[mip-1].Get();
        ID3D11UnorderedAccessView* uav = s.MipUAVs[mip].Get();
        s.Context->CSSetShader(mip == 0 ? s.Copy.Get() : s.Reduce.Get(),nullptr,0);
        s.Context->CSSetShaderResources(0,1,&srv); s.Context->CSSetUnorderedAccessViews(0,1,&uav,nullptr);
        s.Context->Dispatch((std::max(1u,s.PaddedWidth>>mip)+7)/8,(std::max(1u,s.PaddedHeight>>mip)+7)/8,1);
        ClearCompute(s.Context.Get());
    }
    const HRESULT removed = s.Device->GetDeviceRemovedReason();
    if (FAILED(removed)) return removed;
    s.Ready = true; return S_OK;
}
ID3D11ShaderResourceView* CGpuVisibilityHiZ::Get_SRV() const noexcept { return m_Impl->Ready ? m_Impl->PyramidSRV.Get() : nullptr; }
uint32_t CGpuVisibilityHiZ::Get_MipCount() const noexcept { return m_Impl->Ready ? static_cast<uint32_t>(m_Impl->MipUAVs.size()) : 0; }
}
