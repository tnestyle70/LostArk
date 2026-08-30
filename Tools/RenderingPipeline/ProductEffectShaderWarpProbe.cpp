// Product shader closure owns this one-file WARP fixture. It intentionally
// compiles in a temporary directory and never recompiles Client product CPPs.
#include <Windows.h>

#include <d3d11.h>
#include <dxgi.h>
#include <wrl/client.h>

#include "Fx11/d3dx11effect.h"

#include <array>
#include <cmath>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

using Microsoft::WRL::ComPtr;
namespace fs = std::filesystem;

namespace
{
    constexpr int EXIT_RESOURCE_ROOT = 2;
    constexpr int EXIT_RENDER_CONTRACT = 3;

    struct VERTEX final
    {
        float Position[3];
        float Normal[3];
        float Tangent[3];
        float Binormal[3];
        float Texcoord[2];
    };

    struct RENDER_TARGETS final
    {
        std::array<ComPtr<ID3D11Texture2D>, 2> Textures;
        std::array<ComPtr<ID3D11RenderTargetView>, 2> Views;
        ComPtr<ID3D11Texture2D> Readback;
        ComPtr<ID3D11Texture2D> Depth;
        ComPtr<ID3D11DepthStencilView> DepthView;
        UINT Width = 64u;
        UINT Height = 64u;
    };

    std::wstring Get_Environment(const wchar_t* pName)
    {
        const DWORD Length = GetEnvironmentVariableW(pName, nullptr, 0u);
        if (0u == Length)
            return {};
        std::vector<wchar_t> Buffer(Length, L'\0');
        if (0u == GetEnvironmentVariableW(pName, Buffer.data(), Length))
            return {};
        return Buffer.data();
    }

    std::string To_Utf8(const std::wstring& Value)
    {
        if (Value.empty())
            return {};
        const int Bytes = WideCharToMultiByte(
            CP_UTF8, WC_ERR_INVALID_CHARS, Value.data(),
            static_cast<int>(Value.size()), nullptr, 0, nullptr, nullptr);
        if (Bytes <= 0)
            return {};
        std::string Result(static_cast<size_t>(Bytes), '\0');
        WideCharToMultiByte(
            CP_UTF8, WC_ERR_INVALID_CHARS, Value.data(),
            static_cast<int>(Value.size()), Result.data(), Bytes, nullptr, nullptr);
        return Result;
    }

    std::string Json_Escape(const std::string& Value)
    {
        std::ostringstream Output;
        for (const unsigned char Character : Value)
        {
            switch (Character)
            {
            case '\\': Output << "\\\\"; break;
            case '"': Output << "\\\""; break;
            case '\b': Output << "\\b"; break;
            case '\f': Output << "\\f"; break;
            case '\n': Output << "\\n"; break;
            case '\r': Output << "\\r"; break;
            case '\t': Output << "\\t"; break;
            default:
                if (Character < 0x20u)
                {
                    const char Hex[] = "0123456789abcdef";
                    Output << "\\u00" << Hex[(Character >> 4u) & 0xfu]
                           << Hex[Character & 0xfu];
                }
                else
                {
                    Output << static_cast<char>(Character);
                }
                break;
            }
        }
        return Output.str();
    }

    bool Is_Directory(const fs::path& Path)
    {
        std::error_code Error;
        return fs::is_directory(Path, Error) && !Error;
    }

    bool Is_Within(const fs::path& Root, const fs::path& Candidate)
    {
        std::error_code Error;
        const fs::path CanonicalRoot = fs::weakly_canonical(Root, Error);
        if (Error)
            return false;
        const fs::path CanonicalCandidate = fs::weakly_canonical(Candidate, Error);
        if (Error)
            return false;
        auto RootIterator = CanonicalRoot.begin();
        auto CandidateIterator = CanonicalCandidate.begin();
        for (; RootIterator != CanonicalRoot.end(); ++RootIterator, ++CandidateIterator)
        {
            if (CandidateIterator == CanonicalCandidate.end() ||
                0 != _wcsicmp(RootIterator->c_str(), CandidateIterator->c_str()))
                return false;
        }
        return true;
    }

    std::optional<fs::path> Resolve_Resource_Root(
        const fs::path& RepositoryRoot, std::string& OutError)
    {
        const std::array<const wchar_t*, 2> Names = {
            L"LOSTARK_RESOURCE_ROOT", L"LOSTARK_SHARED_ASSET_ROOT"};
        for (const wchar_t* pName : Names)
        {
            const std::wstring Configured = Get_Environment(pName);
            if (Configured.empty())
                continue;
            const fs::path Candidate = fs::path(Configured);
            if (!Is_Directory(Candidate))
            {
                OutError = "Product Effect resource root is unavailable: " +
                    To_Utf8(Candidate.wstring());
                return std::nullopt;
            }
            return fs::absolute(Candidate).lexically_normal();
        }
        const fs::path DefaultRoot = RepositoryRoot / L"Client/Bin/Resources";
        if (!Is_Directory(DefaultRoot))
        {
            OutError = "Product Effect resource root is unavailable: " +
                To_Utf8(DefaultRoot.wstring());
            return std::nullopt;
        }
        return fs::absolute(DefaultRoot).lexically_normal();
    }

    std::vector<unsigned char> Read_Bytes(
        const fs::path& Path, std::string& OutError)
    {
        std::ifstream Input(Path, std::ios::binary | std::ios::ate);
        if (!Input)
        {
            OutError = "compiled effect is missing: " + To_Utf8(Path.wstring());
            return {};
        }
        const std::streamoff Size = Input.tellg();
        if (Size <= 0)
        {
            OutError = "compiled effect is empty: " + To_Utf8(Path.wstring());
            return {};
        }
        std::vector<unsigned char> Bytes(static_cast<size_t>(Size));
        Input.seekg(0, std::ios::beg);
        if (!Input.read(reinterpret_cast<char*>(Bytes.data()), Size))
        {
            OutError = "compiled effect could not be read: " +
                To_Utf8(Path.wstring());
            return {};
        }
        return Bytes;
    }

    bool Create_Targets(
        ID3D11Device* pDevice, RENDER_TARGETS& OutTargets, std::string& OutError)
    {
        D3D11_TEXTURE2D_DESC ColorDesc{};
        ColorDesc.Width = OutTargets.Width;
        ColorDesc.Height = OutTargets.Height;
        ColorDesc.MipLevels = 1u;
        ColorDesc.ArraySize = 1u;
        ColorDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
        ColorDesc.SampleDesc.Count = 1u;
        ColorDesc.Usage = D3D11_USAGE_DEFAULT;
        ColorDesc.BindFlags = D3D11_BIND_RENDER_TARGET;
        for (size_t Index = 0u; Index < OutTargets.Textures.size(); ++Index)
        {
            if (FAILED(pDevice->CreateTexture2D(
                    &ColorDesc, nullptr, OutTargets.Textures[Index].GetAddressOf())) ||
                FAILED(pDevice->CreateRenderTargetView(
                    OutTargets.Textures[Index].Get(), nullptr,
                    OutTargets.Views[Index].GetAddressOf())))
            {
                OutError = "WARP render target creation failed";
                return false;
            }
        }
        D3D11_TEXTURE2D_DESC ReadbackDesc = ColorDesc;
        ReadbackDesc.Usage = D3D11_USAGE_STAGING;
        ReadbackDesc.BindFlags = 0u;
        ReadbackDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
        if (FAILED(pDevice->CreateTexture2D(
                &ReadbackDesc, nullptr, OutTargets.Readback.GetAddressOf())))
        {
            OutError = "WARP readback target creation failed";
            return false;
        }
        D3D11_TEXTURE2D_DESC DepthDesc{};
        DepthDesc.Width = OutTargets.Width;
        DepthDesc.Height = OutTargets.Height;
        DepthDesc.MipLevels = 1u;
        DepthDesc.ArraySize = 1u;
        DepthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        DepthDesc.SampleDesc.Count = 1u;
        DepthDesc.Usage = D3D11_USAGE_DEFAULT;
        DepthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        if (FAILED(pDevice->CreateTexture2D(
                &DepthDesc, nullptr, OutTargets.Depth.GetAddressOf())) ||
            FAILED(pDevice->CreateDepthStencilView(
                OutTargets.Depth.Get(), nullptr, OutTargets.DepthView.GetAddressOf())))
        {
            OutError = "WARP depth-stencil creation failed";
            return false;
        }
        return true;
    }

    bool Create_White_Texture(
        ID3D11Device* pDevice,
        ComPtr<ID3D11ShaderResourceView>& OutView,
        std::string& OutError)
    {
        constexpr UINT White = 0xffffffffu;
        D3D11_TEXTURE2D_DESC Desc{};
        Desc.Width = 1u;
        Desc.Height = 1u;
        Desc.MipLevels = 1u;
        Desc.ArraySize = 1u;
        Desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        Desc.SampleDesc.Count = 1u;
        Desc.Usage = D3D11_USAGE_IMMUTABLE;
        Desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        D3D11_SUBRESOURCE_DATA Initial{};
        Initial.pSysMem = &White;
        Initial.SysMemPitch = sizeof(White);
        ComPtr<ID3D11Texture2D> Texture;
        if (FAILED(pDevice->CreateTexture2D(&Desc, &Initial, Texture.GetAddressOf())) ||
            FAILED(pDevice->CreateShaderResourceView(
                Texture.Get(), nullptr, OutView.GetAddressOf())))
        {
            OutError = "WARP white fixture creation failed";
            return false;
        }
        return true;
    }

    bool Set_Matrix(
        ID3DX11Effect* pEffect, const char* pName,
        const std::array<float, 16>& Matrix, std::string& OutError)
    {
        ID3DX11EffectMatrixVariable* pVariable =
            pEffect->GetVariableByName(pName)->AsMatrix();
        if (!pVariable->IsValid() || FAILED(pVariable->SetMatrix(Matrix.data())))
        {
            OutError = std::string("effect matrix is unavailable: ") + pName;
            return false;
        }
        return true;
    }

    bool Set_Float4(
        ID3DX11Effect* pEffect, const char* pName,
        const std::array<float, 4>& Value, std::string& OutError)
    {
        ID3DX11EffectVectorVariable* pVariable =
            pEffect->GetVariableByName(pName)->AsVector();
        if (!pVariable->IsValid() || FAILED(pVariable->SetFloatVector(Value.data())))
        {
            OutError = std::string("effect vector is unavailable: ") + pName;
            return false;
        }
        return true;
    }

    bool Set_Int4(
        ID3DX11Effect* pEffect, const char* pName,
        const std::array<int, 4>& Value, std::string& OutError)
    {
        ID3DX11EffectVectorVariable* pVariable =
            pEffect->GetVariableByName(pName)->AsVector();
        if (!pVariable->IsValid() || FAILED(pVariable->SetIntVector(Value.data())))
        {
            OutError = std::string("effect uint vector is unavailable: ") + pName;
            return false;
        }
        return true;
    }

    bool Prepare_V1(
        ID3DX11Effect* pEffect,
        ID3D11ShaderResourceView* pWhite,
        std::string& OutError)
    {
        const std::array<float, 16> Identity = {
            1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f,
            0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f};
        if (!Set_Matrix(pEffect, "g_WorldMatrix", Identity, OutError) ||
            !Set_Matrix(pEffect, "g_NormalMatrix", Identity, OutError) ||
            !Set_Matrix(pEffect, "g_ViewMatrix", Identity, OutError) ||
            !Set_Matrix(pEffect, "g_ProjMatrix", Identity, OutError) ||
            !Set_Float4(pEffect, "g_ColorMultiply", {1.f, 1.f, 1.f, 1.f}, OutError) ||
            !Set_Float4(pEffect, "g_ColorOffset", {0.f, 0.f, 0.f, 0.f}, OutError) ||
            !Set_Int4(pEffect, "g_StandardColorV1Header", {1, 1, 1, 1}, OutError) ||
            !Set_Int4(pEffect, "g_StandardColorV1BaseCoverage", {0, 5, 0, 4}, OutError) ||
            !Set_Int4(pEffect, "g_StandardColorV1Dissolve", {0, -1, 0, 1}, OutError) ||
            !Set_Int4(pEffect, "g_StandardColorV1Policies", {0, 1, 1, 1}, OutError) ||
            !Set_Float4(pEffect, "g_StandardColorV1Scalars", {0.f, 0.f, 0.f, 0.f}, OutError))
            return false;

        ID3DX11EffectScalarVariable* pEnabled =
            pEffect->GetVariableByName("g_StandardColorV1Enabled")->AsScalar();
        ID3DX11EffectShaderResourceVariable* pTexture =
            pEffect->GetVariableByName("g_SourceTexture0")->AsShaderResource();
        if (!pEnabled->IsValid() || FAILED(pEnabled->SetInt(1)) ||
            !pTexture->IsValid() || FAILED(pTexture->SetResource(pWhite)))
        {
            OutError = "V1 packet or texture binding is unavailable";
            return false;
        }
        return true;
    }

    bool Prepare_V2(ID3DX11Effect* pEffect, std::string& OutError)
    {
        const std::array<float, 16> Identity = {
            1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f,
            0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f};
        return Set_Matrix(pEffect, "g_WorldMatrix", Identity, OutError) &&
            Set_Matrix(pEffect, "g_ViewMatrix", Identity, OutError) &&
            Set_Matrix(pEffect, "g_ProjMatrix", Identity, OutError) &&
            Set_Float4(pEffect, "g_ColorMul", {1.f, 1.f, 1.f, 1.f}, OutError) &&
            Set_Float4(pEffect, "g_ColorOffset", {0.f, 0.f, 0.f, 0.f}, OutError);
    }

    bool Count_Lit_Pixels(
        ID3D11DeviceContext* pContext,
        RENDER_TARGETS& Targets,
        size_t& OutLitPixels,
        std::string& OutError)
    {
        pContext->CopyResource(Targets.Readback.Get(), Targets.Textures[0].Get());
        D3D11_MAPPED_SUBRESOURCE Mapped{};
        if (FAILED(pContext->Map(
                Targets.Readback.Get(), 0u, D3D11_MAP_READ, 0u, &Mapped)))
        {
            OutError = "WARP readback map failed";
            return false;
        }
        OutLitPixels = 0u;
        for (UINT Y = 0u; Y < Targets.Height; ++Y)
        {
            const float* pRow = reinterpret_cast<const float*>(
                static_cast<const unsigned char*>(Mapped.pData) +
                static_cast<size_t>(Y) * Mapped.RowPitch);
            for (UINT X = 0u; X < Targets.Width; ++X)
            {
                const float* pPixel = pRow + static_cast<size_t>(X) * 4u;
                if (std::isfinite(pPixel[0]) && std::isfinite(pPixel[1]) &&
                    std::isfinite(pPixel[2]) && std::isfinite(pPixel[3]) &&
                    (pPixel[0] > 0.01f || pPixel[1] > 0.01f ||
                     pPixel[2] > 0.01f || pPixel[3] > 0.01f))
                    ++OutLitPixels;
            }
        }
        pContext->Unmap(Targets.Readback.Get(), 0u);
        if (0u == OutLitPixels)
        {
            OutError = "compiled effect produced zero visible WARP pixels";
            return false;
        }
        return true;
    }

    bool Draw_Compiled_Effect(
        ID3D11Device* pDevice,
        ID3D11DeviceContext* pContext,
        RENDER_TARGETS& Targets,
        ID3D11Buffer* pVertexBuffer,
        ID3D11ShaderResourceView* pWhite,
        const fs::path& CompiledEffect,
        const char* pPassName,
        bool IsV1,
        size_t& OutLitPixels,
        std::string& OutError)
    {
        const std::vector<unsigned char> Bytes = Read_Bytes(CompiledEffect, OutError);
        if (Bytes.empty())
            return false;
        ComPtr<ID3DX11Effect> Effect;
        if (FAILED(D3DX11CreateEffectFromMemory(
                Bytes.data(), Bytes.size(), 0u, pDevice, Effect.GetAddressOf())))
        {
            OutError = "compiled effect could not be created: " +
                To_Utf8(CompiledEffect.wstring());
            return false;
        }
        ID3DX11EffectTechnique* pTechnique =
            Effect->GetTechniqueByName("DefaultTechnique");
        ID3DX11EffectPass* pPass = pTechnique->GetPassByName(pPassName);
        if (!pTechnique->IsValid() || !pPass->IsValid())
        {
            OutError = std::string("compiled effect pass is missing: ") + pPassName;
            return false;
        }
        D3DX11_PASS_DESC PassDesc{};
        if (FAILED(pPass->GetDesc(&PassDesc)))
        {
            OutError = "compiled effect pass signature is unavailable";
            return false;
        }
        const std::array<D3D11_INPUT_ELEMENT_DESC, 5> Elements = {{
            {"POSITION", 0u, DXGI_FORMAT_R32G32B32_FLOAT, 0u,
                static_cast<UINT>(offsetof(VERTEX, Position)),
                D3D11_INPUT_PER_VERTEX_DATA, 0u},
            {"NORMAL", 0u, DXGI_FORMAT_R32G32B32_FLOAT, 0u,
                static_cast<UINT>(offsetof(VERTEX, Normal)),
                D3D11_INPUT_PER_VERTEX_DATA, 0u},
            {"TANGENT", 0u, DXGI_FORMAT_R32G32B32_FLOAT, 0u,
                static_cast<UINT>(offsetof(VERTEX, Tangent)),
                D3D11_INPUT_PER_VERTEX_DATA, 0u},
            {"BINORMAL", 0u, DXGI_FORMAT_R32G32B32_FLOAT, 0u,
                static_cast<UINT>(offsetof(VERTEX, Binormal)),
                D3D11_INPUT_PER_VERTEX_DATA, 0u},
            {"TEXCOORD", 0u, DXGI_FORMAT_R32G32_FLOAT, 0u,
                static_cast<UINT>(offsetof(VERTEX, Texcoord)),
                D3D11_INPUT_PER_VERTEX_DATA, 0u},
        }};
        ComPtr<ID3D11InputLayout> InputLayout;
        if (FAILED(pDevice->CreateInputLayout(
                Elements.data(), static_cast<UINT>(Elements.size()),
                PassDesc.pIAInputSignature, PassDesc.IAInputSignatureSize,
                InputLayout.GetAddressOf())))
        {
            OutError = "compiled effect input signature rejected the fixture";
            return false;
        }
        if (!(IsV1 ? Prepare_V1(Effect.Get(), pWhite, OutError) :
                     Prepare_V2(Effect.Get(), OutError)))
            return false;

        const std::array<float, 4> Clear = {0.f, 0.f, 0.f, 0.f};
        pContext->ClearRenderTargetView(Targets.Views[0].Get(), Clear.data());
        pContext->ClearRenderTargetView(Targets.Views[1].Get(), Clear.data());
        pContext->ClearDepthStencilView(
            Targets.DepthView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
            1.f, 0u);
        ID3D11RenderTargetView* Views[] = {
            Targets.Views[0].Get(), Targets.Views[1].Get()};
        pContext->OMSetRenderTargets(2u, Views, Targets.DepthView.Get());
        D3D11_VIEWPORT Viewport{};
        Viewport.Width = static_cast<float>(Targets.Width);
        Viewport.Height = static_cast<float>(Targets.Height);
        Viewport.MaxDepth = 1.f;
        pContext->RSSetViewports(1u, &Viewport);
        const UINT Stride = sizeof(VERTEX);
        const UINT Offset = 0u;
        pContext->IASetInputLayout(InputLayout.Get());
        pContext->IASetVertexBuffers(0u, 1u, &pVertexBuffer, &Stride, &Offset);
        pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        if (FAILED(pPass->Apply(0u, pContext)))
        {
            OutError = "compiled effect pass application failed";
            return false;
        }
        pContext->Draw(3u, 0u);
        pContext->OMSetRenderTargets(0u, nullptr, nullptr);
        return Count_Lit_Pixels(pContext, Targets, OutLitPixels, OutError);
    }
}

int wmain(int ArgumentCount, wchar_t** ppArguments)
{
    if (ArgumentCount < 2)
    {
        std::cerr << "usage: ProductEffectShaderWarpProbe <repository-root> "
                     "[Debug|Release] [--validate-resource-root]\n";
        return 64;
    }
    const fs::path RepositoryRoot = fs::absolute(ppArguments[1]);
    std::wstring Configuration = L"Debug";
    bool ValidateResourceRootOnly = false;
    for (int Index = 2; Index < ArgumentCount; ++Index)
    {
        if (0 == _wcsicmp(ppArguments[Index], L"--validate-resource-root"))
            ValidateResourceRootOnly = true;
        else
            Configuration = ppArguments[Index];
    }
    if (0 != _wcsicmp(Configuration.c_str(), L"Debug") &&
        0 != _wcsicmp(Configuration.c_str(), L"Release"))
    {
        std::cerr << "configuration must be Debug or Release\n";
        return 64;
    }

    std::string Error;
    const std::optional<fs::path> ResourceRoot =
        Resolve_Resource_Root(RepositoryRoot, Error);
    if (!ResourceRoot.has_value())
    {
        std::cerr << Error << '\n';
        return EXIT_RESOURCE_ROOT;
    }
    const bool BoundaryValidated =
        Is_Within(*ResourceRoot, *ResourceRoot / L"Effect") &&
        !Is_Within(*ResourceRoot, ResourceRoot->parent_path());
    if (!BoundaryValidated)
    {
        std::cerr << "Product Effect resource asset path boundary validation failed\n";
        return EXIT_RESOURCE_ROOT;
    }
    if (ValidateResourceRootOnly)
    {
        std::cout << "{\"resourceRoot\":\""
                  << Json_Escape(To_Utf8(ResourceRoot->wstring()))
                  << "\",\"assetPathBoundaryValidated\":true}\n";
        return 0;
    }

    D3D_FEATURE_LEVEL FeatureLevel{};
    ComPtr<ID3D11Device> Device;
    ComPtr<ID3D11DeviceContext> Context;
    if (FAILED(D3D11CreateDevice(
            nullptr, D3D_DRIVER_TYPE_WARP, nullptr, 0u, nullptr, 0u,
            D3D11_SDK_VERSION, Device.GetAddressOf(), &FeatureLevel,
            Context.GetAddressOf())) || FeatureLevel < D3D_FEATURE_LEVEL_11_0)
    {
        std::cerr << "Microsoft WARP D3D11 device creation failed\n";
        return EXIT_RENDER_CONTRACT;
    }

    RENDER_TARGETS Targets;
    ComPtr<ID3D11ShaderResourceView> White;
    if (!Create_Targets(Device.Get(), Targets, Error) ||
        !Create_White_Texture(Device.Get(), White, Error))
    {
        std::cerr << Error << '\n';
        return EXIT_RENDER_CONTRACT;
    }
    const std::array<VERTEX, 3> Vertices = {{
        {{-0.8f, -0.8f, 0.5f}, {0.f, 0.f, -1.f}, {1.f, 0.f, 0.f},
            {0.f, -1.f, 0.f}, {0.f, 1.f}},
        {{0.f, 0.8f, 0.5f}, {0.f, 0.f, -1.f}, {1.f, 0.f, 0.f},
            {0.f, -1.f, 0.f}, {0.5f, 0.f}},
        {{0.8f, -0.8f, 0.5f}, {0.f, 0.f, -1.f}, {1.f, 0.f, 0.f},
            {0.f, -1.f, 0.f}, {1.f, 1.f}},
    }};
    D3D11_BUFFER_DESC BufferDesc{};
    BufferDesc.ByteWidth = sizeof(Vertices);
    BufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
    BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    D3D11_SUBRESOURCE_DATA VertexData{};
    VertexData.pSysMem = Vertices.data();
    ComPtr<ID3D11Buffer> VertexBuffer;
    if (FAILED(Device->CreateBuffer(
            &BufferDesc, &VertexData, VertexBuffer.GetAddressOf())))
    {
        std::cerr << "WARP vertex fixture creation failed\n";
        return EXIT_RENDER_CONTRACT;
    }

    const fs::path ShaderRoot = RepositoryRoot / L"Client/Bin" / Configuration;
    size_t V1LitPixels = 0u;
    size_t V2LitPixels = 0u;
    if (!Draw_Compiled_Effect(
            Device.Get(), Context.Get(), Targets, VertexBuffer.Get(), White.Get(),
            ShaderRoot / L"Shader_VtxEffectMeshPreview.cso",
            "OpaqueBackDepthWrite", true, V1LitPixels, Error) ||
        !Draw_Compiled_Effect(
            Device.Get(), Context.Get(), Targets, VertexBuffer.Get(), White.Get(),
            ShaderRoot / L"Shader_EffectMeshV2.cso",
            "AlphaNoDepth", false, V2LitPixels, Error))
    {
        std::cerr << Error << '\n';
        return EXIT_RENDER_CONTRACT;
    }

    ComPtr<IDXGIDevice> DxgiDevice;
    ComPtr<IDXGIAdapter> Adapter;
    DXGI_ADAPTER_DESC AdapterDesc{};
    Device.As(&DxgiDevice);
    if (DxgiDevice && SUCCEEDED(DxgiDevice->GetAdapter(Adapter.GetAddressOf())) && Adapter)
        Adapter->GetDesc(&AdapterDesc);
    std::cout << "{\"driver\":\"WARP\",\"adapter\":\""
              << Json_Escape(To_Utf8(AdapterDesc.Description))
              << "\",\"v1LitPixels\":" << V1LitPixels
              << ",\"v2LitPixels\":" << V2LitPixels
              << ",\"assetPathBoundaryValidated\":true"
              << ",\"resourceRoot\":\""
              << Json_Escape(To_Utf8(ResourceRoot->wstring())) << "\"}\n";
    return 0;
}
