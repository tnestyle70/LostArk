// Headless verification of the product CShader path; no window, draw or capture.
#include "Shader.h"
#include "Engine_VertexTypes.h"
#include <array>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <stdexcept>

using namespace Engine;

static void Require(bool condition, const char* message)
{
    if (!condition) throw std::runtime_error(message);
}

static bool ContainsBytes(ID3D11Device* device, ID3D11DeviceContext* context,
    ID3D11Buffer* buffer, const void* expected, size_t size)
{
    if (!buffer) return false;
    D3D11_BUFFER_DESC descriptor{};
    buffer->GetDesc(&descriptor);
    if (size > descriptor.ByteWidth) return false;
    descriptor.Usage = D3D11_USAGE_STAGING;
    descriptor.BindFlags = 0;
    descriptor.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    descriptor.MiscFlags = 0;
    ComPtr<ID3D11Buffer> staging;
    Require(SUCCEEDED(device->CreateBuffer(&descriptor, nullptr, &staging)), "staging buffer creation failed");
    context->CopyResource(staging.Get(), buffer);
    D3D11_MAPPED_SUBRESOURCE mapped{};
    Require(SUCCEEDED(context->Map(staging.Get(), 0, D3D11_MAP_READ, 0, &mapped)), "constant readback failed");
    bool found = false;
    for (size_t offset = 0; offset + size <= descriptor.ByteWidth; offset += sizeof(float))
        if (0 == std::memcmp(static_cast<const uint8_t*>(mapped.pData) + offset, expected, size)) { found = true; break; }
    context->Unmap(staging.Get(), 0);
    return found;
}

static bool ConstantsBound(ID3D11Device* device, ID3D11DeviceContext* context,
    bool vertex, const void* expected, size_t size)
{
    ID3D11Buffer* buffers[D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT]{};
    if (vertex) context->VSGetConstantBuffers(0, std::size(buffers), buffers);
    else context->PSGetConstantBuffers(0, std::size(buffers), buffers);
    bool found = false;
    for (auto* buffer : buffers)
    {
        if (buffer) { found |= ContainsBytes(device, context, buffer, expected, size); buffer->Release(); }
    }
    return found;
}

static bool TextureBound(ID3D11DeviceContext* context, ID3D11ShaderResourceView* expected)
{
    ID3D11ShaderResourceView* resources[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT]{};
    context->PSGetShaderResources(0, std::size(resources), resources);
    bool found = false;
    for (auto* resource : resources)
    {
        found |= resource && resource == expected;
        if (resource) resource->Release();
    }
    return found;
}

template<class Function> static double Measure(Function&& function)
{
    constexpr uint32_t iterations = 2000u;
    for (uint32_t i = 0; i < 100u; ++i) function(i);
    const auto start = std::chrono::steady_clock::now();
    for (uint32_t i = 0; i < iterations; ++i) function(i);
    return std::chrono::duration<double, std::micro>(std::chrono::steady_clock::now() - start).count() / iterations;
}

int wmain(int argc, wchar_t** argv)
{
    try
    {
        Require(argc == 2, "expected isolated compiled shader directory");
        const std::filesystem::path directory = argv[1];
        ComPtr<ID3D11Device> device;
        ComPtr<ID3D11DeviceContext> context;
        D3D_FEATURE_LEVEL level{};
        Require(SUCCEEDED(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, 0,
            nullptr, 0, D3D11_SDK_VERSION, &device, &level, &context)), "WARP device creation failed");
        D3D11_TEXTURE2D_DESC textureDesc{};
        textureDesc.Width = textureDesc.Height = textureDesc.MipLevels = textureDesc.ArraySize = 1;
        textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        textureDesc.SampleDesc.Count = 1;
        textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        ComPtr<ID3D11Texture2D> texture;
        ComPtr<ID3D11ShaderResourceView> view;
        Require(SUCCEEDED(device->CreateTexture2D(&textureDesc, nullptr, &texture)) &&
            SUCCEEDED(device->CreateShaderResourceView(texture.Get(), nullptr, &view)), "texture fixture creation failed");
        struct Fixture { const wchar_t* name; const D3D11_INPUT_ELEMENT_DESC* elements; uint32_t count; uint32_t pass; bool light; bool bones; };
        const Fixture fixtures[] = {
            {L"Shader_VtxAnimMeshBinary", VTXANIMMESH::Elements, VTXANIMMESH::iNumElements, 0u, false, true},
            {L"Shader_VtxMeshBinary", VTXMESH::Elements, VTXMESH::iNumElements, 0u, false, false},
            {L"Shader_Deferred", VTXTEX::Elements, VTXTEX::iNumElements, 19u, true, false}
        };
        uint32_t programsChecked = 0u, clonesChecked = 0u, failureCases = 0u, lightPassesChecked = 0u;
        uint32_t nativePassesChecked = 0u, unavailablePassesChecked = 0u;
        uint32_t afterimagePassesChecked = 0u;
        for (const auto& fixture : fixtures)
        {
            const std::wstring logical = std::wstring(fixture.name) + L".hlsl";
            auto prototype = CShader::Create(device, context, logical.c_str(), fixture.elements, fixture.count);
            Require(prototype != nullptr, "product CShader creation / cohort ABI verification failed");
            auto first = std::dynamic_pointer_cast<CShader>(prototype->Clone(nullptr));
            auto second = std::dynamic_pointer_cast<CShader>(prototype->Clone(nullptr));
            Require(first && second, "product CShader clone failed");
            prototype.reset();
            const char* constantsName = fixture.light ? "g_SourceCharacterLightConstants" : "g_SourceCharacterBaseConstants";
            std::array<float, 256> constants{};
            std::vector<float4x4_t> bones(512);
            if (fixture.bones)
            {
                for (size_t i = 0; i < bones.size(); ++i)
                    for (auto& row : bones[i].m) for (float& value : row) value = 20.25f + static_cast<float>(i);
                Require(SUCCEEDED(first->Bind_Matrices("g_BoneMatrices", bones.data(), static_cast<uint32_t>(bones.size()))), "bone matrix array bind failed");
            }
            ComPtr<ID3D11PixelShader> cohortShader;
            for (uint32_t program = 1u; program <= 89u; ++program)
            {
                if (program > 32u && program < 80u) continue;
                for (size_t i = 0; i < constants.size(); ++i) constants[i] = 1000.125f + static_cast<float>(program * 300u + i);
                Require(SUCCEEDED(first->Bind_RawValue(constantsName, constants.data(), sizeof(constants))) &&
                    SUCCEEDED(first->Bind_Texture("g_SourceCharacterTexture0", view)) &&
                    SUCCEEDED(first->Bind_RawValue("g_SourceCharacterProgram", &program, sizeof(program))), "source material bind failed");
                Require(SUCCEEDED(first->Begin(fixture.pass)), "source cohort pass failed");
                Require(ConstantsBound(device.Get(), context.Get(), false, constants.data(), sizeof(constants)), "native constants were not transferred to selected PS");
                Require(TextureBound(context.Get(), view.Get()), "native texture was not transferred to selected PS");
                if (fixture.bones) Require(ConstantsBound(device.Get(), context.Get(), true, bones.data(), sizeof(float4x4_t) * bones.size()), "bone array was not transferred to selected VS");
                if (fixture.light)
                {
                    for (uint32_t pass : {1u, 2u, 16u, 19u, 20u, 21u, 22u, 23u, 24u, 25u, 26u, 27u})
                    {
                        Require(SUCCEEDED(first->Begin(pass)), "legacy / instanced / mask fallback light pass failed");
                        ++lightPassesChecked;
                    }
                    Require(SUCCEEDED(first->Begin(fixture.pass)), "restore primary light pass failed");
                }
                if (fixture.bones && (program == 18u || program == 88u))
                {
                    Require(SUCCEEDED(first->Bind_RawValue("g_SourceCharacterLightConstants", constants.data(), sizeof(constants))), "translucent hair light constants bind failed");
                    for (uint32_t pass : {9u, 10u})
                        Require(SUCCEEDED(first->Begin(pass)) && ConstantsBound(device.Get(), context.Get(), false, constants.data(), sizeof(constants)), "translucent hair light pass lost its constants");
                    Require(SUCCEEDED(first->Begin(fixture.pass)), "restore primary geometry pass failed");
                }
                ComPtr<ID3D11PixelShader> active;
                context->PSGetShader(&active, nullptr, nullptr);
                Require(active != nullptr, "selected pass has no PS");
                Require(SUCCEEDED(second->Begin(fixture.pass)), "clone did not observe shared source program / constants");
                context->PSGetShader(&cohortShader, nullptr, nullptr);
                Require(active == cohortShader, "clones selected different FX state for the same cohort");
                ++programsChecked;
            }
            uint32_t program = 9u;
            for (size_t i = 0; i < constants.size(); ++i) constants[i] = 80000.25f + static_cast<float>(i);
            Require(SUCCEEDED(second->Bind_RawValue(constantsName, constants.data(), sizeof(constants))) &&
                SUCCEEDED(second->Bind_RawValue("g_SourceCharacterProgram", &program, sizeof(program))) &&
                SUCCEEDED(first->Begin(fixture.pass)), "clone cohort switch failed");
            Require(ConstantsBound(device.Get(), context.Get(), false, constants.data(), sizeof(constants)), "older clone revision hid another clone's update");
            clonesChecked += 2u;
            if (fixture.bones)
            {
                auto directA = CShader::Create(device, context, L"Shader_VtxAnimMeshBinary_SourceGroup001.hlsl", fixture.elements, fixture.count);
                auto directB = CShader::Create(device, context, L"Shader_VtxAnimMeshBinary_SourceGroup009.hlsl", fixture.elements, fixture.count);
                Require(directA && directB, "direct cohort benchmark creation failed");

                // Source groups have no base-owned native cue or afterimage PS. Direct callers must fail
                // before applying an invisible pass, while the owning base FX
                // must select its native PS despite a stale surface program.
                for (uint32_t pass : {7u, 8u, 12u, 13u, 14u})
                {
                    Require(FAILED(directA->Begin(pass)) && FAILED(directB->Begin(pass)),
                        "direct source group accepted a base-owned native cue pass");
                    unavailablePassesChecked += 2u;
                }
                std::array<float, 128> nativeConstants{};
                for (size_t i = 0; i < nativeConstants.size(); ++i)
                    nativeConstants[i] = 30000.25f + static_cast<float>(i);
                Require(SUCCEEDED(first->Bind_RawValue("g_ArtistSourceMaterialParameters",
                    nativeConstants.data(), sizeof(nativeConstants))) &&
                    SUCCEEDED(first->Bind_Texture("g_SourceTexture0", view)), "native cue fixture binding failed");
                for (uint32_t nativeProfile : {178u, 460u, 461u, 772u, 1360u, 3828u, 3831u, 3832u, 3833u})
                {
                    Require(SUCCEEDED(first->Bind_RawValue("g_ArtistModelCueProfile",
                        &nativeProfile, sizeof(nativeProfile))), "native cue profile bind failed");
                    uint32_t surfaceProgram = 0u;
                    Require(SUCCEEDED(first->Bind_RawValue("g_SourceCharacterProgram", &surfaceProgram,
                        sizeof(surfaceProgram))) && SUCCEEDED(first->Begin(7u)), "base native cue pass failed");
                    ComPtr<ID3D11PixelShader> expectedNative;
                    context->PSGetShader(&expectedNative, nullptr, nullptr);
                    Require(expectedNative != nullptr, "base native cue PS missing");
                    for (uint32_t staleProgram : {1u, 9u, 18u, 88u})
                    {
                        Require(SUCCEEDED(first->Bind_RawValue("g_SourceCharacterProgram", &staleProgram,
                            sizeof(staleProgram))), "stale source program fixture bind failed");
                        for (uint32_t pass : {7u, 8u, 12u, 13u})
                        {
                            Require(SUCCEEDED(second->Begin(pass)), "shared clone native cue pass failed");
                            ComPtr<ID3D11PixelShader> actualNative;
                            context->PSGetShader(&actualNative, nullptr, nullptr);
                            Require(actualNative == expectedNative, "native cue selected a source-group PS");
                            Require(ConstantsBound(device.Get(), context.Get(), false, nativeConstants.data(),
                                sizeof(nativeConstants)) && TextureBound(context.Get(), view.Get()),
                                "native cue lost base-owned constants or textures");
                            Require(ConstantsBound(device.Get(), context.Get(), true, bones.data(),
                                sizeof(float4x4_t) * bones.size()), "native cue lost base-owned bone matrices");
                            ++nativePassesChecked;
                        }
                    }
                }
                const std::array<float, 4> afterimageColor{1.8125f, 1.6875f, 1.5625f, .3125f};
                uint32_t surfaceProgram = 0u;
                Require(SUCCEEDED(first->Bind_RawValue("g_ChargeAfterimageColor", afterimageColor.data(),
                    sizeof(afterimageColor))) && SUCCEEDED(first->Bind_RawValue("g_SourceCharacterProgram",
                    &surfaceProgram, sizeof(surfaceProgram))) && SUCCEEDED(first->Begin(14u)),
                    "base afterimage pass failed");
                ComPtr<ID3D11PixelShader> expectedAfterimage;
                context->PSGetShader(&expectedAfterimage, nullptr, nullptr);
                Require(expectedAfterimage != nullptr, "base afterimage PS missing");
                for (uint32_t staleProgram : {1u, 9u, 18u, 88u})
                {
                    Require(SUCCEEDED(first->Bind_RawValue("g_SourceCharacterProgram", &staleProgram,
                        sizeof(staleProgram))) && SUCCEEDED(second->Begin(14u)),
                        "shared clone afterimage pass failed with a stale source program");
                    ComPtr<ID3D11PixelShader> actualAfterimage;
                    context->PSGetShader(&actualAfterimage, nullptr, nullptr);
                    Require(actualAfterimage == expectedAfterimage, "afterimage selected a source-group PS");
                    Require(ConstantsBound(device.Get(), context.Get(), false, afterimageColor.data(),
                        sizeof(afterimageColor)), "afterimage lost base-owned color/fade constants");
                    Require(ConstantsBound(device.Get(), context.Get(), true, bones.data(),
                        sizeof(float4x4_t) * bones.size()), "afterimage lost base-owned bone matrices");
                    ++afterimagePassesChecked;
                }
                for (bool updates : {false, true}) for (bool switching : {false, true})
                {
                    const auto execute = [&](CShader& shader, uint32_t i)
                    {
                        const uint32_t selected = switching && (i & 1u) ? 9u : 1u;
                        Require(SUCCEEDED(shader.Bind_RawValue("g_SourceCharacterProgram", &selected, sizeof(selected))), "benchmark program bind failed");
                        if (updates)
                        {
                            constants[0] = static_cast<float>(i);
                            Require(SUCCEEDED(shader.Bind_RawValue(constantsName, constants.data(), sizeof(constants))) &&
                                SUCCEEDED(shader.Bind_Matrices("g_BoneMatrices", bones.data(), static_cast<uint32_t>(bones.size()))), "benchmark constant/bone bind failed");
                        }
                        Require(SUCCEEDED(shader.Begin(fixture.pass)), "benchmark pass failed");
                    };
                    const double directUs = Measure([&](uint32_t i) { execute(switching && (i & 1u) ? *directB : *directA, i); });
                    const double groupedUs = Measure([&](uint32_t i) { execute(*first, i); });
                    std::printf("{\"microbench\":true,\"updates\":%s,\"switching\":%s,\"directUs\":%.3f,\"groupedUs\":%.3f}\n",
                        updates ? "true" : "false", switching ? "true" : "false", directUs, groupedUs);
                }
                // Restore the expected shared clone fixture after the benchmark.
                for (size_t i = 0; i < constants.size(); ++i) constants[i] = 80000.25f + static_cast<float>(i);
                Require(SUCCEEDED(first->Bind_RawValue(constantsName, constants.data(), sizeof(constants))), "post-benchmark fixture restore failed");
            }
            program = 66u;
            Require(SUCCEEDED(first->Bind_RawValue("g_SourceCharacterProgram", &program, sizeof(program))) &&
                FAILED(first->Begin(fixture.pass)), "unknown program did not fail closed");
            ++failureCases;
            program = 9u;
            Require(SUCCEEDED(first->Bind_RawValue("g_SourceCharacterProgram", &program, sizeof(program))), "restore valid program failed");
            const auto shard = directory / (std::wstring(fixture.name) + L"_SourceGroup001.cso");
            const auto saved = directory / (std::wstring(fixture.name) + L"_SourceGroup001.saved");
            std::filesystem::rename(shard, saved);
            const auto missing = CShader::Create(device, context, logical.c_str(), fixture.elements, fixture.count);
            const HRESULT reloaded = first->Initialize_Prototype(logical.c_str(), fixture.elements, fixture.count);
            std::filesystem::rename(saved, shard);
            Require(missing == nullptr && FAILED(reloaded), "missing cohort was accepted");
            Require(SUCCEEDED(first->Begin(fixture.pass)) &&
                ConstantsBound(device.Get(), context.Get(), false, constants.data(), sizeof(constants)), "failed reload replaced existing valid shader state");
            failureCases += 2u;
            std::printf("checked source shader %ls\n", fixture.name);
        }
        std::printf("{\"programsChecked\":%u,\"clonesChecked\":%u,\"failureCases\":%u,\"lightPassesChecked\":%u,\"nativePassesChecked\":%u,\"afterimagePassesChecked\":%u,\"unavailablePassesChecked\":%u,\"windowsCreated\":0,\"draws\":0}\n", programsChecked, clonesChecked, failureCases, lightPassesChecked, nativePassesChecked, afterimagePassesChecked, unavailablePassesChecked);
        context->ClearState();
        return 0;
    }
    catch (const std::exception& error)
    {
        std::fprintf(stderr, "SourceCharacter shader variant probe: %s\n", error.what());
        return 1;
    }
}
